/**************************************************************************/
/*  subsurf_scattering.metal                                              */
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

// SSS kernel definitions based on quality level
#ifdef USE_25_SAMPLES
constant int kernel_size = 25;
constant float2 kernel[25] = {
	float2(0.530605, 0.0),
	float2(0.000973794, -3.0),
	float2(0.00333804, -2.52083),
	float2(0.00500364, -2.08333),
	float2(0.00700976, -1.6875),
	float2(0.0094389, -1.33333),
	float2(0.0128496, -1.02083),
	float2(0.017924, -0.75),
	float2(0.0263642, -0.520833),
	float2(0.0410172, -0.333333),
	float2(0.0493588, -0.1875),
	float2(0.0402784, -0.0833333),
	float2(0.0211412, -0.0208333),
	float2(0.0211412, 0.0208333),
	float2(0.0402784, 0.0833333),
	float2(0.0493588, 0.1875),
	float2(0.0410172, 0.333333),
	float2(0.0263642, 0.520833),
	float2(0.017924, 0.75),
	float2(0.0128496, 1.02083),
	float2(0.0094389, 1.33333),
	float2(0.00700976, 1.6875),
	float2(0.00500364, 2.08333),
	float2(0.00333804, 2.52083),
	float2(0.000973794, 3.0)
};
#endif

#ifdef USE_17_SAMPLES
constant int kernel_size = 17;
constant float2 kernel[17] = {
	float2(0.536343, 0.0),
	float2(0.00317394, -2.0),
	float2(0.0100386, -1.53125),
	float2(0.0144609, -1.125),
	float2(0.0216301, -0.78125),
	float2(0.0347317, -0.5),
	float2(0.0571056, -0.28125),
	float2(0.0582416, -0.125),
	float2(0.0324462, -0.03125),
	float2(0.0324462, 0.03125),
	float2(0.0582416, 0.125),
	float2(0.0571056, 0.28125),
	float2(0.0347317, 0.5),
	float2(0.0216301, 0.78125),
	float2(0.0144609, 1.125),
	float2(0.0100386, 1.53125),
	float2(0.00317394, 2.0)
};
#endif

#ifdef USE_11_SAMPLES
constant int kernel_size = 11;
constant float2 kernel[11] = {
	float2(0.560479, 0.0),
	float2(0.00471691, -2.0),
	float2(0.0192831, -1.28),
	float2(0.03639, -0.72),
	float2(0.0821904, -0.32),
	float2(0.0771802, -0.08),
	float2(0.0771802, 0.08),
	float2(0.0821904, 0.32),
	float2(0.03639, 0.72),
	float2(0.0192831, 1.28),
	float2(0.00471691, 2.0)
};
#endif

// Default to 11 samples if none specified
#if !defined(USE_25_SAMPLES) && !defined(USE_17_SAMPLES) && !defined(USE_11_SAMPLES)
constant int kernel_size = 11;
constant float2 kernel[11] = {
	float2(0.560479, 0.0),
	float2(0.00471691, -2.0),
	float2(0.0192831, -1.28),
	float2(0.03639, -0.72),
	float2(0.0821904, -0.32),
	float2(0.0771802, -0.08),
	float2(0.0771802, 0.08),
	float2(0.0821904, 0.32),
	float2(0.03639, 0.72),
	float2(0.0192831, 1.28),
	float2(0.00471691, 2.0)
};
#endif

// SSS uniforms
struct SSSUniforms {
	float max_radius;
	float camera_z_far;
	float camera_z_near;
	float unit_size;
	float2 dir;
};

// Vertex input structure
struct SSSVertexIn {
	float4 position [[attribute(0)]];
	float2 uv [[attribute(4)]];
};

// Interpolated vertex output / fragment input
struct SSSVertexOut {
	float4 position [[position]];
	float2 uv_interp;
};

// ============================================================================
// Vertex Shader
// ============================================================================

vertex SSSVertexOut vertexFunction(
	SSSVertexIn in [[stage_in]]
) {
	SSSVertexOut out;
	out.position = in.position;
	out.uv_interp = in.uv;
	return out;
}

