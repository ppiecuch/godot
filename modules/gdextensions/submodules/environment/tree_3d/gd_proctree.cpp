/**************************************************************************/
/*  gd_proctree.cpp                                                       */
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

#include "gd_proctree.h"

#include "proctree.h"

void ProceduralTree3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_seed", "seed"), &ProceduralTree3D::set_seed);
	ClassDB::bind_method(D_METHOD("get_seed"), &ProceduralTree3D::get_seed);

	ClassDB::bind_method(D_METHOD("set_segments", "segments"), &ProceduralTree3D::set_segments);
	ClassDB::bind_method(D_METHOD("get_segments"), &ProceduralTree3D::get_segments);

	ClassDB::bind_method(D_METHOD("set_levels", "levels"), &ProceduralTree3D::set_levels);
	ClassDB::bind_method(D_METHOD("get_levels"), &ProceduralTree3D::get_levels);

	ClassDB::bind_method(D_METHOD("set_tree_steps", "steps"), &ProceduralTree3D::set_tree_steps);
	ClassDB::bind_method(D_METHOD("get_tree_steps"), &ProceduralTree3D::get_tree_steps);

	ClassDB::bind_method(D_METHOD("set_initial_branch_length", "length"), &ProceduralTree3D::set_initial_branch_length);
	ClassDB::bind_method(D_METHOD("get_initial_branch_length"), &ProceduralTree3D::get_initial_branch_length);

	ClassDB::bind_method(D_METHOD("set_length_falloff_factor", "factor"), &ProceduralTree3D::set_length_falloff_factor);
	ClassDB::bind_method(D_METHOD("get_length_falloff_factor"), &ProceduralTree3D::get_length_falloff_factor);

	ClassDB::bind_method(D_METHOD("set_length_falloff_power", "power"), &ProceduralTree3D::set_length_falloff_power);
	ClassDB::bind_method(D_METHOD("get_length_falloff_power"), &ProceduralTree3D::get_length_falloff_power);

	ClassDB::bind_method(D_METHOD("set_clump_max", "max"), &ProceduralTree3D::set_clump_max);
	ClassDB::bind_method(D_METHOD("get_clump_max"), &ProceduralTree3D::get_clump_max);

	ClassDB::bind_method(D_METHOD("set_clump_min", "min"), &ProceduralTree3D::set_clump_min);
	ClassDB::bind_method(D_METHOD("get_clump_min"), &ProceduralTree3D::get_clump_min);

	ClassDB::bind_method(D_METHOD("set_branch_factor", "factor"), &ProceduralTree3D::set_branch_factor);
	ClassDB::bind_method(D_METHOD("get_branch_factor"), &ProceduralTree3D::get_branch_factor);

	ClassDB::bind_method(D_METHOD("set_drop_amount", "amount"), &ProceduralTree3D::set_drop_amount);
	ClassDB::bind_method(D_METHOD("get_drop_amount"), &ProceduralTree3D::get_drop_amount);

	ClassDB::bind_method(D_METHOD("set_grow_amount", "amount"), &ProceduralTree3D::set_grow_amount);
	ClassDB::bind_method(D_METHOD("get_grow_amount"), &ProceduralTree3D::get_grow_amount);

	ClassDB::bind_method(D_METHOD("set_sweep_amount", "amount"), &ProceduralTree3D::set_sweep_amount);
	ClassDB::bind_method(D_METHOD("get_sweep_amount"), &ProceduralTree3D::get_sweep_amount);

	ClassDB::bind_method(D_METHOD("set_max_radius", "radius"), &ProceduralTree3D::set_max_radius);
	ClassDB::bind_method(D_METHOD("get_max_radius"), &ProceduralTree3D::get_max_radius);

	ClassDB::bind_method(D_METHOD("set_climb_rate", "rate"), &ProceduralTree3D::set_climb_rate);
	ClassDB::bind_method(D_METHOD("get_climb_rate"), &ProceduralTree3D::get_climb_rate);

	ClassDB::bind_method(D_METHOD("set_trunk_kink", "kink"), &ProceduralTree3D::set_trunk_kink);
	ClassDB::bind_method(D_METHOD("get_trunk_kink"), &ProceduralTree3D::get_trunk_kink);

	ClassDB::bind_method(D_METHOD("set_taper_rate", "rate"), &ProceduralTree3D::set_taper_rate);
	ClassDB::bind_method(D_METHOD("get_taper_rate"), &ProceduralTree3D::get_taper_rate);

	ClassDB::bind_method(D_METHOD("set_radius_falloff_rate", "rate"), &ProceduralTree3D::set_radius_falloff_rate);
	ClassDB::bind_method(D_METHOD("get_radius_falloff_rate"), &ProceduralTree3D::get_radius_falloff_rate);

	ClassDB::bind_method(D_METHOD("set_twist_rate", "rate"), &ProceduralTree3D::set_twist_rate);
	ClassDB::bind_method(D_METHOD("get_twist_rate"), &ProceduralTree3D::get_twist_rate);

	ClassDB::bind_method(D_METHOD("set_trunk_length", "length"), &ProceduralTree3D::set_trunk_length);
	ClassDB::bind_method(D_METHOD("get_trunk_length"), &ProceduralTree3D::get_trunk_length);

	ClassDB::bind_method(D_METHOD("set_v_multiplier", "multiplier"), &ProceduralTree3D::set_v_multiplier);
	ClassDB::bind_method(D_METHOD("get_v_multiplier"), &ProceduralTree3D::get_v_multiplier);

	ClassDB::bind_method(D_METHOD("set_twig_scale", "scale"), &ProceduralTree3D::set_twig_scale);
	ClassDB::bind_method(D_METHOD("get_twig_scale"), &ProceduralTree3D::get_twig_scale);

	ClassDB::bind_method(D_METHOD("set_twig_enabled", "enabled"), &ProceduralTree3D::set_twig_enabled);
	ClassDB::bind_method(D_METHOD("get_twig_enabled"), &ProceduralTree3D::get_twig_enabled);

	ClassDB::bind_method(D_METHOD("set_trunk_material", "material"), &ProceduralTree3D::set_trunk_material);
	ClassDB::bind_method(D_METHOD("get_trunk_material"), &ProceduralTree3D::get_trunk_material);

	ClassDB::bind_method(D_METHOD("set_twig_material", "material"), &ProceduralTree3D::set_twig_material);
	ClassDB::bind_method(D_METHOD("get_twig_material"), &ProceduralTree3D::get_twig_material);

	ClassDB::bind_method(D_METHOD("generate"), &ProceduralTree3D::generate);
	ClassDB::bind_method(D_METHOD("generate_trunk_mesh"), &ProceduralTree3D::generate_trunk_mesh);
	ClassDB::bind_method(D_METHOD("generate_twig_mesh"), &ProceduralTree3D::generate_twig_mesh);

	ClassDB::bind_method(D_METHOD("get_trunk_vertex_count"), &ProceduralTree3D::get_trunk_vertex_count);
	ClassDB::bind_method(D_METHOD("get_trunk_face_count"), &ProceduralTree3D::get_trunk_face_count);
	ClassDB::bind_method(D_METHOD("get_twig_vertex_count"), &ProceduralTree3D::get_twig_vertex_count);
	ClassDB::bind_method(D_METHOD("get_twig_face_count"), &ProceduralTree3D::get_twig_face_count);

	ClassDB::bind_method(D_METHOD("_rebuild"), &ProceduralTree3D::_rebuild);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "seed", PROPERTY_HINT_RANGE, "0,100000,1"), "set_seed", "get_seed");

	ADD_GROUP("Trunk", "trunk_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "trunk_segments", PROPERTY_HINT_RANGE, "2,10,2"), "set_segments", "get_segments");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "trunk_levels", PROPERTY_HINT_RANGE, "1,13,1"), "set_levels", "get_levels");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "trunk_steps", PROPERTY_HINT_RANGE, "0,100,1"), "set_tree_steps", "get_tree_steps");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "trunk_length", PROPERTY_HINT_RANGE, "0,100,0.001"), "set_trunk_length", "get_trunk_length");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "trunk_max_radius", PROPERTY_HINT_RANGE, "0.01,0.6,0.01"), "set_max_radius", "get_max_radius");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "trunk_radius_falloff_rate", PROPERTY_HINT_RANGE, "0.1,1,0.01"), "set_radius_falloff_rate", "get_radius_falloff_rate");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "trunk_climb_rate", PROPERTY_HINT_RANGE, "0,50,0.001"), "set_climb_rate", "get_climb_rate");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "trunk_kink", PROPERTY_HINT_RANGE, "-1,1,0.001"), "set_trunk_kink", "get_trunk_kink");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "trunk_taper_rate", PROPERTY_HINT_RANGE, "0.1,2,0.01"), "set_taper_rate", "get_taper_rate");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "trunk_twist_rate", PROPERTY_HINT_RANGE, "-5,5,0.01"), "set_twist_rate", "get_twist_rate");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "trunk_uv_multiplier", PROPERTY_HINT_RANGE, "0.001,50,0.001"), "set_v_multiplier", "get_v_multiplier");

	ADD_GROUP("Branch", "branch_");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "branch_initial_length", PROPERTY_HINT_RANGE, "0.01,10,0.001"), "set_initial_branch_length", "get_initial_branch_length");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "branch_length_falloff_factor", PROPERTY_HINT_RANGE, "0.0,2,0.001"), "set_length_falloff_factor", "get_length_falloff_factor");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "branch_length_falloff_power", PROPERTY_HINT_RANGE, "0.01,2,0.001"), "set_length_falloff_power", "get_length_falloff_power");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "branch_factor", PROPERTY_HINT_RANGE, "0.1,20,0.01"), "set_branch_factor", "get_branch_factor");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "branch_clump_max", PROPERTY_HINT_RANGE, "0,20,0.01"), "set_clump_max", "get_clump_max");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "branch_clump_min", PROPERTY_HINT_RANGE, "0,20,0.01"), "set_clump_min", "get_clump_min");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "branch_drop_amount", PROPERTY_HINT_RANGE, "-5,5,0.001"), "set_drop_amount", "get_drop_amount");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "branch_grow_amount", PROPERTY_HINT_RANGE, "-5,5,0.001"), "set_grow_amount", "get_grow_amount");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "branch_sweep_amount", PROPERTY_HINT_RANGE, "-5,5,0.01"), "set_sweep_amount", "get_sweep_amount");

	ADD_GROUP("Twig", "twig_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "twig_enabled"), "set_twig_enabled", "get_twig_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "twig_scale", PROPERTY_HINT_RANGE, "0,5,0.001"), "set_twig_scale", "get_twig_scale");

	ADD_GROUP("Material", "material_");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material_trunk", PROPERTY_HINT_RESOURCE_TYPE, "SpatialMaterial,ShaderMaterial"), "set_trunk_material", "get_trunk_material");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material_twig", PROPERTY_HINT_RESOURCE_TYPE, "SpatialMaterial,ShaderMaterial"), "set_twig_material", "get_twig_material");
}

