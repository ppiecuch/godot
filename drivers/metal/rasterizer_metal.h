/*************************************************************************/
/*  rasterizer_metal.h                                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef RASTERIZER_METAL_H
#define RASTERIZER_METAL_H

#include "servers/visual/rasterizer.h"
#include "servers/visual_server.h"

class RasterizerSceneMetal : public RasterizerScene {
public:
	/* SHADOW ATLAS API */

	RID shadow_atlas_create();
	void shadow_atlas_set_size(RID p_atlas, int p_size);
	void shadow_atlas_set_quadrant_subdivision(RID p_atlas, int p_quadrant, int p_subdivision);
	bool shadow_atlas_update_light(RID p_atlas, RID p_light_intance, float p_coverage, uint64_t p_light_version);

	int get_directional_light_shadow_size(RID p_light_intance);
	void set_directional_shadow_count(int p_count);

	/* ENVIRONMENT API */

	RID environment_create();

	void environment_set_background(RID p_env, VS::EnvironmentBG p_bg);
	void environment_set_sky(RID p_env, RID p_sky);
	void environment_set_sky_custom_fov(RID p_env, float p_scale);
	void environment_set_sky_orientation(RID p_env, const Basis &p_orientation);
	void environment_set_bg_color(RID p_env, const Color &p_color);
	void environment_set_bg_energy(RID p_env, float p_energy);
	void environment_set_canvas_max_layer(RID p_env, int p_max_layer);
	void environment_set_ambient_light(RID p_env, const Color &p_color, float p_energy = 1.0, float p_sky_contribution = 0.0);
	void environment_set_camera_feed_id(RID p_env, int p_camera_feed_id);

	void environment_set_dof_blur_near(RID p_env, bool p_enable, float p_distance, float p_transition, float p_far_amount, VS::EnvironmentDOFBlurQuality p_quality);
	void environment_set_dof_blur_far(RID p_env, bool p_enable, float p_distance, float p_transition, float p_far_amount, VS::EnvironmentDOFBlurQuality p_quality);
	void environment_set_glow(RID p_env, bool p_enable, int p_level_flags, float p_intensity, float p_strength, float p_bloom_threshold, VS::EnvironmentGlowBlendMode p_blend_mode, float p_hdr_bleed_threshold, float p_hdr_bleed_scale, float p_hdr_luminance_cap, bool p_bicubic_upscale, bool p_high_quality);

	void environment_set_fog(RID p_env, bool p_enable, float p_begin, float p_end, RID p_gradient_texture);

	void environment_set_ssr(RID p_env, bool p_enable, int p_max_steps, float p_fade_int, float p_fade_out, float p_depth_tolerance, bool p_roughness);
	void environment_set_ssao(RID p_env, bool p_enable, float p_radius, float p_intensity, float p_radius2, float p_intensity2, float p_bias, float p_light_affect, float p_ao_channel_affect, const Color &p_color, VS::EnvironmentSSAOQuality p_quality, VS::EnvironmentSSAOBlur p_blur, float p_bilateral_sharpness);

	void environment_set_tonemap(RID p_env, VS::EnvironmentToneMapper p_tone_mapper, float p_exposure, float p_white, bool p_auto_exposure, float p_min_luminance, float p_max_luminance, float p_auto_exp_speed, float p_auto_exp_scale);

	void environment_set_adjustment(RID p_env, bool p_enable, float p_brightness, float p_contrast, float p_saturation, RID p_ramp);

	void environment_set_fog(RID p_env, bool p_enable, const Color &p_color, const Color &p_sun_color, float p_sun_amount);
	void environment_set_fog_depth(RID p_env, bool p_enable, float p_depth_begin, float p_depth_end, float p_depth_curve, bool p_transmit, float p_transmit_curve);
	void environment_set_fog_height(RID p_env, bool p_enable, float p_min_height, float p_max_height, float p_height_curve);

	bool is_environment(RID p_env);
	VS::EnvironmentBG environment_get_background(RID p_env);
	int environment_get_canvas_max_layer(RID p_env);

	RID light_instance_create(RID p_light);
	void light_instance_set_transform(RID p_light_instance, const Transform &p_transform);
	void light_instance_set_shadow_transform(RID p_light_instance, const CameraMatrix &p_projection, const Transform &p_transform, float p_far, float p_split, int p_pass, float p_bias_scale = 1.0);
	void light_instance_mark_visible(RID p_light_instance);

	RID reflection_atlas_create();
	void reflection_atlas_set_size(RID p_ref_atlas, int p_size);
	void reflection_atlas_set_subdivision(RID p_ref_atlas, int p_subdiv);

	RID reflection_probe_instance_create(RID p_probe);
	void reflection_probe_instance_set_transform(RID p_instance, const Transform &p_transform);
	void reflection_probe_release_atlas_index(RID p_instance);
	bool reflection_probe_instance_needs_redraw(RID p_instance);
	bool reflection_probe_instance_has_reflection(RID p_instance);
	bool reflection_probe_instance_begin_render(RID p_instance, RID p_reflection_atlas);
	bool reflection_probe_instance_postprocess_step(RID p_instance);

	RID gi_probe_instance_create();
	void gi_probe_instance_set_light_data(RID p_probe, RID p_base, RID p_data);
	void gi_probe_instance_set_transform_to_data(RID p_probe, const Transform &p_xform);
	void gi_probe_instance_set_bounds(RID p_probe, const Vector3 &p_bounds);

	void render_scene(const Transform &p_cam_transform, const CameraMatrix &p_cam_projection, const int p_eye, bool p_cam_ortogonal, InstanceBase **p_cull_result, int p_cull_count, RID *p_light_cull_result, int p_light_cull_count, RID *p_reflection_probe_cull_result, int p_reflection_probe_cull_count, RID p_environment, RID p_shadow_atlas, RID p_reflection_atlas, RID p_reflection_probe, int p_reflection_probe_pass);
	void render_shadow(RID p_light, RID p_shadow_atlas, int p_pass, InstanceBase **p_cull_result, int p_cull_count);

	void set_scene_pass(uint64_t p_pass);
	void set_debug_draw_mode(VS::ViewportDebugDraw p_debug_draw);

	bool free(RID p_rid);

	RasterizerSceneMetal();
	~RasterizerSceneMetal();
};

