/**************************************************************************/
/*  random_number_generator.h                                             */
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

#ifndef RANDOM_NUMBER_GENERATOR_H
#define RANDOM_NUMBER_GENERATOR_H

#include "core/math/random_pcg.h"
#include "core/reference.h"

class RandomNumberGenerator : public Reference {
	GDCLASS(RandomNumberGenerator, Reference);

protected:
	RandomPCG randbase;

	static void _bind_methods();

public:
	_FORCE_INLINE_ void set_seed(uint64_t seed) { randbase.seed(seed); }

	_FORCE_INLINE_ uint64_t get_seed() { return randbase.get_seed(); }

	_FORCE_INLINE_ void set_state(uint64_t p_state) { randbase.set_state(p_state); }

	_FORCE_INLINE_ uint64_t get_state() const { return randbase.get_state(); }

	_FORCE_INLINE_ void randomize() { randbase.randomize(); }

	_FORCE_INLINE_ uint32_t randi() { return randbase.rand(); }

	_FORCE_INLINE_ real_t randf() { return randbase.randf(); }

	_FORCE_INLINE_ real_t randf_range(real_t from, real_t to) { return randbase.random(from, to); }

	_FORCE_INLINE_ real_t randfn(real_t mean = 0.0, real_t deviation = 1.0) { return randbase.randfn(mean, deviation); }

	_FORCE_INLINE_ int randi_range(int from, int to) {
		const unsigned int ret = randbase.rand();
		if (to < from) {
			return ret % (from - to + 1) + to;
		} else {
			return ret % (to - from + 1) + from;
		}
	}

	_FORCE_INLINE_ Vector2 randv_circle() {
		const real_t t = randbase.randf() * Math_TAU;
		return Vector2(Math::cos(t), Math::sin(t));
	}

	// https://karthikkaranth.me/blog/generating-random-points-in-a-sphere/
	_FORCE_INLINE_ Vector3 randv_sphere() {
		Vector3 R;
		real_t L;
		do {
			// Check random vectors in the unit sphere so result is statistically uniform.
			R.x = randbase.random(real_t(-1), real_t(1));
			R.y = randbase.random(real_t(-1), real_t(1));
			R.z = randbase.random(real_t(-1), real_t(1));
			L = R.length_squared();
		} while (L > 1 || L < CMP_EPSILON);
		// return unsafe normal
		const real_t scale = Math::inv_sqrt(R.x * R.x + R.y * R.y + R.z * R.z);
		return R * scale;
	}

	RandomNumberGenerator();
};

#endif // RANDOM_NUMBER_GENERATOR_H
