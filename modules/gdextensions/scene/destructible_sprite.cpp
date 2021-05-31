/*************************************************************************/
/*  destructible_sprite.cpp                                              */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

// Reference:
// ----------
// https://github.com/mjholtzem/Unity-2D-Destruction.git
// https://github.com/hiulit/Godot-3-2D-Destructible-Objects
// https://github.com/godotengine/godot/issues/31323#issuecomment-520517893
// https://www.reddit.com/r/godot/comments/nimkqg/how_to_break_a_2d_sprite_in_a_cool_and_easy_way/

#include "destructible_sprite.h"
#include "explosion_particles.h"
#include "scene/2d/collision_shape_2d.h"
#include "scene/2d/cpu_particles_2d.h"
#include "scene/2d/particles_2d.h"
#include "scene/animation/tween.h"
#include "scene/main/timer.h"
#include "scene/resources/rectangle_shape_2d.h"

#include "common/gd_core.h"

#include <algorithm>
#include <vector>

#define FSIGN(a) ((a) > 0 ? 1 : -1)
#define FDIFFSIGN(x, y) (((x) > (y)) - ((x) < (y)))

const Vector2 GravityFactor(0, 10);

struct explo_object_t {
	uint64_t id;
	real_t delay;
	DestructibleSprite::DestructionType destruction_type;
	DestructibleSprite::DestructionPhysics destruction_physics;
	std::vector<Node2D *> blocks;
	Node2D *blocks_container;
	real_t blocks_gravity_scale;
	real_t blocks_impulse;
	int blocks_per_side;
	Vector2 collision_extents;
	int collision_layers;
	int collision_masks;
	bool collision_one_way;
	Point2 collision_position;
	Timer *debris_timer;
	bool remove_debris;
};

static inline CollisionShape2D *_safe_collision_shape_2d(Node *node) {
	if (CollisionShape2D *collision = Object::cast_to<CollisionShape2D>(node)) {
		return collision;
	} else {
		WARN_PRINT(vformat("%s: Collision object not found.", node->get_name()));
		return nullptr;
	}
}

static inline Sprite *_safe_sprite(Node *node) {
	if (Sprite *collision = Object::cast_to<Sprite>(node)) {
		return collision;
	} else {
		WARN_PRINT(vformat("%s: Sprite object not found.", node->get_name()));
		return nullptr;
	}
}

static inline real_t _get_random_time() {
	Math::randomize();
	return Math::random(0.25, 0.5);
}

static inline Vector2 _get_random_velocity(real_t initial_velocity = 500) {
	const real_t min_particles_velocity = initial_velocity * 0.8;
	const real_t max_particles_velocity = initial_velocity;
	Math::randomize();
	return Vector2(
			Math::random(-min_particles_velocity * 0.2, max_particles_velocity * 0.2),
			Math::random(-min_particles_velocity, -max_particles_velocity));
}

static inline Vector2 _get_random_velocity_increment() {
	Math::randomize();
	return Vector2(
			Math::random(0.901, 1.005), Math::random(0.92, 0.98));
}

void DestructibleSprite::_on_opacity_tween_completed(Object *obj, String key) {
	ERR_FAIL_NULL(obj);
	const uint64_t object_id = obj->get_meta("simulation_id");
	ERR_FAIL_COND(_simulations.count(object_id) == 0);

	if (Node *node = Object::cast_to<Node>(obj)) {
		ERR_FAIL_NULL(node->get_parent());

		explo_object_t *object = _simulations[object_id];
		object->blocks.erase(
				std::remove(object->blocks.begin(), object->blocks.end(), node->get_parent()), object->blocks.end());
		if (object->blocks.empty()) {
			object->id = 0; // dead
			if (debug_mode) {
				DEBUG_PRINT(vformat("[%s/opacity_tween_completed] explosion 0x%x removed.", get_name(), object_id));
			}
		}
	}
}

