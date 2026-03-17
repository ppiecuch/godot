/**************************************************************************/
/*  iso_static_body.cpp                                                   */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#include "iso_static_body.h"
#include "iso_world.h"

void IsoStaticBody::_find_iso_world() {
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

void IsoStaticBody::_sync_to_screen() {
	if (_iso_world) {
		set_position(_iso_world->iso_to_screen(_iso_position));
	}
}

void IsoStaticBody::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_find_iso_world();
			_sync_to_screen();
		} break;
		case NOTIFICATION_EXIT_TREE: {
			_iso_world = nullptr;
		} break;
	}
}

void IsoStaticBody::set_iso_position(const Vector3 &p_position) {
	_iso_position = p_position;
	_sync_to_screen();
}

Vector3 IsoStaticBody::get_iso_position() const {
	return _iso_position;
}

void IsoStaticBody::set_iso_size(const Vector3 &p_size) {
	_iso_size = Vector3(MAX(p_size.x, 0.0f), MAX(p_size.y, 0.0f), MAX(p_size.z, 0.0f));
}

Vector3 IsoStaticBody::get_iso_size() const {
	return _iso_size;
}

IsoWorld *IsoStaticBody::get_iso_world() const {
	return _iso_world;
}

void IsoStaticBody::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_iso_position", "position"), &IsoStaticBody::set_iso_position);
	ClassDB::bind_method(D_METHOD("get_iso_position"), &IsoStaticBody::get_iso_position);
	ClassDB::bind_method(D_METHOD("set_iso_size", "size"), &IsoStaticBody::set_iso_size);
	ClassDB::bind_method(D_METHOD("get_iso_size"), &IsoStaticBody::get_iso_size);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "iso_position"), "set_iso_position", "get_iso_position");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "iso_size"), "set_iso_size", "get_iso_size");
}

IsoStaticBody::IsoStaticBody() :
		_iso_position(Vector3()),
		_iso_size(Vector3(1, 1, 1)),
		_iso_world(nullptr) {
}

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_SUITE("[[isotools]] IsoStaticBody") {
	TEST_CASE("Default properties") {
		IsoStaticBody body;
		CHECK(body.get_iso_position() == Vector3());
		CHECK(body.get_iso_size() == Vector3(1, 1, 1));
	}

	TEST_CASE("Set iso_position") {
		IsoStaticBody body;
		body.set_iso_position(Vector3(3, 4, 5));
		CHECK(body.get_iso_position() == Vector3(3, 4, 5));
	}

	TEST_CASE("Set iso_size clamps negatives") {
		IsoStaticBody body;
		body.set_iso_size(Vector3(-2, 3, -1));
		CHECK(body.get_iso_size().x == 0.0f);
		CHECK(body.get_iso_size().y == 3.0f);
		CHECK(body.get_iso_size().z == 0.0f);
	}

	TEST_CASE("No iso_world when not in tree") {
		IsoStaticBody body;
		CHECK(body.get_iso_world() == nullptr);
	}
}

#endif // DOCTEST
