/**************************************************************************/
/*  gd_math.h                                                             */
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

#ifndef GD_MATH_H
#define GD_MATH_H

#include "core/math/math_defs.h"
#include "core/math/math_funcs.h"

#define Math_360_Deg 360
#define Math_180_Deg 180

constexpr real_t Math_PI_by_180 = Math_PI / 180;
constexpr real_t Math_180_by_PI = 180 / Math_PI;

#define Math_Half_PI 1.57079632679
#define Math_Two_PI 6.28318530717958647692
#define Math_PI_Deg 0.0174532925
#define Math_Deg_PI 57.2957795130

#define Degrees_To_Radians(a) ((a)*Math_PI_Deg)
#define Radians_To_Degree(a) ((a)*Math_Deg_PI)

#if defined __STRICT_ANSI__
#define _hypot(x, y) Math::sqrt((x) * (x) + (y) * (y))
#else
#ifdef REAL_T_IS_DOUBLE
static inline double _hypot(double x, double y) {
	return hypot(x, y);
}
#else
static inline float _hypot(float x, float y) {
	return hypotf(x, y);
}
#endif
#endif

#endif // GD_MATH_H
