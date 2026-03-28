/**************************************************************************/
/*  audio_driver_oboe.h                                                   */
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

#ifndef AUDIO_DRIVER_OBOE_H
#define AUDIO_DRIVER_OBOE_H

#include "core/os/mutex.h"
#include "servers/audio_server.h"

#include <oboe/Oboe.h>

class AudioDriverOboe : public AudioDriver, public oboe::AudioStreamDataCallback {
	bool active;
	Mutex mutex;
	bool pause;

	uint32_t buffer_size;
	int32_t *mixdown_buffer;

	oboe::ManagedStream output_stream;
	oboe::ManagedStream input_stream;

	int mix_rate;

	Vector<int16_t> rec_buffer;

	static AudioDriverOboe *s_ad;

	virtual Error capture_init_device();

public:
	// AudioDriver interface.
	virtual const char *get_name() const;
	virtual Error init();
	virtual void start();
	virtual int get_mix_rate() const;
	virtual SpeakerMode get_speaker_mode() const;
	virtual void lock();
	virtual void unlock();
	virtual void finish();
	virtual void set_pause(bool p_pause);

	virtual Error capture_start();
	virtual Error capture_stop();

	// Oboe AudioStreamDataCallback.
	oboe::DataCallbackResult onAudioReady(
			oboe::AudioStream *p_stream,
			void *p_audio_data,
			int32_t p_num_frames);

	AudioDriverOboe();
	~AudioDriverOboe();
};

#endif // AUDIO_DRIVER_OBOE_H
