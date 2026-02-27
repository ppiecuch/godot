/**************************************************************************/
/*  scene.metal                                                           */
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
#define SHADER_IS_SRGB false
#define RADIANCE_MAX_LOD 5.0

#ifndef MAX_LIGHT_DATA_STRUCTS
#define MAX_LIGHT_DATA_STRUCTS 256
#endif

#ifndef MAX_FORWARD_LIGHTS
#define MAX_FORWARD_LIGHTS 32
#endif

#ifndef MAX_REFLECTION_DATA_STRUCTS
#define MAX_REFLECTION_DATA_STRUCTS 64
#endif

// ============================================================================
// Uniform Buffer Objects
// ============================================================================

// SceneData UBO
struct SceneData {
	float4x4 projection_matrix;
	float4x4 inv_projection_matrix;
	float4x4 camera_inverse_matrix;
	float4x4 camera_matrix;

	float4 ambient_light_color;
	float4 bg_color;

	float4 fog_color_enabled;
	float4 fog_sun_color_amount;

	float ambient_energy;
	float bg_energy;

	float z_offset;
	float z_slope_scale;
	float shadow_dual_paraboloid_render_zfar;
	float shadow_dual_paraboloid_render_side;

	float2 viewport_size;
	float2 screen_pixel_size;
	float2 shadow_atlas_pixel_size;
	float2 directional_shadow_pixel_size;

	float time;
	float z_far;
	float reflection_multiplier;
	float subsurface_scatter_width;
	float ambient_occlusion_affect_light;
	float ambient_occlusion_affect_ao_channel;
	float opaque_prepass_threshold;

	bool fog_depth_enabled;
	float fog_depth_begin;
	float fog_depth_end;
	float fog_density;
	float fog_depth_curve;
	bool fog_transmit_enabled;
	float fog_transmit_curve;
	bool fog_height_enabled;
	float fog_height_min;
	float fog_height_max;
	float fog_height_curve;

	int view_index;
};

// Light data structure for omni and spot lights
struct LightData {
	float4 light_pos_inv_radius;
	float4 light_direction_attenuation;
	float4 light_color_energy;
	float4 light_params; // cone attenuation, angle, specular, shadow enabled
	float4 light_clamp;
	float4 shadow_color_contact;
	float4x4 shadow_matrix;
};

// DirectionalLightData UBO
struct DirectionalLightData {
	float4 light_pos_inv_radius;
	float4 light_direction_attenuation;
	float4 light_color_energy;
	float4 light_params;
	float4 light_clamp;
	float4 shadow_color_contact;
	float4x4 shadow_matrix1;
	float4x4 shadow_matrix2;
	float4x4 shadow_matrix3;
	float4x4 shadow_matrix4;
	float4 shadow_split_offsets;
	float fade_from;
	float3 pad;
};

// Reflection probe data
struct ReflectionData {
	float4 box_extents;
	float4 box_offset;
	float4 params; // intensity, 0, interior, boxproject
	float4 ambient; // ambient color, energy
	float4 atlas_clamp;
	float4x4 local_matrix;
};

// Radiance UBO
struct RadianceData {
	float4x4 radiance_inverse_xform;
	float radiance_ambient_contribution;
};

// Scene uniforms
struct SceneUniforms {
	float4x4 world_transform;
#ifdef USE_LIGHTMAP
	float4 lightmap_uv_rect;
#endif
#ifdef USE_SKELETON
	float4x4 skeleton_transform;
	float4x4 skeleton_transform_inverse;
#endif
};

// ============================================================================
// Vertex Input Structure
// ============================================================================

struct SceneVertexIn {
	float4 position [[attribute(0)]];
#ifdef ENABLE_OCTAHEDRAL_COMPRESSION
	float4 normal_tangent [[attribute(2)]];
#else
	float3 normal [[attribute(1)]];
#if defined(ENABLE_TANGENT_INTERP) || defined(ENABLE_NORMALMAP) || defined(LIGHT_USE_ANISOTROPY)
	float4 tangent [[attribute(2)]];
#endif
#endif
#if defined(ENABLE_COLOR_INTERP)
	float4 color [[attribute(3)]];
#endif
#if defined(ENABLE_UV_INTERP)
	float2 uv [[attribute(4)]];
#endif
#if defined(ENABLE_UV2_INTERP) || defined(USE_LIGHTMAP)
	float2 uv2 [[attribute(5)]];
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
#if defined(ENABLE_INSTANCE_CUSTOM)
	float4 instance_custom [[attribute(12)]];
#endif
#endif
};

// ============================================================================
// Vertex Output / Fragment Input
// ============================================================================

