/**************************************************************************/
/*  particles.cpp                                                         */
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

#include "particles.h"

#include <Box2D/Box2D.h>

/***********************************************************************
 * ParticleDefB2
 **********************************************************************/

void ParticleDefB2::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_flags"), &ParticleDefB2::get_flags);
	ClassDB::bind_method(D_METHOD("set_flags", "v"), &ParticleDefB2::set_flags);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "flags"), "set_flags", "get_flags");

	ClassDB::bind_method(D_METHOD("get_position"), &ParticleDefB2::get_position);
	ClassDB::bind_method(D_METHOD("set_position", "v"), &ParticleDefB2::set_position);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "position"), "set_position", "get_position");

	ClassDB::bind_method(D_METHOD("get_velocity"), &ParticleDefB2::get_velocity);
	ClassDB::bind_method(D_METHOD("set_velocity", "v"), &ParticleDefB2::set_velocity);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "velocity"), "set_velocity", "get_velocity");

	ClassDB::bind_method(D_METHOD("get_color"), &ParticleDefB2::get_color);
	ClassDB::bind_method(D_METHOD("set_color", "c"), &ParticleDefB2::set_color);
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_color", "get_color");

	ClassDB::bind_method(D_METHOD("get_lifetime"), &ParticleDefB2::get_lifetime);
	ClassDB::bind_method(D_METHOD("set_lifetime", "v"), &ParticleDefB2::set_lifetime);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "lifetime"), "set_lifetime", "get_lifetime");
}

/***********************************************************************
 * ParticleGroupB2
 **********************************************************************/

ParticleGroupB2::ParticleGroupB2(b2ParticleGroup *g) :
		entity(g) {
	entity->SetUserData(this);
}

ParticleGroupB2::~ParticleGroupB2() {
	if (entity) {
		entity->SetUserData(nullptr);
	}
}

ParticleGroupB2 *ParticleGroupB2::get(b2ParticleGroup *g) {
	return g ? static_cast<ParticleGroupB2 *>(g->GetUserData()) : nullptr;
}

int ParticleGroupB2::get_particle_count() const { return entity->GetParticleCount(); }
int ParticleGroupB2::get_buffer_index() const { return entity->GetBufferIndex(); }
int ParticleGroupB2::get_all_particle_flags() const { return (int)entity->GetAllParticleFlags(); }
int ParticleGroupB2::get_group_flags() const { return (int)entity->GetGroupFlags(); }

void ParticleGroupB2::set_group_flags(int flags) {
	entity->SetGroupFlags((uint32_t)flags);
}

float ParticleGroupB2::get_mass() const { return entity->GetMass(); }
float ParticleGroupB2::get_inertia() const { return entity->GetInertia(); }
Vector2 ParticleGroupB2::get_center() const { return GD(entity->GetCenter()); }
Vector2 ParticleGroupB2::get_linear_velocity() const { return GD(entity->GetLinearVelocity()); }
float ParticleGroupB2::get_angular_velocity() const { return entity->GetAngularVelocity(); }
Vector2 ParticleGroupB2::get_position() const { return GD(entity->GetPosition()); }
float ParticleGroupB2::get_angle() const { return entity->GetAngle(); }

void ParticleGroupB2::apply_force(const Vector2 &force) {
	entity->ApplyForce(B2(force));
}

void ParticleGroupB2::apply_linear_impulse(const Vector2 &impulse) {
	entity->ApplyLinearImpulse(B2(impulse));
}

void ParticleGroupB2::destroy_particles() {
	entity->DestroyParticles(false);
}

void ParticleGroupB2::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_particle_count"), &ParticleGroupB2::get_particle_count);
	ClassDB::bind_method(D_METHOD("get_buffer_index"), &ParticleGroupB2::get_buffer_index);
	ClassDB::bind_method(D_METHOD("get_all_particle_flags"), &ParticleGroupB2::get_all_particle_flags);
	ClassDB::bind_method(D_METHOD("get_group_flags"), &ParticleGroupB2::get_group_flags);
	ClassDB::bind_method(D_METHOD("set_group_flags", "flags"), &ParticleGroupB2::set_group_flags);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "group_flags"), "set_group_flags", "get_group_flags");

	ClassDB::bind_method(D_METHOD("get_mass"), &ParticleGroupB2::get_mass);
	ClassDB::bind_method(D_METHOD("get_inertia"), &ParticleGroupB2::get_inertia);
	ClassDB::bind_method(D_METHOD("get_center"), &ParticleGroupB2::get_center);
	ClassDB::bind_method(D_METHOD("get_linear_velocity"), &ParticleGroupB2::get_linear_velocity);
	ClassDB::bind_method(D_METHOD("get_angular_velocity"), &ParticleGroupB2::get_angular_velocity);
	ClassDB::bind_method(D_METHOD("get_position"), &ParticleGroupB2::get_position);
	ClassDB::bind_method(D_METHOD("get_angle"), &ParticleGroupB2::get_angle);

	ClassDB::bind_method(D_METHOD("apply_force", "force"), &ParticleGroupB2::apply_force);
	ClassDB::bind_method(D_METHOD("apply_linear_impulse", "impulse"), &ParticleGroupB2::apply_linear_impulse);
	ClassDB::bind_method(D_METHOD("destroy_particles"), &ParticleGroupB2::destroy_particles);
}

/***********************************************************************
 * ParticleGroupDefB2
 **********************************************************************/

