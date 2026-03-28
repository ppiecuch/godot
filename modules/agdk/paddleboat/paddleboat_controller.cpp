/**************************************************************************/
/*  paddleboat_controller.cpp                                             */
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

#include "paddleboat_controller.h"

#include "../agdk_manager.h"
#include "core/os/os.h"
#include "core/print_string.h"

#include <paddleboat/paddleboat.h>

// Maximum controllers supported by Paddleboat.
#define AGDK_MAX_CONTROLLERS PADDLEBOAT_MAX_CONTROLLERS

void PaddleboatController::_status_callback(const int32_t p_controller_index, const Paddleboat_ControllerStatus p_controller_status, void *p_user_data) {
	PaddleboatController *self = static_cast<PaddleboatController *>(p_user_data);
	if (!self) {
		return;
	}

	MutexLock lock(self->_mutex);

	ConnectionEvent event;
	event.index = p_controller_index;
	event.connected = (p_controller_status == PADDLEBOAT_CONTROLLER_JUST_CONNECTED);
	self->_pending_events.push_back(event);
}

Error PaddleboatController::init(JNIEnv *p_env, jobject p_context) {
	if (_initialized) {
		return OK;
	}

	Paddleboat_ErrorCode err = Paddleboat_init(p_env, p_context);
	if (err != PADDLEBOAT_NO_ERROR) {
		print_line("AGDK Paddleboat: Failed to initialize, error: " + itos(err));
		return ERR_CANT_CREATE;
	}

	Paddleboat_setControllerStatusCallback(_status_callback, this);

	_initialized = true;
	print_line("AGDK Paddleboat: Game controller library initialized.");
	return OK;
}

void PaddleboatController::update() {
	if (!_initialized) {
		return;
	}

	// Get JNIEnv for this thread.
	JNIEnv *env = nullptr;
	// Thread attachment is handled by the caller (GL thread has JNIEnv).
	// Paddleboat_update requires the current JNIEnv.
	JavaVM *vm = nullptr;
	// In practice, the JNI env is obtained from ThreadJAndroid.
	// For now, we call update without env — this needs the actual JNI env
	// to be passed from the JNI layer.
	// Paddleboat_update(env);

	// Process pending connection events.
	MutexLock lock(_mutex);
	for (int i = 0; i < _pending_events.size(); i++) {
		const ConnectionEvent &evt = _pending_events[i];
		AGDKManager *mgr = AGDKManager::get_singleton();
		if (mgr) {
			if (evt.connected) {
				Dictionary info = get_controller_info(evt.index);
				mgr->emit_signal("controller_connected", evt.index, info);
			} else {
				mgr->emit_signal("controller_disconnected", evt.index);
			}
		}
	}
	_pending_events.clear();
}

void PaddleboatController::destroy() {
	if (!_initialized) {
		return;
	}

	Paddleboat_setControllerStatusCallback(nullptr, nullptr);

	JNIEnv *env = nullptr;
	// Paddleboat_destroy(env);

	_initialized = false;
	print_line("AGDK Paddleboat: Controller library destroyed.");
}

void PaddleboatController::on_pause() {
	if (!_initialized) {
		return;
	}
	JNIEnv *env = nullptr;
	// Paddleboat_onStop(env);
}

void PaddleboatController::on_resume() {
	if (!_initialized) {
		return;
	}
	JNIEnv *env = nullptr;
	// Paddleboat_onStart(env);
}

int PaddleboatController::get_controller_count() const {
	if (!_initialized) {
		return 0;
	}

	int count = 0;
	for (int i = 0; i < AGDK_MAX_CONTROLLERS; i++) {
		Paddleboat_ControllerStatus status = Paddleboat_getControllerStatus(i);
		if (status == PADDLEBOAT_CONTROLLER_ACTIVE) {
			count++;
		}
	}
	return count;
}

Dictionary PaddleboatController::get_controller_info(int p_index) const {
	Dictionary info;
	if (!_initialized || p_index < 0 || p_index >= AGDK_MAX_CONTROLLERS) {
		return info;
	}

	Paddleboat_Controller_Info pb_info;
	Paddleboat_ErrorCode err = Paddleboat_getControllerInfo(p_index, &pb_info);
	if (err != PADDLEBOAT_NO_ERROR) {
		return info;
	}

	info["vendor_id"] = pb_info.vendorId;
	info["product_id"] = pb_info.productId;
	info["device_id"] = pb_info.deviceId;
	info["controller_number"] = pb_info.controllerNumber;

	// Button layout.
	int layout = pb_info.controllerFlags & PADDLEBOAT_CONTROLLER_LAYOUT_MASK;
	switch (layout) {
		case PADDLEBOAT_CONTROLLER_LAYOUT_STANDARD:
			info["layout"] = "STANDARD";
			break;
		case PADDLEBOAT_CONTROLLER_LAYOUT_SHAPES:
			info["layout"] = "SHAPES";
			break;
		case PADDLEBOAT_CONTROLLER_LAYOUT_REVERSE:
			info["layout"] = "REVERSE";
			break;
		case PADDLEBOAT_CONTROLLER_LAYOUT_ARCADE_STICK:
			info["layout"] = "ARCADE_STICK";
			break;
		default:
			info["layout"] = "UNKNOWN";
			break;
	}

	info["has_touchpad"] = (pb_info.controllerFlags & PADDLEBOAT_CONTROLLER_FLAG_TOUCHPAD) != 0;
	info["has_virtual_mouse"] = (pb_info.controllerFlags & PADDLEBOAT_CONTROLLER_FLAG_VIRTUAL_MOUSE) != 0;

	// Controller name.
	char name_buf[256];
	if (Paddleboat_getControllerName(p_index, sizeof(name_buf), name_buf) == PADDLEBOAT_NO_ERROR) {
		info["name"] = String::utf8(name_buf);
	}

	return info;
}

Dictionary PaddleboatController::get_controller_data(int p_index) const {
	Dictionary data;
	if (!_initialized || p_index < 0 || p_index >= AGDK_MAX_CONTROLLERS) {
		return data;
	}

	Paddleboat_Controller_Data pb_data;
	Paddleboat_ErrorCode err = Paddleboat_getControllerData(p_index, &pb_data);
	if (err != PADDLEBOAT_NO_ERROR) {
		return data;
	}

	data["buttons_down"] = (int64_t)pb_data.buttonsDown;
	data["left_stick_x"] = pb_data.leftStick.stickX;
	data["left_stick_y"] = pb_data.leftStick.stickY;
	data["right_stick_x"] = pb_data.rightStick.stickX;
	data["right_stick_y"] = pb_data.rightStick.stickY;
	data["trigger_l1"] = pb_data.triggerL1;
	data["trigger_l2"] = pb_data.triggerL2;
	data["trigger_r1"] = pb_data.triggerR1;
	data["trigger_r2"] = pb_data.triggerR2;

	return data;
}

bool PaddleboatController::is_initialized() const {
	return _initialized;
}

PaddleboatController::PaddleboatController() {
	_initialized = false;
}

PaddleboatController::~PaddleboatController() {
	destroy();
}
