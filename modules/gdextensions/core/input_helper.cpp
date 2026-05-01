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

static const char *DEVICE_KEYBOARD = "keyboard";
static const char *DEVICE_XBOX = "xbox";
static const char *DEVICE_PLAYSTATION = "playstation";
static const char *DEVICE_SWITCH = "switch";
static const char *DEVICE_GENERIC = "generic";

InputHelper *InputHelper::singleton = nullptr;

InputHelper::InputHelper() {
	singleton = this;
	device = DEVICE_GENERIC;
	device_index = -1;
	device_last_changed_at = 0;
	deadzone = 0.5f;
}

InputHelper::~InputHelper() {
	singleton = nullptr;
}

void InputHelper::update_from_event(const Ref<InputEvent> &p_event) {
	if (p_event.is_null()) {
		return;
	}

	String next_device = device;
	int next_device_index = device_index;

	Ref<InputEventKey> ek = p_event;
	Ref<InputEventJoypadButton> ejb = p_event;
	Ref<InputEventJoypadMotion> ejm = p_event;

	if (ek.is_valid() && ek->is_pressed()) {
		next_device = DEVICE_KEYBOARD;
		next_device_index = -1;
	} else if (ejb.is_valid() && ejb->is_pressed()) {
		next_device = get_simplified_device_name(Input::get_singleton()->get_joy_name(ejb->get_device()));
		next_device_index = ejb->get_device();
	} else if (ejm.is_valid() && Math::abs(ejm->get_axis_value()) > deadzone) {
		next_device = get_simplified_device_name(Input::get_singleton()->get_joy_name(ejm->get_device()));
		next_device_index = ejm->get_device();
	} else {
		return;
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

String InputHelper::get_simplified_device_name(const String &p_raw) const {
	if (p_raw == "XInput Gamepad" || p_raw == "Xbox Series Controller" || p_raw.findn("xbox") != -1) {
		return DEVICE_XBOX;
	}
	if (p_raw == "Sony DualSense" || p_raw == "PS5 Controller" || p_raw == "PS4 Controller" ||
			p_raw == "PS3 Controller" || p_raw.findn("dualshock") != -1 || p_raw.findn("dualsense") != -1) {
		return DEVICE_PLAYSTATION;
	}
	if (p_raw == "Switch" || p_raw.findn("switch") != -1 || p_raw.findn("joy-con") != -1) {
		return DEVICE_SWITCH;
	}
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
	// Map JOY_XBOX_* button indices (0..3 face buttons + shoulders/menu) to
	// the labels printed on each platform's controller.
	static const char *xbox_face[4] = { "A", "B", "X", "Y" };
	static const char *ps_face[4] = { "Cross", "Circle", "Square", "Triangle" };
	static const char *switch_face[4] = { "B", "A", "Y", "X" }; // Nintendo swaps A/B and X/Y.

	if (p_joy_button >= 0 && p_joy_button < 4) {
		if (p_device == DEVICE_PLAYSTATION) {
			return ps_face[p_joy_button];
		}
		if (p_device == DEVICE_SWITCH) {
			return switch_face[p_joy_button];
		}
		return xbox_face[p_joy_button]; // xbox + generic
	}
	switch (p_joy_button) {
		case 4:
			return p_device == DEVICE_PLAYSTATION ? "L1" : "LB";
		case 5:
			return p_device == DEVICE_PLAYSTATION ? "R1" : "RB";
		case 6:
			return p_device == DEVICE_PLAYSTATION ? "L2" : "LT";
		case 7:
			return p_device == DEVICE_PLAYSTATION ? "R2" : "RT";
		case 8:
			return "Select";
		case 9:
			return "Start";
		case 10:
			return "L3";
		case 11:
			return "R3";
		case 12:
			return "DPad Up";
		case 13:
			return "DPad Down";
		case 14:
			return "DPad Left";
		case 15:
			return "DPad Right";
		default:
			return vformat("Btn%d", p_joy_button);
	}
}

String InputHelper::get_label(int p_scancode, int p_joy_button) const {
	if (is_keyboard()) {
		return keycode_get_string(p_scancode);
	}
	return get_gamepad_button_label(device, p_joy_button);
}

void InputHelper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("update_from_event", "event"), &InputHelper::update_from_event);
	ClassDB::bind_method(D_METHOD("get_simplified_device_name", "raw_name"), &InputHelper::get_simplified_device_name);
	ClassDB::bind_method(D_METHOD("guess_device_name"), &InputHelper::guess_device_name);
	ClassDB::bind_method(D_METHOD("has_gamepad"), &InputHelper::has_gamepad);
	ClassDB::bind_method(D_METHOD("is_keyboard"), &InputHelper::is_keyboard);
	ClassDB::bind_method(D_METHOD("is_gamepad"), &InputHelper::is_gamepad);
	ClassDB::bind_method(D_METHOD("get_device"), &InputHelper::get_device);
	ClassDB::bind_method(D_METHOD("get_device_index"), &InputHelper::get_device_index);
	ClassDB::bind_method(D_METHOD("set_deadzone", "deadzone"), &InputHelper::set_deadzone);
	ClassDB::bind_method(D_METHOD("get_deadzone"), &InputHelper::get_deadzone);
	ClassDB::bind_method(D_METHOD("get_action_key", "action"), &InputHelper::get_action_key);
	ClassDB::bind_method(D_METHOD("get_action_button", "action"), &InputHelper::get_action_button);
	ClassDB::bind_method(D_METHOD("get_label", "scancode", "joy_button"), &InputHelper::get_label);
	ClassDB::bind_method(D_METHOD("get_gamepad_button_label", "device", "joy_button"), &InputHelper::get_gamepad_button_label);

	BIND_CONSTANT(0); // placeholder; constants exposed via ProjectSettings would clutter — use string literals.

	ADD_SIGNAL(MethodInfo("device_changed", PropertyInfo(Variant::STRING, "device"), PropertyInfo(Variant::INT, "device_index")));
}

#ifdef DOCTEST
TEST_CASE("[InputHelper] device-name simplification") {
	InputHelper h;
	CHECK(h.get_simplified_device_name("XInput Gamepad") == "xbox");
	CHECK(h.get_simplified_device_name("Xbox Series Controller") == "xbox");
	CHECK(h.get_simplified_device_name("PS4 Controller") == "playstation");
	CHECK(h.get_simplified_device_name("Sony DualSense") == "playstation");
	CHECK(h.get_simplified_device_name("Switch") == "switch");
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
	CHECK(h.get_gamepad_button_label("generic", 99) == "Btn99");
	CHECK(h.get_gamepad_button_label("xbox", 4) == "LB");
	CHECK(h.get_gamepad_button_label("playstation", 4) == "L1");
}
#endif
