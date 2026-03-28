/**************************************************************************/
/*  adpf_thermal_manager.h                                                */
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

#ifndef ADPF_THERMAL_MANAGER_H
#define ADPF_THERMAL_MANAGER_H

#include "core/os/os.h"
#include "core/variant.h"

#include <android/api-level.h>

// Forward declare to avoid header dependency on API 30+ headers.
struct AThermalManager;

class ADPFThermalManager {
	AThermalManager *_thermal_manager;
	bool _available;

	float _cached_headroom;
	int _cached_status;
	int _last_emitted_status;
	uint64_t _last_poll_usec;

	// Rate limit: don't poll more than once per 10 seconds (API requirement).
	static const uint64_t POLL_INTERVAL_USEC = 10000000ULL;

	// Function pointers loaded at runtime via dlsym.
	typedef AThermalManager *(*PFN_AThermal_acquireManager)();
	typedef void (*PFN_AThermal_releaseManager)(AThermalManager *);
	typedef float (*PFN_AThermal_getThermalHeadroom)(AThermalManager *, int);
	typedef int (*PFN_AThermal_getCurrentThermalStatus)(AThermalManager *);

	PFN_AThermal_acquireManager _fn_acquireManager;
	PFN_AThermal_releaseManager _fn_releaseManager;
	PFN_AThermal_getThermalHeadroom _fn_getThermalHeadroom;
	PFN_AThermal_getCurrentThermalStatus _fn_getCurrentThermalStatus;

	bool _load_functions();

public:
	Error initialize();
	void update();
	void destroy();

	float get_thermal_headroom(int p_forecast_seconds = 1) const;
	int get_thermal_status() const;

	bool is_available() const;

	ADPFThermalManager();
	~ADPFThermalManager();
};

#endif // ADPF_THERMAL_MANAGER_H
