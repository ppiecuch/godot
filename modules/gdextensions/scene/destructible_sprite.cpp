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

#include "destructible_sprite.h"
#include "explosion_particles.h"
#include "scene/2d/collision_shape_2d.h"
#include "scene/2d/cpu_particles_2d.h"
#include "scene/2d/particles_2d.h"
#include "scene/animation/tween.h"
#include "scene/main/timer.h"
#include "scene/resources/rectangle_shape_2d.h"

#include <vector>

struct explo_object_t {
	uint64_t id;
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
	bool detonate;
	int frame;
	bool can_detonate;
	bool has_detonated;
	int height, width;
	int hframes, vframes;
	Vector2 offset;
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

void DestructibleSprite::_explosion(real_t delta, explo_object_t &object) {
	if (object.detonate) {
		if (debug_mode) {
			print_line(vformat("'%s' object exploded!", get_name()));
		}

		for (int i = 0; i < object.blocks_container->get_child_count(); i++) {
			RigidBody2D *child = Object::cast_to<RigidBody2D>(object.blocks_container->get_child(i));
			child->apply_central_impulse(Vector2(Math::random(-object.blocks_impulse, object.blocks_impulse), -object.blocks_impulse));
		}

		// Add a delay before setting 'object.detonate' to 'false'.
		// Sometimes 'object.detonate' is set to 'false' so quickly that the explosion never happens.
		// If this happens, try setting 'explosion_delay' to 'true'.
		if (explosion_delay) {
			// Removed the yield timer because it was throwing
			// 'Resumed after yield, but class instance is gone' errors
			// when freeing the blocks.
			// yield(get_tree().create_timer(delta), "timeout")
			_explosion_delay_timer_limit = delta;
			_explosion_delay_timer += delta;
			if (_explosion_delay_timer > _explosion_delay_timer_limit) {
				_explosion_delay_timer -= _explosion_delay_timer_limit;
				object.detonate = false;
			}
		} else {
			object.detonate = false;
		}
	}
}

void DestructibleSprite::_on_debris_timer_timeout(uint64_t object_id) {
	ERR_FAIL_COND(_simulations.count(object_id) == 0);

	const explo_object_t &object = *_simulations[object_id];

	if (debug_mode) {
		print_line(vformat("'%s' object's debris timer timed out.", get_name()));
	}

	for (int i = 0; i < object.blocks_container->get_child_count(); i++) {
		Node *child = object.blocks_container->get_child(i);
		Sprite *child_sprite = nullptr;
		if (RigidBody2D *body = Object::cast_to<RigidBody2D>(child)) {
			if (object.remove_debris) {
				if (Sprite *sprite = _safe_sprite(body->get_child(0))) {
					child_sprite = sprite;
				}
			} else {
				body->set_mode(RigidBody2D::MODE_STATIC);
				if (CollisionShape2D *collision = _safe_collision_shape_2d(body->get_child(1))) {
					collision->set_disabled(true);
				}
			}
		} else if (object.remove_debris) {
			child_sprite = _safe_sprite(child);
		}
		if (child_sprite) {
			Color modulate_color = child_sprite->get_modulate();
			Tween *opacity_tween = memnew(Tween);
			opacity_tween->connect("tween_completed", this, "_on_opacity_tween_completed");
			opacity_tween->interpolate_property(
					child_sprite,
					NodePath("modulate"),
					modulate_color,
					modulate_color.with_alpha(0.0),
					Math::random(0.0, 1.0),
					Tween::TRANS_LINEAR,
					Tween::EASE_IN);
			add_child(opacity_tween, true);
			opacity_tween->start();
		}
	}
}

void DestructibleSprite::_detonate(explo_object_t &object) {
	for (int i = 0; i < get_child_count(); i++) {
		Node *child = get_child(i);
		if (Particles2D *particles = Object::cast_to<Particles2D>(child)) {
			particles->set_emitting(true);
		} else if (CPUParticles2D *particles = Object::cast_to<CPUParticles2D>(child)) {
			particles->set_emitting(true);
		} else if (FakeExplosionParticles2D *particles = Object::cast_to<FakeExplosionParticles2D>(child)) {
			particles->set_particles_explode(true);
		}
	}

	for (int i = 0; i < object.blocks_container->get_child_count(); i++) {
		Node *child = object.blocks_container->get_child(i);

		if (RigidBody2D *body = Object::cast_to<RigidBody2D>(child)) {
			const real_t child_gravity_scale = object.blocks_gravity_scale;
			body->set_gravity_scale(child_gravity_scale);

			const real_t child_scale = Math::random(0.5, 1.5);
			if (Sprite *sprite = _safe_sprite(body->get_child(0))) {
				sprite->set_scale(Vector2(child_scale, child_scale));
				child = sprite; // for tween modulate
			}
			if (CollisionShape2D *collision = _safe_collision_shape_2d(body->get_child(1))) {
				collision->set_scale(Vector2(child_scale, child_scale));
			}
			body->set_mass(child_scale);
			body->set_collision_layer(Math::randf() < 0.5 ? 0 : object.collision_layers);
			body->set_collision_mask(Math::randf() < 0.5 ? 0 : object.collision_masks);

			if (random_depth) {
				body->set_z_index(Math::randf() < 0.5 ? 0 : -1);
			}
			body->set_mode(RigidBody2D::MODE_RIGID);
		}

		if (Sprite *sprite = Object::cast_to<Sprite>(child)) {
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
	}

	object.debris_timer->start();
}

void DestructibleSprite::_add_children(const explo_object_t &child_object) {
	for (int i = 0; i < child_object.blocks.size(); i++) {
		child_object.blocks_container->add_child(child_object.blocks[i], true);
	}

	add_child(child_object.blocks_container, true);
}

void DestructibleSprite::_setup(explo_object_t &object) {
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
	object.debris_timer->connect("timeout", this, "_on_debris_timer_timeout");
	object.debris_timer->set_one_shot(true);
	object.debris_timer->set_wait_time(debris_max_time);
	object.debris_timer->set_name("debris_timer");
	add_child(object.debris_timer, true);

	// Check if the sprite is using 'Region' to get the proper width and height.
	if (is_region()) {
		object.width = get_region_rect().size.x;
		object.height = get_region_rect().size.y;
	} else {
		object.width = get_texture()->get_width();
		object.height = get_texture()->get_height();
	}

	// Check if the sprite is centered to get the offset.
	if (is_centered()) {
		object.offset = Vector2(object.width / 2, object.height / 2);
	}

	object.collision_extents = Vector2((object.width / 2) / object.hframes, (object.height / 2) / object.vframes);
	object.collision_position = Vector2((ceil(object.collision_extents.x) - object.collision_extents.x) * -1,
			(ceil(object.collision_extents.y) - object.collision_extents.y) * -1);

	object.blocks.clear();

	for (int n = 0; n < object.vframes * object.hframes; n++) {
		Node2D *duplicated_object = Object::cast_to<Node2D>(duplicate(DUPLICATE_USE_INSTANCING));
		duplicated_object->set_name(vformat("%s_block_%d", get_name(), n)); // Add a unique name to each block
		if (Sprite *sprite = Object::cast_to<Sprite>(duplicated_object)) {
			sprite->set_vframes(object.vframes);
			sprite->set_hframes(object.hframes);
			sprite->set_frame(n);
		}

		if (destruction_physics == DESTRUCTION_PHYSICS_OFF) {
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

		if (debug_mode) {
			object.blocks[n]->set_modulate(Color(Math::random(0, 1), Math::random(0, 1), Math::random(0, 1), 0.9));
		}
	}

	object.frame = 0;

	// Position each block in place to create the whole sprite.
	for (int x = 0; x < object.hframes; x++) {
		for (int y = 0; y < object.vframes; y++) {
			object.blocks[object.frame]->set_position(Vector2(
					y * (object.width / object.hframes) - object.offset.x + object.collision_extents.x + get_position().x,
					x * (object.height / object.vframes) - object.offset.y + object.collision_extents.y + get_position().y));
			if (debug_mode) {
				print_line(vformat("object[%d] position: %s", object.frame, object.blocks[object.frame]->get_position()));
			}

			object.frame++;
		}
	}

	call_deferred("add_children", object.id);
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

void DestructibleSprite::set_explosion_delay(bool p_explosion_delay) {
	explosion_delay = p_explosion_delay;
}

void DestructibleSprite::set_randomize_seed(bool p_randomize_seed) {
	randomize_seed = p_randomize_seed;
}

void DestructibleSprite::set_debug_mode(bool p_debug_mode) {
	debug_mode = p_debug_mode;
}

void DestructibleSprite::explode() {
	explo_object_t *object = memnew(explo_object_t);
	object->id = reinterpret_cast<uint64_t>(object);

	_setup(*object);
}

void DestructibleSprite::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
		} break;
		case NOTIFICATION_PARENTED:
		case NOTIFICATION_ENTER_TREE: {
		} break;
		case NOTIFICATION_PROCESS: {
		} break;
		case NOTIFICATION_DRAW: {
		}
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
	ClassDB::bind_method(D_METHOD("set_explosion_delay", "delay"), &DestructibleSprite::set_explosion_delay);
	ClassDB::bind_method(D_METHOD("get_explosion_delay"), &DestructibleSprite::get_explosion_delay);
	ClassDB::bind_method(D_METHOD("set_randomize_seed", "seed"), &DestructibleSprite::set_randomize_seed);
	ClassDB::bind_method(D_METHOD("get_randomize_seed"), &DestructibleSprite::get_randomize_seed);
	ClassDB::bind_method(D_METHOD("set_debug_mode", "mode"), &DestructibleSprite::set_debug_mode);
	ClassDB::bind_method(D_METHOD("get_debug_mode"), &DestructibleSprite::get_debug_mode);

	ClassDB::bind_method(D_METHOD("_on_debris_timer_timeout", "object_id"), &DestructibleSprite::_on_debris_timer_timeout);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "destruction_type", PROPERTY_HINT_ENUM, "Explode,Collapse"), "set_destruction_types", "get_destruction_types");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "destruction_physics", PROPERTY_HINT_ENUM, "Off,Standard,High"), "set_destruction_physics", "get_destruction_physics");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "blocks_per_side"), "set_blocks_per_side", "get_blocks_per_side");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "blocks_impulse"), "set_blocks_impulse", "get_blocks_impulse");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "blocks_gravity_scale"), "set_blocks_gravity_scale", "get_blocks_gravity_scale");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "debris_max_time"), "set_debris_max_time", "get_debris_max_time");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "remove_debris"), "set_remove_debris", "get_remove_debris");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_layers"), "set_collision_layers", "get_collision_layers");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_masks"), "set_collision_masks", "get_collision_masks");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "collision_one_way"), "set_collision_one_way", "get_collision_one_way");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "explosion_delay"), "set_explosion_delay", "get_explosion_delay");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "randomize_seed"), "set_randomize_seed", "get_randomize_seed");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug_mode"), "set_debug_mode", "get_debug_mode");
}

DestructibleSprite::DestructibleSprite() {
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
	explosion_delay = false;
	random_depth = true;
	randomize_seed = false;
	debug_mode = false;
	_explosion_delay_timer = 0;
	_explosion_delay_timer_limit = 0;
}

DestructibleSprite::~DestructibleSprite() {
}
