/**************************************************************************/
/*  runtime_profiler_overlay.cpp                                          */
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

#include "runtime_profiler_overlay.h"

#include "runtime_profiler.h"

#include "core/input_map.h"
#include "core/os/input.h"
#include "core/os/input_event.h"
#include "core/os/keyboard.h"
#include "core/project_settings.h"
#include "scene/gui/panel_container.h"
#include "scene/main/canvas_layer.h"
#include "scene/main/scene_tree.h"
#include "scene/main/viewport.h"

RuntimeProfilerOverlay *RuntimeProfilerOverlay::singleton = nullptr;

void RuntimeProfilerOverlay::_setup_input_action() {
	const String action_name = GLOBAL_GET("runtime_profiler/overlay_toggle_action");

	if (InputMap::get_singleton()->has_action(action_name)) {
		return;
	}

	InputMap::get_singleton()->add_action(action_name);

	// Keyboard: Ctrl+Shift+F12
	{
		Ref<InputEventKey> key;
		key.instance();
		key->set_scancode(KEY_F12);
		key->set_control(true);
		key->set_shift(true);
		InputMap::get_singleton()->action_add_event(action_name, key);
	}

	// Also add gamepad combo as an action:
	// L1 + R1 are checked manually; Select triggers the action.
	// This allows the user to override via project input map.
	{
		Ref<InputEventJoypadButton> btn;
		btn.instance();
		btn->set_button_index(JOY_SELECT);
		InputMap::get_singleton()->action_add_event(action_name, btn);
	}
}

bool RuntimeProfilerOverlay::_init_overlay() {
	SceneTree *st = SceneTree::get_singleton();
	ERR_FAIL_NULL_V(st, false);
	ERR_FAIL_NULL_V(st->get_root(), false);

	_setup_input_action();

	// CanvasLayer at very high layer to render above everything
	canvas_layer = memnew(CanvasLayer);
	canvas_layer->set_layer(128);
	canvas_layer->set_name("__RuntimeProfilerOverlay");
	st->get_root()->call_deferred("add_child", canvas_layer);

	// Semi-transparent panel occupying bottom 55% of screen
	panel = memnew(PanelContainer);
	panel->set_anchor(MARGIN_LEFT, 0.0);
	panel->set_anchor(MARGIN_RIGHT, 1.0);
	panel->set_anchor(MARGIN_TOP, 0.45);
	panel->set_anchor(MARGIN_BOTTOM, 1.0);
	panel->set_margin(MARGIN_LEFT, 4);
	panel->set_margin(MARGIN_RIGHT, -4);
	panel->set_margin(MARGIN_BOTTOM, -4);
	panel->set_self_modulate(Color(1, 1, 1, 0.92));
	canvas_layer->call_deferred("add_child", panel);

	// The profiler widget
	profiler = memnew(RuntimeProfiler);
	panel->call_deferred("add_child", profiler);

	panel->set_visible(false);

	// Poll input on each idle frame
	st->connect("idle_frame", this, "_idle_frame");

	initialized = true;
	return true;
}

void RuntimeProfilerOverlay::_idle_frame() {
	if (!enabled) {
		return;
	}

	Input *input = Input::get_singleton();
	if (!input) {
		return;
	}

	const String action_name = GLOBAL_GET("runtime_profiler/overlay_toggle_action");
	if (action_name.empty()) {
		return;
	}

	bool action_pressed = input->is_action_pressed(action_name);

	// For gamepad: require L1+R1 held when Select is pressed.
	// For keyboard: the action itself (Ctrl+Shift+F12) is sufficient.
	// We detect the keyboard case by checking if L1/R1 are irrelevant.
	bool pad_l1 = input->is_joy_button_pressed(0, JOY_L);
	bool pad_r1 = input->is_joy_button_pressed(0, JOY_R);
	bool pad_select = input->is_joy_button_pressed(0, JOY_SELECT);

	// Gamepad combo: all three shoulder+select held
	bool pad_combo_active = pad_l1 && pad_r1 && pad_select;

	// Detect rising edge of keyboard action (not gamepad select alone)
	bool keyboard_trigger = action_pressed && !action_was_pressed && !pad_select;

	// Detect rising edge of gamepad combo
	bool pad_trigger = pad_combo_active && !pad_combo_was_active;

	if (keyboard_trigger || pad_trigger) {
		_toggle_overlay();
	}

	action_was_pressed = action_pressed;
	pad_combo_was_active = pad_combo_active;
}

