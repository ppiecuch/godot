/**************************************************************************/
/*  explosion_particles.cpp                                               */
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

// Reference:
// ----------
// https://github.com/danboo/godot-RigidBodyParticles2D/blob/master/examples/sparks/Particle.gd
// https://github.com/hiulit/Godot-3-2D-Destructible-Objects/blob/master/fake_explosion_particles.gd
// https://github.com/hiulit/Godot-3-2D-Fake-Explosion-Particles

#include "explosion_particles.h"

#include "common/gd_core.h"

#include <algorithm> // remove_if

#define rand_range random

#ifdef TOOLS_ENABLED
Dictionary FakeExplosionParticles2D::_edit_get_state() const {
	Dictionary state = Node2D::_edit_get_state();
	state["view_size"] = get_view_size();

	return state;
}

void FakeExplosionParticles2D::_edit_set_state(const Dictionary &p_state) {
	Node2D::_edit_set_state(p_state);
	set_view_size(p_state["view_size"]);
}

void FakeExplosionParticles2D::_edit_set_pivot(const Point2 &p_pivot) {
}

Point2 FakeExplosionParticles2D::_edit_get_pivot() const {
	return view_size / 2;
}

bool FakeExplosionParticles2D::_edit_use_pivot() const {
	return view_centered;
}

bool FakeExplosionParticles2D::_edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const {
	return _edit_get_rect().has_point(p_point);
};

Rect2 FakeExplosionParticles2D::_edit_get_rect() const {
	const Point2 ofs = view_centered ? -(view_size / 2) : Point2();
	return Rect2(ofs, get_view_size());
}

void FakeExplosionParticles2D::_edit_set_rect(const Rect2 &p_rect) {
	set_view_size(p_rect.size);
	_change_notify();
}

bool FakeExplosionParticles2D::_edit_use_rect() const {
	return true;
}
#endif

