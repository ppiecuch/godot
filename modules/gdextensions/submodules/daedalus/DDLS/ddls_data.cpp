/**************************************************************************/
/*  data.cpp                                                              */
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

#include "ddls_constraint_shape.h"
#include "ddls_constraint_segment.h"
#include "ddls_face.h"
#include "ddls_edge.h"
#include "ddls_vertex.h"
#include "ddls_object.h"
#include "ddls_mesh.h"

static unsigned CONSTRAINT_SHAPE_COUNTER = 0;
static unsigned CONSTRAINT_SEGMENT_COUNTER = 0;
static unsigned FACE_COUNTER = 0;
static unsigned EDGE_COUNTER = 0;
static unsigned VERTEX_COUNTER = 0;
static unsigned OBJECT_COUNTER = 0;
static unsigned MESH_COUNTER = 0;

/// DDLS_ConstraintShape

Vector<DDLSConstraintSegment> DDLS_ConstraintShape::get_segments() const { return segments; }

DDLS_ConstraintShape::DDLS_ConstraintShape() : id(CONSTRAINT_SHAPE_COUNTER++) {}

/// DDLS_ConstraintSegment

DDLSConstraintShape DDLS_ConstraintSegment::get_from_shape() const { return from_shape; }
void DDLS_ConstraintSegment::set_from_shape(DDLSConstraintShape p_value) { from_shape = p_value; }

Vector<DDLSEdge> DDLS_ConstraintSegment::get_edges() const { return edges; }

void DDLS_ConstraintSegment::add_edge(DDLSEdge p_edge) {
	if (!edges.has(p_edge) && !edges.has(p_edge->get_opposite_edge())) {
		edges.push_back(p_edge);
	}
}

void DDLS_ConstraintSegment::remove_edge(DDLSEdge p_edge) {
	int index = edges.find(p_edge);
	if (index == -1) {
		index = edges.find(p_edge->get_opposite_edge());
	}
	if (index != -1) {
		edges.remove(index);
	}
}

DDLS_ConstraintSegment::DDLS_ConstraintSegment() : id(CONSTRAINT_SEGMENT_COUNTER++) {}

/// DDLS_Face

void DDLS_Face::set_datas(DDLSEdge p_edge, bool p_is_real) {
	is_real = p_is_real;
	edge = p_edge;
}

DDLSEdge DDLS_Face::get_edge() const { return edge; }

DDLS_Face::DDLS_Face() : id(FACE_COUNTER++) {}

/// DDLS_Edge

void DDLS_Edge::set_datas(DDLSVertex p_origin_vertex, DDLSEdge p_opposite_edge, DDLSEdge p_next_left_edge, DDLSFace p_left_face, bool p_is_real, bool p_is_constrained) {
	is_constrained = p_is_constrained;
	is_real = p_is_real;
	origin_vertex = p_origin_vertex;
	opposite_edge = p_opposite_edge;
	next_left_edge = p_next_left_edge;
	left_face = p_left_face;
}

void DDLS_Edge::add_from_constraint_segment(DDLSConstraintSegment p_segment) {
	if (!from_constraint_segments.has(p_segment)) {
		from_constraint_segments.push_back(p_segment);
	}
}

void DDLS_Edge::remove_from_constraint_segment(DDLSConstraintSegment p_segment) {
	from_constraint_segments.erase(p_segment);
}

void DDLS_Edge::set_origin_vertex(DDLSVertex p_vertex) { origin_vertex = p_vertex; }
void DDLS_Edge::set_next_left_edge(DDLSEdge p_edge) { next_left_edge = p_edge; }
void DDLS_Edge::set_left_face(DDLSFace p_face) { left_face = p_face; }
void DDLS_Edge::set_is_constrained(bool p_value) { is_constrained = p_value; }

Vector<DDLSConstraintSegment> DDLS_Edge::get_from_constraint_segments() const { return from_constraint_segments; }

