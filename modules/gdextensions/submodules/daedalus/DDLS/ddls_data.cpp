/**************************************************************************/
/*  ddls_data.cpp                                                         */
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

#include "data/ddls_constants.h"
#include "data/ddls_constraint_segment.h"
#include "data/ddls_constraint_shape.h"
#include "data/ddls_edge.h"
#include "data/ddls_face.h"
#include "data/ddls_mesh.h"
#include "data/ddls_object.h"
#include "data/ddls_vertex.h"
#include "data/graph/ddls_graph.h"
#include "data/graph/ddls_graph_edge.h"
#include "data/graph/ddls_graph_node.h"
#include "data/math/ddls_geom2d.h"
#include "iterators/iterator_from_vertex_to_outgoing_edges.h"

#include "core/error_macros.h"

static unsigned CONSTRAINT_SHAPE_COUNTER = 0;
static unsigned CONSTRAINT_SEGMENT_COUNTER = 0;
static unsigned FACE_COUNTER = 0;
static unsigned EDGE_COUNTER = 0;
static unsigned VERTEX_COUNTER = 0;
static unsigned OBJECT_COUNTER = 0;
static unsigned MESH_COUNTER = 0;
static unsigned GRAPH_NODE_COUNTER = 0;
static unsigned GRAPH_EDGE_COUNTER = 0;
static unsigned GRAPH_COUNTER = 0;

/// DDLS_ConstraintShape

Vector<DDLSConstraintSegment> DDLS_ConstraintShape::get_segments() const { return segments; }
void DDLS_ConstraintShape::add_segment(DDLSConstraintSegment p_segment1, DDLSConstraintSegment p_segment2, DDLSConstraintSegment p_segment3, DDLSConstraintSegment p_segment4) {
	ERR_FAIL_NULL(p_segment1);
	segments.push_back(p_segment1);
	if (p_segment2)
		segments.push_back(p_segment2);
	if (p_segment3)
		segments.push_back(p_segment3);
	if (p_segment4)
		segments.push_back(p_segment4);
}

DDLS_ConstraintShape::DDLS_ConstraintShape() :
		id(CONSTRAINT_SHAPE_COUNTER++) {}

/// DDLS_ConstraintSegment

DDLSConstraintShape DDLS_ConstraintSegment::get_from_shape() const { return from_shape; }
void DDLS_ConstraintSegment::set_from_shape(DDLSConstraintShape p_value) { from_shape = p_value; }

Vector<DDLSEdge> DDLS_ConstraintSegment::get_edges() const { return edges; }

DDLSEdge DDLS_ConstraintSegment::get_edge(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, edges.size(), DDLSEdge());
	return edges[p_index];
}

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

DDLS_ConstraintSegment::DDLS_ConstraintSegment() :
		id(CONSTRAINT_SEGMENT_COUNTER++) {}

/// DDLS_Face

void DDLS_Face::set_datas(DDLSEdge p_edge, bool p_is_real) {
	is_real = p_is_real;
	edge = p_edge;
}

DDLSEdge DDLS_Face::get_edge() const { return edge; }

DDLS_Face::DDLS_Face() :
		id(FACE_COUNTER++) {}

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
DDLSEdge DDLS_Edge::get_prev_right_edge() const { return opposite_edge->next_left_edge->opposite_edge; }
DDLSEdge DDLS_Edge::get_rot_left_edge() const { return next_left_edge->next_left_edge->opposite_edge; }
DDLSEdge DDLS_Edge::get_rot_right_edge() const { return opposite_edge->next_left_edge; }
DDLSFace DDLS_Edge::get_left_face() const { return left_face; }
DDLSFace DDLS_Edge::get_right_face() const { return opposite_edge->left_face; }

String DDLS_Edge::string() const { return vformat("edge %d - %d", get_origin_vertex()->get_id(), get_destination_vertex()->get_id()); }

DDLS_Edge::DDLS_Edge() :
		id(EDGE_COUNTER++) {}

/// DDLS_Vertex

Vector<DDLSConstraintSegment> DDLS_Vertex::get_from_constraint_segments() const { return from_constraint_segments; }
void DDLS_Vertex::set_from_constraint_segments(Vector<DDLSConstraintSegment> p_value) { from_constraint_segments = p_value; }
void DDLS_Vertex::set_datas(DDLSEdge p_edge, bool p_is_real) {
	is_real = p_is_real;
	edge = p_edge;
}

void DDLS_Vertex::add_from_constraint_segment(DDLSConstraintSegment p_segment1, DDLSConstraintSegment p_segment2, DDLSConstraintSegment p_segment3, DDLSConstraintSegment p_segment4) {
	ERR_FAIL_NULL(p_segment1);
	if (!from_constraint_segments.has(p_segment1)) {
		from_constraint_segments.push_back(p_segment1);
	}
	if (p_segment2 && !from_constraint_segments.has(p_segment2)) {
		from_constraint_segments.push_back(p_segment2);
	}
	if (p_segment3 && !from_constraint_segments.has(p_segment3)) {
		from_constraint_segments.push_back(p_segment3);
	}
	if (p_segment4 && !from_constraint_segments.has(p_segment4)) {
		from_constraint_segments.push_back(p_segment4);
	}
}
void DDLS_Vertex::remove_from_constraint_segment(DDLSConstraintSegment p_segment) {
	from_constraint_segments.erase(p_segment);
}

DDLSEdge DDLS_Vertex::get_edge() const { return edge; }
void DDLS_Vertex::set_edge(DDLSEdge p_edge) { edge = p_edge; }

DDLS_Vertex::DDLS_Vertex() :
		id(VERTEX_COUNTER++) {}

/// DDLS_Object

DDLSConstraintShape DDLS_Object::get_constraint_shape() const { return constraint_shape; }
void DDLS_Object::set_constraint_shape(DDLSConstraintShape p_shape) {
	constraint_shape = p_shape;
	has_changed = true;
}

Vector<DDLSEdge> DDLS_Object::get_edges() const {
	Vector<DDLSEdge> res;
	const Vector<DDLSConstraintSegment> segments = constraint_shape->get_segments();
	for (int i = 0; i < segments.size(); i++) {
		const Vector<DDLSEdge> edges = segments[i]->get_edges();
		for (int j = 0; j < edges.size(); j++)
			res.push_back(edges[j]);
	}
	return res;
}

DDLS_Object::DDLS_Object() :
		id(OBJECT_COUNTER++) {
	pivot = { 0, 0 };
	scale = { 1, 1 };
	translate = { 0, 0 };
	rotation = 0;

	has_changed = false;
}

/// DDLS_Mesh

Vector<DDLSVertex> DDLS_Mesh::get_vertices() const { return vertices; }
Vector<DDLSEdge> DDLS_Mesh::get_edges() const { return edges; }
Vector<DDLSFace> DDLS_Mesh::get_faces() const { return faces; }
Vector<DDLSConstraintShape> DDLS_Mesh::get_constraint_shapes() const { return constraint_shapes; }

void DDLS_Mesh::add_vertex(DDLSVertex p_vertex1, DDLSVertex p_vertex2, DDLSVertex p_vertex3, DDLSVertex p_vertex4) {
	ERR_FAIL_NULL(p_vertex1);
	vertices.push_back(p_vertex1);
	if (p_vertex2) {
		vertices.push_back(p_vertex2);
	}
	if (p_vertex3) {
		vertices.push_back(p_vertex3);
	}
	if (p_vertex4) {
		vertices.push_back(p_vertex4);
	}
}

