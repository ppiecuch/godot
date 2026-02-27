/**************************************************************************/
/*  cube_to_dp.metal                                                      */
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

// Cube to dual paraboloid uniforms
struct CubeToDPUniforms {
	bool z_flip;
	float z_far;
	float z_near;
	float bias;
};

// Vertex input structure
struct CubeToDPVertexIn {
	float4 position [[attribute(0)]];
	float2 uv [[attribute(4)]];
};

// Interpolated vertex output / fragment input
struct CubeToDPVertexOut {
	float4 position [[position]];
	float2 uv_interp;
};

// ============================================================================
// Vertex Shader
// ============================================================================

vertex CubeToDPVertexOut vertexFunction(
	CubeToDPVertexIn in [[stage_in]]
) {
	CubeToDPVertexOut out;
	out.position = in.position;
	out.uv_interp = in.uv;
	return out;
}

// ============================================================================
// Fragment Shader
// ============================================================================

struct CubeToDPFragmentOut {
	float depth [[depth(any)]];
};

fragment CubeToDPFragmentOut fragmentFunction(
	CubeToDPVertexOut in [[stage_in]],
	constant CubeToDPUniforms& uniforms [[buffer(1)]],
	texturecube<float> source_cube [[texture(0)]],
	sampler cubeSampler [[sampler(0)]]
) {
	CubeToDPFragmentOut out;

	float3 normal = float3(in.uv_interp * 2.0 - 1.0, 0.0);

	normal.z = 0.5 - 0.5 * ((normal.x * normal.x) + (normal.y * normal.y));
	normal = normalize(normal);

	if (!uniforms.z_flip) {
		normal.z = -normal.z;
	}

	float depth = source_cube.sample(cubeSampler, normal).r;

	// absolute values for direction cosines, bigger value equals closer to basis axis
	float3 unorm = abs(normal);

	if ((unorm.x >= unorm.y) && (unorm.x >= unorm.z)) {
		// x code
		unorm = normal.x > 0.0 ? float3(1.0, 0.0, 0.0) : float3(-1.0, 0.0, 0.0);
	} else if ((unorm.y > unorm.x) && (unorm.y >= unorm.z)) {
		// y code
		unorm = normal.y > 0.0 ? float3(0.0, 1.0, 0.0) : float3(0.0, -1.0, 0.0);
	} else if ((unorm.z > unorm.x) && (unorm.z > unorm.y)) {
		// z code
		unorm = normal.z > 0.0 ? float3(0.0, 0.0, 1.0) : float3(0.0, 0.0, -1.0);
	} else {
		// oh-no we messed up code
		// has to be
		unorm = float3(1.0, 0.0, 0.0);
	}

	float depth_fix = 1.0 / dot(normal, unorm);

	depth = 2.0 * depth - 1.0;
	float linear_depth = 2.0 * uniforms.z_near * uniforms.z_far / (uniforms.z_far + uniforms.z_near - depth * (uniforms.z_far - uniforms.z_near));
	out.depth = (linear_depth * depth_fix + uniforms.bias) / uniforms.z_far;

	return out;
}