struct SceneVertexOut {
	float4 position [[position]];
	float3 vertex_interp;
	float3 normal_interp;
#if defined(ENABLE_COLOR_INTERP)
	float4 color_interp;
#endif
#if defined(ENABLE_UV_INTERP)
	float2 uv_interp;
#endif
#if defined(ENABLE_UV2_INTERP) || defined(USE_LIGHTMAP)
	float2 uv2_interp;
#endif
#if defined(ENABLE_TANGENT_INTERP) || defined(ENABLE_NORMALMAP) || defined(LIGHT_USE_ANISOTROPY)
	float3 tangent_interp;
	float3 binormal_interp;
#endif
#ifdef USE_VERTEX_LIGHTING
	float4 diffuse_light_interp;
	float4 specular_light_interp;
#endif
#ifdef RENDER_DEPTH_DUAL_PARABOLOID
	float dp_clip;
#endif
	float4 position_interp;
	float point_size [[point_size]];
};

// MRT Fragment Output
struct SceneFragmentOut {
	float4 frag_color [[color(0)]];
#ifdef USE_MULTIPLE_RENDER_TARGETS
	float4 specular_buffer [[color(1)]];
	float4 normal_mr_buffer [[color(2)]];
#if defined(ENABLE_SSS)
	float sss_buffer [[color(3)]];
#endif
#endif
};

// ============================================================================
// Vertex Shader Globals (placeholder for custom shader code)
// ============================================================================

/* VERTEX_SHADER_GLOBALS */

// ============================================================================
// Helper Functions - Octahedral Compression
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
// Vertex Shader
// ============================================================================

vertex SceneVertexOut vertexFunction(
	SceneVertexIn in [[stage_in]],
	constant SceneData& scene_data [[buffer(0)]],
	constant SceneUniforms& uniforms [[buffer(1)]]
#ifdef USE_LIGHT_DIRECTIONAL
	, constant DirectionalLightData& directional_light [[buffer(3)]]
#endif
#ifdef USE_SKELETON
	, texture2d<float> skeleton_texture [[texture(1)]]
#endif
#if defined(USE_MATERIAL)
	, constant void* material_data [[buffer(2)]]
#endif
) {
	SceneVertexOut out;

	float4 vertex = in.position;
	float4x4 world_matrix = uniforms.world_transform;

#ifdef USE_INSTANCING
	{
		float4x4 m = float4x4(in.instance_xform0, in.instance_xform1, in.instance_xform2, float4(0.0, 0.0, 0.0, 1.0));
		world_matrix = world_matrix * transpose(m);
	}
#endif

	float3 normal;
#ifdef ENABLE_OCTAHEDRAL_COMPRESSION
	normal = oct_to_vec3(in.normal_tangent.xy);
#else
	normal = in.normal;
#endif

#if defined(ENABLE_TANGENT_INTERP) || defined(ENABLE_NORMALMAP) || defined(LIGHT_USE_ANISOTROPY)
	float3 tangent;
	float binormalf;
#ifdef ENABLE_OCTAHEDRAL_COMPRESSION
	tangent = oct_to_vec3(float2(in.normal_tangent.z, abs(in.normal_tangent.w) * 2.0 - 1.0));
	binormalf = sign(in.normal_tangent.w);
#else
	tangent = in.tangent.xyz;
	binormalf = in.tangent.w;
#endif
	float3 binormal = normalize(cross(normal, tangent) * binormalf);
#endif

#if defined(ENABLE_COLOR_INTERP)
	out.color_interp = in.color;
#ifdef USE_INSTANCING
	out.color_interp *= in.instance_color;
#endif
#endif

#if defined(ENABLE_UV_INTERP)
	out.uv_interp = in.uv;
#endif

#ifdef USE_LIGHTMAP
	out.uv2_interp = uniforms.lightmap_uv_rect.zw * in.uv2 + uniforms.lightmap_uv_rect.xy;
#else
#if defined(ENABLE_UV2_INTERP)
	out.uv2_interp = in.uv2;
#endif
#endif

	float4 instance_custom = float4(0.0);
#ifdef USE_INSTANCING
#if defined(ENABLE_INSTANCE_CUSTOM)
	instance_custom = in.instance_custom;
#endif
#endif

	float4x4 local_projection = scene_data.projection_matrix;

	// Transform using world coordinates
#if !defined(SKIP_TRANSFORM_USED) && defined(VERTEX_WORLD_COORDS_USED)
	vertex = world_matrix * vertex;
#if defined(ENSURE_CORRECT_NORMALS)
	float3x3 normal_matrix = float3x3(transpose(float4x4(world_matrix[0], world_matrix[1], world_matrix[2], float4(0.0, 0.0, 0.0, 1.0))));
	normal = normal_matrix * normal;
#else
	normal = normalize((world_matrix * float4(normal, 0.0)).xyz);
#endif
#if defined(ENABLE_TANGENT_INTERP) || defined(ENABLE_NORMALMAP) || defined(LIGHT_USE_ANISOTROPY)
	tangent = normalize((world_matrix * float4(tangent, 0.0)).xyz);
	binormal = normalize((world_matrix * float4(binormal, 0.0)).xyz);
#endif
#endif

	float roughness = 1.0;
	float point_size = 1.0;

	{
		/* VERTEX_SHADER_CODE */
	}

	out.point_size = point_size;

	// Using local coordinates
#if !defined(SKIP_TRANSFORM_USED) && !defined(VERTEX_WORLD_COORDS_USED)
	float4x4 modelview = scene_data.camera_inverse_matrix * world_matrix;
	vertex = modelview * vertex;
#if defined(ENSURE_CORRECT_NORMALS)
	float3x3 normal_matrix = float3x3(transpose(modelview));
	normal = normal_matrix * normal;
#else
	normal = normalize((modelview * float4(normal, 0.0)).xyz);
#endif
#if defined(ENABLE_TANGENT_INTERP) || defined(ENABLE_NORMALMAP) || defined(LIGHT_USE_ANISOTROPY)
	tangent = normalize((modelview * float4(tangent, 0.0)).xyz);
	binormal = normalize((modelview * float4(binormal, 0.0)).xyz);
#endif
#endif

	// World coordinates path
#if !defined(SKIP_TRANSFORM_USED) && defined(VERTEX_WORLD_COORDS_USED)
	vertex = scene_data.camera_inverse_matrix * vertex;
	normal = normalize((scene_data.camera_inverse_matrix * float4(normal, 0.0)).xyz);
#if defined(ENABLE_TANGENT_INTERP) || defined(ENABLE_NORMALMAP) || defined(LIGHT_USE_ANISOTROPY)
	tangent = normalize((scene_data.camera_inverse_matrix * float4(tangent, 0.0)).xyz);
	binormal = normalize((scene_data.camera_inverse_matrix * float4(binormal, 0.0)).xyz);
#endif
#endif

	out.vertex_interp = vertex.xyz;
	out.normal_interp = normal;

#if defined(ENABLE_TANGENT_INTERP) || defined(ENABLE_NORMALMAP) || defined(LIGHT_USE_ANISOTROPY)
	out.tangent_interp = tangent;
	out.binormal_interp = binormal;
#endif

#ifdef RENDER_DEPTH
#ifdef RENDER_DEPTH_DUAL_PARABOLOID
	out.vertex_interp.z *= scene_data.shadow_dual_paraboloid_render_side;
	out.normal_interp.z *= scene_data.shadow_dual_paraboloid_render_side;
	out.dp_clip = out.vertex_interp.z;

	float3 vtx = out.vertex_interp + normalize(out.vertex_interp) * scene_data.z_offset;
	float distance = length(vtx);
	vtx = normalize(vtx);
	vtx.xy /= 1.0 - vtx.z;
	vtx.z = (distance / scene_data.shadow_dual_paraboloid_render_zfar);
	vtx.z = vtx.z * 2.0 - 1.0;
	out.vertex_interp = vtx;
#else
	float z_ofs = scene_data.z_offset;
	z_ofs += (1.0 - abs(out.normal_interp.z)) * scene_data.z_slope_scale;
	out.vertex_interp.z -= z_ofs;
#endif
#endif

#if defined(OVERRIDE_POSITION)
	out.position = position;
#else
	out.position = local_projection * float4(out.vertex_interp, 1.0);
#endif

	out.position_interp = out.position;

#ifdef USE_VERTEX_LIGHTING
	out.diffuse_light_interp = float4(0.0);
	out.specular_light_interp = float4(0.0);
	// Vertex lighting computation would go here
#endif

	return out;
}

