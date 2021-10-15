/*************************************************************************/
/*  rasterizer_scene_metal.mm                                            */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifdef METAL_ENABLED

#include "rasterizer_metal.h"

#include "core/math/camera_matrix.h"
#include "core/os/os.h"
#include "core/self_list.h"
#include "scene/resources/mesh.h"
#include "servers/visual/rasterizer.h"
#include "servers/visual_server.h"

#import "shaders/_godot_common.h"
#import <Metal/Metal.h>

/* SHADOW ATLAS API */

RID RasterizerSceneMetal::shadow_atlas_create() {
	return RID();
}
void RasterizerSceneMetal::shadow_atlas_set_size(RID p_atlas, int p_size) {}
void RasterizerSceneMetal::shadow_atlas_set_quadrant_subdivision(RID p_atlas, int p_quadrant, int p_subdivision) {}
bool RasterizerSceneMetal::shadow_atlas_update_light(RID p_atlas, RID p_light_intance, float p_coverage, uint64_t p_light_version) {
	return false;
}

int RasterizerSceneMetal::get_directional_light_shadow_size(RID p_light_intance) {
	return 0;
}
void RasterizerSceneMetal::set_directional_shadow_count(int p_count) {}

/* ENVIRONMENT API */

RID environment_create() {
	return RID();
}

void RasterizerSceneMetal::environment_set_background(RID p_env, VS::EnvironmentBG p_bg) {}
void RasterizerSceneMetal::environment_set_sky(RID p_env, RID p_sky) {}
void RasterizerSceneMetal::environment_set_sky_custom_fov(RID p_env, float p_scale) {}
void RasterizerSceneMetal::environment_set_sky_orientation(RID p_env, const Basis &p_orientation) {}
void RasterizerSceneMetal::environment_set_bg_color(RID p_env, const Color &p_color) {}
void RasterizerSceneMetal::environment_set_bg_energy(RID p_env, float p_energy) {}
void RasterizerSceneMetal::environment_set_canvas_max_layer(RID p_env, int p_max_layer) {}
void RasterizerSceneMetal::environment_set_ambient_light(RID p_env, const Color &p_color, float p_energy, float p_sky_contribution) {}
void RasterizerSceneMetal::environment_set_camera_feed_id(RID p_env, int p_camera_feed_id) {}

void RasterizerSceneMetal::environment_set_dof_blur_near(RID p_env, bool p_enable, float p_distance, float p_transition, float p_far_amount, VS::EnvironmentDOFBlurQuality p_quality) {}
void RasterizerSceneMetal::environment_set_dof_blur_far(RID p_env, bool p_enable, float p_distance, float p_transition, float p_far_amount, VS::EnvironmentDOFBlurQuality p_quality) {}
void RasterizerSceneMetal::environment_set_glow(RID p_env, bool p_enable, int p_level_flags, float p_intensity, float p_strength, float p_bloom_threshold, VS::EnvironmentGlowBlendMode p_blend_mode, float p_hdr_bleed_threshold, float p_hdr_bleed_scale, float p_hdr_luminance_cap, bool p_bicubic_upscale, bool p_high_quality) {}

void RasterizerSceneMetal::environment_set_fog(RID p_env, bool p_enable, float p_begin, float p_end, RID p_gradient_texture) {}

void RasterizerSceneMetal::environment_set_ssr(RID p_env, bool p_enable, int p_max_steps, float p_fade_int, float p_fade_out, float p_depth_tolerance, bool p_roughness) {}
void RasterizerSceneMetal::environment_set_ssao(RID p_env, bool p_enable, float p_radius, float p_intensity, float p_radius2, float p_intensity2, float p_bias, float p_light_affect, float p_ao_channel_affect, const Color &p_color, VS::EnvironmentSSAOQuality p_quality, VS::EnvironmentSSAOBlur p_blur, float p_bilateral_sharpness) {}

void RasterizerSceneMetal::environment_set_tonemap(RID p_env, VS::EnvironmentToneMapper p_tone_mapper, float p_exposure, float p_white, bool p_auto_exposure, float p_min_luminance, float p_max_luminance, float p_auto_exp_speed, float p_auto_exp_scale) {}

void RasterizerSceneMetal::environment_set_adjustment(RID p_env, bool p_enable, float p_brightness, float p_contrast, float p_saturation, RID p_ramp) {}

void RasterizerSceneMetal::environment_set_fog(RID p_env, bool p_enable, const Color &p_color, const Color &p_sun_color, float p_sun_amount) {}
void RasterizerSceneMetal::environment_set_fog_depth(RID p_env, bool p_enable, float p_depth_begin, float p_depth_end, float p_depth_curve, bool p_transmit, float p_transmit_curve) {}
void RasterizerSceneMetal::environment_set_fog_height(RID p_env, bool p_enable, float p_min_height, float p_max_height, float p_height_curve) {}

