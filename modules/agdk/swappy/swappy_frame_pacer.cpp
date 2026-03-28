/**************************************************************************/
/*  swappy_frame_pacer.cpp                                                */
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

#include "swappy_frame_pacer.h"

#include "core/os/os.h"
#include "core/print_string.h"

#include <swappy/swappyGL.h>
#include <swappy/swappyGL_extra.h>

Error SwappyFramePacer::init(JNIEnv *p_env, jobject p_activity) {
	if (_initialized) {
		return OK;
	}

	SwappyGL_init(p_env, p_activity);

	if (!SwappyGL_isEnabled()) {
		print_line("AGDK Swappy: Failed to initialize frame pacing.");
		return ERR_CANT_CREATE;
	}

	// Apply stored configuration.
	uint64_t swap_ns = NANOS_PER_SECOND / (uint64_t)_target_fps;
	SwappyGL_setSwapIntervalNS(swap_ns);
	SwappyGL_setAutoSwapInterval(_auto_swap_interval);
	SwappyGL_setAutoPipelineMode(_auto_pipeline_mode);

	_initialized = true;
	print_line("AGDK Swappy: Frame pacing initialized at " + itos(_target_fps) + " FPS.");
	return OK;
}

bool SwappyFramePacer::swap(EGLDisplay p_display, EGLSurface p_surface) {
	if (!_initialized) {
		return false;
	}
	return SwappyGL_swap(p_display, p_surface);
}

void SwappyFramePacer::destroy() {
	if (!_initialized) {
		return;
	}
	SwappyGL_destroy();
	_initialized = false;
	print_line("AGDK Swappy: Frame pacing destroyed.");
}

void SwappyFramePacer::set_target_fps(int p_fps) {
	_target_fps = CLAMP(p_fps, 15, 120);
	if (_initialized) {
		uint64_t swap_ns = NANOS_PER_SECOND / (uint64_t)_target_fps;
		SwappyGL_setSwapIntervalNS(swap_ns);
	}
}

int SwappyFramePacer::get_target_fps() const {
	return _target_fps;
}

void SwappyFramePacer::set_auto_swap_interval(bool p_enabled) {
	_auto_swap_interval = p_enabled;
	if (_initialized) {
		SwappyGL_setAutoSwapInterval(p_enabled);
	}
}

bool SwappyFramePacer::get_auto_swap_interval() const {
	return _auto_swap_interval;
}

void SwappyFramePacer::set_auto_pipeline_mode(bool p_enabled) {
	_auto_pipeline_mode = p_enabled;
	if (_initialized) {
		SwappyGL_setAutoPipelineMode(p_enabled);
	}
}

bool SwappyFramePacer::get_auto_pipeline_mode() const {
	return _auto_pipeline_mode;
}

bool SwappyFramePacer::is_initialized() const {
	return _initialized;
}

Dictionary SwappyFramePacer::get_stats() const {
	Dictionary stats;
	if (!_initialized) {
		return stats;
	}

	SwappyStats swappy_stats;
	SwappyGL_getStats(&swappy_stats);

	stats["total_frames"] = (int64_t)swappy_stats.totalFrames;

	return stats;
}

SwappyFramePacer::SwappyFramePacer() {
	_initialized = false;
	_target_fps = 60;
	_auto_swap_interval = true;
	_auto_pipeline_mode = true;
}

SwappyFramePacer::~SwappyFramePacer() {
	destroy();
}
