/**************************************************************************/
/*  GodotBackgroundService.java                                           */
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

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ServiceInfo;
import android.os.Build;
import android.os.IBinder;
import android.util.Log;

import androidx.annotation.Nullable;
import androidx.core.app.NotificationCompat;

/**
 * Foreground service that keeps the Godot application alive when the activity
 * is in the background. Modeled after the background work pattern in
 * playcorenative/android.cpp (APP_WORK_IN_BACKGROUND).
 *
 * <p>On Android 8.0+ a foreground service with a visible notification is
 * required to keep the process alive when the activity is not visible.
 * This service creates a notification channel and posts a persistent
 * notification for the duration of background work.</p>
 *
 * <p>Usage from Godot.java:
 * <pre>
 *   GodotBackgroundService.start(context, "My app is running");
 *   GodotBackgroundService.stop(context);
 *   GodotBackgroundService.isRunning();
 * </pre>
 */
public class GodotBackgroundService extends Service {
	private static final String TAG = "GodotBgService";
	private static final String CHANNEL_ID = "godot_background_channel";
	private static final int NOTIFICATION_ID = 0x600D07; // "GODOT" in hex-ish
	private static final String EXTRA_NOTIFICATION_TEXT = "notification_text";

	private static volatile boolean running = false;

	// --- Static convenience API (called from Godot.java) ---

	/**
	 * Start the background service with a notification message.
	 * On Android O+ this uses startForegroundService(); on older versions
	 * it falls back to startService().
	 */
	public static void start(Context context, String notificationText) {
		if (running) {
			Log.w(TAG, "Background service already running");
			return;
		}
		Intent intent = new Intent(context, GodotBackgroundService.class);
		intent.putExtra(EXTRA_NOTIFICATION_TEXT, notificationText);
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
			context.startForegroundService(intent);
		} else {
			context.startService(intent);
		}
	}

	/**
	 * Stop the background service.
	 */
	public static void stop(Context context) {
		if (!running) {
			return;
		}
		Intent intent = new Intent(context, GodotBackgroundService.class);
		context.stopService(intent);
	}

	/**
	 * Check whether the background service is currently running.
	 */
	public static boolean isRunning() {
		return running;
	}

	// --- Service lifecycle ---

	@Override
	public void onCreate() {
		super.onCreate();
		Log.i(TAG, "Background service created");
		createNotificationChannel();
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		String text = "App running in background";
		if (intent != null && intent.hasExtra(EXTRA_NOTIFICATION_TEXT)) {
			text = intent.getStringExtra(EXTRA_NOTIFICATION_TEXT);
		}

		Notification notification = buildNotification(text);

		// startForeground() must be called within 5 seconds of startForegroundService()
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.UPSIDE_DOWN_CAKE) {
			// Android 14+ requires specifying the foreground service type
			startForeground(NOTIFICATION_ID, notification,
					ServiceInfo.FOREGROUND_SERVICE_TYPE_SPECIAL_USE);
		} else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
			startForeground(NOTIFICATION_ID, notification,
					ServiceInfo.FOREGROUND_SERVICE_TYPE_NONE);
		} else {
			startForeground(NOTIFICATION_ID, notification);
		}

		running = true;
		Log.i(TAG, "Background service started: " + text);

		// If killed by the system, restart with the last intent
		return START_REDELIVER_INTENT;
	}

	@Override
	public void onDestroy() {
		running = false;
		Log.i(TAG, "Background service destroyed");
		super.onDestroy();
	}

	@Nullable
	@Override
	public IBinder onBind(Intent intent) {
		// Not a bound service
		return null;
	}

	// --- Notification helpers ---

	private void createNotificationChannel() {
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
			NotificationChannel channel = new NotificationChannel(
					CHANNEL_ID,
					"Godot Background",
					NotificationManager.IMPORTANCE_LOW);
			channel.setDescription("Keeps the Godot application alive in the background");
			channel.setShowBadge(false);
			channel.enableLights(false);
			channel.enableVibration(false);

			NotificationManager nm = getSystemService(NotificationManager.class);
			if (nm != null) {
				nm.createNotificationChannel(channel);
			}
		}
	}

	private Notification buildNotification(String text) {
		// Build a PendingIntent that brings the user back to the app's main activity
		Intent launchIntent = getPackageManager().getLaunchIntentForPackage(getPackageName());
		int pendingFlags = PendingIntent.FLAG_UPDATE_CURRENT;
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
			pendingFlags |= PendingIntent.FLAG_IMMUTABLE;
		}
		PendingIntent pendingIntent = (launchIntent != null)
				? PendingIntent.getActivity(this, 0, launchIntent, pendingFlags)
				: null;

		// Use the application icon as the notification icon.
		// Falls back to the Godot default icon if the app doesn't provide one.
		int iconRes = getApplicationInfo().icon;
		if (iconRes == 0) {
			iconRes = R.mipmap.icon;
		}

		NotificationCompat.Builder builder = new NotificationCompat.Builder(this, CHANNEL_ID)
													 .setSmallIcon(iconRes)
													 .setContentTitle(getApplicationLabel())
													 .setContentText(text)
													 .setPriority(NotificationCompat.PRIORITY_LOW)
													 .setCategory(NotificationCompat.CATEGORY_SERVICE)
													 .setOngoing(true)
													 .setShowWhen(false);

		if (pendingIntent != null) {
			builder.setContentIntent(pendingIntent);
		}

		return builder.build();
	}

	private String getApplicationLabel() {
		try {
			return getPackageManager()
					.getApplicationLabel(getApplicationInfo())
					.toString();
		} catch (Exception e) {
			return "Godot";
		}
	}

	/**
	 * Update the notification text while the service is running.
	 */
	public static void updateNotification(Context context, String newText) {
		if (!running)
			return;

		// Re-post the notification with updated text through the service
		Intent intent = new Intent(context, GodotBackgroundService.class);
		intent.putExtra(EXTRA_NOTIFICATION_TEXT, newText);
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
			context.startForegroundService(intent);
		} else {
			context.startService(intent);
		}
	}
}
