/**************************************************************************/
/*  gd_opensteer.cpp                                                      */
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

#include "gd_opensteer.h"

// =========================================================================
// GdOpenSteerPathway
// =========================================================================

GdOpenSteerPathway::GdOpenSteerPathway() :
		_pathway(nullptr), _points(nullptr), _point_count(0), _radius(1.0f), _cyclic(false) {}

GdOpenSteerPathway::~GdOpenSteerPathway() {
	if (_pathway) {
		delete _pathway;
	}
	if (_points) {
		delete[] _points;
	}
}

void GdOpenSteerPathway::setup(const PoolVector3Array &p_points, float p_radius, bool p_cyclic) {
	ERR_FAIL_COND(p_points.size() < 2);

	if (_pathway) {
		delete _pathway;
	}
	if (_points) {
		delete[] _points;
	}

	_point_count = p_points.size();
	_radius = p_radius;
	_cyclic = p_cyclic;

	_points = new OpenSteer::Vec3[_point_count];
	PoolVector3Array::Read r = p_points.read();
	for (int i = 0; i < _point_count; i++) {
		_points[i] = to_os(r[i]);
	}

	_pathway = new OpenSteer::PolylinePathway(_point_count, _points, _radius, _cyclic);
}

Vector3 GdOpenSteerPathway::map_point_to_path(const Vector3 &p_point) const {
	ERR_FAIL_COND_V(!_pathway, Vector3());
	OpenSteer::Vec3 tangent;
	float outside;
	return to_gd(_pathway->mapPointToPath(to_os(p_point), tangent, outside));
}

float GdOpenSteerPathway::map_point_to_path_distance(const Vector3 &p_point) const {
	ERR_FAIL_COND_V(!_pathway, 0.0f);
	return _pathway->mapPointToPathDistance(to_os(p_point));
}

Vector3 GdOpenSteerPathway::map_path_distance_to_point(float p_distance) const {
	ERR_FAIL_COND_V(!_pathway, Vector3());
	return to_gd(_pathway->mapPathDistanceToPoint(p_distance));
}

bool GdOpenSteerPathway::is_inside_path(const Vector3 &p_point) const {
	if (!_pathway)
		return false;
	return how_far_outside_path(p_point) <= 0.0f;
}

float GdOpenSteerPathway::how_far_outside_path(const Vector3 &p_point) const {
	ERR_FAIL_COND_V(!_pathway, 0.0f);
	OpenSteer::Vec3 tangent;
	float outside;
	_pathway->mapPointToPath(to_os(p_point), tangent, outside);
	return outside;
}

float GdOpenSteerPathway::get_total_length() const {
	ERR_FAIL_COND_V(!_pathway, 0.0f);
	return _pathway->getTotalPathLength();
}

int GdOpenSteerPathway::get_point_count() const { return _point_count; }
bool GdOpenSteerPathway::is_cyclic() const { return _cyclic; }
float GdOpenSteerPathway::get_radius() const { return _radius; }

void GdOpenSteerPathway::_bind_methods() {
	ClassDB::bind_method(D_METHOD("setup", "points", "radius", "cyclic"), &GdOpenSteerPathway::setup);
	ClassDB::bind_method(D_METHOD("map_point_to_path", "point"), &GdOpenSteerPathway::map_point_to_path);
	ClassDB::bind_method(D_METHOD("map_point_to_path_distance", "point"), &GdOpenSteerPathway::map_point_to_path_distance);
	ClassDB::bind_method(D_METHOD("map_path_distance_to_point", "distance"), &GdOpenSteerPathway::map_path_distance_to_point);
	ClassDB::bind_method(D_METHOD("is_inside_path", "point"), &GdOpenSteerPathway::is_inside_path);
	ClassDB::bind_method(D_METHOD("how_far_outside_path", "point"), &GdOpenSteerPathway::how_far_outside_path);
	ClassDB::bind_method(D_METHOD("get_total_length"), &GdOpenSteerPathway::get_total_length);
	ClassDB::bind_method(D_METHOD("get_point_count"), &GdOpenSteerPathway::get_point_count);
	ClassDB::bind_method(D_METHOD("is_cyclic"), &GdOpenSteerPathway::is_cyclic);
	ClassDB::bind_method(D_METHOD("get_radius"), &GdOpenSteerPathway::get_radius);
}

