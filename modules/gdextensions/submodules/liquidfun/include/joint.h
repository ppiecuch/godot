#ifndef BOX2D_JOINT_H
#define BOX2D_JOINT_H

#include "core/reference.h"

class JointB2 : public Object {
	GDCLASS(JointB2, Object);
	BOX2D_CLASS(Joint);

protected:
	static void _bind_methods();

public:
	/** Box2D methods */
	class BodyB2 *get_body_a() const;
	class BodyB2 *get_body_b() const;

	Vector2 get_anchor_a() const;
	Vector2 get_anchor_b() const;

	Vector2 get_reaction_force(float inv_dt) const;
	float get_reaction_torque(float inv_dt) const;

	BOX2D_GET_SET_DATA(Variant, metadata);

	bool is_active() const;
	bool get_collide_connected() const;
};

class JointDefB2 : public Reference {
	GDCLASS(JointDefB2, Reference);

protected:
	static void _bind_methods();

	/** Internal definition */
	JointDefB2(struct b2JointDef *);
	struct b2JointDef *def;

public:
	/** Lifecycle */
	virtual class JointB2 *instance(class WorldB2 *) = 0;
	~JointDefB2();

	/** Getters/setters */
	BOX2D_GET_SET(class BodyB2 *, body_a);
	BOX2D_GET_SET(class BodyB2 *, body_b);
	BOX2D_GET_SET(bool, collide_connected);
};

#endif // BOX2D_JOINT_H
