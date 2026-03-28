/**************************************************************************/
/*  SwappyBridge.java                                                     */
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

import org.godotengine.godot.gl.GLSurfaceView;

import android.app.Activity;
import android.util.Log;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;

/**
 * Bridge between Godot's GLSurfaceView swap mechanism and Swappy frame pacing.
 *
 * Implements GLSurfaceView.SwapStrategy to intercept eglSwapBuffers calls
 * and route them through Swappy for consistent frame pacing.
 */
public class SwappyBridge implements GLSurfaceView.SwapStrategy {
	private static final String TAG = "SwappyBridge";

	// Native methods.
	private static native void nativeInit(Activity activity);
	private static native boolean nativeSwap(long eglDisplay, long eglSurface);
	private static native void nativeDestroy();

	private static SwappyBridge instance;

	/**
	 * Install the Swappy swap strategy into the GLSurfaceView.
	 * Should be called from the GL thread after the surface is created.
	 */
	public static void install(Activity activity) {
		if (instance != null) {
			return; // Already installed.
		}

		// Initialize Swappy native side.
		nativeInit(activity);

		// Set ourselves as the swap strategy.
		instance = new SwappyBridge();
		GLSurfaceView.setSwapStrategy(instance);
		Log.i(TAG, "Swappy frame pacing installed.");
	}

	/**
	 * Uninstall the Swappy swap strategy.
	 */
	public static void uninstall() {
		if (instance == null) {
			return;
		}
		GLSurfaceView.setSwapStrategy(null);
		nativeDestroy();
		instance = null;
		Log.i(TAG, "Swappy frame pacing uninstalled.");
	}

	@Override
	public int swap(EGL10 egl, EGLDisplay display, EGLSurface surface) {
		// Get native EGL handles.
		// EGL10 objects wrap native pointers. We use reflection or direct cast
		// to extract them. In practice, the Android EGL10 implementation stores
		// the native handle in a field.
		long nativeDisplay = getEGLNativeHandle(display);
		long nativeSurface = getEGLNativeHandle(surface);

		if (nativeSwap(nativeDisplay, nativeSurface)) {
			return EGL10.EGL_SUCCESS;
		}

		// Swappy failed — fall back to standard swap.
		if (!egl.eglSwapBuffers(display, surface)) {
			return egl.eglGetError();
		}
		return EGL10.EGL_SUCCESS;
	}

	/**
	 * Extract the native EGL handle from a javax.microedition.khronos EGL wrapper object.
	 * Android's EGL10 implementation stores the native handle in a private field.
	 */
	private static long getEGLNativeHandle(Object eglObject) {
		try {
			// Android's EGLObjectHandle has a getNativeHandle() method (API 29+)
			// or a getHandle() method.
			java.lang.reflect.Method method;
			try {
				method = eglObject.getClass().getMethod("getNativeHandle");
				return (Long)method.invoke(eglObject);
			} catch (NoSuchMethodException e) {
				// Fall back to getHandle() which returns int.
				method = eglObject.getClass().getMethod("getHandle");
				return ((Integer)method.invoke(eglObject)).longValue();
			}
		} catch (Exception e) {
			Log.e(TAG, "Failed to get EGL native handle: " + e.getMessage());
			return 0;
		}
	}
}