void DDLS_Mesh::add_edge(DDLSEdge p_edge1, DDLSEdge p_edge2, DDLSEdge p_edge3, DDLSEdge p_edge4) {
	ERR_FAIL_NULL(p_edge1);
	vertices.push_back(p_edge1);
	if (p_edge2) {
		vertices.push_back(p_edge2);
	}
	if (p_edge3) {
		vertices.push_back(p_edge3);
	}
	if (p_edge4) {
		vertices.push_back(p_edge4);
	}
}

void DDLS_Mesh::add_face(DDLSFace p_face1, DDLSFace p_face2, DDLSFace p_face3, DDLSFace p_face4) {
	ERR_FAIL_NULL(p_face1);
	vertices.push_back(p_face1);
	if (p_face2) {
		vertices.push_back(p_face2);
	}
	if (p_face3) {
		vertices.push_back(p_face3);
	}
	if (p_face4) {
		vertices.push_back(p_face4);
	}
}

void DDLS_Mesh::insert_object(DDLSObject p_object) {
	delete_object(p_object);
	if (p_object->get_constraint_shape()) {
	}
	DDLSConstraintShape shape;
	shape.instance();
	Vector<Point2> coordinates = p_object->get_coordinates();
	Transform2D m = p_object->get_matrix();
	p_object->update_matrix_from_values();
	for (int i = 0; i < coordinates.size() - 1; i += 2) {
		const Point2 p1 = coordinates[i + 0];
		const Point2 p2 = coordinates[i + 1];
		const Point2 transf1 = { m.tdotx(p1), m.tdoty(p1) };
		const Point2 transf2 = { m.tdotx(p2), m.tdoty(p2) };

		DDLSConstraintSegment segment = insert_constraint_segment(transf1, transf2);
		if (segment) {
			segment->set_from_shape(shape);
			shape->add_segment(segment);
		}
	}

	constraint_shapes.push_back(shape);
	p_object->set_constraint_shape(shape);

	if (!_objects_update_in_progress) {
		objects.push_back(p_object);
	}
}

void DDLS_Mesh::delete_object(DDLSObject p_object) {
	if (!p_object->get_constraint_shape()) {
		return;
	}
	delete_constraint_shape(p_object->get_constraint_shape());
	p_object->set_constraint_shape(nullptr);

	if (!_objects_update_in_progress) {
		objects.erase(p_object);
	}
}

void DDLS_Mesh::update_objects() {
	_objects_update_in_progress = true;
	for (int i = 0; i < objects.size(); i++) {
		if (objects[i]->is_changed()) {
			delete_object(objects[i]);
			insert_object(objects[i]);
			objects.write[i]->set_changed(false);
		}
	}
	_objects_update_in_progress = false;
}

DDLSConstraintShape DDLS_Mesh::insert_constraint_shape(const Vector<Point2> &p_coordinates) {
	ERR_FAIL_COND_V(p_coordinates.size() < 2, DDLSConstraintShape());

	DDLSConstraintShape shape;
	for (int i = 0; i < p_coordinates.size(); i += 2) {
		DDLSConstraintSegment segment = insert_constraint_segment(p_coordinates[i], p_coordinates[i + 1]);
		if (segment) {
			segment->set_from_shape(shape);
			shape->add_segment(segment);
		}
	}
	constraint_shapes.push_back(shape);
	return shape;
}

void DDLS_Mesh::delete_constraint_shape(DDLSConstraintShape p_shape) {
	Vector<DDLSConstraintSegment> segments = p_shape->get_segments();
	for (int i = 0; i < segments.size(); i++) {
		delete_constraint_segment(segments[i]);
	}
	constraint_shapes.erase(p_shape);
}

