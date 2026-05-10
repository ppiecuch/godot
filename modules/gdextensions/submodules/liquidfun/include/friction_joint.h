
#ifndef BOX2D_FRICTION_JOINT_H
#define BOX2D_FRICTION_JOINT_H

#include "joint.h"

class FrictionJointB2 : public JointB2 {
	GDCLASS(FrictionJointB2, JointB2);
	BOX2D_JOINT(FrictionJoint);

protected:
	static void _bind_methods();

public:
	BOX2D_GET_SET(float, max_force);
	BOX2D_GET_SET(float, max_torque);
};

class FrictionJointDefB2 : public JointDefB2 {
	GDCLASS(FrictionJointDefB2, JointDefB2);

protected:
	static void _bind_methods();

public:
	class JointB2 *instance(class WorldB2 *);

	BOX2D_GET_SET_DATA(Vector2, anchor_a);
	BOX2D_GET_SET_DATA(Vector2, anchor_b);
	BOX2D_GET_SET(float, max_force);
	BOX2D_GET_SET(float, max_torque);

	FrictionJointDefB2();
};

#endif // BOX2D_FRICTION_JOINT_H
