/**************************************************************************/
/*  vnc_overlay.cpp                                                       */
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

#include "vnc_overlay.h"

#include "vnc_font_boxxy.h"
#include "vnc_font_boxxy_bold.h"

VNCOverlay::VNCOverlay() {
	margin = 2;
}

void VNCOverlay::set_text(int p_id, int p_x, int p_y, const String &p_text, const Color &p_color, bool p_bold) {
	VNCOverlayEntry entry;
	entry.x = p_x;
	entry.y = p_y;
	entry.text = p_text;
	entry.color = p_color;
	entry.bold = p_bold;
	entry.has_background = false;
	entry.bg_color = Color(0, 0, 0, 0.7);

	if (entries.has(p_id)) {
		entries[p_id] = entry;
	} else {
		entries[p_id] = entry;
	}
}

void VNCOverlay::set_background(int p_id, const Color &p_bg_color) {
	if (entries.has(p_id)) {
		entries[p_id].has_background = true;
		entries[p_id].bg_color = p_bg_color;
	}
}

void VNCOverlay::remove_text(int p_id) {
	entries.erase(p_id);
}

void VNCOverlay::clear() {
	entries.clear();
}

void VNCOverlay::set_margin(int p_margin) {
	margin = p_margin;
}

static void _fill_rect_rgb8(uint8_t *p_pixels, int p_buf_width, int p_buf_height,
		int p_x, int p_y, int p_w, int p_h, const Color &p_color) {
	uint8_t r = (uint8_t)(p_color.r * 255);
	uint8_t g = (uint8_t)(p_color.g * 255);
	uint8_t b = (uint8_t)(p_color.b * 255);
	float alpha = p_color.a;

	for (int row = p_y; row < p_y + p_h; row++) {
		if (row < 0 || row >= p_buf_height)
			continue;
		for (int col = p_x; col < p_x + p_w; col++) {
			if (col < 0 || col >= p_buf_width)
				continue;
			int idx = (row * p_buf_width + col) * 3;
			if (alpha >= 1.0f) {
				p_pixels[idx + 0] = r;
				p_pixels[idx + 1] = g;
				p_pixels[idx + 2] = b;
			} else {
				p_pixels[idx + 0] = (uint8_t)(p_pixels[idx + 0] * (1.0f - alpha) + r * alpha);
				p_pixels[idx + 1] = (uint8_t)(p_pixels[idx + 1] * (1.0f - alpha) + g * alpha);
				p_pixels[idx + 2] = (uint8_t)(p_pixels[idx + 2] * (1.0f - alpha) + b * alpha);
			}
		}
	}
}

static void _blit_glyph_rgb8(uint8_t *p_pixels, int p_buf_width, int p_buf_height,
		int p_x, int p_y, uint8_t p_char, const Color &p_color, bool p_bold) {
	if (p_char > VNC_FONT_BOXXY_LAST_CHAR)
		return;

	const uint8_t *glyph = p_bold ? vnc_font_boxxy_bold[p_char] : vnc_font_boxxy[p_char];
	uint8_t r = (uint8_t)(p_color.r * 255);
	uint8_t g = (uint8_t)(p_color.g * 255);
	uint8_t b = (uint8_t)(p_color.b * 255);

	for (int row = 0; row < VNC_FONT_BOXXY_HEIGHT; row++) {
		int py = p_y + row;
		if (py < 0 || py >= p_buf_height)
			continue;
		uint8_t bits = glyph[row];
		for (int col = 0; col < VNC_FONT_BOXXY_WIDTH; col++) {
			if (bits & (0x80 >> col)) {
				int px = p_x + col;
				if (px < 0 || px >= p_buf_width)
					continue;
				int idx = (py * p_buf_width + px) * 3;
				p_pixels[idx + 0] = r;
				p_pixels[idx + 1] = g;
				p_pixels[idx + 2] = b;
			}
		}
	}
}

void VNCOverlay::render(uint8_t *p_pixels, int p_width, int p_height) const {
	const int *key = NULL;
	while ((key = entries.next(key))) {
		const VNCOverlayEntry &entry = entries[*key];
		CharString utf8 = entry.text.utf8();
		int text_len = utf8.length();

		if (entry.has_background) {
			int bg_w = text_len * VNC_FONT_BOXXY_WIDTH + margin * 2;
			int bg_h = VNC_FONT_BOXXY_HEIGHT + margin * 2;
			_fill_rect_rgb8(p_pixels, p_width, p_height,
					entry.x - margin, entry.y - margin, bg_w, bg_h, entry.bg_color);
		}

		const char *str = utf8.get_data();
		int cx = entry.x;
		for (int i = 0; i < text_len; i++) {
			uint8_t ch = (uint8_t)str[i];
			_blit_glyph_rgb8(p_pixels, p_width, p_height, cx, entry.y, ch, entry.color, entry.bold);
			cx += VNC_FONT_BOXXY_WIDTH;
		}
	}
}