class RasterizerStorageMetal : public RasterizerStorage {
public:
	/* TEXTURE API */
	struct MetalTexture;
	struct MetalSurface;
	struct MetalMesh;

	mutable RID_Owner<MetalTexture> texture_owner;
	mutable RID_Owner<MetalMesh> mesh_owner;

	RID texture_create();

	void texture_allocate(RID p_texture, int p_width, int p_height, int p_depth_3d, Image::Format p_format, VisualServer::TextureType p_type = VS::TEXTURE_TYPE_2D, uint32_t p_flags = VS::TEXTURE_FLAGS_DEFAULT);
	void texture_set_data(RID p_texture, const Ref<Image> &p_image, int p_layer);

	void texture_set_data_partial(RID p_texture, const Ref<Image> &p_image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int p_dst_mip, int p_layer);

	Ref<Image> texture_get_data(RID p_texture, int p_layer) const;
	void texture_set_flags(RID p_texture, uint32_t p_flags);
	uint32_t texture_get_flags(RID p_texture) const;
	Image::Format texture_get_format(RID p_texture) const;

	VisualServer::TextureType texture_get_type(RID p_texture) const;
	uint32_t texture_get_texid(RID p_texture) const;
	uint32_t texture_get_width(RID p_texture) const;
	uint32_t texture_get_height(RID p_texture) const;
	uint32_t texture_get_depth(RID p_texture) const;
	void texture_set_size_override(RID p_texture, int p_width, int p_height, int p_depth_3d);
	void texture_bind(RID p_texture, uint32_t p_texture_no);