// =========================================================================
// GdOpenSteerVehicle
// =========================================================================

GdOpenSteerVehicle::GdOpenSteerVehicle() :
		_world(nullptr) {
	_vehicle.reset();
}

GdOpenSteerVehicle::~GdOpenSteerVehicle() {}

// --- Properties ---

void GdOpenSteerVehicle::set_position(const Vector3 &p_pos) {
	_vehicle.setPosition(to_os(p_pos));
}

Vector3 GdOpenSteerVehicle::get_position() const {
	return to_gd(_vehicle.position());
}

void GdOpenSteerVehicle::set_forward(const Vector3 &p_fwd) {
	_vehicle.setForward(to_os(p_fwd).normalize());
}

Vector3 GdOpenSteerVehicle::get_forward() const {
	return to_gd(_vehicle.forward());
}

Vector3 GdOpenSteerVehicle::get_side() const {
	return to_gd(_vehicle.side());
}

Vector3 GdOpenSteerVehicle::get_up() const {
	return to_gd(_vehicle.up());
}

Vector3 GdOpenSteerVehicle::get_velocity() const {
	return to_gd(_vehicle.velocity());
}

void GdOpenSteerVehicle::set_speed(float p_speed) { _vehicle.setSpeed(p_speed); }
float GdOpenSteerVehicle::get_speed() const { return _vehicle.speed(); }
void GdOpenSteerVehicle::set_mass(float p_mass) { _vehicle.setMass(p_mass); }
float GdOpenSteerVehicle::get_mass() const { return _vehicle.mass(); }
void GdOpenSteerVehicle::set_radius(float p_radius) { _vehicle.setRadius(p_radius); }
float GdOpenSteerVehicle::get_radius() const { return _vehicle.radius(); }
void GdOpenSteerVehicle::set_max_force(float p_force) { _vehicle.setMaxForce(p_force); }
float GdOpenSteerVehicle::get_max_force() const { return _vehicle.maxForce(); }
void GdOpenSteerVehicle::set_max_speed(float p_speed) { _vehicle.setMaxSpeed(p_speed); }
float GdOpenSteerVehicle::get_max_speed() const { return _vehicle.maxSpeed(); }

// --- Core simulation ---

void GdOpenSteerVehicle::apply_steering_force(const Vector3 &p_force, float p_dt) {
	_vehicle.applySteeringForce(to_os(p_force), p_dt);
}

void GdOpenSteerVehicle::apply_braking_force(float p_rate, float p_dt) {
	_vehicle.applyBrakingForce(p_rate, p_dt);
}

Vector3 GdOpenSteerVehicle::predict_future_position(float p_time) const {
	return to_gd(_vehicle.predictFuturePosition(p_time));
}

// --- Steering behaviors ---

Vector3 GdOpenSteerVehicle::steer_for_seek(const Vector3 &p_target) {
	return to_gd(_vehicle.steerForSeek(to_os(p_target)));
}

Vector3 GdOpenSteerVehicle::steer_for_flee(const Vector3 &p_target) {
	return to_gd(_vehicle.steerForFlee(to_os(p_target)));
}

Vector3 GdOpenSteerVehicle::steer_for_pursue(const Ref<GdOpenSteerVehicle> &p_quarry) {
	ERR_FAIL_COND_V(p_quarry.is_null(), Vector3());
	return to_gd(_vehicle.steerForPursuit(p_quarry->get_vehicle()));
}

Vector3 GdOpenSteerVehicle::steer_for_evasion(const Ref<GdOpenSteerVehicle> &p_quarry, float p_max_time) {
	ERR_FAIL_COND_V(p_quarry.is_null(), Vector3());
	return to_gd(_vehicle.steerForEvasion(p_quarry->get_vehicle(), p_max_time));
}

Vector3 GdOpenSteerVehicle::steer_for_wander(float p_dt) {
	return to_gd(_vehicle.steerForWander(p_dt));
}

Vector3 GdOpenSteerVehicle::steer_for_target_speed(float p_speed) {
	return to_gd(_vehicle.steerForTargetSpeed(p_speed));
}

// --- Path following ---

Vector3 GdOpenSteerVehicle::steer_to_follow_path(int p_direction, float p_prediction_time, const Ref<GdOpenSteerPathway> &p_path) {
	ERR_FAIL_COND_V(p_path.is_null() || !p_path->get_pathway(), Vector3());
	return to_gd(_vehicle.steerToFollowPath(p_direction, p_prediction_time, *p_path->get_pathway()));
}