static bool _is_inside_circle(const Point2 &center, real_t r, const Point2 &pt) {
	const real_t dist = (pt.x - center.x) * (pt.x - center.x) + (pt.y - center.y) * (pt.y - center.y);
	return dist <= r * r;
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

static bool _is_inside_ellipse(const Point2 &center, const Size2 &r, const Point2 &pt) {
	return (pt.x - center.x) * (pt.x - center.x) / (r.x * r.x) + (pt.y - center.y) * (pt.y - center.y) / (r.y * r.y);
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

real_t FakeExplosionParticles2D::_get_random_alpha() {
	Math::randomize();
	return Math::rand_range(2, 10);
}

Color FakeExplosionParticles2D::_get_random_color() {
	Math::randomize();
	return _rand_array(_particles_colors_with_weights).c;
}

Vector2 FakeExplosionParticles2D::_get_random_gravity() {
	Math::randomize();
	return Vector2(
			Math::rand_range(
					-Math::rand_range(min_particles_gravity, max_particles_gravity),
					Math::rand_range(min_particles_gravity, max_particles_gravity)),
			Math::rand_range(
					Math::rand_range(min_particles_gravity * 2, max_particles_gravity * 2),
					Math::rand_range(min_particles_gravity * 2, max_particles_gravity * 2)));
}

int FakeExplosionParticles2D::_get_random_number() {
	Math::randomize();
	return Math::rand_range(min_particles_number, max_particles_number);
}

Point2 FakeExplosionParticles2D::_get_random_position() {
	Math::randomize();
	const real_t random_position_x = Math::rand_range(real_t(0), view_size.x);
	const real_t random_position_y = Math::rand_range(real_t(0), view_size.y);
	return Point2(random_position_x, random_position_y);
}

Size2 FakeExplosionParticles2D::_get_random_size() {
	Math::randomize();
	const real_t random_size = Math::rand() % max_particles_size + min_particles_size;
	return Size2(random_size, random_size);
}

Vector2 FakeExplosionParticles2D::_get_random_velocity() {
	Math::randomize();
	return Vector2(
			Math::rand_range(
					-Math::rand_range(min_particles_velocity, max_particles_velocity),
					Math::rand_range(min_particles_velocity, max_particles_velocity)),
			Math::rand_range(
					-Math::rand_range(min_particles_velocity * 2, max_particles_velocity * 2),
					-Math::rand_range(min_particles_velocity * 2, max_particles_velocity * 2)));
}

Vector2 FakeExplosionParticles2D::_get_random_velocity_increment() {
	Math::randomize();
	return Vector2(Math::rand_range(0.991, 1.009), Math::rand_range(0.991, 1.009));
}

real_t FakeExplosionParticles2D::_get_random_time() {
	Math::randomize();
	return Math::rand_range(0.05, 0.1);
}

template <typename T>
T FakeExplosionParticles2D::_rand_array(const std::vector<T> &array) {
	// Code from @CowThing (https://pastebin.com/HhdBuUzT).
	// T must be [w:weight, v:value].

	real_t sum_of_weights = 0;
	for (auto &t : array)
		sum_of_weights += t.w;

	const real_t x = Math::randf() * sum_of_weights;

	real_t cumulative_weight = 0;
	for (auto &t : array) {
		cumulative_weight += t.w;

		if (x < cumulative_weight)
			return t;
	}

	return T();
}

void FakeExplosionParticles2D::_particles_explode(real_t delta) {
	for (auto &particle : _particles) {
		particle.velocity.x *= particle.velocity_increment.x;
		particle.velocity.y *= particle.velocity_increment.y;
		particle.position += (particle.velocity + particle.gravity) * delta;

		particle.time += delta;

		if (particle.time > _get_random_time()) {
			// fade out the particles
			if (particle.color.a > 0) {
				particle.color.a -= particle.alpha * delta;
			}
			if (particle.color.a < 0) {
				particle.color.a = 0;
			}
		}
		// if the particle is invisible ...
		if (particle.color.a == 0) {
			particle.dead = true;
		}
	}
	// remove dead ones
	_particles.erase(std::remove_if(_particles.begin(), _particles.end(), [](const ParticleType &p) { return p.dead; }), _particles.end());
}

void FakeExplosionParticles2D::_create_particles() {
	// Set initial values.
	const int particles_number = _get_random_number();
	// Empty the particles array.
	_particles.clear();
	_particles.resize(particles_number);

	for (int i = 0; i < particles_number; i++) {
		// Create the particle object.
		ParticleType &particle = _particles[i];
		// Assign random variables to the particle object.
		particle.dead = false;
		particle.alpha = _get_random_alpha();
		particle.color = _get_random_color();
		particle.gravity = _get_random_gravity();
		particle.size = _get_random_size();
		particle.velocity = _get_random_velocity();
		particle.velocity_increment = _get_random_velocity_increment();
		particle.position = Point2();
	}
}

void FakeExplosionParticles2D::_on_particles_timer_timeout() {
	// Create new particles every time the timer times out.
	_create_particles();
	if (_repeat_counter == 0)
		_particles_timer->stop();
	else
		_repeat_counter--;
}

void FakeExplosionParticles2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
		} break;
		case NOTIFICATION_ENTER_TREE: {
			add_to_group(group_name); // Add to a group so it can be found from anywhere.
			if ((_particles_timer = memnew(Timer))) {
				_particles_timer->set_one_shot(false);
				_particles_timer->set_wait_time(timer_wait_time);
				_particles_timer->set_timer_process_mode(Timer::TIMER_PROCESS_IDLE);
				_particles_timer->connect("timeout", this, "_on_particles_timer_timeout");

				add_child(_particles_timer);
			}
		} break;
		case NOTIFICATION_PROCESS: {
			if (_particles.size() > 0 && particles_explode) {
				_particles_explode(get_process_delta_time());
				update();
			}
		} break;
		case NOTIFICATION_DRAW: {
			const Point2 emitter_origin = view_size * emitter_offset;
			for (auto &particle : _particles) {
				const Point2 particle_position = emitter_origin + particle.position;
				switch (clipping_mode) {
					case CLIP_NONE: {
					} break;
					case CLIP_INSIDE: {
						if (_is_inside_circle(emitter_origin, spread_radius, particle_position))
							continue;
					} break;
					case CLIP_OUTSIDE: {
						if (!_is_inside_circle(emitter_origin, spread_radius, particle_position))
							continue;
					} break;
					default: {
						WARN_PRINT("Invalid view clipping mode.");
					}
				}
				draw_rect(Rect2(particle_position, particle.size), particle.color);
			}
		} break;
	}
}

