#include "godot_box2d.h"

#include "motor_joint.h"

#include <Box2D/Box2D.h>

#define JOINT ((b2MotorJoint *)entity)
#define DEF ((b2MotorJointDef *)def)

Vector2 MotorJointB2::get_linear_offset() const { return GD(JOINT->GetLinearOffset()); }
void MotorJointB2::set_linear_offset(const Vector2 &rhs) { JOINT->SetLinearOffset(B2(rhs)); }

float MotorJointB2::get_angular_offset() const { return JOINT->GetAngularOffset(); }
void MotorJointB2::set_angular_offset(float rhs) { JOINT->SetAngularOffset(rhs); }

float MotorJointB2::get_max_force() const { return JOINT->GetMaxForce(); }
void MotorJointB2::set_max_force(float rhs) { JOINT->SetMaxForce(rhs); }

float MotorJointB2::get_max_torque() const { return JOINT->GetMaxTorque(); }
void MotorJointB2::set_max_torque(float rhs) { JOINT->SetMaxTorque(rhs); }

float MotorJointB2::get_correction_factor() const { return JOINT->GetCorrectionFactor(); }
void MotorJointB2::set_correction_factor(float rhs) { JOINT->SetCorrectionFactor(rhs); }

void MotorJointB2::_bind_methods() {
	BOX2D_PROPERTY(MotorJointB2, linear_offset, Variant::VECTOR2);
	BOX2D_PROPERTY(MotorJointB2, angular_offset, Variant::REAL);
	BOX2D_PROPERTY(MotorJointB2, max_force, Variant::REAL);
	BOX2D_PROPERTY(MotorJointB2, max_torque, Variant::REAL);
	BOX2D_PROPERTY(MotorJointB2, correction_factor, Variant::REAL);
}

JointB2 *MotorJointDefB2::instance(WorldB2 *world) {
	ERR_FAIL_NULL_V(world, NULL);
	auto o = world->get_b2()->CreateJoint(def);
	return memnew(MotorJointB2(o));
}

MotorJointDefB2::MotorJointDefB2() :
		JointDefB2(memnew(b2MotorJointDef)) {}

Vector2 MotorJointDefB2::get_linear_offset() const { return GD(DEF->linearOffset); }
void MotorJointDefB2::set_linear_offset(const Vector2 &o) { DEF->linearOffset = B2(o); }

float MotorJointDefB2::get_angular_offset() const { return DEF->angularOffset; }
void MotorJointDefB2::set_angular_offset(float o) { DEF->angularOffset = o; }

float MotorJointDefB2::get_max_force() const { return DEF->maxForce; }
void MotorJointDefB2::set_max_force(float o) { DEF->maxForce = o; }

float MotorJointDefB2::get_max_torque() const { return DEF->maxTorque; }
void MotorJointDefB2::set_max_torque(float o) { DEF->maxTorque = o; }

float MotorJointDefB2::get_correction_factor() const { return DEF->correctionFactor; }
void MotorJointDefB2::set_correction_factor(float o) { DEF->correctionFactor = o; }

void MotorJointDefB2::_bind_methods() {
	BOX2D_PROPERTY(MotorJointDefB2, linear_offset, Variant::VECTOR2);
	BOX2D_PROPERTY(MotorJointDefB2, angular_offset, Variant::REAL);
	BOX2D_PROPERTY(MotorJointDefB2, max_force, Variant::REAL);
	BOX2D_PROPERTY(MotorJointDefB2, max_torque, Variant::REAL);
	BOX2D_PROPERTY(MotorJointDefB2, correction_factor, Variant::REAL);
}

#ifdef DOCTEST
#include "doctest/doctest.h"

static WorldB2 *make_motor_world() {
	return memnew(WorldB2(memnew(b2World(b2Vec2(0.0f, 0.0f)))));
}

static BodyB2 *make_motor_body(WorldB2 *w, float x = 0.0f, float y = 0.0f) {
	b2BodyDef bdef;
	bdef.type = b2_dynamicBody;
	bdef.position.Set(x, y);
	return memnew(BodyB2(w->get_b2()->CreateBody(&bdef)));
}

