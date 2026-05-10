
#include "distance_joint.h"
#include "godot_box2d.h"

#include <Box2D/Box2D.h>

#define JOINT ((b2DistanceJoint *)entity)
#define DEF ((b2DistanceJointDef *)def)

float DistanceJointB2::get_length() const { return JOINT->GetLength(); }
void DistanceJointB2::set_length(float rhs) { JOINT->SetLength(rhs); }

float DistanceJointB2::get_frequency() const { return JOINT->GetFrequency(); }
void DistanceJointB2::set_frequency(float rhs) { JOINT->SetFrequency(rhs); }

float DistanceJointB2::get_damping() const { return JOINT->GetDampingRatio(); }
void DistanceJointB2::set_damping(float rhs) { JOINT->SetDampingRatio(rhs); }

void DistanceJointB2::_bind_methods() {
	BOX2D_PROPERTY(DistanceJointB2, length, Variant::REAL);
	BOX2D_PROPERTY(DistanceJointB2, frequency, Variant::REAL);
	BOX2D_PROPERTY(DistanceJointB2, damping, Variant::REAL);
}

JointB2 *DistanceJointDefB2::instance(WorldB2 *world) {
	ERR_FAIL_NULL_V(world, NULL);
	auto o = world->get_b2()->CreateJoint(def);
	return memnew(DistanceJointB2(o));
}

DistanceJointDefB2::DistanceJointDefB2() :
		JointDefB2(memnew(b2DistanceJointDef)) {}

Vector2 DistanceJointDefB2::get_anchor_a() const { return GD(DEF->localAnchorA); }
void DistanceJointDefB2::set_anchor_a(const Vector2 &o) { DEF->localAnchorA = B2(o); }

Vector2 DistanceJointDefB2::get_anchor_b() const { return GD(DEF->localAnchorB); }
void DistanceJointDefB2::set_anchor_b(const Vector2 &o) { DEF->localAnchorB = B2(o); }

float DistanceJointDefB2::get_length() const { return DEF->length; }
void DistanceJointDefB2::set_length(float o) { DEF->length = o; }

float DistanceJointDefB2::get_frequency() const { return DEF->frequencyHz; }
void DistanceJointDefB2::set_frequency(float o) { DEF->frequencyHz = o; }

float DistanceJointDefB2::get_damping() const { return DEF->dampingRatio; }
void DistanceJointDefB2::set_damping(float o) { DEF->dampingRatio = o; }

void DistanceJointDefB2::_bind_methods() {
	ClassDB::bind_method(D_METHOD("instance", "world"), &DistanceJointDefB2::instance);

	BOX2D_PROPERTY(DistanceJointDefB2, anchor_a, Variant::VECTOR2);
	BOX2D_PROPERTY(DistanceJointDefB2, anchor_b, Variant::VECTOR2);
	BOX2D_PROPERTY(DistanceJointDefB2, length, Variant::REAL);
	BOX2D_PROPERTY(DistanceJointDefB2, frequency, Variant::REAL);
	BOX2D_PROPERTY(DistanceJointDefB2, damping, Variant::REAL);
}
