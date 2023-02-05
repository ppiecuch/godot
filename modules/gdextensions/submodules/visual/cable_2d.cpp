/*************************************************************************/
/*  cable_2d.cpp                                                         */
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

#include "cable_2d.h"

void Cable2D::set_active(bool status) {
	if (_active != status) {
		_active = status;
		update();
		set_process_internal(status);
	}
}

bool Cable2D::is_active() const {
	return _active;
}

void Cable2D::set_points(const PoolVector<Vector2> &p_points) {
	const int old_size = _points.size();
	_points = p_points;
	if (_points.size() != old_size) {
		rebuild_points();
	}
	update_rest_length();
	update();
}

PoolVector<Vector2> Cable2D::get_points() const {
	return _points;
}

void Cable2D::set_points_forces(const PoolVector<Vector2> &p_forces) {
	_point_forces = p_forces;
	if (_point_forces.size() != _points.size()) {
		_point_forces.resize(_points.size());
	}
	update();
}

PoolVector<Vector2> Cable2D::get_points_forces() const {
	return _point_forces;
}

void Cable2D::set_point_force(int index, Vector2 force) {
	if (index > -1 && index < _point_forces.size()) {
		_point_forces.set(index, force);
		update();
	}
}

Vector2 Cable2D::get_point_force(int index) const {
	if (index > -1 && index < _point_forces.size()) {
		return _point_forces[index];
	}
	return Vector2();
}

void Cable2D::set_color(Color color) {
	_color = color;
	update();
}

Color Cable2D::get_color() const {
	return _color;
}

void Cable2D::set_width(float width) {
	if (width < 0) {
		width = 0;
	}
	_width = width;
	update();
}

float Cable2D::get_width() const {
	return _width;
}

void Cable2D::set_segments(int segments) {
	if (segments < 1) {
		segments = 1;
	}
	int old_segments = _segments;
	_segments = segments;
	if (_segments != old_segments) {
		rebuild_points();
	}
	update();
}

int Cable2D::get_segments() const {
	return _segments;
}

void Cable2D::set_restlength_scale(float scale) {
	if (scale < 0.1) {
		scale = 0.1;
	}
	_restlength_scale = scale;
	update();
}

float Cable2D::get_restlength_scale() const {
	return _restlength_scale;
}

void Cable2D::set_iterations(int iterations) {
	if (iterations < 1) {
		iterations = 1;
	}
	_iterations = iterations;
}

int Cable2D::get_iterations() const {
	return _iterations;
}

void Cable2D::rebuild_points() {
	if (_points.size() < 2) {
		_rendered_points = _points;
		return;
	}

	int pointcount = _segments * (_points.size() - 1);

	_rendered_points.resize(pointcount + 1);
	_point_forces.resize(_points.size());

	if (pointcount == 0) {
		return;
	}
	Vector2 interp_pt;
	for (int i = 0; i < _points.size() - 1; ++i) {
		for (int j = 0; j < _segments; ++j) {
			interp_pt = _points[i] + (_points[i + 1] - _points[i]) * (float(j) / _segments);
			_rendered_points.set(i * _segments + j, interp_pt);
		}
	}
	_rendered_points.set(pointcount, _points[_points.size() - 1]);
	_old_points = _rendered_points;

	update_rest_length();
}

void Cable2D::update_rest_length() {
	if (_points.size() < 2) {
		return;
	}
	_rest_lengths.resize(_points.size() - 1);
	for (int i = 0; i < _points.size() - 1; ++i) {
		float rest_length = _points[i].distance_to(_points[i + 1]) / float(_segments);
		_rest_lengths.set(i, rest_length);
	}
}

void Cable2D::update_cable(float delta) {
	if (_rendered_points.size() == 0) {
		return;
	}
	for (int i = 0; i < _rendered_points.size(); ++i) {
		const int segment = i / _segments;
		float force_blend = float(i % _segments) / float(_segments);
		if (i % _segments == 0) {
			// pinned point
			_rendered_points.set(i, _points[i / _segments]);
		} else {
			Vector2 point_force = (_point_forces[segment] * (1.0 - force_blend)) + (_point_forces[segment + 1] * force_blend);
			Vector2 pt = _rendered_points[i];
			Vector2 new_pt = pt;
			Vector2 vel = (pt - _old_points[i]) * delta;
			new_pt += vel;
			new_pt.y += 50 * delta;
			new_pt += point_force;
			_rendered_points.set(i, new_pt);
		}
	}
	_old_points = _rendered_points;

	// Damp the forces per point
	for (int i = 0; i < _point_forces.size(); i++) {
		_point_forces.set(i, _point_forces[i] * _force_damping);
	}
}

