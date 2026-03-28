/**************************************************************************/
/*  agdk_manager.cpp                                                      */
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

#include "agdk_manager.h"

#include "memory/memory_advisor.h"
#include "paddleboat/paddleboat_controller.h"
#include "swappy/swappy_frame_pacer.h"
#include "thermal/adpf_thermal_manager.h"

AGDKManager *AGDKManager::singleton = nullptr;

AGDKManager *AGDKManager::get_singleton() {
	return singleton;
}

void AGDKManager::_bind_methods() {
	// Frame pacing.
	ClassDB::bind_method(D_METHOD("set_target_fps", "fps"), &AGDKManager::set_target_fps);
	ClassDB::bind_method(D_METHOD("get_target_fps"), &AGDKManager::get_target_fps);
	ClassDB::bind_method(D_METHOD("set_auto_swap_interval", "enabled"), &AGDKManager::set_auto_swap_interval);
	ClassDB::bind_method(D_METHOD("get_auto_swap_interval"), &AGDKManager::get_auto_swap_interval);
	ClassDB::bind_method(D_METHOD("set_auto_pipeline_mode", "enabled"), &AGDKManager::set_auto_pipeline_mode);
	ClassDB::bind_method(D_METHOD("get_auto_pipeline_mode"), &AGDKManager::get_auto_pipeline_mode);
	ClassDB::bind_method(D_METHOD("is_frame_pacing_enabled"), &AGDKManager::is_frame_pacing_enabled);
	ClassDB::bind_method(D_METHOD("get_frame_pacing_stats"), &AGDKManager::get_frame_pacing_stats);

	// Thermal.
	ClassDB::bind_method(D_METHOD("get_thermal_headroom", "forecast_seconds"), &AGDKManager::get_thermal_headroom, DEFVAL(1));
	ClassDB::bind_method(D_METHOD("get_thermal_status"), &AGDKManager::get_thermal_status);
	ClassDB::bind_method(D_METHOD("get_thermal_status_string"), &AGDKManager::get_thermal_status_string);

	// Memory.
	ClassDB::bind_method(D_METHOD("get_memory_state"), &AGDKManager::get_memory_state);
	ClassDB::bind_method(D_METHOD("get_memory_state_string"), &AGDKManager::get_memory_state_string);

	// Controllers.
	ClassDB::bind_method(D_METHOD("get_controller_count"), &AGDKManager::get_controller_count);
	ClassDB::bind_method(D_METHOD("get_controller_info", "index"), &AGDKManager::get_controller_info);

	// Lifecycle (called from JNI, not typically from GDScript).
	ClassDB::bind_method(D_METHOD("process_frame"), &AGDKManager::process_frame);

	// Signals.
	ADD_SIGNAL(MethodInfo("thermal_status_changed", PropertyInfo(Variant::INT, "status")));
	ADD_SIGNAL(MethodInfo("memory_state_changed", PropertyInfo(Variant::INT, "state")));
	ADD_SIGNAL(MethodInfo("controller_connected", PropertyInfo(Variant::INT, "index"), PropertyInfo(Variant::DICTIONARY, "info")));
	ADD_SIGNAL(MethodInfo("controller_disconnected", PropertyInfo(Variant::INT, "index")));

	// Enums.
	BIND_ENUM_CONSTANT(THERMAL_STATUS_NONE);
	BIND_ENUM_CONSTANT(THERMAL_STATUS_LIGHT);
	BIND_ENUM_CONSTANT(THERMAL_STATUS_MODERATE);
	BIND_ENUM_CONSTANT(THERMAL_STATUS_SEVERE);
	BIND_ENUM_CONSTANT(THERMAL_STATUS_CRITICAL);
	BIND_ENUM_CONSTANT(THERMAL_STATUS_EMERGENCY);
	BIND_ENUM_CONSTANT(THERMAL_STATUS_SHUTDOWN);

	BIND_ENUM_CONSTANT(MEMORY_STATE_OK);
	BIND_ENUM_CONSTANT(MEMORY_STATE_APPROACHING_LIMIT);
	BIND_ENUM_CONSTANT(MEMORY_STATE_CRITICAL);
}

// Frame pacing.

void AGDKManager::set_target_fps(int p_fps) {
	if (swappy) {
		swappy->set_target_fps(p_fps);
	}
}

int AGDKManager::get_target_fps() const {
	if (swappy) {
		return swappy->get_target_fps();
	}
	return 60;
}

void AGDKManager::set_auto_swap_interval(bool p_enabled) {
	if (swappy) {
		swappy->set_auto_swap_interval(p_enabled);
	}
}

bool AGDKManager::get_auto_swap_interval() const {
	if (swappy) {
		return swappy->get_auto_swap_interval();
	}
	return false;
}

void AGDKManager::set_auto_pipeline_mode(bool p_enabled) {
	if (swappy) {
		swappy->set_auto_pipeline_mode(p_enabled);
	}
}

bool AGDKManager::get_auto_pipeline_mode() const {
	if (swappy) {
		return swappy->get_auto_pipeline_mode();
	}
	return false;
}

bool AGDKManager::is_frame_pacing_enabled() const {
	if (swappy) {
		return swappy->is_initialized();
	}
	return false;
}

Dictionary AGDKManager::get_frame_pacing_stats() const {
	if (swappy) {
		return swappy->get_stats();
	}
	return Dictionary();
}

// Thermal.

float AGDKManager::get_thermal_headroom(int p_forecast_seconds) const {
	if (thermal) {
		return thermal->get_thermal_headroom(p_forecast_seconds);
	}
	return -1.0f;
}

