#include "godot_box2d.h"

#include "rope_joint.h"

#include <Box2D/Box2D.h>

#define JOINT ((b2RopeJoint *)entity)
#define DEF ((b2RopeJointDef *)def)

float RopeJointB2::get_max_length() const { return JOINT->GetMaxLength(); }
void RopeJointB2::set_max_length(float rhs) { JOINT->SetMaxLength(rhs); }

int RopeJointB2::get_limit_state() const { return (int)JOINT->GetLimitState(); }

void RopeJointB2::_bind_methods() {
	BOX2D_PROPERTY(RopeJointB2, max_length, Variant::REAL);
	ClassDB::bind_method(D_METHOD("get_limit_state"), &RopeJointB2::get_limit_state);
}

JointB2 *RopeJointDefB2::instance(WorldB2 *world) {
	ERR_FAIL_NULL_V(world, NULL);
	auto o = world->get_b2()->CreateJoint(def);
	return memnew(RopeJointB2(o));
}

RopeJointDefB2::RopeJointDefB2() :
		JointDefB2(memnew(b2RopeJointDef)) {}

Vector2 RopeJointDefB2::get_anchor_a() const { return GD(DEF->localAnchorA); }
void RopeJointDefB2::set_anchor_a(const Vector2 &o) { DEF->localAnchorA = B2(o); }

Vector2 RopeJointDefB2::get_anchor_b() const { return GD(DEF->localAnchorB); }
void RopeJointDefB2::set_anchor_b(const Vector2 &o) { DEF->localAnchorB = B2(o); }

float RopeJointDefB2::get_max_length() const { return DEF->maxLength; }
void RopeJointDefB2::set_max_length(float o) { DEF->maxLength = o; }

void RopeJointDefB2::_bind_methods() {
	BOX2D_PROPERTY(RopeJointDefB2, anchor_a, Variant::VECTOR2);
	BOX2D_PROPERTY(RopeJointDefB2, anchor_b, Variant::VECTOR2);
	BOX2D_PROPERTY(RopeJointDefB2, max_length, Variant::REAL);
}

#ifdef DOCTEST
#include "doctest/doctest.h"

static WorldB2 *make_rope_world() {
	return memnew(WorldB2(memnew(b2World(b2Vec2(0.0f, -10.0f)))));
}

static BodyB2 *make_rope_body(WorldB2 *w, float x = 0.0f, float y = 0.0f, bool dynamic = true) {
	b2BodyDef bdef;
	bdef.type = dynamic ? b2_dynamicBody : b2_staticBody;
	bdef.position.Set(x, y);
	return memnew(BodyB2(w->get_b2()->CreateBody(&bdef)));
}

TEST_SUITE("[liquidfun] RopeJointDefB2") {
	TEST_CASE("[liquidfun] RopeJointDefB2 defaults match b2RopeJointDef") {
		RopeJointDefB2 *def = memnew(RopeJointDefB2);
		CHECK(def->get_anchor_a() == Vector2(-1.0f, 0.0f));
		CHECK(def->get_anchor_b() == Vector2(1.0f, 0.0f));
		CHECK(def->get_max_length() == doctest::Approx(0.0f));
		memdelete(def);
	}

	TEST_CASE("[liquidfun] RopeJointDefB2 setters roundtrip") {
		RopeJointDefB2 *def = memnew(RopeJointDefB2);
		def->set_anchor_a(Vector2(0.0f, 0.5f));
		def->set_anchor_b(Vector2(0.0f, -0.5f));
		def->set_max_length(5.0f);
		CHECK(def->get_anchor_a() == Vector2(0.0f, 0.5f));
		CHECK(def->get_anchor_b() == Vector2(0.0f, -0.5f));
		CHECK(def->get_max_length() == doctest::Approx(5.0f));
		memdelete(def);
	}
} // TEST_SUITE("[liquidfun] RopeJointDefB2")

TEST_SUITE("[liquidfun] RopeJointB2") {
	TEST_CASE("[liquidfun] RopeJointB2 max_length get/set and inactive limit state") {
		WorldB2 *w = make_rope_world();
		BodyB2 *anchor = make_rope_body(w, 0.0f, 0.0f, false);
		BodyB2 *ball = make_rope_body(w, 0.0f, -2.0f);

		b2CircleShape cs;
		cs.m_radius = 0.2f;
		memnew(FixtureB2(ball->get_b2()->CreateFixture(&cs, 1.0f)));

		RopeJointDefB2 *def = memnew(RopeJointDefB2);
		def->set_body_a(anchor);
		def->set_body_b(ball);
		def->set_anchor_a(Vector2(0.0f, 0.0f));
		def->set_anchor_b(Vector2(0.0f, 0.0f));
		def->set_max_length(3.0f);

		JointB2 *joint = def->instance(w);
		memdelete(def);

		CHECK(joint != nullptr);
		CHECK(joint->is_active() == true);

		auto *rj = Object::cast_to<RopeJointB2>(joint);
		CHECK(rj != nullptr);
		CHECK(rj->get_max_length() == doctest::Approx(3.0f));
		// e_inactiveLimit = 0 before step (rope is slack)
		CHECK(rj->get_limit_state() == 0);

		rj->set_max_length(4.0f);
		CHECK(rj->get_max_length() == doctest::Approx(4.0f));

		memdelete(w);
	}
} // TEST_SUITE("[liquidfun] RopeJointB2")

#endif // DOCTEST
