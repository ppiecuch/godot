#include "godot_box2d.h"

#include "mouse_joint.h"

#include <Box2D/Box2D.h>

#define JOINT ((b2MouseJoint *)entity)
#define DEF ((b2MouseJointDef *)def)

Vector2 MouseJointB2::get_target() const {
	return GD(JOINT->GetTarget());
}

void MouseJointB2::set_target(const Vector2 &rhs) {
	return JOINT->SetTarget(B2(rhs));
}

float MouseJointB2::get_max_force() const {
	return JOINT->GetMaxForce();
}

void MouseJointB2::set_max_force(float rhs) {
	JOINT->SetMaxForce(rhs);
}

float MouseJointB2::get_frequency() const {
	return JOINT->GetFrequency();
}

void MouseJointB2::set_frequency(float rhs) {
	JOINT->SetFrequency(rhs);
}

float MouseJointB2::get_damping() const {
	return JOINT->GetDampingRatio();
}

void MouseJointB2::set_damping(float rhs) {
	JOINT->SetDampingRatio(rhs);
}

void MouseJointB2::_bind_methods() {
	BOX2D_PROPERTY(MouseJointB2, target, Variant::VECTOR2);
	BOX2D_PROPERTY(MouseJointB2, max_force, Variant::REAL);
	BOX2D_PROPERTY(MouseJointB2, frequency, Variant::REAL);
	BOX2D_PROPERTY(MouseJointB2, damping, Variant::REAL);
}

JointB2 *MouseJointDefB2::instance(WorldB2 *world) {
	ERR_FAIL_NULL_V(world, NULL);
	auto o = world->get_b2()->CreateJoint(def);
	return memnew(MouseJointB2(o));
}

MouseJointDefB2::MouseJointDefB2() :
		JointDefB2(memnew(b2MouseJointDef)) {
}

Vector2 MouseJointDefB2::get_target() const {
	return GD(DEF->target);
}

void MouseJointDefB2::set_target(const Vector2 &rhs) {
	DEF->target = B2(rhs);
}

float MouseJointDefB2::get_max_force() const {
	return DEF->maxForce;
}

void MouseJointDefB2::set_max_force(float rhs) {
	DEF->maxForce = rhs;
}

float MouseJointDefB2::get_frequency() const {
	return DEF->frequencyHz;
}

void MouseJointDefB2::set_frequency(float rhs) {
	DEF->frequencyHz = rhs;
}

float MouseJointDefB2::get_damping() const {
	return DEF->dampingRatio;
}

void MouseJointDefB2::set_damping(float rhs) {
	DEF->dampingRatio = rhs;
}

void MouseJointDefB2::_bind_methods() {
	BOX2D_PROPERTY(MouseJointDefB2, target, Variant::VECTOR2);
	BOX2D_PROPERTY(MouseJointDefB2, max_force, Variant::REAL);
	BOX2D_PROPERTY(MouseJointDefB2, frequency, Variant::REAL);
	BOX2D_PROPERTY(MouseJointDefB2, damping, Variant::REAL);
}

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_SUITE("[liquidfun] MouseJointDefB2") {
	TEST_CASE("[liquidfun] MouseJointDefB2 defaults match b2MouseJointDef") {
		MouseJointDefB2 *def = memnew(MouseJointDefB2);
		CHECK(def->get_target() == Vector2(0.0f, 0.0f));
		CHECK(def->get_max_force() == doctest::Approx(0.0f));
		CHECK(def->get_frequency() == doctest::Approx(5.0f));
		CHECK(def->get_damping() == doctest::Approx(0.7f));
		memdelete(def);
	}

	TEST_CASE("[liquidfun] MouseJointDefB2 setters roundtrip") {
		MouseJointDefB2 *def = memnew(MouseJointDefB2);
		def->set_target(Vector2(3.0f, 4.0f));
		def->set_max_force(100.0f);
		def->set_frequency(8.0f);
		def->set_damping(0.5f);
		CHECK(def->get_target() == Vector2(3.0f, 4.0f));
		CHECK(def->get_max_force() == doctest::Approx(100.0f));
		CHECK(def->get_frequency() == doctest::Approx(8.0f));
		CHECK(def->get_damping() == doctest::Approx(0.5f));
		memdelete(def);
	}
} // TEST_SUITE("[liquidfun] MouseJointDefB2")

#endif // DOCTEST
