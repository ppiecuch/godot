/**************************************************************************/
/*  console_raster.cpp                                                    */
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

#include "console_raster.h"

static _FORCE_INLINE_ uint32_t _pack(const Color &p_color, ConsolePixelFormat p_format) {
	const uint32_t a = uint32_t(CLAMP(int(p_color.a * 255.0f + 0.5f), 0, 255));
	const uint32_t r = uint32_t(CLAMP(int(p_color.r * 255.0f + 0.5f), 0, 255));
	const uint32_t g = uint32_t(CLAMP(int(p_color.g * 255.0f + 0.5f), 0, 255));
	const uint32_t b = uint32_t(CLAMP(int(p_color.b * 255.0f + 0.5f), 0, 255));

	if (p_format == CONSOLE_PIXEL_ABGR32) {
		return (a << 24) | (b << 16) | (g << 8) | r;
	}
	return (a << 24) | (r << 16) | (g << 8) | b;
}

static _FORCE_INLINE_ int _zoom(const TextConsole &p_console) {
	return MAX(1, p_console.get_pixel_scale());
}

Size2i console_raster_size(const TextConsole &p_console) {
	const int zoom = _zoom(p_console);
	return Size2i(p_console._con_size.width * p_console._font_size.width * zoom,
			p_console._con_size.height * p_console._font_size.height * zoom);
}

void console_blit(const TextConsole &p_console, uint32_t *p_dst, int p_stride_px, int p_width, int p_height, ConsolePixelFormat p_format) {
	ERR_FAIL_COND(p_dst == nullptr);
	ERR_FAIL_COND(p_console._screen == nullptr);
	ERR_FAIL_COND(p_stride_px < p_width);

	const ConsoleFontSource font = console_get_font_source(p_console._font_face);
	ERR_FAIL_COND(font.pixels == nullptr);

	const Color *pal = console_get_palette();
	const int zoom = _zoom(p_console);
	const int cell_w = font.char_w * zoom;
	const int cell_h = font.char_h * zoom;

	// The palette is tiny and constant per blit, so pack it once instead of per pixel.
	uint32_t packed[TextConsole::COLOR_COUNT];
	for (int i = 0; i < TextConsole::COLOR_COUNT; ++i) {
		packed[i] = _pack(pal[i], p_format);
	}

	const TextConsole::cell *p = p_console._screen;
	for (int row = 0; row < p_console._con_size.height; ++row) {
		const int y0 = row * cell_h;
		if (y0 >= p_height) {
			break;
		}
		for (int col = 0; col < p_console._con_size.width; ++col, ++p) {
			const int x0 = col * cell_w;
			if (x0 >= p_width) {
				continue;
			}

			TextConsole::resolved_cell rc;
			if (!p_console._resolve_cell(*p, rc)) {
				continue;
			}

			// Bank 0 glyphs sit in a 16x16 grid; bank 1 patches are a flat blob indexed by
			// their position in patch_codes.
			const uint8_t *glyph = nullptr;
			int glyph_stride = 0;
			if (rc.bank == TextConsole::BANK_PATCH) {
				for (int i = 0; i < font.patch_count; ++i) {
					if (font.patch_codes[i] == rc.code) {
						glyph = font.patch + i * font.char_w * font.char_h;
						glyph_stride = font.char_w;
						break;
					}
				}
			}
			if (glyph == nullptr) {
				const int gx = (rc.code % 16) * font.char_w;
				const int gy = (rc.code / 16) * font.char_h;
				glyph = font.pixels + gy * font.atlas_w + gx;
				glyph_stride = font.atlas_w;
			}

			const bool fg_visible = rc.foreground != TextConsole::COLOR_TRANSPARENT;
			const bool bg_visible = rc.background != TextConsole::COLOR_TRANSPARENT;
			if (!fg_visible && !bg_visible) {
				continue;
			}
			const uint32_t fg = packed[rc.foreground];
			const uint32_t bg = packed[rc.background];

			for (int gy = 0; gy < font.char_h; ++gy) {
				const uint8_t *src = glyph + gy * glyph_stride;
				for (int sy = 0; sy < zoom; ++sy) {
					const int y = y0 + gy * zoom + sy;
					if (y >= p_height) {
						break;
					}
					uint32_t *dst = p_dst + y * p_stride_px;
					for (int gx = 0; gx < font.char_w; ++gx) {
						const bool lit = src[gx] > 127;
						if (lit ? !fg_visible : !bg_visible) {
							continue;
						}
						const uint32_t color = lit ? fg : bg;
						const int x = x0 + gx * zoom;
						for (int sx = 0; sx < zoom; ++sx) {
							if (x + sx >= p_width) {
								break;
							}
							dst[x + sx] = color;
						}
					}
				}
			}
		}
	}
}
