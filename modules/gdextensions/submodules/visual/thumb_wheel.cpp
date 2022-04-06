/*************************************************************************/
/*  thumb_wheel.cpp                                                      */
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

#include "thumb_wheel.h"

#include "core/math/math_funcs.h"
#include "core/os/input.h"
#include "core/os/input_event.h"

static void _draw_wheel(CanvasItem *c, Rect2 r, int offset, bool disabled, int orientation) {
	static uint8_t GRAY00 = 32;
	static uint8_t GRAY33 = 39;
	static uint8_t GRAY60 = 45;
	static uint8_t GRAY75 = 49;
	static uint8_t GRAY85 = 51;
	static uint8_t GRAY90 = 53;

	static real_t _gray_ramp[] = {
		0x00 / 255.f, // 0x20
		0x0d / 255.f, // 0x21
		0x1a / 255.f, // 0x22
		0x27 / 255.f, // 0x23
		0x34 / 255.f, // 0x24
		0x41 / 255.f, // 0x25
		0x4f / 255.f, // 0x26
		0x5c / 255.f, // 0x27
		0x69 / 255.f, // 0x28
		0x76 / 255.f, // 0x29
		0x83 / 255.f, // 0x2a
		0x90 / 255.f, // 0x2b
		0x9e / 255.f, // 0x2c
		0xab / 255.f, // 0x2d
		0xb8 / 255.f, // 0x2e
		0xc5 / 255.f, // 0x2f
		0xd2 / 255.f, // 0x30
		0xe0 / 255.f, // 0x31
		0xe5 / 255.f, // 0x32
		0xea / 255.f, // 0x33
		0xef / 255.f, // 0x34
		0xf4 / 255.f, // 0x35
		0xf9 / 255.f, // 0x36
		0xff / 255.f, // 0x37
	};

	const real_t ARC = 1.5; // 1/2 the number of radians visible
	const real_t delta = .2; // radians per knurl

	// Draws a series of line segments around the given box.
	// The string `s` must contain groups of 4 letters which specify one of 24
	// standard grayscale values, where 'A' is black and 'X' is white.
	// The order of each set of 4 characters is: bottom, right, top, left.
	auto draw_frame = [&](const char *s, const Rect2 &rc, real_t width = 1.0) {
		const real_t *g = _gray_ramp - 'A';
		real_t x = rc.position.x;
		real_t y = rc.position.y;
		real_t w = rc.size.width;
		real_t h = rc.size.height;
		if (h > 0 && w > 0)
			for (; *s;) {
				c->draw_line(Point2(x, y), Point2(x + w - 1, y), Color::solid(g[(int)*s++]), width); // draw top line
				y++;
				if (--h <= 0)
					break;
				c->draw_line(Point2(x, y + h - 1), Point2(x, y), Color::solid(g[(int)*s++]), width); // draw left line
				x++;
				if (--w <= 0)
					break;
				c->draw_line(Point2(x, y + h - 1), Point2(x + w - 1, y + h - 1), Color::solid(g[(int)*s++]), width); // draw bottom line
				if (--h <= 0)
					break;
				c->draw_line(Point2(x + w - 1, y + h - 1), Point2(x + w - 1, y), Color::solid(g[(int)*s++]), width); // draw right line
				if (--w <= 0)
					break;
			}
	};

	auto draw_frame2 = [&](const char *s, const Rect2 &rc, real_t width = 1.0) {
		const real_t *g = _gray_ramp - 'A';
		real_t x = rc.position.x;
		real_t y = rc.position.y;
		real_t w = rc.size.width;
		real_t h = rc.size.height;
		if (h > 0 && w > 0)
			for (; *s;) {
				c->draw_line(Point2(x, y + h - 1), Point2(x + w - 1, y + h - 1), Color::solid(g[(int)*s++]), width); // draw bottom line
				if (--h <= 0)
					break;
				c->draw_line(Point2(x + w - 1, y + h - 1), Point2(x + w - 1, y), Color::solid(g[(int)*s++]), width); // draw right line
				if (--w <= 0)
					break;
				c->draw_line(Point2(x, y), Point2(x + w - 1, y), Color::solid(g[(int)*s++]), width); // draw top line
				y++;
				if (--h <= 0)
					break;
				c->draw_line(Point2(x, y + h - 1), Point2(x, y), Color::solid(g[(int)*s++]), width); // draw left line
				x++;
				if (--w <= 0)
					break;
			}
	};

#define _G(I) { (_gray_ramp[I - GRAY00]), (_gray_ramp[I - GRAY00]), (_gray_ramp[I - GRAY00]) }

	draw_frame("NNTUJJWWAAAA", r, 1);
	r.grow_by(-2);

	if (orientation == OrientationHorizontal) {
		// draw shaded ends of wheel:
		int h1 = r.size.width / 4 + 1; // distance from end that shading starts
		c->draw_rect(Rect2(r.left() + h1, r.top(), r.size.width - 2 * h1, r.size.height), _G(GRAY75), true);
		for (int i = 0; h1; i++) {
			const int shade = GRAY75 - i - 1;
			const int h2 = shade > GRAY33 ? 2 * h1 / 3 + 1 : 0;
			c->draw_rect(Rect2(r.left() + h2, r.top(), h1 - h2, r.size.height), _G(shade), true);
			c->draw_rect(Rect2(r.right() - h1, r.top(), h1 - h2, r.size.height), _G(shade), true);
			h1 = h2;
		}
		if (!disabled) {
			// draw ridges:
			real_t junk;
			for (real_t y = -ARC + Math::modf(offset * Math::sin(ARC) / (r.size.width / 2) / delta, &junk) * delta;; y += delta) {
				int y1 = int((Math::sin(y) / Math::sin(ARC) + 1) * r.size.width / 2);
				if (y1 <= 0)
					continue;
				else if (y1 >= r.size.width - 1)
					break;
				c->draw_line(Point2(r.left() + y1, r.top() + 1), Point2(r.left() + y1, r.bottom() - 1), _G(GRAY33), 2);
				if (y < 0)
					y1--;
				else
					y1++;
				c->draw_line(Point2(r.left() + y1, r.top() + 1), Point2(r.left() + y1, r.bottom() - 1), _G(GRAY85));
			}
			// draw edges:
			h1 = r.size.width / 8 + 1; // distance from end the color inverts
			c->draw_line(Point2(r.left() + h1, r.bottom() - 1), Point2(r.right() - h1, r.bottom() - 1), _G(GRAY60));
			c->draw_line(Point2(r.left(), r.bottom()), Point2(r.left(), r.top()), _G(GRAY33));
			c->draw_line(Point2(r.left(), r.top()), Point2(r.left() + h1, r.top()), _G(GRAY33));
			c->draw_line(Point2(r.bottom() - h1, r.top()), Point2(r.right(), r.top()), _G(GRAY33));
			c->draw_line(Point2(r.left() + h1, r.top()), Point2(r.right() - h1, r.top()), _G(GRAY90));
			c->draw_line(Point2(r.right(), r.top()), Point2(r.right(), r.bottom()), _G(GRAY90));
			c->draw_line(Point2(r.right(), r.bottom()), Point2(r.right() - h1, r.bottom()), _G(GRAY90));
			c->draw_line(Point2(r.left() + h1, r.bottom()), Point2(r.left(), r.bottom()), _G(GRAY90));
		}
	} else if (orientation == OrientationVerical) {
		offset = (1 - offset);
		// draw shaded ends of wheel:
		int h1 = r.size.height / 4 + 1; // distance from end that shading starts
		c->draw_rect(Rect2(r.left(), r.top() + h1, r.size.width, r.size.height - 2 * h1), _G(GRAY75), true);
		for (int i = 0; h1; i++) {
			const int shade = GRAY75 - i - 1;
			const int h2 = shade > GRAY33 ? 2 * h1 / 3 + 1 : 0;
			c->draw_rect(Rect2(r.left(), r.top() + h2, r.size.width, h1 - h2), _G(shade), true);
			c->draw_rect(Rect2(r.left(), r.bottom() - h1, r.size.width, h1 - h2), _G(shade), true);
			h1 = h2;
		}
		if (!disabled) {
			// draw ridges:
			real_t junk;
			for (real_t y = -ARC + Math::modf(offset * Math::sin(ARC) / (r.size.height / 2) / delta, &junk) * delta;; y += delta) {
				int y1 = int((Math::sin(y) / Math::sin(ARC) + 1) * r.size.height / 2);
				if (y1 <= 0)
					continue;
				else if (y1 >= r.size.height - 1)
					break;
				c->draw_line(Point2(r.left() + 1, r.top() + y1), Point2(r.right() - 1, r.top() + y1), _G(GRAY33), 2);
				if (y < 0)
					y1--;
				else
					y1++;
				c->draw_line(Point2(r.left() + 1, r.top() + y1), Point2(r.right() - 1, r.top() + y1), _G(GRAY85));
			}
		}
		// draw edges:
		h1 = r.size.height / 8 + 1; // distance from end the color inverts
		c->draw_line(Point2(r.right() - 1, r.top() + h1), Point2(r.right() - 1, r.bottom() - h1), _G(GRAY60));
		c->draw_line(Point2(r.right(), r.top()), Point2(r.left(), r.top()), _G(GRAY33));
		c->draw_line(Point2(r.left(), r.top()), Point2(r.left(), r.top() + h1), _G(GRAY33));
		c->draw_line(Point2(r.left(), r.bottom() - h1), Point2(r.left(), r.bottom()), _G(GRAY33));
		c->draw_line(Point2(r.left(), r.top() + h1), Point2(r.left(), r.bottom() - h1), _G(GRAY90));
		c->draw_line(Point2(r.left(), r.bottom()), Point2(r.right(), r.bottom()), _G(GRAY90));
		c->draw_line(Point2(r.right(), r.bottom()), Point2(r.right(), r.bottom() - h1), _G(GRAY90));
		c->draw_line(Point2(r.right(), r.top() + h1), Point2(r.right(), r.top()), _G(GRAY90));
	} else {
		WARN_PRINT("Invalid value");
	}
}