// ============================================================================
// Fragment Shader Globals (placeholder for custom shader code)
// ============================================================================

/* FRAGMENT_SHADER_GLOBALS */

// ============================================================================
// PBR Helper Functions
// ============================================================================

// Schlick Fresnel
float SchlickFresnel(float u) {
	float m = 1.0 - u;
	float m2 = m * m;
	return m2 * m2 * m; // pow(m, 5)
}

// F0 for metallic workflow
float3 F0(float metallic, float specular, float3 albedo) {
	float dielectric = 0.16 * specular * specular;
	return mix(float3(dielectric), albedo, float3(metallic));
}

// GGX Distribution
float D_GGX(float cos_theta_m, float alpha) {
	float alpha2 = alpha * alpha;
	float d = 1.0 + (alpha2 - 1.0) * cos_theta_m * cos_theta_m;
	return alpha2 / (M_PI * d * d);
}

// GGX Geometry (Schlick approximation)
float G_GGX_2cos(float cos_theta_m, float alpha) {
	float k = 0.5 * alpha;
	return 0.5 / (cos_theta_m * (1.0 - k) + k);
}

// Anisotropic GGX
float G_GGX_anisotropic_2cos(float cos_theta_m, float alpha_x, float alpha_y, float cos_phi, float sin_phi) {
	float cos2 = cos_theta_m * cos_theta_m;
	float sin2 = (1.0 - cos2);
	float s_x = alpha_x * cos_phi;
	float s_y = alpha_y * sin_phi;
	return 1.0 / max(cos_theta_m + sqrt(cos2 + (s_x * s_x + s_y * s_y) * sin2), 0.001);
}

