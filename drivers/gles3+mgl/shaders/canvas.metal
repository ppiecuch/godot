/**************************************************************************/
/*  canvas.metal                                                          */
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

// ============================================================================
// Uniform Buffer Objects
// ============================================================================

// CanvasItemData UBO
struct CanvasItemData {
	float4x4 projection_matrix;
	float time;
};

// LightData UBO
struct LightData {
	float4x4 light_matrix;
	float4x4 light_local_matrix;
	float4x4 shadow_matrix;
	float4 light_color;
	float4 light_shadow_color;
	float2 light_pos;
	float shadowpixel_size;
	float shadow_gradient;
	float light_height;
	float light_outside_alpha;
	float shadow_distance_mult;
};

// Canvas uniforms
struct CanvasUniforms {
	float4x4 modelview_matrix;
	float4x4 extra_matrix;
	float4x4 world_matrix;
	float4x4 inv_world_matrix;
	float2 color_texpixel_size;
	float4 final_modulate;
#ifdef USE_TEXTURE_RECT
	float4 dst_rect;
	float4 src_rect;
	bool clip_rect_uv;
#ifdef USE_NINEPATCH
	int np_repeat_v;
	int np_repeat_h;
	bool np_draw_center;
	float4 np_margins;
#endif
#endif
#ifdef USE_SKELETON
	float4x4 skeleton_transform;
	float4x4 skeleton_transform_inverse;
#endif
	bool use_default_normal;
	bool use_default_mask;
	float mask_cut_off;
	float3 mask_channels_mixer;
#ifdef SCREEN_UV_USED
	float2 screen_pixel_size;
#endif
};

// ============================================================================
// Vertex Input Structures
// ============================================================================

struct CanvasVertexIn {
#ifdef VERTEX_VEC3_USED
	float3 position [[attribute(0)]];
#else
	float2 position [[attribute(0)]];
#endif
#ifdef USE_ATTRIB_NORMAL
	float3 normal [[attribute(1)]];
#endif
#ifdef USE_ATTRIB_LIGHT_ANGLE
	float light_angle [[attribute(2)]];
#endif
	float4 color [[attribute(3)]];
#ifndef USE_TEXTURE_RECT
	float2 uv [[attribute(4)]];
#ifdef USE_ATTRIB_UV2
	float2 uv2 [[attribute(5)]];
#endif
#endif
#ifdef USE_ATTRIB_MODULATE
	float4 modulate [[attribute(5)]];
#endif
#ifdef USE_ATTRIB_LARGE_VERTEX
	float2 translate [[attribute(6)]];
	float4 basis [[attribute(7)]];
#endif
#ifdef USE_SKELETON
	uint4 bone_indices [[attribute(6)]];
	float4 bone_weights [[attribute(7)]];
#endif
#ifdef USE_INSTANCING
	float4 instance_xform0 [[attribute(8)]];
	float4 instance_xform1 [[attribute(9)]];
	float4 instance_xform2 [[attribute(10)]];
	float4 instance_color [[attribute(11)]];
#ifdef USE_INSTANCE_CUSTOM
	float4 instance_custom [[attribute(12)]];
#endif
#endif
};

// ============================================================================
// Vertex Output / Fragment Input
// ============================================================================

struct CanvasVertexOut {
	float4 position [[position]];
	float2 uv_interp;
#ifdef USE_ATTRIB_UV2
	float2 uv2_interp;
#endif
	float4 color_interp;
#ifdef USE_ATTRIB_MODULATE
	float4 modulate_interp [[flat]];
#endif
#ifdef USE_NINEPATCH
	float2 pixel_size_interp;
#endif
#ifdef USE_LIGHTING
	float4 light_uv_interp;
	float2 transformed_light_uv;
	float4 local_rot;
#ifdef USE_SHADOWS
	float2 pos;
#endif
#endif
	float point_size [[point_size]];
};

// ============================================================================
// Vertex Shader Globals (placeholder for custom shader code)
// ============================================================================

/* VERTEX_SHADER_GLOBALS */

// ============================================================================
// Vertex Shader
// ============================================================================