DDLSConstraintSegment DDLS_Mesh::insert_constraint_segment(const Point2 &p1, const Point2 &p2) {
	// we clip against AABB
	Point2 new_1 = p1;
	Point2 new_2 = p2;

	if ((p1.x > width && p2.x > width) || (p1.x < 0 && p2.x < 0) || (p1.y > height && p2.y > height) || (p1.y < 0 && p2.y < 0)) {
		return nullptr;
	} else {
		Point2 pn = p2 - p1;

		real_t tmin = DDLS::NEGATIVE_INFINITY;
		real_t tmax = DDLS::POSITIVE_INFINITY;

		if (pn.x != 0) {
			const real_t tx1 = (0 - p1.x) / pn.x;
			const real_t tx2 = (width - p1.x) / pn.x;

			tmin = MAX(tmin, MIN(tx1, tx2));
			tmax = MIN(tmax, MAX(tx1, tx2));
		}

		if (pn.y != 0) {
			const real_t ty1 = (0 - p1.y) / pn.y;
			const real_t ty2 = (height - p1.y) / pn.y;

			tmin = MAX(tmin, MIN(ty1, ty2));
			tmax = MIN(tmax, MAX(ty1, ty2));
		}

		if (tmax >= tmin) {
			if (tmax < 1) {
				new_2 = pn * tmax + p1; // clip end point
			}
			if (tmin > 0) {
				new_1 = pn * tmin + p1; // clip start point
			}
		} else {
			return nullptr;
		}
	}

	// we check the vertices insertions
	DDLSVertex vertex_down = insert_vertex(new_1);
	if (!vertex_down) {
		return nullptr;
	}
	DDLSVertex vertex_up = insert_vertex(new_2);
	if (!vertex_up) {
		return nullptr;
	}
	if (vertex_down == vertex_up) {
		return nullptr;
	}

	// printv_verbose("vertices", vertex_down->get_id(), vertex_up->get_id())

	// the new constraint segment
	DDLSConstraintSegment segment;
	segment.instance();

	DDLSEdge temp_edge_down_up, temp_edge_up_down;
	temp_edge_down_up->set_datas(vertex_down, temp_edge_up_down, nullptr, nullptr, true, true);
	temp_edge_up_down->set_datas(vertex_up, temp_edge_down_up, nullptr, nullptr, true, true);

	Vector<DDLSEdge> intersected_edges;
	Vector<DDLSEdge> left_bounding_edges;
	Vector<DDLSEdge> right_bounding_edges;

	Point2 p_intersect;
	DDLSEdge new_edge_down_up, new_edge_up_down;
	Variant curr_object = vertex_down;
	while (true) {
		bool done = false;
		IteratorFromVertexToOutgoingEdges iter_edges;
		if (DDLSVertex curr_vertex = curr_object) {
			// printv_verbose("case vertex");
			iter_edges = IteratorFromVertexToOutgoingEdges().set_from_vertex(curr_vertex);
			while (DDLSEdge curr_edge = iter_edges.next()) {
				// if we meet directly the end vertex
				if (curr_edge->get_destination_vertex() == vertex_up) {
					// printv_verbose("we met the end vertex");
					if (!curr_edge->if_is_constrained()) {
						curr_edge->set_is_constrained(true);
						curr_edge->get_opposite_edge()->set_is_constrained(true);
					}
					curr_edge->add_from_constraint_segment(segment);
					curr_edge->get_opposite_edge()->set_from_constraint_segments(curr_edge->get_from_constraint_segments());
					vertex_down->add_from_constraint_segment(segment);
					vertex_up->add_from_constraint_segment(segment);
					segment->add_edge(curr_edge);
					return segment;
				}
				// if we meet a vertex
				if (DDLSGeom2D::distance_squared_vertex_to_edge(curr_edge->get_destination_vertex(), temp_edge_down_up) <= DDLS::EPSILON_SQUARED) {
					// printv_verbose("we met a vertex");
					if (!curr_edge->if_is_constrained()) {
						// printv_verbose("edge is not constrained");
						curr_edge->set_is_constrained(true);
						curr_edge->get_opposite_edge()->set_is_constrained(true);
					}
					curr_edge->add_from_constraint_segment(segment);
					curr_edge->get_opposite_edge()->set_from_constraint_segments(curr_edge->get_from_constraint_segments());
					vertex_down->add_from_constraint_segment(segment);
					segment->add_edge(curr_edge);
					vertex_down = curr_edge->get_destination_vertex();
					temp_edge_down_up->set_origin_vertex(vertex_down);
					curr_object = vertex_down;
					done = true;
					break;
				}
			}

			if (done) {
				continue;
			}

			iter_edges = IteratorFromVertexToOutgoingEdges().set_from_vertex(curr_vertex);
			while (DDLSEdge curr_edge = iter_edges.next()) {
				curr_edge = curr_edge->get_next_left_edge();
				if (DDLSGeom2D::intersections2edges(curr_edge, temp_edge_down_up, &p_intersect)) {
					// print_verbose("edge intersection");
					if (curr_edge->if_is_constrained()) {
						// print_verbose("edge is constrained");
						vertex_down = split_edge(curr_edge, p_intersect);
						iter_edges = IteratorFromVertexToOutgoingEdges().set_from_vertex(curr_vertex);
						while ((curr_edge = iter_edges.next())) {
							if (curr_edge->get_destination_vertex() == vertex_down) {
								curr_edge->set_is_constrained(true);
								curr_edge->get_opposite_edge()->set_is_constrained(true);
								curr_edge->add_from_constraint_segment(segment);
								curr_edge->get_opposite_edge()->set_from_constraint_segments(curr_edge->get_from_constraint_segments());
								segment->add_edge(curr_edge);
								break;
							}
						}
						curr_vertex->add_from_constraint_segment(segment);
						temp_edge_down_up->set_origin_vertex(vertex_down);
						curr_object = vertex_down;
					} else {
						// print_verbose("edge is not constrained");
						intersected_edges.push_back(curr_edge);
						left_bounding_edges.insert(curr_edge->get_next_left_edge());
						right_bounding_edges.push_back(curr_edge->get_prev_left_edge());
						curr_edge = curr_edge->get_opposite_edge(); // we keep the edge from left to right
						curr_object = curr_edge;
					}
					break;
				}
			}
		} else if (DDLSEdge curr_edge = curr_object) {
			// printv_verbose("case edge");
			DDLSEdge edge_left = curr_edge->get_next_left_edge();
			if (edge_left->get_destination_vertex() == vertex_up) {
				// print_verbose("end point reached");
				left_bounding_edges.insert(edge_left->get_next_left_edge());
				right_bounding_edges.push_back(edge_left);

				new_edge_down_up.instance();
				new_edge_up_down.instance();
				new_edge_down_up->set_datas(vertex_down, new_edge_up_down, nullptr, nullptr, true, true);
				new_edge_up_down->set_datas(vertex_up, new_edge_down_up, nullptr, nullptr, true, true);
				left_bounding_edges.push_back(new_edge_down_up);
				right_bounding_edges.push_back(new_edge_up_down);
				insert_new_constrained_edge(segment, new_edge_down_up, intersected_edges, left_bounding_edges, right_bounding_edges);

				return segment;
			} else if (DDLSGeom2D::distance_squared_vertex_to_edge(edge_left->get_destination_vertex(), temp_edge_down_up) <= DDLS::EPSILON_SQUARED) {
				// print_verbose("we met a vertex");
				left_bounding_edges.insert(edge_left->get_next_left_edge());
				right_bounding_edges.push_back(edge_left);

				new_edge_down_up.instance();
				new_edge_up_down.instance();
				new_edge_down_up->set_datas(vertex_down, new_edge_up_down, nullptr, nullptr, true, true);
				new_edge_up_down->set_datas(edge_left->get_destination_vertex(), new_edge_down_up, nullptr, nullptr, true, true);
				left_bounding_edges.push_back(new_edge_down_up);
				right_bounding_edges.push_back(new_edge_up_down);
				insert_new_constrained_edge(segment, new_edge_down_up, intersected_edges, left_bounding_edges, right_bounding_edges);

				intersected_edges.clear();
				left_bounding_edges.clear();
				right_bounding_edges.clear();

				vertex_down = edge_left->get_destination_vertex();
				temp_edge_down_up->set_origin_vertex(vertex_down);
				curr_object = vertex_down;
			} else {
				if (DDLSGeom2D::intersections2edges(edge_left, temp_edge_down_up, &p_intersect)) {
					// print_verbose("1st left edge intersected");
					if (edge_left->if_is_constrained()) {
						//print_verbose("edge is constrained");
						curr_vertex = split_edge(edge_left, p_intersect);

						iter_edges = IteratorFromVertexToOutgoingEdges().set_from_vertex(curr_vertex);
						while ((curr_edge = iter_edges.next())) {
							if (curr_edge->get_destination_vertex() == left_bounding_edges[0]->get_origin_vertex()) {
								left_bounding_edges.insert(curr_edge);
							}
							if (curr_edge->get_destination_vertex() == right_bounding_edges[right_bounding_edges.size() - 1]->get_destination_vertex()) {
								right_bounding_edges.push_back(curr_edge->get_opposite_edge());
							}
						}

						new_edge_down_up.instance();
						new_edge_up_down.instance();
						new_edge_down_up->set_datas(vertex_down, new_edge_up_down, nullptr, nullptr, true, true);
						new_edge_up_down->set_datas(curr_vertex, new_edge_down_up, nullptr, nullptr, true, true);
						left_bounding_edges.push_back(new_edge_down_up);
						right_bounding_edges.push_back(new_edge_up_down);
						insert_new_constrained_edge(segment, new_edge_down_up, intersected_edges, left_bounding_edges, right_bounding_edges);

						intersected_edges.clear();
						left_bounding_edges.clear();
						right_bounding_edges.clear();
						vertex_down = curr_vertex;
						temp_edge_down_up->set_origin_vertex(vertex_down);
						curr_object = vertex_down;
					} else {
						// printv_verbose("edge is not constrained");
						intersected_edges.push_back(edge_left);
						left_bounding_edges.insert(edge_left->get_next_left_edge());
						curr_edge = edge_left->get_opposite_edge(); // we keep the edge from left to right
						curr_object = curr_edge;
					}
				} else {
					// printv_verbose("2nd left edge intersected");
					edge_left = edge_left->get_next_left_edge();
					DDLSGeom2D::intersections2edges(edge_left, temp_edge_down_up, &p_intersect);
					if (edge_left->if_is_constrained()) {
						//printv_verbose("edge is constrained");
						curr_vertex = split_edge(edge_left, p_intersect);

						iter_edges = IteratorFromVertexToOutgoingEdges().set_from_vertex(curr_vertex);
						while ((curr_edge = iter_edges.next())) {
							if (curr_edge->get_destination_vertex() == left_bounding_edges[0]->get_origin_vertex()) {
								left_bounding_edges.insert(curr_edge);
							}
							if (curr_edge->get_destination_vertex() == right_bounding_edges[right_bounding_edges.size() - 1]->get_destination_vertex()) {
								right_bounding_edges.push_back(curr_edge->get_opposite_edge());
							}
						}

						new_edge_down_up.instance();
						new_edge_up_down.instance();
						new_edge_down_up->set_datas(vertex_down, new_edge_up_down, nullptr, nullptr, true, true);
						new_edge_up_down->set_datas(curr_vertex, new_edge_down_up, nullptr, nullptr, true, true);
						left_bounding_edges.push_back(new_edge_down_up);
						right_bounding_edges.push_back(new_edge_up_down);
						insert_new_constrained_edge(segment, new_edge_down_up, intersected_edges, left_bounding_edges, right_bounding_edges);

						intersected_edges.clear();
						left_bounding_edges.clear();
						right_bounding_edges.clear();
						vertex_down = curr_vertex;
						temp_edge_down_up->set_origin_vertex(vertex_down);
						curr_object = vertex_down;
					} else {
						// printv_verbose("edge is not constrained");
						intersected_edges.push_back(edge_left);
						right_bounding_edges.push_back(edge_left->get_prev_left_edge());
						curr_edge = edge_left->get_opposite_edge(); // we keep the edge from left to right
						curr_object = curr_edge;
					}
				}
			}
		}
	}

	return segment;
}

