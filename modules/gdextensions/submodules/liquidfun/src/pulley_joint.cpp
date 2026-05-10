/**************************************************************************/
/*  pulley_joint.cpp                                                      */
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

#include "pulley_joint.h"

#include <Box2D/Box2D.h>

#define JOINT ((b2PulleyJoint *)entity)
#define DEF ((b2PulleyJointDef *)def)

Vector2 PulleyJointB2::get_ground_anchor_a() const { return GD(JOINT->GetGroundAnchorA()); }
Vector2 PulleyJointB2::get_ground_anchor_b() const { return GD(JOINT->GetGroundAnchorB()); }

float PulleyJointB2::get_length_a() const { return JOINT->GetLengthA(); }
float PulleyJointB2::get_length_b() const { return JOINT->GetLengthB(); }
float PulleyJointB2::get_current_length_a() const { return JOINT->GetCurrentLengthA(); }
float PulleyJointB2::get_current_length_b() const { return JOINT->GetCurrentLengthB(); }
float PulleyJointB2::get_ratio() const { return JOINT->GetRatio(); }

void PulleyJointB2::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_ground_anchor_a"), &PulleyJointB2::get_ground_anchor_a);
	ClassDB::bind_method(D_METHOD("get_ground_anchor_b"), &PulleyJointB2::get_ground_anchor_b);
	ClassDB::bind_method(D_METHOD("get_length_a"), &PulleyJointB2::get_length_a);
	ClassDB::bind_method(D_METHOD("get_length_b"), &PulleyJointB2::get_length_b);
	ClassDB::bind_method(D_METHOD("get_current_length_a"), &PulleyJointB2::get_current_length_a);
	ClassDB::bind_method(D_METHOD("get_current_length_b"), &PulleyJointB2::get_current_length_b);
	ClassDB::bind_method(D_METHOD("get_ratio"), &PulleyJointB2::get_ratio);
}

JointB2 *PulleyJointDefB2::instance(WorldB2 *world) {
	ERR_FAIL_NULL_V(world, NULL);
	auto o = world->get_b2()->CreateJoint(def);
	return memnew(PulleyJointB2(o));
}

PulleyJointDefB2::PulleyJointDefB2() :
		JointDefB2(memnew(b2PulleyJointDef)) {}

Vector2 PulleyJointDefB2::get_ground_anchor_a() const { return GD(DEF->groundAnchorA); }
void PulleyJointDefB2::set_ground_anchor_a(const Vector2 &o) { DEF->groundAnchorA = B2(o); }

Vector2 PulleyJointDefB2::get_ground_anchor_b() const { return GD(DEF->groundAnchorB); }
void PulleyJointDefB2::set_ground_anchor_b(const Vector2 &o) { DEF->groundAnchorB = B2(o); }

Vector2 PulleyJointDefB2::get_anchor_a() const { return GD(DEF->localAnchorA); }
void PulleyJointDefB2::set_anchor_a(const Vector2 &o) { DEF->localAnchorA = B2(o); }

Vector2 PulleyJointDefB2::get_anchor_b() const { return GD(DEF->localAnchorB); }
void PulleyJointDefB2::set_anchor_b(const Vector2 &o) { DEF->localAnchorB = B2(o); }

float PulleyJointDefB2::get_length_a() const { return DEF->lengthA; }
void PulleyJointDefB2::set_length_a(float o) { DEF->lengthA = o; }

float PulleyJointDefB2::get_length_b() const { return DEF->lengthB; }
void PulleyJointDefB2::set_length_b(float o) { DEF->lengthB = o; }

float PulleyJointDefB2::get_ratio() const { return DEF->ratio; }
void PulleyJointDefB2::set_ratio(float o) { DEF->ratio = o; }

void PulleyJointDefB2::_bind_methods() {
	BOX2D_PROPERTY(PulleyJointDefB2, ground_anchor_a, Variant::VECTOR2);
	BOX2D_PROPERTY(PulleyJointDefB2, ground_anchor_b, Variant::VECTOR2);
	BOX2D_PROPERTY(PulleyJointDefB2, anchor_a, Variant::VECTOR2);
	BOX2D_PROPERTY(PulleyJointDefB2, anchor_b, Variant::VECTOR2);
	BOX2D_PROPERTY(PulleyJointDefB2, length_a, Variant::REAL);
	BOX2D_PROPERTY(PulleyJointDefB2, length_b, Variant::REAL);
	BOX2D_PROPERTY(PulleyJointDefB2, ratio, Variant::REAL);
}

#ifdef DOCTEST
#include "doctest/doctest.h"

