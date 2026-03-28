/**************************************************************************/
/*  agdk_manager.h                                                        */
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

#ifndef AGDK_MANAGER_H
#define AGDK_MANAGER_H

#include "core/object.h"
#include "core/os/mutex.h"
#include "core/project_settings.h"
#include "core/variant.h"

class SwappyFramePacer;
class PaddleboatController;
class ADPFThermalManager;
class MemoryAdvisor;

class AGDKManager : public Object {
	GDCLASS(AGDKManager, Object);

	static AGDKManager *singleton;

	SwappyFramePacer *swappy;
	PaddleboatController *paddleboat;
	ADPFThermalManager *thermal;
	MemoryAdvisor *memory;

	bool initialized;

public:
	enum ThermalStatus {
		THERMAL_STATUS_NONE = 0,
		THERMAL_STATUS_LIGHT = 1,
		THERMAL_STATUS_MODERATE = 2,
		THERMAL_STATUS_SEVERE = 3,
		THERMAL_STATUS_CRITICAL = 4,
		THERMAL_STATUS_EMERGENCY = 5,
		THERMAL_STATUS_SHUTDOWN = 6,
	};

	enum MemoryState {
		MEMORY_STATE_OK = 0,
		MEMORY_STATE_APPROACHING_LIMIT = 1,
		MEMORY_STATE_CRITICAL = 2,
	};

private:
protected:
	static void _bind_methods();

public:
	static AGDKManager *get_singleton();

	// Frame pacing (Swappy).
	void set_target_fps(int p_fps);
	int get_target_fps() const;
	void set_auto_swap_interval(bool p_enabled);
	bool get_auto_swap_interval() const;
	void set_auto_pipeline_mode(bool p_enabled);
	bool get_auto_pipeline_mode() const;
	bool is_frame_pacing_enabled() const;
	Dictionary get_frame_pacing_stats() const;

	// Thermal (ADPF).
	float get_thermal_headroom(int p_forecast_seconds = 1) const;
	int get_thermal_status() const;
	String get_thermal_status_string() const;

	// Memory.
	int get_memory_state() const;
	String get_memory_state_string() const;

	// Controllers (Paddleboat).
	int get_controller_count() const;
	Dictionary get_controller_info(int p_index) const;

	// Direct access for JNI layer.
	SwappyFramePacer *get_swappy() const { return swappy; }
	PaddleboatController *get_paddleboat() const { return paddleboat; }

	// Per-frame update called from JNI/plugin.
	void process_frame();

	// Lifecycle.
	void initialize();
	void on_pause();
	void on_resume();
	void finalize();

	AGDKManager();
	~AGDKManager();
};

VARIANT_ENUM_CAST(AGDKManager::ThermalStatus);
VARIANT_ENUM_CAST(AGDKManager::MemoryState);

#endif // AGDK_MANAGER_H
