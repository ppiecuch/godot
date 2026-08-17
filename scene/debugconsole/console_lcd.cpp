/**************************************************************************/
/*  console_lcd.cpp                                                       */
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

#include "console_lcd.h"

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

// One character of the LCD "big font": three rows of up to three CP437 codes, left-aligned
// and padded with 0x20 (space). `width` is the cursor advance in cells -- three for every
// glyph except the space, which is two, exactly as LcdBoxProc() stepped it.
//
// Ported verbatim from the `boxf`/`boxf_offsets` tables at textUI.c:13371, reindexed by the
// printable ASCII range so a lookup is a subtraction rather than an offset table. Lowercase
// duplicates uppercase (as it did in the source), and `<` `>` `^` and undefined codes are
// blank there and stay blank here. The original's one typo -- digit 4's middle row wrote a
// truncated "\xc0\xc4\xb" (byte 0x0b) where it plainly meant 0xb4 (the box char that closes
// the crossbar down into the stem) -- is corrected.
struct LcdGlyph {
	uint8_t width;
	uint8_t rows[3][3];
};

// Printable ASCII 0x20..0x7F.
static const LcdGlyph LCD_FONT[96] = {
	{ 2, { { 0x20, 0x20, 0x20 }, { 0x20, 0x20, 0x20 }, { 0x20, 0x20, 0x20 } } }, // ' '
	{ 3, { { 0xb3, 0x20, 0x20 }, { 0xb3, 0x20, 0x20 }, { 0x07, 0x20, 0x20 } } }, // '!'
	{ 3, { { 0xb3, 0xb3, 0x20 }, { 0x20, 0x20, 0x20 }, { 0x20, 0x20, 0x20 } } }, // '"'
	{ 3, { { 0x20, 0x20, 0x20 }, { 0x20, 0x20, 0x20 }, { 0x20, 0x20, 0x20 } } }, // '#'
	{ 3, { { 0xda, 0xc2, 0xbf }, { 0xc0, 0xc5, 0xbf }, { 0xc0, 0xc1, 0xd9 } } }, // '$'
	{ 3, { { 0xda, 0xbf, 0xb3 }, { 0xda, 0xc4, 0xd9 }, { 0xb3, 0xc0, 0xd9 } } }, // '%'
	{ 3, { { 0xda, 0xbf, 0x20 }, { 0xc3, 0xc5, 0xc4 }, { 0xc0, 0xc4, 0xd9 } } }, // '&'
	{ 3, { { 0xb3, 0x20, 0x20 }, { 0x20, 0x20, 0x20 }, { 0x20, 0x20, 0x20 } } }, // '\''
	{ 3, { { 0xda, 0xc4, 0x20 }, { 0xb3, 0x20, 0x20 }, { 0xc0, 0xc4, 0x20 } } }, // '('
	{ 3, { { 0xc4, 0xbf, 0x20 }, { 0x20, 0xb3, 0x20 }, { 0xc4, 0xd9, 0x20 } } }, // ')'
	{ 3, { { 0xb3, 0x20, 0xb3 }, { 0xc4, 0xc5, 0xc4 }, { 0xb3, 0x20, 0xb3 } } }, // '*'
	{ 3, { { 0x20, 0xb3, 0x20 }, { 0xc4, 0xc5, 0xc4 }, { 0x20, 0xb3, 0x20 } } }, // '+'
	{ 3, { { 0x20, 0x20, 0x20 }, { 0x20, 0x20, 0x20 }, { 0x20, 0xd9, 0x20 } } }, // ','
	{ 3, { { 0x20, 0x20, 0x20 }, { 0xc4, 0xc4, 0xc4 }, { 0x20, 0x20, 0x20 } } }, // '-'
	{ 3, { { 0x20, 0x20, 0x20 }, { 0x20, 0x20, 0x20 }, { 0xb3, 0x20, 0x20 } } }, // '.'
	{ 3, { { 0x20, 0xb3, 0x20 }, { 0xda, 0xd9, 0x20 }, { 0xb3, 0x20, 0x20 } } }, // '/'
	{ 3, { { 0xda, 0xc4, 0xbf }, { 0xb3, 0xb3, 0xb3 }, { 0xc0, 0xc4, 0xd9 } } }, // '0'
	{ 3, { { 0xc4, 0xbf, 0x20 }, { 0x20, 0xb3, 0x20 }, { 0xc4, 0xc1, 0xc4 } } }, // '1'
	{ 3, { { 0xda, 0xc4, 0xbf }, { 0xda, 0xc4, 0xd9 }, { 0xc0, 0xc4, 0xc4 } } }, // '2'
	{ 3, { { 0xda, 0xc4, 0xbf }, { 0xc4, 0xc4, 0xb4 }, { 0xc0, 0xc4, 0xd9 } } }, // '3'
	{ 3, { { 0xb3, 0x20, 0xb3 }, { 0xc0, 0xc4, 0xb4 }, { 0x20, 0x20, 0xb3 } } }, // '4'
	{ 3, { { 0xda, 0xc4, 0xc4 }, { 0xc0, 0xc4, 0xbf }, { 0xc0, 0xc4, 0xd9 } } }, // '5'
	{ 3, { { 0xda, 0xc4, 0xbf }, { 0xc3, 0xc4, 0xbf }, { 0xc0, 0xc4, 0xd9 } } }, // '6'
	{ 3, { { 0xda, 0xc4, 0xbf }, { 0x20, 0x20, 0xb3 }, { 0x20, 0x20, 0xb3 } } }, // '7'
	{ 3, { { 0xda, 0xc4, 0xbf }, { 0xc3, 0xc4, 0xb4 }, { 0xc0, 0xc4, 0xd9 } } }, // '8'
	{ 3, { { 0xda, 0xc4, 0xbf }, { 0xc0, 0xc4, 0xb4 }, { 0xc0, 0xc4, 0xd9 } } }, // '9'
	{ 3, { { 0x20, 0x20, 0x20 }, { 0xb3, 0x20, 0x20 }, { 0xb3, 0x20, 0x20 } } }, // ':'
	{ 3, { { 0x20, 0x20, 0x20 }, { 0x20, 0xb3, 0x20 }, { 0x20, 0xd9, 0x20 } } }, // ';'
	{ 3, { { 0x20, 0x20, 0x20 }, { 0x20, 0x20, 0x20 }, { 0x20, 0x20, 0x20 } } }, // '<'
	{ 3, { { 0x20, 0x20, 0x20 }, { 0xc4, 0xc4, 0xc4 }, { 0xc4, 0xc4, 0xc4 } } }, // '='
	{ 3, { { 0x20, 0x20, 0x20 }, { 0x20, 0x20, 0x20 }, { 0x20, 0x20, 0x20 } } }, // '>'
	{ 3, { { 0xda, 0xc4, 0xbf }, { 0x20, 0xc4, 0xd9 }, { 0x20, 0xb3, 0x20 } } }, // '?'
	{ 3, { { 0x20, 0x20, 0x20 }, { 0x20, 0x20, 0x20 }, { 0x3c, 0x20, 0x20 } } }, // '@'
	{ 3, { { 0xda, 0xc4, 0xbf }, { 0xc3, 0xc4, 0xb4 }, { 0xb3, 0x20, 0xb3 } } }, // 'A'
	{ 3, { { 0xda, 0xbf, 0x20 }, { 0xc3, 0xc1, 0xbf }, { 0xc0, 0xc4, 0xd9 } } }, // 'B'
	{ 3, { { 0xda, 0xc4, 0xc4 }, { 0xb3, 0x20, 0x20 }, { 0xc0, 0xc4, 0xc4 } } }, // 'C'
	{ 3, { { 0xc4, 0xc2, 0xbf }, { 0x20, 0xb3, 0xb3 }, { 0xc4, 0xc1, 0xd9 } } }, // 'D'
	{ 3, { { 0xda, 0xc4, 0xc4 }, { 0xc3, 0xc4, 0x20 }, { 0xc0, 0xc4, 0xc4 } } }, // 'E'
	{ 3, { { 0xda, 0xc4, 0xc4 }, { 0xc3, 0xc4, 0x20 }, { 0xb3, 0x20, 0x20 } } }, // 'F'
	{ 3, { { 0xda, 0xc4, 0xc4 }, { 0xb3, 0xc4, 0xbf }, { 0xc0, 0xc4, 0xd9 } } }, // 'G'
	{ 3, { { 0xb3, 0x20, 0xb3 }, { 0xc3, 0xc4, 0xb4 }, { 0xb3, 0x20, 0xb3 } } }, // 'H'
	{ 3, { { 0xb3, 0x20, 0x20 }, { 0xb3, 0x20, 0x20 }, { 0xb3, 0x20, 0x20 } } }, // 'I'
	{ 3, { { 0x20, 0xda, 0xbf }, { 0x20, 0x20, 0xb3 }, { 0xc0, 0xc4, 0xd9 } } }, // 'J'
	{ 3, { { 0xb3, 0xda, 0x20 }, { 0xc3, 0xc1, 0xbf }, { 0xb3, 0x20, 0xb3 } } }, // 'K'
	{ 3, { { 0xb3, 0x20, 0x20 }, { 0xb3, 0x20, 0x20 }, { 0xc0, 0xc4, 0xc4 } } }, // 'L'
	{ 3, { { 0xda, 0xc2, 0xbf }, { 0xb3, 0xb3, 0xb3 }, { 0xb3, 0x20, 0xb3 } } }, // 'M'
	{ 3, { { 0xda, 0xbf, 0xb3 }, { 0xb3, 0xc0, 0xb4 }, { 0xb3, 0x20, 0xb3 } } }, // 'N'
	{ 3, { { 0xda, 0xc4, 0xbf }, { 0xb3, 0x20, 0xb3 }, { 0xc0, 0xc4, 0xd9 } } }, // 'O'
	{ 3, { { 0xda, 0xc4, 0xbf }, { 0xc3, 0xc4, 0xd9 }, { 0xb3, 0x20, 0x20 } } }, // 'P'
	{ 3, { { 0xda, 0xc4, 0xbf }, { 0xb3, 0xbf, 0xb3 }, { 0xc0, 0xc1, 0xd9 } } }, // 'Q'
	{ 3, { { 0xda, 0xc4, 0xbf }, { 0xc3, 0xc2, 0xd9 }, { 0xb3, 0xc0, 0xc4 } } }, // 'R'
	{ 3, { { 0xda, 0xc4, 0xbf }, { 0xc0, 0xc4, 0xbf }, { 0xc0, 0xc4, 0xd9 } } }, // 'S'
	{ 3, { { 0xc4, 0xc2, 0xc4 }, { 0x20, 0xb3, 0x20 }, { 0x20, 0xb3, 0x20 } } }, // 'T'
	{ 3, { { 0xb3, 0x20, 0xb3 }, { 0xb3, 0x20, 0xb3 }, { 0xc0, 0xc4, 0xd9 } } }, // 'U'
	{ 3, { { 0xb3, 0x20, 0xb3 }, { 0xb3, 0xda, 0xd9 }, { 0xc0, 0xd9, 0x20 } } }, // 'V'
	{ 3, { { 0xb3, 0x20, 0xb3 }, { 0xb3, 0xb3, 0xb3 }, { 0xc0, 0xc1, 0xd9 } } }, // 'W'
	{ 3, { { 0xb3, 0x20, 0xb3 }, { 0xda, 0xc5, 0xd9 }, { 0xb3, 0x20, 0xb3 } } }, // 'X'
	{ 3, { { 0xb3, 0x20, 0xb3 }, { 0xc0, 0xc2, 0xd9 }, { 0x20, 0xb3, 0x20 } } }, // 'Y'
	{ 3, { { 0xc4, 0xc4, 0xbf }, { 0xda, 0xc4, 0xd9 }, { 0xc0, 0xc4, 0xc4 } } }, // 'Z'
	{ 3, { { 0xda, 0xc4, 0x20 }, { 0xb3, 0x20, 0x20 }, { 0xc0, 0xc4, 0x20 } } }, // '['
	{ 3, { { 0xb3, 0x20, 0x20 }, { 0xc0, 0xbf, 0x20 }, { 0x20, 0xb3, 0x20 } } }, // '\\'
	{ 3, { { 0x20, 0xc4, 0xbf }, { 0x20, 0x20, 0xb3 }, { 0x20, 0xc4, 0xd9 } } }, // ']'
	{ 3, { { 0x20, 0x20, 0x20 }, { 0x20, 0x20, 0x20 }, { 0x20, 0x20, 0x20 } } }, // '^'
	{ 3, { { 0x20, 0x20, 0x20 }, { 0x20, 0x20, 0x20 }, { 0xc4, 0xc4, 0xc4 } } }, // '_'
	{ 3, { { 0x20, 0xbf, 0x20 }, { 0x20, 0x20, 0x20 }, { 0x20, 0x20, 0x20 } } }, // '`'
	{ 3, { { 0xda, 0xc4, 0xbf }, { 0xc3, 0xc4, 0xb4 }, { 0xb3, 0x20, 0xb3 } } }, // 'a'
	{ 3, { { 0xda, 0xbf, 0x20 }, { 0xc3, 0xc1, 0xbf }, { 0xc0, 0xc4, 0xd9 } } }, // 'b'
	{ 3, { { 0xda, 0xc4, 0xc4 }, { 0xb3, 0x20, 0x20 }, { 0xc0, 0xc4, 0xc4 } } }, // 'c'
	{ 3, { { 0xc4, 0xc2, 0xbf }, { 0x20, 0xb3, 0xb3 }, { 0xc4, 0xc1, 0xd9 } } }, // 'd'
	{ 3, { { 0xda, 0xc4, 0xc4 }, { 0xc3, 0xc4, 0x20 }, { 0xc0, 0xc4, 0xc4 } } }, // 'e'
	{ 3, { { 0xda, 0xc4, 0xc4 }, { 0xc3, 0xc4, 0x20 }, { 0xb3, 0x20, 0x20 } } }, // 'f'
	{ 3, { { 0xda, 0xc4, 0xc4 }, { 0xb3, 0xc4, 0xbf }, { 0xc0, 0xc4, 0xd9 } } }, // 'g'
	{ 3, { { 0xb3, 0x20, 0xb3 }, { 0xc3, 0xc4, 0xb4 }, { 0xb3, 0x20, 0xb3 } } }, // 'h'
	{ 3, { { 0xb3, 0x20, 0x20 }, { 0xb3, 0x20, 0x20 }, { 0xb3, 0x20, 0x20 } } }, // 'i'
	{ 3, { { 0x20, 0xda, 0xbf }, { 0x20, 0x20, 0xb3 }, { 0xc0, 0xc4, 0xd9 } } }, // 'j'
	{ 3, { { 0xb3, 0xda, 0x20 }, { 0xc3, 0xc1, 0xbf }, { 0xb3, 0x20, 0xb3 } } }, // 'k'
	{ 3, { { 0xb3, 0x20, 0x20 }, { 0xb3, 0x20, 0x20 }, { 0xc0, 0xc4, 0xc4 } } }, // 'l'
	{ 3, { { 0xda, 0xc2, 0xbf }, { 0xb3, 0xb3, 0xb3 }, { 0xb3, 0x20, 0xb3 } } }, // 'm'
	{ 3, { { 0xda, 0xbf, 0xb3 }, { 0xb3, 0xc0, 0xb4 }, { 0xb3, 0x20, 0xb3 } } }, // 'n'
	{ 3, { { 0xda, 0xc4, 0xbf }, { 0xb3, 0x20, 0xb3 }, { 0xc0, 0xc4, 0xd9 } } }, // 'o'
	{ 3, { { 0xda, 0xc4, 0xbf }, { 0xc3, 0xc4, 0xd9 }, { 0xb3, 0x20, 0x20 } } }, // 'p'
	{ 3, { { 0xda, 0xc4, 0xbf }, { 0xb3, 0xbf, 0xb3 }, { 0xc0, 0xc1, 0xd9 } } }, // 'q'
	{ 3, { { 0xda, 0xc4, 0xbf }, { 0xc3, 0xc2, 0xd9 }, { 0xb3, 0xc0, 0xc4 } } }, // 'r'
	{ 3, { { 0xda, 0xc4, 0xbf }, { 0xc0, 0xc4, 0xbf }, { 0xc0, 0xc4, 0xd9 } } }, // 's'
	{ 3, { { 0xc4, 0xc2, 0xc4 }, { 0x20, 0xb3, 0x20 }, { 0x20, 0xb3, 0x20 } } }, // 't'
	{ 3, { { 0xb3, 0x20, 0xb3 }, { 0xb3, 0x20, 0xb3 }, { 0xc0, 0xc4, 0xd9 } } }, // 'u'
	{ 3, { { 0xb3, 0x20, 0xb3 }, { 0xb3, 0xda, 0xd9 }, { 0xc0, 0xd9, 0x20 } } }, // 'v'
	{ 3, { { 0xb3, 0x20, 0xb3 }, { 0xb3, 0xb3, 0xb3 }, { 0xc0, 0xc1, 0xd9 } } }, // 'w'
	{ 3, { { 0xb3, 0x20, 0xb3 }, { 0xda, 0xc5, 0xd9 }, { 0xb3, 0x20, 0xb3 } } }, // 'x'
	{ 3, { { 0xb3, 0x20, 0xb3 }, { 0xc0, 0xc2, 0xd9 }, { 0x20, 0xb3, 0x20 } } }, // 'y'
	{ 3, { { 0xc4, 0xc4, 0xbf }, { 0xda, 0xc4, 0xd9 }, { 0xc0, 0xc4, 0xc4 } } }, // 'z'
	{ 3, { { 0x20, 0xda, 0xc4 }, { 0xc4, 0xb4, 0x20 }, { 0x20, 0xc0, 0xc4 } } }, // '{'
	{ 3, { { 0xb3, 0x20, 0x20 }, { 0xb3, 0x20, 0x20 }, { 0xb3, 0x20, 0x20 } } }, // '|'
	{ 3, { { 0xc4, 0xbf, 0x20 }, { 0x20, 0xc3, 0xc4 }, { 0xc4, 0xd9, 0x20 } } }, // '}'
	{ 3, { { 0x20, 0x20, 0x20 }, { 0xda, 0xc4, 0xd9 }, { 0x20, 0x20, 0x20 } } }, // '~'
	{ 3, { { 0x20, 0x20, 0x20 }, { 0x20, 0x20, 0x20 }, { 0x20, 0x20, 0x20 } } }, // 0x7f
};