void DDLS_Mesh::insert_new_constrained_edge(DDLSConstraintSegment p_from_segment, DDLSEdge p_edge_down_up, Vector<DDLSEdge> p_intersected_edges, Vector<DDLSEdge> p_left_bounding_edges, Vector<DDLSEdge> p_right_bounding_edges) {
	// printv_verbose("insertNewConstrainedEdge");
	edges.push_back(p_edge_down_up);
	edges.push_back(p_edge_down_up->get_opposite_edge());

	p_edge_down_up->add_from_constraint_segment(p_from_segment);
	p_edge_down_up->get_opposite_edge()->set_from_constraint_segments(p_edge_down_up->get_from_constraint_segments());

	p_from_segment->add_edge(p_edge_down_up);

	p_edge_down_up->get_origin_vertex()->add_from_constraint_segment(p_from_segment);
	p_edge_down_up->get_destination_vertex()->add_from_constraint_segment(p_from_segment);

	untriangulate(p_intersected_edges);

	triangulate(p_left_bounding_edges, true);
	triangulate(p_right_bounding_edges, true);
}

void DDLS_Mesh::delete_constraint_segment(DDLSConstraintSegment p_segment) {
	// printv_verbose("deleteConstraintSegment id", segment.id);
	Vector<DDLSVertex> vertex_to_delete;
	DDLSEdge edge;
	const Vector<DDLSEdge> edges = p_segment->get_edges();
	for (int i = 0; i < edges.size(); i++) {
		edge = edges[i];
		// printv_verbose("unconstrain edge ", edge);
		edge->remove_from_constraint_segment(p_segment);
		if (edge->get_from_constraint_segments().empty()) {
			edge->set_is_constrained(false);
			edge->get_opposite_edge()->set_is_constrained(false);
		}

		DDLSVertex vertex = edge->get_origin_vertex();
		vertex->remove_from_constraint_segment(p_segment);
		vertex_to_delete.push_back(vertex);
	}
	DDLSVertex vertex = edge->get_destination_vertex();
	vertex->remove_from_constraint_segment(p_segment);
	vertex_to_delete.push_back(vertex);

	// print_verbose("clean the useless vertices");
	for (int i = 0; i < vertex_to_delete.size(); i++) {
		delete_vertex(vertex_to_delete[i]);
	}
	// print_verbose("clean done");
}

DDLSVertex DDLS_Mesh::get_vertex(int p_index) const {
	return vertices[p_index];
}

DDLSVertex DDLS_Mesh::insert_vertex(const Point2 &p_pos) {
	// printv_verbose("insert_vertex", p_pos);
	if (p_pos.x < 0 || p_pos.y < 0 || p_pos.x > width || p_pos.y > height) {
		return nullptr;
	}
	_edges_to_check.clear();

	Variant in_object = DDLSGeom2D::locate_position(p_pos, this);
	DDLSVertex new_vertex;
	if (DDLSVertex in_vertex = in_object) {
		// printv_verbose("in_vertex", in_vertex->get_id());
		new_vertex = in_vertex;
	} else if (DDLSEdge in_edge = in_object) {
		// printv_verbose("in_edge", in_edge);
		new_vertex = split_edge(in_edge, p_pos);
	} else if (DDLSFace in_face = in_object) {
		// print_verbose("in_face");
		new_vertex = split_face(in_face, p_pos);
	}

	restore_as_delaunay();
	return new_vertex;
}

DDLSEdge DDLS_Mesh::flip_edge(DDLSEdge p_edge) {
	// retrieve and create useful objets
	DDLSEdge e_bot_top = p_edge;
	DDLSEdge e_top_bot = p_edge->get_opposite_edge();
	DDLSEdge e_left_right;
	e_left_right.instance();
	DDLSEdge e_right_left;
	e_right_left.instance();
	DDLSEdge e_top_left = e_bot_top->get_next_left_edge();
	DDLSEdge e_left_bot = e_top_left->get_next_left_edge();
	DDLSEdge e_bot_right = e_top_bot->get_next_left_edge();
	DDLSEdge e_right_top = e_bot_right->get_next_left_edge();

	DDLSVertex v_bot = e_bot_top->get_origin_vertex();
	DDLSVertex v_top = e_top_bot->get_origin_vertex();
	DDLSVertex v_left = e_left_bot->get_origin_vertex();
	DDLSVertex v_right = e_right_top->get_origin_vertex();

	DDLSFace f_left;
	DDLSFace f_right;
	DDLSFace f_bot;
	DDLSFace f_top;

	// add the new edges
	edges.push_back(e_left_right);
	edges.push_back(e_right_left);

	// add the new faces
	faces.push_back(f_top);
	faces.push_back(f_bot);

	// set_vertex, edge and face references for the new LEFT_RIGHT and RIGHT-LEFT edges
	e_left_right->set_datas(v_left, e_right_left, e_right_top, f_top, p_edge->if_is_real(), p_edge->if_is_constrained());
	e_right_left->set_datas(v_right, e_left_right, e_left_bot, f_bot, p_edge->if_is_real(), p_edge->if_is_constrained());

	// set_edge references for the new TOP and BOTTOM faces
	f_top->set_datas(e_left_right);
	f_bot->set_datas(e_right_left);

	// check the edge references of TOP and BOTTOM vertices
	if (v_top->get_edge() == e_top_bot) {
		v_top->set_datas(e_top_left);
	}
	if (v_bot->get_edge() == e_bot_top) {
		v_bot->set_datas(e_bot_right);
	}
	// set_the new edge and face references for the 4 bouding edges
	e_top_left->get_next_left_edge() = e_left_right;
	e_top_left->set_left_face(f_top);
	e_left_bot->get_next_left_edge() = e_bot_right;
	e_left_bot->set_left_face(f_bot);
	e_bot_right->get_next_left_edge() = e_right_left;
	e_bot_right->set_left_face(f_bot);
	e_right_top->get_next_left_edge() = e_top_left;
	e_right_top->set_left_face(f_top);

	// remove the old TOP-BOTTOM and BOTTOM-TOP edges
	edges.erase(e_bot_top);
	edges.erase(e_top_bot);

	// remove the old LEFT and RIGHT faces
	faces.erase(f_left);
	faces.erase(f_right);

	return e_right_left;
}

