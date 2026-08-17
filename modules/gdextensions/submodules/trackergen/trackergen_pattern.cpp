/**************************************************************************/
/*  trackergen_pattern.cpp                                                */
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

#include "trackergen_pattern.h"

#include "core/dictionary.h"

void TrackerPattern::set_steps(int p_steps) {
	steps = MAX(1, p_steps);
	_dirty = true;
}

void TrackerPattern::set_events(const Array &p_events) {
	events = p_events;
	_dirty = true;
}

void TrackerPattern::add_event(const Dictionary &p_event) {
	events.push_back(p_event);
	_dirty = true;
}

void TrackerPattern::add_note(int p_step, int p_lane, int p_note_index, int p_octave, int p_instrument) {
	Dictionary d;
	d["step"] = p_step;
	d["lane"] = p_lane;
	d["type"] = (int)NOTE_CHORD_TONE;
	d["index"] = p_note_index;
	d["octave"] = p_octave;
	d["instrument"] = p_instrument;
	d["volume"] = 1.0;
	events.push_back(d);
	_dirty = true;
}

void TrackerPattern::clear() {
	events.clear();
	_dirty = true;
}

void TrackerPattern::ensure_baked() {
	if (!_dirty && _by_step.size() == steps) {
		return;
	}
	_by_step.clear();
	_by_step.resize(steps);

	for (int i = 0; i < events.size(); i++) {
		Dictionary d = events[i];
		SeqEvent e;
		e.step = (int)d.get("step", 0);
		e.lane = (int)d.get("lane", 0);
		e.note_type = (int)d.get("type", (int)NOTE_CHORD_TONE);
		e.note_index = (int)d.get("index", 0);
		e.octave = (int)d.get("octave", 0);
		e.instrument = (int)d.get("instrument", 0);
		e.volume = (float)d.get("volume", 1.0);
		if (e.step < 0 || e.step >= steps) {
			continue;
		}
		_by_step.write[e.step].push_back(e);
	}
	_dirty = false;
}

const Vector<SeqEvent> &TrackerPattern::events_at(int p_step) const {
	static const Vector<SeqEvent> empty;
	if (p_step < 0 || p_step >= _by_step.size()) {
		return empty;
	}
	return _by_step[p_step];
}

void TrackerPattern::_bind_methods() {
	BIND_ENUM_CONSTANT(NOTE_CHORD_TONE);
	BIND_ENUM_CONSTANT(NOTE_SCALE_DEGREE);
	BIND_ENUM_CONSTANT(NOTE_ABSOLUTE);
	BIND_ENUM_CONSTANT(NOTE_OFF);

	ClassDB::bind_method(D_METHOD("set_steps", "steps"), &TrackerPattern::set_steps);
	ClassDB::bind_method(D_METHOD("get_steps"), &TrackerPattern::get_steps);
	ClassDB::bind_method(D_METHOD("set_events", "events"), &TrackerPattern::set_events);
	ClassDB::bind_method(D_METHOD("get_events"), &TrackerPattern::get_events);
	ClassDB::bind_method(D_METHOD("add_event", "event"), &TrackerPattern::add_event);
	ClassDB::bind_method(D_METHOD("add_note", "step", "lane", "note_index", "octave", "instrument"), &TrackerPattern::add_note);
	ClassDB::bind_method(D_METHOD("clear"), &TrackerPattern::clear);
	ClassDB::bind_method(D_METHOD("ensure_baked"), &TrackerPattern::ensure_baked);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "steps", PROPERTY_HINT_RANGE, "1,256,1"), "set_steps", "get_steps");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "events"), "set_events", "get_events");
}

TrackerPattern::TrackerPattern() {
	steps = 16;
	_dirty = true;
}
