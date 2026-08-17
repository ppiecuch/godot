/**************************************************************************/
/*  secondary_display_jni.cpp                                             */
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

#include <android/native_window_jni.h>
#include <jni.h>

// Java side: platform/android/java/lib/src/org/godotengine/godot/GodotSecondaryDisplay.java,
// declared in GodotLib.java. Everything here runs on the Android UI thread, so nothing may
// touch the scene tree -- state is handed to SecondaryDisplayAndroid, which is mutex guarded.

extern "C" {

JNIEXPORT void JNICALL Java_org_godotengine_godot_GodotLib_secondaryDisplayAttached(JNIEnv *env, jclass clazz, jobject p_surface, jint p_display_id, jint p_width, jint p_height, jint p_dpi) {
	if (!p_surface) {
		secondary_display_detached(p_display_id);
		return;
	}

	// Owns one reference from here on; SecondaryDisplayAndroid releases it.
	ANativeWindow *window = ANativeWindow_fromSurface(env, p_surface);
	if (!window) {
		return;
	}
	secondary_display_attached(window, p_display_id, p_width, p_height, p_dpi);
}

JNIEXPORT void JNICALL Java_org_godotengine_godot_GodotLib_secondaryDisplayDetached(JNIEnv *env, jclass clazz, jint p_display_id) {
	secondary_display_detached(p_display_id);
}

JNIEXPORT void JNICALL Java_org_godotengine_godot_GodotLib_secondaryDisplayTouch(JNIEnv *env, jclass clazz, jfloat p_x, jfloat p_y, jboolean p_pressed) {
	secondary_display_touch(p_x, p_y, p_pressed);
}
}
