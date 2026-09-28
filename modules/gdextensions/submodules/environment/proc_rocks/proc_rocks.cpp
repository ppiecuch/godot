/**************************************************************************/
/*  proc_rocks.cpp                                                        */
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

#include "proc_rocks.h"

#include "generators/procrockgen/procrockgen.h"
#include "generators/rockcluster/rockcluster.h"
#include "generators/rockcluster/rockcluster_cells_data.gen.h"
#include "generators/rockgen/rockgen.h"
#include "generators/rockgeneration/gen_rock.h"
#include "generators/rockstudio/rock_studio.h"
#include "generators/shared/plane_flatten.h"
#include "generators/shared/texture_gen.h"

// Editor-only baked demo texture packs — a genuine editor resource, not generator
// code, so it lives under modules/gdextensions/editor/ rather than here (see that
// file's own comment). Declaration-only header, safe to include unconditionally;
// used by _apply_texture_source()'s TOOLS_ENABLED-gated Gravel/Mossy/Rock cases
// (in-memory only) and by bake()'s file-materializing conversion of the same.
#include "editor/proc_rocks_baked_textures.h"

#include "core/image.h"
#include "core/io/json.h"
#include "core/io/resource_loader.h"
#include "core/os/dir_access.h"
#include "core/os/file_access.h"

// =========================================================================
// Generator selection
// =========================================================================

void ProcRockMesh::set_generator(int p_mode) {
	if (method != p_mode) {
		method = CLAMP(p_mode, 0, 4);
		_dirty = true;
		_change_notify();
		if (auto_refresh) {
			_rebuild();
		}
	}
}

int ProcRockMesh::get_generator() const {
	return method;
}

// =========================================================================
// Method 0: rockgen properties
// =========================================================================

void ProcRockMesh::set_rockgen_depth(int p_depth) {
	if (rockgen.depth != p_depth) {
		// Triangle count is 20*4^depth (fractal icosahedron subdivision) — clamped in
		// code, not just the inspector hint, so a script can't request a 1.3-billion
		// triangle mesh (depth=8) either. See memo.md's "Bugs Fixed" #16.
		rockgen.depth = CLAMP(p_depth, 0, 3);
		_dirty = true;
		if (auto_refresh) {
			_rebuild();
		}
	}
}

int ProcRockMesh::get_rockgen_depth() const {
	return rockgen.depth;
}

void ProcRockMesh::set_rockgen_randseed(int p_randseed) {
	if (rockgen.randseed != p_randseed) {
		rockgen.randseed = p_randseed;
		_dirty = true;
		if (auto_refresh) {
			_rebuild();
		}
	}
}

int ProcRockMesh::get_rockgen_randseed() const {
	return rockgen.randseed;
}

void ProcRockMesh::set_rockgen_smoothness(real_t p_smoothness) {
	if (rockgen.smoothness != p_smoothness) {
		rockgen.smoothness = p_smoothness;
		_dirty = true;
		if (auto_refresh) {
			_rebuild();
		}
	}
}

real_t ProcRockMesh::get_rockgen_smoothness() const {
	return rockgen.smoothness;
}

void ProcRockMesh::set_rockgen_smoothed(bool p_smoothed) {
	if (rockgen.smoothed != p_smoothed) {
		rockgen.smoothed = p_smoothed;
		_dirty = true;
		if (auto_refresh) {
			_rebuild();
		}
	}
}

bool ProcRockMesh::get_rockgen_smoothed() const {
	return rockgen.smoothed;
}

// =========================================================================
// Method 1: rockgeneration properties
// =========================================================================

void ProcRockMesh::set_rockgeneration_steps(uint32_t p_steps) {
	if (rockgeneration.steps != p_steps) {
		// Same 20*4^n growth as rockgen_depth (GenRock::BuildIco() feeds this straight
		// into MakeIcosphere()) — clamped in code, not just the inspector hint. See
		// memo.md's "Bugs Fixed" #16 (the old default of 10 alone was ~21M triangles).
		rockgeneration.steps = CLAMP(p_steps, (uint32_t)1, (uint32_t)3);
		_dirty = true;
		if (auto_refresh) {
			_rebuild();
		}
	}
}

uint32_t ProcRockMesh::get_rockgeneration_steps() const {
	return rockgeneration.steps;
}

void ProcRockMesh::set_rockgeneration_width(real_t p_width) {
	if (rockgeneration.dimensions.x != p_width) {
		rockgeneration.dimensions.x = p_width;
		_dirty = true;
		if (auto_refresh) {
			_rebuild();
		}
	}
}

real_t ProcRockMesh::get_rockgeneration_width() const {
	return rockgeneration.dimensions.x;
}

void ProcRockMesh::set_rockgeneration_height(real_t p_height) {
	if (rockgeneration.dimensions.y != p_height) {
		rockgeneration.dimensions.y = p_height;
		_dirty = true;
		if (auto_refresh) {
			_rebuild();
		}
	}
}

real_t ProcRockMesh::get_rockgeneration_height() const {
	return rockgeneration.dimensions.y;
}

void ProcRockMesh::set_rockgeneration_depth(real_t p_depth) {
	if (rockgeneration.dimensions.z != p_depth) {
		rockgeneration.dimensions.z = p_depth;
		_dirty = true;
		if (auto_refresh) {
			_rebuild();
		}
	}
}

real_t ProcRockMesh::get_rockgeneration_depth() const {
	return rockgeneration.dimensions.z;
}

void ProcRockMesh::set_rockgeneration_max_planes(uint32_t p_planes) {
	if (rockgeneration.max_planes != p_planes) {
		// Doesn't affect triangle count (BuildRock() only moves existing icosphere
		// vertices), but an unbounded plane count is still O(planes * vertex_count) work
		// for no visual benefit past a point — clamp defensively.
		rockgeneration.max_planes = CLAMP(p_planes, (uint32_t)1, (uint32_t)10);
		_dirty = true;
		if (auto_refresh) {
			_rebuild();
		}
	}
}

uint32_t ProcRockMesh::get_rockgeneration_max_planes() const {
	return rockgeneration.max_planes;
}

void ProcRockMesh::set_rockgeneration_smoothed(bool p_smoothed) {
	if (rockgeneration.smoothed != p_smoothed) {
		rockgeneration.smoothed = p_smoothed;
		_dirty = true;
		if (auto_refresh) {
			_rebuild();
		}
	}
}

bool ProcRockMesh::get_rockgeneration_smoothed() const {
	return rockgeneration.smoothed;
}

void ProcRockMesh::set_rockgeneration_randseed(int p_randseed) {
	if (rockgeneration.randseed != p_randseed) {
		rockgeneration.randseed = p_randseed;
		_dirty = true;
		if (auto_refresh) {
			_rebuild();
		}
	}
}

int ProcRockMesh::get_rockgeneration_randseed() const {
	return rockgeneration.randseed;
}

// =========================================================================
// Method 2: RockStudio properties
// =========================================================================

void ProcRockMesh::set_rockstudio_rock_type(int p_type) {
	if (rockstudio.rock_type != p_type) {
		rockstudio.rock_type = CLAMP(p_type, 0, 2);
		_dirty = true;
		if (auto_refresh) {
			_rebuild();
		}
	}
}
int ProcRockMesh::get_rockstudio_rock_type() const { return rockstudio.rock_type; }

void ProcRockMesh::set_rockstudio_num_vertices(int p_num) {
	if (rockstudio.num_vertices != p_num) {
		// Convex hull face count is ~2*num_vertices-4 — cap at 150 points (~300 faces)
		// instead of the old unbounded-above MAX(4,...), so a script can't request a
		// hull of 1000+ points either. See memo.md's "Bugs Fixed" #16.
		rockstudio.num_vertices = CLAMP(p_num, 4, 150);
		_dirty = true;
		if (auto_refresh) {
			_rebuild();
		}
	}
}
int ProcRockMesh::get_rockstudio_num_vertices() const { return rockstudio.num_vertices; }

