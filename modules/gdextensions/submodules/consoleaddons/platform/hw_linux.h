/**************************************************************************/
/*  hw_linux.h                                                            */
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

#ifndef HW_LINUX_H
#define HW_LINUX_H

// Linux embedded hardware acceleration helpers.
// Wraps Dingux IPU scaler, ODROID Go backlight, and battery sysfs access.
// Only functional on actual Linux embedded devices; stubs on other platforms.

#ifdef __linux__
#include "core/os/file_access.h"

// --- IPU sysfs paths ---
#define DINGUX_ALLOW_DOWNSCALING_FILE "/sys/devices/platform/jz-lcd.0/allow_downscaling"
#define DINGUX_KEEP_ASPECT_RATIO_FILE "/sys/devices/platform/jz-lcd.0/keep_aspect_ratio"
#define DINGUX_INTEGER_SCALING_FILE "/sys/devices/platform/jz-lcd.0/integer_scaling"
#define DINGUX_SHARPNESS_UPSCALING_FILE "/sys/devices/platform/jz-lcd.0/sharpness_upscaling"
#define DINGUX_SHARPNESS_DOWNSCALING_FILE "/sys/devices/platform/jz-lcd.0/sharpness_downscaling"
#define DINGUX_BATTERY_CAPACITY_FILE "/sys/class/power_supply/battery/capacity"

// --- Backlight sysfs paths ---
#define BACKLIGHT_BRIGHTNESS_FILE "/sys/class/backlight/backlight/brightness"
#define BACKLIGHT_BRIGHTNESS_MAX_FILE "/sys/class/backlight/backlight/max_brightness"

static bool _hw_linux_write_sysfs(const char *path, const char *data, size_t data_size) {
	FileAccessRef fa(FileAccess::open(path, FileAccess::READ_WRITE));
	if (fa) {
		fa->store_buffer((const uint8_t *)data, data_size);
		return true;
	}
	return false;
}

static int _hw_linux_read_sysfs_int(const char *path) {
	FileAccessRef fa(FileAccess::open(path, FileAccess::READ));
	if (fa) {
		String line = fa->get_line();
		if (!line.empty()) {
			return line.to_int();
		}
	}
	return -1;
}

static bool hw_linux_has_ipu_scaler() {
	return FileAccess::exists(DINGUX_KEEP_ASPECT_RATIO_FILE);
}

static bool hw_linux_has_backlight() {
	return FileAccess::exists(BACKLIGHT_BRIGHTNESS_FILE);
}

static bool hw_linux_has_battery() {
	return FileAccess::exists(DINGUX_BATTERY_CAPACITY_FILE);
}

static bool hw_linux_set_scaler_filter(int p_filter) {
	// filter: 0=nearest, 1=bilinear, 2=bicubic
	const char *sharpness_str = "8"; // default bicubic
	if (p_filter == 0) {
		sharpness_str = "0";
	} else if (p_filter == 1) {
		sharpness_str = "1";
	}
	if (!FileAccess::exists(DINGUX_SHARPNESS_UPSCALING_FILE)) {
		return false;
	}
	bool ok = _hw_linux_write_sysfs(DINGUX_SHARPNESS_UPSCALING_FILE, sharpness_str, 1);
	if (FileAccess::exists(DINGUX_SHARPNESS_DOWNSCALING_FILE)) {
		ok = ok && _hw_linux_write_sysfs(DINGUX_SHARPNESS_DOWNSCALING_FILE, sharpness_str, 1);
	}
	return ok;
}

static bool hw_linux_set_scaler_keep_aspect(bool p_enable) {
	if (!FileAccess::exists(DINGUX_KEEP_ASPECT_RATIO_FILE)) {
		return false;
	}
	const char *val = p_enable ? "1" : "0";
	return _hw_linux_write_sysfs(DINGUX_KEEP_ASPECT_RATIO_FILE, val, 1);
}

static bool hw_linux_set_scaler_integer_scaling(bool p_enable) {
	if (!FileAccess::exists(DINGUX_INTEGER_SCALING_FILE)) {
		return false;
	}
	const char *val = p_enable ? "1" : "0";
	return _hw_linux_write_sysfs(DINGUX_INTEGER_SCALING_FILE, val, 1);
}

static bool hw_linux_set_scaler_downscaling(bool p_enable) {
	if (!FileAccess::exists(DINGUX_ALLOW_DOWNSCALING_FILE)) {
		return false;
	}
	const char *val = p_enable ? "1" : "0";
	return _hw_linux_write_sysfs(DINGUX_ALLOW_DOWNSCALING_FILE, val, 1);
}

static int hw_linux_get_backlight() {
	int max_val = _hw_linux_read_sysfs_int(BACKLIGHT_BRIGHTNESS_MAX_FILE);
	if (max_val <= 0) {
		return -1;
	}
	int cur_val = _hw_linux_read_sysfs_int(BACKLIGHT_BRIGHTNESS_FILE);
	if (cur_val < 0) {
		return -1;
	}
	return (int)((float)cur_val / (float)max_val * 100.0f);
}

static void hw_linux_set_backlight(int p_percent) {
	if (p_percent < 0)
		p_percent = 0;
	if (p_percent > 100)
		p_percent = 100;
	int max_val = _hw_linux_read_sysfs_int(BACKLIGHT_BRIGHTNESS_MAX_FILE);
	if (max_val <= 0) {
		return;
	}
	int brightness = (int)((float)p_percent / 100.0f * (float)max_val);
	char buf[16];
	snprintf(buf, sizeof(buf), "%d", brightness);
	_hw_linux_write_sysfs(BACKLIGHT_BRIGHTNESS_FILE, buf, strlen(buf));
}

static int hw_linux_get_battery_level() {
	return _hw_linux_read_sysfs_int(DINGUX_BATTERY_CAPACITY_FILE);
}

#else // !__linux__ — stubs

static bool hw_linux_has_ipu_scaler() { return false; }
static bool hw_linux_has_backlight() { return false; }
static bool hw_linux_has_battery() { return false; }
static bool hw_linux_set_scaler_filter(int p_filter) { return false; }
static bool hw_linux_set_scaler_keep_aspect(bool p_enable) { return false; }
static bool hw_linux_set_scaler_integer_scaling(bool p_enable) { return false; }
static bool hw_linux_set_scaler_downscaling(bool p_enable) { return false; }
static int hw_linux_get_backlight() { return -1; }
static void hw_linux_set_backlight(int p_percent) {}
static int hw_linux_get_battery_level() { return -1; }

#endif // __linux__

#endif // HW_LINUX_H
