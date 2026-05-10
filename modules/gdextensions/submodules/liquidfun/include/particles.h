/**************************************************************************/
/*  particles.h                                                           */
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

#ifndef BOX2D_PARTICLES_H
#define BOX2D_PARTICLES_H

#include "core/object.h"
#include "core/reference.h"

// Configures a single particle before creation.
class ParticleDefB2 : public Reference {
	GDCLASS(ParticleDefB2, Reference);

	int flags = 0;
	Vector2 position;
	Vector2 velocity;
	Color color;
	float lifetime = 0.0f;

protected:
	static void _bind_methods();

public:
	int get_flags() const { return flags; }
	void set_flags(int v) { flags = v; }

	Vector2 get_position() const { return position; }
	void set_position(const Vector2 &v) { position = v; }

	Vector2 get_velocity() const { return velocity; }
	void set_velocity(const Vector2 &v) { velocity = v; }

	Color get_color() const { return color; }
	void set_color(const Color &c) { color = c; }

	float get_lifetime() const { return lifetime; }
	void set_lifetime(float v) { lifetime = v; }
};

// Thin wrapper around a live b2ParticleGroup.  b2 owns the group; we do not
// delete it.  The wrapper becomes invalid after the group is destroyed.
class ParticleGroupB2 : public Object {
	GDCLASS(ParticleGroupB2, Object);

	class b2ParticleGroup *entity;

protected:
	static void _bind_methods();

public:
	ParticleGroupB2(class b2ParticleGroup *g);
	~ParticleGroupB2();

	class b2ParticleGroup *get_b2() const { return entity; }
	static ParticleGroupB2 *get(class b2ParticleGroup *g);

	int get_particle_count() const;
	int get_buffer_index() const;
	int get_all_particle_flags() const;
	int get_group_flags() const;
	void set_group_flags(int flags);

	float get_mass() const;
	float get_inertia() const;
	Vector2 get_center() const;
	Vector2 get_linear_velocity() const;
	float get_angular_velocity() const;
	Vector2 get_position() const;
	float get_angle() const;

	void apply_force(const Vector2 &force);
	void apply_linear_impulse(const Vector2 &impulse);
	void destroy_particles();
};

// Configures a particle group before creation.
class ParticleGroupDefB2 : public Reference {
	GDCLASS(ParticleGroupDefB2, Reference);

	struct b2ParticleGroupDef *def;
	Ref<class ShapeB2> shape_ref;

protected:
	static void _bind_methods();

public:
	ParticleGroupDefB2();
	~ParticleGroupDefB2();

	int get_flags() const;
	void set_flags(int v);
	int get_group_flags() const;
	void set_group_flags(int v);
	Vector2 get_position() const;
	void set_position(const Vector2 &v);
	float get_angle() const;
	void set_angle(float v);
	Vector2 get_linear_velocity() const;
	void set_linear_velocity(const Vector2 &v);
	float get_angular_velocity() const;
	void set_angular_velocity(float v);
	Color get_color() const;
	void set_color(const Color &c);
	float get_strength() const;
	void set_strength(float v);
	float get_stride() const;
	void set_stride(float v);
	float get_lifetime() const;
	void set_lifetime(float v);
	Ref<class ShapeB2> get_shape() const;
	void set_shape(const Ref<class ShapeB2> &s);

	class ParticleGroupB2 *instance(class ParticleSystemB2 *ps);
};

// Wraps a live b2ParticleSystem.  Created by WorldB2::create_particle_system().
class ParticleSystemB2 : public Object {
	GDCLASS(ParticleSystemB2, Object);

	class b2ParticleSystem *entity;
	class b2World *b2_world;

protected:
	static void _bind_methods();

public:
	ParticleSystemB2(class b2ParticleSystem *ps, class b2World *world);
	~ParticleSystemB2();

	class b2ParticleSystem *get_b2() const { return entity; }

