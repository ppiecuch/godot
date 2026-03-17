/**************************************************************************/
/*  iso_kinematic_body.h                                                  */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#ifndef ISO_KINEMATIC_BODY_H
#define ISO_KINEMATIC_BODY_H

#include "scene/2d/physics_body_2d.h"

class IsoWorld;

class IsoKinematicBody : public KinematicBody2D {
	GDCLASS(IsoKinematicBody, KinematicBody2D);

	Vector3 _iso_position;
	Vector3 _iso_size;
	IsoWorld *_iso_world;

	void _find_iso_world();
	void _sync_to_screen();

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_iso_position(const Vector3 &p_position);
	Vector3 get_iso_position() const;

	void set_iso_size(const Vector3 &p_size);
	Vector3 get_iso_size() const;

	IsoWorld *get_iso_world() const;

	// Iso-aware movement
	Vector3 iso_move_and_slide(const Vector3 &p_iso_velocity, const Vector3 &p_iso_floor_normal = Vector3(0, 0, 1));
	bool iso_move_and_collide(const Vector3 &p_iso_motion);

	IsoKinematicBody();
};

#endif // ISO_KINEMATIC_BODY_H
