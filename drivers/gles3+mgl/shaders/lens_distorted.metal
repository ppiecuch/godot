/**************************************************************************/
/*  lens_distorted.metal                                                  */
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

// Lens distortion uniforms
struct LensDistortedUniforms {
	float2 offset;
	float2 scale;
	float2 eye_center;
	float k1;
	float k2;
	float upscale;
	float aspect_ratio;
};

// Vertex input structure
struct LensDistortedVertexIn {
	float4 position [[attribute(0)]];
};

// Interpolated vertex output / fragment input
struct LensDistortedVertexOut {
	float4 position [[position]];
	float2 uv_interp;
};

// ============================================================================
// Vertex Shader
// ============================================================================

vertex LensDistortedVertexOut vertexFunction(
	LensDistortedVertexIn in [[stage_in]],
	constant LensDistortedUniforms& uniforms [[buffer(1)]]
) {
	LensDistortedVertexOut out;

	out.uv_interp = in.position.xy * 2.0 - 1.0;

	float2 v = in.position.xy * uniforms.scale + uniforms.offset;
	out.position = float4(v, 0.0, 1.0);

	return out;
}

// ============================================================================
// Fragment Shader
// ============================================================================

fragment float4 fragmentFunction(
	LensDistortedVertexOut in [[stage_in]],
	constant LensDistortedUniforms& uniforms [[buffer(1)]],
	texture2d<float> source [[texture(0)]],
	sampler sourceSampler [[sampler(0)]]
) {
	float2 coords = in.uv_interp;
	float2 offset = coords - uniforms.eye_center;

	// take aspect ratio into account
	offset.y /= uniforms.aspect_ratio;

	// distort
	float2 offset_sq = offset * offset;
	float radius_sq = offset_sq.x + offset_sq.y;
	float radius_s4 = radius_sq * radius_sq;
	float distortion_scale = 1.0 + (uniforms.k1 * radius_sq) + (uniforms.k2 * radius_s4);
	offset *= distortion_scale;

	// reapply aspect ratio
	offset.y *= uniforms.aspect_ratio;

	// add our eye center back in
	coords = offset + uniforms.eye_center;
	coords /= uniforms.upscale;

	// and check our color
	if (coords.x < -1.0 || coords.y < -1.0 || coords.x > 1.0 || coords.y > 1.0) {
		return float4(0.0, 0.0, 0.0, 1.0);
	} else {
		coords = (coords + float2(1.0)) / float2(2.0);
		return source.sample(sourceSampler, coords, level(0.0));
	}
}
