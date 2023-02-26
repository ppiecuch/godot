/**************************************************************************/
/*  TTFMath.h                                                             */
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

#include <cstdint>

namespace TTFCore {

struct vec2f {
	float x, y;

	vec2f();
	vec2f(float x, float y);

	bool operator==(vec2f) const;
	bool operator!=(vec2f) const;

	vec2f &operator+=(vec2f);
	vec2f &operator-=(vec2f);
	vec2f &operator*=(vec2f);
	vec2f &operator/=(vec2f);

	vec2f &operator+=(float);
	vec2f &operator-=(float);
	vec2f &operator*=(float);
	vec2f &operator/=(float);
};

vec2f operator+(vec2f, vec2f);
vec2f operator-(vec2f, vec2f);
vec2f operator*(vec2f, vec2f);
vec2f operator/(vec2f, vec2f);

vec2f operator+(float, vec2f);
vec2f operator-(float, vec2f);
vec2f operator*(float, vec2f);
vec2f operator/(float, vec2f);

vec2f operator+(vec2f, float);
vec2f operator-(vec2f, float);
vec2f operator*(vec2f, float);
vec2f operator/(vec2f, float);

float dot(vec2f, vec2f); // 2d dot product
float cross(vec2f, vec2f); // 2d cross product

struct matrix3x2f {
	float a, b, c, d, e, f;
	matrix3x2f(); // intiailized to the identity
};

vec2f mul(vec2f, const matrix3x2f &);
vec2f normal(vec2f);
vec2f lerp(vec2f, vec2f, float); // linear interpolate between the two points
vec2f quad_lerp(vec2f, vec2f, vec2f, float); // quadratic interpolation

struct vec3f {
	float x, y, z;
	vec3f();
	vec3f(float x, float y, float z);
};

vec3f normal(vec3f);

struct vec4f {
	float x, y, z, w;
	vec4f();
	vec4f(float x, float y, float z, float w);
};

struct vec2t {
	int16_t x, y;

	vec2t();
	vec2t(int16_t x, int16_t y);

	bool operator==(vec2t) const;
	bool operator!=(vec2t) const;

	vec2t &operator+=(vec2t);
	vec2t &operator-=(vec2t);
	vec2t &operator*=(vec2t);
	vec2t &operator/=(vec2t);

	vec2t &operator+=(int16_t);
	vec2t &operator-=(int16_t);
	vec2t &operator*=(int16_t);
	vec2t &operator/=(int16_t);
};

vec2t operator+(vec2t, vec2t);
vec2t operator-(vec2t, vec2t);
vec2t operator*(vec2t, vec2t);
vec2t operator/(vec2t, vec2t);

vec2t operator+(int16_t, vec2t);
vec2t operator-(int16_t, vec2t);
vec2t operator*(int16_t, vec2t);
vec2t operator/(int16_t, vec2t);

vec2t operator+(vec2t, int16_t);
vec2t operator-(vec2t, int16_t);
vec2t operator*(vec2t, int16_t);
vec2t operator/(vec2t, int16_t);

int32_t dot(vec2t, vec2t); // 2d dot product
int32_t cross(vec2t, vec2t); // 2d cross product

struct matrix3x2t {
	int32_t a, b, c, d, e, f; // in 16.16 fixed point format
	matrix3x2t(); // intialized to the identity
};

int32_t fxmul(int32_t, int32_t);
matrix3x2t mul(const matrix3x2t &, const matrix3x2t &);
vec2t mul(vec2t, const matrix3x2t &);

struct vec3t {
	int16_t x, y, z;
	vec3t();
	vec3t(int16_t x, int16_t y, int16_t z);
	vec3t(vec2t);

	operator vec3f() const;
	explicit operator vec2t() const;
};

bool operator==(vec3t, vec3t);
bool operator!=(vec3t, vec3t);

vec3t &operator+=(vec3t &, vec3t);
vec3t &operator-=(vec3t &, vec3t);
vec3t &operator*=(vec3t &, vec3t);
vec3t &operator/=(vec3t &, vec3t);

vec3t &operator+=(vec3t &, int16_t);
vec3t &operator-=(vec3t &, int16_t);
vec3t &operator*=(vec3t &, int16_t);
vec3t &operator/=(vec3t &, int16_t);

vec3t operator+(vec3t, vec3t);
vec3t operator-(vec3t, vec3t);
vec3t operator*(vec3t, vec3t);
vec3t operator/(vec3t, vec3t);

vec3t operator+(int16_t, vec3t);
vec3t operator-(int16_t, vec3t);
vec3t operator*(int16_t, vec3t);
vec3t operator/(int16_t, vec3t);

vec3t operator+(vec3t, int16_t);
vec3t operator-(vec3t, int16_t);
vec3t operator*(vec3t, int16_t);
vec3t operator/(vec3t, int16_t);

int32_t dot(vec3t, vec3t);
vec3t mul(vec3t, const matrix3x2t &);

struct vec4t {
	int16_t x, y, z, w;
	vec4t();
	vec4t(int16_t x, int16_t y, int16_t z, int16_t w);
};

int32_t Abs(int32_t); // absolute value
int32_t IntSqrt(int32_t); // integer square root (negative numbers return 0)
int16_t RoundBy16(int32_t); // divides by 16 bits and rounds
int32_t RoundBy16Large(int32_t); // divides by 16 bits and rounds
int16_t RoundBy2(int16_t); // divides by 2 and rounds
vec2t RoundBy2(vec2t); // divides by 2 and rounds

int32_t GetTriSign(vec2t p0, vec2t p1, vec2t p2);
bool IsContained(vec2t p0, vec2t p1, vec2t p2, vec2t p3);
bool Intersect2D(vec2t p0, vec2t p1, vec2t p2, vec2t p3);
int32_t SqrdLen(vec2t p0, vec2t p1);
vec2t MidPoint(vec2t p0, vec2t p1);
vec2t ScaledNormal(vec2t v, int16_t w); // normalizes the vector and scales by w
vec2t ScaledPerpCCW(vec2t v, int16_t w); // returns a perpendicular vector to v, scaled by w
vec2t ScaledPerpCCW(vec2t p0, vec2t p1, int16_t w); // returns a perpendicular vector to p0 -> p1 scaled by w
vec2t ScaledPerpCCW(vec2t p0, vec2t p1, vec2t p2, int16_t w); // returns the average of the perpendicular vectors of p0 -> p1, p1 -> p2, scaled by w
vec2t lerp(vec2t, vec2t, uint16_t); // linear interpolate
vec2t quad_lerp(vec2t, vec2t, vec2t, uint16_t); // quadratic interpolation

int32_t GetTriSign(vec3t p0, vec3t p1, vec3t p2);
bool IsContained(vec3t p0, vec3t p1, vec3t p2, vec3t p3);
bool Intersect2D(vec3t p0, vec3t p1, vec3t p2, vec3t p3);
int32_t SqrdLen(vec3t p0, vec3t p1);
vec3t MidPoint(vec3t p0, vec3t p1);
vec3t ScaledNormal(vec3t v, int16_t w);
vec3t ScaledPerpCCW(vec3t v, int16_t w);
vec3t ScaledPerpCCW(vec3t p0, vec3t p1, int16_t w);
vec3t ScaledPerpCCW(vec3t p0, vec3t p1, vec3t p2, int16_t w);
vec3t lerp(vec3t, vec3t, uint16_t);
vec3t quad_lerp(vec3t, vec3t, vec3t, uint16_t);

} //namespace TTFCore
