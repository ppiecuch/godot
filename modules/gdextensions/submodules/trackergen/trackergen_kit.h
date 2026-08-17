/**************************************************************************/
/*  trackergen_kit.h                                                      */
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

#ifndef TRACKERGEN_KIT_H
#define TRACKERGEN_KIT_H

#include "trackergen_instrument.h"

#include "core/resource.h"

// A named set of instruments — the "soundfont" of a track. Patterns reference
// instruments by index or by id.
class TrackerKit : public Resource {
	GDCLASS(TrackerKit, Resource);

	Vector<Ref<TrackerInstrument>> instruments;

protected:
	static void _bind_methods();

public:
	void set_instruments(const Array &p_array);
	Array get_instruments() const;

	int get_instrument_count() const { return instruments.size(); }
	Ref<TrackerInstrument> get_instrument(int p_index) const;
	int find_by_id(const StringName &p_id) const;

	// Bake every instrument's wavetables (call off the audio thread).
	void ensure_baked();

	TrackerKit() {}
};

#endif // TRACKERGEN_KIT_H
