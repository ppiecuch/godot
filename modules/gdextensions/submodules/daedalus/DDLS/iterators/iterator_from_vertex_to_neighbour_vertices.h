#include "data/ddls_edge.h"
#include "data/ddls_vertex.h"

class IteratorFromVertexToNeighbourVertices {
	DDLSVertex from_vertex;
	DDLSEdge next_edge;

	DDLSVertex result_vertex;

public:
	// can be chained, eg.: it = ...set_from_vertex()
	IteratorFromVertexToNeighbourVertices &set_from_vertex(DDLSVertex p_vertex) {
		from_vertex = p_vertex;
		next_edge = from_vertex->get_edge();
	}

	DDLSVertex next() {
		if (next_edge) {
			result_vertex = next_edge->get_opposite_edge()->get_origin_vertex();
			do {
				next_edge = next_edge->get_rot_left_edge();
			} while (!next_edge->if_is_real());
			if (next_edge == from_vertex->get_edge()) {
				next_edge = nullptr;
			}
		} else {
			result_vertex = nullptr;
		}
		return result_vertex;
	}

	IteratorFromVertexToNeighbourVertices() { }
};
