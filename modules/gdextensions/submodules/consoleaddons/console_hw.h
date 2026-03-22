/**************************************************************************/
/*  console_hw.h                                                          */
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

#ifndef CONSOLE_HW_H
#define CONSOLE_HW_H

#include "core/image.h"
#include "core/object.h"
#include "core/variant.h"

class ConsoleHw : public Object {
	GDCLASS(ConsoleHw, Object);

	static ConsoleHw *instance;

protected:
	static void _bind_methods();

public:
	static ConsoleHw *get_singleton();

	// --- Platform identification ---
	String get_platform_name() const;
	Dictionary get_hw_capabilities() const;

	// --- Display/Scaler (Dingux IPU, RGA, VideoCore) ---
	enum ScalerFilter {
		FILTER_NEAREST,
		FILTER_BILINEAR,
		FILTER_BICUBIC,
	};

	bool set_scaler_filter(ScalerFilter p_filter);
	bool set_scaler_keep_aspect(bool p_enable);
	bool set_scaler_integer_scaling(bool p_enable);
	bool set_scaler_downscaling(bool p_enable);

	// --- Backlight ---
	int get_backlight() const;
	void set_backlight(int p_percent);

	// --- Battery ---
	int get_battery_level() const;

	// --- 3DS-specific (WiFi/sensors via ctru) ---
	Error wifi_init();
	void wifi_exit();
	int wifi_get_status();
	String wifi_get_ssid();

	Error sensors_enable_accelerometer();
	Error sensors_disable_accelerometer();
	Error sensors_enable_gyroscope();
	Error sensors_disable_gyroscope();
	Vector3 sensors_read_accelerometer();
	Vector3 sensors_read_gyroscope();

	// --- PSP-specific (VFPU math) ---
	float vfpu_sqrt(float p_value);
	float vfpu_sin(float p_value);
	float vfpu_cos(float p_value);

	// --- Accelerated image operations (Arm-2D kernels) ---
	void image_blit(Ref<Image> p_src, Rect2 p_src_rect, Ref<Image> p_dst, Point2 p_dst_pos);
	void image_blend(Ref<Image> p_src, Rect2 p_src_rect, Ref<Image> p_dst, Point2 p_dst_pos);
	void image_fill_rect(Ref<Image> p_dst, Rect2 p_rect, Color p_color);
	void image_flip_x(Ref<Image> p_img);
	void image_flip_y(Ref<Image> p_img);
	void image_invert_colors(Ref<Image> p_img);
	void image_premultiply_alpha(Ref<Image> p_img);
	bool has_neon() const;

	ConsoleHw();
	~ConsoleHw();
};

VARIANT_ENUM_CAST(ConsoleHw::ScalerFilter);

#endif // CONSOLE_HW_H
