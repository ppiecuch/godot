/**************************************************************************/
/*  iso_object.cpp                                                        */
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

#include "iso_object.h"
#include "iso_world.h"

void IsoObject::_find_iso_world() {
	_iso_world = nullptr;
	Node *parent = get_parent();
	while (parent) {
		IsoWorld *w = Object::cast_to<IsoWorld>(parent);
		if (w) {
			_iso_world = w;
			return;
		}
		parent = parent->get_parent();
	}
}

void IsoObject::_mark_dirty_iso_world() {
	if (_iso_world) {
		_iso_world->internal_mark_dirty(this);
	}
}

void IsoObject::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_find_iso_world();
			fix_transform();
			if (_iso_world) {
				_iso_world->internal_add_iso_object(this);
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			if (_iso_world) {
				_iso_world->internal_remove_iso_object(this);
			}
			_iso_world = nullptr;
		} break;
		case NOTIFICATION_MOVED_IN_PARENT: {
			_find_iso_world();
			fix_transform();
		} break;
	}
}

void IsoObject::set_iso_position(const Vector3 &p_position) {
	_iso_position = p_position;
	fix_transform();
}

Vector3 IsoObject::get_iso_position() const {
	return _iso_position;
}

void IsoObject::set_iso_position_x(real_t p_value) {
	_iso_position.x = p_value;
	fix_transform();
}

real_t IsoObject::get_iso_position_x() const {
	return _iso_position.x;
}

void IsoObject::set_iso_position_y(real_t p_value) {
	_iso_position.y = p_value;
	fix_transform();
}

real_t IsoObject::get_iso_position_y() const {
	return _iso_position.y;
}

void IsoObject::set_iso_position_z(real_t p_value) {
	_iso_position.z = p_value;
	fix_transform();
}

real_t IsoObject::get_iso_position_z() const {
	return _iso_position.z;
}

void IsoObject::set_iso_size(const Vector3 &p_size) {
	_iso_size = Vector3(MAX(p_size.x, 0.0f), MAX(p_size.y, 0.0f), MAX(p_size.z, 0.0f));
	fix_transform();
}

Vector3 IsoObject::get_iso_size() const {
	return _iso_size;
}

void IsoObject::set_iso_size_x(real_t p_value) {
	_iso_size.x = MAX(p_value, 0.0f);
	fix_transform();
}

real_t IsoObject::get_iso_size_x() const {
	return _iso_size.x;
}

void IsoObject::set_iso_size_y(real_t p_value) {
	_iso_size.y = MAX(p_value, 0.0f);
	fix_transform();
}

real_t IsoObject::get_iso_size_y() const {
	return _iso_size.y;
}

void IsoObject::set_iso_size_z(real_t p_value) {
	_iso_size.z = MAX(p_value, 0.0f);
	fix_transform();
}

real_t IsoObject::get_iso_size_z() const {
	return _iso_size.z;
}

void IsoObject::set_tile_position(const Vector3 &p_position) {
	_iso_position = p_position;
	fix_transform();
}

Vector3 IsoObject::get_tile_position() const {
	return IsoUtils::vec3_round(_iso_position);
}

void IsoObject::set_renderers_mode(RenderersMode p_mode) {
	_renderers_mode = p_mode;
	fix_transform();
}

IsoObject::RenderersMode IsoObject::get_renderers_mode() const {
	return _renderers_mode;
}

IsoWorld *IsoObject::get_iso_world() const {
	return _iso_world;
}

void IsoObject::fix_transform() {
	if (_iso_world) {
		Vector2 screen_pos = _iso_world->iso_to_screen(_iso_position);
		set_position(screen_pos);
		fix_screen_bounds();
		_mark_dirty_iso_world();
	}
}

void IsoObject::fix_iso_position() {
	if (_iso_world) {
		_iso_position = _iso_world->screen_to_iso(get_position(), _iso_position.z);
	}
}