float D_GGX_anisotropic(float cos_theta_m, float alpha_x, float alpha_y, float cos_phi, float sin_phi) {
	float cos2 = cos_theta_m * cos_theta_m;
	float sin2 = (1.0 - cos2);
	float r_x = cos_phi / alpha_x;
	float r_y = sin_phi / alpha_y;
	float d = cos2 + sin2 * (r_x * r_x + r_y * r_y);
	return 1.0 / max(M_PI * alpha_x * alpha_y * d * d, 0.001);
}

// GTR1 for clearcoat
float GTR1(float NdotH, float a) {
	if (a >= 1.0)
		return 1.0 / M_PI;
	float a2 = a * a;
	float t = 1.0 + (a2 - 1.0) * NdotH * NdotH;
	return (a2 - 1.0) / (M_PI * log(a2) * t);
}

// ============================================================================
// Light Compute Function
// ============================================================================

void light_compute(
	float3 N, float3 L, float3 V, float3 B, float3 T,
	float3 light_color, float3 attenuation, float3 diffuse_color,
	float3 transmission, float specular_blob_intensity,
	float roughness, float metallic, float specular,
	float rim, float rim_tint, float clearcoat, float clearcoat_gloss,
	float anisotropy, thread float3& diffuse_light, thread float3& specular_light, thread float& alpha
) {
	float NdotL = dot(N, L);
	float cNdotL = max(NdotL, 0.0);
	float NdotV = dot(N, V);
	float cNdotV = max(NdotV, 0.0);

	float3 H = normalize(V + L);
	float cNdotH = max(dot(N, H), 0.0);
	float cLdotH = max(dot(L, H), 0.0);

	if (metallic < 1.0) {
		// Diffuse BRDF (Burley)
		float FD90_minus_1 = 2.0 * cLdotH * cLdotH * roughness - 0.5;
		float FdV = 1.0 + FD90_minus_1 * SchlickFresnel(cNdotV);
		float FdL = 1.0 + FD90_minus_1 * SchlickFresnel(cNdotL);
		float diffuse_brdf_NL = (1.0 / M_PI) * FdV * FdL * cNdotL;

		diffuse_light += light_color * diffuse_color * diffuse_brdf_NL * attenuation;

#if defined(TRANSMISSION_USED)
		diffuse_light += light_color * diffuse_color * (float3(1.0 / M_PI) - diffuse_brdf_NL) * transmission * attenuation;
#endif

#if defined(LIGHT_USE_RIM)
		float rim_light = pow(max(0.0, 1.0 - cNdotV), max(0.0, (1.0 - roughness) * 16.0));
		diffuse_light += rim_light * rim * mix(float3(1.0), diffuse_color, rim_tint) * light_color;
#endif
	}

	if (roughness > 0.0) {
		// Specular BRDF (Schlick GGX)
#if defined(LIGHT_USE_ANISOTROPY)
		float alpha_ggx = roughness * roughness;
		float aspect = sqrt(1.0 - anisotropy * 0.9);
		float ax = alpha_ggx / aspect;
		float ay = alpha_ggx * aspect;
		float XdotH = dot(T, H);
		float YdotH = dot(B, H);
		float D = D_GGX_anisotropic(cNdotH, ax, ay, XdotH, YdotH);
		float G = G_GGX_anisotropic_2cos(cNdotL, ax, ay, XdotH, YdotH) * G_GGX_anisotropic_2cos(cNdotV, ax, ay, XdotH, YdotH);
#else
		float alpha_ggx = roughness * roughness;
		float D = D_GGX(cNdotH, alpha_ggx);
		float G = G_GGX_2cos(cNdotL, alpha_ggx) * G_GGX_2cos(cNdotV, alpha_ggx);
#endif
		// Fresnel
		float3 f0 = F0(metallic, specular, diffuse_color);
		float cLdotH5 = SchlickFresnel(cLdotH);
		float3 F = mix(float3(cLdotH5), float3(1.0), f0);

		float3 specular_brdf_NL = cNdotL * D * F * G;
		specular_light += specular_brdf_NL * light_color * specular_blob_intensity * attenuation;

#if defined(LIGHT_USE_CLEARCOAT)
		float Dr = GTR1(cNdotH, mix(0.1, 0.001, clearcoat_gloss));
		float Fr = mix(0.04, 1.0, cLdotH5);
		float Gr = G_GGX_2cos(cNdotL, 0.25) * G_GGX_2cos(cNdotV, 0.25);
		float clearcoat_specular_brdf_NL = 0.25 * clearcoat * Gr * Fr * Dr * cNdotL;
		specular_light += clearcoat_specular_brdf_NL * light_color * specular_blob_intensity * attenuation;
#endif
	}

#if defined(USE_SHADOW_TO_OPACITY)
	alpha = min(alpha, clamp(1.0 - length(attenuation), 0.0, 1.0));
#endif
}

// ============================================================================
// Shadow Sampling
// ============================================================================

