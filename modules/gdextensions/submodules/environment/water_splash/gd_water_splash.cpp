/*************************************************************************/
/*  gd_water_splash.cpp                                                  */
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

#include "gd_water_splash.h"

#include "core/math/math_funcs.h"
#include "scene/2d/physics_body_2d.h"

#include <iostream>
#include <vector>

void GdWaterSplashColumn::update(const real_t &p_tension, const real_t &p_damping) {
	const real_t dh = _target_height - _height;
	_speed += p_tension * dh - p_damping * _speed;
	_height += _speed;
}

void GdWaterSplashColumn::body_enter_shape(int p_body_id, Object *p_body, int p_body_shape, int p_area_shape) {
	RigidBody2D *obj = cast_to<RigidBody2D>(p_body);
	if (obj) {
		Vector2 v = obj->get_linear_velocity();
		_speed = _drag.x * Math::abs(v.x) + _drag.y * Math::abs(v.y);
	}
}

void GdWaterSplashColumn::_bind_methods() {
	ClassDB::bind_method(D_METHOD("body_enter_shape", "bodyshape"), &GdWaterSplashColumn::body_enter_shape);
}

GdWaterSplashColumn::GdWaterSplashColumn(const Vector2 &p_pos, const Vector2 &p_delta, const Vector2 &p_drag) :
		_target_height(p_delta.y), _height(p_delta.y), _speed(0), _drag(p_drag) {
	Ref<RectangleShape2D> shape = memnew(RectangleShape2D);
	shape->set_extents(p_delta);

	Transform2D m;
	m[2] = p_pos; // set matrix origin

	int owner_id = create_shape_owner(this);
	shape_owner_add_shape(owner_id, shape);
	shape_owner_set_transform(owner_id, m);
	connect("body_shape_entered", this, "body_enter_shape");
}

GdWaterSplashColumn::~GdWaterSplashColumn() {
	disconnect("body_shape_entered", this, "body_enter_shape");
}

void GdWaterSplash::_update() {
	// Is size changed,
	if (_size_changed) {
		const real_t dx = _rect.size.x;
		const real_t dy = _rect.size.y;
		_size_changed = false;
		_ncols = uint32_t(dx / _resolution);
		for (int i = 0; i < _columns.size(); ++i) {
			memdelete(_columns[i]);
		}
		_columns.clear();
		real_t x = _rect.position.x;
		real_t y = _rect.position.y;
		for (uint32_t i = 0; i < _ncols; ++i) {
			GdWaterSplashColumn *col = memnew(GdWaterSplashColumn(Vector2(x, y), Vector2(_resolution, dy), _drag));
			_columns.push_back(col);
			add_child(col);
			x += _resolution;
		}
	}
	update();
}

void GdWaterSplash::_notification(int p_what) {
	static std::vector<real_t> l, r;
	switch (p_what) {
		case NOTIFICATION_READY: {
			l.resize(128);
			r.resize(128);
		} break;

		case NOTIFICATION_ENTER_TREE: {
			set_physics_process(true);
			update();
		} break;

		case NOTIFICATION_EXIT_TREE: {
			set_physics_process(false);
			update();
		} break;

		case NOTIFICATION_LOCAL_TRANSFORM_CHANGED: {
			if (!is_inside_tree()) {
				break;
			}
		} break;

		case NOTIFICATION_PHYSICS_PROCESS: {
			const int n = _columns.size();
			l.resize(n), r.resize(n);
			for (int i = 0; i < n; ++i) {
				_columns[i]->update(_tension, _damping);
			}
			for (int iter = 0; iter < 8; ++iter) {
				for (int i = 0; i < n; ++i) {
					if (i > 0) {
						l[i] = _spread * (_columns[i]->_height - _columns[i - 1]->_height);
						_columns[i - 1]->_speed += l[i];
					}
					if (i < n - 1) {
						r[i] = _spread * (_columns[i]->_height - _columns[i + 1]->_height);
						_columns[i + 1]->_speed += r[i];
					}
				}
				for (int i = 0; i < n; ++i) {
					if (i > 0) {
						_columns[i - 1]->_height += l[i];
					}
					if (i < n - 1) {
						_columns[i + 1]->_height += r[i];
					}
				}
			}
			update();
		} break;

		case NOTIFICATION_DRAW: {
			if (texture.is_null()) {
				return;
			}

			Vector2 pos = _rect.position;
			Vector<Color> colors;
			colors.push_back(_color);
			colors.push_back(_color);
			colors.push_back(_color);
			colors.push_back(_color);

			for (uint32_t i = 0; i < _ncols - 1; ++i) {
				Vector<Vector2> pts;
				pts.push_back(pos);
				pts.push_back(Vector2(pos.x + _resolution, pos.y));
				pts.push_back(Vector2(pos.x + _resolution, pos.y - _columns[i + 1]->_height));
				pts.push_back(Vector2(pos.x, pos.y - _columns[i]->_height));

				Vector<Vector2> uvs;
				uvs.push_back(Vector2(1, 1));
				uvs.push_back(Vector2(0, 1));
				uvs.push_back(Vector2(0, 0));
				uvs.push_back(Vector2(1, 0));

				draw_polygon(pts, colors, uvs, texture);
				pos.x += _resolution;
			}

		} break;
	}
}

