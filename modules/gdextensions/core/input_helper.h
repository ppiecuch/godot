/**************************************************************************/
/*  input_helper.h                                                        */
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

// Port of nathanhoad/godot_input_helper (MIT). Detects which input device the
// player is currently using and helps render device-aware shortcut labels.
// Registered as Engine singleton "InputHelper".
//
// Typical use:
//   func _input(event):
//       InputHelper.update_from_event(event)
//   func _ready():
//       InputHelper.connect("device_changed", self, "_on_device_changed")
//   func _on_device_changed(device, _idx):
//       $btn.text = InputHelper.get_label_for_input(some_event)

#ifndef INPUT_HELPER_H
#define INPUT_HELPER_H

#include "core/object.h"
#include "core/reference.h"

class InputEvent;

class InputHelper : public Object {
	GDCLASS(InputHelper, Object)

	String device;
	int device_index;
	int device_last_changed_at;
	float deadzone;
	int mouse_motion_threshold;

	// Persists across keyboard-only intervals so scripts can query "last gamepad".
	String last_known_joypad_device;
	int last_known_joypad_index;

	static InputHelper *singleton;

	void _on_joy_connection_changed(int p_index, bool p_connected);

protected:
	static void _bind_methods();

public:
	// Device name constants.
	static const char *DEVICE_KEYBOARD;
	static const char *DEVICE_XBOX;
	static const char *DEVICE_PLAYSTATION;
	static const char *DEVICE_SWITCH;
	static const char *DEVICE_STEAMDECK;
	static const char *DEVICE_GENERIC;

	// Sub-device constants (granular, from Godot 4 upstream).
	static const char *SUB_DEVICE_XBOX_ONE;
	static const char *SUB_DEVICE_XBOX_SERIES;
	static const char *SUB_DEVICE_PS3;
	static const char *SUB_DEVICE_PS4;
	static const char *SUB_DEVICE_PS5;
	static const char *SUB_DEVICE_SWITCH_JOYCON_LEFT;
	static const char *SUB_DEVICE_SWITCH_JOYCON_RIGHT;

	static InputHelper *get_singleton() { return singleton; }

	// Call from your script's _input() to keep the device tracker fresh.
	void update_from_event(const Ref<InputEvent> &p_event);

	// Map an Input.get_joy_name() string to one of the DEVICE_* constants.
	String get_simplified_device_name(const String &p_raw_name) const;

	// Granular variant: returns SUB_DEVICE_* when possible.
	String get_granular_device_name(const String &p_raw_name) const;

	// Detect device from event without changing internal state.
	String get_device_from_event(const Ref<InputEvent> &p_event) const;

	// Best-effort device guess at startup, before any input has arrived.
	String guess_device_name() const;

	bool has_gamepad() const;
	bool is_keyboard() const { return device == DEVICE_KEYBOARD; }
	bool is_gamepad() const { return !is_keyboard(); }

	String get_device() const { return device; }
	int get_device_index() const { return device_index; }
	String get_last_known_joypad_device() const { return last_known_joypad_device; }
	int get_last_known_joypad_index() const { return last_known_joypad_index; }

	void set_deadzone(float p_d) { deadzone = p_d; }
	float get_deadzone() const { return deadzone; }
	void set_mouse_motion_threshold(int p_t) { mouse_motion_threshold = p_t; }
	int get_mouse_motion_threshold() const { return mouse_motion_threshold; }

	// InputMap inspection helpers (return the first matching event).
	String get_action_key(const String &p_action) const;
	int get_action_button(const String &p_action) const;

	// Universal label for any InputEvent (key, mouse, button, axis).
	String get_label_for_input(const Ref<InputEvent> &p_event) const;

	// Device-aware label: keyboard → "A"/"Space"; gamepad → platform label.
	String get_label(int p_scancode, int p_joy_button) const;

	// Lower-level: gamepad-button-only label for the given device.
	String get_gamepad_button_label(const String &p_device, int p_joy_button) const;

	// Gamepad axis/motion label ("Left Stick Left", "Right Trigger", etc.)
	String get_gamepad_axis_label(int p_axis, float p_value) const;

	// Input remapping (from upstream v2.x). Swap-if-taken avoids duplicate bindings.
	Error set_action_key(const String &p_action, const String &p_key, bool p_swap_if_taken = true);
	Error set_action_button(const String &p_action, int p_button, bool p_swap_if_taken = true);
	void reset_all_actions();

	// Device-aware action query: returns first key or joypad event depending on current device.
	Ref<InputEvent> get_keyboard_or_joypad_input_for_action(const String &p_action) const;

	// Rumble convenience (from upstream).
	void rumble_small(int p_device = 0);
	void rumble_medium(int p_device = 0);
	void rumble_large(int p_device = 0);
	void start_rumble_small(int p_device = 0);
	void start_rumble_medium(int p_device = 0);
	void start_rumble_large(int p_device = 0);
	void stop_rumble(int p_device = 0);

	InputHelper();
	~InputHelper();
};

#endif // INPUT_HELPER_H
