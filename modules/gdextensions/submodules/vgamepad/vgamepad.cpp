/*************************************************************************/
/*  vgamepad.cpp                                                         */
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

#include "vgamepad.h"

#include "core/input_map.h"
#include "core/os/input.h"
#include "core/vector.h"

void VGamePad::_input(const Ref<InputEvent> &p_event) {
	if (const InputEventMouseMotion *m = cast_to<InputEventMouseMotion>(*p_event)) {
	} else if (const InputEventKey *k = cast_to<InputEventKey>(*p_event)) {
	}
}

void VGamePad::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
		} break;
	}
}

#define JOY_AXIS_LEFT_X JOY_AXIS_0
#define JOY_AXIS_LEFT_Y JOY_AXIS_1
#define JOY_AXIS_RIGHT_X JOY_AXIS_2
#define JOY_AXIS_RIGHT_Y JOY_AXIS_3
#define JOY_AXIS_TRIGGER_LEFT JOY_AXIS_6
#define JOY_AXIS_TRIGGER_RIGHT JOY_AXIS_7
#define JOY_BUTTON_LEFT_SHOULDER JOY_L
#define JOY_BUTTON_RIGHT_SHOULDER JOY_R
#define JOY_BUTTON_LEFT_STICK JOY_L3
#define JOY_BUTTON_RIGHT_STICK JOY_R3
#define JOY_BUTTON_X JOY_BUTTON_2
#define JOY_BUTTON_Y JOY_BUTTON_3
#define JOY_BUTTON_B JOY_BUTTON_1
#define JOY_BUTTON_A JOY_BUTTON_0
#define JOY_BUTTON_BACK JOY_SELECT
#define JOY_BUTTON_START JOY_START

void VGamePad::_debug_draw(CanvasItem *canvas) {
	Vector2 size = Vector2(81, 69); // Set the size, the layout isn't dynamic and based on something I sketched!

	canvas->draw_rect(Rect2(0, 0, size.x, size.y), Color(0.15, 0.15, 0.15, 0.45)); // Draw the background

	_debug_draw_trigger(canvas, Vector2(5, 5), Input::get_singleton()->get_joy_axis(device, JOY_AXIS_TRIGGER_LEFT));
	_debug_draw_bumper(canvas, Vector2(5, 17), Input::get_singleton()->is_joy_button_pressed(device, JOY_BUTTON_LEFT_SHOULDER));

	_debug_draw_trigger(canvas, Vector2(52, 5), Input::get_singleton()->get_joy_axis(device, JOY_AXIS_TRIGGER_RIGHT));
	_debug_draw_bumper(canvas, Vector2(52, 17), Input::get_singleton()->is_joy_button_pressed(device, JOY_BUTTON_RIGHT_SHOULDER));

	const Vector2 dpad_position = Vector2(4, 26);

	// Draw dpad going from left, top, right and bottom
	_debug_draw_dpad_arrow(canvas, dpad_position + Vector2(1, 8), Math::deg2rad(-90.0), Input::get_singleton()->is_joy_button_pressed(device, JOY_DPAD_LEFT));
	_debug_draw_dpad_arrow(canvas, dpad_position + Vector2(8, 1), Math::deg2rad(0.0), Input::get_singleton()->is_joy_button_pressed(device, JOY_DPAD_UP));
	_debug_draw_dpad_arrow(canvas, dpad_position + Vector2(15, 8), Math::deg2rad(90.0), Input::get_singleton()->is_joy_button_pressed(device, JOY_DPAD_RIGHT));
	_debug_draw_dpad_arrow(canvas, dpad_position + Vector2(8, 15), Math::deg2rad(180.0), Input::get_singleton()->is_joy_button_pressed(device, JOY_DPAD_DOWN));

	_debug_draw_joystick(canvas, Vector2(30, 56), Input::get_singleton()->get_joy_axis(device, JOY_AXIS_LEFT_X), Input::get_singleton()->get_joy_axis(device, JOY_AXIS_LEFT_Y), Input::get_singleton()->is_joy_button_pressed(device, JOY_BUTTON_LEFT_STICK));
	_debug_draw_joystick(canvas, Vector2(49, 56), Input::get_singleton()->get_joy_axis(device, JOY_AXIS_RIGHT_X), Input::get_singleton()->get_joy_axis(device, JOY_AXIS_RIGHT_Y), Input::get_singleton()->is_joy_button_pressed(device, JOY_BUTTON_RIGHT_STICK));

	Vector2 face_buttons_position = Vector2(49, 25);

	// Draw face buttons going from left, top, right and bottom
	_debug_draw_face_button(canvas, face_buttons_position + Vector2(1, 9), Input::get_singleton()->is_joy_button_pressed(device, JOY_BUTTON_X));
	_debug_draw_face_button(canvas, face_buttons_position + Vector2(9, 1), Input::get_singleton()->is_joy_button_pressed(device, JOY_BUTTON_Y));
	_debug_draw_face_button(canvas, face_buttons_position + Vector2(17, 9), Input::get_singleton()->is_joy_button_pressed(device, JOY_BUTTON_B));
	_debug_draw_face_button(canvas, face_buttons_position + Vector2(9, 17), Input::get_singleton()->is_joy_button_pressed(device, JOY_BUTTON_A));

	_debug_draw_option_button(canvas, Vector2(33, 27), Input::get_singleton()->is_joy_button_pressed(device, JOY_BUTTON_BACK));
	_debug_draw_option_button(canvas, Vector2(42, 27), Input::get_singleton()->is_joy_button_pressed(device, JOY_BUTTON_START));
}