void IsoObject::fix_screen_bounds() {
	if (_iso_world) {
		real_t l = _iso_world->iso_to_screen(_iso_position + IsoUtils::vec3_from_y(_iso_size.y)).x;
		real_t r = _iso_world->iso_to_screen(_iso_position + IsoUtils::vec3_from_x(_iso_size.x)).x;
		real_t b = _iso_world->iso_to_screen(_iso_position).y;
		real_t t = _iso_world->iso_to_screen(_iso_position + _iso_size).y;
		internal.qt_bounds.set(l, b, r, t);
	} else {
		internal.qt_bounds.set(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

void IsoObject::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_iso_position", "position"), &IsoObject::set_iso_position);
	ClassDB::bind_method(D_METHOD("get_iso_position"), &IsoObject::get_iso_position);

	ClassDB::bind_method(D_METHOD("set_iso_position_x", "value"), &IsoObject::set_iso_position_x);
	ClassDB::bind_method(D_METHOD("get_iso_position_x"), &IsoObject::get_iso_position_x);
	ClassDB::bind_method(D_METHOD("set_iso_position_y", "value"), &IsoObject::set_iso_position_y);
	ClassDB::bind_method(D_METHOD("get_iso_position_y"), &IsoObject::get_iso_position_y);
	ClassDB::bind_method(D_METHOD("set_iso_position_z", "value"), &IsoObject::set_iso_position_z);
	ClassDB::bind_method(D_METHOD("get_iso_position_z"), &IsoObject::get_iso_position_z);

	ClassDB::bind_method(D_METHOD("set_iso_size", "size"), &IsoObject::set_iso_size);
	ClassDB::bind_method(D_METHOD("get_iso_size"), &IsoObject::get_iso_size);

	ClassDB::bind_method(D_METHOD("set_iso_size_x", "value"), &IsoObject::set_iso_size_x);
	ClassDB::bind_method(D_METHOD("get_iso_size_x"), &IsoObject::get_iso_size_x);
	ClassDB::bind_method(D_METHOD("set_iso_size_y", "value"), &IsoObject::set_iso_size_y);
	ClassDB::bind_method(D_METHOD("get_iso_size_y"), &IsoObject::get_iso_size_y);
	ClassDB::bind_method(D_METHOD("set_iso_size_z", "value"), &IsoObject::set_iso_size_z);
	ClassDB::bind_method(D_METHOD("get_iso_size_z"), &IsoObject::get_iso_size_z);

	ClassDB::bind_method(D_METHOD("set_tile_position", "position"), &IsoObject::set_tile_position);
	ClassDB::bind_method(D_METHOD("get_tile_position"), &IsoObject::get_tile_position);

	ClassDB::bind_method(D_METHOD("set_renderers_mode", "mode"), &IsoObject::set_renderers_mode);
	ClassDB::bind_method(D_METHOD("get_renderers_mode"), &IsoObject::get_renderers_mode);

	ClassDB::bind_method(D_METHOD("fix_transform"), &IsoObject::fix_transform);
	ClassDB::bind_method(D_METHOD("fix_iso_position"), &IsoObject::fix_iso_position);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "iso_position"), "set_iso_position", "get_iso_position");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "iso_position_x"), "set_iso_position_x", "get_iso_position_x");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "iso_position_y"), "set_iso_position_y", "get_iso_position_y");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "iso_position_z"), "set_iso_position_z", "get_iso_position_z");

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "iso_size"), "set_iso_size", "get_iso_size");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "iso_size_x"), "set_iso_size_x", "get_iso_size_x");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "iso_size_y"), "set_iso_size_y", "get_iso_size_y");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "iso_size_z"), "set_iso_size_z", "get_iso_size_z");

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "tile_position"), "set_tile_position", "get_tile_position");

	ADD_PROPERTY(PropertyInfo(Variant::INT, "renderers_mode", PROPERTY_HINT_ENUM, "2D,3D"), "set_renderers_mode", "get_renderers_mode");

	BIND_ENUM_CONSTANT(MODE_2D);
	BIND_ENUM_CONSTANT(MODE_3D);
}

IsoObject::IsoObject() :
		_iso_position(Vector3()),
		_iso_size(Vector3(1, 1, 1)),
		_renderers_mode(MODE_2D),
		_iso_world(nullptr) {
}

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_SUITE("[[isotools]] IsoAssocList") {
	TEST_CASE("Empty list") {
		IsoAssocList<int> list;
		CHECK(list.count() == 0);
		CHECK_FALSE(list.contains(42));
	}

	TEST_CASE("Add and contains") {
		IsoAssocList<int> list;
		CHECK(list.add(10));
		CHECK(list.add(20));
		CHECK(list.add(30));
		CHECK(list.count() == 3);
		CHECK(list.contains(10));
		CHECK(list.contains(20));
		CHECK(list.contains(30));
		CHECK_FALSE(list.contains(40));
	}

	TEST_CASE("Add duplicate returns false") {
		IsoAssocList<int> list;
		CHECK(list.add(10));
		CHECK_FALSE(list.add(10));
		CHECK(list.count() == 1);
	}

	TEST_CASE("Remove") {
		IsoAssocList<int> list;
		list.add(10);
		list.add(20);
		list.add(30);
		CHECK(list.remove(20));
		CHECK(list.count() == 2);
		CHECK_FALSE(list.contains(20));
		CHECK(list.contains(10));
		CHECK(list.contains(30));
	}

	TEST_CASE("Remove nonexistent returns false") {
		IsoAssocList<int> list;
		list.add(10);
		CHECK_FALSE(list.remove(99));
		CHECK(list.count() == 1);
	}

	TEST_CASE("Pop") {
		IsoAssocList<int> list;
		list.add(10);
		list.add(20);
		int val = list.pop();
		CHECK(list.count() == 1);
		CHECK((val == 10 || val == 20));
	}

	TEST_CASE("Clear") {
		IsoAssocList<int> list;
		list.add(10);
		list.add(20);
		list.clear();
		CHECK(list.count() == 0);
		CHECK_FALSE(list.contains(10));
	}

	TEST_CASE("Index operator") {
		IsoAssocList<int> list;
		list.add(100);
		list.add(200);
		CHECK(list[0] == 100);
		CHECK(list[1] == 200);
	}
}

#endif // DOCTEST
