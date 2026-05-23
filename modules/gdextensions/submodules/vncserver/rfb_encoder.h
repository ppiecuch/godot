/**************************************************************************/
/*  rfb_encoder.h                                                         */
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

#ifndef VNC_RFB_ENCODER_H
#define VNC_RFB_ENCODER_H

#include "rfb_protocol.h"

#include "core/vector.h"

#include <string.h>

struct RFBDirtyRect {
	uint16_t x, y, w, h;
};

class RFBEncoder {
	int fb_width;
	int fb_height;
	Vector<uint8_t> prev_frame; // previous RGB8 frame for dirty detection
	Vector<uint8_t> send_buf; // reusable send buffer

public:
	RFBEncoder();

	void set_framebuffer_size(int p_width, int p_height);

	// Detect dirty rectangles by comparing current frame against previous.
	// Updates prev_frame to current. Returns list of dirty rects.
	// p_pixels: RGB8 data, p_width * p_height * 3 bytes.
	Vector<RFBDirtyRect> find_dirty_rects(const uint8_t *p_pixels, int p_width, int p_height);

	// Encode a full framebuffer update message (Raw encoding).
	// p_pixels: RGB8 source, p_rects: dirty rectangles.
	// Returns a buffer ready to send over TCP (includes RFB headers).
	const Vector<uint8_t> &encode_update(const uint8_t *p_pixels, int p_width,
			const Vector<RFBDirtyRect> &p_rects);

	// Encode a full-screen update (single rect covering entire framebuffer).
	const Vector<uint8_t> &encode_full_update(const uint8_t *p_pixels, int p_width, int p_height);

	// Build the ServerInit message.
	static Vector<uint8_t> build_server_init(int p_width, int p_height, const char *p_name);
};

#endif // VNC_RFB_ENCODER_H
