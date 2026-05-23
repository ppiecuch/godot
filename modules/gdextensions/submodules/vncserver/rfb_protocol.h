/**************************************************************************/
/*  rfb_protocol.h                                                        */
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

// RFB (Remote Framebuffer) Protocol v3.3 — clean-room from published spec.

#ifndef VNC_RFB_PROTOCOL_H
#define VNC_RFB_PROTOCOL_H

#include <stdint.h>

// --- Protocol version ---

#define RFB_PROTOCOL_VERSION_STRING "RFB 003.003\n"
#define RFB_PROTOCOL_VERSION_LENGTH 12
#define RFB_PROTOCOL_MAJOR 3
#define RFB_PROTOCOL_MINOR 3

// --- Security types ---

#define RFB_SECURITY_CONN_FAILED 0
#define RFB_SECURITY_NONE 1
#define RFB_SECURITY_VNC_AUTH 2

// --- Server → client message types ---

#define RFB_MSG_FRAMEBUFFER_UPDATE 0
#define RFB_MSG_SET_COLOUR_MAP 1
#define RFB_MSG_BELL 2
#define RFB_MSG_SERVER_CUT_TEXT 3

// --- Client → server message types ---

#define RFB_MSG_SET_PIXEL_FORMAT 0
#define RFB_MSG_SET_ENCODINGS 2
#define RFB_MSG_FB_UPDATE_REQUEST 3
#define RFB_MSG_KEY_EVENT 4
#define RFB_MSG_POINTER_EVENT 5
#define RFB_MSG_CLIENT_CUT_TEXT 6

// --- Encoding types ---

#define RFB_ENCODING_RAW 0
#define RFB_ENCODING_COPYRECT 1
#define RFB_ENCODING_RRE 2
#define RFB_ENCODING_CORRE 4
#define RFB_ENCODING_HEXTILE 5

// --- Byte order helpers ---

_FORCE_INLINE_ uint16_t rfb_htons(uint16_t v) {
	return (v >> 8) | (v << 8);
}

_FORCE_INLINE_ uint32_t rfb_htonl(uint32_t v) {
	return ((v >> 24) & 0xFF) |
			((v >> 8) & 0xFF00) |
			((v << 8) & 0xFF0000) |
			((v << 24) & 0xFF000000);
}

_FORCE_INLINE_ uint16_t rfb_ntohs(uint16_t v) { return rfb_htons(v); }
_FORCE_INLINE_ uint32_t rfb_ntohl(uint32_t v) { return rfb_htonl(v); }

// --- Pixel format ---

#pragma pack(push, 1)

struct RFBPixelFormat {
	uint8_t bits_per_pixel; // 8, 16, or 32
	uint8_t depth;
	uint8_t big_endian;
	uint8_t true_colour;
	uint16_t red_max;
	uint16_t green_max;
	uint16_t blue_max;
	uint8_t red_shift;
	uint8_t green_shift;
	uint8_t blue_shift;
	uint8_t pad[3];
};

// --- Server init ---

struct RFBServerInit {
	uint16_t fb_width;
	uint16_t fb_height;
	RFBPixelFormat pixel_format;
	uint32_t name_length;
	// followed by name_length bytes of name
};

// --- Client init ---

struct RFBClientInit {
	uint8_t shared;
};

// --- FramebufferUpdate (server → client) ---

struct RFBFramebufferUpdateHeader {
	uint8_t type; // RFB_MSG_FRAMEBUFFER_UPDATE
	uint8_t pad;
	uint16_t num_rects;
};

struct RFBRectHeader {
	uint16_t x;
	uint16_t y;
	uint16_t w;
	uint16_t h;
	uint32_t encoding;
};

// --- SetPixelFormat (client → server) ---

struct RFBSetPixelFormat {
	uint8_t type; // RFB_MSG_SET_PIXEL_FORMAT
	uint8_t pad[3];
	RFBPixelFormat pixel_format;
};

// --- SetEncodings (client → server) ---

struct RFBSetEncodingsHeader {
	uint8_t type; // RFB_MSG_SET_ENCODINGS
	uint8_t pad;
	uint16_t num_encodings;
	// followed by num_encodings * uint32_t
};

// --- FramebufferUpdateRequest (client → server) ---

struct RFBFramebufferUpdateRequest {
	uint8_t type; // RFB_MSG_FB_UPDATE_REQUEST
	uint8_t incremental;
	uint16_t x;
	uint16_t y;
	uint16_t w;
	uint16_t h;
};

// --- KeyEvent (client → server) ---

struct RFBKeyEvent {
	uint8_t type; // RFB_MSG_KEY_EVENT
	uint8_t down;
	uint16_t pad;
	uint32_t key;
};

// --- PointerEvent (client → server) ---

struct RFBPointerEvent {
	uint8_t type; // RFB_MSG_POINTER_EVENT
	uint8_t button_mask;
	uint16_t x;
	uint16_t y;
};

// --- CopyRect source position ---

struct RFBCopyRect {
	uint16_t src_x;
	uint16_t src_y;
};

#pragma pack(pop)

// --- Default pixel format: 32bpp RGB (what we serve) ---

_FORCE_INLINE_ RFBPixelFormat rfb_default_pixel_format() {
	RFBPixelFormat pf;
	memset(&pf, 0, sizeof(pf));
	pf.bits_per_pixel = 32;
	pf.depth = 24;
	pf.big_endian = 0;
	pf.true_colour = 1;
	pf.red_max = rfb_htons(255);
	pf.green_max = rfb_htons(255);
	pf.blue_max = rfb_htons(255);
	pf.red_shift = 16;
	pf.green_shift = 8;
	pf.blue_shift = 0;
	return pf;
}

#endif // VNC_RFB_PROTOCOL_H