	void texture_set_path(RID p_texture, const String &p_path);
	String texture_get_path(RID p_texture) const;

	void texture_set_shrink_all_x2_on_set_data(bool p_enable);

	void texture_debug_usage(List<VS::TextureInfo> *r_info);

	RID texture_create_radiance_cubemap(RID p_source, int p_resolution = -1) const;

	void texture_set_detect_3d_callback(RID p_texture, VisualServer::TextureDetectCallback p_callback, void *p_userdata);
	void texture_set_detect_srgb_callback(RID p_texture, VisualServer::TextureDetectCallback p_callback, void *p_userdata);
	void texture_set_detect_normal_callback(RID p_texture, VisualServer::TextureDetectCallback p_callback, void *p_userdata);

	void textures_keep_original(bool p_enable);

	void texture_set_proxy(RID p_proxy, RID p_base);
	virtual Size2 texture_size_with_proxy(RID p_texture) const;
	void texture_set_force_redraw_if_visible(RID p_texture, bool p_enable);

	/* SKY API */

	RID sky_create();
	void sky_set_texture(RID p_sky, RID p_cube_map, int p_radiance_size);

	/* SHADER API */

	RID shader_create();

	void shader_set_code(RID p_shader, const String &p_code);
	String shader_get_code(RID p_shader) const;
	void shader_get_param_list(RID p_shader, List<PropertyInfo> *p_param_list) const;

	void shader_set_default_texture_param(RID p_shader, const StringName &p_name, RID p_texture);
	RID shader_get_default_texture_param(RID p_shader, const StringName &p_name) const;

	void shader_add_custom_define(RID p_shader, const String &p_define);
	void shader_get_custom_defines(RID p_shader, Vector<String> *p_defines) const;
	void shader_remove_custom_define(RID p_shader, const String &p_define);

	void set_shader_async_hidden_forbidden(bool p_forbidden);
	bool is_shader_async_hidden_forbidden();

	/* COMMON MATERIAL API */

	RID material_create();

	void material_set_render_priority(RID p_material, int priority);
	void material_set_shader(RID p_shader_material, RID p_shader);
	RID material_get_shader(RID p_shader_material) const;

	void material_set_param(RID p_material, const StringName &p_param, const Variant &p_value);
	Variant material_get_param(RID p_material, const StringName &p_param) const;
	Variant material_get_param_default(RID p_material, const StringName &p_param) const;

	void material_set_line_width(RID p_material, float p_width);

	void material_set_next_pass(RID p_material, RID p_next_material);

	bool material_is_animated(RID p_material);
	bool material_casts_shadows(RID p_material);

	void material_add_instance_owner(RID p_material, RasterizerScene::InstanceBase *p_instance);
	void material_remove_instance_owner(RID p_material, RasterizerScene::InstanceBase *p_instance);

	/* MESH API */

	RID mesh_create();

	void mesh_add_surface(RID p_mesh, uint32_t p_format, VS::PrimitiveType p_primitive, const PoolVector<uint8_t> &p_array, int p_vertex_count, const PoolVector<uint8_t> &p_index_array, int p_index_count, const AABB &p_aabb, const Vector<PoolVector<uint8_t>> &p_blend_shapes = Vector<PoolVector<uint8_t>>(), const Vector<AABB> &p_bone_aabbs = Vector<AABB>());

	void mesh_set_blend_shape_count(RID p_mesh, int p_amount);
	int mesh_get_blend_shape_count(RID p_mesh) const;

	void mesh_set_blend_shape_mode(RID p_mesh, VS::BlendShapeMode p_mode);
	VS::BlendShapeMode mesh_get_blend_shape_mode(RID p_mesh) const;

	void mesh_set_blend_shape_values(RID p_mesh, PoolVector<float> p_values);
	PoolVector<float> mesh_get_blend_shape_values(RID p_mesh) const;

	void mesh_surface_update_region(RID p_mesh, int p_surface, int p_offset, const PoolVector<uint8_t> &p_data);

