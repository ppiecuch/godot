/**************************************************************************/
/*  rfb_encoder.cpp                                                       */
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

#include "rfb_encoder.h"

RFBEncoder::RFBEncoder() {
	fb_width = 0;
	fb_height = 0;
}

void RFBEncoder::set_framebuffer_size(int p_width, int p_height) {
	fb_width = p_width;
	fb_height = p_height;
	prev_frame.clear();
}

Vector<RFBDirtyRect> RFBEncoder::find_dirty_rects(const uint8_t *p_pixels, int p_width, int p_height) {
	Vector<RFBDirtyRect> rects;
	int frame_size = p_width * p_height * 3;

	if (prev_frame.size() != frame_size) {
		// First frame or size changed — everything is dirty.
		prev_frame.resize(frame_size);
		memcpy(prev_frame.ptrw(), p_pixels, frame_size);
		RFBDirtyRect r;
		r.x = 0;
		r.y = 0;
		r.w = p_width;
		r.h = p_height;
		rects.push_back(r);
		return rects;
	}

	const uint8_t *prev = prev_frame.ptr();
	int row_bytes = p_width * 3;

	// Scan rows to find dirty bands, then coalesce.
	int dirty_start = -1;
	for (int y = 0; y < p_height; y++) {
		int offset = y * row_bytes;
		bool row_dirty = (memcmp(prev + offset, p_pixels + offset, row_bytes) != 0);

		if (row_dirty && dirty_start < 0) {
			dirty_start = y;
		} else if (!row_dirty && dirty_start >= 0) {
			RFBDirtyRect r;
			r.x = 0;
			r.y = dirty_start;
			r.w = p_width;
			r.h = y - dirty_start;
			rects.push_back(r);
			dirty_start = -1;
		}
	}
	if (dirty_start >= 0) {
		RFBDirtyRect r;
		r.x = 0;
		r.y = dirty_start;
		r.w = p_width;
		r.h = p_height - dirty_start;
		rects.push_back(r);
	}

	memcpy(prev_frame.ptrw(), p_pixels, frame_size);
	return rects;
}

static void _append_bytes(Vector<uint8_t> &buf, const void *data, int size) {
	int old_size = buf.size();
	buf.resize(old_size + size);
	memcpy(buf.ptrw() + old_size, data, size);
}

static void _encode_rect_raw(Vector<uint8_t> &buf, const uint8_t *p_pixels, int p_fb_width,
		const RFBDirtyRect &rect) {
	// Rect header
	RFBRectHeader hdr;
	hdr.x = rfb_htons(rect.x);
	hdr.y = rfb_htons(rect.y);
	hdr.w = rfb_htons(rect.w);
	hdr.h = rfb_htons(rect.h);
	hdr.encoding = rfb_htonl(RFB_ENCODING_RAW);
	_append_bytes(buf, &hdr, sizeof(hdr));

	// Pixel data: convert RGB8 → 32bpp (BGRx, little-endian with our pixel format)
	for (int y = rect.y; y < rect.y + rect.h; y++) {
		for (int x = rect.x; x < rect.x + rect.w; x++) {
			int src_idx = (y * p_fb_width + x) * 3;
			uint8_t r = p_pixels[src_idx + 0];
			uint8_t g = p_pixels[src_idx + 1];
			uint8_t b = p_pixels[src_idx + 2];
			// 32bpp: pixel = (R << red_shift) | (G << green_shift) | (B << blue_shift)
			// With our default format: R at shift 16, G at shift 8, B at shift 0
			uint32_t pixel = ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
			_append_bytes(buf, &pixel, 4);
		}
	}
}

const Vector<uint8_t> &RFBEncoder::encode_update(const uint8_t *p_pixels, int p_width,
		const Vector<RFBDirtyRect> &p_rects) {
	send_buf.clear();

	RFBFramebufferUpdateHeader hdr;
	hdr.type = RFB_MSG_FRAMEBUFFER_UPDATE;
	hdr.pad = 0;
	hdr.num_rects = rfb_htons(p_rects.size());
	_append_bytes(send_buf, &hdr, sizeof(hdr));

	for (int i = 0; i < p_rects.size(); i++) {
		_encode_rect_raw(send_buf, p_pixels, p_width, p_rects[i]);
	}

	return send_buf;
}

const Vector<uint8_t> &RFBEncoder::encode_full_update(const uint8_t *p_pixels, int p_width, int p_height) {
	Vector<RFBDirtyRect> rects;
	RFBDirtyRect r;
	r.x = 0;
	r.y = 0;
	r.w = p_width;
	r.h = p_height;
	rects.push_back(r);
	return encode_update(p_pixels, p_width, rects);
}

Vector<uint8_t> RFBEncoder::build_server_init(int p_width, int p_height, const char *p_name) {
	Vector<uint8_t> buf;

	RFBServerInit si;
	si.fb_width = rfb_htons(p_width);
	si.fb_height = rfb_htons(p_height);
	si.pixel_format = rfb_default_pixel_format();
	int name_len = strlen(p_name);
	si.name_length = rfb_htonl(name_len);
	_append_bytes(buf, &si, sizeof(si));
	_append_bytes(buf, p_name, name_len);

	return buf;
}