// ThumbWheelH

bool ThumbWheelH::_is_point_inside(const Point2 &point) const {
	return get_global_rect().has_point(point);
}

Point2 ThumbWheelH::_to_local(Point2 global) const {
	return get_global_transform().affine_inverse().xform(global);
}

void ThumbWheelH::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			const Rect2 r = Rect2(Point2(), get_size());
			const int offset = int(value / resolution);
			_draw_wheel(this, r, offset, disabled, OrientationHorizontal);
		}
	}
}

void ThumbWheelH::_gui_input(Ref<InputEvent> p_event) {
	if (!p_event.is_valid()) {
		return;
	}
	if (!get_tree()) {
		return;
	}

	ERR_FAIL_COND(!is_visible_in_tree());

	if (const InputEventMouseButton *e = Object::cast_to<InputEventMouseButton>(*p_event)) {
		if (e->get_button_index() == BUTTON_LEFT) {
			if (e->is_pressed() && _is_point_inside(e->get_global_position())) {
				if (!_state.active) {
					_state.active = true;
					_state.click_pos = _to_local(e->get_position());
					_state.base_value = value;
					Input::get_singleton()->set_default_cursor_shape(Input::CURSOR_HSIZE);
				}
				get_tree()->set_input_as_handled();
			} else if (!e->is_pressed()) {
				if (_state.active) {
					_state.active = false;
					Input::get_singleton()->set_default_cursor_shape(Input::CURSOR_ARROW);
				}
			}
		}
	}

	if (const InputEventMouseMotion *e = Object::cast_to<InputEventMouseMotion>(*p_event)) {
		if (_state.active) {
			const Point2 p = _to_local(e->get_position());
			const Point2 pf = p - _state.click_pos;
			if (pf.x) {
				value = _state.base_value + pf.x;
				emit_signal("changed", pf.y, value);
				update();
			}
		}
	}
}

