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
#include "scene/resources/curve.h"
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
		real_t rand_offset_percent;
		real_t rand_shift;
		Vector2i plane_verts_range;
		uint32_t max_planes;
		bool smoothed;
		int randseed;
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

	// Method 4: RockCluster — see memo.md's "Method 4: RockCluster" for the
	// algorithm writeup.
	struct {
		int style; // 0=Boulder, 1=Sharp, 2=Crystal
		int randseed;
		// Boulder/Sharp only
		int density;
		real_t radius;
		real_t asymmetry;
		real_t wave;
		real_t decentralize;
		real_t scale_local;
		Ref<Curve> scale_by_distance;
		real_t tallness, flatness, wideness;
		real_t rotation, rotation_local, rotation_rnd;
		// Crystal only (shares `density` above)
		real_t crystal_scale; // flat size multiplier — Crystal has no other absolute-scale control
		real_t scale_by_angle;
		real_t scale_random_offset;
		real_t scale_bias;
		real_t bloom;
	} rockcluster;

	// Generic texture-source selection, applying to every generator (0-3) — unlike
	// `pipeline` above, which is Method-3 (ProcRock)-only. See memo.md's "Texture
	// generation" section: Method 3 keeps `pipeline.generate_textures` as its own
	// richer, JSON-capable path, taking precedence over this when enabled.
	struct {
		int source; // 0=None,1=Generated,2=Gravel,3=Mossy,4=Rock,5=File — see TextureSource

		// Generated
		int gen_size;
		int gen_seed;
		real_t gen_noise_frequency;
		int gen_noise_octaves;
		real_t gen_noise_persistence;
		Color gen_albedo_low, gen_albedo_high;
		real_t gen_normal_strength;
		real_t gen_roughness_scale, gen_roughness_bias;
		real_t gen_metalness_scale, gen_metalness_bias;
		real_t gen_ao_scale, gen_ao_bias;

		// From File — res://-relative paths; the 4 companion fields are
		// auto-populated when file_albedo is set (see _auto_detect_companion_textures()),
		// but remain individually editable afterward.
		String file_albedo, file_normal, file_roughness, file_metalness, file_ambient_occlusion;
	} texture;

	// GRAVEL/MOSSY/ROCK apply their baked demo pack entirely in memory (same as
	// GENERATED) — see _apply_texture_source()'s TOOLS_ENABLED-gated case. A real file
	// only gets written out at bake() time (see bake()'s comment for why: writing one
	// during live editing and immediately trying to ResourceLoader::load() it back
	// fails, since a freshly-written file hasn't been through the editor's import
	// pipeline yet).
	enum TextureSource {
		TEXTURE_SOURCE_NONE,
		TEXTURE_SOURCE_GENERATED,
		TEXTURE_SOURCE_GRAVEL,
		TEXTURE_SOURCE_MOSSY,
		TEXTURE_SOURCE_ROCK,
		TEXTURE_SOURCE_FILE,
	};

	bool auto_refresh;
	int method;

	// Generic cross-generator "flatten base" cut — applies to methods 0-3 (RockGen,
	// IcoRock, RockStudio, ProcRock), each of which normally produces a free-floating
	// blob with no flat side. Not applied to method 4 (RockCluster): it already clip+caps
	// its Boulder/Sharp cells individually for the same terrain-sitting purpose (see
	// memo.md's "Bugs Fixed" #24) — running this generic single-plane cut on RockCluster's
	// already-multi-piece combined mesh would hit the same "several disjoint
	// cross-sections fanned into one shared cap" failure that #24 specifically fixed by
	// clipping per-cell instead.
	bool flatten_base_enabled;
	real_t flatten_base_offset;

	bool _dirty;
	void _rebuild();

#ifdef TOOLS_ENABLED
	// Applies `texture.source` to surface 0 — called from _rebuild() for every
	// generator (Method 3 only when its own pipeline.generate_textures is off, to
	// preserve that legacy path's exact existing behavior). See generators/shared/
	// texture_gen.h / baked_textures_gen.h for the underlying per-source logic.
	void _apply_texture_source();

	// For the given texture.file_albedo path, searches its directory for
	// companion PBR maps by filename convention (see proc_rocks.cpp for the exact
	// token/extension lists) and overwrites all 4 texture.file_* companion fields
	// with whatever is found (empty if nothing matches) — called from
	// set_texture_file_albedo().
	void _auto_detect_companion_textures();
#endif

	// When true, this resource's surface geometry is real, frozen data serialized
	// through ArrayMesh's own inherited (de)serialization instead of being
	// regenerated on load — see set_baked()/bake() and memo.md's "Baking" section.
	bool _baked;

