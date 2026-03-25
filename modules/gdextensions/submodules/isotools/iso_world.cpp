/**************************************************************************/
/*  iso_world.cpp                                                         */
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

#include "iso_world.h"
#include "iso_object.h"

// Default constants
static const real_t DEF_TILE_SIZE = 32.0f;
static const real_t MIN_TILE_SIZE = CMP_EPSILON;
static const real_t MAX_TILE_SIZE = 10000.0f;

static const real_t DEF_TILE_RATIO = 0.5f;
static const real_t MIN_TILE_RATIO = 0.25f;
static const real_t MAX_TILE_RATIO = 1.0f;

static const real_t DEF_TILE_ANGLE = 45.0f;
static const real_t MIN_TILE_ANGLE = 0.0f;
static const real_t MAX_TILE_ANGLE = 90.0f;

static const real_t DEF_TILE_HEIGHT = 32.0f;
static const real_t MIN_TILE_HEIGHT = CMP_EPSILON;
static const real_t MAX_TILE_HEIGHT = 10000.0f;

static const real_t DEF_STEP_DEPTH = 0.1f;
static const real_t DEF_START_DEPTH = 1.0f;

void IsoWorld::_update_iso_matrix() {
	// Unity: Scale(1, ratio, 1) * TRS(zero, AngleAxis(90-angle, Z), (size*sqrt2, size*sqrt2, height))
	// In 2D, this projects to:
	real_t a = Math::deg2rad(90.0f - _tile_angle);
	real_t s = _tile_size * (real_t)Math_SQRT2;

	_iso_matrix[0] = Vector2(Math::cos(a) * s, Math::sin(a) * s * _tile_ratio);
	_iso_matrix[1] = Vector2(-Math::sin(a) * s, Math::cos(a) * s * _tile_ratio);
	_iso_matrix[2] = Vector2(0, 0);

	_iso_rmatrix = _iso_matrix.affine_inverse();
}

void IsoWorld::_fix_iso_object_transforms() {
	for (int i = 0; i < _iso_objects.size(); i++) {
		_iso_objects[i]->fix_transform();
	}
}

void IsoWorld::_change_sorting_property() {
	_update_iso_matrix();
	_fix_iso_object_transforms();
}

void IsoWorld::_step_sorting_process() {
	_screen_solver.step_sorting_action(this);
	if (_sorting_solver.step_sorting_action(this, _screen_solver)) {
		// Sorting changed
	}
	_warning_solver.step_sorting_action(this);
}

void IsoWorld::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_change_sorting_property();
			_step_sorting_process();
			set_process_internal(true);
		} break;
		case NOTIFICATION_EXIT_TREE: {
			_screen_solver.clear();
			_sorting_solver.clear();
			_warning_solver.clear();
			set_process_internal(false);
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
			_step_sorting_process();
		} break;
	}
}

// --- Properties ---

void IsoWorld::set_tile_size(real_t p_size) {
	_tile_size = CLAMP(p_size, MIN_TILE_SIZE, MAX_TILE_SIZE);
	_change_sorting_property();
}

real_t IsoWorld::get_tile_size() const {
	return _tile_size;
}

void IsoWorld::set_tile_ratio(real_t p_ratio) {
	_tile_ratio = CLAMP(p_ratio, MIN_TILE_RATIO, MAX_TILE_RATIO);
	_change_sorting_property();
}

real_t IsoWorld::get_tile_ratio() const {
	return _tile_ratio;
}

void IsoWorld::set_tile_angle(real_t p_angle) {
	_tile_angle = CLAMP(p_angle, MIN_TILE_ANGLE, MAX_TILE_ANGLE);
	_change_sorting_property();
}

real_t IsoWorld::get_tile_angle() const {
	return _tile_angle;
}

void IsoWorld::set_tile_height(real_t p_height) {
	_tile_height = CLAMP(p_height, MIN_TILE_HEIGHT, MAX_TILE_HEIGHT);
	_change_sorting_property();
}

real_t IsoWorld::get_tile_height() const {
	return _tile_height;
}

void IsoWorld::set_step_depth(real_t p_depth) {
	_step_depth = MAX(p_depth, CMP_EPSILON);
	_change_sorting_property();
}

real_t IsoWorld::get_step_depth() const {
	return _step_depth;
}

void IsoWorld::set_start_depth(real_t p_depth) {
	_start_depth = p_depth;
	_change_sorting_property();
}

real_t IsoWorld::get_start_depth() const {
	return _start_depth;
}

