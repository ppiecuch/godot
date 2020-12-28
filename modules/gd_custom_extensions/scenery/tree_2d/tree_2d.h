#ifndef GODOT_TREE_2D_H
#define GODOT_TREE_2D_H

#include "core/typedefs.h"
#include "core/object.h"
#include "core/math/vector2.h"
#include "scene/resources/texture.h"
#include "scene/resources/mesh.h"
#include "scene/2d/node_2d.h"


class Tree2D : public Node2D {
	GDCLASS(Tree2D, Node2D);
private:

protected:
	void _notification(int p_what);
	static void _bind_methods();

	Ref<Mesh> mesh;
	Ref<Texture> texture;

public:
#ifdef TOOLS_ENABLED
	virtual Rect2 _edit_get_rect() const;
#endif

	void set_mesh(const Ref<Mesh> &p_mesh);
	Ref<Mesh> get_mesh() const;

	void set_texture(const Ref<Texture> &p_texture);
	Ref<Texture> get_texture() const;

	Tree2D();
	~Tree2D();
};

#endif //GODOT_TREE_2D_H