ParticleGroupDefB2::ParticleGroupDefB2() :
		def(memnew(b2ParticleGroupDef)) {}

ParticleGroupDefB2::~ParticleGroupDefB2() {
	memdelete(def);
}

int ParticleGroupDefB2::get_flags() const { return (int)def->flags; }
void ParticleGroupDefB2::set_flags(int v) { def->flags = (uint32_t)v; }

int ParticleGroupDefB2::get_group_flags() const { return (int)def->groupFlags; }
void ParticleGroupDefB2::set_group_flags(int v) { def->groupFlags = (uint32_t)v; }

Vector2 ParticleGroupDefB2::get_position() const { return GD(def->position); }
void ParticleGroupDefB2::set_position(const Vector2 &v) { def->position = B2(v); }

float ParticleGroupDefB2::get_angle() const { return def->angle; }
void ParticleGroupDefB2::set_angle(float v) { def->angle = v; }

Vector2 ParticleGroupDefB2::get_linear_velocity() const { return GD(def->linearVelocity); }
void ParticleGroupDefB2::set_linear_velocity(const Vector2 &v) { def->linearVelocity = B2(v); }

float ParticleGroupDefB2::get_angular_velocity() const { return def->angularVelocity; }
void ParticleGroupDefB2::set_angular_velocity(float v) { def->angularVelocity = v; }

Color ParticleGroupDefB2::get_color() const {
	return Color(def->color.r / 255.0f, def->color.g / 255.0f, def->color.b / 255.0f, def->color.a / 255.0f);
}

void ParticleGroupDefB2::set_color(const Color &c) {
	def->color.Set(
			(uint8_t)(c.r * 255), (uint8_t)(c.g * 255),
			(uint8_t)(c.b * 255), (uint8_t)(c.a * 255));
}

float ParticleGroupDefB2::get_strength() const { return def->strength; }
void ParticleGroupDefB2::set_strength(float v) { def->strength = v; }

float ParticleGroupDefB2::get_stride() const { return def->stride; }
void ParticleGroupDefB2::set_stride(float v) { def->stride = v; }

float ParticleGroupDefB2::get_lifetime() const { return def->lifetime; }
void ParticleGroupDefB2::set_lifetime(float v) { def->lifetime = v; }

Ref<ShapeB2> ParticleGroupDefB2::get_shape() const { return shape_ref; }

void ParticleGroupDefB2::set_shape(const Ref<ShapeB2> &s) {
	shape_ref = s;
	def->shape = shape_ref.is_valid() ? shape_ref->get_b2() : nullptr;
}

ParticleGroupB2 *ParticleGroupDefB2::instance(ParticleSystemB2 *ps) {
	ERR_FAIL_NULL_V(ps, nullptr);
	def->shape = shape_ref.is_valid() ? shape_ref->get_b2() : nullptr;
	b2ParticleGroup *g = ps->get_b2()->CreateParticleGroup(*def);
	ERR_FAIL_NULL_V(g, nullptr);
	return memnew(ParticleGroupB2(g));
}

void ParticleGroupDefB2::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_flags"), &ParticleGroupDefB2::get_flags);
	ClassDB::bind_method(D_METHOD("set_flags", "v"), &ParticleGroupDefB2::set_flags);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "flags"), "set_flags", "get_flags");

	ClassDB::bind_method(D_METHOD("get_group_flags"), &ParticleGroupDefB2::get_group_flags);
	ClassDB::bind_method(D_METHOD("set_group_flags", "v"), &ParticleGroupDefB2::set_group_flags);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "group_flags"), "set_group_flags", "get_group_flags");

	ClassDB::bind_method(D_METHOD("get_position"), &ParticleGroupDefB2::get_position);
	ClassDB::bind_method(D_METHOD("set_position", "v"), &ParticleGroupDefB2::set_position);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "position"), "set_position", "get_position");

	ClassDB::bind_method(D_METHOD("get_angle"), &ParticleGroupDefB2::get_angle);
	ClassDB::bind_method(D_METHOD("set_angle", "v"), &ParticleGroupDefB2::set_angle);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "angle"), "set_angle", "get_angle");

	ClassDB::bind_method(D_METHOD("get_linear_velocity"), &ParticleGroupDefB2::get_linear_velocity);
	ClassDB::bind_method(D_METHOD("set_linear_velocity", "v"), &ParticleGroupDefB2::set_linear_velocity);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "linear_velocity"), "set_linear_velocity", "get_linear_velocity");

	ClassDB::bind_method(D_METHOD("get_angular_velocity"), &ParticleGroupDefB2::get_angular_velocity);
	ClassDB::bind_method(D_METHOD("set_angular_velocity", "v"), &ParticleGroupDefB2::set_angular_velocity);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "angular_velocity"), "set_angular_velocity", "get_angular_velocity");

	ClassDB::bind_method(D_METHOD("get_color"), &ParticleGroupDefB2::get_color);
	ClassDB::bind_method(D_METHOD("set_color", "c"), &ParticleGroupDefB2::set_color);
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_color", "get_color");

	ClassDB::bind_method(D_METHOD("get_strength"), &ParticleGroupDefB2::get_strength);
	ClassDB::bind_method(D_METHOD("set_strength", "v"), &ParticleGroupDefB2::set_strength);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "strength"), "set_strength", "get_strength");

	ClassDB::bind_method(D_METHOD("get_stride"), &ParticleGroupDefB2::get_stride);
	ClassDB::bind_method(D_METHOD("set_stride", "v"), &ParticleGroupDefB2::set_stride);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "stride"), "set_stride", "get_stride");

	ClassDB::bind_method(D_METHOD("get_lifetime"), &ParticleGroupDefB2::get_lifetime);
	ClassDB::bind_method(D_METHOD("set_lifetime", "v"), &ParticleGroupDefB2::set_lifetime);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "lifetime"), "set_lifetime", "get_lifetime");

	ClassDB::bind_method(D_METHOD("get_shape"), &ParticleGroupDefB2::get_shape);
	ClassDB::bind_method(D_METHOD("set_shape", "s"), &ParticleGroupDefB2::set_shape);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "shape"), "set_shape", "get_shape");

	ClassDB::bind_method(D_METHOD("instance", "ps"), &ParticleGroupDefB2::instance);
}