void ThumbWheelH::set_disabled(bool p_disabled) {
	disabled = p_disabled;
	update();
}

bool ThumbWheelH::is_disabled() const {
	return disabled;
}

void ThumbWheelH::set_resolution(real_t p_res) {
	resolution = p_res;
}

real_t ThumbWheelH::get_resolution() const {
	return resolution;
}

void ThumbWheelH::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_disabled"), &ThumbWheelH::set_disabled);
	ClassDB::bind_method(D_METHOD("is_disabled"), &ThumbWheelH::is_disabled);
	ClassDB::bind_method(D_METHOD("set_resolution"), &ThumbWheelH::set_resolution);
	ClassDB::bind_method(D_METHOD("get_resolution"), &ThumbWheelH::get_resolution);

	ClassDB::bind_method(D_METHOD("_gui_input"), &ThumbWheelH::_gui_input);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_disabled", "is_disabled");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "resolution"), "set_resolution", "get_resolution");

	ADD_SIGNAL(MethodInfo("changed", PropertyInfo(Variant::REAL, "delta")));
}

ThumbWheelH::ThumbWheelH() {
	set_size({ 100, 15 });
	value = 0;
	resolution = 1;
	disabled = false;
	_state = { false };
}

// ThumbWheelV

bool ThumbWheelV::_is_point_inside(const Point2 &point) const {
	return get_global_rect().has_point(point);
}

