/**************************************************************************/
/*  iso_rigid_body.cpp                                                    */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#include "iso_rigid_body.h"
#include "iso_world.h"

void IsoRigidBody::_find_iso_world() {
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

void IsoRigidBody::_sync_to_screen() {
	if (_iso_world && !_syncing) {
		_syncing = true;
		set_position(_iso_world->iso_to_screen(_iso_position));
		_syncing = false;
	}
}

void IsoRigidBody::_sync_from_screen() {
	if (_iso_world && !_syncing) {
		_syncing = true;
		_iso_position = _iso_world->screen_to_iso(get_position(), _iso_position.z);
		_syncing = false;
	}
}

void IsoRigidBody::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_find_iso_world();
			_sync_to_screen();
			set_physics_process_internal(true);
		} break;
		case NOTIFICATION_EXIT_TREE: {
			_iso_world = nullptr;
			set_physics_process_internal(false);
		} break;
		case NOTIFICATION_INTERNAL_PHYSICS_PROCESS: {
			_sync_from_screen();
		} break;
	}
}

void IsoRigidBody::set_iso_position(const Vector3 &p_position) {
	_iso_position = p_position;
	_sync_to_screen();
}

Vector3 IsoRigidBody::get_iso_position() const {
	return _iso_position;
}

void IsoRigidBody::set_iso_size(const Vector3 &p_size) {
	_iso_size = Vector3(MAX(p_size.x, 0.0f), MAX(p_size.y, 0.0f), MAX(p_size.z, 0.0f));
}

Vector3 IsoRigidBody::get_iso_size() const {
	return _iso_size;
}

void IsoRigidBody::set_use_iso_gravity(bool p_use) {
	_use_iso_gravity = p_use;
}

bool IsoRigidBody::get_use_iso_gravity() const {
	return _use_iso_gravity;
}

IsoWorld *IsoRigidBody::get_iso_world() const {
	return _iso_world;
}

void IsoRigidBody::iso_add_central_force(const Vector3 &p_iso_force) {
	ERR_FAIL_COND(!_iso_world);
	Vector2 screen_force = _iso_world->iso_vector_to_screen(p_iso_force);
	add_central_force(screen_force);
}

void IsoRigidBody::iso_add_central_impulse(const Vector3 &p_iso_impulse) {
	ERR_FAIL_COND(!_iso_world);
	Vector2 screen_impulse = _iso_world->iso_vector_to_screen(p_iso_impulse);
	apply_central_impulse(screen_impulse);
}

void IsoRigidBody::iso_add_explosion_force(real_t p_force, const Vector3 &p_iso_origin, real_t p_radius) {
	ERR_FAIL_COND(!_iso_world);
	Vector3 diff = _iso_position - p_iso_origin;
	real_t dist = diff.length();
	if (dist <= 0 || dist > p_radius) {
		return;
	}
	real_t falloff = 1.0f - (dist / p_radius);
	Vector3 iso_impulse = diff.normalized() * p_force * falloff;
	Vector2 screen_impulse = _iso_world->iso_vector_to_screen(iso_impulse);
	apply_central_impulse(screen_impulse);
}

void IsoRigidBody::set_iso_velocity(const Vector3 &p_velocity) {
	ERR_FAIL_COND(!_iso_world);
	set_linear_velocity(_iso_world->iso_vector_to_screen(p_velocity));
}

Vector3 IsoRigidBody::get_iso_velocity() const {
	if (!_iso_world) {
		return Vector3();
	}
	return _iso_world->screen_vector_to_iso(get_linear_velocity());
}

void IsoRigidBody::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_iso_position", "position"), &IsoRigidBody::set_iso_position);
	ClassDB::bind_method(D_METHOD("get_iso_position"), &IsoRigidBody::get_iso_position);
	ClassDB::bind_method(D_METHOD("set_iso_size", "size"), &IsoRigidBody::set_iso_size);
	ClassDB::bind_method(D_METHOD("get_iso_size"), &IsoRigidBody::get_iso_size);
	ClassDB::bind_method(D_METHOD("set_use_iso_gravity", "use"), &IsoRigidBody::set_use_iso_gravity);
	ClassDB::bind_method(D_METHOD("get_use_iso_gravity"), &IsoRigidBody::get_use_iso_gravity);

	ClassDB::bind_method(D_METHOD("iso_add_central_force", "iso_force"), &IsoRigidBody::iso_add_central_force);
	ClassDB::bind_method(D_METHOD("iso_add_central_impulse", "iso_impulse"), &IsoRigidBody::iso_add_central_impulse);
	ClassDB::bind_method(D_METHOD("iso_add_explosion_force", "force", "iso_origin", "radius"), &IsoRigidBody::iso_add_explosion_force);

	ClassDB::bind_method(D_METHOD("set_iso_velocity", "velocity"), &IsoRigidBody::set_iso_velocity);
	ClassDB::bind_method(D_METHOD("get_iso_velocity"), &IsoRigidBody::get_iso_velocity);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "iso_position"), "set_iso_position", "get_iso_position");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "iso_size"), "set_iso_size", "get_iso_size");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_iso_gravity"), "set_use_iso_gravity", "get_use_iso_gravity");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "iso_velocity"), "set_iso_velocity", "get_iso_velocity");
}

IsoRigidBody::IsoRigidBody() :
		_iso_position(Vector3()),
		_iso_size(Vector3(1, 1, 1)),
		_iso_world(nullptr),
		_use_iso_gravity(false),
		_syncing(false) {
}

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_SUITE("[[isotools]] IsoRigidBody") {
	TEST_CASE("Default properties") {
		IsoRigidBody body;
		CHECK(body.get_iso_position() == Vector3());
		CHECK(body.get_iso_size() == Vector3(1, 1, 1));
		CHECK_FALSE(body.get_use_iso_gravity());
	}

	TEST_CASE("Set iso_position") {
		IsoRigidBody body;
		body.set_iso_position(Vector3(5, 3, 2));
		CHECK(body.get_iso_position() == Vector3(5, 3, 2));
	}

	TEST_CASE("Set iso_size clamps negatives") {
		IsoRigidBody body;
		body.set_iso_size(Vector3(-1, 2, -3));
		CHECK(body.get_iso_size().x == 0.0f);
		CHECK(body.get_iso_size().y == 2.0f);
		CHECK(body.get_iso_size().z == 0.0f);
	}
}

#endif // DOCTEST
