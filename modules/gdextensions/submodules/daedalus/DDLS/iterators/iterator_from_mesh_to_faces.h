#include "data/ddls_face.h"
#include "data/ddls_mesh.h"

class IteratorFromMeshToFaces {
	DDLSMesh from_mesh;
	int curr_index;

	DDLSFace result_face;

public:
	// can be chained, eg.: it = ...set_from_mesh()
	IteratorFromMeshToFaces &set_from_mesh(DDLSMesh value) {
		from_mesh = value;
		curr_index = 0;
		return *this;
	}

	DDLSFace next() {
		const Vector<DDLSFace> faces = from_mesh->get_faces();
		do {
			if (curr_index < faces.size()) {
				result_face = faces[curr_index];
				curr_index++;
			} else {
				result_face = nullptr;
				break;
			}
		} while(!result_face.if_is_real());
		return result_face;
	}

	IteratorFromMeshToFaces() { }
};
