/*************************************************************************/
/*  glm_params.h                                                         */
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

/*
 * Copyright (C) Michael Larson on 1/6/2022
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * glm_params.h
 * MGL
 *
 */

#ifndef glm_params_h
#define glm_params_h

#include "GL/glcorearb.h"

#include "glm_limits.h"

typedef struct GLMHints_t {
	GLint line_smooth_hint;
	GLint polygon_smooth_hint;
	GLint texture_compression_hint;
	GLint fragment_shader_derivative_hint;
} GLMHints;

typedef struct GLMCaps_t {
	GLboolean blend;
	GLboolean multisample;
	GLboolean sample_alpha_to_coverage;
	GLboolean sample_alpha_to_one;
	GLboolean sample_coverage;
	GLboolean rasterizer_discard;
	GLboolean framebuffer_srgb;
	GLboolean depth_clamp;
	GLboolean texture_cube_map_seamless;
	GLboolean sample_mask;
	GLboolean sample_shading;
	GLboolean debug_output_synchronous;
	GLboolean debug_output;
	GLboolean line_smooth;
	GLboolean polygon_smooth;
	GLboolean cull_face;
	GLboolean depth_test;
	GLboolean stencil_test;
	GLboolean dither;
	GLboolean scissor_test;
	GLboolean point_smooth;
	GLboolean line_stipple;
	GLboolean polygon_stipple;
	GLboolean lighting;
	GLboolean color_material;
	GLboolean fog;
	GLboolean normalize;
	GLboolean alpha_test;
	GLboolean texture_gen_s;
	GLboolean texture_gen_t;
	GLboolean texture_gen_r;
	GLboolean texture_gen_q;
	GLboolean auto_normal;
	GLboolean color_logic_op;
	GLboolean polygon_offset_point;
	GLboolean polygon_offset_line;
	GLboolean polygon_offset_fill;
	GLboolean index_logic_op;
	GLboolean normal_array;
	GLboolean color_array;
	GLboolean index_array;
	GLboolean texture_coord_array;
	GLboolean edge_flag_array;
	GLboolean program_point_size;
	GLboolean primitive_restart;
	GLboolean primitive_restart_fixed_index;

	GLboolean clip_distances[MAX_CLIP_DISTANCES];

	// local enables
	GLboolean use_color_mask[MAX_COLOR_ATTACHMENTS];
} GLMCaps;

