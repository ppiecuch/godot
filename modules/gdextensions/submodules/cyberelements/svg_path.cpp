/*************************************************************************/
/*  svg_path.cpp                                                         */
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

// Copyright (C) 2013  Nicholas Gill

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

bool parse_number(const char *&c, const char *const end, float &x) {
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

bool parse_nonnegative_number(const char *&c, const char *const end, float &x) {
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
	float x;
	float y;
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
	float x;
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
	float y;
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
	float rx, ry;
	float x_rotation;
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

parser::~parser() {
}

} //namespace path
} //namespace parsers
} //namespace types
} //namespace svg

namespace svg {
namespace types {
namespace parsers {
namespace transform {

static const float DEG_TO_RAD = 0.0174532925;

struct matrix {
	union {
		float cell[3][3];
		float data[9];
	};
	float *operator[](int i) { return cell[i]; }
	const float *operator[](int i) const { return cell[i]; }
	float &operator()(int x, int y) { return cell[x][y]; }
	float operator()(int x, int y) const { return cell[x][y]; }
	matrix() {}
	matrix(float v0, float v1, float v2) {
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

/*
matrix ::=
	"matrix" wsp* "(" wsp*
	number comma-wsp
	number comma-wsp
	number comma-wsp
	number comma-wsp
	number comma-wsp
	number wsp* ")"
*/
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
	float a;
	THROW_IF(!parse_number(c, end, a), "expected number");
	THROW_IF(c == end, "unexpected eof");

	float x = 0;
	float y = 0;
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
std::array<float, 6> parse_transforms(const char *c, const char *const end) {
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
