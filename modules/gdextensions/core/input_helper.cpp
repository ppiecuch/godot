/**************************************************************************/
/*  input_helper.cpp                                                      */
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

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

#include "input_helper.h"

#include "core/engine.h"
#include "core/input_map.h"
#include "core/os/input.h"
#include "core/os/input_event.h"
#include "core/os/keyboard.h"
#include "core/os/os.h"

const char *InputHelper::DEVICE_KEYBOARD = "keyboard";
const char *InputHelper::DEVICE_XBOX = "xbox";
const char *InputHelper::DEVICE_PLAYSTATION = "playstation";
const char *InputHelper::DEVICE_SWITCH = "switch";
const char *InputHelper::DEVICE_STEAMDECK = "steamdeck";
const char *InputHelper::DEVICE_GENERIC = "generic";

const char *InputHelper::SUB_DEVICE_XBOX_ONE = "xbox_one";
const char *InputHelper::SUB_DEVICE_XBOX_SERIES = "xbox_series";
const char *InputHelper::SUB_DEVICE_PS3 = "playstation3";
const char *InputHelper::SUB_DEVICE_PS4 = "playstation4";
const char *InputHelper::SUB_DEVICE_PS5 = "playstation5";
const char *InputHelper::SUB_DEVICE_SWITCH_JOYCON_LEFT = "switch_left_joycon";
const char *InputHelper::SUB_DEVICE_SWITCH_JOYCON_RIGHT = "switch_right_joycon";

InputHelper *InputHelper::singleton = nullptr;

InputHelper::InputHelper() {
	singleton = this;
	device = DEVICE_GENERIC;
	device_index = -1;
	device_last_changed_at = 0;
	deadzone = 0.5f;
	mouse_motion_threshold = 10;
	last_known_joypad_device = "";
	last_known_joypad_index = -1;

	// Connect joypad connect/disconnect signals.
	Input::get_singleton()->connect("joy_connection_changed", this, "_on_joy_connection_changed");
}

InputHelper::~InputHelper() {
	if (Input::get_singleton()) {
		Input::get_singleton()->disconnect("joy_connection_changed", this, "_on_joy_connection_changed");
	}
	singleton = nullptr;
}

void InputHelper::_on_joy_connection_changed(int p_index, bool p_connected) {
	emit_signal("joypad_changed", p_index, p_connected);
	if (!p_connected && device_index == p_index) {
		// Active gamepad disconnected — fall back to keyboard.
		device = DEVICE_KEYBOARD;
		device_index = -1;
		emit_signal("device_changed", device, device_index);
	}
}

void InputHelper::update_from_event(const Ref<InputEvent> &p_event) {
	if (p_event.is_null()) {
		return;
	}

	String next_device = device;
	int next_device_index = device_index;

	Ref<InputEventKey> ek = p_event;
	Ref<InputEventMouseButton> emb = p_event;
	Ref<InputEventMouseMotion> emm = p_event;
	Ref<InputEventJoypadButton> ejb = p_event;
	Ref<InputEventJoypadMotion> ejm = p_event;

	if (ek.is_valid() && ek->is_pressed()) {
		next_device = DEVICE_KEYBOARD;
		next_device_index = -1;
	} else if (emb.is_valid() && emb->is_pressed()) {
		// Mouse input counts as keyboard device (like upstream).
		next_device = DEVICE_KEYBOARD;
		next_device_index = -1;
	} else if (emm.is_valid()) {
		// Only switch to keyboard on significant mouse movement.
		Vector2 rel = emm->get_relative();
		if (rel.length() > mouse_motion_threshold) {
			next_device = DEVICE_KEYBOARD;
			next_device_index = -1;
		} else {
			return;
		}
	} else if (ejb.is_valid() && ejb->is_pressed()) {
		next_device = get_simplified_device_name(Input::get_singleton()->get_joy_name(ejb->get_device()));
		next_device_index = ejb->get_device();
	} else if (ejm.is_valid() && Math::abs(ejm->get_axis_value()) > deadzone) {
		next_device = get_simplified_device_name(Input::get_singleton()->get_joy_name(ejm->get_device()));
		next_device_index = ejm->get_device();
	} else {
		return;
	}

	// Track last known joypad even when switching away.
	if (next_device != DEVICE_KEYBOARD) {
		last_known_joypad_device = next_device;
		last_known_joypad_index = next_device_index;
	}

	// Debounce — some Windows drivers register a press twice across two devices.
	int idle_frames = Engine::get_singleton()->get_idle_frames();
	int fps = Engine::get_singleton()->get_frames_per_second();
	bool not_changed_just_then = (idle_frames - device_last_changed_at) > fps;

	if (next_device != device || (next_device_index != device_index && not_changed_just_then)) {
		device_last_changed_at = idle_frames;
		device = next_device;
		device_index = next_device_index;
		emit_signal("device_changed", device, device_index);
	}
}

