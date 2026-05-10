#ifndef BOX2D_WELD_JOINT_H
#define BOX2D_WELD_JOINT_H

#include "joint.h"

class WeldJointB2 : public JointB2 {
	GDCLASS(WeldJointB2, JointB2);
	BOX2D_JOINT(WeldJoint);

protected:
	static void _bind_methods();

public:
	float get_reference_angle() const;
	BOX2D_GET_SET(float, frequency);
	BOX2D_GET_SET(float, damping);
};

class WeldJointDefB2 : public JointDefB2 {
	GDCLASS(WeldJointDefB2, JointDefB2);

protected:
	static void _bind_methods();

public:
	class JointB2 *instance(class WorldB2 *);

	BOX2D_GET_SET_DATA(Vector2, anchor_a);
	BOX2D_GET_SET_DATA(Vector2, anchor_b);
	BOX2D_GET_SET(float, reference_angle);
	BOX2D_GET_SET(float, frequency);
	BOX2D_GET_SET(float, damping);

	WeldJointDefB2();
};

#endif // BOX2D_WELD_JOINT_H
