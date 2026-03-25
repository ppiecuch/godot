/**************************************************************************/
/*  svg_path.cpp                                                          */
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

// Copyright (C) 2013  Nicholas Gill

#include "core/math/math_defs.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <stdexcept>

#include "svg_path/basic.h"
#include "svg_path/path.h"
#include "svg_path/transform.h"

#include "core/error_macros.h"
#include "core/print_string.h"

#define THROW_IF(cond, what) ERR_FAIL_COND_V_MSG(cond, false, "(SVG)" what)

namespace svg {
namespace types {
namespace parsers {

bool ws_p(const char c) {
	switch (c) {
		case '\t':
		case '\n':
		case '\r':
		case ' ':
			return true;
		default:
			return false;
	}
}

bool parse_whitespace(const char *&c, const char *const end) {
	if (!ws_p(*c)) {
		return false;
	}
	while (c != end && ws_p(*c)) {
		++c;
	}
	return true;
}

bool number_p(const char c) {
	switch (c) {
		case '+':
		case '-':
		case '.':
			return true;
		default:
			return std::isdigit(c);
	}
}

bool nonnegative_number_p(const char c) {
	switch (c) {
		case '.':
			return true;
		default:
			return std::isdigit(c);
	}
}

bool parse_number(const char *&c, const char *const end, real_t &x) {
	if (!number_p(*c)) {
		return false;
	}
	const auto begin = c;

	errno = 0;
	x = strtof(c, const_cast<char **>(&c));
	THROW_IF(c == begin || errno, "expected number");
	THROW_IF(c > end, "unexpected eof; strtof consumed too much");
	return true;
}

bool parse_nonnegative_number(const char *&c, const char *const end, real_t &x) {
	if (!nonnegative_number_p(*c)) {
		return false;
	}
	const auto begin = c;

	errno = 0;
	x = strtof(c, const_cast<char **>(&c));
	THROW_IF(c == begin || errno, "expected number");
	THROW_IF(c > end, "unexpected eof; strtof consumed too much");
	return true;
}

bool parse_comma_wsp(const char *&c, const char *const end) {
	if (!ws_p(*c) && *c != ',') {
		return false;
	}
	if (parse_whitespace(c, end) && c == end) {
		return true;
	}
	if (*c == ',' && ++c == end) {
		return true;
	}
	parse_whitespace(c, end);
	return true;
}

bool parse_flag(const char *&c, const char *const, bool &flag) {
	if (*c == '0') {
		++c;
		flag = false;
		return true;
	} else if (*c == '1') {
		++c;
		flag = true;
		return true;
	}
	return false;
}

} //namespace parsers
} //namespace types
} //namespace svg