void IsoWorld::set_show_iso_bounds(bool p_show) { _show_iso_bounds = p_show; }
bool IsoWorld::get_show_iso_bounds() const { return _show_iso_bounds; }

void IsoWorld::set_show_screen_bounds(bool p_show) { _show_screen_bounds = p_show; }
bool IsoWorld::get_show_screen_bounds() const { return _show_screen_bounds; }

void IsoWorld::set_show_depends(bool p_show) { _show_depends = p_show; }
bool IsoWorld::get_show_depends() const { return _show_depends; }

void IsoWorld::set_show_quad_tree(bool p_show) { _show_quad_tree = p_show; }
bool IsoWorld::get_show_quad_tree() const { return _show_quad_tree; }

// --- Coordinate conversion ---

Vector2 IsoWorld::iso_to_screen(const Vector3 &p_iso) const {
	Vector2 screen_pos = _iso_matrix.xform(Vector2(p_iso.x, p_iso.y));
	screen_pos.y += p_iso.z * _tile_height;
	return screen_pos;
}

Vector3 IsoWorld::screen_to_iso(const Vector2 &p_screen) const {
	Vector2 iso_2d = _iso_rmatrix.xform(p_screen);
	return Vector3(iso_2d.x, iso_2d.y, 0);
}

Vector3 IsoWorld::screen_to_iso(const Vector2 &p_screen, real_t p_iso_z) const {
	Vector2 adjusted = Vector2(p_screen.x, p_screen.y - p_iso_z * _tile_height);
	Vector3 result = screen_to_iso(adjusted);
	result.z = p_iso_z;
	return result;
}

Vector2 IsoWorld::iso_vector_to_screen(const Vector3 &p_iso_vec) const {
	// Same as iso_to_screen but for direction vectors (no origin offset)
	Vector2 screen_vec = _iso_matrix.basis_xform(Vector2(p_iso_vec.x, p_iso_vec.y));
	screen_vec.y += p_iso_vec.z * _tile_height;
	return screen_vec;
}

Vector3 IsoWorld::screen_vector_to_iso(const Vector2 &p_screen_vec) const {
	Vector2 iso_2d = _iso_rmatrix.basis_xform(p_screen_vec);
	return Vector3(iso_2d.x, iso_2d.y, 0);
}

Vector3 IsoWorld::mouse_iso_position(real_t p_iso_z) const {
	Vector2 mouse_pos = get_viewport()->get_mouse_position();
	// Convert from viewport to world coordinates
	Vector2 world_pos = get_canvas_transform().affine_inverse().xform(mouse_pos);
	return screen_to_iso(world_pos, p_iso_z);
}

Vector3 IsoWorld::mouse_iso_tile_position(real_t p_iso_z) const {
	return IsoUtils::vec3_floor(mouse_iso_position(p_iso_z));
}

// --- Internal object management ---

void IsoWorld::internal_add_iso_object(IsoObject *p_object) {
	_iso_objects.push_back(p_object);
	_screen_solver.on_add_iso_object(p_object);
	_sorting_solver.on_add_iso_object(p_object);
	_warning_solver.on_add_iso_object(p_object);
}

void IsoWorld::internal_remove_iso_object(IsoObject *p_object) {
	int idx = _iso_objects.find(p_object);
	if (idx >= 0) {
		int last = _iso_objects.size() - 1;
		if (idx != last) {
			_iso_objects.write[idx] = _iso_objects[last];
		}
		_iso_objects.resize(last);
	}
	_screen_solver.on_remove_iso_object(p_object);
	_sorting_solver.on_remove_iso_object(p_object);
	_warning_solver.on_remove_iso_object(p_object);
}

void IsoWorld::internal_mark_dirty(IsoObject *p_object) {
	_screen_solver.on_mark_dirty_iso_object(p_object);
	_sorting_solver.on_mark_dirty_iso_object(p_object);
	_warning_solver.on_mark_dirty_iso_object(p_object);
}

bool IsoWorld::internal_is_visible(IsoObject *p_object) const {
	return _screen_solver.get_cur_visibles().contains(p_object);
}

// --- Bind methods ---

