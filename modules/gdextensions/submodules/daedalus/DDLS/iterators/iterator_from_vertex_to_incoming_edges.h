/**************************************************************************/
/*  iterator_from_vertex_to_incoming_edges.h                              */
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

#include "data/ddls_edge.h"
#include "data/ddls_vertex.h"

class IteratorFromVertexToIncomingEdges {
	DDLSVertex from_vertex;
	DDLSEdge next_edge;

	DDLSEdge result_edge;

public:
	// can be chained, eg.: it = ...set_from_vertex()
	IteratorFromVertexToIncomingEdges &set_from_vertex(DDLSVertex p_vertex) {
		from_vertex = p_vertex;
		next_edge = p_vertex->get_edge();
		while (!next_edge->if_is_real()) {
			next_edge = next_edge->get_rot_left_edge();
		}
		return *this;
	}

	DDLSEdge next() {
		if (next_edge) {
			result_edge = next_edge->get_opposite_edge();
			do {
				next_edge = next_edge->get_rot_left_edge();
				if (next_edge == from_vertex->get_edge()) {
					next_edge = nullptr;
					break;
				}
			} while (!next_edge->if_is_real());
		} else {
			result_edge = nullptr;
		}

		return result_edge;
	}

	IteratorFromVertexToIncomingEdges() {}
};
