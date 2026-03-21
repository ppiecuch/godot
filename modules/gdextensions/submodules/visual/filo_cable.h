/**************************************************************************/
/*  filo_cable.h                                                          */
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

#ifndef FILO_CABLE_H
#define FILO_CABLE_H

#include "core/math/math_funcs.h"
#include "core/math/quat.h"
#include "core/math/transform.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"
#include "core/reference.h"
#include "core/vector.h"
#include "scene/3d/spatial.h"

class RigidBody;

namespace filo {

// ── utils ──

struct utils {
	static bool Catenary(const Vector3 &p1, const Vector3 &p2, real_t l, int samples, Vector<Vector2> &points);
	static bool Sinusoid(const Vector3 &origin, const Vector3 &direction, real_t l, unsigned frequency, int samples, Vector<Vector3> &points);
	static real_t Mod(real_t a, real_t b);
	static Vector3 Rotate2D(const Vector3 &v, real_t angle);
	static Quat LookRotation(const Vector3 &forward, const Vector3 &upwards = Vector3::UP);
};

// ── ConvexHull2D ──

class ConvexHull2D : public Reference {
	GDCLASS(ConvexHull2D, Reference);

	Vector<Vector2> input;

	int left_most_input_point() const;
	void clean_input();

protected:
	static void _bind_methods();

public:
	Vector<Vector2> hull_points;
	real_t perimeter;
	Rect2 bounds;

	static int Orientation(const Vector2 &p, const Vector2 &q, const Vector2 &r);

	void set_input(const Vector<Vector2> &p_input);
	Vector<Vector2> get_input() const;
	void compute(const Vector<Vector2> &p_input);
	void compute_hull();
	void update_perimeter_and_bounds();
	real_t get_perimeter() const { return perimeter; }

	ConvexHull2D();
};

// ── CurveFrame ──

struct CurveFrame {
	Vector3 position;
	Vector3 tangent;
	Vector3 normal;
	Vector3 binormal;

	CurveFrame();
	void reset();
	void set_twist(real_t twist);
	void set_twist_and_tangent(real_t twist, const Vector3 &p_tangent);
	void transport(const Vector3 &new_position, const Vector3 &new_tangent, real_t twist);
};

// ── CableSection ──

class CableSection : public Reference {
	GDCLASS(CableSection, Reference);

protected:
	static void _bind_methods();

public:
	Vector<Vector2> vertices;

	int get_segments() const;
	void circle_preset(int segments);
	static int snap_to(real_t val, int snap_interval, int threshold);

	CableSection();
};

// ── SampledCable ──

class SampledCable {
	Vector<Vector<Vector3>> segments;
	int segment_count;
	real_t total_length;
	Vector3 last_sample;

public:
	const Vector<Vector<Vector3>> &get_segments() const { return segments; }
	real_t get_length() const { return total_length; }

	void append_sample(const Vector3 &sample, bool accumulate_length = true);
	void reverse_last_samples(int count);
	void new_segment();
	void clear();
	void close();

	SampledCable();
};

// ── Forward declarations ──

class Cable;

// ── CableBody ──

class CableBody : public Spatial {
	GDCLASS(CableBody, Spatial);

public:
	enum CablePlane {
		PLANE_XY,
		PLANE_XZ,
		PLANE_YZ,
	};

private:
	CablePlane plane;
	bool freeze_rotation;
	RigidBody *rbody;

	void find_rigidbody();

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	RigidBody *get_rigidbody() const { return rbody; }

	void set_plane(CablePlane p_plane);
	CablePlane get_plane() const;
	void set_freeze_rotation(bool p_freeze);
	bool get_freeze_rotation() const;

	Vector3 world_to_cable(const Vector3 &ws_point) const;
	Vector3 cable_to_world(const Vector3 &cable_point) const;
	Vector2 world_to_cable_plane(const Vector3 &ws_point) const;
	Vector3 cable_plane_to_world(const Vector2 &cable_point) const;
	Vector3 get_cable_plane_normal() const;
	Vector3 get_world_space_tangent(const Vector3 &origin, bool orientation);

	virtual void apply_freezing();
	virtual void calculate_inertia_tensor();

	virtual Vector3 random_hull_point();
	virtual Vector2 get_tangent_from_origin(const Vector2 &origin, bool orientation);
	virtual real_t surface_distance(const Vector2 &p1, const Vector2 &p2, bool orientation, bool shortest = true);
	virtual Vector3 surface_point_at_distance(const Vector3 &origin, real_t distance, bool orientation, int &out_index);
	virtual void append_samples(SampledCable &samples, const Vector3 &origin, real_t distance, real_t spool_separation, bool reverse, bool orientation);

	CableBody();
};

// ── CablePoint ──

class CablePoint : public CableBody {
	GDCLASS(CablePoint, CableBody);

protected:
	static void _bind_methods();

public:
	void apply_freezing() override;
	void calculate_inertia_tensor() override;
	Vector3 random_hull_point() override;
	Vector2 get_tangent_from_origin(const Vector2 &origin, bool orientation) override;
	real_t surface_distance(const Vector2 &p1, const Vector2 &p2, bool orientation, bool shortest = true) override;
	Vector3 surface_point_at_distance(const Vector3 &origin, real_t distance, bool orientation, int &out_index) override;
	void append_samples(SampledCable &samples, const Vector3 &origin, real_t distance, real_t spool_separation, bool reverse, bool orientation) override;
};

// ── CableDisc ──

class CableDisc : public CableBody {
	GDCLASS(CableDisc, CableBody);

