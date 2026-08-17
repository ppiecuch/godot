/**************************************************************************/
/*  audio_stream_tracker.h                                                */
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

#ifndef AUDIO_STREAM_TRACKER_H
#define AUDIO_STREAM_TRACKER_H

#include "trackergen_kit.h"
#include "trackergen_sfx_ring.h"
#include "trackergen_song.h"
#include "trackergen_voice_manager.h"

#include "servers/audio/audio_stream.h"

#include <atomic>

// The tracker audio stream. Two modes:
//  * song mode  — a full TrackerSong is assigned: the playback runs a sample-
//    accurate step sequencer (patterns x chords x kit) through a shared voice
//    pool with priority stealing.
//  * test mode  — no song, just a kit + test note (the P0 demo path).
// A `tension` scalar (from gameplay) drives per-voice variant selection.
class AudioStreamTracker : public AudioStream {
	GDCLASS(AudioStreamTracker, AudioStream);

	friend class AudioStreamPlaybackTracker;

	Ref<TrackerSong> song; // song mode when valid

	Ref<TrackerKit> kit; // test mode
	int test_instrument;
	real_t test_frequency;

	Ref<TrackerKit> sfx_kit; // SFX share the music voice pool (priority stealing)

	int voices; // size of the shared voice pool

	// Gameplay -> audio control (P2): lock-free. `tension` is an atomic scalar;
	// SFX triggers go through an SPSC ring drained on the audio thread.
	std::atomic<float> tension;
	SfxRing sfx_ring;

	// When > 0, get_sample_rate() returns this instead of the AudioServer rate —
	// used for deterministic offline (faster-than-real-time) rendering.
	real_t render_rate;

protected:
	static void _bind_methods();

public:
	void set_song(const Ref<TrackerSong> &p_song) { song = p_song; }
	Ref<TrackerSong> get_song() const { return song; }

	void set_kit(const Ref<TrackerKit> &p_kit) { kit = p_kit; }
	Ref<TrackerKit> get_kit() const { return kit; }
	void set_test_instrument(int p_i) { test_instrument = p_i; }
	int get_test_instrument() const { return test_instrument; }
	void set_test_frequency(real_t p_hz) { test_frequency = p_hz; }
	real_t get_test_frequency() const { return test_frequency; }

	void set_sfx_kit(const Ref<TrackerKit> &p_kit) { sfx_kit = p_kit; }
	Ref<TrackerKit> get_sfx_kit() const { return sfx_kit; }

	void set_voices(int p_v) { voices = CLAMP(p_v, 1, (int)TrackerVoiceManager::MAX_VOICES); }
	int get_voices() const { return voices; }

	void set_tension(float p_t) { tension.store(CLAMP(p_t, 0.0f, 1.0f), std::memory_order_relaxed); }
	float get_tension() const { return tension.load(std::memory_order_relaxed); }

	// Fire a one-shot from the sfx_kit (game thread). Lock-free; shares the voice
	// pool so a busy fight steals music voices by priority.
	void queue_sfx(int p_index, float p_freq = 440.0f, float p_velocity = 1.0f);
	_FORCE_INLINE_ SfxRing &_get_sfx_ring() { return sfx_ring; }

	float get_sample_rate() const;

	// Offline: render the assigned song (or test note) to a 16-bit stereo WAV,
	// faster than real time. p_seconds is the capture length; p_sample_rate <= 0
	// uses 44100. Great for trailer music / verification.
	Error save_to_wav(const String &p_path, float p_seconds, int p_sample_rate = 0);

	virtual Ref<AudioStreamPlayback> instance_playback();
	virtual String get_stream_name() const;
	virtual float get_length() const;

	AudioStreamTracker();
};

class AudioStreamPlaybackTracker : public AudioStreamPlayback {
	GDCLASS(AudioStreamPlaybackTracker, AudioStreamPlayback);

	friend class AudioStreamTracker;

	Ref<AudioStreamTracker> stream;
	TrackerVoiceManager vm;
	bool active;
	int loops;
	double position_frames;
	double sample_rate;

	// Sequencer state (song mode).
	bool song_mode;
	double samples_per_step;
	double step_countdown; // samples until the next step fires
	int order_pos;
	int cur_step;
	int total_steps; // musical steps elapsed, for bar/chord tracking

	void _init_sequencer();
	void _do_step();

protected:
	virtual void mix(AudioFrame *p_buffer, float p_rate_scale, int p_frames);

public:
	virtual void start(float p_from_pos = 0);
	virtual void stop();
	virtual bool is_playing() const;
	virtual int get_loop_count() const;
	virtual float get_playback_position() const;
	virtual void seek(float p_time);
	virtual float get_length() const;

	AudioStreamPlaybackTracker();
};

#endif // AUDIO_STREAM_TRACKER_H