	void mesh_surface_set_material(RID p_mesh, int p_surface, RID p_material);
	RID mesh_surface_get_material(RID p_mesh, int p_surface) const;

	int mesh_surface_get_array_len(RID p_mesh, int p_surface) const;
	int mesh_surface_get_array_index_len(RID p_mesh, int p_surface) const;

	PoolVector<uint8_t> mesh_surface_get_array(RID p_mesh, int p_surface) const;
	PoolVector<uint8_t> mesh_surface_get_index_array(RID p_mesh, int p_surface) const;

	uint32_t mesh_surface_get_format(RID p_mesh, int p_surface) const;
	VS::PrimitiveType mesh_surface_get_primitive_type(RID p_mesh, int p_surface) const;

	AABB mesh_surface_get_aabb(RID p_mesh, int p_surface) const;
	Vector<PoolVector<uint8_t>> mesh_surface_get_blend_shapes(RID p_mesh, int p_surface) const;
	Vector<AABB> mesh_surface_get_skeleton_aabb(RID p_mesh, int p_surface) const;

	void mesh_remove_surface(RID p_mesh, int p_index);
	int mesh_get_surface_count(RID p_mesh) const;

	void mesh_surface_set_active(RID p_mesh, int p_surface, bool p_active);

	void mesh_set_custom_aabb(RID p_mesh, const AABB &p_aabb);
	AABB mesh_get_custom_aabb(RID p_mesh) const;

	AABB mesh_get_aabb(RID p_mesh, RID p_skeleton) const;
	void mesh_clear(RID p_mesh);

	/* MULTIMESH API */

	virtual RID multimesh_create();

	void multimesh_allocate(RID p_multimesh, int p_instances, VS::MultimeshTransformFormat p_transform_format, VS::MultimeshColorFormat p_color_format, VS::MultimeshCustomDataFormat p_data = VS::MULTIMESH_CUSTOM_DATA_NONE);
	int multimesh_get_instance_count(RID p_multimesh) const;

	void multimesh_set_mesh(RID p_multimesh, RID p_mesh);
	void multimesh_instance_set_transform(RID p_multimesh, int p_index, const Transform &p_transform);
	void multimesh_instance_set_transform_2d(RID p_multimesh, int p_index, const Transform2D &p_transform);
	void multimesh_instance_set_color(RID p_multimesh, int p_index, const Color &p_color);
	void multimesh_instance_set_custom_data(RID p_multimesh, int p_index, const Color &p_color);

	RID multimesh_get_mesh(RID p_multimesh) const;

	Transform multimesh_instance_get_transform(RID p_multimesh, int p_index) const;
	Transform2D multimesh_instance_get_transform_2d(RID p_multimesh, int p_index) const;
	Color multimesh_instance_get_color(RID p_multimesh, int p_index) const;
	Color multimesh_instance_get_custom_data(RID p_multimesh, int p_index) const;

	void multimesh_set_as_bulk_array(RID p_multimesh, const PoolVector<float> &p_array);

	void multimesh_set_visible_instances(RID p_multimesh, int p_visible);
	int multimesh_get_visible_instances(RID p_multimesh) const;

	AABB multimesh_get_aabb(RID p_multimesh) const;

	/* IMMEDIATE API */

	RID immediate_create();
	void immediate_begin(RID p_immediate, VS::PrimitiveType p_rimitive, RID p_texture = RID());
	void immediate_vertex(RID p_immediate, const Vector3 &p_vertex);
	void immediate_normal(RID p_immediate, const Vector3 &p_normal);
	void immediate_tangent(RID p_immediate, const Plane &p_tangent);
	void immediate_color(RID p_immediate, const Color &p_color);
	void immediate_uv(RID p_immediate, const Vector2 &tex_uv);
	void immediate_uv2(RID p_immediate, const Vector2 &tex_uv);
	void immediate_end(RID p_immediate);
	void immediate_clear(RID p_immediate);
	void immediate_set_material(RID p_immediate, RID p_material);
	RID immediate_get_material(RID p_immediate) const;
	AABB immediate_get_aabb(RID p_immediate) const;