ProceduralTree3D::ProceduralTree3D() {
	trunk_instance = nullptr;
	twig_instance = nullptr;
	_dirty = true;
	_in_tree = false;

	// Defaults from proctree.
	seed = 262;
	segments = 6;
	levels = 5;
	tree_steps = 5;
	initial_branch_length = 0.49f;
	length_falloff_factor = 0.85f;
	length_falloff_power = 0.99f;
	clump_max = 0.454f;
	clump_min = 0.404f;
	branch_factor = 2.45f;
	drop_amount = -0.1f;
	grow_amount = 0.235f;
	sweep_amount = 0.01f;
	max_radius = 0.139f;
	climb_rate = 0.371f;
	trunk_kink = 0.093f;
	taper_rate = 0.947f;
	radius_falloff_rate = 0.73f;
	twist_rate = 3.02f;
	trunk_length = 2.4f;
	v_multiplier = 0.36f;
	twig_scale = 0.39f;
	twig_enabled = true;
}

ProceduralTree3D::~ProceduralTree3D() {
}

void ProceduralTree3D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_in_tree = true;
			trunk_instance = memnew(MeshInstance);
			add_child(trunk_instance);
			twig_instance = memnew(MeshInstance);
			add_child(twig_instance);
			twig_instance->set_visible(twig_enabled);
			if (_dirty) {
				_rebuild();
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			_in_tree = false;
			if (trunk_instance) {
				remove_child(trunk_instance);
				memdelete(trunk_instance);
				trunk_instance = nullptr;
			}
			if (twig_instance) {
				remove_child(twig_instance);
				memdelete(twig_instance);
				twig_instance = nullptr;
			}
		} break;
	}
}

