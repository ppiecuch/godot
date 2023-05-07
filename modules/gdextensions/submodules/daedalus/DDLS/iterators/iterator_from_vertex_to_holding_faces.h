#include "data/ddls_edge.h"
#include "data/ddls_face.h"
#include "data/ddls_vertex.h"

class IteratorFromVertexToHoldingFaces {
	DDLSVertex from_vertex;
	DDLSEdge next_edge;

	DDLSFace result_face;

public:
	// can be chained, eg.: it = ...set_from_vertex()
	IteratorFromVertexToHoldingFaces &set_from_vertex(DDLSVertex p_vertex) {
		from_vertex = p_vertex;
		next_edge = from_vertex->get_edge();
		return *this;
	}

	DDLSFace next() {
		if (next_edge) {
			do {
				result_face = next_edge->get_left_face();
				next_edge = next_edge->get_rot_left_edge();
				if (next_edge == from_vertex->get_edge()) {
					next_edge = nullptr;
					if (!result_face->if_is_real()) {
						result_face = nullptr;
					}
					break;
				}
			} while (!result_face->if_is_real());
		} else {
			result_face = nullptr;
		}
		return result_face;
	}

	function IteratorFromVertexToHoldingFaces() { }
};
