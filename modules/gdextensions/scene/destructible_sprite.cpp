/*************************************************************************/
/*  destructible_object.cpp                                              */
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
// https://github.com/mjholtzem/Unity-2D-Destruction.git
// https://github.com/hiulit/Godot-3-2D-Destructible-Objects

#include "destructible_object.h"

void DestructibleObject::_bind_methods() {

	ADD_PROPERTY(PropertyInfo(Variant::INT, "blocks_per_side"), "set_blocks_per_side", "get_blocks_per_side");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "blocks_impulse"), "set_blocks_impulse", "get_blocks_impulse");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "blocks_gravity_scale"), "set_blocks_gravity_scale", "get_blocks_gravity_scale");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "debris_max_time"), "set_debris_max_time", "get_debris_max_time");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "remove_debris"), "set_remove_debris", "get_remove_debris");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_layers"), "set_collision_layers", "get_collision_layers");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_masks"), "set_collision_masks", "get_collision_masks");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "collision_one_way"), "set_collision_one_way", "get_collision_one_way");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "explosion_delay"), "set_explosion_delay", "get_explosion_delay");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "fake_explosions_group"), "set_fake_explosions_group", "get_fake_explosions_group");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "randomize_seed"), "set_randomize_seed", "get_randomize_seed");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug_mode"), "set_debug_mode", "get_debug_mode");
}

DestructibleObject::DestructibleObject() {

	blocks_per_side = 6;
	blocks_impulse = 600;
	blocks_gravity_scale = 10;
	debris_max_time = 5;
	remove_debris = false;
	collision_layers = 1;
	collision_masks = 1;
	collision_one_way = false;
	explosion_delay = false;
	fake_explosions_group = "fake_explosion_particles";
	randomize_seed = false;
	debug_mode = false;
	_explosion_delay_timer = 0;
	_explosion_delay_timer_limit = 0;
}