#ifdef USE_SHADOW
float sample_shadow(depth2d<float> shadow, sampler shadowSampler, float2 shadow_pixel_size, float2 pos, float depth, float4 clamp_rect) {
#ifdef SHADOW_MODE_PCF_13
	float avg = shadow.sample_compare(shadowSampler, pos + float2(shadow_pixel_size.x * 2.0, 0.0), depth);
	avg += shadow.sample_compare(shadowSampler, pos + float2(-shadow_pixel_size.x * 2.0, 0.0), depth);
	avg += shadow.sample_compare(shadowSampler, pos + float2(0.0, shadow_pixel_size.y * 2.0), depth);
	avg += shadow.sample_compare(shadowSampler, pos + float2(0.0, -shadow_pixel_size.y * 2.0), depth);

	if (avg <= 0.000001) return 0.0;
	else if (avg >= 3.999999) return 1.0;

	avg += shadow.sample_compare(shadowSampler, pos, depth);
	avg += shadow.sample_compare(shadowSampler, pos + float2(shadow_pixel_size.x, 0.0), depth);
	avg += shadow.sample_compare(shadowSampler, pos + float2(-shadow_pixel_size.x, 0.0), depth);
	avg += shadow.sample_compare(shadowSampler, pos + float2(0.0, shadow_pixel_size.y), depth);
	avg += shadow.sample_compare(shadowSampler, pos + float2(0.0, -shadow_pixel_size.y), depth);
	avg += shadow.sample_compare(shadowSampler, pos + shadow_pixel_size, depth);
	avg += shadow.sample_compare(shadowSampler, pos + float2(-shadow_pixel_size.x, shadow_pixel_size.y), depth);
	avg += shadow.sample_compare(shadowSampler, pos + float2(shadow_pixel_size.x, -shadow_pixel_size.y), depth);
	avg += shadow.sample_compare(shadowSampler, pos - shadow_pixel_size, depth);
	return avg * (1.0 / 13.0);
#elif defined(SHADOW_MODE_PCF_5)
	float avg = shadow.sample_compare(shadowSampler, pos, depth);
	avg += shadow.sample_compare(shadowSampler, pos + float2(shadow_pixel_size.x, 0.0), depth);
	avg += shadow.sample_compare(shadowSampler, pos + float2(-shadow_pixel_size.x, 0.0), depth);
	avg += shadow.sample_compare(shadowSampler, pos + float2(0.0, shadow_pixel_size.y), depth);
	avg += shadow.sample_compare(shadowSampler, pos + float2(0.0, -shadow_pixel_size.y), depth);
	return avg * (1.0 / 5.0);
#else
	return shadow.sample_compare(shadowSampler, pos, depth);
#endif
}
#endif

// ============================================================================
// Radiance Map Sampling
// ============================================================================

#ifdef USE_RADIANCE_MAP
#ifdef USE_RADIANCE_MAP_ARRAY
float3 textureDualParaboloidArray(texture2d_array<float> p_tex, sampler smp, float3 p_vec, float p_roughness) {
	float3 norm = normalize(p_vec);
	norm.xy /= 1.0 + abs(norm.z);
	norm.xy = norm.xy * float2(0.5, 0.25) + float2(0.5, 0.25);

	float2 normg = norm.xy;
	if (norm.z > 0.0) {
		norm.y = 0.5 - norm.y + 0.5;
	}

	float index = p_roughness * RADIANCE_MAX_LOD;
	int indexi = int(index * 256.0);
	float3 base = p_tex.sample(smp, norm.xy, indexi / 256, gradient2d(dfdx(normg), dfdy(normg))).xyz;
	float3 next = p_tex.sample(smp, norm.xy, indexi / 256 + 1, gradient2d(dfdx(normg), dfdy(normg))).xyz;
	return mix(base, next, float(indexi % 256) / 256.0);
}
#else
float3 textureDualParaboloid(texture2d<float> p_tex, sampler smp, float3 p_vec, float p_roughness) {
	float3 norm = normalize(p_vec);
	norm.xy /= 1.0 + abs(norm.z);
	norm.xy = norm.xy * float2(0.5, 0.25) + float2(0.5, 0.25);
	if (norm.z > 0.0) {
		norm.y = 0.5 - norm.y + 0.5;
	}
	return p_tex.sample(smp, norm.xy, level(p_roughness * RADIANCE_MAX_LOD)).xyz;
}
#endif
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
// Fragment Shader
// ============================================================================