TEST_SUITE("[liquidfun] MotorJointDefB2") {
	TEST_CASE("[liquidfun] MotorJointDefB2 defaults match b2MotorJointDef") {
		MotorJointDefB2 *def = memnew(MotorJointDefB2);
		CHECK(def->get_linear_offset() == Vector2(0.0f, 0.0f));
		CHECK(def->get_angular_offset() == doctest::Approx(0.0f));
		CHECK(def->get_max_force() == doctest::Approx(1.0f));
		CHECK(def->get_max_torque() == doctest::Approx(1.0f));
		CHECK(def->get_correction_factor() == doctest::Approx(0.3f));
		memdelete(def);
	}

	TEST_CASE("[liquidfun] MotorJointDefB2 setters roundtrip") {
		MotorJointDefB2 *def = memnew(MotorJointDefB2);
		def->set_linear_offset(Vector2(2.0f, 1.0f));
		def->set_angular_offset(0.5f);
		def->set_max_force(100.0f);
		def->set_max_torque(50.0f);
		def->set_correction_factor(0.8f);
		CHECK(def->get_linear_offset() == Vector2(2.0f, 1.0f));
		CHECK(def->get_angular_offset() == doctest::Approx(0.5f));
		CHECK(def->get_max_force() == doctest::Approx(100.0f));
		CHECK(def->get_max_torque() == doctest::Approx(50.0f));
		CHECK(def->get_correction_factor() == doctest::Approx(0.8f));
		memdelete(def);
	}
} // TEST_SUITE("[liquidfun] MotorJointDefB2")

TEST_SUITE("[liquidfun] MotorJointB2") {
	TEST_CASE("[liquidfun] MotorJointB2 all live properties get/set") {
		WorldB2 *w = make_motor_world();
		BodyB2 *body_a = make_motor_body(w, 0.0f, 0.0f);
		BodyB2 *body_b = make_motor_body(w, 1.0f, 0.0f);

		b2CircleShape cs;
		cs.m_radius = 0.3f;
		memnew(FixtureB2(body_a->get_b2()->CreateFixture(&cs, 1.0f)));
		memnew(FixtureB2(body_b->get_b2()->CreateFixture(&cs, 1.0f)));

		MotorJointDefB2 *def = memnew(MotorJointDefB2);
		def->set_body_a(body_a);
		def->set_body_b(body_b);
		def->set_linear_offset(Vector2(1.0f, 0.0f));
		def->set_angular_offset(0.2f);
		def->set_max_force(200.0f);
		def->set_max_torque(100.0f);
		def->set_correction_factor(0.5f);

		JointB2 *joint = def->instance(w);
		memdelete(def);

		CHECK(joint != nullptr);
		CHECK(joint->is_active() == true);

		auto *mj = Object::cast_to<MotorJointB2>(joint);
		CHECK(mj != nullptr);
		CHECK(mj->get_linear_offset() == Vector2(1.0f, 0.0f));
		CHECK(mj->get_angular_offset() == doctest::Approx(0.2f));
		CHECK(mj->get_max_force() == doctest::Approx(200.0f));
		CHECK(mj->get_max_torque() == doctest::Approx(100.0f));
		CHECK(mj->get_correction_factor() == doctest::Approx(0.5f));

		mj->set_linear_offset(Vector2(2.0f, 1.0f));
		mj->set_angular_offset(0.8f);
		mj->set_max_force(300.0f);
		mj->set_max_torque(150.0f);
		mj->set_correction_factor(0.9f);
		CHECK(mj->get_linear_offset() == Vector2(2.0f, 1.0f));
		CHECK(mj->get_angular_offset() == doctest::Approx(0.8f));
		CHECK(mj->get_max_force() == doctest::Approx(300.0f));
		CHECK(mj->get_max_torque() == doctest::Approx(150.0f));
		CHECK(mj->get_correction_factor() == doctest::Approx(0.9f));

		memdelete(w);
	}
} // TEST_SUITE("[liquidfun] MotorJointB2")

#endif // DOCTEST