	/* SKELETON API */

	RID skeleton_create();
	void skeleton_allocate(RID p_skeleton, int p_bones, bool p_2d_skeleton = false);
	void skeleton_set_base_transform_2d(RID p_skeleton, const Transform2D &p_base_transform);
	void skeleton_set_world_transform(RID p_skeleton, bool p_enable, const Transform &p_world_transform);
	int skeleton_get_bone_count(RID p_skeleton) const;
	void skeleton_bone_set_transform(RID p_skeleton, int p_bone, const Transform &p_transform);
	Transform skeleton_bone_get_transform(RID p_skeleton, int p_bone) const;
	void skeleton_bone_set_transform_2d(RID p_skeleton, int p_bone, const Transform2D &p_transform);
	Transform2D skeleton_bone_get_transform_2d(RID p_skeleton, int p_bone) const;

	/* Light API */

	RID light_create(VS::LightType p_type);

	RID directional_light_create();
	RID omni_light_create();
	RID spot_light_create();

	void light_set_color(RID p_light, const Color &p_color);
	void light_set_param(RID p_light, VS::LightParam p_param, float p_value);
	void light_set_shadow(RID p_light, bool p_enabled);
	void light_set_shadow_color(RID p_light, const Color &p_color);
	void light_set_projector(RID p_light, RID p_texture);
	void light_set_negative(RID p_light, bool p_enable);
	void light_set_cull_mask(RID p_light, uint32_t p_mask);
	void light_set_reverse_cull_face_mode(RID p_light, bool p_enabled);
	void light_set_use_gi(RID p_light, bool p_enabled);
	void light_set_bake_mode(RID p_light, VS::LightBakeMode p_bake_mode);

	void light_omni_set_shadow_mode(RID p_light, VS::LightOmniShadowMode p_mode);
	void light_omni_set_shadow_detail(RID p_light, VS::LightOmniShadowDetail p_detail);

	void light_directional_set_shadow_mode(RID p_light, VS::LightDirectionalShadowMode p_mode);
	void light_directional_set_blend_splits(RID p_light, bool p_enable);
	bool light_directional_get_blend_splits(RID p_light) const;
	void light_directional_set_shadow_depth_range_mode(RID p_light, VS::LightDirectionalShadowDepthRangeMode p_range_mode);
	VS::LightDirectionalShadowDepthRangeMode light_directional_get_shadow_depth_range_mode(RID p_light) const;

	VS::LightDirectionalShadowMode light_directional_get_shadow_mode(RID p_light);
	VS::LightOmniShadowMode light_omni_get_shadow_mode(RID p_light);

	bool light_has_shadow(RID p_light) const;

	VS::LightType light_get_type(RID p_light) const;
	AABB light_get_aabb(RID p_light) const;
	float light_get_param(RID p_light, VS::LightParam p_param);
	Color light_get_color(RID p_light);
	bool light_get_use_gi(RID p_light);
	VS::LightBakeMode light_get_bake_mode(RID p_light);
	uint64_t light_get_version(RID p_light) const;

	/* PROBE API */

	RID reflection_probe_create();

	void reflection_probe_set_update_mode(RID p_probe, VS::ReflectionProbeUpdateMode p_mode);
	void reflection_probe_set_intensity(RID p_probe, float p_intensity);
	void reflection_probe_set_interior_ambient(RID p_probe, const Color &p_ambient);
	void reflection_probe_set_interior_ambient_energy(RID p_probe, float p_energy);
	void reflection_probe_set_interior_ambient_probe_contribution(RID p_probe, float p_contrib);
	void reflection_probe_set_max_distance(RID p_probe, float p_distance);
	void reflection_probe_set_extents(RID p_probe, const Vector3 &p_extents);
	void reflection_probe_set_origin_offset(RID p_probe, const Vector3 &p_offset);
	void reflection_probe_set_as_interior(RID p_probe, bool p_enable);
	void reflection_probe_set_enable_box_projection(RID p_probe, bool p_enable);
	void reflection_probe_set_enable_shadows(RID p_probe, bool p_enable);
	void reflection_probe_set_cull_mask(RID p_probe, uint32_t p_layers);
	void reflection_probe_set_resolution(RID p_probe, int p_resolution);

