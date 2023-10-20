/**************************************************************************/
/*  ddls_graph_node.h                                                     */
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

#include "core/map.h"
#include "core/reference.h"
#include "core/variant.h"

class DDLS_GraphNode : public Reference {
	unsigned id;

	DDLSGraphNode prev;
	DDLSGraphNode next;

	DDLSGraphEdge outgoing_edge;
	Map<DDLSGraphNode, DDLSGraphEdge> successor_nodes;

	Variant data;

public:
	unsigned get_id() const { return id; }

	DDLSGraphNode get_prev() const;
	void set_prev(DDLSGraphNode p_value);
	DDLSGraphNode get_next() const;
	void set_next(DDLSGraphNode p_value);
	DDLSGraphEdge get_outgoing_edge() const;
	void set_outgoing_edge(DDLSGraphEdge p_value);
	DDLSGraphEdge get_successor_node(DDLSGraphNode p_node) const { return successor_nodes[p_node]; }
	void set_successor_node(DDLSGraphNode p_node, DDLSGraphEdge p_edge) { successor_nodes[p_node] = p_edge; }
	Variant get_data() const { return data; }
	void set_data(Variant p_value) { data = p_value; }

	DDLS_GraphNode();
};
