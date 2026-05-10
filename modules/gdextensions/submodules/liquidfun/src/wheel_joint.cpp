/**************************************************************************/
/*  wheel_joint.cpp                                                       */
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

#include "wheel_joint.h"

#include <Box2D/Box2D.h>

#define JOINT ((b2WheelJoint *)entity)
#define DEF ((b2WheelJointDef *)def)

float WheelJointB2::get_joint_translation() const { return JOINT->GetJointTranslation(); }
float WheelJointB2::get_joint_speed() const { return JOINT->GetJointSpeed(); }

bool WheelJointB2::is_motor_enabled() const { return JOINT->IsMotorEnabled(); }
void WheelJointB2::set_motor_enabled(bool rhs) { JOINT->EnableMotor(rhs); }

float WheelJointB2::get_motor_speed() const { return JOINT->GetMotorSpeed(); }
void WheelJointB2::set_motor_speed(float rhs) { JOINT->SetMotorSpeed(rhs); }

float WheelJointB2::get_max_motor_torque() const { return JOINT->GetMaxMotorTorque(); }
void WheelJointB2::set_max_motor_torque(float rhs) { JOINT->SetMaxMotorTorque(rhs); }

float WheelJointB2::get_motor_torque(float inv_dt) const { return JOINT->GetMotorTorque(inv_dt); }

float WheelJointB2::get_spring_frequency() const { return JOINT->GetSpringFrequencyHz(); }
void WheelJointB2::set_spring_frequency(float rhs) { JOINT->SetSpringFrequencyHz(rhs); }

float WheelJointB2::get_spring_damping() const { return JOINT->GetSpringDampingRatio(); }
void WheelJointB2::set_spring_damping(float rhs) { JOINT->SetSpringDampingRatio(rhs); }

void WheelJointB2::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_joint_translation"), &WheelJointB2::get_joint_translation);
	ClassDB::bind_method(D_METHOD("get_joint_speed"), &WheelJointB2::get_joint_speed);

	BOX2D_PROPERTY_BOOL(WheelJointB2, motor_enabled);
	BOX2D_PROPERTY(WheelJointB2, motor_speed, Variant::REAL);
	BOX2D_PROPERTY(WheelJointB2, max_motor_torque, Variant::REAL);
	ClassDB::bind_method(D_METHOD("get_motor_torque", "inv_dt"), &WheelJointB2::get_motor_torque);

	BOX2D_PROPERTY(WheelJointB2, spring_frequency, Variant::REAL);
	BOX2D_PROPERTY(WheelJointB2, spring_damping, Variant::REAL);
}

JointB2 *WheelJointDefB2::instance(WorldB2 *world) {
	ERR_FAIL_NULL_V(world, NULL);
	auto o = world->get_b2()->CreateJoint(def);
	return memnew(WheelJointB2(o));
}

WheelJointDefB2::WheelJointDefB2() :
		JointDefB2(memnew(b2WheelJointDef)) {}

Vector2 WheelJointDefB2::get_anchor_a() const { return GD(DEF->localAnchorA); }
void WheelJointDefB2::set_anchor_a(const Vector2 &o) { DEF->localAnchorA = B2(o); }

Vector2 WheelJointDefB2::get_anchor_b() const { return GD(DEF->localAnchorB); }
void WheelJointDefB2::set_anchor_b(const Vector2 &o) { DEF->localAnchorB = B2(o); }

Vector2 WheelJointDefB2::get_local_axis_a() const { return GD(DEF->localAxisA); }
void WheelJointDefB2::set_local_axis_a(const Vector2 &o) { DEF->localAxisA = B2(o); }

bool WheelJointDefB2::get_enable_motor() const { return DEF->enableMotor; }
void WheelJointDefB2::set_enable_motor(bool o) { DEF->enableMotor = o; }

float WheelJointDefB2::get_max_motor_torque() const { return DEF->maxMotorTorque; }
void WheelJointDefB2::set_max_motor_torque(float o) { DEF->maxMotorTorque = o; }

float WheelJointDefB2::get_motor_speed() const { return DEF->motorSpeed; }
void WheelJointDefB2::set_motor_speed(float o) { DEF->motorSpeed = o; }

float WheelJointDefB2::get_frequency() const { return DEF->frequencyHz; }
void WheelJointDefB2::set_frequency(float o) { DEF->frequencyHz = o; }

float WheelJointDefB2::get_damping() const { return DEF->dampingRatio; }
void WheelJointDefB2::set_damping(float o) { DEF->dampingRatio = o; }

void WheelJointDefB2::_bind_methods() {
	BOX2D_PROPERTY(WheelJointDefB2, anchor_a, Variant::VECTOR2);
	BOX2D_PROPERTY(WheelJointDefB2, anchor_b, Variant::VECTOR2);
	BOX2D_PROPERTY(WheelJointDefB2, local_axis_a, Variant::VECTOR2);
	BOX2D_PROPERTY(WheelJointDefB2, enable_motor, Variant::BOOL);
	BOX2D_PROPERTY(WheelJointDefB2, max_motor_torque, Variant::REAL);
	BOX2D_PROPERTY(WheelJointDefB2, motor_speed, Variant::REAL);
	BOX2D_PROPERTY(WheelJointDefB2, frequency, Variant::REAL);
	BOX2D_PROPERTY(WheelJointDefB2, damping, Variant::REAL);
}

