/**************************************************************************/
/*  console_gauge.cpp                                                       */
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

#include "console_gauge.h"

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

namespace {

// Append one cell at fill level p_level (0..CONSOLE_GAUGE_STEPS) of the family starting at
// p_first, toggling the glyph bank only when the run actually changes bank. r_patch tracks
// the bank the string is currently in; the caller closes it out with _close_bank().
void _append_level(String &r_out, bool &r_patch, int p_level, uint8_t p_first) {
	uint8_t code;
	bool patch;
	if (p_level <= 0) {
		code = ' '; // an empty cell needs no glyph of its own
		patch = false;
	} else if (p_level >= CONSOLE_GAUGE_STEPS) {
		code = CONSOLE_GAUGE_FULL_BLOCK; // and neither does a full one
		patch = false;
	} else {
		code = p_first + p_level - 1;
		patch = true;
	}
	if (patch != r_patch) {
		r_out += String::chr(0x01);
		r_patch = patch;
	}
	r_out += String::chr(code);
}

// Leave the bank on BANK_CP437 so text appended after the gauge is not drawn from the patch
// bank (where a code with no patch renders inverted).
void _close_bank(String &r_out, bool &r_patch) {
	if (r_patch) {
		r_out += String::chr(0x01);
		r_patch = false;
	}
}

} // namespace

String console_meter(real_t p_fraction, int p_cells) {
	if (p_cells <= 0) {
		return String();
	}
	const real_t frac = CLAMP(p_fraction, real_t(0.0), real_t(1.0));
	// Work in eighths across the whole bar so rounding happens once, at the end of the fill,
	// rather than once per cell where it would accumulate.
	const int total = int(Math::round(frac * real_t(p_cells * CONSOLE_GAUGE_STEPS)));

	String out;
	bool patch = false;
	for (int i = 0; i < p_cells; ++i) {
		_append_level(out, patch, CLAMP(total - i * CONSOLE_GAUGE_STEPS, 0, CONSOLE_GAUGE_STEPS),
				CONSOLE_GAUGE_VBAR_FIRST);
	}
	_close_bank(out, patch);
	return out;
}

String console_graph(const Vector<real_t> &p_values, int p_cells, real_t p_min, real_t p_max) {
	if (p_cells <= 0) {
		return String();
	}

	const int count = MIN(p_values.size(), p_cells);
	const int first = p_values.size() - count; // plot the tail: the most recent samples

	real_t lo = p_min, hi = p_max;
	if (!(hi > lo)) {
		// No explicit range: take it from the samples actually being plotted, so the plot
		// uses the full cell height instead of hugging the floor.
		lo = hi = count > 0 ? p_values[first] : real_t(0.0);
		for (int i = first; i < p_values.size(); ++i) {
			lo = MIN(lo, p_values[i]);
			hi = MAX(hi, p_values[i]);
		}
	}
	const real_t span = hi - lo;

	String out;
	bool patch = false;
	for (int i = 0; i < p_cells - count; ++i) {
		_append_level(out, patch, 0, CONSOLE_GAUGE_SPARK_FIRST); // right-align a partial series
	}
	for (int i = first; i < p_values.size(); ++i) {
		int level = 1; // a flat series draws the baseline rather than a solid block
		if (span > CMP_EPSILON) {
			const real_t norm = CLAMP((p_values[i] - lo) / span, real_t(0.0), real_t(1.0));
			// Map onto 1..8, never 0: a sample at the floor is still a sample and should be
			// drawn, otherwise it is indistinguishable from the blank left padding.
			level = 1 + int(Math::round(norm * real_t(CONSOLE_GAUGE_STEPS - 1)));
		}
		_append_level(out, patch, level, CONSOLE_GAUGE_SPARK_FIRST);
	}
	_close_bank(out, patch);
	return out;
}

#ifdef DOCTEST

namespace {

// Decode a gauge string back into one fill level per cell, mirroring the bank tracking the
// console itself does, so the tests assert on what will be drawn rather than on the escapes.
Vector<int> _decode(const String &p_gauge, uint8_t p_first) {
	Vector<int> levels;
	bool patch = false;
	const CharString ascii = p_gauge.ascii();
	// length(), not size(): the latter counts the null terminator, which is not a cell.
	for (int i = 0; i < ascii.length(); ++i) {
		const uint8_t code = uint8_t(ascii[i]);
		if (code == 0x01) {
			patch = !patch;
			continue;
		}
		if (!patch) {
			levels.push_back(code == CONSOLE_GAUGE_FULL_BLOCK ? CONSOLE_GAUGE_STEPS : 0);
		} else {
			levels.push_back(int(code - p_first) + 1);
		}
	}
	return levels;
}

// Vector<int> has no operator==, and doctest needs one to compare decoded gauges.
bool _same(const Vector<int> &p_a, const Vector<int> &p_b) {
	if (p_a.size() != p_b.size()) {
		return false;
	}
	for (int i = 0; i < p_a.size(); ++i) {
		if (p_a[i] != p_b[i]) {
			return false;
		}
	}
	return true;
}

} // namespace

