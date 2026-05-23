/**************************************************************************/
/*  vnc_overlay.h                                                         */
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

#ifndef VNC_OVERLAY_H
#define VNC_OVERLAY_H

#include "core/color.h"
#include "core/hash_map.h"
#include "core/ustring.h"

#include <stdint.h>

struct VNCOverlayEntry {
	int x;
	int y;
	String text;
	Color color;
	bool bold;
	bool has_background;
	Color bg_color;
};

class VNCOverlay {
	HashMap<int, VNCOverlayEntry> entries;
	int margin;

public:
	VNCOverlay();

	void set_text(int p_id, int p_x, int p_y, const String &p_text, const Color &p_color = Color(1, 1, 1), bool p_bold = false);
	void set_background(int p_id, const Color &p_bg_color);
	void remove_text(int p_id);
	void clear();
	void set_margin(int p_margin);

	// Blit all overlay entries onto an RGB8 pixel buffer.
	// p_pixels: raw RGB8 data (3 bytes per pixel, row-major).
	// p_width, p_height: buffer dimensions.
	void render(uint8_t *p_pixels, int p_width, int p_height) const;

	bool is_empty() const { return entries.empty(); }
};

#endif // VNC_OVERLAY_H
