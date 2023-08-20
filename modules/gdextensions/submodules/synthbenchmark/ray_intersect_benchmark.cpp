/**************************************************************************/
/*  ray_intersect_benchmark.cpp                                           */
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

// simple real_t ALU heavy (some branching) benchmark
// some dependency on our library (not ideal)

// Line Check With Triangle
// Algorithm based on "Fast, Minimum Storage Ray/Triangle Intersection"
// Returns true if the line segment does hit the triangle
//
// code duplication to not get a different result if the source gets optimized

#include "core/math/math_funcs.h"
#include "core/math/random_number_generator.h"
#include "core/math/vector3.h"

_FORCE_INLINE_ static bool LineCheckWithTriangle(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3, const Vector3 &start, const Vector3 &end) {
	const Vector3 direction = end - start;

	const Vector3 edge1 = v3 - v1;
	const Vector3 edge2 = v2 - v1;
	const Vector3 P = direction.cross(edge2);
	const real_t determinant = edge1.dot(P);

	if (determinant < 0.00001) {
		return false;
	}

	const Vector3 T = start - v1;
	const real_t U = T.dot(P);

	if (U < 0 || U > determinant) {
		return false;
	}

	const Vector3 Q = T.cross(edge1);
	const real_t V = direction.dot(Q);

	if (V < 0 || U + V > determinant) {
		return false;
	}

	const real_t tt = (edge2.dot(Q)) / determinant;

	return (tt >= 0);
}

real_t RayIntersectBenchmark() {
	RandomNumberGenerator rnd;
	rnd.set_seed(0x1234);

	const int32_t step_count = 200000;

	int32_t hit_count = 0;

	for (int32_t i = 0; i < step_count; ++i) {
		Vector3 tri[3] = { Vector3(0.1, 0.2, 2.3), Vector3(2.1, 0.2, 0.3), Vector3(-2.1, 0.2, 0.3) };

		const Vector3 start = rnd.randv_sphere() * 3.0;
		const Vector3 end = rnd.randv_sphere() * 3.0;

		if (LineCheckWithTriangle(tri[0], tri[1], tri[2], start, end)) {
			hit_count++;
		}
	}

	// to avoid getting optimized out
	return hit_count / (real_t)step_count;
}
