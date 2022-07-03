/*************************************************************************/
/*  gd_videodecoder.cpp                                                  */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#include "packet_queue.h"
#include "string_set.h"

#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <mach/mach_time.h>
#endif

#ifdef __psp2__
#include <psp2/rtc.h>
#endif

#include "core/print_string.h"
#include "core/variant.h"
#include "gdnative/gdnative.h"
#include "gdnative_api_struct.gen.h"
#include "videodecoder/godot_videodecoder.h"

// Reference:
// ----------
// https://github.com/jasonchuang/CameraStreamer/blob/master/jni/video_api.c

// TODO: is this sample rate defined somewhere in the godot api etc?
#define AUDIO_MIX_RATE 22050

enum POSITION_TYPE { POS_V_PTS,
	POS_TIME,
	POS_A_TIME
};

typedef struct videodecoder_data_struct {
	godot_object *instance; // Don't clean
	AVIOContext *io_ctx;
	AVFormatContext *format_ctx;
	AVCodecContext *vcodec_ctx;
	AVFrame *frame_yuv;
	AVFrame *frame_rgb;

	struct SwsContext *sws_ctx;
	uint8_t *frame_buffer;

	int videostream_idx;
	int frame_buffer_size;
	PoolByteArray unwrapped_frame;
	real_t time;

	double audio_time;
	double diff_tolerance;

	int audiostream_idx;
	AVCodecContext *acodec_ctx;
	bool acodec_open;
	AVFrame *audio_frame;
	void *mix_udata;

	int num_decoded_samples;
	float *audio_buffer;
	int audio_buffer_pos;

	SwrContext *swr_ctx;

	PacketQueue *audio_packet_queue;
	PacketQueue *video_packet_queue;

	unsigned long drop_frame;
	unsigned long total_frame;

	double seek_time;

	enum POSITION_TYPE position_type;
	uint8_t *io_buffer;
	godot_bool vcodec_open;
	godot_bool input_open;
	bool frame_unwrapped;

} videodecoder_data_struct;

static const godot_int IO_BUFFER_SIZE = 512 * 1024; // File reading buffer of 512 KiB
static const godot_int AUDIO_BUFFER_MAX_SIZE = 192000;

static const godot_gdnative_ext_nativescript_api_struct *nativescript_api = nullptr;
static const godot_gdnative_ext_nativescript_1_1_api_struct *nativescript_api_1_1 = nullptr;
static const godot_gdnative_ext_videodecoder_api_struct *videodecoder_api = nullptr;

extern const godot_gdnative_core_api_struct api_struct;
extern const godot_videodecoder_interface_gdnative plugin_interface;

static const char *plugin_name = "ffmpeg_videoplayer";
static int num_supported_ext = 0;
static const char **supported_ext = nullptr;

/// Clock Setup function (used by get_ticks_usec)
static uint64_t _clock_start = 0;
#if defined(__APPLE__)
static double _clock_scale = 0;
static void _setup_clock() {
	mach_timebase_info_data_t info;
	if (KERN_SUCCESS == mach_timebase_info(&info)) {
		_clock_scale = ((double)info.numer / (double)info.denom) / 1000.0;
		_clock_start = mach_absolute_time() * _clock_scale;
	}
}
#elif defined(__psp2__)
static uint64_t _tick_freq = 0;
static void _setup_clock() {
	_tick_freq = 1000000L / sceRtcGetTickResolution();
	if (!sceRtcGetCurrentTick((SceRtcTick *)&_clock_start))
		_clock_start *= _tick_freq;
}
#else
#if defined(CLOCK_MONOTONIC_RAW) && !defined(JAVASCRIPT_ENABLED) // This is a better clock on Linux.
#define GODOT_CLOCK CLOCK_MONOTONIC_RAW
#else
#define GODOT_CLOCK CLOCK_MONOTONIC
#endif
static void _setup_clock() {
	struct timespec tv_now = { 0, 0 };
	clock_gettime(GODOT_CLOCK, &tv_now);
	_clock_start = ((uint64_t)tv_now.tv_nsec / 1000L) + (uint64_t)tv_now.tv_sec * 1000000L;
}
#endif

static uint64_t get_ticks_usec() {
#if defined(__APPLE__)
	uint64_t longtime = mach_absolute_time() * _clock_scale;
#elif defined(__psp2__)
	uint64_t longtime = 0;
	if (!sceRtcGetCurrentTick(&longtime))
		longtime *= _tick_freq;
#else
	struct timespec tv_now = { 0, 0 };
	clock_gettime(GODOT_CLOCK, &tv_now);
	uint64_t longtime = ((uint64_t)tv_now.tv_nsec / 1000L) + (uint64_t)tv_now.tv_sec * 1000000L;
#endif
	longtime -= _clock_start;

	return longtime;
}

static uint64_t get_ticks_msec() {
	return get_ticks_usec() / 1000L;
}

#define STRINGIFY(x) #x
#define PROFILE_START(sig, line)                                                         \
	const char __profile_sig__[] = __FILE__ "::" STRINGIFY(line) "::" sig; \
	uint64_t __profile_ticks_start__ = get_ticks_usec()
#define PROFILE_END()                                            \
	if (nativescript_api_1_1)                                    \
	nativescript_api_1_1->godot_nativescript_profiling_add_data( \
			__profile_sig__, get_ticks_usec() - __profile_ticks_start__)

