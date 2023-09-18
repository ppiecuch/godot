/**************************************************************************/
/*  fb_font_view.cpp                                                      */
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

#include "fb_font_view.h"
#include "fb_font_draw.h"
#include "fb_font_symbol.h"

// Reference:
// ----------
// 1. https://github.com/lyokato/FBDigitalFont/blob/master/FBDigitalFont/Classes/FBBitmapFontView.m#L84
// 2. https://github.com/verhovsky/squircle/blob/master/squircle.py
// 3. https://squircular.blogspot.com/2015/09/mapping-circle-to-square.html
// 4. https://dev.to/ndesmic/how-to-draw-squircles-and-superellipses-3d14
// 5. https://arxiv.org/vc/arxiv/papers/1604/1604.02174v1.pdf
// 6. https://codepen.io/mxfh/pen/gRLqXX
// 7. https://stackoverflow.com/questions/13211595/how-can-i-convert-coordinates-on-a-circle-to-coordinates-on-a-square

/// FBBitmapFontView

static _FORCE_INLINE_ Color color_from_rgb(uint32_t rgb, uint8_t alpha = 255) {
	return Color::hex(alpha | (rgb << 8));
}

static const Color _white = Color(1, 1, 1);

template <typename T>
std::vector<T> make_vector(size_t reservation) {
	std::vector<T> vec;
	vec.reserve(reservation);
	return vec;
}

FBBitmapFontView::FBBitmapFontView(const RID &canvas_item) :
		canvas_item(canvas_item) {
	dot_style = FBFontDotStyleFlatSquare;
	edge_length = 10;
	margin = 2;
	glow_size = 0;
	inner_glow_size = 0;
	glow_color = color_from_rgb(0xee3300, 0x40);
	inner_glow_color = color_from_rgb(0xee3300, 0x32);
	on_color = color_from_rgb(0xffdd66);
	off_color = color_from_rgb(0x222222);

	padding.number_of_left_dot = 0;
	padding.number_of_top_dot = 0;
	padding.number_of_right_dot = 0;
	padding.number_of_bottom_dot = 0;
	padding.number_of_between_dot = 1;

	ERR_FAIL_COND(!canvas_item.is_valid());
}

void FBBitmapFontView::set_text(const String &p_text) {
	text = p_text;
	symbols = symbols_for_string(text);

	_update_2d_view();
	// _dump_2d_view();
}

void FBBitmapFontView::_update_2d_view() {
	const int vd = number_of_vertical_dot();
	const int hd = number_of_horizontal_dot();
	_dots = std::vector<std::vector<bool>>(vd, std::vector<bool>(hd, false));
	int adv = 0;
	for (const auto &symbol : symbols) {
		const auto &coord = coord_for_symbol(symbol);
		for (int r = 0; r < coord.size(); r++) {
			for (int p = 0; p < padding.number_of_left_dot; p++) {
				_dots[r][adv + p] = false;
			}
			const auto &column = coord[r];
			for (int c = 0; c < column.size(); c++) {
				_dots[r][adv + padding.number_of_left_dot + c] = column[c];
			}
		}
		adv += number_of_dots_wide_for_symbol(symbol) + padding.number_of_between_dot + padding.number_of_right_dot;
	}
}

void FBBitmapFontView::_dump_2d_view(const char dot) {
	print_line(vformat("Text: '%s'", text));
	for (const auto &line : _dots) {
		std::vector<char> ln = make_vector<char>(line.size() + 1);
		for (auto &&b : line) {
			ln.push_back(b ? dot : ' ');
		}
		ln.push_back(0);
		print_line(&ln[0]);
	}
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

	return total_dots_from_symbols + (padding.number_of_between_dot * (symbols.size() - 1)) + padding.number_of_left_dot + padding.number_of_right_dot;
}

int FBBitmapFontView::number_of_vertical_dot() const {
	return get_font_height() + padding.number_of_top_dot + padding.number_of_bottom_dot;
}