void DestructibleSprite::_on_debris_timer_timeout(uint64_t object_id) {
	ERR_FAIL_COND(object_id == 0);
	ERR_FAIL_COND(_simulations.count(object_id) == 0);

	const explo_object_t &object = *_simulations[object_id];

	if (debug_mode) {
		DEBUG_PRINT(vformat("[%s/debris_timer_timeout] object's debris timer timed out.", get_name()));
	}

	for (auto *block : object.blocks) {
		if (RigidBody2D *body = Object::cast_to<RigidBody2D>(block)) {
			if (object.remove_debris) {
				if (Sprite *sprite = _safe_sprite(body->get_child(0))) {
					Color modulate_color = sprite->get_modulate();
					Tween *opacity_tween = memnew(Tween);
					opacity_tween->connect("tween_completed", this, "_on_opacity_tween_completed");
					opacity_tween->interpolate_property(
							sprite,
							NodePath("modulate"),
							modulate_color,
							modulate_color.with_alpha(0.0),
							Math::random(0.0, 1.0),
							Tween::TRANS_LINEAR,
							Tween::EASE_IN);
					object.blocks_container->add_child(opacity_tween, true);
					opacity_tween->start();
				}
			} else {
				body->set_mode(RigidBody2D::MODE_STATIC);
				if (CollisionShape2D *collision = _safe_collision_shape_2d(body->get_child(1))) {
					collision->set_disabled(true);
				}
				if (debug_mode) {
					DEBUG_PRINT(vformat("[%s/debris_timer_timeout] explosion 0x%x completed.", get_name(), object_id));
				}
			}
		}
	}
}

void DestructibleSprite::_initiate_detonation(uint64_t object_id) {
	ERR_FAIL_COND(object_id == 0);
	ERR_FAIL_COND(_simulations.count(object_id) == 0);

	const explo_object_t &object = *_simulations[object_id];

	// add blocks childeren
	for (size_t i = 0; i < object.blocks.size(); i++) {
		object.blocks_container->add_child(object.blocks[i], true);
	}

	add_child(object.blocks_container, true);

	if (debug_mode) {
		DEBUG_PRINT(vformat("[%s/initiate_detonate] %d blocks and 1 container added to hierarchy", get_name(), int(object.blocks.size())));
	}

	// start connected particles
	for (int i = 0; i < get_child_count(); i++) {
		Node *child = get_child(i);
		if (Particles2D *particles2d = Object::cast_to<Particles2D>(child)) {
			particles2d->set_emitting(true);
		} else if (CPUParticles2D *particlescpu = Object::cast_to<CPUParticles2D>(child)) {
			particlescpu->set_emitting(true);
		} else if (FakeExplosionParticles2D *fakeparticles = Object::cast_to<FakeExplosionParticles2D>(child)) {
			fakeparticles->single_explosion();
		}
	}

	// initiate blocks
	if (object.destruction_physics == DESTRUCTION_PHYSICS_OFF) {
		for (auto *block : object.blocks) {
			const real_t block_scale = Math::random(0.5, 1.2);
			if (Sprite *sprite = Object::cast_to<Sprite>(block)) {
				sprite->set_scale(Vector2(block_scale, block_scale));
			}
			if (random_depth) {
				block->set_z_index(Math::randf() < 0.5 ? 0 : -1);
			}
		}
	} else {
		for (auto *block : object.blocks) {
			if (RigidBody2D *body = Object::cast_to<RigidBody2D>(block)) {
				const real_t child_gravity_scale = object.blocks_gravity_scale;
				body->set_gravity_scale(child_gravity_scale);

				const real_t block_scale = Math::random(0.5, 1.5);
				if (Sprite *sprite = _safe_sprite(body->get_child(0))) {
					sprite->set_scale(Vector2(block_scale, block_scale));
					// color
					const float child_color = Math::random(100, 255) / 255;
					Tween *color_tween = memnew(Tween);
					color_tween->interpolate_property(
							sprite,
							NodePath("modulate"),
							Color(1.0, 1.0, 1.0, 1.0),
							Color(child_color, child_color, child_color, 1.0),
							0.25,
							Tween::TRANS_LINEAR,
							Tween::EASE_IN);
					add_child(color_tween, true);
					color_tween->start();
				}
				if (CollisionShape2D *collision = _safe_collision_shape_2d(body->get_child(1))) {
					collision->set_scale(Vector2(block_scale, block_scale));
				}
				body->set_mass(block_scale);
				body->set_collision_layer(Math::randf() < 0.5 ? 0 : object.collision_layers);
				body->set_collision_mask(Math::randf() < 0.5 ? 0 : object.collision_masks);

				if (random_depth) {
					body->set_z_index(Math::randf() < 0.5 ? 0 : -1);
				}
				body->set_mode(RigidBody2D::MODE_RIGID);
				// trigger impulse
				//body->apply_central_impulse(Vector2(Math::random(-object.blocks_impulse, object.blocks_impulse), -object.blocks_impulse));
			} else {
				WARN_PRINT("Block should be RigidBody2D, not a " + block->get_class());
			}
		}

		object.debris_timer->start();
	}
}

