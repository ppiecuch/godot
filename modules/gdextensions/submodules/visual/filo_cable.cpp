/**************************************************************************/
/*  filo_cable.cpp                                                        */
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

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

#include "filo_cable.h"

#include "scene/3d/physics_body.h"
#include "servers/physics_server.h"

namespace filo {

// ════════════════════════════════════════════════════════════════════════
// utils
// ════════════════════════════════════════════════════════════════════════

bool utils::Catenary(const Vector3 &p1, const Vector3 &p2, real_t l, int samples, Vector<Vector2> &points) {
	ERR_FAIL_COND_V(samples < 2, false);

	points.resize(samples);

	const Vector3 vector = p2 - p1;
	const Vector3 dir = vector * Vector3(1, 0, 1);

	const Quat rot = LookRotation(dir);
	const Quat irot = rot.inverse();
	const Vector3 n = irot.xform(vector);

	real_t r = 0;
	real_t s = 0;
	real_t u = n.z;
	real_t v = n.y;

	if (r > u) {
		SWAP(r, u);
		SWAP(s, v);
	}

	real_t z = 0.005;
	real_t target = Math::sqrt(l * l - (v - s) * (v - s)) / (u - r);
	while (Math::sinh(z) / z < target) {
		z += 0.005;
	}

	if (z > 0.005) {
		const real_t a = (u - r) / 2 / z;
		const real_t p = (r + u - a * Math::log((l + v - s) / (l - v + s))) / 2;
		const real_t q = (v + s - l * (real_t)Math::cosh(z) / (real_t)Math::sinh(z)) / 2;

		const real_t inc = (u - r) * (1.0 / (samples - 1));

		for (int i = 0; i < samples; ++i) {
			const real_t x = r + inc * i;
			points.write[i] = Vector2(x, a * Math::cosh((x - p) / a) + q);
		}
		return true;
	} else {
		return false;
	}
}

bool utils::Sinusoid(const Vector3 &origin, const Vector3 &direction, real_t l, unsigned frequency, int samples, Vector<Vector3> &points) {
	ERR_FAIL_COND_V(samples < 2, false);

	static const Vector3 Forward = Vector3(0, 0, -1);
	points.resize(samples);
	Vector3 ndirection = direction;
	real_t magnitude = ndirection.length();
	if (magnitude > 1e-4) {
		ndirection /= magnitude;
		Vector3 ortho = ndirection.cross(Forward);

		const real_t inc = magnitude / (samples - 1);
		const real_t d = frequency * 4;
		const real_t d2 = d * d;

		const real_t amplitude = Math::sqrt(l * l / d2 - magnitude * magnitude / d2);

		if (Math::is_nan(amplitude)) {
			return false;
		}
		for (int i = 0; i < samples; ++i) {
			real_t pctg = i / real_t(samples - 1);
			points.write[i] = origin + ndirection * inc * i + ortho * Math::sin(pctg * Math_PI * 2 * frequency) * amplitude;
		}
		return true;
	} else {
		return false;
	}
}

real_t utils::Mod(real_t a, real_t b) {
	return a - b * Math::floor(a / b);
}

Vector3 utils::Rotate2D(const Vector3 &v, real_t angle) {
	return Vector3(
			v.x * Math::cos(angle) - v.y * Math::sin(angle),
			v.x * Math::sin(angle) + v.y * Math::cos(angle),
			v.z);
}

Quat utils::LookRotation(const Vector3 &forward, const Vector3 &upwards) {
	return Transform().looking_at(forward, upwards).basis.get_quat();
}

// ════════════════════════════════════════════════════════════════════════
// ConvexHull2D
// ════════════════════════════════════════════════════════════════════════

ConvexHull2D::ConvexHull2D() {
	perimeter = 0;
	bounds = Rect2();
}

int ConvexHull2D::Orientation(const Vector2 &p, const Vector2 &q, const Vector2 &r) {
	real_t val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
	if (val == 0)
		return 0;
	return (val > 0) ? 1 : -1;
}

int ConvexHull2D::left_most_input_point() const {
	int left_most = 0;
	for (int i = 0; i < input.size(); ++i) {
		if (input[i].x < input[left_most].x)
			left_most = i;
	}
	return left_most;
}

void ConvexHull2D::clean_input() {
	int i = 0;
	while (i < input.size()) {
		Vector2 value = input[i];
		int j = i + 1;
		while (j < input.size()) {
			if ((value - input[j]).length_squared() < 1e-4)
				input.remove(j);
			else
				++j;
		}
		++i;
	}
}

void ConvexHull2D::set_input(const Vector<Vector2> &p_input) {
	input = p_input;
}

Vector<Vector2> ConvexHull2D::get_input() const {
	return input;
}

void ConvexHull2D::compute(const Vector<Vector2> &p_input) {
	input = p_input;
	clean_input();
	compute_hull();
	update_perimeter_and_bounds();
}

void ConvexHull2D::compute_hull() {
	if (input.size() <= 3) {
		hull_points = input;
		return;
	}

	int l = left_most_input_point();
	hull_points.clear();

	int p = l, q;
	do {
		hull_points.push_back(input[p]);
		q = (p + 1) % input.size();

		for (int i = 0; i < input.size(); ++i) {
			if (Orientation(input[p], input[i], input[q]) < 0)
				q = i;
		}

		p = q;
	} while (p != l);
}

void ConvexHull2D::update_perimeter_and_bounds() {
	perimeter = 0;
	if (hull_points.empty())
		return;

	bounds = Rect2(hull_points[0], Size2());

	for (int i = 0; i < hull_points.size(); ++i) {
		bounds.expand_to(hull_points[i]);

		int next = (i + 1) % hull_points.size();
		perimeter += hull_points[i].distance_to(hull_points[next]);
	}
}

void ConvexHull2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("compute", "input"), &ConvexHull2D::compute);
	ClassDB::bind_method(D_METHOD("get_perimeter"), &ConvexHull2D::get_perimeter);

	ADD_PROPERTY(PropertyInfo(Variant::REAL, "perimeter"), "", "get_perimeter");
}

// ════════════════════════════════════════════════════════════════════════
// CurveFrame
// ════════════════════════════════════════════════════════════════════════

CurveFrame::CurveFrame() {
	reset();
}

void CurveFrame::reset() {
	position = Vector3();
	tangent = Vector3(0, 0, 1);
	normal = Vector3(0, 1, 0);
	binormal = Vector3(-1, 0, 0);
}

void CurveFrame::set_twist(real_t twist) {
	Quat twist_q(tangent, Math::deg2rad(twist));
	normal = twist_q.xform(normal);
	binormal = twist_q.xform(binormal);
}

void CurveFrame::set_twist_and_tangent(real_t twist, const Vector3 &p_tangent) {
	tangent = p_tangent;
	normal = Vector3(-p_tangent.y, p_tangent.x, 0).normalized();
	binormal = tangent.cross(normal);

	Quat twist_q(tangent, Math::deg2rad(twist));
	normal = twist_q.xform(normal);
	binormal = twist_q.xform(binormal);
}

