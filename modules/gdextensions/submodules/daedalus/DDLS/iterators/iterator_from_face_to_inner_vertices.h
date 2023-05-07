#include "data/ddls_edge.h"
#include "data/ddls_face.h"

class IteratorFromFaceToInnerVertices {
	DDLSFace from_face;
	DDLSEdge next_edge;

	DDLSVertex result_vertex;

public:
	// can be chained, eg.: it = ...set_from_face()
	IteratorFromFaceToInnerVertices &set_from_face(DDLSFace p_face) {
		from_face = p_face;
		next_edge = from_face->get_edge();
		return *this;
	}

	DDLSVertex next() {
		if (next_edge) {
			result_vertex = next_edge->get_origin_vertex();
			next_edge = next_edge->get_next_left_edge();
			if (next_edge == from_face->get_edge()) {
				next_edge = nullptr;
			}
		} else {
			result_vertex = nullptr;
		}
		return result_vertex;
	}

	IteratorFromFaceToInnerVertices() { }
};
