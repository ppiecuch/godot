/**************************************************************************/
/*  gd_mpm_fluid.cpp                                                      */
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

#include "gd_mpm_fluid.h"

GDMPMFluid::GDMPMFluid() :
		_ready(false) {}

void GDMPMFluid::setup(int max_particles) {
	_fluid.setup(max_particles);
	_fluid.numParticles = max_particles;
	_ready = true;
}

void GDMPMFluid::update() {
	if (_ready)
		_fluid.update();
}

void GDMPMFluid::apply_impulse(float cx, float cy, float dx, float dy, float radius) {
	if (_ready)
		_fluid.applyImpulse(cx, cy, dx, dy, radius);
}

int GDMPMFluid::get_particle_count() const {
	return _fluid.numParticles;
}

PoolVector2Array GDMPMFluid::get_particle_positions() const {
	PoolVector2Array out;
	if (!_ready)
		return out;
	const auto &parts = _fluid.getParticles();
	int n = _fluid.numParticles;
	out.resize(n);
	PoolVector2Array::Write w = out.write();
	for (int i = 0; i < n && i < (int)parts.size(); i++) {
		w[i] = Vector2(parts[i]->x, parts[i]->y);
	}
	return out;
}

PoolVector2Array GDMPMFluid::get_particle_velocities() const {
	PoolVector2Array out;
	if (!_ready)
		return out;
	const auto &parts = _fluid.getParticles();
	int n = _fluid.numParticles;
	out.resize(n);
	PoolVector2Array::Write w = out.write();
	for (int i = 0; i < n && i < (int)parts.size(); i++) {
		w[i] = Vector2(parts[i]->u, parts[i]->v);
	}
	return out;
}

Vector2 GDMPMFluid::get_grid_size() const {
	return Vector2((float)_fluid.getGridSizeX(), (float)_fluid.getGridSizeY());
}

void GDMPMFluid::set_num_particles(int n) { _fluid.numParticles = n; }
int GDMPMFluid::get_num_particles() const { return _fluid.numParticles; }

void GDMPMFluid::set_density_setting(float v) { _fluid.densitySetting = v; }
float GDMPMFluid::get_density_setting() const { return _fluid.densitySetting; }

void GDMPMFluid::set_stiffness(float v) { _fluid.stiffness = v; }
float GDMPMFluid::get_stiffness() const { return _fluid.stiffness; }

void GDMPMFluid::set_bulk_viscosity(float v) { _fluid.bulkViscosity = v; }
float GDMPMFluid::get_bulk_viscosity() const { return _fluid.bulkViscosity; }

void GDMPMFluid::set_elasticity(float v) { _fluid.elasticity = v; }
float GDMPMFluid::get_elasticity() const { return _fluid.elasticity; }

void GDMPMFluid::set_viscosity(float v) { _fluid.viscosity = v; }
float GDMPMFluid::get_viscosity() const { return _fluid.viscosity; }

void GDMPMFluid::set_yield_rate(float v) { _fluid.yieldRate = v; }
float GDMPMFluid::get_yield_rate() const { return _fluid.yieldRate; }

void GDMPMFluid::set_gravity(float v) { _fluid.gravity = v; }
float GDMPMFluid::get_gravity() const { return _fluid.gravity; }

void GDMPMFluid::set_smoothing(float v) { _fluid.smoothing = v; }
float GDMPMFluid::get_smoothing() const { return _fluid.smoothing; }

void GDMPMFluid::set_do_obstacles(bool v) { _fluid.bDoObstacles = v; }
bool GDMPMFluid::get_do_obstacles() const { return _fluid.bDoObstacles; }

void GDMPMFluid::set_scale_factor(float v) { _fluid.scaleFactor = v; }
float GDMPMFluid::get_scale_factor() const { return _fluid.scaleFactor; }

