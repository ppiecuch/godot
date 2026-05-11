/**************************************************************************/
/*  gd_msa_physics.cpp                                                    */
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

#include "gd_msa_physics.h"

MSAPhysicsWorld2D::MSAPhysicsWorld2D() {
	_world.setWorldSize(Vector2(0.0f, 0.0f), Vector2(800.0f, 600.0f));
	_world.setGravity(Vector2(0.0f, 9.8f));
	_world.setDrag(0.99f);
}

int MSAPhysicsWorld2D::add_particle(Vector2 pos, float mass) {
	msa::physics::ParticleT<Vector2> *p = _world.makeParticle(pos, mass);
	int idx = (int)_particles.size();
	_particles.push_back(p);
	return idx;
}

Vector2 MSAPhysicsWorld2D::get_particle_position(int idx) const {
	ERR_FAIL_INDEX_V(idx, (int)_particles.size(), Vector2());
	return _particles[idx]->getPosition();
}

void MSAPhysicsWorld2D::set_particle_position(int idx, Vector2 pos) {
	ERR_FAIL_INDEX(idx, (int)_particles.size());
	_particles[idx]->moveTo(pos);
}

Vector2 MSAPhysicsWorld2D::get_particle_velocity(int idx) const {
	ERR_FAIL_INDEX_V(idx, (int)_particles.size(), Vector2());
	return _particles[idx]->getVelocity();
}

void MSAPhysicsWorld2D::set_particle_velocity(int idx, Vector2 vel) {
	ERR_FAIL_INDEX(idx, (int)_particles.size());
	_particles[idx]->setVelocity(vel);
}

void MSAPhysicsWorld2D::set_particle_fixed(int idx, bool fixed) {
	ERR_FAIL_INDEX(idx, (int)_particles.size());
	if (fixed)
		_particles[idx]->makeFixed();
	else
		_particles[idx]->makeFree();
}

bool MSAPhysicsWorld2D::get_particle_fixed(int idx) const {
	ERR_FAIL_INDEX_V(idx, (int)_particles.size(), false);
	return _particles[idx]->isFixed();
}

int MSAPhysicsWorld2D::get_particle_count() const {
	return (int)_particles.size();
}

PoolVector2Array MSAPhysicsWorld2D::get_particle_positions() const {
	PoolVector2Array out;
	int n = (int)_particles.size();
	out.resize(n);
	PoolVector2Array::Write w = out.write();
	for (int i = 0; i < n; i++) {
		w[i] = _particles[i]->getPosition();
	}
	return out;
}

int MSAPhysicsWorld2D::add_spring(int a, int b, float strength, float rest_length) {
	ERR_FAIL_INDEX_V(a, (int)_particles.size(), -1);
	ERR_FAIL_INDEX_V(b, (int)_particles.size(), -1);
	msa::physics::ParticleT<Vector2> *pa = _particles[a];
	msa::physics::ParticleT<Vector2> *pb = _particles[b];
	if (rest_length < 0.0f) {
		rest_length = (pb->getPosition() - pa->getPosition()).length();
	}
	msa::physics::SpringT<Vector2> *s = _world.makeSpring(pa, pb, strength, rest_length);
	int idx = (int)_springs.size();
	_springs.push_back(s);
	return idx;
}

void MSAPhysicsWorld2D::set_spring_strength(int idx, float strength) {
	ERR_FAIL_INDEX(idx, (int)_springs.size());
	_springs[idx]->setStrength(strength);
}

float MSAPhysicsWorld2D::get_spring_strength(int idx) const {
	ERR_FAIL_INDEX_V(idx, (int)_springs.size(), 0.0f);
	return _springs[idx]->getStrength();
}

int MSAPhysicsWorld2D::get_spring_count() const {
	return (int)_springs.size();
}

int MSAPhysicsWorld2D::add_attraction(int a, int b, float strength) {
	ERR_FAIL_INDEX_V(a, (int)_particles.size(), -1);
	ERR_FAIL_INDEX_V(b, (int)_particles.size(), -1);
	msa::physics::AttractionT<Vector2> *at =
			_world.makeAttraction(_particles[a], _particles[b], strength);
	int idx = (int)_attractions.size();
	_attractions.push_back(at);
	return idx;
}