void DDLS_Edge::set_from_constraint_segments(Vector<DDLSConstraintSegment> p_value) { from_constraint_segments = p_value; }

DDLSVertex DDLS_Edge::get_origin_vertex() const { return origin_vertex; }
DDLSVertex DDLS_Edge::get_destination_vertex() const { return opposite_edge->origin_vertex; }
DDLSEdge DDLS_Edge::get_opposite_edge() const { return opposite_edge; }
DDLSEdge DDLS_Edge::get_next_left_edge() const { return next_left_edge; }
DDLSEdge DDLS_Edge::get_prev_left_edge() const { return next_left_edge->next_left_edge; }
DDLSEdge DDLS_Edge::get_next_right_edge() const { return opposite_edge->next_left_edge->next_left_edge->opposite_edge; }
DDLSEdge DDLS_Edge::get_prev_right_rdge() const { return opposite_edge->next_left_edge->opposite_edge; }
DDLSEdge DDLS_Edge::get_rot_left_edge() const { return next_left_edge->next_left_edge->opposite_edge; }
DDLSEdge DDLS_Edge::get_rot_right_edge() const { return opposite_edge->next_left_edge; }
DDLSFace DDLS_Edge::get_left_face() const { return left_face; }
DDLSFace DDLS_Edge::get_right_face() const { return opposite_edge->left_face; }

String DDLS_Edge::string() const { return vformat("edge %d - %d", get_origin_vertex()->get_id(), get_destination_vertex()->get_id()); }

DDLS_Edge::DDLS_Edge() : id(EDGE_COUNTER++) {}

/// DDLS_Vertex

Point2 DDLS_Vertex::get_pos() const { return pos; }

Vector<DDLSConstraintSegment> DDLS_Vertex::get_from_constraint_segments() const { return from_constraint_segments; }
void DDLS_Vertex::set_from_constraint_segments(Vector<DDLSConstraintSegment> p_value) { from_constraint_segments = p_value; }
void DDLS_Vertex::set_datas(DDLSEdge p_edge, bool p_is_real) {
	is_real = p_is_real;
	edge = p_edge;
}

void DDLS_Vertex::add_from_constraint_segment(DDLSConstraintSegment p_segment) {
	if (!from_constraint_segments.has(p_segment)) {
		from_constraint_segments.push_back(p_segment);
	}
}
void DDLS_Vertex::remove_from_constraint_segment(DDLSConstraintSegment p_segment) {
	from_constraint_segments.erase(p_segment);
}

DDLSEdge DDLS_Vertex::get_edge() const { return edge; }
void DDLS_Vertex::set_edge(DDLSEdge p_edge) { edge = p_edge; }

DDLS_Vertex::DDLS_Vertex() : id(VERTEX_COUNTER++) {}

/// DDLS_Object

DDLSConstraintShape DDLS_Object::get_constraint_shape() const { return constraint_shape; }
void DDLS_Object::set_constraint_shape(DDLSConstraintShape p_shape) {
	constraint_shape = p_shape;
	has_changed = true;
}

Vector<DDLSEdge> DDLS_Object::get_edges() const {
	Vector<DDLSEdge> res;
	const Vector<DDLSConstraintSegment> segments = constraint_shape->get_segments();
	for (int i = 0 ; i < segments.size() ; i++) {
		const Vector<DDLSEdge> edges = segments[i]->get_edges();
		for (int j = 0 ; j < edges.size() ; j++)
			res.push_back(edges[j]);
	}
	return res;
}

DDLS_Object::DDLS_Object() : id(OBJECT_COUNTER++) {
	pivot_x = pivot_y = 0;
	scale_x = scale_y = 1;
	rotation = 0;
	x = y = 0;

	has_changed = false;
}

/// DDLS_Mesh

DDLS_Mesh::DDLS_Mesh(real_t p_width, real_t p_height) : id(MESH_COUNTER++) {
	width = p_width;
	height = p_height;
	clipping = true;
}
