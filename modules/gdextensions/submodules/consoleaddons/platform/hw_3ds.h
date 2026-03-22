/**************************************************************************/
/*  hw_3ds.h                                                              */
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

#ifndef HW_3DS_H
#define HW_3DS_H

#include "core/error_list.h"
#include "core/math/vector3.h"
#include "core/ustring.h"

#ifdef _3DS
#include <3ds.h>

#define _CTRU_ASSERT_IPC_OK(result) \
	if (R_FAILED(result)) {         \
		return FAILED;              \
	}

static Error hw_3ds_wifi_init() {
	Result result = acInit();
	if (R_FAILED(result)) {
		return FAILED;
	}
	result = acWaitInternetConnection();
	return R_FAILED(result) ? FAILED : OK;
}

static void hw_3ds_wifi_exit() {
	acExit();
}

static int hw_3ds_wifi_get_status() {
	u32 out = 0;
	ACU_GetWifiStatus(&out);
	return out;
}

static String hw_3ds_wifi_get_ssid() {
	char ssid_buf[33] = {};
	ACU_GetSSID(ssid_buf);
	return String(ssid_buf);
}

static int hw_3ds_get_battery_level() {
	u8 level = 0;
	MCUHWC_GetBatteryLevel(&level);
	return (int)level;
}

static Error hw_3ds_enable_accelerometer() {
	Result result = HIDUSER_EnableAccelerometer();
	return R_FAILED(result) ? FAILED : OK;
}

static Error hw_3ds_disable_accelerometer() {
	Result result = HIDUSER_DisableAccelerometer();
	return R_FAILED(result) ? FAILED : OK;
}

static Error hw_3ds_enable_gyroscope() {
	Result result = HIDUSER_EnableGyroscope();
	return R_FAILED(result) ? FAILED : OK;
}

static Error hw_3ds_disable_gyroscope() {
	Result result = HIDUSER_DisableGyroscope();
	return R_FAILED(result) ? FAILED : OK;
}

static Vector3 hw_3ds_read_accelerometer() {
	accelVector vector;
	hidAccelRead(&vector);
	return Vector3(vector.x, vector.y, vector.z);
}

static Vector3 hw_3ds_read_gyroscope() {
	angularRate rate;
	hidGyroRead(&rate);
	return Vector3(rate.x, rate.y, rate.z);
}

#else // !_3DS — stubs

static Error hw_3ds_wifi_init() { return ERR_UNAVAILABLE; }
static void hw_3ds_wifi_exit() {}
static int hw_3ds_wifi_get_status() { return -1; }
static String hw_3ds_wifi_get_ssid() { return String(); }
static int hw_3ds_get_battery_level() { return -1; }
static Error hw_3ds_enable_accelerometer() { return ERR_UNAVAILABLE; }
static Error hw_3ds_disable_accelerometer() { return ERR_UNAVAILABLE; }
static Error hw_3ds_enable_gyroscope() { return ERR_UNAVAILABLE; }
static Error hw_3ds_disable_gyroscope() { return ERR_UNAVAILABLE; }
static Vector3 hw_3ds_read_accelerometer() { return Vector3(); }
static Vector3 hw_3ds_read_gyroscope() { return Vector3(); }

#endif // _3DS

#endif // HW_3DS_H