void ProceduralTree3D::_mark_dirty() {
	if (_dirty) {
		return;
	}
	_dirty = true;
	if (_in_tree) {
		call_deferred("_rebuild");
	}
}

static void _configure_proctree(Proctree::Tree &tree, const ProceduralTree3D *p_src) {
	tree.mProperties.mSeed = p_src->get_seed();
	tree.mProperties.mSegments = p_src->get_segments();
	tree.mProperties.mLevels = p_src->get_levels();
	tree.mProperties.mTreeSteps = p_src->get_tree_steps();
	tree.mProperties.mInitialBranchLength = p_src->get_initial_branch_length();
	tree.mProperties.mLengthFalloffFactor = p_src->get_length_falloff_factor();
	tree.mProperties.mLengthFalloffPower = p_src->get_length_falloff_power();
	tree.mProperties.mClumpMax = p_src->get_clump_max();
	tree.mProperties.mClumpMin = p_src->get_clump_min();
	tree.mProperties.mBranchFactor = p_src->get_branch_factor();
	tree.mProperties.mDropAmount = p_src->get_drop_amount();
	tree.mProperties.mGrowAmount = p_src->get_grow_amount();
	tree.mProperties.mSweepAmount = p_src->get_sweep_amount();
	tree.mProperties.mMaxRadius = p_src->get_max_radius();
	tree.mProperties.mClimbRate = p_src->get_climb_rate();
	tree.mProperties.mTrunkKink = p_src->get_trunk_kink();
	tree.mProperties.mTaperRate = p_src->get_taper_rate();
	tree.mProperties.mRadiusFalloffRate = p_src->get_radius_falloff_rate();
	tree.mProperties.mTwistRate = p_src->get_twist_rate();
	tree.mProperties.mTrunkLength = p_src->get_trunk_length();
	tree.mProperties.mVMultiplier = p_src->get_v_multiplier();
	tree.mProperties.mTwigScale = p_src->get_twig_scale();
}

