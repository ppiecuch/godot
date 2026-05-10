/**************************************************************************/
/*  world.h                                                               */
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

#ifndef BOX2D_WORLD_H
#define BOX2D_WORLD_H

#include "core/object.h"
#include "core/vector.h"

class ParticleSystemB2;
class ParticleSystemDefB2;

class WorldB2 : public Object {
	GDCLASS(WorldB2, Object);
	BOX2D_CLASS(World);

	Vector<class ParticleSystemB2 *> m_particle_systems;

protected:
	static void _bind_methods();

public:
	/** Box2D methods */
	void step(float timeStep, int velocityIterations, int positionIterations);
	void clear_forces();

	void query_aabb(class FuncRef *callback, const Rect2 &aabb) const;
	void ray_cast(class FuncRef *callback, const Vector2 &a, const Vector2 &b) const;

	Vector2 get_gravity() const;
	void set_gravity(const Vector2 &);

	bool is_locked() const;

	bool get_auto_clear_forces() const;
	void set_auto_clear_forces(bool);

	void shift_origin(const Vector2 &);

	Variant get_metadata() const;
	void set_metadata(const Variant &);

	/** Particle systems */
	class ParticleSystemB2 *create_particle_system(class ParticleSystemDefB2 *psd);
	void destroy_particle_system(class ParticleSystemB2 *ps);
	Array get_particle_systems() const;
};

#endif // BOX2D_WORLD_H
