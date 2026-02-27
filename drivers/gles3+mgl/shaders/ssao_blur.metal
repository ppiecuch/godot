/**************************************************************************/
/*  ssao_blur.metal                                                       */
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

// Filter radius in pixels. This will be multiplied by SCALE.
#define R (4)

// SSAO blur uniforms
struct SSAOBlurUniforms {
	float edge_sharpness;
	int filter_scale;
	int2 axis;
	float camera_z_far;
	float camera_z_near;
	int2 screen_size;
};

// Vertex input structure
struct SSAOBlurVertexIn {
	float4 position [[attribute(0)]];
};

// Vertex output / fragment input
struct SSAOBlurVertexOut {
	float4 position [[position]];
};

// Gaussian coefficients (stddev = 2.0)
constant float gaussian[R + 1] = { 0.153170, 0.144893, 0.122649, 0.092902, 0.062970 };

// ============================================================================
// Vertex Shader
// ============================================================================

vertex SSAOBlurVertexOut vertexFunction(
	SSAOBlurVertexIn in [[stage_in]]
) {
	SSAOBlurVertexOut out;
	out.position = in.position;
	out.position.z = 1.0;
	return out;
}

// ============================================================================
// Fragment Shader
// ============================================================================

fragment float fragmentFunction(
	SSAOBlurVertexOut in [[stage_in]],
	constant SSAOBlurUniforms& uniforms [[buffer(1)]],
	texture2d<float> source_ssao [[texture(0)]],
	texture2d<float> source_depth [[texture(1)]],
	texture2d<float> source_normal [[texture(3)]]
) {
	int2 ssC = int2(in.position.xy);

	float depth = source_depth.read(uint2(ssC), 0).r;

	depth = depth * 2.0 - 1.0;
	depth = 2.0 * uniforms.camera_z_near * uniforms.camera_z_far / (uniforms.camera_z_far + uniforms.camera_z_near - depth * (uniforms.camera_z_far - uniforms.camera_z_near));

	float sum = source_ssao.read(uint2(ssC), 0).r;

	// Base weight for depth falloff. Increase this for more blurriness,
	// decrease it for better edge discrimination
	float BASE = gaussian[0];
	float totalWeight = BASE;
	sum *= totalWeight;

	int2 clamp_limit = uniforms.screen_size - int2(1);

	for (int r = -R; r <= R; ++r) {
		// We already handled the zero case above. This loop should be unrolled and the static branch optimized out,
		// so the IF statement has no runtime cost
		if (r != 0) {
			int2 ppos = ssC + uniforms.axis * (r * uniforms.filter_scale);
			float value = source_ssao.read(uint2(clamp(ppos, int2(0), clamp_limit)), 0).r;
			int2 rpos = clamp(ppos, int2(0), clamp_limit);
			float temp_depth = source_depth.read(uint2(rpos), 0).r;

			temp_depth = temp_depth * 2.0 - 1.0;
			temp_depth = 2.0 * uniforms.camera_z_near * uniforms.camera_z_far / (uniforms.camera_z_far + uniforms.camera_z_near - temp_depth * (uniforms.camera_z_far - uniforms.camera_z_near));

			// spatial domain: offset gaussian tap
			float weight = 0.3 + gaussian[abs(r)];

			// range domain (the "bilateral" weight). As depth difference increases, decrease weight.
			weight *= max(0.0, 1.0 - uniforms.edge_sharpness * abs(temp_depth - depth));

			sum += value * weight;
			totalWeight += weight;
		}
	}

	const float epsilon = 0.0001;
	return sum / (totalWeight + epsilon);
}
