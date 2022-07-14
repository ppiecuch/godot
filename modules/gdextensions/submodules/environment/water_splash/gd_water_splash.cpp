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

void GdWaterSplashColumn::update(real_t &tension, real_t &damping) {
	const real_t dh = target_height_ - height_;
	speed_ += tension * dh - damping * speed_;
	height_ += speed_;
}

void GdWaterSplashColumn::body_enter_shape(int body_id, Object *body, int body_shape, int area_shape) {
	RigidBody2D *obj = cast_to<RigidBody2D>(body);
	if (obj) {
		Vector2 v = obj->get_linear_velocity();
		speed_ = drag_.x * Math::abs(v.x) + drag_.y * Math::abs(v.y);
	}
}

void GdWaterSplashColumn::_bind_methods() {
	ClassDB::bind_method(D_METHOD("body_enter_shape", "bodyshape"), &GdWaterSplashColumn::body_enter_shape);
}

GdWaterSplashColumn::GdWaterSplashColumn(const Vector2 &pos, const Vector2 &delta, const Vector2 &drag) :
		target_height_(delta.y), height_(delta.y), speed_(0), drag_(drag) {
	Ref<RectangleShape2D> shape = memnew(RectangleShape2D);
	shape->set_extents(delta);

	Transform2D m;
	m[2] = pos; // set matrix origin

	int owner_id = create_shape_owner(this);
	shape_owner_add_shape(owner_id, shape);
	shape_owner_set_transform(owner_id, m);
	connect("body_shape_entered", this, "body_enter_shape");
}

void GdWaterSplash::_update() {
	// Is size changed,
	if (size_changed_) {
		real_t dx = rect_.size.x;
		real_t dy = rect_.size.y;
		size_changed_ = false;
		ncols_ = uint32_t(dx / resolution_);
		for (int i = 0; i < columns_.size(); ++i) {
			memdelete(columns_[i]);
		}
		columns_.clear();
		real_t x = rect_.position.x;
		real_t y = rect_.position.y;
		for (uint32_t i = 0; i < ncols_; ++i) {
			GdWaterSplashColumn *col = memnew(GdWaterSplashColumn(Vector2(x, y), Vector2(resolution_, dy), drag_));
			columns_.push_back(col);
			add_child(col);
			x += resolution_;
		}
	}
	update();
}

void GdWaterSplash::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			update();
		} break;

		case NOTIFICATION_LOCAL_TRANSFORM_CHANGED: {
			if (!is_inside_tree()) {
				break;
			}
		} break;

		case NOTIFICATION_PHYSICS_PROCESS: {
			const int n = columns_.size();
			real_t l[n], r[n];
			for (int i = 0; i < n; ++i) {
				columns_[i]->update(tension_, damping_);
			}
			for (int iter = 0; iter < 8; ++iter) {
				for (int i = 0; i < n; ++i) {
					if (i > 0) {
						l[i] = spread_ * (columns_[i]->height_ - columns_[i - 1]->height_);
						columns_[i - 1]->speed_ += l[i];
					}
					if (i < n - 1) {
						r[i] = spread_ * (columns_[i]->height_ - columns_[i + 1]->height_);
						columns_[i + 1]->speed_ += r[i];
					}
				}
				for (int i = 0; i < n; ++i) {
					if (i > 0) {
						columns_[i - 1]->height_ += l[i];
					}
					if (i < n - 1) {
						columns_[i + 1]->height_ += r[i];
					}
				}
			}
			update();
		} break;

		case NOTIFICATION_DRAW: {
			if (texture.is_null())
				return;

			Vector2 pos = rect_.position;

			Vector<Color> colors;
			colors.push_back(color_);
			colors.push_back(color_);
			colors.push_back(color_);
			colors.push_back(color_);

			for (uint32_t i = 0; i < ncols_ - 1; ++i) {
				Vector<Vector2> pts;
				pts.push_back(pos);
				pts.push_back(Vector2(pos.x + resolution_, pos.y));
				pts.push_back(Vector2(pos.x + resolution_, pos.y - columns_[i + 1]->height_));
				pts.push_back(Vector2(pos.x, pos.y - columns_[i]->height_));

				Vector<Vector2> uvs;
				uvs.push_back(Vector2(1, 1));
				uvs.push_back(Vector2(0, 1));
				uvs.push_back(Vector2(0, 0));
				uvs.push_back(Vector2(1, 0));

				draw_polygon(pts, colors, uvs, texture);
				pos.x += resolution_;
			}

		} break;
	}
}

void GdWaterSplash::set_size(const Rect2 &rect) {
	rect_ = rect;
	size_changed_ = true;
	_update();
}

Rect2 GdWaterSplash::get_size() const {
	return rect_;
}

void GdWaterSplash::set_resolution(const uint32_t &resolution) {
	ERR_FAIL_COND(resolution <= 0);
	resolution_ = resolution;
	size_changed_ = true;
	_update();
}

uint32_t GdWaterSplash::get_resolution() const {
	return resolution_;
}

void GdWaterSplash::set_color(const Color &color) {
	color_ = color;
	size_changed_ = false;
	_update();
}

Color GdWaterSplash::get_color() const {
	return color_;
}

void GdWaterSplash::set_damping(const real_t &val) {
	damping_ = val;
}

real_t GdWaterSplash::get_damping() const {
	return damping_;
}

void GdWaterSplash::set_tension(const real_t &val) {
	tension_ = val;
}

real_t GdWaterSplash::get_tension() const {
	return tension_;
}

void GdWaterSplash::set_spread(const real_t &val) {
	spread_ = val;
}

real_t GdWaterSplash::get_spread() const {
	return spread_;
}

Vector2 GdWaterSplash::get_drag() const {
	return drag_;
}

void GdWaterSplash::set_drag(const Vector2 &val) {
	drag_ = val;
	for (int i = 0; i < columns_.size(); ++i) {
		columns_[i]->drag_ = drag_;
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
	resolution_ = 5;
	color_ = Color(1, 1, 1, 1);
	damping_ = 0.025;
	tension_ = 0.025;
	spread_ = 0.25;
	drag_ = Vector2(0.01, 0.03);
	size_changed_ = true;
	set_size(Rect2(0, 0, 100, 20));
	set_physics_process(true);
}
