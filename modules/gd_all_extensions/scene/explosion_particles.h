/*************************************************************************/
/*  explosion_particles.h                                                */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef GODOT_EXPLOSION_PARTICLES_2D_H
#define GODOT_EXPLOSION_PARTICLES_2D_H

#include "core/math/vector2.h"
#include "core/object.h"
#include "core/typedefs.h"
#include "scene/2d/mesh_instance_2d.h"
#include "scene/main/timer.h"

class ExplosionParticles2D : public Node2D {
	GDCLASS(ExplosionParticles2D, Node2D);

private:
	int min_particles_number;
	int max_particles_number;

	float min_particles_gravity;
	float max_particles_gravity;

	float min_particles_velocity;
	float max_particles_velocity;

	int max_particles_position_x;
	int max_particles_position_y;

	int min_particles_size;
	int max_particles_size;

	bool get_random_position;
	bool start_timer;
	float timer_wait_time;
	bool particles_explode;

	String group_name;

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	ExplosionParticles2D();
	~ExplosionParticles2D();

private:
	void _create_particles();

	Array particles;
	int particles_number;
	Vector2 particles_initial_position;

	struct {
		int w;
		Color c;
	} particles_colors_with_weights[5] = {
		{ 4, Color::html("#ffffff") },
		{ 2, Color::html("#000000") },
		{ 8, Color::html("#ff004d") },
		{ 8, Color::html("#ffa300") },
		{ 10, Color::html("#ffec27") }
	};

	Timer *particles_timer = 0;
};

#endif // GODOT_EXPLOSION_PARTICLES_2D_H