/***********************************************************************
 * ParticleSystemB2
 **********************************************************************/

ParticleSystemB2::ParticleSystemB2(b2ParticleSystem *ps, b2World *world) :
		entity(ps), b2_world(world) {}

ParticleSystemB2::~ParticleSystemB2() {
	// Destroy Godot wrappers for all live particle groups before b2 cleans them up.
	for (b2ParticleGroup *g = entity->GetParticleGroupList(); g != nullptr;) {
		b2ParticleGroup *next = g->GetNext();
		ParticleGroupB2 *gw = ParticleGroupB2::get(g);
		if (gw) {
			memdelete(gw);
		}
		g = next;
	}
	b2_world->DestroyParticleSystem(entity);
}

int ParticleSystemB2::create_particle(ParticleDefB2 *pdef) {
	ERR_FAIL_NULL_V(pdef, b2_invalidParticleIndex);
	b2ParticleDef pd;
	pd.flags = (uint32_t)pdef->get_flags();
	pd.position = B2(pdef->get_position());
	pd.velocity = B2(pdef->get_velocity());
	Color c = pdef->get_color();
	pd.color.Set(
			(uint8_t)(c.r * 255), (uint8_t)(c.g * 255),
			(uint8_t)(c.b * 255), (uint8_t)(c.a * 255));
	pd.lifetime = pdef->get_lifetime();
	return entity->CreateParticle(pd);
}

int ParticleSystemB2::create_particle_at(const Vector2 &position, int flags) {
	b2ParticleDef pd;
	pd.flags = (uint32_t)flags;
	pd.position = B2(position);
	return entity->CreateParticle(pd);
}

void ParticleSystemB2::destroy_particle(int index) {
	entity->DestroyParticle(index, false);
}

ParticleGroupB2 *ParticleSystemB2::create_group(ParticleGroupDefB2 *gdef) {
	ERR_FAIL_NULL_V(gdef, nullptr);
	return gdef->instance(this);
}

PoolVector2Array ParticleSystemB2::get_position_buffer() const {
	PoolVector2Array out;
	int count = entity->GetParticleCount();
	if (count <= 0)
		return out;
	const b2Vec2 *buf = entity->GetPositionBuffer();
	out.resize(count);
	PoolVector2Array::Write w = out.write();
	for (int i = 0; i < count; ++i) {
		w[i] = GD(buf[i]);
	}
	return out;
}

PoolVector2Array ParticleSystemB2::get_velocity_buffer() const {
	PoolVector2Array out;
	int count = entity->GetParticleCount();
	if (count <= 0)
		return out;
	const b2Vec2 *buf = entity->GetVelocityBuffer();
	out.resize(count);
	PoolVector2Array::Write w = out.write();
	for (int i = 0; i < count; ++i) {
		w[i] = GD(buf[i]);
	}
	return out;
}

PoolColorArray ParticleSystemB2::get_color_buffer() const {
	PoolColorArray out;
	int count = entity->GetParticleCount();
	if (count <= 0)
		return out;
	const b2ParticleColor *buf = entity->GetColorBuffer();
	out.resize(count);
	PoolColorArray::Write w = out.write();
	for (int i = 0; i < count; ++i) {
		w[i] = Color(buf[i].r / 255.0f, buf[i].g / 255.0f,
				buf[i].b / 255.0f, buf[i].a / 255.0f);
	}
	return out;
}

int ParticleSystemB2::get_particle_count() const { return entity->GetParticleCount(); }
int ParticleSystemB2::get_particle_group_count() const { return entity->GetParticleGroupCount(); }

Array ParticleSystemB2::get_particle_group_list() const {
	Array arr;
	for (b2ParticleGroup *g = entity->GetParticleGroupList(); g != nullptr; g = g->GetNext()) {
		ParticleGroupB2 *gw = ParticleGroupB2::get(g);
		if (gw)
			arr.push_back(gw);
	}
	return arr;
}

float ParticleSystemB2::get_density() const { return entity->GetDensity(); }
void ParticleSystemB2::set_density(float v) { entity->SetDensity(v); }

float ParticleSystemB2::get_gravity_scale() const { return entity->GetGravityScale(); }
void ParticleSystemB2::set_gravity_scale(float v) { entity->SetGravityScale(v); }

float ParticleSystemB2::get_radius() const { return entity->GetRadius(); }
void ParticleSystemB2::set_radius(float v) { entity->SetRadius(v); }

float ParticleSystemB2::get_damping() const { return entity->GetDamping(); }
void ParticleSystemB2::set_damping(float v) { entity->SetDamping(v); }

int ParticleSystemB2::get_max_particle_count() const { return entity->GetMaxParticleCount(); }
void ParticleSystemB2::set_max_particle_count(int v) { entity->SetMaxParticleCount(v); }

