/**************************************************************************/
/*  trackergen_song.h                                                     */
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

#ifndef TRACKERGEN_SONG_H
#define TRACKERGEN_SONG_H

#include "trackergen_chords.h"
#include "trackergen_kit.h"
#include "trackergen_pattern.h"

#include "core/pool_vector.h"
#include "core/resource.h"

// A full song: kit + chord progression + patterns + an order list, plus tempo
// and meter. The sequencer (AudioStreamPlaybackTracker) walks this.
class TrackerSong : public Resource {
	GDCLASS(TrackerSong, Resource);

	Ref<TrackerKit> kit;
	Ref<ChordProgression> chords;
	Vector<Ref<TrackerPattern>> patterns;
	PoolIntArray order; // indices into patterns

	real_t tempo_bpm; // base tempo
	real_t tempo_min; // for tension mapping (P2)
	real_t tempo_max;
	int steps_per_beat; // rows per beat (4 = 16th notes)
	int beats_per_bar; // meter numerator (4/4 -> 4)
	bool loop;
	bool tempo_reacts; // P2: if true, tempo = lerp(tempo_min, tempo_max, tension)

protected:
	static void _bind_methods();

public:
	void set_kit(const Ref<TrackerKit> &p_kit) { kit = p_kit; }
	Ref<TrackerKit> get_kit() const { return kit; }
	void set_chords(const Ref<ChordProgression> &p_chords) { chords = p_chords; }
	Ref<ChordProgression> get_chords() const { return chords; }

	void set_patterns(const Array &p_patterns);
	Array get_patterns() const;
	int get_pattern_count() const { return patterns.size(); }
	Ref<TrackerPattern> get_pattern(int p_index) const;

	void set_order(const PoolIntArray &p_order) { order = p_order; }
	PoolIntArray get_order() const { return order; }
	int get_order_length() const { return order.size(); }
	int pattern_index_at_order(int p_order_pos) const;

	void set_tempo_bpm(real_t p_v) { tempo_bpm = p_v; }
	real_t get_tempo_bpm() const { return tempo_bpm; }
	void set_tempo_min(real_t p_v) { tempo_min = p_v; }
	real_t get_tempo_min() const { return tempo_min; }
	void set_tempo_max(real_t p_v) { tempo_max = p_v; }
	real_t get_tempo_max() const { return tempo_max; }
	void set_steps_per_beat(int p_v) { steps_per_beat = MAX(1, p_v); }
	int get_steps_per_beat() const { return steps_per_beat; }
	void set_beats_per_bar(int p_v) { beats_per_bar = MAX(1, p_v); }
	int get_beats_per_bar() const { return beats_per_bar; }
	void set_loop(bool p_v) { loop = p_v; }
	bool get_loop() const { return loop; }
	void set_tempo_reacts(bool p_v) { tempo_reacts = p_v; }
	bool get_tempo_reacts() const { return tempo_reacts; }

	_FORCE_INLINE_ int get_steps_per_bar() const { return steps_per_beat * beats_per_bar; }

	// Bake kit wavetables + pattern step tables (call off the audio thread).
	void ensure_baked();

	TrackerSong();
};

#endif // TRACKERGEN_SONG_H