// Cleanup should empty the struct to the point where you can open a new file from.
static void _cleanup(videodecoder_data_struct *data) {
	if (data->audio_packet_queue != nullptr) {
		packet_queue_deinit(data->audio_packet_queue);
		data->audio_packet_queue = nullptr;
	}

	if (data->video_packet_queue != nullptr) {
		packet_queue_deinit(data->video_packet_queue);
		data->video_packet_queue = nullptr;
	}

	if (data->sws_ctx != nullptr) {
		sws_freeContext(data->sws_ctx);
		data->sws_ctx = nullptr;
	}

	if (data->audio_frame != nullptr) {
		av_frame_unref(data->audio_frame);
		data->audio_frame = nullptr;
	}

	if (data->frame_rgb != nullptr) {
		av_frame_unref(data->frame_rgb);
		data->frame_rgb = nullptr;
	}

	if (data->frame_yuv != nullptr) {
		av_frame_unref(data->frame_yuv);
		data->frame_yuv = nullptr;
	}

	if (data->frame_buffer != nullptr) {
		memfree(data->frame_buffer);
		data->frame_buffer = nullptr;
		data->frame_buffer_size = 0;
	}

	if (data->vcodec_ctx != nullptr) {
		if (data->vcodec_open) {
			avcodec_close(data->vcodec_ctx);
			data->vcodec_open = false;
		}
		avcodec_free_context(&data->vcodec_ctx);
		data->vcodec_ctx = nullptr;
	}

	if (data->acodec_ctx != nullptr) {
		if (data->acodec_open) {
			avcodec_close(data->acodec_ctx);
			data->vcodec_open = false;
			avcodec_free_context(&data->acodec_ctx);
			data->acodec_ctx = nullptr;
		}
	}

	if (data->format_ctx != nullptr) {
		if (data->input_open) {
			avformat_close_input(&data->format_ctx);
			data->input_open = false;
		}
		avformat_free_context(data->format_ctx);
		data->format_ctx = nullptr;
	}

	if (data->io_ctx != nullptr) {
		av_freep(&data->io_ctx);
		data->io_ctx = nullptr;
	}

	if (data->io_buffer != nullptr) {
		memfree(data->io_buffer);
		data->io_buffer = nullptr;
	}

	if (data->audio_buffer != nullptr) {
		memfree(data->audio_buffer);
		data->audio_buffer = nullptr;
	}

	if (data->swr_ctx != nullptr) {
		swr_free(&data->swr_ctx);
		data->swr_ctx = nullptr;
	}

	data->time = 0;
	data->seek_time = 0;
	data->diff_tolerance = 0;
	data->videostream_idx = -1;
	data->audiostream_idx = -1;
	data->num_decoded_samples = 0;
	data->audio_buffer_pos = 0;

	data->drop_frame = data->total_frame = 0;
}

static void _unwrap_video_frame(PoolByteArray *dest, AVFrame *frame, int width, int height) {
	int frame_size = width * height * 4;
	if (dest->size() != frame_size) {
		dest->resize(frame_size);
	}

	uint8_t *write_ptr = dest->write().ptr();
	for (int y = 0; y < height; y++) {
		memcpy(write_ptr, frame->data[0] + y * frame->linesize[0], width * 4);
		write_ptr += width * 4;
	}
}

#ifdef DECODE_AUDIO_INTERLEAVE
static int _interleave_audio_frame(float *dest, AVFrame *audio_frame) {
	float **audio_frame_data = (float **)audio_frame->data;
	for (int j = 0, count = 0; j != audio_frame->nb_samples; j++) {
		for (int i = 0; i != audio_frame->channels; i++) {
			dest[count++] = audio_frame_data[i][j];
		}
	}
	return audio_frame->nb_samples;
}
#endif // DECODE_AUDIO_INTERLEAVE

static void _update_extensions() {
	if (num_supported_ext > 0)
		return;

	const AVInputFormat *current_fmt = nullptr;
	set_t *sup_ext_set = nullptr;
#if (LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(58, 9, 100))
	void *iterator_opaque = nullptr;
	while ((current_fmt = av_demuxer_iterate(&iterator_opaque)) != nullptr)
#else
	for (current_fmt = av_iformat_next(NULL); current_fmt; current_fmt = av_iformat_next(current_fmt))
#endif
	{
		if (current_fmt->extensions != nullptr) {
			char *exts = (char *)memalloc(strlen(current_fmt->extensions) + 1);
			strcpy(exts, current_fmt->extensions);
			char *token = strtok(exts, ",");
			while (token != nullptr) {
				sup_ext_set = set_insert(sup_ext_set, token);
				token = strtok(NULL, ", ");
			}
			memfree(exts);
			if (current_fmt->mime_type) {
				char *mime_types = (char *)memalloc(strlen(current_fmt->mime_type) + 1);
				strcpy(mime_types, current_fmt->mime_type);
				char *token = strtok(mime_types, ",");
				// for some reason the webm extension is missing from the format that supports it
				while (token != nullptr) {
					if (strcmp("video/webm", token) == 0) {
						sup_ext_set = set_insert(sup_ext_set, "webm");
					}
					token = strtok(NULL, ",");
				}
				memfree(mime_types);
			}
		}
	}

	list_t ext_list = set_create_list(sup_ext_set);
	num_supported_ext = list_size(&ext_list);
	supported_ext = (const char **)memalloc(sizeof(char *) * num_supported_ext);
	list_node_t *cur_node = ext_list.start;
	int i = 0;
	while (cur_node != nullptr) {
		supported_ext[i] = cur_node->value;
		cur_node->value = nullptr;
		cur_node = cur_node->next;
		i++;
	}
	list_free(&ext_list);
	set_free(sup_ext_set);
}

