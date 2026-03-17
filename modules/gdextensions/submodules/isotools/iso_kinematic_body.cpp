/**************************************************************************/
/*  iso_kinematic_body.cpp                                                */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#include "iso_kinematic_body.h"
#include "iso_world.h"

void IsoKinematicBody::_find_iso_world() {
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

void IsoKinematicBody::_sync_to_screen() {
	if (_iso_world) {
		set_position(_iso_world->iso_to_screen(_iso_position));
	}
}

void IsoKinematicBody::_notification(int p_what) {
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

void IsoKinematicBody::set_iso_position(const Vector3 &p_position) {
	_iso_position = p_position;
	_sync_to_screen();
}

Vector3 IsoKinematicBody::get_iso_position() const {
	return _iso_position;
}

void IsoKinematicBody::set_iso_size(const Vector3 &p_size) {
	_iso_size = Vector3(MAX(p_size.x, 0.0f), MAX(p_size.y, 0.0f), MAX(p_size.z, 0.0f));
}

Vector3 IsoKinematicBody::get_iso_size() const {
	return _iso_size;
}

IsoWorld *IsoKinematicBody::get_iso_world() const {
	return _iso_world;
}

Vector3 IsoKinematicBody::iso_move_and_slide(const Vector3 &p_iso_velocity, const Vector3 &p_iso_floor_normal) {
	ERR_FAIL_COND_V(!_iso_world, Vector3());

	Vector2 screen_velocity = _iso_world->iso_vector_to_screen(p_iso_velocity);
	Vector2 screen_floor_normal = _iso_world->iso_vector_to_screen(p_iso_floor_normal).normalized();

	Vector2 result = move_and_slide(screen_velocity, screen_floor_normal);

	// Update iso_position from new screen position
	_iso_position = _iso_world->screen_to_iso(get_position(), _iso_position.z);

	return _iso_world->screen_vector_to_iso(result);
}

bool IsoKinematicBody::iso_move_and_collide(const Vector3 &p_iso_motion) {
	ERR_FAIL_COND_V(!_iso_world, false);

	Vector2 screen_motion = _iso_world->iso_vector_to_screen(p_iso_motion);
	Collision collision;
	bool collided = move_and_collide(screen_motion, true, collision);

	// Update iso_position from new screen position
	_iso_position = _iso_world->screen_to_iso(get_position(), _iso_position.z);

	return collided;
}

void IsoKinematicBody::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_iso_position", "position"), &IsoKinematicBody::set_iso_position);
	ClassDB::bind_method(D_METHOD("get_iso_position"), &IsoKinematicBody::get_iso_position);
	ClassDB::bind_method(D_METHOD("set_iso_size", "size"), &IsoKinematicBody::set_iso_size);
	ClassDB::bind_method(D_METHOD("get_iso_size"), &IsoKinematicBody::get_iso_size);

	ClassDB::bind_method(D_METHOD("iso_move_and_slide", "iso_velocity", "iso_floor_normal"), &IsoKinematicBody::iso_move_and_slide, DEFVAL(Vector3(0, 0, 1)));
	ClassDB::bind_method(D_METHOD("iso_move_and_collide", "iso_motion"), &IsoKinematicBody::iso_move_and_collide);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "iso_position"), "set_iso_position", "get_iso_position");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "iso_size"), "set_iso_size", "get_iso_size");
}

IsoKinematicBody::IsoKinematicBody() :
		_iso_position(Vector3()),
		_iso_size(Vector3(1, 1, 1)),
		_iso_world(nullptr) {
}

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_SUITE("[[isotools]] IsoKinematicBody") {
	TEST_CASE("Default properties") {
		IsoKinematicBody body;
		CHECK(body.get_iso_position() == Vector3());
		CHECK(body.get_iso_size() == Vector3(1, 1, 1));
	}

	TEST_CASE("Set iso_position") {
		IsoKinematicBody body;
		body.set_iso_position(Vector3(1, 2, 3));
		CHECK(body.get_iso_position() == Vector3(1, 2, 3));
	}

	TEST_CASE("Set iso_size clamps negatives") {
		IsoKinematicBody body;
		body.set_iso_size(Vector3(-1, 5, -2));
		CHECK(body.get_iso_size().x == 0.0f);
		CHECK(body.get_iso_size().y == 5.0f);
		CHECK(body.get_iso_size().z == 0.0f);
	}

	TEST_CASE("No iso_world when not in tree") {
		IsoKinematicBody body;
		CHECK(body.get_iso_world() == nullptr);
	}
}

#endif // DOCTEST
