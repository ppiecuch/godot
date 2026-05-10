/**************************************************************************/
/*  revolute_joint.cpp                                                    */
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

#include "godot_box2d.h"

#include "revolute_joint.h"

#include <Box2D/Box2D.h>

#define JOINT ((b2RevoluteJoint *)entity)
#define DEF ((b2RevoluteJointDef *)def)

float RevoluteJointB2::get_reference_angle() const {
	return JOINT->GetReferenceAngle();
}

float RevoluteJointB2::get_joint_angle() const {
	return JOINT->GetJointAngle();
}

float RevoluteJointB2::get_joint_speed() const {
	return JOINT->GetJointSpeed();
}

bool RevoluteJointB2::is_limit_enabled() const {
	return JOINT->IsLimitEnabled();
}

void RevoluteJointB2::set_limit_enabled(bool rhs) {
	return JOINT->EnableLimit(rhs);
}

float RevoluteJointB2::get_lower_limit() const {
	return JOINT->GetLowerLimit();
}

void RevoluteJointB2::set_lower_limit(float rhs) {
	JOINT->SetLimits(rhs, JOINT->GetUpperLimit());
}

float RevoluteJointB2::get_upper_limit() const {
	return JOINT->GetUpperLimit();
}

void RevoluteJointB2::set_upper_limit(float rhs) {
	JOINT->SetLimits(JOINT->GetLowerLimit(), rhs);
}

bool RevoluteJointB2::is_motor_enabled() const {
	return JOINT->IsMotorEnabled();
}

void RevoluteJointB2::set_motor_enabled(bool rhs) {
	return JOINT->EnableMotor(rhs);
}

float RevoluteJointB2::get_motor_speed() const {
	return JOINT->GetMotorSpeed();
}

void RevoluteJointB2::set_motor_speed(float rhs) {
	JOINT->SetMotorSpeed(rhs);
}

float RevoluteJointB2::get_max_motor_torque() const {
	return JOINT->GetMaxMotorTorque();
}

void RevoluteJointB2::set_max_motor_torque(float rhs) {
	JOINT->SetMaxMotorTorque(rhs);
}

float RevoluteJointB2::get_motor_torque(float inv_dt) const {
	return JOINT->GetMotorTorque(inv_dt);
}

void RevoluteJointB2::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_reference_angle"), &RevoluteJointB2::get_reference_angle);

	ClassDB::bind_method(D_METHOD("get_joint_angle"), &RevoluteJointB2::get_joint_angle);
	ClassDB::bind_method(D_METHOD("get_joint_speed"), &RevoluteJointB2::get_joint_speed);

	BOX2D_PROPERTY_BOOL(RevoluteJointB2, limit_enabled);
	BOX2D_PROPERTY(RevoluteJointB2, lower_limit, Variant::REAL);
	BOX2D_PROPERTY(RevoluteJointB2, upper_limit, Variant::REAL);
	BOX2D_PROPERTY_BOOL(RevoluteJointB2, motor_enabled);
	BOX2D_PROPERTY(RevoluteJointB2, motor_speed, Variant::REAL);
	BOX2D_PROPERTY(RevoluteJointB2, max_motor_torque, Variant::REAL);

	ClassDB::bind_method(D_METHOD("get_motor_torque", "inv_dt"), &RevoluteJointB2::get_motor_torque);
}

JointB2 *RevoluteJointDefB2::instance(WorldB2 *world) {
	ERR_FAIL_NULL_V(world, NULL);
	auto o = world->get_b2()->CreateJoint(def);
	return memnew(RevoluteJointB2(o));
}

RevoluteJointDefB2::RevoluteJointDefB2() :
		JointDefB2(memnew(b2RevoluteJointDef)) {
}

Vector2 RevoluteJointDefB2::get_anchor_a() const {
	return GD(DEF->localAnchorA);
}

void RevoluteJointDefB2::set_anchor_a(const Vector2 &o) {
	DEF->localAnchorA = B2(o);
}

Vector2 RevoluteJointDefB2::get_anchor_b() const {
	return GD(DEF->localAnchorB);
}

void RevoluteJointDefB2::set_anchor_b(const Vector2 &o) {
	DEF->localAnchorB = B2(o);
}

float RevoluteJointDefB2::get_reference_angle() const {
	return DEF->referenceAngle;
}

void RevoluteJointDefB2::set_reference_angle(float o) {
	DEF->referenceAngle = o;
}

bool RevoluteJointDefB2::get_enable_limit() const {
	return DEF->enableLimit;
}

void RevoluteJointDefB2::set_enable_limit(bool o) {
	DEF->enableLimit = o;
}

float RevoluteJointDefB2::get_lower_angle() const {
	return DEF->lowerAngle;
}

void RevoluteJointDefB2::set_lower_angle(float o) {
	DEF->lowerAngle = o;
}

float RevoluteJointDefB2::get_upper_angle() const {
	return DEF->upperAngle;
}

void RevoluteJointDefB2::set_upper_angle(float o) {
	DEF->upperAngle = o;
}

bool RevoluteJointDefB2::get_enable_motor() const {
	return DEF->enableMotor;
}

void RevoluteJointDefB2::set_enable_motor(bool o) {
	DEF->enableMotor = o;
}

float RevoluteJointDefB2::get_motor_speed() const {
	return DEF->motorSpeed;
}

void RevoluteJointDefB2::set_motor_speed(float o) {
	DEF->motorSpeed = o;
}

float RevoluteJointDefB2::get_max_motor_torque() const {
	return DEF->maxMotorTorque;
}

