/**************************************************************************/
/*  copy.metal                                                            */
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

#define M_PI 3.14159265359

// Copy shader uniforms
struct CopyUniforms {
	float4 copy_section;      // USE_COPY_SECTION
	float4x4 display_transform; // USE_DISPLAY_TRANSFORM
	float4x4 sky_transform;   // USE_PANORAMA / USE_ASYM_PANO
	float4 asym_proj;         // USE_ASYM_PANO
	float4x4 pano_transform;  // USE_ASYM_PANO
	float2 pixel_size;
	float3 bcs;               // USE_BCS (brightness, contrast, saturation)
	float mip_level;          // USE_LOD
	float layer;              // USE_TEXTURE3D / USE_TEXTURE2DARRAY
	float multiplier;         // USE_MULTIPLIER
};

// Vertex input structure
struct CopyVertexIn {
	float4 position [[attribute(0)]];
#if defined(USE_CUBEMAP) || defined(USE_PANORAMA)
	float3 cube_in [[attribute(4)]];
#else
	float2 uv_in [[attribute(4)]];
#endif
	float2 uv2_in [[attribute(5)]];
};

// Interpolated vertex output / fragment input
struct CopyVertexOut {
	float4 position [[position]];
#if defined(USE_CUBEMAP) || defined(USE_PANORAMA)
	float3 cube_interp;
#else
	float2 uv_interp;
#endif
	float2 uv2_interp;
};

// ============================================================================
// Vertex Shader
// ============================================================================

vertex CopyVertexOut vertexFunction(
	CopyVertexIn in [[stage_in]],
	constant CopyUniforms& uniforms [[buffer(1)]]
) {
	CopyVertexOut out;

#if defined(USE_CUBEMAP) || defined(USE_PANORAMA)
	out.cube_interp = in.cube_in;
#elif defined(USE_ASYM_PANO)
	out.uv_interp = in.position.xy;
#else
	out.uv_interp = in.uv_in;
#ifdef V_FLIP
	out.uv_interp.y = 1.0 - out.uv_interp.y;
#endif
#endif

	out.uv2_interp = in.uv2_in;
	out.position = in.position;

#ifdef USE_COPY_SECTION
	out.uv_interp = uniforms.copy_section.xy + out.uv_interp * uniforms.copy_section.zw;
	out.position.xy = (uniforms.copy_section.xy + (out.position.xy * 0.5 + 0.5) * uniforms.copy_section.zw) * 2.0 - 1.0;
#elif defined(USE_DISPLAY_TRANSFORM)
	out.uv_interp = (uniforms.display_transform * float4(in.uv_in, 1.0, 1.0)).xy;
#endif

	return out;
}

// ============================================================================
// Helper Functions
// ============================================================================

#if defined(USE_PANORAMA) || defined(USE_ASYM_PANO)
float4 texturePanorama(float3 normal, texture2d<float> pano, sampler smp) {
	float2 st = float2(
		atan2(normal.x, normal.z),
		acos(normal.y));

	if (st.x < 0.0)
		st.x += M_PI * 2.0;

	st /= float2(M_PI * 2.0, M_PI);

	return pano.sample(smp, st, level(0.0));
}
#endif

float3 linear_to_srgb(float3 color) {
	const float3 a = float3(0.055);
	return select((float3(1.0) + a) * pow(color, float3(1.0 / 2.4)) - a, 12.92 * color, color < float3(0.0031308));
}

float3 srgb_to_linear(float3 color) {
	return select(pow((color + float3(0.055)) * (1.0 / (1.0 + 0.055)), float3(2.4)), color * (1.0 / 12.92), color < float3(0.04045));
}

// ============================================================================
// Fragment Shader
// ============================================================================

