/**************************************************************************/
/*  fractal_benchmark.cpp                                                 */
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

// simple real_t ALU heavy benchmark
// no dependency on any math library

#include "core/math/math_funcs.h"

static uint32_t EvaluateJuliaFractalAt(real_t x, real_t y) {
	uint32_t ret = 0;

	real_t dist2 = 0;
	const real_t maxdist2 = 1600;
	uint32_t maxiter = 300;

	// pretty Julia fractal
	real_t cx = -0.73;
	real_t cy = 0.176;

	while (dist2 <= maxdist2 && ret < maxiter) {
		const real_t x2 = x * x - y * y + cx;
		const real_t y2 = 2 * x * y + cy;

		x = x2;
		y = y2;

		++ret;
		dist2 = x * x + y * y;
	}

	return ret;
}

real_t FractalBenchmark() {
	const int32_t extent = 256; // scales quadratic with the time

	real_t sum = 0;

	for (int y = 0; y < extent; ++y) {
		for (int x = 0; x < extent; ++x) {
			sum += EvaluateJuliaFractalAt(x / (real_t)extent * 2 - 1, y / (real_t)extent * 2 - 1);
		}
	}

	// Aggressive math optimizations and inline expansion can optimize this benchmark's return value into a constant
	// Introduce some randomness to avoid this, noting the return value of this function is not used, rather the time it takes to complete
	return sum / (real_t)(extent * extent) + Math::rand();
}