void RevoluteJointDefB2::set_max_motor_torque(float o) {
	DEF->maxMotorTorque = o;
}

void RevoluteJointDefB2::_bind_methods() {
	BOX2D_PROPERTY(RevoluteJointDefB2, anchor_a, Variant::VECTOR2);
	BOX2D_PROPERTY(RevoluteJointDefB2, anchor_b, Variant::VECTOR2);
	BOX2D_PROPERTY(RevoluteJointDefB2, reference_angle, Variant::REAL);
	BOX2D_PROPERTY(RevoluteJointDefB2, enable_limit, Variant::BOOL);
	BOX2D_PROPERTY(RevoluteJointDefB2, lower_angle, Variant::REAL);
	BOX2D_PROPERTY(RevoluteJointDefB2, upper_angle, Variant::REAL);
	BOX2D_PROPERTY(RevoluteJointDefB2, enable_motor, Variant::BOOL);
	BOX2D_PROPERTY(RevoluteJointDefB2, motor_speed, Variant::REAL);
	BOX2D_PROPERTY(RevoluteJointDefB2, max_motor_torque, Variant::REAL);
}

#ifdef DOCTEST
#include "doctest/doctest.h"

static WorldB2 *make_revolute_test_world() {
	return memnew(WorldB2(memnew(b2World(b2Vec2(0.0f, -9.8f)))));
}

static BodyB2 *make_revolute_test_body(WorldB2 *w, float x = 0.0f, float y = 0.0f) {
	b2BodyDef bdef;
	bdef.position.Set(x, y);
	return memnew(BodyB2(w->get_b2()->CreateBody(&bdef)));
}

TEST_SUITE("[liquidfun] RevoluteJointDefB2") {
	TEST_CASE("[liquidfun] RevoluteJointDefB2 defaults match b2RevoluteJointDef") {
		RevoluteJointDefB2 *def = memnew(RevoluteJointDefB2);
		CHECK(def->get_anchor_a() == Vector2(0.0f, 0.0f));
		CHECK(def->get_anchor_b() == Vector2(0.0f, 0.0f));
		CHECK(def->get_reference_angle() == doctest::Approx(0.0f));
		CHECK(def->get_enable_limit() == false);
		CHECK(def->get_lower_angle() == doctest::Approx(0.0f));
		CHECK(def->get_upper_angle() == doctest::Approx(0.0f));
		CHECK(def->get_enable_motor() == false);
		CHECK(def->get_motor_speed() == doctest::Approx(0.0f));
		CHECK(def->get_max_motor_torque() == doctest::Approx(0.0f));
		memdelete(def);
	}

	TEST_CASE("[liquidfun] RevoluteJointDefB2 setters roundtrip") {
		RevoluteJointDefB2 *def = memnew(RevoluteJointDefB2);
		def->set_anchor_a(Vector2(1.0f, 0.0f));
		def->set_anchor_b(Vector2(-1.0f, 0.0f));
		def->set_reference_angle(0.5f);
		def->set_enable_limit(true);
		def->set_lower_angle(-1.0f);
		def->set_upper_angle(1.0f);
		def->set_enable_motor(true);
		def->set_motor_speed(2.0f);
		def->set_max_motor_torque(10.0f);
		CHECK(def->get_anchor_a() == Vector2(1.0f, 0.0f));
		CHECK(def->get_anchor_b() == Vector2(-1.0f, 0.0f));
		CHECK(def->get_reference_angle() == doctest::Approx(0.5f));
		CHECK(def->get_enable_limit() == true);
		CHECK(def->get_lower_angle() == doctest::Approx(-1.0f));
		CHECK(def->get_upper_angle() == doctest::Approx(1.0f));
		CHECK(def->get_enable_motor() == true);
		CHECK(def->get_motor_speed() == doctest::Approx(2.0f));
		CHECK(def->get_max_motor_torque() == doctest::Approx(10.0f));
		memdelete(def);
	}
} // TEST_SUITE("[liquidfun] RevoluteJointDefB2")

TEST_SUITE("[liquidfun] RevoluteJointB2") {
	TEST_CASE("[liquidfun] RevoluteJointB2 is_active and body refs after creation") {
		WorldB2 *w = make_revolute_test_world();
		BodyB2 *body_a = make_revolute_test_body(w, 0.0f, 0.0f);
		BodyB2 *body_b = make_revolute_test_body(w, 2.0f, 0.0f);
		RevoluteJointDefB2 *def = memnew(RevoluteJointDefB2);
		def->set_body_a(body_a);
		def->set_body_b(body_b);
		JointB2 *joint = def->instance(w);
		memdelete(def);
		CHECK(joint != nullptr);
		CHECK(joint->is_active() == true);
		CHECK(joint->get_body_a() == body_a);
		CHECK(joint->get_body_b() == body_b);
		CHECK(body_a->get_joint_list().size() == 1);
		memdelete(w);
	}

	TEST_CASE("[liquidfun] RevoluteJointB2 metadata roundtrip") {
		WorldB2 *w = make_revolute_test_world();
		BodyB2 *body_a = make_revolute_test_body(w);
		BodyB2 *body_b = make_revolute_test_body(w, 1.0f, 0.0f);
		RevoluteJointDefB2 *def = memnew(RevoluteJointDefB2);
		def->set_body_a(body_a);
		def->set_body_b(body_b);
		JointB2 *joint = def->instance(w);
		memdelete(def);
		joint->set_metadata(String("revolute"));
		CHECK(String(joint->get_metadata()) == String("revolute"));
		memdelete(w);
	}
} // TEST_SUITE("[liquidfun] RevoluteJointB2")

#endif // DOCTEST