bool RasterizerSceneMetal::is_environment(RID p_env) {
	return false;
}
VS::EnvironmentBG RasterizerSceneMetal::environment_get_background(RID p_env) {
	return VS::ENV_BG_KEEP;
}
int RasterizerSceneMetal::environment_get_canvas_max_layer(RID p_env) {
	return 0;
}

RID RasterizerSceneMetal::light_instance_create(RID p_light) {
	return RID();
}
void RasterizerSceneMetal::light_instance_set_transform(RID p_light_instance, const Transform &p_transform) {}
void RasterizerSceneMetal::light_instance_set_shadow_transform(RID p_light_instance, const CameraMatrix &p_projection, const Transform &p_transform, float p_far, float p_split, int p_pass, float p_bias_scale) {}
void RasterizerSceneMetal::light_instance_mark_visible(RID p_light_instance) {}

RID RasterizerSceneMetal::reflection_atlas_create() {
	return RID();
}
void RasterizerSceneMetal::reflection_atlas_set_size(RID p_ref_atlas, int p_size) {}
void RasterizerSceneMetal::reflection_atlas_set_subdivision(RID p_ref_atlas, int p_subdiv) {}

RID RasterizerSceneMetal::reflection_probe_instance_create(RID p_probe) {
	return RID();
}
void RasterizerSceneMetal::reflection_probe_instance_set_transform(RID p_instance, const Transform &p_transform) {}
void RasterizerSceneMetal::reflection_probe_release_atlas_index(RID p_instance) {}
bool reflection_probe_instance_needs_redraw(RID p_instance) {
	return false;
}
bool reflection_probe_instance_has_reflection(RID p_instance) {
	return false;
}
bool reflection_probe_instance_begin_render(RID p_instance, RID p_reflection_atlas) {
	return false;
}
bool reflection_probe_instance_postprocess_step(RID p_instance) {
	return true;
}

RID RasterizerSceneMetal::gi_probe_instance_create() {
	return RID();
}
void RasterizerSceneMetal::gi_probe_instance_set_light_data(RID p_probe, RID p_base, RID p_data) {}
void RasterizerSceneMetal::gi_probe_instance_set_transform_to_data(RID p_probe, const Transform &p_xform) {}
void RasterizerSceneMetal::gi_probe_instance_set_bounds(RID p_probe, const Vector3 &p_bounds) {}

void RasterizerSceneMetal::render_scene(const Transform &p_cam_transform, const CameraMatrix &p_cam_projection, const int p_eye, bool p_cam_ortogonal, InstanceBase **p_cull_result, int p_cull_count, RID *p_light_cull_result, int p_light_cull_count, RID *p_reflection_probe_cull_result, int p_reflection_probe_cull_count, RID p_environment, RID p_shadow_atlas, RID p_reflection_atlas, RID p_reflection_probe, int p_reflection_probe_pass) {}
void RasterizerSceneMetal::render_shadow(RID p_light, RID p_shadow_atlas, int p_pass, InstanceBase **p_cull_result, int p_cull_count) {}

void RasterizerSceneMetal::set_scene_pass(uint64_t p_pass) {}
void RasterizerSceneMetal::set_debug_draw_mode(VS::ViewportDebugDraw p_debug_draw) {}

bool RasterizerSceneMetal::free(RID p_rid) {
	return true;
}

RasterizerSceneMetal::RasterizerSceneMetal() {}
RasterizerSceneMetal::~RasterizerSceneMetal() {}

RID RasterizerCanvasMetal::light_internal_create() {
	return RID();
}
void RasterizerCanvasMetal::light_internal_update(RID p_rid, Light *p_light) {}
void RasterizerCanvasMetal::light_internal_free(RID p_rid) {}

void RasterizerCanvasMetal::canvas_begin() {}
void RasterizerCanvasMetal::canvas_end() {}

void RasterizerCanvasMetal::canvas_render_items(Item *p_item_list, int p_z, const Color &p_modulate, Light *p_light, const Transform2D &p_transform) {}
void RasterizerCanvasMetal::canvas_debug_viewport_shadows(Light *p_lights_with_shadow) {}

void RasterizerCanvasMetal::canvas_light_shadow_buffer_update(RID p_buffer, const Transform2D &p_light_xform, int p_light_mask, float p_near, float p_far, LightOccluderInstance *p_occluders, CameraMatrix *p_xform_cache) {}

void RasterizerCanvasMetal::reset_canvas() {}

void RasterizerCanvasMetal::draw_window_margins(int *p_margins, RID *p_margin_textures) {}

RasterizerCanvasMetal::RasterizerCanvasMetal() {}
RasterizerCanvasMetal::~RasterizerCanvasMetal() {}

#endif // METAL_ENABLED
