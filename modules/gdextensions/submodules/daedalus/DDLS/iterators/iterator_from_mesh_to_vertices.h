#include "data/ddls_vertex.h"
#include "data/ddls_mesh.h"

class IteratorFromMeshToVertices {
	DDLSMesh from_mesh;
	int curr_index;

	DDLSVertex result_vertex;

public:
	// can be chained, eg.: it = ...set_from_mesh()
	IteratorFromMeshToVertices &set_from_mesh(DDLSMesh p_mesh) {
		from_mesh = p_mesh;
		curr_index = 0;
		return *this;
	}

	DDLSVertex next() {
		Vector<DDLSVertex> vertices = from_mesh->get_vertices();
		do {
			if (curr_index < vertices.size()) {
				result_vertex = vertices[curr_index];
				curr_index++;
			} else {
				result_vertex = nullptr;
				break;
			}
		} while (!result_vertex->if_is_real());
		return result_vertex;
	}

	IteratorFromMeshToVertices() { }
};