// ============================================================================
// Fragment Shader
// ============================================================================

fragment float4 fragmentFunction(
	SSSVertexOut in [[stage_in]],
	constant SSSUniforms& uniforms [[buffer(1)]],
	texture2d<float> source_diffuse [[texture(0)]],
	sampler diffuseSampler [[sampler(0)]],
	texture2d<float> source_sss [[texture(1)]],
	sampler sssSampler [[sampler(1)]],
	texture2d<float> source_depth [[texture(2)]],
	sampler depthSampler [[sampler(2)]]
) {
	float strength = source_sss.sample(sssSampler, in.uv_interp).r;
	strength *= strength; // stored as sqrt

	// Fetch color of current pixel:
	float4 base_color = source_diffuse.sample(diffuseSampler, in.uv_interp);

	if (strength > 0.0) {
		// Fetch linear depth of current pixel:
		float depth = source_depth.sample(depthSampler, in.uv_interp).r * 2.0 - 1.0;
#ifdef USE_ORTHOGONAL_PROJECTION
		depth = ((depth + (uniforms.camera_z_far + uniforms.camera_z_near) / (uniforms.camera_z_far - uniforms.camera_z_near)) * (uniforms.camera_z_far - uniforms.camera_z_near)) / 2.0;
		float scale = uniforms.unit_size; // remember depth is negative by default in OpenGL
#else
		depth = 2.0 * uniforms.camera_z_near * uniforms.camera_z_far / (uniforms.camera_z_far + uniforms.camera_z_near - depth * (uniforms.camera_z_far - uniforms.camera_z_near));
		float scale = uniforms.unit_size / depth; // remember depth is negative by default in OpenGL
#endif

		// Calculate the final step to fetch the surrounding pixels:
		float2 step = uniforms.max_radius * scale * uniforms.dir;
		step *= strength; // Modulate it using the alpha channel.
		step *= 1.0 / 3.0; // Divide by 3 as the kernels range from -3 to 3.

		// Accumulate the center sample:
		float3 color_accum = base_color.rgb;
		color_accum *= kernel[0].x;
#ifdef ENABLE_STRENGTH_WEIGHTING
		float color_weight = kernel[0].x;
#endif

		// Accumulate the other samples:
		for (int i = 1; i < kernel_size; i++) {
			// Fetch color and depth for current sample:
			float2 offset = in.uv_interp + kernel[i].y * step;
			float3 color = source_diffuse.sample(diffuseSampler, offset).rgb;

#ifdef ENABLE_FOLLOW_SURFACE
			// If the difference in depth is huge, we lerp color back to "colorM":
			float depth_cmp = source_depth.sample(depthSampler, offset).r * 2.0 - 1.0;

#ifdef USE_ORTHOGONAL_PROJECTION
			depth_cmp = ((depth_cmp + (uniforms.camera_z_far + uniforms.camera_z_near) / (uniforms.camera_z_far - uniforms.camera_z_near)) * (uniforms.camera_z_far - uniforms.camera_z_near)) / 2.0;
#else
			depth_cmp = 2.0 * uniforms.camera_z_near * uniforms.camera_z_far / (uniforms.camera_z_far + uniforms.camera_z_near - depth_cmp * (uniforms.camera_z_far - uniforms.camera_z_near));
#endif

			float s = clamp(300.0f * scale * uniforms.max_radius * abs(depth - depth_cmp), 0.0, 1.0);
			color = mix(color, base_color.rgb, s);
#endif

			// Accumulate:
			color *= kernel[i].x;

#ifdef ENABLE_STRENGTH_WEIGHTING
			float color_s = source_sss.sample(sssSampler, offset).r;
			color_weight += color_s * kernel[i].x;
			color *= color_s;
#endif
			color_accum += color;
		}

#ifdef ENABLE_STRENGTH_WEIGHTING
		color_accum /= color_weight;
#endif
		return float4(color_accum, base_color.a); // keep alpha (used for SSAO)
	} else {
		return base_color;
	}
}
