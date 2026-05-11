/**************************************************************************/
/*  gd_msa_physics.h                                                      */
/**************************************************************************/
/* Godot Reference wrapper around msa::physics::WorldT<Vector2>.          */
/* Index-based API: particles, springs, and attractions by integer id.    */
/**************************************************************************/

#ifndef GD_MSA_PHYSICS_H
#define GD_MSA_PHYSICS_H

#include "core/math/vector2.h"
#include "core/object.h"
#include "core/pool_vector.h"
#include "core/reference.h"

#include "msaphysics.h"

// Verlet-integration 2D physics world.
//
// Usage:
//   var w = MSAPhysicsWorld2D.new()
//   w.set_world_bounds(Vector2(0,0), Vector2(800,600))
//   w.set_gravity(Vector2(0, 9.8))
//   var p0 = w.add_particle(Vector2(100, 100))
//   var p1 = w.add_particle(Vector2(200, 100))
//   var s = w.add_spring(p0, p1, 0.5)
//   w.update()
//   var positions = w.get_particle_positions()  # PoolVector2Array
class MSAPhysicsWorld2D : public Reference {
	GDCLASS(MSAPhysicsWorld2D, Reference);

	mutable msa::physics::WorldT<Vector2> _world;

	// Stable index arrays. Pointers owned by _world.
	std::vector<msa::physics::ParticleT<Vector2> *> _particles;
	std::vector<msa::physics::SpringT<Vector2> *> _springs;
	std::vector<msa::physics::AttractionT<Vector2> *> _attractions;

protected:
	static void _bind_methods();

public:
	// Particles
	int add_particle(Vector2 pos, float mass = 1.0f);
	Vector2 get_particle_position(int idx) const;
	void set_particle_position(int idx, Vector2 pos);
	Vector2 get_particle_velocity(int idx) const;
	void set_particle_velocity(int idx, Vector2 vel);
	void set_particle_fixed(int idx, bool fixed);
	bool get_particle_fixed(int idx) const;
	int get_particle_count() const;
	PoolVector2Array get_particle_positions() const;

	// Springs
	int add_spring(int a, int b, float strength, float rest_length = -1.0f);
	void set_spring_strength(int idx, float strength);
	float get_spring_strength(int idx) const;
	int get_spring_count() const;

	// Attractions
	int add_attraction(int a, int b, float strength);
	void set_attraction_strength(int idx, float strength);
	float get_attraction_strength(int idx) const;
	int get_attraction_count() const;

	// World settings
	void set_gravity(Vector2 g);
	Vector2 get_gravity() const;
	void set_drag(float d);
	float get_drag() const;
	void set_time_step(float ts);
	float get_time_step() const;
	void set_world_bounds(Vector2 mn, Vector2 mx);
	void set_collision_enabled(bool b);
	bool get_collision_enabled() const;

	void update();
	void clear();

	MSAPhysicsWorld2D();
};

#endif // GD_MSA_PHYSICS_H