String InputHelper::get_device_from_event(const Ref<InputEvent> &p_event) const {
	if (p_event.is_null()) {
		return DEVICE_KEYBOARD;
	}

	Ref<InputEventKey> ek = p_event;
	Ref<InputEventMouseButton> emb = p_event;
	Ref<InputEventMouseMotion> emm = p_event;
	Ref<InputEventJoypadButton> ejb = p_event;
	Ref<InputEventJoypadMotion> ejm = p_event;

	if (ek.is_valid() || emb.is_valid() || emm.is_valid()) {
		return DEVICE_KEYBOARD;
	}
	if (ejb.is_valid()) {
		return get_simplified_device_name(Input::get_singleton()->get_joy_name(ejb->get_device()));
	}
	if (ejm.is_valid()) {
		return get_simplified_device_name(Input::get_singleton()->get_joy_name(ejm->get_device()));
	}
	return DEVICE_KEYBOARD;
}

String InputHelper::get_simplified_device_name(const String &p_raw) const {
	if (p_raw == "XInput Gamepad" || p_raw == "Xbox Series Controller" ||
			p_raw == "Xbox One Controller" || p_raw.findn("xbox") != -1) {
		return DEVICE_XBOX;
	}
	if (p_raw == "Sony DualSense" || p_raw == "PS5 Controller" || p_raw == "PS4 Controller" ||
			p_raw == "PS3 Controller" || p_raw.findn("dualshock") != -1 || p_raw.findn("dualsense") != -1) {
		return DEVICE_PLAYSTATION;
	}
	if (p_raw == "Switch" || p_raw.findn("switch") != -1 || p_raw.findn("joy-con") != -1 ||
			p_raw.findn("pro controller") != -1) {
		return DEVICE_SWITCH;
	}
	if (p_raw.findn("steam") != -1 || p_raw.findn("deck") != -1) {
		return DEVICE_STEAMDECK;
	}
	return DEVICE_GENERIC;
}

String InputHelper::get_granular_device_name(const String &p_raw) const {
	// Xbox sub-devices.
	if (p_raw.findn("Xbox One") != -1)
		return SUB_DEVICE_XBOX_ONE;
	if (p_raw.findn("Xbox Series") != -1 || p_raw.findn("Xbox Wireless") != -1)
		return SUB_DEVICE_XBOX_SERIES;
	if (p_raw == "XInput Gamepad" || p_raw.findn("xbox") != -1 || p_raw.findn("xinput") != -1)
		return DEVICE_XBOX;

	// PlayStation sub-devices.
	if (p_raw.findn("PS3") != -1)
		return SUB_DEVICE_PS3;
	if (p_raw.findn("PS4") != -1 || p_raw.findn("DUALSHOCK 4") != -1 || p_raw.findn("Nacon Revolution") != -1)
		return SUB_DEVICE_PS4;
	if (p_raw.findn("PS5") != -1 || p_raw.findn("DualSense") != -1)
		return SUB_DEVICE_PS5;

	// Steam Deck.
	if (p_raw.findn("steam") != -1 || p_raw.findn("deck") != -1)
		return DEVICE_STEAMDECK;

	// Switch sub-devices.
	if (p_raw.findn("Joy-Con (L)") != -1)
		return SUB_DEVICE_SWITCH_JOYCON_LEFT;
	if (p_raw.findn("Joy-Con (R)") != -1)
		return SUB_DEVICE_SWITCH_JOYCON_RIGHT;
	if (p_raw.findn("switch") != -1 || p_raw.findn("joy-con") != -1 || p_raw.findn("pro controller") != -1)
		return DEVICE_SWITCH;

	return DEVICE_GENERIC;
}

String InputHelper::guess_device_name() const {
	Input *in = Input::get_singleton();
	Array pads = in->get_connected_joypads();
	if (pads.size() == 0) {
		return DEVICE_KEYBOARD;
	}
	return get_simplified_device_name(in->get_joy_name(0));
}

