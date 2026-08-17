/**************************************************************************/
/*  console_raster.h                                                      */
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

#ifndef CONSOLE_RASTER_H
#define CONSOLE_RASTER_H

#include "debug_console.h"

/* Software rasterizer for TextConsole.
 *
 * Draws straight from the embedded glyph data into a caller-owned 32bpp buffer. It touches
 * no VisualServer, no GL and no scene tree, so it can run on a raw ANativeWindow surface on
 * the Android secondary display (see CONSOLE.md sections 4 and 12).
 *
 * Cell resolution goes through TextConsole::_resolve_cell(), the same helper the GPU mesh
 * path uses, so both backends render identically by construction.
 */

enum ConsolePixelFormat {
	CONSOLE_PIXEL_ARGB32, // 0xAARRGGBB in a native-endian uint32_t
	CONSOLE_PIXEL_ABGR32, // 0xAABBGGRR -- byte order R,G,B,A, what ANativeWindow's
						  // WINDOW_FORMAT_RGBA_8888 expects on little-endian devices
};

/* Blits the console into p_dst.
 *
 * p_stride_px is the destination row stride in *pixels* (ANativeWindow reports exactly
 * that). Drawing is clipped to p_width x p_height, and cells whose colour is
 * COLOR_TRANSPARENT are skipped rather than blended, so the caller decides what shows
 * through -- fill the buffer first if it needs an opaque background.
 *
 * TextConsole::pixel_scale is honoured as an integer zoom factor (values below 1 mean 1).
 */
void console_blit(const TextConsole &p_console, uint32_t *p_dst, int p_stride_px, int p_width, int p_height, ConsolePixelFormat p_format = CONSOLE_PIXEL_ARGB32);

/* Pixel size the console occupies at its current font and pixel_scale. */
Size2i console_raster_size(const TextConsole &p_console);

/* Where the embedded glyphs for a face live. Bank 0 is a 16x16 grid of char_w x char_h
 * glyphs, 1 byte per pixel (0 or 255); bank 1 is the flat patch blob, one char_w * char_h
 * block per entry of patch_codes. */
struct ConsoleFontSource {
	const uint8_t *pixels;
	const uint8_t *patch;
	const uint8_t *patch_codes;
	int patch_count;
	int char_w, char_h;
	int atlas_w; // bank 0 row stride in pixels, i.e. 16 * char_w
};

ConsoleFontSource console_get_font_source(TextConsole::FontSize p_font);
const Color *console_get_palette(); // TextConsole::COLOR_COUNT entries

#endif // CONSOLE_RASTER_H