bool ParticleSystemB2::is_paused() const { return entity->GetPaused(); }
void ParticleSystemB2::set_paused(bool v) { entity->SetPaused(v); }

void ParticleSystemB2::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_particle", "pdef"), &ParticleSystemB2::create_particle);
	ClassDB::bind_method(D_METHOD("create_particle_at", "position", "flags"), &ParticleSystemB2::create_particle_at, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("destroy_particle", "index"), &ParticleSystemB2::destroy_particle);
	ClassDB::bind_method(D_METHOD("create_group", "gdef"), &ParticleSystemB2::create_group);

	ClassDB::bind_method(D_METHOD("get_position_buffer"), &ParticleSystemB2::get_position_buffer);
	ClassDB::bind_method(D_METHOD("get_velocity_buffer"), &ParticleSystemB2::get_velocity_buffer);
	ClassDB::bind_method(D_METHOD("get_color_buffer"), &ParticleSystemB2::get_color_buffer);

	ClassDB::bind_method(D_METHOD("get_particle_count"), &ParticleSystemB2::get_particle_count);
	ClassDB::bind_method(D_METHOD("get_particle_group_count"), &ParticleSystemB2::get_particle_group_count);
	ClassDB::bind_method(D_METHOD("get_particle_group_list"), &ParticleSystemB2::get_particle_group_list);

	BOX2D_PROPERTY(ParticleSystemB2, density, Variant::REAL);
	BOX2D_PROPERTY(ParticleSystemB2, gravity_scale, Variant::REAL);
	BOX2D_PROPERTY(ParticleSystemB2, radius, Variant::REAL);
	BOX2D_PROPERTY(ParticleSystemB2, damping, Variant::REAL);
	BOX2D_PROPERTY(ParticleSystemB2, max_particle_count, Variant::INT);
	BOX2D_PROPERTY_BOOL(ParticleSystemB2, paused);

	BIND_CONSTANT(PARTICLE_WATER);
	BIND_CONSTANT(PARTICLE_ZOMBIE);
	BIND_CONSTANT(PARTICLE_WALL);
	BIND_CONSTANT(PARTICLE_SPRING);
	BIND_CONSTANT(PARTICLE_ELASTIC);
	BIND_CONSTANT(PARTICLE_VISCOUS);
	BIND_CONSTANT(PARTICLE_POWDER);
	BIND_CONSTANT(PARTICLE_TENSILE);
	BIND_CONSTANT(PARTICLE_COLOR_MIXING);
	BIND_CONSTANT(PARTICLE_DESTRUCTION_LISTENER);
	BIND_CONSTANT(PARTICLE_BARRIER);
	BIND_CONSTANT(PARTICLE_STATIC_PRESSURE);
	BIND_CONSTANT(PARTICLE_REACTIVE);
	BIND_CONSTANT(PARTICLE_REPULSIVE);
}

/***********************************************************************
 * ParticleSystemDefB2
 **********************************************************************/

ParticleSystemDefB2::ParticleSystemDefB2() :
		def(memnew(b2ParticleSystemDef)) {}

ParticleSystemDefB2::~ParticleSystemDefB2() {
	memdelete(def);
}

bool ParticleSystemDefB2::is_strict_contact_check() const { return def->strictContactCheck; }
void ParticleSystemDefB2::set_strict_contact_check(bool v) { def->strictContactCheck = v; }

float ParticleSystemDefB2::get_density() const { return def->density; }
void ParticleSystemDefB2::set_density(float v) { def->density = v; }

float ParticleSystemDefB2::get_gravity_scale() const { return def->gravityScale; }
void ParticleSystemDefB2::set_gravity_scale(float v) { def->gravityScale = v; }

float ParticleSystemDefB2::get_radius() const { return def->radius; }
void ParticleSystemDefB2::set_radius(float v) { def->radius = v; }

int ParticleSystemDefB2::get_max_count() const { return def->maxCount; }
void ParticleSystemDefB2::set_max_count(int v) { def->maxCount = v; }

float ParticleSystemDefB2::get_pressure_strength() const { return def->pressureStrength; }
void ParticleSystemDefB2::set_pressure_strength(float v) { def->pressureStrength = v; }

float ParticleSystemDefB2::get_damping_strength() const { return def->dampingStrength; }
void ParticleSystemDefB2::set_damping_strength(float v) { def->dampingStrength = v; }

float ParticleSystemDefB2::get_elastic_strength() const { return def->elasticStrength; }
void ParticleSystemDefB2::set_elastic_strength(float v) { def->elasticStrength = v; }

float ParticleSystemDefB2::get_spring_strength() const { return def->springStrength; }
void ParticleSystemDefB2::set_spring_strength(float v) { def->springStrength = v; }

float ParticleSystemDefB2::get_viscous_strength() const { return def->viscousStrength; }
void ParticleSystemDefB2::set_viscous_strength(float v) { def->viscousStrength = v; }

float ParticleSystemDefB2::get_surface_tension_pressure_strength() const { return def->surfaceTensionPressureStrength; }
void ParticleSystemDefB2::set_surface_tension_pressure_strength(float v) { def->surfaceTensionPressureStrength = v; }

float ParticleSystemDefB2::get_surface_tension_normal_strength() const { return def->surfaceTensionNormalStrength; }
void ParticleSystemDefB2::set_surface_tension_normal_strength(float v) { def->surfaceTensionNormalStrength = v; }