void CurveFrame::transport(const Vector3 &new_position, const Vector3 &new_tangent, real_t twist) {
	if (new_tangent.length_squared() < 1e-8) {
		position = new_position;
		return;
	}
	Vector3 nt = new_tangent.normalized();

	Quat rot_q(tangent, nt);
	Quat twist_q(nt, Math::deg2rad(twist));
	Quat final_q = twist_q * rot_q;

	normal = final_q.xform(normal);
	binormal = final_q.xform(binormal);
	tangent = nt;
	position = new_position;
}

// ════════════════════════════════════════════════════════════════════════
// CableSection
// ════════════════════════════════════════════════════════════════════════

CableSection::CableSection() {
	circle_preset(8);
}

int CableSection::get_segments() const {
	return vertices.size() > 0 ? vertices.size() - 1 : 0;
}

void CableSection::circle_preset(int segments) {
	vertices.clear();
	for (int j = 0; j <= segments; ++j) {
		real_t angle = 2.0 * Math_PI / segments * j;
		vertices.push_back(Vector2(Math::cos(angle), Math::sin(angle)));
	}
}

int CableSection::snap_to(real_t val, int snap_interval, int threshold) {
	int int_val = (int)val;
	if (snap_interval <= 0)
		return int_val;
	int under = Math::floor(val / snap_interval) * snap_interval;
	int over = under + snap_interval;
	if (int_val - under < threshold)
		return under;
	if (over - int_val < threshold)
		return over;
	return int_val;
}

void CableSection::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_segments"), &CableSection::get_segments);
	ClassDB::bind_method(D_METHOD("circle_preset", "segments"), &CableSection::circle_preset);
}

// ════════════════════════════════════════════════════════════════════════
// SampledCable
// ════════════════════════════════════════════════════════════════════════

SampledCable::SampledCable() {
	segment_count = 0;
	total_length = 0;
}

void SampledCable::append_sample(const Vector3 &sample, bool accumulate_length) {
	if (accumulate_length && segment_count > 0 && segments[0].size() > 0) {
		total_length += sample.distance_to(last_sample);
	}

	if (segment_count == 0)
		segment_count = 1;

	if (segments.empty())
		segments.push_back(Vector<Vector3>());

	segments.write[segment_count - 1].push_back(sample);
	last_sample = sample;
}

void SampledCable::reverse_last_samples(int count) {
	if (segment_count > 0) {
		Vector<Vector3> &segment = segments.write[segment_count - 1];
		if (count <= segment.size()) {
			int start = segment.size() - count;
			// Reverse in place
			for (int i = 0; i < count / 2; ++i) {
				SWAP(segment.write[start + i], segment.write[start + count - 1 - i]);
			}

			for (int i = start; i < segment.size() - 1; ++i) {
				total_length += segment[i].distance_to(segment[i + 1]);
			}

			last_sample = segment[segment.size() - 1];
		}
	}
}

void SampledCable::new_segment() {
	segment_count++;
	if (segments.size() < segment_count)
		segments.push_back(Vector<Vector3>());
}

void SampledCable::clear() {
	for (int i = 0; i < segments.size(); ++i)
		segments.write[i].clear();
	segment_count = 0;
	total_length = 0;
}

void SampledCable::close() {
	if (segment_count > 0 && segments[0].size() > 0) {
		segments.write[segment_count - 1].push_back(segments[0][0]);
	}
}

// ════════════════════════════════════════════════════════════════════════
// CableBody
// ════════════════════════════════════════════════════════════════════════

CableBody::CableBody() {
	plane = PLANE_XY;
	freeze_rotation = true;
	rbody = nullptr;
}

void CableBody::find_rigidbody() {
	Node *p = get_parent();
	while (p) {
		rbody = Object::cast_to<RigidBody>(p);
		if (rbody)
			break;
		p = p->get_parent();
	}
}

void CableBody::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY) {
		find_rigidbody();
	}
}

void CableBody::set_plane(CablePlane p_plane) {
	plane = p_plane;
}

CableBody::CablePlane CableBody::get_plane() const {
	return plane;
}

void CableBody::set_freeze_rotation(bool p_freeze) {
	freeze_rotation = p_freeze;
}

bool CableBody::get_freeze_rotation() const {
	return freeze_rotation;
}

Vector3 CableBody::world_to_cable(const Vector3 &ws_point) const {
	Vector3 ls = get_global_transform().affine_inverse().xform(ws_point);
	switch (plane) {
		case PLANE_XY:
			return ls;
		case PLANE_XZ:
			return Vector3(ls.x, ls.z, ls.y);
		case PLANE_YZ:
			return Vector3(ls.y, ls.z, ls.x);
		default:
			return Vector3();
	}
}

Vector3 CableBody::cable_to_world(const Vector3 &cable_point) const {
	Vector3 ls;
	switch (plane) {
		case PLANE_XY:
			ls = cable_point;
			break;
		case PLANE_XZ:
			ls = Vector3(cable_point.x, cable_point.z, cable_point.y);
			break;
		case PLANE_YZ:
			ls = Vector3(cable_point.z, cable_point.x, cable_point.y);
			break;
		default:
			ls = Vector3();
			break;
	}
	return get_global_transform().xform(ls);
}

Vector2 CableBody::world_to_cable_plane(const Vector3 &ws_point) const {
	Vector3 ls = get_global_transform().affine_inverse().xform(ws_point);
	switch (plane) {
		case PLANE_XY:
			return Vector2(ls.x, ls.y);
		case PLANE_XZ:
			return Vector2(ls.x, ls.z);
		case PLANE_YZ:
			return Vector2(ls.y, ls.z);
		default:
			return Vector2();
	}
}

Vector3 CableBody::cable_plane_to_world(const Vector2 &cable_point) const {
	Vector3 ls;
	switch (plane) {
		case PLANE_XY:
			ls = Vector3(cable_point.x, cable_point.y, 0);
			break;
		case PLANE_XZ:
			ls = Vector3(cable_point.x, 0, cable_point.y);
			break;
		case PLANE_YZ:
			ls = Vector3(0, cable_point.x, cable_point.y);
			break;
		default:
			ls = Vector3();
			break;
	}
	return get_global_transform().xform(ls);
}

Vector3 CableBody::get_cable_plane_normal() const {
	switch (plane) {
		case PLANE_XY:
			return -get_global_transform().basis.get_axis(2); // forward
		case PLANE_XZ:
			return get_global_transform().basis.get_axis(1); // up
		case PLANE_YZ:
			return get_global_transform().basis.get_axis(0); // right
		default:
			return Vector3();
	}
}

Vector3 CableBody::get_world_space_tangent(const Vector3 &origin, bool orientation) {
	return cable_plane_to_world(get_tangent_from_origin(world_to_cable_plane(origin), orientation));
}

void CableBody::apply_freezing() {
	if (rbody != nullptr) {
		Vector3 plane_normal = get_cable_plane_normal();
		Vector3 av = rbody->get_angular_velocity();
		rbody->set_angular_velocity(plane_normal * av.dot(plane_normal));
	}
}

void CableBody::calculate_inertia_tensor() {
	// Base implementation — subclasses may override
}

Vector3 CableBody::random_hull_point() {
	return get_global_transform().origin;
}

Vector2 CableBody::get_tangent_from_origin(const Vector2 &origin, bool orientation) {
	return Vector2();
}

