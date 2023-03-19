/**************************************************************************/
/*  _godot_common.h                                                       */
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

#ifndef METAL_GODOT_COMMON_H
#define METAL_GODOT_COMMON_H

typedef enum {
	VertexPosition = 0, // [[attribute(n)]]
	VertexNormal = 1,
	VertexUV = 2,

	UniformsBuffer = 0, // [[buffer(n)]]
	VertexBuffer = 1,

	TextureBaseColor = 0, // [[texture(n)]]
	TextureNormalMap = 1,
	TextureMask = 2,
	TextureSkeleton = 3,
	TextureScreen = 4,
	TextureLight = 5,
	TextureShadow = 6,
} IndexSematic;

#include <simd/simd.h>

#ifdef __METAL_VERSION__
#define __VERTEX_POSITION__ [[position]]
#define __ATTRIB_POSITION__ [[attribute(VertexPosition)]]
#define __ATTRIB_NORMAL__ [[attribute(VertexNormal)]]
#define __ATTRIB_UV__ [[attribute(VertexUV)]]
#else
#define __VERTEX_POSITION__
#define __ATTRIB_POSITION__
#define __ATTRIB_NORMAL__
#define __ATTRIB_UV__
#endif

// Reference:
// ----------
// 1. https://whackylabs.com/metal/cpp/2019/01/19/metal-types/
// 2. https://whackylabs.com/metal/2020/04/30/multiple-objects-single-frame-metal/
// 3. https://stackoverflow.com/questions/51790490/explaining-the-different-types-in-metal-and-simd

typedef struct __attribute__((__aligned__(256))) {
	simd::float4x4 light_matrix;
	simd::float4x4 local_matrix;
	simd::float4x4 shadow_matrix;
	simd::float4 color;
	simd::float4 shadow_color;
	simd::float2 light_pos;
	float shadowpixel_size;
	float shadow_gradient;
	float light_height;
	float light_outside_alpha;
	float shadow_distance_mult;
} uniforms_t;

typedef struct __attribute__((__aligned__(256))) {
	simd::float4x4 modelview_matrix;
	simd::float4x4 extra_matrix;
	simd::float4x4 world_matrix;
	simd::float4x4 inv_world_matrix;
	simd::float4x4 projection_matrix;
	float time;
} canvas_item_uniforms_t;

typedef struct __attribute__((__aligned__(256))) {
	simd::float3 position __ATTRIB_POSITION__;
	simd::float2 texture __ATTRIB_UV__;
	simd::float4 color;
	simd::float4 modulate;
} attributes_t;

#endif /* METAL_GODOT_COMMON_H */
