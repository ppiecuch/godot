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
#include "generators/rockgen/rockgen.h"
#include "generators/rockgeneration/gen_rock.h"
#include "generators/rockstudio/rock_studio.h"

#include "core/io/json.h"
#include "core/os/file_access.h"

// =========================================================================
// Generator selection
// =========================================================================

void ProcRockMesh::set_generator(int p_mode) {
	if (method != p_mode) {
		method = CLAMP(p_mode, 0, 3);
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
		rockgen.depth = p_depth;
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
		rockgeneration.steps = p_steps;
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
		rockgeneration.max_planes = p_planes;
		_dirty = true;
		if (auto_refresh) {
			_rebuild();
		}
	}
}

uint32_t ProcRockMesh::get_rockgeneration_max_planes() const {
	return rockgeneration.max_planes;
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
		rockstudio.num_vertices = MAX(4, p_num);
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
		pipeline.subdivisions = CLAMP(p_val, 0, 6);
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
}

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
			p_list->push_back(PropertyInfo(Variant::INT, "rockgen_depth", PROPERTY_HINT_RANGE, "0,8"));
			p_list->push_back(PropertyInfo(Variant::INT, "rockgen_randseed"));
			p_list->push_back(PropertyInfo(Variant::REAL, "rockgen_smoothness", PROPERTY_HINT_RANGE, "0,5,0.1"));
			p_list->push_back(PropertyInfo(Variant::BOOL, "rockgen_smoothed"));
		} break;
		case 1: {
			p_list->push_back(PropertyInfo(Variant::INT, "rockgeneration_steps", PROPERTY_HINT_RANGE, "1,20"));
			p_list->push_back(PropertyInfo(Variant::REAL, "rockgeneration_width", PROPERTY_HINT_RANGE, "1,200,0.5"));
			p_list->push_back(PropertyInfo(Variant::REAL, "rockgeneration_height", PROPERTY_HINT_RANGE, "1,200,0.5"));
			p_list->push_back(PropertyInfo(Variant::REAL, "rockgeneration_depth", PROPERTY_HINT_RANGE, "1,200,0.5"));
			p_list->push_back(PropertyInfo(Variant::INT, "rockgeneration_max_planes", PROPERTY_HINT_RANGE, "1,20"));
		} break;
		case 2: {
			p_list->push_back(PropertyInfo(Variant::INT, "rockstudio_rock_type", PROPERTY_HINT_ENUM, "Cubic,Boulder,Quartz"));
			p_list->push_back(PropertyInfo(Variant::INT, "rockstudio_num_vertices", PROPERTY_HINT_RANGE, "4,1000"));
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
			p_list->push_back(PropertyInfo(Variant::INT, "pipeline_subdivisions", PROPERTY_HINT_RANGE, "0,6"));
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
	return false;
}

// =========================================================================
// Mesh generation
// =========================================================================

