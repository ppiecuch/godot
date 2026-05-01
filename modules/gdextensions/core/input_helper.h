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

// Port of nathanhoad/godot_input_helper (v2.x branch, MIT). Detects which
// input device the player is currently using and helps render device-aware
// shortcut labels. Registered as Engine singleton "InputHelper".
//
// Typical use:
//   func _input(event):
//       InputHelper.update_from_event(event)
//   func _ready():
//       InputHelper.connect("device_changed", self, "_on_device_changed")
//   func _on_device_changed(device, _idx):
//       $btn.text = "Active [%s]" % InputHelper.get_label(KEY_A, JOY_XBOX_A)

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

	static InputHelper *singleton;

protected:
	static void _bind_methods();

public:
	static InputHelper *get_singleton() { return singleton; }

	// Call from your script's _input() to keep the device tracker fresh.
	void update_from_event(const Ref<InputEvent> &p_event);

	// Map an Input.get_joy_name() string to one of the DEVICE_* constants.
	String get_simplified_device_name(const String &p_raw_name) const;

	// Best-effort device guess at startup, before any input has arrived.
	String guess_device_name() const;

	bool has_gamepad() const;
	bool is_keyboard() const { return device == "keyboard"; }
	bool is_gamepad() const { return !is_keyboard(); }

	String get_device() const { return device; }
	int get_device_index() const { return device_index; }

	void set_deadzone(float p_d) { deadzone = p_d; }
	float get_deadzone() const { return deadzone; }

	// InputMap inspection helpers (return the first matching event).
	String get_action_key(const String &p_action) const;
	int get_action_button(const String &p_action) const;

	// Device-aware label: keyboard → "A"/"Space"; gamepad → "A"/"B"/"X"/"Y"
	// (Xbox-style for xbox/generic/switch, PlayStation symbols for ps).
	// For Switch, A↔B and X↔Y are swapped to match Nintendo's labeling.
	String get_label(int p_scancode, int p_joy_button) const;

	// Lower-level: gamepad-button-only label for the given device.
	String get_gamepad_button_label(const String &p_device, int p_joy_button) const;

	InputHelper();
	~InputHelper();
};

#endif // INPUT_HELPER_H
