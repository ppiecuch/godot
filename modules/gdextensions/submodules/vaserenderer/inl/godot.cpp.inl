/* Godot renderer and backend */

namespace VASErin { // VASEr internal namespace

	Ref<ArrayMesh> _mesh;

	void backend::vah_draw(vertex_array_holder& vah) {
		if (!vah.vert.empty()) { // save some effort
			Array mesh_array;
			mesh_array.resize(VS::ARRAY_MAX);
			mesh_array[VS::ARRAY_VERTEX] = vah.vert;
			mesh_array[VS::ARRAY_COLOR] = vah.color;
			_mesh->add_surface_from_arrays(vah.drawmode, mesh_array, Array(), Mesh::ARRAY_FLAG_USE_2D_VERTICES);
		}
	}
} // namespace VASErin

void renderer::init() { }
void renderer::before() { }
void renderer::after() { }
