/**************************************************************************/
/*  starfield_2d.cpp                                                      */
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

#include "starfield_2d.h"
#include "common/gd_core.h"

#ifdef TOOLS_ENABLED
Dictionary Starfield2D::_edit_get_state() const {
	Dictionary state = Node2D::_edit_get_state();
	state["view_size"] = get_view_size();

	return state;
}

void Starfield2D::_edit_set_state(const Dictionary &p_state) {
	Node2D::_edit_set_state(p_state);
	set_view_size(p_state["view_size"]);
}

bool Starfield2D::_edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const {
	return _edit_get_rect().has_point(p_point);
};

Rect2 Starfield2D::_edit_get_rect() const {
	return Rect2(Point2(), get_view_size());
}

void Starfield2D::_edit_set_rect(const Rect2 &p_rect) {
	set_view_size(p_rect.size);
	_change_notify();
}

bool Starfield2D::_edit_use_rect() const {
	return true;
}
#endif

void Starfield2D::_notification(int p_what) {
	ERR_FAIL_COND(_starfield.is_null());

	switch (p_what) {
		case NOTIFICATION_READY: {
			_starfield->ready(this);
			set_process(true);
		} break;

		case NOTIFICATION_PROCESS: {
			if (movement_active) {
				const real_t dt = get_process_delta_time();
				_starfield->move(dt, movement_vector);
				update();
			}
		} break;

		case NOTIFICATION_DRAW: {
			_starfield->draw(this);
		} break;
	}
}

void Starfield2D::set_movement_vector(const Vector2 &p_movement) {
	if (p_movement != movement_vector) {
		movement_vector = p_movement;
		emit_signal("movement_changed");
		update();
	}
}

Vector2 Starfield2D::get_movement_vector() const {
	return movement_vector;
}

void Starfield2D::set_view_size(const Size2 &p_size) {
	if (p_size != view_size) {
		view_size = p_size;
		item_rect_changed();
		emit_signal("view_size_changed");
		update();
	}
}

Size2 Starfield2D::get_view_size() const {
	return view_size;
}

void Starfield2D::set_movement_active(bool p_state) {
	if (p_state != movement_active) {
		movement_active = p_state;
	}
}

bool Starfield2D::is_movement_active() const {
	return movement_active;
}

int Starfield2D::add_stars_layer(int p_num_stars, Vector2 p_expanse_size, real_t p_star_size, Color p_star_color) {
	ERR_FAIL_COND_V(_starfield.is_null(), -1);
	ERR_FAIL_COND_V(p_star_size <= 0, -1);

	return _starfield->add_stars(p_num_stars, p_expanse_size, p_star_size, Starfield::STAR_POINT, p_star_color);
}

int Starfield2D::add_point_stars_layer(int p_num_stars, Vector2 p_expanse_size, Color p_star_color) {
	ERR_FAIL_COND_V(_starfield.is_null(), -1);

	return _starfield->add_stars(p_num_stars, p_expanse_size, 0, Starfield::STAR_POINT, p_star_color);
}

int Starfield2D::add_texture_stars_layer(int p_num_stars, Vector2 p_expanse_size, real_t p_star_size, Starfield::StarTexture p_texture_id, Color p_star_color) {
	ERR_FAIL_COND_V(_starfield.is_null(), -1);
	ERR_FAIL_COND_V(p_star_size <= 0, -1);

	return _starfield->add_stars(p_num_stars, p_expanse_size, p_star_size, p_texture_id, p_star_color);
}

void Starfield2D::set_layer_movement_opt(int p_layer, Vector2 p_movement_scale, bool p_with_alpha) {
	ERR_FAIL_COND(_starfield.is_null());
	ERR_FAIL_COND(p_layer < 0);

	_starfield->set_movement(p_layer, p_movement_scale, p_with_alpha);
}

void Starfield2D::set_layer_color_opt(int p_layer, Color p_base_color, real_t p_alpha_pulsation) {
	ERR_FAIL_COND(_starfield.is_null());
	ERR_FAIL_COND(p_layer < 0);

	_starfield->set_color(p_layer, p_base_color, p_alpha_pulsation);
}

