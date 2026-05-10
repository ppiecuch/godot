/**************************************************************************/
/*  world.cpp                                                             */
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

#include "godot_box2d.h"

#include "world.h"

#include "core/func_ref.h"

#include <Box2D/Box2D.h>

WorldB2::WorldB2(b2World *entity) :
		entity(entity) {
	entity->SetUserData(this);
}

WorldB2::~WorldB2() {
	// Destroy particle systems first (each calls DestroyParticleSystem internally)
	for (int i = m_particle_systems.size() - 1; i >= 0; --i) {
		memdelete(m_particle_systems[i]);
	}
	m_particle_systems.clear();

	// Destroy all bodies
	while (auto *o = entity->GetBodyList())
		memdelete_notnull(BodyB2::get(o));

	memdelete(entity);
}

ParticleSystemB2 *WorldB2::create_particle_system(ParticleSystemDefB2 *psd) {
	ERR_FAIL_NULL_V(psd, nullptr);
	b2ParticleSystem *b2ps = entity->CreateParticleSystem(psd->get_b2_def());
	ParticleSystemB2 *ps = memnew(ParticleSystemB2(b2ps, entity));
	m_particle_systems.push_back(ps);
	return ps;
}

void WorldB2::destroy_particle_system(ParticleSystemB2 *ps) {
	ERR_FAIL_NULL(ps);
	int idx = m_particle_systems.find(ps);
	if (idx >= 0)
		m_particle_systems.remove(idx);
	memdelete(ps);
}

Array WorldB2::get_particle_systems() const {
	Array arr;
	for (int i = 0; i < m_particle_systems.size(); ++i) {
		arr.push_back(m_particle_systems[i]);
	}
	return arr;
}

void WorldB2::step(float timeStep, int velocityIterations, int positionIterations) {
	entity->Step(timeStep, velocityIterations, positionIterations);
}

void WorldB2::clear_forces() {
	entity->ClearForces();
}

void WorldB2::query_aabb(FuncRef *callback, const Rect2 &aabb) const {
	static struct CB : public b2QueryCallback {
		FuncRef *func;
		bool ReportFixture(b2Fixture *fixture) {
			// declare stuff necessary for the call
			static Variant arg;
			static Variant::CallError ce;
			static const Variant *args[] = { &arg };
			// get fixture object
			arg = FixtureB2::get(fixture);
			// call function
			Variant ret = func->call_func(args, 1, ce);
			return ce.error == Variant::CallError::CALL_OK && ret;
		}
	} cb;

	ERR_FAIL_NULL(callback);
	cb.func = callback;
	entity->QueryAABB(&cb, B2(aabb));
}

void WorldB2::ray_cast(FuncRef *callback, const Vector2 &a, const Vector2 &b) const {
	static struct CB : public b2RayCastCallback {
		FuncRef *func;
		float ReportFixture(b2Fixture *fixture, const b2Vec2 &point, const b2Vec2 &normal, float fraction) {
			// declare stuff necessary for the call
			static Variant arg[4];
			static Variant::CallError ce;
			static const Variant *args[] = { &arg[0], &arg[1], &arg[2], &arg[3] };
			// get fixture object
			arg[0] = FixtureB2::get(fixture);
			arg[1] = GD(point);
			arg[2] = GD(normal);
			arg[3] = fraction;
			// call function
			Variant ret = func->call_func(args, 4, ce);
			return ce.error == Variant::CallError::CALL_OK ? (float)ret : .0f;
		}
	} cb;

	ERR_FAIL_NULL(callback);
	cb.func = callback;
	entity->RayCast(&cb, B2(a), B2(b));
}

Vector2 WorldB2::get_gravity() const {
	return GD(entity->GetGravity());
}

void WorldB2::set_gravity(const Vector2 &value) {
	entity->SetGravity(B2(value));
}

bool WorldB2::is_locked() const {
	return entity->IsLocked();
}

bool WorldB2::get_auto_clear_forces() const {
	return entity->GetAutoClearForces();
}

void WorldB2::set_auto_clear_forces(bool rhs) {
	entity->SetAutoClearForces(rhs);
}

void WorldB2::shift_origin(const Vector2 &origin) {
	entity->ShiftOrigin(B2(origin));
}

