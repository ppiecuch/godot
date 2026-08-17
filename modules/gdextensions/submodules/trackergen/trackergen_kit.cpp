/**************************************************************************/
/*  trackergen_kit.cpp                                                    */
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

#include "trackergen_kit.h"

void TrackerKit::set_instruments(const Array &p_array) {
	instruments.clear();
	for (int i = 0; i < p_array.size(); i++) {
		Ref<TrackerInstrument> inst = p_array[i];
		instruments.push_back(inst); // may be null; kept for index stability
	}
}

Array TrackerKit::get_instruments() const {
	Array a;
	a.resize(instruments.size());
	for (int i = 0; i < instruments.size(); i++) {
		a[i] = instruments[i];
	}
	return a;
}

Ref<TrackerInstrument> TrackerKit::get_instrument(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, instruments.size(), Ref<TrackerInstrument>());
	return instruments[p_index];
}

int TrackerKit::find_by_id(const StringName &p_id) const {
	for (int i = 0; i < instruments.size(); i++) {
		if (instruments[i].is_valid() && instruments[i]->get_id() == p_id) {
			return i;
		}
	}
	return -1;
}

void TrackerKit::ensure_baked() {
	for (int i = 0; i < instruments.size(); i++) {
		Ref<TrackerInstrument> inst = instruments[i]; // non-const copy (Vector::operator[] is const)
		if (inst.is_valid()) {
			inst->ensure_baked();
		}
	}
}

void TrackerKit::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_instruments", "instruments"), &TrackerKit::set_instruments);
	ClassDB::bind_method(D_METHOD("get_instruments"), &TrackerKit::get_instruments);
	ClassDB::bind_method(D_METHOD("get_instrument_count"), &TrackerKit::get_instrument_count);
	ClassDB::bind_method(D_METHOD("get_instrument", "index"), &TrackerKit::get_instrument);
	ClassDB::bind_method(D_METHOD("find_by_id", "id"), &TrackerKit::find_by_id);
	ClassDB::bind_method(D_METHOD("ensure_baked"), &TrackerKit::ensure_baked);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "instruments", PROPERTY_HINT_NONE, "",
						 PROPERTY_USAGE_DEFAULT, "TrackerInstrument"),
			"set_instruments", "get_instruments");
}
