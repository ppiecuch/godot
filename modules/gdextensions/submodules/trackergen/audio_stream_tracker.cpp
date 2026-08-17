/**************************************************************************/
/*  audio_stream_tracker.cpp                                              */
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

#include "audio_stream_tracker.h"

#include "core/math/math_funcs.h"
#include "core/os/file_access.h"
#include "servers/audio_server.h"

/// AudioStreamTracker

float AudioStreamTracker::get_sample_rate() const {
	if (render_rate > 0) {
		return render_rate; // offline render override
	}
	AudioServer *as = AudioServer::get_singleton();
	return as ? (float)as->get_mix_rate() : 44100.0f;
}

Error AudioStreamTracker::save_to_wav(const String &p_path, float p_seconds, int p_sample_rate) {
	const double sr = (p_sample_rate > 0) ? (double)p_sample_rate : 44100.0;
	ERR_FAIL_COND_V(p_seconds <= 0.0f, ERR_INVALID_PARAMETER);

	Error err;
	FileAccess *f = FileAccess::open(p_path, FileAccess::WRITE, &err);
	ERR_FAIL_COND_V_MSG(!f, err, "trackergen: cannot open for writing: " + p_path);

	// WAV header (16-bit PCM stereo); sizes patched after we know the count.
	f->store_buffer((const uint8_t *)"RIFF", 4);
	f->store_32(0); // RIFF size (patched)
	f->store_buffer((const uint8_t *)"WAVE", 4);
	f->store_buffer((const uint8_t *)"fmt ", 4);
	f->store_32(16); // fmt chunk size
	f->store_16(1); // PCM
	f->store_16(2); // channels
	f->store_32((uint32_t)sr); // sample rate
	f->store_32((uint32_t)(sr * 2 * 2)); // byte rate
	f->store_16(4); // block align
	f->store_16(16); // bits per sample
	f->store_buffer((const uint8_t *)"data", 4);
	const size_t data_size_pos = f->get_position();
	f->store_32(0); // data size (patched)

	// Deterministic offline render through the normal playback path.
	render_rate = (real_t)sr;
	Ref<AudioStreamPlaybackTracker> pb;
	pb.instance();
	pb->stream = Ref<AudioStreamTracker>(this);
	pb->start(0);

	const int total = (int)(p_seconds * sr);
	const int BLK = 512;
	AudioFrame block[BLK];
	uint32_t data_bytes = 0;
	int written = 0;
	while (written < total && pb->is_playing()) {
		const int n = MIN(BLK, total - written);
		pb->mix(block, 1.0, n);
		for (int i = 0; i < n; i++) {
			int l = (int)(CLAMP(block[i].l, -1.0f, 1.0f) * 32767.0f);
			int r = (int)(CLAMP(block[i].r, -1.0f, 1.0f) * 32767.0f);
			f->store_16((uint16_t)(int16_t)l);
			f->store_16((uint16_t)(int16_t)r);
			data_bytes += 4;
		}
		written += n;
	}
	pb->stop();
	render_rate = 0;

	// Patch sizes.
	f->seek(4);
	f->store_32(36 + data_bytes);
	f->seek(data_size_pos);
	f->store_32(data_bytes);
	f->close();
	memdelete(f);
	return OK;
}

Ref<AudioStreamPlayback> AudioStreamTracker::instance_playback() {
	Ref<AudioStreamPlaybackTracker> pb;
	pb.instance();
	pb->stream = Ref<AudioStreamTracker>(this);
	return pb;
}

String AudioStreamTracker::get_stream_name() const {
	return "Tracker Stream";
}

float AudioStreamTracker::get_length() const {
	return 0; // endless / loop-driven
}

void AudioStreamTracker::queue_sfx(int p_index, float p_freq, float p_velocity) {
	SfxTrigger e;
	e.inst_index = p_index;
	e.freq = p_freq;
	e.velocity = p_velocity;
	sfx_ring.push(e); // lock-free; silently dropped if the ring is full
}

