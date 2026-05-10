/**************************************************************************/
/*  friction_joint.cpp                                                    */
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

#include "friction_joint.h"

#include <Box2D/Box2D.h>

#define JOINT ((b2FrictionJoint *)entity)
#define DEF ((b2FrictionJointDef *)def)

float FrictionJointB2::get_max_force() const { return JOINT->GetMaxForce(); }
void FrictionJointB2::set_max_force(float rhs) { JOINT->SetMaxForce(rhs); }

float FrictionJointB2::get_max_torque() const { return JOINT->GetMaxTorque(); }
void FrictionJointB2::set_max_torque(float rhs) { JOINT->SetMaxTorque(rhs); }

void FrictionJointB2::_bind_methods() {
	BOX2D_PROPERTY(FrictionJointB2, max_force, Variant::REAL);
	BOX2D_PROPERTY(FrictionJointB2, max_torque, Variant::REAL);
}

JointB2 *FrictionJointDefB2::instance(WorldB2 *world) {
	ERR_FAIL_NULL_V(world, NULL);
	auto o = world->get_b2()->CreateJoint(def);
	return memnew(FrictionJointB2(o));
}

FrictionJointDefB2::FrictionJointDefB2() :
		JointDefB2(memnew(b2FrictionJointDef)) {}

Vector2 FrictionJointDefB2::get_anchor_a() const { return GD(DEF->localAnchorA); }
void FrictionJointDefB2::set_anchor_a(const Vector2 &o) { DEF->localAnchorA = B2(o); }

Vector2 FrictionJointDefB2::get_anchor_b() const { return GD(DEF->localAnchorB); }
void FrictionJointDefB2::set_anchor_b(const Vector2 &o) { DEF->localAnchorB = B2(o); }

float FrictionJointDefB2::get_max_force() const { return DEF->maxForce; }
void FrictionJointDefB2::set_max_force(float o) { DEF->maxForce = o; }

float FrictionJointDefB2::get_max_torque() const { return DEF->maxTorque; }
void FrictionJointDefB2::set_max_torque(float o) { DEF->maxTorque = o; }

void FrictionJointDefB2::_bind_methods() {
	BOX2D_PROPERTY(FrictionJointDefB2, anchor_a, Variant::VECTOR2);
	BOX2D_PROPERTY(FrictionJointDefB2, anchor_b, Variant::VECTOR2);
	BOX2D_PROPERTY(FrictionJointDefB2, max_force, Variant::REAL);
	BOX2D_PROPERTY(FrictionJointDefB2, max_torque, Variant::REAL);
}

#ifdef DOCTEST
#include "doctest/doctest.h"

static WorldB2 *make_friction_world() {
	return memnew(WorldB2(memnew(b2World(b2Vec2(0.0f, 0.0f)))));
}

static BodyB2 *make_friction_body(WorldB2 *w, float x = 0.0f, float y = 0.0f) {
	b2BodyDef bdef;
	bdef.position.Set(x, y);
	return memnew(BodyB2(w->get_b2()->CreateBody(&bdef)));
}

TEST_SUITE("[liquidfun] FrictionJointDefB2") {
	TEST_CASE("[liquidfun] FrictionJointDefB2 defaults match b2FrictionJointDef") {
		FrictionJointDefB2 *def = memnew(FrictionJointDefB2);
		CHECK(def->get_anchor_a() == Vector2(0.0f, 0.0f));
		CHECK(def->get_anchor_b() == Vector2(0.0f, 0.0f));
		CHECK(def->get_max_force() == doctest::Approx(0.0f));
		CHECK(def->get_max_torque() == doctest::Approx(0.0f));
		memdelete(def);
	}

	TEST_CASE("[liquidfun] FrictionJointDefB2 setters roundtrip") {
		FrictionJointDefB2 *def = memnew(FrictionJointDefB2);
		def->set_anchor_a(Vector2(0.5f, 0.0f));
		def->set_anchor_b(Vector2(-0.5f, 0.0f));
		def->set_max_force(50.0f);
		def->set_max_torque(10.0f);
		CHECK(def->get_anchor_a() == Vector2(0.5f, 0.0f));
		CHECK(def->get_anchor_b() == Vector2(-0.5f, 0.0f));
		CHECK(def->get_max_force() == doctest::Approx(50.0f));
		CHECK(def->get_max_torque() == doctest::Approx(10.0f));
		memdelete(def);
	}
} // TEST_SUITE("[liquidfun] FrictionJointDefB2")

TEST_SUITE("[liquidfun] FrictionJointB2") {
	TEST_CASE("[liquidfun] FrictionJointB2 max_force and max_torque live get/set") {
		WorldB2 *w = make_friction_world();
		BodyB2 *body_a = make_friction_body(w, 0.0f, 0.0f);
		BodyB2 *body_b = make_friction_body(w, 1.0f, 0.0f);

		FrictionJointDefB2 *def = memnew(FrictionJointDefB2);
		def->set_body_a(body_a);
		def->set_body_b(body_b);
		def->set_max_force(20.0f);
		def->set_max_torque(5.0f);

		JointB2 *joint = def->instance(w);
		memdelete(def);

		CHECK(joint != nullptr);
		CHECK(joint->is_active() == true);

		auto *fj = Object::cast_to<FrictionJointB2>(joint);
		CHECK(fj != nullptr);
		CHECK(fj->get_max_force() == doctest::Approx(20.0f));
		CHECK(fj->get_max_torque() == doctest::Approx(5.0f));

		fj->set_max_force(100.0f);
		fj->set_max_torque(25.0f);
		CHECK(fj->get_max_force() == doctest::Approx(100.0f));
		CHECK(fj->get_max_torque() == doctest::Approx(25.0f));

		memdelete(w);
	}
} // TEST_SUITE("[liquidfun] FrictionJointB2")

#endif // DOCTEST
