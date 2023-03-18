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
	LCDLeftButton = 16,
	LCDRightTop = 32,
	LCDRightButton = 64,
};

static uint8_t parts_map_for_lcd_symbol(FBFontSymbolType symbol) {
	switch (symbol) {
		case FBFontSymbol0:
			return LCDTop | LCDBottom | LCDLeftTop | LCDLeftButton | LCDRightTop | LCDRightButton;
		case FBFontSymbol1:
			return LCDRightTop | LCDRightButton;
		case FBFontSymbol2:
			return LCDTop | LCDBottom | LCDLeftButton | LCDRightTop | LCDCenter;
		case FBFontSymbol3:
			return LCDTop | LCDBottom | LCDRightTop | LCDRightButton | LCDCenter;
		case FBFontSymbol4:
			return LCDLeftTop | LCDRightTop | LCDRightButton | LCDCenter;
		case FBFontSymbol5:
			return LCDTop | LCDBottom | LCDLeftTop | LCDRightButton | LCDCenter;
		case FBFontSymbol6:
			return LCDTop | LCDBottom | LCDLeftTop | LCDLeftButton | LCDRightButton | LCDCenter;
		case FBFontSymbol7:
			return LCDTop | LCDRightTop | LCDRightButton;
		case FBFontSymbol8:
			return LCDTop | LCDBottom | LCDLeftTop | LCDLeftButton | LCDRightTop | LCDRightButton | LCDCenter;
		case FBFontSymbol9:
			return LCDTop | LCDBottom | LCDLeftTop | LCDRightTop | LCDRightButton | LCDCenter;
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
		Point2 start_point) {

	const int spacer = 1;
	const uint8_t parts = parts_map_for_lcd_symbol(symbol);

	if (parts & LCDTop) {
		Vector<Point2> seg;
		seg.push_back(start_point + Vector2(spacer, 0));
		seg.push_back(start_point + Vector2(edge_length - spacer, 0));
		seg.push_back(start_point + Vector2(edge_length - spacer - line_width, line_width));
		seg.push_back(start_point + Vector2(spacer + line_width, line_width));
		path.push_back(seg);
	}
}