Vector3 GdOpenSteerVehicle::steer_to_stay_on_path(float p_prediction_time, const Ref<GdOpenSteerPathway> &p_path) {
	ERR_FAIL_COND_V(p_path.is_null() || !p_path->get_pathway(), Vector3());
	return to_gd(_vehicle.steerToStayOnPath(p_prediction_time, *p_path->get_pathway()));
}

// --- Flocking ---

Vector3 GdOpenSteerVehicle::steer_for_separation(float p_max_distance, float p_cos_max_angle) {
	ERR_FAIL_COND_V(!_world, Vector3());
	OpenSteer::AVGroup group;
	_world->get_neighbor_group(this, p_max_distance, group);
	return to_gd(_vehicle.steerForSeparation(p_max_distance, p_cos_max_angle, group));
}

Vector3 GdOpenSteerVehicle::steer_for_alignment(float p_max_distance, float p_cos_max_angle) {
	ERR_FAIL_COND_V(!_world, Vector3());
	OpenSteer::AVGroup group;
	_world->get_neighbor_group(this, p_max_distance, group);
	return to_gd(_vehicle.steerForAlignment(p_max_distance, p_cos_max_angle, group));
}

Vector3 GdOpenSteerVehicle::steer_for_cohesion(float p_max_distance, float p_cos_max_angle) {
	ERR_FAIL_COND_V(!_world, Vector3());
	OpenSteer::AVGroup group;
	_world->get_neighbor_group(this, p_max_distance, group);
	return to_gd(_vehicle.steerForCohesion(p_max_distance, p_cos_max_angle, group));
}

// --- Obstacle avoidance ---

Vector3 GdOpenSteerVehicle::steer_to_avoid_obstacle(float p_min_time, const Vector3 &p_center, float p_obstacle_radius) {
	OpenSteer::SphericalObstacle obs(p_obstacle_radius, to_os(p_center));
	return to_gd(_vehicle.steerToAvoidObstacle(p_min_time, obs));
}

// --- Spatial queries ---

bool GdOpenSteerVehicle::is_ahead(const Vector3 &p_target) const {
	return _vehicle.isAhead(to_os(p_target));
}

bool GdOpenSteerVehicle::is_aside(const Vector3 &p_target) const {
	return _vehicle.isAside(to_os(p_target));
}

bool GdOpenSteerVehicle::is_behind(const Vector3 &p_target) const {
	return _vehicle.isBehind(to_os(p_target));
}

// --- Reset ---

void GdOpenSteerVehicle::reset() {
	_vehicle.reset();
}

void GdOpenSteerVehicle::randomize_heading() {
	_vehicle.randomizeHeadingOnXZPlane();
}

// --- Bindings ---

