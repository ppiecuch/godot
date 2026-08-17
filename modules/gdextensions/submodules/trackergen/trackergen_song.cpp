/**************************************************************************/
/*  trackergen_song.cpp                                                   */
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

#include "trackergen_song.h"

void TrackerSong::set_patterns(const Array &p_patterns) {
	patterns.clear();
	for (int i = 0; i < p_patterns.size(); i++) {
		Ref<TrackerPattern> p = p_patterns[i];
		patterns.push_back(p);
	}
}

Array TrackerSong::get_patterns() const {
	Array a;
	a.resize(patterns.size());
	for (int i = 0; i < patterns.size(); i++) {
		a[i] = patterns[i];
	}
	return a;
}

Ref<TrackerPattern> TrackerSong::get_pattern(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, patterns.size(), Ref<TrackerPattern>());
	return patterns[p_index];
}

int TrackerSong::pattern_index_at_order(int p_order_pos) const {
	if (order.size() == 0) {
		return 0;
	}
	int pos = p_order_pos % order.size();
	if (pos < 0) {
		pos += order.size();
	}
	return order.get(pos);
}

void TrackerSong::ensure_baked() {
	if (kit.is_valid()) {
		kit->ensure_baked();
	}
	for (int i = 0; i < patterns.size(); i++) {
		Ref<TrackerPattern> pat = patterns[i]; // non-const copy (Vector::operator[] is const)
		if (pat.is_valid()) {
			pat->ensure_baked();
		}
	}
}

void TrackerSong::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_kit", "kit"), &TrackerSong::set_kit);
	ClassDB::bind_method(D_METHOD("get_kit"), &TrackerSong::get_kit);
	ClassDB::bind_method(D_METHOD("set_chords", "chords"), &TrackerSong::set_chords);
	ClassDB::bind_method(D_METHOD("get_chords"), &TrackerSong::get_chords);
	ClassDB::bind_method(D_METHOD("set_patterns", "patterns"), &TrackerSong::set_patterns);
	ClassDB::bind_method(D_METHOD("get_patterns"), &TrackerSong::get_patterns);
	ClassDB::bind_method(D_METHOD("get_pattern_count"), &TrackerSong::get_pattern_count);
	ClassDB::bind_method(D_METHOD("get_pattern", "index"), &TrackerSong::get_pattern);
	ClassDB::bind_method(D_METHOD("set_order", "order"), &TrackerSong::set_order);
	ClassDB::bind_method(D_METHOD("get_order"), &TrackerSong::get_order);
	ClassDB::bind_method(D_METHOD("pattern_index_at_order", "order_pos"), &TrackerSong::pattern_index_at_order);

	ClassDB::bind_method(D_METHOD("set_tempo_bpm", "bpm"), &TrackerSong::set_tempo_bpm);
	ClassDB::bind_method(D_METHOD("get_tempo_bpm"), &TrackerSong::get_tempo_bpm);
	ClassDB::bind_method(D_METHOD("set_tempo_min", "bpm"), &TrackerSong::set_tempo_min);
	ClassDB::bind_method(D_METHOD("get_tempo_min"), &TrackerSong::get_tempo_min);
	ClassDB::bind_method(D_METHOD("set_tempo_max", "bpm"), &TrackerSong::set_tempo_max);
	ClassDB::bind_method(D_METHOD("get_tempo_max"), &TrackerSong::get_tempo_max);
	ClassDB::bind_method(D_METHOD("set_steps_per_beat", "n"), &TrackerSong::set_steps_per_beat);
	ClassDB::bind_method(D_METHOD("get_steps_per_beat"), &TrackerSong::get_steps_per_beat);
	ClassDB::bind_method(D_METHOD("set_beats_per_bar", "n"), &TrackerSong::set_beats_per_bar);
	ClassDB::bind_method(D_METHOD("get_beats_per_bar"), &TrackerSong::get_beats_per_bar);
	ClassDB::bind_method(D_METHOD("set_loop", "loop"), &TrackerSong::set_loop);
	ClassDB::bind_method(D_METHOD("get_loop"), &TrackerSong::get_loop);
	ClassDB::bind_method(D_METHOD("set_tempo_reacts", "enabled"), &TrackerSong::set_tempo_reacts);
	ClassDB::bind_method(D_METHOD("get_tempo_reacts"), &TrackerSong::get_tempo_reacts);
	ClassDB::bind_method(D_METHOD("ensure_baked"), &TrackerSong::ensure_baked);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "kit", PROPERTY_HINT_RESOURCE_TYPE, "TrackerKit"), "set_kit", "get_kit");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "chords", PROPERTY_HINT_RESOURCE_TYPE, "ChordProgression"), "set_chords", "get_chords");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "patterns"), "set_patterns", "get_patterns");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_INT_ARRAY, "order"), "set_order", "get_order");

	ADD_GROUP("Tempo", "");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "tempo_bpm", PROPERTY_HINT_RANGE, "20,400,0.1"), "set_tempo_bpm", "get_tempo_bpm");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "tempo_min", PROPERTY_HINT_RANGE, "20,400,0.1"), "set_tempo_min", "get_tempo_min");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "tempo_max", PROPERTY_HINT_RANGE, "20,400,0.1"), "set_tempo_max", "get_tempo_max");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "steps_per_beat", PROPERTY_HINT_RANGE, "1,16,1"), "set_steps_per_beat", "get_steps_per_beat");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "beats_per_bar", PROPERTY_HINT_RANGE, "1,16,1"), "set_beats_per_bar", "get_beats_per_bar");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "tempo_reacts"), "set_tempo_reacts", "get_tempo_reacts");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "loop"), "set_loop", "get_loop");
}

TrackerSong::TrackerSong() {
	tempo_bpm = 120.0;
	tempo_min = 90.0;
	tempo_max = 160.0;
	steps_per_beat = 4;
	beats_per_bar = 4;
	loop = true;
	tempo_reacts = false;
}
