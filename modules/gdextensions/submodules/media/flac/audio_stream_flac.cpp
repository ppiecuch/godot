/*************************************************************************/
/*  audio_stream_flac.cpp                                                */
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

#define DR_FLAC_IMPLEMENTATION
#define DR_FLAC_NO_STDIO
#define DR_FLAC_NO_OGG
#include "audio_stream_flac.h"

#include "core/os/file_access.h"

void AudioStreamPlaybackFLAC::_mix_internal(AudioFrame *p_buffer, int p_frames) {
	ERR_FAIL_COND(!active);

	int todo = p_frames;
	int start_buffer = 0;

	while (todo && active) {
		float *p_samples = (float *)AudioServer::get_singleton()->audio_data_alloc(todo * p_flac->channels * sizeof(float));
		float *buffer = (float *)p_buffer;
		if (start_buffer > 0) {
			buffer = (buffer + start_buffer * 2);
		}
		int mixed = drflac_read_pcm_frames_f32(p_flac, todo, p_samples);
		for (int i = 0; i < mixed; i++) {
			buffer[i * 2] = p_samples[i * flac_stream->channels];
			buffer[i * 2 + 1] = p_samples[i * flac_stream->channels + flac_stream->channels - 1];
		}

		todo -= mixed;
		frames_mixed += mixed;

		if (todo) {
			//end of file!
			if (flac_stream->loop) {
				//loop
				seek(flac_stream->loop_offset);
				loops++;
				// we still have buffer to fill, start from this element in the next iteration.
				start_buffer = p_frames - todo;
			} else {
				for (int i = p_frames - todo; i < p_frames; i++) {
					p_buffer[i] = AudioFrame(0, 0);
				}
				active = false;
				todo = 0;
			}
		}
		AudioServer::get_singleton()->audio_data_free(p_samples);
	}
}

float AudioStreamPlaybackFLAC::get_stream_sampling_rate() {
	return flac_stream->sample_rate;
}

void AudioStreamPlaybackFLAC::start(float p_from_pos) {
	active = true;
	seek(p_from_pos);
	loops = 0;
	_begin_resample();
}

void AudioStreamPlaybackFLAC::stop() {
	active = false;
}
bool AudioStreamPlaybackFLAC::is_playing() const {
	return active;
}

int AudioStreamPlaybackFLAC::get_loop_count() const {
	return loops;
}

float AudioStreamPlaybackFLAC::get_playback_position() const {
	return float(frames_mixed) / float(flac_stream->sample_rate);
}
void AudioStreamPlaybackFLAC::seek(float p_time) {
	if (!active) {
		return;
	}
	if (p_time >= flac_stream->get_length()) {
		p_time = 0;
	}
	frames_mixed = flac_stream->sample_rate * p_time;
	drflac_seek_to_pcm_frame(p_flac, frames_mixed);
}

AudioStreamPlaybackFLAC::~AudioStreamPlaybackFLAC() {
	if (p_flac) {
		p_flac = nullptr;
	}
}

Ref<AudioStreamPlayback> AudioStreamFLAC::instance_playback() {
	Ref<AudioStreamPlaybackFLAC> flacs;

	ERR_FAIL_COND_V(data == nullptr, flacs);

	flacs.instance();
	flacs->flac_stream = Ref<AudioStreamFLAC>(this);

	flacs->p_flac = drflac_open_memory(data, data_len);

	flacs->frames_mixed = 0;
	flacs->active = false;
	flacs->loops = 0;

	if (!flacs->p_flac) {
		ERR_FAIL_COND_V(!flacs->p_flac, Ref<AudioStreamPlaybackFLAC>());
	}

	return flacs;
}

String AudioStreamFLAC::get_stream_name() const {
	return ""; //return stream_name;
}

void AudioStreamFLAC::clear_data() {
	if (data) {
		AudioServer::get_singleton()->audio_data_free(data);
		data = NULL;
		data_len = 0;
	}
}

void AudioStreamFLAC::set_data(const PoolVector<uint8_t> &p_data) {
	int src_data_len = p_data.size();

	PoolVector<uint8_t>::Read src_datar = p_data.read();

	drflac *pflac = drflac_open_memory(src_datar.ptr(), src_data_len);

	channels = pflac->channels;
	sample_rate = pflac->sampleRate;
	length = (float(pflac->totalSampleCount) / float(sample_rate)) / float(channels);

	clear_data();

	data = AudioServer::get_singleton()->audio_data_alloc(src_data_len, src_datar.ptr());
	data_len = src_data_len;
}

PoolVector<uint8_t> AudioStreamFLAC::get_data() const {
	PoolVector<uint8_t> vdata;

	if (data_len && data) {
		vdata.resize(data_len);
		{
			PoolVector<uint8_t>::Write w = vdata.write();
			memcpy(w.ptr(), data, data_len);
		}
	}

	return vdata;
}

void AudioStreamFLAC::set_loop(bool p_enable) {
	loop = p_enable;
}

bool AudioStreamFLAC::has_loop() const {
	return loop;
}

void AudioStreamFLAC::set_loop_offset(float p_seconds) {
	loop_offset = p_seconds;
}

float AudioStreamFLAC::get_loop_offset() const {
	return loop_offset;
}

float AudioStreamFLAC::get_length() const {
	return length;
}

void AudioStreamFLAC::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_data", "data"), &AudioStreamFLAC::set_data);
	ClassDB::bind_method(D_METHOD("get_data"), &AudioStreamFLAC::get_data);

	ClassDB::bind_method(D_METHOD("set_loop", "enable"), &AudioStreamFLAC::set_loop);
	ClassDB::bind_method(D_METHOD("has_loop"), &AudioStreamFLAC::has_loop);

	ClassDB::bind_method(D_METHOD("set_loop_offset", "seconds"), &AudioStreamFLAC::set_loop_offset);
	ClassDB::bind_method(D_METHOD("get_loop_offset"), &AudioStreamFLAC::get_loop_offset);

	ADD_PROPERTY(PropertyInfo(Variant::POOL_BYTE_ARRAY, "data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_data", "get_data");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "loop", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_loop", "has_loop");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "loop_offset", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_loop_offset", "get_loop_offset");
}

AudioStreamFLAC::AudioStreamFLAC() {
	data = nullptr;
	data_len = 0;
	length = 0;
	sample_rate = 1;
	channels = 1;
	loop_offset = 0;
	loop = false;
}

AudioStreamFLAC::~AudioStreamFLAC() {
	clear_data();
}
