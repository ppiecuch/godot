/*************************************************************************/
/*  asteroids.c                                                          */
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

// Super simple font from Asteroids.
//
// http://www.edge-online.com/wp-content/uploads/edgeonline/oldfiles/images/feature_article/2009/05/asteroids2.jpg

#include <stdint.h>

typedef struct {
	uint8_t points[8]; // 4 bits x, 4 bits y
} asteroids_char_t;

#define FONT_UP 0xFE
#define FONT_LAST 0xFF

#define P(x, y) ((((x)&0xF) << 4) | (((y)&0xF) << 0))

const asteroids_char_t asteroids_font[] = {
	/* 00 */ { },
	/* 01 */ { },
	/* 02 */ { },
	/* 03 */ { },
	/* 04 */ { },
	/* 05 */ { },
	/* 06 */ { },
	/* 07 */ { },
	/* 08 */ { },
	/* 09 */ { },
	/* 0a */ { },
	/* 0b */ { },
	/* 0c */ { },
	/* 0d */ { },
	/* 0e */ { },
	/* 0f */ { },
	/* 10 */ { },
	/* 11 */ { },
	/* 12 */ { },
	/* 13 */ { },
	/* 14 */ { },
	/* 15 */ { },
	/* 16 */ { },
	/* 17 */ { },
	/* 18 */ { },
	/* 19 */ { },
	/* 1a */ { },
	/* 1b */ { },
	/* 1c */ { },
	/* 1d */ { },
	/* 1e */ { },
	/* 1f */ { },
	/* '0' */ { P(0, 0), P(8, 0), P(8, 12), P(0, 12), P(0, 0), P(8, 12), FONT_LAST },
	/* '1' */ { P(4, 0), P(4, 12), P(3, 10), FONT_LAST },
	/* '2' */ { P(0, 12), P(8, 12), P(8, 7), P(0, 5), P(0, 0), P(8, 0), FONT_LAST },
	/* '3' */ { P(0, 12), P(8, 12), P(8, 0), P(0, 0), FONT_UP, P(0, 6), P(8, 6), FONT_LAST },
	/* '4' */ { P(0, 12), P(0, 6), P(8, 6), FONT_UP, P(8, 12), P(8, 0), FONT_LAST },
	/* '5' */ { P(0, 0), P(8, 0), P(8, 6), P(0, 7), P(0, 12), P(8, 12), FONT_LAST },
	/* '6' */ { P(0, 12), P(0, 0), P(8, 0), P(8, 5), P(0, 7), FONT_LAST },
	/* '7' */ { P(0, 12), P(8, 12), P(8, 6), P(4, 0), FONT_LAST },
	/* '8' */ { P(0, 0), P(8, 0), P(8, 12), P(0, 12), P(0, 0), FONT_UP, P(0, 6), P(8, 6) },
	/* '9' */ { P(8, 0), P(8, 12), P(0, 12), P(0, 7), P(8, 5), FONT_LAST },
	/* ' ' */ { FONT_LAST },
	/* '.' */ { P(3, 0), P(4, 0), FONT_LAST },
	/* ',' */ { P(2, 0), P(4, 2), FONT_LAST },
	/* '-' */ { P(2, 6), P(6, 6), FONT_LAST },
	/* '+' */ { P(1, 6), P(7, 6), FONT_UP, P(4, 9), P(4, 3), FONT_LAST },
	/* '!' */ { P(4, 0), P(3, 2), P(5, 2), P(4, 0), FONT_UP, P(4, 4), P(4, 12), FONT_LAST },
	/* '#' */ { P(0, 4), P(8, 4), P(6, 2), P(6, 10), P(8, 8), P(0, 8), P(2, 10), P(2, 2) },
	/* '^' */ { P(2, 6), P(4, 12), P(6, 6), FONT_LAST },
	/* '=' */ { P(1, 4), P(7, 4), FONT_UP, P(1, 8), P(7, 8), FONT_LAST },
	/* '*' */ { P(0, 0), P(4, 12), P(8, 0), P(0, 8), P(8, 8), P(0, 0), FONT_LAST },
	/* '_' */ { P(0, 0), P(8, 0), FONT_LAST },
	/* '/' */ { P(0, 0), P(8, 12), FONT_LAST },
	/* '\' */ { P(0, 12), P(8, 0), FONT_LAST },
	/* '@' */ { P(8, 4), P(4, 0), P(0, 4), P(0, 8), P(4, 12), P(8, 8), P(4, 4), P(3, 6) },
	/* '$' */ { P(6, 2), P(2, 6), P(6, 10), FONT_UP, P(4, 12), P(4, 0), FONT_LAST },
	/* '&' */ { P(8, 0), P(4, 12), P(8, 8), P(0, 4), P(4, 0), P(8, 4), FONT_LAST },
	/* '/' */ { P(6, 0), P(2, 0), P(2, 12), P(6, 12), FONT_LAST },
	/* ']' */ { P(2, 0), P(6, 0), P(6, 12), P(2, 12), FONT_LAST },
	/* '(' */ { P(6, 0), P(2, 4), P(2, 8), P(6, 12), FONT_LAST },
	/* ')' */ { P(2, 0), P(6, 4), P(6, 8), P(2, 12), FONT_LAST },
	/* '{' */ { P(6, 0), P(4, 2), P(4, 10), P(6, 12), FONT_UP, P(2, 6), P(4, 6), FONT_LAST },
	/* '}' */ { P(4, 0), P(6, 2), P(6, 10), P(4, 12), FONT_UP, P(6, 6), P(8, 6), FONT_LAST },
	/* '%' */ { P(0, 0), P(8, 12), FONT_UP, P(2, 10), P(2, 8), FONT_UP, P(6, 4), P(6, 2) },
	/* '<' */ { P(6, 0), P(2, 6), P(6, 12), FONT_LAST },
	/* '>' */ { P(2, 0), P(6, 6), P(2, 12), FONT_LAST },
	/* '|' */ { P(4, 0), P(4, 5), FONT_UP, P(4, 6), P(4, 12), FONT_LAST },
	/* ':' */ { P(4, 9), P(4, 7), FONT_UP, P(4, 5), P(4, 3), FONT_LAST },
	/* ';' */ { P(4, 9), P(4, 7), FONT_UP, P(4, 5), P(1, 2), FONT_LAST },
	/* '"' */ { P(2, 10), P(2, 6), FONT_UP, P(6, 10), P(6, 6), FONT_LAST },
	/* '\'' */ { P(2, 6), P(6, 10), FONT_LAST },
	/* '`' */ { P(2, 10), P(6, 6), FONT_LAST },
	/* '~' */ { P(0, 4), P(2, 8), P(6, 4), P(8, 8), FONT_LAST },
	/* '?' */ { P(0, 8), P(4, 12), P(8, 8), P(4, 4), FONT_UP, P(4, 1), P(4, 0), FONT_LAST },
	/* 'A' */ { P(0, 0), P(0, 8), P(4, 12), P(8, 8), P(8, 0), FONT_UP, P(0, 4), P(8, 4) },
	/* 'B' */ { P(0, 0), P(0, 12), P(4, 12), P(8, 10), P(4, 6), P(8, 2), P(4, 0), P(0, 0) },
	/* 'C' */ { P(8, 0), P(0, 0), P(0, 12), P(8, 12), FONT_LAST },
	/* 'D' */ { P(0, 0), P(0, 12), P(4, 12), P(8, 8), P(8, 4), P(4, 0), P(0, 0), FONT_LAST },
	/* 'E' */ { P(8, 0), P(0, 0), P(0, 12), P(8, 12), FONT_UP, P(0, 6), P(6, 6), FONT_LAST },
	/* 'F' */ { P(0, 0), P(0, 12), P(8, 12), FONT_UP, P(0, 6), P(6, 6), FONT_LAST },
	/* 'G' */ { P(6, 6), P(8, 4), P(8, 0), P(0, 0), P(0, 12), P(8, 12), FONT_LAST },
	/* 'H' */ { P(0, 0), P(0, 12), FONT_UP, P(0, 6), P(8, 6), FONT_UP, P(8, 12), P(8, 0) },
	/* 'I' */ { P(0, 0), P(8, 0), FONT_UP, P(4, 0), P(4, 12), FONT_UP, P(0, 12), P(8, 12) },
	/* 'J' */ { P(0, 4), P(4, 0), P(8, 0), P(8, 12), FONT_LAST },
	/* 'K' */ { P(0, 0), P(0, 12), FONT_UP, P(8, 12), P(0, 6), P(6, 0), FONT_LAST },
	/* 'L' */ { P(8, 0), P(0, 0), P(0, 12), FONT_LAST },
	/* 'M' */ { P(0, 0), P(0, 12), P(4, 8), P(8, 12), P(8, 0), FONT_LAST },
	/* 'N' */ { P(0, 0), P(0, 12), P(8, 0), P(8, 12), FONT_LAST },
	/* 'O' */ { P(0, 0), P(0, 12), P(8, 12), P(8, 0), P(0, 0), FONT_LAST },
	/* 'P' */ { P(0, 0), P(0, 12), P(8, 12), P(8, 6), P(0, 5), FONT_LAST },
	/* 'Q' */ { P(0, 0), P(0, 12), P(8, 12), P(8, 4), P(0, 0), FONT_UP, P(4, 4), P(8, 0) },
	/* 'R' */ { P(0, 0), P(0, 12), P(8, 12), P(8, 6), P(0, 5), FONT_UP, P(4, 5), P(8, 0) },
	/* 'S' */ { P(0, 2), P(2, 0), P(8, 0), P(8, 5), P(0, 7), P(0, 12), P(6, 12), P(8, 10) },
	/* 'T' */ { P(0, 12), P(8, 12), FONT_UP, P(4, 12), P(4, 0), FONT_LAST },
	/* 'U' */ { P(0, 12), P(0, 2), P(4, 0), P(8, 2), P(8, 12), FONT_LAST },
	/* 'V' */ { P(0, 12), P(4, 0), P(8, 12), FONT_LAST },
	/* 'W' */ { P(0, 12), P(2, 0), P(4, 4), P(6, 0), P(8, 12), FONT_LAST },
	/* 'X' */ { P(0, 0), P(8, 12), FONT_UP, P(0, 12), P(8, 0), FONT_LAST },
	/* 'Y' */ { P(0, 12), P(4, 6), P(8, 12), FONT_UP, P(4, 6), P(4, 0), FONT_LAST },
	/* 'Z' */ { P(0, 12), P(8, 12), P(0, 0), P(8, 0), FONT_UP, P(2, 6), P(6, 6), FONT_LAST },
};

static Size2 _draw_asteroid_glyph(PoolVector2Array &data, char c, const Point2 &pos, const Vector2 &size) {
	if ('a' <= c && c <= 'z') {
		c -= 'a' - 'A'; // Asteroids font only has upper case
	}
	const uint8_t *const pts = asteroids_font[c - ' '].points;
	bool next_moveto = true;
	Point2 cursor;

	for (int i = 0; i < 8; i++) {
		const uint8_t op = pts[i];
		switch (op) {
			case FONT_LAST:
				break;
			case FONT_UP:
				next_moveto = true;
				continue;
		}
		const Vector2 delta = { ((op >> 4) & 0xF) * size.x, ((op >> 0) & 0xF) * size.y };

		if (!next_moveto) {
			data.push_back(cursor);
			data.push_back(pos + delta);
		}
		cursor = pos + delta;
		next_moveto = false;
	}

	return 12 * size;
}
