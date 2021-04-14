/*************************************************************************/
/*  destructible_object.h                                                */
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

#ifndef GD_DESTRUCTIBLE_2D_H
#define GD_DESTRUCTIBLE_2D_H

#include "core/reference.h"
#include "scene/2d/physics_body_2d.h"

class DestructibleObject : public RigidBody2D {
	GDCLASS(DestructibleObject, RigidBody2D);

private:
	int blocks_per_side;
	real_t blocks_impulse;
	real_t blocks_gravity_scale;
	real_t debris_max_time;
	bool remove_debris;
	int collision_layers;
	int collision_masks;
	bool collision_one_way;
	bool explosion_delay;
	String fake_explosions_group = "fake_explosion_particles";
	bool randomize_seed;
	bool debug_mode;

	real_t _explosion_delay_timer;
	real_t _explosion_delay_timer_limit;

public:
	DestructibleObject();
	~DestructibleObject();

protected:
	void _notification(int p_what);
	static void _bind_methods();
};

#endif /* GD_DESTRUCTIBLE_2D_H */
