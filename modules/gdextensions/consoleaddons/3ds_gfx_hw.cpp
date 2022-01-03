/*************************************************************************/
/*  3ds_gfx_hw.cpp                                                       */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "3ds_gfx_hw.h"

#include <3ds.h>

#include <nds/arm9/trig_lut.h>
#include <nds/background.h>
#include <nds/sprite.h>

#include "common/current_function.h"

static const int SPRITE_DMA_CHANNEL = 3;

typedef struct {
	int oam_id;
	int width;
	int height;
	int angle;
	SpriteEntry *entry;
} SpriteInfo;

// NdsCtru

#define _CTRU_ASSERT_IPC_OK(result) \
	if (R_FAILED(result)) {         \
		return FAILED;              \
	}

#define _CTRU_WARN_IPC_FAIL(result)                                  \
	if (R_FAILED(result)) {                                          \
		WARN_PRINT(GD_CURRENT_FUNCTION " failed to retrive result"); \
	}

NdsCtru *NdsCtru::get_singleton() {
	if (instance == nullptr) {
		instance = memnew(NdsCtru);
	}
	return instance;
}

Error NdsCtru::ac_init() {
	Result result = acInit();
	return R_FAILED(result) ? FAILED : OK;
}

Error NdsCtru::ac_exit() {
	acExit();
	return OK;
}

Error NdsCtru::ac_wait_internet_connection() {
	Result result = acWaitInternetConnection();
	return R_FAILED(result) ? FAILED : OK;
}

int NdsCtru::acu_get_wifi_status() {
	u32 out = 0;
	Result result = ACU_GetWifiStatus(&out);
	_CTRU_WARN_IPC_FAIL(result);
	return out;
}

int NdsCtru::acu_get_status() {
	u32 out = 0;
	Result result = ACU_GetStatus(&out);
	_CTRU_WARN_IPC_FAIL(result);
	return out;
}

int NdsCtru::acu_get_security_mode() {
	acSecurityMode mode;
	Result result = ACU_GetSecurityMode(&mode);
	_CTRU_WARN_IPC_FAIL(result);
	return mode;
}

String NdsCtru::acu_get_ssid() {
	char *ssid = "";
	Result result = ACU_GetSSID(ssid);
	_CTRU_WARN_IPC_FAIL(result);
	return ssid;
}

int NdsCtru::acu_get_ssid_length() {
	u32 out = -1;
	Result result = ACU_GetSSIDLength(&out);
	_CTRU_WARN_IPC_FAIL(result);
	return out;
}

bool NdsCtru::acu_get_proxy_enable() {
	bool enable = false;
	Result result = ACU_GetProxyEnable(&enable);
	_CTRU_WARN_IPC_FAIL(result);
	return enable;
}

int NdsCtru::acu_get_proxy_port() {
	u32 out = 0;
	Result result = ACU_GetProxyPort(&out);
	_CTRU_WARN_IPC_FAIL(result);
	return out;
}

String NdsCtru::acu_get_proxy_user_name() {
	char *username = "";
	Result result = ACU_GetProxyUserName(username);
	_CTRU_WARN_IPC_FAIL(result);
	return username;
}

String NdsCtru::acu_get_proxy_password() {
	char *password = "";
	Result result = ACU_GetProxyPassword(password);
	_CTRU_WARN_IPC_FAIL(result);
	return password;
}

int NdsCtru::acu_get_last_error_code() {
	u32 error_code = 0;
	Result result = ACU_GetLastErrorCode(&error_code);
	_CTRU_WARN_IPC_FAIL(result);
	return error_code;
}

int NdsCtru::acu_get_last_detail_error_code() {
	u32 error_code = 0;
	Result result = ACU_GetLastDetailErrorCode(&error_code);
	_CTRU_WARN_IPC_FAIL(result);
	return error_code;
}

Error NdsCtru::hid_init() {
	Result result = hidInit();
	return R_FAILED(result) ? FAILED : OK;
}

Error NdsCtru::hid_exit() {
	hidExit();
	return OK;
}

Error NdsCtru::hid_scan_input() {
	hidScanInput();
	return OK;
}

int NdsCtru::hid_keys_held() {
	u32 result = hidKeysHeld();
	return result;
}

int NdsCtru::hid_keys_down() {
	u32 result = hidKeysDown();
	return result;
}

