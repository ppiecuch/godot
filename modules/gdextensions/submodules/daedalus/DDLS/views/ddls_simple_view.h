/**************************************************************************/
/*  ddls_simple_view.h                                                    */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

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
