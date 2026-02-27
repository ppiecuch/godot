/**************************************************************************/
/*  blend_shape.metal                                                     */
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

// Blend shape compute shader - converts Transform Feedback to compute
// This shader blends between base mesh and blend shape mesh attributes

// Uniform data
struct BlendShapeUniforms {
	float blend_amount;
	uint vertex_count;
};

// Vertex data layout matching VisualServer:
// ARRAY_VERTEX=0, ARRAY_NORMAL=1, ARRAY_TANGENT=2, ARRAY_COLOR=3,
// ARRAY_TEX_UV=4, ARRAY_TEX_UV2=5, ARRAY_BONES=6, ARRAY_WEIGHTS=7

#ifdef USE_2D_VERTEX
typedef float2 VertexType;
#else
typedef float3 VertexType;
#endif

// ============================================================================
// Helper Functions
// ============================================================================

#ifdef ENABLE_OCTAHEDRAL_COMPRESSION
float3 oct_to_vec3(float2 e) {
	float3 v = float3(e.xy, 1.0 - abs(e.x) - abs(e.y));
	float t = max(-v.z, 0.0);
	v.xy += t * -sign(v.xy);
	return normalize(v);
}
#endif

// ============================================================================
// Compute Kernel
// ============================================================================

// The blend shape system uses separate buffer bindings for each attribute
// to match the flexible vertex format of Godot's mesh system.