float ParticleSystemDefB2::get_repulsive_strength() const { return def->repulsiveStrength; }
void ParticleSystemDefB2::set_repulsive_strength(float v) { def->repulsiveStrength = v; }

float ParticleSystemDefB2::get_powder_strength() const { return def->powderStrength; }
void ParticleSystemDefB2::set_powder_strength(float v) { def->powderStrength = v; }

float ParticleSystemDefB2::get_ejection_strength() const { return def->ejectionStrength; }
void ParticleSystemDefB2::set_ejection_strength(float v) { def->ejectionStrength = v; }

float ParticleSystemDefB2::get_static_pressure_strength() const { return def->staticPressureStrength; }
void ParticleSystemDefB2::set_static_pressure_strength(float v) { def->staticPressureStrength = v; }

float ParticleSystemDefB2::get_static_pressure_relaxation() const { return def->staticPressureRelaxation; }
void ParticleSystemDefB2::set_static_pressure_relaxation(float v) { def->staticPressureRelaxation = v; }

int ParticleSystemDefB2::get_static_pressure_iterations() const { return def->staticPressureIterations; }
void ParticleSystemDefB2::set_static_pressure_iterations(int v) { def->staticPressureIterations = v; }

float ParticleSystemDefB2::get_color_mixing_strength() const { return def->colorMixingStrength; }
void ParticleSystemDefB2::set_color_mixing_strength(float v) { def->colorMixingStrength = v; }

bool ParticleSystemDefB2::is_destroy_by_age() const { return def->destroyByAge; }
void ParticleSystemDefB2::set_destroy_by_age(bool v) { def->destroyByAge = v; }

float ParticleSystemDefB2::get_lifetime_granularity() const { return def->lifetimeGranularity; }
void ParticleSystemDefB2::set_lifetime_granularity(float v) { def->lifetimeGranularity = v; }

ParticleSystemB2 *ParticleSystemDefB2::instance(WorldB2 *world) {
	ERR_FAIL_NULL_V(world, nullptr);
	return world->create_particle_system(this);
}

void ParticleSystemDefB2::_bind_methods() {
	BOX2D_PROPERTY_BOOL(ParticleSystemDefB2, strict_contact_check);
	BOX2D_PROPERTY(ParticleSystemDefB2, density, Variant::REAL);
	BOX2D_PROPERTY(ParticleSystemDefB2, gravity_scale, Variant::REAL);
	BOX2D_PROPERTY(ParticleSystemDefB2, radius, Variant::REAL);
	BOX2D_PROPERTY(ParticleSystemDefB2, max_count, Variant::INT);
	BOX2D_PROPERTY(ParticleSystemDefB2, pressure_strength, Variant::REAL);
	BOX2D_PROPERTY(ParticleSystemDefB2, damping_strength, Variant::REAL);
	BOX2D_PROPERTY(ParticleSystemDefB2, elastic_strength, Variant::REAL);
	BOX2D_PROPERTY(ParticleSystemDefB2, spring_strength, Variant::REAL);
	BOX2D_PROPERTY(ParticleSystemDefB2, viscous_strength, Variant::REAL);
	BOX2D_PROPERTY(ParticleSystemDefB2, surface_tension_pressure_strength, Variant::REAL);
	BOX2D_PROPERTY(ParticleSystemDefB2, surface_tension_normal_strength, Variant::REAL);
	BOX2D_PROPERTY(ParticleSystemDefB2, repulsive_strength, Variant::REAL);
	BOX2D_PROPERTY(ParticleSystemDefB2, powder_strength, Variant::REAL);
	BOX2D_PROPERTY(ParticleSystemDefB2, ejection_strength, Variant::REAL);
	BOX2D_PROPERTY(ParticleSystemDefB2, static_pressure_strength, Variant::REAL);
	BOX2D_PROPERTY(ParticleSystemDefB2, static_pressure_relaxation, Variant::REAL);
	BOX2D_PROPERTY(ParticleSystemDefB2, static_pressure_iterations, Variant::INT);
	BOX2D_PROPERTY(ParticleSystemDefB2, color_mixing_strength, Variant::REAL);
	BOX2D_PROPERTY_BOOL(ParticleSystemDefB2, destroy_by_age);
	BOX2D_PROPERTY(ParticleSystemDefB2, lifetime_granularity, Variant::REAL);

	ClassDB::bind_method(D_METHOD("instance", "world"), &ParticleSystemDefB2::instance);
}

#ifdef DOCTEST
#include "doctest/doctest.h"

static WorldB2 *make_ps_world() {
	return memnew(WorldB2(memnew(b2World(b2Vec2(0.0f, -10.0f)))));
}

static ParticleSystemB2 *make_ps(WorldB2 *w, float radius = 1.0f) {
	ParticleSystemDefB2 *psd = memnew(ParticleSystemDefB2);
	psd->set_radius(radius);
	ParticleSystemB2 *ps = psd->instance(w);
	memdelete(psd);
	return ps;
}

