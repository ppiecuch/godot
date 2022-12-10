/*************************************************************************/
/*  path.h                                                               */
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

#ifndef PARSERS_PATH_H_
#define PARSERS_PATH_H_

#include "core/ustring.h"

namespace svg {
namespace types {
namespace parsers {
namespace path {

// TODO use template - the functions below don't need dynamic binding.
class parser {
	bool parse_moveto(const char *&c, const char *const end);
	bool parse_lineto(const char *&c, const char *const end);
	bool parse_horizontal_lineto(const char *&c, const char *const end);
	bool parse_vertical_lineto(const char *&c, const char *const end);
	bool parse_curveto(const char *&c, const char *const end);
	bool parse_smooth_curveto(const char *&c, const char *const end);
	bool parse_quadratic_bezier_curveto(const char *&c, const char *const end);
	bool parse_smooth_quadratic_bezier_curveto(const char *&c, const char *const end);
	bool parse_elliptical_arc(const char *&c, const char *const end);
	bool parse_closepath(const char *&c, const char *const end);

public:
	virtual void move_to(bool rel, float x, float y) = 0;

	virtual void line_to(bool rel, float x, float y) = 0;
	virtual void horizontal_line_to(bool rel, float x) = 0;
	virtual void vertical_line_to(bool rel, float y) = 0;

	virtual void curve_to(bool rel, float x1, float y1, float x2, float y2, float x, float y) = 0;
	virtual void smooth_curve_to(bool rel, float x2, float y2, float x, float y) = 0;

	virtual void bezier_curve_to(bool rel, float x1, float y1, float x, float y) = 0;
	virtual void smooth_bezier_curve_to(bool rel, float x, float y) = 0;
	virtual void elliptical_arc_to(bool rel, float rx, float ry, float x_rotation, bool large_arc, bool sweep, float x, float y) = 0;

	virtual void close_path() = 0;
	virtual void eof() = 0;

	bool parse(const char *c, const char *const end);
	bool parse(const String &s);

	virtual ~parser();
};

} //namespace path
} //namespace parsers
} //namespace types
} //namespace svg

#endif /* PARSERS_PATH_H_ */
