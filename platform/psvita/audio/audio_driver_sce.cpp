/*************************************************************************/
/*  audio_driver_sce.cpp                                                 */
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

#include "audio_driver_sce.h"

#include "core/os/os.h"
#include "core/project_settings.h"

#include <string.h>

Error AudioDriverSCE::init() {
	active = false;
	thread_exited = false;
	exit_thread = false;
	pcm_open = false;
	samples_in = NULL;

	mix_rate = 44100;
	output_format = SPEAKER_MODE_STEREO;
	channels = 2;

	int latency = GLOBAL_GET("audio/output_latency");
	buffer_size = closest_power_of_2(latency * mix_rate / 1000);

	samples_in = memnew_arr(int32_t, buffer_size * channels);

	thread.start(&thread_func, this);

	return OK;
};

void AudioDriverSCE::thread_func(void *p_udata) {
	int buffer_index = 0;
	AudioDriverSCE *ad = (AudioDriverSCE *)p_udata;

	while (!ad->exit_thread) {
		while (!ad->exit_thread && (!ad->active))
			OS::get_singleton()->delay_usec(10000);

		if (ad->exit_thread)
			break;

		if (ad->active) {
			ad->lock();
			ad->audio_server_process(ad->buffer_size, ad->samples_in);
			ad->unlock();
		}

		buffer_index++;
	}

	ad->thread_exited = true;
};

void AudioDriverSCE::start() {
	active = true;
};

int AudioDriverSCE::get_mix_rate() const {
	return mix_rate;
};

AudioDriver::SpeakerMode AudioDriverSCE::get_speaker_mode() const {
	return output_format;
};

void AudioDriverSCE::lock() {
	mutex.lock();
};

void AudioDriverSCE::unlock() {
	mutex.unlock();
};

void AudioDriverSCE::finish() {
	exit_thread = true;
	if (thread.is_started()) {
		thread.wait_to_finish();
	}

	if (samples_in) {
		memdelete_arr(samples_in);
	};
};

AudioDriverSCE::AudioDriverSCE(){

};

AudioDriverSCE::~AudioDriverSCE(){

};