TEST_SUITE("[liquidfun] ParticleSystemDefB2") {
	TEST_CASE("[liquidfun] ParticleSystemDefB2 defaults") {
		ParticleSystemDefB2 *d = memnew(ParticleSystemDefB2);
		CHECK(d->is_strict_contact_check() == false);
		CHECK(d->get_density() == doctest::Approx(1.0f));
		CHECK(d->get_gravity_scale() == doctest::Approx(1.0f));
		CHECK(d->get_radius() == doctest::Approx(1.0f));
		CHECK(d->get_max_count() == 0);
		CHECK(d->get_pressure_strength() == doctest::Approx(0.05f));
		CHECK(d->get_damping_strength() == doctest::Approx(1.0f));
		CHECK(d->get_elastic_strength() == doctest::Approx(0.25f));
		CHECK(d->get_spring_strength() == doctest::Approx(0.25f));
		CHECK(d->get_viscous_strength() == doctest::Approx(0.25f));
		CHECK(d->get_surface_tension_pressure_strength() == doctest::Approx(0.2f));
		CHECK(d->get_surface_tension_normal_strength() == doctest::Approx(0.2f));
		CHECK(d->get_repulsive_strength() == doctest::Approx(1.0f));
		CHECK(d->get_powder_strength() == doctest::Approx(0.5f));
		CHECK(d->get_ejection_strength() == doctest::Approx(0.5f));
		CHECK(d->get_static_pressure_strength() == doctest::Approx(0.2f));
		CHECK(d->get_static_pressure_relaxation() == doctest::Approx(0.2f));
		CHECK(d->get_static_pressure_iterations() == 8);
		CHECK(d->get_color_mixing_strength() == doctest::Approx(0.5f));
		CHECK(d->is_destroy_by_age() == true);
		memdelete(d);
	}

	TEST_CASE("[liquidfun] ParticleSystemDefB2 setters roundtrip") {
		ParticleSystemDefB2 *d = memnew(ParticleSystemDefB2);
		d->set_density(2.5f);
		d->set_gravity_scale(0.5f);
		d->set_radius(0.3f);
		d->set_max_count(100);
		d->set_pressure_strength(0.1f);
		d->set_damping_strength(0.8f);
		d->set_destroy_by_age(false);
		d->set_static_pressure_iterations(4);
		CHECK(d->get_density() == doctest::Approx(2.5f));
		CHECK(d->get_gravity_scale() == doctest::Approx(0.5f));
		CHECK(d->get_radius() == doctest::Approx(0.3f));
		CHECK(d->get_max_count() == 100);
		CHECK(d->get_pressure_strength() == doctest::Approx(0.1f));
		CHECK(d->get_damping_strength() == doctest::Approx(0.8f));
		CHECK(d->is_destroy_by_age() == false);
		CHECK(d->get_static_pressure_iterations() == 4);
		memdelete(d);
	}
} // TEST_SUITE("[liquidfun] ParticleSystemDefB2")

