/**************************************************************************/
/*  tonemap.metal                                                         */
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

#include "_metal_common.h"
#include "_godot_common.h"

// Tonemap-specific uniforms structure
struct TonemapUniforms {
	float exposure;
	float white;
	float auto_exposure_grey;
	float glow_intensity;
	float glow_map_strength;
	float sharpen_intensity;
	float2 pixel_size;
	int2 glow_texture_size;
	float3 bcs; // brightness, contrast, saturation
};

// Vertex input structure
struct TonemapVertexIn {
	float4 position [[attribute(0)]];
	float2 uv [[attribute(4)]];
};

// Interpolated vertex output / fragment input
struct TonemapVertexOut {
	float4 position [[position]];
	float2 uv_interp;
};

// ============================================================================
// Vertex Shader
// ============================================================================

vertex TonemapVertexOut vertexFunction(
	TonemapVertexIn in [[stage_in]],
	constant TonemapUniforms& uniforms [[buffer(1)]]
) {
	TonemapVertexOut out;
	out.position = in.position;
	out.uv_interp = in.uv;

#ifdef V_FLIP
	out.uv_interp.y = 1.0f - out.uv_interp.y;
#endif

	return out;
}

// ============================================================================
// Helper Functions for Bicubic Glow Filtering
// ============================================================================

#ifdef USE_GLOW_FILTER_BICUBIC
// w0, w1, w2, and w3 are the four cubic B-spline basis functions
float w0(float a) {
	return (1.0f / 6.0f) * (a * (a * (-a + 3.0f) - 3.0f) + 1.0f);
}

float w1(float a) {
	return (1.0f / 6.0f) * (a * a * (3.0f * a - 6.0f) + 4.0f);
}

float w2(float a) {
	return (1.0f / 6.0f) * (a * (a * (-3.0f * a + 3.0f) + 3.0f) + 1.0f);
}

float w3(float a) {
	return (1.0f / 6.0f) * (a * a * a);
}

// g0 and g1 are the two amplitude functions
float g0(float a) {
	return w0(a) + w1(a);
}

float g1(float a) {
	return w2(a) + w3(a);
}

// h0 and h1 are the two offset functions
float h0(float a) {
	return -1.0f + w1(a) / (w0(a) + w1(a));
}

float h1(float a) {
	return 1.0f + w3(a) / (w2(a) + w3(a));
}

float4 texture2D_bicubic(texture2d<float> tex, sampler smp, float2 uv, int p_lod, int2 glow_texture_size) {
	float lod = float(p_lod);
	float2 tex_size = float2(glow_texture_size >> p_lod);
	float2 texel_size = float2(1.0f) / tex_size;

	uv = uv * tex_size + float2(0.5f);

	float2 iuv = floor(uv);
	float2 fuv = fract(uv);

	float g0x = g0(fuv.x);
	float g1x = g1(fuv.x);
	float h0x = h0(fuv.x);
	float h1x = h1(fuv.x);
	float h0y = h0(fuv.y);
	float h1y = h1(fuv.y);

	float2 p0 = (float2(iuv.x + h0x, iuv.y + h0y) - float2(0.5f)) * texel_size;
	float2 p1 = (float2(iuv.x + h1x, iuv.y + h0y) - float2(0.5f)) * texel_size;
	float2 p2 = (float2(iuv.x + h0x, iuv.y + h1y) - float2(0.5f)) * texel_size;
	float2 p3 = (float2(iuv.x + h1x, iuv.y + h1y) - float2(0.5f)) * texel_size;

	return (g0(fuv.y) * (g0x * tex.sample(smp, p0, level(lod)) + g1x * tex.sample(smp, p1, level(lod)))) +
		   (g1(fuv.y) * (g0x * tex.sample(smp, p2, level(lod)) + g1x * tex.sample(smp, p3, level(lod))));
}

#define GLOW_TEXTURE_SAMPLE(tex, smp, uv, lod, glow_size) texture2D_bicubic(tex, smp, uv, lod, glow_size)
#else
#define GLOW_TEXTURE_SAMPLE(tex, smp, uv, lod, glow_size) tex.sample(smp, uv, level(float(lod)))
#endif

// ============================================================================
// Tonemapping Functions
// ============================================================================

