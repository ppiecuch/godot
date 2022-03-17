/*************************************************************************/
/*  _metal_common.h                                                      */
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

#ifndef METAL_COMMON_H
#define METAL_COMMON_H

#ifndef SKIP_STD_HEADERS
#include <simd/simd.h>
#include <metal_stdlib>
#endif

using namespace metal;

// Full screen triangle's vertices
constant float2 gCorners[3] = { float2(-1.0f, -1.0f), float2(3.0f, -1.0f), float2(-1.0f, 3.0f) };

// Macros
#define MTL_KERNEL_GUARD(IDX, MAX_COUNT) \
	if (IDX >= MAX_COUNT) {              \
		return;                          \
	}

// Functions
float linstep(float low, float high, float v) {
	return clamp((v - low) / (high - low), 0.0f, 1.0f);
}

float random(float2 p) {
	return fract(sin(dot(p, float2(15.79, 81.93)) * 45678.9123));
}

float3 hsv2rgb(float h, float s, float v) {
	float3 a = fract(h + float3(0.0, 2.0, 1.0) / 3.0) * 6.0 - 3.0;
	a = clamp(abs(a) - 1.0, 0.0, 1.0) - 1.0;
	a = a * s + 1.0;
	return a * v;
}

float grid(float2 p) {
	float g = 0.0;
	p = fract(p);
	g = max(g, step(0.98, p.x));
	g = max(g, step(0.98, p.y));
	return g;
}

float noise(float2 p) {
	float2 i = floor(p);
	float2 f = fract(p);
	f = f * f * (3.0 - 2.0 * f);
	float bottom = mix(random(i + float2(0)), random(i + float2(1.0, 0.0)), f.x);
	float top = mix(random(i + float2(0.0, 1.0)), random(i + float2(1)), f.x);
	float t = mix(bottom, top, f.y);
	return t;
}

float fbm(float2 uv) {
	float sum = 0;
	float amp = 0.7;
	for (int i = 0; i < 4; ++i) {
		sum += noise(uv) * amp;
		uv += uv * 1.2;
		amp *= 0.4;
	}
	return sum;
}

float2 select2(float2 a, float2 b, bool2 c) {
	float2 ret;

	ret.x = c.x ? b.x : a.x;
	ret.y = c.y ? b.y : a.y;

	return ret;
}

float3 select3(float3 a, float3 b, bool3 c) {
	float3 ret;

	ret.x = c.x ? b.x : a.x;
	ret.y = c.y ? b.y : a.y;
	ret.z = c.z ? b.z : a.z;

	return ret;
}

float4 select4(float4 a, float4 b, bool4 c) {
	float4 ret;

	ret.x = c.x ? b.x : a.x;
	ret.y = c.y ? b.y : a.y;
	ret.z = c.z ? b.z : a.z;
	ret.w = c.w ? b.w : a.w;

	return ret;
}

template <typename T>
float4 texel2DFetch(texture2d<T> tex, sampler smp, int2 size, int2 coord) {
	float x_coord = float(2 * coord.x + 1) / float(size.x * 2);
	float y_coord = float(2 * coord.y + 1) / float(size.y * 2);

	return tex.sample(smp, float2(x_coord, y_coord), 0.0);
}

#if defined(SINH_USED)

float sinh(float x) {
	return 0.5 * (exp(x) - exp(-x));
}

float2 sinh(float2 x) {
	return 0.5 * float2(exp(x.x) - exp(-x.x), exp(x.y) - exp(-x.y));
}

float3 sinh(float3 x) {
	return 0.5 * float3(exp(x.x) - exp(-x.x), exp(x.y) - exp(-x.y), exp(x.z) - exp(-x.z));
}

float4 sinh(float4 x) {
	return 0.5 * float4(exp(x.x) - exp(-x.x), exp(x.y) - exp(-x.y), exp(x.z) - exp(-x.z), exp(x.w) - exp(-x.w));
}

#endif

#if defined(COSH_USED)

float cosh(float x) {
	return 0.5 * (exp(x) + exp(-x));
}

float2 cosh(float2 x) {
	return 0.5 * float2(exp(x.x) + exp(-x.x), exp(x.y) + exp(-x.y));
}

float3 cosh(float3 x) {
	return 0.5 * float3(exp(x.x) + exp(-x.x), exp(x.y) + exp(-x.y), exp(x.z) + exp(-x.z));
}

float4 cosh(float4 x) {
	return 0.5 * float4(exp(x.x) + exp(-x.x), exp(x.y) + exp(-x.y), exp(x.z) + exp(-x.z), exp(x.w) + exp(-x.w));
}

#endif

#if defined(TANH_USED)

float tanh(float x) {
	float exp2x = exp(2.0 * x);
	return (exp2x - 1.0) / (exp2x + 1.0);
}