void AudioStreamTracker::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_song", "song"), &AudioStreamTracker::set_song);
	ClassDB::bind_method(D_METHOD("get_song"), &AudioStreamTracker::get_song);
	ClassDB::bind_method(D_METHOD("set_kit", "kit"), &AudioStreamTracker::set_kit);
	ClassDB::bind_method(D_METHOD("get_kit"), &AudioStreamTracker::get_kit);
	ClassDB::bind_method(D_METHOD("set_test_instrument", "index"), &AudioStreamTracker::set_test_instrument);
	ClassDB::bind_method(D_METHOD("get_test_instrument"), &AudioStreamTracker::get_test_instrument);
	ClassDB::bind_method(D_METHOD("set_test_frequency", "hz"), &AudioStreamTracker::set_test_frequency);
	ClassDB::bind_method(D_METHOD("get_test_frequency"), &AudioStreamTracker::get_test_frequency);
	ClassDB::bind_method(D_METHOD("set_sfx_kit", "kit"), &AudioStreamTracker::set_sfx_kit);
	ClassDB::bind_method(D_METHOD("get_sfx_kit"), &AudioStreamTracker::get_sfx_kit);
	ClassDB::bind_method(D_METHOD("queue_sfx", "index", "freq", "velocity"), &AudioStreamTracker::queue_sfx, DEFVAL(440.0), DEFVAL(1.0));
	ClassDB::bind_method(D_METHOD("save_to_wav", "path", "seconds", "sample_rate"), &AudioStreamTracker::save_to_wav, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("set_voices", "count"), &AudioStreamTracker::set_voices);
	ClassDB::bind_method(D_METHOD("get_voices"), &AudioStreamTracker::get_voices);
	ClassDB::bind_method(D_METHOD("set_tension", "tension"), &AudioStreamTracker::set_tension);
	ClassDB::bind_method(D_METHOD("get_tension"), &AudioStreamTracker::get_tension);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "song", PROPERTY_HINT_RESOURCE_TYPE, "TrackerSong"), "set_song", "get_song");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "kit", PROPERTY_HINT_RESOURCE_TYPE, "TrackerKit"), "set_kit", "get_kit");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "sfx_kit", PROPERTY_HINT_RESOURCE_TYPE, "TrackerKit"), "set_sfx_kit", "get_sfx_kit");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "test_instrument"), "set_test_instrument", "get_test_instrument");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "test_frequency", PROPERTY_HINT_RANGE, "20,4000,0.1"), "set_test_frequency", "get_test_frequency");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "voices", PROPERTY_HINT_RANGE, "1,16,1"), "set_voices", "get_voices");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "tension", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_tension", "get_tension");
}

AudioStreamTracker::AudioStreamTracker() {
	test_instrument = 0;
	test_frequency = 220.0;
	voices = 8;
	tension = 0.0f;
	render_rate = 0;
}

/// AudioStreamPlaybackTracker

void AudioStreamPlaybackTracker::_init_sequencer() {
	order_pos = 0;
	cur_step = 0;
	total_steps = 0;
	step_countdown = 0.0; // fire the first step immediately
	samples_per_step = sample_rate * 0.25; // placeholder; recomputed per step
}

void AudioStreamPlaybackTracker::_do_step() {
	Ref<TrackerSong> song = stream->get_song();
	if (song.is_null()) {
		active = false;
		return;
	}

	// Recompute step duration each step so tempo changes take effect smoothly.
	// P2 mod-route: tension -> tempo when the song opts in.
	double bpm;
	if (song->get_tempo_reacts()) {
		const double tmin = song->get_tempo_min();
		const double tmax = song->get_tempo_max();
		bpm = tmin + (tmax - tmin) * (double)stream->get_tension();
	} else {
		bpm = song->get_tempo_bpm();
	}
	bpm = MAX(1.0, bpm);
	const int spb = song->get_steps_per_beat();
	samples_per_step = sample_rate * 60.0 / (bpm * spb);

	Ref<ChordProgression> chords = song->get_chords();
	Ref<TrackerKit> kit = song->get_kit();

	const int pidx = song->pattern_index_at_order(order_pos);
	Ref<TrackerPattern> pat = (pidx >= 0 && pidx < song->get_pattern_count()) ? song->get_pattern(pidx) : Ref<TrackerPattern>();

	if (pat.is_valid()) {
		const int steps_per_bar = MAX(1, song->get_steps_per_bar());
		const int bar = total_steps / steps_per_bar;
		const int chord_index = chords.is_valid() ? chords->chord_index_at_bar(bar) : 0;

		const Vector<SeqEvent> &evs = pat->events_at(cur_step);
		for (int i = 0; i < evs.size(); i++) {
			const SeqEvent &e = evs[i];
			if (e.note_type == TrackerPattern::NOTE_OFF) {
				vm.note_off_lane(e.lane);
				continue;
			}
			float freq;
			if (chords.is_valid()) {
				freq = chords->resolve_to_freq(chord_index, e.note_type, e.note_index, e.octave);
			} else {
				freq = ChordProgression::midi_to_freq(e.note_index);
			}
			Ref<TrackerInstrument> inst;
			if (kit.is_valid() && e.instrument >= 0 && e.instrument < kit->get_instrument_count()) {
				inst = kit->get_instrument(e.instrument);
			}
			if (inst.is_valid()) {
				vm.note_on(inst, freq, (float)sample_rate, e.volume, e.lane);
			}
		}
	}

	// Advance the step / order cursor.
	total_steps++;
	cur_step++;
	const int pat_steps = pat.is_valid() ? pat->get_steps() : 16;
	if (cur_step >= pat_steps) {
		cur_step = 0;
		order_pos++;
		if (order_pos >= song->get_order_length()) {
			if (song->get_loop()) {
				order_pos = 0;
				loops++;
			} else {
				active = false;
			}
		}
	}
}