int NdsCtru::hid_keys_up() {
	u32 result = hidKeysUp();
	return result;
}

Point2 NdsCtru::hid_touch_read() {
	touchPosition pos;
	hidTouchRead(&pos);
	return Point2(pos.px, pos.py);
}

Point2 NdsCtru::hid_circle_read() {
	circlePosition pos;
	hidCircleRead(&pos);
	return Point2(pos.dx, pos.dy);
}

Vector3 NdsCtru::hid_accel_read() {
	accelVector vector;
	hidAccelRead(&vector);
	return Vector3(vector.x, vector.y, vector.z);
}

Vector3 NdsCtru::hid_gyro_read() {
	angularRate rate;
	hidGyroRead(&rate);
	return Vector3(rate.x, rate.y, rate.z);
}

Error NdsCtru::hid_wait_for_event(int p_id, bool p_next_event) {
	HID_Event id = (HID_Event)p_id;
	hidWaitForEvent(id, p_next_event);
	return OK;
}

Array NdsCtru::hiduser_get_handles() {
	Handle out_mem_handle, eventpad0, eventpad1, eventaccel, eventgyro, eventdebugpad;
	Result result = HIDUSER_GetHandles(&out_mem_handle, &eventpad0, &eventpad1, &eventaccel, &eventgyro, &eventdebugpad);
	_CTRU_WARN_IPC_FAIL(result);
	return varray(out_mem_handle, eventpad0, eventpad1, eventaccel, eventgyro, eventdebugpad);
}

Error NdsCtru::ctru_hiduser_enable_accelerometer() {
	Result result = HIDUSER_EnableAccelerometer();
	return R_FAILED(result) ? FAILED : OK;
}

Error NdsCtru::ctru_hiduser_disable_accelerometer() {
	Result result = HIDUSER_DisableAccelerometer();
	return R_FAILED(result) ? FAILED : OK;
}

NdsCtru::hiduser_enable_gyroscope() {
	Result result = HIDUSER_EnableGyroscope();
	return R_FAILED(result) ? FAILED : OK;
}

NdsCtru::hiduser_disable_gyroscope() {
	Result result = HIDUSER_DisableGyroscope();
	return R_FAILED(result) ? FAILED : OK;
}

float NdsCtru::hiduser_get_gyroscope_raw_to_dps_coefficient() {
	float coeff = 0;
	Result result = HIDUSER_GetGyroscopeRawToDpsCoefficient(&coeff);
	_CTRU_WARN_IPC_FAIL(result);
	return coeff;
}

int NdsCtru::hiduser_get_sound_volume() {
	u8 volume;
	Result result = HIDUSER_GetSoundVolume(&volume);
	_CTRU_WARN_IPC_FAIL(result);
	return volume;
}

// NdsSprite

void NdsSprite::_update_oam(OAMTable *p_oam) {
	DC_FlushAll();
	dmaCopyHalfWords(SPRITE_DMA_CHANNEL, p_oam->oamBuffer, OAM, SPRITE_COUNT * sizeof(SpriteEntry));
}

void NdsSprite::_init_oam(OAMTable *p_oam) {
	for (int i = 0; i < SPRITE_COUNT; i++) {
		p_oam->oamBuffer[i].attribute[0] = ATTR0_DISABLED;
		p_oam->oamBuffer[i].attribute[1] = 0;
		p_oam->oamBuffer[i].attribute[2] = 0;
	}
	for (int i = 0; i < MATRIX_COUNT; i++) {
		p_oam->matrixBuffer[i].hdx = 1 << 8;
		p_oam->matrixBuffer[i].hdy = 0;
		p_oam->matrixBuffer[i].vdx = 0;
		p_oam->matrixBuffer[i].vdy = 1 << 8;
	}
	_update_oam(oam);
}

void NdsSprite::set_visibility(SpriteEntry *p_sprite_entry, bool p_hidden, bool p_affine, bool p_double_bound) {
	if (p_hidden) {
		p_sprite_entry->isRotateScale = false; // Bit 9 off
		p_sprite_entry->isHidden = true; // Bit 8 on
	} else {
		if (p_affine) {
			p_sprite_entry->isRotateScale = true;
			p_sprite_entry->isSizeDouble = p_double_bound;
		} else {
			p_sprite_entry->isHidden = false;
		}
	}
}
