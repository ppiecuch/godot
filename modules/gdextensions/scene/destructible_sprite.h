/*************************************************************************/
/*  destructible_sprite.h                                                */
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

// -*- C++ -*-
//

#ifndef GD_DESTRUCTIBLE_SPRITE_H
#define GD_DESTRUCTIBLE_SPRITE_H

#include "core/reference.h"
#include "scene/2d/physics_body_2d.h"
#include "scene/2d/sprite.h"

#include <map>

struct explo_object_t;

class DestructibleSprite : public Sprite {
	GDCLASS(DestructibleSprite, Sprite);

public:
	enum DestructionType {
		DESTRUCTION_EXPLODE,
		DESTRUCTION_COLLAPSE,
		DestructionTypeCount,
	};
	enum DestructionPhysics {
		DESTRUCTION_PHYSICS_OFF, // simple particles
		DESTRUCTION_PHYSICS_STANDARD, //< debris are quad shapes
		DESTRUCTION_PHYSICS_HIGH, //< debris are polygon shapes
		DestructionPhysicsCount,
	};

private:
	DestructionType destruction_type;
	DestructionPhysics destruction_physics;
	int blocks_per_side;
	real_t blocks_impulse;
	real_t blocks_gravity_scale;
	bool blocks_random_depth;
	bool blocks_fade_size;
	real_t debris_max_time;
	bool remove_debris;
	int collision_layers;
	int collision_masks;
	bool collision_one_way;
	bool randomize_seed;
	bool debug_mode;

	void _prepare_detonation(explo_object_t &object);
	void _initiate_detonation(uint64_t object_id);
	void _on_debris_timer_timeout(uint64_t object_id);
	void _on_opacity_tween_completed(Object *obj, String key);
	void _on_debris_screen_exit(uint64_t object_id, Object *obj);

	void _simulate_particles(explo_object_t &object, real_t delta);

	bool _reset_at_end;
	std::map<uint64_t, explo_object_t *> _simulations;

public:
	void set_destruction_types(DestructionType p_type);
	DestructionType get_destruction_types() const { return destruction_type; }
	void set_destruction_physics(DestructionPhysics p_physics);
	DestructionPhysics get_destruction_physics() const { return destruction_physics; }
	void set_blocks_per_side(int p_blocks_per_side);
	int get_blocks_per_side() const { return blocks_per_side; }
	void set_blocks_impulse(real_t p_blocks_impulse);
	real_t get_blocks_impulse() const { return blocks_impulse; }
	void set_blocks_gravity_scale(real_t p_blocks_gravity_scale);
	real_t get_blocks_gravity_scale() const { return blocks_gravity_scale; }
	void set_blocks_fade_size(bool p_blocks_fade_size);
	bool is_blocks_fade_size() const { return blocks_fade_size; }
	void set_blocks_random_depth(bool p_blocks_random_depth);
	bool is_blocks_random_depth() const { return blocks_random_depth; }
	void set_debris_max_time(real_t p_debris_max_time);
	real_t get_debris_max_time() const { return debris_max_time; }
	void set_remove_debris(bool p_remove_debris);
	bool get_remove_debris() const { return remove_debris; }
	void set_collision_layers(int p_collision_layers);
	int get_collision_layers() const { return collision_layers; }
	void set_collision_masks(int p_collision_masks);
	int get_collision_masks() const { return collision_masks; }
	void set_collision_one_way(bool p_collision_one_way);
	bool get_collision_one_way() const { return collision_one_way; }
	void set_randomize_seed(bool p_randomize_seed);
	bool get_randomize_seed() const { return randomize_seed; }
	void set_debug_mode(bool p_debug_mode);
	bool get_debug_mode() const { return debug_mode; }

	bool is_active_explosions();
	int explosions_in_progress();
	void explode(real_t delay);
	void reset();

	virtual String get_configuration_warning() const;

	DestructibleSprite();
	~DestructibleSprite();

protected:
	void _notification(int p_what);
	static void _bind_methods();
};

VARIANT_ENUM_CAST(DestructibleSprite::DestructionType);
VARIANT_ENUM_CAST(DestructibleSprite::DestructionPhysics);

#endif /* GD_DESTRUCTIBLE_SPRITE_H */