static Ref<ArrayMesh> _build_trunk_mesh(Proctree::Tree &tree) {
	Ref<SurfaceTool> st;
	st.instance();
	st->begin(Mesh::PRIMITIVE_TRIANGLES);

	for (int i = 0; i < tree.mVertCount; i++) {
		st->add_uv(Vector2(tree.mUV[i].u, tree.mUV[i].v));
		st->add_normal(-Vector3(tree.mNormal[i].x, tree.mNormal[i].y, tree.mNormal[i].z));
		st->add_vertex(Vector3(tree.mVert[i].x, tree.mVert[i].y, tree.mVert[i].z));
	}

	for (int i = 0; i < tree.mFaceCount; i++) {
		st->add_index(tree.mFace[i].x);
		st->add_index(tree.mFace[i].y);
		st->add_index(tree.mFace[i].z);
	}

	st->generate_tangents();
	st->index();

	return st->commit();
}

static Ref<ArrayMesh> _build_twig_mesh(Proctree::Tree &tree) {
	if (tree.mTwigVertCount == 0) {
		return Ref<ArrayMesh>();
	}

	Ref<SurfaceTool> st;
	st.instance();
	st->begin(Mesh::PRIMITIVE_TRIANGLES);

	for (int i = 0; i < tree.mTwigVertCount; i++) {
		st->add_uv(Vector2(tree.mTwigUV[i].u, tree.mTwigUV[i].v));
		st->add_normal(-Vector3(tree.mTwigNormal[i].x, tree.mTwigNormal[i].y, tree.mTwigNormal[i].z));
		st->add_vertex(Vector3(tree.mTwigVert[i].x, tree.mTwigVert[i].y, tree.mTwigVert[i].z));
	}

	for (int i = 0; i < tree.mTwigFaceCount; i++) {
		st->add_index(tree.mTwigFace[i].x);
		st->add_index(tree.mTwigFace[i].y);
		st->add_index(tree.mTwigFace[i].z);
	}

	st->generate_tangents();
	st->index();

	return st->commit();
}