	real_t radius;

protected:
	static void _bind_methods();

public:
	void set_radius(real_t p_radius);
	real_t get_radius() const;
	real_t get_scaled_radius() const;

	static Vector2 tangent_point_circle(const Vector2 &from, const Vector2 &center, real_t r, bool orientation);

	Vector3 random_hull_point() override;
	Vector2 get_tangent_from_origin(const Vector2 &origin, bool orientation) override;
	real_t surface_distance(const Vector2 &p1, const Vector2 &p2, bool orientation, bool shortest = true) override;
	Vector3 surface_point_at_distance(const Vector3 &origin, real_t distance, bool orientation, int &out_index) override;
	void append_samples(SampledCable &samples, const Vector3 &origin, real_t distance, real_t spool_separation, bool reverse, bool orientation) override;

	CableDisc();
};

// ── CableShape ──

class CableShape : public CableBody {
	GDCLASS(CableShape, CableBody);

	Ref<ConvexHull2D> hull;

protected:
	static void _bind_methods();

public:
	void set_hull(const Ref<ConvexHull2D> &p_hull);
	Ref<ConvexHull2D> get_hull() const;

	Vector3 project_to_surface(const Vector3 &p, int &out_vertex) const;

	Vector3 random_hull_point() override;
	Vector2 get_tangent_from_origin(const Vector2 &origin, bool orientation) override;
	real_t surface_distance(const Vector2 &p1, const Vector2 &p2, bool orientation, bool shortest = true) override;
	Vector3 surface_point_at_distance(const Vector3 &origin, real_t distance, bool orientation, int &out_index) override;
	void append_samples(SampledCable &samples, const Vector3 &origin, real_t distance, real_t spool_separation, bool reverse, bool orientation) override;
};

// ── CableJoint ──

struct CableJoint {
	CableBody *body1;
	Vector3 offset1;

	CableBody *body2;
	Vector3 offset2;

	real_t length;
	real_t rest_length;

	CableJoint();
	CableJoint(CableBody *p_body1, CableBody *p_body2, const Vector3 &p_offset1, const Vector3 &p_offset2, real_t p_rest_length);

	Vector3 world_space_attachment1() const;
	Vector3 world_space_attachment2() const;

	void initialize();
	void solve(real_t dt, real_t bias);

private:
	RigidBody *rb1;
	RigidBody *rb2;

	real_t total_lambda;
	real_t inv_mass1;
	real_t inv_mass2;
	Basis inv_inertia_tensor1;
	Basis inv_inertia_tensor2;
	Vector3 world_offset1;
	Vector3 world_offset2;
	Vector3 r1;
	Vector3 r2;
	Vector3 jacobian;
	real_t k;
};

// ── Cable ──

class Cable : public Spatial {
	GDCLASS(Cable, Spatial);

public:
	struct Link {
		enum Type {
			ATTACHMENT,
			ROLLING,
			PINHOLE,
			HYBRID,
		};

		CableBody *body;
		Type type;
		bool orientation;

		bool hybrid_rolling;
		real_t stored_cable;
		real_t spool_separation;

		Vector3 in_anchor;
		Vector3 out_anchor;

		Link();
	};

private:
	real_t looseness_scale;
	real_t max_loose_cable;
	real_t vertical_threshold;
	unsigned int vertical_curlyness;

	Vector<Link> links;
	Vector<CableJoint> joints;
	real_t rest_length;

	Vector<Vector2> catenary_buffer;
	Vector<Vector3> sinusoid_buffer;

	void find_common_tangents(const Link &link1, const Link &link2, Vector3 &out_t1, Vector3 &out_t2);
	void update_pinhole(CableJoint &joint1, CableJoint &joint2);
	void update_pinholes();
	void update_hybrid_link(Link &link, bool cable_goes_in, const Vector3 &attachment);
	void update_hybrid_links();
	void update_joints();
	void initialize_joints();
	void calculate_rest_length();

	CableJoint *get_previous_joint(int link_index, bool closed);
	CableJoint *get_next_joint(int link_index, bool closed);

	void sample_link(CableJoint *prev_joint, Link &link, CableJoint *next_joint);
	void sample_joint(CableJoint &joint);

protected:
	static void _bind_methods();

public:
	SampledCable sampled_cable;

	void set_looseness_scale(real_t p_scale);
	real_t get_looseness_scale() const;
	void set_max_loose_cable(real_t p_max);
	real_t get_max_loose_cable() const;
	void set_vertical_threshold(real_t p_threshold);
	real_t get_vertical_threshold() const;
	void set_vertical_curlyness(int p_curlyness);
	int get_vertical_curlyness() const;

	real_t get_rest_length() const { return rest_length; }
	int get_joint_count() const { return joints.size(); }

	void add_link(CableBody *body, Link::Type type, bool orientation);
	void clear_links();
	int get_link_count() const;

	void setup();
	void update_cable();
	void solve(real_t delta_time, real_t bias);
	void sample_cable();

	Cable();
};

// ── CableSolver ──

class CableSolver : public Node {
	GDCLASS(CableSolver, Node);

	int iterations;
	real_t bias;

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_iterations(int p_iterations);
	int get_iterations() const;
	void set_bias(real_t p_bias);
	real_t get_bias() const;

	CableSolver();
};

} // namespace filo

VARIANT_ENUM_CAST(filo::CableBody::CablePlane);

#endif // FILO_CABLE_H
