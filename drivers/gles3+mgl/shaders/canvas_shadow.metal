/**************************************************************************/
/*  canvas_shadow.metal                                                   */
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

// Canvas shadow uniforms
struct CanvasShadowUniforms {
	float4x4 projection_matrix;
	float4x4 light_matrix;
	float4x4 world_matrix;
	float distance_norm;
};

// Vertex input structure
struct CanvasShadowVertexIn {
	float3 vertex [[attribute(0)]];
};

// Interpolated vertex output / fragment input
struct CanvasShadowVertexOut {
	float4 position [[position]];
	float4 position_interp;
};

// ============================================================================
// Vertex Shader
// ============================================================================

vertex CanvasShadowVertexOut vertexFunction(
	CanvasShadowVertexIn in [[stage_in]],
	constant CanvasShadowUniforms& uniforms [[buffer(1)]]
) {
	CanvasShadowVertexOut out;

	out.position = uniforms.projection_matrix * (uniforms.light_matrix * (uniforms.world_matrix * float4(in.vertex, 1.0)));
	out.position_interp = out.position;

	return out;
}

// ============================================================================
// Fragment Shader
// ============================================================================

#ifdef USE_RGBA_SHADOWS
// Output RGBA encoded depth for platforms without float depth buffers
struct CanvasShadowFragmentOut {
	float4 distance_buf [[color(0)]];
};

fragment CanvasShadowFragmentOut fragmentFunction(
	CanvasShadowVertexOut in [[stage_in]]
) {
	CanvasShadowFragmentOut out;

	float depth = ((in.position_interp.z / in.position_interp.w) + 1.0) * 0.5 + 0.0; // bias

	// Encode depth as RGBA
	float4 comp = fract(depth * float4(255.0 * 255.0 * 255.0, 255.0 * 255.0, 255.0, 1.0));
	comp -= comp.xxyz * float4(0.0, 1.0 / 255.0, 1.0 / 255.0, 1.0 / 255.0);
	out.distance_buf = comp;

	return out;
}
#else
// Output single float depth
struct CanvasShadowFragmentOut {
	float distance_buf [[color(0)]];
};

fragment CanvasShadowFragmentOut fragmentFunction(
	CanvasShadowVertexOut in [[stage_in]]
) {
	CanvasShadowFragmentOut out;

	float depth = ((in.position_interp.z / in.position_interp.w) + 1.0) * 0.5 + 0.0; // bias
	out.distance_buf = depth;

	return out;
}
#endif