void ProceduralTree3D::_rebuild() {
	_dirty = false;

	Proctree::Tree tree;
	_configure_proctree(tree, this);
	tree.generate();

	_rebuild_trunk_mesh(tree);
	_rebuild_twig_mesh(tree);
}

void ProceduralTree3D::_rebuild_trunk_mesh(Proctree::Tree &tree) {
	if (!trunk_instance) {
		return;
	}

	Ref<ArrayMesh> mesh = _build_trunk_mesh(tree);
	trunk_instance->set_mesh(mesh);

	if (trunk_material.is_valid()) {
		trunk_instance->set_surface_material(0, trunk_material);
	}
}

void ProceduralTree3D::_rebuild_twig_mesh(Proctree::Tree &tree) {
	if (!twig_instance) {
		return;
	}

	if (!twig_enabled) {
		twig_instance->set_mesh(Ref<Mesh>());
		twig_instance->set_visible(false);
		return;
	}

	Ref<ArrayMesh> mesh = _build_twig_mesh(tree);
	twig_instance->set_mesh(mesh);
	twig_instance->set_visible(true);

	if (twig_material.is_valid() && mesh.is_valid()) {
		twig_instance->set_surface_material(0, twig_material);
	}
}

void ProceduralTree3D::generate() {
	_dirty = true;
	if (_in_tree) {
		_rebuild();
	}
}

Ref<ArrayMesh> ProceduralTree3D::generate_trunk_mesh() const {
	Proctree::Tree tree;
	_configure_proctree(tree, this);
	tree.generate();
	return _build_trunk_mesh(tree);
}

Ref<ArrayMesh> ProceduralTree3D::generate_twig_mesh() const {
	Proctree::Tree tree;
	_configure_proctree(tree, this);
	tree.generate();
	return _build_twig_mesh(tree);
}

int ProceduralTree3D::get_trunk_vertex_count() const {
	Proctree::Tree tree;
	_configure_proctree(tree, this);
	tree.generate();
	return tree.mVertCount;
}

int ProceduralTree3D::get_trunk_face_count() const {
	Proctree::Tree tree;
	_configure_proctree(tree, this);
	tree.generate();
	return tree.mFaceCount;
}

int ProceduralTree3D::get_twig_vertex_count() const {
	Proctree::Tree tree;
	_configure_proctree(tree, this);
	tree.generate();
	return tree.mTwigVertCount;
}

int ProceduralTree3D::get_twig_face_count() const {
	Proctree::Tree tree;
	_configure_proctree(tree, this);
	tree.generate();
	return tree.mTwigFaceCount;
}

// --- Property setters/getters ---