static inline real_t _avtime_to_sec(int64_t avtime) {
	return avtime / (real_t)AV_TIME_BASE;
}

static void _godot_print(char *msg) {
	print_verbose(msg);
}

static void print_codecs() {
	const AVCodecDescriptor *desc = nullptr;
	char msg[512] = { 0 };
	snprintf(msg, sizeof(msg) - 1, "%s: Supported video codecs:", plugin_name);
	_godot_print(msg);
	while ((desc = avcodec_descriptor_next(desc))) {
		const AVCodec *codec = nullptr;
		bool found = false;
		while ((codec = av_codec_next(codec))) {
			if (codec->id == desc->id && av_codec_is_decoder(codec)) {
				if (!found && (avcodec_find_decoder(desc->id) || avcodec_find_encoder(desc->id))) {
					snprintf(msg, sizeof(msg) - 1, "\t%s%s%s",
							avcodec_find_decoder(desc->id) ? "decode " : "",
							avcodec_find_encoder(desc->id) ? "encode " : "",
							desc->name);
					found = true;
					_godot_print(msg);
				}
				if (strcmp(codec->name, desc->name) != 0) {
					snprintf(msg, sizeof(msg) - 1, "\t  codec: %s", codec->name);
					_godot_print(msg);
				}
			}
		}
	}
}

inline static bool api_ver(godot_gdnative_api_version v, unsigned int want_major, unsigned int want_minor) {
	return v.major == want_major && v.minor == want_minor;
}

void gdffmpeg_init() {
	av_register_all();
	_setup_clock();
	for (int i = 0; i < api_struct.num_extensions; i++) {
		switch (api_struct.extensions[i]->type) {
			case GDNATIVE_EXT_VIDEODECODER: {
				videodecoder_api = (godot_gdnative_ext_videodecoder_api_struct *)api_struct.extensions[i];
				if (videodecoder_api != nullptr) {
					videodecoder_api->godot_videodecoder_register_decoder(&plugin_interface);
				}
			} break;
			case GDNATIVE_EXT_NATIVESCRIPT: {
				nativescript_api = (godot_gdnative_ext_nativescript_api_struct *)api_struct.extensions[i];
				const godot_gdnative_api_struct *ext_next = nativescript_api->next;
				while (ext_next) {
					if (api_ver(ext_next->version, 1, 1)) {
						nativescript_api_1_1 = (godot_gdnative_ext_nativescript_1_1_api_struct *)ext_next;
						break;
					}
					ext_next = ext_next->next;
				}
			} break;
			default:
				break;
		}
	}
	print_codecs();
}

void gdffmpeg_terminate() {
}

void *godot_videodecoder_constructor(godot_object *p_instance) {
	videodecoder_data_struct *data = (videodecoder_data_struct *)memalloc(sizeof(videodecoder_data_struct));

	data->instance = p_instance;

	data->io_buffer = nullptr;
	data->io_ctx = nullptr;

	data->format_ctx = nullptr;
	data->input_open = false;

	data->videostream_idx = -1;
	data->vcodec_ctx = nullptr;
	data->vcodec_open = false;

	data->frame_rgb = nullptr;
	data->frame_yuv = nullptr;
	data->sws_ctx = nullptr;

	data->frame_buffer = nullptr;
	data->frame_buffer_size = 0;

	data->audiostream_idx = -1;
	data->acodec_ctx = nullptr;
	data->acodec_open = false;
	data->audio_frame = nullptr;
	data->audio_buffer = nullptr;

	data->swr_ctx = nullptr;

	data->num_decoded_samples = 0;
	data->audio_buffer_pos = 0;

	data->audio_packet_queue = nullptr;
	data->video_packet_queue = nullptr;

	data->position_type = POS_A_TIME;
	data->time = 0;
	data->audio_time = NAN;

	data->frame_unwrapped = false;

	return data;
}

void godot_videodecoder_destructor(void *p_data) {
	videodecoder_data_struct *data = (videodecoder_data_struct *)p_data;
	_cleanup(data);

	data->instance = nullptr;

	memfree(data);
	data = nullptr; // Not needed, but just to be safe.

	if (num_supported_ext > 0) {
		for (int i = 0; i < num_supported_ext; i++) {
			if (supported_ext[i] != nullptr) {
				memfree((void *)supported_ext[i]);
			}
		}
		memfree(supported_ext);
		num_supported_ext = 0;
	}
}

const char **godot_videodecoder_get_supported_ext(int *p_count) {
	_update_extensions();
	*p_count = num_supported_ext;
	return supported_ext;
}

const char *godot_videodecoder_get_plugin_name(void) {
	return plugin_name;
}