DDLSVertex DDLS_Mesh::split_edge(DDLSEdge p_edge, const Point2 &p_pos) {
	_edges_to_check.clear(); // empty old references

	// retrieve useful objets
	DDLSEdge e_left_right = p_edge;
	DDLSEdge e_right_left = e_left_right->get_opposite_edge();
	DDLSEdge e_right_top = e_left_right->get_next_left_edge();
	DDLSEdge e_top_left = e_right_top->get_next_left_edge();
	DDLSEdge e_left_bot = e_right_left->get_next_left_edge();
	DDLSEdge e_bot_right = e_left_bot->get_next_left_edge();

	DDLSVertex v_top = e_top_left->get_origin_vertex();
	DDLSVertex v_left = e_left_right->get_origin_vertex();
	DDLSVertex v_bot = e_bot_right->get_origin_vertex();
	DDLSVertex v_right = e_right_left->get_origin_vertex();

	DDLSFace f_top = e_left_right->get_left_face();
	DDLSFace f_bot = e_right_left->get_left_face();

	// check distance from the position to edge end points
	if ((v_left->get_pos() - p_pos).length_squared() <= DDLS::EPSILON_SQUARED) {
		return v_left;
	}
	if ((v_right->get_pos() - p_pos).length_squared() <= DDLS::EPSILON_SQUARED) {
		return v_right;
	}
	// create new objects
	DDLSVertex v_center;

	DDLSEdge e_top_center;
	DDLSEdge e_center_top;
	DDLSEdge e_bot_center;
	DDLSEdge e_center_bot;

	DDLSEdge e_left_center;
	DDLSEdge e_center_left;
	DDLSEdge e_right_center;
	DDLSEdge e_center_right;

	DDLSFace f_top_left;
	DDLSFace f_bot_left;
	DDLSFace f_bot_right;
	DDLSFace f_top_right;

	// add the new vertex
	vertices.push_back(v_center);

	// add the new edges
	edges.push_back(e_center_top);
	edges.push_back(e_top_center);
	edges.push_back(e_center_left);
	edges.push_back(e_left_center);
	edges.push_back(e_center_bot);
	edges.push_back(e_bot_center);
	edges.push_back(e_center_right);
	edges.push_back(e_right_center);

	// add the new faces
	faces.push_back(f_top_right);
	faces.push_back(f_bot_right);
	faces.push_back(f_bot_left);
	faces.push_back(f_top_left);

	// set_pos and edge reference for the new CENTER vertex
	v_center->set_datas(f_top->if_is_real() ? e_center_top : e_center_bot);
	v_center->set_pos(DDLSGeom2D::project_orthogonaly(p_pos, e_left_right));

	// set_the new vertex, edge and face references for the new 8 center crossing edges
	e_center_top->set_datas(v_center, e_top_center, e_top_left, f_top_left, f_top->if_is_real());
	e_top_center->set_datas(v_top, e_center_top, e_center_right, f_top_right, f_top->if_is_real());
	e_center_left->set_datas(v_center, e_left_center, e_left_bot, f_bot_left, p_edge->if_is_real(), p_edge->if_is_constrained());
	e_left_center->set_datas(v_left, e_center_left, e_center_top, f_top_left, p_edge->if_is_real(), p_edge->if_is_constrained());
	e_center_bot->set_datas(v_center, e_bot_center, e_bot_right, f_bot_right, f_bot->if_is_real());
	e_bot_center->set_datas(v_bot, e_center_bot, e_center_left, f_bot_left, f_bot->if_is_real());
	e_center_right->set_datas(v_center, e_right_center, e_right_top, f_top_right, p_edge->if_is_real(), p_edge->if_is_constrained());
	e_right_center->set_datas(v_right, e_center_right, e_center_bot, f_bot_right, p_edge->if_is_real(), p_edge->if_is_constrained());

	// set_the new edge references for the new 4 faces
	f_top_left->set_datas(e_center_top, f_top->if_is_real());
	f_bot_left->set_datas(e_center_left, f_bot->if_is_real());
	f_bot_right->set_datas(e_center_bot, f_bot->if_is_real());
	f_top_right->set_datas(e_center_right, f_top->if_is_real());

	// check the edge references of LEFT and RIGHT vertices
	if (v_left->get_edge() == e_left_right) {
		v_left->set_datas(e_left_center);
	}
	if (v_right->get_edge() == e_right_left) {
		v_right->set_datas(e_right_center);
	}

	// set_the new edge and face references for the 4 bounding edges
	e_top_left->set_next_left_edge(e_left_center);
	e_top_left->set_left_face(f_top_left);
	e_left_bot->set_next_left_edge(e_bot_center);
	e_left_bot->set_left_face(f_bot_left);
	e_bot_right->set_next_left_edge(e_right_center);
	e_bot_right->set_left_face(f_bot_right);
	e_right_top->set_next_left_edge(e_top_center);
	e_right_top->set_left_face(f_top_right);

	// if the edge was constrained, we must:
	// - add the segments the edge is from to the 2 new
	// - update the segments the edge is from by deleting the old edge and inserting the 2 new
	// - add the segments the edge is from to the new vertex
	if (e_left_right->if_is_constrained()) {
		Vector<DDLSConstraintSegment> from_segments = e_left_right->get_from_constraint_segments();
		e_left_center->set_from_constraint_segments(from_segments);
		e_center_left->set_from_constraint_segments(e_left_center->get_from_constraint_segments());
		e_center_right->set_from_constraint_segments(from_segments);
		e_right_center->set_from_constraint_segments(e_center_right->get_from_constraint_segments());

		Vector<DDLSConstraintSegment> segments = e_left_right->get_from_constraint_segments();
		for (int i = 0; i < segments.size(); i++) {
			Vector<DDLSEdge> edges = segments[i]->get_edges();
			const int index = edges.find(e_left_right);
			if (index != -1) {
				edges.splice(index, 1, make_vector(e_left_center, e_center_right));
			} else {
				edges.splice(edges.find(e_right_left), 1, make_vector(e_right_center, e_center_left));
			}
		}

		v_center->set_from_constraint_segments(from_segments);
	}

	// remove the old LEFT-RIGHT and RIGHT-LEFT edges
	edges.erase(e_left_right);
	edges.erase(e_right_left);

	// remove the old TOP and BOTTOM faces
	faces.erase(f_top);
	faces.erase(f_bot);

	// add new bounds references for Delaunay restoring
	_center_vertex = v_center;
	_edges_to_check.push_back(e_top_left);
	_edges_to_check.push_back(e_left_bot);
	_edges_to_check.push_back(e_bot_right);
	_edges_to_check.push_back(e_right_top);

	return v_center;
}

DDLSVertex DDLS_Mesh::split_face(DDLSFace p_face, const Point2 &p_pos) {
	_edges_to_check.clear(); // empty old references

	// retrieve useful objects
	DDLSEdge e_top_left = p_face->get_edge();
	DDLSEdge e_left_right = e_top_left->get_next_left_edge();
	DDLSEdge e_right_top = e_left_right->get_next_left_edge();

	DDLSVertex v_top = e_top_left->get_origin_vertex();
	DDLSVertex v_left = e_left_right->get_origin_vertex();
	DDLSVertex v_right = e_right_top->get_origin_vertex();

	// create new objects
	DDLSVertex v_center;

	DDLSEdge e_top_center;
	DDLSEdge e_center_top;
	DDLSEdge e_left_center;
	DDLSEdge e_center_left;
	DDLSEdge e_right_center;
	DDLSEdge e_center_right;

	DDLSFace f_top_left;
	DDLSFace f_bot;
	DDLSFace f_top_right;

	// add the new vertex
	vertices.push_back(v_center);

	// add the new edges
	edges.push_back(e_top_center);
	edges.push_back(e_center_top);
	edges.push_back(e_left_center);
	edges.push_back(e_center_left);
	edges.push_back(e_right_center);
	edges.push_back(e_center_right);

	// add the new faces
	faces.push_back(f_top_left);
	faces.push_back(f_bot);
	faces.push_back(f_top_right);

	// set_pos and edge reference for the new CENTER vertex
	v_center->set_datas(e_center_top);
	v_center->set_pos(p_pos);

	// set_the new vertex, edge and face references for the new 6 center crossing edges
	e_top_center->set_datas(v_top, e_center_top, e_center_right, f_top_right);
	e_center_top->set_datas(v_center, e_top_center, e_top_left, f_top_left);
	e_left_center->set_datas(v_left, e_center_left, e_center_top, f_top_left);
	e_center_left->set_datas(v_center, e_left_center, e_left_right, f_bot);
	e_right_center->set_datas(v_right, e_center_right, e_center_left, f_bot);
	e_center_right->set_datas(v_center, e_right_center, e_right_top, f_top_right);

	// set_the new edge references for the new 3 faces
	f_top_left->set_datas(e_center_top);
	f_bot->set_datas(e_center_left);
	f_top_right->set_datas(e_center_right);

	// set_the new edge and face references for the 3 bounding edges
	e_top_left->set_next_left_edge(e_left_center);
	e_top_left->set_left_face(f_top_left);
	e_left_right->set_next_left_edge(e_right_center);
	e_left_right->set_left_face(f_bot);
	e_right_top->set_next_left_edge(e_top_center);
	e_right_top->set_left_face(f_top_right);

	// we remove the old face
	faces.erase(p_face);

	// add new bounds references for Delaunay restoring
	_center_vertex = v_center;
	_edges_to_check.push_back(e_top_left);
	_edges_to_check.push_back(e_left_right);
	_edges_to_check.push_back(e_right_top);

	return v_center;
}