TEST_SUITE("[liquidfun] ParticleSystemB2") {
	TEST_CASE("[liquidfun] create particle system from def") {
		WorldB2 *w = make_ps_world();
		ParticleSystemB2 *ps = make_ps(w);
		CHECK(ps != nullptr);
		CHECK(ps->get_particle_count() == 0);
		CHECK(ps->get_particle_group_count() == 0);
		memdelete(w);
	}

	TEST_CASE("[liquidfun] particle system physics props defaults") {
		WorldB2 *w = make_ps_world();
		ParticleSystemB2 *ps = make_ps(w, 0.5f);
		CHECK(ps->get_radius() == doctest::Approx(0.5f));
		CHECK(ps->get_density() == doctest::Approx(1.0f));
		CHECK(ps->get_gravity_scale() == doctest::Approx(1.0f));
		CHECK(ps->is_paused() == false);
		CHECK(ps->get_max_particle_count() == 0);
		memdelete(w);
	}

	TEST_CASE("[liquidfun] particle system physics props set/get") {
		WorldB2 *w = make_ps_world();
		ParticleSystemB2 *ps = make_ps(w);
		ps->set_density(2.0f);
		ps->set_gravity_scale(0.5f);
		ps->set_damping(0.3f);
		ps->set_max_particle_count(50);
		ps->set_paused(true);
		CHECK(ps->get_density() == doctest::Approx(2.0f));
		CHECK(ps->get_gravity_scale() == doctest::Approx(0.5f));
		CHECK(ps->get_damping() == doctest::Approx(0.3f));
		CHECK(ps->get_max_particle_count() == 50);
		CHECK(ps->is_paused() == true);
		memdelete(w);
	}

	TEST_CASE("[liquidfun] create and count particles") {
		WorldB2 *w = make_ps_world();
		ParticleSystemB2 *ps = make_ps(w);

		ParticleDefB2 *pd = memnew(ParticleDefB2);
		pd->set_position(Vector2(0.0f, 5.0f));
		pd->set_flags(0); // water
		int idx = ps->create_particle(pd);
		memdelete(pd);

		CHECK(idx >= 0);
		CHECK(ps->get_particle_count() == 1);
		memdelete(w);
	}

	TEST_CASE("[liquidfun] create_particle_at convenience") {
		WorldB2 *w = make_ps_world();
		ParticleSystemB2 *ps = make_ps(w);
		int i0 = ps->create_particle_at(Vector2(1.0f, 0.0f));
		int i1 = ps->create_particle_at(Vector2(2.0f, 0.0f));
		int i2 = ps->create_particle_at(Vector2(3.0f, 0.0f));
		CHECK(i0 >= 0);
		CHECK(i1 > i0);
		CHECK(i2 > i1);
		CHECK(ps->get_particle_count() == 3);
		memdelete(w);
	}

	TEST_CASE("[liquidfun] position buffer matches particle count") {
		WorldB2 *w = make_ps_world();
		ParticleSystemB2 *ps = make_ps(w);
		ps->create_particle_at(Vector2(1.0f, 2.0f));
		ps->create_particle_at(Vector2(3.0f, 4.0f));
		PoolVector2Array pos = ps->get_position_buffer();
		CHECK(pos.size() == ps->get_particle_count());
		memdelete(w);
	}

	TEST_CASE("[liquidfun] velocity buffer reflects initial velocity") {
		WorldB2 *w = make_ps_world();
		ParticleSystemB2 *ps = make_ps(w);
		ParticleDefB2 *pd = memnew(ParticleDefB2);
		pd->set_position(Vector2(0.0f, 0.0f));
		pd->set_velocity(Vector2(5.0f, 0.0f));
		ps->create_particle(pd);
		memdelete(pd);
		PoolVector2Array vel = ps->get_velocity_buffer();
		REQUIRE(vel.size() == 1);
		CHECK(vel[0].x == doctest::Approx(5.0f));
		CHECK(vel[0].y == doctest::Approx(0.0f));
		memdelete(w);
	}

	TEST_CASE("[liquidfun] color buffer roundtrip") {
		WorldB2 *w = make_ps_world();
		ParticleSystemB2 *ps = make_ps(w);
		ParticleDefB2 *pd = memnew(ParticleDefB2);
		pd->set_position(Vector2(0.0f, 0.0f));
		pd->set_color(Color(1.0f, 0.0f, 0.0f, 1.0f)); // red
		ps->create_particle(pd);
		memdelete(pd);
		PoolColorArray colors = ps->get_color_buffer();
		REQUIRE(colors.size() == 1);
		// b2ParticleColor uses uint8, so check within rounding tolerance
		CHECK(colors[0].r > 0.99f);
		CHECK(colors[0].g < 0.01f);
		CHECK(colors[0].b < 0.01f);
		memdelete(w);
	}

	TEST_CASE("[liquidfun] max particle count set/get roundtrip") {
		WorldB2 *w = make_ps_world();
		ParticleSystemB2 *ps = make_ps(w);
		CHECK(ps->get_max_particle_count() == 0); // 0 means no limit
		ps->set_max_particle_count(50);
		CHECK(ps->get_max_particle_count() == 50);
		ps->set_max_particle_count(0);
		CHECK(ps->get_max_particle_count() == 0);
		memdelete(w);
	}

	TEST_CASE("[liquidfun] paused system does not advance particles") {
		WorldB2 *w = make_ps_world();
		ParticleSystemB2 *ps = make_ps(w);
		ps->create_particle_at(Vector2(0.0f, 10.0f));
		ps->set_paused(true);
		PoolVector2Array pos_before = ps->get_position_buffer();
		w->step(1.0f / 60.0f, 6, 2);
		PoolVector2Array pos_after = ps->get_position_buffer();
		REQUIRE(pos_before.size() == 1);
		REQUIRE(pos_after.size() == 1);
		// y should be unchanged — system was paused
		CHECK(pos_after[0].y == doctest::Approx(pos_before[0].y));
		memdelete(w);
	}

	TEST_CASE("[liquidfun] gravity moves particles downward") {
		WorldB2 *w = make_ps_world(); // gravity (0, -10)
		ParticleSystemB2 *ps = make_ps(w);
		ps->create_particle_at(Vector2(0.0f, 10.0f));
		float y_before = ps->get_position_buffer()[0].y;
		w->step(1.0f / 60.0f, 6, 2);
		float y_after = ps->get_position_buffer()[0].y;
		CHECK(y_after < y_before);
		memdelete(w);
	}

	TEST_CASE("[liquidfun] world tracks particle systems") {
		WorldB2 *w = make_ps_world();
		CHECK(w->get_particle_systems().size() == 0);
		ParticleSystemB2 *ps1 = make_ps(w);
		make_ps(w); // ps2: tracked by world, freed on memdelete(w)
		CHECK(w->get_particle_systems().size() == 2);
		w->destroy_particle_system(ps1);
		CHECK(w->get_particle_systems().size() == 1);
		memdelete(w);
	}
} // TEST_SUITE("[liquidfun] ParticleSystemB2")

TEST_SUITE("[liquidfun] ParticleGroupDefB2") {
	TEST_CASE("[liquidfun] ParticleGroupDefB2 defaults") {
		ParticleGroupDefB2 *d = memnew(ParticleGroupDefB2);
		CHECK(d->get_flags() == 0);
		CHECK(d->get_group_flags() == 0);
		CHECK(d->get_position() == Vector2(0, 0));
		CHECK(d->get_angle() == doctest::Approx(0.0f));
		CHECK(d->get_strength() == doctest::Approx(1.0f));
		CHECK(d->get_stride() == doctest::Approx(0.0f));
		CHECK(d->get_lifetime() == doctest::Approx(0.0f));
		CHECK(d->get_shape().is_null());
		memdelete(d);
	}

	TEST_CASE("[liquidfun] ParticleGroupDefB2 setters roundtrip") {
		ParticleGroupDefB2 *d = memnew(ParticleGroupDefB2);
		d->set_position(Vector2(3.0f, -1.0f));
		d->set_angle(0.5f);
		d->set_linear_velocity(Vector2(1.0f, 2.0f));
		d->set_angular_velocity(0.3f);
		d->set_strength(0.7f);
		d->set_flags(4); // spring
		d->set_group_flags(1); // solid
		d->set_color(Color(0.0f, 1.0f, 0.0f, 1.0f));
		CHECK(d->get_position().x == doctest::Approx(3.0f));
		CHECK(d->get_position().y == doctest::Approx(-1.0f));
		CHECK(d->get_angle() == doctest::Approx(0.5f));
		CHECK(d->get_linear_velocity().x == doctest::Approx(1.0f));
		CHECK(d->get_angular_velocity() == doctest::Approx(0.3f));
		CHECK(d->get_strength() == doctest::Approx(0.7f));
		CHECK(d->get_flags() == 4);
		CHECK(d->get_group_flags() == 1);
		memdelete(d);
	}
} // TEST_SUITE("[liquidfun] ParticleGroupDefB2")

