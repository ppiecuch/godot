/**************************************************************************/
/*  iso_rigid_body.h                                                      */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#ifndef ISO_RIGID_BODY_H
#define ISO_RIGID_BODY_H

#include "scene/2d/physics_body_2d.h"

class IsoWorld;

class IsoRigidBody : public RigidBody2D {
	GDCLASS(IsoRigidBody, RigidBody2D);

	Vector3 _iso_position;
	Vector3 _iso_size;
	IsoWorld *_iso_world;
	bool _use_iso_gravity;
	bool _syncing;

	void _find_iso_world();
	void _sync_to_screen();
	void _sync_from_screen();

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_iso_position(const Vector3 &p_position);
	Vector3 get_iso_position() const;

	void set_iso_size(const Vector3 &p_size);
	Vector3 get_iso_size() const;

	void set_use_iso_gravity(bool p_use);
	bool get_use_iso_gravity() const;

	IsoWorld *get_iso_world() const;

	// Iso-aware force/impulse methods
	void iso_add_central_force(const Vector3 &p_iso_force);
	void iso_add_central_impulse(const Vector3 &p_iso_impulse);
	void iso_add_explosion_force(real_t p_force, const Vector3 &p_iso_origin, real_t p_radius);

	// Iso velocity
	void set_iso_velocity(const Vector3 &p_velocity);
	Vector3 get_iso_velocity() const;

	IsoRigidBody();
};

#endif // ISO_RIGID_BODY_H
