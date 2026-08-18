/**************************************************************************/
/*  secondary_display_android.h                                           */
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

#ifndef SECONDARY_DISPLAY_ANDROID_H
#define SECONDARY_DISPLAY_ANDROID_H

#include "core/os/secondary_display.h"

#include <android/native_window.h>

/* Android implementation of SecondaryDisplay, backed by an ANativeWindow taken from the
 * Surface of the SurfaceView inside GodotSecondaryDisplay's Presentation (see
 * platform/android/java/lib/src/org/godotengine/godot/GodotSecondaryDisplay.java).
 *
 * The Java side runs on the UI thread and only ever calls the free functions at the bottom
 * of this header, which take the class mutex. Rendering happens on the main thread through
 * the SecondaryDisplay interface. The mutex is held from lock_buffer() until
 * unlock_and_post(), so the surface cannot be released mid-blit.
 */
class SecondaryDisplayAndroid : public SecondaryDisplay {
	friend void secondary_display_attached(ANativeWindow *p_window, int p_display_id, int p_width, int p_height, int p_dpi);
	friend void secondary_display_detached(int p_display_id);
	friend void secondary_display_touch(float p_x, float p_y, int p_type, int p_index);

	static SecondaryDisplayAndroid *singleton;

	mutable Mutex mutex;
	Mutex touch_mutex;

	ANativeWindow *window = nullptr;
	int display_id = -1;
	int width = 0;
	int height = 0;
	int dpi = 0;
	bool locked = false; // a lock_buffer() is in flight, `mutex` is held by the render thread

	Vector<TouchEvent> touch_queue;

	void _attach(ANativeWindow *p_window, int p_display_id, int p_width, int p_height, int p_dpi);
	void _detach(int p_display_id);
	void _push_touch(float p_x, float p_y, int p_type, int p_index);
	void _release_window(); // mutex must be held

public:
	static SecondaryDisplayAndroid *get_singleton() { return singleton; }

	virtual bool is_available() const;
	virtual Size2i get_size() const;
	virtual int get_dpi() const;

	virtual uint32_t *lock_buffer(int &r_stride_px, Size2i &r_size);
	virtual void unlock_and_post();

	virtual bool pop_touch(TouchEvent &r_event);

	SecondaryDisplayAndroid();
	~SecondaryDisplayAndroid();
};

/* Entry points for the JNI layer (secondary_display_jni.cpp), safe to call before the
 * SecondaryDisplayAndroid instance exists -- the surface is then simply dropped. */
void secondary_display_attached(ANativeWindow *p_window, int p_display_id, int p_width, int p_height, int p_dpi);
void secondary_display_detached(int p_display_id);
void secondary_display_touch(float p_x, float p_y, int p_type, int p_index);

#endif // SECONDARY_DISPLAY_ANDROID_H
