/**************************************************************************/
/*  secondary_display_android.cpp                                         */
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

#include "secondary_display_android.h"

#include "core/os/os.h"
#include "core/print_string.h"
#include "core/ustring.h"
#include "core/variant.h"

SecondaryDisplayAndroid *SecondaryDisplayAndroid::singleton = nullptr;

SecondaryDisplayAndroid::SecondaryDisplayAndroid() {
	singleton = this;
}

SecondaryDisplayAndroid::~SecondaryDisplayAndroid() {
	{
		MutexLock lock(mutex);
		_release_window();
	}
	if (singleton == this) {
		singleton = nullptr;
	}
}

void SecondaryDisplayAndroid::_release_window() {
	if (window) {
		ANativeWindow_release(window);
		window = nullptr;
	}
	display_id = -1;
	width = 0;
	height = 0;
	dpi = 0;
}

void SecondaryDisplayAndroid::_attach(ANativeWindow *p_window, int p_display_id, int p_width, int p_height, int p_dpi) {
	MutexLock lock(mutex);

	// surfaceChanged() fires for every geometry change, so an existing surface is simply
	// swapped out; the reference taken by the JNI layer moves into this object.
	_release_window();

	window = p_window;
	display_id = p_display_id;
	width = p_width;
	height = p_height;
	dpi = p_dpi > 0 ? p_dpi : 160;

	// 8 bits per channel, R,G,B,A in memory order -- CONSOLE_PIXEL_ABGR32 on little endian.
	ANativeWindow_setBuffersGeometry(window, width, height, WINDOW_FORMAT_RGBA_8888);

	print_verbose(vformat("Secondary display attached: %dx%d @ %d dpi (id %d)", width, height, dpi, display_id));
}

void SecondaryDisplayAndroid::_detach(int p_display_id) {
	MutexLock lock(mutex);

	if (!window) {
		return;
	}
	// -1 means "whatever is attached", used when the activity goes away.
	if (p_display_id != -1 && p_display_id != display_id) {
		return;
	}
	print_verbose(vformat("Secondary display detached (id %d)", display_id));
	_release_window();
}

void SecondaryDisplayAndroid::_push_touch(float p_x, float p_y, bool p_pressed) {
	MutexLock lock(touch_mutex);

	// The console drains this once per frame; a flood of moves must not grow unbounded if
	// the console is not running at all.
	if (touch_queue.size() > 32) {
		touch_queue.remove(0);
	}
	TouchEvent ev;
	ev.position = Vector2(p_x, p_y);
	ev.pressed = p_pressed;
	ev.time_msec = OS::get_singleton()->get_ticks_msec();
	touch_queue.push_back(ev);
}

bool SecondaryDisplayAndroid::is_available() const {
	MutexLock lock(mutex);
	return window != nullptr && width > 0 && height > 0;
}

Size2i SecondaryDisplayAndroid::get_size() const {
	MutexLock lock(mutex);
	return Size2i(width, height);
}

int SecondaryDisplayAndroid::get_dpi() const {
	MutexLock lock(mutex);
	return dpi;
}

uint32_t *SecondaryDisplayAndroid::lock_buffer(int &r_stride_px, Size2i &r_size) {
	mutex.lock();

	if (!window) {
		mutex.unlock();
		return nullptr;
	}

	ANativeWindow_Buffer buffer;
	if (ANativeWindow_lock(window, &buffer, nullptr) < 0) {
		mutex.unlock();
		return nullptr;
	}

	locked = true;
	r_stride_px = buffer.stride;
	r_size = Size2i(buffer.width, buffer.height);
	return (uint32_t *)buffer.bits;
}

void SecondaryDisplayAndroid::unlock_and_post() {
	if (!locked) {
		return;
	}
	if (window) {
		ANativeWindow_unlockAndPost(window);
	}
	locked = false;
	mutex.unlock();
}

bool SecondaryDisplayAndroid::pop_touch(TouchEvent &r_event) {
	MutexLock lock(touch_mutex);

	if (touch_queue.empty()) {
		return false;
	}
	r_event = touch_queue[0];
	touch_queue.remove(0);
	return true;
}

void secondary_display_attached(ANativeWindow *p_window, int p_display_id, int p_width, int p_height, int p_dpi) {
	SecondaryDisplayAndroid *display = SecondaryDisplayAndroid::get_singleton();
	if (!display) {
		// No engine-side display yet (or already torn down); drop the surface reference.
		if (p_window) {
			ANativeWindow_release(p_window);
		}
		return;
	}
	display->_attach(p_window, p_display_id, p_width, p_height, p_dpi);
}

void secondary_display_detached(int p_display_id) {
	if (SecondaryDisplayAndroid *display = SecondaryDisplayAndroid::get_singleton()) {
		display->_detach(p_display_id);
	}
}

void secondary_display_touch(float p_x, float p_y, bool p_pressed) {
	if (SecondaryDisplayAndroid *display = SecondaryDisplayAndroid::get_singleton()) {
		display->_push_touch(p_x, p_y, p_pressed);
	}
}