godot_bool godot_videodecoder_open_file(void *p_data, void *file) {
	videodecoder_data_struct *data = (videodecoder_data_struct *)p_data;

	// Clean up the previous file.
	_cleanup(data);

	data->io_buffer = (uint8_t *)memalloc(IO_BUFFER_SIZE * sizeof(uint8_t));
	if (data->io_buffer == nullptr) {
		_cleanup(data);
		WARN_PRINT("Buffer alloc error");
		return GODOT_FALSE;
	}

	godot_int read_bytes = videodecoder_api->godot_videodecoder_file_read(file, data->io_buffer, IO_BUFFER_SIZE);

	if (read_bytes < IO_BUFFER_SIZE) {
		// something went wrong, we should be able to read atleast one buffer length.
		_cleanup(data);
		WARN_PRINT("File less then minimum buffer.");
		return GODOT_FALSE;
	}

	// Rewind to 0
	videodecoder_api->godot_videodecoder_file_seek(file, 0, SEEK_SET);

	// Determine input format
	AVProbeData probe_data;
	probe_data.buf = data->io_buffer;
	probe_data.buf_size = read_bytes;
	probe_data.filename = "";
	probe_data.mime_type = "";

	AVInputFormat *input_format = nullptr;
	input_format = av_probe_input_format(&probe_data, 1);
	if (input_format == nullptr) {
		_cleanup(data);
		char msg[512] = { 0 };
		snprintf(msg, sizeof(msg) - 1, "Format not recognized: %s (%s)", input_format->name, input_format->long_name);
		ERR_PRINT(msg);
		return GODOT_FALSE;
	}
	input_format->flags |= AVFMT_SEEK_TO_PTS;

	data->io_ctx = avio_alloc_context(data->io_buffer, IO_BUFFER_SIZE, 0, file,
			videodecoder_api->godot_videodecoder_file_read, nullptr,
			videodecoder_api->godot_videodecoder_file_seek);
	if (data->io_ctx == nullptr) {
		_cleanup(data);
		ERR_PRINT("IO context alloc error.");
		return GODOT_FALSE;
	}

	data->format_ctx = avformat_alloc_context();
	if (data->format_ctx == nullptr) {
		_cleanup(data);
		ERR_PRINT("Format context alloc error.");
		return GODOT_FALSE;
	}

	data->format_ctx->pb = data->io_ctx;
	data->format_ctx->flags = AVFMT_FLAG_CUSTOM_IO;
	data->format_ctx->iformat = input_format;

	if (avformat_open_input(&data->format_ctx, "", NULL, NULL) != 0) {
		_cleanup(data);
		ERR_PRINT("Input stream failed to open");
		return GODOT_FALSE;
	}
	data->input_open = GODOT_TRUE;

	if (avformat_find_stream_info(data->format_ctx, NULL) < 0) {
		_cleanup(data);
		ERR_PRINT("Could not find stream info.");
		return GODOT_FALSE;
	}

	data->videostream_idx = -1; // should be -1 anyway, just being paranoid.
	data->audiostream_idx = -1;
	// find stream
	for (int i = 0; i < data->format_ctx->nb_streams; i++) {
		if (data->format_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			data->videostream_idx = i;
		} else if (data->format_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
			data->audiostream_idx = i;
		}
	}
	if (data->videostream_idx == -1) {
		_cleanup(data);
		ERR_PRINT("Video Stream not found.");
		return GODOT_FALSE;
	}

	AVCodecContext *vcodec_ctx = data->format_ctx->streams[data->videostream_idx]->codec;

	AVCodec *vcodec = nullptr;
	vcodec = avcodec_find_decoder(vcodec_ctx->codec_id);
	if (vcodec == nullptr) {
		const AVCodecDescriptor *desc = avcodec_descriptor_get(vcodec_ctx->codec_id);
		if (desc) {
			char msg[512] = { 0 };
			snprintf(msg, sizeof(msg) - 1, "Videodecoder %s (%s) not found.", desc->name, desc->long_name);
			WARN_PRINT(msg);
		}
		_cleanup(data);
		return GODOT_FALSE;
	}

	data->vcodec_ctx = avcodec_alloc_context3(vcodec);
	if (data->vcodec_ctx == nullptr) {
		_cleanup(data);
		WARN_PRINT("Videocodec allocation error.");
		return GODOT_FALSE;
	}

	// enable multi-thread decoding based on CPU core count
	data->vcodec_ctx->thread_count = 0;

	if (avcodec_open2(data->vcodec_ctx, vcodec, NULL) < 0) {
		_cleanup(data);
		WARN_PRINT("Videocodec failed to open.");
		return GODOT_FALSE;
	}
	data->vcodec_open = GODOT_TRUE;

	AVCodecContext *acodec_ctx = nullptr;
	AVCodec *acodec = nullptr;
	if (data->audiostream_idx >= 0) {
		acodec_ctx = data->format_ctx->streams[data->audiostream_idx]->codec;

		acodec = avcodec_find_decoder(acodec_ctx->codec_id);
		if (acodec == nullptr) {
			const AVCodecDescriptor *desc = avcodec_descriptor_get(acodec_ctx->codec_id);
			if (desc) {
				char msg[512] = { 0 };
				snprintf(msg, sizeof(msg) - 1, "Audiodecoder %s (%s) not found.", desc->name, desc->long_name);
				WARN_PRINT(msg);
			}
			_cleanup(data);
			return GODOT_FALSE;
		}
		data->acodec_ctx = avcodec_alloc_context3(acodec);
		if (data->acodec_ctx == nullptr) {
			_cleanup(data);
			ERR_PRINT("Audiocodec allocation error.");
			return GODOT_FALSE;
		}

		if (avcodec_open2(data->acodec_ctx, acodec, NULL) < 0) {
			_cleanup(data);
			ERR_PRINT("Audiocodec failed to open.");
			return GODOT_FALSE;
		}
		data->acodec_open = true;

		data->audio_buffer = (float *)memalloc(AUDIO_BUFFER_MAX_SIZE * sizeof(float));
		if (data->audio_buffer == nullptr) {
			_cleanup(data);
			ERR_PRINT("Audio buffer alloc failed.");
			return GODOT_FALSE;
		}

		data->audio_frame = av_frame_alloc();
		if (data->audio_frame == nullptr) {
			_cleanup(data);
			ERR_PRINT("Frame alloc fail.");
			return GODOT_FALSE;
		}

		data->swr_ctx = swr_alloc();
		av_opt_set_int(data->swr_ctx, "in_channel_layout", data->acodec_ctx->channel_layout, 0);
		av_opt_set_int(data->swr_ctx, "out_channel_layout", data->acodec_ctx->channel_layout, 0);
		av_opt_set_int(data->swr_ctx, "in_sample_rate", data->acodec_ctx->sample_rate, 0);
		av_opt_set_int(data->swr_ctx, "out_sample_rate", AUDIO_MIX_RATE, 0);
		av_opt_set_sample_fmt(data->swr_ctx, "in_sample_fmt", AV_SAMPLE_FMT_FLTP, 0);
		av_opt_set_sample_fmt(data->swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0);
		swr_init(data->swr_ctx);
	}

	// NOTE: Align of 1 (I think it is for 32 bit alignment.) Doesn't work otherwise
	data->frame_buffer_size = av_image_get_buffer_size(AV_PIX_FMT_RGB32,
			data->vcodec_ctx->width, data->vcodec_ctx->height, 1);

	data->frame_buffer = (uint8_t *)memalloc(data->frame_buffer_size);
	if (data->frame_buffer == nullptr) {
		_cleanup(data);
		ERR_PRINT("Framebuffer alloc fail.");
		return GODOT_FALSE;
	}

	data->frame_rgb = av_frame_alloc();
	if (data->frame_rgb == nullptr) {
		_cleanup(data);
		ERR_PRINT("Frame alloc fail.");
		return GODOT_FALSE;
	}

	data->frame_yuv = av_frame_alloc();
	if (data->frame_yuv == nullptr) {
		_cleanup(data);
		ERR_PRINT("Frame alloc fail.");
		return GODOT_FALSE;
	}

	int width = data->vcodec_ctx->width;
	int height = data->vcodec_ctx->height;
	if (av_image_fill_arrays(data->frame_rgb->data, data->frame_rgb->linesize, data->frame_buffer,
				AV_PIX_FMT_RGB32, width, height, 1) < 0) {
		_cleanup(data);
		ERR_PRINT("Frame fill.");
		return GODOT_FALSE;
	}

	data->sws_ctx = sws_getContext(width, height, data->vcodec_ctx->pix_fmt,
			width, height, AV_PIX_FMT_RGB0, SWS_BILINEAR,
			NULL, NULL, NULL);
	if (data->sws_ctx == nullptr) {
		_cleanup(data);
		ERR_PRINT("Swscale context not created.");
		return GODOT_FALSE;
	}

	data->time = 0;
	data->num_decoded_samples = 0;

	data->audio_packet_queue = packet_queue_init();
	data->video_packet_queue = packet_queue_init();

	data->drop_frame = data->total_frame = 0;

	return GODOT_TRUE;
}

