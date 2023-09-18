/**************************************************************************/
/*  fb_lcd_font.cpp                                                       */
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

#include "core/image.h"
#include "core/math/math_defs.h"
#include "core/math/rect2.h"
#include "core/variant.h"
#include "servers/visual_server.h"

#include "fb_font_draw.h"

enum LCDParts {
	LCDTop = 1,
	LCDCenter = 2,
	LCDBottom = 4,
	LCDLeftTop = 8,
	LCDLeftBottom = 16,
	LCDRightTop = 32,
	LCDRightBottom = 64,
};

static uint8_t parts_map_for_lcd_symbol(FBFontSymbolType symbol) {
	switch (symbol) {
		case FBFontSymbol0:
			return LCDTop | LCDBottom | LCDLeftTop | LCDLeftBottom | LCDRightTop | LCDRightBottom;
		case FBFontSymbol1:
			return LCDRightTop | LCDRightBottom;
		case FBFontSymbol2:
			return LCDTop | LCDBottom | LCDLeftBottom | LCDRightTop | LCDCenter;
		case FBFontSymbol3:
			return LCDTop | LCDBottom | LCDRightTop | LCDRightBottom | LCDCenter;
		case FBFontSymbol4:
			return LCDLeftTop | LCDRightTop | LCDRightBottom | LCDCenter;
		case FBFontSymbol5:
			return LCDTop | LCDBottom | LCDLeftTop | LCDRightBottom | LCDCenter;
		case FBFontSymbol6:
			return LCDTop | LCDBottom | LCDLeftTop | LCDLeftBottom | LCDRightBottom | LCDCenter;
		case FBFontSymbol7:
			return LCDTop | LCDRightTop | LCDRightBottom;
		case FBFontSymbol8:
			return LCDTop | LCDBottom | LCDLeftTop | LCDLeftBottom | LCDRightTop | LCDRightBottom | LCDCenter;
		case FBFontSymbol9:
			return LCDTop | LCDBottom | LCDLeftTop | LCDRightTop | LCDRightBottom | LCDCenter;
		case FBFontSymbolDash:
			return LCDCenter;
		default:
			return 0;
	}
}

void path_lcd_symbol(
		Vector<Vector<Point2>> &path,
		FBFontSymbolType symbol,
		int edge_length,
		int line_width,
		Point2 start_point,
		bool rev) {
	const int spacer = 1;
	const uint8_t mask = rev ? 0xff : 0;
	const uint8_t parts = parts_map_for_lcd_symbol(symbol) ^ mask;

	if (parts & LCDTop) {
		Vector<Point2> seg;
		seg.push_back(start_point + Vector2(spacer, 0));
		seg.push_back(start_point + Vector2(edge_length - spacer, 0));
		seg.push_back(start_point + Vector2(edge_length - spacer - line_width, line_width));
		seg.push_back(start_point + Vector2(spacer + line_width, line_width));
		seg.push_back(seg[0]);
		path.push_back(seg);
	}
	if (parts & LCDCenter) {
		Vector<Point2> seg;
		seg.push_back(start_point + Vector2(spacer, edge_length));
		seg.push_back(start_point + Vector2(spacer * 2 + line_width / 2.0, edge_length - line_width / 2.0));
		seg.push_back(start_point + Vector2(edge_length - spacer * 2 - line_width / 2.0, edge_length - line_width / 2.0));
		seg.push_back(start_point + Vector2(edge_length - spacer, edge_length));
		seg.push_back(start_point + Vector2(edge_length - spacer * 2 - line_width / 2.0, edge_length + line_width / 2.0));
		seg.push_back(start_point + Vector2(spacer * 2 + line_width / 2.0, edge_length + line_width / 2.0));
		seg.push_back(seg[0]);
		path.push_back(seg);
	}
	if (parts & LCDBottom) {
		Vector<Point2> seg;
		seg.push_back(start_point + Vector2(spacer, edge_length * 2));
		seg.push_back(start_point + Vector2(edge_length - spacer, edge_length * 2));
		seg.push_back(start_point + Vector2(edge_length - spacer - line_width, edge_length * 2 - line_width));
		seg.push_back(start_point + Vector2(spacer + line_width, edge_length * 2 - line_width));
		seg.push_back(seg[0]);
		path.push_back(seg);
	}
	if (parts & LCDLeftTop) {
		Vector<Point2> seg;
		seg.push_back(start_point + Vector2(0, spacer));
		seg.push_back(start_point + Vector2(0, edge_length - spacer));
		seg.push_back(start_point + Vector2(line_width, edge_length - spacer * 2 - line_width / 2));
		seg.push_back(start_point + Vector2(line_width, spacer + line_width));
		seg.push_back(seg[0]);
		path.push_back(seg);
	}
	if (parts & LCDLeftBottom) {
		Vector<Point2> seg;
		seg.push_back(start_point + Vector2(0, edge_length * 2 - spacer));
		seg.push_back(start_point + Vector2(0, edge_length + spacer));
		seg.push_back(start_point + Vector2(line_width, edge_length + spacer * 2 + line_width / 2));
		seg.push_back(start_point + Vector2(line_width, edge_length * 2 - spacer - line_width));
		seg.push_back(seg[0]);
		path.push_back(seg);
	}
	if (parts & LCDRightTop) {
		Vector<Point2> seg;
		seg.push_back(start_point + Vector2(edge_length, spacer));
		seg.push_back(start_point + Vector2(edge_length, edge_length - spacer));
		seg.push_back(start_point + Vector2(edge_length - line_width, edge_length - spacer * 2 - line_width / 2));
		seg.push_back(start_point + Vector2(edge_length - line_width, spacer + line_width));
		seg.push_back(seg[0]);
		path.push_back(seg);
	}
	if (parts & LCDRightBottom) {
		Vector<Point2> seg;
		seg.push_back(start_point + Vector2(edge_length, edge_length * 2 - spacer));
		seg.push_back(start_point + Vector2(edge_length, edge_length + spacer));
		seg.push_back(start_point + Vector2(edge_length - line_width, edge_length + spacer * 2 + line_width / 2));
		seg.push_back(start_point + Vector2(edge_length - line_width, edge_length * 2 - spacer - line_width));
		seg.push_back(seg[0]);
		path.push_back(seg);
	}
}
