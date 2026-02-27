/**************************************************************************/
/*  screen_space_reflection.metal                                         */
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

#define M_PI 3.14159265359

// SSR uniforms
struct SSRUniforms {
	float camera_z_near;
	float camera_z_far;
	float2 viewport_size;
	float2 pixel_size;
	float filter_mipmap_levels;
	float4x4 inverse_projection;
	float4x4 projection;
	int num_steps;
	float depth_tolerance;
	float distance_fade;
	float curve_fade_in;
};

// Vertex input structure
struct SSRVertexIn {
	float4 position [[attribute(0)]];
	float2 uv [[attribute(4)]];
};

// Interpolated vertex output / fragment input
struct SSRVertexOut {
	float4 position [[position]];
	float2 uv_interp;
	float2 pos_interp;
};

// ============================================================================
// Vertex Shader
// ============================================================================

vertex SSRVertexOut vertexFunction(
	SSRVertexIn in [[stage_in]]
) {
	SSRVertexOut out;
	out.uv_interp = in.uv;
	out.position = in.position;
	out.pos_interp = out.position.xy;
	return out;
}

// ============================================================================
// Helper Functions
// ============================================================================

float2 view_to_screen(float3 view_pos, thread float& w, float4x4 projection) {
	float4 projected = projection * float4(view_pos, 1.0);
	projected.xyz /= projected.w;
	projected.xy = projected.xy * 0.5 + 0.5;
	w = projected.w;
	return projected.xy;
}

// ============================================================================
// Fragment Shader
// ============================================================================