vertex CanvasVertexOut vertexFunction(
	CanvasVertexIn in [[stage_in]],
	constant CanvasItemData& canvas_data [[buffer(0)]],
	constant CanvasUniforms& uniforms [[buffer(1)]]
#ifdef USE_LIGHTING
	, constant LightData& light_data [[buffer(2)]]
#endif
#ifdef USE_SKELETON
	, texture2d<float> skeleton_texture [[texture(4)]]
#endif
#if defined(USE_MATERIAL)
	, constant void* material_data [[buffer(3)]]
#endif
) {
	CanvasVertexOut out;

	float4 color = in.color;

#ifdef VERTEX_VEC3_USED
	float vertex_z = in.position.z;
#else
	float vertex_z = 0.0;
#endif

#ifdef USE_ATTRIB_NORMAL
	float3 normal = in.normal;
#endif

	float4x4 extra_matrix_instance = uniforms.extra_matrix;
	float4 instance_custom = float4(0.0);

#ifdef USE_INSTANCING
	extra_matrix_instance = uniforms.extra_matrix * transpose(float4x4(
		in.instance_xform0,
		in.instance_xform1,
		in.instance_xform2,
		float4(0.0, 0.0, 0.0, 1.0)
	));
	color *= in.instance_color;
#ifdef USE_INSTANCE_CUSTOM
	instance_custom = in.instance_custom;
#endif
#endif

	float4 outvec;
	float2 uv_interp;

#ifdef USE_TEXTURE_RECT
	if (uniforms.dst_rect.z < 0.0) { // Transpose is encoded as negative dst_rect.z
		uv_interp = uniforms.src_rect.xy + abs(uniforms.src_rect.zw) * in.position.yx;
	} else {
		uv_interp = uniforms.src_rect.xy + abs(uniforms.src_rect.zw) * in.position.xy;
	}

	float2 flip_mask = select(float2(0.0), float2(1.0), uniforms.src_rect.zw < float2(0.0));
	outvec = float4(uniforms.dst_rect.xy + abs(uniforms.dst_rect.zw) * mix(in.position.xy, float2(1.0) - in.position.xy, flip_mask), vertex_z, 1.0);
#else
	uv_interp = in.uv;
#ifdef USE_ATTRIB_UV2
	out.uv2_interp = in.uv2;
#endif
	outvec = float4(in.position.xy, vertex_z, 1.0);
#endif

#ifdef USE_PARTICLES
	// Scale by texture size
	outvec.xyz /= float3(uniforms.color_texpixel_size, 1.0);
#endif

#if !defined(SKIP_TRANSFORM_USED) && defined(VERTEX_WORLD_COORDS_USED)
	outvec = uniforms.world_matrix * extra_matrix_instance * outvec;
#endif

	float point_size = 1.0;
	float2 uv = uv_interp;
#ifdef USE_ATTRIB_UV2
	float2 uv2 = out.uv2_interp;
#endif

	{
		/* VERTEX_SHADER_CODE */
	}

	out.point_size = point_size;
	out.uv_interp = uv;
#ifdef USE_ATTRIB_UV2
	out.uv2_interp = uv2;
#endif

#ifdef USE_NINEPATCH
	out.pixel_size_interp = abs(uniforms.dst_rect.zw) * in.position.xy;
#endif

#ifdef USE_ATTRIB_MODULATE
	out.modulate_interp = in.modulate;
#endif

#ifdef USE_ATTRIB_LARGE_VERTEX
	// Transform is in attributes
	float2 temp;
	temp.x = (outvec.x * in.basis.x) + (outvec.y * in.basis.z);
	temp.y = (outvec.x * in.basis.y) + (outvec.y * in.basis.w);
	temp += in.translate;
	outvec.xyz = float3(temp, outvec.z);
#else
	// Transform is in uniforms
#ifndef SKIP_TRANSFORM_USED
#ifdef VERTEX_WORLD_COORDS_USED
	outvec = uniforms.inv_world_matrix * outvec;
#else
	outvec = extra_matrix_instance * outvec;
#endif
	outvec = uniforms.modelview_matrix * outvec;
#endif
#endif

	out.color_interp = color;

#ifdef USE_PIXEL_SNAP
	outvec.xy = floor(outvec.xy + 0.5);
	out.uv_interp += 1e-5;
#endif

#ifdef USE_SKELETON
	if (any(in.bone_weights != float4(0.0))) {
		int4 bone_indicesi = int4(in.bone_indices);

		int2 tex_ofs = int2(bone_indicesi.x % 256, (bone_indicesi.x / 256) * 2);
		float4x2 m;
		m[0] = skeleton_texture.read(uint2(tex_ofs), 0).xy;
		m[1] = skeleton_texture.read(uint2(tex_ofs + int2(0, 1)), 0).xy;
		// Continue reading bone matrices...
		// (Simplified - full implementation would read all 4 bones)

		// Apply bone transform
		float4x4 bone_matrix = uniforms.skeleton_transform *
			transpose(float4x4(
				float4(m[0].x, m[0].y, 0.0, 0.0),
				float4(m[1].x, m[1].y, 0.0, 0.0),
				float4(0.0, 0.0, 1.0, 0.0),
				float4(0.0, 0.0, 0.0, 1.0)
			)) * uniforms.skeleton_transform_inverse;

		outvec = bone_matrix * outvec;
	}
#endif

	out.position = canvas_data.projection_matrix * outvec;

#ifdef USE_LIGHTING
	out.light_uv_interp.xy = (light_data.light_matrix * outvec).xy;
	out.light_uv_interp.zw = (light_data.light_local_matrix * outvec).xy;

	float3x3 inverse_light_matrix = float3x3(
		normalize(light_data.light_matrix[0].xyz),
		normalize(light_data.light_matrix[1].xyz),
		normalize(light_data.light_matrix[2].xyz)
	);
	out.transformed_light_uv = (inverse_light_matrix * float3(out.light_uv_interp.zw, 0.0)).xy;

#ifdef USE_SHADOWS
	out.pos = outvec.xy;
#endif

#ifdef USE_ATTRIB_LIGHT_ANGLE
	float la = abs(in.light_angle) - 1.0;
	float4 vla;
	vla.xy = float2(cos(la), sin(la));
	vla.zw = float2(-vla.y, vla.x);
	vla.zw *= sign(in.light_angle);

	out.local_rot.xy = normalize((uniforms.modelview_matrix * (extra_matrix_instance * float4(vla.xy, 0.0, 0.0))).xy);
	out.local_rot.zw = normalize((uniforms.modelview_matrix * (extra_matrix_instance * float4(vla.zw, 0.0, 0.0))).xy);
#else
	out.local_rot.xy = normalize((uniforms.modelview_matrix * (extra_matrix_instance * float4(1.0, 0.0, 0.0, 0.0))).xy);
	out.local_rot.zw = normalize((uniforms.modelview_matrix * (extra_matrix_instance * float4(0.0, 1.0, 0.0, 0.0))).xy);
#ifdef USE_TEXTURE_RECT
	out.local_rot.xy *= sign(uniforms.src_rect.z);
	out.local_rot.zw *= sign(uniforms.src_rect.w);
#endif
#endif

#endif // USE_LIGHTING

	return out;
}