void GDMPMFluid::_bind_methods() {
	ClassDB::bind_method(D_METHOD("setup", "max_particles"), &GDMPMFluid::setup);
	ClassDB::bind_method(D_METHOD("update"), &GDMPMFluid::update);
	ClassDB::bind_method(D_METHOD("apply_impulse", "cx", "cy", "dx", "dy", "radius"), &GDMPMFluid::apply_impulse, DEFVAL(10.0f));

	ClassDB::bind_method(D_METHOD("get_particle_count"), &GDMPMFluid::get_particle_count);
	ClassDB::bind_method(D_METHOD("get_particle_positions"), &GDMPMFluid::get_particle_positions);
	ClassDB::bind_method(D_METHOD("get_particle_velocities"), &GDMPMFluid::get_particle_velocities);
	ClassDB::bind_method(D_METHOD("get_grid_size"), &GDMPMFluid::get_grid_size);

	ClassDB::bind_method(D_METHOD("set_num_particles", "n"), &GDMPMFluid::set_num_particles);
	ClassDB::bind_method(D_METHOD("get_num_particles"), &GDMPMFluid::get_num_particles);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "num_particles"), "set_num_particles", "get_num_particles");

	ClassDB::bind_method(D_METHOD("set_density_setting", "v"), &GDMPMFluid::set_density_setting);
	ClassDB::bind_method(D_METHOD("get_density_setting"), &GDMPMFluid::get_density_setting);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "density_setting"), "set_density_setting", "get_density_setting");

	ClassDB::bind_method(D_METHOD("set_stiffness", "v"), &GDMPMFluid::set_stiffness);
	ClassDB::bind_method(D_METHOD("get_stiffness"), &GDMPMFluid::get_stiffness);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "stiffness"), "set_stiffness", "get_stiffness");

	ClassDB::bind_method(D_METHOD("set_bulk_viscosity", "v"), &GDMPMFluid::set_bulk_viscosity);
	ClassDB::bind_method(D_METHOD("get_bulk_viscosity"), &GDMPMFluid::get_bulk_viscosity);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "bulk_viscosity"), "set_bulk_viscosity", "get_bulk_viscosity");

	ClassDB::bind_method(D_METHOD("set_elasticity", "v"), &GDMPMFluid::set_elasticity);
	ClassDB::bind_method(D_METHOD("get_elasticity"), &GDMPMFluid::get_elasticity);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "elasticity"), "set_elasticity", "get_elasticity");

	ClassDB::bind_method(D_METHOD("set_viscosity", "v"), &GDMPMFluid::set_viscosity);
	ClassDB::bind_method(D_METHOD("get_viscosity"), &GDMPMFluid::get_viscosity);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "viscosity"), "set_viscosity", "get_viscosity");

	ClassDB::bind_method(D_METHOD("set_yield_rate", "v"), &GDMPMFluid::set_yield_rate);
	ClassDB::bind_method(D_METHOD("get_yield_rate"), &GDMPMFluid::get_yield_rate);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "yield_rate"), "set_yield_rate", "get_yield_rate");

	ClassDB::bind_method(D_METHOD("set_gravity", "v"), &GDMPMFluid::set_gravity);
	ClassDB::bind_method(D_METHOD("get_gravity"), &GDMPMFluid::get_gravity);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "gravity"), "set_gravity", "get_gravity");

	ClassDB::bind_method(D_METHOD("set_smoothing", "v"), &GDMPMFluid::set_smoothing);
	ClassDB::bind_method(D_METHOD("get_smoothing"), &GDMPMFluid::get_smoothing);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "smoothing"), "set_smoothing", "get_smoothing");

	ClassDB::bind_method(D_METHOD("set_do_obstacles", "v"), &GDMPMFluid::set_do_obstacles);
	ClassDB::bind_method(D_METHOD("get_do_obstacles"), &GDMPMFluid::get_do_obstacles);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "do_obstacles"), "set_do_obstacles", "get_do_obstacles");

	ClassDB::bind_method(D_METHOD("set_scale_factor", "v"), &GDMPMFluid::set_scale_factor);
	ClassDB::bind_method(D_METHOD("get_scale_factor"), &GDMPMFluid::get_scale_factor);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "scale_factor"), "set_scale_factor", "get_scale_factor");
}

// ============================================================
// Doctests
// ============================================================

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_CASE("[msalibs][GDMPMFluid] setup allocates particles") {
	GDMPMFluid f;
	f.setup(500);
	CHECK(f.get_particle_positions().size() == 500);
}

TEST_CASE("[msalibs][GDMPMFluid] grid size is 160x120") {
	GDMPMFluid f;
	Vector2 gs = f.get_grid_size();
	CHECK((int)gs.x == 160);
	CHECK((int)gs.y == 120);
}

TEST_CASE("[msalibs][GDMPMFluid] update runs without crash for 5 steps") {
	GDMPMFluid f;
	f.setup(200);
	for (int i = 0; i < 5; i++)
		f.update();
	PoolVector2Array pos = f.get_particle_positions();
	CHECK(pos.size() == 200);
}

TEST_CASE("[msalibs][GDMPMFluid] particles start within grid bounds") {
	GDMPMFluid f;
	f.setup(100);
	PoolVector2Array pos = f.get_particle_positions();
	for (int i = 0; i < pos.size(); i++) {
		CHECK(pos[i].x >= 0.0f);
		CHECK(pos[i].x <= 160.0f);
		CHECK(pos[i].y >= 0.0f);
		CHECK(pos[i].y <= 120.0f);
	}
}
#endif // DOCTEST