fragment float4 fragmentFunction(
	SSRVertexOut in [[stage_in]],
	constant SSRUniforms& uniforms [[buffer(1)]],
	texture2d<float> source_diffuse [[texture(0)]],
	sampler diffuseSampler [[sampler(0)]],
	texture2d<float> source_normal_roughness [[texture(1)]],
	sampler normalSampler [[sampler(1)]],
	texture2d<float> source_depth [[texture(2)]],
	sampler depthSampler [[sampler(2)]]
) {
	float4 diffuse = source_diffuse.sample(diffuseSampler, in.uv_interp);
	float4 normal_roughness = source_normal_roughness.sample(normalSampler, in.uv_interp);

	float3 normal = normal_roughness.xyz * 2.0 - 1.0;
	float roughness = normal_roughness.w;

	float depth_tex = source_depth.sample(depthSampler, in.uv_interp).r;

	float4 world_pos = uniforms.inverse_projection * float4(in.uv_interp * 2.0 - 1.0, depth_tex * 2.0 - 1.0, 1.0);
	float3 vertex = world_pos.xyz / world_pos.w;

#ifdef USE_ORTHOGONAL_PROJECTION
	float3 view_dir = float3(0.0, 0.0, -1.0);
#else
	float3 view_dir = normalize(vertex);
#endif
	float3 ray_dir = normalize(reflect(view_dir, normal));

	if (dot(ray_dir, normal) < 0.001) {
		return float4(0.0);
	}

	// make ray length and clip it against the near plane
	float ray_len = (vertex.z + ray_dir.z * uniforms.camera_z_far) > -uniforms.camera_z_near ? (-uniforms.camera_z_near - vertex.z) / ray_dir.z : uniforms.camera_z_far;
	float3 ray_end = vertex + ray_dir * ray_len;

	float w_begin;
	float2 vp_line_begin = view_to_screen(vertex, w_begin, uniforms.projection);
	float w_end;
	float2 vp_line_end = view_to_screen(ray_end, w_end, uniforms.projection);
	float2 vp_line_dir = vp_line_end - vp_line_begin;

	// interpolate w along the ray for perspective correct reflections
	w_begin = 1.0 / w_begin;
	w_end = 1.0 / w_end;

	float z_begin = vertex.z * w_begin;
	float z_end = ray_end.z * w_end;

	float2 line_begin = vp_line_begin / uniforms.pixel_size;
	float2 line_dir = vp_line_dir / uniforms.pixel_size;
	float z_dir = z_end - z_begin;
	float w_dir = w_end - w_begin;

	// clip the line to the viewport edges
	float scale_max_x = min(1.0, 0.99 * (1.0 - vp_line_begin.x) / max(1e-5, vp_line_dir.x));
	float scale_max_y = min(1.0, 0.99 * (1.0 - vp_line_begin.y) / max(1e-5, vp_line_dir.y));
	float scale_min_x = min(1.0, 0.99 * vp_line_begin.x / max(1e-5, -vp_line_dir.x));
	float scale_min_y = min(1.0, 0.99 * vp_line_begin.y / max(1e-5, -vp_line_dir.y));
	float line_clip = min(scale_max_x, scale_max_y) * min(scale_min_x, scale_min_y);
	line_dir *= line_clip;
	z_dir *= line_clip;
	w_dir *= line_clip;

	// clip z and w advance to line advance
	float2 line_advance = normalize(line_dir);
	float step_size = length(line_advance) / length(line_dir);
	float z_advance = z_dir * step_size;
	float w_advance = w_dir * step_size;

	// make line advance faster if direction is closer to pixel edges
	float advance_angle_adj = 1.0 / max(abs(line_advance.x), abs(line_advance.y));
	line_advance *= advance_angle_adj;
	z_advance *= advance_angle_adj;
	w_advance *= advance_angle_adj;

	float2 pos = line_begin;
	float z = z_begin;
	float w = w_begin;
	float z_from = z / w;
	float z_to = z_from;
	float depth;
	float2 prev_pos = pos;

	bool found = false;
	float steps_taken = 0.0;

	for (int i = 0; i < uniforms.num_steps; i++) {
		pos += line_advance;
		z += z_advance;
		w += w_advance;

		// convert to linear depth
		depth = source_depth.sample(depthSampler, pos * uniforms.pixel_size).r * 2.0 - 1.0;
#ifdef USE_ORTHOGONAL_PROJECTION
		depth = ((depth + (uniforms.camera_z_far + uniforms.camera_z_near) / (uniforms.camera_z_far - uniforms.camera_z_near)) * (uniforms.camera_z_far - uniforms.camera_z_near)) / 2.0;
#else
		depth = 2.0 * uniforms.camera_z_near * uniforms.camera_z_far / (uniforms.camera_z_far + uniforms.camera_z_near - depth * (uniforms.camera_z_far - uniforms.camera_z_near));
#endif
		depth = -depth;

		z_from = z_to;
		z_to = z / w;

		if (depth > z_to) {
			// if depth was surpassed
			if ((depth <= max(z_to, z_from) + uniforms.depth_tolerance) && (-depth < uniforms.camera_z_far)) {
				// check the depth tolerance and far clip
				found = true;
			}
			break;
		}

		steps_taken += 1.0;
		prev_pos = pos;
	}

	if (found) {
		float margin_blend = 1.0;

		float2 margin = float2((uniforms.viewport_size.x + uniforms.viewport_size.y) * 0.5 * 0.05);
		if (any(pos < float2(0.0)) || any(pos > uniforms.viewport_size * 0.5)) {
			// clip at the screen edges
			return float4(0.0);
		}

		{
			// blend fading out towards inner margin
			float2 margin_grad = mix(uniforms.viewport_size * 0.5 - pos, pos, float2(pos < uniforms.viewport_size * 0.25));
			margin_blend = smoothstep(0.0, margin.x * margin.y, margin_grad.x * margin_grad.y);
		}

		float2 final_pos;
		float grad = (steps_taken + 1.0) / float(uniforms.num_steps);
		float initial_fade = uniforms.curve_fade_in == 0.0 ? 1.0 : pow(clamp(grad, 0.0, 1.0), uniforms.curve_fade_in);
		float fade = pow(clamp(1.0 - grad, 0.0, 1.0), uniforms.distance_fade) * initial_fade;
		final_pos = pos;

#ifdef REFLECT_ROUGHNESS
		float4 final_color;
		// if roughness is enabled, do screen space cone tracing
		if (roughness > 0.001) {
			float gloss = 1.0 - roughness;
			float cone_angle = roughness * M_PI * 0.5;
			float2 cone_dir = final_pos - line_begin;
			float cone_len = length(cone_dir);
			cone_dir = normalize(cone_dir);
			float max_mipmap = uniforms.filter_mipmap_levels - 1.0;
			float gloss_mult = gloss;

			float rem_alpha = 1.0;
			final_color = float4(0.0);

			for (int i = 0; i < 7; i++) {
				float op_len = 2.0 * tan(cone_angle) * cone_len;
				float radius;
				{
					float a = op_len;
					float h = cone_len;
					float a2 = a * a;
					float fh2 = 4.0 * h * h;
					radius = (a * (sqrt(a2 + fh2) - a)) / (4.0 * h);
				}

				float2 sample_pos = (line_begin + cone_dir * (cone_len - radius)) * uniforms.pixel_size;
				float mipmap = clamp(log2(radius), 0.0, max_mipmap);

				float4 sample_color = source_diffuse.sample(diffuseSampler, sample_pos, level(mipmap));

				sample_color.rgb *= gloss_mult;
				sample_color.a = gloss_mult;

				rem_alpha -= sample_color.a;
				if (rem_alpha < 0.0) {
					sample_color.rgb *= (1.0 - abs(rem_alpha));
				}

				final_color += sample_color;

				if (final_color.a >= 0.95) {
					break;
				}

				cone_len -= radius * 2.0;
				gloss_mult *= gloss;
			}
		} else {
			final_color = source_diffuse.sample(diffuseSampler, final_pos * uniforms.pixel_size, level(0.0));
		}

		return float4(final_color.rgb, fade * margin_blend);
#else
		return float4(source_diffuse.sample(diffuseSampler, final_pos * uniforms.pixel_size, level(0.0)).rgb, fade * margin_blend);
#endif

	} else {
		return float4(0.0, 0.0, 0.0, 0.0);
	}
}
