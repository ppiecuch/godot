/**************************************************************************/
/*  fb_square_font.cpp                                                    */
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

void path_square_symbol(
		Vector<Vector<Point2>> &path,
		FBFontSymbolType symbol,
		int horizontal_edge_length,
		int vertical_edge_length,
		const Point2 &start_point) {
	switch (symbol) {
		case FBFontSymbolDash: {
			Vector<Point2> seg;
			seg.push_back(start_point + Vector2(0, vertical_edge_length));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, vertical_edge_length));
			path.push_back(seg);
		} break;
		case FBFontSymbol0: {
			Vector<Point2> seg;
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, vertical_edge_length * 2));
			seg.push_back(start_point, Vector2(0, vertical_edge_length * 2));
			seg.push_back(seg[0]); // close
			path.push_back(seg);
		} break;
		case FBFontSymbol1: {
			Vector<Point2> seg;
			seg.push_back(start_point + Vector2(horizontal_edge_length, 0));
			seg.push_back(start_point + Vector2(horizontal_edge_length, vertical_edge_length * 2));
			path.push_back(seg);
		} break;
		case FBFontSymbol2: {
			Vector<Point2> seg;
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, vertical_edge_length));
			seg.push_back(start_point + Vector2(0, vertical_edge_length));
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			path.push_back(seg);
		} break;
		case FBFontSymbol3: {
			Vector<Point2> seg;
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			path.push_back(seg);

			seg.clear();
			seg.push_back(start_point + Vector2(0, vertical_edge_length));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, vertical_edge_length));
			path.push_back(seg);
		} break;
		case FBFontSymbol4: {
			Vector<Point2> seg;
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(0, vertical_edge_length));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, vertical_edge_length));
			path.push_back(seg);

			seg.clear();
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			path.push_back(seg);
		} break;
		case FBFontSymbol5: {
			Vector<Point2> seg;
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(0, vertical_edge_length));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, vertical_edge_length));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			path.push_back(seg);
		} break;
		case FBFontSymbol6: {
			Vector<Point2> seg;
			seg.push_back(start_point + Vector2(0, vertical_edge_length));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, vertical_edge_length));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			path.push_back(seg);
		} break;
		case FBFontSymbol7: {
			Vector<Point2> seg;
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			path.push_back(seg);
		} break;
		case FBFontSymbol8: {
			Vector<Point2> seg;
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			seg.push_back(seg[0]);
			path.push_back(seg);

			seg.clear();
			seg.push_back(start_point + Vector2(0, vertical_edge_length));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, vertical_edge_length));
			path.push_back(seg);
		} break;
		case FBFontSymbol9: {
			Vector<Point2> seg;
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, vertical_edge_length));
			seg.push_back(start_point + Vector2(0, vertical_edge_length));
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			path.push_back(seg);
		} break;
		case FBFontSymbolA: {
			Vector<Point2> seg;
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			path.push_back(seg);

			seg.clear();
			seg.push_back(start_point + Vector2(0, vertical_edge_length));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, vertical_edge_length));
			path.push_back(seg);
		} break;
		case FBFontSymbolB: {
			Vector<Point2> seg;
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			seg.push_back(seg[0]);
			path.push_back(seg);

			seg.clear();
			seg.push_back(start_point + Vector2(0, vertical_edge_length));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, vertical_edge_length));
			path.push_back(seg);
		} break;
		case FBFontSymbolC: {
			Vector<Point2> seg;
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			path.push_back(seg);
		} break;
		case FBFontSymbolD: {
			Vector<Point2> seg;
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 3 / 2, vertical_edge_length * 2));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, vertical_edge_length));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 3 / 2, 0));
			seg.push_back(seg[0]);
			path.push_back(seg);
		} break;
		case FBFontSymbolE: {
			Vector<Point2> seg;
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			path.push_back(seg);

			seg.clear();
			seg.push_back(start_point + Vector2(0, vertical_edge_length));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, vertical_edge_length));
			path.push_back(seg);
		} break;
		case FBFontSymbolF: {
			Vector<Point2> seg;
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			path.push_back(seg);

			seg.clear();
			seg.push_back(start_point + Vector2(0, vertical_edge_length));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, vertical_edge_length));
			path.push_back(seg);
		} break;
		case FBFontSymbolG: {
			Vector<Point2> seg;
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, vertical_edge_length));
			seg.push_back(start_point + Vector2(horizontal_edge_length, vertical_edge_length));
			path.push_back(seg);
		} break;
		case FBFontSymbolH: {
			Vector<Point2> seg;
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			path.push_back(seg);

			seg.clear();
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			path.push_back(seg);

			seg.clear();
			seg.push_back(start_point + Vector2(0, vertical_edge_length));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, vertical_edge_length));
			path.push_back(seg);
		} break;
		case FBFontSymbolI: {
			Vector<Point2> seg;
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			path.push_back(seg);

			seg.clear();
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			path.push_back(seg);

			seg.clear();
			seg.push_back(start_point + Vector2(horizontal_edge_length, 0));
			seg.push_back(start_point + Vector2(horizontal_edge_length, vertical_edge_length * 2));
			path.push_back(seg);
		} break;
		case FBFontSymbolJ: {
			Vector<Point2> seg;
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			path.push_back(seg);

			seg.clear();
			//Vector<Point2> seg;
			seg.push_back(start_point + Vector2(horizontal_edge_length * 3 / 2, 0));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 3 / 2, vertical_edge_length * 2));
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			path.push_back(seg);
		} break;
		case FBFontSymbolK: {
			/*Vector<Point2> seg;
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			path.push_back(seg);*/

			Vector<Point2> seg;
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point + Vector2(0, vertical_edge_length));
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			path.push_back(seg);

			seg.clear();
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			seg.push_back(start_point + Vector2(0, vertical_edge_length));
			seg.push_back(start_point);
			path.push_back(seg);
		} break;
		case FBFontSymbolL: {
			Vector<Point2> seg;
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			path.push_back(seg);
		} break;
		case FBFontSymbolM: {
			Vector<Point2> seg;
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			path.push_back(seg);

			seg.clear();
			seg.push_back(start_point + Vector2(horizontal_edge_length, 0));
			seg.push_back(start_point + Vector2(horizontal_edge_length, vertical_edge_length * 2));
			path.push_back(seg);
		} break;
		case FBFontSymbolN: {
			Vector<Point2> seg;
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			seg.push_back(start_point);

			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));

			/*seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));*/
			path.push_back(seg);
		} break;
		case FBFontSymbolO: {
			Vector<Point2> seg;
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			seg.push_back(seg[0]);
			path.push_back(seg);
		} break;
		case FBFontSymbolP: {
			Vector<Point2> seg;
			seg.push_back(start_point + Vector2(0, vertical_edge_length));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, vertical_edge_length));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			seg.push_back(seg[0]);
			path.push_back(seg);
		} break;
		case FBFontSymbolQ: {
			Vector<Point2> seg;
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			seg.push_back(seg[0]);
			path.push_back(seg);

			seg.clear();
			seg.push_back(start_point + Vector2(horizontal_edge_length, vertical_edge_length));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			path.push_back(seg);
		} break;
		case FBFontSymbolR: {
			Vector<Point2> seg;
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, vertical_edge_length));
			seg.push_back(start_point + Vector2(0, vertical_edge_length));
			seg.push_back(start_point + Vector2(horizontal_edge_length, vertical_edge_length));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			path.push_back(seg);
			/*Vector<Point2> seg;
			seg.push_back(start_point + Vector2(0, vertical_edge_length));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, vertical_edge_length));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			path.push_back(seg);

			seg.clear();
			seg.push_back(start_point + Vector2(horizontal_edge_length, vertical_edge_length));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			path.push_back(seg);*/
		} break;
		case FBFontSymbolS: {
			Vector<Point2> seg;
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(0, vertical_edge_length));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, vertical_edge_length));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			path.push_back(seg);
		} break;
		case FBFontSymbolT: {
			Vector<Point2> seg;
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			path.push_back(seg);

			seg.clear();
			seg.push_back(start_point + Vector2(horizontal_edge_length, 0));
			seg.push_back(start_point + Vector2(horizontal_edge_length, vertical_edge_length * 2));
			path.push_back(seg);
		} break;
		case FBFontSymbolU: {
			Vector<Point2> seg;
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			path.push_back(seg);
		} break;
		case FBFontSymbolV: {
			Vector<Point2> seg;
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(horizontal_edge_length, vertical_edge_length * 2));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			path.push_back(seg);
		} break;
		case FBFontSymbolW: {
			Vector<Point2> seg;
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			path.push_back(seg);

			seg.clear();
			seg.push_back(start_point + Vector2(horizontal_edge_length, 0));
			seg.push_back(start_point + Vector2(horizontal_edge_length, vertical_edge_length * 2));
			path.push_back(seg);
		} break;
		case FBFontSymbolX: {
			Vector<Point2> seg;
			seg.push_back(start_point);
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			path.push_back(seg);

			seg.clear();
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			path.push_back(seg);
		} break;
		case FBFontSymbolY: {
			Vector<Point2> seg;
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(horizontal_edge_length, vertical_edge_length));
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			path.push_back(seg);

			seg.clear();
			seg.push_back(start_point + Vector2(horizontal_edge_length, vertical_edge_length));
			seg.push_back(start_point + Vector2(horizontal_edge_length, vertical_edge_length * 2));
			path.push_back(seg);
		} break;
		case FBFontSymbolZ: {
			Vector<Point2> seg;
			seg.push_back(start_point);
			seg.push_back(start_point + Vector2(horizontal_edge_length * 2, 0));
			seg.push_back(start_point + Vector2(0, vertical_edge_length * 2));
			seg.push_back(start_point + 2 * Vector2(horizontal_edge_length, vertical_edge_length));
			path.push_back(seg);
		}
		default: {
			// Unsupported character
		} break;
	}
}