real_t godot_videodecoder_get_length(const void *p_data) {
	videodecoder_data_struct *data = (videodecoder_data_struct *)p_data;

	if (data->format_ctx == nullptr) {
		WARN_PRINT("Format context is null.");
		return -1;
	}

	return data->format_ctx->streams[data->videostream_idx]->duration * av_q2d(data->format_ctx->streams[data->videostream_idx]->time_base);
}

static bool read_frame(videodecoder_data_struct *data) {
	while (data->video_packet_queue->nb_packets < 8) {
		AVPacket pkt;
		int ret = av_read_frame(data->format_ctx, &pkt);
		if (ret >= 0) {
			if (pkt.stream_index == data->videostream_idx) {
				packet_queue_put(data->video_packet_queue, &pkt);
			} else if (pkt.stream_index == data->audiostream_idx) {
				packet_queue_put(data->audio_packet_queue, &pkt);
			} else {
				av_packet_unref(&pkt);
			}
		} else {
			return false;
		}
	}
	return true;
}

void godot_videodecoder_update(void *p_data, real_t p_delta) {
	PROFILE_START("update", __LINE__);
	videodecoder_data_struct *data = (videodecoder_data_struct *)p_data;
	// during an 'update' make sure to use the video frame's pts timestamp
	// otherwise the godot VideoStreamNative update method
	// won't even try to request a frame since it expects the plugin
	// to use video presentation timestamp as the source of time.

	data->position_type = POS_V_PTS;

	data->time += p_delta;
	// afford one frame worth of slop when decoding
	data->diff_tolerance = p_delta;

	if (!isnan(data->audio_time)) {
		data->audio_time += p_delta;
	}
	read_frame(data);
	PROFILE_END();
}

