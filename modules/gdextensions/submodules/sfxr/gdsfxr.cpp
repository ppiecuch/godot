/*************************************************************************/
/*  gdsfxr.cpp                                                           */
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

#define GAUDIO_NO_ERROR 0
#define GAUDIO_CANNOT_OPEN_FILE -1
#define GAUDIO_UNRECOGNIZED_FORMAT -2
#define GAUDIO_ERROR_WHILE_READING -3
#define GAUDIO_UNSUPPORTED_FORMAT -4

#define g_id unsigned int
#define gaudio_Error int

#define LOG1(a) std::cout << a << "\n";
#define LOG2(a, b) std::cout << a << b << "\n";
#define LOG3(a, b, c) std::cout << a << b << c << "\n";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <iostream>
#include <map>

#include "gdsfxr.h"
#include "retrosfxvoice.h"

#include "core/os/file_access.h"

/// AudioStreamSfxr

void AudioStreamSfxr::_update_voice() {
	struct SampleBuffer : public BufferCallback {
		PoolRealArray data;

		virtual void append_sample(float sample) { data.push_back(sample); }
		size_t samples() const { return data.size(); }
	};

	SampleBuffer buffer;
	sfx_voice.Play();
	while (sfx_voice.IsActive()) {
		sfx_voice.Render(256, &buffer);
	}

	_cache = buffer.data;
	_dirty = false;
}

void AudioStreamSfxr::from_file(const String p_file) {
	if (!sfx_voice.LoadSettings(p_file.utf8().get_data())) {
		ERR_PRINT("Failed to load sfx settings from " + p_file);
	}
}

void AudioStreamSfxr::from_dict(const Dictionary &p_dict) {
}

Error AudioStreamSfxr::save_to_wav(const String &p_path, int quality, int sample_size) {
	ERR_FAIL_COND_V_MSG(sample_size != 8 && sample_size != 16, ERR_PARAMETER_RANGE_ERROR, "Invalid wave bits size (8/16)");
	ERR_FAIL_COND_V_MSG(quality != 11025 && quality != 22050 && quality != 44100, ERR_PARAMETER_RANGE_ERROR, "Invalid wave freq. size (11025/22050/44100)");

	if (!sfx_voice.ExportWav(p_path.utf8().get_data(), sample_size, quality)) {
		WARN_PRINT("Export to wav file failed: " + p_path);
		return ERR_FILE_CANT_WRITE;
	}

	return OK;
}

int AudioStreamSfxr::fill(AudioFrame *p_buffer, int p_frames, int p_from) {
	if (_dirty) {
		const_cast<AudioStreamSfxr *>(this)->_update_voice();
	}

	ERR_FAIL_COND_V(p_from >= _cache.size(), 0);

	for (int p = p_from; p < p_from + p_frames; p++) {
		if (p == _cache.size()) {
			return p - p_from;
		}
		const float sample = _cache[p];
		*p_buffer++ = AudioFrame(sample, sample);
	}

	return p_frames;
}

float AudioStreamSfxr::get_length() const {
	if (_dirty) {
		const_cast<AudioStreamSfxr *>(this)->_update_voice();
	}
	return sfx_voice.GetVoiceLengthInSamples() / 44100.0;
}

Ref<AudioStreamPlayback> AudioStreamSfxr::instance_playback() {
	Ref<AudioStreamPlaybackSfxr> sfx;

	sfx.instance();
	sfx->sfx_stream = Ref<AudioStreamSfxr>(this);

	return sfx;
}

String AudioStreamSfxr::get_stream_name() const {
	return "SFXR Stream";
}

AudioStreamSfxr::AudioStreamSfxr() {
	_dirty = true;
	loop = false;
	loop_offset = 0;
	sfx_voice.ResetParams();
}

/// AudioStreamPlaybackSfxr

void AudioStreamPlaybackSfxr::mix(AudioFrame *p_buffer, float p_rate_scale, int p_frames) {
	ERR_FAIL_COND(!active);

	int filled = sfx_stream->fill(p_buffer, p_frames, sample_position);
	int todo = p_frames - filled;

	sample_position += filled;

	if (todo) {
		//end of file!
		if (sfx_stream->loop) {
			do {
				seek(sfx_stream->loop_offset);
				filled = sfx_stream->fill(p_buffer, p_frames, sample_position);
				todo = p_frames - filled;
				sample_position += filled;
			} while (todo > 0);
			loops++;
		} else {
			for (int i = filled; i < p_frames; i++) {
				p_buffer[i] = AudioFrame(0, 0);
			}
			active = false;
		}
	}
}

void AudioStreamPlaybackSfxr::start(float p_from_pos) {
	active = true;
	loops = 0;
	seek(p_from_pos);
}
void AudioStreamPlaybackSfxr::stop() {
	active = false;
}

bool AudioStreamPlaybackSfxr::is_playing() const {
	return active;
}

int AudioStreamPlaybackSfxr::get_loop_count() const {
	return loops;
}

float AudioStreamPlaybackSfxr::get_playback_position() const {
	return sample_position / sfx_stream->get_sample_rate();
}

void AudioStreamPlaybackSfxr::seek(float p_time) {
	if (!active) {
		return;
	}
	if (p_time >= sfx_stream->get_length()) {
		p_time = 0;
	}
	sample_position = sfx_stream->get_sample_rate() * p_time;
}

float AudioStreamPlaybackSfxr::get_length() const {
	return sfx_stream->get_length();
}

AudioStreamPlaybackSfxr::AudioStreamPlaybackSfxr() {
	active = false;
	loops = 0;
	sample_position = 0;
}

/// Sfx resource importer

String ResourceImporterSfxr::get_preset_name(int p_idx) const {
	return String();
}

void ResourceImporterSfxr::get_import_options(List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "edit/loop"), false));
	r_options->push_back(ImportOption(PropertyInfo(Variant::REAL, "edit/loop_offset"), 0));
}

bool ResourceImporterSfxr::get_option_visibility(const String &p_option, const Map<StringName, Variant> &p_options) const {
	return true;
}

String ResourceImporterSfxr::get_importer_name() const {
	return "SFXR";
}

String ResourceImporterSfxr::get_visible_name() const {
	return "RetroSoundFX";
}

void ResourceImporterSfxr::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("sfx");
}

String ResourceImporterSfxr::get_save_extension() const {
	return "res";
}

String ResourceImporterSfxr::get_resource_type() const {
	return "AudioStreamSfxr";
}

int ResourceImporterSfxr::get_preset_count() const {
	return 0;
}

Error ResourceImporterSfxr::import(const String &p_source_file, const String &p_save_path, const Map<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata) {
	const bool loop = p_options["edit/loop"];
	const float loop_offset = p_options["edit/loop_offset"];

	Ref<AudioStreamSfxr> sfx_stream;
	sfx_stream.instance();

	sfx_stream->from_file(p_source_file);
	sfx_stream->set_loop(loop);
	sfx_stream->set_loop_offset(loop_offset);

	return ResourceSaver::save(p_save_path + ".res", sfx_stream);
}