static WorldB2 *make_pulley_world() {
	return memnew(WorldB2(memnew(b2World(b2Vec2(0.0f, -10.0f)))));
}

static BodyB2 *make_pulley_body(WorldB2 *w, float x, float y, bool dynamic = true) {
	b2BodyDef bdef;
	bdef.type = dynamic ? b2_dynamicBody : b2_staticBody;
	bdef.position.Set(x, y);
	return memnew(BodyB2(w->get_b2()->CreateBody(&bdef)));
}

TEST_SUITE("[liquidfun] PulleyJointDefB2") {
	TEST_CASE("[liquidfun] PulleyJointDefB2 defaults match b2PulleyJointDef") {
		PulleyJointDefB2 *def = memnew(PulleyJointDefB2);
		CHECK(def->get_ground_anchor_a() == Vector2(-1.0f, 1.0f));
		CHECK(def->get_ground_anchor_b() == Vector2(1.0f, 1.0f));
		CHECK(def->get_anchor_a() == Vector2(-1.0f, 0.0f));
		CHECK(def->get_anchor_b() == Vector2(1.0f, 0.0f));
		CHECK(def->get_length_a() == doctest::Approx(0.0f));
		CHECK(def->get_length_b() == doctest::Approx(0.0f));
		CHECK(def->get_ratio() == doctest::Approx(1.0f));
		memdelete(def);
	}

	TEST_CASE("[liquidfun] PulleyJointDefB2 setters roundtrip") {
		PulleyJointDefB2 *def = memnew(PulleyJointDefB2);
		def->set_ground_anchor_a(Vector2(-2.0f, 3.0f));
		def->set_ground_anchor_b(Vector2(2.0f, 3.0f));
		def->set_anchor_a(Vector2(-1.0f, -1.0f));
		def->set_anchor_b(Vector2(1.0f, -1.0f));
		def->set_length_a(2.0f);
		def->set_length_b(2.0f);
		def->set_ratio(2.0f);
		CHECK(def->get_ground_anchor_a() == Vector2(-2.0f, 3.0f));
		CHECK(def->get_ground_anchor_b() == Vector2(2.0f, 3.0f));
		CHECK(def->get_anchor_a() == Vector2(-1.0f, -1.0f));
		CHECK(def->get_anchor_b() == Vector2(1.0f, -1.0f));
		CHECK(def->get_length_a() == doctest::Approx(2.0f));
		CHECK(def->get_length_b() == doctest::Approx(2.0f));
		CHECK(def->get_ratio() == doctest::Approx(2.0f));
		memdelete(def);
	}
} // TEST_SUITE("[liquidfun] PulleyJointDefB2")

TEST_SUITE("[liquidfun] PulleyJointB2") {
	TEST_CASE("[liquidfun] PulleyJointB2 ratio and lengths after creation") {
		WorldB2 *w = make_pulley_world();
		BodyB2 *body_a = make_pulley_body(w, -1.0f, 0.0f);
		BodyB2 *body_b = make_pulley_body(w, 1.0f, 0.0f);

		b2CircleShape cs;
		cs.m_radius = 0.1f;
		memnew(FixtureB2(body_a->get_b2()->CreateFixture(&cs, 1.0f)));
		memnew(FixtureB2(body_b->get_b2()->CreateFixture(&cs, 1.0f)));

		PulleyJointDefB2 *def = memnew(PulleyJointDefB2);
		def->set_body_a(body_a);
		def->set_body_b(body_b);
		def->set_ground_anchor_a(Vector2(-1.0f, 2.0f));
		def->set_ground_anchor_b(Vector2(1.0f, 2.0f));
		def->set_anchor_a(Vector2(0.0f, 0.0f));
		def->set_anchor_b(Vector2(0.0f, 0.0f));
		def->set_length_a(2.0f);
		def->set_length_b(2.0f);
		def->set_ratio(1.5f);

		JointB2 *joint = def->instance(w);
		memdelete(def);

		CHECK(joint != nullptr);
		CHECK(joint->is_active() == true);

		auto *pj = Object::cast_to<PulleyJointB2>(joint);
		CHECK(pj != nullptr);
		CHECK(pj->get_ratio() == doctest::Approx(1.5f));
		CHECK(pj->get_length_a() == doctest::Approx(2.0f));
		CHECK(pj->get_length_b() == doctest::Approx(2.0f));
		CHECK(pj->get_current_length_a() >= 0.0f);
		CHECK(pj->get_current_length_b() >= 0.0f);

		memdelete(w);
	}
} // TEST_SUITE("[liquidfun] PulleyJointB2")

#endif // DOCTEST