void DDLS_Mesh::restore_as_delaunay() {
	while (_edges_to_check.size()) {
		DDLSEdge edge = _edges_to_check.shift();
		if (edge->if_is_real() && !edge->if_is_constrained() && !DDLSGeom2D::is_delaunay(edge)) {
			if (edge->get_next_left_edge()->get_destination_vertex() == _center_vertex) {
				_edges_to_check.push_back(edge->get_next_right_edge());
				_edges_to_check.push_back(edge->get_prev_right_edge());
			} else {
				_edges_to_check.push_back(edge->get_next_left_edge());
				_edges_to_check.push_back(edge->get_prev_left_edge());
			}
			flip_edge(edge);
		}
	}
}

// Delete a vertex IF POSSIBLE and then fill the hole with a new triangulation.
// A vertex can be deleted if:
// - it is free of constraint segment (no adjacency to any constrained edge)
// - it is adjacent to exactly 2 contrained edges and is not an end point of any constraint segment
bool DDLS_Mesh::delete_vertex(DDLSVertex p_vertex) {
	// printv_verbose("tryToDeleteVertex id", p_vertex->get_id());
	const bool free_of_constraint = p_vertex->get_from_constraint_segments().size() == 0;
	IteratorFromVertexToOutgoingEdges iter_edges;
	iter_edges.set_from_vertex(p_vertex);
	iter_edges.set_real_edges_only(false);
	Vector<DDLSEdge> outgoing_edges;

	// printv_verbose("  -> free_of_constraint", free_of_constraint);

	Vector<DDLSEdge> bound;
	// moved _a, _b vars out of if cond.
	Vector<DDLSEdge> bound_a, bound_b;
	bool real_a, real_b;

	if (free_of_constraint) {
		while (DDLSEdge edge = iter_edges.next()) {
			outgoing_edges.push_back(edge);
			bound.push_back(edge->get_next_left_edge());
		}
	} else {
		// we check if the vertex is an end point of a constraint segment
		Vector<DDLSConstraintSegment> segments = p_vertex->get_from_constraint_segments();
		for (int i = 0; i < segments.size(); i++) {
			Vector<DDLSEdge> edges = segments[i]->get_edges();
			if (edges[0]->get_origin_vertex() == p_vertex || edges[edges.size() - 1]->get_destination_vertex() == p_vertex) {
				// print_verbose("  -> is end point of a constraint segment");
				return false;
			}
		}

		// we check the count of adjacent constrained edges
		int count = 0;
		while (DDLSEdge edge = iter_edges.next()) {
			outgoing_edges.push_back(edge);

			if (edge->if_is_constrained()) {
				count++;
				if (count > 2) {
					// printv_verbose("  -> count of adjacent constrained edges", count);
					return false;
				}
			}
		}

		// if not disqualified, then we can process
		// print_verbose("process vertex deletion");
		DDLSEdge constrained_edge_a, constrained_edge_b;
		DDLSEdge edge_a, edge_b;
		edges.push_back(edge_a);
		edges.push_back(edge_b);
		for (int i = 0; i < outgoing_edges.size(); i++) {
			DDLSEdge edge = outgoing_edges[i];
			if (edge->if_is_constrained()) {
				if (!constrained_edge_a) {
					edge_b->set_datas(edge->get_destination_vertex(), edge_a, nullptr, nullptr, true, true);
					bound_a.push_back(edge_a, edge->get_next_left_edge());
					bound_b.push_back(edge_b);
					constrained_edge_a = edge;
				} else if (!constrained_edge_b) {
					edge_a->set_datas(edge->get_destination_vertex(), edge_b, nullptr, nullptr, true, true);
					bound_b.push_back(edge->get_next_left_edge());
					constrained_edge_b = edge;
				}
			} else {
				if (!constrained_edge_a) {
					bound_b.push_back(edge->get_next_left_edge());
				} else if (!constrained_edge_b) {
					bound_a.push_back(edge->get_next_left_edge());
				} else {
					bound_b.push_back(edge->get_next_left_edge());
				}
			}
		}

		// keep infos about reality
		real_a = constrained_edge_a->get_left_face()->if_is_real();
		real_b = constrained_edge_b->get_left_face()->if_is_real();

		// we update the segments infos
		edge_a->set_from_constraint_segments(constrained_edge_a->get_from_constraint_segments());
		edge_b->set_from_constraint_segments(edge_a->get_from_constraint_segments());
		Vector<DDLSConstraintSegment> constraint_segments = p_vertex->get_from_constraint_segments();
		for (int i = 0; i < constraint_segments.size(); i++) {
			Vector<DDLSEdge> edges = constraint_segments[i]->get_edges();
			const int index = edges.find(constrained_edge_a);
			if (index != -1) {
				edges.splice(index - 1, 2, make_vector(edge_a));
			} else {
				edges.splice(edges.find(constrained_edge_b) - 1, 2, make_vector(edge_b));
			}
		}
	}

	// Deletion of old faces and edges
	for (int i = 0; i < outgoing_edges.size(); i++) {
		DDLSEdge edge = outgoing_edges[i];
		faces.erase(edge->get_left_face());
		edge->get_destination_vertex()->set_edge(edge->get_next_left_edge());
		edges.erase(edge->get_opposite_edge());
		edges.erase(edge);
	}

	vertices.erase(p_vertex);

	// finally we triangulate
	if (free_of_constraint) {
		// print_verbose("trigger single hole triangulation");
		triangulate(bound, true);
	} else {
		// print_verbose("trigger dual holes triangulation");
		triangulate(bound_a, real_a);
		triangulate(bound_b, real_b);
	}
#ifdef DEBUG_ENABLED
	check();
#endif
	return true;
}

/// PRIVATE