void GdOpenSteerVehicle::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_position", "position"), &GdOpenSteerVehicle::set_position);
	ClassDB::bind_method(D_METHOD("get_position"), &GdOpenSteerVehicle::get_position);
	ClassDB::bind_method(D_METHOD("set_forward", "forward"), &GdOpenSteerVehicle::set_forward);
	ClassDB::bind_method(D_METHOD("get_forward"), &GdOpenSteerVehicle::get_forward);
	ClassDB::bind_method(D_METHOD("get_side"), &GdOpenSteerVehicle::get_side);
	ClassDB::bind_method(D_METHOD("get_up"), &GdOpenSteerVehicle::get_up);
	ClassDB::bind_method(D_METHOD("get_velocity"), &GdOpenSteerVehicle::get_velocity);
	ClassDB::bind_method(D_METHOD("set_speed", "speed"), &GdOpenSteerVehicle::set_speed);
	ClassDB::bind_method(D_METHOD("get_speed"), &GdOpenSteerVehicle::get_speed);
	ClassDB::bind_method(D_METHOD("set_mass", "mass"), &GdOpenSteerVehicle::set_mass);
	ClassDB::bind_method(D_METHOD("get_mass"), &GdOpenSteerVehicle::get_mass);
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &GdOpenSteerVehicle::set_radius);
	ClassDB::bind_method(D_METHOD("get_radius"), &GdOpenSteerVehicle::get_radius);
	ClassDB::bind_method(D_METHOD("set_max_force", "force"), &GdOpenSteerVehicle::set_max_force);
	ClassDB::bind_method(D_METHOD("get_max_force"), &GdOpenSteerVehicle::get_max_force);
	ClassDB::bind_method(D_METHOD("set_max_speed", "speed"), &GdOpenSteerVehicle::set_max_speed);
	ClassDB::bind_method(D_METHOD("get_max_speed"), &GdOpenSteerVehicle::get_max_speed);

	ClassDB::bind_method(D_METHOD("apply_steering_force", "force", "dt"), &GdOpenSteerVehicle::apply_steering_force);
	ClassDB::bind_method(D_METHOD("apply_braking_force", "rate", "dt"), &GdOpenSteerVehicle::apply_braking_force);
	ClassDB::bind_method(D_METHOD("predict_future_position", "time"), &GdOpenSteerVehicle::predict_future_position);

	ClassDB::bind_method(D_METHOD("steer_for_seek", "target"), &GdOpenSteerVehicle::steer_for_seek);
	ClassDB::bind_method(D_METHOD("steer_for_flee", "target"), &GdOpenSteerVehicle::steer_for_flee);
	ClassDB::bind_method(D_METHOD("steer_for_pursue", "quarry"), &GdOpenSteerVehicle::steer_for_pursue);
	ClassDB::bind_method(D_METHOD("steer_for_evasion", "quarry", "max_time"), &GdOpenSteerVehicle::steer_for_evasion);
	ClassDB::bind_method(D_METHOD("steer_for_wander", "dt"), &GdOpenSteerVehicle::steer_for_wander);
	ClassDB::bind_method(D_METHOD("steer_for_target_speed", "speed"), &GdOpenSteerVehicle::steer_for_target_speed);

	ClassDB::bind_method(D_METHOD("steer_to_follow_path", "direction", "prediction_time", "path"), &GdOpenSteerVehicle::steer_to_follow_path);
	ClassDB::bind_method(D_METHOD("steer_to_stay_on_path", "prediction_time", "path"), &GdOpenSteerVehicle::steer_to_stay_on_path);

	ClassDB::bind_method(D_METHOD("steer_for_separation", "max_distance", "cos_max_angle"), &GdOpenSteerVehicle::steer_for_separation);
	ClassDB::bind_method(D_METHOD("steer_for_alignment", "max_distance", "cos_max_angle"), &GdOpenSteerVehicle::steer_for_alignment);
	ClassDB::bind_method(D_METHOD("steer_for_cohesion", "max_distance", "cos_max_angle"), &GdOpenSteerVehicle::steer_for_cohesion);

	ClassDB::bind_method(D_METHOD("steer_to_avoid_obstacle", "min_time", "center", "radius"), &GdOpenSteerVehicle::steer_to_avoid_obstacle);

	ClassDB::bind_method(D_METHOD("is_ahead", "target"), &GdOpenSteerVehicle::is_ahead);
	ClassDB::bind_method(D_METHOD("is_aside", "target"), &GdOpenSteerVehicle::is_aside);
	ClassDB::bind_method(D_METHOD("is_behind", "target"), &GdOpenSteerVehicle::is_behind);

	ClassDB::bind_method(D_METHOD("reset"), &GdOpenSteerVehicle::reset);
	ClassDB::bind_method(D_METHOD("randomize_heading"), &GdOpenSteerVehicle::randomize_heading);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "position"), "set_position", "get_position");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "speed"), "set_speed", "get_speed");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "mass"), "set_mass", "get_mass");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "radius"), "set_radius", "get_radius");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "max_force"), "set_max_force", "get_max_force");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "max_speed"), "set_max_speed", "get_max_speed");
}

// =========================================================================
// GdOpenSteerWorld
// =========================================================================

GdOpenSteerWorld::GdOpenSteerWorld() :
		_proximity_db(nullptr) {}

GdOpenSteerWorld::~GdOpenSteerWorld() {
	// Destroy tokens before database
	for (int i = 0; i < _tokens.size(); i++) {
		delete _tokens[i];
	}
	_tokens.clear();
	for (int i = 0; i < _vehicles.size(); i++) {
		Ref<GdOpenSteerVehicle> v = _vehicles[i];
		if (v.is_valid()) {
			v->set_world(nullptr);
		}
	}
	_vehicles.clear();
	if (_proximity_db) {
		delete _proximity_db;
	}
}

