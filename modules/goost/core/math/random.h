/**************************************************************************/
/*  random.h                                                              */
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

#pragma once

#include "core/math/random_number_generator.h"

class RandomBase : public RandomNumberGenerator {
public:
	uint32_t get_number();
	real_t get_value();
	Color get_color();
	bool get_condition();

	Color color_hsv(float h_min = 0.0, float h_max = 1.0, float s_min = 0.0, float s_max = 1.0, float v_min = 0.0, float v_max = 1.0, float a_min = 1.0, float a_max = 1.0);
	Color color_rgb(float r_min = 0.0, float r_max = 1.0, float g_min = 0.0, float g_max = 1.0, float b_min = 0.0, float b_max = 1.0, float a_min = 1.0, float a_max = 1.0);

	Variant range(const Variant &p_from, const Variant &p_to);
	Variant pick(const Variant &p_sequence);
	Variant pop(const Variant &p_sequence);
	Array choices(const Variant &p_sequence, int p_count = 1, const PoolIntArray &p_weights = Variant(), bool p_is_cumulative = false);
	void shuffle(Array p_array);
	bool decision(float probability);

	RandomBase() {}
};

class Random : public RandomBase {
	GDCLASS(Random, RandomBase);

private:
	static Random *singleton;

protected:
	static void _bind_methods();

public:
	static Random *get_singleton() { return singleton; }
	virtual Ref<Reference> new_instance() const { return memnew(Random); }

	Random() {
		if (!singleton) {
			randomize(); // Only the global one is randomized automatically.
			singleton = this;
		}
	}
};
