#include "data/ddls_edge.h"
#include "data/ddls_face.h"

class IteratorFromFaceToNeighbourFaces {
	DDLSFace from_face;
	DDLSEdge next_edge;

	DDLSFace result_face;

public:
	// can be chained, eg.: it = ...set_from_vertex()
	IteratorFromFaceToNeighbourFaces &set_from_face(DDLSFace p_face) {
		from_face = p_face;
		next_edge = from_face->get_edge();
		return *this;
	}

	DDLSFace next() {
		if (next_edge) {
			do {
				result_face = next_edge->get_right_face();
				next_edge = next_edge->get_next_left_edge();
				if (next_edge == from_face->get_edge()) {
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

	IteratorFromFaceToNeighbourFaces() { }
};
