#ifndef BOX2D_DISTANCE_JOINT_H
#define BOX2D_DISTANCE_JOINT_H

#include "joint.h"

class DistanceJointB2 : public JointB2 {
	GDCLASS(DistanceJointB2, JointB2);
	BOX2D_JOINT(DistanceJoint);

protected:
	static void _bind_methods();

public:
	BOX2D_GET_SET(float, length);
	BOX2D_GET_SET(float, frequency);
	BOX2D_GET_SET(float, damping);
};

class DistanceJointDefB2 : public JointDefB2 {
	GDCLASS(DistanceJointDefB2, JointDefB2);

protected:
	static void _bind_methods();

public:
	class JointB2 *instance(class WorldB2 *);

	BOX2D_GET_SET_DATA(Vector2, anchor_a);
	BOX2D_GET_SET_DATA(Vector2, anchor_b);
	BOX2D_GET_SET(float, length);
	BOX2D_GET_SET(float, frequency);
	BOX2D_GET_SET(float, damping);

	DistanceJointDefB2();
};

#endif // BOX2D_DISTANCE_JOINT_H
