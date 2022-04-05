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

static void _draw_wheel(CanvasItem *c, const Rect2 &r, int offset, bool disabled, int orientation) {
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


#define _G(I) { (_gray_ramp[I-GRAY00]), (_gray_ramp[I-GRAY00]), (_gray_ramp[I-GRAY00]) }

	if (orientation == OrientationHorizontal) {
		// draw shaded ends of wheel:
		int h1 = r.size.width / 4 + 1; // distance from end that shading starts
		c->draw_rect(Rect2(r.left() + h1, r.top(), r.size.width - 2 * h1, r.size.height), _G(GRAY75), true);
		for (int i = 0; h1; i++) {
			const int shade = GRAY75 - i - 1;
			const int h2 = shade > GRAY33 ? 2*h1/3 + 1 : 0;
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
			const int h2 = shade > GRAY33 ? 2*h1/3 + 1 : 0;
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

void ThumbWheelH::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			const Rect2 r = Rect2(Point2(), get_size());
			const int offset = int(value / resolution);
			_draw_wheel(this, r, offset, disabled, _orientation);
		}
	}
}

void ThumbWheelH::_input(Ref<InputEvent> p_event) {
}

void ThumbWheelH::_gui_input(Ref<InputEvent> p_event) {
}

void ThumbWheelH::_unhandled_input(Ref<InputEvent> p_event) {
}

void ThumbWheelH::set_disabled(bool p_disabled) {
	disabled = p_disabled;
	update();
}

bool ThumbWheelH::is_disabled() const {
	return disabled;
}

void ThumbWheelH::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_disabled"), &ThumbWheelH::set_disabled);
	ClassDB::bind_method(D_METHOD("is_disabled"), &ThumbWheelH::is_disabled);

	ClassDB::bind_method(D_METHOD("_input"), &ThumbWheelH::_input);
	ClassDB::bind_method(D_METHOD("_gui_input"), &ThumbWheelH::_gui_input);
	ClassDB::bind_method(D_METHOD("_unhandled_input"), &ThumbWheelH::_unhandled_input);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_disabled", "is_disabled");

	ADD_SIGNAL(MethodInfo("transformation_changed", PropertyInfo(Variant::TRANSFORM, "tr")));
}

ThumbWheelH::ThumbWheelH() {
	set_size({100, 20});
	disabled = false;
}

// ThumbWheelV

void ThumbWheelV::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			const Rect2 r = Rect2(Point2(), get_size());
			const int offset = int(value / resolution);
			_draw_wheel(this, r, offset, disabled, _orientation);
		}
	}
}

void ThumbWheelV::_input(Ref<InputEvent> p_event) {
}

void ThumbWheelV::_gui_input(Ref<InputEvent> p_event) {
}

void ThumbWheelV::_unhandled_input(Ref<InputEvent> p_event) {
}

void ThumbWheelV::set_disabled(bool p_disabled) {
	disabled = p_disabled;
	update();
}

bool ThumbWheelV::is_disabled() const {
	return disabled;
}

void ThumbWheelV::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_disabled"), &ThumbWheelV::set_disabled);
	ClassDB::bind_method(D_METHOD("is_disabled"), &ThumbWheelV::is_disabled);

	ClassDB::bind_method(D_METHOD("_input"), &ThumbWheelV::_input);
	ClassDB::bind_method(D_METHOD("_gui_input"), &ThumbWheelV::_gui_input);
	ClassDB::bind_method(D_METHOD("_unhandled_input"), &ThumbWheelV::_unhandled_input);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_disabled", "is_disabled");

	ADD_SIGNAL(MethodInfo("transformation_changed", PropertyInfo(Variant::TRANSFORM, "tr")));
}

ThumbWheelV::ThumbWheelV() {
	set_size({20, 100});
	disabled = false;
}