// ============================================================================
// Fragment Shader Globals (placeholder for custom shader code)
// ============================================================================

/* FRAGMENT_SHADER_GLOBALS */

// ============================================================================
// NinePatch Helper Functions
// ============================================================================

#ifdef USE_NINEPATCH
#ifdef USE_NINEPATCH_SCALING
float map_ninepatch_axis(float pixel, float draw_size, float tex_pixel_size,
						 float margin_begin, float margin_end, float s_ratio,
						 int np_repeat, thread int& draw_center, bool np_draw_center) {
	float tex_size = 1.0 / tex_pixel_size;
	float screen_margin_begin = margin_begin / s_ratio;
	float screen_margin_end = margin_end / s_ratio;

	if (pixel < screen_margin_begin) {
		return pixel * s_ratio * tex_pixel_size;
	} else if (pixel >= draw_size - screen_margin_end) {
		return (tex_size - (draw_size - pixel) * s_ratio) * tex_pixel_size;
	} else {
		if (!np_draw_center) {
			draw_center--;
		}
		if (np_repeat == 0) { // Stretch
			float ratio = (pixel - screen_margin_begin) / (draw_size - screen_margin_begin - screen_margin_end);
			return (margin_begin + ratio * (tex_size - margin_begin - margin_end)) * tex_pixel_size;
		} else if (np_repeat == 1) { // Tile
			float ofs = fmod((pixel - screen_margin_begin), tex_size - margin_begin - margin_end);
			return (margin_begin + ofs) * tex_pixel_size;
		} else { // Tile Fit
			float src_area = draw_size - screen_margin_begin - screen_margin_end;
			float dst_area = tex_size - margin_begin - margin_end;
			float scale = max(1.0, floor(src_area / max(dst_area, 0.0000001) + 0.5));
			float ratio = (pixel - screen_margin_begin) / src_area;
			ratio = fmod(ratio * scale, 1.0);
			return (margin_begin + ratio * dst_area) * tex_pixel_size;
		}
	}
}
#else
float map_ninepatch_axis(float pixel, float draw_size, float tex_pixel_size,
						 float margin_begin, float margin_end,
						 int np_repeat, thread int& draw_center, bool np_draw_center) {
	float tex_size = 1.0 / tex_pixel_size;

	if (pixel < margin_begin) {
		return pixel * tex_pixel_size;
	} else if (pixel >= draw_size - margin_end) {
		return (tex_size - (draw_size - pixel)) * tex_pixel_size;
	} else {
		if (!np_draw_center) {
			draw_center--;
		}
		if (np_repeat == 0) { // Stretch
			float ratio = (pixel - margin_begin) / (draw_size - margin_begin - margin_end);
			return (margin_begin + ratio * (tex_size - margin_begin - margin_end)) * tex_pixel_size;
		} else if (np_repeat == 1) { // Tile
			float ofs = fmod((pixel - margin_begin), tex_size - margin_begin - margin_end);
			return (margin_begin + ofs) * tex_pixel_size;
		} else if (np_repeat == 2) { // Tile Fit
			float src_area = draw_size - margin_begin - margin_end;
			float dst_area = tex_size - margin_begin - margin_end;
			float scale = max(1.0, floor(src_area / max(dst_area, 0.0000001) + 0.5));
			float ratio = (pixel - margin_begin) / src_area;
			ratio = fmod(ratio * scale, 1.0);
			return (margin_begin + ratio * dst_area) * tex_pixel_size;
		}
		return 0.0;
	}
}
#endif
#endif