void IsoWorld::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_tile_size", "size"), &IsoWorld::set_tile_size);
	ClassDB::bind_method(D_METHOD("get_tile_size"), &IsoWorld::get_tile_size);

	ClassDB::bind_method(D_METHOD("set_tile_ratio", "ratio"), &IsoWorld::set_tile_ratio);
	ClassDB::bind_method(D_METHOD("get_tile_ratio"), &IsoWorld::get_tile_ratio);

	ClassDB::bind_method(D_METHOD("set_tile_angle", "angle"), &IsoWorld::set_tile_angle);
	ClassDB::bind_method(D_METHOD("get_tile_angle"), &IsoWorld::get_tile_angle);

	ClassDB::bind_method(D_METHOD("set_tile_height", "height"), &IsoWorld::set_tile_height);
	ClassDB::bind_method(D_METHOD("get_tile_height"), &IsoWorld::get_tile_height);

	ClassDB::bind_method(D_METHOD("set_step_depth", "depth"), &IsoWorld::set_step_depth);
	ClassDB::bind_method(D_METHOD("get_step_depth"), &IsoWorld::get_step_depth);

	ClassDB::bind_method(D_METHOD("set_start_depth", "depth"), &IsoWorld::set_start_depth);
	ClassDB::bind_method(D_METHOD("get_start_depth"), &IsoWorld::get_start_depth);

	ClassDB::bind_method(D_METHOD("set_show_iso_bounds", "show"), &IsoWorld::set_show_iso_bounds);
	ClassDB::bind_method(D_METHOD("get_show_iso_bounds"), &IsoWorld::get_show_iso_bounds);

	ClassDB::bind_method(D_METHOD("set_show_screen_bounds", "show"), &IsoWorld::set_show_screen_bounds);
	ClassDB::bind_method(D_METHOD("get_show_screen_bounds"), &IsoWorld::get_show_screen_bounds);

	ClassDB::bind_method(D_METHOD("set_show_depends", "show"), &IsoWorld::set_show_depends);
	ClassDB::bind_method(D_METHOD("get_show_depends"), &IsoWorld::get_show_depends);

	ClassDB::bind_method(D_METHOD("set_show_quad_tree", "show"), &IsoWorld::set_show_quad_tree);
	ClassDB::bind_method(D_METHOD("get_show_quad_tree"), &IsoWorld::get_show_quad_tree);

	ClassDB::bind_method(D_METHOD("iso_to_screen", "iso_position"), &IsoWorld::iso_to_screen);
	ClassDB::bind_method(D_METHOD("screen_to_iso_2d", "screen_position"), static_cast<Vector3 (IsoWorld::*)(const Vector2 &) const>(&IsoWorld::screen_to_iso));
	ClassDB::bind_method(D_METHOD("screen_to_iso_3d", "screen_position", "iso_z"), static_cast<Vector3 (IsoWorld::*)(const Vector2 &, real_t) const>(&IsoWorld::screen_to_iso));

	ClassDB::bind_method(D_METHOD("iso_vector_to_screen", "iso_vector"), &IsoWorld::iso_vector_to_screen);
	ClassDB::bind_method(D_METHOD("screen_vector_to_iso", "screen_vector"), &IsoWorld::screen_vector_to_iso);

	ClassDB::bind_method(D_METHOD("mouse_iso_position", "iso_z"), &IsoWorld::mouse_iso_position, DEFVAL(0.0f));
	ClassDB::bind_method(D_METHOD("mouse_iso_tile_position", "iso_z"), &IsoWorld::mouse_iso_tile_position, DEFVAL(0.0f));

	ADD_GROUP("Sorting", "");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "tile_size", PROPERTY_HINT_RANGE, "0.001,10000,0.1"), "set_tile_size", "get_tile_size");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "tile_ratio", PROPERTY_HINT_RANGE, "0.25,1.0,0.01"), "set_tile_ratio", "get_tile_ratio");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "tile_angle", PROPERTY_HINT_RANGE, "0,90,0.1"), "set_tile_angle", "get_tile_angle");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "tile_height", PROPERTY_HINT_RANGE, "0.001,10000,0.1"), "set_tile_height", "get_tile_height");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "step_depth", PROPERTY_HINT_RANGE, "0.001,100,0.01"), "set_step_depth", "get_step_depth");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "start_depth"), "set_start_depth", "get_start_depth");

	ADD_GROUP("Debug", "show_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_iso_bounds"), "set_show_iso_bounds", "get_show_iso_bounds");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_screen_bounds"), "set_show_screen_bounds", "get_show_screen_bounds");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_depends"), "set_show_depends", "get_show_depends");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_quad_tree"), "set_show_quad_tree", "get_show_quad_tree");
}

