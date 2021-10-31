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
// https://github.com/ScyDev/Godot-Scripts/blob/master/polygon-merge.gd

#include "destructible_sprite.h"
#include "core/io/config_file.h"
#include "explosion_particles.h"
#include "scene/2d/collision_polygon_2d.h"
#include "scene/2d/collision_shape_2d.h"
#include "scene/2d/cpu_particles_2d.h"
#include "scene/2d/particles_2d.h"
#include "scene/2d/visibility_notifier_2d.h"
#include "scene/animation/tween.h"
#include "scene/main/timer.h"
#include "scene/resources/bit_map.h"
#include "scene/resources/rectangle_shape_2d.h"

#include "common/gd_core.h"

#include <algorithm>
#include <functional>
#include <vector>

#define FSIGN(a) ((a) > 0 ? 1 : -1)
#define FDIFFSIGN(x, y) (((x) > (y)) - ((x) < (y)))

#define SPRITE_CHILD 0
#define VISIBILITY_CHILD 1
#define COLLISIONS_CHILD 2

const Vector2 GravityFactor(0, 10);

struct explo_object_t {
	uint64_t id;
	real_t delay;
	DestructibleSprite::DestructionType destruction_type;
	DestructibleSprite::DestructionPhysics destruction_physics;
	std::vector<Node2D *> blocks;
	Node2D *blocks_container;
	real_t blocks_impulse;
	Vector2 collision_extents;
	Point2 collision_position;
	Timer *debris_timer;
	bool remove_debris;
};

static inline CollisionShape2D *_safe_collision_shape(Node *node) {
	if (CollisionShape2D *collision = Object::cast_to<CollisionShape2D>(node->get_child(COLLISIONS_CHILD))) {
		return collision;
	} else if (Object::cast_to<CollisionPolygon2D>(node->get_child(COLLISIONS_CHILD))) {
		return nullptr;
	} else {
		WARN_PRINT(vformat("%s: Collision object not found.", node->get_name()));
		return nullptr;
	}
}

static inline CollisionPolygon2D *_safe_collision_polygon(Node *node) {
	if (CollisionPolygon2D *collision = Object::cast_to<CollisionPolygon2D>(node->get_child(COLLISIONS_CHILD))) {
		return collision;
	} else if (Object::cast_to<CollisionShape2D>(node->get_child(COLLISIONS_CHILD))) {
		return nullptr;
	} else {
		WARN_PRINT(vformat("%s: Collision object not found.", node->get_name()));
		return nullptr;
	}
}