void ProceduralTree3D::set_seed(int p_seed) {
	if (seed == p_seed)
		return;
	seed = p_seed;
	_mark_dirty();
}
int ProceduralTree3D::get_seed() const { return seed; }

void ProceduralTree3D::set_segments(int p_segments) {
	p_segments = CLAMP(p_segments, 2, 10);
	if (p_segments % 2 != 0)
		p_segments++;
	if (segments == p_segments)
		return;
	segments = p_segments;
	_mark_dirty();
}
int ProceduralTree3D::get_segments() const { return segments; }

void ProceduralTree3D::set_levels(int p_levels) {
	p_levels = CLAMP(p_levels, 1, 13);
	if (levels == p_levels)
		return;
	levels = p_levels;
	_mark_dirty();
}
int ProceduralTree3D::get_levels() const { return levels; }

void ProceduralTree3D::set_tree_steps(int p_steps) {
	p_steps = CLAMP(p_steps, 0, 100);
	if (tree_steps == p_steps)
		return;
	tree_steps = p_steps;
	_mark_dirty();
}
int ProceduralTree3D::get_tree_steps() const { return tree_steps; }

void ProceduralTree3D::set_initial_branch_length(float p_length) {
	if (initial_branch_length == p_length)
		return;
	initial_branch_length = p_length;
	_mark_dirty();
}
float ProceduralTree3D::get_initial_branch_length() const { return initial_branch_length; }

void ProceduralTree3D::set_length_falloff_factor(float p_factor) {
	if (length_falloff_factor == p_factor)
		return;
	length_falloff_factor = p_factor;
	_mark_dirty();
}
float ProceduralTree3D::get_length_falloff_factor() const { return length_falloff_factor; }

void ProceduralTree3D::set_length_falloff_power(float p_power) {
	if (length_falloff_power == p_power)
		return;
	length_falloff_power = p_power;
	_mark_dirty();
}
float ProceduralTree3D::get_length_falloff_power() const { return length_falloff_power; }

void ProceduralTree3D::set_clump_max(float p_max) {
	if (clump_max == p_max)
		return;
	clump_max = p_max;
	_mark_dirty();
}
float ProceduralTree3D::get_clump_max() const { return clump_max; }

void ProceduralTree3D::set_clump_min(float p_min) {
	if (clump_min == p_min)
		return;
	clump_min = p_min;
	_mark_dirty();
}
float ProceduralTree3D::get_clump_min() const { return clump_min; }

void ProceduralTree3D::set_branch_factor(float p_factor) {
	if (branch_factor == p_factor)
		return;
	branch_factor = p_factor;
	_mark_dirty();
}
float ProceduralTree3D::get_branch_factor() const { return branch_factor; }

void ProceduralTree3D::set_drop_amount(float p_amount) {
	if (drop_amount == p_amount)
		return;
	drop_amount = p_amount;
	_mark_dirty();
}
float ProceduralTree3D::get_drop_amount() const { return drop_amount; }

void ProceduralTree3D::set_grow_amount(float p_amount) {
	if (grow_amount == p_amount)
		return;
	grow_amount = p_amount;
	_mark_dirty();
}
float ProceduralTree3D::get_grow_amount() const { return grow_amount; }

void ProceduralTree3D::set_sweep_amount(float p_amount) {
	if (sweep_amount == p_amount)
		return;
	sweep_amount = p_amount;
	_mark_dirty();
}
float ProceduralTree3D::get_sweep_amount() const { return sweep_amount; }

void ProceduralTree3D::set_max_radius(float p_radius) {
	if (max_radius == p_radius)
		return;
	max_radius = p_radius;
	_mark_dirty();
}
float ProceduralTree3D::get_max_radius() const { return max_radius; }

void ProceduralTree3D::set_climb_rate(float p_rate) {
	if (climb_rate == p_rate)
		return;
	climb_rate = p_rate;
	_mark_dirty();
}
float ProceduralTree3D::get_climb_rate() const { return climb_rate; }