void FakeExplosionParticles2D::set_min_particles_number(int p_num) {
	min_particles_number = p_num;
}

int FakeExplosionParticles2D::get_min_particles_number() const {
	return min_particles_number;
}

void FakeExplosionParticles2D::set_max_particles_number(int p_num) {
	max_particles_number = p_num;
}

int FakeExplosionParticles2D::get_max_particles_number() const {
	return max_particles_number;
}

void FakeExplosionParticles2D::set_min_particles_gravity(real_t p_gravity) {
	min_particles_gravity = p_gravity;
}

real_t FakeExplosionParticles2D::get_min_particles_gravity() const {
	return min_particles_gravity;
}

void FakeExplosionParticles2D::set_max_particles_gravity(real_t p_gravity) {
	max_particles_gravity = p_gravity;
}

real_t FakeExplosionParticles2D::get_max_particles_gravity() const {
	return max_particles_gravity;
}

void FakeExplosionParticles2D::set_min_particles_velocity(real_t p_velocity) {
	min_particles_velocity = p_velocity;
}

real_t FakeExplosionParticles2D::get_min_particles_velocity() const {
	return min_particles_velocity;
}

void FakeExplosionParticles2D::set_max_particles_velocity(real_t p_velocity) {
	max_particles_velocity = p_velocity;
}

real_t FakeExplosionParticles2D::get_max_particles_velocity() const {
	return max_particles_velocity;
}

void FakeExplosionParticles2D::set_min_particles_size(int p_size) {
	min_particles_size = p_size;
}

int FakeExplosionParticles2D::get_min_particles_size() const {
	return min_particles_size;
}

void FakeExplosionParticles2D::set_max_particles_size(int p_size) {
	max_particles_size = p_size;
}

int FakeExplosionParticles2D::get_max_particles_size() const {
	return max_particles_size;
}

void FakeExplosionParticles2D::set_particles_explode(bool p_start) {
	particles_explode = p_start;
	set_process(particles_explode);
}

void FakeExplosionParticles2D::set_emitter_offset(const Vector2 &p_offset) {
	emitter_offset = p_offset;
}

Vector2 FakeExplosionParticles2D::get_emitter_offset() const {
	return emitter_offset;
}

bool FakeExplosionParticles2D::get_particles_explode() const {
	return particles_explode;
}

void FakeExplosionParticles2D::set_emitter_position(const Vector2 &p_position) {
	emitter_offset = p_position / view_size;
}

void FakeExplosionParticles2D::set_particles_clipping_mode(ClipMode p_clip) {
	ERR_FAIL_INDEX(p_clip, ClippingCount);
	clipping_mode = p_clip;
}

FakeExplosionParticles2D::ClipMode FakeExplosionParticles2D::get_particles_clipping_mode() const {
	return clipping_mode;
}

void FakeExplosionParticles2D::set_particles_spread_radius(int p_radius) {
	spread_radius = p_radius;
}

int FakeExplosionParticles2D::get_particles_spread_radius() const {
	return spread_radius;
}

void FakeExplosionParticles2D::set_view_centered(bool p_centered) {
	view_centered = p_centered;
	update();
	item_rect_changed();
}

bool FakeExplosionParticles2D::is_view_centered() const {
	return view_centered;
}

