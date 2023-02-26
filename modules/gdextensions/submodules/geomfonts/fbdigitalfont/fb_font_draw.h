/**************************************************************************/
/*  fb_font_draw.h                                                        */
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

#pragma once

#ifndef FBFONTDRAW_H
#define FBFONTDRAW_H

#include "fb_font_symbol.h"

#include "core/color.h"
#include "core/dictionary.h"
#include "core/math/vector2.h"
#include "core/rid.h"

typedef enum {
} FBLineJoin;

typedef enum {
} FBLineCap;

/// Bitmap Font

typedef enum {
	FBFontDotTypeSquare,
	FBFontDotTypeCircle
} FBFontDotType;

void draw_background_with_dot_type(
		const RID &canvas_item,
		Dictionary &cache,
		FBFontDotType dot_type,
		const Color &color,
		real_t edge_length,
		real_t margin,
		real_t horizontal_amount,
		real_t vertical_amount);

void draw_bitmap_symbol(
		const RID &canvas_item,
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
		const Point2 &start_point);

int number_of_dots_wide_for_symbol(FBFontSymbolType symbol);

/// LCD Font

void draw_lcd_symbol(
		const RID &canvas_item,
		FBFontSymbolType symbol,
		real_t edge_length,
		real_t line_width,
		Point2 start_point);

/// Square Font

void draw_square_symbol(
		const RID &canvas_item,
		FBFontSymbolType symbol,
		real_t horizontal_edge_length,
		real_t vertical_edge_length,
		Point2 start_point);

#endif // FBFONTDRAW_H
