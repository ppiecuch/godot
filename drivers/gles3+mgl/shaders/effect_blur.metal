/**************************************************************************/
/*  effect_blur.metal                                                     */
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

// DOF kernel definitions
#ifdef DOF_QUALITY_LOW
constant int dof_kernel_size = 5;
constant int dof_kernel_from = 2;
constant float dof_kernel[5] = { 0.153388, 0.221461, 0.250301, 0.221461, 0.153388 };
#endif

#ifdef DOF_QUALITY_MEDIUM
constant int dof_kernel_size = 11;
constant int dof_kernel_from = 5;
constant float dof_kernel[11] = { 0.055037, 0.072806, 0.090506, 0.105726, 0.116061, 0.119726, 0.116061, 0.105726, 0.090506, 0.072806, 0.055037 };
#endif

#ifdef DOF_QUALITY_HIGH
constant int dof_kernel_size = 21;
constant int dof_kernel_from = 10;
constant float dof_kernel[21] = { 0.028174, 0.032676, 0.037311, 0.041944, 0.046421, 0.050582, 0.054261, 0.057307, 0.059587, 0.060998, 0.061476, 0.060998, 0.059587, 0.057307, 0.054261, 0.050582, 0.046421, 0.041944, 0.037311, 0.032676, 0.028174 };
#endif

// Default DOF kernel if none specified
#if !defined(DOF_QUALITY_LOW) && !defined(DOF_QUALITY_MEDIUM) && !defined(DOF_QUALITY_HIGH)
constant int dof_kernel_size = 5;
constant int dof_kernel_from = 2;
constant float dof_kernel[5] = { 0.153388, 0.221461, 0.250301, 0.221461, 0.153388 };
#endif

// Effect blur uniforms
struct EffectBlurUniforms {
	float4 blur_section;
	float lod;
	float2 pixel_size;
	float4 ssao_color;
	float glow_strength;
	float dof_begin;
	float dof_end;
	float2 dof_dir;
	float dof_radius;
	float camera_z_far;
	float camera_z_near;
	float exposure;
	float white;
	float luminance_cap;
	float auto_exposure_grey;
	float glow_bloom;
	float glow_hdr_threshold;
	float glow_hdr_scale;
};

// Vertex input structure
struct EffectBlurVertexIn {
	float4 position [[attribute(0)]];
	float2 uv [[attribute(4)]];
};

// Interpolated vertex output / fragment input
struct EffectBlurVertexOut {
	float4 position [[position]];
	float2 uv_interp;
};

// ============================================================================
// Vertex Shader
// ============================================================================

vertex EffectBlurVertexOut vertexFunction(
	EffectBlurVertexIn in [[stage_in]],
	constant EffectBlurUniforms& uniforms [[buffer(1)]]
) {
	EffectBlurVertexOut out;
	out.uv_interp = in.uv;
	out.position = in.position;

#ifdef USE_BLUR_SECTION
	out.uv_interp = uniforms.blur_section.xy + out.uv_interp * uniforms.blur_section.zw;
	out.position.xy = (uniforms.blur_section.xy + (out.position.xy * 0.5 + 0.5) * uniforms.blur_section.zw) * 2.0 - 1.0;
#endif

	return out;
}

// ============================================================================
// Fragment Shader
// ============================================================================

