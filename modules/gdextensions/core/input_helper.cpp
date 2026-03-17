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

#include "input_helper.h"

const String DEVICE_KEYBOARD = "keyboard";
const String DEVICE_XBOX_CONTROLLER = "xbox";
const String DEVICE_SWITCH_CONTROLLER = "switch";
const String DEVICE_PLAYSTATION_CONTROLLER = "playstation";
const String DEVICE_GENERIC = "generic";

static InputHelper *instance = nullptr;

InputHelper::InputHelper() {
	instance = this;
}

InputHelper::~InputHelper() {
	instance = nullptr;
}

// extends Node
//
// signal device_changed(device, device_index)
//
// const DEVICE_KEYBOARD = "keyboard"
// const DEVICE_XBOX_CONTROLLER = "xbox"
// const DEVICE_SWITCH_CONTROLLER = "switch"
// const DEVICE_PLAYSTATION_CONTROLLER = "playstation"
// const DEVICE_GENERIC = "generic"
//
// var device: String = DEVICE_GENERIC
// var device_index: int = -1
//
// func _input(event: InputEvent) -> void:
//   var next_device: String = device
//   var next_device_index: int = device_index
//
//   # Did we just press a key on the keyboard?
//   if event is InputEventKey and event.is_pressed():
//     next_device = DEVICE_KEYBOARD
//     next_device_index = -1
//
//   # Did we just use a gamepad?
//   elif (event is InputEventJoypadButton and event.is_pressed()) \
//     or (event is InputEventJoypadMotion and int(event.axis_value * 10) != 0):
//     next_device = get_simplified_device_name(Input.get_joy_name(event.device))
//     next_device_index = event.device
//
//   if next_device != device or next_device_index != device_index:
//     device = next_device
//     device_index = next_device_index
//     emit_signal("device_changed", device, device_index)
//
// func get_simplified_device_name(raw_name: String) -> String:
//   match raw_name:
//     "XInput Gamepad", "Xbox Series Controller":
//       return DEVICE_XBOX_CONTROLLER
//
//     "PS4 Controller", "PS3 Controller", "PS2 Controller":
//       return DEVICE_PLAYSTATION_CONTROLLER
//
//     "Switch":
//       return DEVICE_SWITCH_CONTROLLER
//
//     _:
//       return DEVICE_GENERIC
//
// func get_key_label(action_name: String) -> String:
//   var inputs = InputMap.get_action_list(action_name)
//   # Get the first key input
//   for input in inputs:
//     if input is InputEventKey:
//       return input.as_text()
//
//   return ""
//
// func get_button_index(action_name: String) -> int:
//   var inputs = InputMap.get_action_list(action_name)
//   # Get the first button input
//   for input in inputs:
//     if input is InputEventJoypadButton:
//       return input.button_index
//
//   return -1
