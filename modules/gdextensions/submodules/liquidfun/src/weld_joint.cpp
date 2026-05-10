#include "godot_box2d.h"

#include "weld_joint.h"

#include <Box2D/Box2D.h>

#define JOINT ((b2WeldJoint *)entity)
#define DEF ((b2WeldJointDef *)def)

float WeldJointB2::get_reference_angle() const { return JOINT->GetReferenceAngle(); }

float WeldJointB2::get_frequency() const { return JOINT->GetFrequency(); }
void WeldJointB2::set_frequency(float rhs) { JOINT->SetFrequency(rhs); }

float WeldJointB2::get_damping() const { return JOINT->GetDampingRatio(); }
void WeldJointB2::set_damping(float rhs) { JOINT->SetDampingRatio(rhs); }

void WeldJointB2::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_reference_angle"), &WeldJointB2::get_reference_angle);
	BOX2D_PROPERTY(WeldJointB2, frequency, Variant::REAL);
	BOX2D_PROPERTY(WeldJointB2, damping, Variant::REAL);
}

JointB2 *WeldJointDefB2::instance(WorldB2 *world) {
	ERR_FAIL_NULL_V(world, NULL);
	auto o = world->get_b2()->CreateJoint(def);
	return memnew(WeldJointB2(o));
}

WeldJointDefB2::WeldJointDefB2() :
		JointDefB2(memnew(b2WeldJointDef)) {}

Vector2 WeldJointDefB2::get_anchor_a() const { return GD(DEF->localAnchorA); }
void WeldJointDefB2::set_anchor_a(const Vector2 &o) { DEF->localAnchorA = B2(o); }

Vector2 WeldJointDefB2::get_anchor_b() const { return GD(DEF->localAnchorB); }
void WeldJointDefB2::set_anchor_b(const Vector2 &o) { DEF->localAnchorB = B2(o); }

float WeldJointDefB2::get_reference_angle() const { return DEF->referenceAngle; }
void WeldJointDefB2::set_reference_angle(float o) { DEF->referenceAngle = o; }

float WeldJointDefB2::get_frequency() const { return DEF->frequencyHz; }
void WeldJointDefB2::set_frequency(float o) { DEF->frequencyHz = o; }

float WeldJointDefB2::get_damping() const { return DEF->dampingRatio; }
void WeldJointDefB2::set_damping(float o) { DEF->dampingRatio = o; }

void WeldJointDefB2::_bind_methods() {
	BOX2D_PROPERTY(WeldJointDefB2, anchor_a, Variant::VECTOR2);
	BOX2D_PROPERTY(WeldJointDefB2, anchor_b, Variant::VECTOR2);
	BOX2D_PROPERTY(WeldJointDefB2, reference_angle, Variant::REAL);
	BOX2D_PROPERTY(WeldJointDefB2, frequency, Variant::REAL);
	BOX2D_PROPERTY(WeldJointDefB2, damping, Variant::REAL);
}
