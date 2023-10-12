/**************************************************************************/
/*  ddls_edge.h                                                           */
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
#include "core/reference.h"
#include "core/ustring.h"
#include "core/vector.h"

#include "ddls_fwd.h"

class DDLS_Edge : public Reference {
	unsigned id;

	// root datas
	bool is_real;
	bool is_constrained;
	DDLSVertex origin_vertex;
	DDLSEdge opposite_edge;
	DDLSEdge next_left_edge;
	DDLSFace left_face;

	Vector<DDLSConstraintSegment> from_constraint_segments;

	Color color_debug;

public:
	unsigned get_id() const { return id; }
	bool if_is_real() const { return is_real; }
	bool if_is_constrained() const { return is_constrained; }

	void set_datas(DDLSVertex p_origin_vertex, DDLSEdge p_opposite_edge, DDLSEdge p_next_left_edge, DDLSFace p_left_face, bool p_is_real = true, bool p_is_constrained = false);

	void add_from_constraint_segment(DDLSConstraintSegment p_segment);
	void remove_from_constraint_segment(DDLSConstraintSegment p_segment);

	void set_origin_vertex(DDLSVertex p_vertex);
	void set_next_left_edge(DDLSEdge p_edge);
	void set_left_face(DDLSFace p_face);
	void set_is_constrained(bool p_value);

	Vector<DDLSConstraintSegment> get_from_constraint_segments() const;

	void set_from_constraint_segments(Vector<DDLSConstraintSegment> p_value);

	DDLSVertex get_origin_vertex() const;
	DDLSVertex get_destination_vertex() const;
	DDLSEdge get_opposite_edge() const;
	DDLSEdge get_next_left_edge() const;
	DDLSEdge get_prev_left_edge() const;
	DDLSEdge get_next_right_edge() const;
	DDLSEdge get_prev_right_edge() const;
	DDLSEdge get_rot_left_edge() const;
	DDLSEdge get_rot_right_edge() const;
	DDLSFace get_left_face() const;
	DDLSFace get_right_face() const;

	String string() const;
	operator String() const { return string(); }

	DDLS_Edge();
};
