/**************************************************************************/
/*  prismatic_joint.cpp                                                   */
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

#include "prismatic_joint.h"

#include <Box2D/Box2D.h>

#define JOINT ((b2PrismaticJoint *)entity)
#define DEF ((b2PrismaticJointDef *)def)

float PrismaticJointB2::get_reference_angle() const { return JOINT->GetReferenceAngle(); }
float PrismaticJointB2::get_joint_translation() const { return JOINT->GetJointTranslation(); }
float PrismaticJointB2::get_joint_speed() const { return JOINT->GetJointSpeed(); }

bool PrismaticJointB2::is_limit_enabled() const { return JOINT->IsLimitEnabled(); }
void PrismaticJointB2::set_limit_enabled(bool rhs) { JOINT->EnableLimit(rhs); }

float PrismaticJointB2::get_lower_limit() const { return JOINT->GetLowerLimit(); }
void PrismaticJointB2::set_lower_limit(float rhs) { JOINT->SetLimits(rhs, JOINT->GetUpperLimit()); }

float PrismaticJointB2::get_upper_limit() const { return JOINT->GetUpperLimit(); }
void PrismaticJointB2::set_upper_limit(float rhs) { JOINT->SetLimits(JOINT->GetLowerLimit(), rhs); }

bool PrismaticJointB2::is_motor_enabled() const { return JOINT->IsMotorEnabled(); }
void PrismaticJointB2::set_motor_enabled(bool rhs) { JOINT->EnableMotor(rhs); }

float PrismaticJointB2::get_motor_speed() const { return JOINT->GetMotorSpeed(); }
void PrismaticJointB2::set_motor_speed(float rhs) { JOINT->SetMotorSpeed(rhs); }

float PrismaticJointB2::get_max_motor_force() const { return JOINT->GetMaxMotorForce(); }
void PrismaticJointB2::set_max_motor_force(float rhs) { JOINT->SetMaxMotorForce(rhs); }

float PrismaticJointB2::get_motor_force(float inv_dt) const { return JOINT->GetMotorForce(inv_dt); }

void PrismaticJointB2::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_reference_angle"), &PrismaticJointB2::get_reference_angle);
	ClassDB::bind_method(D_METHOD("get_joint_translation"), &PrismaticJointB2::get_joint_translation);
	ClassDB::bind_method(D_METHOD("get_joint_speed"), &PrismaticJointB2::get_joint_speed);

	BOX2D_PROPERTY_BOOL(PrismaticJointB2, limit_enabled);
	BOX2D_PROPERTY(PrismaticJointB2, lower_limit, Variant::REAL);
	BOX2D_PROPERTY(PrismaticJointB2, upper_limit, Variant::REAL);

	BOX2D_PROPERTY_BOOL(PrismaticJointB2, motor_enabled);
	BOX2D_PROPERTY(PrismaticJointB2, motor_speed, Variant::REAL);
	BOX2D_PROPERTY(PrismaticJointB2, max_motor_force, Variant::REAL);

	ClassDB::bind_method(D_METHOD("get_motor_force", "inv_dt"), &PrismaticJointB2::get_motor_force);
}

JointB2 *PrismaticJointDefB2::instance(WorldB2 *world) {
	ERR_FAIL_NULL_V(world, NULL);
	auto o = world->get_b2()->CreateJoint(def);
	return memnew(PrismaticJointB2(o));
}

PrismaticJointDefB2::PrismaticJointDefB2() :
		JointDefB2(memnew(b2PrismaticJointDef)) {}

Vector2 PrismaticJointDefB2::get_anchor_a() const { return GD(DEF->localAnchorA); }
void PrismaticJointDefB2::set_anchor_a(const Vector2 &o) { DEF->localAnchorA = B2(o); }

Vector2 PrismaticJointDefB2::get_anchor_b() const { return GD(DEF->localAnchorB); }
void PrismaticJointDefB2::set_anchor_b(const Vector2 &o) { DEF->localAnchorB = B2(o); }

Vector2 PrismaticJointDefB2::get_local_axis_a() const { return GD(DEF->localAxisA); }
void PrismaticJointDefB2::set_local_axis_a(const Vector2 &o) { DEF->localAxisA = B2(o); }

float PrismaticJointDefB2::get_reference_angle() const { return DEF->referenceAngle; }
void PrismaticJointDefB2::set_reference_angle(float o) { DEF->referenceAngle = o; }

bool PrismaticJointDefB2::get_enable_limit() const { return DEF->enableLimit; }
void PrismaticJointDefB2::set_enable_limit(bool o) { DEF->enableLimit = o; }

float PrismaticJointDefB2::get_lower_translation() const { return DEF->lowerTranslation; }
void PrismaticJointDefB2::set_lower_translation(float o) { DEF->lowerTranslation = o; }

float PrismaticJointDefB2::get_upper_translation() const { return DEF->upperTranslation; }
void PrismaticJointDefB2::set_upper_translation(float o) { DEF->upperTranslation = o; }

bool PrismaticJointDefB2::get_enable_motor() const { return DEF->enableMotor; }
void PrismaticJointDefB2::set_enable_motor(bool o) { DEF->enableMotor = o; }

float PrismaticJointDefB2::get_max_motor_force() const { return DEF->maxMotorForce; }
void PrismaticJointDefB2::set_max_motor_force(float o) { DEF->maxMotorForce = o; }

float PrismaticJointDefB2::get_motor_speed() const { return DEF->motorSpeed; }
void PrismaticJointDefB2::set_motor_speed(float o) { DEF->motorSpeed = o; }

void PrismaticJointDefB2::_bind_methods() {
	BOX2D_PROPERTY(PrismaticJointDefB2, anchor_a, Variant::VECTOR2);
	BOX2D_PROPERTY(PrismaticJointDefB2, anchor_b, Variant::VECTOR2);
	BOX2D_PROPERTY(PrismaticJointDefB2, local_axis_a, Variant::VECTOR2);
	BOX2D_PROPERTY(PrismaticJointDefB2, reference_angle, Variant::REAL);
	BOX2D_PROPERTY(PrismaticJointDefB2, enable_limit, Variant::BOOL);
	BOX2D_PROPERTY(PrismaticJointDefB2, lower_translation, Variant::REAL);
	BOX2D_PROPERTY(PrismaticJointDefB2, upper_translation, Variant::REAL);
	BOX2D_PROPERTY(PrismaticJointDefB2, enable_motor, Variant::BOOL);
	BOX2D_PROPERTY(PrismaticJointDefB2, max_motor_force, Variant::REAL);
	BOX2D_PROPERTY(PrismaticJointDefB2, motor_speed, Variant::REAL);
}
