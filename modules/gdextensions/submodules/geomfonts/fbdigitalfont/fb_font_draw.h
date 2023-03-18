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

/// Bitmap Font

typedef enum {
	FBFontDotStyleFlatCircle,
	FBFontDotStyleFlatSquare,
	FBFontDotStyleTextureCircle,
	FBFontDotStyleTextureSquare,
	FBFontDotStyleTexture3D,
} FBFontDotStyle;

struct CharPadding {
	int number_of_left_dot;
	int number_of_top_dot;
	int number_of_bottom_dot;
	int number_of_right_dot;
	int number_of_between_dot;
};

void init_bitmap_symbol(Dictionary &cache, real_t fall_off);

void draw_padding_with_dot_style(
		const RID &canvas_item,
		Dictionary &cache,
		FBFontDotStyle dot_style,
		Point2 start_point,
		Color color,
		int edge_length,
		int margin,
		const Size2i &character,
		const CharPadding &padding);

void draw_background_with_dot_style(
		const RID &canvas_item,
		Dictionary &cache,
		FBFontDotStyle dot_style,
		Point2 start_point,
		Color color,
		int edge_length,
		int margin,
		int horizontal_amount,
		int vertical_amount);

void draw_bitmap_symbol(
		const RID &canvas_item,
		Dictionary &cache,
		FBFontSymbolType symbol,
		FBFontDotStyle dot_style,
		Color color,
		int edge_length,
		int margin,
		const Color &glow_color,
		real_t glow_size,
		const Color &inner_glow_color,
		real_t inner_glow_size,
		const Point2 &start_point);

void draw_bitmap_symbol_with_padding(
		const RID &canvas_item,
		Dictionary &cache,
		FBFontSymbolType symbol,
		FBFontDotStyle dot_style,
		Color color,
		Color off_color,
		int edge_length,
		int margin,
		const CharPadding &padding,
		const Color &glow_color,
		real_t glow_size,
		const Color &inner_glow_color,
		real_t inner_glow_size,
		const Point2 &start_point);

int number_of_dots_wide_for_symbol(FBFontSymbolType symbol);

/// LCD Font

void path_lcd_symbol(
		Vector<Vector<Point2>> &path,
		FBFontSymbolType symbol,
		int edge_length,
		int line_width,
		Point2 start_point);

void draw_lcd_symbol(
		const RID &canvas_item,
		FBFontSymbolType symbol,
		int edge_length,
		int line_width,
		const Color &color,
		Point2 start_point);

/// Square Font

void path_square_symbol(
		Vector<Vector<Point2>> &path,
		FBFontSymbolType symbol,
		int horizontal_edge_length,
		int vertical_edge_length,
		const Color &color,
		const Point2 &start_point);

void draw_square_symbol(
		const RID &canvas_item,
		FBFontSymbolType symbol,
		int horizontal_edge_length,
		int vertical_edge_length,
		const Color &color,
		const Point2 &start_point);

#endif // FBFONTDRAW_H