float3 tonemap_filmic(float3 color, float white) {
	// exposure bias: input scale (color *= bias, white *= bias) to make the brightness consistent with other tonemappers
	// also useful to scale the input to the range that the tonemapper is designed for (some require very high input values)
	// has no effect on the curve's general shape or visual properties
	const float exposure_bias = 2.0f;
	const float A = 0.22f * exposure_bias * exposure_bias; // bias baked into constants for performance
	const float B = 0.30f * exposure_bias;
	const float C = 0.10f;
	const float D = 0.20f;
	const float E = 0.01f;
	const float F = 0.30f;

	float3 color_tonemapped = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
	float white_tonemapped = ((white * (A * white + C * B) + D * E) / (white * (A * white + B) + D * F)) - E / F;

	return clamp(color_tonemapped / white_tonemapped, float3(0.0f), float3(1.0f));
}

float3 tonemap_aces(float3 color, float white) {
	const float exposure_bias = 0.85f;
	const float A = 2.51f * exposure_bias * exposure_bias;
	const float B = 0.03f * exposure_bias;
	const float C = 2.43f * exposure_bias * exposure_bias;
	const float D = 0.59f * exposure_bias;
	const float E = 0.14f;

	float3 color_tonemapped = (color * (A * color + B)) / (color * (C * color + D) + E);
	float white_tonemapped = (white * (A * white + B)) / (white * (C * white + D) + E);

	return clamp(color_tonemapped / white_tonemapped, float3(0.0f), float3(1.0f));
}

// Adapted from https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
// (MIT License).
float3 tonemap_aces_fitted(float3 color, float white) {
	const float exposure_bias = 1.8f;
	const float A = 0.0245786f;
	const float B = 0.000090537f;
	const float C = 0.983729f;
	const float D = 0.432951f;
	const float E = 0.238081f;

	// Exposure bias baked into transform to save shader instructions. Equivalent to `color *= exposure_bias`
	const float3x3 rgb_to_rrt = float3x3(
		float3(0.59719f * exposure_bias, 0.35458f * exposure_bias, 0.04823f * exposure_bias),
		float3(0.07600f * exposure_bias, 0.90834f * exposure_bias, 0.01566f * exposure_bias),
		float3(0.02840f * exposure_bias, 0.13383f * exposure_bias, 0.83777f * exposure_bias));

	const float3x3 odt_to_rgb = float3x3(
		float3(1.60475f, -0.53108f, -0.07367f),
		float3(-0.10208f, 1.10813f, -0.00605f),
		float3(-0.00327f, -0.07276f, 1.07602f));

	color = color * rgb_to_rrt;
	float3 color_tonemapped = (color * (color + A) - B) / (color * (C * color + D) + E);
	color_tonemapped = color_tonemapped * odt_to_rgb;

	white *= exposure_bias;
	float white_tonemapped = (white * (white + A) - B) / (white * (C * white + D) + E);

	return clamp(color_tonemapped / white_tonemapped, float3(0.0f), float3(1.0f));
}

float3 tonemap_reinhard(float3 color, float white) {
	return clamp((white * color + color) / (color * white + white), float3(0.0f), float3(1.0f));
}

float3 linear_to_srgb(float3 color) {
	// convert linear rgb to srgb, assumes clamped input in range [0;1]
	const float3 a = float3(0.055f);
	return select((float3(1.0f) + a) * pow(color, float3(1.0f / 2.4f)) - a, 12.92f * color, color < float3(0.0031308f));
}

// inputs are LINEAR, If Linear tonemapping is selected no transform is performed else outputs are clamped [0, 1] color
float3 apply_tonemapping(float3 color, float white) {
	// Ensure color values are positive.
	// They can be negative in the case of negative lights, which leads to undesired behavior.
#if defined(USE_REINHARD_TONEMAPPER) || defined(USE_FILMIC_TONEMAPPER) || defined(USE_ACES_TONEMAPPER) || defined(USE_ACES_FITTED_TONEMAPPER)
	color = max(float3(0.0f), color);
#endif

#ifdef USE_REINHARD_TONEMAPPER
	return tonemap_reinhard(color, white);
#endif

#ifdef USE_FILMIC_TONEMAPPER
	return tonemap_filmic(color, white);
#endif

#ifdef USE_ACES_TONEMAPPER
	return tonemap_aces(color, white);
#endif

#ifdef USE_ACES_FITTED_TONEMAPPER
	return tonemap_aces_fitted(color, white);
#endif

	return color; // no other selected -> linear: no color transform applied
}