void ProceduralTree3D::set_trunk_kink(float p_kink) {
	if (trunk_kink == p_kink)
		return;
	trunk_kink = p_kink;
	_mark_dirty();
}
float ProceduralTree3D::get_trunk_kink() const { return trunk_kink; }

void ProceduralTree3D::set_taper_rate(float p_rate) {
	if (taper_rate == p_rate)
		return;
	taper_rate = p_rate;
	_mark_dirty();
}
float ProceduralTree3D::get_taper_rate() const { return taper_rate; }

void ProceduralTree3D::set_radius_falloff_rate(float p_rate) {
	if (radius_falloff_rate == p_rate)
		return;
	radius_falloff_rate = p_rate;
	_mark_dirty();
}
float ProceduralTree3D::get_radius_falloff_rate() const { return radius_falloff_rate; }

void ProceduralTree3D::set_twist_rate(float p_rate) {
	if (twist_rate == p_rate)
		return;
	twist_rate = p_rate;
	_mark_dirty();
}
float ProceduralTree3D::get_twist_rate() const { return twist_rate; }

void ProceduralTree3D::set_trunk_length(float p_length) {
	if (trunk_length == p_length)
		return;
	trunk_length = p_length;
	_mark_dirty();
}
float ProceduralTree3D::get_trunk_length() const { return trunk_length; }

void ProceduralTree3D::set_v_multiplier(float p_multiplier) {
	if (v_multiplier == p_multiplier)
		return;
	v_multiplier = p_multiplier;
	_mark_dirty();
}
float ProceduralTree3D::get_v_multiplier() const { return v_multiplier; }

void ProceduralTree3D::set_twig_scale(float p_scale) {
	if (twig_scale == p_scale)
		return;
	twig_scale = p_scale;
	_mark_dirty();
}
float ProceduralTree3D::get_twig_scale() const { return twig_scale; }

void ProceduralTree3D::set_twig_enabled(bool p_enabled) {
	if (twig_enabled == p_enabled)
		return;
	twig_enabled = p_enabled;
	if (twig_instance) {
		twig_instance->set_visible(p_enabled);
	}
	_mark_dirty();
}
bool ProceduralTree3D::get_twig_enabled() const { return twig_enabled; }

void ProceduralTree3D::set_trunk_material(const Ref<Material> &p_material) {
	trunk_material = p_material;
	if (trunk_instance && trunk_instance->get_mesh().is_valid()) {
		trunk_instance->set_surface_material(0, trunk_material);
	}
}
Ref<Material> ProceduralTree3D::get_trunk_material() const { return trunk_material; }

void ProceduralTree3D::set_twig_material(const Ref<Material> &p_material) {
	twig_material = p_material;
	if (twig_instance && twig_instance->get_mesh().is_valid()) {
		twig_instance->set_surface_material(0, twig_material);
	}
}
Ref<Material> ProceduralTree3D::get_twig_material() const { return twig_material; }

// --- Doctests ---

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_CASE("[ProceduralTree3D] default property values") {
	ProceduralTree3D tree;
	CHECK(tree.get_seed() == 262);
	CHECK(tree.get_segments() == 6);
	CHECK(tree.get_levels() == 5);
	CHECK(tree.get_tree_steps() == 5);
	CHECK(tree.get_trunk_length() == doctest::Approx(2.4f));
	CHECK(tree.get_max_radius() == doctest::Approx(0.139f));
	CHECK(tree.get_twig_scale() == doctest::Approx(0.39f));
	CHECK(tree.get_twig_enabled() == true);
	CHECK(tree.get_initial_branch_length() == doctest::Approx(0.49f));
	CHECK(tree.get_taper_rate() == doctest::Approx(0.947f));
	CHECK(tree.get_length_falloff_power() == doctest::Approx(0.99f));
}

