/**************************************************************************/
/*  ddls_graph_edge.h                                                     */
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

#include "ddls_fwd.h"

#include "core/reference.h"
#include "core/variant.h"

class DDLS_GraphEdge : public Reference {
	unsigned id;

	DDLSGraphEdge prev;
	DDLSGraphEdge next;

	DDLSGraphEdge rot_prev_edge;
	DDLSGraphEdge rot_next_edge;
	DDLSGraphEdge opposite_edge;
	DDLSGraphNode source_node;
	DDLSGraphNode destination_node;

	Variant data;

public:
	unsigned get_id() const { return id; }

	DDLSGraphEdge get_prev() const;
	void set_prev(DDLSGraphEdge p_value);
	DDLSGraphEdge get_next() const;
	void set_next(DDLSGraphEdge p_value);
	DDLSGraphEdge get_rot_prev_edge() const;
	void set_rot_prev_edge(DDLSGraphEdge p_value);
	DDLSGraphEdge get_rot_next_edge() const;
	void set_rot_next_edge(DDLSGraphEdge p_value);
	DDLSGraphEdge get_opposite_edge() const;
	void set_opposite_edge(DDLSGraphEdge p_value);
	DDLSGraphNode get_source_node() const;
	void set_source_node(DDLSGraphNode p_value);
	DDLSGraphNode get_destination_node() const;
	void set_destination_node(DDLSGraphNode p_value);
	Variant get_data() const { return data; }
	void set_data(Variant p_value) { data = p_value; }

	DDLS_GraphEdge();
};
