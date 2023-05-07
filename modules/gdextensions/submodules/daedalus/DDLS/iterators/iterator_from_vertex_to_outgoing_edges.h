#pragma once

#include "data/ddls_edge.h"
#include "data/ddls_vertex.h"

class IteratorFromVertexToOutgoingEdges {
	DDLSVertex from_vertex;
	DDLSEdge next_edge;

	DDLSEdge result_edge;

	bool real_edges_only = true;

public:
	// can be chained, eg.: it = ...set_from_vertex()
	IteratorFromVertexToOutgoingEdges &set_from_vertex(DDLSVertex p_vertex) {
		from_vertex = p_vertex;
		next_edge = from_vertex->get_edge();
		while (real_edges_only && ! next_edge->if_is_real()) {
			next_edge = next_edge->get_rot_left_edge();
		}
		return *this;
	}

	DDLSEdge next() {
		if (next_edge) {
			result_edge = next_edge;
			do {
				next_edge = next_edge->get_rot_left_edge();
				if (next_edge == from_vertex->get_edge()) {
					next_edge = nullptr;
					break;
				}
			} while(real_edges_only && ! next_edge->if_is_real());
		} else {
			result_edge = nullptr;
		}
		return result_edge;
	}

	IteratorFromVertexToOutgoingEdges() { }
};
