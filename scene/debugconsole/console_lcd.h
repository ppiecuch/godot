/**************************************************************************/
/*  console_lcd.h                                                         */
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

#ifndef CONSOLE_LCD_H
#define CONSOLE_LCD_H

#include "debug_console.h"

// LCD "big font" readout, ported from the LCDBOX widget of the text_ui toolkit
// (fblauncher/modules/text_ui/textUI.c, LcdBoxProc). Each character is three grid rows tall
// and three cells wide (a space is two), drawn out of the CP437 box-drawing glyphs the
// original composed it from -- all of which live in BANK_CP437 and connect at cell edges, so
// the letters render as continuous line art with no runtime glyph generation. See
// CONSOLE.md, "Direct cell writes and the LCD widget".

// Cells the readout occupies: Size2i(width, 3) for non-empty p_text, Size2i(0, 0) otherwise.
Size2i console_lcd_size(const String &p_text);

// Draw p_text as an LCD readout with its top-left cell at (p_x, p_y). p_lit colours the
// segment glyphs, p_dim the panel behind them (it fills the whole bounding box, including the
// blank cells, so the readout reads as a lit-on-panel display). Clipped silently to the grid.
void console_lcd_draw(TextConsole &p_console, int p_x, int p_y, const String &p_text,
		TextConsole::ColorIndex p_lit, TextConsole::ColorIndex p_dim);

#endif // CONSOLE_LCD_H