void MSAPhysicsWorld2D::set_attraction_strength(int idx, float strength) {
	ERR_FAIL_INDEX(idx, (int)_attractions.size());
	_attractions[idx]->setStrength(strength);
}

float MSAPhysicsWorld2D::get_attraction_strength(int idx) const {
	ERR_FAIL_INDEX_V(idx, (int)_attractions.size(), 0.0f);
	return _attractions[idx]->getStrength();
}

int MSAPhysicsWorld2D::get_attraction_count() const {
	return (int)_attractions.size();
}

void MSAPhysicsWorld2D::set_gravity(Vector2 g) {
	_world.setGravity(g);
}

Vector2 MSAPhysicsWorld2D::get_gravity() const {
	return _world.getGravity();
}

void MSAPhysicsWorld2D::set_drag(float d) {
	_world.setDrag(d);
}

float MSAPhysicsWorld2D::get_drag() const {
	return _world.getParams().drag;
}

void MSAPhysicsWorld2D::set_time_step(float ts) {
	_world.setTimeStep(ts);
}

float MSAPhysicsWorld2D::get_time_step() const {
	return _world.getParams().timeStep;
}

void MSAPhysicsWorld2D::set_world_bounds(Vector2 mn, Vector2 mx) {
	_world.setWorldSize(mn, mx);
}

void MSAPhysicsWorld2D::set_collision_enabled(bool b) {
	if (b)
		_world.enableCollision();
	else
		_world.disableCollision();
}

bool MSAPhysicsWorld2D::get_collision_enabled() const {
	return const_cast<msa::physics::WorldT<Vector2> &>(_world).isCollisionEnabled();
}

void MSAPhysicsWorld2D::update() {
	_world.update();
}

void MSAPhysicsWorld2D::clear() {
	_world.clear();
	_particles.clear();
	_springs.clear();
	_attractions.clear();
}

void MSAPhysicsWorld2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("add_particle", "pos", "mass"), &MSAPhysicsWorld2D::add_particle, DEFVAL(1.0f));
	ClassDB::bind_method(D_METHOD("get_particle_position", "idx"), &MSAPhysicsWorld2D::get_particle_position);
	ClassDB::bind_method(D_METHOD("set_particle_position", "idx", "pos"), &MSAPhysicsWorld2D::set_particle_position);
	ClassDB::bind_method(D_METHOD("get_particle_velocity", "idx"), &MSAPhysicsWorld2D::get_particle_velocity);
	ClassDB::bind_method(D_METHOD("set_particle_velocity", "idx", "vel"), &MSAPhysicsWorld2D::set_particle_velocity);
	ClassDB::bind_method(D_METHOD("set_particle_fixed", "idx", "fixed"), &MSAPhysicsWorld2D::set_particle_fixed);
	ClassDB::bind_method(D_METHOD("get_particle_fixed", "idx"), &MSAPhysicsWorld2D::get_particle_fixed);
	ClassDB::bind_method(D_METHOD("get_particle_count"), &MSAPhysicsWorld2D::get_particle_count);
	ClassDB::bind_method(D_METHOD("get_particle_positions"), &MSAPhysicsWorld2D::get_particle_positions);

	ClassDB::bind_method(D_METHOD("add_spring", "a", "b", "strength", "rest_length"), &MSAPhysicsWorld2D::add_spring, DEFVAL(-1.0f));
	ClassDB::bind_method(D_METHOD("set_spring_strength", "idx", "strength"), &MSAPhysicsWorld2D::set_spring_strength);
	ClassDB::bind_method(D_METHOD("get_spring_strength", "idx"), &MSAPhysicsWorld2D::get_spring_strength);
	ClassDB::bind_method(D_METHOD("get_spring_count"), &MSAPhysicsWorld2D::get_spring_count);

	ClassDB::bind_method(D_METHOD("add_attraction", "a", "b", "strength"), &MSAPhysicsWorld2D::add_attraction);
	ClassDB::bind_method(D_METHOD("set_attraction_strength", "idx", "strength"), &MSAPhysicsWorld2D::set_attraction_strength);
	ClassDB::bind_method(D_METHOD("get_attraction_strength", "idx"), &MSAPhysicsWorld2D::get_attraction_strength);
	ClassDB::bind_method(D_METHOD("get_attraction_count"), &MSAPhysicsWorld2D::get_attraction_count);

	ClassDB::bind_method(D_METHOD("set_gravity", "g"), &MSAPhysicsWorld2D::set_gravity);
	ClassDB::bind_method(D_METHOD("get_gravity"), &MSAPhysicsWorld2D::get_gravity);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "gravity"), "set_gravity", "get_gravity");

	ClassDB::bind_method(D_METHOD("set_drag", "d"), &MSAPhysicsWorld2D::set_drag);
	ClassDB::bind_method(D_METHOD("get_drag"), &MSAPhysicsWorld2D::get_drag);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "drag"), "set_drag", "get_drag");

	ClassDB::bind_method(D_METHOD("set_time_step", "ts"), &MSAPhysicsWorld2D::set_time_step);
	ClassDB::bind_method(D_METHOD("get_time_step"), &MSAPhysicsWorld2D::get_time_step);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "time_step"), "set_time_step", "get_time_step");

	ClassDB::bind_method(D_METHOD("set_world_bounds", "min", "max"), &MSAPhysicsWorld2D::set_world_bounds);

	ClassDB::bind_method(D_METHOD("set_collision_enabled", "enabled"), &MSAPhysicsWorld2D::set_collision_enabled);
	ClassDB::bind_method(D_METHOD("get_collision_enabled"), &MSAPhysicsWorld2D::get_collision_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "collision_enabled"), "set_collision_enabled", "get_collision_enabled");

	ClassDB::bind_method(D_METHOD("update"), &MSAPhysicsWorld2D::update);
	ClassDB::bind_method(D_METHOD("clear"), &MSAPhysicsWorld2D::clear);
}