IsoWorld::IsoWorld() :
		_tile_size(DEF_TILE_SIZE),
		_tile_ratio(DEF_TILE_RATIO),
		_tile_angle(DEF_TILE_ANGLE),
		_tile_height(DEF_TILE_HEIGHT),
		_step_depth(DEF_STEP_DEPTH),
		_start_depth(DEF_START_DEPTH),
		_show_iso_bounds(false),
		_show_screen_bounds(false),
		_show_depends(false),
		_show_quad_tree(false) {
	_update_iso_matrix();
}

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_SUITE("[[isotools]] IsoMinMax") {
	TEST_CASE("Construction") {
		SUBCASE("Default constructor") {
			IsoMinMax mm;
			CHECK(mm.min == 0.0f);
			CHECK(mm.max == 0.0f);
		}
		SUBCASE("Single value constructor") {
			IsoMinMax mm(5.0f);
			CHECK(mm.min == 5.0f);
			CHECK(mm.max == 5.0f);
		}
		SUBCASE("Min/max constructor") {
			IsoMinMax mm(2.0f, 8.0f);
			CHECK(mm.min == 2.0f);
			CHECK(mm.max == 8.0f);
		}
	}

	TEST_CASE("Size and center") {
		IsoMinMax mm(2.0f, 10.0f);
		CHECK(mm.get_size() == doctest::Approx(8.0f));
		CHECK(mm.get_center() == doctest::Approx(6.0f));
	}

	TEST_CASE("Set methods") {
		IsoMinMax mm;
		SUBCASE("set single value") {
			mm.set(3.0f);
			CHECK(mm.min == 3.0f);
			CHECK(mm.max == 3.0f);
		}
		SUBCASE("set min/max") {
			mm.set(1.0f, 5.0f);
			CHECK(mm.min == 1.0f);
			CHECK(mm.max == 5.0f);
		}
		SUBCASE("set from other") {
			IsoMinMax other(7.0f, 12.0f);
			mm.set(other);
			CHECK(mm.min == 7.0f);
			CHECK(mm.max == 12.0f);
		}
	}

	TEST_CASE("Resize") {
		IsoMinMax mm(2.0f, 5.0f);
		mm.resize(10.0f);
		CHECK(mm.min == 2.0f);
		CHECK(mm.max == doctest::Approx(12.0f));
	}

	TEST_CASE("Translate") {
		IsoMinMax mm(2.0f, 5.0f);
		mm.translate(3.0f);
		CHECK(mm.min == doctest::Approx(5.0f));
		CHECK(mm.max == doctest::Approx(8.0f));
	}

	TEST_CASE("Contains") {
		IsoMinMax mm(2.0f, 8.0f);
		SUBCASE("contains value") {
			CHECK(mm.contains(5.0f));
			CHECK(mm.contains(2.0f));
			CHECK(mm.contains(8.0f));
			CHECK_FALSE(mm.contains(1.0f));
			CHECK_FALSE(mm.contains(9.0f));
		}
		SUBCASE("contains range") {
			CHECK(mm.contains(IsoMinMax(3.0f, 7.0f)));
			CHECK(mm.contains(IsoMinMax(2.0f, 8.0f)));
			CHECK_FALSE(mm.contains(IsoMinMax(1.0f, 5.0f)));
			CHECK_FALSE(mm.contains(IsoMinMax(5.0f, 10.0f)));
		}
	}

	TEST_CASE("Overlaps") {
		IsoMinMax mm(2.0f, 8.0f);
		CHECK(mm.overlaps(IsoMinMax(5.0f, 10.0f)));
		CHECK(mm.overlaps(IsoMinMax(0.0f, 3.0f)));
		CHECK_FALSE(mm.overlaps(IsoMinMax(8.0f, 10.0f)));
		CHECK_FALSE(mm.overlaps(IsoMinMax(0.0f, 2.0f)));
		CHECK_FALSE(mm.overlaps(IsoMinMax(10.0f, 15.0f)));
	}

	TEST_CASE("Approximately") {
		IsoMinMax a(2.0f, 8.0f);
		IsoMinMax b(2.0f, 8.0f);
		CHECK(a.approximately(b));
		IsoMinMax c(2.0f, 9.0f);
		CHECK_FALSE(a.approximately(c));
	}

	TEST_CASE("Merge") {
		IsoMinMax a(2.0f, 5.0f);
		IsoMinMax b(3.0f, 8.0f);
		IsoMinMax result = IsoMinMax::merge(a, b);
		CHECK(result.min == doctest::Approx(2.0f));
		CHECK(result.max == doctest::Approx(8.0f));
	}
}