// ============================================================================
// Glow Functions
// ============================================================================

#if defined(USE_GLOW_LEVEL1) || defined(USE_GLOW_LEVEL2) || defined(USE_GLOW_LEVEL3) || defined(USE_GLOW_LEVEL4) || defined(USE_GLOW_LEVEL5) || defined(USE_GLOW_LEVEL6) || defined(USE_GLOW_LEVEL7)
#define USING_GLOW
#endif

#ifdef USING_GLOW
float3 gather_glow(texture2d<float> tex, sampler smp, float2 uv, int2 glow_texture_size) {
	float3 glow = float3(0.0f);

#ifdef USE_GLOW_LEVEL1
	glow += GLOW_TEXTURE_SAMPLE(tex, smp, uv, 1, glow_texture_size).rgb;
#endif

#ifdef USE_GLOW_LEVEL2
	glow += GLOW_TEXTURE_SAMPLE(tex, smp, uv, 2, glow_texture_size).rgb;
#endif

#ifdef USE_GLOW_LEVEL3
	glow += GLOW_TEXTURE_SAMPLE(tex, smp, uv, 3, glow_texture_size).rgb;
#endif

#ifdef USE_GLOW_LEVEL4
	glow += GLOW_TEXTURE_SAMPLE(tex, smp, uv, 4, glow_texture_size).rgb;
#endif

#ifdef USE_GLOW_LEVEL5
	glow += GLOW_TEXTURE_SAMPLE(tex, smp, uv, 5, glow_texture_size).rgb;
#endif

#ifdef USE_GLOW_LEVEL6
	glow += GLOW_TEXTURE_SAMPLE(tex, smp, uv, 6, glow_texture_size).rgb;
#endif

#ifdef USE_GLOW_LEVEL7
	glow += GLOW_TEXTURE_SAMPLE(tex, smp, uv, 7, glow_texture_size).rgb;
#endif

	return glow;
}

float4 apply_glow(float4 color, float3 glow) {
#ifdef USE_GLOW_REPLACE
	color.rgb = glow;
#endif

#ifdef USE_GLOW_SCREEN
	// need color clamping
	color.rgb = clamp(color.rgb, float3(0.0f), float3(1.0f));
	color.rgb = max((color.rgb + glow) - (color.rgb * glow), float3(0.0));
#endif

#ifdef USE_GLOW_SOFTLIGHT
	// need color clamping
	color.rgb = clamp(color.rgb, float3(0.0f), float3(1.0));
	glow = glow * float3(0.5f) + float3(0.5f);

	color.r = (glow.r <= 0.5f) ? (color.r - (1.0f - 2.0f * glow.r) * color.r * (1.0f - color.r)) : (((glow.r > 0.5f) && (color.r <= 0.25f)) ? (color.r + (2.0f * glow.r - 1.0f) * (4.0f * color.r * (4.0f * color.r + 1.0f) * (color.r - 1.0f) + 7.0f * color.r)) : (color.r + (2.0f * glow.r - 1.0f) * (sqrt(color.r) - color.r)));
	color.g = (glow.g <= 0.5f) ? (color.g - (1.0f - 2.0f * glow.g) * color.g * (1.0f - color.g)) : (((glow.g > 0.5f) && (color.g <= 0.25f)) ? (color.g + (2.0f * glow.g - 1.0f) * (4.0f * color.g * (4.0f * color.g + 1.0f) * (color.g - 1.0f) + 7.0f * color.g)) : (color.g + (2.0f * glow.g - 1.0f) * (sqrt(color.g) - color.g)));
	color.b = (glow.b <= 0.5f) ? (color.b - (1.0f - 2.0f * glow.b) * color.b * (1.0f - color.b)) : (((glow.b > 0.5f) && (color.b <= 0.25f)) ? (color.b + (2.0f * glow.b - 1.0f) * (4.0f * color.b * (4.0f * color.b + 1.0f) * (color.b - 1.0f) + 7.0f * color.b)) : (color.b + (2.0f * glow.b - 1.0f) * (sqrt(color.b) - color.b)));
#endif

#if !defined(USE_GLOW_SCREEN) && !defined(USE_GLOW_SOFTLIGHT) && !defined(USE_GLOW_REPLACE)
	// no other selected -> additive
	color.rgb += glow;
#endif

#ifndef USE_GLOW_SOFTLIGHT
	// softlight has no effect on black color
	// compute the alpha from glow
	float a = max(max(glow.r, glow.g), glow.b);
	color.a = a + color.a * (1.0 - a);
	if (color.a == 0.0) {
		color.rgb = float3(0.0);
	} else if (color.a < 1.0) {
		color.rgb /= color.a;
	}
#endif

	return color;
}
#endif // USING_GLOW

