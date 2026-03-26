/**************************************************************************/
/*  console_hw.cpp                                                        */
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

#include "console_hw.h"

#include "platform/hw_3ds.h"
#include "platform/hw_linux.h"
#include "platform/hw_psp.h"
#include "platform/hw_psvita.h"

ConsoleHw *ConsoleHw::instance = nullptr;

ConsoleHw *ConsoleHw::get_singleton() {
	if (!instance) {
		instance = memnew(ConsoleHw);
	}
	return instance;
}

// --- Platform identification ---

String ConsoleHw::get_platform_name() const {
#if defined(_3DS)
	return "3ds";
#elif defined(PSP)
	return "psp";
#elif defined(PSVITA)
	return "psvita";
#elif defined(__linux__)
	return "linux";
#elif defined(__APPLE__)
	return "macos";
#elif defined(_WIN32)
	return "windows";
#else
	return "unknown";
#endif
}

Dictionary ConsoleHw::get_hw_capabilities() const {
	Dictionary caps;
#if defined(_3DS)
	caps["has_gpu_2d"] = true;
	caps["has_hw_scaler"] = false;
	caps["has_wifi"] = true;
	caps["has_accelerometer"] = true;
	caps["has_gyroscope"] = true;
	caps["has_vfpu"] = false;
	caps["has_backlight"] = true;
	caps["has_battery"] = true;
#elif defined(PSP)
	caps["has_gpu_2d"] = true;
	caps["has_hw_scaler"] = false;
	caps["has_wifi"] = true;
	caps["has_accelerometer"] = false;
	caps["has_gyroscope"] = false;
	caps["has_vfpu"] = true;
	caps["has_backlight"] = true;
	caps["has_battery"] = true;
#elif defined(PSVITA)
	caps["has_gpu_2d"] = true;
	caps["has_hw_scaler"] = false;
	caps["has_wifi"] = true;
	caps["has_accelerometer"] = true;
	caps["has_gyroscope"] = true;
	caps["has_vfpu"] = false;
	caps["has_backlight"] = true;
	caps["has_battery"] = true;
#elif defined(__linux__)
	caps["has_gpu_2d"] = false;
	caps["has_hw_scaler"] = hw_linux_has_ipu_scaler();
	caps["has_wifi"] = false;
	caps["has_accelerometer"] = false;
	caps["has_gyroscope"] = false;
	caps["has_vfpu"] = false;
	caps["has_backlight"] = hw_linux_has_backlight();
	caps["has_battery"] = hw_linux_has_battery();
#else
	caps["has_gpu_2d"] = false;
	caps["has_hw_scaler"] = false;
	caps["has_wifi"] = false;
	caps["has_accelerometer"] = false;
	caps["has_gyroscope"] = false;
	caps["has_vfpu"] = false;
	caps["has_backlight"] = false;
	caps["has_battery"] = false;
#endif
	return caps;
}

// --- Display/Scaler ---

bool ConsoleHw::set_scaler_filter(ScalerFilter p_filter) {
#if defined(__linux__)
	return hw_linux_set_scaler_filter((int)p_filter);
#else
	return false;
#endif
}

bool ConsoleHw::set_scaler_keep_aspect(bool p_enable) {
#if defined(__linux__)
	return hw_linux_set_scaler_keep_aspect(p_enable);
#else
	return false;
#endif
}

bool ConsoleHw::set_scaler_integer_scaling(bool p_enable) {
#if defined(__linux__)
	return hw_linux_set_scaler_integer_scaling(p_enable);
#else
	return false;
#endif
}

bool ConsoleHw::set_scaler_downscaling(bool p_enable) {
#if defined(__linux__)
	return hw_linux_set_scaler_downscaling(p_enable);
#else
	return false;
#endif
}

// --- Backlight ---

int ConsoleHw::get_backlight() const {
#if defined(__linux__)
	return hw_linux_get_backlight();
#elif defined(PSVITA)
	return hw_psvita_get_backlight();
#else
	return -1;
#endif
}

void ConsoleHw::set_backlight(int p_percent) {
#if defined(__linux__)
	hw_linux_set_backlight(p_percent);
#elif defined(PSVITA)
	hw_psvita_set_backlight(p_percent);
#endif
}