void DestructibleSprite::_prepare_detonation(explo_object_t &object) {
	object.destruction_type = destruction_type;
	object.destruction_physics = destruction_physics;
	object.blocks_impulse = blocks_impulse;
	object.blocks_gravity_scale = blocks_gravity_scale;
	object.collision_one_way = collision_one_way;
	object.collision_layers = collision_layers;
	object.collision_masks = collision_masks;
	object.remove_debris = remove_debris;

	if (randomize_seed) {
		Math::randomize();
	}

	object.blocks_container = memnew(Node2D);

	// Set the debris timer.
	object.debris_timer = memnew(Timer);
	object.debris_timer->connect("timeout", this, "_on_debris_timer_timeout", varray(object.id));
	object.debris_timer->set_one_shot(true);
	object.debris_timer->set_wait_time(debris_max_time);
	object.debris_timer->set_name("debris_timer");
	object.blocks_container->add_child(object.debris_timer, true);

	// Check if the sprite is using 'Region' to get the proper width and height.
	real_t object_width, object_height;
	if (is_region()) {
		object_width = get_region_rect().size.width;
		object_height = get_region_rect().size.height;
	} else {
		object_width = get_texture()->get_width();
		object_height = get_texture()->get_height();
	}

	// Get the number of blocks in horiontal and vertical.
	const real_t block_size = MAX(object_width, object_height) / blocks_per_side;
	int object_hframes = object_width / block_size;
	int object_vframes = object_height / block_size;

	if (debug_mode) {
		DEBUG_PRINT(vformat("[%s/prepare_detonation] object's blocks per side: %d x %d (block size %d, physics %s)",
				get_name(), object_hframes, object_vframes, block_size,
				destruction_physics == DESTRUCTION_PHYSICS_OFF ? "off" : "on"));
	}

	// Check if the sprite is centered to get the offset.
	Vector2 object_offset = Vector2(block_size, block_size) / -2;
	if (is_centered()) {
		object_offset += Vector2(object_width, object_height) / 2;
	}

	object.collision_extents = Vector2((object_width / 2) / object_hframes, (object_height / 2) / object_vframes);
	object.collision_position = Vector2((ceil(object.collision_extents.x) - object.collision_extents.x) * -1,
			(ceil(object.collision_extents.y) - object.collision_extents.y) * -1);

	object.blocks.clear();

	for (int n = 0; n < object_vframes * object_hframes; n++) {
		Node2D *duplicated_object = Object::cast_to<Node2D>(duplicate(DUPLICATE_USE_INSTANCING));
		duplicated_object->set_name(vformat("%s_block_%d", get_name(), n)); // Add a unique name to each block
		if (Sprite *sprite = Object::cast_to<Sprite>(duplicated_object)) {
			sprite->set_vframes(object_vframes);
			sprite->set_hframes(object_hframes);
			sprite->set_frame(n);
			// duplicates will be a child of this sprite
			sprite->set_position(Vector2(0, 0));
			sprite->set_scale(Vector2(1, 1));
			// always centered
			sprite->set_centered(true);
		}

		if (debug_mode) {
			const float overlay[] = { 0.4, 1 };
			duplicated_object->set_modulate(Color::solid(overlay[(n / object_hframes) % 2 == 0 ? n % 2 == 0 : n % 2 != 0], 0.9));
		}

		duplicated_object->set_meta("simulation_id", object.id);

		if (object.destruction_physics == DESTRUCTION_PHYSICS_OFF) {
			duplicated_object->set_meta("time", 0);
			duplicated_object->set_meta("velocity", _get_random_velocity(blocks_impulse));
			duplicated_object->set_meta("increment", _get_random_velocity_increment());
			object.blocks.push_back(duplicated_object); // Just append it to the blocks array
		} else {
			// Root node
			RigidBody2D *root_object = memnew(RigidBody2D);
			root_object->set_mode(RigidBody2D::MODE_STATIC);
			// Create a new collision shape for each block
			RectangleShape2D *shape = memnew(RectangleShape2D);
			shape->set_extents(object.collision_extents);
			CollisionShape2D *collision = memnew(CollisionShape2D);
			collision->set_shape(shape);
			collision->set_position(object.collision_position);
			if (object.collision_one_way) {
				collision->set_one_way_collision(true);
			}
			root_object->add_child(duplicated_object);
			root_object->add_child(collision);
			object.blocks.push_back(root_object); // Append it to the blocks array
		}
	}

	int object_frame = 0;

	// Position each block in place to create the whole sprite.
	const Vector2 cell = Vector2(object_width / object_hframes, object_height / object_vframes);
	for (int y = 0; y < object_vframes; y++) {
		for (int x = 0; x < object_hframes; x++) {
			Vector2 position = Vector2(x, y) * cell - object_offset;
			if (object.destruction_physics != DESTRUCTION_PHYSICS_OFF) {
				position += object.collision_extents;
			}
			object.blocks[object_frame]->set_position(position);
			if (debug_mode) {
				DEBUG_PRINT(vformat("  block %d position: %s", object_frame, position));
			}
			object_frame++;
		}
	}

	call_deferred("_initiate_detonation", object.id);
}