TEST_SUITE("[[isotools]] IsoRect") {
	TEST_CASE("Construction") {
		SUBCASE("Default constructor") {
			IsoRect r;
			CHECK(r.x.min == 0.0f);
			CHECK(r.x.max == 0.0f);
			CHECK(r.y.min == 0.0f);
			CHECK(r.y.max == 0.0f);
		}
		SUBCASE("Min/max floats constructor") {
			IsoRect r(1.0f, 2.0f, 5.0f, 8.0f);
			CHECK(r.x.min == 1.0f);
			CHECK(r.x.max == 5.0f);
			CHECK(r.y.min == 2.0f);
			CHECK(r.y.max == 8.0f);
		}
		SUBCASE("Vector2 constructor") {
			IsoRect r(Vector2(1.0f, 2.0f), Vector2(5.0f, 8.0f));
			CHECK(r.x.min == 1.0f);
			CHECK(r.x.max == 5.0f);
			CHECK(r.y.min == 2.0f);
			CHECK(r.y.max == 8.0f);
		}
	}

	TEST_CASE("Size and center") {
		IsoRect r(1.0f, 2.0f, 5.0f, 8.0f);
		Vector2 size = r.get_size();
		Vector2 center = r.get_center();
		CHECK(size.x == doctest::Approx(4.0f));
		CHECK(size.y == doctest::Approx(6.0f));
		CHECK(center.x == doctest::Approx(3.0f));
		CHECK(center.y == doctest::Approx(5.0f));
	}

	TEST_CASE("Contains point") {
		IsoRect r(0.0f, 0.0f, 10.0f, 10.0f);
		CHECK(r.contains(Vector2(5.0f, 5.0f)));
		CHECK(r.contains(Vector2(0.0f, 0.0f)));
		CHECK(r.contains(Vector2(10.0f, 10.0f)));
		CHECK_FALSE(r.contains(Vector2(-1.0f, 5.0f)));
		CHECK_FALSE(r.contains(Vector2(11.0f, 5.0f)));
	}

	TEST_CASE("Contains rect") {
		IsoRect r(0.0f, 0.0f, 10.0f, 10.0f);
		CHECK(r.contains(IsoRect(2.0f, 2.0f, 8.0f, 8.0f)));
		CHECK_FALSE(r.contains(IsoRect(-1.0f, 2.0f, 8.0f, 8.0f)));
	}

	TEST_CASE("Overlaps") {
		IsoRect r(0.0f, 0.0f, 10.0f, 10.0f);
		CHECK(r.overlaps(IsoRect(5.0f, 5.0f, 15.0f, 15.0f)));
		CHECK(r.overlaps(IsoRect(-5.0f, -5.0f, 5.0f, 5.0f)));
		CHECK_FALSE(r.overlaps(IsoRect(10.0f, 0.0f, 20.0f, 10.0f)));
		CHECK_FALSE(r.overlaps(IsoRect(20.0f, 20.0f, 30.0f, 30.0f)));
	}

	TEST_CASE("Translate") {
		IsoRect r(0.0f, 0.0f, 10.0f, 10.0f);
		r.translate(5.0f, 3.0f);
		CHECK(r.x.min == doctest::Approx(5.0f));
		CHECK(r.x.max == doctest::Approx(15.0f));
		CHECK(r.y.min == doctest::Approx(3.0f));
		CHECK(r.y.max == doctest::Approx(13.0f));
	}

	TEST_CASE("Merge") {
		IsoRect a(0.0f, 0.0f, 5.0f, 5.0f);
		IsoRect b(3.0f, 3.0f, 10.0f, 10.0f);
		IsoRect result = IsoRect::merge(a, b);
		CHECK(result.x.min == doctest::Approx(0.0f));
		CHECK(result.x.max == doctest::Approx(10.0f));
		CHECK(result.y.min == doctest::Approx(0.0f));
		CHECK(result.y.max == doctest::Approx(10.0f));
	}
}