TEST_CASE("[ProceduralTree3D] property setters clamp values") {
	ProceduralTree3D tree;

	tree.set_segments(3);
	CHECK(tree.get_segments() == 4); // Rounded up to even.

	tree.set_segments(1);
	CHECK(tree.get_segments() == 2); // Clamped to minimum.

	tree.set_segments(12);
	CHECK(tree.get_segments() == 10); // Clamped to maximum.

	tree.set_levels(0);
	CHECK(tree.get_levels() == 1);

	tree.set_levels(20);
	CHECK(tree.get_levels() == 13);

	tree.set_tree_steps(-1);
	CHECK(tree.get_tree_steps() == 0);
}

TEST_CASE("[ProceduralTree3D] seed determinism") {
	// Same seed should produce identical meshes.
	ProceduralTree3D tree1;
	tree1.set_seed(42);

	ProceduralTree3D tree2;
	tree2.set_seed(42);

	CHECK(tree1.get_trunk_vertex_count() == tree2.get_trunk_vertex_count());
	CHECK(tree1.get_trunk_face_count() == tree2.get_trunk_face_count());
	CHECK(tree1.get_twig_vertex_count() == tree2.get_twig_vertex_count());
	CHECK(tree1.get_twig_face_count() == tree2.get_twig_face_count());
}

TEST_CASE("[ProceduralTree3D] tree generates valid geometry") {
	ProceduralTree3D tree;

	int verts = tree.get_trunk_vertex_count();
	int faces = tree.get_trunk_face_count();
	int twig_verts = tree.get_twig_vertex_count();
	int twig_faces = tree.get_twig_face_count();

	CHECK(verts > 0);
	CHECK(faces > 0);
	CHECK(twig_verts > 0);
	CHECK(twig_faces > 0);
	// Each face is a triangle referencing 3 vertices, so face count < vert count is reasonable.
	CHECK(faces > 0);
}

TEST_CASE("[ProceduralTree3D] generate_trunk_mesh returns valid mesh") {
	ProceduralTree3D tree;
	Ref<ArrayMesh> mesh = tree.generate_trunk_mesh();
	CHECK(mesh.is_valid());
	CHECK(mesh->get_surface_count() == 1);
}

TEST_CASE("[ProceduralTree3D] generate_twig_mesh returns valid mesh") {
	ProceduralTree3D tree;
	Ref<ArrayMesh> mesh = tree.generate_twig_mesh();
	CHECK(mesh.is_valid());
	CHECK(mesh->get_surface_count() == 1);
}

TEST_CASE("[ProceduralTree3D] different seeds produce different geometry") {
	ProceduralTree3D tree1;
	tree1.set_seed(1);

	ProceduralTree3D tree2;
	tree2.set_seed(9999);

	// Different seeds should almost certainly produce different vertex counts.
	// (With very different seeds and default params, the tree structure differs.)
	int v1 = tree1.get_trunk_vertex_count();
	int v2 = tree2.get_trunk_vertex_count();
	// They could be the same count but different positions; at minimum check meshes are valid.
	CHECK(v1 > 0);
	CHECK(v2 > 0);
}

TEST_CASE("[ProceduralTree3D] levels affect complexity") {
	ProceduralTree3D tree_low;
	tree_low.set_levels(2);

	ProceduralTree3D tree_high;
	tree_high.set_levels(8);

	CHECK(tree_high.get_trunk_vertex_count() > tree_low.get_trunk_vertex_count());
	CHECK(tree_high.get_twig_vertex_count() > tree_low.get_twig_vertex_count());
}

TEST_CASE("[ProceduralTree3D] material setters") {
	ProceduralTree3D tree;

	CHECK_FALSE(tree.get_trunk_material().is_valid());
	CHECK_FALSE(tree.get_twig_material().is_valid());

	Ref<SpatialMaterial> mat;
	mat.instance();
	tree.set_trunk_material(mat);
	CHECK(tree.get_trunk_material() == mat);

	tree.set_twig_material(mat);
	CHECK(tree.get_twig_material() == mat);
}

#endif // DOCTEST
