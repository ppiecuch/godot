/**************************************************************************/
/*  iso_utils.h                                                           */
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

#ifndef ISO_UTILS_H
#define ISO_UTILS_H

#include "core/color.h"
#include "core/math/math_defs.h"
#include "core/math/math_funcs.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"

// --- IsoMinMax ---

struct IsoMinMax {
	real_t min;
	real_t max;

	_FORCE_INLINE_ real_t get_size() const { return max - min; }
	_FORCE_INLINE_ real_t get_center() const { return min + (max - min) * 0.5f; }

	_FORCE_INLINE_ void set(real_t p_minmax) {
		min = p_minmax;
		max = p_minmax;
	}
	_FORCE_INLINE_ void set(real_t p_min, real_t p_max) {
		min = p_min;
		max = p_max;
	}
	_FORCE_INLINE_ void set(const IsoMinMax &p_other) {
		min = p_other.min;
		max = p_other.max;
	}

	_FORCE_INLINE_ void resize(real_t p_size) { max = min + p_size; }
	_FORCE_INLINE_ void translate(real_t p_delta) {
		min += p_delta;
		max += p_delta;
	}

	_FORCE_INLINE_ bool contains(real_t p_value) const { return p_value >= min && p_value <= max; }
	_FORCE_INLINE_ bool contains(const IsoMinMax &p_other) const { return max >= p_other.max && min <= p_other.min; }
	_FORCE_INLINE_ bool overlaps(const IsoMinMax &p_other) const { return max > p_other.min && min < p_other.max; }

	_FORCE_INLINE_ bool approximately(const IsoMinMax &p_other) const {
		return Math::is_equal_approx(min, p_other.min) && Math::is_equal_approx(max, p_other.max);
	}

	_FORCE_INLINE_ static IsoMinMax merge(const IsoMinMax &a, const IsoMinMax &b) {
		IsoMinMax result;
		result.min = a.min < b.min ? a.min : b.min;
		result.max = a.max > b.max ? a.max : b.max;
		return result;
	}

	_FORCE_INLINE_ IsoMinMax() :
			min(0), max(0) {}
	_FORCE_INLINE_ IsoMinMax(real_t p_minmax) :
			min(p_minmax), max(p_minmax) {}
	_FORCE_INLINE_ IsoMinMax(real_t p_min, real_t p_max) :
			min(p_min), max(p_max) {}

	static const IsoMinMax ZERO;
};

// --- IsoRect ---

struct IsoRect {
	IsoMinMax x;
	IsoMinMax y;

	_FORCE_INLINE_ Vector2 get_size() const { return Vector2(x.get_size(), y.get_size()); }
	_FORCE_INLINE_ Vector2 get_center() const { return Vector2(x.get_center(), y.get_center()); }

	_FORCE_INLINE_ void set(real_t p_min_x, real_t p_min_y, real_t p_max_x, real_t p_max_y) {
		x.set(p_min_x, p_max_x);
		y.set(p_min_y, p_max_y);
	}
	_FORCE_INLINE_ void set(const Vector2 &p_min, const Vector2 &p_max) {
		x.set(p_min.x, p_max.x);
		y.set(p_min.y, p_max.y);
	}
	_FORCE_INLINE_ void set(const IsoMinMax &p_x, const IsoMinMax &p_y) {
		x.set(p_x);
		y.set(p_y);
	}
	_FORCE_INLINE_ void set(const IsoRect &p_other) {
		x.set(p_other.x);
		y.set(p_other.y);
	}

	_FORCE_INLINE_ void resize(real_t p_size_x, real_t p_size_y) {
		x.resize(p_size_x);
		y.resize(p_size_y);
	}
	_FORCE_INLINE_ void resize(const Vector2 &p_size) {
		x.resize(p_size.x);
		y.resize(p_size.y);
	}

	_FORCE_INLINE_ void translate(real_t p_delta_x, real_t p_delta_y) {
		x.translate(p_delta_x);
		y.translate(p_delta_y);
	}
	_FORCE_INLINE_ void translate(const Vector2 &p_delta) {
		x.translate(p_delta.x);
		y.translate(p_delta.y);
	}