static inline Sprite *_safe_sprite(Node *node) {
	if (Sprite *sprite = Object::cast_to<Sprite>(node->get_child(SPRITE_CHILD))) {
		return sprite;
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

void DestructibleSprite::_texture_changed() {
	_outline_cache_dirty = true;
}

void DestructibleSprite::_update_texture_shapes() {
	if (get_texture().is_null()) {
		// nothing to do - keep old cache
		return;
	}

	Ref<Image> image = get_texture()->get_data();
	ERR_FAIL_COND(image.is_null());

	if (image->is_compressed()) {
		image->decompress();
	}

	Rect2 image_rect = is_region() ? get_region_rect() : Rect2(Point2(), image->get_size());
	const real_t block_size = image_rect.size.max() / blocks_per_side;
	const Size2 object_frames = image_rect.size / block_size;
	const int object_hframes = object_frames.width;
	const int object_vframes = object_frames.height;
	const Size2 cell_size = image_rect.size / Size2(object_hframes, object_vframes);

	if (object_hframes * object_vframes == 0) {
		// nothing to do - keep old cache
		return;
	}

	_outline_cache_key = object_frames;
	_outline_cache.resize(object_vframes * object_hframes);
	_outline_cache_dirty = false;

	ConfigFile cache;
	String res_path = get_texture()->get_path();
	String cache_path = vformat("%08x_outline_cache.ini", res_path.hash());
	String cache_key = String::num(_outline_cache_key.width) + "x" + String::num(_outline_cache_key.height);
	if (outline_file_cache) {
		if (res_path.empty()) {
			WARN_PRINT("Cannot use outline cache for " + get_name());
		} else {
			cache.load(cache_path);
			if (cache.has_section_key("cache", cache_key)) {
				PoolByteArray nblocks = cache.get_value("cache", cache_key);
				_outline_cache.resize(nblocks.size());
				// Load cached values
				int lines = 0;
				for (int pi = 0; pi < nblocks.size(); pi++) {
					const int nlines = nblocks[pi];
					_outline_cache.write[pi].resize(nlines);
					for (int i = 0; i < nlines; i++) {
						Vector<Vector2> poly = cache.get_value(cache_key, String::num(lines));
						_outline_cache.write[pi].write[i] = poly;
						lines++;
					}
				}
				DEBUG_PRINT("Using outline cache " + cache_path + " for key: " + cache_key);
				return;
			}
		}
	}

	Ref<BitMap> bm = memnew(BitMap);
	bm->create_from_image_alpha(image);

	int object = 0;
	for (int y = 0; y < object_vframes; y++) {
		for (int x = 0; x < object_hframes; x++) {
			Rect2 rect = Rect2(image_rect.position + cell_size * Size2(x, y), cell_size);
			Vector<Vector<Vector2>> lines = bm->clip_opaque_to_polygons(rect);
			_outline_cache.write[object].resize(lines.size());

			for (int pi = 0; pi < lines.size(); pi++) {
				for (int i = 0; i < lines[pi].size(); i++) {
					Vector2 vtx = lines[pi][i];
					vtx -= rect.size * Size2(0.5, 0.5 + 2 * y); // centering
					vtx *= get_texture_scale(); // texture scaling
					lines.write[pi].write[i] = vtx;
				}
			}
			_outline_cache.write[object] = lines;
			object++;
		}
	}

	if (outline_file_cache) {
		if (!res_path.empty()) {
			cache.set_value("cache", "path", res_path);
			PoolByteArray nlines;
			int lines = 0;
			for (int pi = 0; pi < _outline_cache.size(); pi++) {
				nlines.push_back(_outline_cache[pi].size());
				for (int i = 0; i < _outline_cache[pi].size(); i++) {
					cache.set_value(cache_key, String::num(lines), _outline_cache[pi][i]);
					lines++;
				}
			}
			cache.set_value("cache", cache_key, nlines);
			cache.save(cache_path);
		}
	}
}

void DestructibleSprite::_on_debris_screen_exit(uint64_t object_id, Object *obj) {
	ERR_FAIL_NULL(obj);

	if (_simulations.count(object_id)) { // might already be removed
		explo_object_t *object = _simulations[object_id];
		object->blocks.erase(
				std::remove(object->blocks.begin(), object->blocks.end(), obj), object->blocks.end());
		if (object->blocks.empty()) {
			object->id = 0; // dead
			if (debug_mode) {
				DEBUG_PRINT(vformat("[%s/debris_screen_exit] explosion 0x%x removed.", get_name(), object_id));
			}
		}
	}
}

void DestructibleSprite::_on_opacity_tween_completed(Object *obj, String key) {
	ERR_FAIL_NULL(obj);
	const uint64_t object_id = obj->get_meta("simulation_id");
	ERR_FAIL_COND(_simulations.count(object_id) == 0);

	if (Node *node = cast_to<Node>(obj)) {
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

	explo_object_t &object = *_simulations[object_id];

	if (debug_mode) {
		DEBUG_PRINT(vformat("[%s/debris_timer_timeout] 0x%x object's debris timer timed out.", get_name(), object_id));
	}

	bool restart_timer = false;
	for (auto *block : object.blocks) {
		if (RigidBody2D *body = cast_to<RigidBody2D>(block)) {
			if (object.remove_debris) {
				if (Sprite *sprite = _safe_sprite(body)) {
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
				if (body->get_linear_velocity().abs() < 0.05) {
					body->set_mode(RigidBody2D::MODE_STATIC);
				} else {
					if (debug_mode) {
#ifdef DEBUG_ENABLED
						if (Sprite *sprite = _safe_sprite(body)) {
							DEBUG_PRINT(vformat("[%s/debris_timer_timeout] frame %d still running: %s", get_name(), sprite->get_frame(), body->get_linear_velocity().abs()));
						}
#endif
					}
					restart_timer = true;
				}
				if (CollisionShape2D *collision_shape = _safe_collision_shape(body)) {
					collision_shape->set_disabled(true);
				} else if (CollisionPolygon2D *collision_poly = _safe_collision_polygon(body)) {
					int index = collision_poly->get_index();
					while (CollisionPolygon2D *c = cast_to<CollisionPolygon2D>(body->get_child(index))) {
						c->set_disabled(true);
						if (++index == body->get_child_count())
							break;
					}
				}
			}
		}
	}
	if (restart_timer) {
		object.debris_timer->start();
	} else if (!object.remove_debris) {
		if (debug_mode) {
			DEBUG_PRINT(vformat("[%s/debris_timer_timeout] explosion 0x%x completed static.", get_name(), object_id));
		}
	}
}

void DestructibleSprite::_initiate_detonation(uint64_t object_id) {
	ERR_FAIL_COND(object_id == 0);
	ERR_FAIL_COND(_simulations.count(object_id) == 0);

	const explo_object_t &object = *_simulations[object_id];

	// add blocks children
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
		if (Particles2D *particles2d = cast_to<Particles2D>(child)) {
			particles2d->set_emitting(true);
		} else if (CPUParticles2D *particlescpu = cast_to<CPUParticles2D>(child)) {
			particlescpu->set_emitting(true);
		} else if (FakeExplosionParticles2D *fakeparticles = cast_to<FakeExplosionParticles2D>(child)) {
			fakeparticles->single_explosion();
		}
	}

	// initiate blocks
	if (object.destruction_physics == DESTRUCTION_PHYSICS_OFF) {
		for (auto *block : object.blocks) {
			const real_t block_scale = random_debris_scale ? Math::random(0.8, 1.2) : Math::random(0.6, 1.0);
			if (Sprite *sprite = cast_to<Sprite>(block)) {
				sprite->set_scale(Vector2(block_scale, block_scale));
				sprite->set_meta("scale", block_scale);
			}
			if (random_depth) {
				block->set_z_index(Math::randf() < 0.5 ? 0 : -1);
			}
		}
	} else {
		for (auto *block : object.blocks) {
			if (RigidBody2D *body = cast_to<RigidBody2D>(block)) {
				body->set_gravity_scale(gravity_scale);

				const bool scale_block = random_debris_scale && Math::randf() < 0.5;
				const real_t scale_factor = 0.5;

				if (Sprite *sprite = _safe_sprite(body)) {
					if (scale_block) {
						sprite->set_scale(sprite->get_scale() * scale_factor);
					}
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
				if (scale_block) {
					if (CollisionShape2D *collision_shape = _safe_collision_shape(body)) {
						collision_shape->set_scale(collision_shape->get_scale() * scale_factor);
						collision_shape->set_position(collision_shape->get_position() * scale_factor);
					} else if (CollisionPolygon2D *collision_poly = _safe_collision_polygon(body)) {
						int index = collision_poly->get_index();
						while (CollisionPolygon2D *c = cast_to<CollisionPolygon2D>(body->get_child(index))) {
							c->set_scale(c->get_scale() * scale_factor);
							c->set_position(c->get_position() * scale_factor);
							if (++index == body->get_child_count())
								break;
						}
					}
					body->set_mass(body->get_mass() * scale_factor);
				}
				if (random_collision) {
					body->set_collision_layer(Math::randf() < 0.5 ? 0 : collision_layers);
					body->set_collision_mask(Math::randf() < 0.5 ? 0 : collision_masks);
				}
				if (random_depth) {
					body->set_z_index(Math::randf() < 0.5 ? 0 : -1);
				}
				body->set_mode(RigidBody2D::MODE_RIGID);

				if (object.blocks_impulse > 0) {
					// Create a random angular velocity for each block, depending on its mass.
					const real_t block_angular_velocity = Math::random(-object.blocks_impulse / body->get_weight(), object.blocks_impulse / body->get_weight());
					// Set the angular velocity for each block.
					body->set_angular_velocity(block_angular_velocity);
					// Create a random impulse for each block.
					const real_t block_rotation = Math::random(0, 360);

					// trigger impulse
					switch (object.destruction_type) {
						case DESTRUCTION_EXPLODE: {
							body->apply_central_impulse(Vector2(blocks_impulse, blocks_impulse).rotated(Math::deg2rad(block_rotation)));
						} break;
						case DESTRUCTION_COLLAPSE: {
							body->apply_central_impulse(Vector2(blocks_impulse, -blocks_impulse).rotated(Math::deg2rad(block_rotation)));
						} break;
						default: {
							WARN_PRINT("Cannot trigger unknown destruction type.");
						}
					}
				}
			} else {
				WARN_PRINT("Block should be RigidBody2D, not a " + block->get_class());
			}
		}

		object.debris_timer->start();
	}
}

void DestructibleSprite::_prepare_detonation(explo_object_t &object) {
	if (randomize_seed) {
		Math::randomize();
	}

	object.blocks_container = memnew(Node2D);

	// Set the debris timer.
	if (debris_max_time > 0) {
		object.debris_timer = memnew(Timer);
		object.debris_timer->connect("timeout", this, "_on_debris_timer_timeout", varray(object.id));
		object.debris_timer->set_one_shot(true);
		object.debris_timer->set_wait_time(debris_max_time);
		object.debris_timer->set_name("debris_timer");
		object.blocks_container->add_child(object.debris_timer, true);
	}

	// Check if the sprite is using 'Region' to get the proper width and height.
	Size2 object_size = get_texture_scale() * (is_region() ? get_region_rect().size : get_texture()->get_size());

	// Get the number of blocks in horiontal and vertical.
	const real_t block_size = object_size.max() / blocks_per_side;
	const Size2 object_frames = object_size / block_size;
	const int object_hframes = object_frames.width;
	const int object_vframes = object_frames.height;
	const Size2 cell_size = object_size / Size2(object_hframes, object_vframes);

	if (debug_mode) {
		DEBUG_PRINT(vformat("[%s/prepare_detonation] object's blocks per side: %d x %d (block size %s, physics %s)",
				get_name(), object_hframes, object_vframes, cell_size,
				destruction_physics == DESTRUCTION_PHYSICS_OFF ? "off" : "on"));
	}

	// Check if the sprite is centered to get the offset.
	Vector2 object_offset = cell_size / -2;
	if (is_centered()) {
		object_offset += object_size / 2;
	}

	const Vector2 collision_extents = cell_size / 2;
	const Vector2 collision_position = Vector2((ceil(object.collision_extents.x) - object.collision_extents.x) * -1,
			(ceil(object.collision_extents.y) - object.collision_extents.y) * -1);

	object.blocks.clear();

	const bool outline_cache_valid = _outline_cache.size() && _outline_cache_key == Size2i(object_hframes, object_vframes);
	const int total_objects = object_hframes * object_vframes;
	for (int y = 0; y < object_vframes; y++) {
		for (int x = 0; x < object_hframes; x++) {
			const int n = y * object_hframes + x;

			Node2D *duplicated_object = cast_to<Node2D>(duplicate(DUPLICATE_USE_INSTANCING));
			duplicated_object->set_meta("simulation_id", object.id);

			if (Sprite *sprite = cast_to<Sprite>(duplicated_object)) {
				sprite->set_vframes(object_vframes);
				sprite->set_hframes(object_hframes);
				sprite->set_frame(n);
				// duplicates will be a child of this sprite
				sprite->set_position(Vector2(0, 0));
				// always centered
				sprite->set_centered(true);
			}

			const Vector2 position = Vector2(x, y) * cell_size - object_offset; // Position each block to create the whole sprite.

			if (object.destruction_physics == DESTRUCTION_PHYSICS_OFF) {
				duplicated_object->set_name(vformat("%s_block_%d", get_name(), n)); // Add a unique name to each block
				duplicated_object->set_meta("time", 0);
				duplicated_object->set_meta("velocity", _get_random_velocity(blocks_impulse));
				duplicated_object->set_meta("increment", _get_random_velocity_increment());
				duplicated_object->set_position(position);
				duplicated_object->add_child(memnew(VisibilityNotifier2D));
				object.blocks.push_back(duplicated_object); // Just append it to the blocks array
			} else {
				// Root node
				RigidBody2D *root_object = memnew(RigidBody2D);
				root_object->set_name(vformat("%s_block_%d", get_name(), n)); // Add a unique name to each block
				root_object->set_mode(RigidBody2D::MODE_STATIC);
				// Set each block's mass depending on the object's mass and the total number of blocks.
				root_object->set_mass(object_mass / total_objects);
				root_object->set_position(position);
				// Add child nodes
				root_object->add_child(duplicated_object);
				VisibilityNotifier2D *vis = memnew(VisibilityNotifier2D);
				vis->connect("screen_exited", this, "_on_debris_screen_exit", varray(object.id, root_object));
				root_object->add_child(vis);
				// Create a new collision shape for each block
				if (object.destruction_physics == DESTRUCTION_PHYSICS_STANDARD) {
					RectangleShape2D *shape = memnew(RectangleShape2D);
					shape->set_extents(collision_extents);
					CollisionShape2D *collision = memnew(CollisionShape2D);
					collision->set_shape(shape);
					collision->set_position(collision_position);
					if (collision_one_way) {
						collision->set_one_way_collision(true);
					}
					root_object->add_child(collision);
				} else if (object.destruction_physics == DESTRUCTION_PHYSICS_HIGH) {
					if (outline_cache_valid) {
						const Vector<Vector<Vector2>> &outline = _outline_cache[n];
						if (outline.size() == 0) {
							memdelete(root_object);
							continue;
						}
						for (int p = 0; p < outline.size(); p++) {
							CollisionPolygon2D *collision = memnew(CollisionPolygon2D);
							collision->set_polygon(outline[p]);
							if (collision_one_way) {
								collision->set_one_way_collision(true);
							}
							root_object->add_child(collision);
						}
					} else {
						WARN_PRINT("Polygon shapes are not available.");
					}
				} else {
					WARN_PRINT("Unknown physics mode.");
				}
				object.blocks.push_back(root_object); // Append it to the blocks array
			}
		}
	}

	call_deferred("_initiate_detonation", object.id);
}

static real_t ease_scale(real_t t, real_t b = 1.0, real_t c = -0.9, real_t d = 1.0) {
	t = t / d - 1.0f;
	return (c * (t * t * t + 1.0f) + b);
}

void DestructibleSprite::_simulate_particles(explo_object_t &object, real_t delta) {
	ERR_FAIL_COND(object.id == 0);
	ERR_FAIL_COND(object.blocks.size() == 0);

	bool dead = true;
	for (auto *block : object.blocks) {
		if (block->get_modulate().a > 0) {
			const Vector2 gravity = GravityFactor * gravity_scale;
			const Vector2 velocity = (Vector2)block->get_meta("velocity") * (Vector2)block->get_meta("increment");
			real_t time = block->get_meta("time");
			const Vector2 position = block->get_position() + (velocity + gravity) * delta;

			block->set_position(position);
			const uint64_t block_id = reinterpret_cast<uint64_t>(block) >> 8; // compensate mem. aligment
			block->set_rotation(2 * (block_id & 1 ? time : -time));
			time += delta;

			if (time > _get_random_time()) {
				// fade out the particles
				Color c = block->get_modulate();
				if (c.a > 0) {
					if (random_debris_scale) {
						if (Sprite *sprite = cast_to<Sprite>(block)) {
							const real_t scale = block->get_meta("scale");
							const real_t block_scale = scale * ease_scale(1 - c.a);
							sprite->set_scale(Size2(block_scale, block_scale));
						}
					}
					c.a -= delta;
				}
				if (c.a < 0) {
					c.a = 0;
				}
				// if the particle is out of screen ...
				if (VisibilityNotifier2D *vis = cast_to<VisibilityNotifier2D>(block->get_child(0))) {
					if (!vis->is_on_screen()) {
						c.a = 0;
					}
				}
				block->set_modulate(c);
				// if the particle is invisible ...
				if (dead) {
					dead &= c.a == 0;
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
	if (p_type == DESTRUCTION_COLLAPSE && destruction_physics == DESTRUCTION_PHYSICS_OFF) {
		WARN_PRINT("Collapsing is only possible with enabled destruction physics.");
		return;
	}
	destruction_type = p_type;
}

void DestructibleSprite::set_destruction_physics(DestructionPhysics p_physics) {
	if (p_physics == DESTRUCTION_PHYSICS_OFF && destruction_type == DESTRUCTION_COLLAPSE) {
		WARN_PRINT("Physics is required for collapsing destruction.");
		return;
	}
	destruction_physics = p_physics;
	update_configuration_warning();
}

void DestructibleSprite::set_object_mass(real_t p_object_mass) {
	object_mass = p_object_mass;
}

void DestructibleSprite::set_gravity_scale(real_t p_gravity_scale) {
	gravity_scale = p_gravity_scale;
}

void DestructibleSprite::set_blocks_per_side(int p_blocks_per_side) {
	blocks_per_side = p_blocks_per_side;
	_outline_cache_dirty = true;
}

void DestructibleSprite::set_blocks_impulse(real_t p_blocks_impulse) {
	ERR_FAIL_COND(p_blocks_impulse <= 0);
	ERR_FAIL_COND(p_blocks_impulse > 1000);

	blocks_impulse = p_blocks_impulse;
}

void DestructibleSprite::set_random_debris_scale(bool p_random_debris_scale) {
	random_debris_scale = p_random_debris_scale;
}

void DestructibleSprite::set_debris_max_time(real_t p_debris_max_time) {
	debris_max_time = p_debris_max_time;
}

void DestructibleSprite::set_remove_debris(bool p_remove_debris) {
	remove_debris = p_remove_debris;
}

void DestructibleSprite::set_random_depth(bool p_random_depth) {
	random_depth = p_random_depth;
}

void DestructibleSprite::set_random_collision(bool p_random_collision) {
	random_collision = p_random_collision;
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

void DestructibleSprite::set_outline_file_cache(bool p_cache) {
	outline_file_cache = p_cache;
}

void DestructibleSprite::set_debug_mode(bool p_debug_mode) {
	debug_mode = p_debug_mode;
}

void DestructibleSprite::explode(real_t delay) {
	ERR_FAIL_COND(delay < 0);

	explo_object_t *object = memnew(explo_object_t);
	object->id = reinterpret_cast<uint64_t>(object);
	object->delay = delay;
	object->destruction_type = destruction_type;
	object->destruction_physics = destruction_physics;
	object->blocks_impulse = blocks_impulse * object_mass * gravity_scale;
	object->remove_debris = remove_debris;

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
#ifdef TOOLS_ENABLED
		case NOTIFICATION_LOCAL_TRANSFORM_CHANGED: {
			if (Engine::get_singleton()->is_editor_hint()) {
				update_configuration_warning();
			}
		} break;
#endif
		case NOTIFICATION_READY: {
			connect("texture_changed", this, "_on_texture_changed");
		} break;
		case NOTIFICATION_DRAW: {
		} break;
		case NOTIFICATION_PARENTED:
		case NOTIFICATION_ENTER_TREE: {
#ifdef TOOLS_ENABLED
			if (Engine::get_singleton()->is_editor_hint()) {
				set_notify_local_transform(true); //used for warnings and only in editor
			}
#endif
		} break;
		case NOTIFICATION_PROCESS: {
			const real_t delta = get_process_delta_time();

			if (_outline_cache_dirty) {
				_update_texture_shapes();
			}

			std::vector<uint64_t> dead;
			for (auto &s : _simulations) {
				const uint64_t id = s.first;
				explo_object_t &object = *s.second;
				if (object.id == 0) {
					dead.push_back(id);
				} else if (object.blocks.empty()) {
					if (object.delay <= 0) {
						_prepare_detonation(object);
						_disabled_base_notifications.push_back(NOTIFICATION_DRAW); // we want only blocks to be displayed
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

String DestructibleSprite::get_configuration_warning() const {
	Transform2D t = get_transform();

	String warning = Sprite::get_configuration_warning();

	if (destruction_physics != DESTRUCTION_PHYSICS_OFF && (ABS(t.elements[0].length() - 1.0) > 0.05 || ABS(t.elements[1].length() - 1.0) > 0.05)) {
		if (warning != String()) {
			warning += "\n\n";
		}
		warning += TTR("Non-unit Sprite's scaling can lead to drawing artefacts when the physics engine is running.");
	}

	return warning;
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
	ClassDB::bind_method(D_METHOD("set_object_mass", "per_side"), &DestructibleSprite::set_object_mass);
	ClassDB::bind_method(D_METHOD("get_object_mass"), &DestructibleSprite::get_object_mass);
	ClassDB::bind_method(D_METHOD("set_gravity_scale", "scale"), &DestructibleSprite::set_gravity_scale);
	ClassDB::bind_method(D_METHOD("get_gravity_scale"), &DestructibleSprite::get_gravity_scale);
	ClassDB::bind_method(D_METHOD("set_blocks_per_side", "per_side"), &DestructibleSprite::set_blocks_per_side);
	ClassDB::bind_method(D_METHOD("get_blocks_per_side"), &DestructibleSprite::get_blocks_per_side);
	ClassDB::bind_method(D_METHOD("set_blocks_impulse", "impulse"), &DestructibleSprite::set_blocks_impulse);
	ClassDB::bind_method(D_METHOD("get_blocks_impulse"), &DestructibleSprite::get_blocks_impulse);
	ClassDB::bind_method(D_METHOD("set_random_debris_scale", "scale"), &DestructibleSprite::set_random_debris_scale);
	ClassDB::bind_method(D_METHOD("is_random_debris_scale"), &DestructibleSprite::is_random_debris_scale);
	ClassDB::bind_method(D_METHOD("set_random_depth"), &DestructibleSprite::set_random_depth);
	ClassDB::bind_method(D_METHOD("is_random_depth"), &DestructibleSprite::is_random_depth);
	ClassDB::bind_method(D_METHOD("set_debris_max_time", "max_time"), &DestructibleSprite::set_debris_max_time);
	ClassDB::bind_method(D_METHOD("get_debris_max_time"), &DestructibleSprite::get_debris_max_time);
	ClassDB::bind_method(D_METHOD("set_remove_debris", "remove_debris"), &DestructibleSprite::set_remove_debris);
	ClassDB::bind_method(D_METHOD("is_remove_debris"), &DestructibleSprite::is_remove_debris);
	ClassDB::bind_method(D_METHOD("set_random_collision", "layers"), &DestructibleSprite::set_random_collision);
	ClassDB::bind_method(D_METHOD("is_random_collision"), &DestructibleSprite::is_random_collision);
	ClassDB::bind_method(D_METHOD("set_collision_layers", "layers"), &DestructibleSprite::set_collision_layers);
	ClassDB::bind_method(D_METHOD("get_collision_layers"), &DestructibleSprite::get_collision_layers);
	ClassDB::bind_method(D_METHOD("set_collision_masks", "mask"), &DestructibleSprite::set_collision_masks);
	ClassDB::bind_method(D_METHOD("get_collision_masks"), &DestructibleSprite::get_collision_masks);
	ClassDB::bind_method(D_METHOD("set_collision_one_way", "one_way"), &DestructibleSprite::set_collision_one_way);
	ClassDB::bind_method(D_METHOD("is_collision_one_way"), &DestructibleSprite::is_collision_one_way);
	ClassDB::bind_method(D_METHOD("set_randomize_seed", "seed"), &DestructibleSprite::set_randomize_seed);
	ClassDB::bind_method(D_METHOD("is_randomize_seed"), &DestructibleSprite::is_randomize_seed);
	ClassDB::bind_method(D_METHOD("set_outline_file_cache", "cache"), &DestructibleSprite::set_outline_file_cache);
	ClassDB::bind_method(D_METHOD("is_outline_file_cache"), &DestructibleSprite::is_outline_file_cache);
	ClassDB::bind_method(D_METHOD("set_debug_mode", "mode"), &DestructibleSprite::set_debug_mode);
	ClassDB::bind_method(D_METHOD("is_debug_mode"), &DestructibleSprite::is_debug_mode);

	ClassDB::bind_method(D_METHOD("explode", "delay"), &DestructibleSprite::explode);
	ClassDB::bind_method(D_METHOD("reset"), &DestructibleSprite::reset);
	ClassDB::bind_method(D_METHOD("explosions_in_progress"), &DestructibleSprite::explosions_in_progress);
	ClassDB::bind_method(D_METHOD("is_active_explosions"), &DestructibleSprite::is_active_explosions);

	ClassDB::bind_method(D_METHOD("_on_debris_timer_timeout", "object_id"), &DestructibleSprite::_on_debris_timer_timeout);
	ClassDB::bind_method(D_METHOD("_on_opacity_tween_completed", "object", "key"), &DestructibleSprite::_on_opacity_tween_completed);
	ClassDB::bind_method(D_METHOD("_on_debris_screen_exit", "object_id", "object"), &DestructibleSprite::_on_debris_screen_exit);
	ClassDB::bind_method(D_METHOD("_on_texture_changed"), &DestructibleSprite::_texture_changed);
	ClassDB::bind_method(D_METHOD("_initiate_detonation", "object_id"), &DestructibleSprite::_initiate_detonation);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "destruction_type", PROPERTY_HINT_ENUM, "Explode,Collapse"), "set_destruction_types", "get_destruction_types");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "destruction_physics", PROPERTY_HINT_ENUM, "Off,Standard,High"), "set_destruction_physics", "get_destruction_physics");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "object_mass"), "set_object_mass", "get_object_mass");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "gravity_scale"), "set_gravity_scale", "get_gravity_scale");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "blocks_per_side"), "set_blocks_per_side", "get_blocks_per_side");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "blocks_impulse", PROPERTY_HINT_RANGE, "0,1000,50"), "set_blocks_impulse", "get_blocks_impulse");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "debris_max_time"), "set_debris_max_time", "get_debris_max_time");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "remove_debris"), "set_remove_debris", "is_remove_debris");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "random_debris_scale"), "set_random_debris_scale", "is_random_debris_scale");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "random_depth"), "set_random_depth", "is_random_depth");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "random_collision"), "set_random_collision", "is_random_collision");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_layers"), "set_collision_layers", "get_collision_layers");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_masks"), "set_collision_masks", "get_collision_masks");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "collision_one_way"), "set_collision_one_way", "is_collision_one_way");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "randomize_seed"), "set_randomize_seed", "is_randomize_seed");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "outline_file_cache"), "set_outline_file_cache", "is_outline_file_cache");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug_mode"), "set_debug_mode", "is_debug_mode");
}

DestructibleSprite::DestructibleSprite() {
	_reset_at_end = false;
	_outline_cache_key = Size2i(0, 0);
	_outline_cache_dirty = true;
	destruction_type = DESTRUCTION_EXPLODE;
	destruction_physics = DESTRUCTION_PHYSICS_STANDARD;
	object_mass = 1;
	gravity_scale = 10;
	blocks_per_side = 6;
	blocks_impulse = 300;
	debris_max_time = 5;
	remove_debris = false;
	random_depth = false;
	random_debris_scale = true;
	random_collision = true;
	collision_layers = 1;
	collision_masks = 1;
	collision_one_way = false;
	randomize_seed = false;
	outline_file_cache = false;
	debug_mode = false;
}

DestructibleSprite::~DestructibleSprite() {
}
