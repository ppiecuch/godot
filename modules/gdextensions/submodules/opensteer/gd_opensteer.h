/**************************************************************************/
/*  gd_opensteer.h                                                        */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#ifndef GD_OPENSTEER_H
#define GD_OPENSTEER_H

#include "core/reference.h"

#include "OpenSteer/Obstacle.h"
#include "OpenSteer/Pathway.h"
#include "OpenSteer/Proximity.h"
#include "OpenSteer/SimpleVehicle.h"

// --- Vec3 conversion helpers ---

static inline OpenSteer::Vec3 to_os(const Vector3 &v) {
	return OpenSteer::Vec3(v.x, v.y, v.z);
}

static inline Vector3 to_gd(const OpenSteer::Vec3 &v) {
	return Vector3(v.x, v.y, v.z);
}

class GdOpenSteerWorld;

// --- GdOpenSteerPathway ---

class GdOpenSteerPathway : public Reference {
	GDCLASS(GdOpenSteerPathway, Reference);

	OpenSteer::PolylinePathway *_pathway;
	OpenSteer::Vec3 *_points;
	int _point_count;
	float _radius;
	bool _cyclic;

protected:
	static void _bind_methods();

public:
	void setup(const PoolVector3Array &p_points, float p_radius, bool p_cyclic);

	Vector3 map_point_to_path(const Vector3 &p_point) const;
	float map_point_to_path_distance(const Vector3 &p_point) const;
	Vector3 map_path_distance_to_point(float p_distance) const;
	bool is_inside_path(const Vector3 &p_point) const;
	float how_far_outside_path(const Vector3 &p_point) const;
	float get_total_length() const;
	int get_point_count() const;
	bool is_cyclic() const;
	float get_radius() const;

	OpenSteer::PolylinePathway *get_pathway() const { return _pathway; }

	GdOpenSteerPathway();
	~GdOpenSteerPathway();
};

// --- GdOpenSteerVehicle ---

class GdOpenSteerVehicle : public Reference {
	GDCLASS(GdOpenSteerVehicle, Reference);

	OpenSteer::SimpleVehicle _vehicle;
	GdOpenSteerWorld *_world;

protected:
	static void _bind_methods();

public:
	// Properties
	void set_position(const Vector3 &p_pos);
	Vector3 get_position() const;
	void set_forward(const Vector3 &p_fwd);
	Vector3 get_forward() const;
	Vector3 get_side() const;
	Vector3 get_up() const;
	Vector3 get_velocity() const;
	void set_speed(float p_speed);
	float get_speed() const;
	void set_mass(float p_mass);
	float get_mass() const;
	void set_radius(float p_radius);
	float get_radius() const;
	void set_max_force(float p_force);
	float get_max_force() const;
	void set_max_speed(float p_speed);
	float get_max_speed() const;

	// Core simulation
	void apply_steering_force(const Vector3 &p_force, float p_dt);
	void apply_braking_force(float p_rate, float p_dt);
	Vector3 predict_future_position(float p_time) const;

	// Steering behaviors
	Vector3 steer_for_seek(const Vector3 &p_target);
	Vector3 steer_for_flee(const Vector3 &p_target);
	Vector3 steer_for_pursue(const Ref<GdOpenSteerVehicle> &p_quarry);
	Vector3 steer_for_evasion(const Ref<GdOpenSteerVehicle> &p_quarry, float p_max_time);
	Vector3 steer_for_wander(float p_dt);
	Vector3 steer_for_target_speed(float p_speed);

	// Path following
	Vector3 steer_to_follow_path(int p_direction, float p_prediction_time, const Ref<GdOpenSteerPathway> &p_path);
	Vector3 steer_to_stay_on_path(float p_prediction_time, const Ref<GdOpenSteerPathway> &p_path);

	// Flocking (requires world)
	Vector3 steer_for_separation(float p_max_distance, float p_cos_max_angle);
	Vector3 steer_for_alignment(float p_max_distance, float p_cos_max_angle);
	Vector3 steer_for_cohesion(float p_max_distance, float p_cos_max_angle);

	// Obstacle avoidance
	Vector3 steer_to_avoid_obstacle(float p_min_time, const Vector3 &p_center, float p_obstacle_radius);

	// Spatial queries
	bool is_ahead(const Vector3 &p_target) const;
	bool is_aside(const Vector3 &p_target) const;
	bool is_behind(const Vector3 &p_target) const;

	// Reset
	void reset();
	void randomize_heading();

	// Internal
	OpenSteer::SimpleVehicle &get_vehicle() { return _vehicle; }
	const OpenSteer::SimpleVehicle &get_vehicle() const { return _vehicle; }
	void set_world(GdOpenSteerWorld *p_world) { _world = p_world; }
	GdOpenSteerWorld *get_world() const { return _world; }

	GdOpenSteerVehicle();
	~GdOpenSteerVehicle();
};

// --- GdOpenSteerWorld ---

class GdOpenSteerWorld : public Reference {
	GDCLASS(GdOpenSteerWorld, Reference);

	typedef OpenSteer::AbstractProximityDatabase<OpenSteer::AbstractVehicle *> ProximityDB;
	typedef OpenSteer::AbstractTokenForProximityDatabase<OpenSteer::AbstractVehicle *> ProximityToken;

	Vector<Ref<GdOpenSteerVehicle>> _vehicles;
	Vector<ProximityToken *> _tokens;
	ProximityDB *_proximity_db;

protected:
	static void _bind_methods();

public:
	void setup(const Vector3 &p_center, const Vector3 &p_dimensions, const Vector3 &p_divisions);

	void add_vehicle(Ref<GdOpenSteerVehicle> p_vehicle);
	void remove_vehicle(Ref<GdOpenSteerVehicle> p_vehicle);
	int get_vehicle_count() const;
	Ref<GdOpenSteerVehicle> get_vehicle(int p_index) const;
	Array get_vehicles() const;

	void update_proximity();
	Array get_neighbors(const Ref<GdOpenSteerVehicle> &p_vehicle, float p_radius) const;

	// Internal: build AVGroup for flocking
	void get_neighbor_group(const GdOpenSteerVehicle *p_vehicle, float p_radius,
			OpenSteer::AVGroup &r_group) const;

	GdOpenSteerWorld();
	~GdOpenSteerWorld();
};

#endif // GD_OPENSTEER_H