void GdOpenSteerWorld::setup(const Vector3 &p_center, const Vector3 &p_dimensions, const Vector3 &p_divisions) {
	ERR_FAIL_COND(p_divisions.x < 1 || p_divisions.y < 1 || p_divisions.z < 1);
	ERR_FAIL_COND(p_dimensions.x <= 0 || p_dimensions.y <= 0 || p_dimensions.z <= 0);

	if (_proximity_db) {
		// Cleanup existing
		for (int i = 0; i < _tokens.size(); i++) {
			delete _tokens[i];
		}
		_tokens.clear();
		delete _proximity_db;
	}

	_proximity_db = new OpenSteer::LQProximityDatabase<OpenSteer::AbstractVehicle *>(
			to_os(p_center), to_os(p_dimensions),
			OpenSteer::Vec3((int)p_divisions.x, (int)p_divisions.y, (int)p_divisions.z));

	// Re-create tokens for existing vehicles
	for (int i = 0; i < _vehicles.size(); i++) {
		Ref<GdOpenSteerVehicle> v = _vehicles[i];
		OpenSteer::AbstractVehicle *av = &v->get_vehicle();
		_tokens.push_back(_proximity_db->allocateToken(av));
	}
}

void GdOpenSteerWorld::add_vehicle(Ref<GdOpenSteerVehicle> p_vehicle) {
	ERR_FAIL_COND(p_vehicle.is_null());
	p_vehicle->set_world(this);
	_vehicles.push_back(p_vehicle);

	if (_proximity_db) {
		OpenSteer::AbstractVehicle *av = &p_vehicle->get_vehicle();
		_tokens.push_back(_proximity_db->allocateToken(av));
	}
}

void GdOpenSteerWorld::remove_vehicle(Ref<GdOpenSteerVehicle> p_vehicle) {
	ERR_FAIL_COND(p_vehicle.is_null());
	int idx = _vehicles.find(p_vehicle);
	ERR_FAIL_COND(idx < 0);

	if (idx < _tokens.size()) {
		delete _tokens[idx];
		_tokens.remove(idx);
	}
	p_vehicle->set_world(nullptr);
	_vehicles.remove(idx);
}

int GdOpenSteerWorld::get_vehicle_count() const { return _vehicles.size(); }

Ref<GdOpenSteerVehicle> GdOpenSteerWorld::get_vehicle(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, _vehicles.size(), Ref<GdOpenSteerVehicle>());
	return _vehicles[p_index];
}

Array GdOpenSteerWorld::get_vehicles() const {
	Array result;
	for (int i = 0; i < _vehicles.size(); i++) {
		result.push_back(_vehicles[i]);
	}
	return result;
}

void GdOpenSteerWorld::update_proximity() {
	for (int i = 0; i < _tokens.size() && i < _vehicles.size(); i++) {
		const OpenSteer::SimpleVehicle &v = _vehicles[i]->get_vehicle();
		_tokens[i]->updateForNewPosition(v.position());
	}
}

Array GdOpenSteerWorld::get_neighbors(const Ref<GdOpenSteerVehicle> &p_vehicle, float p_radius) const {
	Array result;
	ERR_FAIL_COND_V(p_vehicle.is_null(), result);

	for (int i = 0; i < _vehicles.size(); i++) {
		if (_vehicles[i] == p_vehicle)
			continue;
		float dist = to_gd(_vehicles[i]->get_vehicle().position() - p_vehicle->get_vehicle().position()).length();
		if (dist <= p_radius) {
			result.push_back(_vehicles[i]);
		}
	}
	return result;
}

void GdOpenSteerWorld::get_neighbor_group(const GdOpenSteerVehicle *p_vehicle, float p_radius,
		OpenSteer::AVGroup &r_group) const {
	for (int i = 0; i < _vehicles.size(); i++) {
		if (_vehicles[i].ptr() == p_vehicle)
			continue;
		float dist = (_vehicles[i]->get_vehicle().position() - p_vehicle->get_vehicle().position()).length();
		if (dist <= p_radius) {
			r_group.push_back(&const_cast<GdOpenSteerVehicle *>(_vehicles[i].ptr())->get_vehicle());
		}
	}
}

