/**************************************************************************/
/*  ddls_rand_generator.h                                                 */
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

#include "core/math/math_funcs.h"
#include "core/math/random_pcg.h"

class DDLSRandGenerator {
	int original_seed;
	int curr_seed;
	int range_min;
	int range_max;

	int num_iter;

	RandomPCG rng;

public:
	void set_seed(int p_value) { original_seed = curr_seed = p_value; }
	void set_range(int p_min_value, int p_max_value) {
		range_min = p_min_value;
		range_max = p_max_value;
	}

	int get_seed() const { return original_seed; }
	int get_range_min() const { return range_min; }
	int get_range_max() const { return range_max; }

	void reset() {
		curr_seed = original_seed;
		num_iter = 0;
	}

	int next() {
		const String str = String::num_int64(curr_seed * curr_seed).pad_zeros(8);
		curr_seed = str.substr(1, 5).to_int();
		const int res = Math::round(range_min + (curr_seed / 99999.0) * (range_max - range_min));

		if (curr_seed == 0) {
			curr_seed = original_seed + num_iter;
		}

		if (num_iter++ == 200) {
			reset();
		}
		return res;
	}

	DDLSRandGenerator(int p_seed = 1234, int p_range_min = 0, int p_range_max = 1) {
		original_seed = curr_seed = p_seed;
		range_min = p_range_min;
		range_max = p_range_max;

		num_iter = 0;
	}
};