Point2 ThumbWheelV::_to_local(Point2 global) const {
	return get_global_transform().affine_inverse().xform(global);
}

void ThumbWheelV::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			const Rect2 r = Rect2(Point2(), get_size());
			const int offset = int(value / resolution);
			_draw_wheel(this, r, offset, disabled, OrientationVerical);
		}
	}
}

void ThumbWheelV::_gui_input(Ref<InputEvent> p_event) {
	if (!p_event.is_valid()) {
		return;
	}
	if (!get_tree()) {
		return;
	}

	ERR_FAIL_COND(!is_visible_in_tree());

	if (const InputEventMouseButton *e = Object::cast_to<InputEventMouseButton>(*p_event)) {
		if (e->get_button_index() == BUTTON_LEFT) {
			if (e->is_pressed() && _is_point_inside(e->get_global_position())) {
				if (!_state.active) {
					_state.active = true;
					_state.click_pos = _to_local(e->get_position());
					_state.base_value = value;
					Input::get_singleton()->set_default_cursor_shape(Input::CURSOR_VSIZE);
				}
				get_tree()->set_input_as_handled();
			} else if (!e->is_pressed()) {
				if (_state.active) {
					_state.active = false;
					Input::get_singleton()->set_default_cursor_shape(Input::CURSOR_ARROW);
				}
			}
		}
	}

	if (const InputEventMouseMotion *e = Object::cast_to<InputEventMouseMotion>(*p_event)) {
		if (_state.active) {
			const Point2 p = _to_local(e->get_position());
			const Point2 pf = _state.click_pos - p;
			if (pf.y) {
				value = _state.base_value + pf.y;
				emit_signal("changed", pf.y, value);
				update();
			}
		}
	}
}

void ThumbWheelV::set_disabled(bool p_disabled) {
	disabled = p_disabled;
	update();
}

bool ThumbWheelV::is_disabled() const {
	return disabled;
}

void ThumbWheelV::set_resolution(real_t p_res) {
	resolution = p_res;
}

real_t ThumbWheelV::get_resolution() const {
	return resolution;
}

void ThumbWheelV::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_disabled"), &ThumbWheelV::set_disabled);
	ClassDB::bind_method(D_METHOD("is_disabled"), &ThumbWheelV::is_disabled);
	ClassDB::bind_method(D_METHOD("set_resolution"), &ThumbWheelV::set_resolution);
	ClassDB::bind_method(D_METHOD("get_resolution"), &ThumbWheelV::get_resolution);

	ClassDB::bind_method(D_METHOD("_gui_input"), &ThumbWheelV::_gui_input);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_disabled", "is_disabled");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "resolution"), "set_resolution", "get_resolution");

	ADD_SIGNAL(MethodInfo("changed", PropertyInfo(Variant::REAL, "delta"), PropertyInfo(Variant::REAL, "value")));
}

ThumbWheelV::ThumbWheelV() {
	set_size({ 15, 100 });
	value = 0;
	resolution = 1;
	disabled = false;
	_state = { false };
}
