/*************************************************************************/
/*  fb_font_view.h                                                       */
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

#pragma once

#ifndef FBFONTVIEW_H
#define FBFONTVIEW_H

#include "fb_font_draw.h"
#include "fb_font_symbol.h"

#include "core/color.h"
#include "core/dictionary.h"
#include "core/vector.h"

class FBBitmapFontView {
	Vector<FBFontSymbolType> symbols;
	FBFontDotType dot_type;
	real_t edge_length;
	real_t margin;
	real_t glow_size;
	real_t inner_glow_size;
	int number_of_left_padding_dot;
	int number_of_top_padding_dot;
	int number_of_bottom_padding_dot;
	int number_of_right_padding_dot;
	int number_of_padding_dots_between_digits;
	Color off_color;
	Color on_color;
	Color glow_color;
	Color inner_glow_color;
	String text;

	int number_of_horizontal_dot() const;
	int number_of_vertical_dot() const;

	Dictionary &cache;

public:
	void draw(RID canvas);
	Size2 size_of_contents() const;

	void set_text(const String &p_text);
	void set_edge_length(real_t p_edge_length) { edge_length = p_edge_length; }
	void set_margin(real_t p_margin) { margin = p_margin; }

	FBBitmapFontView(Dictionary &cache);
};

class FBLCDFontView {
	bool draw_off_line;
	real_t edge_length;
	real_t margin;
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

public:
	Size2 size_of_contents() const;

	void set_text(const String &p_text);

	FBLCDFontView();
};

class FBSquareFontView {
	real_t horizontal_edge_length;
	real_t vertical_edge_length;
	real_t margin;
	real_t line_width;
	real_t horizontal_padding;
	real_t vertical_padding;
	real_t glow_size;
	real_t inner_glow_size;
	FBLineJoin line_join;
	FBLineCap line_cap;
	Color line_color;
	Color glow_color;
	Color inner_glow_color;
	String text;

public:
	Size2 size_of_contents();

	void set_text(const String &p_text);

	FBSquareFontView();
};

#endif // FBFONTVIEW_H
