/**************************************************************************/
/*  matrix_2d.h                                                           */
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

#ifndef MATRIX_2D_H
#define MATRIX_2D_H

#include "core/math/math_funcs.h"
#include "core/math/rect2.h"
#include "core/math/vector2.h"

// mathematical utilities
static inline bool _fuzzy_compare(double p1, double p2) {
	return (Math::abs(p1 - p2) * 1000000000000. <= MIN(Math::abs(p1), Math::abs(p2)));
}
static inline bool _fuzzy_compare(float p1, float p2) {
	return (Math::abs(p1 - p2) * 100000.f <= MIN(Math::abs(p1), Math::abs(p2)));
}
static inline bool _fuzzy_is_null(double d) {
	return Math::abs(d) <= 0.000000000001;
}
static inline bool _fuzzy_is_null(float f) {
	return Math::abs(f) <= 0.00001f;
}

// 2D transform matrix
class Matrix2D {
public:
	Matrix2D();
	Matrix2D(real_t m11, real_t m12, real_t m21, real_t m22,
			real_t dx, real_t dy);

	void set_matrix(real_t m11, real_t m12, real_t m21, real_t m22,
			real_t dx, real_t dy);

	real_t m11() const { return _m11; }
	real_t m12() const { return _m12; }
	real_t m21() const { return _m21; }
	real_t m22() const { return _m22; }
	real_t dx() const { return _dx; }
	real_t dy() const { return _dy; }

	void map(int x, int y, int *tx, int *ty) const;
	void map(real_t x, real_t y, real_t *tx, real_t *ty) const;
	Rect2 map_rect(const Rect2 &) const;
	Vector2 map(const Vector2 &p) const;

	void reset();
	inline bool is_identity() const;

	Matrix2D &translate(real_t dx, real_t dy);
	Matrix2D &scale(real_t sx, real_t sy);
	Matrix2D &shear(real_t sh, real_t sv);
	Matrix2D &rotate(real_t a);

	bool is_invertible() const { return !_fuzzy_is_null(_m11 * _m22 - _m12 * _m21); }
	real_t determinant() const { return _m11 * _m22 - _m12 * _m21; }

	Matrix2D inverted(bool *invertible = nullptr) const;

	bool operator==(const Matrix2D &) const;
	bool operator!=(const Matrix2D &) const;

	Matrix2D &operator*=(const Matrix2D &);
	Matrix2D operator*(const Matrix2D &o) const;

private:
	inline Matrix2D(bool) :
			_m11(1.), _m12(0.), _m21(0.), _m22(1.), _dx(0.), _dy(0.) {}
	inline Matrix2D(real_t am11, real_t am12, real_t am21, real_t am22, real_t adx, real_t ady, bool) :
			_m11(am11), _m12(am12), _m21(am21), _m22(am22), _dx(adx), _dy(ady) {}
	real_t _m11, _m12;
	real_t _m21, _m22;
	real_t _dx, _dy;
};

// mathematical semantics
inline Vector2 operator*(const Vector2 &p, const Matrix2D &m) {
	return m.map(p);
}

inline bool Matrix2D::is_identity() const {
	return _fuzzy_is_null(_m11 - 1) && _fuzzy_is_null(_m22 - 1) && _fuzzy_is_null(_m12) && _fuzzy_is_null(_m21) && _fuzzy_is_null(_dx) && _fuzzy_is_null(_dy);
}

inline bool _fuzzy_compare(const Matrix2D &m1, const Matrix2D &m2) {
	return _fuzzy_compare(m1.m11(), m2.m11()) && _fuzzy_compare(m1.m12(), m2.m12()) && _fuzzy_compare(m1.m21(), m2.m21()) && _fuzzy_compare(m1.m22(), m2.m22()) && _fuzzy_compare(m1.dx(), m2.dx()) && _fuzzy_compare(m1.dy(), m2.dy());
}

#endif // MATRIX_2D_H