typedef struct GLMParams_t {
	GLfloat point_size;
	GLint point_size_range;
	GLint point_size_granularity;
	GLfloat line_width;
	GLint line_width_range;
	GLint line_width_granularity;
	GLint polygon_mode;
	GLint cull_face_mode;
	GLint front_face;
	GLdouble depth_range[2];
	GLboolean depth_writemask;
	GLdouble depth_clear_value;
	GLint depth_func;
	GLint stencil_clear_value;
	GLint stencil_func;
	GLint stencil_value_mask;
	GLint stencil_fail;
	GLint stencil_pass_depth_fail;
	GLint stencil_pass_depth_pass;
	GLint stencil_ref;
	GLint stencil_writemask;
	GLint viewport[4];
	GLint blend_dst;
	GLint blend_src;
	GLint logic_op_mode;
	GLint draw_buffer;
	GLint read_buffer;
	GLint scissor_box[4];
	GLint color_clear_value;
	GLboolean color_writemask[MAX_COLOR_ATTACHMENTS][4];
	GLint max_texture_size;
	GLint max_viewport_dims;
	GLint subpixel_bits;
	GLuint current_color;
	GLint current_index;
	GLuint current_normal;
	GLuint current_raster_color;
	GLint current_raster_index;
	GLint current_raster_texture_coords;
	GLint current_raster_position;
	GLint current_raster_position_valid;
	GLint current_raster_distance;
	GLuint line_stipple_pattern;
	GLint line_stipple_repeat;
	GLint list_mode;
	GLint max_list_nesting;
	GLint list_base;
	GLint list_index;
	GLint edge_flag;
	GLint shade_model;
	GLint color_material_face;
	GLint color_material_parameter;
	GLint accum_clear_value;
	GLint matrix_mode;
	GLint modelview_stack_depth;
	GLint projection_stack_depth;
	GLint texture_stack_depth;
	GLint modelview_matrix;
	GLint projection_matrix;
	GLint attrib_stack_depth;
	GLint alpha_test_func;
	GLint alpha_test_ref;
	GLint logic_op;
	GLint aux_buffers;
	GLint index_clear_value;
	GLint index_writemask;
	GLint index_mode;
	GLint rgba_mode;
	GLint render_mode;
	GLint pixel_map_i_to_i_size;
	GLint pixel_map_s_to_s_size;
	GLint pixel_map_i_to_r_size;
	GLint pixel_map_i_to_g_size;
	GLint pixel_map_i_to_b_size;
	GLint pixel_map_i_to_a_size;
	GLint pixel_map_r_to_r_size;
	GLint pixel_map_g_to_g_size;
	GLint pixel_map_b_to_b_size;
	GLint pixel_map_a_to_a_size;
	GLint zoom_x;
	GLint zoom_y;
	GLint max_eval_order;
	GLint max_lights;
	GLint max_clip_planes;
	GLint max_pixel_map_table;
	GLint max_attrib_stack_depth;
	GLint max_modelview_stack_depth;
	GLint max_name_stack_depth;
	GLint max_projection_stack_depth;
	GLint max_texture_stack_depth;
	GLint index_bits;
	GLint red_bits;
	GLint green_bits;
	GLint blue_bits;
	GLint alpha_bits;
	GLint depth_bits;
	GLint stencil_bits;
	GLint accum_red_bits;
	GLint accum_green_bits;
	GLint accum_blue_bits;
	GLint accum_alpha_bits;
	GLint name_stack_depth;
	GLint map1_grid_domain;
	GLint map1_grid_segments;
	GLint map2_grid_domain;
	GLint map2_grid_segments;
	GLint polygon_offset_units;
	GLint polygon_offset_factor;
	GLint texture_binding_1d;
	GLint texture_binding_2d;
	GLint client_attrib_stack_depth;
	GLint max_client_attrib_stack_depth;
	GLint feedback_buffer_size;
	GLint feedback_buffer_type;
	GLint selection_buffer_size;
	GLint vertex_array_size;
	GLint vertex_array_type;
	GLint vertex_array_stride;
	GLint normal_array_type;
	GLint normal_array_stride;
	GLint color_array_size;
	GLint color_array_type;
	GLint color_array_stride;
	GLint index_array_type;
	GLint index_array_stride;
	GLint texture_coord_array_size;
	GLint texture_coord_array_type;
	GLint texture_coord_array_stride;
	GLint edge_flag_array_stride;
	GLint texture_binding_3d;
	GLint max_3d_texture_size;
	GLint max_elements_vertices;
	GLint max_elements_indices;
	GLint smooth_point_size_range;
	GLint smooth_point_size_granularity;
	GLint smooth_line_width_range;
	GLint smooth_line_width_granularity;
	GLint aliased_line_width_range;
	GLint aliased_point_size_range;
	GLint active_texture;
	GLint sample_coverage_value;
	GLint sample_coverage_invert;
	GLint texture_binding_cube_map;
	GLint max_cube_map_texture_size;
	GLint num_compressed_texture_formats;
	GLint compressed_texture_formats;
	GLint array_buffer_binding;
	GLint element_array_buffer_binding;

	GLfloat blend_color[4];
	GLint blend_dst_rgb[MAX_COLOR_ATTACHMENTS];
	GLint blend_src_rgb[MAX_COLOR_ATTACHMENTS];
	GLint blend_dst_alpha[MAX_COLOR_ATTACHMENTS];
	GLint blend_src_alpha[MAX_COLOR_ATTACHMENTS];
	GLint blend_equation_rgb[MAX_COLOR_ATTACHMENTS];
	GLint blend_equation_alpha[MAX_COLOR_ATTACHMENTS];

	GLint stencil_back_func;
	GLint stencil_back_fail;
	GLint stencil_back_pass_depth_fail;
	GLint stencil_back_pass_depth_pass;

	GLint max_texture_lod_bias;
	GLint max_draw_buffers;
	GLint max_vertex_attribs;
	GLint max_texture_image_units;
	GLint max_fragment_uniform_components;
	GLint max_vertex_uniform_components;
	GLint max_varying_floats;
	GLint max_vertex_texture_image_units;
	GLint max_combined_texture_image_units;
	GLint current_program;
	GLint stencil_back_ref;
	GLint stencil_back_value_mask;
	GLint stencil_back_writemask;
	GLint pixel_pack_buffer_binding;
	GLint pixel_unpack_buffer_binding;
	GLint max_clip_distances;
	GLint major_version;
	GLint minor_version;
	GLint num_extensions;
	GLint context_flags;
	GLint max_array_texture_layers;
	GLint min_program_texel_offset;
	GLint max_program_texel_offset;
	GLint max_varying_components;
	GLint texture_binding_1d_array;
	GLint texture_binding_2d_array;
	GLint max_renderbuffer_size;
	GLint draw_framebuffer_binding;
	GLint renderbuffer_binding;
	GLint read_framebuffer_binding;
	GLint max_color_attachments;
	GLint vertex_array_binding;
	GLint max_texture_buffer_size;
	GLint texture_binding_buffer;
	GLint texture_binding_rectangle;
	GLint max_rectangle_texture_size;
	GLint primitive_restart_index;
	GLint uniform_buffer_binding;
	GLint uniform_buffer_start;
	GLint uniform_buffer_size;
	GLint max_vertex_uniform_blocks;
	GLint max_geometry_uniform_blocks;
	GLint max_fragment_uniform_blocks;
	GLint max_combined_uniform_blocks;
	GLint max_uniform_buffer_bindings;
	GLint max_uniform_block_size;
	GLint max_combined_vertex_uniform_components;
	GLint max_combined_geometry_uniform_components;
	GLint max_combined_fragment_uniform_components;
	GLint uniform_buffer_offset_alignment;
	GLint max_geometry_texture_image_units;
	GLint max_geometry_uniform_components;
	GLint max_vertex_output_components;
	GLint max_geometry_input_components;
	GLint max_geometry_output_components;
	GLint max_fragment_input_components;
	GLint context_profile_mask;
	GLint provoking_vertex;
	GLint max_server_wait_timeout;
	GLint max_sample_mask_words;
	GLint texture_binding_2d_multisample;
	GLint texture_binding_2d_multisample_array;
	GLint max_color_texture_samples;
	GLint max_depth_texture_samples;
	GLint max_integer_samples;
	GLint max_dual_source_draw_buffers;
	GLint sampler_binding;
	GLint max_tess_control_uniform_blocks;
	GLint max_tess_evaluation_uniform_blocks;
	GLint shader_compiler;
	GLint shader_binary_formats;
	GLint num_shader_binary_formats;
	GLint max_vertex_uniform_vectors;
	GLint max_varying_vectors;
	GLint max_fragment_uniform_vectors;
	GLint num_program_binary_formats;
	GLint program_binary_formats;
	GLint program_pipeline_binding;
	GLint max_viewports;
	GLint viewport_subpixel_bits;
	GLint viewport_bounds_range;
	GLint layer_provoking_vertex;
	GLint viewport_index_provoking_vertex;
	GLint min_map_buffer_alignment;
	GLint max_vertex_atomic_counters;
	GLint max_tess_control_atomic_counters;
	GLint max_tess_evaluation_atomic_counters;
	GLint max_geometry_atomic_counters;
	GLint max_fragment_atomic_counters;
	GLint max_combined_atomic_counters;
	GLint max_element_index;
	GLint max_compute_uniform_blocks;
	GLint max_compute_texture_image_units;
	GLint max_compute_uniform_components;
	GLint max_compute_atomic_counter_buffers;
	GLint max_compute_atomic_counters;
	GLint max_combined_compute_uniform_components;
	GLint max_compute_work_group_invocations;
	GLint max_compute_work_group_count[3];
	GLint max_compute_work_group_size[3];
	GLint dispatch_indirect_buffer_binding;
	GLint max_debug_group_stack_depth;
	GLint debug_group_stack_depth;
	GLint max_label_length;
	GLint max_uniform_locations;
	GLint max_framebuffer_width;
	GLint max_framebuffer_height;
	GLint max_framebuffer_layers;
	GLint max_framebuffer_samples;
	GLint shader_storage_buffer_binding;
	GLint shader_storage_buffer_start;
	GLint shader_storage_buffer_size;
	GLint max_vertex_shader_storage_blocks;
	GLint max_geometry_shader_storage_blocks;
	GLint max_tess_control_shader_storage_blocks;
	GLint max_tess_evaluation_shader_storage_blocks;
	GLint max_fragment_shader_storage_blocks;
	GLint max_compute_shader_storage_blocks;
	GLint max_combined_shader_storage_blocks;
	GLint max_shader_storage_buffer_bindings;
	GLint shader_storage_buffer_offset_alignment;
	GLint texture_buffer_offset_alignment;
	GLint vertex_binding_divisor;
	GLint vertex_binding_offset;
	GLint vertex_binding_stride;
	GLint max_vertex_attrib_relative_offset;
	GLint max_vertex_attrib_bindings;
} GLMParams;

#endif /* glm_params_h */