// Codes outside the printable range render as an empty three-wide cell, which is how
// boxf_offsets[] (0 everywhere but 0x20..0x7F) fell through for them in the original.
static const LcdGlyph LCD_BLANK = { 3, { { 0x20, 0x20, 0x20 }, { 0x20, 0x20, 0x20 }, { 0x20, 0x20, 0x20 } } };

enum { LCD_ROWS = 3 };

static const LcdGlyph &lcd_glyph(uint8_t p_code) {
	if (p_code >= 0x20 && p_code < 0x80) {
		return LCD_FONT[p_code - 0x20];
	}
	return LCD_BLANK;
}

Size2i console_lcd_size(const String &p_text) {
	const CharString ascii = p_text.ascii();
	int width = 0;
	for (int i = 0; i < ascii.length(); ++i) {
		width += lcd_glyph(uint8_t(ascii[i])).width;
	}
	return width > 0 ? Size2i(width, LCD_ROWS) : Size2i(0, 0);
}

void console_lcd_draw(TextConsole &p_console, int p_x, int p_y, const String &p_text,
		TextConsole::ColorIndex p_lit, TextConsole::ColorIndex p_dim) {
	const CharString ascii = p_text.ascii();
	int cx = p_x;
	for (int i = 0; i < ascii.length(); ++i) {
		const LcdGlyph &g = lcd_glyph(uint8_t(ascii[i]));
		// Every cell of the advance is written, spaces included: the blank cells carry the
		// panel colour so the readout reads as lit-on-dim rather than lit-on-whatever-was-
		// underneath. put_cell() clips each one, so an off-grid glyph loses only its outside
		// cells.
		for (int row = 0; row < LCD_ROWS; ++row) {
			for (int col = 0; col < g.width; ++col) {
				const uint8_t code = (col < 3) ? g.rows[row][col] : 0x20;
				p_console.put_cell(cx + col, p_y + row, code, p_lit, p_dim, TextConsole::BANK_CP437);
			}
		}
		cx += g.width;
	}
}