TEST_CASE("Console gauge") {
	SUBCASE("meter endpoints and width") {
		CHECK(console_meter(0.5, 0) == String()); // no cells, nothing to draw
		CHECK(console_meter(0.5, -3) == String());

		const Vector<int> empty = _decode(console_meter(0.0, 4), CONSOLE_GAUGE_VBAR_FIRST);
		REQUIRE(empty.size() == 4);
		for (int i = 0; i < empty.size(); ++i) {
			CHECK(empty[i] == 0);
		}

		const Vector<int> full = _decode(console_meter(1.0, 4), CONSOLE_GAUGE_VBAR_FIRST);
		REQUIRE(full.size() == 4);
		for (int i = 0; i < full.size(); ++i) {
			CHECK(full[i] == CONSOLE_GAUGE_STEPS);
		}

		// Out-of-range fractions clamp rather than run off the end of the bar.
		CHECK(_same(_decode(console_meter(-1.0, 4), CONSOLE_GAUGE_VBAR_FIRST), empty));
		CHECK(_same(_decode(console_meter(2.0, 4), CONSOLE_GAUGE_VBAR_FIRST), full));
	}

	SUBCASE("meter fills left to right") {
		const Vector<int> half = _decode(console_meter(0.5, 4), CONSOLE_GAUGE_VBAR_FIRST);
		REQUIRE(half.size() == 4);
		CHECK(half[0] == CONSOLE_GAUGE_STEPS);
		CHECK(half[1] == CONSOLE_GAUGE_STEPS);
		CHECK(half[2] == 0);
		CHECK(half[3] == 0);

		// A quarter of a two-cell bar is half of the first cell: the partial glyph is what
		// buys the sub-cell resolution the eighths exist for.
		const Vector<int> quarter = _decode(console_meter(0.25, 2), CONSOLE_GAUGE_VBAR_FIRST);
		REQUIRE(quarter.size() == 2);
		CHECK(quarter[0] == 4);
		CHECK(quarter[1] == 0);

		// Levels are monotonic in the fraction and never leave the valid range.
		int previous = -1;
		for (int i = 0; i <= 32; ++i) {
			const Vector<int> bar = _decode(console_meter(real_t(i) / 32.0, 4), CONSOLE_GAUGE_VBAR_FIRST);
			REQUIRE(bar.size() == 4);
			int sum = 0;
			for (int c = 0; c < bar.size(); ++c) {
				CHECK(bar[c] >= 0);
				CHECK(bar[c] <= CONSOLE_GAUGE_STEPS);
				sum += bar[c];
			}
			CHECK(sum >= previous);
			previous = sum;
		}
	}

	SUBCASE("graph right-aligns a partial series") {
		Vector<real_t> values;
		values.push_back(1.0);
		values.push_back(2.0);

		const Vector<int> plot = _decode(console_graph(values, 5), CONSOLE_GAUGE_SPARK_FIRST);
		REQUIRE(plot.size() == 5);
		CHECK(plot[0] == 0); // blank padding on the left
		CHECK(plot[1] == 0);
		CHECK(plot[2] == 0);
		CHECK(plot[3] == 1); // the low sample still draws a floor
		CHECK(plot[4] == CONSOLE_GAUGE_STEPS);
	}

	SUBCASE("graph plots the most recent samples") {
		Vector<real_t> values;
		for (int i = 0; i < 10; ++i) {
			values.push_back(real_t(i));
		}
		// Only the tail fits, and it must be the newest end of the series.
		const Vector<int> plot = _decode(console_graph(values, 3), CONSOLE_GAUGE_SPARK_FIRST);
		REQUIRE(plot.size() == 3);
		CHECK(plot[0] == 1);
		CHECK(plot[2] == CONSOLE_GAUGE_STEPS);
		CHECK(plot[0] <= plot[1]);
		CHECK(plot[1] <= plot[2]);
	}

	SUBCASE("graph of a flat series draws a baseline") {
		// Peak normalisation would light every column solid here, which reads as a full-scale
		// plot when the signal is in fact not moving at all.
		Vector<real_t> values;
		for (int i = 0; i < 4; ++i) {
			values.push_back(60.0);
		}
		const Vector<int> plot = _decode(console_graph(values, 4), CONSOLE_GAUGE_SPARK_FIRST);
		REQUIRE(plot.size() == 4);
		for (int i = 0; i < plot.size(); ++i) {
			CHECK(plot[i] == 1);
		}

		Vector<real_t> zeroes;
		zeroes.push_back(0.0);
		zeroes.push_back(0.0);
		const Vector<int> flat = _decode(console_graph(zeroes, 2), CONSOLE_GAUGE_SPARK_FIRST);
		REQUIRE(flat.size() == 2);
		CHECK(flat[0] == 1);
		CHECK(flat[1] == 1);
	}

	SUBCASE("graph honours an explicit range") {
		Vector<real_t> values;
		values.push_back(0.0);
		values.push_back(50.0);
		values.push_back(100.0);

		const Vector<int> plot = _decode(console_graph(values, 3, 0.0, 100.0), CONSOLE_GAUGE_SPARK_FIRST);
		REQUIRE(plot.size() == 3);
		CHECK(plot[0] == 1);
		CHECK(plot[2] == CONSOLE_GAUGE_STEPS);
		CHECK(plot[1] > plot[0]);
		CHECK(plot[1] < plot[2]);

		// Values outside the fixed range clamp instead of overflowing the glyph family.
		Vector<real_t> wild;
		wild.push_back(-1000.0);
		wild.push_back(1000.0);
		const Vector<int> clamped = _decode(console_graph(wild, 2, 0.0, 100.0), CONSOLE_GAUGE_SPARK_FIRST);
		REQUIRE(clamped.size() == 2);
		CHECK(clamped[0] == 1);
		CHECK(clamped[1] == CONSOLE_GAUGE_STEPS);
	}

	SUBCASE("empty series and degenerate widths") {
		const Vector<real_t> none;
		CHECK(console_graph(none, 0) == String());

		const Vector<int> blank = _decode(console_graph(none, 3), CONSOLE_GAUGE_SPARK_FIRST);
		REQUIRE(blank.size() == 3);
		for (int i = 0; i < blank.size(); ++i) {
			CHECK(blank[i] == 0);
		}
	}

	SUBCASE("gauges leave the bank on BANK_CP437") {
		// A gauge that ends on a partial glyph must close its bank toggle, or the text that
		// follows it would be drawn from the patch bank and come out inverted.
		Vector<real_t> values;
		values.push_back(0.0);
		values.push_back(1.0);
		values.push_back(0.5);

		const String cases[] = {
			console_meter(0.3, 4),
			console_meter(0.0, 4),
			console_meter(1.0, 4),
			console_graph(values, 4),
		};
		for (int i = 0; i < 4; ++i) {
			int toggles = 0;
			const CharString ascii = cases[i].ascii();
			for (int c = 0; c < ascii.length(); ++c) {
				if (uint8_t(ascii[c]) == 0x01) {
					++toggles;
				}
			}
			CHECK((toggles % 2) == 0);
		}
	}

	SUBCASE("gauges render through the console") {
		Ref<TextConsole> console = Ref<TextConsole>(memnew(TextConsole));
		console->load_font(TextConsole::DOS_8x16);
		console->resize(8, 2);
		console->clear();

		console->put_text(0, 0, console_meter(0.5, 4), TextConsole::COLOR_GREEN, TextConsole::COLOR_BLACK);

		// The escapes are consumed, not drawn: a full cell stays in CP437, and the cells past
		// the fill are blanked rather than left holding whatever was there before.
		TextConsole::cell c;
		REQUIRE(console->get_cell(0, 0, c));
		CHECK(uint8_t(c.character) == CONSOLE_GAUGE_FULL_BLOCK);
		CHECK(c.bank == TextConsole::BANK_CP437);
		REQUIRE(console->get_cell(2, 0, c));
		CHECK(uint8_t(c.character) == ' ');

		console->clear();
		console->put_text(0, 0, console_meter(0.125, 2), TextConsole::COLOR_GREEN, TextConsole::COLOR_BLACK);
		REQUIRE(console->get_cell(0, 0, c));
		CHECK(uint8_t(c.character) == CONSOLE_GAUGE_VBAR_FIRST + 1); // 2/8 of the first cell
		CHECK(c.bank == TextConsole::BANK_PATCH);
	}
}

#endif // DOCTEST