real_t CableBody::surface_distance(const Vector2 &p1, const Vector2 &p2, bool orientation, bool shortest) {
	return 0;
}

Vector3 CableBody::surface_point_at_distance(const Vector3 &origin, real_t distance, bool orientation, int &out_index) {
	out_index = 0;
	return get_global_transform().origin;
}

void CableBody::append_samples(SampledCable &samples, const Vector3 &origin, real_t distance, real_t spool_separation, bool reverse, bool orientation) {
}

void CableBody::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_plane", "plane"), &CableBody::set_plane);
	ClassDB::bind_method(D_METHOD("get_plane"), &CableBody::get_plane);
	ClassDB::bind_method(D_METHOD("set_freeze_rotation", "freeze"), &CableBody::set_freeze_rotation);
	ClassDB::bind_method(D_METHOD("get_freeze_rotation"), &CableBody::get_freeze_rotation);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "plane", PROPERTY_HINT_ENUM, "XY,XZ,YZ"), "set_plane", "get_plane");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "freeze_rotation"), "set_freeze_rotation", "get_freeze_rotation");

	BIND_ENUM_CONSTANT(PLANE_XY);
	BIND_ENUM_CONSTANT(PLANE_XZ);
	BIND_ENUM_CONSTANT(PLANE_YZ);
}

// ════════════════════════════════════════════════════════════════════════
// CablePoint
// ════════════════════════════════════════════════════════════════════════

void CablePoint::apply_freezing() {
}

void CablePoint::calculate_inertia_tensor() {
}

Vector3 CablePoint::random_hull_point() {
	return get_global_transform().origin;
}

Vector2 CablePoint::get_tangent_from_origin(const Vector2 &origin, bool orientation) {
	return Vector2();
}

real_t CablePoint::surface_distance(const Vector2 &p1, const Vector2 &p2, bool orientation, bool shortest) {
	return 0;
}

Vector3 CablePoint::surface_point_at_distance(const Vector3 &origin, real_t distance, bool orientation, int &out_index) {
	out_index = 0;
	return get_global_transform().origin;
}

void CablePoint::append_samples(SampledCable &samples, const Vector3 &origin, real_t distance, real_t spool_separation, bool reverse, bool orientation) {
}

void CablePoint::_bind_methods() {
}

// ════════════════════════════════════════════════════════════════════════
// CableDisc
// ════════════════════════════════════════════════════════════════════════

CableDisc::CableDisc() {
	radius = 1.0;
}

void CableDisc::set_radius(real_t p_radius) {
	radius = p_radius;
}

real_t CableDisc::get_radius() const {
	return radius;
}

real_t CableDisc::get_scaled_radius() const {
	Vector3 scale = get_global_transform().basis.get_scale();
	return radius * MAX(scale.x, MAX(scale.y, scale.z));
}

Vector2 CableDisc::tangent_point_circle(const Vector2 &from, const Vector2 &center, real_t r, bool orientation) {
	Vector2 d = center - from;
	real_t d_mag = d.length();

	if (d_mag > r) {
		real_t alpha;
		if (d.x >= 0) {
			alpha = Math::asin(d.y / d_mag);
		} else {
			alpha = Math_PI - Math::asin(d.y / d_mag);
		}

		real_t theta = Math::asin(r / d_mag);

		if (orientation)
			alpha = alpha - Math_PI * 0.5 - theta;
		else
			alpha = alpha + Math_PI * 0.5 + theta;

		return center + r * Vector2(Math::cos(alpha), Math::sin(alpha));
	}
	return from;
}

Vector3 CableDisc::random_hull_point() {
	return get_global_transform().origin + get_global_transform().basis.get_axis(0) * radius;
}

Vector2 CableDisc::get_tangent_from_origin(const Vector2 &origin, bool orientation) {
	return tangent_point_circle(origin, Vector2(), radius, orientation);
}

real_t CableDisc::surface_distance(const Vector2 &p1, const Vector2 &p2, bool orientation, bool shortest) {
	Vector2 a = p1;
	Vector2 b = p2;
	if (orientation) {
		SWAP(a, b);
	}

	real_t theta = Math::atan2(a.x * b.y - a.y * b.x, a.x * b.x + a.y * b.y);

	if (!shortest && theta < 0) {
		theta = theta + Math_PI * 2;
	}

	return get_scaled_radius() * theta;
}

Vector3 CableDisc::surface_point_at_distance(const Vector3 &origin, real_t distance, bool orientation, int &out_index) {
	out_index = 0;
	real_t angle = distance / radius * (orientation ? -1 : 1);
	return cable_to_world(utils::Rotate2D(origin, angle));
}

void CableDisc::append_samples(SampledCable &samples, const Vector3 &origin, real_t distance, real_t spool_separation, bool reverse, bool orientation) {
	if (get_scaled_radius() < 1e-4 || distance < 1e-4) {
		samples.append_sample(cable_to_world(origin));
		return;
	}

	real_t angle = distance / get_scaled_radius();
	int sample_count = Math::ceil(angle / (Math_PI * 0.05));

	Vector3 axis_offset = get_cable_plane_normal() * distance * spool_separation / sample_count;

	angle *= (orientation ? -1 : 1) * (reverse ? -1 : 1);
	real_t theta = -angle / sample_count;

	Vector3 result = origin;

	samples.append_sample(cable_to_world(result), !reverse);
	for (int i = 0; i < sample_count; ++i) {
		result = utils::Rotate2D(result, theta);
		samples.append_sample(cable_to_world(result) + axis_offset * (i + 1), !reverse);
	}

	if (reverse) {
		samples.reverse_last_samples(sample_count + 1);
	}
}

void CableDisc::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &CableDisc::set_radius);
	ClassDB::bind_method(D_METHOD("get_radius"), &CableDisc::get_radius);

	ADD_PROPERTY(PropertyInfo(Variant::REAL, "radius"), "set_radius", "get_radius");
}

// ════════════════════════════════════════════════════════════════════════
// CableShape
// ════════════════════════════════════════════════════════════════════════

void CableShape::set_hull(const Ref<ConvexHull2D> &p_hull) {
	hull = p_hull;
}

Ref<ConvexHull2D> CableShape::get_hull() const {
	return hull;
}

Vector3 CableShape::random_hull_point() {
	if (hull.is_null() || hull->hull_points.empty())
		return get_global_transform().origin;
	return cable_plane_to_world(hull->hull_points[0]);
}

Vector2 CableShape::get_tangent_from_origin(const Vector2 &origin, bool orientation) {
	if (hull.is_null() || hull->hull_points.empty())
		return Vector2();

	Vector2 axis = -origin.normalized();

	int result = 0;
	real_t best_proj = FLT_MAX;

	for (int i = 0; i < hull->hull_points.size(); ++i) {
		int side = ConvexHull2D::Orientation(origin, origin + axis, hull->hull_points[i]);

		if ((orientation && side >= 0) || (!orientation && side < 0)) {
			real_t proj = (hull->hull_points[i] - origin).normalized().dot(axis);
			if (proj < best_proj) {
				best_proj = proj;
				result = i;
			}
		}
	}

	return hull->hull_points[result];
}