bool InputHelper::has_gamepad() const {
	return Input::get_singleton()->get_connected_joypads().size() > 0;
}

String InputHelper::get_action_key(const String &p_action) const {
	if (!InputMap::get_singleton()->has_action(p_action)) {
		return String();
	}
	const List<Ref<InputEvent>> *events = InputMap::get_singleton()->get_action_list(p_action);
	if (!events) {
		return String();
	}
	for (const List<Ref<InputEvent>>::Element *E = events->front(); E; E = E->next()) {
		Ref<InputEventKey> k = E->get();
		if (k.is_valid()) {
			return keycode_get_string(k->get_scancode());
		}
	}
	return String();
}

int InputHelper::get_action_button(const String &p_action) const {
	if (!InputMap::get_singleton()->has_action(p_action)) {
		return -1;
	}
	const List<Ref<InputEvent>> *events = InputMap::get_singleton()->get_action_list(p_action);
	if (!events) {
		return -1;
	}
	for (const List<Ref<InputEvent>>::Element *E = events->front(); E; E = E->next()) {
		Ref<InputEventJoypadButton> b = E->get();
		if (b.is_valid()) {
			return b->get_button_index();
		}
	}
	return -1;
}

String InputHelper::get_gamepad_button_label(const String &p_device, int p_joy_button) const {
	// Full 16-button label arrays per device family (matching upstream).
	// Indices 0-3: face, 4-5: shoulder, 6-7: trigger, 8-9: select/start,
	// 10-11: stick click, 12-15: dpad.
	static const char *xbox_labels[16] = {
		"A", "B", "X", "Y", "LB", "RB", "LT", "RT",
		"Back", "Start", "L3", "R3", "DPad Up", "DPad Down", "DPad Left", "DPad Right"
	};
	static const char *ps_labels[16] = {
		"Cross", "Circle", "Square", "Triangle", "L1", "R1", "L2", "R2",
		"Share", "Options", "L3", "R3", "DPad Up", "DPad Down", "DPad Left", "DPad Right"
	};
	static const char *switch_labels[16] = {
		"B", "A", "Y", "X", "L", "R", "ZL", "ZR",
		"-", "+", "L3", "R3", "DPad Up", "DPad Down", "DPad Left", "DPad Right"
	};
	static const char *steamdeck_labels[16] = {
		"A", "B", "X", "Y", "L1", "R1", "L2", "R2",
		"View", "Menu", "L3", "R3", "DPad Up", "DPad Down", "DPad Left", "DPad Right"
	};

	if (p_joy_button >= 0 && p_joy_button < 16) {
		if (p_device == DEVICE_PLAYSTATION) {
			return ps_labels[p_joy_button];
		}
		if (p_device == DEVICE_SWITCH) {
			return switch_labels[p_joy_button];
		}
		if (p_device == DEVICE_STEAMDECK) {
			return steamdeck_labels[p_joy_button];
		}
		return xbox_labels[p_joy_button]; // xbox + generic
	}
	return vformat("Btn%d", p_joy_button);
}

String InputHelper::get_gamepad_axis_label(int p_axis, float p_value) const {
	switch (p_axis) {
		case 0: // JOY_AXIS_LEFT_X
			return p_value < 0 ? "Left Stick Left" : "Left Stick Right";
		case 1: // JOY_AXIS_LEFT_Y
			return p_value < 0 ? "Left Stick Up" : "Left Stick Down";
		case 2: // JOY_AXIS_RIGHT_X
			return p_value < 0 ? "Right Stick Left" : "Right Stick Right";
		case 3: // JOY_AXIS_RIGHT_Y
			return p_value < 0 ? "Right Stick Up" : "Right Stick Down";
		case 6: // JOY_AXIS_TRIGGER_LEFT
			return "Left Trigger";
		case 7: // JOY_AXIS_TRIGGER_RIGHT
			return "Right Trigger";
		default:
			return vformat("Axis %d", p_axis);
	}
}

