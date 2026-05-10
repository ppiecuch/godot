
#ifndef BOX2D_ROPE_JOINT_H
#define BOX2D_ROPE_JOINT_H

#include "joint.h"

class RopeJointB2 : public JointB2 {
	GDCLASS(RopeJointB2, JointB2);
	BOX2D_JOINT(RopeJoint);

protected:
	static void _bind_methods();

public:
	BOX2D_GET_SET(float, max_length);
	int get_limit_state() const;
};

class RopeJointDefB2 : public JointDefB2 {
	GDCLASS(RopeJointDefB2, JointDefB2);

protected:
	static void _bind_methods();

public:
	class JointB2 *instance(class WorldB2 *);

	BOX2D_GET_SET_DATA(Vector2, anchor_a);
	BOX2D_GET_SET_DATA(Vector2, anchor_b);
	BOX2D_GET_SET(float, max_length);

	RopeJointDefB2();
};

#endif // BOX2D_ROPE_JOINT_H
