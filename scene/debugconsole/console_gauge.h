/**************************************************************************/
/*  console_gauge.h                                                         */
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

#ifndef CONSOLE_GAUGE_H
#define CONSOLE_GAUGE_H

#include "debug_console.h"

// Bar and graph strings built from the fill glyphs at 0x80-0x8D of BANK_PATCH.
//
// The glyphs are ordinary hand-drawn font patches, not generated at runtime: they are
// produced offline by modules/gdextensions/_tools/make_sparkline.py and make_vbars.py and
// merged into the source atlases in fonts.altered/, so every font size carries the same
// codes with the same meaning. Each family is seven evenly spaced fill levels of an eighth
// each; the two extremes need no patch of their own, since an empty cell is a space and a
// full one is the CP437 full block. That gives nine states per cell:
//
//   0x80..0x86  bottom-anchored, 1/8..7/8 of the cell height -- graph columns
//   0x87..0x8D  left-anchored, 1/8..7/8 of the cell width    -- meter fills
//
// Both helpers return strings in the console's markup-free escape form (\x01 toggles the
// glyph bank), so they can be handed to log(), _write() or put_text() unchanged, and always
// leave the bank back on BANK_CP437 so whatever follows is unaffected. See CONSOLE.md,
// "Bars and graphs".

enum {
	CONSOLE_GAUGE_STEPS = 8, // a cell is quantised into eighths: 0 empty, 8 full
	CONSOLE_GAUGE_SPARK_FIRST = 0x80, // 0x80..0x86, bottom-anchored eighths
	CONSOLE_GAUGE_VBAR_FIRST = 0x87, // 0x87..0x8D, left-anchored eighths
	CONSOLE_GAUGE_FULL_BLOCK = 0xDB, // CP437 full block, the 8/8 case of both families
};

// Horizontal meter p_cells wide filled to p_fraction (clamped to 0..1), left to right, at
// eighth-of-a-cell resolution. Returns an empty string when p_cells is not positive.
String console_meter(real_t p_fraction, int p_cells);

// Column graph of the *last* p_cells values, one cell per sample, most recent on the right;
// a shorter series is left-padded with blanks so the plot stays right-aligned as it fills.
// Values are normalised onto 1/8..8/8 of the cell height, so every sample stays visible --
// the lowest one draws a floor rather than a gap. Pass p_max > p_min to fix the range,
// otherwise it is taken from the data; a flat series draws the baseline.
String console_graph(const Vector<real_t> &p_values, int p_cells, real_t p_min = 0.0, real_t p_max = 0.0);

#endif // CONSOLE_GAUGE_H
