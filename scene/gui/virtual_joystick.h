/**************************************************************************/
/*  virtual_joystick.h                                                    */
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

#ifndef VIRTUAL_JOYSTICK_H
#define VIRTUAL_JOYSTICK_H

#include "scene/gui/control.h"

class VirtualJoystick : public Control {
	GDCLASS(VirtualJoystick, Control);

public:
	enum JoystickMode {
		JOYSTICK_FIXED,
		JOYSTICK_DYNAMIC,
		JOYSTICK_FOLLOWING,
	};

	enum VisibilityMode {
		VISIBILITY_ALWAYS,
		VISIBILITY_WHEN_TOUCHED,
	};

private:
	JoystickMode joystick_mode;
	float joystick_size;
	float tip_size;
	float deadzone_ratio;
	float clampzone_ratio;
	Vector2 initial_offset_ratio;
	String action_left;
	String action_right;
	String action_up;
	String action_down;
	VisibilityMode visibility;

	bool _is_pressed;
	bool has_input;
	bool has_moved;
	Vector2 raw_input_vector;
	Vector2 input_vector;
	bool is_flick_canceled;
	int touch_index;

	Vector2 joystick_pos;
	Vector2 tip_pos;

	Ref<Texture> joystick_texture;
	Ref<Texture> joystick_texture_pressed;
	Ref<Texture> tip_texture;
	Ref<Texture> tip_texture_pressed;

	Color ring_normal_color;
	Color tip_normal_color;
	Color ring_pressed_color;
	Color tip_pressed_color;

	void _update_joystick(const Vector2 &p_pos);
	void _handle_input_actions();
	void _reset();
	void _texture_changed();

protected:
	void _gui_input(Ref<InputEvent> p_event);
	void _notification(int p_what);
	static void _bind_methods();

public:
	Vector2 get_joystick_position() const;

	void set_joystick_mode(JoystickMode p_mode);
	JoystickMode get_joystick_mode() const;

	void set_joystick_size(float p_size);
	float get_joystick_size() const;

	void set_tip_size(float p_size);
	float get_tip_size() const;

	void set_deadzone_ratio(float p_ratio);
	float get_deadzone_ratio() const;

	void set_clampzone_ratio(float p_ratio);
	float get_clampzone_ratio() const;

	void set_initial_offset_ratio(const Vector2 &p_ratio);
	Vector2 get_initial_offset_ratio() const;

	void set_action_left(const String &p_action);
	String get_action_left() const;
	void set_action_right(const String &p_action);
	String get_action_right() const;
	void set_action_up(const String &p_action);
	String get_action_up() const;
	void set_action_down(const String &p_action);
	String get_action_down() const;

	void set_visibility_mode(VisibilityMode p_mode);
	VisibilityMode get_visibility_mode() const;

	void set_joystick_texture(const Ref<Texture> &p_texture);
	Ref<Texture> get_joystick_texture() const;
	void set_joystick_texture_pressed(const Ref<Texture> &p_texture);
	Ref<Texture> get_joystick_texture_pressed() const;
	void set_tip_texture(const Ref<Texture> &p_texture);
	Ref<Texture> get_tip_texture() const;
	void set_tip_texture_pressed(const Ref<Texture> &p_texture);
	Ref<Texture> get_tip_texture_pressed() const;

	void setup(const Ref<Texture> &p_joystick, const Ref<Texture> &p_joystick_pressed,
			const Ref<Texture> &p_tip, const Ref<Texture> &p_tip_pressed,
			const Vector2 &p_sizes = Vector2(-1, -1));

	bool is_pressed() const;

	VirtualJoystick();
};

VARIANT_ENUM_CAST(VirtualJoystick::JoystickMode);
VARIANT_ENUM_CAST(VirtualJoystick::VisibilityMode);

#endif // VIRTUAL_JOYSTICK_H