void FakeExplosionParticles2D::set_view_size(const Size2 &p_size) {
	if (view_size != p_size) {
		view_size = p_size;
		item_rect_changed();
		emit_signal("view_size_changed");
	}
}

Size2 FakeExplosionParticles2D::get_view_size() const {
	return view_size;
}

void FakeExplosionParticles2D::set_view_group_name(String p_name) {
	group_name = p_name;
}

String FakeExplosionParticles2D::get_view_group_name() const {
	return group_name;
}

void FakeExplosionParticles2D::single_explosion() {
	if (is_inside_tree()) {
		_create_particles();
	}
}

void FakeExplosionParticles2D::repeat_explosion(float p_wait_time, int p_repeat_count) {
	if (is_inside_tree()) {
		ERR_FAIL_COND(_particles_timer == nullptr);

		_repeat_counter = p_repeat_count;
		_particles_timer->set_wait_time(p_wait_time);
		_particles_timer->start();
	}
}

void FakeExplosionParticles2D::_bind_methods() {
	BIND_ENUM_CONSTANT(CLIP_NONE);
	BIND_ENUM_CONSTANT(CLIP_OUTSIDE);
	BIND_ENUM_CONSTANT(CLIP_INSIDE);

	ClassDB::bind_method(D_METHOD("set_min_particles_number"), &FakeExplosionParticles2D::set_min_particles_number);
	ClassDB::bind_method(D_METHOD("get_min_particles_number"), &FakeExplosionParticles2D::get_min_particles_number);
	ClassDB::bind_method(D_METHOD("set_max_particles_number"), &FakeExplosionParticles2D::set_max_particles_number);
	ClassDB::bind_method(D_METHOD("get_max_particles_number"), &FakeExplosionParticles2D::get_max_particles_number);
	ClassDB::bind_method(D_METHOD("set_min_particles_gravity"), &FakeExplosionParticles2D::set_min_particles_gravity);
	ClassDB::bind_method(D_METHOD("get_min_particles_gravity"), &FakeExplosionParticles2D::get_min_particles_gravity);
	ClassDB::bind_method(D_METHOD("set_max_particles_gravity"), &FakeExplosionParticles2D::set_max_particles_gravity);
	ClassDB::bind_method(D_METHOD("get_max_particles_gravity"), &FakeExplosionParticles2D::get_max_particles_gravity);
	ClassDB::bind_method(D_METHOD("set_min_particles_velocity"), &FakeExplosionParticles2D::set_min_particles_velocity);
	ClassDB::bind_method(D_METHOD("get_min_particles_velocity"), &FakeExplosionParticles2D::get_min_particles_velocity);
	ClassDB::bind_method(D_METHOD("set_max_particles_velocity"), &FakeExplosionParticles2D::set_max_particles_velocity);
	ClassDB::bind_method(D_METHOD("get_max_particles_velocity"), &FakeExplosionParticles2D::get_max_particles_velocity);
	ClassDB::bind_method(D_METHOD("set_min_particles_size"), &FakeExplosionParticles2D::set_min_particles_size);
	ClassDB::bind_method(D_METHOD("get_min_particles_size"), &FakeExplosionParticles2D::get_min_particles_size);
	ClassDB::bind_method(D_METHOD("set_max_particles_size"), &FakeExplosionParticles2D::set_max_particles_size);
	ClassDB::bind_method(D_METHOD("get_max_particles_size"), &FakeExplosionParticles2D::get_max_particles_size);
	ClassDB::bind_method(D_METHOD("set_emitter_offset"), &FakeExplosionParticles2D::set_emitter_offset);
	ClassDB::bind_method(D_METHOD("get_emitter_offset"), &FakeExplosionParticles2D::get_emitter_offset);
	ClassDB::bind_method(D_METHOD("set_particles_explode"), &FakeExplosionParticles2D::set_particles_explode);
	ClassDB::bind_method(D_METHOD("get_particles_explode"), &FakeExplosionParticles2D::get_particles_explode);
	ClassDB::bind_method(D_METHOD("set_particles_spread_radius"), &FakeExplosionParticles2D::set_particles_spread_radius);
	ClassDB::bind_method(D_METHOD("get_particles_spread_radius"), &FakeExplosionParticles2D::get_particles_spread_radius);
	ClassDB::bind_method(D_METHOD("set_particles_clipping_mode"), &FakeExplosionParticles2D::set_particles_clipping_mode);
	ClassDB::bind_method(D_METHOD("get_particles_clipping_mode"), &FakeExplosionParticles2D::get_particles_clipping_mode);
	ClassDB::bind_method(D_METHOD("set_view_size"), &FakeExplosionParticles2D::set_view_size);
	ClassDB::bind_method(D_METHOD("get_view_size"), &FakeExplosionParticles2D::get_view_size);
	ClassDB::bind_method(D_METHOD("set_view_centered"), &FakeExplosionParticles2D::set_view_centered);
	ClassDB::bind_method(D_METHOD("is_view_centered"), &FakeExplosionParticles2D::is_view_centered);
	ClassDB::bind_method(D_METHOD("set_view_group_name"), &FakeExplosionParticles2D::set_view_group_name);
	ClassDB::bind_method(D_METHOD("get_view_group_name"), &FakeExplosionParticles2D::get_view_group_name);

	ClassDB::bind_method(D_METHOD("set_emitter_position"), &FakeExplosionParticles2D::set_emitter_position);
	ClassDB::bind_method(D_METHOD("single_explosion"), &FakeExplosionParticles2D::single_explosion);
	ClassDB::bind_method(D_METHOD("repeat_explosion", "wait_time", "repeat_count"), &FakeExplosionParticles2D::repeat_explosion);

	ClassDB::bind_method(D_METHOD("_on_particles_timer_timeout"), &FakeExplosionParticles2D::_on_particles_timer_timeout);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "min_particles_number"), "set_min_particles_number", "get_min_particles_number");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_particles_number"), "set_max_particles_number", "get_max_particles_number");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "min_particles_gravity"), "set_min_particles_gravity", "get_min_particles_gravity");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "max_particles_gravity"), "set_max_particles_gravity", "get_max_particles_gravity");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "min_particles_velocity"), "set_min_particles_velocity", "get_min_particles_velocity");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "max_particles_velocity"), "set_max_particles_velocity", "get_max_particles_velocity");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "min_particles_size"), "set_min_particles_size", "get_min_particles_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_particles_size"), "set_max_particles_size", "get_max_particles_size");
	ADD_GROUP("Particles", "particles_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "particles_explode"), "set_particles_explode", "get_particles_explode");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "particles_emitter_offset"), "set_emitter_offset", "get_emitter_offset");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "particles_spread_radius"), "set_particles_spread_radius", "get_particles_spread_radius");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "particles_clipping", PROPERTY_HINT_ENUM, "None,ClipOutside,ClipInside"), "set_particles_clipping_mode", "get_particles_clipping_mode");
	ADD_GROUP("View", "view_");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "view_size"), "set_view_size", "get_view_size");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "view_centered"), "set_view_centered", "is_view_centered");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "view_group_name"), "set_view_group_name", "get_view_group_name");
	ADD_GROUP("", "");

	ADD_SIGNAL(MethodInfo("view_size_changed"));
}

