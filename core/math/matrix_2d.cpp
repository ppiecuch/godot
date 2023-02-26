/**************************************************************************/
/*  matrix_2d.cpp                                                         */
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

#include "matrix_2d.h"

#include <limits.h>

// some defines to inline some code
#define MAPDOUBLE(x, y, nx, ny)           \
	{                                     \
		real_t fx = x;                    \
		real_t fy = y;                    \
		nx = _m11 * fx + _m21 * fy + _dx; \
		ny = _m12 * fx + _m22 * fy + _dy; \
	}

#define MAPINT(x, y, nx, ny)                           \
	{                                                  \
		real_t fx = x;                                 \
		real_t fy = y;                                 \
		nx = Math::round(_m11 * fx + _m21 * fy + _dx); \
		ny = Math::round(_m12 * fx + _m22 * fy + _dy); \
	}

Matrix2D::Matrix2D() :
		_m11(1.), _m12(0.), _m21(0.), _m22(1.), _dx(0.), _dy(0.) {
}

Matrix2D::Matrix2D(real_t m11, real_t m12, real_t m21, real_t m22, real_t dx, real_t dy) :
		_m11(m11), _m12(m12), _m21(m21), _m22(m22), _dx(dx), _dy(dy) {
}

void Matrix2D::set_matrix(real_t m11, real_t m12, real_t m21, real_t m22, real_t dx, real_t dy) {
	_m11 = m11;
	_m12 = m12;
	_m21 = m21;
	_m22 = m22;
	_dx = dx;
	_dy = dy;
}

void Matrix2D::map(real_t x, real_t y, real_t *tx, real_t *ty) const {
	MAPDOUBLE(x, y, *tx, *ty);
}

void Matrix2D::map(int x, int y, int *tx, int *ty) const {
	MAPINT(x, y, *tx, *ty);
}

Vector2 Matrix2D::map(const Vector2 &point) const {
	real_t fx = point.x;
	real_t fy = point.y;
	return Vector2(_m11 * fx + _m21 * fy + _dx, _m12 * fx + _m22 * fy + _dy);
}

Rect2 Matrix2D::map_rect(const Rect2 &rect) const {
	Rect2 result;
	if (_m12 == 0.0F && _m21 == 0.0F) {
		real_t x = _m11 * rect.position.x + _dx;
		real_t y = _m22 * rect.position.y + _dy;
		real_t w = _m11 * rect.size.width;
		real_t h = _m22 * rect.size.height;
		if (w < 0) {
			w = -w;
			x -= w;
		}
		if (h < 0) {
			h = -h;
			y -= h;
		}
		result = Rect2(x, y, w, h);
	} else {
		real_t x0, y0;
		real_t x, y;
		MAPDOUBLE(rect.position.x, rect.position.y, x0, y0);
		real_t xmin = x0;
		real_t ymin = y0;
		real_t xmax = x0;
		real_t ymax = y0;
		MAPDOUBLE(rect.position.x + rect.size.width, rect.position.y, x, y);
		xmin = MIN(xmin, x);
		ymin = MIN(ymin, y);
		xmax = MAX(xmax, x);
		ymax = MAX(ymax, y);
		MAPDOUBLE(rect.position.x + rect.size.width, rect.position.y + rect.size.height, x, y);
		xmin = MIN(xmin, x);
		ymin = MIN(ymin, y);
		xmax = MAX(xmax, x);
		ymax = MAX(ymax, y);
		MAPDOUBLE(rect.position.x, rect.position.y + rect.size.height, x, y);
		xmin = MIN(xmin, x);
		ymin = MIN(ymin, y);
		xmax = MAX(xmax, x);
		ymax = MAX(ymax, y);
		result = Rect2(xmin, ymin, xmax - xmin, ymax - ymin);
	}
	return result;
}

void Matrix2D::reset() {
	_m11 = _m22 = 1.0;
	_m12 = _m21 = _dx = _dy = 0.0;
}

Matrix2D &Matrix2D::translate(real_t dx, real_t dy) {
	_dx += dx * _m11 + dy * _m21;
	_dy += dy * _m22 + dx * _m12;
	return *this;
}

