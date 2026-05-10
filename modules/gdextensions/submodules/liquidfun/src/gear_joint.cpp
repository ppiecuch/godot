
#include "godot_box2d.h"

#include "gear_joint.h"

#include <Box2D/Box2D.h>

#define JOINT ((b2GearJoint *)entity)
#define DEF ((b2GearJointDef *)def)

JointB2 *GearJointB2::get_joint1() const { return JointB2::get(JOINT->GetJoint1()); }
JointB2 *GearJointB2::get_joint2() const { return JointB2::get(JOINT->GetJoint2()); }

float GearJointB2::get_ratio() const { return JOINT->GetRatio(); }
void GearJointB2::set_ratio(float rhs) { JOINT->SetRatio(rhs); }

void GearJointB2::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_joint1"), &GearJointB2::get_joint1);
	ClassDB::bind_method(D_METHOD("get_joint2"), &GearJointB2::get_joint2);
	BOX2D_PROPERTY(GearJointB2, ratio, Variant::REAL);
}

JointB2 *GearJointDefB2::instance(WorldB2 *world) {
	ERR_FAIL_NULL_V(world, NULL);
	auto o = world->get_b2()->CreateJoint(def);
	return memnew(GearJointB2(o));
}

GearJointDefB2::GearJointDefB2() :
		JointDefB2(memnew(b2GearJointDef)) {}

void GearJointDefB2::set_joint1(JointB2 *joint) {
	joint1_wrapper = joint;
	DEF->joint1 = joint ? joint->get_b2() : nullptr;
}

JointB2 *GearJointDefB2::get_joint1() const {
	return joint1_wrapper;
}

void GearJointDefB2::set_joint2(JointB2 *joint) {
	joint2_wrapper = joint;
	DEF->joint2 = joint ? joint->get_b2() : nullptr;
}

JointB2 *GearJointDefB2::get_joint2() const {
	return joint2_wrapper;
}

float GearJointDefB2::get_ratio() const { return DEF->ratio; }
void GearJointDefB2::set_ratio(float o) { DEF->ratio = o; }

void GearJointDefB2::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_joint1"), &GearJointDefB2::get_joint1);
	ClassDB::bind_method(D_METHOD("set_joint1", "joint"), &GearJointDefB2::set_joint1);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "joint1"), "set_joint1", "get_joint1");

	ClassDB::bind_method(D_METHOD("get_joint2"), &GearJointDefB2::get_joint2);
	ClassDB::bind_method(D_METHOD("set_joint2", "joint"), &GearJointDefB2::set_joint2);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "joint2"), "set_joint2", "get_joint2");

	BOX2D_PROPERTY(GearJointDefB2, ratio, Variant::REAL);
}

#ifdef DOCTEST
#include "doctest/doctest.h"

static WorldB2 *make_gear_world() {
	return memnew(WorldB2(memnew(b2World(b2Vec2(0.0f, 0.0f)))));
}

static BodyB2 *make_gear_body(WorldB2 *w, float x = 0.0f, float y = 0.0f, bool dynamic = true) {
	b2BodyDef bdef;
	bdef.type = dynamic ? b2_dynamicBody : b2_staticBody;
	bdef.position.Set(x, y);
	return memnew(BodyB2(w->get_b2()->CreateBody(&bdef)));
}

TEST_SUITE("[liquidfun] GearJointDefB2") {
	TEST_CASE("[liquidfun] GearJointDefB2 defaults") {
		GearJointDefB2 *def = memnew(GearJointDefB2);
		CHECK(def->get_joint1() == nullptr);
		CHECK(def->get_joint2() == nullptr);
		CHECK(def->get_ratio() == doctest::Approx(1.0f));
		memdelete(def);
	}

	TEST_CASE("[liquidfun] GearJointDefB2 ratio roundtrip") {
		GearJointDefB2 *def = memnew(GearJointDefB2);
		def->set_ratio(3.0f);
		CHECK(def->get_ratio() == doctest::Approx(3.0f));
		memdelete(def);
	}
} // TEST_SUITE("[liquidfun] GearJointDefB2")

TEST_SUITE("[liquidfun] GearJointB2") {
	TEST_CASE("[liquidfun] GearJointB2 links two revolute joints") {
		WorldB2 *w = make_gear_world();
		BodyB2 *ground = make_gear_body(w, 0.0f, 0.0f, false);
		BodyB2 *body_a = make_gear_body(w, -2.0f, 0.0f);
		BodyB2 *body_b = make_gear_body(w, 2.0f, 0.0f);

		b2CircleShape cs;
		cs.m_radius = 0.5f;
		memnew(FixtureB2(body_a->get_b2()->CreateFixture(&cs, 1.0f)));
		memnew(FixtureB2(body_b->get_b2()->CreateFixture(&cs, 1.0f)));

		RevoluteJointDefB2 *rdef1 = memnew(RevoluteJointDefB2);
		rdef1->set_body_a(ground);
		rdef1->set_body_b(body_a);
		JointB2 *joint1 = rdef1->instance(w);
		memdelete(rdef1);

		RevoluteJointDefB2 *rdef2 = memnew(RevoluteJointDefB2);
		rdef2->set_body_a(ground);
		rdef2->set_body_b(body_b);
		JointB2 *joint2 = rdef2->instance(w);
		memdelete(rdef2);

		GearJointDefB2 *gdef = memnew(GearJointDefB2);
		gdef->set_body_a(body_a);
		gdef->set_body_b(body_b);
		gdef->set_joint1(joint1);
		gdef->set_joint2(joint2);
		gdef->set_ratio(2.0f);

		CHECK(gdef->get_joint1() == joint1);
		CHECK(gdef->get_joint2() == joint2);

		JointB2 *gear = gdef->instance(w);
		memdelete(gdef);

		CHECK(gear != nullptr);
		CHECK(gear->is_active() == true);

		auto *gj = Object::cast_to<GearJointB2>(gear);
		CHECK(gj != nullptr);
		CHECK(gj->get_ratio() == doctest::Approx(2.0f));
		CHECK(gj->get_joint1() == joint1);
		CHECK(gj->get_joint2() == joint2);

		gj->set_ratio(0.5f);
		CHECK(gj->get_ratio() == doctest::Approx(0.5f));

		memdelete(w);
	}
} // TEST_SUITE("[liquidfun] GearJointB2")

#endif // DOCTEST