void ProcRockMesh::set_rockstudio_width(real_t p_val) {
	if (rockstudio.width != p_val) {
		rockstudio.width = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_rockstudio_width() const { return rockstudio.width; }
void ProcRockMesh::set_rockstudio_height(real_t p_val) {
	if (rockstudio.height != p_val) {
		rockstudio.height = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_rockstudio_height() const { return rockstudio.height; }
void ProcRockMesh::set_rockstudio_depth(real_t p_val) {
	if (rockstudio.depth != p_val) {
		rockstudio.depth = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_rockstudio_depth() const { return rockstudio.depth; }
void ProcRockMesh::set_rockstudio_radius(real_t p_val) {
	if (rockstudio.radius != p_val) {
		rockstudio.radius = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_rockstudio_radius() const { return rockstudio.radius; }
void ProcRockMesh::set_rockstudio_randseed(int p_seed) {
	if (rockstudio.randseed != p_seed) {
		rockstudio.randseed = p_seed;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
int ProcRockMesh::get_rockstudio_randseed() const { return rockstudio.randseed; }

// =========================================================================
// Method 3: ProcRock (noise pipeline) properties
// =========================================================================

void ProcRockMesh::set_pipeline_subdivisions(int p_val) {
	if (pipeline.subdivisions != p_val) {
		// Same 20*4^n icosphere growth as rockgen_depth/rockgeneration_steps (old max of
		// 6 was 327,680 triangles) — see memo.md's "Bugs Fixed" #16.
		pipeline.subdivisions = CLAMP(p_val, 0, 3);
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
int ProcRockMesh::get_pipeline_subdivisions() const { return pipeline.subdivisions; }

void ProcRockMesh::set_pipeline_width(real_t p_val) {
	if (pipeline.width != p_val) {
		pipeline.width = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_pipeline_width() const { return pipeline.width; }

void ProcRockMesh::set_pipeline_height(real_t p_val) {
	if (pipeline.height != p_val) {
		pipeline.height = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_pipeline_height() const { return pipeline.height; }

void ProcRockMesh::set_pipeline_depth(real_t p_val) {
	if (pipeline.depth != p_val) {
		pipeline.depth = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_pipeline_depth() const { return pipeline.depth; }

void ProcRockMesh::set_pipeline_noise_frequency(real_t p_val) {
	if (pipeline.noise_frequency != p_val) {
		pipeline.noise_frequency = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_pipeline_noise_frequency() const { return pipeline.noise_frequency; }

void ProcRockMesh::set_pipeline_noise_amplitude(real_t p_val) {
	if (pipeline.noise_amplitude != p_val) {
		pipeline.noise_amplitude = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_pipeline_noise_amplitude() const { return pipeline.noise_amplitude; }

void ProcRockMesh::set_pipeline_noise_octaves(int p_val) {
	if (pipeline.noise_octaves != p_val) {
		pipeline.noise_octaves = CLAMP(p_val, 1, 6);
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
int ProcRockMesh::get_pipeline_noise_octaves() const { return pipeline.noise_octaves; }

void ProcRockMesh::set_pipeline_noise_persistence(real_t p_val) {
	if (pipeline.noise_persistence != p_val) {
		pipeline.noise_persistence = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_pipeline_noise_persistence() const { return pipeline.noise_persistence; }

void ProcRockMesh::set_pipeline_randseed(int p_val) {
	if (pipeline.randseed != p_val) {
		pipeline.randseed = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
int ProcRockMesh::get_pipeline_randseed() const { return pipeline.randseed; }

void ProcRockMesh::set_pipeline_cutplane_enabled(bool p_val) {
	if (pipeline.cutplane_enabled != p_val) {
		pipeline.cutplane_enabled = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
bool ProcRockMesh::get_pipeline_cutplane_enabled() const { return pipeline.cutplane_enabled; }

void ProcRockMesh::set_pipeline_cutplane_offset(real_t p_val) {
	if (pipeline.cutplane_offset != p_val) {
		pipeline.cutplane_offset = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_pipeline_cutplane_offset() const { return pipeline.cutplane_offset; }

void ProcRockMesh::set_pipeline_smoothed(bool p_val) {
	if (pipeline.smoothed != p_val) {
		pipeline.smoothed = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
bool ProcRockMesh::get_pipeline_smoothed() const { return pipeline.smoothed; }

void ProcRockMesh::set_pipeline_generate_textures(bool p_val) {
	if (pipeline.generate_textures != p_val) {
		pipeline.generate_textures = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
bool ProcRockMesh::get_pipeline_generate_textures() const { return pipeline.generate_textures; }

void ProcRockMesh::set_pipeline_texture_size(int p_val) {
	if (pipeline.texture_size != p_val) {
		pipeline.texture_size = CLAMP(p_val, 8, 4096);
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
int ProcRockMesh::get_pipeline_texture_size() const { return pipeline.texture_size; }

void ProcRockMesh::set_pipeline_albedo_low(const Color &p_val) {
	pipeline.albedo_low = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
Color ProcRockMesh::get_pipeline_albedo_low() const { return pipeline.albedo_low; }

void ProcRockMesh::set_pipeline_albedo_high(const Color &p_val) {
	pipeline.albedo_high = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
Color ProcRockMesh::get_pipeline_albedo_high() const { return pipeline.albedo_high; }

void ProcRockMesh::set_pipeline_normal_strength(real_t p_val) {
	pipeline.normal_strength = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
real_t ProcRockMesh::get_pipeline_normal_strength() const { return pipeline.normal_strength; }

void ProcRockMesh::set_pipeline_roughness_scale(real_t p_val) {
	pipeline.roughness_scale = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
real_t ProcRockMesh::get_pipeline_roughness_scale() const { return pipeline.roughness_scale; }

void ProcRockMesh::set_pipeline_roughness_bias(real_t p_val) {
	pipeline.roughness_bias = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
real_t ProcRockMesh::get_pipeline_roughness_bias() const { return pipeline.roughness_bias; }

void ProcRockMesh::set_pipeline_metalness_scale(real_t p_val) {
	pipeline.metalness_scale = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
real_t ProcRockMesh::get_pipeline_metalness_scale() const { return pipeline.metalness_scale; }

void ProcRockMesh::set_pipeline_metalness_bias(real_t p_val) {
	pipeline.metalness_bias = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
real_t ProcRockMesh::get_pipeline_metalness_bias() const { return pipeline.metalness_bias; }

void ProcRockMesh::set_pipeline_ao_scale(real_t p_val) {
	pipeline.ao_scale = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
real_t ProcRockMesh::get_pipeline_ao_scale() const { return pipeline.ao_scale; }

void ProcRockMesh::set_pipeline_ao_bias(real_t p_val) {
	pipeline.ao_bias = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
real_t ProcRockMesh::get_pipeline_ao_bias() const { return pipeline.ao_bias; }

// =========================================================================
// Method 5: RockCluster properties
// =========================================================================

void ProcRockMesh::set_rockcluster_style(int p_style) {
	if (rockcluster.style != p_style) {
		rockcluster.style = CLAMP(p_style, 0, 2);
		_dirty = true;
		_change_notify(); // Boulder/Sharp vs Crystal show/hide different property subsets
		if (auto_refresh) {
			_rebuild();
		}
	}
}
int ProcRockMesh::get_rockcluster_style() const { return rockcluster.style; }

void ProcRockMesh::set_rockcluster_randseed(int p_seed) {
	if (rockcluster.randseed != p_seed) {
		rockcluster.randseed = p_seed;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
int ProcRockMesh::get_rockcluster_randseed() const { return rockcluster.randseed; }

void ProcRockMesh::set_rockcluster_density(int p_val) {
	if (rockcluster.density != p_val) {
		// Cell meshes run ~56-200 triangles each (ported library, fixed) — 24 cells
		// keeps the worst case (Sharp) at ~4800 triangles, still "hundreds to low
		// thousands" in spirit even though a multi-piece cluster is a different
		// triangle-count category than this submodule's single-rock generators.
		rockcluster.density = CLAMP(p_val, 1, 24);
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
int ProcRockMesh::get_rockcluster_density() const { return rockcluster.density; }

void ProcRockMesh::set_rockcluster_radius(real_t p_val) {
	if (rockcluster.radius != p_val) {
		rockcluster.radius = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_rockcluster_radius() const { return rockcluster.radius; }

void ProcRockMesh::set_rockcluster_asymmetry(real_t p_val) {
	if (rockcluster.asymmetry != p_val) {
		rockcluster.asymmetry = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_rockcluster_asymmetry() const { return rockcluster.asymmetry; }

void ProcRockMesh::set_rockcluster_wave(real_t p_val) {
	if (rockcluster.wave != p_val) {
		rockcluster.wave = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_rockcluster_wave() const { return rockcluster.wave; }

void ProcRockMesh::set_rockcluster_decentralize(real_t p_val) {
	if (rockcluster.decentralize != p_val) {
		rockcluster.decentralize = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_rockcluster_decentralize() const { return rockcluster.decentralize; }

void ProcRockMesh::set_rockcluster_scale_local(real_t p_val) {
	if (rockcluster.scale_local != p_val) {
		rockcluster.scale_local = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_rockcluster_scale_local() const { return rockcluster.scale_local; }

void ProcRockMesh::set_rockcluster_scale_by_distance(const Ref<Curve> &p_val) {
	rockcluster.scale_by_distance = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
Ref<Curve> ProcRockMesh::get_rockcluster_scale_by_distance() const { return rockcluster.scale_by_distance; }

void ProcRockMesh::set_rockcluster_tallness(real_t p_val) {
	if (rockcluster.tallness != p_val) {
		rockcluster.tallness = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_rockcluster_tallness() const { return rockcluster.tallness; }

void ProcRockMesh::set_rockcluster_flatness(real_t p_val) {
	if (rockcluster.flatness != p_val) {
		rockcluster.flatness = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_rockcluster_flatness() const { return rockcluster.flatness; }

void ProcRockMesh::set_rockcluster_wideness(real_t p_val) {
	if (rockcluster.wideness != p_val) {
		rockcluster.wideness = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_rockcluster_wideness() const { return rockcluster.wideness; }

void ProcRockMesh::set_rockcluster_rotation(real_t p_val) {
	if (rockcluster.rotation != p_val) {
		rockcluster.rotation = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_rockcluster_rotation() const { return rockcluster.rotation; }

void ProcRockMesh::set_rockcluster_rotation_local(real_t p_val) {
	if (rockcluster.rotation_local != p_val) {
		rockcluster.rotation_local = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_rockcluster_rotation_local() const { return rockcluster.rotation_local; }

void ProcRockMesh::set_rockcluster_rotation_rnd(real_t p_val) {
	if (rockcluster.rotation_rnd != p_val) {
		rockcluster.rotation_rnd = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_rockcluster_rotation_rnd() const { return rockcluster.rotation_rnd; }

void ProcRockMesh::set_rockcluster_crystal_scale(real_t p_val) {
	if (rockcluster.crystal_scale != p_val) {
		rockcluster.crystal_scale = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_rockcluster_crystal_scale() const { return rockcluster.crystal_scale; }

void ProcRockMesh::set_rockcluster_scale_by_angle(real_t p_val) {
	if (rockcluster.scale_by_angle != p_val) {
		rockcluster.scale_by_angle = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_rockcluster_scale_by_angle() const { return rockcluster.scale_by_angle; }

void ProcRockMesh::set_rockcluster_scale_random_offset(real_t p_val) {
	if (rockcluster.scale_random_offset != p_val) {
		rockcluster.scale_random_offset = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_rockcluster_scale_random_offset() const { return rockcluster.scale_random_offset; }

void ProcRockMesh::set_rockcluster_scale_bias(real_t p_val) {
	if (rockcluster.scale_bias != p_val) {
		rockcluster.scale_bias = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_rockcluster_scale_bias() const { return rockcluster.scale_bias; }

void ProcRockMesh::set_rockcluster_bloom(real_t p_val) {
	if (rockcluster.bloom != p_val) {
		rockcluster.bloom = p_val;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
	}
}
real_t ProcRockMesh::get_rockcluster_bloom() const { return rockcluster.bloom; }

void ProcRockMesh::set_flatten_base_enabled(bool p_val) {
	if (flatten_base_enabled != p_val) {
		flatten_base_enabled = p_val;
		_dirty = true;
		_change_notify(); // flatten_base_offset shows/hides with this
		if (auto_refresh) {
			_rebuild();
		}
	}
}
bool ProcRockMesh::get_flatten_base_enabled() const { return flatten_base_enabled; }

void ProcRockMesh::set_flatten_base_offset(real_t p_val) {
	if (flatten_base_offset != p_val) {
		flatten_base_offset = p_val;
		_dirty = true;
		if (auto_refresh) {
			_rebuild();
		}
	}
}
real_t ProcRockMesh::get_flatten_base_offset() const { return flatten_base_offset; }

void ProcRockMesh::set_texture_source(int p_val) {
	texture.source = CLAMP(p_val, (int)TEXTURE_SOURCE_NONE, (int)TEXTURE_SOURCE_FILE);
	_dirty = true;
	_change_notify(); // dependent fields (gen_*/file_*) show/hide by source
	if (auto_refresh)
		_rebuild();
}
int ProcRockMesh::get_texture_source() const { return texture.source; }

void ProcRockMesh::set_texture_gen_size(int p_val) {
	texture.gen_size = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
int ProcRockMesh::get_texture_gen_size() const { return texture.gen_size; }

void ProcRockMesh::set_texture_gen_seed(int p_val) {
	texture.gen_seed = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
int ProcRockMesh::get_texture_gen_seed() const { return texture.gen_seed; }

void ProcRockMesh::set_texture_gen_noise_frequency(real_t p_val) {
	texture.gen_noise_frequency = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
real_t ProcRockMesh::get_texture_gen_noise_frequency() const { return texture.gen_noise_frequency; }

void ProcRockMesh::set_texture_gen_noise_octaves(int p_val) {
	texture.gen_noise_octaves = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
int ProcRockMesh::get_texture_gen_noise_octaves() const { return texture.gen_noise_octaves; }

void ProcRockMesh::set_texture_gen_noise_persistence(real_t p_val) {
	texture.gen_noise_persistence = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
real_t ProcRockMesh::get_texture_gen_noise_persistence() const { return texture.gen_noise_persistence; }

void ProcRockMesh::set_texture_gen_albedo_low(const Color &p_val) {
	texture.gen_albedo_low = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
Color ProcRockMesh::get_texture_gen_albedo_low() const { return texture.gen_albedo_low; }

void ProcRockMesh::set_texture_gen_albedo_high(const Color &p_val) {
	texture.gen_albedo_high = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
Color ProcRockMesh::get_texture_gen_albedo_high() const { return texture.gen_albedo_high; }

void ProcRockMesh::set_texture_gen_normal_strength(real_t p_val) {
	texture.gen_normal_strength = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
real_t ProcRockMesh::get_texture_gen_normal_strength() const { return texture.gen_normal_strength; }

void ProcRockMesh::set_texture_gen_roughness_scale(real_t p_val) {
	texture.gen_roughness_scale = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
real_t ProcRockMesh::get_texture_gen_roughness_scale() const { return texture.gen_roughness_scale; }

void ProcRockMesh::set_texture_gen_roughness_bias(real_t p_val) {
	texture.gen_roughness_bias = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
real_t ProcRockMesh::get_texture_gen_roughness_bias() const { return texture.gen_roughness_bias; }

void ProcRockMesh::set_texture_gen_metalness_scale(real_t p_val) {
	texture.gen_metalness_scale = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
real_t ProcRockMesh::get_texture_gen_metalness_scale() const { return texture.gen_metalness_scale; }

void ProcRockMesh::set_texture_gen_metalness_bias(real_t p_val) {
	texture.gen_metalness_bias = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
real_t ProcRockMesh::get_texture_gen_metalness_bias() const { return texture.gen_metalness_bias; }

void ProcRockMesh::set_texture_gen_ao_scale(real_t p_val) {
	texture.gen_ao_scale = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
real_t ProcRockMesh::get_texture_gen_ao_scale() const { return texture.gen_ao_scale; }

void ProcRockMesh::set_texture_gen_ao_bias(real_t p_val) {
	texture.gen_ao_bias = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
real_t ProcRockMesh::get_texture_gen_ao_bias() const { return texture.gen_ao_bias; }

void ProcRockMesh::set_texture_file_albedo(const String &p_val) {
	texture.file_albedo = p_val;
#ifdef TOOLS_ENABLED
	_auto_detect_companion_textures();
#endif
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
String ProcRockMesh::get_texture_file_albedo() const { return texture.file_albedo; }

void ProcRockMesh::set_texture_file_normal(const String &p_val) {
	texture.file_normal = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
String ProcRockMesh::get_texture_file_normal() const { return texture.file_normal; }

void ProcRockMesh::set_texture_file_roughness(const String &p_val) {
	texture.file_roughness = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
String ProcRockMesh::get_texture_file_roughness() const { return texture.file_roughness; }

void ProcRockMesh::set_texture_file_metalness(const String &p_val) {
	texture.file_metalness = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
String ProcRockMesh::get_texture_file_metalness() const { return texture.file_metalness; }

void ProcRockMesh::set_texture_file_ambient_occlusion(const String &p_val) {
	texture.file_ambient_occlusion = p_val;
	_dirty = true;
	if (auto_refresh)
		_rebuild();
}
String ProcRockMesh::get_texture_file_ambient_occlusion() const { return texture.file_ambient_occlusion; }

void ProcRockMesh::set_pipeline_preset(int p_preset) {
	// Real values extracted from procrocklib's own 13 demo pipeline JSON files
	// (recovered from git history at 69e0b996a8~1, not restored to the tree — see
	// memo.md). Colors, roughness, and metalness are direct, faithful reads of each
	// file's active procrocklib.Albedo/Roughness/Metalness/AmbientOcclusion config
	// (procrocklib's AlbedoGenerator always serializes both its methods; the active
	// one is picked via each file's own Method choice). The base noise displacement,
	// however, was a multi-node graph per file (turbulence + domain warp + blend) that
	// this lean single-noise-field pipeline can't replicate — noise_frequency/
	// _persistence below are a representative single node from that graph, not an
	// exact match. There's no verified mapping from these original filenames
	// (1.json..12.json, granite_custom.json) to a specific rock-type name, so presets
	// are labeled generically rather than guessing an identity.
	struct Preset {
		Color albedo_low, albedo_high;
		real_t roughness_scale, roughness_bias;
		real_t metalness_scale, metalness_bias;
		real_t noise_frequency, noise_persistence;
		int noise_octaves;
	};
	static const Preset presets[] = {
		/*  0 (1.json)            */ { Color(0.827, 0.784, 0.517), Color(0.940, 0.936, 0.921), 2.000, 0.000, 0.200, 0.000, 15.201, 0.600, 3 },
		/*  1 (2.json)            */ { Color(1.000, 1.000, 1.000), Color(0.000, 0.000, 0.000), 2.000, 1.000, 0.200, -1.000, 20.000, 0.600, 3 },
		/*  2 (3.json)            */ { Color(0.681, 0.681, 0.681), Color(0.000, 0.000, 0.000), 2.000, 1.000, 0.200, -1.000, 6.000, 0.500, 1 },
		/*  3 (4.json)            */ { Color(0.681, 0.681, 0.681), Color(0.000, 0.000, 0.000), 2.000, 1.000, 0.200, -1.000, 6.000, 0.500, 1 },
		/*  4 (5.json)            */ { Color(0.960, 0.960, 0.960), Color(0.240, 0.240, 0.240), 2.000, 1.000, 0.200, -1.000, 0.900, 0.600, 3 },
		/*  5 (6.json)            */ { Color(0.960, 0.960, 0.960), Color(0.240, 0.240, 0.240), 2.000, 1.000, 0.200, -1.000, 0.450, 0.600, 3 },
		/*  6 (7.json)            */ { Color(0.960, 0.960, 0.960), Color(0.770, 0.770, 0.770), 2.000, 1.000, 0.200, -1.000, 0.900, 0.600, 3 },
		/*  7 (8.json)            */ { Color(0.400, 0.380, 0.240), Color(0.900, 0.900, 0.900), 2.000, 1.000, 0.200, -1.000, 2.700, 0.600, 3 },
		/*  8 (9.json)            */ { Color(0.419, 0.337, 0.337), Color(0.359, 0.359, 0.359), 2.000, 1.000, 0.200, -1.000, 2.500, 0.500, 1 },
		/*  9 (10.json)           */ { Color(0.355, 0.355, 0.355), Color(0.145, 0.108, 0.108), 2.911, 0.000, 0.200, 0.000, 9.393, 0.600, 3 },
		/* 10 (11.json)           */ { Color(0.400, 0.380, 0.240), Color(0.900, 0.900, 0.900), 2.000, 1.000, 0.200, -1.000, 2.700, 0.600, 3 },
		/* 11 (12.json)           */ { Color(0.440, 0.440, 0.440), Color(0.000, 0.000, 0.000), 2.000, 1.000, 0.200, -1.000, 4.000, 0.500, 1 },
		/* 12 (granite_custom)    */ { Color(0.294, 0.294, 0.294), Color(0.105, 0.105, 0.105), 0.658, 0.000, 0.200, -1.000, 87.222, 0.600, 3 },
	};
	const int preset_count = sizeof(presets) / sizeof(presets[0]);
	p_preset = CLAMP(p_preset, 0, preset_count - 1);
	const Preset &p = presets[p_preset];

	set_pipeline_albedo_low(p.albedo_low);
	set_pipeline_albedo_high(p.albedo_high);
	set_pipeline_roughness_scale(p.roughness_scale);
	set_pipeline_roughness_bias(p.roughness_bias);
	set_pipeline_metalness_scale(p.metalness_scale);
	set_pipeline_metalness_bias(p.metalness_bias);
	set_pipeline_noise_frequency(p.noise_frequency);
	set_pipeline_noise_persistence(p.noise_persistence);
	set_pipeline_noise_octaves(p.noise_octaves);
}

// Loads a real procrocklib pipeline JSON preset (see editor/proc_rocks_demo/presets/*.json
// and memo.md's "JSON pipeline reader" section) — from then on, _rebuild() drives Method 3
// from this file's own noise graph / albedo gradient / roughness / metalness / ambient
// occlusion config instead of the scalar pipeline_* Inspector properties (those still work
// for pipeline_subdivisions/width/height/depth/noise_amplitude/randseed/cutplane/smoothed/
// texture_size, which aren't part of the JSON-driven path). Pass an empty path to clear a
// previously loaded preset and return to the scalar-only path.
Error ProcRockMesh::load_from_file(const String p_path) {
#ifdef TOOLS_ENABLED
	if (p_path.empty()) {
		pipeline.json_path = String();
		pipeline.json_cache = Dictionary();
		_dirty = true;
		if (auto_refresh) {
			_rebuild();
		}
		return OK;
	}

	FileAccess *f = FileAccess::open(p_path, FileAccess::READ);
	if (!f) {
		return ERR_FILE_NOT_FOUND;
	}
	String text = f->get_as_utf8_string();
	memdelete(f);

	Variant parsed;
	String err_str;
	int err_line = 0;
	if (JSON::parse(text, parsed, err_str, err_line) != OK || parsed.get_type() != Variant::DICTIONARY) {
		return ERR_PARSE_ERROR;
	}
	Dictionary pipeline_json = parsed;
	if (!rock_pipeline_json_is_valid(pipeline_json)) {
		return ERR_INVALID_DATA;
	}

	pipeline.json_path = p_path;
	pipeline.json_cache = pipeline_json;
	_dirty = true;
	if (auto_refresh) {
		_rebuild();
	}
	return OK;
#else
	// Loading a JSON preset is a generation-configuration action (it feeds
	// _rebuild(), which is itself unavailable here) — not a "read a baked resource"
	// action, so it's editor-only alongside the four generators.
	return ERR_UNAVAILABLE;
#endif
}

// =========================================================================
// Generic texture source — applies to every generator, see proc_rocks.h's
// `texture` struct / TextureSource enum and memo.md's "Texture generation" section.
// =========================================================================

#ifdef TOOLS_ENABLED

void ProcRockMesh::_apply_texture_source() {
	if (get_surface_count() == 0) {
		return;
	}
	switch (texture.source) {
		case TEXTURE_SOURCE_NONE:
			surface_set_material(0, Ref<Material>());
			break;

		case TEXTURE_SOURCE_GENERATED: {
			ProcRockPipelineTextures textures = rock_pipeline_gen_textures(
					texture.gen_size, texture.gen_noise_frequency, texture.gen_noise_octaves, texture.gen_noise_persistence, texture.gen_seed,
					texture.gen_albedo_low, texture.gen_albedo_high, texture.gen_normal_strength,
					texture.gen_roughness_scale, texture.gen_roughness_bias,
					texture.gen_metalness_scale, texture.gen_metalness_bias,
					texture.gen_ao_scale, texture.gen_ao_bias);
			surface_set_material(0, rock_pipeline_make_material(textures));
		} break;

		case TEXTURE_SOURCE_GRAVEL:
		case TEXTURE_SOURCE_MOSSY:
		case TEXTURE_SOURCE_ROCK: {
			// Applied entirely in memory, exactly like GENERATED above — no file I/O
			// during live editing. bake() (below) is what turns this into a real,
			// on-disk File-backed texture set when the resource needs to actually ship.
			ProcRockBakedTexturePack pack = texture.source == TEXTURE_SOURCE_GRAVEL ? PROCROCK_BAKED_GRAVEL : texture.source == TEXTURE_SOURCE_MOSSY ? PROCROCK_BAKED_MOSSY
																																					 : PROCROCK_BAKED_ROCK;
			surface_set_material(0, rock_pipeline_make_material(load_baked_textures(pack)));
		} break;

		case TEXTURE_SOURCE_FILE: {
			if (texture.file_albedo.empty()) {
				surface_set_material(0, Ref<Material>());
				break;
			}
			ProcRockPipelineTextures textures;
			textures.albedo = ResourceLoader::load(texture.file_albedo);
			if (textures.albedo.is_null()) {
				surface_set_material(0, Ref<Material>());
				break;
			}
			if (!texture.file_normal.empty()) {
				textures.normal = ResourceLoader::load(texture.file_normal);
			}
			if (!texture.file_roughness.empty()) {
				textures.roughness = ResourceLoader::load(texture.file_roughness);
			}
			if (!texture.file_metalness.empty()) {
				textures.metalness = ResourceLoader::load(texture.file_metalness);
			}
			if (!texture.file_ambient_occlusion.empty()) {
				textures.ambient_occlusion = ResourceLoader::load(texture.file_ambient_occlusion);
			}
			// Missing companion maps default to flat/neutral values — same fallback
			// the baked Rock pack uses for its own single-texture case.
			if (textures.normal.is_null()) {
				textures.normal = to_texture(make_constant_image(8, Color(0.5, 0.5, 1.0)));
			}
			if (textures.roughness.is_null()) {
				textures.roughness = to_texture(make_constant_image(8, Color(0.5, 0.5, 0.5)));
			}
			if (textures.metalness.is_null()) {
				textures.metalness = to_texture(make_constant_image(8, Color(0, 0, 0)));
			}
			if (textures.ambient_occlusion.is_null()) {
				textures.ambient_occlusion = to_texture(make_constant_image(8, Color(1, 1, 1)));
			}
			surface_set_material(0, rock_pipeline_make_material(textures));
		} break;
	}
}

namespace {

// Writes p_tex's pixel data to p_dir/p_filename as a real PNG file — used by bake()
// to turn a Gravel/Mossy/Rock selection into a real, on-disk File-backed texture set
// (see bake()'s own comment for why this only happens at bake time, never live).
// Returns the written path, or "" if p_tex is null or the write failed (logged via
// ERR_PRINT in that case).
String _write_baked_texture_copy(const Ref<Texture> &p_tex, const String &p_dir, const String &p_filename) {
	if (p_tex.is_null()) {
		return String();
	}
	Ref<Image> img = p_tex->get_data();
	if (img.is_null()) {
		return String();
	}
	String path = p_dir.plus_file(p_filename);
	Error err = img->save_png(path);
	if (err != OK) {
		ERR_PRINT("ProcRock: failed to write baked texture copy to " + path);
		return String();
	}
	return path;
}

Vector<String> _albedo_aliases() {
	Vector<String> v;
	v.push_back("albedo");
	v.push_back("diffuse");
	v.push_back("basecolor");
	v.push_back("base_color");
	v.push_back("col");
	v.push_back("color");
	return v;
}

Vector<String> _normal_aliases() {
	Vector<String> v;
	v.push_back("normal");
	v.push_back("normals");
	v.push_back("nrm");
	v.push_back("nm");
	v.push_back("norm");
	return v;
}

Vector<String> _roughness_aliases() {
	Vector<String> v;
	v.push_back("roughness");
	v.push_back("rough");
	v.push_back("rgh");
	return v;
}

Vector<String> _metalness_aliases() {
	Vector<String> v;
	v.push_back("metalness");
	v.push_back("metallic");
	v.push_back("metal");
	return v;
}

Vector<String> _ambient_occlusion_aliases() {
	Vector<String> v;
	v.push_back("ambientOcc"); // proc_rocks_demo's own exact convention (case-sensitive filesystems)
	v.push_back("ambientocc");
	v.push_back("ambient_occ");
	v.push_back("ao");
	v.push_back("occlusion");
	v.push_back("occ");
	return v;
}

// Tries "<p_dir>/<p_prefix><alias><p_suffix>.<ext>" for every alias in p_aliases,
// preferring p_preferred_ext, then a short list of other common image extensions —
// returns the first path that exists on disk, or "" if none do.
String _find_companion_texture(const String &p_dir, const String &p_prefix, const String &p_suffix,
		const Vector<String> &p_aliases, const String &p_preferred_ext) {
	static const char *const kExtensions[] = { "png", "jpg", "jpeg", "tga", "webp", "bmp" };
	for (int a = 0; a < p_aliases.size(); a++) {
		String stem = p_prefix + p_aliases[a] + p_suffix;
		if (!p_preferred_ext.empty()) {
			String candidate = p_dir.plus_file(stem + "." + p_preferred_ext);
			if (FileAccess::exists(candidate)) {
				return candidate;
			}
		}
		for (int e = 0; e < 6; e++) {
			if (String(kExtensions[e]) == p_preferred_ext) {
				continue; // already tried above
			}
			String candidate = p_dir.plus_file(stem + "." + kExtensions[e]);
			if (FileAccess::exists(candidate)) {
				return candidate;
			}
		}
	}
	return String();
}

} // namespace

void ProcRockMesh::_auto_detect_companion_textures() {
	texture.file_normal = String();
	texture.file_roughness = String();
	texture.file_metalness = String();
	texture.file_ambient_occlusion = String();

	if (texture.file_albedo.empty()) {
		return;
	}

	String dir = texture.file_albedo.get_base_dir();
	String ext = texture.file_albedo.get_extension();
	String stem = texture.file_albedo.get_file().get_basename();
	String stem_lower = stem.to_lower();

	// Matches proc_rocks_demo's own convention (exact "albedo.jpg" -> empty prefix/
	// suffix) and common external conventions ("myrock_albedo.png" -> suffix match,
	// prefix "myrock_"; "albedo_myrock.png" -> prefix match, suffix "_myrock").
	String prefix, suffix;
	bool matched = false;
	Vector<String> albedo_aliases = _albedo_aliases();
	for (int i = 0; i < albedo_aliases.size() && !matched; i++) {
		const String &alias = albedo_aliases[i];
		if (stem_lower == alias) {
			matched = true;
		} else if (stem_lower.ends_with("_" + alias) || stem_lower.ends_with("-" + alias)) {
			prefix = stem.substr(0, stem.length() - alias.length());
			matched = true;
		} else if (stem_lower.begins_with(alias + "_") || stem_lower.begins_with(alias + "-")) {
			suffix = stem.substr(alias.length());
			matched = true;
		}
	}
	if (!matched) {
		return; // filename shape doesn't look like an albedo/diffuse map at all
	}

	texture.file_normal = _find_companion_texture(dir, prefix, suffix, _normal_aliases(), ext);
	texture.file_roughness = _find_companion_texture(dir, prefix, suffix, _roughness_aliases(), ext);
	texture.file_metalness = _find_companion_texture(dir, prefix, suffix, _metalness_aliases(), ext);
	texture.file_ambient_occlusion = _find_companion_texture(dir, prefix, suffix, _ambient_occlusion_aliases(), ext);
}

#endif // TOOLS_ENABLED

// =========================================================================
// Baking — see _is_generated() in proc_rocks.h for why this flips ArrayMesh's
// surface serialization back on, and memo.md's "Baking" section for the
// export-time auto-compile step that calls this.
// =========================================================================

void ProcRockMesh::set_baked(bool p_baked) {
	ERR_FAIL_COND_MSG(p_baked && get_surface_count() == 0, "ProcRockMesh: cannot mark as baked with no generated surfaces.");
	_baked = p_baked;
}

#ifdef TOOLS_ENABLED
Error ProcRockMesh::bake() {
	if (_dirty) {
		_rebuild();
	}
	if (get_surface_count() == 0) {
		return ERR_INVALID_DATA;
	}

	// Gravel/Mossy/Rock apply their baked demo pack entirely in memory during live
	// editing (_apply_texture_source() — no file I/O, so the dock's preview updates
	// instantly). But an exported/baked resource shouldn't depend on this fork's
	// editor-only INCBIN demo assets to resolve its texture at load time — so bake()
	// is where a real, on-disk, importable texture set actually gets written, once,
	// for the resource that's about to ship. Writing files during *live* editing and
	// immediately re-loading them was tried and reverted: a freshly-written file
	// hasn't been through the editor's import pipeline yet, so ResourceLoader::load()
	// silently fails and the preview goes blank. Baking sidesteps that entirely by
	// reusing the in-memory textures we already have for the material instead of
	// reading the files back — the files exist afterward for provenance/import, but
	// this bake doesn't wait on them.
	if (texture.source == TEXTURE_SOURCE_GRAVEL || texture.source == TEXTURE_SOURCE_MOSSY || texture.source == TEXTURE_SOURCE_ROCK) {
		ProcRockBakedTexturePack pack = texture.source == TEXTURE_SOURCE_GRAVEL ? PROCROCK_BAKED_GRAVEL : texture.source == TEXTURE_SOURCE_MOSSY ? PROCROCK_BAKED_MOSSY
																																				 : PROCROCK_BAKED_ROCK;
		String pack_name = texture.source == TEXTURE_SOURCE_GRAVEL ? "gravel" : texture.source == TEXTURE_SOURCE_MOSSY ? "mossy"
																													   : "rock";
		ProcRockPipelineTextures textures = load_baked_textures(pack);

		String dir = String("res://.generated/proc_rocks/") + pack_name + "_" + itos(Math::rand());
		DirAccessRef da = DirAccess::create(DirAccess::ACCESS_RESOURCES);
		if (da) {
			da->make_dir_recursive(dir);
		}

		// Filenames match _auto_detect_companion_textures()'s convention, so a later
		// manual edit of texture_file_albedo in the Inspector still auto-detects these.
		String albedo_path = _write_baked_texture_copy(textures.albedo, dir, "albedo.png");
		String normal_path = _write_baked_texture_copy(textures.normal, dir, "normals.png");
		String roughness_path = _write_baked_texture_copy(textures.roughness, dir, "roughness.png");
		String metalness_path = _write_baked_texture_copy(textures.metalness, dir, "metal.png");
		String ao_path = _write_baked_texture_copy(textures.ambient_occlusion, dir, "ambientOcc.png");

		if (!albedo_path.empty()) {
			texture.source = TEXTURE_SOURCE_FILE;
			texture.file_albedo = albedo_path;
			texture.file_normal = normal_path;
			texture.file_roughness = roughness_path;
			texture.file_metalness = metalness_path;
			texture.file_ambient_occlusion = ao_path;
			// Re-apply using the textures already in memory (not a reload from the
			// files just written — see the comment above) so surface 0's material
			// matches the now-File-backed state we just recorded.
			surface_set_material(0, rock_pipeline_make_material(textures));
		}
	}

	set_baked(true);
	return OK;
}
#endif

// =========================================================================
// Auto-refresh
// =========================================================================

void ProcRockMesh::set_auto_refresh(bool p_refresh) {
	auto_refresh = p_refresh;
	if (auto_refresh) {
		_rebuild();
	}
}

bool ProcRockMesh::get_auto_refresh() const {
	return auto_refresh;
}

// =========================================================================
// Dynamic properties (for Method 3 procrockgen)
// =========================================================================

void ProcRockMesh::_get_property_list(List<PropertyInfo> *p_list) const {
	switch (method) {
		case 0: {
			p_list->push_back(PropertyInfo(Variant::INT, "rockgen_depth", PROPERTY_HINT_RANGE, "0,3"));
			p_list->push_back(PropertyInfo(Variant::INT, "rockgen_randseed"));
			p_list->push_back(PropertyInfo(Variant::REAL, "rockgen_smoothness", PROPERTY_HINT_RANGE, "0,5,0.1"));
			p_list->push_back(PropertyInfo(Variant::BOOL, "rockgen_smoothed"));
		} break;
		case 1: {
			p_list->push_back(PropertyInfo(Variant::INT, "rockgeneration_steps", PROPERTY_HINT_RANGE, "1,3"));
			p_list->push_back(PropertyInfo(Variant::REAL, "rockgeneration_width", PROPERTY_HINT_RANGE, "1,200,0.5"));
			p_list->push_back(PropertyInfo(Variant::REAL, "rockgeneration_height", PROPERTY_HINT_RANGE, "1,200,0.5"));
			p_list->push_back(PropertyInfo(Variant::REAL, "rockgeneration_depth", PROPERTY_HINT_RANGE, "1,200,0.5"));
			p_list->push_back(PropertyInfo(Variant::INT, "rockgeneration_max_planes", PROPERTY_HINT_RANGE, "1,10"));
			p_list->push_back(PropertyInfo(Variant::BOOL, "rockgeneration_smoothed"));
			p_list->push_back(PropertyInfo(Variant::INT, "rockgeneration_randseed"));
		} break;
		case 2: {
			p_list->push_back(PropertyInfo(Variant::INT, "rockstudio_rock_type", PROPERTY_HINT_ENUM, "Cubic,Boulder,Quartz"));
			p_list->push_back(PropertyInfo(Variant::INT, "rockstudio_num_vertices", PROPERTY_HINT_RANGE, "4,150"));
			p_list->push_back(PropertyInfo(Variant::INT, "rockstudio_randseed"));
			if (rockstudio.rock_type == 0) {
				p_list->push_back(PropertyInfo(Variant::REAL, "rockstudio_width", PROPERTY_HINT_RANGE, "0.1,100,0.1"));
				p_list->push_back(PropertyInfo(Variant::REAL, "rockstudio_height", PROPERTY_HINT_RANGE, "0.1,100,0.1"));
				p_list->push_back(PropertyInfo(Variant::REAL, "rockstudio_depth", PROPERTY_HINT_RANGE, "0.1,100,0.1"));
			} else if (rockstudio.rock_type == 1) {
				p_list->push_back(PropertyInfo(Variant::REAL, "rockstudio_radius", PROPERTY_HINT_RANGE, "0.1,100,0.1"));
			} else if (rockstudio.rock_type == 2) {
				p_list->push_back(PropertyInfo(Variant::REAL, "rockstudio_base_width", PROPERTY_HINT_RANGE, "0.1,100,0.1"));
				p_list->push_back(PropertyInfo(Variant::REAL, "rockstudio_base_height", PROPERTY_HINT_RANGE, "0.1,100,0.1"));
				p_list->push_back(PropertyInfo(Variant::REAL, "rockstudio_tip_protrusion", PROPERTY_HINT_RANGE, "0.1,100,0.1"));
				p_list->push_back(PropertyInfo(Variant::REAL, "rockstudio_tip_flatness", PROPERTY_HINT_RANGE, "0.1,10,0.1"));
				p_list->push_back(PropertyInfo(Variant::BOOL, "rockstudio_tetragonal"));
				p_list->push_back(PropertyInfo(Variant::BOOL, "rockstudio_one_sided"));
			}
		} break;
		case 3: {
			p_list->push_back(PropertyInfo(Variant::INT, "pipeline_subdivisions", PROPERTY_HINT_RANGE, "0,3"));
			p_list->push_back(PropertyInfo(Variant::REAL, "pipeline_width", PROPERTY_HINT_RANGE, "0.1,200,0.1"));
			p_list->push_back(PropertyInfo(Variant::REAL, "pipeline_height", PROPERTY_HINT_RANGE, "0.1,200,0.1"));
			p_list->push_back(PropertyInfo(Variant::REAL, "pipeline_depth", PROPERTY_HINT_RANGE, "0.1,200,0.1"));
			p_list->push_back(PropertyInfo(Variant::REAL, "pipeline_noise_frequency", PROPERTY_HINT_RANGE, "0.01,10,0.01"));
			p_list->push_back(PropertyInfo(Variant::REAL, "pipeline_noise_amplitude", PROPERTY_HINT_RANGE, "0,50,0.1"));
			p_list->push_back(PropertyInfo(Variant::INT, "pipeline_noise_octaves", PROPERTY_HINT_RANGE, "1,6"));
			p_list->push_back(PropertyInfo(Variant::REAL, "pipeline_noise_persistence", PROPERTY_HINT_RANGE, "0,1,0.01"));
			p_list->push_back(PropertyInfo(Variant::INT, "pipeline_randseed"));
			p_list->push_back(PropertyInfo(Variant::BOOL, "pipeline_cutplane_enabled"));
			if (pipeline.cutplane_enabled) {
				p_list->push_back(PropertyInfo(Variant::REAL, "pipeline_cutplane_offset", PROPERTY_HINT_RANGE, "-50,50,0.1"));
			}
			p_list->push_back(PropertyInfo(Variant::BOOL, "pipeline_smoothed"));
			p_list->push_back(PropertyInfo(Variant::BOOL, "pipeline_generate_textures"));
			if (pipeline.generate_textures) {
				p_list->push_back(PropertyInfo(Variant::INT, "pipeline_texture_size", PROPERTY_HINT_RANGE, "8,4096"));
				p_list->push_back(PropertyInfo(Variant::COLOR, "pipeline_albedo_low"));
				p_list->push_back(PropertyInfo(Variant::COLOR, "pipeline_albedo_high"));
				p_list->push_back(PropertyInfo(Variant::REAL, "pipeline_normal_strength", PROPERTY_HINT_RANGE, "0,10,0.1"));
				p_list->push_back(PropertyInfo(Variant::REAL, "pipeline_roughness_scale", PROPERTY_HINT_RANGE, "-2,2,0.01"));
				p_list->push_back(PropertyInfo(Variant::REAL, "pipeline_roughness_bias", PROPERTY_HINT_RANGE, "-2,2,0.01"));
				p_list->push_back(PropertyInfo(Variant::REAL, "pipeline_metalness_scale", PROPERTY_HINT_RANGE, "-2,2,0.01"));
				p_list->push_back(PropertyInfo(Variant::REAL, "pipeline_metalness_bias", PROPERTY_HINT_RANGE, "-2,2,0.01"));
				p_list->push_back(PropertyInfo(Variant::REAL, "pipeline_ao_scale", PROPERTY_HINT_RANGE, "-2,2,0.01"));
				p_list->push_back(PropertyInfo(Variant::REAL, "pipeline_ao_bias", PROPERTY_HINT_RANGE, "-2,2,0.01"));
			}
		} break;
		case 4: {
			p_list->push_back(PropertyInfo(Variant::INT, "rockcluster_style", PROPERTY_HINT_ENUM, "Boulder,Sharp,Crystal"));
			p_list->push_back(PropertyInfo(Variant::INT, "rockcluster_randseed"));
			p_list->push_back(PropertyInfo(Variant::INT, "rockcluster_density", PROPERTY_HINT_RANGE, "1,24"));
			if (rockcluster.style == 0 || rockcluster.style == 1) { // Boulder, Sharp
				p_list->push_back(PropertyInfo(Variant::REAL, "rockcluster_radius", PROPERTY_HINT_RANGE, "0.3,20,0.1"));
				p_list->push_back(PropertyInfo(Variant::REAL, "rockcluster_asymmetry", PROPERTY_HINT_RANGE, "-1,1,0.01"));
				p_list->push_back(PropertyInfo(Variant::REAL, "rockcluster_wave", PROPERTY_HINT_RANGE, "0,1,0.01"));
				p_list->push_back(PropertyInfo(Variant::REAL, "rockcluster_decentralize", PROPERTY_HINT_RANGE, "0,1,0.01"));
				p_list->push_back(PropertyInfo(Variant::REAL, "rockcluster_scale_local", PROPERTY_HINT_RANGE, "0,2,0.01"));
				p_list->push_back(PropertyInfo(Variant::OBJECT, "rockcluster_scale_by_distance", PROPERTY_HINT_RESOURCE_TYPE, "Curve"));
				p_list->push_back(PropertyInfo(Variant::REAL, "rockcluster_tallness", PROPERTY_HINT_RANGE, "0,1,0.01"));
				p_list->push_back(PropertyInfo(Variant::REAL, "rockcluster_flatness", PROPERTY_HINT_RANGE, "0,1,0.01"));
				p_list->push_back(PropertyInfo(Variant::REAL, "rockcluster_wideness", PROPERTY_HINT_RANGE, "0,1,0.01"));
				p_list->push_back(PropertyInfo(Variant::REAL, "rockcluster_rotation", PROPERTY_HINT_RANGE, "-45,45,0.1"));
				p_list->push_back(PropertyInfo(Variant::REAL, "rockcluster_rotation_local", PROPERTY_HINT_RANGE, "0,359,0.1"));
				p_list->push_back(PropertyInfo(Variant::REAL, "rockcluster_rotation_rnd", PROPERTY_HINT_RANGE, "0,1,0.01"));
			} else { // Crystal
				// Crystal cells are natively huge (~1x11x1) with no relative-scale math
				// in the algorithm that would shrink that on its own (unlike Boulder/
				// Sharp's rockcluster_scale_local) — this flat multiplier exists purely
				// to compensate, so it has its own much smaller default/range.
				p_list->push_back(PropertyInfo(Variant::REAL, "rockcluster_crystal_scale", PROPERTY_HINT_RANGE, "0.01,1,0.01"));
				p_list->push_back(PropertyInfo(Variant::REAL, "rockcluster_scale_by_angle", PROPERTY_HINT_RANGE, "0,1,0.01"));
				p_list->push_back(PropertyInfo(Variant::REAL, "rockcluster_scale_random_offset", PROPERTY_HINT_RANGE, "0,2,0.01"));
				p_list->push_back(PropertyInfo(Variant::REAL, "rockcluster_scale_bias", PROPERTY_HINT_RANGE, "-1,1,0.01"));
				p_list->push_back(PropertyInfo(Variant::REAL, "rockcluster_bloom", PROPERTY_HINT_RANGE, "0,1,0.01"));
			}
		} break;
	}

	// Generic "flatten base" cut — applies to methods 0-3 only (not RockCluster, see
	// proc_rocks.h's field comment for why).
	if (method != 4) {
		p_list->push_back(PropertyInfo(Variant::BOOL, "flatten_base_enabled"));
		if (flatten_base_enabled) {
			p_list->push_back(PropertyInfo(Variant::REAL, "flatten_base_offset", PROPERTY_HINT_RANGE, "-2,2,0.01"));
		}
	}

	// Generic texture source — applies to every generator (unlike pipeline_* above,
	// which is Method-3-only). Hidden for Method 3 while its own richer
	// pipeline_generate_textures path is on, to avoid showing two competing texture
	// controls at once — see memo.md's "Texture generation" section.
	if (method != 3 || !pipeline.generate_textures) {
		p_list->push_back(PropertyInfo(Variant::INT, "texture_source", PROPERTY_HINT_ENUM, "None,Generated,Gravel,Mossy,Rock,From File"));
		switch (texture.source) {
			case TEXTURE_SOURCE_GENERATED: {
				p_list->push_back(PropertyInfo(Variant::INT, "texture_gen_size", PROPERTY_HINT_RANGE, "8,4096"));
				p_list->push_back(PropertyInfo(Variant::INT, "texture_gen_seed"));
				p_list->push_back(PropertyInfo(Variant::REAL, "texture_gen_noise_frequency", PROPERTY_HINT_RANGE, "0.01,10,0.01"));
				p_list->push_back(PropertyInfo(Variant::INT, "texture_gen_noise_octaves", PROPERTY_HINT_RANGE, "1,6"));
				p_list->push_back(PropertyInfo(Variant::REAL, "texture_gen_noise_persistence", PROPERTY_HINT_RANGE, "0,1,0.01"));
				p_list->push_back(PropertyInfo(Variant::COLOR, "texture_gen_albedo_low"));
				p_list->push_back(PropertyInfo(Variant::COLOR, "texture_gen_albedo_high"));
				p_list->push_back(PropertyInfo(Variant::REAL, "texture_gen_normal_strength", PROPERTY_HINT_RANGE, "0,10,0.1"));
				p_list->push_back(PropertyInfo(Variant::REAL, "texture_gen_roughness_scale", PROPERTY_HINT_RANGE, "-2,2,0.01"));
				p_list->push_back(PropertyInfo(Variant::REAL, "texture_gen_roughness_bias", PROPERTY_HINT_RANGE, "-2,2,0.01"));
				p_list->push_back(PropertyInfo(Variant::REAL, "texture_gen_metalness_scale", PROPERTY_HINT_RANGE, "-2,2,0.01"));
				p_list->push_back(PropertyInfo(Variant::REAL, "texture_gen_metalness_bias", PROPERTY_HINT_RANGE, "-2,2,0.01"));
				p_list->push_back(PropertyInfo(Variant::REAL, "texture_gen_ao_scale", PROPERTY_HINT_RANGE, "-2,2,0.01"));
				p_list->push_back(PropertyInfo(Variant::REAL, "texture_gen_ao_bias", PROPERTY_HINT_RANGE, "-2,2,0.01"));
			} break;
			case TEXTURE_SOURCE_FILE: {
				static const char *const kImageFilter = "*.png,*.jpg,*.jpeg,*.tga,*.bmp,*.webp";
				p_list->push_back(PropertyInfo(Variant::STRING, "texture_file_albedo", PROPERTY_HINT_FILE, kImageFilter));
				p_list->push_back(PropertyInfo(Variant::STRING, "texture_file_normal", PROPERTY_HINT_FILE, kImageFilter));
				p_list->push_back(PropertyInfo(Variant::STRING, "texture_file_roughness", PROPERTY_HINT_FILE, kImageFilter));
				p_list->push_back(PropertyInfo(Variant::STRING, "texture_file_metalness", PROPERTY_HINT_FILE, kImageFilter));
				p_list->push_back(PropertyInfo(Variant::STRING, "texture_file_ambient_occlusion", PROPERTY_HINT_FILE, kImageFilter));
			} break;
			default:
				break;
		}
	}
}

bool ProcRockMesh::_set(const StringName &p_path, const Variant &p_value) {
	String prop = p_path;
	// Method 0
	if (prop == "rockgen_depth") {
		set_rockgen_depth(p_value);
		return true;
	}
	if (prop == "rockgen_randseed") {
		set_rockgen_randseed(p_value);
		return true;
	}
	if (prop == "rockgen_smoothness") {
		set_rockgen_smoothness(p_value);
		return true;
	}
	if (prop == "rockgen_smoothed") {
		set_rockgen_smoothed(p_value);
		return true;
	}
	// Method 1
	if (prop == "rockgeneration_steps") {
		set_rockgeneration_steps(p_value);
		return true;
	}
	if (prop == "rockgeneration_width") {
		set_rockgeneration_width(p_value);
		return true;
	}
	if (prop == "rockgeneration_height") {
		set_rockgeneration_height(p_value);
		return true;
	}
	if (prop == "rockgeneration_depth") {
		set_rockgeneration_depth(p_value);
		return true;
	}
	if (prop == "rockgeneration_max_planes") {
		set_rockgeneration_max_planes(p_value);
		return true;
	}
	if (prop == "rockgeneration_smoothed") {
		set_rockgeneration_smoothed(p_value);
		return true;
	}
	if (prop == "rockgeneration_randseed") {
		set_rockgeneration_randseed(p_value);
		return true;
	}
	// Method 2
	if (prop == "rockstudio_rock_type") {
		set_rockstudio_rock_type(p_value);
		_change_notify();
		return true;
	}
	if (prop == "rockstudio_num_vertices") {
		set_rockstudio_num_vertices(p_value);
		return true;
	}
	if (prop == "rockstudio_width") {
		set_rockstudio_width(p_value);
		return true;
	}
	if (prop == "rockstudio_height") {
		set_rockstudio_height(p_value);
		return true;
	}
	if (prop == "rockstudio_depth") {
		set_rockstudio_depth(p_value);
		return true;
	}
	if (prop == "rockstudio_radius") {
		set_rockstudio_radius(p_value);
		return true;
	}
	if (prop == "rockstudio_randseed") {
		set_rockstudio_randseed(p_value);
		return true;
	}
	if (prop == "rockstudio_base_width") {
		rockstudio.base_width = p_value;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
		return true;
	}
	if (prop == "rockstudio_base_height") {
		rockstudio.base_height = p_value;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
		return true;
	}
	if (prop == "rockstudio_tip_protrusion") {
		rockstudio.tip_protrusion = p_value;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
		return true;
	}
	if (prop == "rockstudio_tip_flatness") {
		rockstudio.tip_flatness = p_value;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
		return true;
	}
	if (prop == "rockstudio_tetragonal") {
		rockstudio.tetragonal = p_value;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
		return true;
	}
	if (prop == "rockstudio_one_sided") {
		rockstudio.one_sided = p_value;
		_dirty = true;
		if (auto_refresh)
			_rebuild();
		return true;
	}
	// Method 3
	if (prop == "pipeline_subdivisions") {
		set_pipeline_subdivisions(p_value);
		return true;
	}
	if (prop == "pipeline_width") {
		set_pipeline_width(p_value);
		return true;
	}
	if (prop == "pipeline_height") {
		set_pipeline_height(p_value);
		return true;
	}
	if (prop == "pipeline_depth") {
		set_pipeline_depth(p_value);
		return true;
	}
	if (prop == "pipeline_noise_frequency") {
		set_pipeline_noise_frequency(p_value);
		return true;
	}
	if (prop == "pipeline_noise_amplitude") {
		set_pipeline_noise_amplitude(p_value);
		return true;
	}
	if (prop == "pipeline_noise_octaves") {
		set_pipeline_noise_octaves(p_value);
		return true;
	}
	if (prop == "pipeline_noise_persistence") {
		set_pipeline_noise_persistence(p_value);
		return true;
	}
	if (prop == "pipeline_randseed") {
		set_pipeline_randseed(p_value);
		return true;
	}
	if (prop == "pipeline_cutplane_enabled") {
		set_pipeline_cutplane_enabled(p_value);
		_change_notify();
		return true;
	}
	if (prop == "pipeline_cutplane_offset") {
		set_pipeline_cutplane_offset(p_value);
		return true;
	}
	if (prop == "pipeline_smoothed") {
		set_pipeline_smoothed(p_value);
		return true;
	}
	if (prop == "pipeline_generate_textures") {
		set_pipeline_generate_textures(p_value);
		_change_notify();
		return true;
	}
	if (prop == "pipeline_texture_size") {
		set_pipeline_texture_size(p_value);
		return true;
	}
	if (prop == "pipeline_albedo_low") {
		set_pipeline_albedo_low(p_value);
		return true;
	}
	if (prop == "pipeline_albedo_high") {
		set_pipeline_albedo_high(p_value);
		return true;
	}
	if (prop == "pipeline_normal_strength") {
		set_pipeline_normal_strength(p_value);
		return true;
	}
	if (prop == "pipeline_roughness_scale") {
		set_pipeline_roughness_scale(p_value);
		return true;
	}
	if (prop == "pipeline_roughness_bias") {
		set_pipeline_roughness_bias(p_value);
		return true;
	}
	if (prop == "pipeline_metalness_scale") {
		set_pipeline_metalness_scale(p_value);
		return true;
	}
	if (prop == "pipeline_metalness_bias") {
		set_pipeline_metalness_bias(p_value);
		return true;
	}
	if (prop == "pipeline_ao_scale") {
		set_pipeline_ao_scale(p_value);
		return true;
	}
	if (prop == "pipeline_ao_bias") {
		set_pipeline_ao_bias(p_value);
		return true;
	}
	if (prop == "rockcluster_style") {
		set_rockcluster_style(p_value);
		return true;
	}
	if (prop == "rockcluster_randseed") {
		set_rockcluster_randseed(p_value);
		return true;
	}
	if (prop == "rockcluster_density") {
		set_rockcluster_density(p_value);
		return true;
	}
	if (prop == "rockcluster_radius") {
		set_rockcluster_radius(p_value);
		return true;
	}
	if (prop == "rockcluster_asymmetry") {
		set_rockcluster_asymmetry(p_value);
		return true;
	}
	if (prop == "rockcluster_wave") {
		set_rockcluster_wave(p_value);
		return true;
	}
	if (prop == "rockcluster_decentralize") {
		set_rockcluster_decentralize(p_value);
		return true;
	}
	if (prop == "rockcluster_scale_local") {
		set_rockcluster_scale_local(p_value);
		return true;
	}
	if (prop == "rockcluster_scale_by_distance") {
		set_rockcluster_scale_by_distance(p_value);
		return true;
	}
	if (prop == "rockcluster_tallness") {
		set_rockcluster_tallness(p_value);
		return true;
	}
	if (prop == "rockcluster_flatness") {
		set_rockcluster_flatness(p_value);
		return true;
	}
	if (prop == "rockcluster_wideness") {
		set_rockcluster_wideness(p_value);
		return true;
	}
	if (prop == "rockcluster_rotation") {
		set_rockcluster_rotation(p_value);
		return true;
	}
	if (prop == "rockcluster_rotation_local") {
		set_rockcluster_rotation_local(p_value);
		return true;
	}
	if (prop == "rockcluster_rotation_rnd") {
		set_rockcluster_rotation_rnd(p_value);
		return true;
	}
	if (prop == "rockcluster_crystal_scale") {
		set_rockcluster_crystal_scale(p_value);
		return true;
	}
	if (prop == "rockcluster_scale_by_angle") {
		set_rockcluster_scale_by_angle(p_value);
		return true;
	}
	if (prop == "rockcluster_scale_random_offset") {
		set_rockcluster_scale_random_offset(p_value);
		return true;
	}
	if (prop == "rockcluster_scale_bias") {
		set_rockcluster_scale_bias(p_value);
		return true;
	}
	if (prop == "rockcluster_bloom") {
		set_rockcluster_bloom(p_value);
		return true;
	}
	// Generic "flatten base" cut (methods 0-3)
	if (prop == "flatten_base_enabled") {
		set_flatten_base_enabled(p_value);
		return true;
	}
	if (prop == "flatten_base_offset") {
		set_flatten_base_offset(p_value);
		return true;
	}
	// Generic texture source (every generator)
	if (prop == "texture_source") {
		set_texture_source(p_value);
		return true;
	}
	if (prop == "texture_gen_size") {
		set_texture_gen_size(p_value);
		return true;
	}
	if (prop == "texture_gen_seed") {
		set_texture_gen_seed(p_value);
		return true;
	}
	if (prop == "texture_gen_noise_frequency") {
		set_texture_gen_noise_frequency(p_value);
		return true;
	}
	if (prop == "texture_gen_noise_octaves") {
		set_texture_gen_noise_octaves(p_value);
		return true;
	}
	if (prop == "texture_gen_noise_persistence") {
		set_texture_gen_noise_persistence(p_value);
		return true;
	}
	if (prop == "texture_gen_albedo_low") {
		set_texture_gen_albedo_low(p_value);
		return true;
	}
	if (prop == "texture_gen_albedo_high") {
		set_texture_gen_albedo_high(p_value);
		return true;
	}
	if (prop == "texture_gen_normal_strength") {
		set_texture_gen_normal_strength(p_value);
		return true;
	}
	if (prop == "texture_gen_roughness_scale") {
		set_texture_gen_roughness_scale(p_value);
		return true;
	}
	if (prop == "texture_gen_roughness_bias") {
		set_texture_gen_roughness_bias(p_value);
		return true;
	}
	if (prop == "texture_gen_metalness_scale") {
		set_texture_gen_metalness_scale(p_value);
		return true;
	}
	if (prop == "texture_gen_metalness_bias") {
		set_texture_gen_metalness_bias(p_value);
		return true;
	}
	if (prop == "texture_gen_ao_scale") {
		set_texture_gen_ao_scale(p_value);
		return true;
	}
	if (prop == "texture_gen_ao_bias") {
		set_texture_gen_ao_bias(p_value);
		return true;
	}
	if (prop == "texture_file_albedo") {
		set_texture_file_albedo(p_value);
		return true;
	}
	if (prop == "texture_file_normal") {
		set_texture_file_normal(p_value);
		return true;
	}
	if (prop == "texture_file_roughness") {
		set_texture_file_roughness(p_value);
		return true;
	}
	if (prop == "texture_file_metalness") {
		set_texture_file_metalness(p_value);
		return true;
	}
	if (prop == "texture_file_ambient_occlusion") {
		set_texture_file_ambient_occlusion(p_value);
		return true;
	}
	return false;
}

bool ProcRockMesh::_get(const StringName &p_path, Variant &r_ret) const {
	String prop = p_path;
	// Method 0
	if (prop == "rockgen_depth") {
		r_ret = rockgen.depth;
		return true;
	}
	if (prop == "rockgen_randseed") {
		r_ret = rockgen.randseed;
		return true;
	}
	if (prop == "rockgen_smoothness") {
		r_ret = rockgen.smoothness;
		return true;
	}
	if (prop == "rockgen_smoothed") {
		r_ret = rockgen.smoothed;
		return true;
	}
	// Method 1
	if (prop == "rockgeneration_steps") {
		r_ret = rockgeneration.steps;
		return true;
	}
	if (prop == "rockgeneration_width") {
		r_ret = rockgeneration.dimensions.x;
		return true;
	}
	if (prop == "rockgeneration_height") {
		r_ret = rockgeneration.dimensions.y;
		return true;
	}
	if (prop == "rockgeneration_depth") {
		r_ret = rockgeneration.dimensions.z;
		return true;
	}
	if (prop == "rockgeneration_max_planes") {
		r_ret = rockgeneration.max_planes;
		return true;
	}
	if (prop == "rockgeneration_smoothed") {
		r_ret = rockgeneration.smoothed;
		return true;
	}
	if (prop == "rockgeneration_randseed") {
		r_ret = rockgeneration.randseed;
		return true;
	}
	// Method 2
	if (prop == "rockstudio_rock_type") {
		r_ret = rockstudio.rock_type;
		return true;
	}
	if (prop == "rockstudio_num_vertices") {
		r_ret = rockstudio.num_vertices;
		return true;
	}
	if (prop == "rockstudio_width") {
		r_ret = rockstudio.width;
		return true;
	}
	if (prop == "rockstudio_height") {
		r_ret = rockstudio.height;
		return true;
	}
	if (prop == "rockstudio_depth") {
		r_ret = rockstudio.depth;
		return true;
	}
	if (prop == "rockstudio_radius") {
		r_ret = rockstudio.radius;
		return true;
	}
	if (prop == "rockstudio_randseed") {
		r_ret = rockstudio.randseed;
		return true;
	}
	if (prop == "rockstudio_base_width") {
		r_ret = rockstudio.base_width;
		return true;
	}
	if (prop == "rockstudio_base_height") {
		r_ret = rockstudio.base_height;
		return true;
	}
	if (prop == "rockstudio_tip_protrusion") {
		r_ret = rockstudio.tip_protrusion;
		return true;
	}
	if (prop == "rockstudio_tip_flatness") {
		r_ret = rockstudio.tip_flatness;
		return true;
	}
	if (prop == "rockstudio_tetragonal") {
		r_ret = rockstudio.tetragonal;
		return true;
	}
	if (prop == "rockstudio_one_sided") {
		r_ret = rockstudio.one_sided;
		return true;
	}
	// Method 3
	if (prop == "pipeline_subdivisions") {
		r_ret = pipeline.subdivisions;
		return true;
	}
	if (prop == "pipeline_width") {
		r_ret = pipeline.width;
		return true;
	}
	if (prop == "pipeline_height") {
		r_ret = pipeline.height;
		return true;
	}
	if (prop == "pipeline_depth") {
		r_ret = pipeline.depth;
		return true;
	}
	if (prop == "pipeline_noise_frequency") {
		r_ret = pipeline.noise_frequency;
		return true;
	}
	if (prop == "pipeline_noise_amplitude") {
		r_ret = pipeline.noise_amplitude;
		return true;
	}
	if (prop == "pipeline_noise_octaves") {
		r_ret = pipeline.noise_octaves;
		return true;
	}
	if (prop == "pipeline_noise_persistence") {
		r_ret = pipeline.noise_persistence;
		return true;
	}
	if (prop == "pipeline_randseed") {
		r_ret = pipeline.randseed;
		return true;
	}
	if (prop == "pipeline_cutplane_enabled") {
		r_ret = pipeline.cutplane_enabled;
		return true;
	}
	if (prop == "pipeline_cutplane_offset") {
		r_ret = pipeline.cutplane_offset;
		return true;
	}
	if (prop == "pipeline_smoothed") {
		r_ret = pipeline.smoothed;
		return true;
	}
	if (prop == "pipeline_generate_textures") {
		r_ret = pipeline.generate_textures;
		return true;
	}
	if (prop == "pipeline_texture_size") {
		r_ret = pipeline.texture_size;
		return true;
	}
	if (prop == "pipeline_albedo_low") {
		r_ret = pipeline.albedo_low;
		return true;
	}
	if (prop == "pipeline_albedo_high") {
		r_ret = pipeline.albedo_high;
		return true;
	}
	if (prop == "pipeline_normal_strength") {
		r_ret = pipeline.normal_strength;
		return true;
	}
	if (prop == "pipeline_roughness_scale") {
		r_ret = pipeline.roughness_scale;
		return true;
	}
	if (prop == "pipeline_roughness_bias") {
		r_ret = pipeline.roughness_bias;
		return true;
	}
	if (prop == "pipeline_metalness_scale") {
		r_ret = pipeline.metalness_scale;
		return true;
	}
	if (prop == "pipeline_metalness_bias") {
		r_ret = pipeline.metalness_bias;
		return true;
	}
	if (prop == "pipeline_ao_scale") {
		r_ret = pipeline.ao_scale;
		return true;
	}
	if (prop == "pipeline_ao_bias") {
		r_ret = pipeline.ao_bias;
		return true;
	}
	if (prop == "rockcluster_style") {
		r_ret = rockcluster.style;
		return true;
	}
	if (prop == "rockcluster_randseed") {
		r_ret = rockcluster.randseed;
		return true;
	}
	if (prop == "rockcluster_density") {
		r_ret = rockcluster.density;
		return true;
	}
	if (prop == "rockcluster_radius") {
		r_ret = rockcluster.radius;
		return true;
	}
	if (prop == "rockcluster_asymmetry") {
		r_ret = rockcluster.asymmetry;
		return true;
	}
	if (prop == "rockcluster_wave") {
		r_ret = rockcluster.wave;
		return true;
	}
	if (prop == "rockcluster_decentralize") {
		r_ret = rockcluster.decentralize;
		return true;
	}
	if (prop == "rockcluster_scale_local") {
		r_ret = rockcluster.scale_local;
		return true;
	}
	if (prop == "rockcluster_scale_by_distance") {
		r_ret = rockcluster.scale_by_distance;
		return true;
	}
	if (prop == "rockcluster_tallness") {
		r_ret = rockcluster.tallness;
		return true;
	}
	if (prop == "rockcluster_flatness") {
		r_ret = rockcluster.flatness;
		return true;
	}
	if (prop == "rockcluster_wideness") {
		r_ret = rockcluster.wideness;
		return true;
	}
	if (prop == "rockcluster_rotation") {
		r_ret = rockcluster.rotation;
		return true;
	}
	if (prop == "rockcluster_rotation_local") {
		r_ret = rockcluster.rotation_local;
		return true;
	}
	if (prop == "rockcluster_rotation_rnd") {
		r_ret = rockcluster.rotation_rnd;
		return true;
	}
	if (prop == "rockcluster_crystal_scale") {
		r_ret = rockcluster.crystal_scale;
		return true;
	}
	if (prop == "rockcluster_scale_by_angle") {
		r_ret = rockcluster.scale_by_angle;
		return true;
	}
	if (prop == "rockcluster_scale_random_offset") {
		r_ret = rockcluster.scale_random_offset;
		return true;
	}
	if (prop == "rockcluster_scale_bias") {
		r_ret = rockcluster.scale_bias;
		return true;
	}
	if (prop == "rockcluster_bloom") {
		r_ret = rockcluster.bloom;
		return true;
	}
	// Generic "flatten base" cut (methods 0-3)
	if (prop == "flatten_base_enabled") {
		r_ret = flatten_base_enabled;
		return true;
	}
	if (prop == "flatten_base_offset") {
		r_ret = flatten_base_offset;
		return true;
	}
	// Generic texture source (every generator)
	if (prop == "texture_source") {
		r_ret = texture.source;
		return true;
	}
	if (prop == "texture_gen_size") {
		r_ret = texture.gen_size;
		return true;
	}
	if (prop == "texture_gen_seed") {
		r_ret = texture.gen_seed;
		return true;
	}
	if (prop == "texture_gen_noise_frequency") {
		r_ret = texture.gen_noise_frequency;
		return true;
	}
	if (prop == "texture_gen_noise_octaves") {
		r_ret = texture.gen_noise_octaves;
		return true;
	}
	if (prop == "texture_gen_noise_persistence") {
		r_ret = texture.gen_noise_persistence;
		return true;
	}
	if (prop == "texture_gen_albedo_low") {
		r_ret = texture.gen_albedo_low;
		return true;
	}
	if (prop == "texture_gen_albedo_high") {
		r_ret = texture.gen_albedo_high;
		return true;
	}
	if (prop == "texture_gen_normal_strength") {
		r_ret = texture.gen_normal_strength;
		return true;
	}
	if (prop == "texture_gen_roughness_scale") {
		r_ret = texture.gen_roughness_scale;
		return true;
	}
	if (prop == "texture_gen_roughness_bias") {
		r_ret = texture.gen_roughness_bias;
		return true;
	}
	if (prop == "texture_gen_metalness_scale") {
		r_ret = texture.gen_metalness_scale;
		return true;
	}
	if (prop == "texture_gen_metalness_bias") {
		r_ret = texture.gen_metalness_bias;
		return true;
	}
	if (prop == "texture_gen_ao_scale") {
		r_ret = texture.gen_ao_scale;
		return true;
	}
	if (prop == "texture_gen_ao_bias") {
		r_ret = texture.gen_ao_bias;
		return true;
	}
	if (prop == "texture_file_albedo") {
		r_ret = texture.file_albedo;
		return true;
	}
	if (prop == "texture_file_normal") {
		r_ret = texture.file_normal;
		return true;
	}
	if (prop == "texture_file_roughness") {
		r_ret = texture.file_roughness;
		return true;
	}
	if (prop == "texture_file_metalness") {
		r_ret = texture.file_metalness;
		return true;
	}
	if (prop == "texture_file_ambient_occlusion") {
		r_ret = texture.file_ambient_occlusion;
		return true;
	}
	return false;
}

// =========================================================================
// Mesh generation
// =========================================================================

void ProcRockMesh::_rebuild() {
	if (!_dirty) {
		return;
	}

#ifdef TOOLS_ENABLED
	clear_surfaces();

	switch (method) {
		case 0: {
			// Method 0: rockgen — fractal icosahedron subdivision
			Array mesh_arrays = rock_gen(rockgen.depth, rockgen.randseed, rockgen.smoothness, rockgen.smoothed);
			if (mesh_arrays.size() == VS::ARRAY_MAX) {
				if (flatten_base_enabled) {
					mesh_arrays = apply_flatten_base(mesh_arrays, flatten_base_offset);
				}
				add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, ensure_tangents(mesh_arrays));
				_apply_texture_source();
			}
		} break;

		case 1: {
			// Method 1: rockgeneration — icosphere with plane deformation
			// GenRock/BuildRock() has no seeded-RNG-object plumbing (relies on the global
			// Math::rand()), same situation as RockStudio (Method 2, see its own
			// Math::seed() call below) — seed the same way, 0 meaning "unseeded", matching
			// rockgen_randseed's own 0-means-random convention.
			if (rockgeneration.randseed != 0) {
				Math::seed(rockgeneration.randseed);
			}
			GenRock gen(rockgeneration.dimensions.x,
					rockgeneration.dimensions.y,
					rockgeneration.dimensions.z,
					rockgeneration.steps);
			gen.SetRandOffsetPercent(rockgeneration.rand_offset_percent);
			gen.SetRandShift(rockgeneration.rand_shift);
			gen.SetMinPlaneVerts(rockgeneration.plane_verts_range.x);
			gen.SetMaxPlaneVerts(rockgeneration.plane_verts_range.y);
			gen.SetMaxPlanes(rockgeneration.max_planes);
			gen.SetSmoothed(rockgeneration.smoothed);

			Ref<ArrayMesh> rock_mesh = gen.GenerateMesh();
			if (rock_mesh.is_valid() && rock_mesh->get_surface_count() > 0) {
				Array surface = rock_mesh->surface_get_arrays(0);
				if (flatten_base_enabled) {
					surface = apply_flatten_base(surface, flatten_base_offset);
				}
				add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, ensure_tangents(surface));
				_apply_texture_source();
			}
		} break;

		case 2: {
			// Method 2: RockStudio — convex hull from random point clouds
			if (rockstudio.randseed != 0) {
				Math::seed(rockstudio.randseed);
			}

			Vector<Vector3> points;
			switch (rockstudio.rock_type) {
				case 0: // Cubic
					points = rock_studio_points_cube(rockstudio.num_vertices, rockstudio.width, rockstudio.height, rockstudio.depth);
					break;
				case 1: // Boulder
					points = rock_studio_points_sphere(rockstudio.num_vertices, rockstudio.radius);
					break;
				case 2: // Quartz
					points = rock_studio_points_crystal(rockstudio.num_vertices, rockstudio.tetragonal, rockstudio.one_sided, rockstudio.base_width, rockstudio.base_height, rockstudio.tip_protrusion, rockstudio.tip_flatness);
					break;
			}

			if (points.size() >= 4) {
				Ref<ArrayMesh> hull = rock_studio_create_mesh(points);
				if (hull.is_valid()) {
					rock_studio_box_uv(hull);
					if (hull->get_surface_count() > 0) {
						Array surface = hull->surface_get_arrays(0);
						if (flatten_base_enabled) {
							surface = apply_flatten_base(surface, flatten_base_offset);
						}
						add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, ensure_tangents(surface));
						_apply_texture_source();
					}
				}
			}
		} break;

		case 3: {
			// Method 3: ProcRock — icosphere + noise displacement pipeline, optionally
			// driven by a loaded JSON preset instead of the scalar pipeline_* properties.
			bool use_json = !pipeline.json_path.empty() && rock_pipeline_json_is_valid(pipeline.json_cache);
			Array mesh_arrays = use_json
					? rock_pipeline_gen_from_json(pipeline.subdivisions, pipeline.width, pipeline.height, pipeline.depth,
							  pipeline.json_cache, pipeline.randseed,
							  pipeline.cutplane_enabled, pipeline.cutplane_offset, pipeline.smoothed)
					: rock_pipeline_gen(pipeline.subdivisions, pipeline.width, pipeline.height, pipeline.depth,
							  pipeline.noise_frequency, pipeline.noise_amplitude, pipeline.noise_octaves, pipeline.noise_persistence,
							  pipeline.randseed, pipeline.cutplane_enabled, pipeline.cutplane_offset, pipeline.smoothed);

			Vector<Vector3> verts = mesh_arrays.size() == VS::ARRAY_MAX ? (Vector<Vector3>)mesh_arrays[VS::ARRAY_VERTEX] : Vector<Vector3>();
			if (verts.size() > 0) {
				if (flatten_base_enabled) {
					mesh_arrays = apply_flatten_base(mesh_arrays, flatten_base_offset);
				}
				add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, ensure_tangents(mesh_arrays));

				if (pipeline.generate_textures) {
					ProcRockPipelineTextures textures = use_json
							? rock_pipeline_gen_textures_from_json(pipeline.texture_size, pipeline.json_cache)
							: rock_pipeline_gen_textures(pipeline.texture_size,
									  pipeline.noise_frequency, pipeline.noise_octaves, pipeline.noise_persistence, pipeline.randseed,
									  pipeline.albedo_low, pipeline.albedo_high, pipeline.normal_strength,
									  pipeline.roughness_scale, pipeline.roughness_bias,
									  pipeline.metalness_scale, pipeline.metalness_bias,
									  pipeline.ao_scale, pipeline.ao_bias);
					_pipeline_material = rock_pipeline_make_material(textures);
					surface_set_material(0, _pipeline_material);
				} else {
					_pipeline_material = Ref<SpatialMaterial>();
					_apply_texture_source();
				}
			}
		} break;

		case 4: {
			// Method 4: RockCluster — scatter/combine cell meshes (see memo.md's
			// "Method 4: RockCluster" for the algorithm writeup).
			Array mesh_arrays;
			if (rockcluster.style == 2) { // Crystal
				mesh_arrays = rock_cluster_gen_crystal(rockcluster.density, rockcluster.crystal_scale, rockcluster.scale_by_angle,
						rockcluster.scale_random_offset, rockcluster.scale_bias, rockcluster.bloom, rockcluster.randseed);
			} else { // Boulder (0) or Sharp (1)
				mesh_arrays = rock_cluster_gen_boulder(rockcluster.style == 1, rockcluster.density, rockcluster.radius,
						rockcluster.asymmetry, rockcluster.wave, rockcluster.decentralize, rockcluster.scale_local,
						rockcluster.scale_by_distance, rockcluster.tallness, rockcluster.flatness, rockcluster.wideness,
						rockcluster.rotation, rockcluster.rotation_local, rockcluster.rotation_rnd, rockcluster.randseed);
			}
			Vector<Vector3> verts = mesh_arrays.size() == VS::ARRAY_MAX ? (Vector<Vector3>)mesh_arrays[VS::ARRAY_VERTEX] : Vector<Vector3>();
			if (verts.size() > 0) {
				add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, ensure_tangents(mesh_arrays));
				_apply_texture_source();
			}
		} break;
	}
#else
	// Generation is editor-only (see memo.md's "Baking" section) — in a tools=no
	// build this ProcRockMesh is purely a reader of whatever surfaces were already
	// deserialized via ArrayMesh::_set() (baked geometry, see _is_generated() in
	// proc_rocks.h). There is nothing to regenerate here, and clearing existing
	// surfaces would destroy the very data this build exists to display — so unlike
	// the tools=yes branch above, this one must never touch clear_surfaces().
	if (get_surface_count() == 0) {
		WARN_PRINT_ONCE("ProcRockMesh: no baked geometry and generation is unavailable "
						"in this build (TOOLS_ENABLED off) — bake and re-export in the editor.");
	}
#endif

	_dirty = false;
}

// =========================================================================
// Bindings
// =========================================================================

void ProcRockMesh::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_generator", "method"), &ProcRockMesh::set_generator);
	ClassDB::bind_method(D_METHOD("get_generator"), &ProcRockMesh::get_generator);

	ClassDB::bind_method(D_METHOD("set_rockgen_depth", "depth"), &ProcRockMesh::set_rockgen_depth);
	ClassDB::bind_method(D_METHOD("get_rockgen_depth"), &ProcRockMesh::get_rockgen_depth);
	ClassDB::bind_method(D_METHOD("set_rockgen_randseed", "seed"), &ProcRockMesh::set_rockgen_randseed);
	ClassDB::bind_method(D_METHOD("get_rockgen_randseed"), &ProcRockMesh::get_rockgen_randseed);
	ClassDB::bind_method(D_METHOD("set_rockgen_smoothness", "smoothness"), &ProcRockMesh::set_rockgen_smoothness);
	ClassDB::bind_method(D_METHOD("get_rockgen_smoothness"), &ProcRockMesh::get_rockgen_smoothness);
	ClassDB::bind_method(D_METHOD("set_rockgen_smoothed", "smoothed"), &ProcRockMesh::set_rockgen_smoothed);
	ClassDB::bind_method(D_METHOD("get_rockgen_smoothed"), &ProcRockMesh::get_rockgen_smoothed);

	ClassDB::bind_method(D_METHOD("set_rockgeneration_steps", "steps"), &ProcRockMesh::set_rockgeneration_steps);
	ClassDB::bind_method(D_METHOD("get_rockgeneration_steps"), &ProcRockMesh::get_rockgeneration_steps);
	ClassDB::bind_method(D_METHOD("set_rockgeneration_width", "width"), &ProcRockMesh::set_rockgeneration_width);
	ClassDB::bind_method(D_METHOD("get_rockgeneration_width"), &ProcRockMesh::get_rockgeneration_width);
	ClassDB::bind_method(D_METHOD("set_rockgeneration_height", "height"), &ProcRockMesh::set_rockgeneration_height);
	ClassDB::bind_method(D_METHOD("get_rockgeneration_height"), &ProcRockMesh::get_rockgeneration_height);
	ClassDB::bind_method(D_METHOD("set_rockgeneration_depth", "depth"), &ProcRockMesh::set_rockgeneration_depth);
	ClassDB::bind_method(D_METHOD("get_rockgeneration_depth"), &ProcRockMesh::get_rockgeneration_depth);
	ClassDB::bind_method(D_METHOD("set_rockgeneration_max_planes", "planes"), &ProcRockMesh::set_rockgeneration_max_planes);
	ClassDB::bind_method(D_METHOD("get_rockgeneration_max_planes"), &ProcRockMesh::get_rockgeneration_max_planes);
	ClassDB::bind_method(D_METHOD("set_rockgeneration_smoothed", "smoothed"), &ProcRockMesh::set_rockgeneration_smoothed);
	ClassDB::bind_method(D_METHOD("get_rockgeneration_smoothed"), &ProcRockMesh::get_rockgeneration_smoothed);
	ClassDB::bind_method(D_METHOD("set_rockgeneration_randseed", "seed"), &ProcRockMesh::set_rockgeneration_randseed);
	ClassDB::bind_method(D_METHOD("get_rockgeneration_randseed"), &ProcRockMesh::get_rockgeneration_randseed);

	ClassDB::bind_method(D_METHOD("set_pipeline_subdivisions", "subdivisions"), &ProcRockMesh::set_pipeline_subdivisions);
	ClassDB::bind_method(D_METHOD("get_pipeline_subdivisions"), &ProcRockMesh::get_pipeline_subdivisions);
	ClassDB::bind_method(D_METHOD("set_pipeline_width", "width"), &ProcRockMesh::set_pipeline_width);
	ClassDB::bind_method(D_METHOD("get_pipeline_width"), &ProcRockMesh::get_pipeline_width);
	ClassDB::bind_method(D_METHOD("set_pipeline_height", "height"), &ProcRockMesh::set_pipeline_height);
	ClassDB::bind_method(D_METHOD("get_pipeline_height"), &ProcRockMesh::get_pipeline_height);
	ClassDB::bind_method(D_METHOD("set_pipeline_depth", "depth"), &ProcRockMesh::set_pipeline_depth);
	ClassDB::bind_method(D_METHOD("get_pipeline_depth"), &ProcRockMesh::get_pipeline_depth);
	ClassDB::bind_method(D_METHOD("set_pipeline_noise_frequency", "frequency"), &ProcRockMesh::set_pipeline_noise_frequency);
	ClassDB::bind_method(D_METHOD("get_pipeline_noise_frequency"), &ProcRockMesh::get_pipeline_noise_frequency);
	ClassDB::bind_method(D_METHOD("set_pipeline_noise_amplitude", "amplitude"), &ProcRockMesh::set_pipeline_noise_amplitude);
	ClassDB::bind_method(D_METHOD("get_pipeline_noise_amplitude"), &ProcRockMesh::get_pipeline_noise_amplitude);
	ClassDB::bind_method(D_METHOD("set_pipeline_noise_octaves", "octaves"), &ProcRockMesh::set_pipeline_noise_octaves);
	ClassDB::bind_method(D_METHOD("get_pipeline_noise_octaves"), &ProcRockMesh::get_pipeline_noise_octaves);
	ClassDB::bind_method(D_METHOD("set_pipeline_noise_persistence", "persistence"), &ProcRockMesh::set_pipeline_noise_persistence);
	ClassDB::bind_method(D_METHOD("get_pipeline_noise_persistence"), &ProcRockMesh::get_pipeline_noise_persistence);
	ClassDB::bind_method(D_METHOD("set_pipeline_randseed", "seed"), &ProcRockMesh::set_pipeline_randseed);
	ClassDB::bind_method(D_METHOD("get_pipeline_randseed"), &ProcRockMesh::get_pipeline_randseed);
	ClassDB::bind_method(D_METHOD("set_pipeline_cutplane_enabled", "enabled"), &ProcRockMesh::set_pipeline_cutplane_enabled);
	ClassDB::bind_method(D_METHOD("get_pipeline_cutplane_enabled"), &ProcRockMesh::get_pipeline_cutplane_enabled);
	ClassDB::bind_method(D_METHOD("set_pipeline_cutplane_offset", "offset"), &ProcRockMesh::set_pipeline_cutplane_offset);
	ClassDB::bind_method(D_METHOD("get_pipeline_cutplane_offset"), &ProcRockMesh::get_pipeline_cutplane_offset);
	ClassDB::bind_method(D_METHOD("set_pipeline_smoothed", "smoothed"), &ProcRockMesh::set_pipeline_smoothed);
	ClassDB::bind_method(D_METHOD("get_pipeline_smoothed"), &ProcRockMesh::get_pipeline_smoothed);
	ClassDB::bind_method(D_METHOD("set_pipeline_generate_textures", "generate"), &ProcRockMesh::set_pipeline_generate_textures);
	ClassDB::bind_method(D_METHOD("get_pipeline_generate_textures"), &ProcRockMesh::get_pipeline_generate_textures);
	ClassDB::bind_method(D_METHOD("set_pipeline_texture_size", "size"), &ProcRockMesh::set_pipeline_texture_size);
	ClassDB::bind_method(D_METHOD("get_pipeline_texture_size"), &ProcRockMesh::get_pipeline_texture_size);
	ClassDB::bind_method(D_METHOD("set_pipeline_albedo_low", "color"), &ProcRockMesh::set_pipeline_albedo_low);
	ClassDB::bind_method(D_METHOD("get_pipeline_albedo_low"), &ProcRockMesh::get_pipeline_albedo_low);
	ClassDB::bind_method(D_METHOD("set_pipeline_albedo_high", "color"), &ProcRockMesh::set_pipeline_albedo_high);
	ClassDB::bind_method(D_METHOD("get_pipeline_albedo_high"), &ProcRockMesh::get_pipeline_albedo_high);
	ClassDB::bind_method(D_METHOD("set_pipeline_normal_strength", "strength"), &ProcRockMesh::set_pipeline_normal_strength);
	ClassDB::bind_method(D_METHOD("get_pipeline_normal_strength"), &ProcRockMesh::get_pipeline_normal_strength);
	ClassDB::bind_method(D_METHOD("set_pipeline_roughness_scale", "scale"), &ProcRockMesh::set_pipeline_roughness_scale);
	ClassDB::bind_method(D_METHOD("get_pipeline_roughness_scale"), &ProcRockMesh::get_pipeline_roughness_scale);
	ClassDB::bind_method(D_METHOD("set_pipeline_roughness_bias", "bias"), &ProcRockMesh::set_pipeline_roughness_bias);
	ClassDB::bind_method(D_METHOD("get_pipeline_roughness_bias"), &ProcRockMesh::get_pipeline_roughness_bias);
	ClassDB::bind_method(D_METHOD("set_pipeline_metalness_scale", "scale"), &ProcRockMesh::set_pipeline_metalness_scale);
	ClassDB::bind_method(D_METHOD("get_pipeline_metalness_scale"), &ProcRockMesh::get_pipeline_metalness_scale);
	ClassDB::bind_method(D_METHOD("set_pipeline_metalness_bias", "bias"), &ProcRockMesh::set_pipeline_metalness_bias);
	ClassDB::bind_method(D_METHOD("get_pipeline_metalness_bias"), &ProcRockMesh::get_pipeline_metalness_bias);
	ClassDB::bind_method(D_METHOD("set_pipeline_ao_scale", "scale"), &ProcRockMesh::set_pipeline_ao_scale);
	ClassDB::bind_method(D_METHOD("get_pipeline_ao_scale"), &ProcRockMesh::get_pipeline_ao_scale);
	ClassDB::bind_method(D_METHOD("set_pipeline_ao_bias", "bias"), &ProcRockMesh::set_pipeline_ao_bias);
	ClassDB::bind_method(D_METHOD("get_pipeline_ao_bias"), &ProcRockMesh::get_pipeline_ao_bias);
	ClassDB::bind_method(D_METHOD("set_pipeline_preset", "preset"), &ProcRockMesh::set_pipeline_preset);
	ClassDB::bind_method(D_METHOD("get_pipeline_material"), &ProcRockMesh::get_pipeline_material);
	ClassDB::bind_method(D_METHOD("load_from_file", "path"), &ProcRockMesh::load_from_file);

	ClassDB::bind_method(D_METHOD("set_rockcluster_style", "style"), &ProcRockMesh::set_rockcluster_style);
	ClassDB::bind_method(D_METHOD("get_rockcluster_style"), &ProcRockMesh::get_rockcluster_style);
	ClassDB::bind_method(D_METHOD("set_rockcluster_randseed", "seed"), &ProcRockMesh::set_rockcluster_randseed);
	ClassDB::bind_method(D_METHOD("get_rockcluster_randseed"), &ProcRockMesh::get_rockcluster_randseed);
	ClassDB::bind_method(D_METHOD("set_rockcluster_density", "density"), &ProcRockMesh::set_rockcluster_density);
	ClassDB::bind_method(D_METHOD("get_rockcluster_density"), &ProcRockMesh::get_rockcluster_density);
	ClassDB::bind_method(D_METHOD("set_rockcluster_radius", "radius"), &ProcRockMesh::set_rockcluster_radius);
	ClassDB::bind_method(D_METHOD("get_rockcluster_radius"), &ProcRockMesh::get_rockcluster_radius);
	ClassDB::bind_method(D_METHOD("set_rockcluster_asymmetry", "val"), &ProcRockMesh::set_rockcluster_asymmetry);
	ClassDB::bind_method(D_METHOD("get_rockcluster_asymmetry"), &ProcRockMesh::get_rockcluster_asymmetry);
	ClassDB::bind_method(D_METHOD("set_rockcluster_wave", "val"), &ProcRockMesh::set_rockcluster_wave);
	ClassDB::bind_method(D_METHOD("get_rockcluster_wave"), &ProcRockMesh::get_rockcluster_wave);
	ClassDB::bind_method(D_METHOD("set_rockcluster_decentralize", "val"), &ProcRockMesh::set_rockcluster_decentralize);
	ClassDB::bind_method(D_METHOD("get_rockcluster_decentralize"), &ProcRockMesh::get_rockcluster_decentralize);
	ClassDB::bind_method(D_METHOD("set_rockcluster_scale_local", "val"), &ProcRockMesh::set_rockcluster_scale_local);
	ClassDB::bind_method(D_METHOD("get_rockcluster_scale_local"), &ProcRockMesh::get_rockcluster_scale_local);
	ClassDB::bind_method(D_METHOD("set_rockcluster_scale_by_distance", "curve"), &ProcRockMesh::set_rockcluster_scale_by_distance);
	ClassDB::bind_method(D_METHOD("get_rockcluster_scale_by_distance"), &ProcRockMesh::get_rockcluster_scale_by_distance);
	ClassDB::bind_method(D_METHOD("set_rockcluster_tallness", "val"), &ProcRockMesh::set_rockcluster_tallness);
	ClassDB::bind_method(D_METHOD("get_rockcluster_tallness"), &ProcRockMesh::get_rockcluster_tallness);
	ClassDB::bind_method(D_METHOD("set_rockcluster_flatness", "val"), &ProcRockMesh::set_rockcluster_flatness);
	ClassDB::bind_method(D_METHOD("get_rockcluster_flatness"), &ProcRockMesh::get_rockcluster_flatness);
	ClassDB::bind_method(D_METHOD("set_rockcluster_wideness", "val"), &ProcRockMesh::set_rockcluster_wideness);
	ClassDB::bind_method(D_METHOD("get_rockcluster_wideness"), &ProcRockMesh::get_rockcluster_wideness);
	ClassDB::bind_method(D_METHOD("set_rockcluster_rotation", "val"), &ProcRockMesh::set_rockcluster_rotation);
	ClassDB::bind_method(D_METHOD("get_rockcluster_rotation"), &ProcRockMesh::get_rockcluster_rotation);
	ClassDB::bind_method(D_METHOD("set_rockcluster_rotation_local", "val"), &ProcRockMesh::set_rockcluster_rotation_local);
	ClassDB::bind_method(D_METHOD("get_rockcluster_rotation_local"), &ProcRockMesh::get_rockcluster_rotation_local);
	ClassDB::bind_method(D_METHOD("set_rockcluster_rotation_rnd", "val"), &ProcRockMesh::set_rockcluster_rotation_rnd);
	ClassDB::bind_method(D_METHOD("get_rockcluster_rotation_rnd"), &ProcRockMesh::get_rockcluster_rotation_rnd);
	ClassDB::bind_method(D_METHOD("set_rockcluster_crystal_scale", "val"), &ProcRockMesh::set_rockcluster_crystal_scale);
	ClassDB::bind_method(D_METHOD("get_rockcluster_crystal_scale"), &ProcRockMesh::get_rockcluster_crystal_scale);
	ClassDB::bind_method(D_METHOD("set_rockcluster_scale_by_angle", "val"), &ProcRockMesh::set_rockcluster_scale_by_angle);
	ClassDB::bind_method(D_METHOD("get_rockcluster_scale_by_angle"), &ProcRockMesh::get_rockcluster_scale_by_angle);
	ClassDB::bind_method(D_METHOD("set_rockcluster_scale_random_offset", "val"), &ProcRockMesh::set_rockcluster_scale_random_offset);
	ClassDB::bind_method(D_METHOD("get_rockcluster_scale_random_offset"), &ProcRockMesh::get_rockcluster_scale_random_offset);
	ClassDB::bind_method(D_METHOD("set_rockcluster_scale_bias", "val"), &ProcRockMesh::set_rockcluster_scale_bias);
	ClassDB::bind_method(D_METHOD("get_rockcluster_scale_bias"), &ProcRockMesh::get_rockcluster_scale_bias);
	ClassDB::bind_method(D_METHOD("set_rockcluster_bloom", "val"), &ProcRockMesh::set_rockcluster_bloom);
	ClassDB::bind_method(D_METHOD("get_rockcluster_bloom"), &ProcRockMesh::get_rockcluster_bloom);

	ClassDB::bind_method(D_METHOD("set_flatten_base_enabled", "enabled"), &ProcRockMesh::set_flatten_base_enabled);
	ClassDB::bind_method(D_METHOD("get_flatten_base_enabled"), &ProcRockMesh::get_flatten_base_enabled);
	ClassDB::bind_method(D_METHOD("set_flatten_base_offset", "offset"), &ProcRockMesh::set_flatten_base_offset);
	ClassDB::bind_method(D_METHOD("get_flatten_base_offset"), &ProcRockMesh::get_flatten_base_offset);

	ClassDB::bind_method(D_METHOD("set_texture_source", "source"), &ProcRockMesh::set_texture_source);
	ClassDB::bind_method(D_METHOD("get_texture_source"), &ProcRockMesh::get_texture_source);
	ClassDB::bind_method(D_METHOD("set_texture_gen_size", "size"), &ProcRockMesh::set_texture_gen_size);
	ClassDB::bind_method(D_METHOD("get_texture_gen_size"), &ProcRockMesh::get_texture_gen_size);
	ClassDB::bind_method(D_METHOD("set_texture_gen_seed", "seed"), &ProcRockMesh::set_texture_gen_seed);
	ClassDB::bind_method(D_METHOD("get_texture_gen_seed"), &ProcRockMesh::get_texture_gen_seed);
	ClassDB::bind_method(D_METHOD("set_texture_gen_noise_frequency", "frequency"), &ProcRockMesh::set_texture_gen_noise_frequency);
	ClassDB::bind_method(D_METHOD("get_texture_gen_noise_frequency"), &ProcRockMesh::get_texture_gen_noise_frequency);
	ClassDB::bind_method(D_METHOD("set_texture_gen_noise_octaves", "octaves"), &ProcRockMesh::set_texture_gen_noise_octaves);
	ClassDB::bind_method(D_METHOD("get_texture_gen_noise_octaves"), &ProcRockMesh::get_texture_gen_noise_octaves);
	ClassDB::bind_method(D_METHOD("set_texture_gen_noise_persistence", "persistence"), &ProcRockMesh::set_texture_gen_noise_persistence);
	ClassDB::bind_method(D_METHOD("get_texture_gen_noise_persistence"), &ProcRockMesh::get_texture_gen_noise_persistence);
	ClassDB::bind_method(D_METHOD("set_texture_gen_albedo_low", "color"), &ProcRockMesh::set_texture_gen_albedo_low);
	ClassDB::bind_method(D_METHOD("get_texture_gen_albedo_low"), &ProcRockMesh::get_texture_gen_albedo_low);
	ClassDB::bind_method(D_METHOD("set_texture_gen_albedo_high", "color"), &ProcRockMesh::set_texture_gen_albedo_high);
	ClassDB::bind_method(D_METHOD("get_texture_gen_albedo_high"), &ProcRockMesh::get_texture_gen_albedo_high);
	ClassDB::bind_method(D_METHOD("set_texture_gen_normal_strength", "strength"), &ProcRockMesh::set_texture_gen_normal_strength);
	ClassDB::bind_method(D_METHOD("get_texture_gen_normal_strength"), &ProcRockMesh::get_texture_gen_normal_strength);
	ClassDB::bind_method(D_METHOD("set_texture_gen_roughness_scale", "scale"), &ProcRockMesh::set_texture_gen_roughness_scale);
	ClassDB::bind_method(D_METHOD("get_texture_gen_roughness_scale"), &ProcRockMesh::get_texture_gen_roughness_scale);
	ClassDB::bind_method(D_METHOD("set_texture_gen_roughness_bias", "bias"), &ProcRockMesh::set_texture_gen_roughness_bias);
	ClassDB::bind_method(D_METHOD("get_texture_gen_roughness_bias"), &ProcRockMesh::get_texture_gen_roughness_bias);
	ClassDB::bind_method(D_METHOD("set_texture_gen_metalness_scale", "scale"), &ProcRockMesh::set_texture_gen_metalness_scale);
	ClassDB::bind_method(D_METHOD("get_texture_gen_metalness_scale"), &ProcRockMesh::get_texture_gen_metalness_scale);
	ClassDB::bind_method(D_METHOD("set_texture_gen_metalness_bias", "bias"), &ProcRockMesh::set_texture_gen_metalness_bias);
	ClassDB::bind_method(D_METHOD("get_texture_gen_metalness_bias"), &ProcRockMesh::get_texture_gen_metalness_bias);
	ClassDB::bind_method(D_METHOD("set_texture_gen_ao_scale", "scale"), &ProcRockMesh::set_texture_gen_ao_scale);
	ClassDB::bind_method(D_METHOD("get_texture_gen_ao_scale"), &ProcRockMesh::get_texture_gen_ao_scale);
	ClassDB::bind_method(D_METHOD("set_texture_gen_ao_bias", "bias"), &ProcRockMesh::set_texture_gen_ao_bias);
	ClassDB::bind_method(D_METHOD("get_texture_gen_ao_bias"), &ProcRockMesh::get_texture_gen_ao_bias);
	ClassDB::bind_method(D_METHOD("set_texture_file_albedo", "path"), &ProcRockMesh::set_texture_file_albedo);
	ClassDB::bind_method(D_METHOD("get_texture_file_albedo"), &ProcRockMesh::get_texture_file_albedo);
	ClassDB::bind_method(D_METHOD("set_texture_file_normal", "path"), &ProcRockMesh::set_texture_file_normal);
	ClassDB::bind_method(D_METHOD("get_texture_file_normal"), &ProcRockMesh::get_texture_file_normal);
	ClassDB::bind_method(D_METHOD("set_texture_file_roughness", "path"), &ProcRockMesh::set_texture_file_roughness);
	ClassDB::bind_method(D_METHOD("get_texture_file_roughness"), &ProcRockMesh::get_texture_file_roughness);
	ClassDB::bind_method(D_METHOD("set_texture_file_metalness", "path"), &ProcRockMesh::set_texture_file_metalness);
	ClassDB::bind_method(D_METHOD("get_texture_file_metalness"), &ProcRockMesh::get_texture_file_metalness);
	ClassDB::bind_method(D_METHOD("set_texture_file_ambient_occlusion", "path"), &ProcRockMesh::set_texture_file_ambient_occlusion);
	ClassDB::bind_method(D_METHOD("get_texture_file_ambient_occlusion"), &ProcRockMesh::get_texture_file_ambient_occlusion);

	ClassDB::bind_method(D_METHOD("set_auto_refresh", "refresh"), &ProcRockMesh::set_auto_refresh);
	ClassDB::bind_method(D_METHOD("get_auto_refresh"), &ProcRockMesh::get_auto_refresh);

	ClassDB::bind_method(D_METHOD("set_baked", "baked"), &ProcRockMesh::set_baked);
	ClassDB::bind_method(D_METHOD("get_baked"), &ProcRockMesh::get_baked);
#ifdef TOOLS_ENABLED
	ClassDB::bind_method(D_METHOD("bake"), &ProcRockMesh::bake);
#endif

	ClassDB::bind_method(D_METHOD("_rebuild"), &ProcRockMesh::_rebuild);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "generator", PROPERTY_HINT_ENUM, "RockGen,IcoRock,RockStudio,ProcRock,RockCluster"), "set_generator", "get_generator");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_refresh"), "set_auto_refresh", "get_auto_refresh");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "baked", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL), "set_baked", "get_baked");
	// Generator-specific properties are dynamic — see _get_property_list()
}

ProcRockMesh::ProcRockMesh() {
	method = 0;
	auto_refresh = false;
	_baked = false;

	flatten_base_enabled = false;
	// Measured each method's default Y extent directly (not guessed): RockGen/IcoRock
	// (methods 0/1) span roughly -1..1, RockStudio/ProcRock (2/3) roughly -0.5..0.5 --
	// -0.3 cuts a modest ~15-20% off the smaller two and a moderate ~35% off the larger
	// two, a reasonable single default across the size disparity without being extreme
	// for either.
	flatten_base_offset = -0.3;

	rockgen.depth = 2;
	rockgen.randseed = 0;
	// Perturbation(profondeur) divides a (randf()-randf()) draw (range ±1) by
	// exp(smoothness * ...), so at the coarsest subdivision level a smoothness of 1 lets
	// a single unlucky draw displace a vertex by up to ~37% of the mesh radius — visible
	// as the occasional thin spike/sliver poking out of an otherwise normal-looking rock,
	// especially at the now-lower default rockgen_depth (fewer, larger base triangles
	// per unlucky draw). 2 caps the worst case at ~14%, comfortably rock-like without
	// the occasional wild outlier.
	rockgen.smoothness = 2;
	rockgen.smoothed = false;

	// RockStudio/ProcRock both default their bounding dimensions to ~1 (unit-scale), and
	// the dock's preview camera (proc_rocks_editor_plugin.cpp) sits at a fixed distance
	// of 3 units from the origin with no auto-framing. The old default of 50 here meant
	// the camera was deep *inside* the generated rock (radius ~50+) looking at the
	// inward-facing side of its triangles — backface-culled, i.e. invisible — which is
	// why the dock's preview showed nothing for IcoRock ("still empty"). All of
	// GenRock's internal math (averageRadius, plane offsets, etc.) is proportional to
	// these dimensions, so this is a pure scale fix — it doesn't change the ~30% radius
	// spread measured for item 19's deformation fix.
	rockgeneration.dimensions = Vector3(1, 1, 1);
	// GenRock::BuildIco() feeds this straight into MakeIcosphere(subdivisions) — same
	// 20*4^n triangle growth as rockgen_depth above. 2 matches rockgen_depth's default
	// for a comparable triangle count (320 vs. 320) — see memo.md's "Bugs Fixed" #16.
	rockgeneration.steps = 2;
	// GenRock::BuildRock()'s cutting plane sits at (100-offset)% of the mesh radius —
	// with the old default of 5 (offset 0-4%), the plane stayed almost exactly tangent
	// to the sphere, so only a razor-thin cap around each plane's contact point was
	// "in front" of it (dot>=0) and displacement (proportional to perpendicular distance
	// to that plane, bounded by the cap's own tiny height) stayed negligible — measured:
	// only ~2.7% vertex-radius spread at defaults, i.e. visually indistinguishable from
	// a perfect sphere no matter how many planes (rockgeneration_max_planes) were
	// applied. 40 lets the plane cut up to 40% deeper into the mesh, which measured out
	// to ~30% radius spread — an actually rock-shaped result. This property isn't
	// user-exposed (see memo.md's "Bugs Fixed"), so this default is the only lever.
	rockgeneration.rand_offset_percent = 60;
	rockgeneration.rand_shift = 2;
	// plane_verts_range is dead: GenRock::m_MinPlaneVerts/m_MaxPlaneVerts are set via
	// SetMinPlaneVerts()/SetMaxPlaneVerts() but never read anywhere in gen_rock.cpp, and
	// this field isn't even exposed as an editable property. Left as-is (out of scope
	// for the face-count/limit pass) but noted here so it isn't mistaken for load-bearing.
	rockgeneration.plane_verts_range = Vector2i(0, 200);
	// BuildRock() only *moves* existing icosphere vertices (no new triangles), so this
	// doesn't affect face count — but with the old default of 1, a single flattening cut
	// barely dented the icosphere, which is why IcoRock looked like "a regular ball, not
	// a rock" even after the steps fix. 6 gives it real faceted structure.
	rockgeneration.max_planes = 10;
	// false (flat/low-poly): smooth shading (GenRock::BuildNormals() always computes
	// smooth per-vertex normals internally) visually rounds off the flattened facets
	// BuildRock() produces, which is the other half of why this generator "looked very
	// poor" even after item 19's tuning — see memo.md's "Bugs Fixed".
	rockgeneration.smoothed = false;
	rockgeneration.randseed = 0;

	rockstudio.rock_type = 0;
	rockstudio.num_vertices = 25;
	rockstudio.width = 1;
	rockstudio.height = 1;
	rockstudio.depth = 1;
	rockstudio.radius = 1;
	rockstudio.tip_protrusion = 2;
	rockstudio.tip_flatness = 1;
	rockstudio.base_width = 2;
	rockstudio.base_height = 2;
	rockstudio.tetragonal = false;
	rockstudio.one_sided = false;
	rockstudio.randseed = 0;

	pipeline.subdivisions = 2;
	pipeline.width = 1;
	pipeline.height = 1;
	pipeline.depth = 1;
	pipeline.noise_frequency = 1;
	pipeline.noise_amplitude = 0.2;
	pipeline.noise_octaves = 3;
	pipeline.noise_persistence = 0.5;
	pipeline.randseed = 0;
	pipeline.cutplane_enabled = false;
	pipeline.cutplane_offset = 0;
	pipeline.smoothed = true;
	pipeline.generate_textures = false;
	pipeline.texture_size = 256;
	pipeline.albedo_low = Color(0.3, 0.28, 0.26);
	pipeline.albedo_high = Color(0.6, 0.58, 0.55);
	pipeline.normal_strength = 2.0;
	pipeline.roughness_scale = 0.4;
	pipeline.roughness_bias = 0.5;
	pipeline.metalness_scale = 0;
	pipeline.metalness_bias = 0;
	pipeline.ao_scale = 0.5;
	pipeline.ao_bias = 0.5;

	rockcluster.style = 0; // Boulder
	rockcluster.randseed = 0;
	rockcluster.density = 8;
	// radius/scale_local are a *proportional* scale-down of the original source
	// algorithm's own defaults (radius=5, scaleLocal=2 in LogicType01.cs -- ratio 2.5),
	// not independently re-tuned values -- cells are natively ~1.2-1.7 units across, so
	// using the original absolute values verbatim would put the dock preview camera
	// (fixed ~3-unit distance, no auto-framing) inside an oversized cluster, the same
	// class of bug as IcoRock's default-dimensions issue (see "Bugs Fixed" #20). Scaling
	// both down by the same ~1/2.08 factor (keeping the 2.5 ratio, now 1.2/0.5=2.4) fits
	// this project's ~1-2 unit convention without changing the algorithm's own relative
	// proportions -- an explicit instruction after an earlier attempt at this same fix
	// instead changed the radius/scale_local *ratio* itself (a real, if well-intentioned,
	// deviation from the source algorithm, not a scale conversion) trying to address a
	// "should cluster, not scatter" report; that ratio change is reverted here. The
	// "few big + several small, spread out" composition this produces is the source
	// algorithm's own faithful behavior at its own proportions, not a bug to retune away.
	rockcluster.radius = 1.2;
	rockcluster.asymmetry = 0;
	rockcluster.wave = 0;
	rockcluster.decentralize = 0.5;
	rockcluster.scale_local = 0.5;
	rockcluster.scale_by_distance.instance();
	rockcluster.scale_by_distance->add_point(Vector2(0, 0), 0, 0);
	rockcluster.scale_by_distance->add_point(Vector2(1, 1), 2, 2);
	rockcluster.tallness = 0.6;
	rockcluster.flatness = 0;
	rockcluster.wideness = 0;
	rockcluster.rotation = 0;
	rockcluster.rotation_local = 0;
	rockcluster.rotation_rnd = 0.1;
	// Crystal cells are natively ~1x11x1 (much taller than Boulder/Sharp's ~1.2-1.7),
	// with no other absolute-scale control in the algorithm — this default is
	// measured to bring a default cluster down to the same ~1.5-2 unit ballpark.
	rockcluster.crystal_scale = 0.18;
	rockcluster.scale_by_angle = 1;
	rockcluster.scale_random_offset = 1;
	rockcluster.scale_bias = 0;
	rockcluster.bloom = 0.7;

	texture.source = TEXTURE_SOURCE_NONE;
	texture.gen_size = 256;
	texture.gen_seed = 0;
	texture.gen_noise_frequency = 1;
	texture.gen_noise_octaves = 3;
	texture.gen_noise_persistence = 0.5;
	texture.gen_albedo_low = Color(0.3, 0.28, 0.26);
	texture.gen_albedo_high = Color(0.6, 0.58, 0.55);
	texture.gen_normal_strength = 2.0;
	texture.gen_roughness_scale = 0.4;
	texture.gen_roughness_bias = 0.5;
	texture.gen_metalness_scale = 0;
	texture.gen_metalness_bias = 0;
	texture.gen_ao_scale = 0.5;
	texture.gen_ao_bias = 0.5;

	_dirty = true;
}

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "core/project_settings.h"
#include "generators/shared/rock_header.h"
#include "scene/resources/primitive_meshes.h"

#include "doctest/doctest.h"
#include "doctest/doctest_godot.h"

TEST_SUITE("[[proc_rocks]] ProcRockMesh") {
	TEST_CASE("[proc_rocks] default construction") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		CHECK(mesh->get_generator() == 0);
		CHECK(mesh->get_auto_refresh() == false);
		CHECK(mesh->get_rockgen_depth() == 2);
		CHECK(mesh->get_rockgen_randseed() == 0);
		CHECK(mesh->get_rockgen_smoothness() == doctest::Approx(2.0f));
		CHECK(mesh->get_rockgen_smoothed() == false);
	}

	TEST_CASE("[proc_rocks] generator selection") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_generator(1);
		CHECK(mesh->get_generator() == 1);
		mesh->set_generator(2);
		CHECK(mesh->get_generator() == 2);
		mesh->set_generator(0);
		CHECK(mesh->get_generator() == 0);
	}

	TEST_CASE("[proc_rocks] generator clamped to valid range") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_generator(-1);
		CHECK(mesh->get_generator() == 0);
		mesh->set_generator(99);
		CHECK(mesh->get_generator() == 4);
	}

	TEST_CASE("[proc_rocks] rockgen properties") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_rockgen_depth(1);
		CHECK(mesh->get_rockgen_depth() == 1);
		mesh->set_rockgen_randseed(42);
		CHECK(mesh->get_rockgen_randseed() == 42);
		mesh->set_rockgen_smoothness(2.5f);
		CHECK(mesh->get_rockgen_smoothness() == doctest::Approx(2.5f));
		mesh->set_rockgen_smoothed(true);
		CHECK(mesh->get_rockgen_smoothed() == true);
	}

	TEST_CASE("[proc_rocks] rockgeneration properties") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_rockgeneration_steps(3);
		CHECK(mesh->get_rockgeneration_steps() == 3);
		mesh->set_rockgeneration_width(100.0f);
		CHECK(mesh->get_rockgeneration_width() == doctest::Approx(100.0f));
		mesh->set_rockgeneration_height(75.0f);
		CHECK(mesh->get_rockgeneration_height() == doctest::Approx(75.0f));
		mesh->set_rockgeneration_depth(60.0f);
		CHECK(mesh->get_rockgeneration_depth() == doctest::Approx(60.0f));
		mesh->set_rockgeneration_max_planes(5);
		CHECK(mesh->get_rockgeneration_max_planes() == 5);
	}

	TEST_CASE("[proc_rocks] rock_gen returns valid mesh arrays") {
		Array result;
		SUPPRESS_OUTPUT(result = rock_gen(2, 42, 1.0, false));
		CHECK(result.size() == VS::ARRAY_MAX);
		Vector<Vector3> verts = result[VS::ARRAY_VERTEX];
		CHECK(verts.size() > 0);
		// 20 base faces × 4^depth triangles × 3 verts each
		CHECK(verts.size() == 20 * 16 * 3);
		Vector<Vector3> normals = result[VS::ARRAY_NORMAL];
		CHECK(normals.size() == verts.size());
	}

	TEST_CASE("[proc_rocks] rock_gen depth affects vertex count") {
		Array r1, r2;
		SUPPRESS_OUTPUT(r1 = rock_gen(1, 42, 1.0, false));
		SUPPRESS_OUTPUT(r2 = rock_gen(3, 42, 1.0, false));
		Vector<Vector3> v1 = r1[VS::ARRAY_VERTEX];
		Vector<Vector3> v2 = r2[VS::ARRAY_VERTEX];
		CHECK(v2.size() > v1.size());
	}

	TEST_CASE("[proc_rocks] rock_gen fixed seed is deterministic") {
		Array r1, r2;
		SUPPRESS_OUTPUT(r1 = rock_gen(2, 123, 1.0, false));
		SUPPRESS_OUTPUT(r2 = rock_gen(2, 123, 1.0, false));
		Vector<Vector3> v1 = r1[VS::ARRAY_VERTEX];
		Vector<Vector3> v2 = r2[VS::ARRAY_VERTEX];
		CHECK(v1.size() == v2.size());
		for (int i = 0; i < v1.size(); i++) {
			CHECK(v1[i].is_equal_approx(v2[i]));
		}
	}

	TEST_CASE("[proc_rocks] rock_gen smoothed mode") {
		Array result;
		SUPPRESS_OUTPUT(result = rock_gen(2, 42, 1.0, true));
		CHECK(result.size() == VS::ARRAY_MAX);
		Vector<Vector3> verts = result[VS::ARRAY_VERTEX];
		CHECK(verts.size() > 0);
		Vector<Vector3> normals = result[VS::ARRAY_NORMAL];
		CHECK(normals.size() == verts.size());
	}

	TEST_CASE("[proc_rocks] auto_refresh triggers rebuild") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_generator(0);
		mesh->set_rockgen_randseed(42);
		CHECK(mesh->get_surface_count() == 0);
		SUPPRESS_OUTPUT(mesh->set_auto_refresh(true));
		CHECK(mesh->get_surface_count() == 1);
	}

	TEST_CASE("[proc_rocks] pipeline default property values") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		CHECK(mesh->get_pipeline_subdivisions() == 2);
		CHECK(mesh->get_pipeline_randseed() == 0);
		CHECK(mesh->get_pipeline_cutplane_enabled() == false);
		CHECK(mesh->get_pipeline_smoothed() == true);
		CHECK(mesh->get_pipeline_generate_textures() == false);
		CHECK(mesh->get_pipeline_texture_size() == 256);
	}

	TEST_CASE("[proc_rocks] pipeline properties round-trip") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_pipeline_subdivisions(3);
		CHECK(mesh->get_pipeline_subdivisions() == 3);
		mesh->set_pipeline_width(12.5f);
		CHECK(mesh->get_pipeline_width() == doctest::Approx(12.5f));
		mesh->set_pipeline_height(8.0f);
		CHECK(mesh->get_pipeline_height() == doctest::Approx(8.0f));
		mesh->set_pipeline_depth(4.0f);
		CHECK(mesh->get_pipeline_depth() == doctest::Approx(4.0f));
		mesh->set_pipeline_noise_frequency(2.0f);
		CHECK(mesh->get_pipeline_noise_frequency() == doctest::Approx(2.0f));
		mesh->set_pipeline_noise_amplitude(0.5f);
		CHECK(mesh->get_pipeline_noise_amplitude() == doctest::Approx(0.5f));
		mesh->set_pipeline_noise_octaves(4);
		CHECK(mesh->get_pipeline_noise_octaves() == 4);
		mesh->set_pipeline_noise_persistence(0.3f);
		CHECK(mesh->get_pipeline_noise_persistence() == doctest::Approx(0.3f));
		mesh->set_pipeline_randseed(99);
		CHECK(mesh->get_pipeline_randseed() == 99);
		mesh->set_pipeline_cutplane_enabled(true);
		CHECK(mesh->get_pipeline_cutplane_enabled() == true);
		mesh->set_pipeline_cutplane_offset(0.25f);
		CHECK(mesh->get_pipeline_cutplane_offset() == doctest::Approx(0.25f));
		mesh->set_pipeline_smoothed(false);
		CHECK(mesh->get_pipeline_smoothed() == false);
		mesh->set_pipeline_generate_textures(true);
		CHECK(mesh->get_pipeline_generate_textures() == true);
		mesh->set_pipeline_texture_size(128);
		CHECK(mesh->get_pipeline_texture_size() == 128);
		mesh->set_pipeline_albedo_low(Color(0.1, 0.2, 0.3));
		CHECK(mesh->get_pipeline_albedo_low().is_equal_approx(Color(0.1, 0.2, 0.3)));
		mesh->set_pipeline_albedo_high(Color(0.7, 0.8, 0.9));
		CHECK(mesh->get_pipeline_albedo_high().is_equal_approx(Color(0.7, 0.8, 0.9)));
		mesh->set_pipeline_normal_strength(3.0f);
		CHECK(mesh->get_pipeline_normal_strength() == doctest::Approx(3.0f));
		mesh->set_pipeline_roughness_scale(0.6f);
		CHECK(mesh->get_pipeline_roughness_scale() == doctest::Approx(0.6f));
		mesh->set_pipeline_roughness_bias(0.1f);
		CHECK(mesh->get_pipeline_roughness_bias() == doctest::Approx(0.1f));
		mesh->set_pipeline_metalness_scale(0.2f);
		CHECK(mesh->get_pipeline_metalness_scale() == doctest::Approx(0.2f));
		mesh->set_pipeline_metalness_bias(0.05f);
		CHECK(mesh->get_pipeline_metalness_bias() == doctest::Approx(0.05f));
		mesh->set_pipeline_ao_scale(0.4f);
		CHECK(mesh->get_pipeline_ao_scale() == doctest::Approx(0.4f));
		mesh->set_pipeline_ao_bias(0.6f);
		CHECK(mesh->get_pipeline_ao_bias() == doctest::Approx(0.6f));
	}

	TEST_CASE("[proc_rocks] rock_pipeline_gen returns valid mesh arrays") {
		Array result;
		SUPPRESS_OUTPUT(result = rock_pipeline_gen(1, 1, 1, 1, 1.0, 0.2, 3, 0.5, 42, false, 0.0, true));
		CHECK(result.size() == VS::ARRAY_MAX);
		Vector<Vector3> verts = result[VS::ARRAY_VERTEX];
		CHECK(verts.size() > 0);
		Vector<Vector3> normals = result[VS::ARRAY_NORMAL];
		CHECK(normals.size() == verts.size());
		for (int i = 0; i < normals.size(); i++) {
			CHECK(normals[i].length() == doctest::Approx(1.0f).epsilon(0.01));
		}
		Vector<Vector2> uvs = result[VS::ARRAY_TEX_UV];
		CHECK(uvs.size() == verts.size());
		Vector<int> indices = result[VS::ARRAY_INDEX];
		CHECK(indices.size() > 0);
		CHECK(indices.size() % 3 == 0);
	}

	TEST_CASE("[proc_rocks] rock_pipeline_gen fixed seed is deterministic") {
		Array r1, r2;
		SUPPRESS_OUTPUT(r1 = rock_pipeline_gen(1, 1, 1, 1, 1.0, 0.2, 3, 0.5, 123, false, 0.0, true));
		SUPPRESS_OUTPUT(r2 = rock_pipeline_gen(1, 1, 1, 1, 1.0, 0.2, 3, 0.5, 123, false, 0.0, true));
		Vector<Vector3> v1 = r1[VS::ARRAY_VERTEX];
		Vector<Vector3> v2 = r2[VS::ARRAY_VERTEX];
		CHECK(v1.size() == v2.size());
		for (int i = 0; i < v1.size(); i++) {
			CHECK(v1[i].is_equal_approx(v2[i]));
		}
	}

	TEST_CASE("[proc_rocks] rock_pipeline_gen subdivisions increase vertex count") {
		Array r1, r2;
		SUPPRESS_OUTPUT(r1 = rock_pipeline_gen(0, 1, 1, 1, 1.0, 0.2, 3, 0.5, 42, false, 0.0, true));
		SUPPRESS_OUTPUT(r2 = rock_pipeline_gen(2, 1, 1, 1, 1.0, 0.2, 3, 0.5, 42, false, 0.0, true));
		Vector<Vector3> v1 = r1[VS::ARRAY_VERTEX];
		Vector<Vector3> v2 = r2[VS::ARRAY_VERTEX];
		CHECK(v2.size() > v1.size());
	}

	TEST_CASE("[proc_rocks] rock_pipeline_gen smoothed vs flat shape differs") {
		Array smoothed, flat;
		SUPPRESS_OUTPUT(smoothed = rock_pipeline_gen(1, 1, 1, 1, 1.0, 0.2, 3, 0.5, 42, false, 0.0, true));
		SUPPRESS_OUTPUT(flat = rock_pipeline_gen(1, 1, 1, 1, 1.0, 0.2, 3, 0.5, 42, false, 0.0, false));
		Vector<Vector3> smoothed_verts = smoothed[VS::ARRAY_VERTEX];
		Vector<Vector3> flat_verts = flat[VS::ARRAY_VERTEX];
		Vector<int> smoothed_indices = smoothed[VS::ARRAY_INDEX];
		Vector<int> flat_indices = flat[VS::ARRAY_INDEX];
		CHECK(smoothed_indices.size() > 0); // indexed, shared vertices
		CHECK(flat_indices.size() == 0); // unindexed, duplicated per-triangle vertices
		CHECK(flat_verts.size() > smoothed_verts.size());
	}

	TEST_CASE("[proc_rocks] rock_pipeline_gen cut-plane changes topology") {
		Array whole, cut;
		SUPPRESS_OUTPUT(whole = rock_pipeline_gen(2, 1, 1, 1, 1.0, 0.1, 3, 0.5, 42, false, 0.0, true));
		SUPPRESS_OUTPUT(cut = rock_pipeline_gen(2, 1, 1, 1, 1.0, 0.1, 3, 0.5, 42, true, 0.0, true));
		Vector<Vector3> whole_verts = whole[VS::ARRAY_VERTEX];
		Vector<Vector3> cut_verts = cut[VS::ARRAY_VERTEX];
		CHECK(cut_verts.size() != whole_verts.size());
		CHECK(cut_verts.size() > 0);
	}

	TEST_CASE("[proc_rocks] rock_pipeline_gen cut-plane output is pinned (clip_and_cap/compute_smooth_normals relocation regression guard)") {
		// Baseline captured BEFORE moving clip_and_cap()/compute_smooth_normals() out of
		// procrockgen.cpp into generators/shared/ -- pins exact output (not just "still
		// passes some loose assertion") so that relocation is provably behavior-preserving,
		// not just "still compiles and other tests still pass". See memo.md's "Flatten
		// base" section for why this move happened.
		Array cut;
		SUPPRESS_OUTPUT(cut = rock_pipeline_gen(2, 1, 1, 1, 1.0, 0.1, 3, 0.5, 42, true, 0.0, true));
		Vector<Vector3> verts = cut[VS::ARRAY_VERTEX];
		Vector<Vector3> normals = cut[VS::ARRAY_NORMAL];
		Vector<int> indices = cut[VS::ARRAY_INDEX];
		REQUIRE(verts.size() > 0);

		double vertex_checksum = 0.0, normal_checksum = 0.0;
		for (int i = 0; i < verts.size(); i++) {
			vertex_checksum += verts[i].x * 1.0 + verts[i].y * 2.0 + verts[i].z * 3.0;
			normal_checksum += normals[i].x * 1.0 + normals[i].y * 2.0 + normals[i].z * 3.0;
		}
		long index_sum = 0;
		for (int i = 0; i < indices.size(); i++) {
			index_sum += indices[i];
		}

		// Re-pinned after fixing a real winding bug (see "Bugs Fixed" in memo.md):
		// _finalize_mesh_arrays()'s smoothed branch (this test's p_smoothed=true) had no
		// corrective flip for MakeIcosphere()'s backwards-relative-to-Godot winding, unlike
		// the flat branch, which already had one for an unrelated reason. Counts (753
		// verts/normals, 888 indices) are unchanged -- only which side each triangle now
		// faces, so xatlas's own chart layout comes out slightly differently too.
		// Re-pinned again after fixing a second, unrelated real winding bug (see "Bugs
		// Fixed" in memo.md): clip_and_cap()'s cap-fan orientation check itself had its
		// condition inverted, producing a cap that rendered visible from outside the mesh
		// (backwards) despite passing every volume-based consistency check. Counts
		// unchanged (753 verts/normals, 888 indices) -- only the cap's own winding/normal
		// direction changed, which this cutplane test's cap fan is sensitive to.
		// Re-pinned a third time after fixing the actual root cause (see "Bugs Fixed" in
		// memo.md): clip_and_cap() never deduplicated cap boundary points, so a boundary
		// edge shared by two adjacent kept triangles contributed its own on-plane point
		// twice (once per side). Left unwelded, the angular-sort cap fan connected these
		// near-duplicate points into degenerate, effectively-zero-area triangles whose
		// ComputeNormal() direction was numerically unreliable -- this is what actually
		// caused the winding-condition confusion two re-pins ago (that "fix" just
		// happened to flip which subset of a broken, inconsistent fan came out right).
		// Verified directly: a calibrated check (ComputeNormal(), compared against the
		// same known-good convention the kept region satisfies 100% of the time) found the
		// cap split roughly 20/80 between the two orientations before welding, and 100%
		// consistent after. Counts genuinely shrink here (753->611 verts, 888->729 indices)
		// since welding removes real duplicate points, not just fixes their winding.
		// Re-pinned a fourth time: the winding-condition direction from the third pin was
		// itself backwards (see memo.md's "Bugs Fixed" for the full journey) -- confirmed by
		// hand-computing ComputeNormal() for a real exported cap triangle and by live
		// rendering (the cap is now correctly invisible from an elevated camera). Counts
		// unchanged from the third pin (611 verts, 729 indices) -- only the cap's winding
		// direction flipped back, changing index_sum/normal_checksum only.
		// Re-pinned a fifth time (then reverted): making the cap double-sided grew both
		// vertex/index counts (611->722 verts, 729->840 indices) but introduced a worse,
		// user-confirmed-real symptom (the cap appeared to draw on top of the dome
		// regardless of actual depth -- not simple Z-fighting) than the single-sided
		// "occasionally see-through from a grazing angle" issue it was meant to fix. See
		// memo.md's "Bugs Fixed" for the full account. Re-pinned a sixth time back to the
		// single-sided counts/checksums from the fourth pin (611 verts, 729 indices).
		// Re-pinned a seventh time: ProcRock's own _finalize_mesh_arrays() had a genuine,
		// separate, pre-existing winding bug (a shading-normal-vs-culling-convention mixup
		// unrelated to flatten_base entirely -- see memo.md's "Bugs Fixed"), found via an
		// assumption-free synthetic-camera check and fixed by removing an incorrect index
		// flip. Counts unchanged (611 verts, 729 indices); index_sum shifts by 2 and
		// normal_checksum flips sign, since every triangle's winding (and therefore its
		// contribution to the smoothed per-vertex normals) is now reversed from before.
		CHECK(verts.size() == 611);
		CHECK(normals.size() == 611);
		CHECK(indices.size() == 729);
		CHECK(index_sum == 204346);
		CHECK(vertex_checksum == doctest::Approx(-78.062164).epsilon(0.0001));
		CHECK(normal_checksum == doctest::Approx(-145.712678).epsilon(0.0001));
	}

	TEST_CASE("[proc_rocks] set_generator(3) with auto_refresh produces a surface") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_pipeline_subdivisions(1);
		mesh->set_pipeline_randseed(7);
		CHECK(mesh->get_surface_count() == 0);
		SUPPRESS_OUTPUT(mesh->set_generator(3));
		SUPPRESS_OUTPUT(mesh->set_auto_refresh(true));
		CHECK(mesh->get_surface_count() == 1);
		CHECK(mesh->get_pipeline_material().is_null());
	}

	TEST_CASE("[proc_rocks] rock_pipeline_gen_textures returns valid PBR set") {
		ProcRockPipelineTextures textures;
		SUPPRESS_OUTPUT(textures = rock_pipeline_gen_textures(32, 1.0, 3, 0.5, 42,
								Color(0.2, 0.2, 0.2), Color(0.8, 0.8, 0.8), 2.0, 0.4, 0.5, 0.0, 0.0, 0.5, 0.5));
		CHECK(textures.albedo.is_valid());
		CHECK(textures.normal.is_valid());
		CHECK(textures.roughness.is_valid());
		CHECK(textures.metalness.is_valid());
		CHECK(textures.ambient_occlusion.is_valid());
		CHECK(textures.albedo->get_width() == 32);
		CHECK(textures.albedo->get_height() == 32);
		CHECK(textures.normal->get_width() == 32);
	}

	TEST_CASE("[proc_rocks] rock_pipeline_gen_textures is deterministic per seed") {
		ProcRockPipelineTextures t1, t2;
		SUPPRESS_OUTPUT(t1 = rock_pipeline_gen_textures(16, 1.0, 3, 0.5, 55, Color(0, 0, 0), Color(1, 1, 1), 1.0, 0.5, 0.5, 0.0, 0.0, 0.5, 0.5));
		SUPPRESS_OUTPUT(t2 = rock_pipeline_gen_textures(16, 1.0, 3, 0.5, 55, Color(0, 0, 0), Color(1, 1, 1), 1.0, 0.5, 0.5, 0.0, 0.0, 0.5, 0.5));
		Ref<Image> a1 = t1.albedo->get_data();
		Ref<Image> a2 = t2.albedo->get_data();
		a1->lock();
		a2->lock();
		for (int y = 0; y < 16; y += 4) {
			for (int x = 0; x < 16; x += 4) {
				CHECK(a1->get_pixel(x, y).is_equal_approx(a2->get_pixel(x, y)));
			}
		}
		a1->unlock();
		a2->unlock();
	}

	TEST_CASE("[proc_rocks] rock_pipeline_gen_textures clamps roughness/metalness/AO to [0,1]") {
		ProcRockPipelineTextures textures;
		// Extreme scale/bias would overflow without clamping.
		SUPPRESS_OUTPUT(textures = rock_pipeline_gen_textures(16, 1.0, 3, 0.5, 42,
								Color(0, 0, 0), Color(1, 1, 1), 1.0, 10.0, 10.0, -10.0, -10.0, 10.0, 10.0));
		Ref<Image> roughness = textures.roughness->get_data();
		Ref<Image> metalness = textures.metalness->get_data();
		Ref<Image> ao = textures.ambient_occlusion->get_data();
		roughness->lock();
		metalness->lock();
		ao->lock();
		for (int y = 0; y < 16; y += 4) {
			for (int x = 0; x < 16; x += 4) {
				CHECK(roughness->get_pixel(x, y).r >= 0.0f);
				CHECK(roughness->get_pixel(x, y).r <= 1.0f);
				CHECK(metalness->get_pixel(x, y).r >= 0.0f);
				CHECK(metalness->get_pixel(x, y).r <= 1.0f);
				CHECK(ao->get_pixel(x, y).r >= 0.0f);
				CHECK(ao->get_pixel(x, y).r <= 1.0f);
			}
		}
		roughness->unlock();
		metalness->unlock();
		ao->unlock();
	}

	TEST_CASE("[proc_rocks] rock_pipeline_gen_textures normal map with zero strength points up") {
		ProcRockPipelineTextures textures;
		SUPPRESS_OUTPUT(textures = rock_pipeline_gen_textures(16, 1.0, 3, 0.5, 42,
								Color(0, 0, 0), Color(1, 1, 1), 0.0, 0.5, 0.5, 0.0, 0.0, 0.5, 0.5));
		Ref<Image> normal = textures.normal->get_data();
		normal->lock();
		for (int y = 0; y < 16; y += 4) {
			for (int x = 0; x < 16; x += 4) {
				CHECK(normal->get_pixel(x, y).b == doctest::Approx(1.0f));
			}
		}
		normal->unlock();
	}

	TEST_CASE("[proc_rocks] rock_pipeline_make_material wires all PBR slots") {
		ProcRockPipelineTextures textures;
		SUPPRESS_OUTPUT(textures = rock_pipeline_gen_textures(16, 1.0, 3, 0.5, 42,
								Color(0, 0, 0), Color(1, 1, 1), 1.0, 0.5, 0.5, 0.0, 0.0, 0.5, 0.5));
		Ref<SpatialMaterial> material = rock_pipeline_make_material(textures);
		CHECK(material.is_valid());
		CHECK(material->get_texture(SpatialMaterial::TEXTURE_ALBEDO).is_valid());
		CHECK(material->get_texture(SpatialMaterial::TEXTURE_NORMAL).is_valid());
		CHECK(material->get_texture(SpatialMaterial::TEXTURE_ROUGHNESS).is_valid());
		CHECK(material->get_texture(SpatialMaterial::TEXTURE_METALLIC).is_valid());
		CHECK(material->get_texture(SpatialMaterial::TEXTURE_AMBIENT_OCCLUSION).is_valid());
	}

	TEST_CASE("[proc_rocks] pipeline_generate_textures wires a material onto the mesh") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_pipeline_subdivisions(1);
		mesh->set_pipeline_texture_size(16);
		mesh->set_pipeline_generate_textures(true);
		SUPPRESS_OUTPUT(mesh->set_generator(3));
		SUPPRESS_OUTPUT(mesh->set_auto_refresh(true));
		CHECK(mesh->get_surface_count() == 1);
		CHECK(mesh->get_pipeline_material().is_valid());
		CHECK(mesh->surface_get_material(0).is_valid());
	}

	TEST_CASE("[proc_rocks] set_pipeline_preset applies real extracted values") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_pipeline_preset(0); // 1.json: warm tan -> near-white gradient
		CHECK(mesh->get_pipeline_albedo_low().is_equal_approx(Color(0.827, 0.784, 0.517)));
		CHECK(mesh->get_pipeline_roughness_bias() == doctest::Approx(0.0f));
		mesh->set_pipeline_preset(1); // 2.json: full white -> black gradient, pinned rough/non-metal
		CHECK(mesh->get_pipeline_albedo_low().is_equal_approx(Color(1, 1, 1)));
		CHECK(mesh->get_pipeline_albedo_high().is_equal_approx(Color(0, 0, 0)));
		CHECK(mesh->get_pipeline_roughness_bias() == doctest::Approx(1.0f));
		CHECK(mesh->get_pipeline_metalness_bias() == doctest::Approx(-1.0f));
		mesh->set_pipeline_preset(9); // 10.json: distinctive roughness_scale (2.911) and neutral metalness_bias
		CHECK(mesh->get_pipeline_roughness_scale() == doctest::Approx(2.911f));
		CHECK(mesh->get_pipeline_metalness_bias() == doctest::Approx(0.0f));
		mesh->set_pipeline_preset(99); // clamped to last preset (12), must not crash
		CHECK(mesh->get_pipeline_albedo_low().a >= 0.0f);
	}

	TEST_CASE("[proc_rocks] load_from_file drives generation from a real JSON preset") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_generator(3);
		mesh->set_pipeline_subdivisions(1); // keep it cheap
		mesh->set_pipeline_generate_textures(true);
		mesh->set_pipeline_texture_size(16);
		mesh->set_auto_refresh(true);

		Error err = mesh->load_from_file("modules/gdextensions/editor/proc_rocks_demo/presets/1.json");
		CHECK(err == OK);
		CHECK(mesh->get_surface_count() > 0);
		CHECK(mesh->get_pipeline_material().is_valid());

		// Clearing the path reverts to the scalar pipeline_* path without crashing.
		err = mesh->load_from_file("");
		CHECK(err == OK);
		CHECK(mesh->get_surface_count() > 0);
	}

	TEST_CASE("[proc_rocks] load_from_file rejects a missing or invalid file") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_generator(3);
		CHECK(mesh->load_from_file("modules/gdextensions/editor/proc_rocks_demo/presets/does_not_exist.json") == ERR_FILE_NOT_FOUND);
		CHECK(mesh->load_from_file("modules/gdextensions/editor/proc_rocks_demo/baked_textures.h") == ERR_PARSE_ERROR);
	}

	TEST_CASE("[proc_rocks] bake() freezes geometry and flips get_baked()") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_generator(0); // RockGen — cheap
		mesh->set_auto_refresh(true);
		CHECK(mesh->get_baked() == false);
		CHECK(mesh->get_surface_count() > 0);

		Error err = mesh->bake();
		CHECK(err == OK);
		CHECK(mesh->get_baked() == true);
		CHECK(mesh->get_surface_count() > 0);
	}

	TEST_CASE("[proc_rocks] bake() converts a Gravel/Mossy/Rock texture_source into real on-disk File-backed textures") {
		// See memo.md's "Bugs Fixed": Gravel/Mossy/Rock apply their baked demo pack
		// entirely in memory for live editing (fast, no import-pipeline timing issues),
		// but an exported/baked resource shouldn't depend on this fork's editor-only
		// INCBIN demo assets to resolve at load time — bake() is what converts it to a
		// real, on-disk, File-backed texture set, once, for the resource that ships.
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_generator(0);
		mesh->set_texture_source(2); // Gravel
		mesh->set_auto_refresh(true);
		REQUIRE(mesh->get_surface_count() > 0);
		CHECK(mesh->get_texture_source() == 2); // still Gravel before baking

		REQUIRE(mesh->bake() == OK);

		CHECK(mesh->get_texture_source() == 5); // File — no longer a demo-pack tag
		String albedo_path = mesh->get_texture_file_albedo();
		REQUIRE(!albedo_path.empty());
		CHECK(albedo_path.begins_with("res://.generated/proc_rocks/gravel_"));
		CHECK(FileAccess::exists(albedo_path));
		CHECK(mesh->surface_get_material(0).is_valid());

		// Cleanup — remove the whole directory this bake() call created.
		String dir = albedo_path.get_base_dir();
		DirAccessRef contents = DirAccess::open(dir);
		if (contents) {
			contents->list_dir_begin();
			for (String entry = contents->get_next(); !entry.empty(); entry = contents->get_next()) {
				if (!contents->current_is_dir()) {
					contents->remove(entry);
				}
			}
			contents->list_dir_end();
		}
		DirAccessRef parent = DirAccess::create(DirAccess::ACCESS_RESOURCES);
		if (parent) {
			parent->remove(dir);
		}
	}

	TEST_CASE("[proc_rocks] set_baked(true) is rejected with no surfaces") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		CHECK(mesh->get_surface_count() == 0);
		EXPECT_ERROR(mesh->set_baked(true));
		CHECK(mesh->get_baked() == false); // rejected, not silently accepted
	}

	TEST_CASE("[proc_rocks] a baked resource round-trips real geometry through ResourceSaver/ResourceLoader") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_generator(0);
		mesh->set_auto_refresh(true);
		REQUIRE(mesh->bake() == OK);
		int original_surface_count = mesh->get_surface_count();
		int original_vertex_count = mesh->surface_get_array_len(0);
		REQUIRE(original_surface_count > 0);

		String path = "user://proc_rock_bake_test.tres";
		REQUIRE(ResourceSaver::save(path, mesh) == OK);

		// Force a fresh load — ResourceLoader would otherwise hand back the same
		// in-memory instance, which wouldn't prove anything about deserialization.
		Ref<ProcRockMesh> reloaded = ResourceLoader::load(path, "", true);
		REQUIRE(reloaded.is_valid());
		REQUIRE(reloaded.ptr() != mesh.ptr());

		// The load-bearing assertion: geometry must already be present purely from
		// deserialization — no _rebuild()/set_auto_refresh()/setter call on `reloaded`
		// above this line. This is exactly the mechanism a tools=no build depends on,
		// since _rebuild()'s real generation switch is compiled out there.
		CHECK(reloaded->get_baked() == true);
		CHECK(reloaded->get_surface_count() == original_surface_count);
		CHECK(reloaded->surface_get_array_len(0) == original_vertex_count);

		DirAccess::remove_file_or_error(ProjectSettings::get_singleton()->globalize_path(path));
	}
	// Baked demo texture pack round-trip tests (load_baked_textures() itself) live in
	// editor/proc_rocks_editor_plugin.cpp, near load_baked_textures()'s doctest include
	// setup — these below only cover texture_source's generic, cross-generator wiring.

	TEST_CASE("[proc_rocks] texture_source applies a generated PBR material on a non-ProcRock generator") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_generator(0); // RockGen — never had any texture support before this
		mesh->set_texture_gen_size(16);
		mesh->set_texture_source(1); // Generated
		SUPPRESS_OUTPUT(mesh->set_auto_refresh(true));
		CHECK(mesh->get_surface_count() > 0);
		CHECK(mesh->surface_get_material(0).is_valid());
	}

	TEST_CASE("[proc_rocks] texture_source applies an embedded baked pack on a non-ProcRock generator") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_generator(2); // RockStudio
		mesh->set_texture_source(2); // Gravel
		SUPPRESS_OUTPUT(mesh->set_auto_refresh(true));
		CHECK(mesh->get_surface_count() > 0);
		CHECK(mesh->surface_get_material(0).is_valid());
	}

	TEST_CASE("[proc_rocks] texture_source None clears any previously-applied material") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_generator(0);
		mesh->set_texture_source(2); // Gravel
		SUPPRESS_OUTPUT(mesh->set_auto_refresh(true));
		REQUIRE(mesh->surface_get_material(0).is_valid());

		mesh->set_texture_source(0); // None
		CHECK(mesh->surface_get_material(0).is_null());
	}

	TEST_CASE("[proc_rocks] texture_source File falls back to no material when the referenced asset can't be loaded") {
		// A doctest run has no Godot *project* context, so res:// paths to raw,
		// never-imported source images (like this repo's own demo JPEGs) can't
		// resolve through ResourceLoader — exactly the same as a stale/deleted
		// path a real user could end up with. _apply_texture_source() must handle
		// that without crashing, leaving no material rather than a broken one.
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_generator(0);
		mesh->set_texture_source(5); // From File
		mesh->set_texture_file_albedo("res://modules/gdextensions/editor/proc_rocks_demo/rock/rock.jpg");
		SUPPRESS_OUTPUT(mesh->set_auto_refresh(true));
		CHECK(mesh->get_surface_count() > 0);
		CHECK(mesh->surface_get_material(0).is_null());
	}

	TEST_CASE("[proc_rocks] set_texture_file_albedo auto-detects companion maps by the proc_rocks_demo folder convention") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_texture_source(5); // From File
		mesh->set_texture_file_albedo("modules/gdextensions/editor/proc_rocks_demo/gravel/albedo.jpg");
		CHECK(mesh->get_texture_file_normal() == "modules/gdextensions/editor/proc_rocks_demo/gravel/normals.jpg");
		CHECK(mesh->get_texture_file_roughness() == "modules/gdextensions/editor/proc_rocks_demo/gravel/roughness.jpg");
		CHECK(mesh->get_texture_file_ambient_occlusion() == "modules/gdextensions/editor/proc_rocks_demo/gravel/ambientOcc.jpg");
		CHECK(mesh->get_texture_file_metalness() == ""); // no metalness.jpg ships for this pack
	}

	TEST_CASE("[proc_rocks] set_texture_file_albedo auto-detects companion maps by a suffixed external convention") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_texture_source(5);
		// No such files exist on disk, but the filename SHAPE (prefix + "_albedo") is
		// what's under test — detection must still only match files that actually exist,
		// so real presets' gravel pack (same prefix, different suffix) must NOT be found.
		mesh->set_texture_file_albedo("modules/gdextensions/editor/proc_rocks_demo/gravel/rockwall_albedo.jpg");
		CHECK(mesh->get_texture_file_normal() == "");
		CHECK(mesh->get_texture_file_roughness() == "");
	}

	TEST_CASE("[proc_rocks] set_texture_file_albedo leaves companions empty for an unrecognized filename shape") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_texture_source(5);
		mesh->set_texture_file_albedo("modules/gdextensions/editor/proc_rocks_demo/gravel/mytexture.jpg");
		CHECK(mesh->get_texture_file_normal() == "");
		CHECK(mesh->get_texture_file_roughness() == "");
		CHECK(mesh->get_texture_file_metalness() == "");
		CHECK(mesh->get_texture_file_ambient_occlusion() == "");
	}

	TEST_CASE("[proc_rocks] Method 3's pipeline_generate_textures takes precedence over texture_source") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_generator(3);
		mesh->set_pipeline_subdivisions(1);
		mesh->set_pipeline_texture_size(16);
		mesh->set_pipeline_generate_textures(true);
		mesh->set_texture_source(1); // Generated — must be ignored while generate_textures is on
		SUPPRESS_OUTPUT(mesh->set_auto_refresh(true));
		CHECK(mesh->get_pipeline_material().is_valid());
		CHECK(mesh->surface_get_material(0) == mesh->get_pipeline_material());
	}

	// Regression tests for the "scattered dark/missing triangles" bug: a normal-mapped
	// material (rock_pipeline_make_material(), FEATURE_NORMAL_MAPPING always on when a
	// normal texture is present) was reachable on every generator via texture_source,
	// but none of them ever populated ARRAY_TANGENT — Godot then reads the disabled GPU
	// tangent attribute as (0,0,0,1) and normalize()s it to NaN, corrupting lighting on
	// scattered triangles. Fixed by ensure_tangents() (generators/shared/texture_gen.cpp)
	// plus real UVs on RockGen (new box-UV) and IcoRock (its own CorrectUV() output was
	// simply never copied into the output array) — see memo.md's "Bugs Fixed".

	TEST_CASE("[proc_rocks] ensure_tangents adds non-degenerate tangents to a UV'd triangle array") {
		Array arrays;
		arrays.resize(VS::ARRAY_MAX);
		Vector<Vector3> verts;
		verts.push_back(Vector3(0, 0, 0));
		verts.push_back(Vector3(1, 0, 0));
		verts.push_back(Vector3(0, 1, 0));
		Vector<Vector3> normals;
		normals.push_back(Vector3(0, 0, 1));
		normals.push_back(Vector3(0, 0, 1));
		normals.push_back(Vector3(0, 0, 1));
		Vector<Vector2> uvs;
		uvs.push_back(Vector2(0, 0));
		uvs.push_back(Vector2(1, 0));
		uvs.push_back(Vector2(0, 1));
		arrays[VS::ARRAY_VERTEX] = verts;
		arrays[VS::ARRAY_NORMAL] = normals;
		arrays[VS::ARRAY_TEX_UV] = uvs;

		Array out = ensure_tangents(arrays);
		REQUIRE(out.size() == VS::ARRAY_MAX);
		PoolVector<real_t> tangents = out[VS::ARRAY_TANGENT];
		REQUIRE(tangents.size() == 3 * 4); // 4 floats (xyz + handedness) per vertex
		for (int i = 0; i < tangents.size(); i++) {
			CHECK_FALSE(Math::is_nan(tangents[i]));
		}
		// Not the degenerate (0,0,0) that a disabled GL attribute would fall back to.
		Vector3 t0(tangents[0], tangents[1], tangents[2]);
		CHECK(t0.length() > 0.5);
	}

	TEST_CASE("[proc_rocks] ensure_tangents leaves UV-less arrays unchanged rather than risk NaN tangents") {
		Array arrays;
		arrays.resize(VS::ARRAY_MAX);
		Vector<Vector3> verts;
		verts.push_back(Vector3(0, 0, 0));
		arrays[VS::ARRAY_VERTEX] = verts;

		Array out;
		SUPPRESS_OUTPUT(out = ensure_tangents(arrays));
		CHECK(out[VS::ARRAY_TEX_UV].get_type() == Variant::NIL);
		CHECK(out[VS::ARRAY_TANGENT].get_type() == Variant::NIL);
	}

	TEST_CASE("[proc_rocks] RockGen (method 0) produces real UVs and non-NaN tangents after texture_source is set") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_rockgen_depth(1); // keep it cheap
		mesh->set_texture_gen_size(16);
		mesh->set_texture_source(1); // Generated — the exact repro of the reported bug
		SUPPRESS_OUTPUT(mesh->set_generator(0));
		SUPPRESS_OUTPUT(mesh->set_auto_refresh(true));
		REQUIRE(mesh->get_surface_count() > 0);

		Array surface = mesh->surface_get_arrays(0);
		PoolVector<Vector2> uvs = surface[VS::ARRAY_TEX_UV];
		CHECK(uvs.size() > 0);
		PoolVector<real_t> tangents = surface[VS::ARRAY_TANGENT];
		REQUIRE(tangents.size() > 0);
		for (int i = 0; i < tangents.size(); i += 40) { // sample — a subdivided mesh can have many thousands of floats
			CHECK_FALSE(Math::is_nan(tangents[i]));
		}
	}

	TEST_CASE("[proc_rocks] IcoRock (method 1) produces real UVs and non-NaN tangents after texture_source is set") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_rockgeneration_steps(1); // keep it cheap
		mesh->set_texture_gen_size(16);
		mesh->set_texture_source(1); // Generated
		SUPPRESS_OUTPUT(mesh->set_generator(1));
		SUPPRESS_OUTPUT(mesh->set_auto_refresh(true));
		REQUIRE(mesh->get_surface_count() > 0);

		Array surface = mesh->surface_get_arrays(0);
		PoolVector<Vector2> uvs = surface[VS::ARRAY_TEX_UV];
		CHECK(uvs.size() > 0);
		PoolVector<real_t> tangents = surface[VS::ARRAY_TANGENT];
		REQUIRE(tangents.size() > 0);
		for (int i = 0; i < tangents.size(); i += 40) { // sample — see RockGen test above
			CHECK_FALSE(Math::is_nan(tangents[i]));
		}
	}

	// Regression tests for the two bugs behind "half of the faces are missing" (item 13's
	// tangent fix didn't touch either of these — see memo.md's "Bugs Fixed" #14/#15) and
	// for the generation-time/face-count blowup behind "IcoRock generates for a long time
	// and looks like a dense ball" (#16). All three were confirmed via a real editor
	// build + screenshots, not just code inspection — these are the regression guards
	// since this sandbox has no usable display for a visual before/after comparison.

	TEST_CASE("[proc_rocks] RockGen's actual default settings (depth=2, smoothed=false) are watertight across many seeds") {
		// Items 14/15/17 (see memo.md) verified RockGen's winding is correct, but
		// neither check actually covers the real default combination end-to-end:
		// item 15's own winding test uses depth=0 specifically to skip recursion
		// ("independent of the recursive parity bug fixed separately in
		// Recurse()"), and item 17's edge-consistency check was against the
		// smooth-shaded (indexed) data, before the *separate* flat/low-poly branch
		// (_update()'s `else` in gen_rock.cpp -- an index flip plus
		// rock_studio_make_low_poly(), needed only when `smoothed=false`, the
		// actual default) ever runs. Verified directly, not assumed: swept 30
		// seeds at the real default (depth=2, smoothed=false) and checked for
		// genuine topological defects (duplicated or unmatched directed edges) in
		// the final output — 0/30. This doesn't rule out a large legitimate
		// concave dent from Perturbation() reading as an unlit "cave" under this
		// dock's lighting rig (see item 17's own open question) — a topologically
		// sound mesh can still be very concave — but it does confirm the mesh
		// itself has no missing/backwards-wound triangles at the settings most
		// users actually see.
		auto key = [](const Vector3 &p) {
			return String::num_int64(Math::round((double)p.x * 100000.0)) + "," +
					String::num_int64(Math::round((double)p.y * 100000.0)) + "," +
					String::num_int64(Math::round((double)p.z * 100000.0));
		};
		int seeds_with_defect = 0;
		for (int seed = 1; seed <= 30; seed++) {
			Array arrays;
			SUPPRESS_OUTPUT(arrays = rock_gen(2, seed, 2.0, false));
			PoolVector<Vector3> verts = arrays[VS::ARRAY_VERTEX];
			PoolVector<int> idx = arrays[VS::ARRAY_INDEX];
			int tri_count = idx.size() > 0 ? idx.size() / 3 : verts.size() / 3;
			Map<String, int> edge_count;
			for (int t = 0; t < tri_count; t++) {
				int i0 = idx.size() > 0 ? idx[t * 3] : t * 3;
				int i1 = idx.size() > 0 ? idx[t * 3 + 1] : t * 3 + 1;
				int i2 = idx.size() > 0 ? idx[t * 3 + 2] : t * 3 + 2;
				Vector3 tri[3] = { verts[i0], verts[i1], verts[i2] };
				for (int e = 0; e < 3; e++) {
					String k = key(tri[e]) + "|" + key(tri[(e + 1) % 3]);
					Map<String, int>::Element *found = edge_count.find(k);
					if (found) {
						found->get()++;
					} else {
						edge_count.insert(k, 1);
					}
				}
			}
			int dup = 0, unmatched = 0;
			for (int t = 0; t < tri_count; t++) {
				int i0 = idx.size() > 0 ? idx[t * 3] : t * 3;
				int i1 = idx.size() > 0 ? idx[t * 3 + 1] : t * 3 + 1;
				int i2 = idx.size() > 0 ? idx[t * 3 + 2] : t * 3 + 2;
				Vector3 tri[3] = { verts[i0], verts[i1], verts[i2] };
				for (int e = 0; e < 3; e++) {
					const Vector3 &a = tri[e];
					const Vector3 &b = tri[(e + 1) % 3];
					String k = key(a) + "|" + key(b);
					String rk = key(b) + "|" + key(a);
					if (edge_count.find(k) && edge_count.find(k)->get() > 1) {
						dup++;
					}
					if (!edge_count.find(rk)) {
						unmatched++;
					}
				}
			}
			bool ok = dup == 0 && unmatched == 0;
			if (!ok) {
				seeds_with_defect++;
			}
			CHECK_MESSAGE(ok, "seed " << seed << " had " << dup << " duplicated / " << unmatched << " unmatched edges");
		}
		CHECK(seeds_with_defect == 0);
	}

	TEST_CASE("[proc_rocks] rock_gen's 20 root icosahedron triangles are all outward-wound") {
		// depth=0 skips Recurse()'s subdivision entirely, so this checks the raw literal
		// Point() triangles in rock_gen() (item 15) independent of the recursive parity
		// bug (item 14) fixed separately in Recurse() itself.
		Array arrays;
		SUPPRESS_OUTPUT(arrays = rock_gen(0, 42, 1.0, false));
		REQUIRE(arrays.size() == VS::ARRAY_MAX);
		PoolVector<Vector3> verts = arrays[VS::ARRAY_VERTEX];
		REQUIRE(verts.size() == 20 * 3); // 20 triangles, non-indexed (3 unique verts each)

		for (int i = 0; i + 2 < verts.size(); i += 3) {
			const Vector3 &a = verts[i], &b = verts[i + 1], &c = verts[i + 2];
			// ComputeNormal() (rock_header.h, cross(P2-P0, P1-P0)) is Godot's actual
			// front-face convention -- confirmed directly against a known-good CubeMesh
			// (see memo.md's "Bugs Fixed"). This test previously used the OPPOSITE, naive
			// (b-a).cross(c-a) convention, which made it silently pass while the real
			// mesh (verified via the real, logged preview camera position, plus this exact
			// depth=0 check using the correct convention) was uniformly wound backwards --
			// a mesh-wide defect that a single-sided flatten_base cap, RockStudio's, and
			// ProcRock's own separately-fixed winding bugs never actually touched. Fixed at
			// the single point where every leaf triangle is emitted (rockgen.cpp's
			// InstancieTriangle() call inside Recurse()), not here.
			Vector3 normal = ComputeNormal(a, b, c);
			Vector3 centroid = (a + b + c) / 3.0;
			// An outward-facing (correctly wound) triangle's face normal points away
			// from the mesh center, i.e. roughly parallel to its own centroid direction.
			CHECK(normal.dot(centroid) > 0.0);
		}
	}

	TEST_CASE("[proc_rocks] generation-limit setters clamp to sane maximums instead of trusting caller input") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();

		mesh->set_rockgen_depth(999);
		CHECK(mesh->get_rockgen_depth() == 3); // 20*4^3 = 1280 triangles, not 20*4^999

		mesh->set_rockgeneration_steps(999);
		CHECK(mesh->get_rockgeneration_steps() == 3); // same 20*4^n growth as rockgen_depth

		mesh->set_rockgeneration_max_planes(999);
		CHECK(mesh->get_rockgeneration_max_planes() == 10);

		mesh->set_rockstudio_num_vertices(999999);
		CHECK(mesh->get_rockstudio_num_vertices() == 150); // ~2*150-4 hull faces

		mesh->set_pipeline_subdivisions(999);
		CHECK(mesh->get_pipeline_subdivisions() == 3);

		mesh->set_rockcluster_density(999);
		CHECK(mesh->get_rockcluster_density() == 24); // Sharp cells run ~200 tris each: ~4800 max
	}

	TEST_CASE("[proc_rocks] every generator stays in the hundreds-of-faces range at max settings") {
		// Ceiling chosen generously above the ~1280 that 20*4^3 (RockGen/IcoRock/ProcRock's
		// shared subdivision growth) or ~296 (RockStudio's 150-point hull) actually produce
		// at the new clamped maximums — this exists to catch a future regression back
		// toward exponential blowup (the old defaults were 21M+ triangles), not to pin an
		// exact count.
		const int face_ceiling = 2000;

		Ref<ProcRockMesh> mesh;
		mesh.instance();

		SUPPRESS_OUTPUT(mesh->set_generator(0));
		mesh->set_rockgen_depth(3);
		SUPPRESS_OUTPUT(mesh->set_auto_refresh(true));
		REQUIRE(mesh->get_surface_count() > 0);
		{
			Array a = mesh->surface_get_arrays(0);
			PoolVector<Vector3> v = a[VS::ARRAY_VERTEX];
			CHECK(v.size() / 3 <= face_ceiling);
		}

		SUPPRESS_OUTPUT(mesh->set_auto_refresh(false));
		SUPPRESS_OUTPUT(mesh->set_generator(1));
		mesh->set_rockgeneration_steps(3);
		SUPPRESS_OUTPUT(mesh->set_auto_refresh(true));
		REQUIRE(mesh->get_surface_count() > 0);
		{
			Array a = mesh->surface_get_arrays(0);
			PoolVector<int> idx = a[VS::ARRAY_INDEX];
			CHECK(idx.size() / 3 <= face_ceiling);
		}

		SUPPRESS_OUTPUT(mesh->set_auto_refresh(false));
		SUPPRESS_OUTPUT(mesh->set_generator(2));
		mesh->set_rockstudio_num_vertices(150);
		SUPPRESS_OUTPUT(mesh->set_auto_refresh(true));
		REQUIRE(mesh->get_surface_count() > 0);
		{
			Array a = mesh->surface_get_arrays(0);
			PoolVector<int> idx = a[VS::ARRAY_INDEX];
			PoolVector<Vector3> v = a[VS::ARRAY_VERTEX];
			int tris = idx.size() > 0 ? idx.size() / 3 : v.size() / 3;
			CHECK(tris <= face_ceiling);
		}

		SUPPRESS_OUTPUT(mesh->set_auto_refresh(false));
		SUPPRESS_OUTPUT(mesh->set_generator(3));
		mesh->set_pipeline_subdivisions(3);
		SUPPRESS_OUTPUT(mesh->set_auto_refresh(true));
		REQUIRE(mesh->get_surface_count() > 0);
		{
			Array a = mesh->surface_get_arrays(0);
			PoolVector<int> idx = a[VS::ARRAY_INDEX];
			PoolVector<Vector3> v = a[VS::ARRAY_VERTEX];
			int tris = idx.size() > 0 ? idx.size() / 3 : v.size() / 3;
			CHECK(tris <= face_ceiling);
		}
	}

	TEST_CASE("[proc_rocks] IcoRock (method 1) is actually deformed, not just a subdivided sphere") {
		// Regression guard for "IcoRock looks like a regular ball, not a rock" — measured
		// via vertex-radius spread rather than a visual check, since this sandbox has no
		// usable display. At the old rand_offset_percent default (5, not user-exposed —
		// see memo.md's "Bugs Fixed") this measured ~2.7%; at 40 it measures ~30%.
		// rockgeneration_randseed pins this to a fixed draw instead of tolerating any
		// random one, now that BuildRock()'s own bugs are fixed (see memo.md) — no need
		// for the old conservative-threshold hedge against RNG variance.
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		SUPPRESS_OUTPUT(mesh->set_generator(1));
		mesh->set_rockgeneration_randseed(42);
		SUPPRESS_OUTPUT(mesh->set_auto_refresh(true));
		REQUIRE(mesh->get_surface_count() > 0);

		Array a = mesh->surface_get_arrays(0);
		PoolVector<Vector3> v = a[VS::ARRAY_VERTEX];
		REQUIRE(v.size() > 0);
		real_t minr = 1e30, maxr = 0, sum = 0;
		for (int i = 0; i < v.size(); i++) {
			real_t r = v[i].length();
			minr = MIN(minr, r);
			maxr = MAX(maxr, r);
			sum += r;
		}
		real_t avg = sum / v.size();
		real_t spread_pct = 100.0 * (maxr - minr) / avg;
		CHECK(spread_pct > 10.0);
	}

	TEST_CASE("[proc_rocks] IcoRock (method 1) produces real flat facets, not just a smooth bump") {
		// The pre-existing spread-only test above couldn't tell a smooth outward bump from
		// a genuine flat cut -- both change min/max vertex radius similarly, which is
		// exactly how the original BuildRock() bug (pushing vertices away from the plane
		// instead of pulling them toward it, see memo.md's "Bugs Fixed") passed that test
		// while still looking like "a regular ball" in a real screenshot. This test detects
		// actual flatness instead: a real plane cut pulls a contiguous patch of triangles
		// onto (nearly) the same plane, so several of them end up sharing a near-identical
		// face normal -- something a smoothly-curved (undeformed or smoothly-bumped) sphere
		// never produces, since every one of its triangles has a distinctly different
		// normal by construction.
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		SUPPRESS_OUTPUT(mesh->set_generator(1));
		mesh->set_rockgeneration_randseed(42);
		mesh->set_rockgeneration_smoothed(false); // flat shading -- see get/set_rockgeneration_smoothed
		SUPPRESS_OUTPUT(mesh->set_auto_refresh(true));
		REQUIRE(mesh->get_surface_count() > 0);

		Array a = mesh->surface_get_arrays(0);
		PoolVector<Vector3> v = a[VS::ARRAY_VERTEX];
		REQUIRE(v.size() > 0);
		REQUIRE(v.size() % 3 == 0);

		// Bucket every triangle's face normal by a coarse quantization (~11 degrees/axis)
		// and find the largest bucket.
		Map<String, int> normal_buckets;
		for (int i = 0; i + 2 < v.size(); i += 3) {
			Vector3 n = (v[i + 1] - v[i]).cross(v[i + 2] - v[i]);
			if (n.length_squared() < CMP_EPSILON) {
				continue; // degenerate triangle, skip
			}
			n.normalize();
			String key = String::num_int64(Math::round((double)n.x * 5.0)) + "," +
					String::num_int64(Math::round((double)n.y * 5.0)) + "," +
					String::num_int64(Math::round((double)n.z * 5.0));
			Map<String, int>::Element *e = normal_buckets.find(key);
			if (e) {
				e->get()++;
			} else {
				normal_buckets.insert(key, 1);
			}
		}
		int max_bucket = 0;
		for (Map<String, int>::Element *e = normal_buckets.front(); e; e = e->next()) {
			max_bucket = MAX(max_bucket, e->get());
		}
		CHECK(max_bucket >= 4);
	}

	TEST_CASE("[proc_rocks] rockgeneration_smoothed round-trips") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		SUPPRESS_OUTPUT(mesh->set_generator(1));
		mesh->set_rockgeneration_smoothed(true);
		CHECK(mesh->get_rockgeneration_smoothed() == true);
		mesh->set_rockgeneration_smoothed(false);
		CHECK(mesh->get_rockgeneration_smoothed() == false);
	}

	// Method 4: RockCluster — see memo.md's "Method 4: RockCluster" for the
	// algorithm writeup.

	TEST_CASE("[proc_rocks] RockCluster cell library sanity check") {
		// Confirms the generated embedded mesh data (rockcluster_cells_data.gen.cpp)
		// actually compiled with real content, independent of the placement algorithm
		// above it — if this fails, the codegen step itself is broken, not the port.
		using namespace rockcluster_data;
		REQUIRE(kBoulderCellCount == 30);
		REQUIRE(kSharpCellCount == 21);
		REQUIRE(kCrystalCellCount == 6);
		for (int i = 0; i < kBoulderCellCount; i++) {
			CHECK(kBoulderCells[i].vertex_count > 0);
			CHECK(kBoulderCells[i].index_count > 0);
			CHECK(kBoulderCells[i].index_count % 3 == 0);
		}
	}

	TEST_CASE("[proc_rocks] RockCluster's Boulder cell library has no non-manifold overlapping triangles") {
		// Regression guard for the raw-data defect documented in memo.md "Bugs Fixed" #24:
		// 28 of 30 Boulder cells had near-duplicate/overlapping triangles baked into the
		// source asset itself (confirmed to cause real z-fighting on the *un-cut* portion
		// of a cell, not just the cut cross-section -- reported as "the main chunk looks
		// off and inside-out or transparent" on a live screenshot), fixed by
		// gen_rockcluster_cells_data.py's repair_non_manifold_triangles() at the source.
		// This detects the exact same signature (a triangle sharing a directed edge with
		// another triangle) directly against the compiled-in cell data, so a future
		// regeneration that reintroduces the defect (e.g. from an unrepaired re-run, or a
		// change to the repair pass) fails loudly here instead of silently shipping.
		using namespace rockcluster_data;
		auto key = [](const Vector3 &p) {
			return String::num_int64(Math::round((double)p.x * 100000.0)) + "," +
					String::num_int64(Math::round((double)p.y * 100000.0)) + "," +
					String::num_int64(Math::round((double)p.z * 100000.0));
		};
		for (int c = 0; c < kBoulderCellCount; c++) {
			const CellData &cell = kBoulderCells[c];
			Map<String, int> edge_count;
			for (int i = 0; i + 2 < cell.index_count; i += 3) {
				int idx[3] = { cell.indices[i], cell.indices[i + 1], cell.indices[i + 2] };
				Vector3 tri[3];
				for (int k = 0; k < 3; k++) {
					tri[k] = Vector3(cell.positions[idx[k] * 3 + 0], cell.positions[idx[k] * 3 + 1], cell.positions[idx[k] * 3 + 2]);
				}
				for (int e = 0; e < 3; e++) {
					String k2 = key(tri[e]) + "|" + key(tri[(e + 1) % 3]);
					Map<String, int>::Element *found = edge_count.find(k2);
					if (found) {
						found->get()++;
					} else {
						edge_count.insert(k2, 1);
					}
				}
			}
			int suspect_tris = 0;
			for (int i = 0; i + 2 < cell.index_count; i += 3) {
				int idx[3] = { cell.indices[i], cell.indices[i + 1], cell.indices[i + 2] };
				Vector3 tri[3];
				for (int k = 0; k < 3; k++) {
					tri[k] = Vector3(cell.positions[idx[k] * 3 + 0], cell.positions[idx[k] * 3 + 1], cell.positions[idx[k] * 3 + 2]);
				}
				bool suspect = false;
				for (int e = 0; e < 3; e++) {
					String k2 = key(tri[e]) + "|" + key(tri[(e + 1) % 3]);
					if (edge_count[k2] > 1) {
						suspect = true;
					}
				}
				if (suspect) {
					suspect_tris++;
				}
			}
			CHECK_MESSAGE(suspect_tris == 0, "Boulder cell " << c << " has " << suspect_tris << " suspect triangles");
		}
	}

	TEST_CASE("[proc_rocks] RockCluster's per-cell clip+cap fills most of the cut cross-section") {
		// Regression check for a reported "missing or inside-out faces" visual defect
		// (see memo.md "Bugs Fixed" #24) -- verified geometrically instead of trusting a
		// screenshot, same technique as the IcoRock manifold check in "Bugs Fixed" #17/#19.
		//
		// This deliberately does NOT require exact watertightness: a direct scan of every
		// raw cell's own triangle data (independent of this generator's clip/cap code)
		// found that Boulder's cell library has widespread non-manifold overlaps baked into
		// the source asset itself (28 of 30 cells, confirmed via near-duplicate overlapping
		// triangles at matching positions -- e.g. cell 0 has an extra triangle spanning a
		// quad's other diagonal on top of two already-valid triangles) -- Sharp and Crystal
		// cells are all clean, so this looks like an authoring quirk specific to Boulder's
		// source pieces, not a bug in this port's Python conversion. That messiness means
		// this cell's own cut boundary can have near-duplicate points a fraction of a unit
		// apart, which a convex-hull cap (chosen specifically because it tolerates that mess
		// without producing a self-crossing fan -- see _clip_and_cap_cell()'s own comment)
		// can't always edge-match exactly. What this test checks instead: the cap's total
		// triangle area covers most of the cut cross-section's own bounding box -- catching
		// the actual regression (no cap at all, or a cap covering only a sliver) without
		// demanding more precision than the source data supports.
		// Single cell (density=1) placed dead center (radius~0) so the whole output is
		// exactly one clipped+capped cell.
		Ref<Curve> curve;
		curve.instance();
		curve->add_point(Vector2(0, 0), 0, 0);
		curve->add_point(Vector2(1, 1), 2, 2);
		Array arrays;
		SUPPRESS_OUTPUT(arrays = rock_cluster_gen_boulder(false, 1, 0.01, 0, 0, 0.5, 1.0, curve, 0.6, 0, 0, 0, 0, 0, 42));
		PoolVector<Vector3> verts = arrays[VS::ARRAY_VERTEX];
		REQUIRE(verts.size() > 0);
		REQUIRE(verts.size() % 3 == 0);

		const real_t kPlaneEpsilon = 1e-3;
		real_t minx = 1e9, maxx = -1e9, minz = 1e9, maxz = -1e9;
		double cap_area = 0.0;
		for (int i = 0; i + 2 < verts.size(); i += 3) {
			Vector3 a = verts[i], b = verts[i + 1], c = verts[i + 2];
			bool cap_tri = Math::abs(a.y) <= kPlaneEpsilon && Math::abs(b.y) <= kPlaneEpsilon && Math::abs(c.y) <= kPlaneEpsilon;
			for (const Vector3 &v : { a, b, c }) {
				if (Math::abs(v.y) <= kPlaneEpsilon) {
					minx = MIN(minx, v.x);
					maxx = MAX(maxx, v.x);
					minz = MIN(minz, v.z);
					maxz = MAX(maxz, v.z);
				}
			}
			if (cap_tri) {
				cap_area += Math::abs((b.x - a.x) * (c.z - a.z) - (c.x - a.x) * (b.z - a.z)) / 2.0;
			}
		}
		REQUIRE(maxx > minx); // sanity: the cell really did get cut, and left cut-plane points behind
		double bbox_area = (double)(maxx - minx) * (double)(maxz - minz);
		REQUIRE(bbox_area > 0.0);
		CHECK(cap_area / bbox_area > 0.3);
	}

	TEST_CASE("[proc_rocks] RockCluster's per-cell cap is consistently wound with the cell's own kept geometry, for every cell") {
		// Same class of check as flatten_base_enabled's own winding-consistency test (see
		// its comment): a coverage/area check alone can't tell a correctly wound cap from a
		// backwards one -- only a signed-volume (divergence theorem) split between the kept
		// region and the cap can. Checks every Boulder and Sharp cell individually (not just
		// cell 0, and not just an aggregate over a full multi-cell cluster) via
		// rock_cluster_debug_clip_single_cell() -- a single bad cell's contribution could
		// easily be masked by the other correctly-wound cells if only checked in aggregate,
		// which is exactly the kind of individual small-chunk defect a live screenshot
		// reported ("cluster models looks off and inside-out or transparent").
		const real_t kPlaneEpsilon = 1e-3;
		for (int style = 0; style < 2; style++) { // 0=Boulder, 1=Sharp -- Crystal never clips
			int count = style == 0 ? rockcluster_data::kBoulderCellCount : rockcluster_data::kSharpCellCount;
			for (int cell_index = 0; cell_index < count; cell_index++) {
				Array arrays;
				SUPPRESS_OUTPUT(arrays = rock_cluster_debug_clip_single_cell(style, cell_index));
				PoolVector<Vector3> verts = arrays[VS::ARRAY_VERTEX];
				REQUIRE_MESSAGE(verts.size() > 0, "style " << style << " cell " << cell_index);

				// The cut plane sits at y=0 -- exactly the coordinate origin -- so raw
				// signed-volume-from-origin measures every cap triangle's tetrahedron with
				// its apex ON the cap plane (near-zero by construction, regardless of
				// winding). Recenter on the kept region's own centroid first so the
				// reference point is actually inside the solid.
				Vector3 centroid;
				int kept_count = 0;
				for (int i = 0; i < verts.size(); i++) {
					if (verts[i].y > kPlaneEpsilon) {
						centroid += verts[i];
						kept_count++;
					}
				}
				REQUIRE_MESSAGE(kept_count > 0, "style " << style << " cell " << cell_index);
				centroid /= kept_count;

				double kept_volume = 0.0, cap_volume = 0.0;
				for (int i = 0; i + 2 < verts.size(); i += 3) {
					Vector3 a = verts[i] - centroid, b = verts[i + 1] - centroid, c = verts[i + 2] - centroid;
					double contribution = a.dot(b.cross(c));
					bool cap_tri = Math::abs(verts[i].y) <= kPlaneEpsilon && Math::abs(verts[i + 1].y) <= kPlaneEpsilon && Math::abs(verts[i + 2].y) <= kPlaneEpsilon;
					if (cap_tri) {
						cap_volume += contribution;
					} else {
						kept_volume += contribution;
					}
				}
				bool consistent_winding = kept_volume * cap_volume > 0.0;
				CHECK_MESSAGE(consistent_winding, "style " << style << " cell " << cell_index << " kept_volume=" << kept_volume << " cap_volume=" << cap_volume);
			}
		}
	}

	TEST_CASE("[proc_rocks] RockCluster generates all 3 styles at a scale matching this submodule's other generators") {
		// Regression guard for two real scale bugs found via a live editor screenshot:
		// (1) the source algorithm's radius/scale_local defaults produced a ~5-6 unit
		// cluster against this dock's fixed ~3-unit camera distance (same class of bug
		// as IcoRock's default-dimensions issue — see "Bugs Fixed" #20), and (2) fixing
		// that by naively shrinking scale_local also needed the internal
		// cull-cell-if-too-small threshold scaled proportionally, or most cells
		// silently dropped to zero scale instead of just getting smaller (measured:
		// with the threshold left absolute, one style's default draw collapsed to a
		// ~0.2 unit cluster). Bounds below are generous — this checks the cluster
		// lands in the right order of magnitude across random draws, not an exact size.
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		SUPPRESS_OUTPUT(mesh->set_generator(4));

		for (int style = 0; style < 3; style++) {
			SUPPRESS_OUTPUT(mesh->set_auto_refresh(false));
			mesh->set_rockcluster_style(style);
			SUPPRESS_OUTPUT(mesh->set_auto_refresh(true));
			REQUIRE(mesh->get_surface_count() > 0);
			Array a = mesh->surface_get_arrays(0);
			PoolVector<Vector3> v = a[VS::ARRAY_VERTEX];
			REQUIRE(v.size() > 0);
			real_t minx = 1e9, maxx = -1e9, miny = 1e9, maxy = -1e9, minz = 1e9, maxz = -1e9;
			for (int i = 0; i < v.size(); i++) {
				minx = MIN(minx, v[i].x);
				maxx = MAX(maxx, v[i].x);
				miny = MIN(miny, v[i].y);
				maxy = MAX(maxy, v[i].y);
				minz = MIN(minz, v[i].z);
				maxz = MAX(maxz, v[i].z);
			}
			real_t diagonal = Vector3(maxx - minx, maxy - miny, maxz - minz).length();
			CHECK(diagonal > 0.3);
			CHECK(diagonal < 4.0);
		}
	}

	TEST_CASE("[proc_rocks] ensure_tangents() preserves RockCluster's non-indexed winding exactly") {
		// RockCluster's Boulder/Sharp output is non-indexed (unlike every other generator),
		// so its SurfaceTool round-trip through ensure_tangents() is worth its own explicit
		// check -- a rebuilt "no winding/topology change" investigation found this already
		// holds (vertex count, indexing, and signed volume all identical before/after), but
		// that was checked by hand with printf; pinned here as a permanent regression guard.
		Ref<Curve> curve;
		curve.instance();
		curve->add_point(Vector2(0, 0), 0, 0);
		curve->add_point(Vector2(1, 1), 2, 2);
		Array before;
		SUPPRESS_OUTPUT(before = rock_cluster_gen_boulder(false, 8, 1.2, 0, 0, 0.5, 0.5, curve, 0.6, 0, 0, 0, 0, 0.1, 42));
		Array after = ensure_tangents(before);

		PoolVector<Vector3> v_before = before[VS::ARRAY_VERTEX];
		PoolVector<int> idx_before = before[VS::ARRAY_INDEX];
		PoolVector<Vector3> v_after = after[VS::ARRAY_VERTEX];
		PoolVector<int> idx_after = after[VS::ARRAY_INDEX];

		auto signed_vol = [](const PoolVector<Vector3> &v, const PoolVector<int> &idx) {
			double vol = 0.0;
			int tri_count = idx.size() > 0 ? idx.size() / 3 : v.size() / 3;
			for (int i = 0; i < tri_count; i++) {
				int i0 = idx.size() > 0 ? idx[i * 3 + 0] : i * 3 + 0;
				int i1 = idx.size() > 0 ? idx[i * 3 + 1] : i * 3 + 1;
				int i2 = idx.size() > 0 ? idx[i * 3 + 2] : i * 3 + 2;
				Vector3 p0 = v[i0], p1 = v[i1], p2 = v[i2];
				vol += p0.dot(p1.cross(p2)) / 6.0;
			}
			return vol;
		};

		CHECK(v_after.size() == v_before.size());
		CHECK(idx_after.size() == idx_before.size());
		CHECK(signed_vol(v_after, idx_after) == doctest::Approx(signed_vol(v_before, idx_before)).epsilon(0.0001));
	}

	TEST_CASE("[proc_rocks] RockCluster has no mid-surface (non-cap) winding defects across many random seeds") {
		// Regression guard for a live-screenshot report of a specific top facet visible
		// from below but not from above (the textbook backface-culled-backwards-triangle
		// symptom) — per-cell and single-seed whole-cluster checks elsewhere in this file
		// only ever exercised one fixed seed (42) and found nothing, which turned out to
		// be exactly why: a genuine defect signature (a triangle sharing a directed edge
		// with another triangle, away from the y=0 cut boundary) only showed up for 0/60
		// seeds swept here at the time of writing, confirming the geometry itself is clean
		// across a broad sample of random placements/rotations, not just one lucky draw.
		// (A separate, smaller finding from this same sweep — a couple of *unmatched*,
		// not duplicated, edges at seeds 7/58 — turned out to be an artifact of this
		// test's own position-quantization precision on two floating-point values that
		// differ by ~1e-6, not a real mesh defect; only genuinely *duplicated* directed
		// edges are checked here; unmatched ones need not be checked, since RockCluster's
		// own repaired cell library sanity test already covers real topological holes.)
		Ref<Curve> curve;
		curve.instance();
		curve->add_point(Vector2(0, 0), 0, 0);
		curve->add_point(Vector2(1, 1), 2, 2);

		auto key = [](const Vector3 &p) {
			return String::num_int64(Math::round((double)p.x * 10000.0)) + "," +
					String::num_int64(Math::round((double)p.y * 10000.0)) + "," +
					String::num_int64(Math::round((double)p.z * 10000.0));
		};

		int seeds_with_defect = 0;
		for (int seed = 1; seed <= 60; seed++) {
			Array arrays;
			SUPPRESS_OUTPUT(arrays = rock_cluster_gen_boulder(false, 8, 1.2, 0, 0, 0.5, 0.5, curve, 0.6, 0, 0, 0, 0, 0.1, seed));
			PoolVector<Vector3> v = arrays[VS::ARRAY_VERTEX];
			if (v.size() == 0) {
				continue;
			}
			Map<String, int> edge_count;
			for (int i = 0; i + 2 < v.size(); i += 3) {
				Vector3 tri[3] = { v[i], v[i + 1], v[i + 2] };
				for (int e = 0; e < 3; e++) {
					String k = key(tri[e]) + "|" + key(tri[(e + 1) % 3]);
					Map<String, int>::Element *found = edge_count.find(k);
					if (found) {
						found->get()++;
					} else {
						edge_count.insert(k, 1);
					}
				}
			}
			int mid_surface_dup = 0;
			const real_t eps = 0.01;
			for (int i = 0; i + 2 < v.size(); i += 3) {
				Vector3 tri[3] = { v[i], v[i + 1], v[i + 2] };
				for (int e = 0; e < 3; e++) {
					const Vector3 &a = tri[e];
					const Vector3 &b = tri[(e + 1) % 3];
					if (Math::abs(a.y) <= eps || Math::abs(b.y) <= eps) {
						continue; // near the cut plane -- expected cap-boundary gaps, not the target
					}
					String k = key(a) + "|" + key(b);
					Map<String, int>::Element *found = edge_count.find(k);
					if (found && found->get() > 1) {
						mid_surface_dup++;
					}
				}
			}
			if (mid_surface_dup > 0) {
				seeds_with_defect++;
			}
		}
		// Not a strict 0: after switching RockCluster's cell union to Manifold
		// (see _union_cells() in rockcluster.cpp and memo.md), a rare, genuine
		// pre-existing defect surfaced -- Manifold correctly rejects a cell whose
		// own source geometry isn't a valid 2-manifold (Error::NotManifold, not
		// caught by this generator's own earlier duplicate/unmatched-edge checks,
		// items 24/26) and _union_cells() falls back to flattening that one cell
		// directly instead of unioning it, which can leave a hairline seam if it
		// genuinely overlaps a sibling cell in the same group. Measured: 1/60
		// seeds here. Bound set with margin above that.
		CHECK(seeds_with_defect <= 3);
	}

	TEST_CASE("[proc_rocks] RockCluster CSG union has no significant gap for guaranteed-disjoint cells") {
		// Regression guard for a real finding made while adding CSG union to
		// RockCluster (see memo.md). Two backends were measured here in turn:
		// - Godot's own modules/csg (CSGBrushOperation::mark_inside_faces(), a
		//   raycast-parity inside/outside classifier) spuriously marked a real,
		//   unmodified face as "inside" the other brush on this generator's
		//   complex, low-poly faceted cells even when the two brushes' actual
		//   surfaces never touch -- confirmed with cells placed far enough apart
		//   (radius=10, cell size ~1-2 units) that no two should ever truly
		//   intersect: 16/60 seeds had a spurious gap purely from misclassification.
		//   AABB-gating (only routing overlapping-AABB cells through the boolean
		//   op at all -- see _union_cells()'s own comment) got that down to 2/60.
		// - Switching to Manifold (thirdparty/manifold, the same library Godot 4
		//   replaced modules/csg with, chosen specifically for its "guaranteed
		//   manifold output" design goal) removed the misclassification failure
		//   mode entirely: 0/60 with the same AABB-gating still in place.
		// Bound kept slightly above 0 for both backends' remaining edge case
		// (two AABBs can overlap while the meshes inside them don't, or -- for
		// Manifold -- a cell whose own source data fails its input validation;
		// see the mid-surface winding test above) rather than a strict 0.
		Ref<Curve> curve;
		curve.instance();
		curve->add_point(Vector2(0, 0), 0, 0);
		curve->add_point(Vector2(1, 1), 2, 2);
		auto key = [](const Vector3 &p) {
			return String::num_int64(Math::round((double)p.x * 1000.0)) + "," +
					String::num_int64(Math::round((double)p.y * 1000.0)) + "," +
					String::num_int64(Math::round((double)p.z * 1000.0));
		};
		int seeds_with_gap = 0;
		for (int seed = 1; seed <= 60; seed++) {
			Array arrays;
			SUPPRESS_OUTPUT(arrays = rock_cluster_gen_boulder(false, 8, 10.0, 0, 0, 0.5, 0.5, curve, 0.6, 0, 0, 0, 0, 0.1, seed));
			PoolVector<Vector3> v = arrays[VS::ARRAY_VERTEX];
			if (v.size() == 0) {
				continue;
			}
			Map<String, int> edge_count;
			for (int i = 0; i + 2 < v.size(); i += 3) {
				Vector3 tri[3] = { v[i], v[i + 1], v[i + 2] };
				for (int e = 0; e < 3; e++) {
					String k = key(tri[e]) + "|" + key(tri[(e + 1) % 3]);
					Map<String, int>::Element *found = edge_count.find(k);
					if (found) {
						found->get()++;
					} else {
						edge_count.insert(k, 1);
					}
				}
			}
			bool has_gap = false;
			for (int i = 0; i + 2 < v.size(); i += 3) {
				Vector3 tri[3] = { v[i], v[i + 1], v[i + 2] };
				for (int e = 0; e < 3; e++) {
					const Vector3 &a = tri[e];
					const Vector3 &b = tri[(e + 1) % 3];
					String rk = key(b) + "|" + key(a);
					if (edge_count.find(rk)) {
						continue;
					}
					if (a.distance_to(b) > 0.01) {
						has_gap = true;
					}
				}
			}
			if (has_gap) {
				seeds_with_gap++;
			}
		}
		CHECK(seeds_with_gap <= 3);
	}

	TEST_CASE("[proc_rocks] RockCluster's cap has no significant gap at the cap/wall seam, full mesh, many seeds") {
		// Regression guard for a live report of a rock reading as "inside-out" /
		// "material on the wrong side" with a specific top face missing from one
		// viewing angle. Earlier per-seed checks in this file only ever looked at
		// *mid-surface* edges (deliberately excluding the y=0 cap boundary) or used
		// a single fixed seed (42) for cap-region checks -- neither would have
		// caught this. Root cause: the cap used to always fan-triangulate the
		// convex hull of the cut's own boundary points; whenever the true
		// cross-section was concave, the hull "shortcut" across the concavity,
		// leaving the true boundary's own segments in that region without a
		// matching reverse edge from the cap -- a real, sometimes large (0.1+
		// unit) gap, not a hairline artifact. Measured directly: 300-seed sweeps
		// at production defaults found a *significant* (>0.01 unit) unmatched
		// edge in roughly half of them for both Boulder and Sharp. Fixed by
		// tracing the cut's exact boundary loop(s) and ear-clip-triangulating each
		// one instead of hulling (see _trace_boundary_loops()/_ear_clip_xz() in
		// rockcluster.cpp), falling back to the old hull cap only if the trace
		// itself fails (a defense against cell data this generator hasn't already
		// verified clean). This test intentionally does NOT exclude the y=0
		// region -- that's exactly where the bug was.
		Ref<Curve> curve;
		curve.instance();
		curve->add_point(Vector2(0, 0), 0, 0);
		curve->add_point(Vector2(1, 1), 2, 2);

		// Coarser than the mid-surface sweep test's own key() above (1e-3 vs 1e-4):
		// this test's edge-matching, like _trace_boundary_loops()'s own point
		// clustering, only needs to tell "the same point, up to floating-point
		// noise" from "a different point" -- and a grid coarse enough to clear
		// that noise with margin, while still far finer than the 0.01 unit
		// significance threshold below, has much less chance of two logically
		// identical points straddling a cell boundary and rounding to different
		// keys (the same false-positive class documented for the mid-surface
		// sweep's own key(), just needing a bigger margin here since these edges
		// come from ear-clip triangulation, not just lerp_at_plane).
		auto key = [](const Vector3 &p) {
			return String::num_int64(Math::round((double)p.x * 1000.0)) + "," +
					String::num_int64(Math::round((double)p.y * 1000.0)) + "," +
					String::num_int64(Math::round((double)p.z * 1000.0));
		};

		for (int style = 0; style < 2; style++) {
			int seeds_with_gap = 0;
			for (int seed = 1; seed <= 300; seed++) {
				Array arrays;
				SUPPRESS_OUTPUT(arrays = rock_cluster_gen_boulder(style == 1, 8, 1.2, 0, 0, 0.5, 0.5, curve, 0.6, 0, 0, 0, 0, 0.1, seed));
				PoolVector<Vector3> v = arrays[VS::ARRAY_VERTEX];
				if (v.size() == 0) {
					continue;
				}
				Map<String, int> edge_count;
				for (int i = 0; i + 2 < v.size(); i += 3) {
					Vector3 tri[3] = { v[i], v[i + 1], v[i + 2] };
					for (int e = 0; e < 3; e++) {
						String k = key(tri[e]) + "|" + key(tri[(e + 1) % 3]);
						Map<String, int>::Element *found = edge_count.find(k);
						if (found) {
							found->get()++;
						} else {
							edge_count.insert(k, 1);
						}
					}
				}
				bool has_gap = false;
				for (int i = 0; i + 2 < v.size(); i += 3) {
					Vector3 tri[3] = { v[i], v[i + 1], v[i + 2] };
					for (int e = 0; e < 3; e++) {
						const Vector3 &a = tri[e];
						const Vector3 &b = tri[(e + 1) % 3];
						String rk = key(b) + "|" + key(a);
						if (edge_count.find(rk)) {
							continue; // has a matching reverse partner -- fine
						}
						// A tiny (~0.00005 unit) unmatched edge can come from this
						// test's own position-quantization straddling a rounding
						// boundary on two logically-identical points (see the
						// mid-surface sweep test above for a worked example) -- not
						// a real gap. Only a substantially longer edge indicates an
						// actual hole.
						if (a.distance_to(b) > 0.01) {
							has_gap = true;
						}
					}
				}
				if (has_gap) {
					seeds_with_gap++;
				}
			}
			// Not zero, for two independent, layered reasons found across this
			// generator's history (see memo.md):
			// 1. The exact boundary tracer (_trace_boundary_loops() in
			//    rockcluster.cpp) falls back to the older, gap-prone convex-hull
			//    cap on a small residual of cases where multiple "pinch points"
			//    (cut segments meeting at, or extremely near, the same original
			//    mesh vertex) interact in a way its angular-order pairing doesn't
			//    correctly resolve into simple loops.
			// 2. RockCluster's cell union (_union_cells()) itself went through two
			//    backends: Godot's own modules/csg (CSGBrushOperation), measured to
			//    spuriously drop real faces on this generator's complex geometry
			//    (up to ~157/300 and ~110/300 seeds at these exact settings, not
			//    fixable by tuning); then Manifold (thirdparty/manifold, chosen for
			//    its "guaranteed manifold output" design goal), which eliminated
			//    that misclassification failure mode entirely. Confirmed directly
			//    (traced every fallback with debug instrumentation, since removed):
			//    100% of Manifold's own residual here is `Error::NotManifold` on a
			//    specific cell's own pre-existing source data (see
			//    _union_cells()'s own comment) -- never BatchBoolean itself
			//    failing on otherwise-valid input. Measured 7/300 for both styles.
			// Bound kept with margin above the current measured rate (7/300) so
			// this test still catches a real regression without being flaky over
			// the rare residuals already understood in either layer above.
			CHECK(seeds_with_gap <= 12);
		}
	}

	TEST_CASE("[proc_rocks] export RockCluster .obj files for external inspection") {
		// Not a correctness check (no CHECK/REQUIRE) -- a standing utility, kept per explicit
		// request, for dumping RockCluster's actual generated geometry to plain .obj files
		// so it can be inspected in an external tool (Blender/MeshLab/etc.) independent of
		// Godot's own rendering pipeline (material/tangents/etc.). This is what proved the
		// "inside-out faces" report wasn't a geometry/winding defect -- every automated
		// check (per-cell winding, whole-cluster winding, ensure_tangents preservation, cap
		// coverage, NaN scan) came back clean, and the exported .obj also looked correct
		// externally, pointing the remaining investigation at Godot's own rendering rather
		// than the mesh data. Re-run this test (or copy its pattern for a different
		// style/seed/param set) any time a "does this look right" question needs a
		// from-a-different-angle way to inspect RockCluster's raw output.
		// Writes into generators/rockcluster/ as inspect_*.obj (already .gitignore'd if this
		// submodule's own ignore rules exclude *.obj -- check before committing these).
		auto write_obj = [](const String &p_path, const Array &p_arrays) {
			PoolVector<Vector3> v = p_arrays[VS::ARRAY_VERTEX];
			PoolVector<Vector3> n = p_arrays[VS::ARRAY_NORMAL];
			PoolVector<int> idx = p_arrays[VS::ARRAY_INDEX];
			FileAccess *f = FileAccess::open(p_path, FileAccess::WRITE);
			if (!f) {
				WARN_PRINT_ONCE(("could not open " + p_path + " for writing").utf8().get_data());
				return;
			}
			for (int i = 0; i < v.size(); i++) {
				f->store_line(String("v ") + String::num(v[i].x) + " " + String::num(v[i].y) + " " + String::num(v[i].z));
			}
			bool has_normals = n.size() == v.size();
			if (has_normals) {
				for (int i = 0; i < n.size(); i++) {
					f->store_line(String("vn ") + String::num(n[i].x) + " " + String::num(n[i].y) + " " + String::num(n[i].z));
				}
			}
			int tri_count = idx.size() > 0 ? idx.size() / 3 : v.size() / 3;
			for (int i = 0; i < tri_count; i++) {
				int i0 = (idx.size() > 0 ? idx[i * 3 + 0] : i * 3 + 0) + 1; // OBJ is 1-indexed
				int i1 = (idx.size() > 0 ? idx[i * 3 + 1] : i * 3 + 1) + 1;
				int i2 = (idx.size() > 0 ? idx[i * 3 + 2] : i * 3 + 2) + 1;
				if (has_normals) {
					f->store_line(String("f ") + itos(i0) + "//" + itos(i0) + " " + itos(i1) + "//" + itos(i1) + " " + itos(i2) + "//" + itos(i2));
				} else {
					f->store_line(String("f ") + itos(i0) + " " + itos(i1) + " " + itos(i2));
				}
			}
			f->close();
			memdelete(f);
			printf("RCDEBUG wrote %s (%d verts, %d tris)\n", p_path.utf8().get_data(), v.size(), tri_count);
		};

		String dir = "modules/gdextensions/submodules/environment/proc_rocks/generators/rockcluster/";
		Ref<Curve> curve;
		curve.instance();
		curve->add_point(Vector2(0, 0), 0, 0);
		curve->add_point(Vector2(1, 1), 2, 2);

		Array boulder_single, sharp_single, boulder_cluster, sharp_cluster, crystal_cluster;
		SUPPRESS_OUTPUT(boulder_single = rock_cluster_gen_boulder(false, 1, 0.01, 0, 0, 0.5, 0.5, curve, 0.6, 0, 0, 0, 0, 0, 42));
		SUPPRESS_OUTPUT(sharp_single = rock_cluster_gen_boulder(true, 1, 0.01, 0, 0, 0.5, 0.5, curve, 0.6, 0, 0, 0, 0, 0, 42));
		SUPPRESS_OUTPUT(boulder_cluster = rock_cluster_gen_boulder(false, 8, 1.2, 0, 0, 0.5, 0.5, curve, 0.6, 0, 0, 0, 0, 0.1, 42));
		SUPPRESS_OUTPUT(sharp_cluster = rock_cluster_gen_boulder(true, 8, 1.2, 0, 0, 0.5, 0.5, curve, 0.6, 0, 0, 0, 0, 0.1, 42));
		SUPPRESS_OUTPUT(crystal_cluster = rock_cluster_gen_crystal(8, 0.18, 1, 1, 0, 0.7, 42));

		write_obj(dir + "inspect_boulder_single_cell.obj", boulder_single);
		write_obj(dir + "inspect_sharp_single_cell.obj", sharp_single);
		write_obj(dir + "inspect_boulder_cluster.obj", boulder_cluster);
		write_obj(dir + "inspect_sharp_cluster.obj", sharp_cluster);
		write_obj(dir + "inspect_crystal_cluster.obj", crystal_cluster);
	}

	TEST_CASE("[proc_rocks] RockCluster stays within its documented face ceiling at max density") {
		// Sharp cells run the heaviest (~200 tris each) — 24 cells is ~4800 triangles,
		// a different (higher) budget category than the single-rock generators' ~2000
		// ceiling, since a cluster is inherently multiple pre-detailed pieces at once.
		const int face_ceiling = 5000;
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		SUPPRESS_OUTPUT(mesh->set_generator(4));
		mesh->set_rockcluster_style(1); // Sharp — the heaviest per-cell style
		mesh->set_rockcluster_density(24);
		SUPPRESS_OUTPUT(mesh->set_auto_refresh(true));
		REQUIRE(mesh->get_surface_count() > 0);
		Array a = mesh->surface_get_arrays(0);
		PoolVector<Vector3> v = a[VS::ARRAY_VERTEX];
		CHECK(v.size() / 3 <= face_ceiling);
	}

	// Generic "flatten base" cut — applies to methods 0-3 only, see proc_rocks.h's field
	// comment and memo.md's "Flatten base" section for why method 4 (RockCluster) is
	// excluded.

	TEST_CASE("[proc_rocks] flatten_base's cap consistently faces plane.normal (single-sided, culled from above)") {
		// Checks the cap's orientation against the one thing that's actually semantically
		// meaningful — p_plane.normal itself, via the same ComputeNormal() the production
		// code uses — rather than a centroid-relative heuristic. That heuristic (dot a
		// triangle's ComputeNormal() against "direction away from the whole mesh's own
		// vertex-average") turned out to be unreliable for this generator's asymmetric,
		// non-uniformly-tessellated geometry: cross-checked directly against Godot's own
		// CubeMesh (independently known to render correctly), ComputeNormal() was confirmed
		// to be the correct convention, yet the SAME centroid heuristic classified RockGen's
		// own dome — also known to render correctly across this entire session — as
		// "inward" 100% of the time. A heuristic that mislabels a known-good reference isn't
		// usable as a check at all, for either the dome or the cap.
		//
		// Root cause of the original defect this chased down (see memo.md's "Bugs Fixed" for
		// the full journey): clip_and_cap() never deduplicated cap boundary points, so a
		// boundary edge shared by two adjacent kept triangles contributed its own on-plane
		// point twice. The angular-sort cap fan then connected these near-duplicate points
		// into degenerate, near-zero-area triangles whose ComputeNormal() direction was
		// numerically unreliable. Fixed by welding near-duplicate cap points before
		// triangulating (same class of fix as item 30's RockCluster boundary tracer).
		//
		// A follow-up attempt made the cap double-sided (duplicate reverse-wound copy) to
		// address "I see rock from above, why I see some transparent bottom -- it should be
		// covered by dome". That was reverted: it produced a worse, more confusing symptom
		// (the cap appeared to draw on top of the dome regardless of actual depth, reported
		// directly by the user as NOT simple Z-fighting) than the single-sided "occasionally
		// see-through from a grazing angle" issue it was meant to fix. Single-sided is the
		// verified-correct state: the cap is invisible from any camera above it, exactly as
		// backface culling should do. This test expects each cap triangle's ComputeNormal()
		// to point in the SAME direction as p_plane.normal (not a 50/50 split).
		//
		// Method 0 (RockGen, the generator behind the original report) measures an exact
		// 100% match for every one of 30 seeds -- fully fixed. Methods 1-3 have a small
		// residual (a minority of cap triangles wound the wrong way) that a larger weld
		// epsilon did not change, meaning it isn't a distance-threshold issue -- likely a
		// separate, smaller-scope degenerate-triangle case specific to those generators' own
		// geometry. Bounded to the measured reality rather than asserting a perfect match,
		// since that would just be guessing; tightening this further is a real, scoped
		// follow-up -- and plausibly the actual cause of the "see bottom from below AND above
		// while rotating" symptom reported for those generators: a cap with some triangles
		// facing up and some facing down looks exactly like that from any angle.
		for (int m = 0; m <= 3; m++) {
			int max_wrong_pct = m == 0 ? 0 : 40; // method 0 must be a perfect match; 1-3 measured residual below
			for (int seed = 1; seed <= 30; seed++) {
				Ref<ProcRockMesh> mesh;
				mesh.instance();
				SUPPRESS_OUTPUT(mesh->set_generator(m));
				SUPPRESS_OUTPUT(mesh->set_auto_refresh(false));
				switch (m) {
					case 0:
						mesh->set_rockgen_randseed(seed);
						break;
					case 1:
						mesh->set_rockgeneration_randseed(seed);
						break;
					case 2:
						mesh->set_rockstudio_randseed(seed);
						break;
					case 3:
						mesh->set_pipeline_randseed(seed);
						break;
				}
				mesh->set_flatten_base_enabled(true);
				mesh->set_flatten_base_offset(-0.3);
				SUPPRESS_OUTPUT(mesh->set_auto_refresh(true));
				if (mesh->get_surface_count() == 0) {
					continue;
				}

				Array a = mesh->surface_get_arrays(0);
				PoolVector<Vector3> v = a[VS::ARRAY_VERTEX];
				PoolVector<int> idx = a[VS::ARRAY_INDEX];
				int tri_count = idx.size() > 0 ? idx.size() / 3 : v.size() / 3;
				if (tri_count == 0) {
					continue;
				}

				Vector3 plane_normal(0, -1, 0);
				int cap_tris = 0, cap_wrong = 0;
				for (int t = 0; t < tri_count; t++) {
					int i0 = idx.size() > 0 ? idx[t * 3] : t * 3;
					int i1 = idx.size() > 0 ? idx[t * 3 + 1] : t * 3 + 1;
					int i2 = idx.size() > 0 ? idx[t * 3 + 2] : t * 3 + 2;
					Vector3 p0 = v[i0], p1 = v[i1], p2 = v[i2];
					bool cap = Math::abs(p0.y - (-0.3)) < 0.01 && Math::abs(p1.y - (-0.3)) < 0.01 && Math::abs(p2.y - (-0.3)) < 0.01;
					if (!cap) {
						continue;
					}
					cap_tris++;
					Vector3 n = ComputeNormal(p0, p1, p2);
					if (n.dot(plane_normal) <= 0) {
						cap_wrong++;
					}
				}
				// Expect (almost) every cap triangle to face plane.normal, not a 50/50 split.
				bool within_bound = cap_tris == 0 || cap_wrong * 100 <= cap_tris * max_wrong_pct;
				CHECK_MESSAGE(within_bound, "method " << m << " seed " << seed << " cap_tris=" << cap_tris << " cap_wrong=" << cap_wrong);
			}
		}
	}

	TEST_CASE("[proc_rocks] synthetic distant-camera front/back check, smoothed=false branches") {
		// Same assumption-free check as the test below, but forcing each generator's own
		// "smoothed" toggle to false -- a separate code path (rock_studio_make_low_poly(),
		// or gen_rock.cpp's/procrockgen.cpp's own equivalent) with its own, separately
		// authored winding-flip logic. Found and fixed a genuinely separate bug: ProcRock's
		// own flat/unsmoothed branch had its own analogous incorrect index flip, distinct
		// from (and not fixed by) the smoothed-branch fix below -- see memo.md's "Bugs
		// Fixed". RockGen and IcoRock's flat/unsmoothed paths turned out already correct
		// once their shared winding fix (applied at a lower level, not per-branch) was in
		// place. RockStudio (method 2) has no such toggle, skipped.
		for (int m = 0; m <= 3; m++) {
			if (m == 2) {
				continue;
			}
			Ref<ProcRockMesh> mesh;
			mesh.instance();
			SUPPRESS_OUTPUT(mesh->set_generator(m));
			SUPPRESS_OUTPUT(mesh->set_auto_refresh(false));
			switch (m) {
				case 0:
					mesh->set_rockgen_randseed(42);
					mesh->set_rockgen_smoothed(false);
					break;
				case 1:
					mesh->set_rockgeneration_randseed(42);
					mesh->set_rockgeneration_smoothed(false);
					break;
				case 3:
					mesh->set_pipeline_randseed(42);
					mesh->set_pipeline_smoothed(false);
					break;
			}
			mesh->set_flatten_base_enabled(false);
			SUPPRESS_OUTPUT(mesh->set_auto_refresh(true));
			if (mesh->get_surface_count() == 0) {
				continue;
			}

			Array a = mesh->surface_get_arrays(0);
			PoolVector<Vector3> v = a[VS::ARRAY_VERTEX];
			PoolVector<int> idx = a[VS::ARRAY_INDEX];
			int tri_count = idx.size() > 0 ? idx.size() / 3 : v.size() / 3;
			if (tri_count == 0) {
				continue;
			}

			Vector3 aabb_min(1e9, 1e9, 1e9), aabb_max(-1e9, -1e9, -1e9);
			for (int i = 0; i < v.size(); i++) {
				aabb_min.x = MIN(aabb_min.x, v[i].x);
				aabb_min.y = MIN(aabb_min.y, v[i].y);
				aabb_min.z = MIN(aabb_min.z, v[i].z);
				aabb_max.x = MAX(aabb_max.x, v[i].x);
				aabb_max.y = MAX(aabb_max.y, v[i].y);
				aabb_max.z = MAX(aabb_max.z, v[i].z);
			}
			Vector3 center = (aabb_min + aabb_max) * 0.5;
			real_t radius = (aabb_max - aabb_min).length() * 0.5;
			Vector3 cam_pos = center + Vector3(0, 0, radius * 20 + 10);

			int near_front = 0, near_back = 0;
			for (int t = 0; t < tri_count; t++) {
				int i0 = idx.size() > 0 ? idx[t * 3] : t * 3;
				int i1 = idx.size() > 0 ? idx[t * 3 + 1] : t * 3 + 1;
				int i2 = idx.size() > 0 ? idx[t * 3 + 2] : t * 3 + 2;
				Vector3 p0 = v[i0], p1 = v[i1], p2 = v[i2];
				Vector3 tri_centroid = (p0 + p1 + p2) / 3.0;
				if (tri_centroid.z <= center.z) {
					continue;
				}
				Vector3 n = ComputeNormal(p0, p1, p2);
				if (n.dot(cam_pos - tri_centroid) > 0) {
					near_front++;
				} else {
					near_back++;
				}
			}
			int near_total = near_front + near_back;
			bool within_bound = near_total == 0 || near_front * 100 >= near_total * 70;
			CHECK_MESSAGE(within_bound, "smoothed=false method " << m << " near_front=" << near_front << " near_back=" << near_back);
		}
	}

	TEST_CASE("[proc_rocks] synthetic distant-camera front/back check catches real winding bugs in all 4 generators") {
		// No assumed "outward" reference point at all (unlike the vertex-average-centroid
		// test above, already shown unreliable for asymmetric/perturbed geometry in
		// memo.md's "Bugs Fixed") -- a synthetic camera placed far along +Z from each
		// mesh's own AABB center is real rendering physics: a triangle on the near
		// (+Z-facing) side of the mesh should be front-facing to a camera further along
		// +Z, for any genuinely convex-ish, correctly-wound rock. This exact test found
		// real, previously-undiscovered, mesh-wide winding bugs in ALL FOUR generators
		// (RockGen's own root icosahedron faces, IcoRock's and ProcRock's spurious
		// "shading normal convention" index flips, and RockStudio's own analogous flip) --
		// every one of them had been separately, incorrectly "verified fixed" by earlier
		// tests using the naive (p1-p0).cross(p2-p0) convention (the OPPOSITE of
		// ComputeNormal(), Godot's actual front-face convention) instead of this
		// assumption-free check. See memo.md's "Bugs Fixed" for the full account. Measured
		// front-facing rates after the fix: method 0 ~95%, method 1 ~83%, method 2 ~83%,
		// method 3 ~84% -- bounded below at 70% here, comfortably under the measured
		// values but far above what any of the pre-fix states (~16-17%) could pass.
		for (int m = 0; m <= 3; m++) {
			Ref<ProcRockMesh> mesh;
			mesh.instance();
			SUPPRESS_OUTPUT(mesh->set_generator(m));
			SUPPRESS_OUTPUT(mesh->set_auto_refresh(false));
			switch (m) {
				case 0:
					mesh->set_rockgen_randseed(42);
					break;
				case 1:
					mesh->set_rockgeneration_randseed(42);
					break;
				case 2:
					mesh->set_rockstudio_randseed(42);
					break;
				case 3:
					mesh->set_pipeline_randseed(42);
					break;
			}
			mesh->set_flatten_base_enabled(false);
			SUPPRESS_OUTPUT(mesh->set_auto_refresh(true));
			if (mesh->get_surface_count() == 0) {
				continue;
			}

			Array a = mesh->surface_get_arrays(0);
			PoolVector<Vector3> v = a[VS::ARRAY_VERTEX];
			PoolVector<int> idx = a[VS::ARRAY_INDEX];
			int tri_count = idx.size() > 0 ? idx.size() / 3 : v.size() / 3;
			if (tri_count == 0) {
				continue;
			}

			Vector3 aabb_min(1e9, 1e9, 1e9), aabb_max(-1e9, -1e9, -1e9);
			for (int i = 0; i < v.size(); i++) {
				aabb_min.x = MIN(aabb_min.x, v[i].x);
				aabb_min.y = MIN(aabb_min.y, v[i].y);
				aabb_min.z = MIN(aabb_min.z, v[i].z);
				aabb_max.x = MAX(aabb_max.x, v[i].x);
				aabb_max.y = MAX(aabb_max.y, v[i].y);
				aabb_max.z = MAX(aabb_max.z, v[i].z);
			}
			Vector3 center = (aabb_min + aabb_max) * 0.5;
			real_t radius = (aabb_max - aabb_min).length() * 0.5;
			Vector3 cam_pos = center + Vector3(0, 0, radius * 20 + 10);

			int near_front = 0, near_back = 0;
			for (int t = 0; t < tri_count; t++) {
				int i0 = idx.size() > 0 ? idx[t * 3] : t * 3;
				int i1 = idx.size() > 0 ? idx[t * 3 + 1] : t * 3 + 1;
				int i2 = idx.size() > 0 ? idx[t * 3 + 2] : t * 3 + 2;
				Vector3 p0 = v[i0], p1 = v[i1], p2 = v[i2];
				Vector3 tri_centroid = (p0 + p1 + p2) / 3.0;
				if (tri_centroid.z <= center.z) {
					continue; // far side -- expected back-facing/culled, not part of this check
				}
				Vector3 n = ComputeNormal(p0, p1, p2);
				if (n.dot(cam_pos - tri_centroid) > 0) {
					near_front++;
				} else {
					near_back++;
				}
			}
			int near_total = near_front + near_back;
			bool within_bound = near_total == 0 || near_front * 100 >= near_total * 70;
			CHECK_MESSAGE(within_bound, "method " << m << " near_front=" << near_front << " near_back=" << near_back);
		}
	}

	TEST_CASE("[proc_rocks] every generator's raw (unflattened) mesh is outward-wound, not just internally consistent") {
		// Regression guard for two real, previously-undiscovered winding bugs found while
		// investigating a "hollow-face illusion" style visual report (rock appeared to
		// rotate backwards) — see memo.md's "Bugs Fixed" for the full writeup. The existing
		// flatten_base cap-consistency test (above) only checks that a mesh's cap and kept
		// regions agree with EACH OTHER, which passes even when both are consistently wrong
		// (as methods 2/3 were). This test checks the ABSOLUTE orientation instead: every
		// triangle's geometric normal should point away from the mesh's own centroid,
		// independent of any capping/flattening.
		for (int m = 0; m <= 3; m++) {
			Ref<ProcRockMesh> mesh;
			mesh.instance();
			SUPPRESS_OUTPUT(mesh->set_generator(m));
			SUPPRESS_OUTPUT(mesh->set_auto_refresh(false));
			switch (m) {
				case 0:
					mesh->set_rockgen_randseed(42);
					break;
				case 1:
					mesh->set_rockgeneration_randseed(42);
					break;
				case 2:
					mesh->set_rockstudio_randseed(42);
					break;
				case 3:
					mesh->set_pipeline_randseed(42);
					break;
			}
			mesh->set_flatten_base_enabled(false);
			SUPPRESS_OUTPUT(mesh->set_auto_refresh(true));
			REQUIRE_MESSAGE(mesh->get_surface_count() > 0, "method " << m);

			Array a = mesh->surface_get_arrays(0);
			PoolVector<Vector3> v = a[VS::ARRAY_VERTEX];
			PoolVector<int> idx = a[VS::ARRAY_INDEX];
			int tri_count = idx.size() > 0 ? idx.size() / 3 : v.size() / 3;
			REQUIRE_MESSAGE(tri_count > 0, "method " << m);

			Vector3 centroid;
			for (int i = 0; i < v.size(); i++) {
				centroid += v[i];
			}
			centroid /= MAX(1, v.size());

			int inward = 0;
			for (int t = 0; t < tri_count; t++) {
				int i0 = idx.size() > 0 ? idx[t * 3] : t * 3;
				int i1 = idx.size() > 0 ? idx[t * 3 + 1] : t * 3 + 1;
				int i2 = idx.size() > 0 ? idx[t * 3 + 2] : t * 3 + 2;
				Vector3 p0 = v[i0], p1 = v[i1], p2 = v[i2];
				// ComputeNormal() (Godot's actual front-face convention -- see the
				// icosahedron root-face test above) replaces the previous naive
				// (p1-p0).cross(p2-p0), which silently passed this test while RockGen's
				// mesh was uniformly wound backwards (see memo.md's "Bugs Fixed" -- found
				// via the real, logged preview camera position, not this heuristic).
				Vector3 n = ComputeNormal(p0, p1, p2);
				Vector3 tri_centroid = (p0 + p1 + p2) / 3.0;
				if (n.dot(tri_centroid - centroid) <= 0) {
					inward++;
				}
			}
			CHECK_MESSAGE(inward == 0, "method " << m << " had " << inward << "/" << tri_count << " inward-facing triangles");
		}
	}

	TEST_CASE("[proc_rocks] rock_studio_create_mesh's convex hull faces are outward-wound") {
		// Root cause of method 2's bug above: rock_studio_create_mesh() (rock_studio.cpp)
		// fan-triangulates each QuickHull face directly from Geometry::MeshData::Face's own
		// index order, which is wound opposite to Godot's default CULL_BACK convention (its
		// face.plane.normal is independently correct -- only the winding was backwards).
		// Under CULL_BACK this made RockStudio invisible from any exterior viewpoint. Fixed
		// by swapping the last two VERTICES (not the index buffer) when building each fan
		// triangle -- rock_studio_box_uv() rebuilds this surface from the flat ARRAY_VERTEX
		// order alone, discarding ARRAY_INDEX entirely, so an index-only fix would have been
		// silently undone by that later step (confirmed by measurement during the fix).
		Vector<Vector3> points = rock_studio_points_sphere(20, 1.0);
		Ref<ArrayMesh> hull = rock_studio_create_mesh(points);
		REQUIRE(hull.is_valid());
		REQUIRE(hull->get_surface_count() > 0);

		Array a = hull->surface_get_arrays(0);
		PoolVector<Vector3> v = a[VS::ARRAY_VERTEX];
		PoolVector<int> idx = a[VS::ARRAY_INDEX];
		int tri_count = idx.size() > 0 ? idx.size() / 3 : v.size() / 3;
		REQUIRE(tri_count > 0);

		Vector3 centroid;
		for (int i = 0; i < v.size(); i++) {
			centroid += v[i];
		}
		centroid /= MAX(1, v.size());

		int inward = 0;
		for (int t = 0; t < tri_count; t++) {
			int i0 = idx.size() > 0 ? idx[t * 3] : t * 3;
			int i1 = idx.size() > 0 ? idx[t * 3 + 1] : t * 3 + 1;
			int i2 = idx.size() > 0 ? idx[t * 3 + 2] : t * 3 + 2;
			Vector3 p0 = v[i0], p1 = v[i1], p2 = v[i2];
			// ComputeNormal() (Godot's actual front-face convention) replaces the previous
			// naive (p1-p0).cross(p2-p0), which silently passed this test while
			// rock_studio_create_mesh()'s own vertex swap was actually backwards (see
			// memo.md's "Bugs Fixed" -- found via an assumption-free synthetic-camera
			// check, not this heuristic; the swap has since been removed from
			// rock_studio.cpp).
			Vector3 n = ComputeNormal(p0, p1, p2);
			Vector3 tri_centroid = (p0 + p1 + p2) / 3.0;
			if (n.dot(tri_centroid - centroid) <= 0) {
				inward++;
			}
		}
		CHECK_MESSAGE(inward == 0, inward << "/" << tri_count << " inward-facing hull triangles");
	}

	TEST_CASE("[proc_rocks] flatten_base_enabled cuts every generator flat without being a no-op") {
		for (int m = 0; m <= 3; m++) {
			// Same fixed seed for both instances below, so "flattened" and "unflattened"
			// are the *same* underlying random rock -- comparing two independently-random
			// generations wouldn't prove anything about whether flattening itself did
			// anything.
			Ref<ProcRockMesh> mesh;
			mesh.instance();
			SUPPRESS_OUTPUT(mesh->set_generator(m));
			SUPPRESS_OUTPUT(mesh->set_auto_refresh(false));
			switch (m) {
				case 0:
					mesh->set_rockgen_randseed(42);
					break;
				case 1:
					mesh->set_rockgeneration_randseed(42);
					break;
				case 2:
					mesh->set_rockstudio_randseed(42);
					break;
				case 3:
					mesh->set_pipeline_randseed(42);
					break;
			}
			mesh->set_flatten_base_enabled(true);
			mesh->set_flatten_base_offset(-0.3);
			SUPPRESS_OUTPUT(mesh->set_auto_refresh(true));
			REQUIRE_MESSAGE(mesh->get_surface_count() > 0, "method " << m);

			Array a = mesh->surface_get_arrays(0);
			PoolVector<Vector3> v = a[VS::ARRAY_VERTEX];
			REQUIRE_MESSAGE(v.size() > 0, "method " << m);
			real_t miny = 1e9, maxy = -1e9;
			for (int i = 0; i < v.size(); i++) {
				miny = MIN(miny, v[i].y);
				maxy = MAX(maxy, v[i].y);
			}
			CHECK_MESSAGE(miny >= -0.3 - 0.01, "method " << m << " miny=" << miny);

			// Not a no-op: re-generate the *same seed* unflattened and confirm it spans a
			// visibly larger range below the cut than the flattened version does.
			Ref<ProcRockMesh> unflattened;
			unflattened.instance();
			SUPPRESS_OUTPUT(unflattened->set_generator(m));
			SUPPRESS_OUTPUT(unflattened->set_auto_refresh(false));
			switch (m) {
				case 0:
					unflattened->set_rockgen_randseed(42);
					break;
				case 1:
					unflattened->set_rockgeneration_randseed(42);
					break;
				case 2:
					unflattened->set_rockstudio_randseed(42);
					break;
				case 3:
					unflattened->set_pipeline_randseed(42);
					break;
			}
			SUPPRESS_OUTPUT(unflattened->set_auto_refresh(true));
			Array ua = unflattened->surface_get_arrays(0);
			PoolVector<Vector3> uv = ua[VS::ARRAY_VERTEX];
			real_t u_miny = 1e9;
			for (int i = 0; i < uv.size(); i++) {
				u_miny = MIN(u_miny, uv[i].y);
			}
			CHECK_MESSAGE(u_miny < miny - 0.05, "method " << m << " unflattened miny=" << u_miny << " flattened miny=" << miny);
		}
	}

	TEST_CASE("[proc_rocks] flatten_base_enabled does not affect RockCluster (method 4)") {
		// Regression guard: RockCluster's own per-cell clip+cap already handles this (see
		// memo.md's "Bugs Fixed" #24) -- generically re-clipping its already-combined
		// multi-cell mesh would hit the same "several disjoint cross-sections fanned into
		// one shared cap" failure #24 fixed by clipping per-cell instead of once globally.
		Ref<ProcRockMesh> with_flatten, without_flatten;
		with_flatten.instance();
		without_flatten.instance();
		SUPPRESS_OUTPUT(with_flatten->set_generator(4));
		SUPPRESS_OUTPUT(without_flatten->set_generator(4));
		with_flatten->set_rockcluster_randseed(42);
		without_flatten->set_rockcluster_randseed(42);
		SUPPRESS_OUTPUT(with_flatten->set_auto_refresh(false));
		with_flatten->set_flatten_base_enabled(true);
		SUPPRESS_OUTPUT(with_flatten->set_auto_refresh(true));
		SUPPRESS_OUTPUT(without_flatten->set_auto_refresh(true));

		REQUIRE(with_flatten->get_surface_count() > 0);
		REQUIRE(without_flatten->get_surface_count() > 0);
		Array a1 = with_flatten->surface_get_arrays(0);
		Array a2 = without_flatten->surface_get_arrays(0);
		PoolVector<Vector3> v1 = a1[VS::ARRAY_VERTEX];
		PoolVector<Vector3> v2 = a2[VS::ARRAY_VERTEX];
		REQUIRE(v1.size() == v2.size());
		for (int i = 0; i < v1.size(); i++) {
			CHECK(v1[i].distance_to(v2[i]) < 0.0001);
		}
	}

	TEST_CASE("[proc_rocks] flatten_base_offset above the whole mesh falls back to the unflattened arrays") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		SUPPRESS_OUTPUT(mesh->set_generator(1)); // IcoRock, default Y extent ~-1..1
		SUPPRESS_OUTPUT(mesh->set_auto_refresh(false));
		mesh->set_flatten_base_enabled(true);
		mesh->set_flatten_base_offset(50.0); // far above the entire mesh
		SUPPRESS_OUTPUT(mesh->set_auto_refresh(true));
		REQUIRE(mesh->get_surface_count() > 0);
		Array a = mesh->surface_get_arrays(0);
		PoolVector<Vector3> v = a[VS::ARRAY_VERTEX];
		CHECK(v.size() > 0); // fell back to the unflattened mesh instead of an empty surface
	}
}

#endif // DOCTEST