TEST_SUITE("[[isotools]] IsoUtils") {
	TEST_CASE("Vector construction helpers") {
		CHECK(IsoUtils::vec3_from_x(5.0f) == Vector3(5, 0, 0));
		CHECK(IsoUtils::vec3_from_y(3.0f) == Vector3(0, 3, 0));
		CHECK(IsoUtils::vec3_from_z(7.0f) == Vector3(0, 0, 7));
		CHECK(IsoUtils::vec3_from_xy(2.0f, 4.0f) == Vector3(2, 4, 0));
		CHECK(IsoUtils::vec3_from_vec2(Vector2(3.0f, 5.0f)) == Vector3(3, 5, 0));
		CHECK(IsoUtils::vec3_from_vec2(Vector2(3.0f, 5.0f), 2.0f) == Vector3(3, 5, 2));
	}

	TEST_CASE("Vector component change helpers") {
		Vector3 v(1, 2, 3);
		CHECK(IsoUtils::vec3_change_x(v, 10) == Vector3(10, 2, 3));
		CHECK(IsoUtils::vec3_change_y(v, 20) == Vector3(1, 20, 3));
		CHECK(IsoUtils::vec3_change_z(v, 30) == Vector3(1, 2, 30));
		CHECK(IsoUtils::vec3_change_xy(v, 10, 20) == Vector3(10, 20, 3));
	}

	TEST_CASE("Vector min/max") {
		Vector2 a(1, 5);
		Vector2 b(3, 2);
		CHECK(IsoUtils::vec2_min(a, b) == Vector2(1, 2));
		CHECK(IsoUtils::vec2_max(a, b) == Vector2(3, 5));
	}

	TEST_CASE("Vector abs") {
		CHECK(IsoUtils::vec2_abs(Vector2(-3, 4)) == Vector2(3, 4));
		CHECK(IsoUtils::vec3_abs(Vector3(-1, -2, 3)) == Vector3(1, 2, 3));
	}

	TEST_CASE("Vector floor/ceil/round") {
		Vector3 v(1.3f, 2.7f, -0.5f);
		Vector3 floored = IsoUtils::vec3_floor(v);
		Vector3 ceiled = IsoUtils::vec3_ceil(v);
		Vector3 rounded = IsoUtils::vec3_round(v);
		CHECK(floored.x == doctest::Approx(1.0f));
		CHECK(floored.y == doctest::Approx(2.0f));
		CHECK(floored.z == doctest::Approx(-1.0f));
		CHECK(ceiled.x == doctest::Approx(2.0f));
		CHECK(ceiled.y == doctest::Approx(3.0f));
		CHECK(ceiled.z == doctest::Approx(0.0f));
		CHECK(rounded.x == doctest::Approx(1.0f));
		CHECK(rounded.y == doctest::Approx(3.0f));
		CHECK(rounded.z == doctest::Approx(-1.0f));
	}

	TEST_CASE("Approximately") {
		CHECK(IsoUtils::vec2_approximately(Vector2(1.0f, 2.0f), Vector2(1.0f, 2.0f)));
		CHECK_FALSE(IsoUtils::vec2_approximately(Vector2(1.0f, 2.0f), Vector2(1.1f, 2.0f)));
		CHECK(IsoUtils::vec2_approximately(Vector2(1.0f, 2.0f), Vector2(1.05f, 2.0f), 0.1f));
		CHECK_FALSE(IsoUtils::vec2_approximately(Vector2(1.0f, 2.0f), Vector2(1.2f, 2.0f), 0.1f));
	}

	TEST_CASE("Float beautifier") {
		CHECK(IsoUtils::float_beautifier(3.14159265f) == doctest::Approx(3.1416f));
		CHECK(IsoUtils::float_beautifier(0.0f) == doctest::Approx(0.0f));
		CHECK(IsoUtils::float_beautifier(-1.23456789f) == doctest::Approx(-1.2346f));
	}

	TEST_CASE("Min/max float helpers") {
		CHECK(IsoUtils::vec2_min_f(Vector2(3, 7)) == doctest::Approx(3.0f));
		CHECK(IsoUtils::vec2_max_f(Vector2(3, 7)) == doctest::Approx(7.0f));
		CHECK(IsoUtils::vec3_min_f(Vector3(5, 2, 8)) == doctest::Approx(2.0f));
		CHECK(IsoUtils::vec3_max_f(Vector3(5, 2, 8)) == doctest::Approx(8.0f));
	}
}