void AudioStreamPlaybackTracker::mix(AudioFrame *p_buffer, float p_rate_scale, int p_frames) {
	// Start from silence; voices sum into the buffer.
	for (int i = 0; i < p_frames; i++) {
		p_buffer[i] = AudioFrame(0, 0);
	}
	if (!active || stream.is_null()) {
		return;
	}

	const float tension = stream->get_tension();

	// Drain queued SFX (both modes): trigger one-shots into the shared voice pool.
	{
		Ref<TrackerKit> sfxk = stream->get_sfx_kit();
		if (sfxk.is_valid()) {
			SfxTrigger e;
			while (stream->_get_sfx_ring().pop(e)) {
				if (e.inst_index >= 0 && e.inst_index < sfxk->get_instrument_count()) {
					Ref<TrackerInstrument> inst = sfxk->get_instrument(e.inst_index);
					if (inst.is_valid()) {
						vm.note_on(inst, e.freq, (float)sample_rate, e.velocity, -1); // lane -1 = free voice
					}
				}
			}
		}
	}

	if (song_mode) {
		int off = 0;
		while (off < p_frames && active) {
			if (step_countdown <= 0.0) {
				_do_step();
				step_countdown += samples_per_step;
				if (step_countdown <= 0.0) {
					step_countdown = 1.0; // guard against pathological tempo
				}
			}
			int n = p_frames - off;
			const int until_step = (int)Math::ceil(step_countdown);
			if (until_step >= 1 && until_step < n) {
				n = until_step;
			}
			vm.render(p_buffer + off, n, tension);
			step_countdown -= n;
			off += n;
		}
		position_frames += p_frames;
	} else {
		// P0 test path: keep a single instrument sounding.
		vm.render(p_buffer, p_frames, tension);
		position_frames += p_frames;
		if (vm.active_count() == 0) {
			start(0); // retrigger the test note
		}
	}
}

void AudioStreamPlaybackTracker::start(float p_from_pos) {
	active = false;
	ERR_FAIL_COND(stream.is_null());
	sample_rate = stream->get_sample_rate();
	position_frames = p_from_pos * sample_rate;

	vm.configure(stream->get_voices());

	// Bake the SFX kit up front (never on the audio thread).
	Ref<TrackerKit> sfxk = stream->get_sfx_kit();
	if (sfxk.is_valid()) {
		sfxk->ensure_baked();
	}

	Ref<TrackerSong> song = stream->get_song();
	if (song.is_valid()) {
		song->ensure_baked(); // bakes kit + patterns off the audio thread
		song_mode = true;
		_init_sequencer();
		active = true;
		loops = 0;
		return;
	}

	// Test mode (P0): play one instrument from the kit as a sustained note.
	song_mode = false;
	Ref<TrackerKit> kit = stream->get_kit();
	ERR_FAIL_COND_MSG(kit.is_null(), "AudioStreamTracker has neither a song nor a kit assigned.");
	const int idx = stream->get_test_instrument();
	ERR_FAIL_INDEX_MSG(idx, kit->get_instrument_count(), "test_instrument index out of range.");
	Ref<TrackerInstrument> inst = kit->get_instrument(idx);
	ERR_FAIL_COND(inst.is_null());
	inst->ensure_baked();
	vm.note_on(inst, stream->get_test_frequency(), (float)sample_rate, 1.0f, 0);
	active = true;
	loops = 0;
}

void AudioStreamPlaybackTracker::stop() {
	active = false;
	vm.reset();
}

bool AudioStreamPlaybackTracker::is_playing() const {
	return active;
}

int AudioStreamPlaybackTracker::get_loop_count() const {
	return loops;
}

float AudioStreamPlaybackTracker::get_playback_position() const {
	return (float)(position_frames / (sample_rate > 0 ? sample_rate : 44100.0));
}

void AudioStreamPlaybackTracker::seek(float p_time) {
	position_frames = p_time * (sample_rate > 0 ? sample_rate : 44100.0);
}

float AudioStreamPlaybackTracker::get_length() const {
	return 0;
}

AudioStreamPlaybackTracker::AudioStreamPlaybackTracker() {
	active = false;
	loops = 0;
	position_frames = 0;
	sample_rate = 44100.0;
	song_mode = false;
	samples_per_step = 0;
	step_countdown = 0;
	order_pos = 0;
	cur_step = 0;
	total_steps = 0;
}