	AABB reflection_probe_get_aabb(RID p_probe) const;
	VS::ReflectionProbeUpdateMode reflection_probe_get_update_mode(RID p_probe) const;
	uint32_t reflection_probe_get_cull_mask(RID p_probe) const;
	Vector3 reflection_probe_get_extents(RID p_probe) const;
	Vector3 reflection_probe_get_origin_offset(RID p_probe) const;
	float reflection_probe_get_origin_max_distance(RID p_probe) const;
	bool reflection_probe_renders_shadows(RID p_probe) const;

	void instance_add_skeleton(RID p_skeleton, RasterizerScene::InstanceBase *p_instance);
	void instance_remove_skeleton(RID p_skeleton, RasterizerScene::InstanceBase *p_instance);

	void instance_add_dependency(RID p_base, RasterizerScene::InstanceBase *p_instance);
	void instance_remove_dependency(RID p_base, RasterizerScene::InstanceBase *p_instance);

	/* GI PROBE API */

	RID gi_probe_create();

	void gi_probe_set_bounds(RID p_probe, const AABB &p_bounds);
	AABB gi_probe_get_bounds(RID p_probe) const;

	void gi_probe_set_cell_size(RID p_probe, float p_range);
	float gi_probe_get_cell_size(RID p_probe) const;

	void gi_probe_set_to_cell_xform(RID p_probe, const Transform &p_xform);
	Transform gi_probe_get_to_cell_xform(RID p_probe) const;

	void gi_probe_set_dynamic_data(RID p_probe, const PoolVector<int> &p_data);
	PoolVector<int> gi_probe_get_dynamic_data(RID p_probe) const;

	void gi_probe_set_dynamic_range(RID p_probe, int p_range);
	int gi_probe_get_dynamic_range(RID p_probe) const;

	void gi_probe_set_energy(RID p_probe, float p_range);
	float gi_probe_get_energy(RID p_probe) const;

	void gi_probe_set_bias(RID p_probe, float p_range);
	float gi_probe_get_bias(RID p_probe) const;

	void gi_probe_set_normal_bias(RID p_probe, float p_range);
	float gi_probe_get_normal_bias(RID p_probe) const;

	void gi_probe_set_propagation(RID p_probe, float p_range);
	float gi_probe_get_propagation(RID p_probe) const;

	void gi_probe_set_interior(RID p_probe, bool p_enable);
	bool gi_probe_is_interior(RID p_probe) const;

	void gi_probe_set_compress(RID p_probe, bool p_enable);
	bool gi_probe_is_compressed(RID p_probe) const;

	uint32_t gi_probe_get_version(RID p_probe);

	GIProbeCompression gi_probe_get_dynamic_data_get_preferred_compression() const;
	RID gi_probe_dynamic_data_create(int p_width, int p_height, int p_depth, GIProbeCompression p_compression);
	void gi_probe_dynamic_data_update(RID p_gi_probe_data, int p_depth_slice, int p_slice_count, int p_mipmap, const void *p_data);

	/* LIGHTMAP CAPTURE */
	struct Instantiable : public RID_Data {
		SelfList<RasterizerScene::InstanceBase>::List instance_list;

		_FORCE_INLINE_ void instance_change_notify(bool p_aabb = true, bool p_materials = true) {
			SelfList<RasterizerScene::InstanceBase> *instances = instance_list.first();
			while (instances) {
				instances->self()->base_changed(p_aabb, p_materials);
				instances = instances->next();
			}
		}

