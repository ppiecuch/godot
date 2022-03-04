/*************************************************************************/
/*  _stdlib.cpp                                                          */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "core/math/math_funcs.h"

extern "C" double SDL_atan(double x) {
	return Math::atan(x);
}
extern "C" float SDL_atanf(float x) {
	return Math::atan(x);
}

extern "C" double SDL_atan2(double y, double x) {
	return Math::atan2(y, x);
}
extern "C" float SDL_atan2f(float y, float x) {
	return Math::atan2(y, x);
}

extern "C" double SDL_acos(double val) {
	return Math::acos(val);
}
extern "C" float SDL_acosf(float val) {
	return Math::acos(val);
}

extern "C" double SDL_asin(double val) {
	return Math::asin(val);
}
extern "C" float SDL_asinf(float val) {
	return Math::asin(val);
}

extern "C" double SDL_ceil(double x) {
	return Math::ceil(x);
}
extern "C" float SDL_ceilf(float x) {
	return Math::ceil(x);
}

extern "C" double SDL_cos(double x) {
	return Math::cos(x);
}
extern "C" float SDL_cosf(float x) {
	return Math::cos(x);
}

extern "C" double SDL_exp(double x) {
	return Math::exp(x);
}
extern "C" float SDL_expf(float x) {
	return Math::exp(x);
}

extern "C" double SDL_fabs(double x) {
	return Math::absf(x);
}
extern "C" float SDL_fabsf(float x) {
	return Math::absf(x);
}

extern "C" double SDL_floor(double x) {
	return Math::floor(x);
}
extern "C" float SDL_floorf(float x) {
	return Math::floor(x);
}

extern "C" double SDL_fmod(double x, double y) {
	return Math::fmod(x, y);
}
extern "C" float SDL_fmodf(float x, float y) {
	return Math::fmod(x, y);
}

extern "C" double SDL_log(double x) {
	return Math::log(x);
}
extern "C" float SDL_logf(float x) {
	return Math::log(x);
}

extern "C" double SDL_log10(double x) {
	return Math::log10(x);
}
extern "C" float SDL_log10f(float x) {
	return Math::log10(x);
}

extern "C" double SDL_pow(double x, double y) {
	return Math::pow(x, y);
}
extern "C" float SDL_powf(float x, float y) {
	return Math::pow(x, y);
}

extern "C" double SDL_round(double arg) {
	return Math::round(arg);
}
extern "C" float SDL_roundf(float arg) {
	return Math::round(arg);
}

extern "C" double SDL_sin(double x) {
	return Math::sin(x);
}
extern "C" float SDL_sinf(float x) {
	return Math::sin(x);
}

extern "C" double SDL_sqrt(double x) {
	return Math::sqrt(x);
}
extern "C" float SDL_sqrtf(float x) {
	return Math::sqrt(x);
}

extern "C" double SDL_tan(double x) {
	return Math::tan(x);
}
extern "C" float SDL_tanf(float x) {
	return Math::tan(x);
}

extern "C" int SDL_abs(int x) {
	return Math::abs(x);
}

/* vi: set ts=4 sw=4 expandtab: */
