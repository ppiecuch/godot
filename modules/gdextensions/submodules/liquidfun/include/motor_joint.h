
#ifndef BOX2D_MOTOR_JOINT_H
#define BOX2D_MOTOR_JOINT_H

#include "joint.h"

class MotorJointB2 : public JointB2 {
	GDCLASS(MotorJointB2, JointB2);
	BOX2D_JOINT(MotorJoint);

protected:
	static void _bind_methods();

public:
	BOX2D_GET_SET_DATA(Vector2, linear_offset);
	BOX2D_GET_SET(float, angular_offset);
	BOX2D_GET_SET(float, max_force);
	BOX2D_GET_SET(float, max_torque);
	BOX2D_GET_SET(float, correction_factor);
};

class MotorJointDefB2 : public JointDefB2 {
	GDCLASS(MotorJointDefB2, JointDefB2);

protected:
	static void _bind_methods();

public:
	class JointB2 *instance(class WorldB2 *);

	BOX2D_GET_SET_DATA(Vector2, linear_offset);
	BOX2D_GET_SET(float, angular_offset);
	BOX2D_GET_SET(float, max_force);
	BOX2D_GET_SET(float, max_torque);
	BOX2D_GET_SET(float, correction_factor);

	MotorJointDefB2();
};

#endif // BOX2D_MOTOR_JOINT_H