FakeExplosionParticles2D::FakeExplosionParticles2D() {
	min_particles_number = 20;
	max_particles_number = 40;

	min_particles_gravity = 200.0;
	max_particles_gravity = 600.0;

	min_particles_velocity = 200.0;
	max_particles_velocity = 600.0;

	min_particles_size = 1;
	max_particles_size = 3;

	timer_wait_time = 1.0;
	particles_explode = false;
	clipping_mode = CLIP_NONE;

	emitter_offset = Vector2();
	spread_radius = 50;
	view_size = Size2(100, 100);
	view_centered = true;
	group_name = "fake_explosion_particles";

	_repeat_counter = 0;
}

void RigidBodyParticlesTracker::_on_remover_timeout() {
	if (particle) {
		particle->queue_delete();
	}
}

void RigidBodyParticlesTracker::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			if ((_remover = memnew(Timer))) {
				_remover->set_wait_time(lifetime);
				_remover->connect("timeout", this, "_on_remover_timeout");
				_remover->start();
				add_child(_remover);
			}

			const real_t impulse_angle_rad = Math::deg2rad(impulse_angle + get_global_rotation_degrees());
			const Vector2 impulse_angle_vector = Vector2(Math::cos(impulse_angle_rad), Math::sin(impulse_angle_rad)).normalized();

			const real_t force_angle_rad = Math::deg2rad(force_angle + get_global_rotation_degrees());
			const Vector2 force_angle_vector = Vector2(Math::cos(force_angle_rad), Math::sin(force_angle_rad)).normalized();

			if (particle) {
				particle->apply_impulse(Vector2(0, 0), impulse_angle_vector * impulse);
				particle->add_force(Vector2(0, 0), force_angle_vector * force);
				particle->set_rotation_degrees(initial_rotation);
			}
		} break;
	}
}