godot_pool_byte_array *godot_videodecoder_get_videoframe(void *p_data) {
	PROFILE_START("get_videoframe", __LINE__);
	videodecoder_data_struct *data = (videodecoder_data_struct *)p_data;
	AVPacket pkt = { 0 };
	int ret, frame_finished;
	size_t drop_count = 0;
	// to maintain a decent game frame rate
	// don't let frame decoding take more than this number of ms
	uint64_t max_frame_drop_time = 5;
	// but we do need to drop frames, so try to drop at least some frames even if it's a bit slow :(
	size_t min_frame_drop_count = 5;
	uint64_t start = get_ticks_msec();

retry:
#if (LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 106, 102))
	ret = avcodec_receive_frame(data->vcodec_ctx, data->frame_yuv);
	if (ret == AVERROR(EAGAIN)) {
		// need to call avcodedc_send_packet, get a packet from queue to send it
		while (!packet_queue_get(data->video_packet_queue, &pkt)) {
			// WARN_PRINT("video packet queue empty");
			if (!read_frame(data)) {
				PROFILE_END();
				return nullptr;
			}
		}
		ret = avcodec_send_packet(data->vcodec_ctx, &pkt);
		if (ret < 0) {
			char err[512] = { 0 }, msg[768] = { 0 };
			av_strerror(ret, err, sizeof(err) - 1);
			snprintf(msg, sizeof(msg) - 1, "avcodec_send_packet returns %d (%s)", ret, err);
			ERR_PRINT(msg);
			av_packet_unref(&pkt);
			PROFILE_END();
			return nullptr;
		}
		av_packet_unref(&pkt);
		goto retry;
	} else if (ret < 0) {
		char msg[512] = { 0 };
		snprintf(msg, sizeof(msg) - 1, "avcodec_receive_frame returns %d", ret);
		ERR_PRINT(msg);
		PROFILE_END();
		return nullptr;
	}
#else
	while (!packet_queue_get(data->video_packet_queue, &pkt)) {
		//WARN_PRINT("video packet queue empty", "godot_videodecoder_get_videoframe()", __FILE__, __LINE__);
		if (!read_frame(data)) {
			PROFILE_END();
			return nullptr;
		}
	}
	ret = avcodec_decode_video2(data->vcodec_ctx, data->frame_yuv, &frame_finished, &pkt);
	if (ret < 0) {
		char msg[512] = { 0 };
		snprintf(msg, sizeof(msg) - 1, "avcodec_decode_video2 returns %d", ret);
		ERR_PRINT(msg);
		av_packet_unref(&pkt);
		PROFILE_END();
		return nullptr;
	}
	if (!frame_finished) {
		av_packet_unref(&pkt);
		PROFILE_END();
		return nullptr;
	}
#endif
	bool pts_correct = data->frame_yuv->pts == AV_NOPTS_VALUE;
	int64_t pts = pts_correct ? data->frame_yuv->pkt_dts : data->frame_yuv->pts;

	double ts = pts * av_q2d(data->format_ctx->streams[data->videostream_idx]->time_base);

	data->total_frame++;

	// frame successfully decoded here, now if it lags behind too much (diff_tolerance sec)
	// let's discard this frame and get the next frame instead
	bool drop = ts < data->time - data->diff_tolerance;
	uint64_t drop_duration = get_ticks_msec() - start;
	if (drop && drop_duration > max_frame_drop_time && drop_count < min_frame_drop_count) {
		// only discard frames for max_frame_drop_time ms or we'll slow down the game's main thread!
		if (fabs(data->seek_time - data->time) > data->diff_tolerance * 10) {
			char msg[512];
			snprintf(msg, sizeof(msg) - 1, "Slow CPU? Dropped  %d frames for %llums frame dropped: %ld/%ld (%.1f%%) pts=%.1f t=%.1f",
					(int)drop_count,
					drop_duration,
					data->drop_frame,
					data->total_frame,
					100.0 * data->drop_frame / data->total_frame,
					ts, (double)data->time);
			WARN_PRINT(msg);
		}
	} else if (drop) {
		drop_count++;
		data->drop_frame++;
		av_packet_unref(&pkt);
		goto retry;
	}
	if (!drop || fabs(data->seek_time - data->time) > data->diff_tolerance * 2) {
		// Don't overwrite the current frame when dropping frames for performance reasons
		// except when the time is within 2 frames of the most recent seek
		// because we don't want a glitchy 'fast forward' effect when seeking.
		// NOTE: VideoPlayer currently doesnt' ask for a frame when seeking while paused so you'd
		// have to fake it inside godot by unpausing briefly. (see FIG1 below)
		data->frame_unwrapped = true;
		sws_scale(data->sws_ctx, (uint8_t const *const *)data->frame_yuv->data, data->frame_yuv->linesize, 0,
				data->vcodec_ctx->height, data->frame_rgb->data, data->frame_rgb->linesize);
		_unwrap_video_frame(&data->unwrapped_frame, data->frame_rgb, data->vcodec_ctx->width, data->vcodec_ctx->height);
	}
	av_packet_unref(&pkt);

	// hack to get video_stream_gdnative to stop asking for frames.
	// stop trusting video pts until the next time update() is called.
	// this will unblock VideoStreamPlaybackGDNative::update() which
	// keeps calling get_texture() until the time matches
	// we don't need this behavior as we already handle frame skipping internally.
	data->position_type = POS_TIME;
	PROFILE_END();
	return data->frame_unwrapped ? (godot_pool_byte_array *)&data->unwrapped_frame : nullptr;
}

/*
FIG1: how to seek while paused...

var _paused_seeking = 0
func seek_player(value):
	var was_playing = _playing
	if _playing:
		stop()
	_player.stream_position = value

	if was_playing:
		play(value)
		if _player.paused || _paused_seeking > 0:
			_player.paused = false
			_paused_seeking = _paused_seeking + 1
			# yes, it seems like 5 idle frames _is_ the magic number.
			# VideoPlayer gets notified to do NOTIFICATION_INTERNAL_PROCESS on idle frames
			# so this should always work?
			for i in range(5):
				yield(get_tree(), 'idle_frame')
			# WARNING: -= double decrements here somehow?
			_paused_seeking = _paused_seeking - 1
			assert(_paused_seeking >= 0)
			if _paused_seeking == 0:
				_player.paused = true

*/

