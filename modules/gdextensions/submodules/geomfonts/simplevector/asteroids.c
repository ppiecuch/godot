/**************************************************************************/
/*  asteroids.c                                                           */
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

// Super simple font from Asteroids.
//
// http://www.edge-online.com/wp-content/uploads/edgeonline/oldfiles/images/feature_article/2009/05/asteroids2.jpg

#include <stdint.h>

typedef struct {
	uint8_t points[9]; // 4 bits x, 4 bits y
} asteroids_char_t;

#define FONT_UP 0xFE
#define FONT_LAST 0xFF
#define GLYPH_LAST 0x78

#define P(x, y) ((((x)&0xF) << 4) | (((y)&0xF) << 0))

const asteroids_char_t asteroids_font[] = {
	/* 00 */ { 0 },
	/* 01 */ { 0 },
	/* 02 */ { 0 },
	/* 03 */ { 0 },
	/* 04 */ { 0 },
	/* 05 */ { 0 },
	/* 06 */ { 0 },
	/* 07 */ { 0 },
	/* 08 */ { 0 },
	/* 09 */ { 0 },
	/* 0a */ { 0 },
	/* 0b */ { 0 },
	/* 0c */ { 0 },
	/* 0d */ { 0 },
	/* 0e */ { 0 },
	/* 0f */ { 0 },
	/* 10 */ { 0 },
	/* 11 */ { 0 },
	/* 12 */ { 0 },
	/* 13 */ { 0 },
	/* 14 */ { 0 },
	/* 15 */ { 0 },
	/* 16 */ { 0 },
	/* 17 */ { 0 },
	/* 18 */ { 0 },
	/* 19 */ { 0 },
	/* 1a */ { 0 },
	/* 1b */ { 0 },
	/* 1c */ { 0 },
	/* 1d */ { 0 },
	/* 1e */ { 0 },
	/* 1f */ { 0 },
	/* ' ' */ { 1, FONT_LAST },
	/* '!' */ { 1, P(4, 0), P(3, 2), P(5, 2), P(4, 0), FONT_UP, P(4, 4), P(4, 12), FONT_LAST },
	/* '"' */ { 1, P(2, 10), P(2, 6), FONT_UP, P(6, 10), P(6, 6), FONT_LAST },
	/* '#' */ { 1, P(0, 4), P(8, 4), P(6, 2), P(6, 10), P(8, 8), P(0, 8), P(2, 10), P(2, 2) },
	/* '$' */ { 1, P(6, 2), P(2, 6), P(6, 10), FONT_UP, P(4, 12), P(4, 0), FONT_LAST },
	/* '%' */ { 1, P(0, 0), P(8, 12), FONT_UP, P(2, 10), P(2, 8), FONT_UP, P(6, 4), P(6, 2) },
	/* '&' */ { 1, P(8, 0), P(4, 12), P(8, 8), P(0, 4), P(4, 0), P(8, 4), FONT_LAST },
	/* ''' */ { 1, P(2, 6), P(6, 10), FONT_LAST },
	/* '(' */ { 1, P(6, 0), P(2, 4), P(2, 8), P(6, 12), FONT_LAST },
	/* ')' */ { 1, P(2, 0), P(6, 4), P(6, 8), P(2, 12), FONT_LAST },
	/* '*' */ { 1, P(0, 0), P(4, 12), P(8, 0), P(0, 8), P(8, 8), P(0, 0), FONT_LAST },
	/* '+' */ { 1, P(1, 6), P(7, 6), FONT_UP, P(4, 9), P(4, 3), FONT_LAST },
	/* ',' */ { 1, P(2, 0), P(4, 2), FONT_LAST },
	/* '-' */ { 1, P(2, 6), P(6, 6), FONT_LAST },
	/* '.' */ { 1, P(3, 0), P(4, 0), FONT_LAST },
	/* '/' */ { 1, P(0, 0), P(8, 12), FONT_LAST },
	/* '0' */ { 1, P(0, 0), P(8, 0), P(8, 12), P(0, 12), P(0, 0), P(8, 12), FONT_LAST },
	/* '1' */ { 1, P(4, 0), P(4, 12), P(3, 10), FONT_LAST },
	/* '2' */ { 1, P(0, 12), P(8, 12), P(8, 7), P(0, 5), P(0, 0), P(8, 0), FONT_LAST },
	/* '3' */ { 1, P(0, 12), P(8, 12), P(8, 0), P(0, 0), FONT_UP, P(0, 6), P(8, 6), FONT_LAST },
	/* '4' */ { 1, P(0, 12), P(0, 6), P(8, 6), FONT_UP, P(8, 12), P(8, 0), FONT_LAST },
	/* '5' */ { 1, P(0, 0), P(8, 0), P(8, 6), P(0, 7), P(0, 12), P(8, 12), FONT_LAST },
	/* '6' */ { 1, P(0, 12), P(0, 0), P(8, 0), P(8, 5), P(0, 7), FONT_LAST },
	/* '7' */ { 1, P(0, 12), P(8, 12), P(8, 6), P(4, 0), FONT_LAST },
	/* '8' */ { 1, P(0, 0), P(8, 0), P(8, 12), P(0, 12), P(0, 0), FONT_UP, P(0, 6), P(8, 6) },
	/* '9' */ { 1, P(8, 0), P(8, 12), P(0, 12), P(0, 7), P(8, 5), FONT_LAST },
	/* ':' */ { 1, P(4, 9), P(4, 7), FONT_UP, P(4, 5), P(4, 3), FONT_LAST },
	/* ';' */ { 1, P(4, 9), P(4, 7), FONT_UP, P(4, 5), P(1, 2), FONT_LAST },
	/* '<' */ { 1, P(6, 0), P(2, 6), P(6, 12), FONT_LAST },
	/* '=' */ { 1, P(1, 4), P(7, 4), FONT_UP, P(1, 8), P(7, 8), FONT_LAST },
	/* '>' */ { 1, P(2, 0), P(6, 6), P(2, 12), FONT_LAST },
	/* '?' */ { 1, P(0, 8), P(4, 12), P(8, 8), P(4, 4), FONT_UP, P(4, 1), P(4, 0), FONT_LAST },
	/* '@' */ { 1, P(8, 4), P(4, 0), P(0, 4), P(0, 8), P(4, 12), P(8, 8), P(4, 4), P(3, 6) },
	/* 'A' */ { 1, P(0, 0), P(0, 8), P(4, 12), P(8, 8), P(8, 0), FONT_UP, P(0, 4), P(8, 4) },
	/* 'B' */ { 1, P(0, 0), P(0, 12), P(4, 12), P(8, 10), P(4, 6), P(8, 2), P(4, 0), P(0, 0) },
	/* 'C' */ { 1, P(8, 0), P(0, 0), P(0, 12), P(8, 12), FONT_LAST },
	/* 'D' */ { 1, P(0, 0), P(0, 12), P(4, 12), P(8, 8), P(8, 4), P(4, 0), P(0, 0), FONT_LAST },
	/* 'E' */ { 1, P(8, 0), P(0, 0), P(0, 12), P(8, 12), FONT_UP, P(0, 6), P(6, 6), FONT_LAST },
	/* 'F' */ { 1, P(0, 0), P(0, 12), P(8, 12), FONT_UP, P(0, 6), P(6, 6), FONT_LAST },
	/* 'G' */ { 1, P(6, 6), P(8, 4), P(8, 0), P(0, 0), P(0, 12), P(8, 12), FONT_LAST },
	/* 'H' */ { 1, P(0, 0), P(0, 12), FONT_UP, P(0, 6), P(8, 6), FONT_UP, P(8, 12), P(8, 0) },
	/* 'I' */ { 1, P(0, 0), P(8, 0), FONT_UP, P(4, 0), P(4, 12), FONT_UP, P(0, 12), P(8, 12) },
	/* 'J' */ { 1, P(0, 4), P(4, 0), P(8, 0), P(8, 12), FONT_LAST },
	/* 'K' */ { 1, P(0, 0), P(0, 12), FONT_UP, P(8, 12), P(0, 6), P(6, 0), FONT_LAST },
	/* 'L' */ { 1, P(8, 0), P(0, 0), P(0, 12), FONT_LAST },
	/* 'M' */ { 1, P(0, 0), P(0, 12), P(4, 8), P(8, 12), P(8, 0), FONT_LAST },
	/* 'N' */ { 1, P(0, 0), P(0, 12), P(8, 0), P(8, 12), FONT_LAST },
	/* 'O' */ { 1, P(0, 0), P(0, 12), P(8, 12), P(8, 0), P(0, 0), FONT_LAST },
	/* 'P' */ { 1, P(0, 0), P(0, 12), P(8, 12), P(8, 6), P(0, 5), FONT_LAST },
	/* 'Q' */ { 1, P(0, 0), P(0, 12), P(8, 12), P(8, 4), P(0, 0), FONT_UP, P(4, 4), P(8, 0) },
	/* 'R' */ { 1, P(0, 0), P(0, 12), P(8, 12), P(8, 6), P(0, 5), FONT_UP, P(4, 5), P(8, 0) },
	/* 'S' */ { 1, P(0, 2), P(2, 0), P(8, 0), P(8, 5), P(0, 7), P(0, 12), P(6, 12), P(8, 10) },
	/* 'T' */ { 1, P(0, 12), P(8, 12), FONT_UP, P(4, 12), P(4, 0), FONT_LAST },
	/* 'U' */ { 1, P(0, 12), P(0, 2), P(4, 0), P(8, 2), P(8, 12), FONT_LAST },
	/* 'V' */ { 1, P(0, 12), P(4, 0), P(8, 12), FONT_LAST },
	/* 'W' */ { 1, P(0, 12), P(2, 0), P(4, 4), P(6, 0), P(8, 12), FONT_LAST },
	/* 'X' */ { 1, P(0, 0), P(8, 12), FONT_UP, P(0, 12), P(8, 0), FONT_LAST },
	/* 'Y' */ { 1, P(0, 12), P(4, 6), P(8, 12), FONT_UP, P(4, 6), P(4, 0), FONT_LAST },
	/* 'Z' */ { 1, P(0, 12), P(8, 12), P(0, 0), P(8, 0), FONT_UP, P(2, 6), P(6, 6), FONT_LAST },
	/* '[' */ { 1, P(6, 0), P(2, 0), P(2, 12), P(6, 12), FONT_LAST },
	/* '\' */ { 1, P(0, 12), P(8, 0), FONT_LAST },
	/* ']' */ { 1, P(2, 0), P(6, 0), P(6, 12), P(2, 12), FONT_LAST },
	/* '^' */ { 1, P(2, 6), P(4, 12), P(6, 6), FONT_LAST },
	/* '_' */ { 1, P(0, 0), P(8, 0), FONT_LAST },
	/* '`' */ { 1, P(2, 10), P(6, 6), FONT_LAST },
	/* 61 */ { 0 },
	/* 62 */ { 0 },
	/* 63 */ { 0 },
	/* 64 */ { 0 },
	/* 65 */ { 0 },
	/* 66 */ { 0 },
	/* 67 */ { 0 },
	/* 68 */ { 0 },
	/* 69 */ { 0 },
	/* 6a */ { 0 },
	/* 6b */ { 0 },
	/* 6c */ { 0 },
	/* 6d */ { 0 },
	/* 6e */ { 0 },
	/* 6f */ { 0 },
	/* 70 */ { 0 },
	/* 71 */ { 0 },
	/* 72 */ { 0 },
	/* 73 */ { 0 },
	/* 74 */ { 0 },
	/* '{' */ { 1, P(6, 0), P(4, 2), P(4, 10), P(6, 12), FONT_UP, P(2, 6), P(4, 6), FONT_LAST },
	/* '|' */ { 1, P(4, 0), P(4, 5), FONT_UP, P(4, 6), P(4, 12), FONT_LAST },
	/* '}' */ { 1, P(4, 0), P(6, 2), P(6, 10), P(4, 12), FONT_UP, P(6, 6), P(8, 6), FONT_LAST },
	/* '~' */ { 1, P(0, 4), P(2, 8), P(6, 4), P(8, 8), FONT_LAST },
	/* 79 */
};

static Size2 _draw_asteroid_glyph(PoolVector2Array &data, uint8_t c, const Point2 &pos, const Vector2 &size) {
	if ('a' <= c && c <= 'z') {
		c -= 'a' - 'A'; // Asteroids font only has upper case
	}
	if (c > GLYPH_LAST || asteroids_font[c].points[0] == 0) {
		return Size2(); // invalid character
	}
	const uint8_t *const pts = asteroids_font[c].points;
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

static void _draw_asteroid_text(Ref<ArrayMesh> &mesh, Point2 pos, const Vector2 &size, const char *text) {
	PoolVector2Array data;
	char ch = *text;
	while ((ch = *text) != 0) {
		const Size2 adv = _draw_asteroid_glyph(data, ch, pos, size);
		pos.x += adv.x;
		text++;
	}
	Array mesh_array;
	mesh_array.resize(VS::ARRAY_MAX);
	mesh_array[VS::ARRAY_VERTEX] = data;
	mesh->add_surface_from_arrays(Mesh::PRIMITIVE_LINES, mesh_array, Array(), Mesh::ARRAY_FLAG_USE_2D_VERTICES);
}