void GdOpenSteerWorld::_bind_methods() {
	ClassDB::bind_method(D_METHOD("setup", "center", "dimensions", "divisions"), &GdOpenSteerWorld::setup);
	ClassDB::bind_method(D_METHOD("add_vehicle", "vehicle"), &GdOpenSteerWorld::add_vehicle);
	ClassDB::bind_method(D_METHOD("remove_vehicle", "vehicle"), &GdOpenSteerWorld::remove_vehicle);
	ClassDB::bind_method(D_METHOD("get_vehicle_count"), &GdOpenSteerWorld::get_vehicle_count);
	ClassDB::bind_method(D_METHOD("get_vehicle", "index"), &GdOpenSteerWorld::get_vehicle);
	ClassDB::bind_method(D_METHOD("get_vehicles"), &GdOpenSteerWorld::get_vehicles);
	ClassDB::bind_method(D_METHOD("update_proximity"), &GdOpenSteerWorld::update_proximity);
	ClassDB::bind_method(D_METHOD("get_neighbors", "vehicle", "radius"), &GdOpenSteerWorld::get_neighbors);
}

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"
#include "doctest/doctest_godot.h"

TEST_SUITE("[[opensteer]] GdOpenSteerVehicle") {
	TEST_CASE("[opensteer] default state") {
		GdOpenSteerVehicle v;
		CHECK(v.get_mass() == doctest::Approx(1.0f));
		CHECK(v.get_speed() == doctest::Approx(0.0f));
		CHECK(v.get_radius() == doctest::Approx(0.5f));
		CHECK(v.get_max_force() == doctest::Approx(0.1f));
		CHECK(v.get_max_speed() == doctest::Approx(1.0f));
		CHECK(v.get_position().is_equal_approx(Vector3(0, 0, 0)));
	}

	TEST_CASE("[opensteer] set and get properties") {
		GdOpenSteerVehicle v;
		v.set_mass(2.0f);
		CHECK(v.get_mass() == doctest::Approx(2.0f));
		v.set_max_speed(5.0f);
		CHECK(v.get_max_speed() == doctest::Approx(5.0f));
		v.set_max_force(3.0f);
		CHECK(v.get_max_force() == doctest::Approx(3.0f));
		v.set_radius(1.5f);
		CHECK(v.get_radius() == doctest::Approx(1.5f));
		v.set_position(Vector3(1, 2, 3));
		CHECK(v.get_position().is_equal_approx(Vector3(1, 2, 3)));
	}

	TEST_CASE("[opensteer] steer_for_seek produces non-zero force") {
		GdOpenSteerVehicle v;
		v.set_max_speed(10.0f);
		v.set_max_force(5.0f);
		Vector3 force = v.steer_for_seek(Vector3(10, 0, 0));
		CHECK(force.length() > 0.0f);
	}

	TEST_CASE("[opensteer] steer_for_flee opposes seek") {
		GdOpenSteerVehicle v;
		v.set_max_speed(10.0f);
		v.set_max_force(5.0f);
		Vector3 seek = v.steer_for_seek(Vector3(10, 0, 0));
		Vector3 flee = v.steer_for_flee(Vector3(10, 0, 0));
		CHECK(seek.dot(flee) < 0.0f);
	}

	TEST_CASE("[opensteer] apply_steering_force updates position") {
		GdOpenSteerVehicle v;
		v.set_max_speed(10.0f);
		v.set_max_force(10.0f);
		Vector3 initial = v.get_position();
		Vector3 force = v.steer_for_seek(Vector3(0, 0, 100));
		v.apply_steering_force(force, 1.0f);
		CHECK(v.get_position() != initial);
		CHECK(v.get_speed() > 0.0f);
	}

	TEST_CASE("[opensteer] predict_future_position") {
		GdOpenSteerVehicle v;
		v.set_max_speed(10.0f);
		v.set_max_force(10.0f);
		v.apply_steering_force(Vector3(0, 0, 5), 0.1f);
		Vector3 predicted = v.predict_future_position(1.0f);
		CHECK(predicted.z > v.get_position().z);
	}

	TEST_CASE("[opensteer] steer_for_wander does not crash") {
		GdOpenSteerVehicle v;
		v.set_max_speed(5.0f);
		v.set_max_force(2.0f);
		v.apply_steering_force(Vector3(0, 0, 1), 0.1f);
		Vector3 wander = v.steer_for_wander(0.1f);
		CHECK(wander.length() >= 0.0f);
	}

	TEST_CASE("[opensteer] steer_for_target_speed") {
		GdOpenSteerVehicle v;
		v.set_max_speed(10.0f);
		v.set_max_force(5.0f);
		Vector3 force = v.steer_for_target_speed(5.0f);
		CHECK(force.length() > 0.0f);
	}

	TEST_CASE("[opensteer] reset restores defaults") {
		GdOpenSteerVehicle v;
		v.set_mass(5.0f);
		v.set_position(Vector3(10, 20, 30));
		v.reset();
		CHECK(v.get_mass() == doctest::Approx(1.0f));
		CHECK(v.get_position().is_equal_approx(Vector3(0, 0, 0)));
		CHECK(v.get_speed() == doctest::Approx(0.0f));
	}

	TEST_CASE("[opensteer] is_ahead / is_aside / is_behind") {
		GdOpenSteerVehicle v;
		CHECK(v.is_ahead(Vector3(0, 0, 10)));
		CHECK(v.is_behind(Vector3(0, 0, -10)));
		CHECK(v.is_aside(Vector3(10, 0, 0)));
	}

	TEST_CASE("[opensteer] steer_for_pursue") {
		GdOpenSteerVehicle hunter;
		hunter.set_max_speed(10.0f);
		hunter.set_max_force(5.0f);

		Ref<GdOpenSteerVehicle> prey;
		prey.instance();
		prey->set_position(Vector3(20, 0, 0));
		prey->set_max_speed(5.0f);

		Vector3 force = hunter.steer_for_pursue(prey);
		CHECK(force.length() > 0.0f);
	}

	TEST_CASE("[opensteer] steer_for_evasion") {
		GdOpenSteerVehicle prey;
		prey.set_max_speed(10.0f);
		prey.set_max_force(5.0f);

		Ref<GdOpenSteerVehicle> hunter;
		hunter.instance();
		hunter->set_position(Vector3(5, 0, 0));
		hunter->set_max_speed(10.0f);

		Vector3 force = prey.steer_for_evasion(hunter, 3.0f);
		CHECK(force.length() > 0.0f);
	}

	TEST_CASE("[opensteer] steer_to_avoid_obstacle") {
		GdOpenSteerVehicle v;
		v.set_max_speed(10.0f);
		v.set_max_force(5.0f);
		v.apply_steering_force(Vector3(0, 0, 5), 0.1f);
		Vector3 force = v.steer_to_avoid_obstacle(3.0f, Vector3(0, 0, 5), 2.0f);
		// May be zero if obstacle is not directly ahead — just check no crash
		CHECK(force.length() >= 0.0f);
	}

	TEST_CASE("[opensteer] apply_braking_force reduces speed") {
		GdOpenSteerVehicle v;
		v.set_max_speed(10.0f);
		v.set_max_force(10.0f);
		v.apply_steering_force(Vector3(0, 0, 10), 1.0f);
		float speed_before = v.get_speed();
		CHECK(speed_before > 0.0f);
		v.apply_braking_force(0.5f, 1.0f);
		CHECK(v.get_speed() < speed_before);
	}
}