Matrix2D &Matrix2D::scale(real_t sx, real_t sy) {
	_m11 *= sx;
	_m12 *= sx;
	_m21 *= sy;
	_m22 *= sy;
	return *this;
}

Matrix2D &Matrix2D::shear(real_t sh, real_t sv) {
	real_t tm11 = sv * _m21;
	real_t tm12 = sv * _m22;
	real_t tm21 = sh * _m11;
	real_t tm22 = sh * _m12;
	_m11 += tm11;
	_m12 += tm12;
	_m21 += tm21;
	_m22 += tm22;
	return *this;
}

const real_t deg2rad = real_t(0.017453292519943295769); // pi/180

Matrix2D &Matrix2D::rotate(real_t a) {
	real_t sina = 0;
	real_t cosa = 0;
	if (a == 90. || a == -270.)
		sina = 1.;
	else if (a == 270. || a == -90.)
		sina = -1.;
	else if (a == 180.)
		cosa = -1.;
	else {
		real_t b = deg2rad * a; // convert to radians
		sina = Math::sin(b); // fast and convenient
		cosa = Math::cos(b);
	}
	real_t tm11 = cosa * _m11 + sina * _m21;
	real_t tm12 = cosa * _m12 + sina * _m22;
	real_t tm21 = -sina * _m11 + cosa * _m21;
	real_t tm22 = -sina * _m12 + cosa * _m22;
	_m11 = tm11;
	_m12 = tm12;
	_m21 = tm21;
	_m22 = tm22;
	return *this;
}

Matrix2D Matrix2D::inverted(bool *invertible) const {
	real_t dtr = determinant();
	if (dtr == 0.0) {
		if (invertible)
			*invertible = false; // singular matrix
		return Matrix2D(true);
	} else { // invertible matrix
		if (invertible)
			*invertible = true;
		real_t dinv = 1.0 / dtr;
		return Matrix2D((_m22 * dinv), (-_m12 * dinv),
				(-_m21 * dinv), (_m11 * dinv),
				((_m21 * _dy - _m22 * _dx) * dinv),
				((_m12 * _dx - _m11 * _dy) * dinv),
				true);
	}
}

bool Matrix2D::operator==(const Matrix2D &m) const {
	return _m11 == m._m11 &&
			_m12 == m._m12 &&
			_m21 == m._m21 &&
			_m22 == m._m22 &&
			_dx == m._dx &&
			_dy == m._dy;
}

bool Matrix2D::operator!=(const Matrix2D &m) const {
	return _m11 != m._m11 ||
			_m12 != m._m12 ||
			_m21 != m._m21 ||
			_m22 != m._m22 ||
			_dx != m._dx ||
			_dy != m._dy;
}

Matrix2D &Matrix2D::operator*=(const Matrix2D &m) {
	real_t tm11 = _m11 * m._m11 + _m12 * m._m21;
	real_t tm12 = _m11 * m._m12 + _m12 * m._m22;
	real_t tm21 = _m21 * m._m11 + _m22 * m._m21;
	real_t tm22 = _m21 * m._m12 + _m22 * m._m22;

	real_t tdx = _dx * m._m11 + _dy * m._m21 + m._dx;
	real_t tdy = _dx * m._m12 + _dy * m._m22 + m._dy;

	_m11 = tm11;
	_m12 = tm12;
	_m21 = tm21;
	_m22 = tm22;
	_dx = tdx;
	_dy = tdy;
	return *this;
}

Matrix2D Matrix2D::operator*(const Matrix2D &m) const {
	real_t tm11 = _m11 * m._m11 + _m12 * m._m21;
	real_t tm12 = _m11 * m._m12 + _m12 * m._m22;
	real_t tm21 = _m21 * m._m11 + _m22 * m._m21;
	real_t tm22 = _m21 * m._m12 + _m22 * m._m22;

	real_t tdx = _dx * m._m11 + _dy * m._m21 + m._dx;
	real_t tdy = _dx * m._m12 + _dy * m._m22 + m._dy;
	return Matrix2D(tm11, tm12, tm21, tm22, tdx, tdy, true);
}
