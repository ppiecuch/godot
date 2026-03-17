/**************************************************************************/
/*  iso_physics.h                                                         */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#ifndef ISO_PHYSICS_H
#define ISO_PHYSICS_H

#include "core/reference.h"

class IsoWorld;

// Iso-aware physics query helper.
// Used from GDScript: var result = IsoPhysics.new().iso_raycast(world, from, to)
// Or instantiate once and reuse.
class IsoPhysics : public Reference {
	GDCLASS(IsoPhysics, Reference);

protected:
	static void _bind_methods();

public:
	Dictionary iso_raycast(Object *p_world, const Vector3 &p_iso_from, const Vector3 &p_iso_to,
			const Array &p_exclude = Array(), int p_collision_mask = 0x7FFFFFFF);

	Array iso_intersect_point(Object *p_world, const Vector3 &p_iso_point,
			int p_max_results = 32, const Array &p_exclude = Array(), int p_collision_mask = 0x7FFFFFFF);
};

#endif // ISO_PHYSICS_H
