
/*
 *  parallel_transport_frames.cpp
 *
 *  Copyright (c) 2012, Neil Mendoza, http://www.neilmendoza.com
 *  All rights reserved. 
 *
 * https://forum.openframeworks.cc/t/how-to-specify-the-starting-orientation-with-ofxptf/28299
 * https://github.com/neilmendoza/ofxPtf/blob/master/src/ParallelTransportFrames.cpp
 *
 */

#include "parallel_transport_frames.h"
#include <limits>

ParallelTransportFrames::ParallelTransportFrames() : max_points(4), max_frames(std::numeric_limits<unsigned>::max()) { }

bool ParallelTransportFrames::add_point(real_t x, real_t y, real_t z) {
	return add_point(Vector3(x, y, z));
}

bool ParallelTransportFrames::add_point(const Vector3& point) {
	points.push_back(point);
	while (points.size() > max_points) {
		points.pop_front();
	}
	if (points.size() == 3) {
		first_frame();
	} else if (points.size() > 3) {
		next_frame();
		return true;
	}
	return false;
}

void ParallelTransportFrames::first_frame() {
	Vector3 t = points[1] - points[0];
	t.normalize();
	Vector3 n = t.cross(points[2] - points[0]);
	n.normalize();

	if(n.length_squared() == 0) {
		int i = Math::abs(t[0]) < Math::abs(t[1]) ? 0 : 1;
		if (Math::abs(t[2]) < Math::abs(t[i])) i = 2;

		Vector3 v(0);
		v[i] = 1;
		n = t.cross(v);
		n.normalize();
	}

	Vector3 b = t.cross(n);

	Transform m(Basis(t, b, n), points[0]);
	frames.push_back(m);

	prev_tangent = t;
	start_normal = n;
}

void ParallelTransportFrames::next_frame() {
	cur_tangent = points.back() - points[points.size() - 2];
	Vector3 axis;
	real_t angle = 0;

	if(prev_tangent.length_squared() != 0 && cur_tangent.length_squared() != 0) {

		cur_tangent.normalize();

		real_t dot = prev_tangent.dot(cur_tangent);

		if( dot > 1 ) dot = 1;
		else if( dot < -1.0 ) dot = -1;

		angle = Math::acos( dot );
		axis = prev_tangent.cross(cur_tangent);
	}

	if (axis.length_squared() != 0 && angle != 0) {

		Transform r(Basis(axis, angle), Vector3());
		Transform tj(Basis(), points.back());
		Transform ti(Basis(),-points[points.size() - 2]);

		frames.push_back(tj * r * ti * frames.back());
	} else {
		Transform tr(Basis(), points.back() - points[points.size() - 2]);

		frames.push_back(tr * frames.back());
	}
	prev_tangent = cur_tangent;
	while (frames.size() > max_frames) {
		frames.pop_front();
	}
}

// https://community.khronos.org/t/glm-to-create-gl-normalmatrix/61174
Basis ParallelTransportFrames::normal_matrix() const {
	Basis transform3 = frames.back().get_basis();
	return transform3.inverse().transposed();
}

Vector3 ParallelTransportFrames::calc_current_normal() const {
	return normal_matrix().xform(get_start_normal());
}

void ParallelTransportFrames::debug_draw(CanvasItem &canvas, real_t axis_size) {
	for (int i = 0; i < frames.size(); ++i) {
		Transform tr = frames[i].rotated(Vector3(0, 1, 0, 90); // ??
		canvas.draw_circle(0, 0, axis_size * 2);
		canvas.draw_axis(axis_size);
	}
}

void ParallelTransportFrames::clear() {
	points.clear();
	frames.clear();
}
