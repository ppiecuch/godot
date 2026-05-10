#ifndef BOX2D_PRISMATIC_JOINT_H
#define BOX2D_PRISMATIC_JOINT_H

#include "joint.h"

class PrismaticJointB2 : public JointB2 {
	GDCLASS(PrismaticJointB2, JointB2);
	BOX2D_JOINT(PrismaticJoint);

protected:
	static void _bind_methods();

public:
	float get_reference_angle() const;
	float get_joint_translation() const;
	float get_joint_speed() const;

	BOX2D_IS_SET(limit_enabled);
	BOX2D_GET_SET(float, lower_limit);
	BOX2D_GET_SET(float, upper_limit);

	BOX2D_IS_SET(motor_enabled);
	BOX2D_GET_SET(float, motor_speed);
	BOX2D_GET_SET(float, max_motor_force);

	float get_motor_force(float inv_dt) const;
};

class PrismaticJointDefB2 : public JointDefB2 {
	GDCLASS(PrismaticJointDefB2, JointDefB2);

protected:
	static void _bind_methods();

public:
	class JointB2 *instance(class WorldB2 *);

	BOX2D_GET_SET_DATA(Vector2, anchor_a);
	BOX2D_GET_SET_DATA(Vector2, anchor_b);
	BOX2D_GET_SET_DATA(Vector2, local_axis_a);
	BOX2D_GET_SET(float, reference_angle);
	BOX2D_GET_SET(bool, enable_limit);
	BOX2D_GET_SET(float, lower_translation);
	BOX2D_GET_SET(float, upper_translation);
	BOX2D_GET_SET(bool, enable_motor);
	BOX2D_GET_SET(float, max_motor_force);
	BOX2D_GET_SET(float, motor_speed);

	PrismaticJointDefB2();
};

#endif // BOX2D_PRISMATIC_JOINT_H
