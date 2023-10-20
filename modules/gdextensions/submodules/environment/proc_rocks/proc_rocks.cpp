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
#ifdef TOOLS_ENABLED
#include "generators/procrockgen/procrockgen.h"
#endif

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

void ProcRockMesh::set_auto_refresh(bool p_refresh) {
	auto_refresh = p_refresh;
	if (auto_refresh) {
		_rebuild();
	}
}

bool ProcRockMesh::get_auto_refresh() const {
	return auto_refresh;
}

void ProcRockMesh::_get_property_list(List<PropertyInfo> *p_list) const {
#ifdef TOOLS_ENABLED
	if (method == 3 && procrock.pipeline) {
		if (procrock.pipeline->hasGenerator()) {
			const auto gen = &procrock.pipeline->getGenerator();
			const auto config = gen->getConfiguration();
		}
	}
#endif
}

bool ProcRockMesh::_set(const StringName &p_path, const Variant &p_value) {
	return false;
}

bool ProcRockMesh::_get(const StringName &p_path, Variant &r_ret) const {
	return false;
}

void ProcRockMesh::_rebuild() {
	if (_dirty) {
		switch (method) {
			case 0: {
				clear_surfaces();
				Array mesh = rock_gen(rockgen.depth, rockgen.randseed, rockgen.smoothness, rockgen.smoothed);
			} break;
			case 1: {
			} break;
			case 2: {
			} break;
		}
		_dirty = false;
	}
}

void ProcRockMesh::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_rockgen_depth", "depth"), &ProcRockMesh::set_rockgen_depth);
	ClassDB::bind_method(D_METHOD("get_rockgen_depth"), &ProcRockMesh::get_rockgen_depth);
	ClassDB::bind_method(D_METHOD("set_rockgen_randseed", "seed"), &ProcRockMesh::set_rockgen_randseed);
	ClassDB::bind_method(D_METHOD("get_rockgen_randseed"), &ProcRockMesh::get_rockgen_randseed);
	ClassDB::bind_method(D_METHOD("set_rockgen_smoothness", "smoothness"), &ProcRockMesh::set_rockgen_smoothness);
	ClassDB::bind_method(D_METHOD("get_rockgen_smoothness"), &ProcRockMesh::get_rockgen_smoothness);
	ClassDB::bind_method(D_METHOD("set_rockgen_smoothed", "smoothed"), &ProcRockMesh::set_rockgen_smoothed);
	ClassDB::bind_method(D_METHOD("get_rockgen_smoothed"), &ProcRockMesh::get_rockgen_smoothed);

	ClassDB::bind_method(D_METHOD("set_auto_refresh", "refresh"), &ProcRockMesh::set_auto_refresh);
	ClassDB::bind_method(D_METHOD("get_auto_refresh"), &ProcRockMesh::get_auto_refresh);

	ClassDB::bind_method(D_METHOD("_rebuild"), &ProcRockMesh::_rebuild);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_refresh"), "set_auto_refresh", "get_auto_refresh");

	ADD_GROUP("Method1", "rockgen_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "rockgen_depth"), "set_rockgen_depth", "get_rockgen_depth");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "rockgen_randseed"), "set_rockgen_randseed", "get_rockgen_randseed");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "rockgen_smoothness"), "set_rockgen_smoothness", "get_rockgen_smoothness");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "rockgen_smoothed"), "set_rockgen_smoothed", "get_rockgen_smoothed");

	ADD_GROUP("Method2", "rockgeneration_");

#ifdef TOOLS_ENABLED
	ADD_GROUP("Method3", "procrock_");
#endif
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

	procrock.pipeline = nullptr;

	_dirty = true;
}