float2 tanh(float2 x) {
	float exp2x = exp(2.0 * x.x);
	float exp2y = exp(2.0 * x.y);
	return float2((exp2x - 1.0) / (exp2x + 1.0), (exp2y - 1.0) / (exp2y + 1.0));
}

float3 tanh(float3 x) {
	float exp2x = exp(2.0 * x.x);
	float exp2y = exp(2.0 * x.y);
	float exp2z = exp(2.0 * x.z);
	return float3((exp2x - 1.0) / (exp2x + 1.0), (exp2y - 1.0) / (exp2y + 1.0), (exp2z - 1.0) / (exp2z + 1.0));
}

float4 tanh(float4 x) {
	float exp2x = exp(2.0 * x.x);
	float exp2y = exp(2.0 * x.y);
	float exp2z = exp(2.0 * x.z);
	float exp2w = exp(2.0 * x.w);
	return float4((exp2x - 1.0) / (exp2x + 1.0), (exp2y - 1.0) / (exp2y + 1.0), (exp2z - 1.0) / (exp2z + 1.0), (exp2w - 1.0) / (exp2w + 1.0));
}

#endif

#if defined(ASINH_USED)

float asinh(float x) {
	return sign(x) * log(abs(x) + sqrt(1.0 + x * x));
}

float2 asinh(float2 x) {
	return float2(sign(x.x) * log(abs(x.x) + sqrt(1.0 + x.x * x.x)), sign(x.y) * log(abs(x.y) + sqrt(1.0 + x.y * x.y)));
}

float3 asinh(float3 x) {
	return float3(sign(x.x) * log(abs(x.x) + sqrt(1.0 + x.x * x.x)), sign(x.y) * log(abs(x.y) + sqrt(1.0 + x.y * x.y)), sign(x.z) * log(abs(x.z) + sqrt(1.0 + x.z * x.z)));
}

float4 asinh(float4 x) {
	return float4(sign(x.x) * log(abs(x.x) + sqrt(1.0 + x.x * x.x)), sign(x.y) * log(abs(x.y) + sqrt(1.0 + x.y * x.y)), sign(x.z) * log(abs(x.z) + sqrt(1.0 + x.z * x.z)), sign(x.w) * log(abs(x.w) + sqrt(1.0 + x.w * x.w)));
}

#endif

#if defined(ACOSH_USED)

float acosh(float x) {
	return log(x + sqrt(x * x - 1.0));
}

float2 acosh(float2 x) {
	return float2(log(x.x + sqrt(x.x * x.x - 1.0)), log(x.y + sqrt(x.y * x.y - 1.0)));
}

float3 acosh(float3 x) {
	return float3(log(x.x + sqrt(x.x * x.x - 1.0)), log(x.y + sqrt(x.y * x.y - 1.0)), log(x.z + sqrt(x.z * x.z - 1.0)));
}

float4 acosh(float4 x) {
	return float4(log(x.x + sqrt(x.x * x.x - 1.0)), log(x.y + sqrt(x.y * x.y - 1.0)), log(x.z + sqrt(x.z * x.z - 1.0)), log(x.w + sqrt(x.w * x.w - 1.0)));
}

#endif

#if defined(ATANH_USED)

float atanh(float x) {
	return 0.5 * log((1.0 + x) / (1.0 - x));
}

float2 atanh(float2 x) {
	return 0.5 * float2(log((1.0 + x.x) / (1.0 - x.x)), log((1.0 + x.y) / (1.0 - x.y)));
}

float3 atanh(float3 x) {
	return 0.5 * float3(log((1.0 + x.x) / (1.0 - x.x)), log((1.0 + x.y) / (1.0 - x.y)), log((1.0 + x.z) / (1.0 - x.z)));
}

float4 atanh(float4 x) {
	return 0.5 * float4(log((1.0 + x.x) / (1.0 - x.x)), log((1.0 + x.y) / (1.0 - x.y)), log((1.0 + x.z) / (1.0 - x.z)), log((1.0 + x.w) / (1.0 - x.w)));
}

#endif

#if defined(ROUND_USED)

float round(float x) {
	return floor(x + 0.5);
}

float2 round(float2 x) {
	return floor(x + float2(0.5));
}

float3 round(float3 x) {
	return floor(x + float3(0.5));
}

float4 round(float4 x) {
	return floor(x + float4(0.5));
}

#endif

#if defined(ROUND_EVEN_USED)

float roundEven(float x) {
	float t = x + 0.5;
	float f = floor(t);
	float r;
	if (t == f) {
		if (x > 0)
			r = f - mod(f, 2);
		else
			r = f + mod(f, 2);
	} else
		r = f;
	return r;
}