void FBBitmapFontView::draw(const Point2 &p_pos) {
	draw_bitmap_map(canvas_item, _dots, dot_style, on_color, off_color, edge_length, margin, padding, inner_glow_color, glow_color, glow_size, p_pos);
}

/// FBSquareFontView

FBSquareFontView::FBSquareFontView(const RID &canvas_item) :
		canvas_item(canvas_item) {
	horizontal_padding = 5;
	vertical_padding = 5;
	horizontal_edge_length = 10;
	vertical_edge_length = 10;
	line_width = 2;
	line_join = CanvasItem::LINE_JOIN_ROUND;
	line_cap = CanvasItem::LINE_CAP_ROUND;
	margin = 5;
	line_color = _white;
	glow_color = _white;
	inner_glow_color = _white;
	glow_size = 0;
	inner_glow_size = 0;
}

Size2 FBSquareFontView::size_of_contents() const {
	const real_t w = horizontal_padding * 2 + (horizontal_edge_length * 2) * symbols.size() + margin * (symbols.size() - 1);
	const real_t h = vertical_padding * 2 + vertical_edge_length * 2;
	return { w, h };
}

void FBSquareFontView::set_text(const String &p_text) {
	text = p_text;
	symbols = symbols_for_string(text);
}

void FBSquareFontView::draw(const Point2 &p_pos) {
	const real_t x = p_pos.x + horizontal_padding;
	const real_t y = p_pos.y + vertical_padding;
	const real_t l = (horizontal_edge_length * 2) + margin;

	Vector<Vector<Point2>> path;
	for (int i = 0; i < symbols.size(); i++) {
		path_square_symbol(path, symbols[i], horizontal_edge_length, vertical_edge_length, { x + i * l, y });
	}
	if (path.size()) {
		const Vector<Color> color = make_vector(line_color);
		for (const auto &seg : path) {
			VisualServer::get_singleton()->canvas_item_add_polyline(canvas_item, seg, color, line_width, true, VisualServer::LineDrawMode(line_join), VisualServer::LineDrawMode(line_cap));
		}
	}
}

// FBLCDFontView

FBLCDFontView::FBLCDFontView(const RID &canvas_item) :
		canvas_item(canvas_item) {
	draw_off_segments = true;
	horizontal_padding = 5;
	vertical_padding = 5;
	edge_length = 10;
	line_width = 2;
	margin = 5;
	line_color = _white;
	glow_color = _white;
	inner_glow_color = _white;
	off_color = color_from_rgb(0x222222);
	glow_size = 0;
	inner_glow_size = 0;
}

Size2 FBLCDFontView::size_of_contents() const {
	const real_t w = horizontal_padding * 2 + edge_length * symbols.size() + margin * (symbols.size() - 1);
	const real_t h = vertical_padding * 2 + edge_length * 2;
	return { w, h };
}

void FBLCDFontView::set_text(const String &p_text) {
	text = p_text;
	symbols = symbols_for_string(text);
}

void FBLCDFontView::draw(const Point2 &p_pos) {
	const real_t x = p_pos.x + horizontal_padding;
	const real_t y = p_pos.y + vertical_padding;
	const real_t l = edge_length + margin;

	Vector<Vector<Point2>> path, path_off;
	for (int i = 0; i < symbols.size(); i++) {
		path_lcd_symbol(path, symbols[i], edge_length, line_width, { x + i * l, y });
		if (draw_off_segments) {
			path_lcd_symbol(path_off, symbols[i], edge_length, line_width, { x + i * l, y }, true);
		}
	}
	if (path.size()) {
		const Vector<Color> color = make_vector(line_color);
		for (const auto &seg : path) {
			VisualServer::get_singleton()->canvas_item_add_polygon(canvas_item, seg, color);
		}
	}
	if (path_off.size()) {
		const Vector<Color> color = make_vector(off_color);
		for (const auto &seg : path_off) {
			VisualServer::get_singleton()->canvas_item_add_polygon(canvas_item, seg, color);
		}
	}
}