protected:
	static void _bind_methods();

	// Godot's PrimitiveMesh/CubeMesh/SphereMesh convention: a "generated" mesh never
	// serializes its own surface arrays (ArrayMesh::_get()/_get_property_list() skip
	// them), since they're cheap to regenerate on load. A baked ProcRockMesh is the
	// opposite — its whole point is to be loadable without regenerating — so it opts
	// back into ArrayMesh's normal surface serialization once frozen.
	bool _is_generated() const { return !_baked; }

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
	void set_rockgeneration_smoothed(bool p_smoothed);
	bool get_rockgeneration_smoothed() const;
	void set_rockgeneration_randseed(int p_randseed);
	int get_rockgeneration_randseed() const;

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

	// Gen. method 5 — RockCluster (scatter/combine cell meshes)
	void set_rockcluster_style(int p_style);
	int get_rockcluster_style() const;
	void set_rockcluster_randseed(int p_seed);
	int get_rockcluster_randseed() const;
	void set_rockcluster_density(int p_val);
	int get_rockcluster_density() const;
	void set_rockcluster_radius(real_t p_val);
	real_t get_rockcluster_radius() const;
	void set_rockcluster_asymmetry(real_t p_val);
	real_t get_rockcluster_asymmetry() const;
	void set_rockcluster_wave(real_t p_val);
	real_t get_rockcluster_wave() const;
	void set_rockcluster_decentralize(real_t p_val);
	real_t get_rockcluster_decentralize() const;
	void set_rockcluster_scale_local(real_t p_val);
	real_t get_rockcluster_scale_local() const;
	void set_rockcluster_scale_by_distance(const Ref<Curve> &p_val);
	Ref<Curve> get_rockcluster_scale_by_distance() const;
	void set_rockcluster_tallness(real_t p_val);
	real_t get_rockcluster_tallness() const;
	void set_rockcluster_flatness(real_t p_val);
	real_t get_rockcluster_flatness() const;
	void set_rockcluster_wideness(real_t p_val);
	real_t get_rockcluster_wideness() const;
	void set_rockcluster_rotation(real_t p_val);
	real_t get_rockcluster_rotation() const;
	void set_rockcluster_rotation_local(real_t p_val);
	real_t get_rockcluster_rotation_local() const;
	void set_rockcluster_rotation_rnd(real_t p_val);
	real_t get_rockcluster_rotation_rnd() const;
	void set_rockcluster_crystal_scale(real_t p_val);
	real_t get_rockcluster_crystal_scale() const;
	void set_rockcluster_scale_by_angle(real_t p_val);
	real_t get_rockcluster_scale_by_angle() const;
	void set_rockcluster_scale_random_offset(real_t p_val);
	real_t get_rockcluster_scale_random_offset() const;
	void set_rockcluster_scale_bias(real_t p_val);
	real_t get_rockcluster_scale_bias() const;
	void set_rockcluster_bloom(real_t p_val);
	real_t get_rockcluster_bloom() const;

	// Generic "flatten base" cut — applies to methods 0-3 only, see the field comments
	// above.
	void set_flatten_base_enabled(bool p_val);
	bool get_flatten_base_enabled() const;
	void set_flatten_base_offset(real_t p_val);
	real_t get_flatten_base_offset() const;

	// Generic texture source — applies to every generator (0-3), see the `texture`
	// struct/TextureSource enum above.
	void set_texture_source(int p_val);
	int get_texture_source() const;
	void set_texture_gen_size(int p_val);
	int get_texture_gen_size() const;
	void set_texture_gen_seed(int p_val);
	int get_texture_gen_seed() const;
	void set_texture_gen_noise_frequency(real_t p_val);
	real_t get_texture_gen_noise_frequency() const;
	void set_texture_gen_noise_octaves(int p_val);
	int get_texture_gen_noise_octaves() const;
	void set_texture_gen_noise_persistence(real_t p_val);
	real_t get_texture_gen_noise_persistence() const;
	void set_texture_gen_albedo_low(const Color &p_val);
	Color get_texture_gen_albedo_low() const;
	void set_texture_gen_albedo_high(const Color &p_val);
	Color get_texture_gen_albedo_high() const;
	void set_texture_gen_normal_strength(real_t p_val);
	real_t get_texture_gen_normal_strength() const;
	void set_texture_gen_roughness_scale(real_t p_val);
	real_t get_texture_gen_roughness_scale() const;
	void set_texture_gen_roughness_bias(real_t p_val);
	real_t get_texture_gen_roughness_bias() const;
	void set_texture_gen_metalness_scale(real_t p_val);
	real_t get_texture_gen_metalness_scale() const;
	void set_texture_gen_metalness_bias(real_t p_val);
	real_t get_texture_gen_metalness_bias() const;
	void set_texture_gen_ao_scale(real_t p_val);
	real_t get_texture_gen_ao_scale() const;
	void set_texture_gen_ao_bias(real_t p_val);
	real_t get_texture_gen_ao_bias() const;
	void set_texture_file_albedo(const String &p_val);
	String get_texture_file_albedo() const;
	void set_texture_file_normal(const String &p_val);
	String get_texture_file_normal() const;
	void set_texture_file_roughness(const String &p_val);
	String get_texture_file_roughness() const;
	void set_texture_file_metalness(const String &p_val);
	String get_texture_file_metalness() const;
	void set_texture_file_ambient_occlusion(const String &p_val);
	String get_texture_file_ambient_occlusion() const;

	Error load_from_file(const String p_path);

	// Baking freezes the current geometry into real, serializable surface data (see
	// _is_generated() above) so it can be loaded without regenerating — the mechanism
	// an export-time auto-bake step (or a script) relies on. Always available for
	// introspection; only bake() itself needs generation to be compiled in.
	void set_baked(bool p_baked);
	bool get_baked() const { return _baked; }
#ifdef TOOLS_ENABLED
	Error bake();
#endif

	ProcRockMesh();
};

#endif // PROC_ROCK_MESH_H
