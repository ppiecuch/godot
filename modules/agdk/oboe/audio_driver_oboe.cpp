/**************************************************************************/
/*  audio_driver_oboe.cpp                                                 */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "audio_driver_oboe.h"

#include "core/os/os.h"
#include "core/print_string.h"
#include "core/project_settings.h"

AudioDriverOboe *AudioDriverOboe::s_ad = nullptr;

const char *AudioDriverOboe::get_name() const {
	return "Oboe";
}

Error AudioDriverOboe::init() {
	active = false;
	mix_rate = GLOBAL_DEF("audio/mix_rate", 44100);

	oboe::AudioStreamBuilder builder;
	builder.setDirection(oboe::Direction::Output)
			->setPerformanceMode(oboe::PerformanceMode::LowLatency)
			->setSharingMode(oboe::SharingMode::Exclusive)
			->setFormat(oboe::AudioFormat::I16)
			->setChannelCount(oboe::ChannelCount::Stereo)
			->setSampleRate(mix_rate)
			->setDataCallback(this);

	oboe::Result result = builder.openManagedStream(output_stream);
	if (result != oboe::Result::OK) {
		print_line("AGDK Oboe: Failed to open output stream: " + String(oboe::convertToText(result)));
		return ERR_CANT_OPEN;
	}

	// Update mix rate to what the stream actually gave us.
	mix_rate = output_stream->getSampleRate();

	// Allocate mixdown buffer.
	int frames_per_buffer = output_stream->getFramesPerBurst();
	buffer_size = frames_per_buffer * 2; // stereo
	mixdown_buffer = memnew_arr(int32_t, buffer_size);

	s_ad = this;

	print_line("AGDK Oboe: Audio initialized at " + itos(mix_rate) + " Hz, " +
			itos(frames_per_buffer) + " frames/burst.");
	return OK;
}

void AudioDriverOboe::start() {
	if (output_stream) {
		oboe::Result result = output_stream->requestStart();
		if (result == oboe::Result::OK) {
			active = true;
		} else {
			print_line("AGDK Oboe: Failed to start output stream.");
		}
	}
}

int AudioDriverOboe::get_mix_rate() const {
	return mix_rate;
}

AudioDriver::SpeakerMode AudioDriverOboe::get_speaker_mode() const {
	return SPEAKER_MODE_STEREO;
}

void AudioDriverOboe::lock() {
	mutex.lock();
}

void AudioDriverOboe::unlock() {
	mutex.unlock();
}

void AudioDriverOboe::finish() {
	if (output_stream) {
		output_stream->requestStop();
		output_stream->close();
		output_stream.reset();
	}
	if (input_stream) {
		input_stream->requestStop();
		input_stream->close();
		input_stream.reset();
	}

	if (mixdown_buffer) {
		memdelete_arr(mixdown_buffer);
		mixdown_buffer = nullptr;
	}

	active = false;
	s_ad = nullptr;

	print_line("AGDK Oboe: Audio finished.");
}

void AudioDriverOboe::set_pause(bool p_pause) {
	pause = p_pause;
	if (output_stream) {
		if (p_pause) {
			output_stream->requestPause();
		} else {
			output_stream->requestStart();
		}
	}
}

oboe::DataCallbackResult AudioDriverOboe::onAudioReady(
		oboe::AudioStream *p_stream,
		void *p_audio_data,
		int32_t p_num_frames) {
	if (pause) {
		memset(p_audio_data, 0, p_num_frames * 2 * sizeof(int16_t));
		return oboe::DataCallbackResult::Continue;
	}

	MutexLock lock(mutex);

	// Mix audio into our 32-bit buffer, then convert to 16-bit.
	int frames = p_num_frames;
	int total_samples = frames * 2; // stereo

	audio_server_process(frames, mixdown_buffer);

	int16_t *out = static_cast<int16_t *>(p_audio_data);
	for (int i = 0; i < total_samples; i++) {
		out[i] = CLAMP(mixdown_buffer[i] >> 16, -32768, 32767);
	}

	return oboe::DataCallbackResult::Continue;
}

Error AudioDriverOboe::capture_init_device() {
	oboe::AudioStreamBuilder builder;
	builder.setDirection(oboe::Direction::Input)
			->setPerformanceMode(oboe::PerformanceMode::LowLatency)
			->setSharingMode(oboe::SharingMode::Shared)
			->setFormat(oboe::AudioFormat::I16)
			->setChannelCount(oboe::ChannelCount::Mono)
			->setSampleRate(mix_rate);

	oboe::Result result = builder.openManagedStream(input_stream);
	if (result != oboe::Result::OK) {
		print_line("AGDK Oboe: Failed to open input stream: " + String(oboe::convertToText(result)));
		return ERR_CANT_OPEN;
	}
	return OK;
}

Error AudioDriverOboe::capture_start() {
	Error err = capture_init_device();
	if (err != OK) {
		return err;
	}
	if (input_stream) {
		oboe::Result result = input_stream->requestStart();
		if (result != oboe::Result::OK) {
			return ERR_CANT_OPEN;
		}
	}
	return OK;
}

Error AudioDriverOboe::capture_stop() {
	if (input_stream) {
		input_stream->requestStop();
		input_stream->close();
		input_stream.reset();
	}
	return OK;
}

AudioDriverOboe::AudioDriverOboe() {
	active = false;
	pause = false;
	buffer_size = 0;
	mixdown_buffer = nullptr;
	mix_rate = 44100;
}

AudioDriverOboe::~AudioDriverOboe() {
	finish();
}