void DestructibleSprite::_simulate_particles(explo_object_t &object, real_t delta) {
	ERR_FAIL_COND(object.id == 0);
	ERR_FAIL_COND(object.blocks.size() == 0);

	bool dead = true;
	for (auto *block : object.blocks) {
		if (block->get_modulate().a > 0) {
			const Vector2 gravity = GravityFactor * object.blocks_gravity_scale;
			const Vector2 velocity = (Vector2)block->get_meta("velocity") * (Vector2)block->get_meta("increment");
			real_t time = block->get_meta("time");
			const Vector2 position = block->get_position() + (velocity + gravity) * delta;

			block->set_position(position);
			block->set_rotation(2 * (object.id & 1 ? time : -time));
			time += delta;

			if (time > _get_random_time()) {
				// fade out the particles
				Color c = block->get_modulate();
				if (c.a > 0) {
					c.a -= delta;
				}
				if (c.a < 0) {
					c.a = 0;
				}
				block->set_modulate(c);
				// if the particle is invisible ...
				if (dead) {
					dead &= block->get_modulate().a == 0;
				}
			} else {
				dead = false;
			}
			block->set_meta("velocity", velocity);
			block->set_meta("time", time);
		}
	}
	if (dead) {
		object.id = 0;
	}
}

void DestructibleSprite::set_destruction_types(DestructionType p_type) {
	destruction_type = p_type;
}

void DestructibleSprite::set_destruction_physics(DestructionPhysics p_physics) {
	destruction_physics = p_physics;
}

void DestructibleSprite::set_blocks_per_side(int p_blocks_per_side) {
	blocks_per_side = p_blocks_per_side;
}

void DestructibleSprite::set_blocks_impulse(real_t p_blocks_impulse) {
	ERR_FAIL_COND(p_blocks_impulse <= 0);
	ERR_FAIL_COND(p_blocks_impulse > 1000);

	blocks_impulse = p_blocks_impulse;
}

void DestructibleSprite::set_blocks_gravity_scale(real_t p_blocks_gravity_scale) {
	blocks_gravity_scale = p_blocks_gravity_scale;
}

void DestructibleSprite::set_debris_max_time(real_t p_debris_max_time) {
	debris_max_time = p_debris_max_time;
}

void DestructibleSprite::set_remove_debris(bool p_remove_debris) {
	remove_debris = p_remove_debris;
}

void DestructibleSprite::set_collision_layers(int p_collision_layers) {
	collision_layers = p_collision_layers;
}

void DestructibleSprite::set_collision_masks(int p_collision_masks) {
	collision_masks = p_collision_masks;
}

void DestructibleSprite::set_collision_one_way(bool p_collision_one_way) {
	collision_one_way = p_collision_one_way;
}

void DestructibleSprite::set_randomize_seed(bool p_randomize_seed) {
	randomize_seed = p_randomize_seed;
}

void DestructibleSprite::set_debug_mode(bool p_debug_mode) {
	debug_mode = p_debug_mode;
}