void ProcRockMesh::_rebuild() {
	if (!_dirty) {
		return;
	}

	clear_surfaces();

	switch (method) {
		case 0: {
			// Method 0: rockgen — fractal icosahedron subdivision
			Array mesh_arrays = rock_gen(rockgen.depth, rockgen.randseed, rockgen.smoothness, rockgen.smoothed);
			if (mesh_arrays.size() == VS::ARRAY_MAX) {
				add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, mesh_arrays);
			}
		} break;

		case 1: {
			// Method 1: rockgeneration — icosphere with plane deformation
			GenRock gen(rockgeneration.dimensions.x,
					rockgeneration.dimensions.y,
					rockgeneration.dimensions.z,
					rockgeneration.steps);
			gen.SetRandAngleMin(rockgeneration.rand_angle_range.x);
			gen.SetRandAngleMax(rockgeneration.rand_angle_range.y);
			gen.SetRandOffsetPercent(rockgeneration.rand_offset_percent);
			gen.SetRandShift(rockgeneration.rand_shift);
			gen.SetMinPlaneVerts(rockgeneration.plane_verts_range.x);
			gen.SetMaxPlaneVerts(rockgeneration.plane_verts_range.y);
			gen.SetMaxPlanes(rockgeneration.max_planes);

			Ref<ArrayMesh> rock_mesh = gen.GenerateMesh();
			if (rock_mesh.is_valid() && rock_mesh->get_surface_count() > 0) {
				Array surface = rock_mesh->surface_get_arrays(0);
				add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, surface);
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
						add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, surface);
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
							  pipeline.json_cache, pipeline.noise_amplitude, pipeline.randseed,
							  pipeline.cutplane_enabled, pipeline.cutplane_offset, pipeline.smoothed)
					: rock_pipeline_gen(pipeline.subdivisions, pipeline.width, pipeline.height, pipeline.depth,
							  pipeline.noise_frequency, pipeline.noise_amplitude, pipeline.noise_octaves, pipeline.noise_persistence,
							  pipeline.randseed, pipeline.cutplane_enabled, pipeline.cutplane_offset, pipeline.smoothed);

			Vector<Vector3> verts = mesh_arrays.size() == VS::ARRAY_MAX ? (Vector<Vector3>)mesh_arrays[VS::ARRAY_VERTEX] : Vector<Vector3>();
			if (verts.size() > 0) {
				add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, mesh_arrays);

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
				}
			}
		} break;
	}

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

	ClassDB::bind_method(D_METHOD("set_auto_refresh", "refresh"), &ProcRockMesh::set_auto_refresh);
	ClassDB::bind_method(D_METHOD("get_auto_refresh"), &ProcRockMesh::get_auto_refresh);

	ClassDB::bind_method(D_METHOD("_rebuild"), &ProcRockMesh::_rebuild);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "generator", PROPERTY_HINT_ENUM, "RockGen,IcoRock,RockStudio,ProcRock"), "set_generator", "get_generator");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_refresh"), "set_auto_refresh", "get_auto_refresh");
	// Generator-specific properties are dynamic — see _get_property_list()
}

ProcRockMesh::ProcRockMesh() {
	method = 0;
	auto_refresh = false;

	rockgen.depth = 3;
	rockgen.randseed = 0;
	rockgen.smoothness = 1;
	rockgen.smoothed = false;

	rockgeneration.dimensions = Vector3(50, 50, 50);
	rockgeneration.steps = 10;
	rockgeneration.rand_angle_range = Vector2(0, 10);
	rockgeneration.rand_offset_percent = 5;
	rockgeneration.rand_shift = 2;
	rockgeneration.plane_verts_range = Vector2i(0, 200);
	rockgeneration.max_planes = 1;

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

	pipeline.subdivisions = 3;
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

	_dirty = true;
}

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"
#include "doctest/doctest_godot.h"

TEST_SUITE("[[proc_rocks]] ProcRockMesh") {
	TEST_CASE("[proc_rocks] default construction") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		CHECK(mesh->get_generator() == 0);
		CHECK(mesh->get_auto_refresh() == false);
		CHECK(mesh->get_rockgen_depth() == 3);
		CHECK(mesh->get_rockgen_randseed() == 0);
		CHECK(mesh->get_rockgen_smoothness() == doctest::Approx(1.0f));
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
		CHECK(mesh->get_generator() == 3);
	}

	TEST_CASE("[proc_rocks] rockgen properties") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_rockgen_depth(5);
		CHECK(mesh->get_rockgen_depth() == 5);
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
		mesh->set_rockgeneration_steps(15);
		CHECK(mesh->get_rockgeneration_steps() == 15);
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
		CHECK(mesh->get_pipeline_subdivisions() == 3);
		CHECK(mesh->get_pipeline_randseed() == 0);
		CHECK(mesh->get_pipeline_cutplane_enabled() == false);
		CHECK(mesh->get_pipeline_smoothed() == true);
		CHECK(mesh->get_pipeline_generate_textures() == false);
		CHECK(mesh->get_pipeline_texture_size() == 256);
	}

	TEST_CASE("[proc_rocks] pipeline properties round-trip") {
		Ref<ProcRockMesh> mesh;
		mesh.instance();
		mesh->set_pipeline_subdivisions(5);
		CHECK(mesh->get_pipeline_subdivisions() == 5);
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
	// Baked demo texture pack tests live in editor/proc_rocks_editor_plugin.cpp — that's
	// the only place the loader exists now (editor-only, moved out of the generator API).
}

#endif // DOCTEST