	// Particle creation/destruction
	int create_particle(class ParticleDefB2 *pdef);
	int create_particle_at(const Vector2 &position, int flags = 0);
	void destroy_particle(int index);

	// Group creation
	class ParticleGroupB2 *create_group(class ParticleGroupDefB2 *gdef);

	// Buffer snapshots (length == get_particle_count())
	PoolVector2Array get_position_buffer() const;
	PoolVector2Array get_velocity_buffer() const;
	PoolColorArray get_color_buffer() const;

	// Counts
	int get_particle_count() const;
	int get_particle_group_count() const;
	Array get_particle_group_list() const;

	// Physics properties
	float get_density() const;
	void set_density(float v);
	float get_gravity_scale() const;
	void set_gravity_scale(float v);
	float get_radius() const;
	void set_radius(float v);
	float get_damping() const;
	void set_damping(float v);
	int get_max_particle_count() const;
	void set_max_particle_count(int v);
	bool is_paused() const;
	void set_paused(bool v);

	// Particle type flags (b2ParticleFlag)
	enum {
		PARTICLE_WATER = 0,
		PARTICLE_ZOMBIE = 1 << 1,
		PARTICLE_WALL = 1 << 2,
		PARTICLE_SPRING = 1 << 3,
		PARTICLE_ELASTIC = 1 << 4,
		PARTICLE_VISCOUS = 1 << 5,
		PARTICLE_POWDER = 1 << 6,
		PARTICLE_TENSILE = 1 << 7,
		PARTICLE_COLOR_MIXING = 1 << 8,
		PARTICLE_DESTRUCTION_LISTENER = 1 << 9,
		PARTICLE_BARRIER = 1 << 10,
		PARTICLE_STATIC_PRESSURE = 1 << 11,
		PARTICLE_REACTIVE = 1 << 12,
		PARTICLE_REPULSIVE = 1 << 13,
	};
};

// Configures a particle system before creation via WorldB2::create_particle_system().
class ParticleSystemDefB2 : public Reference {
	GDCLASS(ParticleSystemDefB2, Reference);

	struct b2ParticleSystemDef *def;

protected:
	static void _bind_methods();

public:
	ParticleSystemDefB2();
	~ParticleSystemDefB2();

	bool is_strict_contact_check() const;
	void set_strict_contact_check(bool v);
	float get_density() const;
	void set_density(float v);
	float get_gravity_scale() const;
	void set_gravity_scale(float v);
	float get_radius() const;
	void set_radius(float v);
	int get_max_count() const;
	void set_max_count(int v);
	float get_pressure_strength() const;
	void set_pressure_strength(float v);
	float get_damping_strength() const;
	void set_damping_strength(float v);
	float get_elastic_strength() const;
	void set_elastic_strength(float v);
	float get_spring_strength() const;
	void set_spring_strength(float v);
	float get_viscous_strength() const;
	void set_viscous_strength(float v);
	float get_surface_tension_pressure_strength() const;
	void set_surface_tension_pressure_strength(float v);
	float get_surface_tension_normal_strength() const;
	void set_surface_tension_normal_strength(float v);
	float get_repulsive_strength() const;
	void set_repulsive_strength(float v);
	float get_powder_strength() const;
	void set_powder_strength(float v);
	float get_ejection_strength() const;
	void set_ejection_strength(float v);
	float get_static_pressure_strength() const;
	void set_static_pressure_strength(float v);
	float get_static_pressure_relaxation() const;
	void set_static_pressure_relaxation(float v);
	int get_static_pressure_iterations() const;
	void set_static_pressure_iterations(int v);
	float get_color_mixing_strength() const;
	void set_color_mixing_strength(float v);
	bool is_destroy_by_age() const;
	void set_destroy_by_age(bool v);
	float get_lifetime_granularity() const;
	void set_lifetime_granularity(float v);

	class ParticleSystemB2 *instance(class WorldB2 *world);

	const struct b2ParticleSystemDef *get_b2_def() const { return def; }
};

#endif // BOX2D_PARTICLES_H
