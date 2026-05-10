
#ifndef BOX2D_PULLEY_JOINT_H
#define BOX2D_PULLEY_JOINT_H

#include "joint.h"

class PulleyJointB2 : public JointB2 {
	GDCLASS(PulleyJointB2, JointB2);
	BOX2D_JOINT(PulleyJoint);

protected:
	static void _bind_methods();

public:
	Vector2 get_ground_anchor_a() const;
	Vector2 get_ground_anchor_b() const;

	float get_length_a() const;
	float get_length_b() const;
	float get_current_length_a() const;
	float get_current_length_b() const;
	float get_ratio() const;
};

class PulleyJointDefB2 : public JointDefB2 {
	GDCLASS(PulleyJointDefB2, JointDefB2);

protected:
	static void _bind_methods();

public:
	class JointB2 *instance(class WorldB2 *);

	BOX2D_GET_SET_DATA(Vector2, ground_anchor_a);
	BOX2D_GET_SET_DATA(Vector2, ground_anchor_b);
	BOX2D_GET_SET_DATA(Vector2, anchor_a);
	BOX2D_GET_SET_DATA(Vector2, anchor_b);
	BOX2D_GET_SET(float, length_a);
	BOX2D_GET_SET(float, length_b);
	BOX2D_GET_SET(float, ratio);

	PulleyJointDefB2();
};

#endif // BOX2D_PULLEY_JOINT_H