String InputHelper::get_label_for_input(const Ref<InputEvent> &p_event) const {
	if (p_event.is_null()) {
		return "";
	}

	Ref<InputEventKey> ek = p_event;
	if (ek.is_valid()) {
		int sc = ek->get_scancode();
		return sc != 0 ? keycode_get_string(sc) : ek->as_text();
	}

	Ref<InputEventMouseButton> emb = p_event;
	if (emb.is_valid()) {
		switch (emb->get_button_index()) {
			case 1:
				return "Mouse Left Button";
			case 2:
				return "Mouse Right Button";
			case 3:
				return "Mouse Middle Button";
			default:
				return vformat("Mouse Button %d", emb->get_button_index());
		}
	}

	Ref<InputEventJoypadButton> ejb = p_event;
	if (ejb.is_valid()) {
		String dev = last_known_joypad_device.empty() ? String(DEVICE_GENERIC) : last_known_joypad_device;
		return get_gamepad_button_label(dev, ejb->get_button_index());
	}

	Ref<InputEventJoypadMotion> ejm = p_event;
	if (ejm.is_valid()) {
		return get_gamepad_axis_label(ejm->get_axis(), ejm->get_axis_value());
	}

	return p_event->as_text();
}

String InputHelper::get_label(int p_scancode, int p_joy_button) const {
	if (is_keyboard()) {
		return keycode_get_string(p_scancode);
	}
	return get_gamepad_button_label(device, p_joy_button);
}

// --- Input remapping (from upstream v2.x) ---

Error InputHelper::set_action_key(const String &p_action, const String &p_key, bool p_swap_if_taken) {
	if (!InputMap::get_singleton()->has_action(p_action)) {
		return ERR_DOES_NOT_EXIST;
	}

	int new_scancode = find_keycode(p_key);
	if (new_scancode == 0) {
		return ERR_INVALID_DATA;
	}

	// Find clashing action.
	String clashing_action;
	Ref<InputEventKey> clashing_event;
	if (p_swap_if_taken) {
		List<StringName> actions;
		actions = InputMap::get_singleton()->get_actions();
		for (List<StringName>::Element *A = actions.front(); A; A = A->next()) {
			const List<Ref<InputEvent>> *events = InputMap::get_singleton()->get_action_list(A->get());
			if (!events)
				continue;
			for (const List<Ref<InputEvent>>::Element *E = events->front(); E; E = E->next()) {
				Ref<InputEventKey> k = E->get();
				if (k.is_valid() && k->get_scancode() == (uint32_t)new_scancode) {
					clashing_action = A->get();
					clashing_event = k;
					break;
				}
			}
			if (!clashing_action.empty())
				break;
		}
	}

	// Remove current key from target action, optionally swap.
	const List<Ref<InputEvent>> *events = InputMap::get_singleton()->get_action_list(p_action);
	if (events) {
		for (const List<Ref<InputEvent>>::Element *E = events->front(); E; E = E->next()) {
			Ref<InputEventKey> k = E->get();
			if (k.is_valid()) {
				if (!clashing_action.empty() && clashing_event.is_valid()) {
					InputMap::get_singleton()->action_erase_event(clashing_action, clashing_event);
					InputMap::get_singleton()->action_add_event(clashing_action, k);
					emit_signal("action_key_changed", clashing_action, keycode_get_string(k->get_scancode()));
				}
				InputMap::get_singleton()->action_erase_event(p_action, k);
				break;
			}
		}
	}

	// Add new key.
	Ref<InputEventKey> new_event;
	new_event.instance();
	new_event->set_scancode(new_scancode);
	InputMap::get_singleton()->action_add_event(p_action, new_event);
	emit_signal("action_key_changed", p_action, p_key);
	return OK;
}

