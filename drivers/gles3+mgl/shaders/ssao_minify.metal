/**************************************************************************/
/*  ssao_minify.metal                                                     */
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

// SSAO minify uniforms
struct SSAOMinifyUniforms {
	float camera_z_far;
	float camera_z_near;
	int2 from_size;
	int source_mipmap;
};

// Vertex input structure
struct SSAOMinifyVertexIn {
	float4 position [[attribute(0)]];
};

// Vertex output / fragment input
struct SSAOMinifyVertexOut {
	float4 position [[position]];
};

// ============================================================================
// Vertex Shader
// ============================================================================

vertex SSAOMinifyVertexOut vertexFunction(
	SSAOMinifyVertexIn in [[stage_in]]
) {
	SSAOMinifyVertexOut out;
	out.position = in.position;
	return out;
}

// ============================================================================
// Fragment Shader
// ============================================================================

#ifdef MINIFY_START
// Initial pass - read from float depth buffer
fragment uint fragmentFunction(
	SSAOMinifyVertexOut in [[stage_in]],
	constant SSAOMinifyUniforms& uniforms [[buffer(1)]],
	texture2d<float> source_depth [[texture(0)]]
) {
	int2 ssP = int2(in.position.xy);

	// Rotated grid subsampling to avoid XY directional bias or Z precision bias while downsampling.
	int2 coord = clamp(ssP * 2 + int2(ssP.y & 1, ssP.x & 1), int2(0), uniforms.from_size - int2(1));
	float fdepth = source_depth.read(uint2(coord), uniforms.source_mipmap).r;

	fdepth = fdepth * 2.0 - 1.0;
#ifdef USE_ORTHOGONAL_PROJECTION
	fdepth = ((fdepth + (uniforms.camera_z_far + uniforms.camera_z_near) / (uniforms.camera_z_far - uniforms.camera_z_near)) * (uniforms.camera_z_far - uniforms.camera_z_near)) / 2.0;
#else
	fdepth = 2.0 * uniforms.camera_z_near * uniforms.camera_z_far / (uniforms.camera_z_far + uniforms.camera_z_near - fdepth * (uniforms.camera_z_far - uniforms.camera_z_near));
#endif
	fdepth /= uniforms.camera_z_far;

	return uint(clamp(fdepth * 65535.0, 0.0, 65535.0));
}
#else
// Subsequent passes - read from uint mipmap chain
fragment uint fragmentFunction(
	SSAOMinifyVertexOut in [[stage_in]],
	constant SSAOMinifyUniforms& uniforms [[buffer(1)]],
	texture2d<uint> source_depth_mipmaps [[texture(0)]]
) {
	int2 ssP = int2(in.position.xy);

	// Rotated grid subsampling to avoid XY directional bias or Z precision bias while downsampling.
	int2 coord = clamp(ssP * 2 + int2(ssP.y & 1, ssP.x & 1), int2(0), uniforms.from_size - int2(1));

	return source_depth_mipmaps.read(uint2(coord), uniforms.source_mipmap).r;
}
#endif
