/**************************************************************************/
/*  fb_font_view.h                                                        */
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

#ifndef FBFONTVIEW_H
#define FBFONTVIEW_H

#include "fb_font_draw.h"
#include "fb_font_symbol.h"

#include "core/color.h"
#include "core/dictionary.h"
#include "core/vector.h"

#include <vector>

class FBBitmapFontView {
	Vector<FBFontSymbolType> symbols;
	FBFontDotStyle dot_style;
	int edge_length;
	int margin;
	real_t glow_size;
	real_t inner_glow_size;
	CharPadding padding;
	Color off_color;
	Color on_color;
	Color glow_color;
	Color inner_glow_color;
	String text;

	std::vector<std::vector<bool>> _dots;

	int number_of_horizontal_dot() const;
	int number_of_vertical_dot() const;

	void _update_2d_view();
	void _dump_2d_view(const char dot = '*');

	const RID &canvas_item;

public:
	void draw(const Point2 &p_pos = Point2());
	Size2 size_of_contents() const;
	int get_font_height() const { return 7; }

	void set_text(const String &p_text);
	void set_style(FBFontDotStyle p_style) { dot_style = p_style; }
	void set_edge_length(int p_edge_length) { edge_length = p_edge_length; }
	void set_margin(int p_margin) { margin = p_margin; }
	void set_padding(int p_left, int p_right, int p_top, int p_bottom) {
		padding.number_of_left_dot = p_left;
		padding.number_of_right_dot = p_right;
		padding.number_of_top_dot = p_top;
		padding.number_of_bottom_dot = p_bottom;
	}
	void set_spacing(int p_spacing) { padding.number_of_between_dot = p_spacing; }
	void set_on_color(const Color &p_color) { on_color = p_color; }
	void set_on_color(unsigned p_color) { on_color = Color::hex(p_color); }
	void set_off_color(const Color &p_color) { off_color = p_color; }
	void set_off_color(unsigned p_color) { off_color = Color::hex(p_color); }
	void set_glow_color(const Color &p_color) { glow_color = p_color; }
	void set_glow_color(unsigned p_color) { glow_color = Color::hex(p_color); }
	void set_inner_glow_color(const Color &p_color) { inner_glow_color = p_color; }
	void set_inner_glow_color(unsigned p_color) { inner_glow_color = Color::hex(p_color); }

	FBBitmapFontView(const RID &canvas_item);
};

class FBLCDFontView {
	Vector<FBFontSymbolType> symbols;
	bool draw_off_segments;
	int edge_length;
	int margin;
	real_t line_width;
	real_t horizontal_padding;
	real_t vertical_padding;
	real_t glow_size;
	real_t inner_glow_size;
	Color line_color;
	Color off_color;
	Color glow_color;
	Color inner_glow_color;
	String text;

	const RID &canvas_item;

public:
	Size2 size_of_contents() const;

	void set_text(const String &p_text);
	void set_edge_length(int p_edge_length) { edge_length = p_edge_length; }
	void set_margin(int p_margin) { margin = p_margin; }

	void draw(const Point2 &p_pos);

	FBLCDFontView(const RID &canvas_item);
};

class FBSquareFontView {
	Vector<FBFontSymbolType> symbols;
	int horizontal_edge_length;
	int vertical_edge_length;
	int margin;
	int line_width;
	int horizontal_padding;
	int vertical_padding;
	real_t glow_size;
	real_t inner_glow_size;
	int line_join;
	int line_cap;
	Color line_color;
	Color glow_color;
	Color inner_glow_color;
	String text;

	const RID &canvas_item;

public:
	Size2 size_of_contents() const;

	void set_text(const String &p_text);
	void set_margin(int p_margin) { margin = p_margin; }

	void draw(const Point2 &p_pos);

	FBSquareFontView(const RID &canvas_item);
};

#endif // FBFONTVIEW_H