// ============================================================================
// Light Compute Function (placeholder for custom light shader code)
// ============================================================================

#ifdef USE_LIGHTING
void light_compute(
	thread float4& light,
	thread float2& light_vec,
	thread float& light_height,
	thread float4& light_color,
	float2 light_uv,
	thread float4& shadow_color,
	thread float2& shadow_vec,
	float3 normal,
	float2 uv,
#ifdef SCREEN_UV_USED
	float2 screen_uv,
#endif
	float4 color
) {
	/* LIGHT_SHADER_CODE */
}
#endif

// ============================================================================
// sRGB Conversion
// ============================================================================

float3 linear_to_srgb(float3 color) {
	float3 a = float3(0.055);
	return select((float3(1.0) + a) * pow(color, float3(1.0 / 2.4)) - a,
				  12.92 * color,
				  color < float3(0.0031308));
}

// ============================================================================
// Shadow Depth Reading
// ============================================================================

#ifdef USE_SHADOWS
#ifdef USE_RGBA_SHADOWS
float shadow_depth(float4 shadow_sample) {
	return dot(shadow_sample, float4(1.0 / (255.0 * 255.0 * 255.0), 1.0 / (255.0 * 255.0), 1.0 / 255.0, 1.0));
}
#else
float shadow_depth(float4 shadow_sample) {
	return shadow_sample.r;
}
#endif
#endif

// ============================================================================
// Fragment Shader
// ============================================================================

