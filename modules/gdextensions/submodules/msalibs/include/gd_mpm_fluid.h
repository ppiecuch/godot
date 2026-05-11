/**************************************************************************/
/*  gd_mpm_fluid.h                                                        */
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

#ifndef GD_MPM_FLUID_H
#define GD_MPM_FLUID_H

#include "core/math/vector2.h"
#include "core/object.h"
#include "core/pool_vector.h"
#include "core/reference.h"

#include "mpmfluid.h"

// MPM 2D fluid simulation.
//
// Grid is always 160×120 internal units. Use scale_factor to map to pixels.
// Call setup() once, then update() each frame. Render via get_particle_positions().
//
// Usage:
//   var f = GDMPMFluid.new()
//   f.setup(5000)
//   f.num_particles = 3000
//   func _process(_delta):
//     f.update()
//     var pts = f.get_particle_positions()  # PoolVector2Array (grid space)
class GDMPMFluid : public Reference {
	GDCLASS(GDMPMFluid, Reference);

	mutable mpm::fluid::MPMFluid _fluid;
	bool _ready;

protected:
	static void _bind_methods();

public:
	void setup(int max_particles);
	void update();

	// Apply a velocity impulse at grid position (cx, cy) blending toward (dx, dy).
	void apply_impulse(float cx, float cy, float dx, float dy, float radius = 10.0f);

	// Particle data (grid-space coordinates)
	int get_particle_count() const;
	PoolVector2Array get_particle_positions() const;
	PoolVector2Array get_particle_velocities() const;

	Vector2 get_grid_size() const;

	// Simulation parameters
	void set_num_particles(int n);
	int get_num_particles() const;

	void set_density_setting(float v);
	float get_density_setting() const;

	void set_stiffness(float v);
	float get_stiffness() const;

	void set_bulk_viscosity(float v);
	float get_bulk_viscosity() const;

	void set_elasticity(float v);
	float get_elasticity() const;

	void set_viscosity(float v);
	float get_viscosity() const;

	void set_yield_rate(float v);
	float get_yield_rate() const;

	void set_gravity(float v);
	float get_gravity() const;

	void set_smoothing(float v);
	float get_smoothing() const;

	void set_do_obstacles(bool v);
	bool get_do_obstacles() const;

	void set_scale_factor(float v);
	float get_scale_factor() const;

	GDMPMFluid();
};

#endif // GD_MPM_FLUID_H
