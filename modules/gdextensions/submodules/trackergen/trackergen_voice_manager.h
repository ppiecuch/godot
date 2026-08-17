/**************************************************************************/
/*  trackergen_voice_manager.h                                           */
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

#ifndef TRACKERGEN_VOICE_MANAGER_H
#define TRACKERGEN_VOICE_MANAGER_H

#include "trackergen_voice.h"

// Fixed voice pool shared by music and (later) SFX. When full, a new note steals
// the lowest-priority voice — releasing voices first — but only if the new note
// is at least as important, so kick/bass (high priority) survive a busy fight
// while melody/fx (low priority) thin out. This is the "big fight costs the
// tune" behaviour, kept deliberately.
class TrackerVoiceManager {
public:
	enum { MAX_VOICES = 16 };

private:
	TrackerVoice voices[MAX_VOICES];
	int voice_count;

public:
	void configure(int p_count) {
		voice_count = CLAMP(p_count, 1, (int)MAX_VOICES);
		reset();
	}

	void reset() {
		for (int i = 0; i < MAX_VOICES; i++) {
			voices[i].kill();
		}
	}

	int get_voice_count() const { return voice_count; }

	int active_count() const {
		int c = 0;
		for (int i = 0; i < voice_count; i++) {
			if (voices[i].is_active()) {
				c++;
			}
		}
		return c;
	}

	void note_on(const Ref<TrackerInstrument> &p_inst, float p_freq, float p_sr, float p_vel, int p_lane) {
		const int prio = p_inst.is_valid() ? p_inst->get_priority() : 0;

		// 1) reuse an idle voice
		int slot = -1;
		for (int i = 0; i < voice_count; i++) {
			if (!voices[i].is_active()) {
				slot = i;
				break;
			}
		}

		// 2) otherwise steal: prefer a releasing voice, then the lowest priority
		if (slot < 0) {
			int best = 0;
			int best_prio = voices[0].get_priority();
			bool best_rel = voices[0].is_releasing();
			for (int i = 1; i < voice_count; i++) {
				const int p = voices[i].get_priority();
				const bool rel = voices[i].is_releasing();
				if (rel != best_rel) {
					if (rel) { // a releasing voice is always a better steal target
						best = i;
						best_prio = p;
						best_rel = rel;
					}
					continue;
				}
				if (p < best_prio) {
					best = i;
					best_prio = p;
				}
			}
			// Protect more important voices: only steal if we're at least equal.
			if (prio < best_prio) {
				return; // drop this note
			}
			slot = best;
		}

		voices[slot].note_on(p_inst, p_freq, p_sr, p_vel, p_lane);
	}

	void note_off_lane(int p_lane) {
		for (int i = 0; i < voice_count; i++) {
			if (voices[i].is_active() && voices[i].get_lane() == p_lane) {
				voices[i].note_off();
			}
		}
	}

	void render(AudioFrame *p_buffer, int p_frames, float p_tension) {
		for (int i = 0; i < voice_count; i++) {
			voices[i].render(p_buffer, p_frames, p_tension);
		}
	}

	TrackerVoiceManager() {
		voice_count = 8;
	}
};

#endif // TRACKERGEN_VOICE_MANAGER_H