kernel void blendShapeKernel(
	constant BlendShapeUniforms& uniforms [[buffer(0)]],
	// Base mesh attributes
	const device VertexType* vertex_in [[buffer(1)]],
#ifdef ENABLE_OCTAHEDRAL_COMPRESSION
	const device float4* normal_tangent_in [[buffer(2)]],
#else
	const device float3* normal_in [[buffer(2)]],
#ifdef ENABLE_TANGENT
	const device float4* tangent_in [[buffer(3)]],
#endif
#endif
#ifdef ENABLE_COLOR
	const device float4* color_in [[buffer(4)]],
#endif
#ifdef ENABLE_UV
	const device float2* uv_in [[buffer(5)]],
#endif
#ifdef ENABLE_UV2
	const device float2* uv2_in [[buffer(6)]],
#endif
#ifdef ENABLE_SKELETON
	const device int4* bone_in [[buffer(7)]],
	const device float4* weight_in [[buffer(8)]],
#endif
#ifdef ENABLE_BLEND
	// Blend shape attributes (accumulated results so far)
	const device VertexType* vertex_blend [[buffer(9)]],
	const device float3* normal_blend [[buffer(10)]],
#ifdef ENABLE_TANGENT
	const device float4* tangent_blend [[buffer(11)]],
#endif
#ifdef ENABLE_COLOR
	const device float4* color_blend [[buffer(12)]],
#endif
#ifdef ENABLE_UV
	const device float2* uv_blend [[buffer(13)]],
#endif
#ifdef ENABLE_UV2
	const device float2* uv2_blend [[buffer(14)]],
#endif
#ifdef ENABLE_SKELETON
	const device int4* bone_blend [[buffer(15)]],
	const device float4* weight_blend [[buffer(16)]],
#endif
#endif
	// Output attributes
	device VertexType* vertex_out [[buffer(17)]],
#ifdef ENABLE_NORMAL
	device float3* normal_out [[buffer(18)]],
#endif
#ifdef ENABLE_TANGENT
	device float4* tangent_out [[buffer(19)]],
#endif
#ifdef ENABLE_COLOR
	device float4* color_out [[buffer(20)]],
#endif
#ifdef ENABLE_UV
	device float2* uv_out [[buffer(21)]],
#endif
#ifdef ENABLE_UV2
	device float2* uv2_out [[buffer(22)]],
#endif
#ifdef ENABLE_SKELETON
	device int4* bone_out [[buffer(23)]],
	device float4* weight_out [[buffer(24)]],
#endif
	uint gid [[thread_position_in_grid]]
) {
	if (gid >= uniforms.vertex_count) {
		return;
	}

#ifdef ENABLE_BLEND
	// Accumulating blend shapes - add scaled base to existing blend result

	vertex_out[gid] = vertex_blend[gid] + vertex_in[gid] * uniforms.blend_amount;

#ifdef ENABLE_NORMAL
#ifdef ENABLE_OCTAHEDRAL_COMPRESSION
	normal_out[gid] = normal_blend[gid] + oct_to_vec3(normal_tangent_in[gid].xy) * uniforms.blend_amount;
#else
	normal_out[gid] = normal_blend[gid] + normal_in[gid] * uniforms.blend_amount;
#endif
#endif

#ifdef ENABLE_TANGENT
#ifdef ENABLE_OCTAHEDRAL_COMPRESSION
	float4 nt = normal_tangent_in[gid];
	float3 tan_vec = oct_to_vec3(float2(nt.z, abs(nt.w) * 2.0 - 1.0));
	tangent_out[gid].xyz = tangent_blend[gid].xyz + tan_vec * uniforms.blend_amount;
	tangent_out[gid].w = sign(tangent_blend[gid].w);
#else
	tangent_out[gid].xyz = tangent_blend[gid].xyz + tangent_in[gid].xyz * uniforms.blend_amount;
	tangent_out[gid].w = tangent_blend[gid].w;  // Just copy, no point in blending
#endif
#endif

#ifdef ENABLE_COLOR
	color_out[gid] = color_blend[gid] + color_in[gid] * uniforms.blend_amount;
#endif

#ifdef ENABLE_UV
	uv_out[gid] = uv_blend[gid] + uv_in[gid] * uniforms.blend_amount;
#endif

#ifdef ENABLE_UV2
	uv2_out[gid] = uv2_blend[gid] + uv2_in[gid] * uniforms.blend_amount;
#endif

#ifdef ENABLE_SKELETON
	bone_out[gid] = bone_blend[gid];
	weight_out[gid] = weight_blend[gid] + weight_in[gid] * uniforms.blend_amount;
#endif

#else  // !ENABLE_BLEND - First blend shape, scale only

	vertex_out[gid] = vertex_in[gid] * uniforms.blend_amount;

#ifdef ENABLE_NORMAL
#ifdef ENABLE_OCTAHEDRAL_COMPRESSION
	normal_out[gid] = oct_to_vec3(normal_tangent_in[gid].xy) * uniforms.blend_amount;
#else
	normal_out[gid] = normal_in[gid] * uniforms.blend_amount;
#endif
#endif

#ifdef ENABLE_TANGENT
#ifdef ENABLE_OCTAHEDRAL_COMPRESSION
	float4 nt = normal_tangent_in[gid];
	float3 tan_vec = oct_to_vec3(float2(nt.z, abs(nt.w) * 2.0 - 1.0));
	tangent_out[gid].xyz = tan_vec * uniforms.blend_amount;
	tangent_out[gid].w = sign(nt.w);
#else
	tangent_out[gid].xyz = tangent_in[gid].xyz * uniforms.blend_amount;
	tangent_out[gid].w = tangent_in[gid].w;  // Just copy
#endif
#endif

#ifdef ENABLE_COLOR
	color_out[gid] = color_in[gid] * uniforms.blend_amount;
#endif

#ifdef ENABLE_UV
	uv_out[gid] = uv_in[gid] * uniforms.blend_amount;
#endif

#ifdef ENABLE_UV2
	uv2_out[gid] = uv2_in[gid] * uniforms.blend_amount;
#endif

#ifdef ENABLE_SKELETON
	bone_out[gid] = bone_in[gid];
	weight_out[gid] = weight_in[gid] * uniforms.blend_amount;
#endif

#endif  // ENABLE_BLEND
}
