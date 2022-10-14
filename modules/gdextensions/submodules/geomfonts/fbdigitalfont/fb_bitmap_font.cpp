/*************************************************************************/
/*  fb_bitmap_font.cpp                                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "common/gd_core.h"
#include "core/image.h"
#include "core/math/math_defs.h"
#include "core/math/rect2.h"
#include "core/variant.h"
#include "scene/resources/texture.h"
#include "servers/visual_server.h"

#include "fb_font_draw.h"

#include <vector>

static std::vector<std::vector<bool>> coord_for_symbol(FBFontSymbolType symbol);

// the point half-width in px. must be a power of two <= 256.
#define phf 16
// some convenience constants for the texture dimensions:
// point size, size minus one, half size squared, double size, point center, radius squared
#define psz (phf << 1)
#define psm (psz - 1)
#define phs (phf * phf)
#define pdb (psz << 1)
#define pct (psz >> 1)
#define prs ((psz - 1) * (psz - 1))

#define frsqrtes(v) (1 / Math::sqrt(v))

static std::vector<real_t> logtbl; // for sphere lookups

static inline real_t ifun(real_t x, real_t y, real_t F) { // compute falloff at x,y with exponent F [-1..1]
	real_t S = x * x + y;
	real_t L;
	if (S) { // estimate sqrt, accurate to about 1/4096
		L = frsqrtes(S);
		L = L * S;
	} else {
		L = 0;
	}
	if (F == 0) { // intensity: [-1..0..1] = bloom..normal..fuzzy
		S = 0;
	} else if (F > 0) {
		S = Math::exp(logtbl[(int)(L * 4)] / F); // this is just pow(L/phf, F) but with a lookup table
	} else {
		// this is a bit more complex - logarithm ramp up to a solid dot with slight S blend at cusp
		S = L < (1 + F) * phf ? 0 : Math::exp((1 + F * 0.9) / (L / phf - 1 - F) * logtbl[(int)(L * 4)]);
	}
	return 1 - S;
}

// 8-way symmetric macro
#define putpixel(x, y, I)                                                                                        \
	{                                                                                                            \
		const uint8_t c = 255 - I * 255;                                                                         \
		buffer[(pct + x) + (pct + y) * psz] =                                                                    \
				buffer[(pct - 1 - x) + (pct + y) * psz] =                                                        \
						buffer[(pct + x) + (pct - 1 - y) * psz] =                                                \
								buffer[(pct - 1 - x) + (pct - 1 - y) * psz] =                                    \
										buffer[(pct + y) + (pct + x) * psz] =                                    \
												buffer[(pct - 1 - y) + (pct + x) * psz] =                        \
														buffer[(pct + y) + (pct - 1 - x) * psz] =                \
																buffer[(pct - 1 - y) + (pct - 1 - x) * psz] = c; \
	}

static Ref<Texture> generate_aa_buffer(real_t fall_off) {
	if (logtbl.empty()) {
		logtbl.resize(pdb);
		for (int i = 1; i <= pdb; i++) {
			logtbl[i - 1] = Math::log((real_t)i / pdb); // lookup table has 4x the bitmap resolution
		}
	}
	int32_t x = phf - 1, y = 0;
	real_t T = 0;
	PoolByteArray data;
	data.resize(psz * psz);
	auto buffer = data.write();
	while (x > y) {
		real_t ys = y * y;
		real_t L = Math::sqrt(prs - ys);
		const real_t D = Math::ceil(L) - L;
		if (D < T) {
			x--;
		}
		for (int ax = y; ax < x; ax++) {
			putpixel(ax, y, ifun(ax, ys, fall_off)); // fill wedge
		}
		putpixel(x, y, (1 - D) * (ifun(x, ys, fall_off))); // outer edge
		T = D;
		y++;
	}
	buffer.release();
	Ref<Image> image = newref(Image);
	image->create(psz, psz, false, Image::FORMAT_L8, data);
	Ref<ImageTexture> texture = newref(ImageTexture);
	texture->create_from_image(image);
	return texture;
}

void draw_background_with_dot_type(
		const RID &canvas_item,
		Dictionary &cache,
		FBFontDotType dot_type,
		const Color &color,
		real_t edge_length,
		real_t margin,
		real_t horizontal_amount,
		real_t vertical_amount) {
	const real_t l = edge_length + margin;

	for (int i = 0; i < vertical_amount; i++) {
		for (int j = 0; j < horizontal_amount; j++) {
			if (dot_type == FBFontDotTypeSquare) {
				VisualServer::get_singleton()->canvas_item_add_rect(canvas_item, { j * l, i * l, edge_length, edge_length }, color);
			} else {
				VisualServer::get_singleton()->canvas_item_add_circle(canvas_item, { j * l, i * l }, edge_length, color);
			}
		}
	}
}

void draw_bitmap_symbol(
		const RID &canvas_item_opaq,
		const RID &canvas_item_trnsp,
		Dictionary &cache,
		FBFontSymbolType symbol,
		FBFontDotType dot_type,
		const Color &color,
		real_t edge_length,
		real_t margin,
		const Color &glow_color,
		real_t glow_size,
		const Color &inner_glow_color,
		real_t inner_glow_size,
		const Point2 &start_point) {
	const real_t l = edge_length + margin;
	const auto &coord = coord_for_symbol(symbol);

	for (int r = 0; r < coord.size(); r++) {
		const auto &column = coord[r];
		for (int c = 0; c < column.size(); c++) {
			if (column[c]) {
				const real_t y = start_point.y + r * l;
				const real_t x = start_point.x + c * l;
				if (dot_type == FBFontDotTypeSquare) {
					VisualServer::get_singleton()->canvas_item_add_rect(canvas_item_opaq, { x, y, edge_length, edge_length }, color);
				} else {
					VisualServer::get_singleton()->canvas_item_add_circle(canvas_item_opaq, { x, y }, edge_length, color);
				}
			}
		}
	}
	if (!cache.has("aa32")) {
		cache["aa32"] = generate_aa_buffer(0.5);
	}
	Ref<Texture> texture = cache["aa32"];
	VisualServer::get_singleton()->canvas_item_add_texture_rect(canvas_item_trnsp, { 0, 0, 256, 256 }, texture->get_rid(), false, inner_glow_color);
}

int number_of_dots_wide_for_symbol(FBFontSymbolType symbol) {
	return coord_for_symbol(symbol).size();
}

static std::vector<std::vector<bool>> coord_for_symbol(FBFontSymbolType symbol) {
	switch (symbol) {
		case FBFontSymbol0:
			return { { 0, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 1, 1, 0 } };
		case FBFontSymbol1:
			return { { 0, 0, 1, 0, 0 },
				{ 0, 1, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 } };
		case FBFontSymbol2:
			return { { 0, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 0, 0, 0, 1 },
				{ 0, 0, 0, 1, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 1, 0, 0, 0 },
				{ 1, 1, 1, 1, 1 } };
		case FBFontSymbol3:
			return { { 0, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 0, 0, 0, 1 },
				{ 0, 0, 1, 1, 0 },
				{ 0, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 1, 1, 0 } };
		case FBFontSymbol4:
			return { { 0, 0, 0, 1, 0 },
				{ 0, 0, 1, 1, 0 },
				{ 0, 1, 0, 1, 0 },
				{ 1, 0, 0, 1, 0 },
				{ 1, 1, 1, 1, 1 },
				{ 0, 0, 0, 1, 0 },
				{ 0, 0, 0, 1, 0 } };
		case FBFontSymbol5:
			return { { 1, 1, 1, 1, 1 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 1, 1, 1, 0 },
				{ 0, 0, 0, 0, 1 },
				{ 0, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 1, 1, 0 } };
		case FBFontSymbol6:
			return { { 0, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 1, 1, 0 } };
		case FBFontSymbol7:
			return { { 1, 1, 1, 1, 1 },
				{ 0, 0, 0, 0, 1 },
				{ 0, 0, 0, 1, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 1, 0, 0, 0 },
				{ 0, 1, 0, 0, 0 },
				{ 0, 1, 0, 0, 0 } };
		case FBFontSymbol8:
			return { { 0, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 1, 1, 0 } };
		case FBFontSymbol9:
			return { { 0, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 1, 1, 1 },
				{ 0, 0, 0, 0, 1 },
				{ 0, 0, 0, 0, 1 },
				{ 0, 1, 1, 1, 0 } };
		case FBFontSymbolA:
			return { { 0, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 1, 1, 1, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 } };
		case FBFontSymbolB:
			return { { 1, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 1, 1, 1, 0 } };
		case FBFontSymbolC:
			return { { 0, 1, 1, 1, 1 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 0, 1, 1, 1, 1 } };
		case FBFontSymbolD:
			return { { 1, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 1, 1, 1, 0 } };
		case FBFontSymbolE:
			return { { 1, 1, 1, 1, 1 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 1, 1, 1, 1 } };
		case FBFontSymbolF:
			return { { 1, 1, 1, 1, 1 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 } };
		case FBFontSymbolG:
			return { { 0, 1, 1, 1, 1 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 1, 1, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 1, 1, 0 } };
		case FBFontSymbolH:
			return { { 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 1, 1, 1, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 } };
		case FBFontSymbolI:
			return { { 1, 1, 1, 1, 1 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 1, 1, 1, 1, 1 } };
		case FBFontSymbolJ:
			return { { 0, 0, 0, 1, 0 },
				{ 0, 0, 0, 1, 0 },
				{ 0, 0, 0, 1, 0 },
				{ 0, 0, 0, 1, 0 },
				{ 1, 0, 0, 1, 0 },
				{ 1, 0, 0, 1, 0 },
				{ 0, 1, 1, 0, 0 } };
		case FBFontSymbolK:
			return { { 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 1, 0 },
				{ 1, 0, 1, 0, 0 },
				{ 1, 1, 0, 0, 0 },
				{ 1, 0, 1, 0, 0 },
				{ 1, 0, 0, 1, 0 },
				{ 1, 0, 0, 0, 1 } };
		case FBFontSymbolL:
			return { { 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 1, 1, 1, 1 } };
		case FBFontSymbolM:
			return { { 1, 0, 0, 0, 1 },
				{ 1, 1, 0, 1, 1 },
				{ 1, 0, 1, 0, 1 },
				{ 1, 0, 1, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 } };
		case FBFontSymbolN:
			return { { 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 1, 0, 0, 1 },
				{ 1, 0, 1, 0, 1 },
				{ 1, 0, 0, 1, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 } };
		case FBFontSymbolO:
			return { { 0, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 1, 1, 0 } };
		case FBFontSymbolP:
			return { { 1, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 } };
		case FBFontSymbolQ:
			return { { 0, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 1, 0, 1 },
				{ 1, 0, 0, 1, 1 },
				{ 0, 1, 1, 1, 1 } };
		case FBFontSymbolR:
			return { { 1, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 1, 1, 1, 0 },
				{ 1, 0, 1, 0, 0 },
				{ 1, 0, 0, 1, 0 },
				{ 1, 0, 0, 0, 1 } };
		case FBFontSymbolS:
			return { { 0, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 0 },
				{ 0, 1, 1, 1, 0 },
				{ 0, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 1, 1, 0 } };
		case FBFontSymbolT:
			return { { 1, 1, 1, 1, 1 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 } };
		case FBFontSymbolU:
			return { { 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 1, 1, 0 } };
		case FBFontSymbolV:
			return { { 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 0, 1, 0 },
				{ 0, 0, 1, 0, 0 } };
		case FBFontSymbolW:
			return { { 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 1, 0, 1 },
				{ 1, 0, 1, 0, 1 },
				{ 1, 1, 0, 1, 1 },
				{ 1, 0, 0, 0, 1 } };
		case FBFontSymbolX:
			return { { 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 0, 1, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 1, 0, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 } };
		case FBFontSymbolY:
			return { { 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 0, 1, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 } };
		case FBFontSymbolZ:
			return { { 1, 1, 1, 1, 1 },
				{ 0, 0, 0, 0, 1 },
				{ 0, 0, 0, 1, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 1, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 1, 1, 1, 1 } };
		case FBFontSymbolArrowUp:
			return { { 0, 0, 0, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 1, 1, 1, 0 },
				{ 1, 0, 1, 0, 1 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 0, 0, 0 } };
		case FBFontSymbolArrowDown:
			return { { 0, 0, 0, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 1, 0, 1, 0, 1 },
				{ 0, 1, 1, 1, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 0, 0, 0 } };
		case FBFontSymbolArrowLeft:
			return { { 0, 0, 0, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 1, 0, 0, 0 },
				{ 1, 1, 1, 1, 1 },
				{ 0, 1, 0, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 0, 0, 0 } };
		case FBFontSymbolArrowRight:
			return { { 0, 0, 0, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 0, 1, 0 },
				{ 1, 1, 1, 1, 1 },
				{ 0, 0, 0, 1, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 0, 0, 0 } };
		case FBFontSymbolDash:
			return { { 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0 },
				{ 1, 1, 1, 1, 1 },
				{ 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0 } };
		case FBFontSymbolSpace:
			return { { 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0 } };
		case FBFontSymbolExclamationMark:
			return { { 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 0, 0, 0 },
				{ 0, 0, 1, 0, 0 } };
		case FBFontSymbolColon:
			return { { 0, 0, 0 },
				{ 0, 1, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 1, 0 },
				{ 0, 0, 0 } };
		default:
			return std::vector<std::vector<bool>>();
	}
}