// ============================================================================
// Post-processing Functions
// ============================================================================

float3 apply_bcs(float3 color, float3 bcs) {
	color = mix(float3(0.0f), color, bcs.x);
	color = mix(float3(0.5f), color, bcs.y);
	color = mix(float3(dot(float3(1.0f), color) * 0.33333f), color, bcs.z);
	return color;
}

float3 apply_color_correction(float3 color, texture2d<float> correction_tex, sampler smp) {
	color.r = correction_tex.sample(smp, float2(color.r, 0.0f)).r;
	color.g = correction_tex.sample(smp, float2(color.g, 0.0f)).g;
	color.b = correction_tex.sample(smp, float2(color.b, 0.0f)).b;
	return color;
}

#ifdef USE_FXAA
float4 apply_fxaa(float4 color, float exposure, float2 uv_interp, float2 pixel_size,
				  texture2d<float> source, sampler smp) {
	const float FXAA_REDUCE_MIN = (1.0 / 128.0);
	const float FXAA_REDUCE_MUL = (1.0 / 8.0);
	const float FXAA_SPAN_MAX = 8.0;
	const float3 luma = float3(0.299, 0.587, 0.114);

	float4 rgbNW = source.sample(smp, uv_interp + float2(-0.5, -0.5) * pixel_size, level(0.0));
	float4 rgbNE = source.sample(smp, uv_interp + float2(0.5, -0.5) * pixel_size, level(0.0));
	float4 rgbSW = source.sample(smp, uv_interp + float2(-0.5, 0.5) * pixel_size, level(0.0));
	float4 rgbSE = source.sample(smp, uv_interp + float2(0.5, 0.5) * pixel_size, level(0.0));
	float3 rgbM = color.rgb;

#ifdef DISABLE_ALPHA
	float lumaNW = dot(rgbNW.rgb * exposure, luma);
	float lumaNE = dot(rgbNE.rgb * exposure, luma);
	float lumaSW = dot(rgbSW.rgb * exposure, luma);
	float lumaSE = dot(rgbSE.rgb * exposure, luma);
	float lumaM = dot(rgbM * exposure, luma);
#else
	float lumaNW = dot(rgbNW.rgb * exposure, luma) - ((1.0 - rgbNW.a) / 8.0);
	float lumaNE = dot(rgbNE.rgb * exposure, luma) - ((1.0 - rgbNE.a) / 8.0);
	float lumaSW = dot(rgbSW.rgb * exposure, luma) - ((1.0 - rgbSW.a) / 8.0);
	float lumaSE = dot(rgbSE.rgb * exposure, luma) - ((1.0 - rgbSE.a) / 8.0);
	float lumaM = dot(rgbM * exposure, luma) - (color.a / 8.0);
#endif

	float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
	float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

	float2 dir;
	dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
	dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

	float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) *
						  (0.25 * FXAA_REDUCE_MUL),
						  FXAA_REDUCE_MIN);

	float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
	dir = min(float2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
			  max(float2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
				  dir * rcpDirMin)) *
		  pixel_size;

	float4 rgbA = 0.5 * exposure * (source.sample(smp, uv_interp + dir * (1.0 / 3.0 - 0.5), level(0.0)) +
									source.sample(smp, uv_interp + dir * (2.0 / 3.0 - 0.5), level(0.0)));
	float4 rgbB = rgbA * 0.5 + 0.25 * exposure * (source.sample(smp, uv_interp + dir * -0.5, level(0.0)) +
												  source.sample(smp, uv_interp + dir * 0.5, level(0.0)));