void RuntimeProfilerOverlay::_toggle_overlay() {
	if (overlay_visible) {
		hide_overlay();
	} else {
		show_overlay();
	}
}

void RuntimeProfilerOverlay::show_overlay() {
	if (!initialized && !_init_overlay()) {
		return;
	}

	panel->set_visible(true);
	overlay_visible = true;

	bool auto_start = GLOBAL_GET("runtime_profiler/overlay_auto_start");
	if (auto_start && profiler && !profiler->is_profiling()) {
		profiler->start_profiling();
	}

	print_verbose("RuntimeProfilerOverlay: shown");
}

void RuntimeProfilerOverlay::hide_overlay() {
	if (!initialized) {
		return;
	}

	// Stop profiling when hiding to avoid overhead
	if (profiler && profiler->is_profiling()) {
		profiler->stop_profiling();
	}

	panel->set_visible(false);
	overlay_visible = false;

	print_verbose("RuntimeProfilerOverlay: hidden");
}

void RuntimeProfilerOverlay::_deferred_init() {
	if (enabled && !initialized) {
		_init_overlay();
	}
}

bool RuntimeProfilerOverlay::_ensure_initialized() {
	if (!initialized) {
		if (!_init_overlay()) {
			return false;
		}
	}
	return true;
}

void RuntimeProfilerOverlay::toggle_overlay() {
	if (!_ensure_initialized()) {
		return;
	}
	_toggle_overlay();
}

bool RuntimeProfilerOverlay::is_overlay_visible() const {
	return overlay_visible;
}

void RuntimeProfilerOverlay::set_enabled(bool p_enabled) {
	enabled = p_enabled;
	if (!p_enabled && overlay_visible) {
		hide_overlay();
	}
}

bool RuntimeProfilerOverlay::is_enabled() const {
	return enabled;
}

RuntimeProfiler *RuntimeProfilerOverlay::get_profiler() const {
	return profiler;
}

RuntimeProfilerOverlay *RuntimeProfilerOverlay::get_singleton() {
	return singleton;
}

void RuntimeProfilerOverlay::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_idle_frame"), &RuntimeProfilerOverlay::_idle_frame);
	ClassDB::bind_method(D_METHOD("_deferred_init"), &RuntimeProfilerOverlay::_deferred_init);

	ClassDB::bind_method(D_METHOD("show_overlay"), &RuntimeProfilerOverlay::show_overlay);
	ClassDB::bind_method(D_METHOD("hide_overlay"), &RuntimeProfilerOverlay::hide_overlay);
	ClassDB::bind_method(D_METHOD("toggle_overlay"), &RuntimeProfilerOverlay::toggle_overlay);
	ClassDB::bind_method(D_METHOD("is_overlay_visible"), &RuntimeProfilerOverlay::is_overlay_visible);
	ClassDB::bind_method(D_METHOD("set_enabled", "enabled"), &RuntimeProfilerOverlay::set_enabled);
	ClassDB::bind_method(D_METHOD("is_enabled"), &RuntimeProfilerOverlay::is_enabled);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_enabled", "is_enabled");
}

RuntimeProfilerOverlay::RuntimeProfilerOverlay() {
	singleton = this;
	canvas_layer = nullptr;
	panel = nullptr;
	profiler = nullptr;
	overlay_visible = false;
	initialized = false;
	action_was_pressed = false;
	pad_combo_was_active = false;

#ifdef DEBUG_ENABLED
	enabled = bool(GLOBAL_DEF("runtime_profiler/overlay_enabled", true));
#else
	enabled = bool(GLOBAL_DEF("runtime_profiler/overlay_enabled", false));
#endif
	GLOBAL_DEF("runtime_profiler/overlay_toggle_action", "toggle_profiler_overlay");
	GLOBAL_DEF("runtime_profiler/overlay_auto_start", false);

	// Auto-initialize on next idle frame so keyboard shortcut works
	// without requiring an explicit toggle_overlay() call first.
	call_deferred("_deferred_init");
}

RuntimeProfilerOverlay::~RuntimeProfilerOverlay() {
	if (singleton == this) {
		singleton = nullptr;
	}
}