int godot_videodecoder_get_audio(void *p_data, float *pcm, int pcm_remaining) {
	PROFILE_START("get_audio", __LINE__);
	videodecoder_data_struct *data = (videodecoder_data_struct *)p_data;
	if (data->audiostream_idx < 0) {
		PROFILE_END();
		return 0;
	}
	bool first_frame = true;

	// if playback has just started or just seeked then we enter the audio_reset state.
	// during audio_reset it's important to skip old samples
	// _and_ avoid sending samples from the future until the presentation timestamp syncs up.
	bool audio_reset = isnan(data->audio_time) || data->audio_time > data->time - data->diff_tolerance;

#if (LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 106, 102))
	const int pcm_buffer_size = pcm_remaining;
#endif
	int pcm_offset = 0;

	double p_time = data->audio_frame->pts * av_q2d(data->format_ctx->streams[data->audiostream_idx]->time_base);

	if (audio_reset && data->num_decoded_samples > 0) {
		// don't send any pcm data if the frame hasn't started yet
		if (p_time > data->time) {
			PROFILE_END();
			return 0;
		}
		// skip the any decoded samples if their presentation timestamp is too old
		if (data->time - p_time > data->diff_tolerance) {
			data->num_decoded_samples = 0;
		}
	}

	int sample_count = (pcm_remaining < data->num_decoded_samples) ? pcm_remaining : data->num_decoded_samples;

	if (sample_count > 0) {
		memcpy(pcm, data->audio_buffer + data->acodec_ctx->channels * data->audio_buffer_pos, sizeof(float) * sample_count * data->acodec_ctx->channels);
		pcm_offset += sample_count;
		pcm_remaining -= sample_count;
		data->num_decoded_samples -= sample_count;
		data->audio_buffer_pos += sample_count;
	}
	while (pcm_remaining > 0) {
		if (data->num_decoded_samples <= 0) {
			AVPacket pkt;

			int ret, frame_finished;
#if (LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 106, 102))
		retry_audio:
			ret = avcodec_receive_frame(data->acodec_ctx, data->audio_frame);
			if (ret == AVERROR(EAGAIN)) {
				// need to call avcodec_send_packet, get a packet from queue to send it
				if (!packet_queue_get(data->audio_packet_queue, &pkt)) {
					if (pcm_offset == 0) {
						// if we haven't got any on-time audio yet, then the audio_time counter is meaningless.
						data->audio_time = NAN;
					}
					PROFILE_END();
					return pcm_offset;
				}
				ret = avcodec_send_packet(data->acodec_ctx, &pkt);
				if (ret < 0) {
					char msg[512];
					snprintf(msg, sizeof(msg) - 1, "avcodec_send_packet returns %d", ret);
					ERR_PRINT(msg);
					av_packet_unref(&pkt);
					PROFILE_END();
					return pcm_offset;
				}
				av_packet_unref(&pkt);
				goto retry_audio;
			} else if (ret < 0) {
				char msg[512];
				snprintf(msg, sizeof(msg) - 1, "avcodec_receive_frame returns %d", ret);
				ERR_PRINT(msg);
				PROFILE_END();
				return pcm_buffer_size - pcm_remaining;
			}
#else
			while (!packet_queue_get(data->audio_packet_queue, &pkt)) {
				//WARN_PRINT("video packet queue empty", "godot_videodecoder_get_videoframe()", __FILE__, __LINE__);
				if (!read_frame(data)) {
					PROFILE_END();
					return 0;
				}
			}
			ret = avcodec_decode_video2(data->acodec_ctx, data->audio_frame, &frame_finished, &pkt);
			if (ret < 0) {
				char msg[512] = { 0 };
				snprintf(msg, sizeof(msg) - 1, "avcodec_decode_video2 returns %d", ret);
				ERR_PRINT(msg);
				av_packet_unref(&pkt);
				PROFILE_END();
				return 0;
			}
			if (!frame_finished) {
				av_packet_unref(&pkt);
				PROFILE_END();
				return 0;
			}
#endif
			// only set the audio frame time if this is the first frame we've decoded during this update.
			// any remaining frames are going into a buffer anyways
			p_time = data->audio_frame->pts * av_q2d(data->format_ctx->streams[data->audiostream_idx]->time_base);
			if (first_frame) {
				data->audio_time = p_time;
				first_frame = false;
			}
			// decoded audio ready here
#ifdef DECODE_AUDIO_INTERLEAVE
			data->num_decoded_samples = _interleave_audio_frame(data->audio_buffer, data->audio_frame);
#else
			data->num_decoded_samples = swr_convert(data->swr_ctx, (uint8_t **)&data->audio_buffer, data->audio_frame->nb_samples, (const uint8_t **)data->audio_frame->extended_data, data->audio_frame->nb_samples);
#endif
			data->audio_buffer_pos = 0;
		}
		if (audio_reset) {
			if (data->time - p_time > data->diff_tolerance) {
				// skip samples if the frame time is too far in the past
				data->num_decoded_samples = 0;
			} else if (p_time > data->time) {
				// don't send any pcm data if the first frame hasn't started yet
				data->audio_time = NAN;
				break;
			}
		}
		sample_count = pcm_remaining < data->num_decoded_samples ? pcm_remaining : data->num_decoded_samples;
		if (sample_count > 0) {
			memcpy(pcm + pcm_offset * data->acodec_ctx->channels, data->audio_buffer + data->acodec_ctx->channels * data->audio_buffer_pos, sizeof(float) * sample_count * data->acodec_ctx->channels);
			pcm_offset += sample_count;
			pcm_remaining -= sample_count;
			data->num_decoded_samples -= sample_count;
			data->audio_buffer_pos += sample_count;
		}
	}

	PROFILE_END();
	return pcm_offset;
}