fragment float4 fragmentFunction(
	CanvasVertexOut in [[stage_in]],
	constant CanvasItemData& canvas_data [[buffer(0)]],
	constant CanvasUniforms& uniforms [[buffer(1)]]
#ifdef USE_LIGHTING
	, constant LightData& light_data [[buffer(2)]]
#endif
	, texture2d<float> color_texture [[texture(0)]]
	, sampler colorSampler [[sampler(0)]]
	, texture2d<float> normal_texture [[texture(1)]]
	, sampler normalSampler [[sampler(1)]]
	, texture2d<float> mask_texture [[texture(2)]]
	, sampler maskSampler [[sampler(2)]]
#ifdef USE_LIGHTING
	, texture2d<float> light_texture [[texture(3)]]
	, sampler lightSampler [[sampler(3)]]
#ifdef USE_SHADOWS
	, texture2d<float> shadow_texture [[texture(5)]]
	, sampler shadowSampler [[sampler(5)]]
#endif
#endif
#ifdef SCREEN_TEXTURE_USED
	, texture2d<float> screen_texture [[texture(6)]]
	, sampler screenSampler [[sampler(6)]]
#endif
#if defined(USE_MATERIAL)
	, constant void* material_data [[buffer(3)]]
#endif
) {
	float4 color = in.color_interp;
	float2 uv = in.uv_interp;

	// Mask testing
	if (uniforms.use_default_mask) {
		float mask = dot(mask_texture.sample(maskSampler, uv).xyz, uniforms.mask_channels_mixer);
		if (mask <= uniforms.mask_cut_off) {
			discard_fragment();
		}
	}

#ifdef USE_ATTRIB_UV2
	float2 uv2 = in.uv2_interp;
#endif

#ifdef USE_TEXTURE_RECT
#ifdef USE_NINEPATCH
	int draw_center = 2;
#ifdef USE_NINEPATCH_SCALING
	float s_ratio = max((1.0 / uniforms.color_texpixel_size.x) / abs(uniforms.dst_rect.z),
						(1.0 / uniforms.color_texpixel_size.y) / abs(uniforms.dst_rect.w));
	s_ratio = max(1.0, s_ratio);
	uv = float2(
		map_ninepatch_axis(in.pixel_size_interp.x, abs(uniforms.dst_rect.z), uniforms.color_texpixel_size.x,
						   uniforms.np_margins.x, uniforms.np_margins.z, s_ratio, uniforms.np_repeat_h, draw_center, uniforms.np_draw_center),
		map_ninepatch_axis(in.pixel_size_interp.y, abs(uniforms.dst_rect.w), uniforms.color_texpixel_size.y,
						   uniforms.np_margins.y, uniforms.np_margins.w, s_ratio, uniforms.np_repeat_v, draw_center, uniforms.np_draw_center)
	);
#else
	uv = float2(
		map_ninepatch_axis(in.pixel_size_interp.x, abs(uniforms.dst_rect.z), uniforms.color_texpixel_size.x,
						   uniforms.np_margins.x, uniforms.np_margins.z, uniforms.np_repeat_h, draw_center, uniforms.np_draw_center),
		map_ninepatch_axis(in.pixel_size_interp.y, abs(uniforms.dst_rect.w), uniforms.color_texpixel_size.y,
						   uniforms.np_margins.y, uniforms.np_margins.w, uniforms.np_repeat_v, draw_center, uniforms.np_draw_center)
	);
#endif
	if (draw_center == 0) {
		color.a = 0.0;
	}
	uv = uv * uniforms.src_rect.zw + uniforms.src_rect.xy;
#endif

	if (uniforms.clip_rect_uv) {
		uv = clamp(uv, uniforms.src_rect.xy, uniforms.src_rect.xy + abs(uniforms.src_rect.zw));
	}
#endif

#ifdef USE_DISTANCE_FIELD
	const float smoothing = 0.125;
	float dist = color_texture.sample(colorSampler, uv, level(0.0)).a;
	color.a = smoothstep(0.5 - smoothing, 0.5 + smoothing, dist);
#else
#if !defined(COLOR_USED)
	color *= color_texture.sample(colorSampler, uv);
#endif
#endif

	float3 normal;
	bool normal_used = false;

#if defined(NORMAL_USED)
	normal_used = true;
#endif

	if (uniforms.use_default_normal) {
		normal.xy = normal_texture.sample(normalSampler, uv, level(0.0)).xy * 2.0 - 1.0;
		normal.z = sqrt(max(0.0, 1.0 - dot(normal.xy, normal.xy)));
		normal_used = true;
	} else {
		normal = float3(0.0, 0.0, 1.0);
	}

#ifdef SCREEN_UV_USED
	float2 screen_uv = in.position.xy * uniforms.screen_pixel_size;
#endif

	{
		float normal_depth = 1.0;
#if defined(NORMALMAP_USED)
		float3 normal_map = float3(0.0, 0.0, 1.0);
		normal_used = true;
#endif

		// Determine final_modulate_alias
#ifdef USE_ATTRIB_MODULATE
		float4 final_modulate_alias = in.modulate_interp;
#else
		float4 final_modulate_alias = uniforms.final_modulate;
#endif

		/* FRAGMENT_SHADER_CODE */

#if defined(NORMALMAP_USED)
		normal = mix(float3(0.0, 0.0, 1.0), normal_map * float3(2.0, -2.0, 1.0) - float3(1.0, -1.0, 0.0), normal_depth);
#endif
	}

#ifdef DEBUG_ENCODED_32
	float enc32 = dot(color, float4(1.0 / (256.0 * 256.0 * 256.0), 1.0 / (256.0 * 256.0), 1.0 / 256.0, 1.0));
	color = float4(float3(enc32), 1.0);
#endif

#if !defined(MODULATE_USED)
#ifdef USE_ATTRIB_MODULATE
	color *= in.modulate_interp;
#else
	color *= uniforms.final_modulate;
#endif
#endif

#ifdef USE_LIGHTING
	float2 light_vec = in.transformed_light_uv;
	float2 shadow_vec = in.transformed_light_uv;

	if (normal_used) {
		float2x2 local_rot_mat = float2x2(in.local_rot.xy, in.local_rot.zw);
		normal.xy = local_rot_mat * normal.xy;
	}

	float att = 1.0;
	float2 light_uv = in.light_uv_interp.xy;
	float4 light = light_texture.sample(lightSampler, light_uv);

	if (any(in.light_uv_interp.xy < float2(0.0)) || any(in.light_uv_interp.xy >= float2(1.0))) {
		color.a *= light_data.light_outside_alpha;
	} else {
		float real_light_height = light_data.light_height;
		float4 real_light_color = light_data.light_color;
		float4 real_light_shadow_color = light_data.light_shadow_color;

#if defined(USE_LIGHT_SHADER_CODE)
		light_compute(
			light,
			light_vec,
			real_light_height,
			real_light_color,
			light_uv,
			real_light_shadow_color,
			shadow_vec,
			normal,
			uv,
#ifdef SCREEN_UV_USED
			screen_uv,
#endif
			color
		);
#endif

		light *= real_light_color;

		if (normal_used) {
			float3 light_normal = normalize(float3(light_vec, -real_light_height));
			light *= max(dot(-light_normal, normal), 0.0);
		}

		color *= light;

#ifdef USE_SHADOWS
#ifdef SHADOW_VEC_USED
		float3x3 inverse_light_matrix = float3x3(
			normalize(light_data.light_matrix[0].xyz),
			normalize(light_data.light_matrix[1].xyz),
			normalize(light_data.light_matrix[2].xyz)
		);
		shadow_vec = (inverse_light_matrix * float3(shadow_vec, 0.0)).xy;
#else
		shadow_vec = in.light_uv_interp.zw;
#endif

		float angle_to_light = -atan2(shadow_vec.x, shadow_vec.y);
		constexpr float PI = 3.14159265358979323846264;

		float su, sz;
		float abs_angle = abs(angle_to_light);
		float2 point;
		float sh;

		if (abs_angle < 45.0 * PI / 180.0) {
			point = shadow_vec;
			sh = 0.0 + (1.0 / 8.0);
		} else if (abs_angle > 135.0 * PI / 180.0) {
			point = -shadow_vec;
			sh = 0.5 + (1.0 / 8.0);
		} else if (angle_to_light > 0.0) {
			point = float2(shadow_vec.y, -shadow_vec.x);
			sh = 0.25 + (1.0 / 8.0);
		} else {
			point = float2(-shadow_vec.y, shadow_vec.x);
			sh = 0.75 + (1.0 / 8.0);
		}

		float4 s = light_data.shadow_matrix * float4(point, 0.0, 1.0);
		s.xyz /= s.w;
		su = s.x * 0.5 + 0.5;
		sz = s.z * 0.5 + 0.5;

		float shadow_attenuation = 0.0;

#ifdef SHADOW_FILTER_NEAREST
		float sd = shadow_depth(shadow_texture.sample(shadowSampler, float2(su, sh)));
#ifdef SHADOW_USE_GRADIENT
		shadow_attenuation = 1.0 - smoothstep(sd, sd + light_data.shadow_gradient, sz);
#else
		shadow_attenuation = step(sz, sd);
#endif
#endif

#ifdef SHADOW_FILTER_PCF3
		for (int i = -1; i <= 1; i++) {
			float sd = shadow_depth(shadow_texture.sample(shadowSampler, float2(su + float(i) * light_data.shadowpixel_size, sh)));
#ifdef SHADOW_USE_GRADIENT
			shadow_attenuation += 1.0 - smoothstep(sd, sd + light_data.shadow_gradient, sz);
#else
			shadow_attenuation += step(sz, sd);
#endif
		}
		shadow_attenuation /= 3.0;
#endif

#ifdef SHADOW_FILTER_PCF5
		for (int i = -2; i <= 2; i++) {
			float sd = shadow_depth(shadow_texture.sample(shadowSampler, float2(su + float(i) * light_data.shadowpixel_size, sh)));
#ifdef SHADOW_USE_GRADIENT
			shadow_attenuation += 1.0 - smoothstep(sd, sd + light_data.shadow_gradient, sz);
#else
			shadow_attenuation += step(sz, sd);
#endif
		}
		shadow_attenuation /= 5.0;
#endif

#ifdef SHADOW_FILTER_PCF7
		for (int i = -3; i <= 3; i++) {
			float sd = shadow_depth(shadow_texture.sample(shadowSampler, float2(su + float(i) * light_data.shadowpixel_size, sh)));
#ifdef SHADOW_USE_GRADIENT
			shadow_attenuation += 1.0 - smoothstep(sd, sd + light_data.shadow_gradient, sz);
#else
			shadow_attenuation += step(sz, sd);
#endif
		}
		shadow_attenuation /= 7.0;
#endif

#ifdef SHADOW_FILTER_PCF9
		for (int i = -4; i <= 4; i++) {
			float sd = shadow_depth(shadow_texture.sample(shadowSampler, float2(su + float(i) * light_data.shadowpixel_size, sh)));
#ifdef SHADOW_USE_GRADIENT
			shadow_attenuation += 1.0 - smoothstep(sd, sd + light_data.shadow_gradient, sz);
#else
			shadow_attenuation += step(sz, sd);
#endif
		}
		shadow_attenuation /= 9.0;
#endif

#ifdef SHADOW_FILTER_PCF13
		for (int i = -6; i <= 6; i++) {
			float sd = shadow_depth(shadow_texture.sample(shadowSampler, float2(su + float(i) * light_data.shadowpixel_size, sh)));
#ifdef SHADOW_USE_GRADIENT
			shadow_attenuation += 1.0 - smoothstep(sd, sd + light_data.shadow_gradient, sz);
#else
			shadow_attenuation += step(sz, sd);
#endif
		}
		shadow_attenuation /= 13.0;
#endif

		color = mix(real_light_shadow_color, color, shadow_attenuation);
#endif // USE_SHADOWS
	}
#endif // USE_LIGHTING

#ifdef LINEAR_TO_SRGB
	color.rgb = linear_to_srgb(color.rgb);
#endif

	return color;
}
