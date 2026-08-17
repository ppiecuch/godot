/**************************************************************************/
/*  trackergen_chords.h                                                   */
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

#ifndef TRACKERGEN_CHORDS_H
#define TRACKERGEN_CHORDS_H

#include "core/array.h"
#include "core/pool_vector.h"
#include "core/resource.h"

// The harmonic spine: a key (root + scale) and a list of chords with bar
// durations. Patterns store notes RELATIVE to the current chord, so resolving a
// note requires this. Note-type values match TrackerPattern::NoteType.
class ChordProgression : public Resource {
	GDCLASS(ChordProgression, Resource);

	int key_root; // 0..11 (C=0)
	PoolIntArray scale; // semitone offsets, e.g. minor [0,2,3,5,7,8,10]
	int base_octave; // octave 0 in note events maps to MIDI 12*base_octave
	// Each chord: Dictionary { degree:int (scale degree), quality:PoolIntArray
	// (semitones from chord root, e.g. [0,3,7]), bars:int }.
	Array chords;

	_FORCE_INLINE_ int scale_at(int p_i) const {
		const int n = scale.size();
		if (n == 0) {
			return 0;
		}
		int i = p_i % n;
		if (i < 0) {
			i += n;
		}
		return scale.get(i);
	}

protected:
	static void _bind_methods();

public:
	void set_key_root(int p_v) { key_root = p_v; }
	int get_key_root() const { return key_root; }
	void set_scale(const PoolIntArray &p_v) { scale = p_v; }
	PoolIntArray get_scale() const { return scale; }
	void set_base_octave(int p_v) { base_octave = p_v; }
	int get_base_octave() const { return base_octave; }
	void set_chords(const Array &p_v) { chords = p_v; }
	Array get_chords() const { return chords; }

	int get_chord_count() const { return chords.size(); }
	int get_total_bars() const;
	int chord_index_at_bar(int p_bar) const;

	// Resolve a pattern note event to a MIDI note number / frequency.
	// p_note_type: 0=CHORD_TONE, 1=SCALE_DEGREE, 2=ABSOLUTE (see TrackerPattern).
	int resolve_to_midi(int p_chord_index, int p_note_type, int p_note_index, int p_octave) const;
	float resolve_to_freq(int p_chord_index, int p_note_type, int p_note_index, int p_octave) const;
	static float midi_to_freq(int p_midi);

	ChordProgression();
};

#endif // TRACKERGEN_CHORDS_H