// --- Battery ---

int ConsoleHw::get_battery_level() const {
#if defined(__linux__)
	return hw_linux_get_battery_level();
#elif defined(PSVITA)
	return hw_psvita_get_battery_level();
#elif defined(_3DS)
	return hw_3ds_get_battery_level();
#else
	return -1;
#endif
}

// --- 3DS WiFi/sensors ---

Error ConsoleHw::wifi_init() {
#if defined(_3DS)
	return hw_3ds_wifi_init();
#else
	return ERR_UNAVAILABLE;
#endif
}

void ConsoleHw::wifi_exit() {
#if defined(_3DS)
	hw_3ds_wifi_exit();
#endif
}

int ConsoleHw::wifi_get_status() {
#if defined(_3DS)
	return hw_3ds_wifi_get_status();
#else
	return -1;
#endif
}

String ConsoleHw::wifi_get_ssid() {
#if defined(_3DS)
	return hw_3ds_wifi_get_ssid();
#else
	return String();
#endif
}

Error ConsoleHw::sensors_enable_accelerometer() {
#if defined(_3DS)
	return hw_3ds_enable_accelerometer();
#else
	return ERR_UNAVAILABLE;
#endif
}

Error ConsoleHw::sensors_disable_accelerometer() {
#if defined(_3DS)
	return hw_3ds_disable_accelerometer();
#else
	return ERR_UNAVAILABLE;
#endif
}

Error ConsoleHw::sensors_enable_gyroscope() {
#if defined(_3DS)
	return hw_3ds_enable_gyroscope();
#else
	return ERR_UNAVAILABLE;
#endif
}

Error ConsoleHw::sensors_disable_gyroscope() {
#if defined(_3DS)
	return hw_3ds_disable_gyroscope();
#else
	return ERR_UNAVAILABLE;
#endif
}

Vector3 ConsoleHw::sensors_read_accelerometer() {
#if defined(_3DS)
	return hw_3ds_read_accelerometer();
#else
	return Vector3();
#endif
}

Vector3 ConsoleHw::sensors_read_gyroscope() {
#if defined(_3DS)
	return hw_3ds_read_gyroscope();
#else
	return Vector3();
#endif
}

// --- PSP VFPU ---

float ConsoleHw::vfpu_sqrt(float p_value) {
#if defined(PSP)
	return hw_psp_vfpu_sqrt(p_value);
#else
	return Math::sqrt(p_value);
#endif
}

float ConsoleHw::vfpu_sin(float p_value) {
#if defined(PSP)
	return hw_psp_vfpu_sin(p_value);
#else
	return Math::sin(p_value);
#endif
}

float ConsoleHw::vfpu_cos(float p_value) {
#if defined(PSP)
	return hw_psp_vfpu_cos(p_value);
#else
	return Math::cos(p_value);
#endif
}

// --- Accelerated image operations ---

#include "acc/arm_2d.h"

bool ConsoleHw::has_neon() const {
#if ARM2D_HAS_NEON
	return true;
#else
	return false;
#endif
}

void ConsoleHw::image_blit(Ref<Image> p_src, Rect2 p_src_rect, Ref<Image> p_dst, Point2 p_dst_pos) {
	ERR_FAIL_COND(p_src.is_null());
	ERR_FAIL_COND(p_dst.is_null());
	ERR_FAIL_COND(p_src->get_format() != p_dst->get_format());
	ERR_FAIL_COND_MSG(p_dst->is_compressed(), "Cannot blit to compressed image formats.");

	Rect2i src_rect = Rect2i(0, 0, p_src->get_width(), p_src->get_height()).clip(p_src_rect);
	Rect2i dest_rect = Rect2i(Point2i(p_dst_pos), src_rect.size);
	dest_rect = Rect2i(0, 0, p_dst->get_width(), p_dst->get_height()).clip(dest_rect);
	if (src_rect.has_no_area() || dest_rect.has_no_area()) {
		return;
	}
	src_rect.size = dest_rect.size;

	int pixel_size = Image::get_format_pixel_size(p_src->get_format());
	int src_stride = p_src->get_width() * pixel_size;
	int dst_stride = p_dst->get_width() * pixel_size;

	p_dst->lock();

	const uint8_t *src_ptr = p_src->get_raw_cptr() + (src_rect.position.y * p_src->get_width() + src_rect.position.x) * pixel_size;
	uint8_t *dst_ptr = p_dst->get_raw_ptr() + (dest_rect.position.y * p_dst->get_width() + dest_rect.position.x) * pixel_size;

	arm2d_blit_rect(src_ptr, src_stride, dst_ptr, dst_stride, dest_rect.size.x, dest_rect.size.y, pixel_size);

	p_dst->unlock();
}