// --- Tests ---------------------------------------------------------------------------------

#ifdef DOCTEST

namespace {

Ref<TextConsole> lcd_test_console(int p_cols = 40, int p_rows = 16) {
	Ref<TextConsole> console = Ref<TextConsole>(memnew(TextConsole));
	console->load_font(TextConsole::DOS_8x16);
	console->resize(p_cols, p_rows);
	console->clear();
	return console;
}

// The glyph a cell holds, or -1 when the cell is off-grid or empty.
int cell_code(const Ref<TextConsole> &p_console, int p_x, int p_y) {
	TextConsole::cell c;
	if (!p_console->get_cell(p_x, p_y, c)) {
		return -1;
	}
	return uint8_t(c.character);
}

} // namespace

TEST_CASE("Console direct write") {
	Ref<TextConsole> console = lcd_test_console();

	SUBCASE("put_cell writes the glyph and colours back verbatim") {
		console->put_cell(2, 1, 'A', TextConsole::COLOR_LIGHTGREEN, TextConsole::COLOR_BLUE, TextConsole::BANK_CP437);
		TextConsole::cell c;
		REQUIRE(console->get_cell(2, 1, c));
		REQUIRE(uint8_t(c.character) == uint8_t('A'));
		REQUIRE(c.foreground == TextConsole::COLOR_LIGHTGREEN);
		REQUIRE(c.background == TextConsole::COLOR_BLUE);
		REQUIRE(c.bank == TextConsole::BANK_CP437);
	}

	SUBCASE("out-of-range writes clip silently at all four edges") {
		const bool was_muted = _print_error_enabled;
		_print_error_enabled = false; // put_cell must not ERR_FAIL, but guard the read-backs too
		console->put_cell(-1, 0, 'X', TextConsole::COLOR_WHITE, TextConsole::COLOR_BLACK);
		console->put_cell(0, -1, 'X', TextConsole::COLOR_WHITE, TextConsole::COLOR_BLACK);
		console->put_cell(console->get_console_size().width, 0, 'X', TextConsole::COLOR_WHITE, TextConsole::COLOR_BLACK);
		console->put_cell(0, console->get_console_size().height, 'X', TextConsole::COLOR_WHITE, TextConsole::COLOR_BLACK);
		_print_error_enabled = was_muted;
		// Nothing landed anywhere on the grid.
		for (int y = 0; y < console->get_console_size().height; ++y) {
			for (int x = 0; x < console->get_console_size().width; ++x) {
				REQUIRE(cell_code(console, x, y) == 0);
			}
		}
		// And the off-grid reads report failure rather than touching the out param.
		TextConsole::cell probe = { 'Z', TextConsole::COLOR_RED, TextConsole::COLOR_RED, TextConsole::BANK_CP437, false };
		REQUIRE_FALSE(console->get_cell(-1, 0, probe));
		REQUIRE(uint8_t(probe.character) == uint8_t('Z'));
	}

	SUBCASE("put_text honours the escape grammar and clips both ends") {
		// \x01 toggles to BANK_PATCH; the doubled \x01\x01 is a literal patch code.
		console->put_text(0, 0, "a\x01"
								"b\x01"
								"c",
				TextConsole::COLOR_WHITE, TextConsole::COLOR_BLACK);
		TextConsole::cell c;
		REQUIRE(console->get_cell(0, 0, c));
		REQUIRE(c.bank == TextConsole::BANK_CP437); // 'a'
		REQUIRE(console->get_cell(1, 0, c));
		REQUIRE(c.bank == TextConsole::BANK_PATCH); // 'b' after the toggle
		REQUIRE(console->get_cell(2, 0, c));
		REQUIRE(c.bank == TextConsole::BANK_CP437); // 'c' after the second toggle

		// A run that starts left of the grid keeps only the cells that land on it.
		console->clear();
		console->put_text(-2, 3, "WXYZ", TextConsole::COLOR_YELLOW, TextConsole::COLOR_BLACK);
		REQUIRE(cell_code(console, 0, 3) == int('Y'));
		REQUIRE(cell_code(console, 1, 3) == int('Z'));
		REQUIRE(cell_code(console, 2, 3) == 0);

		// A run that overruns the right edge is truncated, not wrapped onto the next row.
		console->clear();
		const int w = console->get_console_size().width;
		console->put_text(w - 2, 5, "12345", TextConsole::COLOR_YELLOW, TextConsole::COLOR_BLACK);
		REQUIRE(cell_code(console, w - 2, 5) == int('1'));
		REQUIRE(cell_code(console, w - 1, 5) == int('2'));
		REQUIRE(cell_code(console, 0, 6) == 0); // no wrap
	}

	SUBCASE("fill_rect paints only the requested box and marks the grid dirty") {
		console->clear();
		console->_dirty_screen = false;
		console->fill_rect(1, 1, 3, 2, 0xb0, TextConsole::COLOR_LIGHTGRAY, TextConsole::COLOR_BLACK);
		REQUIRE(console->_dirty_screen);
		REQUIRE(cell_code(console, 1, 1) == 0xb0);
		REQUIRE(cell_code(console, 3, 2) == 0xb0);
		REQUIRE(cell_code(console, 4, 2) == 0); // one past the right edge of the box
		REQUIRE(cell_code(console, 1, 3) == 0); // one past the bottom of the box
	}
}