float2 roundEven(float2 x) {
	return float2(roundEven(x.x), roundEven(x.y));
}

float3 roundEven(float3 x) {
	return float3(roundEven(x.x), roundEven(x.y), roundEven(x.z));
}

float4 roundEven(float4 x) {
	return float4(roundEven(x.x), roundEven(x.y), roundEven(x.z), roundEven(x.w));
}

#endif

#if defined(IS_INF_USED)

bool isinf(float x) {
	return (2 * x == x) && (x != 0);
}

bool2 isinf(float2 x) {
	return bool2((2 * x.x == x.x) && (x.x != 0), (2 * x.y == x.y) && (x.y != 0));
}

bool3 isinf(float3 x) {
	return bool3((2 * x.x == x.x) && (x.x != 0), (2 * x.y == x.y) && (x.y != 0), (2 * x.z == x.z) && (x.z != 0));
}

bool4 isinf(float4 x) {
	return bool4((2 * x.x == x.x) && (x.x != 0), (2 * x.y == x.y) && (x.y != 0), (2 * x.z == x.z) && (x.z != 0), (2 * x.w == x.w) && (x.w != 0));
}

#endif

#if defined(IS_NAN_USED)

bool isnan(float x) {
	return x != x;
}

bool2 isnan(float2 x) {
	return bool2(x.x != x.x, x.y != x.y);
}

bool3 isnan(float3 x) {
	return bool3(x.x != x.x, x.y != x.y, x.z != x.z);
}

bool4 isnan(float4 x) {
	return bool4(x.x != x.x, x.y != x.y, x.z != x.z, x.w != x.w);
}

#endif

#if defined(TRUNC_USED)

float trunc(float x) {
	return x < 0 ? -floor(-x) : floor(x);
}

float2 trunc(float2 x) {
	return float2(x.x < 0 ? -floor(-x.x) : floor(x.x), x.y < 0 ? -floor(-x.y) : floor(x.y));
}

float3 trunc(float3 x) {
	return float3(x.x < 0 ? -floor(-x.x) : floor(x.x), x.y < 0 ? -floor(-x.y) : floor(x.y), x.z < 0 ? -floor(-x.z) : floor(x.z));
}

float4 trunc(float4 x) {
	return float4(x.x < 0 ? -floor(-x.x) : floor(x.x), x.y < 0 ? -floor(-x.y) : floor(x.y), x.z < 0 ? -floor(-x.z) : floor(x.z), x.w < 0 ? -floor(-x.w) : floor(x.w));
}

#endif

#if defined(DETERMINANT_USED)

float determinant(float2x2 m) {
	return m[0].x * m[1].y - m[1].x * m[0].y;
}

float determinant(float3x3 m) {
	return m[0].x * (m[1].y * m[2].z - m[2].y * m[1].z) - m[1].x * (m[0].y * m[2].z - m[2].y * m[0].z) + m[2].x * (m[0].y * m[1].z - m[1].y * m[0].z);
}

float determinant(float4x4 m) {
	float s00 = m[2].z * m[3].w - m[3].z * m[2].w;
	float s01 = m[2].y * m[3].w - m[3].y * m[2].w;
	float s02 = m[2].y * m[3].z - m[3].y * m[2].z;
	float s03 = m[2].x * m[3].w - m[3].x * m[2].w;
	float s04 = m[2].x * m[3].z - m[3].x * m[2].z;
	float s05 = m[2].x * m[3].y - m[3].x * m[2].y;
	float4 c = float4((m[1].y * s00 - m[1].z * s01 + m[1].w * s02), -(m[1].x * s00 - m[1].z * s03 + m[1].w * s04), (m[1].x * s01 - m[1].y * s03 + m[1].w * s05), -(m[1].x * s02 - m[1].y * s04 + m[1].z * s05));
	return m[0].x * c.x + m[0].y * c.y + m[0].z * c.z + m[0].w * c.w;
}

#endif

#if defined(INVERSE_USED)

float2x2 inverse(float2x2 m) {
	float d = 1.0 / (m[0].x * m[1].y - m[1].x * m[0].y);
	return float2x2(
			float2(m[1].y * d, -m[0].y * d),
			float2(-m[1].x * d, m[0].x * d));
}

float3x3 inverse(float3x3 m) {
	float c01 = m[2].z * m[1].y - m[1].z * m[2].y;
	float c11 = -m[2].z * m[1].x + m[1].z * m[2].x;
	float c21 = m[2].y * m[1].x - m[1].y * m[2].x;
	float d = 1.0 / (m[0].x * c01 + m[0].y * c11 + m[0].z * c21);

	return float3x3(c01, (-m[2].z * m[0].y + m[0].z * m[2].y), (m[1].z * m[0].y - m[0].z * m[1].y),
				   c11, (m[2].z * m[0].x - m[0].z * m[2].x), (-m[1].z * m[0].x + m[0].z * m[1].x),
				   c21, (-m[2].y * m[0].x + m[0].y * m[2].x), (m[1].y * m[0].x - m[0].y * m[1].x)) *
			d;
}

