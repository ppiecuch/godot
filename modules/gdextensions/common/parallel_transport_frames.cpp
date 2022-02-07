/*************************************************************************/
/*  parallel_transport_frames.cpp                                        */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

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

#include "scene/3d/multimesh_instance.h"

#include <limits>

ParallelTransportFrames::ParallelTransportFrames() :
		max_points(4), max_frames(std::numeric_limits<unsigned>::max()) {}

bool ParallelTransportFrames::add_point(real_t x, real_t y, real_t z) {
	return add_point(Vector3(x, y, z));
}

bool ParallelTransportFrames::add_point(const Vector3 &point) {
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

	if (n.length_squared() == 0) {
		int i = Math::abs(t[0]) < Math::abs(t[1]) ? 0 : 1;
		if (Math::abs(t[2]) < Math::abs(t[i]))
			i = 2;

		Vector3 v;
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

	if (prev_tangent.length_squared() != 0 && cur_tangent.length_squared() != 0) {
		cur_tangent.normalize();

		real_t dot = prev_tangent.dot(cur_tangent);

		if (dot > 1)
			dot = 1;
		else if (dot < -1.0)
			dot = -1;

		angle = Math::acos(dot);
		axis = prev_tangent.cross(cur_tangent);
	}

	if (axis.length_squared() != 0 && angle != 0) {
		Transform r(Basis(axis, angle), Vector3());
		Transform tj(Basis(), points.back());
		Transform ti(Basis(), -points[points.size() - 2]);

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

// https://godotengine.org/qa/43701/how-do-draw-lines-like-the-ones-that-appear-the-axis-the-editor
MultiMeshInstance *ParallelTransportFrames::debug_draw_node(MultiMeshInstance *&node, real_t axis_size) {
	static Ref<ArrayMesh> _axis;

	if (node == 0) {
		node = memnew(MultiMeshInstance);
	}

	if (!_axis) {
		_axis = Ref<ArrayMesh>(memnew(ArrayMesh));
		Array mesh_array;
		mesh_array.resize(VS::ARRAY_MAX);
		mesh_array[VS::ARRAY_VERTEX] = parray(
				Vector3(0, 0, 0),
				Vector3(1, 0, 0),
				Vector3(0, 0, 0),
				Vector3(0, 1, 0),
				Vector3(0, 0, 0),
				Vector3(0, 0, 1));
		mesh_array[VS::ARRAY_COLOR] = parray(
				Color::named("red"),
				Color::named("red"),
				Color::named("green"),
				Color::named("green"),
				Color::named("blue"),
				Color::named("blue"));
		_axis->add_surface_from_arrays(Mesh::PRIMITIVE_LINES, mesh_array, Array());

		Ref<MultiMesh> multi_mesh = memnew(MultiMesh);
		multi_mesh->set_mesh(_axis);
		node->set_multimesh(multi_mesh);
	}

	Ref<MultiMesh> multi_mesh = node->get_multimesh();
	const int fsize = frames.size();
	if (multi_mesh->get_instance_count() < fsize) {
		multi_mesh->set_instance_count(fsize);
	}
	multi_mesh->set_visible_instance_count(fsize);
	for (int i = 0; i < fsize; ++i) {
		Transform tr = frames[i].scaled(Vector3::fill(axis_size)).rotated(Vector3(0, 1, 0), 90); // ??
		multi_mesh->set_instance_transform(i, tr);
	}

	return node;
}

void ParallelTransportFrames::clear() {
	points.clear();
	frames.clear();
}
