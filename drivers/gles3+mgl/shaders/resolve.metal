/**************************************************************************/
/*  resolve.metal                                                         */
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

// Resolve shader uniforms
struct ResolveUniforms {
	float2 pixel_size;
};

// Vertex input structure
struct ResolveVertexIn {
	float4 position [[attribute(0)]];
	float2 uv [[attribute(4)]];
};

// Interpolated vertex output / fragment input
struct ResolveVertexOut {
	float4 position [[position]];
	float2 uv_interp;
};

// ============================================================================
// Vertex Shader
// ============================================================================

vertex ResolveVertexOut vertexFunction(
	ResolveVertexIn in [[stage_in]]
) {
	ResolveVertexOut out;
	out.position = in.position;
	out.uv_interp = in.uv;
	return out;
}

// ============================================================================
// Fragment Shader
// ============================================================================

fragment float4 fragmentFunction(
	ResolveVertexOut in [[stage_in]],
	constant ResolveUniforms& uniforms [[buffer(1)]],
	texture2d<float> source_specular [[texture(0)]],
	sampler specularSampler [[sampler(0)]]
#ifdef USE_SSR
	, texture2d<float> source_ssr [[texture(1)]]
	, sampler ssrSampler [[sampler(1)]]
#endif
) {
	float4 specular = source_specular.sample(specularSampler, in.uv_interp);

#ifdef USE_SSR
	float4 ssr = source_ssr.sample(ssrSampler, in.uv_interp, level(0.0));
	specular.rgb = mix(specular.rgb, ssr.rgb * specular.a, ssr.a);
#endif

	return float4(specular.rgb, 1.0);
}