void Starfield2D::_bind_methods() {
	BIND_ENUM_CONSTANT_CUSTOM(Starfield::STAR1_TEXTURE, "STAR1_TEXTURE");
	BIND_ENUM_CONSTANT_CUSTOM(Starfield::STAR2_TEXTURE, "STAR2_TEXTURE");
	BIND_ENUM_CONSTANT_CUSTOM(Starfield::STAR3_TEXTURE, "STAR3_TEXTURE");
	BIND_ENUM_CONSTANT_CUSTOM(Starfield::STAR4_TEXTURE, "STAR4_TEXTURE");
	BIND_ENUM_CONSTANT_CUSTOM(Starfield::STAR5_TEXTURE, "STAR5_TEXTURE");
	BIND_ENUM_CONSTANT_CUSTOM(Starfield::STAR6_TEXTURE, "STAR6_TEXTURE");
	BIND_ENUM_CONSTANT_CUSTOM(Starfield::STAR7_TEXTURE, "STAR7_TEXTURE");
	BIND_ENUM_CONSTANT_CUSTOM(Starfield::STAR8_TEXTURE, "STAR8_TEXTURE");
	BIND_ENUM_CONSTANT_CUSTOM(Starfield::STAR9_TEXTURE, "STAR9_TEXTURE");
	BIND_ENUM_CONSTANT_CUSTOM(Starfield::STAR10_TEXTURE, "STAR10_TEXTURE");
	BIND_ENUM_CONSTANT_CUSTOM(Starfield::STAR11_TEXTURE, "STAR11_TEXTURE");
	BIND_ENUM_CONSTANT_CUSTOM(Starfield::STAR12_TEXTURE, "STAR12_TEXTURE");

	ClassDB::bind_method(D_METHOD("set_movement_vector"), &Starfield2D::set_movement_vector);
	ClassDB::bind_method(D_METHOD("get_movement_vector"), &Starfield2D::get_movement_vector);
	ClassDB::bind_method(D_METHOD("set_movement_active"), &Starfield2D::set_movement_active);
	ClassDB::bind_method(D_METHOD("is_movement_active"), &Starfield2D::is_movement_active);
	ClassDB::bind_method(D_METHOD("get_view_size"), &Starfield2D::get_view_size);
	ClassDB::bind_method(D_METHOD("set_view_size"), &Starfield2D::set_view_size);

	ClassDB::bind_method(D_METHOD("add_stars_layer", "num_stars", "expanse_size", "star_size", "star_color"), &Starfield2D::add_stars_layer);
	ClassDB::bind_method(D_METHOD("add_point_stars_layer", "num_stars", "expanse_size", "star_color"), &Starfield2D::add_point_stars_layer);
	ClassDB::bind_method(D_METHOD("add_texture_stars_layer", "num_stars", "expanse_size", "star_size", "texture_id", "star_color"), &Starfield2D::add_texture_stars_layer);
	ClassDB::bind_method(D_METHOD("set_layer_movement_opt", "movement_scale", "with_alpha"), &Starfield2D::set_layer_movement_opt);
	ClassDB::bind_method(D_METHOD("set_layer_color_opt", "base_color", "alpha_pulsation"), &Starfield2D::set_layer_color_opt);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "view_size"), "set_view_size", "get_view_size");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "movement_vector"), "set_movement_vector", "get_movement_vector");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "movement_active"), "set_movement_active", "is_movement_active");

	ADD_SIGNAL(MethodInfo("view_size_changed"));
	ADD_SIGNAL(MethodInfo("movement_changed"));
}

Starfield2D::Starfield2D() {
	movement_vector = Vector2(1, 1);
	movement_active = false;
	view_size = Vector2(100, 100);

	_starfield = Ref<Starfield>(memnew(Starfield));
}

Starfield2D::~Starfield2D() {
}