TEST_SUITE("[liquidfun] ParticleGroupB2") {
	TEST_CASE("[liquidfun] create group with box shape") {
		WorldB2 *w = make_ps_world();
		ParticleSystemB2 *ps = make_ps(w, 0.3f);

		Ref<ShapeB2> box = Box2D::get()->box(Vector2(2.0f, 2.0f));
		ParticleGroupDefB2 *gdef = memnew(ParticleGroupDefB2);
		gdef->set_shape(box);
		gdef->set_position(Vector2(0.0f, 0.0f));

		ParticleGroupB2 *g = gdef->instance(ps);
		memdelete(gdef);

		CHECK(g != nullptr);
		CHECK(g->get_particle_count() > 0);
		CHECK(ps->get_particle_group_count() == 1);
		CHECK(ps->get_particle_count() > 0);

		memdelete(w);
	}

	TEST_CASE("[liquidfun] group particle count matches system count for single group") {
		WorldB2 *w = make_ps_world();
		ParticleSystemB2 *ps = make_ps(w, 0.3f);

		Ref<ShapeB2> box = Box2D::get()->box(Vector2(1.5f, 1.5f));
		ParticleGroupDefB2 *gdef = memnew(ParticleGroupDefB2);
		gdef->set_shape(box);

		ParticleGroupB2 *g = gdef->instance(ps);
		memdelete(gdef);

		CHECK(g->get_particle_count() == ps->get_particle_count());
		memdelete(w);
	}

	TEST_CASE("[liquidfun] group flags set and get") {
		WorldB2 *w = make_ps_world();
		ParticleSystemB2 *ps = make_ps(w, 0.3f);

		Ref<ShapeB2> box = Box2D::get()->box(Vector2(1.0f, 1.0f));
		ParticleGroupDefB2 *gdef = memnew(ParticleGroupDefB2);
		gdef->set_shape(box);
		// b2_solidParticleGroup = 1
		gdef->set_group_flags(1);

		ParticleGroupB2 *g = gdef->instance(ps);
		memdelete(gdef);

		CHECK(g != nullptr);
		// Flags should include solid
		CHECK((g->get_group_flags() & 1) != 0);
		// Can clear flags
		g->set_group_flags(0);
		CHECK((g->get_group_flags() & 1) == 0);
		memdelete(w);
	}

	TEST_CASE("[liquidfun] group center is inside box bounds") {
		WorldB2 *w = make_ps_world();
		ParticleSystemB2 *ps = make_ps(w, 0.3f);

		Ref<ShapeB2> box = Box2D::get()->box(Vector2(2.0f, 2.0f));
		ParticleGroupDefB2 *gdef = memnew(ParticleGroupDefB2);
		gdef->set_shape(box);
		gdef->set_position(Vector2(5.0f, 5.0f));

		ParticleGroupB2 *g = gdef->instance(ps);
		memdelete(gdef);

		// After a step, center should be near the initial position
		w->step(1.0f / 60.0f, 6, 2);
		Vector2 center = g->get_center();
		CHECK(center.x > 3.0f);
		CHECK(center.x < 7.0f);
		CHECK(center.y > 3.0f);
		CHECK(center.y < 7.0f);

		memdelete(w);
	}

	TEST_CASE("[liquidfun] get_particle_group_list returns group wrappers") {
		WorldB2 *w = make_ps_world();
		ParticleSystemB2 *ps = make_ps(w, 0.3f);

		Ref<ShapeB2> box = Box2D::get()->box(Vector2(1.0f, 1.0f));
		ParticleGroupDefB2 *gdef = memnew(ParticleGroupDefB2);
		gdef->set_shape(box);
		ParticleGroupB2 *g = gdef->instance(ps);
		memdelete(gdef);

		Array groups = ps->get_particle_group_list();
		CHECK(groups.size() == 1);
		CHECK(Object::cast_to<ParticleGroupB2>(groups[0]) == g);

		memdelete(w);
	}

	TEST_CASE("[liquidfun] apply_force does not crash") {
		WorldB2 *w = make_ps_world();
		ParticleSystemB2 *ps = make_ps(w, 0.3f);

		Ref<ShapeB2> box = Box2D::get()->box(Vector2(1.0f, 1.0f));
		ParticleGroupDefB2 *gdef = memnew(ParticleGroupDefB2);
		gdef->set_shape(box);
		ParticleGroupB2 *g = gdef->instance(ps);
		memdelete(gdef);

		g->apply_force(Vector2(10.0f, 0.0f));
		g->apply_linear_impulse(Vector2(0.0f, 5.0f));
		w->step(1.0f / 60.0f, 6, 2);
		// Just verify the world didn't crash
		CHECK(ps->get_particle_count() > 0);
		memdelete(w);
	}
} // TEST_SUITE("[liquidfun] ParticleGroupB2")

#endif // DOCTEST