real_t godot_videodecoder_get_playback_position(const void *p_data) {
	videodecoder_data_struct *data = (videodecoder_data_struct *)p_data;

	if (data->format_ctx) {
		bool use_v_pts = data->frame_yuv->pts != AV_NOPTS_VALUE && data->position_type == POS_V_PTS;
		bool use_a_time = data->position_type == POS_A_TIME;
		data->position_type = POS_TIME;

		if (use_v_pts) {
			double pts = (double)data->frame_yuv->pts;
			pts *= av_q2d(data->format_ctx->streams[data->videostream_idx]->time_base);
			return (real_t)pts;
		} else {
			if (!isnan(data->audio_time) && use_a_time) {
				return (real_t)data->audio_time;
			}
			return (real_t)data->time;
		}
	}
	return (real_t)0;
}

static void flush_frames(AVCodecContext *ctx) {
	PROFILE_START("flush_frames", __LINE__);
	/**
	 * from https://www.ffmpeg.org/doxygen/4.1/group__lavc__encdec.html
	 * End of stream situations. These require "flushing" (aka draining) the codec, as the codec might buffer multiple frames or packets internally for performance or out of necessity (consider B-frames). This is handled as follows:
	 * Instead of valid input, send NULL to the avcodec_send_packet() (decoding) or avcodec_send_frame() (encoding) functions. This will enter draining mode.
	 * Call avcodec_receive_frame() (decoding) or avcodec_receive_packet() (encoding) in a loop until AVERROR_EOF is returned. The functions will not return AVERROR(EAGAIN), unless you forgot to enter draining mode.
	 * Before decoding can be resumed again, the codec has to be reset with avcodec_flush_buffers().
	 */
	AVFrame frame = { { 0 } };
	int finished = 0;
#if (LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 106, 102))
	if (avcodec_send_packet(ctx, NULL) <= 0) {
		do {
		} while (avcodec_receive_frame(ctx, &frame) != AVERROR_EOF);
	}
#elif (LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52, 23, 0))
	do {
	} while (avcodec_decode_video2(ctx, &frame, &finished, NULL) > 0);
#else
	do {
	} while (avcodec_decode_video(ctx, &frame, &finished, NULL, 0) > 0);
#endif
	PROFILE_END();
}

void godot_videodecoder_seek(void *p_data, real_t p_time) {
	PROFILE_START("seek", __LINE__);
	videodecoder_data_struct *data = (videodecoder_data_struct *)p_data;
	// Hack to find the end of the video. Really VideoPlayer should expose this!
	if (p_time < 0) {
		p_time = _avtime_to_sec(data->format_ctx->duration);
	}
	int64_t seek_target = p_time * AV_TIME_BASE;
	// seek within 10 seconds of the selected spot.
	int64_t margin = 10 * AV_TIME_BASE;

	// printf("seek(): %fs = %lld\n", p_time, seek_target);
	int ret = avformat_seek_file(data->format_ctx, -1, seek_target - margin, seek_target, seek_target, 0);
	if (ret < 0) {
		WARN_PRINT("avformat_seek_file() can't seek backward?");
		ret = avformat_seek_file(data->format_ctx, -1, seek_target - margin, seek_target, seek_target + margin, 0);
	}
	if (ret < 0) {
		ERR_PRINT("avformat_seek_file() failed");
	} else {
		packet_queue_flush(data->video_packet_queue);
		packet_queue_flush(data->audio_packet_queue);
		flush_frames(data->vcodec_ctx);
		avcodec_flush_buffers(data->vcodec_ctx);
		if (data->acodec_ctx) {
			flush_frames(data->acodec_ctx);
			avcodec_flush_buffers(data->acodec_ctx);
		}
		data->num_decoded_samples = 0;
		data->audio_buffer_pos = 0;
		data->time = p_time;
		data->seek_time = p_time;
		// try to use the audio time as the seek position
		data->position_type = POS_A_TIME;
		data->audio_time = NAN;
	}
	PROFILE_END();
}

/* ---------------------- TODO ------------------------- */

void godot_videodecoder_set_audio_track(void *p_data, godot_int p_audiotrack) {
	// WARN_PRINT("set_audio_track not implemented");
}

godot_int godot_videodecoder_get_channels(const void *p_data) {
	videodecoder_data_struct *data = (videodecoder_data_struct *)p_data;

	if (data->acodec_ctx != nullptr) {
		return data->acodec_ctx->channels;
	}
	return 0;
}

godot_int godot_videodecoder_get_mix_rate(const void *p_data) {
	videodecoder_data_struct *data = (videodecoder_data_struct *)p_data;

	if (data->acodec_ctx != nullptr) {
		return AUDIO_MIX_RATE;
	}
	return 0;
}

godot_vector2 godot_videodecoder_get_texture_size(const void *p_data) {
	videodecoder_data_struct *data = (videodecoder_data_struct *)p_data;
	Vector2 vec;

	if (data->vcodec_ctx != nullptr) {
		vec = Vector2(data->vcodec_ctx->width, data->vcodec_ctx->height);
	}
	return *(godot_vector2 *)&vec;
}

const godot_videodecoder_interface_gdnative plugin_interface = {
	{ GODOTAV_API_MAJOR, GODOTAV_API_MINOR },
	nullptr,
	godot_videodecoder_constructor,
	godot_videodecoder_destructor,
	godot_videodecoder_get_plugin_name,
	godot_videodecoder_get_supported_ext,
	godot_videodecoder_open_file,
	godot_videodecoder_get_length,
	godot_videodecoder_get_playback_position,
	godot_videodecoder_seek,
	godot_videodecoder_set_audio_track,
	godot_videodecoder_update,
	godot_videodecoder_get_videoframe,
	godot_videodecoder_get_audio,
	godot_videodecoder_get_channels,
	godot_videodecoder_get_mix_rate,
	godot_videodecoder_get_texture_size
};