void ConsoleHw::image_blend(Ref<Image> p_src, Rect2 p_src_rect, Ref<Image> p_dst, Point2 p_dst_pos) {
	ERR_FAIL_COND(p_src.is_null());
	ERR_FAIL_COND(p_dst.is_null());
	ERR_FAIL_COND(p_src->get_format() != p_dst->get_format());

	Image::Format fmt = p_src->get_format();
	ERR_FAIL_COND_MSG(fmt != Image::FORMAT_RGBA8, "Accelerated blend only supports RGBA8 format.");

	Rect2i src_rect = Rect2i(0, 0, p_src->get_width(), p_src->get_height()).clip(p_src_rect);
	Rect2i dest_rect = Rect2i(Point2i(p_dst_pos), src_rect.size);
	dest_rect = Rect2i(0, 0, p_dst->get_width(), p_dst->get_height()).clip(dest_rect);
	if (src_rect.has_no_area() || dest_rect.has_no_area()) {
		return;
	}
	src_rect.size = dest_rect.size;

	int pixel_size = 4; // RGBA8
	int src_stride = p_src->get_width() * pixel_size;
	int dst_stride = p_dst->get_width() * pixel_size;

	p_dst->lock();

	const uint8_t *src_ptr = p_src->get_raw_cptr() + (src_rect.position.y * p_src->get_width() + src_rect.position.x) * pixel_size;
	uint8_t *dst_ptr = p_dst->get_raw_ptr() + (dest_rect.position.y * p_dst->get_width() + dest_rect.position.x) * pixel_size;

	arm2d_blend_rect_rgba8(src_ptr, src_stride, dst_ptr, dst_stride, dest_rect.size.x, dest_rect.size.y);

	p_dst->unlock();
}

void ConsoleHw::image_fill_rect(Ref<Image> p_dst, Rect2 p_rect, Color p_color) {
	ERR_FAIL_COND(p_dst.is_null());
	ERR_FAIL_COND_MSG(p_dst->is_compressed(), "Cannot fill compressed image formats.");

	Rect2i r = Rect2i(0, 0, p_dst->get_width(), p_dst->get_height()).clip(p_rect.abs());
	if (r.has_no_area()) {
		return;
	}

	Image::Format fmt = p_dst->get_format();
	int pixel_size = Image::get_format_pixel_size(fmt);
	int dst_stride = p_dst->get_width() * pixel_size;

	// Encode color to pixel bytes
	uint8_t color_bytes[16] = {};
	if (fmt == Image::FORMAT_RGBA8) {
		color_bytes[0] = CLAMP((int)(p_color.r * 255.0f), 0, 255);
		color_bytes[1] = CLAMP((int)(p_color.g * 255.0f), 0, 255);
		color_bytes[2] = CLAMP((int)(p_color.b * 255.0f), 0, 255);
		color_bytes[3] = CLAMP((int)(p_color.a * 255.0f), 0, 255);
	} else if (fmt == Image::FORMAT_RGB8) {
		color_bytes[0] = CLAMP((int)(p_color.r * 255.0f), 0, 255);
		color_bytes[1] = CLAMP((int)(p_color.g * 255.0f), 0, 255);
		color_bytes[2] = CLAMP((int)(p_color.b * 255.0f), 0, 255);
	} else if (fmt == Image::FORMAT_L8 || fmt == Image::FORMAT_R8) {
		color_bytes[0] = CLAMP((int)(p_color.r * 255.0f), 0, 255);
	} else if (fmt == Image::FORMAT_LA8 || fmt == Image::FORMAT_RG8) {
		color_bytes[0] = CLAMP((int)(p_color.r * 255.0f), 0, 255);
		color_bytes[1] = CLAMP((int)(p_color.g * 255.0f), 0, 255);
	} else {
		p_dst->fill_rect(p_rect, p_color);
		return;
	}

	p_dst->lock();
	uint8_t *dst_ptr = p_dst->get_raw_ptr() + (r.position.y * p_dst->get_width() + r.position.x) * pixel_size;
	arm2d_fill_rect(dst_ptr, dst_stride, color_bytes, pixel_size, r.size.x, r.size.y);
	p_dst->unlock();
}