Vector3 CableShape::project_to_surface(const Vector3 &p, int &out_vertex) const {
	out_vertex = 0;
	if (hull.is_null() || hull->hull_points.empty())
		return p;

	Vector2 best_projection = Vector2(p.x, p.y);
	real_t best_distance = FLT_MAX;

	for (int i = 0; i < hull->hull_points.size(); ++i) {
		int next = (i + 1) % hull->hull_points.size();

		Vector2 edge = hull->hull_points[next] - hull->hull_points[i];
		Vector2 v2p = Vector2(p.x, p.y) - hull->hull_points[i];

		real_t dot = v2p.dot(edge) / edge.length_squared();
		dot = CLAMP(dot, 0.0, 1.0 - 1e-2);

		Vector2 proj = hull->hull_points[i] + dot * edge;
		real_t pd = (Vector2(p.x, p.y) - proj).length_squared();

		if (pd < best_distance) {
			best_distance = pd;
			best_projection = proj;
			out_vertex = i;
		}
	}

	return Vector3(best_projection.x, best_projection.y, p.z);
}

real_t CableShape::surface_distance(const Vector2 &p1, const Vector2 &p2, bool orientation, bool shortest) {
	if (hull.is_null() || hull->hull_points.empty() || (p1 - p2).length_squared() < 1e-6)
		return 0;

	int a, b;
	Vector3 proj1 = project_to_surface(Vector3(p1.x, p1.y, 0), a);
	Vector3 proj2 = project_to_surface(Vector3(p2.x, p2.y, 0), b);
	Vector3 w1 = cable_plane_to_world(Vector2(proj1.x, proj1.y));
	Vector3 w2 = cable_plane_to_world(Vector2(proj2.x, proj2.y));

	Vector3 sample = cable_plane_to_world(hull->hull_points[a]);

	real_t distance1 = -w1.distance_to(sample) +
			w2.distance_to(cable_plane_to_world(hull->hull_points[b]));
	real_t distance2 = w1.distance_to(sample) -
			w2.distance_to(cable_plane_to_world(hull->hull_points[b]));

	int c1 = a;
	int c2 = a;

	while (c1 != b) {
		int next = c1 = (int)utils::Mod(c1 + 1, hull->hull_points.size());
		distance1 += sample.distance_to(cable_plane_to_world(hull->hull_points[next]));
		sample = cable_plane_to_world(hull->hull_points[c1]);
	}

	sample = cable_plane_to_world(hull->hull_points[a]);

	while (c2 != b) {
		int next = c2 = (int)utils::Mod(c2 - 1, hull->hull_points.size());
		distance2 += sample.distance_to(cable_plane_to_world(hull->hull_points[next]));
		sample = cable_plane_to_world(hull->hull_points[c2]);
	}

	if (!shortest) {
		return orientation ? distance2 : distance1;
	} else {
		if (distance1 < Math::abs(distance2)) {
			return distance1 * (orientation ? -1 : 1);
		} else {
			return -distance2 * (orientation ? -1 : 1);
		}
	}
}

Vector3 CableShape::surface_point_at_distance(const Vector3 &origin, real_t distance, bool orientation, int &out_index) {
	if (hull.is_null() || hull->hull_points.empty()) {
		out_index = 0;
		return get_global_transform().origin;
	}

	Vector3 proj = project_to_surface(origin, out_index);
	Vector3 next_sample = cable_to_world(proj);
	Vector3 current_sample = next_sample;
	int next_index = out_index;

	int direction = (orientation ? -1 : 1);
	real_t accum_distance = -MIN(distance, Vector2(proj.x, proj.y).distance_to(hull->hull_points[out_index]));
	real_t segment_distance = 0;

	do {
		current_sample = next_sample;
		out_index = next_index;

		next_index = (int)utils::Mod(out_index + direction, hull->hull_points.size());
		next_sample = cable_to_world(Vector3(hull->hull_points[next_index].x, hull->hull_points[next_index].y, 0));

		segment_distance = current_sample.distance_to(next_sample);
		accum_distance += segment_distance;
	} while (accum_distance + 1e-4 < distance);

	if (segment_distance > 1e-8) {
		return next_sample.linear_interpolate(current_sample, (accum_distance - distance) / segment_distance);
	}
	return current_sample;
}

void CableShape::append_samples(SampledCable &samples, const Vector3 &origin, real_t distance, real_t spool_separation, bool reverse, bool orientation) {
	if (hull.is_null() || hull->hull_points.empty())
		return;

	int current;
	Vector3 proj = project_to_surface(origin, current);
	Vector3 origin_sample = cable_to_world(proj);
	samples.append_sample(origin_sample);

	if (distance < 1e-4)
		return;

	int direction = (orientation ? -1 : 1) * (reverse ? -1 : 1);
	Vector3 axis_offset = get_cable_plane_normal() * spool_separation;

	int count = 1;
	real_t accum_distance = 0;
	while (accum_distance < Math::abs(distance)) {
		int next = current = (int)utils::Mod(current - direction, hull->hull_points.size());
		Vector3 sample = cable_to_world(Vector3(hull->hull_points[next].x, hull->hull_points[next].y, proj.z));

		real_t seg_dist = origin_sample.distance_to(sample);

		if (accum_distance + seg_dist <= distance) {
			samples.append_sample(sample + axis_offset * accum_distance, !reverse);
		} else {
			Vector3 interpolated = origin_sample.linear_interpolate(sample, (Math::abs(distance) - accum_distance) / seg_dist);
			samples.append_sample(interpolated + axis_offset * accum_distance, !reverse);
		}

		origin_sample = sample;
		accum_distance += seg_dist;
		count++;
	}

	if (reverse) {
		samples.reverse_last_samples(count);
	}
}

void CableShape::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_hull", "hull"), &CableShape::set_hull);
	ClassDB::bind_method(D_METHOD("get_hull"), &CableShape::get_hull);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "hull", PROPERTY_HINT_RESOURCE_TYPE, "ConvexHull2D"), "set_hull", "get_hull");
}

// ════════════════════════════════════════════════════════════════════════
// CableJoint
// ════════════════════════════════════════════════════════════════════════

CableJoint::CableJoint() {
	body1 = nullptr;
	body2 = nullptr;
	length = 0;
	rest_length = 1;
	rb1 = nullptr;
	rb2 = nullptr;
	total_lambda = 0;
	inv_mass1 = 0;
	inv_mass2 = 0;
	k = 0;
}

CableJoint::CableJoint(CableBody *p_body1, CableBody *p_body2, const Vector3 &p_offset1, const Vector3 &p_offset2, real_t p_rest_length) {
	body1 = p_body1;
	body2 = p_body2;
	rb1 = p_body1 ? p_body1->get_rigidbody() : nullptr;
	rb2 = p_body2 ? p_body2->get_rigidbody() : nullptr;
	offset1 = p_offset1;
	offset2 = p_offset2;
	rest_length = p_rest_length;
	length = 0;
	total_lambda = 0;
	inv_mass1 = 0;
	inv_mass2 = 0;
	k = 0;
}

Vector3 CableJoint::world_space_attachment1() const {
	return body1 != nullptr ? body1->get_global_transform().xform(offset1) : Vector3();
}

