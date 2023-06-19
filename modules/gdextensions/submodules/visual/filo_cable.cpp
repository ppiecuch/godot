/**************************************************************************/
/*  filo_cable.cpp                                                        */
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

#include "core/math/math_funcs.h"
#include "core/math/quat.h"
#include "core/math/transform.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"
#include "core/vector.h"

namespace filo {
struct utils {
	static bool Catenary(const Vector3 &p1, const Vector3 &p2, real_t l, int samples, Vector<Vector2> &points) {
		ERR_FAIL_COND_V(samples < 2, false);

		points.clear();

		const Vector3 vector = p2 - p1;
		const Vector3 dir = vector * Vector3(1, 0, 1);

		const Quat rot = LookRotation(dir);
		const Quat irot = rot.inverse();
		const Vector3 n = irot.xform(vector);

		real_t r = 0;
		real_t s = 0;
		real_t u = n.z;
		real_t v = n.y;

		// swap values if p1 is to the right of p2:
		if (r > u) {
			std::swap(r, u);
			std::swap(s, v);
		}

		// find z:
		real_t z = 0.005;
		real_t target = Math::sqrt(l * l - (v - s) * (v - s)) / (u - r);
		while (Math::sinh(z) / z < target) {
			z += 0.005;
		}

		if (z > 0.005) {
			const real_t a = (u - r) / 2 / z;
			const real_t p = (r + u - a * Math::log((l + v - s) / (l - v + s))) / 2;
			const real_t q = (v + s - l * (real_t)Math::cosh(z) / (real_t)Math::sinh(z)) / 2;

			const real_t inc = (u - r) * (1.0 / (samples - 1));

			for (int i = 0; i < samples; ++i) {
				const real_t x = r + inc * i;
				points.write[i] = Vector2(x, a * Math::cosh((x - p) / a) + q);
			}
			return true;
		} else {
			return false;
		}
	}

	static bool Sinusoid(const Vector3 &origin, const Vector3 &direction, real_t l, unsigned frequency, int samples, Vector<Vector3> &points) {
		ERR_FAIL_COND_V(samples < 2, false);

		static const Vector3 Forward = Vector3(0, 0, -1);
		points.clear();
		Vector3 ndirection = direction;
		real_t magnitude = ndirection.length();
		if (magnitude > 1e-4) {
			ndirection /= magnitude;
			Vector3 ortho = ndirection.cross(Forward);

			const real_t inc = magnitude / (samples - 1);

			const real_t d = frequency * 4;
			const real_t d2 = d * d;

			// analytic approx to amplitude from wave arc length.
			const real_t amplitude = Math::sqrt(l * l / d2 - magnitude * magnitude / d2);

			if (Math::is_nan(amplitude)) {
				return false;
			}
			for (int i = 0; i < samples; ++i) {
				real_t pctg = i / real_t(samples - 1);
				points.write[i] = origin + ndirection * inc * i + ortho * Math::sin(pctg * Math_PI * 2 * frequency) * amplitude;
			}
			return true;
		} else {
			return false;
		}
	}

	// Modulo operator that also follows intuition for negative arguments. That is , -1 mod 3 = 2, not -1.
	static real_t Mod(real_t a, real_t b) { return a - b * Math::floor(a / b); }

	// Rotation in z of 3d vecror
	static Vector3 Rotate2D(const Vector3 &v, real_t angle) {
		return Vector3(
				v.x * Math::cos(angle) - v.y * Math::sin(angle),
				v.x * Math::sin(angle) + v.y * Math::cos(angle),
				v.z);
	}

	// Creates a rotation with the specified forward and upwards directions.
	static Quat LookRotation(const Vector3 &forward, const Vector3 &upwards = Vector3::UP) {
		return Transform().looking_at(forward, upwards).basis.get_quat();
	}
};
} //namespace filo