// untriangulate is usually used while a new edge insertion in order to delete the intersected edges
// edges_list is a list of chained edges oriented from right to left
void DDLS_Mesh::untriangulate(Vector<DDLSEdge> p_edges_list) {
	// we clean useless faces and adjacent vertices
	Map<DDLSVertex, bool> vertices_cleaned;
	for (int i = 0; i < p_edges_list.size(); i++) {
		DDLSEdge curr_edge = p_edges_list[i];
		//
		if (!vertices_cleaned[curr_edge->get_origin_vertex()]) {
			curr_edge->get_origin_vertex()->set_edge(curr_edge->get_prev_left_edge()->get_opposite_edge());
			vertices_cleaned[curr_edge->get_origin_vertex()] = true;
		}
		if (!vertices_cleaned[curr_edge->get_destination_vertex()]) {
			curr_edge->get_destination_vertex()->set_edge(curr_edge->get_next_left_edge());
			vertices_cleaned[curr_edge->get_destination_vertex()] = true;
		}
		//
		faces.erase(curr_edge->get_left_face());
		if (i == p_edges_list.size() - 1) {
			faces.erase(curr_edge->get_right_face());
		}
	}

	// finally we delete the intersected edges
	for (int i = 0; i < p_edges_list.size(); i++) {
		DDLSEdge curr_edge = p_edges_list[i];
		edges.erase(curr_edge->get_opposite_edge());
		edges.erase(curr_edge);
	}
}

// triangulate is usually used to fill the hole after deletion of a vertex from mesh or after untriangulation
// - bounds is the list of edges in CCW bounding the surface to retriangulate,
void DDLS_Mesh::triangulate(Vector<DDLSEdge> p_bound, bool p_is_real) {
	if (p_bound.size() < 2) {
		printv_verbose("BREAK ! the hole has less than 2 edges");
		return;
	} else if (p_bound.size() == 2) { // if the hole is a 2 edges polygon, we have a big problem
		// throw new Error("BREAK ! the hole has only 2 edges! " + "  - edge0: " + p_bound[0]->get_origin_vertex()->get_id() + " -> " + p_bound[0]->get_destination_vertex()->get_id() + "  - edge1: " +  p_bound[1]->get_origin_vertex()->get_id() + " -> " + p_bound[1]->get_destination_vertex()->get_id());
		printv_verbose("BREAK ! the hole has only 2 edges");
		printv_verbose("  - edge0:", p_bound[0]->get_origin_vertex()->get_id(), "->", p_bound[0]->get_destination_vertex()->get_id());
		printv_verbose("  - edge1:", p_bound[1]->get_origin_vertex()->get_id(), "->", p_bound[1]->get_destination_vertex()->get_id());
		return;
	} else if (p_bound.size() == 3) { // if the hole is a 3 edges polygon:
		// printv_verbose("the hole is a 3 edges polygon");
		// printv_verbose("  - edge0:", p_bound[0]->get_origin_vertex()->get_id(), "->", p_bound[0]->get_destination_vertex()->get_id());
		// printv_verbose("  - edge1:", p_bound[1]->get_origin_vertex()->get_id(), "->", p_bound[1]->get_destination_vertex()->get_id());
		// printv_verbose("  - edge2:", p_bound[2]->get_origin_vertex()->get_id(), "->", p_bound[2]->get_destination_vertex()->get_id());
		DDLSFace f;
		f.instance();
		f->set_datas(p_bound[0], p_is_real);
		faces.push_back(f);
		p_bound.write[0]->set_left_face(f);
		p_bound.write[1]->set_left_face(f);
		p_bound.write[2]->set_left_face(f);
		p_bound.write[0]->set_next_left_edge(p_bound[1]);
		p_bound.write[1]->set_next_left_edge(p_bound[2]);
		p_bound.write[2]->set_next_left_edge(p_bound[0]);
	} else { // if more than 3 edges, we process recursively:
		DDLSEdge base_edge = p_bound[0];
		DDLSVertex vertex_a = base_edge->get_origin_vertex();
		DDLSVertex vertex_b = base_edge->get_destination_vertex();
		DDLSVertex vertex_c;
		DDLSVertex vertex_check;
		Point2 circumcenter;
		bool is_delaunay = false;
		int index = 0;
		// printv_verbose("the hole has", p_bound.size(), "edges");
		// for (int i = 0; i < p_bound.size(); i++) {
		//   printv_verbose("  - edge", i, ":", p_bound[i]->get_origin_vertex()->get_id(), "->", p_bound[i]->get_destination_vertex()->get_id());
		// }
		for (int i = 2; i < p_bound.size(); i++) {
			vertex_c = p_bound[i]->get_origin_vertex();
			if (DDLSGeom2D::get_relative_position2(vertex_c->get_pos(), base_edge) == 1) {
				index = i;
				is_delaunay = true;
				circumcenter = DDLSGeom2D::get_circumcenter(vertex_a->get_pos(), vertex_b->get_pos(), vertex_c->get_pos());
				// for perfect regular n-sides polygons, checking strict delaunay circumcircle condition is not possible, so we substract EPSILON to circumcircle radius:
				const real_t radius_squared = (vertex_a->get_pos() - circumcenter).length_squared() - DDLS::EPSILON_SQUARED;
				for (int j = 2; j < p_bound.size(); j++) {
					if (j != i) {
						vertex_check = p_bound[j]->get_origin_vertex();
						const real_t distance_squared = (vertex_check->get_pos() - circumcenter).length_squared();
						if (distance_squared < radius_squared) {
							is_delaunay = false;
							break;
						}
					}
				}
				if (is_delaunay) {
					break;
				}
			}
		}

		if (!is_delaunay) {
			// for perfect regular n-sides polygons, checking delaunay circumcircle condition is not possible
			printv_verbose("NO DELAUNAY FOUND");

			// String s = "";
			// for (int i = 0; i < p_bound.size(); i++) {
			//   s += p_bound[i]->get_origin_vertex()->get_pos().x + " , ";
			//   s += p_bound[i]->get_origin_vertex()->get_pos().y + " , ";
			//   s += p_bound[i]->get_destination_vertex()->get_pos().x + " , ";
			//   s += p_bound[i]->get_destination_vertex()->get_pos().y + " , ";
			// }
			// printv_verbose(s);

			index = 2;
		}

		// printv_verbose("index", index, "on", p_bound.siz());

		DDLSEdge edge_a;
		edge_a.instance();
		DDLSEdge edge_a_opp;
		edge_a_opp.instance();
		DDLSEdge edge_b;
		edge_b.instance();
		DDLSEdge edge_b_opp;
		edge_b_opp.instance();

		Vector<DDLSEdge> bound_a, bound_m, bound_b;

		if (index < p_bound.size() - 1) {
			edges.push_back(edge_a, edge_a_opp);
			edge_a->set_datas(vertex_a, edge_a_opp, nullptr, nullptr, p_is_real, false);
			edge_a_opp->set_datas(p_bound[index]->get_origin_vertex(), edge_a, nullptr, nullptr, p_is_real, false);
			bound_a = p_bound.slice(index);
			bound_a.push_back(edge_a);
			triangulate(bound_a, p_is_real);
		}

		if (index > 2) {
			edges.push_back(edge_b, edge_b_opp);
			edge_b->set_datas(p_bound[1]->get_origin_vertex(), edge_b_opp, nullptr, nullptr, p_is_real, false);
			edge_b_opp->set_datas(p_bound[index]->get_origin_vertex(), edge_b, nullptr, nullptr, p_is_real, false);
			bound_b = p_bound.slice(1, index);
			bound_b.push_back(edge_b_opp);
			triangulate(bound_b, p_is_real);
		}

		if (index == 2) {
			bound_m.push_back(base_edge, p_bound[1], edge_a_opp);
		} else if (index == p_bound.size() - 1) {
			bound_m.push_back(base_edge, edge_b, p_bound[index]);
		} else {
			bound_m.push_back(base_edge, edge_b, edge_a_opp);
		}
		triangulate(bound_m, p_is_real);
	}
}

void DDLS_Mesh::check() const {
	for (int i = 0; i < edges.size(); i++) {
		if (!edges[i]->get_next_left_edge()) {
			WARN_PRINT("(" + itos(i) + ") missing: next_left_edge");
			return;
		}
	}
	printv_verbose("check OK");
}