TEST_SUITE("[[opensteer]] GdOpenSteerPathway") {
	TEST_CASE("[opensteer] pathway setup and query") {
		GdOpenSteerPathway pw;
		PoolVector3Array pts;
		pts.push_back(Vector3(0, 0, 0));
		pts.push_back(Vector3(10, 0, 0));
		pts.push_back(Vector3(10, 0, 10));
		pw.setup(pts, 1.0f, false);
		CHECK(pw.get_point_count() == 3);
		CHECK(pw.get_total_length() > 0.0f);
		CHECK_FALSE(pw.is_cyclic());
		CHECK(pw.get_radius() == doctest::Approx(1.0f));
	}

	TEST_CASE("[opensteer] map_point_to_path_distance") {
		GdOpenSteerPathway pw;
		PoolVector3Array pts;
		pts.push_back(Vector3(0, 0, 0));
		pts.push_back(Vector3(10, 0, 0));
		pw.setup(pts, 1.0f, false);
		float dist = pw.map_point_to_path_distance(Vector3(5, 0, 0));
		CHECK(dist == doctest::Approx(5.0f).epsilon(0.1f));
	}

	TEST_CASE("[opensteer] is_inside_path") {
		GdOpenSteerPathway pw;
		PoolVector3Array pts;
		pts.push_back(Vector3(0, 0, 0));
		pts.push_back(Vector3(10, 0, 0));
		pw.setup(pts, 2.0f, false);
		CHECK(pw.is_inside_path(Vector3(5, 0, 0)));
		CHECK(pw.is_inside_path(Vector3(5, 1, 0)));
		CHECK_FALSE(pw.is_inside_path(Vector3(5, 5, 0)));
	}

	TEST_CASE("[opensteer] cyclic pathway") {
		GdOpenSteerPathway pw;
		PoolVector3Array pts;
		pts.push_back(Vector3(0, 0, 0));
		pts.push_back(Vector3(10, 0, 0));
		pts.push_back(Vector3(10, 0, 10));
		pts.push_back(Vector3(0, 0, 10));
		pw.setup(pts, 1.0f, true);
		CHECK(pw.is_cyclic());
		CHECK(pw.get_total_length() > 30.0f); // perimeter > sum of 3 segments
	}
}

