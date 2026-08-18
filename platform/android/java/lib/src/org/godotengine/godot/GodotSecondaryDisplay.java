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
import android.graphics.Rect;
import android.hardware.display.DisplayManager;
import android.os.Build;
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

import java.util.ArrayList;
import java.util.List;

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

	// Mirrors SecondaryDisplay::TouchType (core/os/secondary_display.h).
	private static final int TOUCH_DOWN = 0;
	private static final int TOUCH_UP = 1;
	private static final int TOUCH_MOVE = 2;

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
			return;
		}
		// Not every handheld flags its auxiliary panel FLAG_PRESENTATION, and the category
		// query returns nothing at all for those. Any display that is not the built-in one
		// is a panel as far as the console is concerned.
		Display[] all = displayManager.getDisplays();
		if (all == null) {
			return;
		}
		for (Display display : all) {
			if (display.getDisplayId() != Display.DEFAULT_DISPLAY) {
				show(display);
				return;
			}
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

			applyImmersive(window);

			surfaceView = new SurfaceView(getContext());
			surfaceView.setFocusable(false);
			surfaceView.setFocusableInTouchMode(false);
			surfaceView.getHolder().addCallback(this);
			surfaceView.setOnTouchListener(new View.OnTouchListener() {
				@Override
				public boolean onTouch(View view, MotionEvent event) {
					final int action = event.getActionMasked();
					try {
						switch (action) {
							case MotionEvent.ACTION_DOWN:
							case MotionEvent.ACTION_POINTER_DOWN: {
								final int idx = event.getActionIndex();
								GodotLib.secondaryDisplayTouch(event.getX(idx), event.getY(idx),
										TOUCH_DOWN, event.getPointerId(idx));
							} break;
							case MotionEvent.ACTION_UP:
							case MotionEvent.ACTION_POINTER_UP:
							case MotionEvent.ACTION_CANCEL: {
								final int idx = event.getActionIndex();
								GodotLib.secondaryDisplayTouch(event.getX(idx), event.getY(idx),
										TOUCH_UP, event.getPointerId(idx));
							} break;
							case MotionEvent.ACTION_MOVE: {
								// One sample per pointer per batch is plenty for a 5 Hz panel.
								for (int i = 0; i < event.getPointerCount(); ++i) {
									GodotLib.secondaryDisplayTouch(event.getX(i), event.getY(i),
											TOUCH_MOVE, event.getPointerId(i));
								}
							} break;
							default:
								break;
						}
					} catch (UnsatisfiedLinkError e) {
						/* Native library not loaded. */
					}
					return true;
				}
			});
			setContentView(surfaceView);
		}

		/**
		 * Keeps the status/navigation bars off the panel and stops the system from claiming
		 * its left/right edges for the back gesture, which would otherwise swallow the taps
		 * the console reads (and shrink the surface the grid is sized against).
		 */
		private void applyImmersive(Window window) {
			if (window == null) {
				return;
			}
			final View decor = window.getDecorView();
			final int uiOptions = View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY |
					View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
					View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
					View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
					View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
					View.SYSTEM_UI_FLAG_FULLSCREEN;
			decor.setSystemUiVisibility(uiOptions);
			decor.setOnSystemUiVisibilityChangeListener(new View.OnSystemUiVisibilityChangeListener() {
				@Override
				public void onSystemUiVisibilityChange(int visibility) {
					if ((visibility & View.SYSTEM_UI_FLAG_FULLSCREEN) == 0) {
						decor.setSystemUiVisibility(uiOptions);
					}
				}
			});

			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
				decor.post(new Runnable() {
					@Override
					public void run() {
						final int edge = Math.min(200, Math.max(1, decor.getWidth() / 8));
						List<Rect> exclusions = new ArrayList<Rect>(2);
						exclusions.add(new Rect(0, 0, edge, decor.getHeight()));
						exclusions.add(new Rect(decor.getWidth() - edge, 0, decor.getWidth(), decor.getHeight()));
						decor.setSystemGestureExclusionRects(exclusions);
					}
				});
			}
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