fragment SceneFragmentOut fragmentFunction(
	SceneVertexOut in [[stage_in]],
	bool is_front_facing [[front_facing]],
	constant SceneData& scene_data [[buffer(0)]],
	constant SceneUniforms& uniforms [[buffer(1)]]
#ifdef USE_LIGHT_DIRECTIONAL
	, constant DirectionalLightData& directional_light [[buffer(3)]]
#endif
#ifdef USE_RADIANCE_MAP
	, constant RadianceData& radiance_data [[buffer(4)]]
#ifdef USE_RADIANCE_MAP_ARRAY
	, texture2d_array<float> radiance_map_array [[texture(3)]]
	, sampler radianceSampler [[sampler(3)]]
#else
	, texture2d<float> radiance_map [[texture(2)]]
	, sampler radianceSampler [[sampler(2)]]
#endif
	, texture2d<float> irradiance_map [[texture(7)]]
	, sampler irradianceSampler [[sampler(7)]]
#endif
#ifdef USE_SHADOW
	, depth2d<float> directional_shadow [[texture(5)]]
	, sampler shadowSampler [[sampler(5)]]
	, depth2d<float> shadow_atlas [[texture(6)]]
#endif
	, texture2d<float> depth_buffer [[texture(9)]]
	, sampler depthSampler [[sampler(9)]]
#ifdef SCREEN_TEXTURE_USED
	, texture2d<float> screen_texture [[texture(8)]]
	, sampler screenSampler [[sampler(8)]]
#endif
#ifdef USE_LIGHTMAP
#ifdef USE_LIGHTMAP_LAYERED
	, texture2d_array<float> lightmap_array [[texture(11)]]
	, sampler lightmapSampler [[sampler(11)]]
#else
	, texture2d<float> lightmap [[texture(10)]]
	, sampler lightmapSampler [[sampler(10)]]
#endif
#endif
#if defined(USE_MATERIAL)
	, constant void* material_data [[buffer(2)]]
#endif
) {
	SceneFragmentOut out;

#ifdef RENDER_DEPTH_DUAL_PARABOLOID
	if (in.dp_clip > 0.0)
		discard_fragment();
#endif

	// Initialize material properties
	float3 vertex = in.vertex_interp;
	float3 view = -normalize(in.vertex_interp);
	float3 albedo = float3(1.0);
	float3 transmission = float3(0.0);
	float metallic = 0.0;
	float specular = 0.5;
	float3 emission = float3(0.0);
	float roughness = 1.0;
	float rim = 0.0;
	float rim_tint = 0.0;
	float clearcoat = 0.0;
	float clearcoat_gloss = 0.0;
	float anisotropy = 0.0;
	float2 anisotropy_flow = float2(1.0, 0.0);

#if defined(ENABLE_AO)
	float ao = 1.0;
	float ao_light_affect = 0.0;
#endif

	float alpha = 1.0;

#if defined(ALPHA_SCISSOR_USED)
	float alpha_scissor = 0.5;
#endif

#if defined(ENABLE_TANGENT_INTERP) || defined(ENABLE_NORMALMAP) || defined(LIGHT_USE_ANISOTROPY)
	float3 binormal = normalize(in.binormal_interp);
	float3 tangent = normalize(in.tangent_interp);
#else
	float3 binormal = float3(0.0);
	float3 tangent = float3(0.0);
#endif

	float3 normal = normalize(in.normal_interp);

#if defined(DO_SIDE_CHECK)
	if (!is_front_facing) {
		normal = -normal;
	}
#endif

#if defined(ENABLE_UV_INTERP)
	float2 uv = in.uv_interp;
#endif

#if defined(ENABLE_UV2_INTERP) || defined(USE_LIGHTMAP)
	float2 uv2 = in.uv2_interp;
#endif

#if defined(ENABLE_COLOR_INTERP)
	float4 color = in.color_interp;
#endif

#if defined(ENABLE_NORMALMAP)
	float3 normalmap = float3(0.5);
#endif

	float normaldepth = 1.0;

#if defined(SCREEN_UV_USED)
	float2 screen_uv = in.position.xy * scene_data.screen_pixel_size;
#endif

#if defined(ENABLE_SSS)
	float sss_strength = 0.0;
#endif

	{
		/* FRAGMENT_SHADER_CODE */
	}

#if !defined(USE_SHADOW_TO_OPACITY)
#if defined(ALPHA_SCISSOR_USED)
	if (alpha < alpha_scissor) {
		discard_fragment();
	}
#endif
#ifdef USE_OPAQUE_PREPASS
#if !defined(ALPHA_SCISSOR_USED)
	if (alpha < scene_data.opaque_prepass_threshold) {
		discard_fragment();
	}
#endif
#endif
#endif

#if defined(ENABLE_NORMALMAP)
	normalmap.xy = normalmap.xy * 2.0 - 1.0;
	normalmap.z = sqrt(max(0.0, 1.0 - dot(normalmap.xy, normalmap.xy)));
	normal = normalize(mix(normal, tangent * normalmap.x + binormal * normalmap.y + normal * normalmap.z, normaldepth));
#endif

#if defined(LIGHT_USE_ANISOTROPY)
	if (anisotropy > 0.01) {
		float3x3 rot = float3x3(tangent, binormal, normal);
		tangent = normalize(rot * float3(anisotropy_flow.x, anisotropy_flow.y, 0.0));
		binormal = normalize(rot * float3(-anisotropy_flow.y, anisotropy_flow.x, 0.0));
	}
#endif

	// Lighting
	float3 specular_light;
	float3 diffuse_light;

#ifdef USE_VERTEX_LIGHTING
	specular_light = in.specular_light_interp.rgb;
	diffuse_light = in.diffuse_light_interp.rgb;
#else
	specular_light = float3(0.0);
	diffuse_light = float3(0.0);
#endif

	float3 ambient_light;
	float3 env_reflection_light = float3(0.0);
	float3 eye_vec = view;

	// IBL precalculations
	float ndotv = clamp(dot(normal, eye_vec), 0.0, 1.0);
	float3 f0 = F0(metallic, specular, albedo);
	float3 F = f0 + (max(float3(1.0 - roughness), f0) - f0) * pow(1.0 - ndotv, 5.0);

#ifdef USE_RADIANCE_MAP
#if defined(AMBIENT_LIGHT_DISABLED)
	ambient_light = float3(0.0);
#else
	{
		float3 ref_vec = reflect(-eye_vec, normal);
		float horizon = min(1.0 + dot(ref_vec, normal), 1.0);
		ref_vec = normalize((radiance_data.radiance_inverse_xform * float4(ref_vec, 0.0)).xyz);
#ifdef USE_RADIANCE_MAP_ARRAY
		float3 radiance = textureDualParaboloidArray(radiance_map_array, radianceSampler, ref_vec, roughness) * scene_data.bg_energy;
#else
		float3 radiance = textureDualParaboloid(radiance_map, radianceSampler, ref_vec, roughness) * scene_data.bg_energy;
#endif
		env_reflection_light = radiance;
		env_reflection_light *= horizon * horizon;
	}

#ifndef USE_LIGHTMAP
	{
		float3 norm = normal;
		norm = normalize((radiance_data.radiance_inverse_xform * float4(norm, 0.0)).xyz);
		norm.xy /= 1.0 + abs(norm.z);
		norm.xy = norm.xy * float2(0.5, 0.25) + float2(0.5, 0.25);
		if (norm.z > 0.0001) {
			norm.y = 0.5 - norm.y + 0.5;
		}
		float3 env_ambient = irradiance_map.sample(irradianceSampler, norm.xy).rgb * scene_data.bg_energy;
		env_ambient *= 1.0 - F;
		ambient_light = mix(scene_data.ambient_light_color.rgb, env_ambient, radiance_data.radiance_ambient_contribution);
	}
#endif
#endif
#else
#if defined(AMBIENT_LIGHT_DISABLED)
	ambient_light = float3(0.0);
#else
	ambient_light = scene_data.ambient_light_color.rgb;
	env_reflection_light = scene_data.bg_color.rgb * scene_data.bg_energy;
#endif
#endif

	ambient_light *= scene_data.ambient_energy;

	float specular_blob_intensity = 1.0;

#if defined(SPECULAR_TOON)
	specular_blob_intensity *= specular * 2.0;
#endif

#ifdef USE_LIGHTMAP
#ifdef USE_LIGHTMAP_LAYERED
	ambient_light = lightmap_array.sample(lightmapSampler, float3(uv2, 0.0)).rgb * uniforms.lightmap_energy;
#else
	ambient_light = lightmap.sample(lightmapSampler, uv2).rgb * uniforms.lightmap_energy;
#endif
#endif

	// Forward lighting - reflection probes would be processed here
	specular_light += env_reflection_light;

	{
		// Environment BRDF approximation (Lazarov 2013)
		constant float4 c0 = float4(-1.0, -0.0275, -0.572, 0.022);
		constant float4 c1 = float4(1.0, 0.0425, 1.04, -0.04);
		float4 r = roughness * c0 + c1;
		float a004 = min(r.x * r.x, exp2(-9.28 * ndotv)) * r.x + r.y;
		float2 env = float2(-1.04, 1.04) * a004 + r.zw;
		specular_light *= env.x * F + env.y;
	}

#ifdef USE_LIGHT_DIRECTIONAL
	float3 light_attenuation = float3(1.0);

#ifdef LIGHT_DIRECTIONAL_SHADOW
#if !defined(SHADOWS_DISABLED)
	float depth_z = -vertex.z;
	// Shadow cascade selection would go here
	// Simplified: use first cascade
	float4 splane = (directional_light.shadow_matrix1 * float4(vertex, 1.0));
	float3 pssm_coord = splane.xyz / splane.w;

	float shadow = sample_shadow(directional_shadow, shadowSampler, scene_data.directional_shadow_pixel_size, pssm_coord.xy, pssm_coord.z, directional_light.light_clamp);

	float pssm_fade = smoothstep(directional_light.fade_from, -directional_light.shadow_split_offsets.w, vertex.z);
	light_attenuation = mix(mix(directional_light.shadow_color_contact.rgb, float3(1.0), shadow), float3(1.0), pssm_fade);
#endif
#endif

#ifndef USE_VERTEX_LIGHTING
	light_compute(normal, -directional_light.light_direction_attenuation.xyz, eye_vec, binormal, tangent,
				  directional_light.light_color_energy.rgb, light_attenuation, albedo, transmission,
				  directional_light.light_params.z * specular_blob_intensity, roughness, metallic, specular,
				  rim, rim_tint, clearcoat, clearcoat_gloss, anisotropy, diffuse_light, specular_light, alpha);
#endif
#endif

#ifdef USE_VERTEX_LIGHTING
	diffuse_light *= albedo;
#endif

	// Omni and spot light loops would go here for USE_FORWARD_LIGHTING

#if defined(USE_SHADOW_TO_OPACITY)
	alpha = min(alpha, clamp(length(ambient_light), 0.0, 1.0));
#if defined(ALPHA_SCISSOR_USED)
	if (alpha < alpha_scissor) {
		discard_fragment();
	}
#endif
#endif

#ifndef RENDER_DEPTH

	specular_light *= scene_data.reflection_multiplier;
	ambient_light *= albedo;

#if defined(ENABLE_AO)
	ambient_light *= ao;
	ao_light_affect = mix(1.0, ao, ao_light_affect);
	specular_light *= ao_light_affect;
	diffuse_light *= ao_light_affect;
#endif

	diffuse_light *= 1.0 - metallic;
	ambient_light *= 1.0 - metallic;

	// Fog
	if (scene_data.fog_color_enabled.a > 0.5) {
		float fog_amount = 0.0;
		float3 fog_color = scene_data.fog_color_enabled.rgb;

#ifdef USE_LIGHT_DIRECTIONAL
		fog_color = mix(fog_color, scene_data.fog_sun_color_amount.rgb,
						scene_data.fog_sun_color_amount.a * pow(max(dot(normalize(vertex), -directional_light.light_direction_attenuation.xyz), 0.0), 8.0));
#endif

		if (scene_data.fog_depth_enabled) {
			float fog_far = scene_data.fog_depth_end > 0.0 ? scene_data.fog_depth_end : scene_data.z_far;
			float fog_z = smoothstep(scene_data.fog_depth_begin, fog_far, length(vertex));
			fog_amount = pow(fog_z, scene_data.fog_depth_curve) * scene_data.fog_density;

			if (scene_data.fog_transmit_enabled) {
				float3 total_light = emission + ambient_light + specular_light + diffuse_light;
				float transmit = pow(fog_z, scene_data.fog_transmit_curve);
				fog_color = mix(max(total_light, fog_color), fog_color, transmit);
			}
		}

		if (scene_data.fog_height_enabled) {
			float y = (scene_data.camera_matrix * float4(vertex, 1.0)).y;
			fog_amount = max(fog_amount, pow(smoothstep(scene_data.fog_height_min, scene_data.fog_height_max, y), scene_data.fog_height_curve));
		}

		float rev_amount = 1.0 - fog_amount;
		emission = emission * rev_amount + fog_color * fog_amount;
		ambient_light *= rev_amount;
		specular_light *= rev_amount;
		diffuse_light *= rev_amount;
	}

#ifdef USE_MULTIPLE_RENDER_TARGETS
#ifdef SHADELESS
	out.frag_color = float4(albedo.rgb, 0.0);
	out.specular_buffer = float4(0.0);
#else
	float max_emission = max(emission.r, max(emission.g, emission.b));
	float max_ambient = max(ambient_light.r, max(ambient_light.g, ambient_light.b));
	float max_diffuse = max(diffuse_light.r, max(diffuse_light.g, diffuse_light.b));
	float total_ambient = max_ambient + max_diffuse;
#ifdef USE_FORWARD_LIGHTING
	total_ambient += max_emission;
#endif
	float ambient_scale = (total_ambient > 0.0) ? (max_ambient + scene_data.ambient_occlusion_affect_light * max_diffuse) / total_ambient : 0.0;

#if defined(ENABLE_AO)
	ambient_scale = mix(0.0, ambient_scale, scene_data.ambient_occlusion_affect_ao_channel);
#endif
	out.frag_color = float4(diffuse_light + ambient_light, ambient_scale);
	out.specular_buffer = float4(specular_light, metallic);

#ifdef USE_FORWARD_LIGHTING
	out.frag_color.rgb += emission;
#endif
#endif

	out.normal_mr_buffer = float4(normalize(normal) * 0.5 + 0.5, roughness);

#if defined(ENABLE_SSS)
	out.sss_buffer = sss_strength;
#endif

#else // Single render target

#ifdef SHADELESS
	out.frag_color = float4(albedo, alpha);
#else
	out.frag_color = float4(ambient_light + diffuse_light + specular_light, alpha);
#ifdef USE_FORWARD_LIGHTING
	out.frag_color.rgb += emission;
#endif
#endif

#endif // USE_MULTIPLE_RENDER_TARGETS

#endif // RENDER_DEPTH

	return out;
}