TEST_SUITE("[[isotools]] IsoQuadTree") {
	TEST_CASE("Add and remove items") {
		IsoQuadTree<int> tree;
		IsoRect bounds(0, 0, 10, 10);
		auto *item = tree.add_item(bounds, 42);
		REQUIRE(item != nullptr);

		int found = 0;
		tree.visit_items_by_bounds(IsoRect(-1, -1, 11, 11), [&found](int val) {
			found = val;
		});
		CHECK(found == 42);

		tree.remove_item(item);
		found = 0;
		tree.visit_items_by_bounds(IsoRect(-1, -1, 11, 11), [&found](int val) {
			found = val;
		});
		CHECK(found == 0);
	}

	TEST_CASE("Spatial query filters by bounds") {
		IsoQuadTree<int> tree;
		tree.add_item(IsoRect(0, 0, 5, 5), 1);
		tree.add_item(IsoRect(10, 10, 15, 15), 2);
		tree.add_item(IsoRect(20, 20, 25, 25), 3);

		int count = 0;
		tree.visit_items_by_bounds(IsoRect(8, 8, 16, 16), [&count](int val) {
			count++;
			CHECK(val == 2);
		});
		CHECK(count == 1);
	}

	TEST_CASE("Move item") {
		IsoQuadTree<int> tree;
		auto *item = tree.add_item(IsoRect(0, 0, 5, 5), 42);
		item = tree.move_item(IsoRect(100, 100, 105, 105), item);
		REQUIRE(item != nullptr);

		int found = 0;
		tree.visit_items_by_bounds(IsoRect(99, 99, 106, 106), [&found](int val) {
			found = val;
		});
		CHECK(found == 42);

		found = 0;
		tree.visit_items_by_bounds(IsoRect(-1, -1, 6, 6), [&found](int val) {
			found = val;
		});
		CHECK(found == 0);
	}

	TEST_CASE("Clear removes all items") {
		IsoQuadTree<int> tree;
		tree.add_item(IsoRect(0, 0, 5, 5), 1);
		tree.add_item(IsoRect(10, 10, 15, 15), 2);
		tree.clear();

		int count = 0;
		tree.visit_items_by_bounds(IsoRect(-100, -100, 100, 100), [&count](int) {
			count++;
		});
		CHECK(count == 0);
	}

	TEST_CASE("Multiple items in same region") {
		IsoQuadTree<int> tree;
		tree.add_item(IsoRect(0, 0, 5, 5), 1);
		tree.add_item(IsoRect(1, 1, 4, 4), 2);
		tree.add_item(IsoRect(2, 2, 3, 3), 3);

		int count = 0;
		tree.visit_items_by_bounds(IsoRect(-1, -1, 6, 6), [&count](int) {
			count++;
		});
		CHECK(count == 3);
	}

	TEST_CASE("Visit items by item") {
		IsoQuadTree<int> tree;
		auto *item1 = tree.add_item(IsoRect(0, 0, 10, 10), 1);
		tree.add_item(IsoRect(5, 5, 15, 15), 2);
		tree.add_item(IsoRect(50, 50, 60, 60), 3);

		int count = 0;
		tree.visit_items_by_item(item1, [&count](int val) {
			count++;
		});
		CHECK(count >= 1);
	}

	TEST_CASE("Tree grows for far items") {
		IsoQuadTree<int> tree;
		tree.add_item(IsoRect(0, 0, 1, 1), 1);
		tree.add_item(IsoRect(1000, 1000, 1001, 1001), 2);

		int count = 0;
		tree.visit_items_by_bounds(IsoRect(-2000, -2000, 2000, 2000), [&count](int) {
			count++;
		});
		CHECK(count == 2);
	}
}

TEST_SUITE("[[isotools]] IsoScreenSolver") {
	TEST_CASE("IsIsoObjectDepends: identical boxes have no ordering") {
		CHECK_FALSE(IsoScreenSolver::is_iso_object_depends(
				Vector3(0, 0, 0), Vector3(1, 1, 1),
				Vector3(0, 0, 0), Vector3(1, 1, 1)));
	}

	TEST_CASE("IsIsoObjectDepends: overlapping boxes have ordering") {
		// B is behind A in iso space, so A depends on B (A must draw after B)
		CHECK(IsoScreenSolver::is_iso_object_depends(
				Vector3(1, 1, 0), Vector3(2, 2, 2),
				Vector3(0, 0, 0), Vector3(2, 2, 2)));
	}

	TEST_CASE("IsIsoObjectDepends: no overlap") {
		CHECK_FALSE(IsoScreenSolver::is_iso_object_depends(
				Vector3(0, 0, 0), Vector3(1, 1, 1),
				Vector3(10, 10, 10), Vector3(1, 1, 1)));
	}

	TEST_CASE("IsIsoObjectDepends: partial overlap determines axis") {
		bool dep_forward = IsoScreenSolver::is_iso_object_depends(
				Vector3(0, 0, 0), Vector3(2, 2, 2),
				Vector3(1, 0, 0), Vector3(2, 2, 2));
		bool dep_reverse = IsoScreenSolver::is_iso_object_depends(
				Vector3(1, 0, 0), Vector3(2, 2, 2),
				Vector3(0, 0, 0), Vector3(2, 2, 2));
		CHECK(dep_forward != dep_reverse);
	}

	TEST_CASE("IsIsoObjectDepends: adjacent objects (no overlap)") {
		CHECK_FALSE(IsoScreenSolver::is_iso_object_depends(
				Vector3(0, 0, 0), Vector3(1, 1, 1),
				Vector3(1, 0, 0), Vector3(1, 1, 1)));
	}

	TEST_CASE("IsIsoObjectDepends: stacked vertically") {
		bool result = IsoScreenSolver::is_iso_object_depends(
				Vector3(0, 0, 0), Vector3(1, 1, 1),
				Vector3(0, 0, 0.5f), Vector3(1, 1, 1));
		CHECK(result);
	}
}