Error InputHelper::set_action_button(const String &p_action, int p_button, bool p_swap_if_taken) {
	if (!InputMap::get_singleton()->has_action(p_action)) {
		return ERR_DOES_NOT_EXIST;
	}

	// Find clashing action.
	String clashing_action;
	Ref<InputEventJoypadButton> clashing_event;
	if (p_swap_if_taken) {
		List<StringName> actions;
		actions = InputMap::get_singleton()->get_actions();
		for (List<StringName>::Element *A = actions.front(); A; A = A->next()) {
			const List<Ref<InputEvent>> *events = InputMap::get_singleton()->get_action_list(A->get());
			if (!events)
				continue;
			for (const List<Ref<InputEvent>>::Element *E = events->front(); E; E = E->next()) {
				Ref<InputEventJoypadButton> b = E->get();
				if (b.is_valid() && b->get_button_index() == p_button) {
					clashing_action = A->get();
					clashing_event = b;
					break;
				}
			}
			if (!clashing_action.empty())
				break;
		}
	}

	// Remove current button from target action, optionally swap.
	const List<Ref<InputEvent>> *events = InputMap::get_singleton()->get_action_list(p_action);
	if (events) {
		for (const List<Ref<InputEvent>>::Element *E = events->front(); E; E = E->next()) {
			Ref<InputEventJoypadButton> b = E->get();
			if (b.is_valid()) {
				if (!clashing_action.empty() && clashing_event.is_valid()) {
					InputMap::get_singleton()->action_erase_event(clashing_action, clashing_event);
					InputMap::get_singleton()->action_add_event(clashing_action, b);
					emit_signal("action_button_changed", clashing_action, b->get_button_index());
				}
				InputMap::get_singleton()->action_erase_event(p_action, b);
				break;
			}
		}
	}

	// Add new button.
	Ref<InputEventJoypadButton> new_event;
	new_event.instance();
	new_event->set_button_index(p_button);
	InputMap::get_singleton()->action_add_event(p_action, new_event);
	emit_signal("action_button_changed", p_action, p_button);
	return OK;
}

void InputHelper::reset_all_actions() {
	InputMap::get_singleton()->load_from_globals();
	List<StringName> actions;
	actions = InputMap::get_singleton()->get_actions();
	for (List<StringName>::Element *A = actions.front(); A; A = A->next()) {
		emit_signal("action_button_changed", String(A->get()), get_action_button(A->get()));
		emit_signal("action_key_changed", String(A->get()), get_action_key(A->get()));
	}
}

// --- Rumble helpers ---

void InputHelper::rumble_small(int p_device) {
	Input::get_singleton()->start_joy_vibration(p_device, 0.4f, 0.0f, 0.1f);
}

void InputHelper::rumble_medium(int p_device) {
	Input::get_singleton()->start_joy_vibration(p_device, 0.0f, 0.7f, 0.1f);
}

void InputHelper::rumble_large(int p_device) {
	Input::get_singleton()->start_joy_vibration(p_device, 0.0f, 1.0f, 0.1f);
}

void InputHelper::stop_rumble(int p_device) {
	Input::get_singleton()->stop_joy_vibration(p_device);
}

void InputHelper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("update_from_event", "event"), &InputHelper::update_from_event);
	ClassDB::bind_method(D_METHOD("get_device_from_event", "event"), &InputHelper::get_device_from_event);
	ClassDB::bind_method(D_METHOD("get_simplified_device_name", "raw_name"), &InputHelper::get_simplified_device_name);
	ClassDB::bind_method(D_METHOD("guess_device_name"), &InputHelper::guess_device_name);
	ClassDB::bind_method(D_METHOD("has_gamepad"), &InputHelper::has_gamepad);
	ClassDB::bind_method(D_METHOD("is_keyboard"), &InputHelper::is_keyboard);
	ClassDB::bind_method(D_METHOD("is_gamepad"), &InputHelper::is_gamepad);
	ClassDB::bind_method(D_METHOD("get_device"), &InputHelper::get_device);
	ClassDB::bind_method(D_METHOD("get_device_index"), &InputHelper::get_device_index);
	ClassDB::bind_method(D_METHOD("get_last_known_joypad_device"), &InputHelper::get_last_known_joypad_device);
	ClassDB::bind_method(D_METHOD("get_last_known_joypad_index"), &InputHelper::get_last_known_joypad_index);
	ClassDB::bind_method(D_METHOD("set_deadzone", "deadzone"), &InputHelper::set_deadzone);
	ClassDB::bind_method(D_METHOD("get_deadzone"), &InputHelper::get_deadzone);
	ClassDB::bind_method(D_METHOD("set_mouse_motion_threshold", "threshold"), &InputHelper::set_mouse_motion_threshold);
	ClassDB::bind_method(D_METHOD("get_mouse_motion_threshold"), &InputHelper::get_mouse_motion_threshold);
	ClassDB::bind_method(D_METHOD("get_action_key", "action"), &InputHelper::get_action_key);
	ClassDB::bind_method(D_METHOD("get_action_button", "action"), &InputHelper::get_action_button);
	ClassDB::bind_method(D_METHOD("get_label", "scancode", "joy_button"), &InputHelper::get_label);
	ClassDB::bind_method(D_METHOD("get_label_for_input", "event"), &InputHelper::get_label_for_input);
	ClassDB::bind_method(D_METHOD("get_gamepad_button_label", "device", "joy_button"), &InputHelper::get_gamepad_button_label);
	ClassDB::bind_method(D_METHOD("get_gamepad_axis_label", "axis", "value"), &InputHelper::get_gamepad_axis_label);
	ClassDB::bind_method(D_METHOD("set_action_key", "action", "key", "swap_if_taken"), &InputHelper::set_action_key, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("set_action_button", "action", "button", "swap_if_taken"), &InputHelper::set_action_button, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("reset_all_actions"), &InputHelper::reset_all_actions);
	ClassDB::bind_method(D_METHOD("rumble_small", "device"), &InputHelper::rumble_small, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("rumble_medium", "device"), &InputHelper::rumble_medium, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("rumble_large", "device"), &InputHelper::rumble_large, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("stop_rumble", "device"), &InputHelper::stop_rumble, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("_on_joy_connection_changed", "index", "connected"), &InputHelper::_on_joy_connection_changed);

	ADD_PROPERTY(PropertyInfo(Variant::REAL, "deadzone"), "set_deadzone", "get_deadzone");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "mouse_motion_threshold"), "set_mouse_motion_threshold", "get_mouse_motion_threshold");

	ADD_SIGNAL(MethodInfo("device_changed", PropertyInfo(Variant::STRING, "device"), PropertyInfo(Variant::INT, "device_index")));
	ADD_SIGNAL(MethodInfo("joypad_changed", PropertyInfo(Variant::INT, "index"), PropertyInfo(Variant::BOOL, "connected")));
	ADD_SIGNAL(MethodInfo("action_key_changed", PropertyInfo(Variant::STRING, "action"), PropertyInfo(Variant::STRING, "key")));
	ADD_SIGNAL(MethodInfo("action_button_changed", PropertyInfo(Variant::STRING, "action"), PropertyInfo(Variant::INT, "button")));
}