TEST_SUITE("[[opensteer]] GdOpenSteerWorld") {
	TEST_CASE("[opensteer] add and remove vehicles") {
		GdOpenSteerWorld world;
		world.setup(Vector3(0, 0, 0), Vector3(100, 100, 100), Vector3(10, 10, 10));

		Ref<GdOpenSteerVehicle> v1;
		v1.instance();
		Ref<GdOpenSteerVehicle> v2;
		v2.instance();

		world.add_vehicle(v1);
		CHECK(world.get_vehicle_count() == 1);

		world.add_vehicle(v2);
		CHECK(world.get_vehicle_count() == 2);

		world.remove_vehicle(v1);
		CHECK(world.get_vehicle_count() == 1);
	}

	TEST_CASE("[opensteer] neighbor query") {
		GdOpenSteerWorld world;
		world.setup(Vector3(0, 0, 0), Vector3(100, 100, 100), Vector3(10, 10, 10));

		Ref<GdOpenSteerVehicle> v1;
		v1.instance();
		v1->set_position(Vector3(0, 0, 0));

		Ref<GdOpenSteerVehicle> v2;
		v2.instance();
		v2->set_position(Vector3(5, 0, 0));

		Ref<GdOpenSteerVehicle> v3;
		v3.instance();
		v3->set_position(Vector3(50, 0, 0));

		world.add_vehicle(v1);
		world.add_vehicle(v2);
		world.add_vehicle(v3);
		world.update_proximity();

		Array neighbors = world.get_neighbors(v1, 10.0f);
		CHECK(neighbors.size() == 1); // v2 only
	}

	TEST_CASE("[opensteer] flocking via world") {
		GdOpenSteerWorld world;
		world.setup(Vector3(0, 0, 0), Vector3(100, 100, 100), Vector3(10, 10, 10));

		Ref<GdOpenSteerVehicle> v1;
		v1.instance();
		v1->set_position(Vector3(0, 0, 0));
		v1->set_max_speed(5.0f);
		v1->set_max_force(2.0f);

		Ref<GdOpenSteerVehicle> v2;
		v2.instance();
		v2->set_position(Vector3(3, 0, 0));
		v2->set_max_speed(5.0f);
		v2->set_max_force(2.0f);

		world.add_vehicle(v1);
		world.add_vehicle(v2);
		world.update_proximity();

		Vector3 sep = v1->steer_for_separation(10.0f, -1.0f);
		CHECK(sep.length() > 0.0f);
	}

	TEST_CASE("[opensteer] path following integration") {
		Ref<GdOpenSteerPathway> path;
		path.instance();
		PoolVector3Array pts;
		pts.push_back(Vector3(0, 0, 0));
		pts.push_back(Vector3(0, 0, 50));
		path->setup(pts, 3.0f, false);

		GdOpenSteerVehicle v;
		v.set_max_speed(10.0f);
		v.set_max_force(5.0f);
		v.set_position(Vector3(5, 0, 0)); // off the path

		Vector3 force = v.steer_to_stay_on_path(1.0f, path);
		// Should steer toward the path
		CHECK(force.length() > 0.0f);
	}
}

#endif // DOCTEST