TEST_SUITE("[[isotools]] IsoWorld coordinate conversion") {
	TEST_CASE("Standard 2:1 isometric: iso_to_screen origin") {
		IsoWorld world;
		Vector2 screen = world.iso_to_screen(Vector3(0, 0, 0));
		CHECK(screen.x == doctest::Approx(0.0f));
		CHECK(screen.y == doctest::Approx(0.0f));
	}

	TEST_CASE("Standard 2:1 isometric: screen_to_iso roundtrip") {
		IsoWorld world;
		Vector3 iso_pos(3.5f, 2.0f, 0.0f);
		Vector2 screen = world.iso_to_screen(iso_pos);
		Vector3 back = world.screen_to_iso(screen);
		CHECK(back.x == doctest::Approx(iso_pos.x).epsilon(0.01));
		CHECK(back.y == doctest::Approx(iso_pos.y).epsilon(0.01));
	}

	TEST_CASE("screen_to_iso with Z offset roundtrip") {
		IsoWorld world;
		Vector3 iso_pos(2.0f, 3.0f, 5.0f);
		Vector2 screen = world.iso_to_screen(iso_pos);
		Vector3 back = world.screen_to_iso(screen, iso_pos.z);
		CHECK(back.x == doctest::Approx(iso_pos.x).epsilon(0.01));
		CHECK(back.y == doctest::Approx(iso_pos.y).epsilon(0.01));
		CHECK(back.z == doctest::Approx(iso_pos.z).epsilon(0.01));
	}

	TEST_CASE("iso_to_screen: Z raises Y") {
		IsoWorld world;
		Vector2 at_z0 = world.iso_to_screen(Vector3(0, 0, 0));
		Vector2 at_z1 = world.iso_to_screen(Vector3(0, 0, 1));
		CHECK(at_z1.y == doctest::Approx(at_z0.y + world.get_tile_height()));
	}

	TEST_CASE("iso_to_screen: X and Y produce diamond") {
		IsoWorld world;
		Vector2 right = world.iso_to_screen(Vector3(1, 0, 0));
		Vector2 down = world.iso_to_screen(Vector3(0, 1, 0));
		CHECK(right.x > 0);
		CHECK(right.y > 0);
		CHECK(down.x < 0);
		CHECK(down.y > 0);
	}

	TEST_CASE("Changing tile_size scales output") {
		IsoWorld world;
		world.set_tile_size(64.0f);
		Vector2 screen_64 = world.iso_to_screen(Vector3(1, 0, 0));
		world.set_tile_size(32.0f);
		Vector2 screen_32 = world.iso_to_screen(Vector3(1, 0, 0));
		CHECK(screen_64.x == doctest::Approx(screen_32.x * 2.0f).epsilon(0.01));
		CHECK(screen_64.y == doctest::Approx(screen_32.y * 2.0f).epsilon(0.01));
	}

	TEST_CASE("Changing tile_ratio affects Y scale") {
		IsoWorld world;
		world.set_tile_ratio(1.0f);
		Vector2 screen_1 = world.iso_to_screen(Vector3(1, 0, 0));
		world.set_tile_ratio(0.5f);
		Vector2 screen_half = world.iso_to_screen(Vector3(1, 0, 0));
		CHECK(screen_1.y == doctest::Approx(screen_half.y * 2.0f).epsilon(0.1));
	}

	TEST_CASE("Property clamping") {
		IsoWorld world;
		world.set_tile_size(-10.0f);
		CHECK(world.get_tile_size() > 0);

		world.set_tile_ratio(5.0f);
		CHECK(world.get_tile_ratio() <= 1.0f);

		world.set_tile_ratio(0.0f);
		CHECK(world.get_tile_ratio() >= 0.25f);

		world.set_tile_angle(-10.0f);
		CHECK(world.get_tile_angle() >= 0.0f);

		world.set_tile_angle(100.0f);
		CHECK(world.get_tile_angle() <= 90.0f);
	}
}

#endif // DOCTEST