namespace svg {
namespace types {
namespace parsers {
namespace path {

namespace {

struct point {
	real_t x;
	real_t y;
};

bool parse_coordinate_pair(const char *&c, const char *const end, point &p) {
	if (!parse_number(c, end, p.x)) {
		return false;
	}
	if (parse_comma_wsp(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	THROW_IF(!parse_number(c, end, p.y), "expected coordinate-pair");
	return true;
}

} //namespace

bool parser::parse_moveto(const char *&c, const char *const end) {
	if (*c != 'M' && *c != 'm') {
		return false;
	}
	const auto cmd = *c++;
	THROW_IF(c == end, "unexpected eof");

	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	point p;
	THROW_IF(!parse_coordinate_pair(c, end, p), "expected coordinate-pair");
	move_to(cmd == 'm', p.x, p.y);

	if (c == end) {
		return true;
	}

	/* This is not as strict as the formal grammer.
	 * This code will allow a comma to terminate the coordinate sequence
	 * where the lineto-argument-sequence production would require another
	 * coordinate. */
	parse_comma_wsp(c, end);
	while (c != end && parse_coordinate_pair(c, end, p)) {
		line_to(cmd == 'm', p.x, p.y);
		parse_comma_wsp(c, end);
	}

	return true;
}

bool parser::parse_lineto(const char *&c, const char *const end) {
	if (*c != 'L' && *c != 'l') {
		return false;
	}

	const auto cmd = *c++;
	THROW_IF(c == end, "unexpected eof");

	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	point p;
	THROW_IF(!parse_coordinate_pair(c, end, p), "expected coordinate-pair");
	line_to(cmd == 'l', p.x, p.y);

	if (c == end) {
		return true;
	}
	/* This is not as strict as the formal grammer.
	 * This code will allow a comma to terminate the coordinate sequence
	 * where the lineto-argument-sequence production would require another
	 * coordinate. */
	parse_comma_wsp(c, end);
	while (c != end && parse_coordinate_pair(c, end, p)) {
		line_to(cmd == 'l', p.x, p.y);
		parse_comma_wsp(c, end);
	}

	return true;
}

bool parser::parse_horizontal_lineto(const char *&c, const char *const end) {
	if (*c != 'H' && *c != 'h') {
		return false;
	}
	const auto cmd = *c++;
	THROW_IF(c == end, "unexpected eof");
	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	real_t x;
	THROW_IF(!parse_number(c, end, x), "expected coordinate");
	horizontal_line_to(cmd == 'h', x);

	if (c == end) {
		return true;
	}
	parse_comma_wsp(c, end);
	while (c != end && parse_number(c, end, x)) {
		horizontal_line_to(cmd == 'h', x);
		parse_comma_wsp(c, end);
	}

	return true;
}

bool parser::parse_vertical_lineto(const char *&c, const char *const end) {
	if (*c != 'V' && *c != 'v') {
		return false;
	}
	const auto cmd = *c++;
	THROW_IF(c == end, "unexpected eof");
	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	real_t y;
	THROW_IF(!parse_number(c, end, y), "expected coordinate");
	vertical_line_to(cmd == 'v', y);

	if (c == end) {
		return true;
	}
	parse_comma_wsp(c, end);
	while (c != end && parse_number(c, end, y)) {
		vertical_line_to(cmd == 'v', y);
		parse_comma_wsp(c, end);
	}

	return true;
}

bool parser::parse_curveto(const char *&c, const char *const end) {
	if (*c != 'C' && *c != 'c') {
		return false;
	}
	const auto cmd = *c++;
	THROW_IF(c == end, "unexpected eof");
	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	point p1, p2, p;
	THROW_IF(!parse_coordinate_pair(c, end, p1), "expected coordinate-pair p1");
	THROW_IF(c == end, "unexpected eof");
	if (parse_comma_wsp(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	THROW_IF(!parse_coordinate_pair(c, end, p2), "expected coordinate-pair p2");
	THROW_IF(c == end, "unexpected eof");
	if (parse_comma_wsp(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	THROW_IF(!parse_coordinate_pair(c, end, p), "expected coordinate-pair p");
	curve_to(cmd == 'c', p1.x, p1.y, p2.x, p2.y, p.x, p.y);
	if (c == end) {
		return true;
	}
	parse_comma_wsp(c, end);
	while (c != end && parse_coordinate_pair(c, end, p1)) {
		THROW_IF(c == end, "unexpected eof");
		if (parse_comma_wsp(c, end)) {
			THROW_IF(c == end, "unexpected eof");
		}
		THROW_IF(!parse_coordinate_pair(c, end, p2), "expected coordinate-pair p2");

		THROW_IF(c == end, "unexpected eof");
		if (parse_comma_wsp(c, end)) {
			THROW_IF(c == end, "unexpected eof");
		}
		THROW_IF(!parse_coordinate_pair(c, end, p), "expected coordinate-pair p");

		curve_to(cmd == 'c', p1.x, p1.y, p2.x, p2.y, p.x, p.y);
		parse_comma_wsp(c, end);
	}

	return true;
}

bool parser::parse_smooth_curveto(const char *&c, const char *const end) {
	if (*c != 'S' && *c != 's') {
		return false;
	}
	const auto cmd = *c++;
	THROW_IF(c == end, "unexpected eof");
	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	point p2;
	point p;
	THROW_IF(!parse_coordinate_pair(c, end, p2), "expected coordinate-pair p2");
	THROW_IF(c == end, "unexpected eof");
	if (parse_comma_wsp(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	THROW_IF(!parse_coordinate_pair(c, end, p), "expected coordinate-pair p");
	smooth_curve_to(cmd == 's', p2.x, p2.y, p.x, p.y);

	if (c == end) {
		return true;
	}
	parse_comma_wsp(c, end);
	while (c != end && parse_coordinate_pair(c, end, p2)) {
		THROW_IF(c == end, "unexpected eof");
		if (parse_comma_wsp(c, end)) {
			THROW_IF(c == end, "unexpected eof");
		}
		THROW_IF(!parse_coordinate_pair(c, end, p), "expected coordinate-pair p");

		smooth_curve_to(cmd == 's', p2.x, p2.y, p.x, p.y);
		parse_comma_wsp(c, end);
	}

	return true;
}

bool parser::parse_quadratic_bezier_curveto(const char *&c, const char *const end) {
	if (*c != 'Q' && *c != 'q') {
		return false;
	}
	const auto cmd = *c++;
	THROW_IF(c == end, "unexpected eof");
	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	point p1;
	point p;
	THROW_IF(!parse_coordinate_pair(c, end, p1), "expected coordinate-pair p1");
	THROW_IF(c == end, "unexpected eof");
	if (parse_comma_wsp(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	THROW_IF(!parse_coordinate_pair(c, end, p), "expected coordinate-pair p");
	bezier_curve_to(cmd == 'q', p1.x, p1.y, p.x, p.y);

	if (c == end) {
		return true;
	}
	parse_comma_wsp(c, end);
	while (c != end && parse_coordinate_pair(c, end, p1)) {
		THROW_IF(c == end, "unexpected eof");
		if (parse_comma_wsp(c, end)) {
			THROW_IF(c == end, "unexpected eof");
		}
		THROW_IF(!parse_coordinate_pair(c, end, p), "expected coordinate-pair p");

		bezier_curve_to(cmd == 'q', p1.x, p1.y, p.x, p.y);
		parse_comma_wsp(c, end);
	}

	return true;
}

bool parser::parse_smooth_quadratic_bezier_curveto(const char *&c, const char *const end) {
	if (*c != 'T' && *c != 't') {
		return false;
	}
	const auto cmd = *c++;
	THROW_IF(c == end, "unexpected eof");
	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	point p;
	THROW_IF(!parse_coordinate_pair(c, end, p), "expected coordinate-pair");
	smooth_bezier_curve_to(cmd == 't', p.x, p.y);

	if (c == end)
		return true;

	parse_comma_wsp(c, end);
	while (c != end && parse_coordinate_pair(c, end, p)) {
		smooth_bezier_curve_to(cmd == 't', p.x, p.y);
		parse_comma_wsp(c, end);
	}

	return true;
}

bool parser::parse_elliptical_arc(const char *&c, const char *const end) {
	if (*c != 'A' && *c != 'a') {
		return false;
	}
	const auto cmd = *c++;
	THROW_IF(c == end, "unexpected eof");
	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	real_t rx, ry, x_rotation;
	bool large_arc;
	bool sweep;
	point p;
	THROW_IF(!parse_nonnegative_number(c, end, rx), "expected x radius");
	THROW_IF(c == end, "unexpected eof");
	if (parse_comma_wsp(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	THROW_IF(!parse_nonnegative_number(c, end, ry), "expected y radius");
	THROW_IF(c == end, "unexpected eof");
	if (parse_comma_wsp(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	THROW_IF(!parse_number(c, end, x_rotation), "expected x axis rotation");
	THROW_IF(c == end, "unexpected eof");
	if (parse_comma_wsp(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	THROW_IF(!parse_flag(c, end, large_arc), "expected large arc flag");
	THROW_IF(c == end, "unexpected eof");
	if (parse_comma_wsp(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	THROW_IF(!parse_flag(c, end, sweep), "expected sweep flag");
	THROW_IF(c == end, "unexpected eof");

	if (parse_comma_wsp(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	THROW_IF(!parse_coordinate_pair(c, end, p), "expected coordinate-pair p");

	elliptical_arc_to(cmd == 'a', rx, ry, x_rotation, large_arc, sweep, p.x, p.y);

	if (c == end) {
		return true;
	}
	parse_comma_wsp(c, end);
	while (c != end && parse_nonnegative_number(c, end, rx)) {
		THROW_IF(c == end, "unexpected eof");
		if (parse_comma_wsp(c, end)) {
			THROW_IF(c == end, "unexpected eof");
		}
		THROW_IF(!parse_nonnegative_number(c, end, ry), "expected y radius");
		THROW_IF(c == end, "unexpected eof");
		if (parse_comma_wsp(c, end)) {
			THROW_IF(c == end, "unexpected eof");
		}
		THROW_IF(!parse_number(c, end, x_rotation), "expected x axis rotation");
		THROW_IF(c == end, "unexpected eof");
		if (parse_comma_wsp(c, end)) {
			THROW_IF(c == end, "unexpected eof");
		}
		THROW_IF(!parse_flag(c, end, large_arc), "expected large arc flag");
		THROW_IF(c == end, "unexpected eof");
		if (parse_comma_wsp(c, end)) {
			THROW_IF(c == end, "unexpected eof");
		}
		THROW_IF(!parse_flag(c, end, sweep), "expected sweep flag");
		THROW_IF(c == end, "unexpected eof");
		if (parse_comma_wsp(c, end)) {
			THROW_IF(c == end, "unexpected eof");
		}
		THROW_IF(!parse_coordinate_pair(c, end, p), "expected coordinate-pair p");

		elliptical_arc_to(cmd == 'a', rx, ry, x_rotation, large_arc, sweep, p.x, p.y);

		parse_comma_wsp(c, end);
	}

	return true;
}

bool parser::parse_closepath(const char *&c, const char *const) {
	if (*c != 'Z' && *c != 'z') {
		return false;
	}
	++c;
	close_path();
	return true;
}

bool parser::parse(const char *c, const char *const end) {
	while (c != end) {
		if (parse_whitespace(c, end) ||
				parse_moveto(c, end) ||
				parse_lineto(c, end) ||
				parse_horizontal_lineto(c, end) ||
				parse_vertical_lineto(c, end) ||
				parse_curveto(c, end) ||
				parse_smooth_curveto(c, end) ||
				parse_quadratic_bezier_curveto(c, end) ||
				parse_smooth_quadratic_bezier_curveto(c, end) ||
				parse_elliptical_arc(c, end) ||
				parse_closepath(c, end)) {
			continue;
		} else {
			ERR_PRINT("expected wsp / moveto / lineto / horizontal-lineto / vertical-lineto / curveto / smooth-curveto / quadratic-bezier-curveto / smooth-quadratic-bezier-curveto / elliptical-arc / closepath");
			return false;
		}
	}
	eof();
	return true;
}

bool parser::parse(const String &s) {
	const CharString _s = s.ascii();
	const char *c = _s.c_str();
	const char *end = c + _s.length();
	return parse(c, end);
}

} //namespace path
} //namespace parsers
} //namespace types
} //namespace svg

namespace svg {
namespace types {
namespace parsers {
namespace transform {

static const real_t DEG_TO_RAD = Math_PI / 180.0;

struct matrix {
	union {
		real_t cell[3][3];
		real_t data[9];
	};
	real_t *operator[](int i) { return cell[i]; }
	const real_t *operator[](int i) const { return cell[i]; }
	real_t &operator()(int x, int y) { return cell[x][y]; }
	real_t operator()(int x, int y) const { return cell[x][y]; }
	matrix() {}
	matrix(real_t v0, real_t v1, real_t v2) {
		cell[0][0] = v0;
		cell[1][1] = v1;
		cell[2][2] = v2;
	}
};
matrix identity_matrix{ 1, 1, 1 };

matrix prod(const matrix &m1, const matrix &m2) {
	matrix out;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			out[i][j] = 0;
			for (int k = 0; k < 3; k++) {
				out[i][j] += m1[i][k] * m2[k][j];
			}
		}
	}
	return out;
}

// matrix ::= "matrix" wsp* "(" wsp*
//  number comma-wsp
//  number comma-wsp
//  number comma-wsp
//  number comma-wsp
//  number comma-wsp
//  number wsp* ")"
bool parse_matrix(const char *&c, const char *const end, matrix &t) {
	char tag[] = { 'm', 'a', 't', 'r', 'i', 'x' };
	auto it = std::search(c, end, std::begin(tag), std::end(tag));
	if (it != c) {
		return false;
	}
	c += sizeof(tag);
	THROW_IF(c == end, "unexpected eof");

	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	THROW_IF(*c++ != '(', "expected '('");
	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	matrix m = identity_matrix;
	THROW_IF(!parse_number(c, end, m(0, 0)), "expected number");
	THROW_IF(c == end, "unexpected eof");

	THROW_IF(!parse_comma_wsp(c, end), "expected comma-wsp");
	THROW_IF(c == end, "unexpected eof");

	THROW_IF(!parse_number(c, end, m(1, 0)), "expected number");
	THROW_IF(c == end, "unexpected eof");

	THROW_IF(!parse_comma_wsp(c, end), "expected comma-wsp");
	THROW_IF(c == end, "unexpected eof");

	THROW_IF(!parse_number(c, end, m(0, 1)), "expected number");
	THROW_IF(c == end, "unexpected eof");

	THROW_IF(!parse_comma_wsp(c, end), "expected comma-wsp");
	THROW_IF(c == end, "unexpected eof");

	THROW_IF(!parse_number(c, end, m(1, 1)), "expected number");
	THROW_IF(c == end, "unexpected eof");

	THROW_IF(!parse_comma_wsp(c, end), "expected comma-wsp");
	THROW_IF(c == end, "unexpected eof");

	THROW_IF(!parse_number(c, end, m(0, 2)), "expected number");
	THROW_IF(c == end, "unexpected eof");

	THROW_IF(!parse_comma_wsp(c, end), "expected comma-wsp");
	THROW_IF(c == end, "unexpected eof");

	THROW_IF(!parse_number(c, end, m(1, 2)), "expected number");
	THROW_IF(c == end, "unexpected eof");

	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	THROW_IF(*c++ != ')', "expected ')'");

	t = prod(t, m);
	return true;
}

// translate ::= "translate" wsp* "(" wsp* number ( comma-wsp number )? wsp* ")"
bool parse_translate(const char *&c, const char *const end, matrix &t) {
	char tag[] = { 't', 'r', 'a', 'n', 's', 'l', 'a', 't', 'e' };
	auto it = std::search(c, end, std::begin(tag), std::end(tag));
	if (it != c) {
		return false;
	}
	c += sizeof(tag);
	THROW_IF(c == end, "unexpected eof");

	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	THROW_IF(*c++ != '(', "expected '('");
	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	matrix m = identity_matrix;
	THROW_IF(!parse_number(c, end, m(0, 2)), "expected number");
	THROW_IF(c == end, "unexpected eof");

	if (parse_comma_wsp(c, end)) {
		THROW_IF(c == end, "unexpected eof");
		if (parse_number(c, end, m(1, 2))) {
			THROW_IF(c == end, "unexpected eof");
		}
	}

	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	THROW_IF(*c++ != ')', "expected ')'");

	t = prod(t, m);
	return true;
}

// scale ::= "scale" wsp* "(" wsp* number ( comma-wsp number )? wsp* ")"
bool parse_scale(const char *&c, const char *const end, matrix &t) {
	char tag[] = { 's', 'c', 'a', 'l', 'e' };
	auto it = std::search(c, end, std::begin(tag), std::end(tag));
	if (it != c) {
		return false;
	}
	c += sizeof(tag);
	THROW_IF(c == end, "unexpected eof");

	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	THROW_IF(*c++ != '(', "expected '('");
	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	matrix m = identity_matrix;
	THROW_IF(!parse_number(c, end, m(0, 0)), "expected number");
	THROW_IF(c == end, "unexpected eof");

	m(1, 1) = m(0, 0);
	if (parse_comma_wsp(c, end)) {
		THROW_IF(c == end, "unexpected eof");
		if (parse_number(c, end, m(1, 1))) {
			THROW_IF(c == end, "unexpected eof");
		}
	}

	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	THROW_IF(*c++ != ')', "expected ')'");

	t = prod(t, m);
	return true;
}

// rotate ::= "rotate" wsp* "(" wsp* number ( comma-wsp number comma-wsp number )? wsp* ")"
bool parse_rotate(const char *&c, const char *const end, matrix &t) {
	char tag[] = { 'r', 'o', 't', 'a', 't', 'e' };
	auto it = std::search(c, end, std::begin(tag), std::end(tag));
	if (it != c) {
		return false;
	}
	c += sizeof(tag);
	THROW_IF(c == end, "unexpected eof");

	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	THROW_IF(*c++ != '(', "expected '('");
	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	real_t a;
	THROW_IF(!parse_number(c, end, a), "expected number");
	THROW_IF(c == end, "unexpected eof");

	real_t x = 0, y = 0;
	if (parse_comma_wsp(c, end)) {
		THROW_IF(c == end, "unexpected eof");

		if (parse_number(c, end, x)) {
			THROW_IF(c == end, "unexpected eof");

			THROW_IF(!parse_comma_wsp(c, end), "expected comma-wsp");
			THROW_IF(c == end, "unexpected eof");

			THROW_IF(!parse_number(c, end, y), "expected number");
			THROW_IF(c == end, "unexpected eof");
		}
	}

	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	THROW_IF(*c++ != ')', "expected ')'");

	matrix m = identity_matrix;
	m(0, 0) = std::cos(a * DEG_TO_RAD);
	m(0, 1) = -std::sin(a * DEG_TO_RAD);
	m(1, 0) = std::sin(a * DEG_TO_RAD);
	m(1, 1) = std::cos(a * DEG_TO_RAD);

	// translate(<cx>, <cy>) rotate(<rotate-angle>) translate(-<cx>, -<cy>)
	matrix tr = identity_matrix;
	tr(0, 2) = x;
	tr(1, 2) = y;
	t = prod(t, tr);

	t = prod(t, m);

	tr(0, 2) = -x;
	tr(1, 2) = -y;

	t = prod(t, tr);
	return true;
}

// skewX ::= "skewX" wsp* "(" wsp* number wsp* ")"
bool parse_skewX(const char *&c, const char *const end, matrix &t) {
	char tag[] = { 's', 'k', 'e', 'w', 'X' };
	auto it = std::search(c, end, std::begin(tag), std::end(tag));
	if (it != c) {
		return false;
	}
	c += sizeof(tag);
	THROW_IF(c == end, "unexpected eof");

	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	THROW_IF(*c++ != '(', "expected '('");
	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	matrix m = identity_matrix;
	THROW_IF(!parse_number(c, end, m(0, 1)), "expected number");
	THROW_IF(c == end, "unexpected eof");

	m(0, 1) = std::tan(m(0, 1) * DEG_TO_RAD);

	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	THROW_IF(*c++ != ')', "expected ')'");

	t = prod(t, m);
	return true;
}

// skewY ::= "skewY" wsp* "(" wsp* number wsp* ")"
bool parse_skewY(const char *&c, const char *const end, matrix &t) {
	char tag[] = { 's', 'k', 'e', 'w', 'Y' };
	auto it = std::search(c, end, std::begin(tag), std::end(tag));
	if (it != c) {
		return false;
	}
	c += sizeof(tag);
	THROW_IF(c == end, "unexpected eof");

	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	THROW_IF(*c++ != '(', "expected '('");
	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	matrix m = identity_matrix;
	THROW_IF(!parse_number(c, end, m(1, 0)), "expected number");
	THROW_IF(c == end, "unexpected eof");

	m(1, 0) = std::tan(m(1, 0) * DEG_TO_RAD);

	if (parse_whitespace(c, end)) {
		THROW_IF(c == end, "unexpected eof");
	}
	THROW_IF(*c++ != ')', "expected ')'");

	t = prod(t, m);
	return true;
}

/*
transform ::=
	matrix
	| translate
	| scale
	| rotate
	| skewX
	| skewY */
bool parse_transform(const char *&c, const char *const end, matrix &t) {
	if (parse_matrix(c, end, t) ||
			parse_translate(c, end, t) ||
			parse_scale(c, end, t) ||
			parse_rotate(c, end, t) ||
			parse_skewX(c, end, t) ||
			parse_skewY(c, end, t)) {
		if (c != end) {
			parse_comma_wsp(c, end);
		}
		return true;
	}
	return false;
}

/*
transform-list ::= wsp* transforms? wsp*
transforms ::=
	transform
	| transform comma-wsp+ transforms
*/
std::array<real_t, 6> parse_transforms(const char *c, const char *const end) {
	matrix t = identity_matrix;

	while (c != end) {
		if (parse_whitespace(c, end) || parse_transform(c, end, t)) {
			continue;
		} else {
			ERR_PRINT("expected wsp / matrix / translate / scale / rotate / skewX / skewY");
		}
	}

	return { { t(0, 0), t(1, 0), t(0, 1), t(1, 1), t(0, 2), t(1, 2) } };
}

} //namespace transform
} //namespace parsers
} //namespace types
} //namespace svg

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"
#include "doctest/doctest_godot.h"

using namespace svg::types::parsers;
using namespace svg::types::parsers::path;

// Test helper: counts path commands for verification
struct TestRecorder : public parser {
	int move_count = 0, line_count = 0, curve_count = 0, close_count = 0, arc_count = 0;
	real_t last_x = 0, last_y = 0;
	bool eof_called = false;

	void move_to(bool, real_t x, real_t y) override {
		last_x = x;
		last_y = y;
		move_count++;
	}
	void line_to(bool, real_t x, real_t y) override {
		last_x = x;
		last_y = y;
		line_count++;
	}
	void horizontal_line_to(bool, real_t x) override {
		last_x = x;
		line_count++;
	}
	void vertical_line_to(bool, real_t y) override {
		last_y = y;
		line_count++;
	}
	void curve_to(bool, real_t, real_t, real_t, real_t, real_t x, real_t y) override {
		last_x = x;
		last_y = y;
		curve_count++;
	}
	void smooth_curve_to(bool, real_t, real_t, real_t x, real_t y) override {
		last_x = x;
		last_y = y;
		curve_count++;
	}
	void bezier_curve_to(bool, real_t, real_t, real_t x, real_t y) override {
		last_x = x;
		last_y = y;
		curve_count++;
	}
	void smooth_bezier_curve_to(bool, real_t x, real_t y) override {
		last_x = x;
		last_y = y;
		curve_count++;
	}
	void elliptical_arc_to(bool, real_t, real_t, real_t, bool, bool, real_t x, real_t y) override {
		last_x = x;
		last_y = y;
		arc_count++;
	}
	void close_path() override { close_count++; }
	void eof() override { eof_called = true; }
	int total() const { return move_count + line_count + curve_count + close_count + arc_count; }
};

TEST_SUITE("[[cyberelements]] svg_path: basic parsers") {
	TEST_CASE("ws_p identifies whitespace") {
		CHECK(ws_p(' '));
		CHECK(ws_p('\t'));
		CHECK(ws_p('\n'));
		CHECK(ws_p('\r'));
		CHECK_FALSE(ws_p('a'));
		CHECK_FALSE(ws_p('0'));
		CHECK_FALSE(ws_p(','));
	}

	TEST_CASE("number_p identifies number starts") {
		CHECK(number_p('0'));
		CHECK(number_p('9'));
		CHECK(number_p('+'));
		CHECK(number_p('-'));
		CHECK(number_p('.'));
		CHECK_FALSE(number_p('a'));
		CHECK_FALSE(number_p(' '));
		CHECK_FALSE(number_p(','));
	}

	TEST_CASE("parse_number: integers") {
		const char *s = "42";
		const char *end = s + 2;
		real_t val = 0;
		CHECK(parse_number(s, end, val));
		CHECK(val == doctest::Approx(42.0f));
		CHECK(s == end);
	}

	TEST_CASE("parse_number: floats") {
		const char *s = "3.14";
		const char *end = s + 4;
		real_t val = 0;
		CHECK(parse_number(s, end, val));
		CHECK(val == doctest::Approx(3.14f));
	}

	TEST_CASE("parse_number: negative") {
		const char *s = "-7.5";
		const char *end = s + 4;
		real_t val = 0;
		CHECK(parse_number(s, end, val));
		CHECK(val == doctest::Approx(-7.5f));
	}

	TEST_CASE("parse_number: no number returns false") {
		const char *s = "abc";
		const char *end = s + 3;
		real_t val = 0;
		CHECK_FALSE(parse_number(s, end, val));
	}

	TEST_CASE("parse_flag: 0 and 1") {
		const char *s0 = "0";
		const char *end0 = s0 + 1;
		bool flag = true;
		CHECK(parse_flag(s0, end0, flag));
		CHECK_FALSE(flag);

		const char *s1 = "1";
		const char *end1 = s1 + 1;
		CHECK(parse_flag(s1, end1, flag));
		CHECK(flag);
	}

	TEST_CASE("parse_flag: non-flag returns false") {
		const char *s = "2";
		const char *end = s + 1;
		bool flag = false;
		CHECK_FALSE(parse_flag(s, end, flag));
	}

	TEST_CASE("parse_comma_wsp: comma") {
		const char *s = ", ";
		const char *end = s + 2;
		CHECK(parse_comma_wsp(s, end));
	}

	TEST_CASE("parse_comma_wsp: space") {
		const char *s = "  ,";
		const char *end = s + 3;
		CHECK(parse_comma_wsp(s, end));
	}

	TEST_CASE("parse_comma_wsp: no separator") {
		const char *s = "M";
		const char *end = s + 1;
		CHECK_FALSE(parse_comma_wsp(s, end));
	}
}

TEST_SUITE("[[cyberelements]] svg_path: path commands") {
	TEST_CASE("MoveTo absolute") {
		TestRecorder r;
		CHECK(r.parse("M 10 20"));
		CHECK(r.move_count == 1);
		CHECK(r.last_x == doctest::Approx(10.0f));
		CHECK(r.last_y == doctest::Approx(20.0f));
	}

	TEST_CASE("MoveTo relative") {
		TestRecorder r;
		CHECK(r.parse("m 5 7"));
		CHECK(r.move_count == 1);
		CHECK(r.last_x == doctest::Approx(5.0f));
		CHECK(r.last_y == doctest::Approx(7.0f));
	}

	TEST_CASE("MoveTo followed by implicit LineTo") {
		TestRecorder r;
		CHECK(r.parse("M 0 0 10 20 30 40"));
		CHECK(r.move_count == 1);
		CHECK(r.line_count == 2);
	}

	TEST_CASE("LineTo absolute and relative") {
		TestRecorder r;
		CHECK(r.parse("M 0 0 L 10 20 l 5 5"));
		CHECK(r.line_count == 2);
	}

	TEST_CASE("HorizontalLineTo") {
		TestRecorder r;
		CHECK(r.parse("M 0 0 H 50"));
		CHECK(r.line_count == 1);
		CHECK(r.last_x == doctest::Approx(50.0f));
	}

	TEST_CASE("VerticalLineTo") {
		TestRecorder r;
		CHECK(r.parse("M 0 0 V 30"));
		CHECK(r.line_count == 1);
		CHECK(r.last_y == doctest::Approx(30.0f));
	}

	TEST_CASE("CurveTo (cubic bezier)") {
		TestRecorder r;
		CHECK(r.parse("M 0 0 C 10,20 30,40 50,60"));
		CHECK(r.curve_count == 1);
	}

	TEST_CASE("SmoothCurveTo") {
		TestRecorder r;
		CHECK(r.parse("M 0 0 S 30,40 50,60"));
		CHECK(r.curve_count == 1);
	}

	TEST_CASE("QuadraticBezier") {
		TestRecorder r;
		CHECK(r.parse("M 0 0 Q 10,20 30,40"));
		CHECK(r.curve_count == 1);
	}

	TEST_CASE("SmoothQuadraticBezier") {
		TestRecorder r;
		CHECK(r.parse("M 0 0 T 30,40"));
		CHECK(r.curve_count == 1);
	}

	TEST_CASE("EllipticalArc") {
		TestRecorder r;
		CHECK(r.parse("M 0 0 A 25 25 0 0 1 50 50"));
		CHECK(r.arc_count == 1);
	}

	TEST_CASE("ClosePath") {
		TestRecorder r;
		CHECK(r.parse("M 0 0 L 10 0 L 10 10 Z"));
		CHECK(r.close_count == 1);
	}

	TEST_CASE("ClosePath lowercase") {
		TestRecorder r;
		CHECK(r.parse("M 0 0 L 10 0 z"));
		CHECK(r.close_count == 1);
	}

	TEST_CASE("Complex path with multiple command types") {
		TestRecorder r;
		CHECK(r.parse("M 10,80 C 40,10 65,10 95,80 S 150,150 180,80"));
		CHECK(r.move_count == 1);
		CHECK(r.curve_count == 2);
	}

	TEST_CASE("Multiple sub-paths") {
		TestRecorder r;
		CHECK(r.parse("M 0 0 L 10 10 Z M 20 20 L 30 30 Z"));
		CHECK(r.move_count == 2);
		CHECK(r.line_count == 2);
		CHECK(r.close_count == 2);
	}

	TEST_CASE("Compact notation (no spaces between numbers)") {
		TestRecorder r;
		CHECK(r.parse("M0,0L10,20"));
		CHECK(r.move_count == 1);
		CHECK(r.line_count == 1);
	}

	TEST_CASE("Negative numbers as separators") {
		TestRecorder r;
		CHECK(r.parse("M10-20L30-40"));
		CHECK(r.move_count == 1);
		CHECK(r.line_count == 1);
	}

	TEST_CASE("Empty string returns true") {
		TestRecorder r;
		CHECK(r.parse(""));
	}

	TEST_CASE("Whitespace only returns true") {
		TestRecorder r;
		CHECK(r.parse("   \t\n  "));
	}

	TEST_CASE("EOF is called") {
		TestRecorder r;
		r.parse("M 0 0");
		CHECK(r.eof_called);
	}

	TEST_CASE("Invalid command returns false") {
		TestRecorder r;
		EXPECT_ERROR(CHECK_FALSE(r.parse("X 10 20"))); // expected: expected wsp / moveto...
	}

	TEST_CASE("Real CyberElement path parses") {
		// Simplified path from CyberEl1
		TestRecorder r;
		CHECK(r.parse("M742 863c-55 0-83-36-83-36l-21 46s37 43 104 43c67 0 104-43 104-43l-21-46s-28 36-83 36z"));
		CHECK(r.move_count >= 1);
		CHECK(r.close_count >= 1);
		int total = r.move_count + r.line_count + r.curve_count + r.close_count;
		CHECK(total > 5);
	}

	TEST_CASE("Multiple repeated curves") {
		TestRecorder r;
		CHECK(r.parse("M 0 0 C 1,2 3,4 5,6 7,8 9,10 11,12"));
		CHECK(r.curve_count == 2);
	}

	TEST_CASE("Arc with flags without separator") {
		// Per SVG spec, flags are single digits and don't need separators
		TestRecorder r;
		CHECK(r.parse("M 0 0 A 10 10 0 01 50 50"));
		CHECK(r.arc_count == 1);
	}
}

TEST_SUITE("[[cyberelements]] svg_path: transform parser") {
	using namespace svg::types::parsers::transform;

	TEST_CASE("parse_transforms: translate") {
		const char *s = "translate(10, 20)";
		auto result = parse_transforms(s, s + strlen(s));
		// result = [a, b, c, d, e, f] = [1, 0, 0, 1, 10, 20]
		CHECK(result[0] == doctest::Approx(1.0f));
		CHECK(result[1] == doctest::Approx(0.0f));
		CHECK(result[2] == doctest::Approx(0.0f));
		CHECK(result[3] == doctest::Approx(1.0f));
		CHECK(result[4] == doctest::Approx(10.0f));
		CHECK(result[5] == doctest::Approx(20.0f));
	}

	TEST_CASE("parse_transforms: scale uniform") {
		const char *s = "scale(2)";
		auto result = parse_transforms(s, s + strlen(s));
		CHECK(result[0] == doctest::Approx(2.0f));
		CHECK(result[3] == doctest::Approx(2.0f));
		CHECK(result[4] == doctest::Approx(0.0f));
		CHECK(result[5] == doctest::Approx(0.0f));
	}

	TEST_CASE("parse_transforms: scale non-uniform") {
		const char *s = "scale(3, 5)";
		auto result = parse_transforms(s, s + strlen(s));
		CHECK(result[0] == doctest::Approx(3.0f));
		CHECK(result[3] == doctest::Approx(5.0f));
	}

	TEST_CASE("parse_transforms: rotate 90") {
		const char *s = "rotate(90)";
		auto result = parse_transforms(s, s + strlen(s));
		CHECK(result[0] == doctest::Approx(0.0f).epsilon(0.001));
		CHECK(result[1] == doctest::Approx(1.0f).epsilon(0.001));
		CHECK(result[2] == doctest::Approx(-1.0f).epsilon(0.001));
		CHECK(result[3] == doctest::Approx(0.0f).epsilon(0.001));
	}

	TEST_CASE("parse_transforms: identity matrix") {
		const char *s = "matrix(1 0 0 1 0 0)";
		auto result = parse_transforms(s, s + strlen(s));
		CHECK(result[0] == doctest::Approx(1.0f));
		CHECK(result[1] == doctest::Approx(0.0f));
		CHECK(result[2] == doctest::Approx(0.0f));
		CHECK(result[3] == doctest::Approx(1.0f));
		CHECK(result[4] == doctest::Approx(0.0f));
		CHECK(result[5] == doctest::Approx(0.0f));
	}

	TEST_CASE("parse_transforms: chained transforms") {
		const char *s = "translate(10, 0) scale(2)";
		auto result = parse_transforms(s, s + strlen(s));
		// translate(10,0) then scale(2): M = translate * scale
		// point (x,y) -> scale -> (2x,2y) -> translate -> (2x+10, 2y)
		CHECK(result[0] == doctest::Approx(2.0f));
		CHECK(result[3] == doctest::Approx(2.0f));
		CHECK(result[4] == doctest::Approx(10.0f));
		CHECK(result[5] == doctest::Approx(0.0f));
	}

	TEST_CASE("parse_transforms: empty string") {
		const char *s = "";
		auto result = parse_transforms(s, s);
		// Should return identity
		CHECK(result[0] == doctest::Approx(1.0f));
		CHECK(result[3] == doctest::Approx(1.0f));
	}
}

#endif // DOCTEST
