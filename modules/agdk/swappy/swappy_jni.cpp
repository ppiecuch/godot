/**************************************************************************/
/*  swappy_jni.cpp                                                        */
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

#include "swappy_jni.h"

#include "../agdk_manager.h"
#include "swappy_frame_pacer.h"

#include <EGL/egl.h>

JNIEXPORT void JNICALL Java_org_godotengine_godot_agdk_SwappyBridge_nativeInit(
		JNIEnv *env, jclass clazz, jobject activity) {
	AGDKManager *mgr = AGDKManager::get_singleton();
	if (!mgr) {
		return;
	}
	// Trigger initialization with the Activity reference for Swappy.
	mgr->initialize();
}

JNIEXPORT jboolean JNICALL Java_org_godotengine_godot_agdk_SwappyBridge_nativeSwap(
		JNIEnv *env, jclass clazz, jlong egl_display, jlong egl_surface) {
	AGDKManager *mgr = AGDKManager::get_singleton();
	if (!mgr || !mgr->is_frame_pacing_enabled()) {
		return JNI_FALSE;
	}

	EGLDisplay display = reinterpret_cast<EGLDisplay>(egl_display);
	EGLSurface surface = reinterpret_cast<EGLSurface>(egl_surface);

	// Run per-frame processing before the swap.
	mgr->process_frame();

	SwappyFramePacer *swappy = mgr->get_swappy();
	if (swappy && swappy->is_initialized()) {
		return swappy->swap(display, surface) ? JNI_TRUE : JNI_FALSE;
	}

	return JNI_FALSE;
}

JNIEXPORT void JNICALL Java_org_godotengine_godot_agdk_SwappyBridge_nativeDestroy(
		JNIEnv *env, jclass clazz) {
	AGDKManager *mgr = AGDKManager::get_singleton();
	if (mgr) {
		mgr->finalize();
	}
}
