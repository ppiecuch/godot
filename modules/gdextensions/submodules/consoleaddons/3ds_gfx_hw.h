/**************************************************************************/
/*  3ds_gfx_hw.h                                                          */
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

#ifndef NDS_GFX_HW_H
#define NDS_GFX_HW_H

#include "core/array.h"
#include "scene/2d/node_2d.h"

struct OAMTable;
struct SpriteEntry;

enum NdsScreen {
	SCREEN_TOP,
	SCREEN_BOTTOM,
};

enum NdsMode5Background {
	MODE5_BG0,
	MODE5_BG1,
	MODE5_BG2,
	MODE5_BG3,
};

enum NdsBackgroundBitmapSize {
	BITMAP_128x128,
	BITMAP_256x256,
	BITMAP_512x256,
	BITMAP_512x512,
};

class NdsCtru : public Object {
	GDCLASS(NdsCtru, Object)

	static NdsCtru *instance = nullptr;

protected:
	NdsCtru();

public:
	NdsCtru *get_singleton();

	/* ac */
	Error ac_init(); /* Initializes AC */
	Error ac_exit(); /* Exits AC */
	Error ac_wait_internet_connection(); /* Waits for the system to connect to the internet */
	/* wifi */
	int acu_get_wifi_status(); /* Gets the connected Wifi status */
	int acu_get_status(); /* "Gets the connected Wifi status */
	int acu_get_security_mode(); /* Gets the connected Wifi security mode */
	String acu_get_ssid(); /* Gets the connected Wifi SSID */
	int acu_get_ssid_length(); /* Gets the connected Wifi SSID length */
	bool acu_get_proxy_enable(); /* Determines whether proxy is enabled for the connected network */
	int acu_get_proxy_port(); /* Gets the connected network's proxy port */
	String acu_get_proxy_user_name(); /* Gets the connected network's proxy username */
	String acu_get_proxy_password(); /* Gets the connected network's proxy password */
	int acu_get_last_error_code(); /* Gets the last error to occur during a connection */
	int acu_get_last_detail_error_code(); /* Gets the last detailed error to occur during a connection */
	/* hid */
	Error hid_init(); /* Initializes HID */
	Error hid_exit(); /* Exits HID */
	int hid_scan_input(); /* Scans HID for input data */
	int hid_keys_held(); /* Returns a bitmask of held buttons */
	int hid_keys_down(); /* Returns a bitmask of newly pressed buttons, this frame */
	int hid_keys_up(); /* Returns a bitmask of newly released buttons, this frame */
	Point2 hid_touch_read(); /* Reads the current touch position */
	Point2 hid_circle_read(); /* Reads the current circle pad position */
	Vector3 hid_accel_read(); /* Reads the current accelerometer data */
	Vector3 hid_gyro_read(); /* Reads the current gyroscope data */
	int hid_wait_for_event(int p_id, bool p_next_event); /* Waits for an HID event */
	Array hiduser_get_handles(); /* Gets the handles for HID operation */
	Error hiduser_enable_accelerometer(); /* Enables the accelerometer */
	Error hiduser_disable_accelerometer(); /* Disables the accelerometer */
	Error hiduser_enable_gyroscope(); /* Enables the gyroscope */
	Error hiduser_disable_gyroscope(); /* Disables the gyroscope */
	float hiduser_get_gyroscope_raw_to_dps_coefficient(); /* Gets the gyroscope raw to dps coefficient */
	int hiduser_get_sound_volume(); /* Gets the current volume slider value */
};

class NdsSprite : public Node2D {
	GDCLASS(NdsSprite, Node2D);

	int oam_id;

	void _update_oam(OAMTable *p_oam);
	void _init_oam(OAMTable *p_oam);
	void _set_visibility(SpriteEntry *p_sprite_entry, bool p_hidden, bool p_affine, bool p_double_bound);

public:
	NdsSprite();
	~NdsSprite();
};

class NdsBackground : public Node2D {
	GDCLASS(NdsSprite, Node2D);

public:
	NdsBackground();
	~NdsBackground();
};

#endif // NDS_GFX_HW_H