#ifdef DOCTEST
#include "doctest/doctest.h"

static WorldB2 *make_wheel_world() {
	return memnew(WorldB2(memnew(b2World(b2Vec2(0.0f, -10.0f)))));
}

static BodyB2 *make_wheel_body(WorldB2 *w, float x = 0.0f, float y = 0.0f, bool dynamic = true) {
	b2BodyDef bdef;
	bdef.type = dynamic ? b2_dynamicBody : b2_staticBody;
	bdef.position.Set(x, y);
	return memnew(BodyB2(w->get_b2()->CreateBody(&bdef)));
}

TEST_SUITE("[liquidfun] WheelJointDefB2") {
	TEST_CASE("[liquidfun] WheelJointDefB2 defaults match b2WheelJointDef") {
		WheelJointDefB2 *def = memnew(WheelJointDefB2);
		CHECK(def->get_anchor_a() == Vector2(0.0f, 0.0f));
		CHECK(def->get_anchor_b() == Vector2(0.0f, 0.0f));
		CHECK(def->get_local_axis_a() == Vector2(1.0f, 0.0f));
		CHECK(def->get_enable_motor() == false);
		CHECK(def->get_max_motor_torque() == doctest::Approx(0.0f));
		CHECK(def->get_motor_speed() == doctest::Approx(0.0f));
		CHECK(def->get_frequency() == doctest::Approx(2.0f));
		CHECK(def->get_damping() == doctest::Approx(0.7f));
		memdelete(def);
	}

	TEST_CASE("[liquidfun] WheelJointDefB2 setters roundtrip") {
		WheelJointDefB2 *def = memnew(WheelJointDefB2);
		def->set_anchor_a(Vector2(0.0f, -0.5f));
		def->set_anchor_b(Vector2(0.0f, 0.5f));
		def->set_local_axis_a(Vector2(0.0f, 1.0f));
		def->set_enable_motor(true);
		def->set_max_motor_torque(50.0f);
		def->set_motor_speed(3.0f);
		def->set_frequency(4.0f);
		def->set_damping(0.5f);
		CHECK(def->get_anchor_a() == Vector2(0.0f, -0.5f));
		CHECK(def->get_anchor_b() == Vector2(0.0f, 0.5f));
		CHECK(def->get_local_axis_a() == Vector2(0.0f, 1.0f));
		CHECK(def->get_enable_motor() == true);
		CHECK(def->get_max_motor_torque() == doctest::Approx(50.0f));
		CHECK(def->get_motor_speed() == doctest::Approx(3.0f));
		CHECK(def->get_frequency() == doctest::Approx(4.0f));
		CHECK(def->get_damping() == doctest::Approx(0.5f));
		memdelete(def);
	}
} // TEST_SUITE("[liquidfun] WheelJointDefB2")

TEST_SUITE("[liquidfun] WheelJointB2") {
	TEST_CASE("[liquidfun] WheelJointB2 spring and motor properties") {
		WorldB2 *w = make_wheel_world();
		BodyB2 *chassis = make_wheel_body(w, 0.0f, 0.0f, false);
		BodyB2 *wheel = make_wheel_body(w, 0.0f, -1.0f);

		b2CircleShape cs;
		cs.m_radius = 0.4f;
		memnew(FixtureB2(wheel->get_b2()->CreateFixture(&cs, 1.0f)));

		WheelJointDefB2 *def = memnew(WheelJointDefB2);
		def->set_body_a(chassis);
		def->set_body_b(wheel);
		def->set_local_axis_a(Vector2(0.0f, 1.0f));
		def->set_frequency(4.0f);
		def->set_damping(0.8f);
		def->set_enable_motor(true);
		def->set_motor_speed(10.0f);
		def->set_max_motor_torque(20.0f);

		JointB2 *joint = def->instance(w);
		memdelete(def);

		CHECK(joint != nullptr);
		CHECK(joint->is_active() == true);

		auto *wj = Object::cast_to<WheelJointB2>(joint);
		CHECK(wj != nullptr);
		CHECK(wj->is_motor_enabled() == true);
		CHECK(wj->get_motor_speed() == doctest::Approx(10.0f));
		CHECK(wj->get_max_motor_torque() == doctest::Approx(20.0f));
		CHECK(wj->get_spring_frequency() == doctest::Approx(4.0f));
		CHECK(wj->get_spring_damping() == doctest::Approx(0.8f));

		wj->set_motor_enabled(false);
		CHECK(wj->is_motor_enabled() == false);
		wj->set_spring_frequency(6.0f);
		CHECK(wj->get_spring_frequency() == doctest::Approx(6.0f));

		memdelete(w);
	}
} // TEST_SUITE("[liquidfun] WheelJointB2")

#endif // DOCTEST