float4x4 inverse(float4x4 m) {
	float c00 = m[2].z * m[3].w - m[3].z * m[2].w;
	float c02 = m[1].z * m[3].w - m[3].z * m[1].w;
	float c03 = m[1].z * m[2].w - m[2].z * m[1].w;

	float c04 = m[2].y * m[3].w - m[3].y * m[2].w;
	float c06 = m[1].y * m[3].w - m[3].y * m[1].w;
	float c07 = m[1].y * m[2].w - m[2].y * m[1].w;

	float c08 = m[2].y * m[3].z - m[3].y * m[2].z;
	float c10 = m[1].y * m[3].z - m[3].y * m[1].z;
	float c11 = m[1].y * m[2].z - m[2].y * m[1].z;

	float c12 = m[2].x * m[3].w - m[3].x * m[2].w;
	float c14 = m[1].x * m[3].w - m[3].x * m[1].w;
	float c15 = m[1].x * m[2].w - m[2].x * m[1].w;

	float c16 = m[2].x * m[3].z - m[3].x * m[2].z;
	float c18 = m[1].x * m[3].z - m[3].x * m[1].z;
	float c19 = m[1].x * m[2].z - m[2].x * m[1].z;

	float c20 = m[2].x * m[3].y - m[3].x * m[2].y;
	float c22 = m[1].x * m[3].y - m[3].x * m[1].y;
	float c23 = m[1].x * m[2].y - m[2].x * m[1].y;

	float4 f0 = float4(c00, c00, c02, c03);
	float4 f1 = float4(c04, c04, c06, c07);
	float4 f2 = float4(c08, c08, c10, c11);
	float4 f3 = float4(c12, c12, c14, c15);
	float4 f4 = float4(c16, c16, c18, c19);
	float4 f5 = float4(c20, c20, c22, c23);

	float4 v0 = float4(m[1].x, m[0].x, m[0].x, m[0].x);
	float4 v1 = float4(m[1].y, m[0].y, m[0].y, m[0].y);
	float4 v2 = float4(m[1].z, m[0].z, m[0].z, m[0].z);
	float4 v3 = float4(m[1].w, m[0].w, m[0].w, m[0].w);

	float4 inv0 = float4(v1 * f0 - v2 * f1 + v3 * f2);
	float4 inv1 = float4(v0 * f0 - v2 * f3 + v3 * f4);
	float4 inv2 = float4(v0 * f1 - v1 * f3 + v3 * f5);
	float4 inv3 = float4(v0 * f2 - v1 * f4 + v2 * f5);

	float4 sa = float4(+1, -1, +1, -1);
	float4 sb = float4(-1, +1, -1, +1);

	float4x4 inv = float4x4(inv0 * sa, inv1 * sb, inv2 * sa, inv3 * sb);

	float4 r0 = float4(inv[0].x, inv[1].x, inv[2].x, inv[3].x);
	float4 d0 = float4(m[0] * r0);

	float d1 = (d0.x + d0.y) + (d0.z + d0.w);
	float d = 1.0 / d1;

	return inv * d;
}

#endif

#if defined(TRANSPOSE_USED)

float2x2 transpose(float2x2 m) {
	return float2x2(
			float2(m[0].x, m[1].x),
			float2(m[0].y, m[1].y));
}

float3x3 transpose(float3x3 m) {
	return float3x3(
			float3(m[0].x, m[1].x, m[2].x),
			float3(m[0].y, m[1].y, m[2].y),
			float3(m[0].z, m[1].z, m[2].z));
}

#endif

float4x4 transpose(float4x4 m) {
	return float4x4(
			float4(m[0].x, m[1].x, m[2].x, m[3].x),
			float4(m[0].y, m[1].y, m[2].y, m[3].y),
			float4(m[0].z, m[1].z, m[2].z, m[3].z),
			float4(m[0].w, m[1].w, m[2].w, m[3].w));
}

#if defined(OUTER_PRODUCT_USED)

float2x2 outerProduct(float2 c, float2 r) {
	return float2x2(c * r.x, c * r.y);
}

float3x3 outerProduct(float3 c, float3 r) {
	return float3x3(c * r.x, c * r.y, c * r.z);
}

float4x4 outerProduct(float4 c, float4 r) {
	return float4x4(c * r.x, c * r.y, c * r.z, c * r.w);
}

#endif

#endif // METAL_COMMON_H