void ConsoleHw::image_flip_x(Ref<Image> p_img) {
	ERR_FAIL_COND(p_img.is_null());
	ERR_FAIL_COND_MSG(p_img->is_compressed(), "Cannot flip compressed image formats.");

	int w = p_img->get_width();
	int h = p_img->get_height();
	int pixel_size = Image::get_format_pixel_size(p_img->get_format());

	p_img->lock();
	uint8_t *data_ptr = p_img->get_raw_ptr();

	for (int y = 0; y < h; y++) {
		uint8_t *row = data_ptr + y * w * pixel_size;
		if (pixel_size == 4) {
			arm2d_reverse_row_4bpp(row, w);
		} else {
			arm2d_reverse_row(row, w, pixel_size);
		}
	}
	p_img->unlock();
}

void ConsoleHw::image_flip_y(Ref<Image> p_img) {
	ERR_FAIL_COND(p_img.is_null());
	ERR_FAIL_COND_MSG(p_img->is_compressed(), "Cannot flip compressed image formats.");

	int w = p_img->get_width();
	int h = p_img->get_height();
	int pixel_size = Image::get_format_pixel_size(p_img->get_format());
	int row_bytes = w * pixel_size;

	p_img->lock();
	uint8_t *data_ptr = p_img->get_raw_ptr();

	for (int y = 0; y < h / 2; y++) {
		uint8_t *row_top = data_ptr + y * row_bytes;
		uint8_t *row_bot = data_ptr + (h - 1 - y) * row_bytes;
		arm2d_swap_rows(row_top, row_bot, row_bytes);
	}
	p_img->unlock();
}

void ConsoleHw::image_invert_colors(Ref<Image> p_img) {
	ERR_FAIL_COND(p_img.is_null());
	ERR_FAIL_COND_MSG(p_img->get_format() != Image::FORMAT_RGBA8, "Accelerated invert only supports RGBA8 format.");

	int pixel_count = p_img->get_width() * p_img->get_height();
	p_img->lock();
	arm2d_invert_colors_rgba8(p_img->get_raw_ptr(), pixel_count);
	p_img->unlock();
}

void ConsoleHw::image_premultiply_alpha(Ref<Image> p_img) {
	ERR_FAIL_COND(p_img.is_null());
	ERR_FAIL_COND_MSG(p_img->get_format() != Image::FORMAT_RGBA8, "Accelerated premultiply only supports RGBA8 format.");

	int pixel_count = p_img->get_width() * p_img->get_height();
	p_img->lock();
	arm2d_premultiply_alpha_rgba8(p_img->get_raw_ptr(), pixel_count);
	p_img->unlock();
}

// --- GDScript bindings ---