Vector3 CableJoint::world_space_attachment2() const {
	return body2 != nullptr ? body2->get_global_transform().xform(offset2) : Vector3();
}

void CableJoint::initialize() {
	total_lambda = 0;

	if (body1 == nullptr || body2 == nullptr)
		return;

	world_offset1 = body1->get_global_transform().xform(offset1);
	world_offset2 = body2->get_global_transform().xform(offset2);

	Vector3 vector = world_offset2 - world_offset1;
	length = vector.length();
	jacobian = vector / (length + 0.00001);

	inv_inertia_tensor1 = Basis();
	inv_inertia_tensor2 = Basis();

	if (rb1 != nullptr) {
		PhysicsDirectBodyState *state = PhysicsServer::get_singleton()->body_get_direct_state(rb1->get_rid());
		if (state) {
			inv_inertia_tensor1 = state->get_inverse_inertia_tensor();
		}
	}

	if (rb2 != nullptr) {
		PhysicsDirectBodyState *state = PhysicsServer::get_singleton()->body_get_direct_state(rb2->get_rid());
		if (state) {
			inv_inertia_tensor2 = state->get_inverse_inertia_tensor();
		}
	}

	inv_mass1 = 0;
	inv_mass2 = 0;
	real_t w1 = 0, w2 = 0;

	if (rb1 != nullptr && rb1->get_mode() != RigidBody::MODE_KINEMATIC) {
		inv_mass1 = 1.0 / rb1->get_mass();
		r1 = world_offset1 - rb1->get_global_transform().origin;
		w1 = inv_inertia_tensor1.xform(r1.cross(jacobian)).cross(r1).dot(jacobian);
	}

	if (rb2 != nullptr && rb2->get_mode() != RigidBody::MODE_KINEMATIC) {
		inv_mass2 = 1.0 / rb2->get_mass();
		r2 = world_offset2 - rb2->get_global_transform().origin;
		w2 = inv_inertia_tensor2.xform(r2.cross(jacobian)).cross(r2).dot(jacobian);
	}

	k = inv_mass1 + inv_mass2 + w1 + w2;
}

void CableJoint::solve(real_t dt, real_t bias) {
	real_t c = length - rest_length;

	if (body1 != nullptr && body2 != nullptr && c > 0 && k > 0) {
		Vector3 vel2 = rb2 != nullptr ? (rb2->get_linear_velocity() + rb2->get_angular_velocity().cross(r2)) : Vector3();
		Vector3 vel1 = rb1 != nullptr ? (rb1->get_linear_velocity() + rb1->get_angular_velocity().cross(r1)) : Vector3();
		Vector3 rel_vel = vel2 - vel1;

		real_t c_dot = rel_vel.dot(jacobian);

		real_t lambda = (-c_dot - c * bias / dt) / k;

		real_t temp_lambda = total_lambda;
		total_lambda = MIN(0.0, total_lambda + lambda);
		lambda = total_lambda - temp_lambda;

		Vector3 impulse = jacobian * lambda;

		if (rb1 != nullptr && rb1->get_mode() != RigidBody::MODE_KINEMATIC) {
			rb1->set_linear_velocity(rb1->get_linear_velocity() - impulse * inv_mass1);
			rb1->set_angular_velocity(rb1->get_angular_velocity() - inv_inertia_tensor1.xform(r1.cross(impulse)));
		}

		if (rb2 != nullptr && rb2->get_mode() != RigidBody::MODE_KINEMATIC) {
			rb2->set_linear_velocity(rb2->get_linear_velocity() + impulse * inv_mass2);
			rb2->set_angular_velocity(rb2->get_angular_velocity() + inv_inertia_tensor2.xform(r2.cross(impulse)));
		}
	}
}

// ════════════════════════════════════════════════════════════════════════
// Cable
// ════════════════════════════════════════════════════════════════════════

Cable::Link::Link() {
	body = nullptr;
	type = ATTACHMENT;
	orientation = false;
	hybrid_rolling = false;
	stored_cable = 0;
	spool_separation = 0;
}

Cable::Cable() {
	looseness_scale = 1.0;
	max_loose_cable = 0.25;
	vertical_threshold = 0.25;
	vertical_curlyness = 1;
	rest_length = 0;

	catenary_buffer.resize(16);
	sinusoid_buffer.resize(24);
}

void Cable::set_looseness_scale(real_t p_scale) {
	looseness_scale = CLAMP(p_scale, 0.0, 1.0);
}

real_t Cable::get_looseness_scale() const {
	return looseness_scale;
}

void Cable::set_max_loose_cable(real_t p_max) {
	max_loose_cable = p_max;
}

real_t Cable::get_max_loose_cable() const {
	return max_loose_cable;
}

void Cable::set_vertical_threshold(real_t p_threshold) {
	vertical_threshold = MAX(1e-4, p_threshold);
}

real_t Cable::get_vertical_threshold() const {
	return vertical_threshold;
}

void Cable::set_vertical_curlyness(int p_curlyness) {
	vertical_curlyness = MAX(1, p_curlyness);
}

int Cable::get_vertical_curlyness() const {
	return vertical_curlyness;
}

void Cable::add_link(CableBody *body, Link::Type type, bool orientation) {
	Link link;
	link.body = body;
	link.type = type;
	link.orientation = orientation;
	links.push_back(link);
}

void Cable::clear_links() {
	links.clear();
	joints.clear();
}

int Cable::get_link_count() const {
	return links.size();
}

void Cable::find_common_tangents(const Link &link1, const Link &link2, Vector3 &out_t1, Vector3 &out_t2) {
	out_t1 = link1.body->random_hull_point();
	out_t2 = link2.body->random_hull_point();

	Vector3 prev_t1, prev_t2;

	do {
		prev_t1 = out_t1;
		prev_t2 = out_t2;

		if (link2.type == Link::ATTACHMENT || link2.type == Link::PINHOLE || (link2.type == Link::HYBRID && !link2.hybrid_rolling))
			out_t2 = link2.body->get_global_transform().xform(link2.in_anchor);
		else
			out_t2 = link2.body->get_world_space_tangent(out_t1, link2.orientation);

		if (link1.type == Link::ATTACHMENT || link1.type == Link::PINHOLE || (link1.type == Link::HYBRID && !link1.hybrid_rolling))
			out_t1 = link1.body->get_global_transform().xform(link1.out_anchor);
		else
			out_t1 = link1.body->get_world_space_tangent(out_t2, !link1.orientation);
	} while ((prev_t1 - out_t1).length_squared() > 1e-6 ||
			(prev_t2 - out_t2).length_squared() > 1e-6);
}

void Cable::update_pinhole(CableJoint &joint1, CableJoint &joint2) {
	real_t rest1 = joint1.rest_length;
	real_t rest2 = joint2.rest_length;

	if (joint1.length > rest1) {
		real_t delta = joint1.length - rest1;
		joint1.rest_length += delta;
		joint2.rest_length -= delta;
	}
	if (joint2.length > rest2) {
		real_t delta = joint2.length - rest2;
		joint1.rest_length -= delta;
		joint2.rest_length += delta;
	}
}

