/**************************************************************************/
/*  proc_rocks.h                                                          */
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

#ifndef PROC_ROCK_MESH_H
#define PROC_ROCK_MESH_H

#include "core/color.h"
#include "core/dictionary.h"
#include "scene/main/timer.h"
#include "scene/resources/material.h"
#include "scene/resources/mesh.h"

class ProcRockMesh : public ArrayMesh {
	GDCLASS(ProcRockMesh, ArrayMesh)

	struct {
		int depth;
		int randseed;
		real_t smoothness;
		bool smoothed;
	} rockgen;

	struct {
		Vector3 dimensions;
		uint32_t steps;
		Vector2 rand_angle_range;
		real_t rand_offset_percent;
		real_t rand_shift;
		Vector2i plane_verts_range;
		uint32_t max_planes;
	} rockgeneration;

	struct {
		int rock_type; // 0=Cubic, 1=Boulder, 2=Quartz, 3=Custom
		int num_vertices;
		real_t width, height, depth;
		real_t radius;
		real_t tip_protrusion, tip_flatness;
		real_t base_width, base_height;
		bool tetragonal, one_sided;
		int randseed;
	} rockstudio;

	struct {
		int subdivisions;
		real_t width, height, depth;
		real_t noise_frequency, noise_amplitude;
		int noise_octaves;
		real_t noise_persistence;
		int randseed;
		bool cutplane_enabled;
		real_t cutplane_offset;
		bool smoothed;

		bool generate_textures;
		int texture_size;
		Color albedo_low, albedo_high;
		real_t normal_strength;
		real_t roughness_scale, roughness_bias;
		real_t metalness_scale, metalness_bias;
		real_t ao_scale, ao_bias;

		// Set by load_from_file() — when non-empty, _rebuild() drives the mesh/texture
		// pipeline from this parsed JSON preset instead of the scalar fields above (see
		// generators/procrockgen/procrockgen.h's rock_pipeline_*_from_json() and memo.md's
		// "JSON pipeline reader" section for scope/limitations).
		String json_path;
		Dictionary json_cache;
	} pipeline;

	Ref<SpatialMaterial> _pipeline_material;

	bool auto_refresh;
	int method;

	bool _dirty;
	void _rebuild();

protected:
	static void _bind_methods();

	bool _is_generated() const { return true; }

	void _get_property_list(List<PropertyInfo> *p_list) const;
	bool _set(const StringName &p_path, const Variant &p_value);
	bool _get(const StringName &p_path, Variant &r_ret) const;

public:
	void set_auto_refresh(bool p_refresh);
	bool get_auto_refresh() const;
	void set_generator(int p_mode);
	int get_generator() const;

	// Gen. method 1
	void set_rockgen_depth(int p_depth);
	int get_rockgen_depth() const;
	void set_rockgen_randseed(int p_randseed);
	int get_rockgen_randseed() const;
	void set_rockgen_smoothness(real_t p_smoothness);
	real_t get_rockgen_smoothness() const;
	void set_rockgen_smoothed(bool p_smoothed);
	bool get_rockgen_smoothed() const;

	// Gen. method 2
	void set_rockgeneration_steps(uint32_t p_steps);
	uint32_t get_rockgeneration_steps() const;
	void set_rockgeneration_width(real_t p_width);
	real_t get_rockgeneration_width() const;
	void set_rockgeneration_height(real_t p_height);
	real_t get_rockgeneration_height() const;
	void set_rockgeneration_depth(real_t p_depth);
	real_t get_rockgeneration_depth() const;
	void set_rockgeneration_max_planes(uint32_t p_planes);
	uint32_t get_rockgeneration_max_planes() const;

	// Gen. method 3 — RockStudio (convex hull)
	void set_rockstudio_rock_type(int p_type);
	int get_rockstudio_rock_type() const;
	void set_rockstudio_num_vertices(int p_num);
	int get_rockstudio_num_vertices() const;
	void set_rockstudio_width(real_t p_val);
	real_t get_rockstudio_width() const;
	void set_rockstudio_height(real_t p_val);
	real_t get_rockstudio_height() const;
	void set_rockstudio_depth(real_t p_val);
	real_t get_rockstudio_depth() const;
	void set_rockstudio_radius(real_t p_val);
	real_t get_rockstudio_radius() const;
	void set_rockstudio_randseed(int p_seed);
	int get_rockstudio_randseed() const;

	// Gen. method 4 — ProcRock (noise pipeline)
	void set_pipeline_subdivisions(int p_val);
	int get_pipeline_subdivisions() const;
	void set_pipeline_width(real_t p_val);
	real_t get_pipeline_width() const;
	void set_pipeline_height(real_t p_val);
	real_t get_pipeline_height() const;
	void set_pipeline_depth(real_t p_val);
	real_t get_pipeline_depth() const;
	void set_pipeline_noise_frequency(real_t p_val);
	real_t get_pipeline_noise_frequency() const;
	void set_pipeline_noise_amplitude(real_t p_val);
	real_t get_pipeline_noise_amplitude() const;
	void set_pipeline_noise_octaves(int p_val);
	int get_pipeline_noise_octaves() const;
	void set_pipeline_noise_persistence(real_t p_val);
	real_t get_pipeline_noise_persistence() const;
	void set_pipeline_randseed(int p_val);
	int get_pipeline_randseed() const;
	void set_pipeline_cutplane_enabled(bool p_val);
	bool get_pipeline_cutplane_enabled() const;
	void set_pipeline_cutplane_offset(real_t p_val);
	real_t get_pipeline_cutplane_offset() const;
	void set_pipeline_smoothed(bool p_val);
	bool get_pipeline_smoothed() const;
	void set_pipeline_generate_textures(bool p_val);
	bool get_pipeline_generate_textures() const;
	void set_pipeline_texture_size(int p_val);
	int get_pipeline_texture_size() const;
	void set_pipeline_albedo_low(const Color &p_val);
	Color get_pipeline_albedo_low() const;
	void set_pipeline_albedo_high(const Color &p_val);
	Color get_pipeline_albedo_high() const;
	void set_pipeline_normal_strength(real_t p_val);
	real_t get_pipeline_normal_strength() const;
	void set_pipeline_roughness_scale(real_t p_val);
	real_t get_pipeline_roughness_scale() const;
	void set_pipeline_roughness_bias(real_t p_val);
	real_t get_pipeline_roughness_bias() const;
	void set_pipeline_metalness_scale(real_t p_val);
	real_t get_pipeline_metalness_scale() const;
	void set_pipeline_metalness_bias(real_t p_val);
	real_t get_pipeline_metalness_bias() const;
	void set_pipeline_ao_scale(real_t p_val);
	real_t get_pipeline_ao_scale() const;
	void set_pipeline_ao_bias(real_t p_val);
	real_t get_pipeline_ao_bias() const;
	void set_pipeline_preset(int p_preset);

	Ref<SpatialMaterial> get_pipeline_material() const { return _pipeline_material; }

	Error load_from_file(const String p_path);

	ProcRockMesh();
};

#endif // PROC_ROCK_MESH_H
