
#ifndef BOX2D_WHEEL_JOINT_H
#define BOX2D_WHEEL_JOINT_H

#include "joint.h"

class WheelJointB2 : public JointB2 {
	GDCLASS(WheelJointB2, JointB2);
	BOX2D_JOINT(WheelJoint);

protected:
	static void _bind_methods();

public:
	float get_joint_translation() const;
	float get_joint_speed() const;

	BOX2D_IS_SET(motor_enabled);
	BOX2D_GET_SET(float, motor_speed);
	BOX2D_GET_SET(float, max_motor_torque);
	float get_motor_torque(float inv_dt) const;

	BOX2D_GET_SET(float, spring_frequency);
	BOX2D_GET_SET(float, spring_damping);
};

class WheelJointDefB2 : public JointDefB2 {
	GDCLASS(WheelJointDefB2, JointDefB2);

protected:
	static void _bind_methods();

public:
	class JointB2 *instance(class WorldB2 *);

	BOX2D_GET_SET_DATA(Vector2, anchor_a);
	BOX2D_GET_SET_DATA(Vector2, anchor_b);
	BOX2D_GET_SET_DATA(Vector2, local_axis_a);
	BOX2D_GET_SET(bool, enable_motor);
	BOX2D_GET_SET(float, max_motor_torque);
	BOX2D_GET_SET(float, motor_speed);
	BOX2D_GET_SET(float, frequency);
	BOX2D_GET_SET(float, damping);

	WheelJointDefB2();
};

#endif // BOX2D_WHEEL_JOINT_H