void Cable::update_pinholes() {
	for (int i = 1; i < links.size() - 1; ++i) {
		if (links[i].body != nullptr && i - 1 < joints.size() && i < joints.size()) {
			if (links[i].type == Link::PINHOLE)
				update_pinhole(joints.write[i - 1], joints.write[i]);
		}
	}
}

void Cable::update_hybrid_link(Link &link, bool cable_goes_in, const Vector3 &attachment) {
	if (link.stored_cable < 0 && link.hybrid_rolling) {
		link.hybrid_rolling = false;
		update_joints();
	} else if (!link.hybrid_rolling) {
		Vector3 tplus = link.body->get_world_space_tangent(attachment, false);
		Vector3 tminus = link.body->get_world_space_tangent(attachment, true);

		Vector2 t = link.body->world_to_cable_plane(
				link.body->get_global_transform().xform(cable_goes_in ? link.in_anchor : link.out_anchor));

		real_t d1 = link.body->surface_distance(link.body->world_to_cable_plane(tplus), t, false);
		real_t d2 = link.body->surface_distance(link.body->world_to_cable_plane(tminus), t, false);

		if (d1 < 0 || d2 > 0) {
			if (Math::abs(d1) < Math::abs(d2)) {
				link.hybrid_rolling = true;
				link.orientation = !cable_goes_in;
			} else {
				link.hybrid_rolling = true;
				link.orientation = cable_goes_in;
			}
		}
	}
}

void Cable::update_hybrid_links() {
	if (links.empty())
		return;

	if (links[0].body != nullptr && links[0].type == Link::HYBRID && !joints.empty())
		update_hybrid_link(links.write[0], false, joints[0].world_space_attachment2());

	if (links.size() > 1) {
		int last = links.size() - 1;
		if (links[last].body != nullptr && links[last].type == Link::HYBRID && !joints.empty())
			update_hybrid_link(links.write[last], true, joints[joints.size() - 1].world_space_attachment1());
	}
}

void Cable::update_joints() {
	for (int i = 0; i < joints.size(); ++i) {
		if (links[i].body != nullptr && links[i + 1].body != nullptr) {
			CableJoint &joint = joints.write[i];

			Vector3 t1, t2;
			find_common_tangents(links[i], links[i + 1], t1, t2);

			Vector2 current_t1 = joint.body1->world_to_cable_plane(joint.world_space_attachment1());
			Vector2 current_t2 = joint.body2->world_to_cable_plane(joint.world_space_attachment2());

			real_t d1 = joint.body1->surface_distance(current_t1, joint.body1->world_to_cable_plane(t1), links[i].orientation);
			real_t d2 = joint.body2->surface_distance(current_t2, joint.body2->world_to_cable_plane(t2), links[i + 1].orientation);

			links.write[i].stored_cable -= d1;
			links.write[i + 1].stored_cable += d2;

			joint.rest_length += d1;
			joint.rest_length -= d2;

			joint.offset1 = joint.body1->get_global_transform().affine_inverse().xform(
					t1 - joint.body1->get_cable_plane_normal() * links[i].stored_cable * links[i].spool_separation);
			joint.offset2 = joint.body2->get_global_transform().affine_inverse().xform(
					t2 - joint.body2->get_cable_plane_normal() * links[i + 1].stored_cable * links[i + 1].spool_separation);
		}
	}
}

void Cable::initialize_joints() {
	for (int i = 0; i < joints.size(); ++i) {
		joints.write[i].initialize();
	}
}

CableJoint *Cable::get_previous_joint(int link_index, bool closed) {
	if (link_index > 0) {
		return &joints.write[link_index - 1];
	} else if (closed && !joints.empty()) {
		return &joints.write[joints.size() - 1];
	}
	return nullptr;
}

CableJoint *Cable::get_next_joint(int link_index, bool closed) {
	if (link_index < joints.size()) {
		return &joints.write[link_index];
	} else if (closed && !joints.empty()) {
		return &joints.write[0];
	}
	return nullptr;
}

void Cable::calculate_rest_length() {
	rest_length = 0;
	if (joints.empty())
		return;

	bool closed = (links.size() > 1 && links[0].body == links[links.size() - 1].body);

	for (int i = 0; i < links.size(); ++i) {
		if (links[i].body != nullptr) {
			CableJoint *prev_joint = get_previous_joint(i, closed);
			CableJoint *next_joint = get_next_joint(i, closed);

			if (next_joint != nullptr && prev_joint != nullptr && !(i == 0 && closed)) {
				links.write[i].stored_cable = Math::abs(
						links[i].body->surface_distance(
								links[i].body->world_to_cable_plane(prev_joint->world_space_attachment2()),
								links[i].body->world_to_cable_plane(next_joint->world_space_attachment1()),
								links[i].orientation));
				rest_length += links[i].stored_cable;
			} else if (links[i].type == Link::HYBRID) {
				rest_length += links[i].stored_cable;

				if (next_joint != nullptr) {
					Vector2 tangent = links[i].body->world_to_cable_plane(next_joint->world_space_attachment1());
					int j = 0;
					links.write[i].out_anchor = links[i].body->get_global_transform().affine_inverse().xform(
							links[i].body->surface_point_at_distance(Vector3(tangent.x, tangent.y, 0), links[i].stored_cable, links[i].orientation, j));
				} else if (prev_joint != nullptr) {
					Vector2 tangent = links[i].body->world_to_cable_plane(prev_joint->world_space_attachment2());
					int j = 0;
					links.write[i].in_anchor = links[i].body->get_global_transform().affine_inverse().xform(
							links[i].body->surface_point_at_distance(Vector3(tangent.x, tangent.y, 0), links[i].stored_cable, !links[i].orientation, j));
				}
			}

			if (i < joints.size()) {
				rest_length += joints[i].rest_length;
			}
		}
	}
}

void Cable::setup() {
	joints.clear();
	if (links.size() > 0) {
		joints.resize(links.size() - 1);

		for (int i = 0; i < links.size() - 1; ++i) {
			if (links[i].body != nullptr && links[i + 1].body != nullptr) {
				Vector3 t1, t2;
				find_common_tangents(links[i], links[i + 1], t1, t2);

				t1 -= links[i].body->get_cable_plane_normal() * links[i].stored_cable * links[i].spool_separation;
				t2 -= links[i + 1].body->get_cable_plane_normal() * links[i + 1].stored_cable * links[i + 1].spool_separation;

				joints.write[i] = CableJoint(links[i].body, links[i + 1].body,
						links[i].body->get_global_transform().affine_inverse().xform(t1),
						links[i + 1].body->get_global_transform().affine_inverse().xform(t2),
						(t2 - t1).length());
			}
		}
	}

	calculate_rest_length();
}

void Cable::update_cable() {
	if (joints.empty())
		return;

	update_joints();
	update_hybrid_links();
	initialize_joints();
	update_pinholes();
}

void Cable::solve(real_t delta_time, real_t bias) {
	if (joints.empty())
		return;

	for (int i = 0; i < joints.size(); ++i) {
		joints.write[i].solve(delta_time, bias);
	}
	for (int i = 0; i < links.size(); ++i) {
		if (links[i].body != nullptr)
			links[i].body->apply_freezing();
	}
}

