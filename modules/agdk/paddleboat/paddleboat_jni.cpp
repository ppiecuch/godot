/**************************************************************************/
/*  paddleboat_jni.cpp                                                    */
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

#include "paddleboat_jni.h"

#include "../agdk_manager.h"
#include "paddleboat_controller.h"

JNIEXPORT void JNICALL Java_org_godotengine_godot_agdk_GodotAGDKPlugin_nativeOnCreate(
		JNIEnv *env, jclass clazz, jobject activity) {
	AGDKManager *mgr = AGDKManager::get_singleton();
	if (!mgr) {
		return;
	}
	mgr->initialize();

	// Initialize Paddleboat with the Activity context.
	PaddleboatController *pb = mgr->get_paddleboat();
	if (pb) {
		pb->init(env, activity);
	}
}

JNIEXPORT void JNICALL Java_org_godotengine_godot_agdk_GodotAGDKPlugin_nativeOnResume(
		JNIEnv *env, jclass clazz) {
	AGDKManager *mgr = AGDKManager::get_singleton();
	if (mgr) {
		mgr->on_resume();
	}
}

JNIEXPORT void JNICALL Java_org_godotengine_godot_agdk_GodotAGDKPlugin_nativeOnPause(
		JNIEnv *env, jclass clazz) {
	AGDKManager *mgr = AGDKManager::get_singleton();
	if (mgr) {
		mgr->on_pause();
	}
}

JNIEXPORT void JNICALL Java_org_godotengine_godot_agdk_GodotAGDKPlugin_nativeOnDestroy(
		JNIEnv *env, jclass clazz) {
	AGDKManager *mgr = AGDKManager::get_singleton();
	if (mgr) {
		mgr->finalize();
	}
}

JNIEXPORT void JNICALL Java_org_godotengine_godot_agdk_GodotAGDKPlugin_nativeProcessFrame(
		JNIEnv *env, jclass clazz) {
	AGDKManager *mgr = AGDKManager::get_singleton();
	if (mgr) {
		mgr->process_frame();
	}
}