void VGamePad::_debug_draw_option_button(CanvasItem *canvas, const Point2 &position, bool is_pressed) {
	const real_t width = 6;
	const real_t height = 5;
	const Color color = is_pressed ? Color::RED : Color::WHITE;

	canvas->draw_rect(Rect2(position.x, position.y, width, height), Color::BLACK);
	canvas->draw_rect(Rect2(position.x + 1, position.y + 1, width - 2, height - 2), color);
}

void VGamePad::_debug_draw_dpad_arrow(CanvasItem *canvas, const Point2 &position, real_t angle, bool is_pressed) {
	const real_t width = 9;
	const real_t height = 9;
	const Color color = is_pressed ? Color::RED : Color::WHITE;

	// Points are for an arrow facing downwards
	Vector<Vector2> points = helper::vector(
			Vector2::ZERO, // Top left corner
			Vector2::ZERO + Vector2(width, 0), // Top right corner
			Vector2::ZERO + Vector2(width, height / 2), // Right edge
			Vector2::ZERO + Vector2(width / 2, height), // Arrow point
			Vector2::ZERO + Vector2(0, height / 2), // Left edge
			Vector2::ZERO // Top left corner again
	);

	Rect2 bounds = Rect2(position, Point2());

	// Rotate all the points and create a bounding box that contains them
	for (int index = 0; index < points.size(); index++) {
		const Point2 point = points[index];
		points.write[index] = point.rotated(angle);
		bounds = bounds.expand(points[index]);
	}

	// Re-align all the points so that the pivot point is always in the top left
	for (int index = 0; index < points.size(); index++) {
		const Point2 point = points[index];
		points.write[index] = position + (point - bounds.position);
	}

	canvas->draw_polygon(points, helper::vector(color));
	canvas->draw_polyline(points, Color::BLACK);
}

void VGamePad::_debug_draw_face_button(CanvasItem *canvas, const Point2 &position, bool is_pressed) {
	const real_t radius = 5;
	const Color color = is_pressed ? Color::RED : Color::WHITE;

	// Add the radius so that the pivot point is in the top left
	canvas->draw_circle(position + Point2(radius, radius), radius, Color::BLACK);
	canvas->draw_circle(position + Point2(radius, radius), radius - 1, color);
}

void VGamePad::_debug_draw_bumper(CanvasItem *canvas, const Point2 &position, bool is_pressed) {
	const real_t width = 24;
	const real_t height = 6;
	const Color color = is_pressed ? Color::RED : Color::WHITE;

	canvas->draw_rect(Rect2(position.x, position.y, width, height), Color::BLACK);
	canvas->draw_rect(Rect2(position.x + 1, position.y + 1, width - 2, height - 2), color);
}

void VGamePad::_debug_draw_trigger(CanvasItem *canvas, const Point2 &position, real_t axis) {
	const real_t width = 24;
	const real_t height = 10;

	// Draw background
	canvas->draw_rect(Rect2(position.x, position.y, width, height), Color::BLACK);
	canvas->draw_rect(Rect2(position.x + 1, position.y + 1, width - 2, height - 2), Color::WHITE);

	// Draw fill that fills up like a nice filling fill
	canvas->draw_rect(Rect2(position.x + 1, position.y + 1, width - 2, (height - 2) * axis), Color::RED);
}

void VGamePad::_debug_draw_joystick(CanvasItem *canvas, const Point2 &position, real_t axis_x, real_t axis_y, bool is_pressed) {
	const real_t radius = 8;
	const real_t max_distance = 4;
	const Color color = is_pressed ? Color::RED : Color::WHITE;

	// Draw stick container
	canvas->draw_circle(position, radius, Color::BLACK);
	canvas->draw_circle(position, radius - 1, Color::GRAY);

	// Draw the actual stick that moves around
	canvas->draw_circle(position + Point2(axis_x, axis_y) * max_distance, radius - 2, Color::BLACK);
	canvas->draw_circle(position + Point2(axis_x, axis_y) * max_distance, radius - 3, color);
}

void VGamePad::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_input"), &VGamePad::_input);
}

VGamePad::VGamePad() {
	_dirty = false;
	device = -1;
}

VGamePad::~VGamePad() {
}
