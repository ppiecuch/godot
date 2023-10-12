/**************************************************************************/
/*  ddls_mesh.h                                                           */
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

#include "common/gd_core.h"
#include "core/reference.h"
#include "core/vector.h"

#include "ddls_fwd.h"

class DDLS_Mesh : public Reference {
	unsigned id;

	real_t width;
	real_t height;
	bool clipping;

	Vector<DDLSVertex> vertices;
	Vector<DDLSEdge> edges;
	Vector<DDLSFace> faces;
	Vector<DDLSConstraintShape> constraint_shapes;
	Vector<DDLSObject> objects;

	// keep references of center vertex and bounding edges when split, useful to restore edges as Delaunay
	DDLSVertex _center_vertex;
	Vector<DDLSEdge> _edges_to_check;

	bool _objects_update_in_progress;

public:
	unsigned get_id() const { return id; }

	real_t get_height() const { return height; }
	real_t get_width() const { return width; }
	void set_size(real_t p_width, real_t p_height) {
		width = p_width;
		height = p_height;
	}

	bool get_clipping() const { return clipping; }
	void set_clipping(bool value) { clipping = value; }

	Vector<DDLSVertex> get_vertices() const;
	Vector<DDLSEdge> get_edges() const;
	Vector<DDLSFace> get_faces() const;
	Vector<DDLSConstraintShape> get_constraint_shapes() const;
	void set_constraint_shapes(const Vector<DDLSConstraintShape> &p_shapes);

#define _Arg(N) DDLSVertex p_vertex##N = DDLSVertex()
	void add_vertex(DDLSVertex p_vertex1, _Arg(2), _Arg(3), _Arg(4));
#undef _Arg
#define _Arg(N) DDLSEdge p_edge##N = DDLSEdge()
	void add_edge(DDLSEdge p_edge1, _Arg(2), _Arg(3), _Arg(4));
#undef _Arg
#define _Arg(N) DDLSFace p_face##N = DDLSFace()
	void add_face(DDLSFace p_face1, _Arg(2), _Arg(3), _Arg(4));
#undef _Arg

	void build_from_record(const String &rec) {
		const Vector<String> p = rec.split(";");
		ERR_FAIL_COND((p.size() & 0x11) != 0);
		for (int i = 0; i < p.size(); i += 4) {
			insert_constraint_segment({ p[i].to_float(), p[i + 1].to_float() }, { p[i + 2].to_float(), p[i + 3].to_float() });
		}
	}

	void insert_object(DDLSObject p_object);
	void delete_object(DDLSObject p_object);
	void update_objects();

	// insert a new collection of constrained edges.
	// Coordinates parameter is a list with form [x0, y0, x1, y1, x2, y2, x3, y3, x4, y4, ....]
	// where each 4-uple sequence (xi, yi, xi+1, yi+1) is a constraint segment (with i % 4 == 0)
	// and where each couple sequence (xi, yi) is a point.
	// Segments are not necessary connected.
	// Segments can overlap (then they will be automaticaly subdivided).
	DDLSConstraintShape insert_constraint_shape(const Vector<Point2> &p_coordinates);
	void add_constraint_shape(DDLSConstraintShape p_shapes);
	void delete_constraint_shape(DDLSConstraintShape p_shape);
	DDLSConstraintSegment insert_constraint_segment(const Point2 &p1, const Point2 &p2);
	void insert_new_constrained_edge(DDLSConstraintSegment p_from_segment, DDLSEdge p_edge_down_up, Vector<DDLSEdge> p_intersected_edges, Vector<DDLSEdge> p_left_bounding_edges, Vector<DDLSEdge> p_right_bounding_edges);
	void delete_constraint_segment(DDLSConstraintSegment p_segment);

	void check() const;

	DDLSVertex get_vertex(int p_index) const;
	DDLSVertex insert_vertex(const Point2 &p_pos);

	DDLSEdge flip_edge(DDLSEdge edge);

	DDLSVertex split_edge(DDLSEdge p_edge, const Point2 &p_pos);
	DDLSVertex split_face(DDLSFace p_face, const Point2 &p_pos);

	void restore_as_delaunay();

	bool delete_vertex(DDLSVertex p_vertex);

	/// PRIVATE

	void untriangulate(Vector<DDLSEdge> p_edges_list);
	void triangulate(Vector<DDLSEdge> p_bound, bool p_is_real);

	void debug() const;

	DDLS_Mesh();
};