	_FORCE_INLINE_ bool contains(const Vector2 &p_point) const { return x.contains(p_point.x) && y.contains(p_point.y); }
	_FORCE_INLINE_ bool contains(const IsoRect &p_other) const { return x.contains(p_other.x) && y.contains(p_other.y); }
	_FORCE_INLINE_ bool overlaps(const IsoRect &p_other) const { return x.overlaps(p_other.x) && y.overlaps(p_other.y); }
	_FORCE_INLINE_ bool approximately(const IsoRect &p_other) const { return x.approximately(p_other.x) && y.approximately(p_other.y); }

	_FORCE_INLINE_ static IsoRect merge(const IsoRect &a, const IsoRect &b) {
		IsoRect result;
		result.x = IsoMinMax::merge(a.x, b.x);
		result.y = IsoMinMax::merge(a.y, b.y);
		return result;
	}

	_FORCE_INLINE_ IsoRect() {}
	_FORCE_INLINE_ IsoRect(real_t p_min_x, real_t p_min_y, real_t p_max_x, real_t p_max_y) {
		x.set(p_min_x, p_max_x);
		y.set(p_min_y, p_max_y);
	}
	_FORCE_INLINE_ IsoRect(const Vector2 &p_min, const Vector2 &p_max) {
		x.set(p_min.x, p_max.x);
		y.set(p_min.y, p_max.y);
	}
	_FORCE_INLINE_ IsoRect(const IsoMinMax &p_x, const IsoMinMax &p_y) {
		x.set(p_x);
		y.set(p_y);
	}

	static const IsoRect ZERO;
};

// --- IsoUtils namespace ---

