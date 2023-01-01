/*************************************************************************/
/*  fb_bitmap_font_view.cpp                                              */
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

#include "fb_font_draw.h"
#include "fb_font_symbol.h"
#include "fb_font_view.h"

// Reference:
// ----------
// 1. https://github.com/lyokato/FBDigitalFont/blob/master/FBDigitalFont/Classes/FBBitmapFontView.m#L84

#define color_from_rgb(rgb) Color::hex(0xff | ((rgb) << 8))

FBBitmapFontView::FBBitmapFontView(const RID &canvas_item, Dictionary &cache) :
		canvas_item(canvas_item), cache(cache) {
	dot_type = FBFontDotTypeSquare;
	edge_length = 10;
	margin = 2;
	glow_size = 0;
	inner_glow_size = 0;
	glow_color = color_from_rgb(0xee3300);
	inner_glow_color = color_from_rgb(0xee3300);
	on_color = color_from_rgb(0xffdd66);
	off_color = color_from_rgb(0x222222);

	number_of_left_padding_dot = 1;
	number_of_top_padding_dot = 0;
	number_of_right_padding_dot = 1;
	number_of_bottom_padding_dot = 0;
	number_of_padding_dots_between_digits = 1;

	ERR_FAIL_COND(!canvas_item.is_valid());
}

void FBBitmapFontView::set_text(const String &p_text) {
	text = p_text;
	symbols = symbols_for_string(text);
}

Size2 FBBitmapFontView::size_of_contents() const {
	const int vd = number_of_vertical_dot();
	const int hd = number_of_horizontal_dot();
	const real_t w = hd * edge_length + (hd - 1) * margin;
	const real_t h = vd * edge_length + (vd - 1) * margin;
	return { w, h };
}

int FBBitmapFontView::number_of_horizontal_dot() const {
	int total_dots_from_symbols = 0;
	for (const auto &symbol : symbols) {
		total_dots_from_symbols += number_of_dots_wide_for_symbol(symbol);
	}

	return total_dots_from_symbols + (number_of_padding_dots_between_digits * (symbols.size() - 1)) + number_of_left_padding_dot + number_of_right_padding_dot;
}

int FBBitmapFontView::number_of_vertical_dot() const {
	return 7 + number_of_top_padding_dot + number_of_bottom_padding_dot;
}

void FBBitmapFontView::draw() {
	// draw base dots:
	draw_background_with_dot_type(canvas_item, cache, dot_type, off_color, edge_length, margin, number_of_horizontal_dot(), number_of_vertical_dot());
	// draw shaded digits:
	real_t y = number_of_top_padding_dot * (edge_length + margin);
	real_t x = number_of_left_padding_dot * (edge_length + margin);
	for (int i = 0; i < symbols.size(); i++) {
		draw_bitmap_symbol(canvas_item, cache, symbols[i], dot_type, on_color, edge_length, margin, glow_color, glow_size, inner_glow_color, inner_glow_size, { x, y });
		const real_t number_wide = number_of_dots_wide_for_symbol(symbols[i]);
		x += (edge_length + margin) * (number_wide + number_of_padding_dots_between_digits);
	}
}