Variant WorldB2::get_metadata() const {
	return metadata;
}

void WorldB2::set_metadata(const Variant &rhs) {
	metadata = rhs;
}

void WorldB2::_bind_methods() {
	ClassDB::bind_method(D_METHOD("step", "timeStep", "velocityIterations", "positionIterations"), &WorldB2::step);
	ClassDB::bind_method(D_METHOD("clear_forces"), &WorldB2::clear_forces);

	ClassDB::bind_method(D_METHOD("query_aabb", "callback", "aabb"), &WorldB2::query_aabb);
	ClassDB::bind_method(D_METHOD("ray_cast", "callback", "a", "b"), &WorldB2::ray_cast);

	BOX2D_PROPERTY(WorldB2, gravity, Variant::VECTOR2);

	ClassDB::bind_method(D_METHOD("is_locked"), &WorldB2::is_locked);

	BOX2D_PROPERTY(WorldB2, auto_clear_forces, Variant::BOOL);

	ClassDB::bind_method(D_METHOD("shift_origin", "new_origin"), &WorldB2::shift_origin);

	BOX2D_PROPERTY(WorldB2, metadata, Variant::NIL);

	ClassDB::bind_method(D_METHOD("create_particle_system", "psd"), &WorldB2::create_particle_system);
	ClassDB::bind_method(D_METHOD("destroy_particle_system", "ps"), &WorldB2::destroy_particle_system);
	ClassDB::bind_method(D_METHOD("get_particle_systems"), &WorldB2::get_particle_systems);
}

WorldB2 *WorldB2::get(const b2World *o) {
	return (WorldB2 *)o->GetUserData();
}

#ifdef DOCTEST
#include "doctest/doctest.h"

static WorldB2 *make_world(float gx = 0.0f, float gy = -9.8f) {
	return memnew(WorldB2(memnew(b2World(b2Vec2(gx, gy)))));
}

TEST_SUITE("[liquidfun] WorldB2") {
	TEST_CASE("[liquidfun] world gravity get/set roundtrip") {
		WorldB2 *w = make_world(0.0f, -9.8f);
		CHECK(w->get_gravity().x == doctest::Approx(0.0f));
		CHECK(w->get_gravity().y == doctest::Approx(-9.8f));
		w->set_gravity(Vector2(1.0f, -5.0f));
		CHECK(w->get_gravity().x == doctest::Approx(1.0f));
		CHECK(w->get_gravity().y == doctest::Approx(-5.0f));
		memdelete(w);
	}

	TEST_CASE("[liquidfun] world not locked outside step") {
		WorldB2 *w = make_world();
		CHECK(w->is_locked() == false);
		memdelete(w);
	}

	TEST_CASE("[liquidfun] world auto_clear_forces get/set") {
		WorldB2 *w = make_world();
		CHECK(w->get_auto_clear_forces() == true);
		w->set_auto_clear_forces(false);
		CHECK(w->get_auto_clear_forces() == false);
		w->set_auto_clear_forces(true);
		CHECK(w->get_auto_clear_forces() == true);
		memdelete(w);
	}

	TEST_CASE("[liquidfun] world metadata roundtrip") {
		WorldB2 *w = make_world();
		w->set_metadata(String("hello"));
		CHECK(String(w->get_metadata()) == String("hello"));
		memdelete(w);
	}

	TEST_CASE("[liquidfun] world step advances simulation") {
		WorldB2 *w = make_world(0.0f, -10.0f);
		b2BodyDef bdef;
		bdef.type = b2_dynamicBody;
		bdef.position.Set(0.0f, 10.0f);
		auto *b2b = w->get_b2()->CreateBody(&bdef);
		BodyB2 *body = memnew(BodyB2(b2b));

		b2CircleShape cs;
		cs.m_radius = 0.5f;
		auto *b2f = b2b->CreateFixture(&cs, 1.0f);
		memnew(FixtureB2(b2f));

		float y_before = body->get_position().y;
		w->step(1.0f / 60.0f, 6, 2);
		float y_after = body->get_position().y;
		CHECK(y_after < y_before); // gravity pulls body down
		memdelete(w);
	}
} // TEST_SUITE("[liquidfun] WorldB2")

#endif // DOCTEST