void RigidBodyParticlesTracker::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_remover_timeout"), &RigidBodyParticlesTracker::_on_remover_timeout);
}

RigidBodyParticlesTracker::RigidBodyParticlesTracker::RigidBodyParticlesTracker() {
	particle = nullptr;
	lifetime = 0;
	impulse_angle = 0;
	impulse = Vector2();
	force_angle = 0;
	force = Vector2();
	initial_rotation = 0;
}

void RigidBodyParticles2D::_on_shot_timer_timeout() {
	_emit_count = 0;
	_iteration += 1;
	_shot_started = false;

	emit_signal("shot_ended");

	if (one_shot) {
		emitting = false;
	}
}

void RigidBodyParticles2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			if ((_shot_timer = new Timer())) {
				_shot_timer->set_timer_process_mode(Timer::TIMER_PROCESS_IDLE);
				_shot_timer->connect("timeout", this, "_on_shot_timer_timeout");
				_shot_timer->set_one_shot(one_shot);
				_shot_timer->start(lifetime);
				add_child(_shot_timer);
			}
		} break;
		case NOTIFICATION_ENTER_TREE: {
		} break;
		case NOTIFICATION_PHYSICS_PROCESS: {
		} break;
		case NOTIFICATION_PROCESS: {
		} break;
		case NOTIFICATION_DRAW: {
		} break;
	}
}

void RigidBodyParticles2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_shot_timer_timeout"), &RigidBodyParticles2D::_on_shot_timer_timeout);

	ADD_SIGNAL(MethodInfo("shot_started"));
	ADD_SIGNAL(MethodInfo("shot_ended"));
}

RigidBodyParticles2D::RigidBodyParticles2D() {
	emitting = true;
	amount = 8;
	amount_random = 0;
	particle_scene = Ref<PackedScene>(NULL);
	one_shot = false;
	explosiveness = 0;
	tracker_name = "ParticleTracker";
	emission_shape = Ref<Shape2D>(NULL);

	lifetime = 2;
	lifetime_random = 0;

	impulse = 200;
	impulse_random = 0;
	impulse_angle_degrees = 0;
	impulse_spread_degrees = 0;

	force = 0;
	force_random = 0;
	force_angle_degrees = 0;
	force_spread_degrees = 0;

	initial_rotation_degrees = 0;
	initial_rotation_degrees_random = 0;

	_shot_timer = nullptr;
	_emit_timer = nullptr;

	_iteration = 0;
	_tracker = nullptr;
	_capsule_circle_frac = 0;

	// per shot state variables
	_particle_count = 0;
	_emit_count = 0;
	_shot_started = false;
}

#undef rand_range