int AGDKManager::get_thermal_status() const {
	if (thermal) {
		return thermal->get_thermal_status();
	}
	return 0; // NONE
}

String AGDKManager::get_thermal_status_string() const {
	int status = get_thermal_status();
	switch (status) {
		case 0:
			return "NONE";
		case 1:
			return "LIGHT";
		case 2:
			return "MODERATE";
		case 3:
			return "SEVERE";
		case 4:
			return "CRITICAL";
		case 5:
			return "EMERGENCY";
		case 6:
			return "SHUTDOWN";
		default:
			return "UNKNOWN";
	}
}

// Memory.

int AGDKManager::get_memory_state() const {
	if (memory) {
		return memory->get_memory_state();
	}
	return 0; // OK
}

String AGDKManager::get_memory_state_string() const {
	int state = get_memory_state();
	switch (state) {
		case 0:
			return "OK";
		case 1:
			return "APPROACHING_LIMIT";
		case 2:
			return "CRITICAL";
		default:
			return "UNKNOWN";
	}
}

// Controllers.

int AGDKManager::get_controller_count() const {
	if (paddleboat) {
		return paddleboat->get_controller_count();
	}
	return 0;
}

Dictionary AGDKManager::get_controller_info(int p_index) const {
	if (paddleboat) {
		return paddleboat->get_controller_info(p_index);
	}
	return Dictionary();
}

// Per-frame processing.

void AGDKManager::process_frame() {
	if (paddleboat) {
		paddleboat->update();
	}
	if (thermal) {
		thermal->update();
	}
	if (memory) {
		memory->update();
	}
}

// Lifecycle.

void AGDKManager::initialize() {
	if (initialized) {
		return;
	}

	// Read ProjectSettings and initialize sub-components.
	bool enable_frame_pacing = GLOBAL_DEF("android/agdk/enable_frame_pacing", true);
	int target_fps = GLOBAL_DEF("android/agdk/target_fps", 60);
	bool auto_swap = GLOBAL_DEF("android/agdk/auto_swap_interval", true);
	bool auto_pipeline = GLOBAL_DEF("android/agdk/auto_pipeline_mode", true);
	bool use_paddleboat = GLOBAL_DEF("android/agdk/use_paddleboat_controllers", true);
	bool enable_thermal = GLOBAL_DEF("android/agdk/enable_thermal_monitoring", true);
	bool enable_memory = GLOBAL_DEF("android/agdk/enable_memory_advice", false);

	ProjectSettings::get_singleton()->set_custom_property_info("android/agdk/enable_frame_pacing", PropertyInfo(Variant::BOOL, "android/agdk/enable_frame_pacing"));
	ProjectSettings::get_singleton()->set_custom_property_info("android/agdk/target_fps", PropertyInfo(Variant::INT, "android/agdk/target_fps", PROPERTY_HINT_RANGE, "15,120,1"));
	ProjectSettings::get_singleton()->set_custom_property_info("android/agdk/auto_swap_interval", PropertyInfo(Variant::BOOL, "android/agdk/auto_swap_interval"));
	ProjectSettings::get_singleton()->set_custom_property_info("android/agdk/auto_pipeline_mode", PropertyInfo(Variant::BOOL, "android/agdk/auto_pipeline_mode"));
	ProjectSettings::get_singleton()->set_custom_property_info("android/agdk/use_paddleboat_controllers", PropertyInfo(Variant::BOOL, "android/agdk/use_paddleboat_controllers"));
	ProjectSettings::get_singleton()->set_custom_property_info("android/agdk/enable_thermal_monitoring", PropertyInfo(Variant::BOOL, "android/agdk/enable_thermal_monitoring"));
	ProjectSettings::get_singleton()->set_custom_property_info("android/agdk/enable_memory_advice", PropertyInfo(Variant::BOOL, "android/agdk/enable_memory_advice"));

	if (enable_frame_pacing) {
		swappy = memnew(SwappyFramePacer);
		swappy->set_target_fps(target_fps);
		swappy->set_auto_swap_interval(auto_swap);
		swappy->set_auto_pipeline_mode(auto_pipeline);
	}

	if (use_paddleboat) {
		paddleboat = memnew(PaddleboatController);
	}

	if (enable_thermal) {
		thermal = memnew(ADPFThermalManager);
		Error err = thermal->initialize();
		if (err != OK) {
			print_line("AGDK: Thermal API not available on this device.");
		}
	}

	if (enable_memory) {
		memory = memnew(MemoryAdvisor);
	}

	initialized = true;
	print_line("AGDK: Manager initialized.");
}

void AGDKManager::on_pause() {
	if (paddleboat) {
		paddleboat->on_pause();
	}
}

void AGDKManager::on_resume() {
	if (paddleboat) {
		paddleboat->on_resume();
	}
}

void AGDKManager::finalize() {
	if (swappy) {
		swappy->destroy();
		memdelete(swappy);
		swappy = nullptr;
	}
	if (paddleboat) {
		paddleboat->destroy();
		memdelete(paddleboat);
		paddleboat = nullptr;
	}
	if (thermal) {
		thermal->destroy();
		memdelete(thermal);
		thermal = nullptr;
	}
	if (memory) {
		memdelete(memory);
		memory = nullptr;
	}
	initialized = false;
}

AGDKManager::AGDKManager() {
	singleton = this;
	swappy = nullptr;
	paddleboat = nullptr;
	thermal = nullptr;
	memory = nullptr;
	initialized = false;
}

AGDKManager::~AGDKManager() {
	finalize();
	singleton = nullptr;
}