void DestructibleSprite::explode(real_t delay) {
	ERR_FAIL_COND(delay < 0);

	explo_object_t *object = memnew(explo_object_t);
	object->id = reinterpret_cast<uint64_t>(object);
	object->delay = delay;

	_simulations[object->id] = object;

	set_process(true);
}

bool DestructibleSprite::is_active_explosions() {
	return _simulations.size() > 0;
}

int DestructibleSprite::explosions_in_progress() {
	return _simulations.size();
}

void DestructibleSprite::reset() {
	if (_simulations.empty()) {
		_disabled_base_notifications.erase(NOTIFICATION_DRAW); // bring back original sprite
		update();
	} else {
		_reset_at_end = true;
	}
}

#define _erasekey(map, id)  \
	{                       \
		memdelete(map[id]); \
		map.erase(id);      \
	}

void DestructibleSprite::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
		} break;
		case NOTIFICATION_DRAW: {
		} break;
		case NOTIFICATION_PARENTED:
		case NOTIFICATION_ENTER_TREE: {
		} break;
		case NOTIFICATION_PROCESS: {
			const real_t delta = get_process_delta_time();
			std::vector<uint64_t> dead;
			for (auto &s : _simulations) {
				const uint64_t id = s.first;
				explo_object_t &object = *s.second;
				if (object.id == 0) {
					dead.push_back(id);
				} else if (object.blocks.empty()) {
					if (object.delay <= 0) {
						_prepare_detonation(object);
						_disabled_base_notifications.append(NOTIFICATION_DRAW); // we want only blocks to be displayed
					} else {
						object.delay -= delta;
					}
				} else {
					if (object.destruction_physics == DESTRUCTION_PHYSICS_OFF) {
						// simulate particles
						_simulate_particles(object, delta);
					}
				}
			}
			if (!dead.empty()) {
				for (const auto &id : dead) {
					_simulations[id]->blocks_container->queue_delete();
					_erasekey(_simulations, id);
				}
			}
			if (_simulations.empty()) {
				set_process(false); // nothing to process
				if (_reset_at_end) {
					_disabled_base_notifications.erase(NOTIFICATION_DRAW); // bring back original sprite
					_reset_at_end = false;
					update();
				}
			}
			if (debug_mode) {
				update();
			}
		} break;
	}
}

