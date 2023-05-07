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
		while (!next_edge.if_is_real()) {
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
			} while(!next_edge->if_is_real());
		} else {
			result_edge = nullptr;
		}

		return result_edge;
	}

	IteratorFromVertexToIncomingEdges() { }
};
