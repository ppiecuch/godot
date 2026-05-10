#ifndef BOX2D_MOUSE_JOINT_H
#define BOX2D_MOUSE_JOINT_H

#include "joint.h"

class MouseJointB2 : public JointB2 {
	GDCLASS(MouseJointB2, JointB2);
	BOX2D_JOINT(MouseJoint);

protected:
	static void _bind_methods();

public:
	/** Box2D methods */
	BOX2D_GET_SET_DATA(Vector2, target);
	BOX2D_GET_SET(float, max_force);
	BOX2D_GET_SET(float, frequency);
	BOX2D_GET_SET(float, damping);
};

class MouseJointDefB2 : public JointDefB2 {
	GDCLASS(MouseJointDefB2, JointDefB2);

protected:
	static void _bind_methods();

public:
	class JointB2 *instance(class WorldB2 *);

	/** Getters/setters */
	BOX2D_GET_SET_DATA(Vector2, target);
	BOX2D_GET_SET(float, max_force);
	BOX2D_GET_SET(float, frequency);
	BOX2D_GET_SET(float, damping);

	MouseJointDefB2();
};

#endif // BOX2D_MOUSE_JOINT_H