void Cable2D::update_constraints() {
	if (_rendered_points.size() == 0)
		return;

	for (int i = 0; i < _rendered_points.size() - 1; ++i) {
		Vector2 pt_a = _rendered_points[i];
		Vector2 pt_b = _rendered_points[i + 1];

		const Vector2 delta = pt_b - pt_a;
		const float diff = _rest_lengths[floor(i / _segments)] * _restlength_scale - delta.length();
		const float amount = diff / delta.length() / 2;
		const Vector2 offset = delta * amount;

		if (i % _segments > 0) {
			_rendered_points.set(i, pt_a - offset);
		}
		if ((i + 1) % _segments > 0) {
			_rendered_points.set(i + 1, pt_b + offset);
		}
	}
	_old_points = _rendered_points;
}

void Cable2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			_draw();
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
			const float time = get_process_delta_time();
			update_cable(time);
			for (int i = 0; i < _iterations; i++) {
				update_constraints();
			}
			update();
		} break;
	}
}

void Cable2D::_draw() {
	if (_points.size() <= 1 || _width == 0) {
		return;
	}
	PoolVector<Vector2>::Read points_read = _rendered_points.read();
	for (int i = 0; i < _rendered_points.size(); ++i) {
		if (i < _rendered_points.size() - 1) {
			draw_line(_rendered_points[i], _rendered_points[i + 1], _color, _width);
		}
		// draw_circle(_rendered_points[i], _width / 2.0, Color(1.0, 0.0, 0.0, 1.0));
	}
}

void Cable2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_active", "status"), &Cable2D::set_active);
	ClassDB::bind_method(D_METHOD("is_active"), &Cable2D::is_active);

	ClassDB::bind_method(D_METHOD("set_points", "points"), &Cable2D::set_points);
	ClassDB::bind_method(D_METHOD("get_points"), &Cable2D::get_points);

	ClassDB::bind_method(D_METHOD("set_points_forces", "forces"), &Cable2D::set_points_forces);
	ClassDB::bind_method(D_METHOD("get_points_forces"), &Cable2D::get_points_forces);

	ClassDB::bind_method(D_METHOD("set_point_force", "index", "force"), &Cable2D::set_point_force);
	ClassDB::bind_method(D_METHOD("get_point_force", "index"), &Cable2D::get_point_force);

	ClassDB::bind_method(D_METHOD("set_color", "color"), &Cable2D::set_color);
	ClassDB::bind_method(D_METHOD("get_color"), &Cable2D::get_color);

	ClassDB::bind_method(D_METHOD("set_width", "width"), &Cable2D::set_width);
	ClassDB::bind_method(D_METHOD("get_width"), &Cable2D::get_width);

	ClassDB::bind_method(D_METHOD("set_segments", "segments"), &Cable2D::set_segments);
	ClassDB::bind_method(D_METHOD("get_segments"), &Cable2D::get_segments);

	ClassDB::bind_method(D_METHOD("set_restlength_scale", "scale"), &Cable2D::set_restlength_scale);
	ClassDB::bind_method(D_METHOD("get_restlength_scale"), &Cable2D::get_restlength_scale);

	ClassDB::bind_method(D_METHOD("set_iterations", "iterations"), &Cable2D::set_iterations);
	ClassDB::bind_method(D_METHOD("get_iterations"), &Cable2D::get_iterations);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "active"), "set_active", "is_active");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR2_ARRAY, "points"), "set_points", "get_points");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR2_ARRAY, "forces"), "set_points_forces", "get_points_forces");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "width"), "set_width", "get_width");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "segments"), "set_segments", "get_segments");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "restlength_scale"), "set_restlength_scale", "get_restlength_scale");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "iterations"), "set_iterations", "get_iterations");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_color", "get_color");
}

Cable2D::Cable2D() {
	_active = true;
	_width = 2;
	_segments = 5;
	_restlength_scale = 1;
	_force_damping = 0.98;
	_iterations = 10;
	_color = Color(0.0, 0.0, 0.0, 1.0);
	set_process_internal(_active);
}
