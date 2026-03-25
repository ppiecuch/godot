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

#include "generators/rockgen/rockgen.h"
#include "generators/rockgeneration/gen_rock.h"
#include "generators/rockstudio/rock_studio.h"
#ifdef TOOLS_ENABLED
#include "generators/procrockgen/procrockgen.h"
#endif

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
			// procrockgen dynamic properties would go here
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
			// Method 3: procrockgen — pipeline-based (TOOLS_ENABLED only)
			// TODO: Integrate when procrocklib is available
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

	procrock.pipeline = nullptr;

	_dirty = true;
}

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_SUITE("[[proc_rocks]] ProcRockMesh") {
	TEST_CASE("[proc_rocks] default construction") {
		ProcRockMesh mesh;
		CHECK(mesh.get_generator() == 0);
		CHECK(mesh.get_auto_refresh() == false);
		CHECK(mesh.get_rockgen_depth() == 3);
		CHECK(mesh.get_rockgen_randseed() == 0);
		CHECK(mesh.get_rockgen_smoothness() == doctest::Approx(1.0f));
		CHECK(mesh.get_rockgen_smoothed() == false);
	}

	TEST_CASE("[proc_rocks] generator selection") {
		ProcRockMesh mesh;
		mesh.set_generator(1);
		CHECK(mesh.get_generator() == 1);
		mesh.set_generator(2);
		CHECK(mesh.get_generator() == 2);
		mesh.set_generator(0);
		CHECK(mesh.get_generator() == 0);
	}

	TEST_CASE("[proc_rocks] generator clamped to valid range") {
		ProcRockMesh mesh;
		mesh.set_generator(-1);
		CHECK(mesh.get_generator() == 0);
		mesh.set_generator(99);
		CHECK(mesh.get_generator() == 3);
	}

	TEST_CASE("[proc_rocks] rockgen properties") {
		ProcRockMesh mesh;
		mesh.set_rockgen_depth(5);
		CHECK(mesh.get_rockgen_depth() == 5);
		mesh.set_rockgen_randseed(42);
		CHECK(mesh.get_rockgen_randseed() == 42);
		mesh.set_rockgen_smoothness(2.5f);
		CHECK(mesh.get_rockgen_smoothness() == doctest::Approx(2.5f));
		mesh.set_rockgen_smoothed(true);
		CHECK(mesh.get_rockgen_smoothed() == true);
	}

	TEST_CASE("[proc_rocks] rockgeneration properties") {
		ProcRockMesh mesh;
		mesh.set_rockgeneration_steps(15);
		CHECK(mesh.get_rockgeneration_steps() == 15);
		mesh.set_rockgeneration_width(100.0f);
		CHECK(mesh.get_rockgeneration_width() == doctest::Approx(100.0f));
		mesh.set_rockgeneration_height(75.0f);
		CHECK(mesh.get_rockgeneration_height() == doctest::Approx(75.0f));
		mesh.set_rockgeneration_depth(60.0f);
		CHECK(mesh.get_rockgeneration_depth() == doctest::Approx(60.0f));
		mesh.set_rockgeneration_max_planes(5);
		CHECK(mesh.get_rockgeneration_max_planes() == 5);
	}

	TEST_CASE("[proc_rocks] rock_gen returns valid mesh arrays") {
		Array result = rock_gen(2, 42, 1.0, false);
		CHECK(result.size() == VS::ARRAY_MAX);
		Vector<Vector3> verts = result[VS::ARRAY_VERTEX];
		CHECK(verts.size() > 0);
		// 20 base faces × 4^depth triangles × 3 verts each
		CHECK(verts.size() == 20 * 16 * 3);
		Vector<Vector3> normals = result[VS::ARRAY_NORMAL];
		CHECK(normals.size() == verts.size());
	}

	TEST_CASE("[proc_rocks] rock_gen depth affects vertex count") {
		Array r1 = rock_gen(1, 42, 1.0, false);
		Array r2 = rock_gen(3, 42, 1.0, false);
		Vector<Vector3> v1 = r1[VS::ARRAY_VERTEX];
		Vector<Vector3> v2 = r2[VS::ARRAY_VERTEX];
		CHECK(v2.size() > v1.size());
	}

	TEST_CASE("[proc_rocks] rock_gen fixed seed is deterministic") {
		Array r1 = rock_gen(2, 123, 1.0, false);
		Array r2 = rock_gen(2, 123, 1.0, false);
		Vector<Vector3> v1 = r1[VS::ARRAY_VERTEX];
		Vector<Vector3> v2 = r2[VS::ARRAY_VERTEX];
		CHECK(v1.size() == v2.size());
		for (int i = 0; i < v1.size(); i++) {
			CHECK(v1[i].is_equal_approx(v2[i]));
		}
	}

	TEST_CASE("[proc_rocks] rock_gen smoothed mode") {
		Array result = rock_gen(2, 42, 1.0, true);
		CHECK(result.size() == VS::ARRAY_MAX);
		Vector<Vector3> verts = result[VS::ARRAY_VERTEX];
		CHECK(verts.size() > 0);
		Vector<Vector3> normals = result[VS::ARRAY_NORMAL];
		CHECK(normals.size() == verts.size());
	}

	TEST_CASE("[proc_rocks] auto_refresh triggers rebuild") {
		ProcRockMesh mesh;
		mesh.set_generator(0);
		mesh.set_rockgen_randseed(42);
		CHECK(mesh.get_surface_count() == 0);
		mesh.set_auto_refresh(true);
		CHECK(mesh.get_surface_count() == 1);
	}
}

#endif // DOCTEST
