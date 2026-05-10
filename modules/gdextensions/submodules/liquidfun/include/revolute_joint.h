/**************************************************************************/
/*  revolute_joint.h                                                      */
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

#ifndef BOX2D_REVOLUTE_JOINT_H
#define BOX2D_REVOLUTE_JOINT_H

#include "joint.h"

class RevoluteJointB2 : public JointB2 {
	GDCLASS(RevoluteJointB2, JointB2);
	BOX2D_JOINT(RevoluteJoint);

protected:
	static void _bind_methods();

public:
	/** Box2D methods */
	float get_reference_angle() const;

	float get_joint_angle() const;
	float get_joint_speed() const;

	BOX2D_IS_SET(limit_enabled);
	BOX2D_GET_SET(float, lower_limit);
	BOX2D_GET_SET(float, upper_limit);
	BOX2D_IS_SET(motor_enabled);
	BOX2D_GET_SET(float, motor_speed);
	BOX2D_GET_SET(float, max_motor_torque);

	float get_motor_torque(float inv_dt) const;
};

class RevoluteJointDefB2 : public JointDefB2 {
	GDCLASS(RevoluteJointDefB2, JointDefB2);

protected:
	static void _bind_methods();

public:
	class JointB2 *instance(class WorldB2 *);

	/** Getters/setters */
	BOX2D_GET_SET_DATA(Vector2, anchor_a);
	BOX2D_GET_SET_DATA(Vector2, anchor_b);
	BOX2D_GET_SET(float, reference_angle);
	BOX2D_GET_SET(bool, enable_limit);
	BOX2D_GET_SET(float, lower_angle);
	BOX2D_GET_SET(float, upper_angle);
	BOX2D_GET_SET(bool, enable_motor);
	BOX2D_GET_SET(float, motor_speed);
	BOX2D_GET_SET(float, max_motor_torque);

	RevoluteJointDefB2();
};

#endif // BOX2D_REVOLUTE_JOINT_H
