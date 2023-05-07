#include "data/ddls_edge.h"
#include "data/ddls_face.h"

class IteratorFromFaceToInnerEdges {
	DDLSFace from_face;
	DDLSEdge next_edge;
	DDLSEdge result_edge;

public:
	// can be chained, eg.: it = ...set_from_face()
	IteratorFromFaceToInnerEdges &set_from_face(DDLSFace p_face) {
		from_face = p_face;
		next_edge = from_face->get_edge();
		return *this;
	}

	DDLSEdge next() {
		if (next_edge) {
			result_edge = next_edge;
			next_edge = next_edge->get_next_left_edge();
			if (next_edge == from_face->get_edge()) {
				next_edge = nullptr;
			}
		} else {
			result_edge = nullptr;
		}
		return result_edge;
	}

	IteratorFromFaceToInnerEdges() { }
};
