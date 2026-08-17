/**************************************************************************/
/*  trackergen_voice.h                                                    */
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

#ifndef TRACKERGEN_VOICE_H
#define TRACKERGEN_VOICE_H

#include "trackergen_instrument.h"

#include "core/math/math_funcs.h"
#include "servers/audio/audio_stream.h" // AudioFrame

// One playing voice: a wavetable reader (one baked cycle) + ADSR + variant
// selection. Not a Godot object; owned by the voice manager. render() is
// real-time-safe (no allocation, no locks).
class TrackerVoice {
	Ref<TrackerInstrument> inst_ref; // keeps baked tables alive
	const TrackerInstrument *inst;

	double phase; // 0 .. tbl_len
	double phase_inc;
	double lfo_phase;
	double lfo_inc;
	float sample_rate;
	float velocity; // 0..1 note velocity (from pattern volume)

	int priority; // cached from instrument at note_on (for stealing)
	int lane; // owning pattern lane (-1 = none), for targeted note_off
	int tbl_len; // baked cycle length

	enum Stage { IDLE,
		ATTACK,
		DECAY,
		SUSTAIN,
		RELEASE };
	int stage;
	float env;

public:
	_FORCE_INLINE_ bool is_active() const { return stage != IDLE && inst != nullptr; }
	_FORCE_INLINE_ bool is_releasing() const { return stage == RELEASE; }
	_FORCE_INLINE_ int get_priority() const { return priority; }
	_FORCE_INLINE_ int get_lane() const { return lane; }

	void note_on(const Ref<TrackerInstrument> &p_inst, float p_freq, float p_sample_rate, float p_velocity = 1.0f, int p_lane = -1) {
		inst_ref = p_inst;
		inst = p_inst.ptr();
		sample_rate = p_sample_rate;
		velocity = p_velocity;
		lane = p_lane;
		if (!inst) {
			stage = IDLE;
			priority = 0;
			return;
		}
		priority = inst->get_priority();
		tbl_len = inst->get_variant(0).size();
		phase = 0.0;
		phase_inc = (double)p_freq * (double)tbl_len / (double)sample_rate; // one cycle per table
		lfo_phase = 0.0;
		lfo_inc = 2.0 * Math_PI * (double)inst->get_lfo_hz() / (double)sample_rate;
		env = 0.0f;
		stage = ATTACK;
	}

	void note_off() {
		if (stage != IDLE) {
			stage = RELEASE;
		}
	}

	void kill() {
		stage = IDLE;
		inst = nullptr;
		inst_ref = Ref<TrackerInstrument>();
	}

	_FORCE_INLINE_ float _advance_env() {
		switch (stage) {
			case ATTACK: {
				const float a = inst->get_attack();
				env += (a > 0.0f) ? (1.0f / (a * sample_rate)) : 1.0f;
				if (env >= 1.0f) {
					env = 1.0f;
					stage = DECAY;
				}
			} break;
			case DECAY: {
				const float d = inst->get_decay();
				const float s = inst->get_sustain();
				env -= (d > 0.0f) ? ((1.0f - s) / (d * sample_rate)) : (1.0f - s);
				if (env <= s) {
					env = s;
					stage = SUSTAIN;
				}
			} break;
			case SUSTAIN:
				env = inst->get_sustain();
				break;
			case RELEASE: {
				const float r = inst->get_release();
				env -= (r > 0.0f) ? (inst->get_sustain() / (r * sample_rate)) : env;
				if (env <= 0.0f) {
					env = 0.0f;
					stage = IDLE;
				}
			} break;
			default:
				env = 0.0f;
				break;
		}
		return env;
	}

	// Sums this voice into p_buffer (does not clear). p_tension in 0..1.
	void render(AudioFrame *p_buffer, int p_frames, float p_tension) {
		if (!is_active()) {
			return;
		}
		const int n = tbl_len;
		const int vc = inst->get_baked_variant_count();
		if (n < 2 || vc < 1) {
			stage = IDLE;
			return;
		}
		const int mode = inst->get_variant_mode();

		// P2: tension can boost per-instrument gain (e.g. an alarm voice).
		const float tgain = 1.0f + inst->get_tension_gain() * p_tension;
		const float gain = inst->get_gain() * velocity * tgain;
		const float pan = inst->get_pan();
		const float gl = gain * 0.5f * (1.0f - pan);
		const float gr = gain * 0.5f * (1.0f + pan);

		for (int f = 0; f < p_frames; f++) {
			// Select the phase-aligned variant table for this sample.
			int idx = 0;
			if (mode == TrackerInstrument::VARIANT_TENSION) {
				idx = CLAMP((int)(p_tension * vc), 0, vc - 1);
			} else if (mode == TrackerInstrument::VARIANT_LFO) {
				const double t = 0.5 * (1.0 + Math::sin(lfo_phase));
				idx = CLAMP((int)(t * vc), 0, vc - 1);
				lfo_phase += lfo_inc;
			}
			const float *tab = inst->get_variant(idx).ptr();
			if (!tab) {
				break;
			}

			const int i0 = (int)phase;
			const float frac = (float)(phase - i0);
			const int i1 = (i0 + 1) % n;
			const float s = tab[i0] * (1.0f - frac) + tab[i1] * frac;

			const float e = _advance_env();
			const float out = s * e;
			p_buffer[f].l += out * gl;
			p_buffer[f].r += out * gr;

			phase += phase_inc;
			while (phase >= n) {
				phase -= n;
			}
			if (stage == IDLE) {
				break;
			}
		}
	}

	TrackerVoice() {
		inst = nullptr;
		phase = 0.0;
		phase_inc = 0.0;
		lfo_phase = 0.0;
		lfo_inc = 0.0;
		sample_rate = 44100.0f;
		velocity = 1.0f;
		priority = 0;
		lane = -1;
		tbl_len = 0;
		stage = IDLE;
		env = 0.0f;
	}
};

#endif // TRACKERGEN_VOICE_H