#ifdef DISABLE_ALPHA
	float lumaB = dot(rgbB.rgb, luma);
	float4 color_output = ((lumaB < lumaMin) || (lumaB > lumaMax)) ? rgbA : rgbB;
	return float4(color_output.rgb, 1.0);
#else
	float lumaB = dot(rgbB.rgb, luma) - ((1.0 - rgbB.a) / 8.0);
	float4 color_output = ((lumaB < lumaMin) || (lumaB > lumaMax)) ? rgbA : rgbB;
	if (color_output.a == 0.0) {
		color_output.rgb = float3(0.0);
	} else if (color_output.a < 1.0) {
		color_output.rgb /= color_output.a;
	}
	return color_output;
#endif
}
#endif // USE_FXAA

// From http://alex.vlachos.com/graphics/Alex_Vlachos_Advanced_VR_Rendering_GDC2015.pdf
// and https://www.shadertoy.com/view/MslGR8 (5th one starting from the bottom)
// NOTE: `frag_coord` is in pixels (i.e. not normalized UV).
float3 screen_space_dither(float2 frag_coord) {
	// Iestyn's RGB dither (7 asm instructions) from Portal 2 X360, slightly modified for VR.
	float3 dither = float3(dot(float2(171.0, 231.0), frag_coord));
	dither.rgb = fract(dither.rgb / float3(103.0, 71.0, 97.0));

	// Subtract 0.5 to avoid slightly brightening the whole viewport.
	return (dither.rgb - 0.5) / 255.0;
}

#ifdef USE_SHARPENING
// Adapted from https://github.com/DadSchoorse/vkBasalt/blob/b929505ba71dea21d6c32a5a59f2d241592b30c4/src/shader/cas.frag.glsl
// (MIT license).
float3 apply_cas(float3 color, float exposure, float2 uv_interp, float sharpen_intensity,
				 texture2d<float> source, sampler smp) {
	// Fetch a 3x3 neighborhood around the pixel 'e',
	//  a b c
	//  d(e)f
	//  g h i
	float3 a = source.sample(smp, uv_interp, level(0.0), int2(-1, -1)).rgb * exposure;
	float3 b = source.sample(smp, uv_interp, level(0.0), int2(0, -1)).rgb * exposure;
	float3 c = source.sample(smp, uv_interp, level(0.0), int2(1, -1)).rgb * exposure;
	float3 d = source.sample(smp, uv_interp, level(0.0), int2(-1, 0)).rgb * exposure;
	float3 e = color.rgb;
	float3 f = source.sample(smp, uv_interp, level(0.0), int2(1, 0)).rgb * exposure;
	float3 g = source.sample(smp, uv_interp, level(0.0), int2(-1, 1)).rgb * exposure;
	float3 h = source.sample(smp, uv_interp, level(0.0), int2(0, 1)).rgb * exposure;
	float3 i = source.sample(smp, uv_interp, level(0.0), int2(1, 1)).rgb * exposure;

	// Soft min and max.
	//  a b c             b
	//  d e f * 0.5  +  d e f * 0.5
	//  g h i             h
	// These are 2.0x bigger (factored out the extra multiply).
	float3 min_rgb = min(min(min(d, e), min(f, b)), h);
	float3 min_rgb2 = min(min(min(min_rgb, a), min(g, c)), i);
	min_rgb += min_rgb2;

	float3 max_rgb = max(max(max(d, e), max(f, b)), h);
	float3 max_rgb2 = max(max(max(max_rgb, a), max(g, c)), i);
	max_rgb += max_rgb2;

	// Smooth minimum distance to signal limit divided by smooth max.
	float3 rcp_max_rgb = float3(1.0) / max_rgb;
	float3 amp_rgb = clamp((min(min_rgb, 2.0 - max_rgb) * rcp_max_rgb), 0.0, 1.0);

	// Shaping amount of sharpening.
	amp_rgb = rsqrt(amp_rgb);
	float peak = 8.0 - 3.0 * sharpen_intensity;
	float3 w_rgb = -float3(1) / (amp_rgb * peak);
	float3 rcp_weight_rgb = float3(1.0) / (1.0 + 4.0 * w_rgb);

	//                          0 w 0
	//  Filter shape:           w 1 w
	//                          0 w 0
	float3 window = b + d + f + h;

	return max(float3(0.0), (window * w_rgb + e) * rcp_weight_rgb);
}
#endif // USE_SHARPENING

