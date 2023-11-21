#pragma once

#include "scene/2d/node_2d.h"

#include "ddls_fwd.h"

class DDLSSimpleView : public Node2D {
	GDCLASS(DDLSSimpleView, Node2D);

	RID _paths;
	RID _edges;
	RID _surface;
	RID _vertices;
	RID _constraints;
	RID _entities;

	bool _show_vertices_indices;

	Ref<Font> _debug_font;

	bool vertex_is_inside_aabb(DDLSVertex p_vertex, DDLSMesh p_mesh);

protected:
	static void _bind_methods();
	void _notification(int p_notification);

public:
	void draw_mesh(DDLSMesh p_mesh);
	void draw_entity(DDLSEntityAI p_entity, bool p_clean_before = true);
	void draw_entities(const Vector<DDLSEntityAI> &p_entities, bool p_clean_before = true);
	void draw_path(const Vector<Point2> &p_path, bool p_clean_before = true);

	DDLSSimpleView();
};