void Cable::sample_link(CableJoint *prev_joint, Link &link, CableJoint *next_joint) {
	Vector3 t1, t2;
	bool has_t1 = false, has_t2 = false;

	if (prev_joint != nullptr) {
		t1 = prev_joint->body2->world_to_cable(prev_joint->world_space_attachment2());
		has_t1 = true;
	}
	if (next_joint != nullptr) {
		t2 = next_joint->body1->world_to_cable(next_joint->world_space_attachment1());
		has_t2 = true;
	}

	if (link.type == Link::ROLLING || link.type == Link::HYBRID) {
		if (has_t1 && has_t2) {
			real_t distance = link.body->surface_distance(Vector2(t1.x, t1.y), Vector2(t2.x, t2.y), !link.orientation, false);
			link.body->append_samples(sampled_cable, t1, distance, link.spool_separation, false, link.orientation);
		} else if (has_t1) {
			link.body->append_samples(sampled_cable, t1, link.stored_cable, link.spool_separation, false, link.orientation);
		} else if (has_t2) {
			link.body->append_samples(sampled_cable, t2, link.stored_cable, link.spool_separation, true, link.orientation);
		}
	} else {
		if (has_t1)
			sampled_cable.append_sample(prev_joint->body2->get_global_transform().xform(link.in_anchor));

		if (has_t1 && has_t2 && t1 != t2)
			sampled_cable.new_segment();

		if (has_t2)
			sampled_cable.append_sample(next_joint->body1->get_global_transform().xform(link.out_anchor));
	}
}

void Cable::sample_joint(CableJoint &joint) {
	Vector3 p1 = joint.world_space_attachment1();
	Vector3 p2 = joint.world_space_attachment2();

	if (joint.length < joint.rest_length) {
		Vector3 point = p2 - p1;
		Vector3 dir = point * Vector3(1, 0, 1);

		if (looseness_scale > 0) {
			real_t sampled_length = Math::lerp(joint.length, MIN(joint.rest_length, joint.length + max_loose_cable), looseness_scale);

			if (dir.length_squared() > vertical_threshold) {
				Quat rot = utils::LookRotation(dir);
				Quat irot = rot.inverse();
				Vector3 n = irot.xform(point);

				if (utils::Catenary(Vector3(0, 0, 0), Vector3(0, n.y, n.z), sampled_length, catenary_buffer.size(), catenary_buffer)) {
					for (int j = 1; j < catenary_buffer.size() - 1; ++j) {
						sampled_cable.append_sample(p1 + rot.xform(Vector3(0, catenary_buffer[j].y, catenary_buffer[j].x)));
					}
				}
			} else {
				if (utils::Sinusoid(p1, point, sampled_length, vertical_curlyness, sinusoid_buffer.size(), sinusoid_buffer)) {
					for (int j = 1; j < sinusoid_buffer.size() - 1; ++j) {
						sampled_cable.append_sample(sinusoid_buffer[j]);
					}
				}
			}
		}
	}
}

void Cable::sample_cable() {
	sampled_cable.clear();

	if (joints.empty() || links.empty())
		return;

	bool closed = (links.size() > 1 && links[0].body == links[links.size() - 1].body);

	for (int i = 0; i < links.size(); ++i) {
		if (links[i].body != nullptr) {
			CableJoint *prev_joint = get_previous_joint(i, closed);
			CableJoint *next_joint = get_next_joint(i, closed);

			if (!(i == 0 && closed) || links[i].type == Link::ATTACHMENT || links[i].type == Link::PINHOLE)
				sample_link(prev_joint, links.write[i], next_joint);

			if (i < joints.size())
				sample_joint(joints.write[i]);
		}
	}

	if (closed) {
		sampled_cable.close();
	}
}

void Cable::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_looseness_scale", "scale"), &Cable::set_looseness_scale);
	ClassDB::bind_method(D_METHOD("get_looseness_scale"), &Cable::get_looseness_scale);
	ClassDB::bind_method(D_METHOD("set_max_loose_cable", "max"), &Cable::set_max_loose_cable);
	ClassDB::bind_method(D_METHOD("get_max_loose_cable"), &Cable::get_max_loose_cable);
	ClassDB::bind_method(D_METHOD("set_vertical_threshold", "threshold"), &Cable::set_vertical_threshold);
	ClassDB::bind_method(D_METHOD("get_vertical_threshold"), &Cable::get_vertical_threshold);
	ClassDB::bind_method(D_METHOD("set_vertical_curlyness", "curlyness"), &Cable::set_vertical_curlyness);
	ClassDB::bind_method(D_METHOD("get_vertical_curlyness"), &Cable::get_vertical_curlyness);

	ClassDB::bind_method(D_METHOD("setup"), &Cable::setup);
	ClassDB::bind_method(D_METHOD("update_cable"), &Cable::update_cable);
	ClassDB::bind_method(D_METHOD("sample_cable"), &Cable::sample_cable);

	ADD_PROPERTY(PropertyInfo(Variant::REAL, "looseness_scale", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_looseness_scale", "get_looseness_scale");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "max_loose_cable"), "set_max_loose_cable", "get_max_loose_cable");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "vertical_threshold"), "set_vertical_threshold", "get_vertical_threshold");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "vertical_curlyness"), "set_vertical_curlyness", "get_vertical_curlyness");
}

// ════════════════════════════════════════════════════════════════════════
// CableSolver
// ════════════════════════════════════════════════════════════════════════

CableSolver::CableSolver() {
	iterations = 4;
	bias = 0.2;
}

void CableSolver::set_iterations(int p_iterations) {
	iterations = p_iterations;
}

int CableSolver::get_iterations() const {
	return iterations;
}

void CableSolver::set_bias(real_t p_bias) {
	bias = p_bias;
}

real_t CableSolver::get_bias() const {
	return bias;
}

void CableSolver::_notification(int p_what) {
	if (p_what == NOTIFICATION_PHYSICS_PROCESS) {
		for (int i = 0; i < get_child_count(); ++i) {
			Cable *cable = Object::cast_to<Cable>(get_child(i));
			if (cable && cable->is_visible_in_tree())
				cable->update_cable();
		}

		for (int j = 0; j < iterations; ++j) {
			for (int i = 0; i < get_child_count(); ++i) {
				Cable *cable = Object::cast_to<Cable>(get_child(i));
				if (cable && cable->is_visible_in_tree())
					cable->solve(get_physics_process_delta_time(), bias);
			}
		}
	}

	if (p_what == NOTIFICATION_READY) {
		set_physics_process(true);
	}
}

void CableSolver::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_iterations", "iterations"), &CableSolver::set_iterations);
	ClassDB::bind_method(D_METHOD("get_iterations"), &CableSolver::get_iterations);
	ClassDB::bind_method(D_METHOD("set_bias", "bias"), &CableSolver::set_bias);
	ClassDB::bind_method(D_METHOD("get_bias"), &CableSolver::get_bias);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "iterations"), "set_iterations", "get_iterations");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "bias"), "set_bias", "get_bias");
}

} // namespace filo

// ════════════════════════════════════════════════════════════════════════
// Doctests
// ════════════════════════════════════════════════════════════════════════

#ifdef DOCTEST

