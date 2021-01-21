/*************************************************************************/
/*  explosion_particles.cpp                                              */
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

// https://github.com/danboo/godot-RigidBodyParticles2D/blob/master/examples/sparks/Particle.gd

#include "explosion_particles.h"

void ExplosionParticles2D::_create_particles() {
}

void ExplosionParticles2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			// Add to a group so it can be found from anywhere.
			add_to_group(group_name);
			// Create the initial particles.
			_create_particles();
			// Create a timer.
			particles_timer = new Timer();
			particles_timer->set_one_shot(false);
			particles_timer->set_wait_time(timer_wait_time);
			particles_timer->set_timer_process_mode(Timer::TIMER_PROCESS_IDLE);
			particles_timer->connect("timeout", this, "_on_particles_timer_timeout");

			add_child(particles_timer, true);

			if (start_timer)
				particles_timer->start();
		} break;
		case NOTIFICATION_ENTER_TREE: {
		} break;
		case NOTIFICATION_PHYSICS_PROCESS: {
		} break;
		case NOTIFICATION_PROCESS: {
			update();
		} break;
		case NOTIFICATION_DRAW: {
		} break;
	}
}

ExplosionParticles2D::ExplosionParticles2D() {

	min_particles_number = 200;
	max_particles_number = 400;

	min_particles_gravity = 200.0;
	max_particles_gravity = 600.0;

	min_particles_velocity = 200.0;
	max_particles_velocity = 600.0;

	max_particles_position_x = ProjectSettings::get_singleton()->get("display/window/size/width");
	max_particles_position_y = ProjectSettings::get_singleton()->get("display/window/size/height");

	min_particles_size = 1;
	max_particles_size = 3;

	get_random_position = false;
	start_timer = false;
	timer_wait_time = 1.0;
	particles_explode = false;

	group_name = "fake_explosion_particles";
}