fragment float4 fragmentFunction(
	CopyVertexOut in [[stage_in]],
	constant CopyUniforms& uniforms [[buffer(1)]]
#ifdef USE_CUBEMAP
	, texturecube<float> source_cube [[texture(0)]]
	, sampler cubeSampler [[sampler(0)]]
#elif defined(USE_TEXTURE3D)
	, texture3d<float> source_3d [[texture(0)]]
	, sampler source3dSampler [[sampler(0)]]
#elif defined(USE_TEXTURE2DARRAY)
	, texture2d_array<float> source_2d_array [[texture(0)]]
	, sampler arrayTexSampler [[sampler(0)]]
#else
	, texture2d<float> source [[texture(0)]]
	, sampler sourceSampler [[sampler(0)]]
#endif
#ifdef SEP_CBCR_TEXTURE
	, texture2d<float> CbCr [[texture(1)]]
	, sampler cbcrSampler [[sampler(1)]]
#endif
#ifdef USE_COLOR_CORRECTION
	, texture2d<float> color_correction [[texture(1)]]
	, sampler colorCorrectionSampler [[sampler(1)]]
#endif
) {
	float4 color;

#ifdef USE_PANORAMA
	float3 cube_normal = normalize(in.cube_interp);
	cube_normal.z = -cube_normal.z;
	cube_normal = float3x3(uniforms.sky_transform[0].xyz, uniforms.sky_transform[1].xyz, uniforms.sky_transform[2].xyz) * cube_normal;
	cube_normal.z = -cube_normal.z;

	color = texturePanorama(cube_normal, source, sourceSampler);

#elif defined(USE_ASYM_PANO)
	// When an asymmetrical projection matrix is used (applicable for stereoscopic rendering i.e. VR)
	float3 cube_normal;
	cube_normal.z = -1.0;
	cube_normal.x = (cube_normal.z * (-in.uv_interp.x - uniforms.asym_proj.x)) / uniforms.asym_proj.y;
	cube_normal.y = (cube_normal.z * (-in.uv_interp.y - uniforms.asym_proj.z)) / uniforms.asym_proj.w;

	float3x3 sky_mat = float3x3(uniforms.sky_transform[0].xyz, uniforms.sky_transform[1].xyz, uniforms.sky_transform[2].xyz);
	float3x3 pano_mat = float3x3(uniforms.pano_transform[0].xyz, uniforms.pano_transform[1].xyz, uniforms.pano_transform[2].xyz);
	cube_normal = sky_mat * pano_mat * cube_normal;
	cube_normal.z = -cube_normal.z;

	color = texturePanorama(normalize(cube_normal), source, sourceSampler);

#elif defined(USE_CUBEMAP)
	color = source_cube.sample(cubeSampler, normalize(in.cube_interp));

#elif defined(USE_TEXTURE3D)
	color = source_3d.sample(source3dSampler, float3(in.uv_interp, uniforms.layer), level(0.0));

#elif defined(USE_TEXTURE2DARRAY)
	color = source_2d_array.sample(arrayTexSampler, in.uv_interp, uint(uniforms.layer), level(0.0));

#elif defined(SEP_CBCR_TEXTURE)
	color.r = source.sample(sourceSampler, in.uv_interp, level(0.0)).r;
	color.gb = CbCr.sample(cbcrSampler, in.uv_interp, level(0.0)).rg - float2(0.5, 0.5);
	color.a = 1.0;

#else
#ifdef USE_LOD
	color = source.sample(sourceSampler, in.uv_interp, level(uniforms.mip_level));
#else
	color = source.sample(sourceSampler, in.uv_interp, level(0.0));
#endif
#endif

#ifdef LINEAR_TO_SRGB
	// regular Linear -> SRGB conversion
	color.rgb = linear_to_srgb(color.rgb);

#elif defined(YCBCR_TO_SRGB)
	// YCbCr -> SRGB conversion using BT.709
	color.rgb = float3x3(
		float3(1.00000, 1.00000, 1.00000),
		float3(0.00000, -0.18732, 1.85560),
		float3(1.57481, -0.46813, 0.00000)) * color.rgb;
#endif

#ifdef SRGB_TO_LINEAR
	color.rgb = srgb_to_linear(color.rgb);
#endif

#ifdef DEBUG_GRADIENT
	color.rg = in.uv_interp;
	color.b = 0.0;
#endif

#ifdef DISABLE_ALPHA
	color.a = 1.0;
#endif

#ifdef GAUSSIAN_HORIZONTAL
	color *= 0.38774;
	color += source.sample(sourceSampler, in.uv_interp + float2(1.0, 0.0) * uniforms.pixel_size) * 0.24477;
	color += source.sample(sourceSampler, in.uv_interp + float2(2.0, 0.0) * uniforms.pixel_size) * 0.06136;
	color += source.sample(sourceSampler, in.uv_interp + float2(-1.0, 0.0) * uniforms.pixel_size) * 0.24477;
	color += source.sample(sourceSampler, in.uv_interp + float2(-2.0, 0.0) * uniforms.pixel_size) * 0.06136;
#endif

#ifdef GAUSSIAN_VERTICAL
	color *= 0.38774;
	color += source.sample(sourceSampler, in.uv_interp + float2(0.0, 1.0) * uniforms.pixel_size) * 0.24477;
	color += source.sample(sourceSampler, in.uv_interp + float2(0.0, 2.0) * uniforms.pixel_size) * 0.06136;
	color += source.sample(sourceSampler, in.uv_interp + float2(0.0, -1.0) * uniforms.pixel_size) * 0.24477;
	color += source.sample(sourceSampler, in.uv_interp + float2(0.0, -2.0) * uniforms.pixel_size) * 0.06136;
#endif

#ifdef USE_BCS
	color.rgb = mix(float3(0.0), color.rgb, uniforms.bcs.x);
	color.rgb = mix(float3(0.5), color.rgb, uniforms.bcs.y);
	color.rgb = mix(float3(dot(float3(1.0), color.rgb) * 0.33333), color.rgb, uniforms.bcs.z);
#endif

#ifdef USE_COLOR_CORRECTION
	color.r = color_correction.sample(colorCorrectionSampler, float2(color.r, 0.0)).r;
	color.g = color_correction.sample(colorCorrectionSampler, float2(color.g, 0.0)).g;
	color.b = color_correction.sample(colorCorrectionSampler, float2(color.b, 0.0)).b;
#endif

#ifdef USE_MULTIPLIER
	color.rgb *= uniforms.multiplier;
#endif

	return color;
}