TEST_CASE("[FiloCable] utils::Mod positive") {
	CHECK(filo::utils::Mod(5, 3) == doctest::Approx(2));
	CHECK(filo::utils::Mod(6, 3) == doctest::Approx(0));
	CHECK(filo::utils::Mod(1.5, 1.0) == doctest::Approx(0.5));
}

TEST_CASE("[FiloCable] utils::Mod negative") {
	CHECK(filo::utils::Mod(-1, 3) == doctest::Approx(2));
	CHECK(filo::utils::Mod(-4, 3) == doctest::Approx(2));
	CHECK(filo::utils::Mod(-0.5, 1.0) == doctest::Approx(0.5));
}

TEST_CASE("[FiloCable] utils::Rotate2D identity") {
	Vector3 v(1, 0, 5);
	Vector3 r = filo::utils::Rotate2D(v, 0);
	CHECK(r.x == doctest::Approx(1));
	CHECK(r.y == doctest::Approx(0));
	CHECK(r.z == doctest::Approx(5));
}

TEST_CASE("[FiloCable] utils::Rotate2D 90 degrees") {
	Vector3 v(1, 0, 7);
	Vector3 r = filo::utils::Rotate2D(v, Math_PI * 0.5);
	CHECK(r.x == doctest::Approx(0).epsilon(1e-5));
	CHECK(r.y == doctest::Approx(1).epsilon(1e-5));
	CHECK(r.z == doctest::Approx(7));
}

TEST_CASE("[FiloCable] ConvexHull2D::Orientation CW/CCW/collinear") {
	CHECK(filo::ConvexHull2D::Orientation(Vector2(0, 0), Vector2(1, 0), Vector2(1, 1)) == -1); // CCW
	CHECK(filo::ConvexHull2D::Orientation(Vector2(0, 0), Vector2(1, 0), Vector2(1, -1)) == 1); // CW
	CHECK(filo::ConvexHull2D::Orientation(Vector2(0, 0), Vector2(1, 0), Vector2(2, 0)) == 0); // collinear
}

TEST_CASE("[FiloCable] ConvexHull2D triangle") {
	Ref<filo::ConvexHull2D> ch;
	ch.instance();
	Vector<Vector2> pts;
	pts.push_back(Vector2(0, 0));
	pts.push_back(Vector2(1, 0));
	pts.push_back(Vector2(0, 1));
	ch->compute(pts);
	CHECK(ch->hull_points.size() == 3);
}

TEST_CASE("[FiloCable] ConvexHull2D square with interior") {
	Ref<filo::ConvexHull2D> ch;
	ch.instance();
	Vector<Vector2> pts;
	pts.push_back(Vector2(0, 0));
	pts.push_back(Vector2(1, 0));
	pts.push_back(Vector2(1, 1));
	pts.push_back(Vector2(0, 1));
	pts.push_back(Vector2(0.5, 0.5)); // interior point
	ch->compute(pts);
	CHECK(ch->hull_points.size() == 4);
}

TEST_CASE("[FiloCable] ConvexHull2D perimeter") {
	Ref<filo::ConvexHull2D> ch;
	ch.instance();
	Vector<Vector2> pts;
	pts.push_back(Vector2(0, 0));
	pts.push_back(Vector2(1, 0));
	pts.push_back(Vector2(1, 1));
	pts.push_back(Vector2(0, 1));
	ch->compute(pts);
	CHECK(ch->perimeter == doctest::Approx(4.0));
}

TEST_CASE("[FiloCable] CableDisc::tangent_point_circle left and right") {
	Vector2 from(3, 0);
	Vector2 center(0, 0);
	real_t r = 1;

	Vector2 left = filo::CableDisc::tangent_point_circle(from, center, r, true);
	Vector2 right = filo::CableDisc::tangent_point_circle(from, center, r, false);

	// Tangent points should be on the circle
	CHECK(left.distance_to(center) == doctest::Approx(r).epsilon(1e-4));
	CHECK(right.distance_to(center) == doctest::Approx(r).epsilon(1e-4));

	// They should be on opposite sides of the x-axis
	CHECK(left.y * right.y < 0);
}

TEST_CASE("[FiloCable] SampledCable append and length") {
	filo::SampledCable sc;
	sc.append_sample(Vector3(0, 0, 0));
	sc.append_sample(Vector3(1, 0, 0));
	sc.append_sample(Vector3(2, 0, 0));

	CHECK(sc.get_length() == doctest::Approx(2.0));
	CHECK(sc.get_segments().size() == 1);
	CHECK(sc.get_segments()[0].size() == 3);
}

TEST_CASE("[FiloCable] SampledCable reverse_last_samples") {
	filo::SampledCable sc;
	sc.append_sample(Vector3(0, 0, 0), false);
	sc.append_sample(Vector3(1, 0, 0), false);
	sc.append_sample(Vector3(2, 0, 0), false);

	sc.reverse_last_samples(3);

	CHECK(sc.get_segments()[0][0].x == doctest::Approx(2));
	CHECK(sc.get_segments()[0][1].x == doctest::Approx(1));
	CHECK(sc.get_segments()[0][2].x == doctest::Approx(0));
}

TEST_CASE("[FiloCable] SampledCable new_segment + clear") {
	filo::SampledCable sc;
	sc.append_sample(Vector3(0, 0, 0));
	sc.new_segment();
	sc.append_sample(Vector3(1, 0, 0));

	CHECK(sc.get_segments().size() >= 2);

	sc.clear();
	CHECK(sc.get_length() == doctest::Approx(0));
}

TEST_CASE("[FiloCable] CurveFrame transport") {
	filo::CurveFrame frame;
	frame.transport(Vector3(1, 0, 0), Vector3(0, 0, 1), 0);

	CHECK(frame.position.x == doctest::Approx(1));
	CHECK(frame.tangent.z == doctest::Approx(1).epsilon(1e-4));
}

TEST_CASE("[FiloCable] CurveFrame set_twist") {
	filo::CurveFrame frame;
	frame.set_twist(90); // 90 degrees

	// After 90-degree twist around Z-forward tangent, normal and binormal should rotate
	CHECK(frame.normal.length() == doctest::Approx(1).epsilon(1e-4));
	CHECK(frame.binormal.length() == doctest::Approx(1).epsilon(1e-4));
}

TEST_CASE("[FiloCable] CableSection circle_preset") {
	filo::CableSection cs;
	cs.circle_preset(4);

	CHECK(cs.vertices.size() == 5); // 4 segments + 1 duplicate end
	CHECK(cs.get_segments() == 4);

	// First vertex should be on the unit circle (1, 0)
	CHECK(cs.vertices[0].x == doctest::Approx(1));
	CHECK(cs.vertices[0].y == doctest::Approx(0));
}

TEST_CASE("[FiloCable] CableSection snap_to") {
	CHECK(filo::CableSection::snap_to(11, 10, 3) == 10); // 11 is within threshold of 10
	CHECK(filo::CableSection::snap_to(15, 10, 3) == 15); // 15 is not near either snap
	CHECK(filo::CableSection::snap_to(18, 10, 3) == 20); // 18 is within threshold of 20
	CHECK(filo::CableSection::snap_to(7, 0, 3) == 7); // snap_interval 0 returns int
}

#endif // DOCTEST