// ============================================================================
// Fragment Shader
// ============================================================================

fragment float4 fragmentFunction(
	TonemapVertexOut in [[stage_in]],
	constant TonemapUniforms& uniforms [[buffer(1)]],
	texture2d<float> source [[texture(0)]],
	sampler sourceSampler [[sampler(0)]]
#ifdef USE_AUTO_EXPOSURE
	, texture2d<float> source_auto_exposure [[texture(1)]]
	, sampler autoExposureSampler [[sampler(1)]]
#endif
#ifdef USING_GLOW
	, texture2d<float> source_glow [[texture(2)]]
	, sampler glowSampler [[sampler(2)]]
	, texture2d<float> glow_map [[texture(3)]]
	, sampler glowMapSampler [[sampler(3)]]
#endif
#ifdef USE_COLOR_CORRECTION
	, texture2d<float> color_correction [[texture(4)]]
	, sampler colorCorrectionSampler [[sampler(4)]]
#endif
) {
	float4 color = source.sample(sourceSampler, in.uv_interp, level(0.0f));

#ifdef DISABLE_ALPHA
	color.a = 1.0;
#endif

	// Exposure
	float full_exposure = uniforms.exposure;

#ifdef USE_AUTO_EXPOSURE
	float auto_exp_value = source_auto_exposure.read(uint2(0, 0)).r;
	full_exposure /= auto_exp_value / uniforms.auto_exposure_grey;
#endif

	color.rgb *= full_exposure;

#ifdef USE_FXAA
	// FXAA must be applied before tonemapping.
	color = apply_fxaa(color, full_exposure, in.uv_interp, uniforms.pixel_size, source, sourceSampler);
#endif

#ifdef USE_SHARPENING
	// CAS gives best results when applied after tonemapping, but `source` isn't tonemapped.
	// As a workaround, apply CAS before tonemapping so that the image still has a correct appearance when tonemapped.
	color.rgb = apply_cas(color.rgb, full_exposure, in.uv_interp, uniforms.sharpen_intensity, source, sourceSampler);
#endif

	// Early Tonemap & SRGB Conversion; note that Linear tonemapping does not clamp to [0, 1]; some operations below expect a [0, 1] range and will clamp
	color.rgb = apply_tonemapping(color.rgb, uniforms.white);

#ifdef KEEP_3D_LINEAR
	// leave color as is (-> don't convert to SRGB)
#else
	// need color clamping
	color.rgb = clamp(color.rgb, float3(0.0f), float3(1.0f));
	color.rgb = linear_to_srgb(color.rgb); // regular linear -> SRGB conversion (needs clamped values)
#endif

	// Glow
#ifdef USING_GLOW
	float3 glow = gather_glow(source_glow, glowSampler, in.uv_interp, uniforms.glow_texture_size) * uniforms.glow_intensity;
	if (uniforms.glow_map_strength > 0.001) {
		glow = mix(glow, glow_map.sample(glowMapSampler, float2(in.uv_interp.x, 1.0 - in.uv_interp.y)).rgb * glow, uniforms.glow_map_strength);
	}

	// high dynamic range -> SRGB
	glow = apply_tonemapping(glow, uniforms.white);
	glow = clamp(glow, float3(0.0f), float3(1.0f));
	glow = linear_to_srgb(glow);

	color = apply_glow(color, glow);
#endif

	// Additional effects

#ifdef USE_BCS
	color.rgb = apply_bcs(color.rgb, uniforms.bcs);
#endif

#ifdef USE_COLOR_CORRECTION
	color.rgb = apply_color_correction(color.rgb, color_correction, colorCorrectionSampler);
#endif

#ifdef USE_DEBANDING
	// Debanding should be done at the end of tonemapping, but before writing to the LDR buffer.
	// Otherwise, we're adding noise to an already-quantized image.
	color.rgb += screen_space_dither(in.position.xy);
#endif

#ifdef DISABLE_ALPHA
	color.a = 1.0;
#endif

	return color;
}