namespace IsoUtils {

static const int FLOAT_BEAUTIFIER_DIGITS = 4;

// Vector construction helpers

_FORCE_INLINE_ Vector2 vec2_from(real_t p_v) { return Vector2(p_v, p_v); }
_FORCE_INLINE_ Vector3 vec3_from_x(real_t p_x) { return Vector3(p_x, 0, 0); }
_FORCE_INLINE_ Vector3 vec3_from_y(real_t p_y) { return Vector3(0, p_y, 0); }
_FORCE_INLINE_ Vector3 vec3_from_z(real_t p_z) { return Vector3(0, 0, p_z); }
_FORCE_INLINE_ Vector3 vec3_from_xy(real_t p_x, real_t p_y) { return Vector3(p_x, p_y, 0); }
_FORCE_INLINE_ Vector3 vec3_from_yz(real_t p_y, real_t p_z) { return Vector3(0, p_y, p_z); }
_FORCE_INLINE_ Vector3 vec3_from_xz(real_t p_x, real_t p_z) { return Vector3(p_x, 0, p_z); }
_FORCE_INLINE_ Vector3 vec3_from_vec2(const Vector2 &p_v, real_t p_z = 0) { return Vector3(p_v.x, p_v.y, p_z); }

// Vector component change helpers

_FORCE_INLINE_ Vector3 vec3_change_x(const Vector3 &p_v, real_t p_x) { return Vector3(p_x, p_v.y, p_v.z); }
_FORCE_INLINE_ Vector3 vec3_change_y(const Vector3 &p_v, real_t p_y) { return Vector3(p_v.x, p_y, p_v.z); }
_FORCE_INLINE_ Vector3 vec3_change_z(const Vector3 &p_v, real_t p_z) { return Vector3(p_v.x, p_v.y, p_z); }
_FORCE_INLINE_ Vector3 vec3_change_xy(const Vector3 &p_v, real_t p_x, real_t p_y) { return Vector3(p_x, p_y, p_v.z); }
_FORCE_INLINE_ Vector3 vec3_change_yz(const Vector3 &p_v, real_t p_y, real_t p_z) { return Vector3(p_v.x, p_y, p_z); }
_FORCE_INLINE_ Vector3 vec3_change_xz(const Vector3 &p_v, real_t p_x, real_t p_z) { return Vector3(p_x, p_v.y, p_z); }

// Vector min/max/abs

_FORCE_INLINE_ Vector2 vec2_min(const Vector2 &a, const Vector2 &b) { return Vector2(MIN(a.x, b.x), MIN(a.y, b.y)); }
_FORCE_INLINE_ Vector2 vec2_max(const Vector2 &a, const Vector2 &b) { return Vector2(MAX(a.x, b.x), MAX(a.y, b.y)); }
_FORCE_INLINE_ Vector2 vec2_abs(const Vector2 &p_v) { return Vector2(Math::abs(p_v.x), Math::abs(p_v.y)); }
_FORCE_INLINE_ Vector3 vec3_abs(const Vector3 &p_v) { return Vector3(Math::abs(p_v.x), Math::abs(p_v.y), Math::abs(p_v.z)); }

_FORCE_INLINE_ real_t vec2_min_f(const Vector2 &p_v) { return MIN(p_v.x, p_v.y); }
_FORCE_INLINE_ real_t vec2_max_f(const Vector2 &p_v) { return MAX(p_v.x, p_v.y); }
_FORCE_INLINE_ real_t vec3_min_f(const Vector3 &p_v) { return MIN(MIN(p_v.x, p_v.y), p_v.z); }
_FORCE_INLINE_ real_t vec3_max_f(const Vector3 &p_v) { return MAX(MAX(p_v.x, p_v.y), p_v.z); }

// Vector floor/ceil/round

_FORCE_INLINE_ Vector2 vec2_floor(const Vector2 &p_v) { return Vector2(Math::floor(p_v.x), Math::floor(p_v.y)); }
_FORCE_INLINE_ Vector2 vec2_ceil(const Vector2 &p_v) { return Vector2(Math::ceil(p_v.x), Math::ceil(p_v.y)); }
_FORCE_INLINE_ Vector2 vec2_round(const Vector2 &p_v) { return Vector2(Math::round(p_v.x), Math::round(p_v.y)); }
_FORCE_INLINE_ Vector3 vec3_floor(const Vector3 &p_v) { return Vector3(Math::floor(p_v.x), Math::floor(p_v.y), Math::floor(p_v.z)); }
_FORCE_INLINE_ Vector3 vec3_ceil(const Vector3 &p_v) { return Vector3(Math::ceil(p_v.x), Math::ceil(p_v.y), Math::ceil(p_v.z)); }
_FORCE_INLINE_ Vector3 vec3_round(const Vector3 &p_v) { return Vector3(Math::round(p_v.x), Math::round(p_v.y), Math::round(p_v.z)); }

// Approximate comparisons

_FORCE_INLINE_ bool vec2_approximately(const Vector2 &a, const Vector2 &b) {
	return Math::is_equal_approx(a.x, b.x) && Math::is_equal_approx(a.y, b.y);
}
_FORCE_INLINE_ bool vec2_approximately(const Vector2 &a, const Vector2 &b, real_t p_precision) {
	return Math::abs(a.x - b.x) < p_precision && Math::abs(a.y - b.y) < p_precision;
}
_FORCE_INLINE_ bool vec3_approximately(const Vector3 &a, const Vector3 &b) {
	return Math::is_equal_approx(a.x, b.x) && Math::is_equal_approx(a.y, b.y) && Math::is_equal_approx(a.z, b.z);
}
_FORCE_INLINE_ bool vec3_approximately(const Vector3 &a, const Vector3 &b, real_t p_precision) {
	return Math::abs(a.x - b.x) < p_precision && Math::abs(a.y - b.y) < p_precision && Math::abs(a.z - b.z) < p_precision;
}

// Beautifier (round to N decimal places)

_FORCE_INLINE_ real_t float_beautifier(real_t p_v) {
	real_t factor = Math::pow(10.0f, (real_t)FLOAT_BEAUTIFIER_DIGITS);
	return Math::round(p_v * factor) / factor;
}

_FORCE_INLINE_ Vector2 vector_beautifier(const Vector2 &p_v) {
	return Vector2(float_beautifier(p_v.x), float_beautifier(p_v.y));
}

_FORCE_INLINE_ Vector3 vector_beautifier(const Vector3 &p_v) {
	return Vector3(float_beautifier(p_v.x), float_beautifier(p_v.y), float_beautifier(p_v.z));
}

// Color helpers

_FORCE_INLINE_ Color color_change_a(const Color &p_color, real_t p_a) {
	return Color(p_color.r, p_color.g, p_color.b, p_a);
}

} // namespace IsoUtils

#endif // ISO_UTILS_H