fragment float4 fragmentFunction(
	EffectBlurVertexOut in [[stage_in]],
	constant EffectBlurUniforms& uniforms [[buffer(1)]],
	texture2d<float> source_color [[texture(0)]],
	sampler colorSampler [[sampler(0)]]
#ifdef SSAO_MERGE
	, texture2d<float> source_ssao [[texture(1)]]
	, sampler ssaoSampler [[sampler(1)]]
#endif
#if defined(DOF_FAR_BLUR) || defined(DOF_NEAR_BLUR)
	, texture2d<float> dof_source_depth [[texture(1)]]
	, sampler depthSampler [[sampler(1)]]
#endif
#ifdef DOF_NEAR_BLUR_MERGE
	, texture2d<float> source_dof_original [[texture(2)]]
	, sampler dofOriginalSampler [[sampler(2)]]
#endif
#ifdef GLOW_USE_AUTO_EXPOSURE
	, texture2d<float> source_auto_exposure [[texture(1)]]
#endif
) {
	float4 frag_color;

#ifdef GAUSSIAN_HORIZONTAL
	float2 pix_size = uniforms.pixel_size;
	pix_size *= 0.5; // reading from larger buffer, so use more samples
	// sigma 2
	float4 color = source_color.sample(colorSampler, in.uv_interp + float2(0.0, 0.0) * pix_size, level(uniforms.lod)) * 0.214607;
	color += source_color.sample(colorSampler, in.uv_interp + float2(1.0, 0.0) * pix_size, level(uniforms.lod)) * 0.189879;
	color += source_color.sample(colorSampler, in.uv_interp + float2(2.0, 0.0) * pix_size, level(uniforms.lod)) * 0.131514;
	color += source_color.sample(colorSampler, in.uv_interp + float2(3.0, 0.0) * pix_size, level(uniforms.lod)) * 0.071303;
	color += source_color.sample(colorSampler, in.uv_interp + float2(-1.0, 0.0) * pix_size, level(uniforms.lod)) * 0.189879;
	color += source_color.sample(colorSampler, in.uv_interp + float2(-2.0, 0.0) * pix_size, level(uniforms.lod)) * 0.131514;
	color += source_color.sample(colorSampler, in.uv_interp + float2(-3.0, 0.0) * pix_size, level(uniforms.lod)) * 0.071303;
	frag_color = color;
#endif

#ifdef GAUSSIAN_VERTICAL
	float4 color = source_color.sample(colorSampler, in.uv_interp + float2(0.0, 0.0) * uniforms.pixel_size, level(uniforms.lod)) * 0.38774;
	color += source_color.sample(colorSampler, in.uv_interp + float2(0.0, 1.0) * uniforms.pixel_size, level(uniforms.lod)) * 0.24477;
	color += source_color.sample(colorSampler, in.uv_interp + float2(0.0, 2.0) * uniforms.pixel_size, level(uniforms.lod)) * 0.06136;
	color += source_color.sample(colorSampler, in.uv_interp + float2(0.0, -1.0) * uniforms.pixel_size, level(uniforms.lod)) * 0.24477;
	color += source_color.sample(colorSampler, in.uv_interp + float2(0.0, -2.0) * uniforms.pixel_size, level(uniforms.lod)) * 0.06136;
	frag_color = color;
#endif

#ifdef GLOW_GAUSSIAN_HORIZONTAL
	float2 pix_size = uniforms.pixel_size;
	pix_size *= 0.5; // reading from larger buffer, so use more samples

#ifdef USE_GLOW_HIGH_QUALITY
	// Sample from two lines to capture single-pixel features.
	float4 color = source_color.sample(colorSampler, in.uv_interp + float2(0.0, 0.0) * pix_size, level(uniforms.lod)) * 0.152781;
	color += source_color.sample(colorSampler, in.uv_interp + float2(1.0, 0.0) * pix_size, level(uniforms.lod)) * 0.144599;
	color += source_color.sample(colorSampler, in.uv_interp + float2(2.0, 0.0) * pix_size, level(uniforms.lod)) * 0.122589;
	color += source_color.sample(colorSampler, in.uv_interp + float2(3.0, 0.0) * pix_size, level(uniforms.lod)) * 0.093095;
	color += source_color.sample(colorSampler, in.uv_interp + float2(4.0, 0.0) * pix_size, level(uniforms.lod)) * 0.063327;
	color += source_color.sample(colorSampler, in.uv_interp + float2(-1.0, 0.0) * pix_size, level(uniforms.lod)) * 0.144599;
	color += source_color.sample(colorSampler, in.uv_interp + float2(-2.0, 0.0) * pix_size, level(uniforms.lod)) * 0.122589;
	color += source_color.sample(colorSampler, in.uv_interp + float2(-3.0, 0.0) * pix_size, level(uniforms.lod)) * 0.093095;
	color += source_color.sample(colorSampler, in.uv_interp + float2(-4.0, 0.0) * pix_size, level(uniforms.lod)) * 0.063327;

	color += source_color.sample(colorSampler, in.uv_interp + float2(0.0, 1.0) * pix_size, level(uniforms.lod)) * 0.152781;
	color += source_color.sample(colorSampler, in.uv_interp + float2(1.0, 1.0) * pix_size, level(uniforms.lod)) * 0.144599;
	color += source_color.sample(colorSampler, in.uv_interp + float2(2.0, 1.0) * pix_size, level(uniforms.lod)) * 0.122589;
	color += source_color.sample(colorSampler, in.uv_interp + float2(3.0, 1.0) * pix_size, level(uniforms.lod)) * 0.093095;
	color += source_color.sample(colorSampler, in.uv_interp + float2(4.0, 1.0) * pix_size, level(uniforms.lod)) * 0.063327;
	color += source_color.sample(colorSampler, in.uv_interp + float2(-1.0, 1.0) * pix_size, level(uniforms.lod)) * 0.144599;
	color += source_color.sample(colorSampler, in.uv_interp + float2(-2.0, 1.0) * pix_size, level(uniforms.lod)) * 0.122589;
	color += source_color.sample(colorSampler, in.uv_interp + float2(-3.0, 1.0) * pix_size, level(uniforms.lod)) * 0.093095;
	color += source_color.sample(colorSampler, in.uv_interp + float2(-4.0, 1.0) * pix_size, level(uniforms.lod)) * 0.063327;
	color *= 0.5;
#else
	float4 color = source_color.sample(colorSampler, in.uv_interp + float2(0.0, 0.0) * pix_size, level(uniforms.lod)) * 0.174938;
	color += source_color.sample(colorSampler, in.uv_interp + float2(1.0, 0.0) * pix_size, level(uniforms.lod)) * 0.165569;
	color += source_color.sample(colorSampler, in.uv_interp + float2(2.0, 0.0) * pix_size, level(uniforms.lod)) * 0.140367;
	color += source_color.sample(colorSampler, in.uv_interp + float2(3.0, 0.0) * pix_size, level(uniforms.lod)) * 0.106595;
	color += source_color.sample(colorSampler, in.uv_interp + float2(-1.0, 0.0) * pix_size, level(uniforms.lod)) * 0.165569;
	color += source_color.sample(colorSampler, in.uv_interp + float2(-2.0, 0.0) * pix_size, level(uniforms.lod)) * 0.140367;
	color += source_color.sample(colorSampler, in.uv_interp + float2(-3.0, 0.0) * pix_size, level(uniforms.lod)) * 0.106595;
#endif // USE_GLOW_HIGH_QUALITY

	color *= uniforms.glow_strength;
	frag_color = color;
#endif // GLOW_GAUSSIAN_HORIZONTAL

#ifdef GLOW_GAUSSIAN_VERTICAL
	float4 color = source_color.sample(colorSampler, in.uv_interp + float2(0.0, 0.0) * uniforms.pixel_size, level(uniforms.lod)) * 0.288713;
	color += source_color.sample(colorSampler, in.uv_interp + float2(0.0, 1.0) * uniforms.pixel_size, level(uniforms.lod)) * 0.233062;
	color += source_color.sample(colorSampler, in.uv_interp + float2(0.0, 2.0) * uniforms.pixel_size, level(uniforms.lod)) * 0.122581;
	color += source_color.sample(colorSampler, in.uv_interp + float2(0.0, -1.0) * uniforms.pixel_size, level(uniforms.lod)) * 0.233062;
	color += source_color.sample(colorSampler, in.uv_interp + float2(0.0, -2.0) * uniforms.pixel_size, level(uniforms.lod)) * 0.122581;
	color *= uniforms.glow_strength;
	frag_color = color;
#endif

#ifdef DOF_FAR_BLUR
	float4 color_accum = float4(0.0);

	float depth = dof_source_depth.sample(depthSampler, in.uv_interp, level(0.0)).r;
	depth = depth * 2.0 - 1.0;
#ifdef USE_ORTHOGONAL_PROJECTION
	depth = ((depth + (uniforms.camera_z_far + uniforms.camera_z_near) / (uniforms.camera_z_far - uniforms.camera_z_near)) * (uniforms.camera_z_far - uniforms.camera_z_near)) / 2.0;
#else
	depth = 2.0 * uniforms.camera_z_near * uniforms.camera_z_far / (uniforms.camera_z_far + uniforms.camera_z_near - depth * (uniforms.camera_z_far - uniforms.camera_z_near));
#endif

	float amount = smoothstep(uniforms.dof_begin, uniforms.dof_end, depth);
	float4 k_accum = float4(0.0);

	for (int i = 0; i < dof_kernel_size; i++) {
		int int_ofs = i - dof_kernel_from;
		float2 tap_uv = in.uv_interp + uniforms.dof_dir * float(int_ofs) * amount * uniforms.dof_radius;

		float tap_k = dof_kernel[i];

		float tap_depth = dof_source_depth.sample(depthSampler, tap_uv, level(0.0)).r;
		tap_depth = tap_depth * 2.0 - 1.0;
#ifdef USE_ORTHOGONAL_PROJECTION
		tap_depth = ((tap_depth + (uniforms.camera_z_far + uniforms.camera_z_near) / (uniforms.camera_z_far - uniforms.camera_z_near)) * (uniforms.camera_z_far - uniforms.camera_z_near)) / 2.0;
#else
		tap_depth = 2.0 * uniforms.camera_z_near * uniforms.camera_z_far / (uniforms.camera_z_far + uniforms.camera_z_near - tap_depth * (uniforms.camera_z_far - uniforms.camera_z_near));
#endif
		float tap_amount = mix(smoothstep(uniforms.dof_begin, uniforms.dof_end, tap_depth), 1.0, float(int_ofs == 0));
		tap_amount *= tap_amount * tap_amount; // prevent undesired glow effect
		tap_amount *= tap_k;

		float4 tap_color = source_color.sample(colorSampler, tap_uv, level(0.0));

		float4 w = float4(tap_amount) * float4(float3(tap_color.a), 1.0);
		k_accum += w;
		color_accum += tap_color * w;
	}

	if (k_accum.r > 0.0) {
		color_accum /= k_accum;
	}

	frag_color = color_accum;
#endif

#ifdef DOF_NEAR_BLUR
	float4 color_accum = float4(0.0);
	float max_accum = 0.0;

	for (int i = 0; i < dof_kernel_size; i++) {
		int int_ofs = i - dof_kernel_from;
		float2 tap_uv = in.uv_interp + uniforms.dof_dir * float(int_ofs) * uniforms.dof_radius;
		float ofs_influence = max(0.0, 1.0 - float(abs(int_ofs)) / float(dof_kernel_from));

		float tap_k = dof_kernel[i];

		float4 tap_color = source_color.sample(colorSampler, tap_uv, level(0.0));

		float tap_depth = dof_source_depth.sample(depthSampler, tap_uv, level(0.0)).r;
		tap_depth = tap_depth * 2.0 - 1.0;
#ifdef USE_ORTHOGONAL_PROJECTION
		tap_depth = ((tap_depth + (uniforms.camera_z_far + uniforms.camera_z_near) / (uniforms.camera_z_far - uniforms.camera_z_near)) * (uniforms.camera_z_far - uniforms.camera_z_near)) / 2.0;
#else
		tap_depth = 2.0 * uniforms.camera_z_near * uniforms.camera_z_far / (uniforms.camera_z_far + uniforms.camera_z_near - tap_depth * (uniforms.camera_z_far - uniforms.camera_z_near));
#endif
		float tap_amount = 1.0 - smoothstep(uniforms.dof_end, uniforms.dof_begin, tap_depth);
		tap_amount *= tap_amount * tap_amount; // prevent undesired glow effect

#ifdef DOF_NEAR_FIRST_TAP
		tap_color.a = 1.0 - smoothstep(uniforms.dof_end, uniforms.dof_begin, tap_depth);
#endif

		max_accum = max(max_accum, tap_amount * ofs_influence);

		color_accum += tap_color * tap_k;
	}

	color_accum.a = max(color_accum.a, sqrt(max_accum));

#ifdef DOF_NEAR_BLUR_MERGE
	float4 original = source_dof_original.sample(dofOriginalSampler, in.uv_interp, level(0.0));
	color_accum = mix(original, color_accum, color_accum.a);
#endif

	frag_color = color_accum;
#endif

#ifdef GLOW_FIRST_PASS
	frag_color = source_color.sample(colorSampler, in.uv_interp, level(0.0));

#ifdef GLOW_USE_AUTO_EXPOSURE
	frag_color /= source_auto_exposure.read(uint2(0, 0), 0).r / uniforms.auto_exposure_grey;
#endif
	frag_color *= uniforms.exposure;

	float luminance = max(frag_color.r, max(frag_color.g, frag_color.b));
	float feedback = max(smoothstep(uniforms.glow_hdr_threshold, uniforms.glow_hdr_threshold + uniforms.glow_hdr_scale, luminance), uniforms.glow_bloom);

	frag_color = min(frag_color * feedback, float4(uniforms.luminance_cap));
#endif

#ifdef SIMPLE_COPY
	float4 color = source_color.sample(colorSampler, in.uv_interp, level(0.0));
	frag_color = color;
#endif

#ifdef SSAO_MERGE
	float4 color = source_color.sample(colorSampler, in.uv_interp, level(0.0));
	float ssao = source_ssao.sample(ssaoSampler, in.uv_interp, level(0.0)).r;

	frag_color = float4(mix(color.rgb, color.rgb * mix(uniforms.ssao_color.rgb, float3(1.0), ssao), color.a), 1.0);
#endif

	return frag_color;
}
