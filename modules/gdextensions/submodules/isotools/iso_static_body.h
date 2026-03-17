/**************************************************************************/
/*  iso_static_body.h                                                     */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#ifndef ISO_STATIC_BODY_H
#define ISO_STATIC_BODY_H

#include "scene/2d/physics_body_2d.h"

class IsoWorld;

class IsoStaticBody : public StaticBody2D {
	GDCLASS(IsoStaticBody, StaticBody2D);

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

	IsoStaticBody();
};

#endif // ISO_STATIC_BODY_H
