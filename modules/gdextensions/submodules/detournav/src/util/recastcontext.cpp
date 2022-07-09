/*************************************************************************/
/*  recastcontext.cpp                                                    */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "recastcontext.h"

#include "core/os/os.h"

RecastContext::RecastContext() {
	doResetTimers();
}

void RecastContext::doResetLog() {
	// ???
}

void RecastContext::doLog(const rcLogCategory category, const char *msg, const int len) {
	// Sanity check
	if (!len || msg == 0) {
		ERR_PRINT("Log message from recast invalid: ");
		return;
	}

	// Assemble the message
	String catMsg = "";
	switch (category) {
		case RC_LOG_PROGRESS:
			catMsg = "progress";
			break;

		case RC_LOG_WARNING:
			catMsg = "warning";
			break;

		case RC_LOG_ERROR:
			catMsg = "error";
			break;

		default:
			catMsg = "unknown";
			break;
	}

	print_verbose(vformat("recast: %s: %s", catMsg, msg));
}

void RecastContext::doResetTimers() {
	for (int i = 0; i < RC_MAX_TIMERS; ++i) {
		_timers[(rcTimerLabel)i] = -1;
		_accumulatedTime[(rcTimerLabel)i] = -1;
	}
}

void RecastContext::doStartTimer(const rcTimerLabel label) {
	_timers[label] = OS::get_singleton()->get_ticks_msec();
}

void RecastContext::doStopTimer(const rcTimerLabel label) {
	int64_t now = OS::get_singleton()->get_ticks_msec();
	int64_t deltaTime = now - _timers[label];
	if (_accumulatedTime[label] == -1) {
		_accumulatedTime[label] = deltaTime;
	} else {
		_accumulatedTime[label] += deltaTime;
	}
}

int RecastContext::doGetAccumulatedTime(const rcTimerLabel label) const {
	return _accumulatedTime.at(label);
}