		_FORCE_INLINE_ void instance_remove_deps() {
			SelfList<RasterizerScene::InstanceBase> *instances = instance_list.first();
			while (instances) {
				SelfList<RasterizerScene::InstanceBase> *next = instances->next();
				instances->self()->base_removed();
				instances = next;
			}
		}

		Instantiable();
		virtual ~Instantiable() {
		}
	};

	struct LightmapCapture : public Instantiable {
		PoolVector<LightmapCaptureOctree> octree;
		AABB bounds;
		Transform cell_xform;
		int cell_subdiv;
		float energy;
		LightmapCapture() {
			energy = 1.0;
			cell_subdiv = 1;
		}
	};

	mutable RID_Owner<LightmapCapture> lightmap_capture_data_owner;
	void lightmap_capture_set_bounds(RID p_capture, const AABB &p_bounds);
	AABB lightmap_capture_get_bounds(RID p_capture) const;
	void lightmap_capture_set_octree(RID p_capture, const PoolVector<uint8_t> &p_octree);
	RID lightmap_capture_create();
	PoolVector<uint8_t> lightmap_capture_get_octree(RID p_capture) const;
	void lightmap_capture_set_octree_cell_transform(RID p_capture, const Transform &p_xform);
	Transform lightmap_capture_get_octree_cell_transform(RID p_capture) const;
	void lightmap_capture_set_octree_cell_subdiv(RID p_capture, int p_subdiv);
	int lightmap_capture_get_octree_cell_subdiv(RID p_capture) const;
	void lightmap_capture_set_energy(RID p_capture, float p_energy);
	float lightmap_capture_get_energy(RID p_capture) const;
	void lightmap_capture_set_interior(RID p_capture, bool p_interior);
	bool lightmap_capture_is_interior(RID p_capture) const;
	const PoolVector<LightmapCaptureOctree> *lightmap_capture_get_octree_ptr(RID p_capture) const;

	/* PARTICLES */

	RID particles_create();

	void particles_set_emitting(RID p_particles, bool p_emitting);
	void particles_set_amount(RID p_particles, int p_amount);
	void particles_set_lifetime(RID p_particles, float p_lifetime);
	void particles_set_one_shot(RID p_particles, bool p_one_shot);
	void particles_set_pre_process_time(RID p_particles, float p_time);
	void particles_set_explosiveness_ratio(RID p_particles, float p_ratio);
	void particles_set_randomness_ratio(RID p_particles, float p_ratio);
	void particles_set_custom_aabb(RID p_particles, const AABB &p_aabb);
	void particles_set_speed_scale(RID p_particles, float p_scale);
	void particles_set_use_local_coordinates(RID p_particles, bool p_enable);
	void particles_set_process_material(RID p_particles, RID p_material);
	void particles_set_fixed_fps(RID p_particles, int p_fps);
	void particles_set_fractional_delta(RID p_particles, bool p_enable);
	void particles_restart(RID p_particles);

	void particles_set_draw_order(RID p_particles, VS::ParticlesDrawOrder p_order);

	void particles_set_draw_passes(RID p_particles, int p_count);
	void particles_set_draw_pass_mesh(RID p_particles, int p_pass, RID p_mesh);

	void particles_request_process(RID p_particles);
	AABB particles_get_current_aabb(RID p_particles);
	AABB particles_get_aabb(RID p_particles) const;

	void particles_set_emission_transform(RID p_particles, const Transform &p_transform);

	bool particles_get_emitting(RID p_particles);
	int particles_get_draw_passes(RID p_particles) const;
	RID particles_get_draw_pass_mesh(RID p_particles, int p_pass) const;

	virtual bool particles_is_inactive(RID p_particles) const;

	/* RENDER TARGET */

	RID render_target_create();
	void render_target_set_position(RID p_render_target, int p_x, int p_y);
	void render_target_set_size(RID p_render_target, int p_width, int p_height);
	RID render_target_get_texture(RID p_render_target) const;
	uint32_t render_target_get_depth_texture_id(RID p_render_target) const;
	void render_target_set_external_texture(RID p_render_target, unsigned int p_texture_id, unsigned int p_depth_id);

