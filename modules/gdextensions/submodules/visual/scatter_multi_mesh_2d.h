#ifndef SCATTER_MULTI_MESH_H
#define SCATTER_MULTI_MESH_H

#include "scene/2d/node_2d.h"

class ScatterMultiMesh: public Node2D {
	GDCLASS(ScatterMultiMesh, Node2D)

	Ref<Mesh> mesh_library;
	int num_layers;

protected:
	static void _bind_methods();
	void _notification(int p_notification);

public:
	void set_mesh_library(Ref<Mesh> p_mesh_library);
	Ref<Mesh> get_mesh_library() const;

	void set_num_layers(int p_num_layers);
	int get_num_layers() const;

	ScatterMultiMesh();
	~ScatterMultiMesh();
};

#endif // SCATTER_MULTI_MESH_H
