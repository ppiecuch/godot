
#ifndef BOX2D_GEAR_JOINT_H
#define BOX2D_GEAR_JOINT_H

#include "joint.h"

class GearJointB2 : public JointB2 {
	GDCLASS(GearJointB2, JointB2);
	BOX2D_JOINT(GearJoint);

protected:
	static void _bind_methods();

public:
	class JointB2 *get_joint1() const;
	class JointB2 *get_joint2() const;
	BOX2D_GET_SET(float, ratio);
};

class GearJointDefB2 : public JointDefB2 {
	GDCLASS(GearJointDefB2, JointDefB2);

protected:
	static void _bind_methods();

private:
	class JointB2 *joint1_wrapper = nullptr;
	class JointB2 *joint2_wrapper = nullptr;

public:
	class JointB2 *instance(class WorldB2 *);

	void set_joint1(class JointB2 *joint);
	class JointB2 *get_joint1() const;
	void set_joint2(class JointB2 *joint);
	class JointB2 *get_joint2() const;
	BOX2D_GET_SET(float, ratio);

	GearJointDefB2();
};

#endif // BOX2D_GEAR_JOINT_H