void DDLS_Mesh::debug() const {
	for (int i = 0; i < vertices.size(); i++) {
		printv_verbose("-- vertex", vertices[i]->get_id());
		printv_verbose("  edge", vertices[i]->get_edge()->get_id(), " - ", vertices[i]->get_edge());
		printv_verbose("  edge is_real:", vertices[i]->get_edge()->if_is_real());
	}
	for (int i = 0; i < edges.size(); i++) {
		printv_verbose("-- edge", edges[i]);
		printv_verbose("  is_real", edges[i]->get_id(), " - ", edges[i]->if_is_real());
		printv_verbose("  next_left_edge", edges[i]->get_next_left_edge());
		printv_verbose("  opposite_edge", edges[i]->get_opposite_edge());
	}
}

DDLS_Mesh::DDLS_Mesh() :
		id(MESH_COUNTER++) {
	clipping = true;
	_objects_update_in_progress = false;
}

/// DDLS_GraphEdge

DDLSGraphEdge DDLS_GraphEdge::get_prev() const { return prev; }
void DDLS_GraphEdge::set_prev(DDLSGraphEdge p_value) { prev = p_value; }
DDLSGraphEdge DDLS_GraphEdge::get_next() const { return next; }
void DDLS_GraphEdge::set_next(DDLSGraphEdge p_value) { next = p_value; }
DDLSGraphEdge DDLS_GraphEdge::get_rot_prev_edge() const { return rot_prev_edge; }
void DDLS_GraphEdge::set_rot_prev_edge(DDLSGraphEdge p_value) { rot_prev_edge = p_value; }
DDLSGraphEdge DDLS_GraphEdge::get_rot_next_edge() const { return rot_next_edge; }
void DDLS_GraphEdge::set_rot_next_edge(DDLSGraphEdge p_value) { rot_next_edge = p_value; }
DDLSGraphEdge DDLS_GraphEdge::get_opposite_edge() const { return opposite_edge; }
void DDLS_GraphEdge::set_opposite_edge(DDLSGraphEdge p_value) { opposite_edge = p_value; }
DDLSGraphNode DDLS_GraphEdge::get_source_node() const { return source_node; }
void DDLS_GraphEdge::set_source_node(DDLSGraphNode p_value) { source_node = p_value; }
DDLSGraphNode DDLS_GraphEdge::get_destination_node() const { return destination_node; }
void DDLS_GraphEdge::set_destination_node(DDLSGraphNode p_value) { destination_node = p_value; }

DDLS_GraphEdge::DDLS_GraphEdge() :
		id(GRAPH_EDGE_COUNTER++) {}

/// DDLS_GraphNode

DDLSGraphNode DDLS_GraphNode::get_prev() const { return prev; }
void DDLS_GraphNode::set_prev(DDLSGraphNode p_value) { prev = p_value; }
DDLSGraphNode DDLS_GraphNode::get_next() const { return next; }
void DDLS_GraphNode::set_next(DDLSGraphNode p_value) { next = p_value; }
DDLSGraphEdge DDLS_GraphNode::get_outgoing_edge() const { return outgoing_edge; }
void DDLS_GraphNode::set_outgoing_edge(DDLSGraphEdge p_value) { outgoing_edge = p_value; }

DDLS_GraphNode::DDLS_GraphNode() :
		id(GRAPH_NODE_COUNTER++) {}

/// DDLS_Graph

DDLSGraphEdge DDLS_Graph::get_edge() const { return edge; }
DDLSGraphNode DDLS_Graph::get_node() const { return node; }

DDLSGraphNode DDLS_Graph::insert_node() {
	DDLSGraphNode next_node;
	next_node.instance();
	if (node) {
		next_node->set_next(node);
		node->set_prev(next_node);
	}
	node = next_node;
	return next_node;
}

void DDLS_Graph::delete_node(DDLSGraphNode p_node) {
	while (p_node->get_outgoing_edge()) {
		if (p_node->get_outgoing_edge()->get_opposite_edge()) {
			delete_edge(p_node->get_outgoing_edge()->get_opposite_edge());
		}
		delete_edge(p_node->get_outgoing_edge());
	}

	DDLSGraphNode other_node = node;
	while (other_node) {
		if (DDLSGraphEdge incoming_edge = other_node->get_successor_node(p_node)) {
			delete_edge(incoming_edge);
		}
		other_node = other_node->get_next();
	}

	if (node == p_node) {
		if (node->get_next()) {
			node->get_next()->set_prev(nullptr);
			node = p_node->get_next();
		} else {
			node = nullptr;
		}
	} else {
		if (p_node->get_next()) {
			p_node->get_prev()->set_next(p_node->get_next());
			p_node->get_next()->set_prev(p_node->get_prev());
		} else {
			p_node->get_prev()->set_next(nullptr);
		}
	}
}

DDLSGraphEdge DDLS_Graph::insert_edge(DDLSGraphNode p_from_node, DDLSGraphNode p_to_node) {
	if (p_from_node->get_successor_node(p_to_node)) {
		return nullptr;
	}
	DDLSGraphEdge next_edge;
	next_edge.instance();
	if (edge) {
		edge->set_prev(next_edge);
		next_edge->set_next(edge);
	}
	edge = next_edge;

	next_edge->set_source_node(p_from_node);
	next_edge->set_destination_node(p_to_node);
	p_from_node->set_successor_node(p_to_node, next_edge);
	if (p_from_node->get_outgoing_edge()) {
		p_from_node->get_outgoing_edge()->set_rot_prev_edge(next_edge);
		next_edge->set_rot_next_edge(p_from_node->get_outgoing_edge());
		p_from_node->set_outgoing_edge(next_edge);
	} else {
		p_from_node->set_outgoing_edge(next_edge);
	}

	if (DDLSGraphEdge opposite_edge = p_to_node->get_successor_node(p_from_node)) {
		next_edge->set_opposite_edge(opposite_edge);
		opposite_edge->set_opposite_edge(next_edge);
	}

	return next_edge;
}

void DDLS_Graph::delete_edge(DDLSGraphEdge p_edge) {
	if (edge == p_edge) {
		if (p_edge->get_next()) {
			p_edge->get_next()->set_prev(nullptr);
			edge = p_edge->get_next();
		} else {
			edge = nullptr;
		}
	} else {
		if (p_edge->get_next()) {
			p_edge->get_prev()->set_next(p_edge->get_next());
			p_edge->get_next()->set_prev(p_edge->get_prev());
		} else {
			p_edge->get_prev()->set_next(nullptr);
		}
	}

	if (p_edge->get_source_node()->get_outgoing_edge() == p_edge) {
		if (p_edge->get_rot_next_edge()) {
			p_edge->get_rot_next_edge()->set_rot_prev_edge(nullptr);
			p_edge->get_source_node()->set_outgoing_edge(p_edge->get_rot_next_edge());
		} else {
			p_edge->get_source_node()->set_outgoing_edge(nullptr);
		}
	} else {
		if (p_edge->get_rot_next_edge()) {
			p_edge->get_rot_prev_edge()->set_rot_next_edge(p_edge->get_rot_next_edge());
			p_edge->get_rot_next_edge()->set_rot_prev_edge(p_edge->get_rot_prev_edge());
		} else {
			p_edge->get_rot_prev_edge()->set_rot_next_edge(nullptr);
		}
	}
}

DDLS_Graph::~DDLS_Graph() {
	while (node) {
		delete_node(node);
	}
}

DDLS_Graph::DDLS_Graph() :
		id(GRAPH_COUNTER++) {}
