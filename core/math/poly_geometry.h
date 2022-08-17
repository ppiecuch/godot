/*************************************************************************/
/*  poly_geometry.h                                                      */
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

#ifndef POLY_GEOMETRY_H
#define POLY_GEOMETRY_H

#include "core/variant.h"

class PolyGeometry {
public:
	enum LineDrawMode {

		LINE_JOIN_MITTER,
		LINE_JOIN_BEVEL,
		LINE_JOIN_ROUND,
		LINE_CAP_SQUARE,
		LINE_CAP_PROJECT,
		LINE_CAP_ROUND,
	};

	/// build polygon stroke for line width 'p_width'
	static Vector<Point2> strokify(const Vector<Point2> &p_contour, real_t p_width, LineDrawMode p_line_cap, LineDrawMode p_line_join, bool p_loop, bool p_antialiased = false, bool p_allow_overlap = false);

	/// extend contour with line joints only
	static Vector<Point2> strokify(const Vector<Point2> &p_contour, LineDrawMode p_line_join, bool p_allow_overlap = false);
};

#endif // POLY_GEOMETRY_H
