/**************************************************************************/
/*  iso_physics.h                                                         */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
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
