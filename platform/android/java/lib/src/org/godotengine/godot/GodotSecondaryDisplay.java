/**************************************************************************/
/*  GodotSecondaryDisplay.java                                            */
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

package org.godotengine.godot;

import android.app.Presentation;
import android.content.Context;
import android.hardware.display.DisplayManager;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

/**
 * Mirrors the debug console (see scene/debugconsole/CONSOLE.md) onto an auxiliary screen.
 *
 * A {@link Presentation} is a Dialog bound to a {@link Display}, so no manifest entry and no
 * permission is needed. It hosts a plain {@link SurfaceView}: the engine draws the character
 * grid into the Surface in software from the main thread (ANativeWindow_lock), which keeps
 * the single GL context of {@link GodotRenderer} untouched.
 *
 * Everything in this class runs on the UI thread; the native side queues what it receives.
 */
public class GodotSecondaryDisplay implements DisplayManager.DisplayListener {
	private static final String TAG = GodotSecondaryDisplay.class.getSimpleName();

	private final Context context;
	private final DisplayManager displayManager;

	private ConsolePresentation presentation;
	private int displayId = -1;

	public GodotSecondaryDisplay(Context context) {
		this.context = context;
		this.displayManager = (DisplayManager)context.getSystemService(Context.DISPLAY_SERVICE);
	}

	/**
	 * Starts listening for presentation displays and attaches to one if it is already there.
	 * Safe to call more than once.
	 */
	public void onCreate() {
		if (displayManager == null) {
			return;
		}
		displayManager.registerDisplayListener(this, null);
		scan();
	}

	/**
	 * Picks up displays that appeared while the activity was in the background.
	 */
	public void onResume() {
		scan();
	}

	/**
	 * The Presentation belongs to the activity window, so it must go before it does.
	 */
	public void onPause() {
		dismiss();
	}

	public void onDestroy() {
		if (displayManager != null) {
			displayManager.unregisterDisplayListener(this);
		}
		dismiss();
	}

	public boolean isActive() {
		return presentation != null;
	}

	private void scan() {
		if (displayManager == null) {
			return;
		}
		Display[] displays = displayManager.getDisplays(DisplayManager.DISPLAY_CATEGORY_PRESENTATION);
		if (displays != null && displays.length > 0) {
			show(displays[0]);
		}
	}

	private boolean isPresentationDisplay(Display display) {
		return (display.getFlags() & Display.FLAG_PRESENTATION) != 0 ||
				display.getDisplayId() != Display.DEFAULT_DISPLAY;
	}

	private void show(Display display) {
		if (display == null) {
			return;
		}
		// Already showing on this very display: keep the surface we have.
		if (presentation != null && displayId == display.getDisplayId()) {
			return;
		}
		dismiss();

		displayId = display.getDisplayId();
		presentation = new ConsolePresentation(context, display);
		try {
			presentation.show();
			Log.i(TAG, "Console presentation shown on display " + displayId + " (" + display.getName() + ")");
		} catch (Exception e) {
			// Happens when the activity window is not attached (yet), e.g. during rotation.
			Log.w(TAG, "Failed to show console presentation: " + e.getMessage());
			presentation = null;
			displayId = -1;
		}
	}

	private void dismiss() {
		if (presentation != null) {
			presentation.dismiss();
			presentation = null;
		}
		if (displayId != -1) {
			nativeDetached(displayId);
			displayId = -1;
		}
	}

	@Override
	public void onDisplayAdded(int addedDisplayId) {
		if (displayManager == null) {
			return;
		}
		Display display = displayManager.getDisplay(addedDisplayId);
		if (display != null && isPresentationDisplay(display)) {
			show(display);
		}
	}

	@Override
	public void onDisplayRemoved(int removedDisplayId) {
		if (removedDisplayId == displayId) {
			dismiss();
		}
	}

	@Override
	public void onDisplayChanged(int changedDisplayId) {
		// Geometry changes arrive through SurfaceHolder.Callback.surfaceChanged().
	}

	/**
	 * GodotLib may not be loaded yet (or already unloaded) when a display event arrives.
	 */
	private static void nativeAttached(android.view.Surface surface, int displayId, int width, int height, int dpi) {
		try {
			GodotLib.secondaryDisplayAttached(surface, displayId, width, height, dpi);
		} catch (UnsatisfiedLinkError e) {
			Log.w(TAG, "Native secondary display support not available");
		}
	}

	private static void nativeDetached(int displayId) {
		try {
			GodotLib.secondaryDisplayDetached(displayId);
		} catch (UnsatisfiedLinkError e) {
			/* Native library gone, nothing to detach from. */
		}
	}

	private class ConsolePresentation extends Presentation implements SurfaceHolder.Callback {
		private SurfaceView surfaceView;

		ConsolePresentation(Context context, Display display) {
			super(context, display);
		}

		@Override
		protected void onCreate(Bundle savedInstanceState) {
			super.onCreate(savedInstanceState);

			// The panel is an output device, not a UI: if this Dialog window ever takes
			// input focus the game window loses it, and the gamepad goes dead until the
			// player taps the main screen again. Touches are still delivered to the
			// SurfaceView below (only FLAG_NOT_TOUCHABLE would stop those); whether they
			// mean anything is decided by the console (debug/console/panel_touch).
			Window window = getWindow();
			if (window != null) {
				window.addFlags(WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE |
						WindowManager.LayoutParams.FLAG_ALT_FOCUSABLE_IM);
			}

			surfaceView = new SurfaceView(getContext());
			surfaceView.setFocusable(false);
			surfaceView.setFocusableInTouchMode(false);
			surfaceView.getHolder().addCallback(this);
			surfaceView.setOnTouchListener(new View.OnTouchListener() {
				@Override
				public boolean onTouch(View view, MotionEvent event) {
					final int action = event.getActionMasked();
					if (action == MotionEvent.ACTION_DOWN || action == MotionEvent.ACTION_UP) {
						try {
							GodotLib.secondaryDisplayTouch(event.getX(), event.getY(), action == MotionEvent.ACTION_DOWN);
						} catch (UnsatisfiedLinkError e) {
							/* Native library not loaded. */
						}
					}
					return true;
				}
			});
			setContentView(surfaceView);
		}

		@Override
		public void surfaceCreated(SurfaceHolder holder) {
			// Geometry is only known in surfaceChanged(), which always follows.
		}

		@Override
		public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
			DisplayMetrics metrics = new DisplayMetrics();
			getDisplay().getMetrics(metrics);
			Log.i(TAG, "Console surface ready: " + width + "x" + height + " @ " + metrics.densityDpi + " dpi");
			nativeAttached(holder.getSurface(), displayId, width, height, metrics.densityDpi);
		}

		@Override
		public void surfaceDestroyed(SurfaceHolder holder) {
			nativeDetached(displayId);
		}
	}
}