void ConsoleHw::_bind_methods() {
	// Platform
	ClassDB::bind_method(D_METHOD("get_platform_name"), &ConsoleHw::get_platform_name);
	ClassDB::bind_method(D_METHOD("get_hw_capabilities"), &ConsoleHw::get_hw_capabilities);

	// Scaler
	ClassDB::bind_method(D_METHOD("set_scaler_filter", "filter"), &ConsoleHw::set_scaler_filter);
	ClassDB::bind_method(D_METHOD("set_scaler_keep_aspect", "enable"), &ConsoleHw::set_scaler_keep_aspect);
	ClassDB::bind_method(D_METHOD("set_scaler_integer_scaling", "enable"), &ConsoleHw::set_scaler_integer_scaling);
	ClassDB::bind_method(D_METHOD("set_scaler_downscaling", "enable"), &ConsoleHw::set_scaler_downscaling);

	// Backlight
	ClassDB::bind_method(D_METHOD("get_backlight"), &ConsoleHw::get_backlight);
	ClassDB::bind_method(D_METHOD("set_backlight", "percent"), &ConsoleHw::set_backlight);

	// Battery
	ClassDB::bind_method(D_METHOD("get_battery_level"), &ConsoleHw::get_battery_level);

	// WiFi
	ClassDB::bind_method(D_METHOD("wifi_init"), &ConsoleHw::wifi_init);
	ClassDB::bind_method(D_METHOD("wifi_exit"), &ConsoleHw::wifi_exit);
	ClassDB::bind_method(D_METHOD("wifi_get_status"), &ConsoleHw::wifi_get_status);
	ClassDB::bind_method(D_METHOD("wifi_get_ssid"), &ConsoleHw::wifi_get_ssid);

	// Sensors
	ClassDB::bind_method(D_METHOD("sensors_enable_accelerometer"), &ConsoleHw::sensors_enable_accelerometer);
	ClassDB::bind_method(D_METHOD("sensors_disable_accelerometer"), &ConsoleHw::sensors_disable_accelerometer);
	ClassDB::bind_method(D_METHOD("sensors_enable_gyroscope"), &ConsoleHw::sensors_enable_gyroscope);
	ClassDB::bind_method(D_METHOD("sensors_disable_gyroscope"), &ConsoleHw::sensors_disable_gyroscope);
	ClassDB::bind_method(D_METHOD("sensors_read_accelerometer"), &ConsoleHw::sensors_read_accelerometer);
	ClassDB::bind_method(D_METHOD("sensors_read_gyroscope"), &ConsoleHw::sensors_read_gyroscope);

	// VFPU
	ClassDB::bind_method(D_METHOD("vfpu_sqrt", "value"), &ConsoleHw::vfpu_sqrt);
	ClassDB::bind_method(D_METHOD("vfpu_sin", "value"), &ConsoleHw::vfpu_sin);
	ClassDB::bind_method(D_METHOD("vfpu_cos", "value"), &ConsoleHw::vfpu_cos);

	// Accelerated image operations
	ClassDB::bind_method(D_METHOD("image_blit", "src", "src_rect", "dst", "dst_pos"), &ConsoleHw::image_blit);
	ClassDB::bind_method(D_METHOD("image_blend", "src", "src_rect", "dst", "dst_pos"), &ConsoleHw::image_blend);
	ClassDB::bind_method(D_METHOD("image_fill_rect", "dst", "rect", "color"), &ConsoleHw::image_fill_rect);
	ClassDB::bind_method(D_METHOD("image_flip_x", "image"), &ConsoleHw::image_flip_x);
	ClassDB::bind_method(D_METHOD("image_flip_y", "image"), &ConsoleHw::image_flip_y);
	ClassDB::bind_method(D_METHOD("image_invert_colors", "image"), &ConsoleHw::image_invert_colors);
	ClassDB::bind_method(D_METHOD("image_premultiply_alpha", "image"), &ConsoleHw::image_premultiply_alpha);
	ClassDB::bind_method(D_METHOD("has_neon"), &ConsoleHw::has_neon);

	// Enums
	BIND_ENUM_CONSTANT(FILTER_NEAREST);
	BIND_ENUM_CONSTANT(FILTER_BILINEAR);
	BIND_ENUM_CONSTANT(FILTER_BICUBIC);
}

ConsoleHw::ConsoleHw() {
	instance = this;
}

ConsoleHw::~ConsoleHw() {
	if (instance == this) {
		instance = nullptr;
	}
}