// ============================================================
// Doctests
// ============================================================

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_CASE("[msalibs][MSAPhysicsWorld2D] particles added by index") {
	MSAPhysicsWorld2D w;
	int a = w.add_particle(Vector2(100, 100));
	int b = w.add_particle(Vector2(200, 100));
	CHECK(a == 0);
	CHECK(b == 1);
	CHECK(w.get_particle_count() == 2);
}

TEST_CASE("[msalibs][MSAPhysicsWorld2D] particle position read back after add") {
	MSAPhysicsWorld2D w;
	int idx = w.add_particle(Vector2(50, 75));
	Vector2 pos = w.get_particle_position(idx);
	CHECK(Math::abs(pos.x - 50.0f) < 0.01f);
	CHECK(Math::abs(pos.y - 75.0f) < 0.01f);
}

TEST_CASE("[msalibs][MSAPhysicsWorld2D] spring added and strength readable") {
	MSAPhysicsWorld2D w;
	int a = w.add_particle(Vector2(0, 0));
	int b = w.add_particle(Vector2(100, 0));
	int s = w.add_spring(a, b, 0.5f);
	CHECK(s == 0);
	CHECK(w.get_spring_count() == 1);
	CHECK(Math::abs(w.get_spring_strength(s) - 0.5f) < 0.001f);
}

TEST_CASE("[msalibs][MSAPhysicsWorld2D] fixed particle does not move under gravity") {
	MSAPhysicsWorld2D w;
	w.set_gravity(Vector2(0, 100.0f));
	w.set_world_bounds(Vector2(0, 0), Vector2(1000, 1000));
	int idx = w.add_particle(Vector2(400, 300));
	w.set_particle_fixed(idx, true);
	for (int i = 0; i < 10; i++)
		w.update();
	Vector2 pos = w.get_particle_position(idx);
	CHECK(Math::abs(pos.y - 300.0f) < 0.01f);
}

TEST_CASE("[msalibs][MSAPhysicsWorld2D] free particle moves under gravity") {
	MSAPhysicsWorld2D w;
	w.set_gravity(Vector2(0, 9.8f));
	w.set_world_bounds(Vector2(0, 0), Vector2(1000, 1000));
	int idx = w.add_particle(Vector2(400, 100));
	Vector2 pos0 = w.get_particle_position(idx);
	for (int i = 0; i < 10; i++)
		w.update();
	Vector2 pos1 = w.get_particle_position(idx);
	CHECK(pos1.y > pos0.y);
}
#endif // DOCTEST
