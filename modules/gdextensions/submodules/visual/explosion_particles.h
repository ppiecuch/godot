/**************************************************************************/
/*  explosion_particles.h                                                 */
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

#ifndef GD_EXPLOSION_PARTICLES_2D_H
#define GD_EXPLOSION_PARTICLES_2D_H

#include "core/math/vector2.h"
#include "core/object.h"
#include "core/typedefs.h"
#include "scene/2d/node_2d.h"
#include "scene/2d/physics_body_2d.h"
#include "scene/main/timer.h"
#include "scene/resources/packed_scene.h"
#include "scene/resources/shape_2d.h"

#include <vector>

// Example:
// --------
// extends KinematicBody2D
//
// export var velocity:Vector2
//
// func _physics_process(delta):
//     var collision = move_and_collide(velocity * delta)
//     if collision:
//         velocity = velocity.bounce(collision.normal)
//         var emiter_position:Vector2 = to_local(collision.position)
//         $marker.position = emiter_position
//         $emitter.set_emitter_position(emiter_position)
//         $emitter.single_explosion()

class FakeExplosionParticles2D : public Node2D {
	GDCLASS(FakeExplosionParticles2D, Node2D);

public:
	enum ClipMode {
		CLIP_NONE,
		CLIP_OUTSIDE,
		CLIP_INSIDE,
		ClippingCount,
	};

private:
	struct ParticleType {
		bool dead;
		real_t alpha;
		Color color;
		Vector2 gravity;
		Point2 position;
		Size2 size;
		real_t time;
		Vector2 velocity_increment;
		Vector2 velocity;
	};

	int min_particles_number;
	int max_particles_number;

	real_t min_particles_gravity;
	real_t max_particles_gravity;

	real_t min_particles_velocity;
	real_t max_particles_velocity;

	int min_particles_size;
	int max_particles_size;

	real_t timer_wait_time;
	bool particles_explode;
	ClipMode clipping_mode;

	Vector2 emitter_offset;
	int spread_radius;
	Size2 view_size;
	bool view_centered;
	String group_name;

	void _create_particles();
	void _particles_explode(real_t delta);

	int _repeat_counter;
	std::vector<ParticleType> _particles;

	struct ColorTable {
		uint8_t w;
		Color c;
	};
	const std::vector<ColorTable> _particles_colors_with_weights{
		{ 4, Color::html("#ffffff") },
		{ 2, Color::html("#000000") },
		{ 8, Color::html("#ff004d") },
		{ 8, Color::html("#ffa300") },
		{ 10, Color::html("#ffec27") }
	};

	Timer *_particles_timer = 0;
	void _on_particles_timer_timeout();

	real_t _get_random_alpha();
	Color _get_random_color();
	Vector2 _get_random_gravity();
	int _get_random_number();
	Point2 _get_random_position();
	Size2 _get_random_size();
	Vector2 _get_random_velocity();
	Vector2 _get_random_velocity_increment();
	real_t _get_random_time();
	template <typename T>
	T _rand_array(const std::vector<T> &array);

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
#ifdef TOOLS_ENABLED
	Dictionary _edit_get_state() const;
	void _edit_set_pivot(const Point2 &p_pivot);
	Point2 _edit_get_pivot() const;
	bool _edit_use_pivot() const;
	void _edit_set_state(const Dictionary &p_state);
	bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;
	Rect2 _edit_get_rect() const;
	void _edit_set_rect(const Rect2 &p_rect);
	bool _edit_use_rect() const;
#endif

	void set_min_particles_number(int p_num);
	int get_min_particles_number() const;

	void set_max_particles_number(int p_num);
	int get_max_particles_number() const;

	void set_min_particles_gravity(real_t p_gravity);
	real_t get_min_particles_gravity() const;

	void set_max_particles_gravity(real_t p_gravity);
	real_t get_max_particles_gravity() const;

	void set_min_particles_velocity(real_t p_velocity);
	real_t get_min_particles_velocity() const;

	void set_max_particles_velocity(real_t p_velocity);
	real_t get_max_particles_velocity() const;

	void set_min_particles_size(int p_size);
	int get_min_particles_size() const;

	void set_max_particles_size(int p_size);
	int get_max_particles_size() const;

	void set_wait_time(real_t p_wait);
	bool get_wait_time() const;

	void set_particles_explode(bool p_start);
	bool get_particles_explode() const;

	void set_emitter_offset(const Vector2 &p_offset);
	Vector2 get_emitter_offset() const;

	void set_particles_clipping_mode(ClipMode p_clip);
	ClipMode get_particles_clipping_mode() const;

	void set_particles_spread_radius(int p_radius);
	int get_particles_spread_radius() const;

	void set_view_centered(bool p_centered);
	bool is_view_centered() const;

	void set_view_size(const Size2 &p_size);
	Size2 get_view_size() const;

	void set_view_group_name(String p_name);
	String get_view_group_name() const;

	void set_emitter_position(const Vector2 &p_position);
	void single_explosion();
	void repeat_explosion(float p_wait_time, int p_repeat_count);

	FakeExplosionParticles2D();
};

VARIANT_ENUM_CAST(FakeExplosionParticles2D::ClipMode);

class RigidBodyParticlesTracker;

class RigidBodyParticles2D : public RigidBody2D {
	GDCLASS(RigidBodyParticles2D, RigidBody2D);

private:
	bool emitting;
	int amount;
	real_t amount_random;
	Ref<PackedScene> particle_scene;
	bool one_shot;
	real_t explosiveness;
	String tracker_name;
	Ref<Shape2D> emission_shape;

	real_t lifetime;
	real_t lifetime_random;

	real_t impulse;
	real_t impulse_random;
	real_t impulse_angle_degrees;
	real_t impulse_spread_degrees;

	real_t force;
	real_t force_random;
	real_t force_angle_degrees;
	real_t force_spread_degrees;

	real_t initial_rotation_degrees;
	real_t initial_rotation_degrees_random;

	Timer *_shot_timer;
	Timer *_emit_timer;

	int _iteration;
	RigidBodyParticlesTracker *_tracker;
	int _capsule_circle_frac;

	// per shot state variables
	int _particle_count;
	int _emit_count;
	int _shot_started;

protected:
	void _notification(int p_what);
	static void _bind_methods();

	void _on_shot_timer_timeout();

public:
	RigidBodyParticles2D();
};

class RigidBodyParticlesTracker : public Node2D {
	GDCLASS(RigidBodyParticlesTracker, Node2D);

private:
	RigidBodyParticles2D *particle;
	real_t lifetime;
	real_t impulse_angle;
	Vector2 impulse;
	real_t force_angle;
	Vector2 force;
	real_t initial_rotation;

	Timer *_remover;

protected:
	void _notification(int p_what);
	static void _bind_methods();

	void _on_remover_timeout();

public:
	RigidBodyParticlesTracker();
};

#endif // GD_EXPLOSION_PARTICLES_2D_H