void DestructibleSprite::_bind_methods() {
	BIND_ENUM_CONSTANT(DESTRUCTION_EXPLODE);
	BIND_ENUM_CONSTANT(DESTRUCTION_COLLAPSE);
	BIND_ENUM_CONSTANT(DESTRUCTION_PHYSICS_OFF);
	BIND_ENUM_CONSTANT(DESTRUCTION_PHYSICS_STANDARD);
	BIND_ENUM_CONSTANT(DESTRUCTION_PHYSICS_HIGH);

	ClassDB::bind_method(D_METHOD("set_destruction_types", "type"), &DestructibleSprite::set_destruction_types);
	ClassDB::bind_method(D_METHOD("get_destruction_types"), &DestructibleSprite::get_destruction_types);
	ClassDB::bind_method(D_METHOD("set_destruction_physics", "physics"), &DestructibleSprite::set_destruction_physics);
	ClassDB::bind_method(D_METHOD("get_destruction_physics"), &DestructibleSprite::get_destruction_physics);
	ClassDB::bind_method(D_METHOD("set_blocks_per_side", "per_side"), &DestructibleSprite::set_blocks_per_side);
	ClassDB::bind_method(D_METHOD("get_blocks_per_side"), &DestructibleSprite::get_blocks_per_side);
	ClassDB::bind_method(D_METHOD("set_blocks_impulse", "impulse"), &DestructibleSprite::set_blocks_impulse);
	ClassDB::bind_method(D_METHOD("get_blocks_impulse"), &DestructibleSprite::get_blocks_impulse);
	ClassDB::bind_method(D_METHOD("set_blocks_gravity_scale", "scale"), &DestructibleSprite::set_blocks_gravity_scale);
	ClassDB::bind_method(D_METHOD("get_blocks_gravity_scale"), &DestructibleSprite::get_blocks_gravity_scale);
	ClassDB::bind_method(D_METHOD("set_debris_max_time", "max_time"), &DestructibleSprite::set_debris_max_time);
	ClassDB::bind_method(D_METHOD("get_debris_max_time"), &DestructibleSprite::get_debris_max_time);
	ClassDB::bind_method(D_METHOD("set_remove_debris", "remove_debris"), &DestructibleSprite::set_remove_debris);
	ClassDB::bind_method(D_METHOD("get_remove_debris"), &DestructibleSprite::get_remove_debris);
	ClassDB::bind_method(D_METHOD("set_collision_layers", "layers"), &DestructibleSprite::set_collision_layers);
	ClassDB::bind_method(D_METHOD("get_collision_layers"), &DestructibleSprite::get_collision_layers);
	ClassDB::bind_method(D_METHOD("set_collision_masks", "mask"), &DestructibleSprite::set_collision_masks);
	ClassDB::bind_method(D_METHOD("get_collision_masks"), &DestructibleSprite::get_collision_masks);
	ClassDB::bind_method(D_METHOD("set_collision_one_way", "one_way"), &DestructibleSprite::set_collision_one_way);
	ClassDB::bind_method(D_METHOD("get_collision_one_way"), &DestructibleSprite::get_collision_one_way);
	ClassDB::bind_method(D_METHOD("set_randomize_seed", "seed"), &DestructibleSprite::set_randomize_seed);
	ClassDB::bind_method(D_METHOD("get_randomize_seed"), &DestructibleSprite::get_randomize_seed);
	ClassDB::bind_method(D_METHOD("set_debug_mode", "mode"), &DestructibleSprite::set_debug_mode);
	ClassDB::bind_method(D_METHOD("get_debug_mode"), &DestructibleSprite::get_debug_mode);

	ClassDB::bind_method(D_METHOD("explode", "delay"), &DestructibleSprite::explode);
	ClassDB::bind_method(D_METHOD("reset"), &DestructibleSprite::reset);
	ClassDB::bind_method(D_METHOD("explosions_in_progress"), &DestructibleSprite::explosions_in_progress);
	ClassDB::bind_method(D_METHOD("is_active_explosions"), &DestructibleSprite::is_active_explosions);

	ClassDB::bind_method(D_METHOD("_on_debris_timer_timeout", "object_id"), &DestructibleSprite::_on_debris_timer_timeout);
	ClassDB::bind_method(D_METHOD("_on_opacity_tween_completed", "object", "key"), &DestructibleSprite::_on_opacity_tween_completed);
	ClassDB::bind_method(D_METHOD("_initiate_detonation", "object_id"), &DestructibleSprite::_initiate_detonation);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "destruction_type", PROPERTY_HINT_ENUM, "Explode,Collapse"), "set_destruction_types", "get_destruction_types");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "destruction_physics", PROPERTY_HINT_ENUM, "Off,Standard,High"), "set_destruction_physics", "get_destruction_physics");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "blocks_per_side"), "set_blocks_per_side", "get_blocks_per_side");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "blocks_impulse", PROPERTY_HINT_RANGE, "0,1000,50"), "set_blocks_impulse", "get_blocks_impulse");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "blocks_gravity_scale"), "set_blocks_gravity_scale", "get_blocks_gravity_scale");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "debris_max_time"), "set_debris_max_time", "get_debris_max_time");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "remove_debris"), "set_remove_debris", "get_remove_debris");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_layers"), "set_collision_layers", "get_collision_layers");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_masks"), "set_collision_masks", "get_collision_masks");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "collision_one_way"), "set_collision_one_way", "get_collision_one_way");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "randomize_seed"), "set_randomize_seed", "get_randomize_seed");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug_mode"), "set_debug_mode", "get_debug_mode");
}

DestructibleSprite::DestructibleSprite() {
	_reset_at_end = false;
	destruction_type = DESTRUCTION_COLLAPSE;
	destruction_physics = DESTRUCTION_PHYSICS_STANDARD;
	blocks_per_side = 6;
	blocks_impulse = 600;
	blocks_gravity_scale = 10;
	debris_max_time = 5;
	remove_debris = false;
	collision_layers = 1;
	collision_masks = 1;
	collision_one_way = false;
	random_depth = true;
	randomize_seed = false;
	debug_mode = false;
}

DestructibleSprite::~DestructibleSprite() {
}
