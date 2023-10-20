/**************************************************************************/
/*  ddls_vertex.h                                                         */
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

#pragma once

#include "core/color.h"
#include "core/math/vector2.h"
#include "core/reference.h"
#include "core/vector.h"

#include "ddls_fwd.h"

class DDLS_Vertex : public Reference {
	unsigned id;

	Point2 pos;

	bool is_real = true;
	DDLSEdge edge;

	Vector<DDLSConstraintSegment> from_constraint_segments;

	Color color_debug;

public:
	unsigned get_id() const { return id; }
	bool if_is_real() const { return is_real; }

	Point2 get_pos() const { return pos; }
	void set_pos(const Point2 &p_pos) { pos = p_pos; }
	void set_pos(real_t p_x, real_t p_y) { pos = { p_x, p_y }; }

	Vector<DDLSConstraintSegment> get_from_constraint_segments() const;
	void set_from_constraint_segments(Vector<DDLSConstraintSegment> p_value);
#define _Arg(N) DDLSConstraintSegment p_segment##N = DDLSConstraintSegment()
	void add_from_constraint_segment(DDLSConstraintSegment p_segment1, _Arg(2), _Arg(3), _Arg(4));
#undef _Arg
	void remove_from_constraint_segment(DDLSConstraintSegment p_segment);

	DDLSEdge get_edge() const;
	void set_edge(DDLSEdge p_edge);

	void set_datas(DDLSEdge p_edge, bool p_is_real = true);

	String string() const { return "ver_id " + itos(id); }
	operator String() const { return string(); }

	DDLS_Vertex();
};