	void render_target_set_flag(RID p_render_target, RenderTargetFlags p_flag, bool p_value);
	bool render_target_was_used(RID p_render_target);
	void render_target_clear_used(RID p_render_target);
	void render_target_set_msaa(RID p_render_target, VS::ViewportMSAA p_msaa);
	void render_target_set_use_fxaa(RID p_render_target, bool p_fxaa);
	void render_target_set_use_debanding(RID p_render_target, bool p_debanding);
	void render_target_set_sharpen_intensity(RID p_render_target, float p_intensity);

	/* CANVAS SHADOW */

	RID canvas_light_shadow_buffer_create(int p_width);

	/* LIGHT SHADOW MAPPING */

	RID canvas_light_occluder_create();
	void canvas_light_occluder_set_polylines(RID p_occluder, const PoolVector<Vector2> &p_lines);

	VS::InstanceType get_base_type(RID p_rid) const;

	bool free(RID p_rid);

	bool has_os_feature(const String &p_feature) const;

	void update_dirty_resources();

	void set_debug_generate_wireframes(bool p_generate);

	void render_info_begin_capture();
	void render_info_end_capture();
	int get_captured_render_info(VS::RenderInfo p_info);

	uint64_t get_render_info(VS::RenderInfo p_info);
	String get_video_adapter_name() const;
	String get_video_adapter_vendor() const;

	RasterizerStorageMetal();
	~RasterizerStorageMetal();
};

class RasterizerCanvasMetal : public RasterizerCanvas {
public:
	RID light_internal_create();
	void light_internal_update(RID p_rid, Light *p_light);
	void light_internal_free(RID p_rid);

	void canvas_begin();
	void canvas_end();

	void canvas_render_items(Item *p_item_list, int p_z, const Color &p_modulate, Light *p_light, const Transform2D &p_transform);
	void canvas_debug_viewport_shadows(Light *p_lights_with_shadow);

	void canvas_light_shadow_buffer_update(RID p_buffer, const Transform2D &p_light_xform, int p_light_mask, float p_near, float p_far, LightOccluderInstance *p_occluders, CameraMatrix *p_xform_cache);

	void reset_canvas();

	void draw_window_margins(int *p_margins, RID *p_margin_textures);

	RasterizerCanvasMetal();
	~RasterizerCanvasMetal();
};

class RasterizerMetal : public Rasterizer {
protected:
	RasterizerCanvasMetal *canvas;
	RasterizerStorageMetal *storage;
	RasterizerSceneMetal *scene;

public:
	RasterizerStorage *get_storage();
	RasterizerCanvas *get_canvas();
	RasterizerScene *get_scene();

	void set_boot_image(const Ref<Image> &p_image, const Color &p_color, bool p_scale, bool p_use_filter = true);
	void set_shader_time_scale(float p_scale);

	void initialize();
	void begin_frame(double frame_step);
	void set_current_render_target(RID p_render_target);
	void restore_render_target(bool p_3d_was_drawn);
	void clear_render_target(const Color &p_color);
	void blit_render_target_to_screen(RID p_render_target, const Rect2 &p_screen_rect, int p_screen = 0);
	void output_lens_distorted_to_screen(RID p_render_target, const Rect2 &p_screen_rect, float p_k1, float p_k2, const Vector2 &p_eye_center, float p_oversample);
	void end_frame(bool p_swap_buffers);
	void finalize();

	Ref<Image> read_pixels(const Rect2 &p_region);

	static Error is_viable();

	static Rasterizer *_create_current();
	static void make_current();

	bool is_low_end() const;

	const char *gl_check_for_error(bool p_print_error = true);

	static void *driverdata;

	RasterizerMetal();
	~RasterizerMetal();
};

#endif // RASTERIZER_METAL_H