#ifdef DOCTEST
TEST_CASE("[InputHelper] device-name simplification") {
	InputHelper h;
	CHECK(h.get_simplified_device_name("XInput Gamepad") == "xbox");
	CHECK(h.get_simplified_device_name("Xbox Series Controller") == "xbox");
	CHECK(h.get_simplified_device_name("Xbox One Controller") == "xbox");
	CHECK(h.get_simplified_device_name("PS4 Controller") == "playstation");
	CHECK(h.get_simplified_device_name("Sony DualSense") == "playstation");
	CHECK(h.get_simplified_device_name("Switch") == "switch");
	CHECK(h.get_simplified_device_name("Nintendo Switch Pro Controller") == "switch");
	CHECK(h.get_simplified_device_name("Steam Deck") == "steamdeck");
	CHECK(h.get_simplified_device_name("Some Random Pad") == "generic");
}

TEST_CASE("[InputHelper] gamepad button labels per platform") {
	InputHelper h;
	CHECK(h.get_gamepad_button_label("xbox", 0) == "A");
	CHECK(h.get_gamepad_button_label("xbox", 1) == "B");
	CHECK(h.get_gamepad_button_label("playstation", 0) == "Cross");
	CHECK(h.get_gamepad_button_label("playstation", 1) == "Circle");
	CHECK(h.get_gamepad_button_label("switch", 0) == "B"); // Nintendo swap
	CHECK(h.get_gamepad_button_label("switch", 1) == "A");
	CHECK(h.get_gamepad_button_label("steamdeck", 0) == "A");
	CHECK(h.get_gamepad_button_label("steamdeck", 4) == "L1");
	CHECK(h.get_gamepad_button_label("generic", 99) == "Btn99");
	CHECK(h.get_gamepad_button_label("xbox", 4) == "LB");
	CHECK(h.get_gamepad_button_label("playstation", 4) == "L1");
}

TEST_CASE("[InputHelper] axis labels") {
	InputHelper h;
	CHECK(h.get_gamepad_axis_label(0, -1.0) == "Left Stick Left");
	CHECK(h.get_gamepad_axis_label(0, 1.0) == "Left Stick Right");
	CHECK(h.get_gamepad_axis_label(1, -1.0) == "Left Stick Up");
	CHECK(h.get_gamepad_axis_label(1, 1.0) == "Left Stick Down");
	CHECK(h.get_gamepad_axis_label(6, 1.0) == "Left Trigger");
	CHECK(h.get_gamepad_axis_label(7, 1.0) == "Right Trigger");
}
#endif