TEST_CASE("Console LCD widget") {
	SUBCASE("size is three rows tall, spaces are two cells, digits three") {
		REQUIRE(console_lcd_size("") == Size2i(0, 0));
		REQUIRE(console_lcd_size("0") == Size2i(3, 3));
		REQUIRE(console_lcd_size("00") == Size2i(6, 3));
		REQUIRE(console_lcd_size(" ") == Size2i(2, 3));
		REQUIRE(console_lcd_size("0 0") == Size2i(3 + 2 + 3, 3));
		REQUIRE(console_lcd_size("12:34") == Size2i(3 * 5, 3));
	}

	SUBCASE("draw lays a known digit out cell by cell") {
		Ref<TextConsole> console = lcd_test_console();
		// '0' is a box: top ".-," / sides "| |" / bottom "`-'" in CP437 corners.
		console_lcd_draw(*console.ptr(), 0, 0, "0", TextConsole::COLOR_LIGHTGREEN, TextConsole::COLOR_BLACK);
		REQUIRE(cell_code(console, 0, 0) == 0xda); // top-left corner
		REQUIRE(cell_code(console, 1, 0) == 0xc4); // horizontal
		REQUIRE(cell_code(console, 2, 0) == 0xbf); // top-right corner
		REQUIRE(cell_code(console, 0, 1) == 0xb3); // left bar
		REQUIRE(cell_code(console, 1, 1) == 0xb3); // centre bar (a barred zero, unlike hollow 'O')
		REQUIRE(cell_code(console, 2, 1) == 0xb3); // right bar
		REQUIRE(cell_code(console, 0, 2) == 0xc0); // bottom-left corner
		REQUIRE(cell_code(console, 2, 2) == 0xd9); // bottom-right corner
		// Every drawn cell carries the lit/dim colour pair, blanks included.
		TextConsole::cell c;
		REQUIRE(console->get_cell(1, 1, c));
		REQUIRE(c.foreground == TextConsole::COLOR_LIGHTGREEN);
		REQUIRE(c.background == TextConsole::COLOR_BLACK);
	}

	SUBCASE("the second glyph is placed one advance to the right") {
		Ref<TextConsole> console = lcd_test_console();
		console_lcd_draw(*console.ptr(), 0, 0, "01", TextConsole::COLOR_WHITE, TextConsole::COLOR_BLACK);
		REQUIRE(cell_code(console, 3, 0) == 0xc4); // '1' top row "-,"
		REQUIRE(cell_code(console, 4, 0) == 0xbf);
	}

	SUBCASE("drawing clips at the grid edge instead of faulting") {
		Ref<TextConsole> console = lcd_test_console(4, 4);
		const bool was_muted = _print_error_enabled;
		_print_error_enabled = false;
		console_lcd_draw(*console.ptr(), 3, 3, "8", TextConsole::COLOR_WHITE, TextConsole::COLOR_BLACK);
		_print_error_enabled = was_muted;
		// Only the top-left cell of '8' lands inside a 4x4 grid at (3,3); the rest is clipped.
		REQUIRE(cell_code(console, 3, 3) == 0xda);
		REQUIRE(cell_code(console, 3, 0) == 0); // nothing wrapped up top
	}
}

#endif // DOCTEST
