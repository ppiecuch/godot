/**************************************************************************/
/*  adpf_thermal_manager.cpp                                              */
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

#include "adpf_thermal_manager.h"

#include "../agdk_manager.h"
#include "core/print_string.h"

#include <dlfcn.h>
#include <math.h>

bool ADPFThermalManager::_load_functions() {
	// Load thermal API functions at runtime via dlsym.
	// This allows compilation against older NDK targets while using API 30+ features.
	void *lib = dlopen("libandroid.so", RTLD_NOW);
	if (!lib) {
		return false;
	}

	_fn_acquireManager = (PFN_AThermal_acquireManager)dlsym(lib, "AThermal_acquireManager");
	_fn_releaseManager = (PFN_AThermal_releaseManager)dlsym(lib, "AThermal_releaseManager");
	_fn_getThermalHeadroom = (PFN_AThermal_getThermalHeadroom)dlsym(lib, "AThermal_getThermalHeadroom");
	_fn_getCurrentThermalStatus = (PFN_AThermal_getCurrentThermalStatus)dlsym(lib, "AThermal_getCurrentThermalStatus");

	// All functions must be available.
	return _fn_acquireManager && _fn_releaseManager &&
			_fn_getThermalHeadroom && _fn_getCurrentThermalStatus;
}

Error ADPFThermalManager::initialize() {
	if (_available) {
		return OK;
	}

	if (!_load_functions()) {
		print_line("AGDK Thermal: API not available (requires API 30+).");
		return ERR_UNAVAILABLE;
	}

	_thermal_manager = _fn_acquireManager();
	if (!_thermal_manager) {
		print_line("AGDK Thermal: Failed to acquire thermal manager.");
		return ERR_CANT_CREATE;
	}

	// Validate API works by making a test call.
	float test_headroom = _fn_getThermalHeadroom(_thermal_manager, 0);
	if (isnan(test_headroom)) {
		print_line("AGDK Thermal: API returned NaN, may not be supported on this device.");
		// Don't fail — some devices support status but not headroom.
	}

	_available = true;
	_last_poll_usec = 0;
	_cached_headroom = 0.0f;
	_cached_status = 0; // NONE
	_last_emitted_status = 0;

	print_line("AGDK Thermal: Monitoring initialized.");
	return OK;
}

void ADPFThermalManager::update() {
	if (!_available) {
		return;
	}

	uint64_t now = OS::get_singleton()->get_ticks_usec();
	if (now - _last_poll_usec < POLL_INTERVAL_USEC) {
		return;
	}
	_last_poll_usec = now;

	// Query headroom (0 = current, no forecast).
	float headroom = _fn_getThermalHeadroom(_thermal_manager, 0);
	if (!isnan(headroom)) {
		_cached_headroom = headroom;
	}

	// Query status.
	int status = _fn_getCurrentThermalStatus(_thermal_manager);
	_cached_status = status;

	// Emit signal if status changed.
	if (status != _last_emitted_status) {
		_last_emitted_status = status;
		AGDKManager *mgr = AGDKManager::get_singleton();
		if (mgr) {
			mgr->emit_signal("thermal_status_changed", status);
		}
	}
}

void ADPFThermalManager::destroy() {
	if (!_available || !_thermal_manager) {
		return;
	}

	_fn_releaseManager(_thermal_manager);
	_thermal_manager = nullptr;
	_available = false;

	print_line("AGDK Thermal: Monitoring destroyed.");
}

float ADPFThermalManager::get_thermal_headroom(int p_forecast_seconds) const {
	if (!_available) {
		return -1.0f;
	}
	// For real-time queries with forecast, call directly (rate-limited externally).
	if (p_forecast_seconds > 0 && _fn_getThermalHeadroom) {
		float h = _fn_getThermalHeadroom(_thermal_manager, p_forecast_seconds);
		if (!isnan(h)) {
			return h;
		}
	}
	return _cached_headroom;
}

int ADPFThermalManager::get_thermal_status() const {
	return _cached_status;
}

bool ADPFThermalManager::is_available() const {
	return _available;
}

ADPFThermalManager::ADPFThermalManager() {
	_thermal_manager = nullptr;
	_available = false;
	_cached_headroom = 0.0f;
	_cached_status = 0;
	_last_emitted_status = 0;
	_last_poll_usec = 0;
	_fn_acquireManager = nullptr;
	_fn_releaseManager = nullptr;
	_fn_getThermalHeadroom = nullptr;
	_fn_getCurrentThermalStatus = nullptr;
}

ADPFThermalManager::~ADPFThermalManager() {
	destroy();
}
