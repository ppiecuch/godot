/**************************************************************************/
/*  GodotAGDKPlugin.java                                                  */
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

package org.godotengine.godot.agdk;

import org.godotengine.godot.Godot;
import org.godotengine.godot.plugin.GodotPlugin;

import android.app.Activity;
import android.util.Log;
import android.view.View;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Android Game Development Kit (AGDK) integration plugin for Godot.
 *
 * Manages lifecycle of AGDK sub-components (Swappy, Paddleboat, ADPF, Oboe)
 * and bridges between Java Activity lifecycle and native C++ code via JNI.
 */
public class GodotAGDKPlugin extends GodotPlugin {
	private static final String TAG = "GodotAGDK";

	// Native methods bridging to C++ AGDK module.
	private static native void nativeOnCreate(Activity activity);
	private static native void nativeOnResume();
	private static native void nativeOnPause();
	private static native void nativeOnDestroy();
	private static native void nativeProcessFrame();

	public GodotAGDKPlugin(Godot godot) {
		super(godot);
	}

	@Override
	public String getPluginName() {
		return "AGDK";
	}

	@Override
	public View onMainCreate(Activity activity) {
		Log.i(TAG, "onMainCreate: Initializing AGDK plugin.");
		try {
			nativeOnCreate(activity);
		} catch (UnsatisfiedLinkError e) {
			Log.e(TAG, "Native AGDK library not loaded: " + e.getMessage());
		}
		return null; // No additional view needed.
	}

	@Override
	public void onMainResume() {
		try {
			nativeOnResume();
		} catch (UnsatisfiedLinkError e) {
			// Silently ignore if native lib not loaded.
		}
	}

	@Override
	public void onMainPause() {
		try {
			nativeOnPause();
		} catch (UnsatisfiedLinkError e) {
			// Silently ignore.
		}
	}

	@Override
	public void onMainDestroy() {
		try {
			nativeOnDestroy();
		} catch (UnsatisfiedLinkError e) {
			// Silently ignore.
		}
	}

	@Override
	public void onGLDrawFrame(GL10 gl) {
		// Per-frame processing: updates Paddleboat, thermal polling, etc.
		try {
			nativeProcessFrame();
		} catch (UnsatisfiedLinkError e) {
			// Silently ignore.
		}
	}

	@Override
	public void onGLSurfaceCreated(GL10 gl, EGLConfig config) {
		Log.i(TAG, "GL surface created — Swappy will be initialized via SwappyBridge.");
		// Swappy initialization happens through SwappyBridge.setSwapStrategy()
		// which is called after the GL context is available.
		try {
			SwappyBridge.install(getActivity());
		} catch (UnsatisfiedLinkError e) {
			Log.w(TAG, "Swappy native bridge not available: " + e.getMessage());
		}
	}
}
