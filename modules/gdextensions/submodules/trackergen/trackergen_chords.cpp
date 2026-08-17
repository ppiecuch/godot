/**************************************************************************/
/*  trackergen_chords.cpp                                                 */
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

#include "trackergen_chords.h"

#include "core/dictionary.h"
#include "core/math/math_funcs.h"

int ChordProgression::get_total_bars() const {
	int total = 0;
	for (int i = 0; i < chords.size(); i++) {
		Dictionary ch = chords[i];
		total += (int)ch.get("bars", 1);
	}
	return total;
}

int ChordProgression::chord_index_at_bar(int p_bar) const {
	const int count = chords.size();
	if (count == 0) {
		return 0;
	}
	const int total = get_total_bars();
	if (total <= 0) {
		return 0;
	}
	int b = p_bar % total;
	if (b < 0) {
		b += total;
	}
	int acc = 0;
	for (int i = 0; i < count; i++) {
		Dictionary ch = chords[i];
		const int bars = MAX(1, (int)ch.get("bars", 1));
		if (b < acc + bars) {
			return i;
		}
		acc += bars;
	}
	return count - 1;
}

int ChordProgression::resolve_to_midi(int p_chord_index, int p_note_type, int p_note_index, int p_octave) const {
	const int base_midi = 12 * base_octave;

	// ABSOLUTE: note_index is a raw MIDI number.
	if (p_note_type == 2) {
		return p_note_index;
	}

	const int sc_size = MAX(1, scale.size());

	// SCALE_DEGREE: index counts steps up the key's scale.
	if (p_note_type == 1) {
		int deg = p_note_index;
		int oct_extra = deg / sc_size;
		int within = deg % sc_size;
		if (within < 0) {
			within += sc_size;
			oct_extra -= 1;
		}
		const int pc = key_root + scale_at(within);
		return base_midi + pc + 12 * p_octave + 12 * oct_extra;
	}

	// CHORD_TONE (0): index counts up the chord's own tones (root, 3rd, 5th...).
	if (chords.size() == 0) {
		return base_midi + key_root + 12 * p_octave;
	}
	int ci = p_chord_index;
	if (ci < 0 || ci >= chords.size()) {
		ci = CLAMP(ci, 0, chords.size() - 1);
	}
	Dictionary ch = chords[ci];
	const int degree = (int)ch.get("degree", 0);

	PoolIntArray quality = ch.get("quality", Variant());
	if (quality.size() == 0) {
		// default minor triad
		quality.push_back(0);
		quality.push_back(3);
		quality.push_back(7);
	}
	const int qn = quality.size();

	const int chord_root_pc = key_root + scale_at(degree % sc_size) + 12 * (degree / sc_size);

	int tone_i = p_note_index % qn;
	int tone_oct = p_note_index / qn;
	if (tone_i < 0) {
		tone_i += qn;
		tone_oct -= 1;
	}
	const int semi = chord_root_pc + quality.get(tone_i);
	return base_midi + semi + 12 * p_octave + 12 * tone_oct;
}

float ChordProgression::midi_to_freq(int p_midi) {
	return 440.0f * Math::pow(2.0f, (float)(p_midi - 69) / 12.0f);
}

float ChordProgression::resolve_to_freq(int p_chord_index, int p_note_type, int p_note_index, int p_octave) const {
	return midi_to_freq(resolve_to_midi(p_chord_index, p_note_type, p_note_index, p_octave));
}

void ChordProgression::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_key_root", "root"), &ChordProgression::set_key_root);
	ClassDB::bind_method(D_METHOD("get_key_root"), &ChordProgression::get_key_root);
	ClassDB::bind_method(D_METHOD("set_scale", "scale"), &ChordProgression::set_scale);
	ClassDB::bind_method(D_METHOD("get_scale"), &ChordProgression::get_scale);
	ClassDB::bind_method(D_METHOD("set_base_octave", "octave"), &ChordProgression::set_base_octave);
	ClassDB::bind_method(D_METHOD("get_base_octave"), &ChordProgression::get_base_octave);
	ClassDB::bind_method(D_METHOD("set_chords", "chords"), &ChordProgression::set_chords);
	ClassDB::bind_method(D_METHOD("get_chords"), &ChordProgression::get_chords);

	ClassDB::bind_method(D_METHOD("get_chord_count"), &ChordProgression::get_chord_count);
	ClassDB::bind_method(D_METHOD("get_total_bars"), &ChordProgression::get_total_bars);
	ClassDB::bind_method(D_METHOD("chord_index_at_bar", "bar"), &ChordProgression::chord_index_at_bar);
	ClassDB::bind_method(D_METHOD("resolve_to_midi", "chord_index", "note_type", "note_index", "octave"), &ChordProgression::resolve_to_midi);
	ClassDB::bind_method(D_METHOD("resolve_to_freq", "chord_index", "note_type", "note_index", "octave"), &ChordProgression::resolve_to_freq);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "key_root", PROPERTY_HINT_RANGE, "0,11,1"), "set_key_root", "get_key_root");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_INT_ARRAY, "scale"), "set_scale", "get_scale");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "base_octave", PROPERTY_HINT_RANGE, "0,9,1"), "set_base_octave", "get_base_octave");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "chords"), "set_chords", "get_chords");
}

ChordProgression::ChordProgression() {
	key_root = 9; // A
	base_octave = 4;
	// Natural minor by default.
	scale.push_back(0);
	scale.push_back(2);
	scale.push_back(3);
	scale.push_back(5);
	scale.push_back(7);
	scale.push_back(8);
	scale.push_back(10);
}