// --- Doctests ---

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_SUITE("[ConsoleHw]") {
	TEST_CASE("[arm2d] blit_rect copies rows via memcpy") {
		uint8_t src[4 * 3] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
		uint8_t dst[4 * 3] = {};
		arm2d_blit_rect(src, 4 * 3, dst, 4 * 3, 3, 1, 4);
		CHECK(memcmp(src, dst, 12) == 0);
	}

	TEST_CASE("[arm2d] blit_rect with stride") {
		// 2x2 pixels (RGBA8) in a 3-pixel-wide source, blit to 4-pixel-wide dest
		uint8_t src[3 * 4 * 2] = {};
		uint8_t dst[4 * 4 * 2] = {};
		// Fill src pixels at (0,0) and (1,0)
		src[0] = 0xAA;
		src[4] = 0xBB;
		// Row 1: (0,1) and (1,1)
		src[12] = 0xCC;
		src[16] = 0xDD;

		arm2d_blit_rect(src, 3 * 4, dst, 4 * 4, 2, 2, 4);
		CHECK(dst[0] == 0xAA);
		CHECK(dst[4] == 0xBB);
		CHECK(dst[16] == 0xCC); // row 1 in dst (stride 16)
		CHECK(dst[20] == 0xDD);
	}

	TEST_CASE("[arm2d] blend_rgba8 fully opaque overwrites") {
		uint8_t src[4] = { 255, 0, 0, 255 }; // opaque red
		uint8_t dst[4] = { 0, 255, 0, 255 }; // opaque green
		arm2d_blend_rgba8(src, dst, 1);
		CHECK(dst[0] == 255); // R
		CHECK(dst[1] == 0); // G
		CHECK(dst[2] == 0); // B
		CHECK(dst[3] == 255); // A
	}

	TEST_CASE("[arm2d] blend_rgba8 fully transparent is no-op") {
		uint8_t src[4] = { 255, 0, 0, 0 }; // transparent red
		uint8_t dst[4] = { 0, 255, 0, 255 }; // opaque green
		arm2d_blend_rgba8(src, dst, 1);
		CHECK(dst[0] == 0);
		CHECK(dst[1] == 255);
		CHECK(dst[2] == 0);
		CHECK(dst[3] == 255);
	}

	TEST_CASE("[arm2d] blend_rgba8 half-alpha blending") {
		uint8_t src[4] = { 255, 0, 0, 128 }; // half-transparent red
		uint8_t dst[4] = { 0, 0, 255, 255 }; // opaque blue
		arm2d_blend_rgba8(src, dst, 1);
		// R: (255*128 + 0*127 + 128) >> 8 = 128
		CHECK(dst[0] >= 127);
		CHECK(dst[0] <= 129);
		// G: (0*128 + 0*127 + 128) >> 8 = 0
		CHECK(dst[1] == 0);
		// B: (0*128 + 255*127 + 128) >> 8 ≈ 127
		CHECK(dst[2] >= 126);
		CHECK(dst[2] <= 128);
		CHECK(dst[3] == 255); // alpha stays opaque
	}

	TEST_CASE("[arm2d] blend_rgba8 multiple pixels") {
		uint8_t src[16] = {
			255, 0, 0, 255, // px0: opaque red
			0, 255, 0, 0, // px1: transparent green
			0, 0, 255, 255, // px2: opaque blue
			128, 128, 128, 128 // px3: half-gray
		};
		uint8_t dst[16] = {
			0, 0, 0, 255, // px0: black
			0, 0, 0, 255, // px1: black
			255, 255, 255, 255, // px2: white
			0, 0, 0, 255 // px3: black
		};
		arm2d_blend_rgba8(src, dst, 4);
		// px0: opaque red overwrites
		CHECK(dst[0] == 255);
		CHECK(dst[1] == 0);
		// px1: transparent, no change
		CHECK(dst[4] == 0);
		CHECK(dst[5] == 0);
		// px2: opaque blue overwrites white
		CHECK(dst[8] == 0);
		CHECK(dst[9] == 0);
		CHECK(dst[10] == 255);
	}

	TEST_CASE("[arm2d] fill_rect solid") {
		uint8_t dst[4 * 4 * 3] = {}; // 4x3 RGBA8
		uint8_t color[4] = { 0xDE, 0xAD, 0xBE, 0xEF };
		arm2d_fill_rect(dst, 4 * 4, color, 4, 4, 3);
		for (int i = 0; i < 12; i++) {
			CHECK(dst[i * 4 + 0] == 0xDE);
			CHECK(dst[i * 4 + 1] == 0xAD);
			CHECK(dst[i * 4 + 2] == 0xBE);
			CHECK(dst[i * 4 + 3] == 0xEF);
		}
	}

	TEST_CASE("[arm2d] swap_rows") {
		uint8_t row_a[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
		uint8_t row_b[8] = { 9, 10, 11, 12, 13, 14, 15, 16 };
		arm2d_swap_rows(row_a, row_b, 8);
		CHECK(row_a[0] == 9);
		CHECK(row_a[7] == 16);
		CHECK(row_b[0] == 1);
		CHECK(row_b[7] == 8);
	}

	TEST_CASE("[arm2d] reverse_row_4bpp") {
		uint8_t row[16] = { 1, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 0 };
		arm2d_reverse_row_4bpp(row, 4);
		CHECK(row[0] == 4);
		CHECK(row[4] == 3);
		CHECK(row[8] == 2);
		CHECK(row[12] == 1);
	}

	TEST_CASE("[arm2d] reverse_row generic 3bpp") {
		uint8_t row[9] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		arm2d_reverse_row(row, 3, 3);
		CHECK(row[0] == 7);
		CHECK(row[1] == 8);
		CHECK(row[2] == 9);
		CHECK(row[6] == 1);
		CHECK(row[7] == 2);
		CHECK(row[8] == 3);
	}

	TEST_CASE("[arm2d] invert_colors_rgba8") {
		uint8_t data[8] = { 100, 150, 200, 255, 0, 255, 128, 200 };
		arm2d_invert_colors_rgba8(data, 2);
		CHECK(data[0] == 155);
		CHECK(data[1] == 105);
		CHECK(data[2] == 55);
		CHECK(data[3] == 255); // alpha unchanged
		CHECK(data[4] == 255);
		CHECK(data[5] == 0);
		CHECK(data[6] == 127);
		CHECK(data[7] == 200); // alpha unchanged
	}

	TEST_CASE("[arm2d] premultiply_alpha_rgba8") {
		uint8_t data[4] = { 200, 100, 50, 128 };
		arm2d_premultiply_alpha_rgba8(data, 1);
		// 200 * 128 / 255 ≈ 100 (via (200*128+128)>>8 = 100)
		CHECK(data[0] >= 99);
		CHECK(data[0] <= 101);
		CHECK(data[3] == 128); // alpha unchanged
	}

	TEST_CASE("[arm2d] convert_rgba8_to_rgb8") {
		uint8_t src[8] = { 10, 20, 30, 255, 40, 50, 60, 128 };
		uint8_t dst[6] = {};
		arm2d_convert_rgba8_to_rgb8(src, dst, 2);
		CHECK(dst[0] == 10);
		CHECK(dst[1] == 20);
		CHECK(dst[2] == 30);
		CHECK(dst[3] == 40);
		CHECK(dst[4] == 50);
		CHECK(dst[5] == 60);
	}

	TEST_CASE("[arm2d] convert_rgb8_to_rgba8") {
		uint8_t src[6] = { 10, 20, 30, 40, 50, 60 };
		uint8_t dst[8] = {};
		arm2d_convert_rgb8_to_rgba8(src, dst, 2);
		CHECK(dst[0] == 10);
		CHECK(dst[1] == 20);
		CHECK(dst[2] == 30);
		CHECK(dst[3] == 255);
		CHECK(dst[4] == 40);
		CHECK(dst[5] == 50);
		CHECK(dst[6] == 60);
		CHECK(dst[7] == 255);
	}

	TEST_CASE("[arm2d] convert_rgba8_to_gray8") {
		// Pure white -> 255
		uint8_t white[4] = { 255, 255, 255, 255 };
		uint8_t gray = 0;
		arm2d_convert_rgba8_to_gray8(white, &gray, 1);
		CHECK(gray == 255);

		// Pure black -> 0
		uint8_t black[4] = { 0, 0, 0, 255 };
		arm2d_convert_rgba8_to_gray8(black, &gray, 1);
		CHECK(gray == 0);
	}

	TEST_CASE("[arm2d] has_neon reports correctly") {
		ConsoleHw *hw = ConsoleHw::get_singleton();
		REQUIRE(hw != nullptr);
#if ARM2D_HAS_NEON
		CHECK(hw->has_neon() == true);
#else
		CHECK(hw->has_neon() == false);
#endif
	}
}
#else
#define DOCTEST_CONFIG_DISABLE
#endif // DOCTEST
