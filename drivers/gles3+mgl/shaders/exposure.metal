/**************************************************************************/
/*  exposure.metal                                                        */
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

// Exposure calculation uniforms
struct ExposureUniforms {
	int2 source_render_size;
	int2 target_size;
	float exposure_adjust;
	float min_luminance;
	float max_luminance;
};

// Vertex input structure
struct ExposureVertexIn {
	float4 position [[attribute(0)]];
};

// Vertex output / fragment input
struct ExposureVertexOut {
	float4 position [[position]];
};

// ============================================================================
// Vertex Shader
// ============================================================================

vertex ExposureVertexOut vertexFunction(
	ExposureVertexIn in [[stage_in]]
) {
	ExposureVertexOut out;
	out.position = in.position;
	return out;
}

// ============================================================================
// Fragment Shader
// ============================================================================

#ifdef EXPOSURE_BEGIN
// Initial pass - compute max luminance from source texture
fragment float fragmentFunction(
	ExposureVertexOut in [[stage_in]],
	constant ExposureUniforms& uniforms [[buffer(1)]],
	texture2d<float> source_exposure [[texture(0)]]
) {
	int2 src_pos = int2(in.position.xy) * uniforms.source_render_size / uniforms.target_size;

	// more precise and expensive, but less jittery
	int2 next_pos = (int2(in.position.xy) + int2(1)) * uniforms.source_render_size / uniforms.target_size;
	next_pos = max(next_pos, src_pos + int2(1)); // so it at least reads one pixel

	float3 source_color = float3(0.0);
	for (int i = src_pos.x; i < next_pos.x; i++) {
		for (int j = src_pos.y; j < next_pos.y; j++) {
			source_color += source_exposure.read(uint2(i, j), 0).rgb;
		}
	}

	source_color /= float((next_pos.x - src_pos.x) * (next_pos.y - src_pos.y));

	return max(source_color.r, max(source_color.g, source_color.b));
}
#else
// Subsequent passes - 3x3 average of exposure values
fragment float fragmentFunction(
	ExposureVertexOut in [[stage_in]],
	constant ExposureUniforms& uniforms [[buffer(1)]],
	texture2d<float> source_exposure [[texture(0)]]
#ifdef EXPOSURE_END
	, texture2d<float> prev_exposure [[texture(1)]]
#endif
) {
	int2 coord = int2(in.position.xy);

	float exposure = source_exposure.read(uint2(coord * 3 + int2(0, 0)), 0).r;
	exposure += source_exposure.read(uint2(coord * 3 + int2(1, 0)), 0).r;
	exposure += source_exposure.read(uint2(coord * 3 + int2(2, 0)), 0).r;
	exposure += source_exposure.read(uint2(coord * 3 + int2(0, 1)), 0).r;
	exposure += source_exposure.read(uint2(coord * 3 + int2(1, 1)), 0).r;
	exposure += source_exposure.read(uint2(coord * 3 + int2(2, 1)), 0).r;
	exposure += source_exposure.read(uint2(coord * 3 + int2(0, 2)), 0).r;
	exposure += source_exposure.read(uint2(coord * 3 + int2(1, 2)), 0).r;
	exposure += source_exposure.read(uint2(coord * 3 + int2(2, 2)), 0).r;
	exposure *= (1.0 / 9.0);

#ifdef EXPOSURE_END

#ifdef EXPOSURE_FORCE_SET
	// will stay as is
#else
	float prev_lum = prev_exposure.read(uint2(0, 0), 0).r; // 1 pixel previous exposure
	exposure = clamp(prev_lum + (exposure - prev_lum) * uniforms.exposure_adjust, uniforms.min_luminance, uniforms.max_luminance);
#endif // EXPOSURE_FORCE_SET

#endif // EXPOSURE_END

	return exposure;
}
#endif