void GdWaterSplash::set_size(const Rect2 &p_rect) {
	if (_rect != p_rect) {
		_rect = p_rect;
		_size_changed = true;
		_update();
	}
}

Rect2 GdWaterSplash::get_size() const {
	return _rect;
}

void GdWaterSplash::set_resolution(const uint32_t &p_resolution) {
	ERR_FAIL_COND(p_resolution <= 0);
	if (_resolution != p_resolution) {
		_resolution = p_resolution;
		_size_changed = true;
		_update();
	}
}

uint32_t GdWaterSplash::get_resolution() const {
	return _resolution;
}

void GdWaterSplash::set_color(const Color &p_color) {
	if (_color != p_color) {
		_color = p_color;
		_size_changed = false;
		_update();
	}
}

Color GdWaterSplash::get_color() const {
	return _color;
}

void GdWaterSplash::set_damping(const real_t &p_value) {
	_damping = p_value;
}

real_t GdWaterSplash::get_damping() const {
	return _damping;
}

void GdWaterSplash::set_tension(const real_t &p_value) {
	_tension = p_value;
}

real_t GdWaterSplash::get_tension() const {
	return _tension;
}

void GdWaterSplash::set_spread(const real_t &p_value) {
	_spread = p_value;
}

real_t GdWaterSplash::get_spread() const {
	return _spread;
}

Vector2 GdWaterSplash::get_drag() const {
	return _drag;
}

void GdWaterSplash::set_drag(const Vector2 &p_value) {
	if (_drag != p_value) {
		_drag = p_value;
		for (int i = 0; i < _columns.size(); ++i) {
			_columns[i]->_drag = _drag;
		}
	}
}

void GdWaterSplash::set_texture(const Ref<Texture> &p_texture) {
	texture = p_texture;
}

Ref<Texture> GdWaterSplash::get_texture() const {
	return texture;
}

void GdWaterSplash::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_size", "size"), &GdWaterSplash::set_size);
	ClassDB::bind_method(D_METHOD("get_size"), &GdWaterSplash::get_size);
	ADD_PROPERTY(PropertyInfo(Variant::RECT2, "size"), "set_size", "get_size");

	ClassDB::bind_method(D_METHOD("set_resolution", "resolution"), &GdWaterSplash::set_resolution);
	ClassDB::bind_method(D_METHOD("get_resolution"), &GdWaterSplash::get_resolution);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "resolution"), "set_resolution", "get_resolution");

	ClassDB::bind_method(D_METHOD("set_color", "color"), &GdWaterSplash::set_color);
	ClassDB::bind_method(D_METHOD("get_color"), &GdWaterSplash::get_color);
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_color", "get_color");

	ClassDB::bind_method(D_METHOD("set_damping", "damping"), &GdWaterSplash::set_damping);
	ClassDB::bind_method(D_METHOD("get_damping"), &GdWaterSplash::get_damping);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "damping"), "set_damping", "get_damping");

	ClassDB::bind_method(D_METHOD("set_tension", "tension"), &GdWaterSplash::set_tension);
	ClassDB::bind_method(D_METHOD("get_tension"), &GdWaterSplash::get_tension);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "tension"), "set_tension", "get_tension");

	ClassDB::bind_method(D_METHOD("set_spread", "spread"), &GdWaterSplash::set_spread);
	ClassDB::bind_method(D_METHOD("get_spread"), &GdWaterSplash::get_spread);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "spread"), "set_spread", "get_spread");

	ClassDB::bind_method(D_METHOD("set_drag", "drag"), &GdWaterSplash::set_drag);
	ClassDB::bind_method(D_METHOD("get_drag"), &GdWaterSplash::get_drag);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "drag"), "set_drag", "get_drag");

	ClassDB::bind_method(D_METHOD("set_texture", "texture"), &GdWaterSplash::set_texture);
	ClassDB::bind_method(D_METHOD("get_texture"), &GdWaterSplash::get_texture);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "config/texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_texture", "get_texture");
}

GdWaterSplash::GdWaterSplash() {
	_resolution = 5;
	_color = Color(1, 1, 1, 1);
	_damping = 0.025;
	_tension = 0.025;
	_spread = 0.25;
	_drag = Vector2(0.01, 0.03);
	_size_changed = true;
	set_size(Rect2(0, 0, 100, 20));
}
