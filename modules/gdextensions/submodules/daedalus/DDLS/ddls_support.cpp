/**************************************************************************/
/*  ddls_support.cpp                                                      */
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
#include "data/ddls_mesh.h"
#include "data/math/ddls_geom2d.h"
#include "data/math/ddls_potrace.h"
#include "data/math/ddls_rand_generator.h"
#include "factories/ddls_rect_mesh_factory.h"
#include "iterators/iterator_from_face_to_inner_edges.h"
#include "iterators/iterator_from_mesh_to_vertices.h"
#include "iterators/iterator_from_vertex_to_holding_faces.h"
#include "iterators/iterator_from_vertex_to_incoming_edges.h"
#include "views/ddls_simple_view.h"

#include "core/error_macros.h"
#include "core/image.h"
#include "core/map.h"
#include "core/math/vector2.h"
#include "core/vector.h"
#include "scene/2d/canvas_item.h"
#include "scene/resources/font.h"

namespace DDLSPotrace {
struct Edge_Data : public Reference {
	real_t sum_distances_squared;
	real_t length;
	int nodes_count;
};
typedef Ref<Edge_Data> EdgeData;

struct Node_Data : public Reference {
	int index;
	Point2 point;
};
typedef Ref<Node_Data> NodeData;

const real_t MaxDistance = 1;

#define _T(X, Y) itos(X) + "_" + itos(Y)

Vector<Vector<Point2>> build_shapes(Ref<Image> p_bmp_data, Ref<Image> p_debug_bmp, CanvasItem *p_debug_shape) {
	ERR_FAIL_NULL_V(p_bmp_data, Vector<Vector<Point2>>());
	// OUTLINES STEP-LIKE SHAPES GENERATION
	Vector<Vector<Point2>> shapes;
	Map<String, bool> dict_pixels_done;
	for (int row = 1; row < p_bmp_data->get_height() - 1; row++) {
		for (int col = 0; col < p_bmp_data->get_width() - 1; col++) {
			if (p_bmp_data->_get_pixel32(col, row) == 0xFFFFFF && p_bmp_data->_get_pixel32(col + 1, row) < 0xFFFFFF) {
				if (!dict_pixels_done[_T(col + 1, row)]) {
					shapes.push_back(build_shape(p_bmp_data, row, col + 1, dict_pixels_done, p_debug_bmp, p_debug_shape));
				}
			}
		}
	}
	return shapes;
}

Vector<Point2> build_shape(Ref<Image> p_bmp_data, int p_from_pixel_row, int p_from_pixel_col, Map<String, bool> &p_dict_pixels_done, Ref<Image> p_debug_bmp, CanvasItem *p_debug_shape) {
	Vector<Point2> path;
	int new_x = p_from_pixel_col;
	int new_y = p_from_pixel_row;
	path.push_back(Point2(new_x, new_y));
	p_dict_pixels_done[_T(new_x, new_y)] = true;

	Point2 cur_dir(0, 1), new_dir;
	int new_pixel_row, new_pixel_col;
	int count = -1;
	while (true) {
		if (p_debug_bmp) {
			p_debug_bmp->_set_pixel32(p_from_pixel_col, p_from_pixel_row, 0xFFFF0000);
		}

		// take the pixel at right
		new_pixel_row = p_from_pixel_row + cur_dir.x + cur_dir.y;
		new_pixel_col = p_from_pixel_col + cur_dir.x - cur_dir.y;
		// if the pixel is not white
		if (p_bmp_data->_get_pixel32(new_pixel_col, new_pixel_row) < 0xFFFFFF) {
			// turn the direction right
			new_dir.x = -cur_dir.y;
			new_dir.y = cur_dir.x;
		} else { // if the pixel is white
			// take the pixel straight
			new_pixel_row = p_from_pixel_row + cur_dir.y;
			new_pixel_col = p_from_pixel_col + cur_dir.x;
			// if the pixel is not white
			if (p_bmp_data->_get_pixel32(new_pixel_col, new_pixel_row) < 0xFFFFFF) {
				// the direction stays the same
				new_dir.x = cur_dir.x;
				new_dir.y = cur_dir.y;
			} else { // if the pixel is white
				// pixel stays the same
				new_pixel_row = p_from_pixel_row;
				new_pixel_col = p_from_pixel_col;
				// turn the direction left
				new_dir.x = cur_dir.y;
				new_dir.y = -cur_dir.x;
			}
		}
		new_x = new_x + cur_dir.x;
		new_y = new_y + cur_dir.y;

		if (new_x == path[0].x && new_y == path[0].y) {
			break;
		} else {
			path.push_back(Point2(new_x, new_y));
			p_dict_pixels_done[_T(new_x, new_y)] = true;
			p_from_pixel_row = new_pixel_row;
			p_from_pixel_col = new_pixel_col;
			cur_dir.x = new_dir.x;
			cur_dir.y = new_dir.y;
		}

		if (--count == 0) {
			break;
		}
	}

	if (p_debug_shape && !path.empty()) {
		for (int i = 1; i < path.size(); i++) {
			p_debug_shape->draw_line(path[i - 1], path[i], Color(0, 1, 0));
		}
		p_debug_shape->draw_line(path[path.size() - 1], path[0], Color(0, 1, 0));
	}

	return path;
}

DDLSGraph build_graph(Vector<Point2> p_shape) {
	DDLSGraph graph;
	graph.instance();
	DDLSGraphNode node;
	for (int i = 0; i < p_shape.size(); i++) {
		node = graph->insert_node();
		NodeData node_data;
		node_data.instance();
		node_data->index = i;
		node_data->point = p_shape[i];
		node->set_data(node_data);
	}

	DDLSGraphNode node1 = graph->get_node();
	while (node1) {
		DDLSGraphNode node2 = node1->get_next() ? node1->get_next() : graph->get_node();
		while (node2 != node1) {
			bool is_valid = true;
			DDLSGraphNode sub_node = node1->get_next() ? node1->get_next() : graph->get_node();
			int count = 2;
			real_t sum_dist_sqrd = 0;
			while (sub_node != node2) {
				const real_t dist_sqrd = MAX(0, DDLSGeom2D::distance_squared_point_to_segment(NodeData(sub_node->get_data())->point, NodeData(node1->get_data())->point, NodeData(node2->get_data())->point));
				if (dist_sqrd >= MaxDistance) {
					is_valid = false; // sub_node not valid
					break;
				}
				count++;
				sum_dist_sqrd += dist_sqrd;
				sub_node = sub_node->get_next() ? sub_node->get_next() : graph->get_node();
			}

			if (!is_valid) {
				break; //segment not valid
			}

			DDLSGraphEdge edge = graph->insert_edge(node1, node2);
			EdgeData edge_data;
			edge_data.instance();
			edge_data->sum_distances_squared = sum_dist_sqrd;
			edge_data->length = NodeData(node1->get_data())->point.distance_to(NodeData(node2->get_data())->point);
			edge_data->nodes_count = count;
			edge->set_data(edge_data);

			node2 = node2->get_next() ? node2->get_next() : graph->get_node();
		}
		node1 = node1->get_next();
	}

	return graph;
}

Vector<Point2> build_polygon(DDLSGraph p_graph, CanvasItem *p_debug_shape) {
	Vector<Point2> polygon;

	int min_node_index = DDLS::MAX_VALUE;
	DDLSGraphEdge edge;
	real_t score, higher_score;
	DDLSGraphEdge lower_score_edge;
	DDLSGraphNode curr_node = p_graph->get_node();
	while (NodeData(curr_node->get_data())->index < min_node_index) {
		min_node_index = NodeData(curr_node->get_data())->index;

		polygon.push_back(NodeData(curr_node->get_data())->point);

		higher_score = DDLS::MIN_VALUE;
		edge = curr_node->get_outgoing_edge();
		while (edge) {
			score = EdgeData(edge->get_data())->nodes_count - EdgeData(edge->get_data())->length * Math::sqrt(EdgeData(edge->get_data())->sum_distances_squared / (EdgeData(edge->get_data())->nodes_count));
			if (score > higher_score) {
				higher_score = score;
				lower_score_edge = edge;
			}

			edge = edge->get_rot_next_edge();
		}

		curr_node = lower_score_edge->get_destination_node();
	}

	if (DDLSGeom2D::get_direction(polygon[polygon.size() - 1], polygon[0], polygon[1]) == 0) {
		(void)polygon.shift();
	}

	if (p_debug_shape && !polygon.empty()) {
		for (int i = 1; i < polygon.size(); i++) {
			p_debug_shape->draw_line(polygon[i - 1], polygon[i], Color(0, 1, 0));
		}
		p_debug_shape->draw_line(polygon[polygon.size() - 1], polygon[0], Color(0, 1, 0));
	}

	return polygon;
}
} //namespace DDLSPotrace

namespace DDLSTools {
void extract_mesh_from_bitmap(Ref<Image> p_bmp_data, Vector<Point2> &r_vertices, Vector<int> &r_triangles) {
	// Outlines step-like shapes generation
	Vector<Vector<Point2>> shapes = DDLSPotrace::build_shapes(p_bmp_data);

	// Graphs of potential segments generation
	Vector<DDLSGraph> graphs;
	for (int i = 0; i < shapes.size(); i++) {
		graphs.push_back(DDLSPotrace::build_graph(shapes[i]));
	}

	// Optimized polygons generation
	Vector<Vector<Point2>> polygons;
	for (int i = 0; i < graphs.size(); i++) {
		polygons.push_back(DDLSPotrace::build_polygon(graphs[i]));
	}

	// Mesh generation
	DDLSMesh mesh = DDLSRectMeshFactory::build_rectangle(p_bmp_data->get_width(), p_bmp_data->get_height());
	Vector<DDLSEdge> edges; // We keep track of 1 edge by shape
	for (int i = 0; i < polygons.size(); i++) {
		for (int j = 0; j < polygons[i].size() - 1; j++) {
			DDLSConstraintSegment segment = mesh->insert_constraint_segment(polygons[i][j], polygons[i][j + 1]);
			if (j == 0) {
				if (segment->get_edge(0)->get_origin_vertex()->get_pos() == polygons[i][j]) {
					edges.push_back(segment->get_edge(0));
				} else {
					edges.push_back(segment->get_edge(0)->get_opposite_edge());
				}
			}
		}
		mesh->insert_constraint_segment(polygons[i][0], polygons[i][1]);
	}

	// Final extraction
	Map<DDLSVertex, int> indices_dict;
	Vector<DDLSVertex> vertices = mesh->get_vertices();
	for (int i = 0; i < vertices.size(); i++) {
		DDLSVertex vertex = vertices[i];
		if (vertex->if_is_real() && vertex->get_pos().x > 0 && vertex->get_pos().x < p_bmp_data->get_width() && vertex->get_pos().y > 0 && vertex->get_pos().y < p_bmp_data->get_height()) {
			Point2 point = vertex->get_pos();
			r_vertices.push_back(point);
			indices_dict[vertex] = r_vertices.size() - 1;
		}
	}

	Map<DDLSFace, bool> faces_done;
	Vector<DDLSFace> open_faces_list;
	for (int i = 0; i < edges.size(); i++) {
		open_faces_list.push_back(edges[i]->get_right_face());
	}
	while (open_faces_list.size() > 0) {
		DDLSFace curr_face = open_faces_list.pop();
		if (faces_done[curr_face]) {
			continue;
		}
		r_triangles.push_back(indices_dict[curr_face->get_edge()->get_origin_vertex()]);
		r_triangles.push_back(indices_dict[curr_face->get_edge()->get_next_left_edge()->get_origin_vertex()]);
		r_triangles.push_back(indices_dict[curr_face->get_edge()->get_next_left_edge()->get_destination_vertex()]);

		if (!curr_face->get_edge()->if_is_constrained()) {
			open_faces_list.push_back(curr_face->get_edge()->get_right_face());
		}
		if (!curr_face->get_edge()->get_next_left_edge()->if_is_constrained()) {
			open_faces_list.push_back(curr_face->get_edge()->get_next_left_edge()->get_right_face());
		}
		if (!curr_face->get_edge()->get_prev_left_edge()->if_is_constrained()) {
			open_faces_list.push_back(curr_face->get_edge()->get_prev_left_edge()->get_right_face());
		}
		faces_done[curr_face] = true;
	}
}

// Simplify polyline (Ramer-Douglas-Peucker) from array of coords defining the polyline.
// Epsilon is a perpendicular distance threshold (typically in the range [1..2]).
Vector<Point2> simplify(Vector<Point2> p_coords, real_t p_epsilon) {
	const int len = p_coords.size();

	if (len <= 2 || p_epsilon < 1) {
		return p_coords;
	}

	const Point2 first_point = p_coords[0];
	const Point2 last_point = p_coords[len - 1];

	int index = -1;
	real_t dist = 0;
	for (int i = 1; i < len; i++) {
		const real_t curr_dist = DDLSGeom2D::distance_squared_point_to_segment(p_coords[i], first_point, last_point);
		if (curr_dist > dist) {
			dist = curr_dist;
			index = i;
		}
	}

	if (dist > p_epsilon * p_epsilon) {
		// recurse
		Vector<Point2> l1 = p_coords.slice(0, index + 1);
		Vector<Point2> l2 = p_coords.slice(index);
		Vector<Point2> r1 = simplify(l1, p_epsilon);
		Vector<Point2> r2 = simplify(l2, p_epsilon);
		// concat r2 to r1 minus the end/startpoint that will be the same
		return r1.slice(0, r1.size() - 1).append_array(r2);
	} else {
		return make_vector(first_point, last_point);
	}
}
} // namespace DDLSTools

namespace DDLSGeom2D {
static DDLSRandGenerator rand_gen;
static Vector<DDLSVertex> samples;
static Point2 _circumcenter;

Variant locate_position(const Point2 &p_pos, DDLSMesh p_mesh) {
	// jump and walk algorithm

	rand_gen.set_seed(p_pos.x * 10 + 4 * p_pos.y);

	samples.clear();
	int num_samples = Math::pow(p_mesh->get_vertices().size(), 1 / 3.0);
	rand_gen.set_range(0, p_mesh->get_vertices().size() - 1);
	for (int i = 0; i < num_samples; i++) {
		samples.push_back(p_mesh->get_vertex(rand_gen.next()));
	}
	real_t min_dist_squared = DDLS::MAX_VALUE;
	DDLSVertex closed_vertex;
	for (int i = 0; i < num_samples; i++) {
		DDLSVertex curr_vertex = samples[i];
		Point2 curr_vertex_pos = curr_vertex->get_pos();
		const real_t dist_squared = (curr_vertex_pos - p_pos).length_squared();
		if (dist_squared < min_dist_squared) {
			min_dist_squared = dist_squared;
			closed_vertex = curr_vertex;
		}
	}

	IteratorFromVertexToHoldingFaces iter_face = IteratorFromVertexToHoldingFaces().set_from_vertex(closed_vertex);
	DDLSFace curr_face = iter_face.next();

	Map<DDLSFace, bool> face_visited;
	Variant object_container;
	int num_iter = 0;
	while (face_visited[curr_face] || !(object_container = is_in_face(p_pos, curr_face))) {
		face_visited[curr_face] = true; // ??
		num_iter++;
		switch (num_iter) {
			case 50: {
				print_verbose("(DDLS) Walk take more than 50 loop");
			} break;
			case 1000: {
				print_verbose("(DDLS) Walk take more than 1000 loop -> we escape");
				return Variant();
			} break;
		}
		IteratorFromFaceToInnerEdges iter_edge = IteratorFromFaceToInnerEdges().set_from_face(curr_face);
		DDLSEdge curr_edge;
		int relativ_pos;
		do {
			curr_edge = iter_edge.next();
			if (curr_edge == nullptr) {
				print_verbose("(DDLS) Kill path");
				return Variant();
			}
			relativ_pos = get_relative_position(p_pos, curr_edge);
		} while (relativ_pos == 1 || relativ_pos == 0);

		curr_face = curr_edge->get_right_face();
	}

	return object_container;
}

bool is_circle_intersecting_any_constraint(const Point2 &p_pos, real_t p_radius, DDLSMesh p_mesh) {
	if (p_pos.x <= 0 || p_pos.x >= p_mesh->get_width() || p_pos.y <= 0 || p_pos.y >= p_mesh->get_height()) {
		return true;
	}
	Variant loc = DDLSGeom2D::locate_position(p_pos, p_mesh);
	DDLSFace face;
	if (DDLSVertex vertex = loc) {
		face = vertex->get_edge()->get_left_face();
	} else if (DDLSEdge edge = loc) {
		face = edge->get_left_face();
	} else {
		face = loc;
	}

	// if a vertex is in the circle, a contrainst must intersect the circle
	// because a vertex always belongs to a contrained edge
	const real_t radius_squared = p_radius * p_radius;
	Point2 pos = face->get_edge()->get_origin_vertex()->get_pos();
	real_t dist_squared = (pos - p_pos).length_squared();
	if (dist_squared <= radius_squared) {
		return true;
	}
	pos = face->get_edge()->get_next_left_edge()->get_origin_vertex()->get_pos();
	dist_squared = (pos - p_pos).length_squared();
	if (dist_squared <= radius_squared) {
		return true;
	}
	pos = face->get_edge()->get_next_left_edge()->get_next_left_edge()->get_origin_vertex()->get_pos();
	dist_squared = (pos - p_pos).length_squared();
	if (dist_squared <= radius_squared) {
		return true;
	}

	// check if edge intersects
	Vector<DDLSEdge> edges_to_check;
	edges_to_check.push_back(face->get_edge());
	edges_to_check.push_back(face->get_edge()->get_next_left_edge());
	edges_to_check.push_back(face->get_edge()->get_next_left_edge()->get_next_left_edge());

	Map<DDLSEdge, bool> checked_edges;
	while (edges_to_check.size() > 0) {
		DDLSEdge edge = edges_to_check.pop();
		checked_edges[edge] = true;
		const Point2 pos1 = edge->get_origin_vertex()->get_pos();
		const Point2 pos2 = edge->get_destination_vertex()->get_pos();
		bool intersecting = intersections_segment_circle(pos1, pos2, p_pos, p_radius);
		if (intersecting) {
			if (edge->if_is_constrained()) {
				return true;
			} else {
				edge = edge->get_opposite_edge()->get_next_left_edge();
				if (!checked_edges[edge] && !checked_edges[edge->get_opposite_edge()] && edges_to_check.find(edge) == -1 && edges_to_check.find(edge->get_opposite_edge()) == -1) {
					edges_to_check.push_back(edge);
				}
				edge = edge->get_next_left_edge();
				if (!checked_edges[edge] && !checked_edges[edge->get_opposite_edge()] && edges_to_check.find(edge) == -1 && edges_to_check.find(edge->get_opposite_edge()) == -1) {
					edges_to_check.push_back(edge);
				}
			}
		}
	}
	return false;
}

int get_direction(const Point2 &p_1, const Point2 &p_2, const Point2 &p_3) {
	// dot product with the orthogonal vector pointing left vector of e_up:
	const real_t dot = (p_3.x - p_1.x) * (p_2.y - p_1.y) + (p_3.y - p_1.y) * (-p_2.x + p_1.x);
	return (dot == 0) ? 0 : ((dot > 0) ? 1 : -1); // check sign
}

int get_direction2(const Point2 &p_1, const Point2 &p_2, const Point2 &p_3) {
	// dot product with the orthogonal vector pointing left vector of e_up:
	const real_t dot = (p_3.x - p_1.x) * (p_2.y - p_1.y) + (p_3.y - p_1.y) * (-p_2.x + p_1.x);
	// check sign more
	if (dot == 0) {
		return 0;
	} else if (dot > 0) {
		if (distance_squared_point_to_line(p_3, p_1, p_2) <= DDLS::EPSILON_SQUARED) {
			return 0;
		} else {
			return 1;
		}
	} else {
		if (distance_squared_point_to_line(p_3, p_1, p_2) <= DDLS::EPSILON_SQUARED) {
			return 0;
		} else {
			return -1;
		}
	}
}

int get_relative_position(const Point2 &p_pos, DDLSEdge p_e_up) {
	return get_direction(p_e_up->get_origin_vertex()->get_pos(), p_e_up->get_destination_vertex()->get_pos(), p_pos);

	// parametric expression of pointing up edge e_up
	//  x(t1) = origin.x + t1*(destination.x - origin.x)
	//  y(t1) = origin.y + t1*(destination.y - origin.y)
	//
	// and orthogonal edge pointing right to e_up
	//  x(t2) = origin.x + t2*(destination.y - origin.y)
	//  y(t2) = origin.y - t2*(destination.x - origin.x)
	//
	// (x, y) position can be expressed as a linear combination of the 2 previous segments
	//  x = origin.x + t2*(destination.y - origin.y) + t1*(destination.x - origin.x)
	//  y = origin.y + t1*(destination.y - origin.y) - t2*(destination.x - origin.x)
	//
	// ---> the sign of t2 will inform us if to_check lies at right or left of e_up

	// set alias letters
	//  real_t a = x;
	//  real_t b = y;
	//  real_t c = origin.pos.x;
	//  real_t d = origin.pos.y;
	//  real_t e = destination.pos.x;
	//  real_t f = destination.pos.y;

	// system to solve:
	//  a = c + t2 (f - d) + t1 (e - c)
	//  b = d + t1 (f - d) - t2 (e - c)

	// giving to wolfram:
	//   Solve[{a = c + t2 (f - d) + t1 (e - c) , b = d + t1 (f - d) - t2 (e - c)}, {t1, t2}]
	// we get:
	//   real_t t2 = (-a*d + a*f + b*c - b*e - c*f + d*e) / (c*c - 2*c*e + d*d - 2*d*f + e*e + f*f);
	//
	// int result;
	// if (t2 == 0)
	//   result = 0;
	// else if (t2 < 0)
	//   result = -1;
	// else
	//   result = 1;
	//
	// return result;
}

int get_relative_position2(const Point2 &p_pos, DDLSEdge p_e_up) {
	return get_direction2(p_e_up->get_origin_vertex()->get_pos(), p_e_up->get_destination_vertex()->get_pos(), p_pos);
}

// the function checks by priority:
// - if the (x, y) lies on a vertex of the polygon, it will return this vertex
// - if the (x, y) lies on a edge of the polygon, it will return this edge
// - if the (x, y) lies inside the polygon, it will return the polygon
// - if the (x, y) lies outside the polygon, it will return null
Variant is_in_face(const Point2 &p_pos, DDLSFace p_polygon) {
	// remember polygons are triangle only,
	// and we suppose we have not degenerated flat polygons !

	Variant result;

	DDLSEdge e1_2 = p_polygon->get_edge();
	DDLSEdge e2_3 = e1_2->get_next_left_edge();
	DDLSEdge e3_1 = e2_3->get_next_left_edge();
	if (get_relative_position(p_pos, e1_2) >= 0 && get_relative_position(p_pos, e2_3) >= 0 && get_relative_position(p_pos, e3_1) >= 0) {
		DDLSVertex v1 = e1_2->get_origin_vertex();
		DDLSVertex v2 = e2_3->get_origin_vertex();
		DDLSVertex v3 = e3_1->get_origin_vertex();

		const Point2 p1 = v1->get_pos();
		const Point2 p2 = v2->get_pos();
		const Point2 p3 = v3->get_pos();

		const real_t v_v1squared_length = (p1 - p_pos).length_squared();
		const real_t v_v2squared_length = (p1 - p_pos).length_squared();
		const real_t v_v3squared_length = (p3 - p_pos).length_squared();
		const real_t v1_v2squared_length = (p2 - p1).length_squared();
		const real_t v2_v3squared_length = (p3 - p2).length_squared();
		const real_t v3_v1squared_length = (p1 - p3).length_squared();

		const real_t dot_v_v1v2 = (p_pos.x - p1.x) * (p2.x - p1.x) + (p_pos.y - p1.y) * (p2.y - p1.y);
		const real_t dot_v_v2v3 = (p_pos.x - p2.x) * (p3.x - p2.x) + (p_pos.y - p2.y) * (p3.y - p2.y);
		const real_t dot_v_v3v1 = (p_pos.x - p3.x) * (p1.x - p3.x) + (p_pos.y - p3.y) * (p1.y - p3.y);

		const real_t v_e1_2squared_length = v_v1squared_length - dot_v_v1v2 * dot_v_v1v2 / v1_v2squared_length;
		const real_t v_e2_3squared_length = v_v2squared_length - dot_v_v2v3 * dot_v_v2v3 / v2_v3squared_length;
		const real_t v_e3_1squared_length = v_v3squared_length - dot_v_v3v1 * dot_v_v3v1 / v3_v1squared_length;

		const bool close_to_e1_2 = v_e1_2squared_length <= DDLS::EPSILON_SQUARED;
		const bool close_to_e2_3 = v_e2_3squared_length <= DDLS::EPSILON_SQUARED;
		const bool close_to_e3_1 = v_e3_1squared_length <= DDLS::EPSILON_SQUARED;

		if (close_to_e1_2) {
			if (close_to_e3_1) {
				result = v1;
			} else if (close_to_e2_3) {
				result = v2;
			} else {
				result = e1_2;
			}
		} else if (close_to_e2_3) {
			if (close_to_e3_1) {
				result = v3;
			} else {
				result = e2_3;
			}
		} else if (close_to_e3_1) {
			result = e3_1;
		} else {
			result = p_polygon;
		}
	}

	return result;

	// we will use barycentric coordinates
	// see http://en.wikipedia.org/wiki/Barycentric_coordinate_system
	//
	// Edge e1_2 = polygon.edge;
	// Edge e2_3 = e1_2.next_left_edge;
	// Edge e3_1 = e2_3.next_left_edge;
	//
	// Vertex v1 = e1_2.origin_vertex;
	// Vertex v2 = e2_3.origin_vertex;
	// Vertex v3 = e3_1.origin_vertex;
	//
	// real_t x1 = v1.pos.x;
	// real_t y1 = v1.pos.y;
	// real_t x2 = v2.pos.x;
	// real_t y2 = v2.pos.y;
	// real_t x3 = v3.pos.x;
	// real_t y3 = v3.pos.y;
	//
	// real_t coef1 = ((y2 - y3)*(x - x3) + (x3 - x2)*(y - y3)) / ((y2 - y3)*(x1 - x3) + (x3 - x2)*(y1 - y3));
	// real_t coef2 = ((y3 - y1)*(x - x3) + (x1 - x3)*(y - y3)) / ((y2 - y3)*(x1 - x3) + (x3 - x2)*(y1 - y3));
	// real_t coef3 = 1 - coef1 - coef2;
	//
	// Variant result;
	// if ( 0 <= coef1 && coef1 <= 1 && 0 <= coef2 && coef2 <= 1 && 0 <= coef3 && coef3 <= 1 ) {
	//   real_t v_v1squared_length = (x1 - x)*(x1 - x) + (y1 - y)*(y1 - y);
	//   real_t v_v2squared_length = (x2 - x)*(x2 - x) + (y2 - y)*(y2 - y);
	//   real_t v_v3squared_length = (x3 - x)*(x3 - x) + (y3 - y)*(y3 - y);
	//   real_t v1_v2squared_length = (x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1);
	//   real_t v2_v3squared_length = (x3 - x2)*(x3 - x2) + (y3 - y2)*(y3 - y2);
	//   real_t v3_v1squared_length = (x1 - x3)*(x1 - x3) + (y1 - y3)*(y1 - y3);
	//
	//   real_t dot_v_v1v2 = (x - x1)*(x2 - x1) + (y - y1)*(y2 - y1);
	//   real_t dot_v_v2v3 = (x - x2)*(x3 - x2) + (y - y2)*(y3 - y2);
	//   real_t dot_v_v3v1 = (x - x3)*(x1 - x3) + (y - y3)*(y1 - y3);
	//
	//   real_t v_e1_2squared_length = v_v1squared_length - dot_v_v1v2 * dot_v_v1v2 / v1_v2squared_length;
	//   real_t v_e2_3squared_length = v_v2squared_length - dot_v_v2v3 * dot_v_v2v3 / v2_v3squared_length;
	//   real_t v_e3_1squared_length = v_v3squared_length - dot_v_v3v1 * dot_v_v3v1 / v3_v1squared_length;
	//
	//   bool close_to_e1_2 = v_e1_2squared_length <= EPSILON_SQUARED;
	//   bool close_to_e2_3 = v_e2_3squared_length <= EPSILON_SQUARED;
	//   bool close_to_e3_1 = v_e3_1squared_length <= EPSILON_SQUARED;
	//
	//   if ( close_to_e1_2 ) {
	//     if ( closeTo_e3_1 )
	//       result = v1;
	//     else if ( close_to_e2_3 )
	//       result = v2;
	//     else
	//       result = e1_2;
	//   } else if ( close_to_e2_3 ) {
	//     if ( closeTo_e3_1 )
	//       result = v3;
	//     else
	//       result = e2_3;
	//   } else if ( close_to_e3_1 )
	//     result = e3_1;
	//   } else {
	//     result = polygon;
	//   }
	// }
	//
	// return result;

	// parametric expression of e_left:
	//   x(t1) = v_corner.x + t1*(v_left.x - v_corner.x)
	//   x(t1) = v_corner.y + t1*(v_left.y - v_corner.y)
	//
	// for e_right:
	//   x(t2) = v_corner.x + t2*(v_right.x - v_corner.x)
	//   x(t2) = v_corner.y + t2*(v_right.y - v_corner.y)
	//
	//   (x, y) position can be expressed as a linear combination of the 2 previous segments
	//
	//   x = v_corner.x + t1*(v_left.x - v_corner.x) + t2*(v_right.x - v_corner.x)
	//   y = v_corner.y + t1*(v_left.y - v_corner.y) + t2*(v_right.y - v_corner.y)
	//
	// values of t1, t2 and s=t1+t2 will inform us if v_to_check lies in the polygon

	// set alias letters
	// real_t a = x;
	// real_t b = y;
	// real_t c = v_corner.pos.x;
	// real_t d = v_corner.pos.y;
	// real_t e = v_left.pos.x;
	// real_t f = v_left.pos.y;
	// real_t g = v_right.pos.x;
	// real_t h = v_right.pos.y;

	// system to solve:
	//  a = c + t1 (e - c) + t2 (g - c)
	//  b = d + t1 (f - d) + t2 (h - d)

	// giving to wolfram:
	//  Solve[{a = c + t1 (e - c) + t2 (g - c) , b = d + t1 (f - d) + t2 (h - d)}, {t1, t2}]
	// we get:
	//  real_t denominator = (c*(f - h) + d*(g - e) + e*h - f*g);
	//  real_t t1 = (a*(h - d) + b*(c - g) - c*h + d*g) / denominator;
	//  real_t t2 = (a*(f - d) + b*(c - e) - c*f + d*e) / -denominator;
	// then we deduce:
	//  real_t s = t1 + t2;

	//  Variant result;
	//  # if inside triangle:
	//  if (0 <= t1 && t1 <=1 && 0 <= t2 && t2 <=1 && 0 <= s && s <=1) {
	//    if (t2*((g - c)*(g - c) + (h - d)*(h - d)) <= QEConstants.EPSILON_SQUARED)
	//  # if near v_corner:
	//    if (((c - a)*(c - a) + (d - b)*(d - b)) <= QEConstants.EPSILON_SQUARED)
	//      result = v_corner;
	//  # if near vLeft:
	//    else if (((e - a)*(e - a) + (f - b)*(f - b)) <= QEConstants.EPSILON_SQUARED)
	//      result = v_left;
	//  # if near vRight:
	//    else if (((g - a)*(g - a) + (h - b)*(h - b)) <= QEConstants.EPSILON_SQUARED)
	//      result = v_right;
	//    else
	//      result = polygon;
	//  } else {
	//    result = nullptr;
	//  }
	//  return result;
}

// return:
// - true if the segment is totally or partially in the triangle
// - false if the segment is totally outside the triangle
bool clip_segment_by_triangle(const Point2 &s1, const Point2 &s2, const Point2 &t1, const Point2 &t2, const Point2 &t3, Point2 &r_result1, Point2 &r_result2) {
	const int side1_1 = get_direction(t1, t2, s1);
	const int side1_2 = get_direction(t1, t2, s2);
	if (side1_1 <= 0 && side1_2 <= 0) {
		return false; // if both segment points are on right side
	}
	const int side2_1 = get_direction(t2, t3, s1);
	const int side2_2 = get_direction(t2, t3, s2);
	if (side2_1 <= 0 && side2_2 <= 0) {
		return false; // if both segment points are on right side
	}
	const int side3_1 = get_direction(t3, t1, s1);
	const int side3_2 = get_direction(t3, t1, s2);
	if (side3_1 <= 0 && side3_2 <= 0) {
		return false; // if both segment points are on right side
	}
	if ((side1_1 >= 0 && side2_1 >= 0 && side3_1 >= 0) && (side1_2 >= 0 && side2_2 >= 0 && side3_2 >= 0)) {
		r_result1 = s1;
		r_result2 = s2;
		return true; // both segment points are in triangle
	}

	int n = 0;
	// check intersection between segment and 1st side triangle
	if (intersections2segments(s1, s2, t1, t2, &r_result1)) {
		n++;
	}

	// if no intersection with 1st side triangle
	if (n == 0) {
		// check intersection between segment and 1st side triangle
		if (intersections2segments(s1, s2, t2, t3, &r_result1)) {
			n++;
		}
	} else {
		if (intersections2segments(s1, s2, t2, t3, &r_result2)) {
			// we check if the segment is not on t2 triangle point
			if (-DDLS::EPSILON > r_result1.x - r_result2.x || r_result1.x - r_result2.x > DDLS::EPSILON || -DDLS::EPSILON > r_result1.y - r_result2.y || r_result1.y - r_result2.y > DDLS::EPSILON) {
				n++;
			}
		}
	}

	// if intersection neither 1st nor 2nd side triangle
	if (n == 0) {
		if (intersections2segments(s1, s2, t3, t1, &r_result1)) {
			n++;
		}
	} else if (n == 1) {
		if (intersections2segments(s1, s2, t3, t1, &r_result2)) {
			if (-DDLS::EPSILON > r_result1.x - r_result2.x || r_result1.x - r_result2.x > DDLS::EPSILON || -DDLS::EPSILON > r_result1.y - r_result2.y || r_result1.y - r_result2.y > DDLS::EPSILON) {
				n++;
			}
		}
	}

	// if one intersection, we identify the segment point in the triangle
	if (n == 1) {
		if (side1_1 >= 0 && side2_1 >= 0 && side3_1 >= 0) {
			r_result2 = s1;
		} else if (side1_2 >= 0 && side2_2 >= 0 && side3_2 >= 0) {
			r_result2 = s2;
		} else {
			n = 0; // 1 intersection and none point in triangle : degenerate case
		}
	}

	if (n > 0) {
		return true;
	} else {
		return false;
	}
}

// test if the segment intersects or lies inside the triangle
bool is_segment_intersecting_triangle(const Point2 &s1, const Point2 &s2, const Point2 &t1, const Point2 &t2, const Point2 &t3) {
	// check sides
	const int side1_1 = get_direction(t1, t2, s1);
	const int side1_2 = get_direction(t1, t2, s2);
	if (side1_1 <= 0 && side1_2 <= 0) {
		return false; // if both segment points are on right side
	}
	const int side2_1 = get_direction(t2, t3, s1);
	const int side2_2 = get_direction(t2, t3, s2);
	if (side2_1 <= 0 && side2_2 <= 0) {
		return false; // if both segment points are on right side
	}
	const int side3_1 = get_direction(t3, t1, s1);
	const int side3_2 = get_direction(t3, t1, s2);
	if (side3_1 <= 0 && side3_2 <= 0) {
		return false; // if both segment points are on right side
	}
	if (side1_1 == 1 && side2_1 == 1 && side3_1 == 1) {
		return true; // if 1st segment point is inside triangle
	}
	if (side1_1 == 1 && side2_1 == 1 && side3_1 == 1) {
		return true; // if 2st segment point is inside triangle
	}
	// if both segment points are on different sides of the 1st triangle side
	if ((side1_1 == 1 && side1_2 <= 0) || (side1_1 <= 0 && side1_2 == 1)) {
		const int side1 = get_direction(s1, s2, t1);
		const int side2 = get_direction(s1, s2, t2);
		if ((side1 == 1 && side2 <= 0) || (side1 <= 0 && side2 == 1)) {
			return true;
		}
	}
	// if both segment points are on different sides of the 2nd triangle side
	if ((side2_1 == 1 && side2_2 <= 0) || (side2_1 <= 0 && side2_2 == 1)) {
		const int side1 = get_direction(s1, s2, t2);
		const int side2 = get_direction(s1, s2, t3);
		if ((side1 == 1 && side2 <= 0) || (side1 <= 0 && side2 == 1)) {
			return true;
		}
	}
	// if both segment points are on different sides of the 3rd triangle side
	if ((side3_1 == 1 && side3_2 <= 0) || (side3_1 <= 0 && side3_2 == 1)) {
		const int side1 = get_direction(s1, s2, t3);
		const int side2 = get_direction(s1, s2, t1);
		if ((side1 == 1 && side2 <= 0) || (side1 <= 0 && side2 == 1)) {
			return true;
		}
	}
	return false;
}

bool is_delaunay(DDLSEdge p_edge) {
	DDLSVertex v_left = p_edge->get_origin_vertex();
	DDLSVertex v_right = p_edge->get_destination_vertex();
	DDLSVertex v_corner = p_edge->get_next_left_edge()->get_destination_vertex();
	DDLSVertex v_opposite = p_edge->get_next_right_edge()->get_destination_vertex();

	// middle points
	//   Point2 v_mid_left;
	//   v_mid_left.x = (v_corner.pos.x + v_left.pos.x) / 2;
	//   v_mid_left.y = (v_corner.pos.y + v_left.pos.y) / 2;
	//
	//   Point2 v_mid_right;
	//   v_mid_right.x = (v_corner.pos.x + v_right.pos.x) / 2;
	//   v_mid_right.y = (v_corner.pos.y + v_right.pos.y) / 2;

	// - parametric expression of orthogonal segments
	//   seg_ortho_left_x(t1) = v_mid_left.x + t1 * (v_left.y - v_corner.y)
	//   seg_ortho_left_y(t1) = v_mid_left.y - t1 * (v_left.x - v_corner.x)
	//
	//   seg_ortho_right_x(t2) = v_mid_right.x + t2 * (v_right.y - v_corner.y)
	//   seg_ortho_right_y(t2) = v_mid_right.y - t2 * (v_right.x - v_corner.x)
	//
	// - the center of circle passing by v_left, v_right, v_corner will lead to:
	//   seg_ortho_left_x(t1) = seg_ortho_right_x(t2)
	//   seg_ortho_left_y(t1) = seg_ortho_right_y(t2)

	// set alias letters
	//   real_t a = v_mid_left.x;
	//   real_t b = v_left.pos.y;
	//   real_t c = v_corner.pos.y;
	//   real_t d = v_mid_right.x;
	//   real_t e = v_right.pos.y;
	//   real_t f = v_corner.pos.y;
	//   real_t g = v_mid_left.y;
	//   real_t h = v_left.pos.x;
	//   real_t i = v_corner.pos.x;
	//   real_t j = v_mid_right.y;
	//   real_t k = v_right.pos.x;
	//   real_t l = v_corner.pos.x;
	//
	// system to solve:
	//   a + t1 (b - c) = d + t2 (e - f)
	//   g - t1 (h - i) = j - t2 (k - l)

	// giving to wolfram:
	//   Solve[{a + t1 (b - c) = d + t2 (e - f) , g - t1 (h - i) = j - t2 (k - l)}, {t1, t2}]
	// we get:
	//   real_t t1 = (-(a-d)*(k-l) + e*(j-g) + f*(g-j)) / ((b-c)*(k-l) + e*(i-h) + f*(h-i));

	// _barycenter.x = a + t1 * (b - c);
	// _barycenter.y = g - t1 * (h - i);
	_circumcenter = get_circumcenter(v_corner->get_pos(), v_left->get_pos(), v_right->get_pos());

	// check if the opposite vertex lies outside the circle:
	const real_t squared_radius = (v_corner->get_pos() - _circumcenter).length_squared();
	const real_t squared_distance = (v_opposite->get_pos() - _circumcenter).length_squared();

	return squared_distance >= squared_radius;
}

Point2 get_circumcenter(const Point2 &p1, const Point2 &p2, const Point2 &p3) {
	// middle points:
	const real_t m1 = (p1.x + p2.x) / 2.0;
	const real_t m2 = (p1.y + p2.y) / 2.0;
	const real_t m3 = (p1.x + p3.x) / 2.0;
	const real_t m4 = (p1.y + p3.y) / 2.0;

	// - parametric expression of orthogonal segments:
	//   seg_ortho1_x(t1) = m1 + t1 * (y2 - y1)
	//   seg_ortho1_y(t1) = m2 - t1 * (x2 - x1)
	//
	//   seg_ortho2_x(t2) = m3 + t2 * (y3 - y1)
	//   seg_ortho2_y(t2) = m4 - t2 * (x3 - x1)
	//
	// - the center of circle passing by v_left, v_right, v_corner will lead to:
	//   seg_ortho1_x(t1) = seg_ortho2_x(t2)
	//   seg_ortho1_y(t1) = seg_ortho2_y(t2)
	//
	// system to solve:
	//   m1 + t1 (y2 - y1) = m3 + t2 (y3 - y1)
	//   m2 - t1 (x2 - x1) = m4 - t2 (x3 - x1)
	//
	// giving to wolfram:
	//   Solve[{m1 + t1 (y2 - y1) = m3 + t2 (y3 - y1) , m2 - t1 (x2 - x1) = m4 - t2 (x3 - x1)}, {t1, t2}]
	// we get:
	const real_t t1 = (m1 * (p1.x - p3.x) + (m2 - m4) * (p1.y - p3.y) + m3 * (p3.x - p1.x)) / (p1.x * (p3.y - p2.y) + p2.x * (p1.y - p3.y) + p3.x * (p2.y - p1.y));

	const real_t rx = m1 + t1 * (p2.y - p1.y);
	const real_t ry = m2 - t1 * (p2.x - p1.x);

	return { rx, ry };
}

bool intersections2segments(const Point2 &s1p1, const Point2 &s1p2, const Point2 &s2p1, const Point2 &s2p2, Point2 *r_pos_intersection, Vector<real_t> *r_param_intersection, bool infinite_line_mode) {
	real_t t1, t2;
	bool result;
	const real_t divisor = (s1p1.x - s1p2.x) * (s2p1.y - s2p2.y) + (s1p2.y - s1p1.y) * (s2p1.x - s2p2.x);
	if (divisor == 0) {
		result = false; // parallel case, no intersection
	} else {
		result = true;

		if (!infinite_line_mode || r_pos_intersection || r_param_intersection) {
			// if we consider edges as finite segments, we must check t1 and t2 values
			t1 = (s1p1.x * (s2p1.y - s2p2.y) + s1p1.y * (s2p2.x - s2p1.x) + s2p1.x * s2p2.y - s2p1.y * s2p2.x) / divisor;
			t2 = (s1p1.x * (s2p1.y - s1p2.y) + s1p1.y * (s1p2.x - s2p1.x) - s1p2.x * s2p1.y + s1p2.y * s2p1.x) / divisor;
			if (!infinite_line_mode && !(0 <= t1 && t1 <= 1 && 0 <= t2 && t2 <= 1)) {
				result = false;
			}
		}
	}
	if (result) {
		if (r_pos_intersection) {
			r_pos_intersection->x = s1p1.x + t1 * (s1p2.x - s1p1.x);
			r_pos_intersection->y = s1p1.y + t1 * (s1p2.y - s1p1.y);
		}
		if (r_param_intersection) {
			r_param_intersection->push_back(t1, t2);
		}
	}
	return result;
}

bool intersections2edges(DDLSEdge edge1, DDLSEdge edge2, Point2 *r_pos_intersection, Vector<real_t> *r_param_intersection, bool infinite_line_mode) {
	return intersections2segments(
			edge1->get_origin_vertex()->get_pos(), edge1->get_destination_vertex()->get_pos(),
			edge2->get_origin_vertex()->get_pos(), edge2->get_destination_vertex()->get_pos(),
			r_pos_intersection, r_param_intersection, infinite_line_mode);
}

// a edge is convex if the polygon formed by the 2 faces at left and right of this edge is convex
bool is_convex(DDLSEdge p_edge) {
	bool result = true;
	DDLSEdge e_left = p_edge->get_next_left_edge()->get_opposite_edge();
	DDLSVertex v_right = p_edge->get_next_right_edge()->get_destination_vertex();
	if (get_relative_position(v_right->get_pos(), e_left) != -1) {
		result = false;
	} else {
		e_left = p_edge->get_prev_right_edge();
		v_right = p_edge->get_prev_left_edge()->get_origin_vertex();
		if (get_relative_position(v_right->get_pos(), e_left) != -1) {
			result = false;
		}
	}
	return result;
}

Point2 project_orthogonaly(const Point2 &p_vertex_pos, DDLSEdge p_edge) {
	// parametric expression of edge
	//   x(t1) = edge.origin_vertex.pos.x + t1*(edge.destination_vertex.pos.x - edge.origin_vertex.pos.x)
	//   y(t1) = edge.origin_vertex.pos.y + t1*(edge.destination_vertex.pos.y - edge.origin_vertex.pos.y)

	// parametric expression of the segment orthogonal to edge and lying by vertex
	//   x(t2) = vertex_pos.x + t2*(edge.destination_vertex.pos.y - edge.origin_vertex.pos.y)
	//   y(t2) = vertex_pos.y - t2*(edge.destination_vertex.pos.x - edge.origin_vertex.pos.x)

	// the orthogonal projection of vertex on edge will lead to:
	//   x(t1) = x(t2)
	//   y(t1) = y(t2)

	// set alias letters
	const real_t a = p_edge->get_origin_vertex()->get_pos().x;
	const real_t b = p_edge->get_origin_vertex()->get_pos().y;
	const real_t c = p_edge->get_destination_vertex()->get_pos().x;
	const real_t d = p_edge->get_destination_vertex()->get_pos().y;
	const real_t e = p_vertex_pos.x;
	const real_t f = p_vertex_pos.y;

	// system to solve:
	//   a + t1 (c - a) = e + t2 (d - b)
	//   b + t1 (d - b) = f - t2 (c - a)

	// solution:
	const real_t t1 = (a * a - a * c - a * e + b * b - b * d - b * f + c * e + d * f) / (a * a - 2 * a * c + b * b - 2 * b * d + c * c + d * d);

	// set position:
	return { a + t1 * (c - a), b + t1 * (d - b) };
}

Point2 project_orthogonaly_on_segment(const Point2 &p_pos, const Point2 &p_sp1, const Point2 &p_sp2) {
	// set alias letters
	const real_t a = p_sp1.x;
	const real_t b = p_sp1.y;
	const real_t c = p_sp2.x;
	const real_t d = p_sp2.y;
	const real_t e = p_pos.x;
	const real_t f = p_pos.y;

	// system to solve:
	//   a + t1 (c - a) = e + t2 (d - b)
	//   b + t1 (d - b) = f - t2 (c - a)

	// solution:
	const real_t t1 = (a * a - a * c - a * e + b * b - b * d - b * f + c * e + d * f) / (a * a - 2 * a * c + b * b - 2 * b * d + c * c + d * d);

	// set position:
	return { a + t1 * (c - a), b + t1 * (d - b) };
}

// fill the result vector with 4 elements, with the form:
//   [intersect0.x, intersect0.y, intersect1.x, intersect1.y]
// empty if no intersection
bool intersections2circles(const Point2 &c1, real_t r1, const Point2 &c2, real_t r2, Vector<Point2> *r_result) {
	const real_t dist_radius_sqrd = (c2 - c1).length_squared();

	if ((c1.x != c2.x || c1.y != c2.y) && dist_radius_sqrd <= ((r1 + r2) * (r1 + r2)) && dist_radius_sqrd >= ((r1 - r2) * (r1 - r2))) {
		const real_t transcend_part = Math::sqrt(((r1 + r2) * (r1 + r2) - dist_radius_sqrd) * (dist_radius_sqrd - (r2 - r1) * (r2 - r1)));
		const Point2 first_part = (c1 + c2) / 2 + (c2 - c1) * (r1 * r1 - r2 * r2) / (2 * dist_radius_sqrd);
		const Point2 factor = (c2 - c1) / (2 * dist_radius_sqrd);
		if (r_result) {
			r_result->push_back({ first_part.x + factor.x * transcend_part, first_part.y - factor.y * transcend_part }, { first_part.x - factor.x * transcend_part, first_part.y + factor.y * transcend_part });
		}
		return true;
	} else {
		return false;
	}
}

bool intersections_segment_circle(const Point2 &p0, const Point2 &p1, const Point2 &cc, real_t r, Vector<real_t> *r_result) {
	const Vector2 p0_sqd = { p0.x * p0.x, p0.y * p0.y };
	const real_t a = p1.y * p1.y - 2 * p1.y * p0.y + p0_sqd.y + p1.x * p1.x - 2 * p1.x * p0.x + p0_sqd.x;
	const real_t b = 2 * p0.y * cc.y - 2 * p0_sqd.x + 2 * p1.y * p0.y - 2 * p0_sqd.y + 2 * p1.x * p0.x - 2 * p1.x * cc.x + 2 * p0.x * cc.x - 2 * p1.y * cc.y;
	const real_t c = p0_sqd.y + cc.y * cc.y + cc.x * cc.x - 2 * p0.y * cc.y - 2 * p0.x * cc.x + p0_sqd.x - r * r;
	const real_t delta = b * b - 4 * a * c;

	if (delta < 0) {
		// no solution
		return false;
	} else if (delta == 0) {
		// unique solution
		const real_t t0 = -b / (2 * a);
		if (t0 < 0 || t0 > 1) {
			return false;
		}
		// we return a 3 elements array, under the form:
		//  [intersect0.x, intersect0.y, t0]
		if (r_result) {
			r_result->push_back(p0.x + t0 * (p1.x - p0.x), p0.y + t0 * (p1.y - p0.y), t0);
		}
		return true;
	} else { // (delta > 0)
		const real_t delta_sqrt = Math::sqrt(delta);
		const real_t t0 = (-b + delta_sqrt) / (2 * a);
		const real_t t1 = (-b - delta_sqrt) / (2 * a);
		// we return a n elements array, under the form:
		//  [ intersect0.x, intersect0.y, t0,
		//    intersect1.x, intersect1.y, t1 ]
		bool intersecting = false;
		if (0 <= t0 && t0 <= 1) {
			if (r_result) {
				r_result->push_back(p0.x + t0 * (p1.x - p0.x), p0.y + t0 * (p1.y - p0.y), t0);
			}
			intersecting = true;
		}
		if (0 <= t1 && t1 <= 1) {
			if (r_result) {
				r_result->push_back(p0.x + t1 * (p1.x - p0.x), p0.y + t1 * (p1.y - p0.y), t1);
			}
			intersecting = true;
		}
		return intersecting;
	}
}

bool intersections_line_circle(const Point2 p0, const Point2 &p1, const Point2 &cc, real_t r, Vector<real_t> *r_result) {
	const Vector2 p0_sqd = { p0.x * p0.x, p0.y * p0.y };
	const real_t a = p1.y * p1.y - 2 * p1.y * p0.y + p0_sqd.y + p1.x * p1.x - 2 * p1.x * p0.x + p0_sqd.x;
	const real_t b = 2 * p0.y * cc.y - 2 * p0_sqd.x + 2 * p1.y * p0.y - 2 * p0_sqd.y + 2 * p1.x * p0.x - 2 * p1.x * cc.x + 2 * p0.x * cc.x - 2 * p1.y * cc.y;
	const real_t c = p0_sqd.y + cc.y * cc.y + cc.x * cc.x - 2 * p0.y * cc.y - 2 * p0.x * cc.x + p0_sqd.x - r * r;
	const real_t delta = b * b - 4 * a * c;

	if (delta < 0) {
		// no solution
		return false;
	} else if (delta == 0) {
		// unique solution
		const real_t t0 = -b / (2 * a);
		// we return a 3 elements array, under the form:
		//  [intersect0.x, intersect0.y, t0]
		if (r_result) {
			r_result->push_back(p0.x + t0 * (p1.x - p0.x), p0.y + t0 * (p1.y - p0.y), t0);
		}
	} else if (delta > 0) {
		const real_t delta_sqrt = Math::sqrt(delta);
		const real_t t0 = (-b + delta_sqrt) / (2 * a);
		const real_t t1 = (-b - delta_sqrt) / (2 * a);
		// we return a 6 elements array, under the form:
		//  [intersect0.x, intersect0.y, t0,
		//   intersect1.x, intersect1.y, t1]
		if (r_result) {
			r_result->push_back(p0.x + t0 * (p1.x - p0.x), p0.y + t0 * (p1.y - p0.y), t0);
			r_result->push_back(p0.x + t1 * (p1.x - p0.x), p0.y + t1 * (p1.y - p0.y), t1);
		}
	}

	return true;
}

// based on intersections2circles method
// fill the result vector with 4 elements, with the form:
//   [point_tangent1, point_tangent2]
// empty if no tangent
bool tangents_point_to_circle(const Point2 &p, const Point2 &cc, real_t r, Vector<Point2> *r_result) {
	const Point2 cc2 = (p + cc) / 2;
	const real_t r2 = 0.5 * Math::sqrt((p.x - cc.x) * (p.x - cc.x) + (p.y - cc.y) * (p.y - cc.y));

	return intersections2circles(cc2, r2, cc, r, r_result);
}

// <!!!> CIRCLES MUST HAVE SAME RADIUS
bool tangents_cross_circle_to_circle(real_t r, const Point2 &c1, const Point2 &c2, Vector<real_t> *r_result) {
	DEV_ASSERT(sizeof(Vector2) != sizeof(real_t) * 2);
	return tangents_cross_circle_to_circle(r, c1, c2, (Vector<Point2> *)r_result);
}

bool tangents_cross_circle_to_circle(real_t r, const Point2 &c1, const Point2 &c2, Vector<Point2> *r_result) {
	const real_t distance = Math::sqrt((c1.x - c2.x) * (c1.x - c2.x) + (c1.y - c2.y) * (c1.y - c2.y));

	// new circle
	const real_t radius = distance / 4;
	const Point2 center = c1 + (c2 - c1) / 4;

	if (intersections2circles(c1, r, center, radius, r_result)) {
		if (r_result) {
			const Point2 t1 = (*r_result)[0];
			const Point2 t2 = (*r_result)[1];

			const Point2 mid = (c1 + c2) / 2;
			const real_t dot_prod = (t1.x - mid.x) * (c2.y - c1.y) + (t1.y - mid.y) * (-c2.x + c1.x);
			const real_t tproj = dot_prod / (distance * distance);
			const real_t projx = mid.x + tproj * (c2.y - c1.y);
			const real_t projy = mid.y - tproj * (c2.x - c1.x);

			const Point2 t4 = 2 * Vector2(projx, projy) - t1;
			const Point2 t3 = t4 + t2 - t1;

			r_result->push_back(t3, t4);
		}
		return true;
	} else {
		return false; // no tangent because cicles are intersecting
	}
}

Vector<Point2> tangents_paral_circle_to_circle(real_t r, const Point2 &c1, const Point2 &c2) {
	const real_t distance = Math::sqrt((c1.x - c2.x) * (c1.x - c2.x) + (c1.y - c2.y) * (c1.y - c2.y));

	const real_t t1x = c1.x + r * (c2.y - c1.y) / distance;
	const real_t t1y = c1.y + r * (-c2.x + c1.x) / distance;

	const Point2 t1 = { t1x, t1y };
	const Point2 t2 = 2 * c1 - t1;
	const Point2 t3 = t2 + c2 - c1;
	const Point2 t4 = t1 + c2 - c1;

	return make_vector(t1, t2, t3, t4);
}

// squared distance from point p to infinite line (a, b)
real_t distance_squared_point_to_line(const Point2 &p, const Point2 &a, const Point2 &b) {
	const real_t a_b_squared_length = (b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y);
	const real_t dot_product = (p.x - a.x) * (b.x - a.x) + (p.y - a.y) * (b.y - a.y);
	const real_t p_a_squared_length = (a.x - p.x) * (a.x - p.x) + (a.y - p.y) * (a.y - p.y);
	return p_a_squared_length - dot_product * dot_product / a_b_squared_length;
}

// squared distance from point p to finite segment [a, b]
real_t distance_squared_point_to_segment(const Point2 &p, const Point2 &a, const Point2 &b) {
	const real_t a_b_squared_length = (b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y);
	const real_t dot_product = ((p.x - a.x) * (b.x - a.x) + (p.y - a.y) * (b.y - a.y)) / a_b_squared_length;
	if (dot_product < 0) {
		return (p.x - a.x) * (p.x - a.x) + (p.y - a.y) * (p.y - a.y);
	} else if (dot_product <= 1) {
		const real_t p_a_squared_length = (a.x - p.x) * (a.x - p.x) + (a.y - p.y) * (a.y - p.y);
		return p_a_squared_length - dot_product * dot_product * a_b_squared_length;
	} else {
		return (p.x - b.x) * (p.x - b.x) + (p.y - b.y) * (p.y - b.y);
	}
}

real_t distance_squared_vertex_to_edge(DDLSVertex p_vertex, DDLSEdge p_edge) {
	return distance_squared_point_to_segment(p_vertex->get_pos(), p_edge->get_origin_vertex()->get_pos(), p_edge->get_destination_vertex()->get_pos());
}

real_t path_length(Vector<Point2> p_path) {
	real_t sum_distance = 0;
	Point2 from_point = p_path[0];
	for (int i = 1; i < p_path.size(); i++) {
		const Point2 next_point = p_path[i];
		sum_distance += (next_point - from_point).length();
		from_point = next_point;
	}
	return sum_distance;
}
} // namespace DDLSGeom2D

/// DDLSRectMeshFactory

namespace DDLSRectMeshFactory {

#if 0
		    TL
		----+-----+ TR
		\   |    /|
		\   |   / |
		\   |  /  |
		\   | /   |
		\   |/    |
		\   +-----+ BR
		\  BL     \
		\----------
#endif
DDLSMesh build_rectangle(real_t p_width, real_t p_height) {
	DDLSVertex v_tl;
	v_tl.instance();
	DDLSVertex v_tr;
	v_tr.instance();
	DDLSVertex v_br;
	v_br.instance();
	DDLSVertex v_bl;
	v_bl.instance();

	DDLSEdge e_tl_tr;
	e_tl_tr.instance();
	DDLSEdge e_tr_tl;
	e_tr_tl.instance();
	DDLSEdge e_tr_br;
	e_tr_br.instance();
	DDLSEdge e_br_tr;
	e_br_tr.instance();
	DDLSEdge e_br_bl;
	e_br_bl.instance();
	DDLSEdge e_bl_br;
	e_bl_br.instance();
	DDLSEdge e_bl_tl;
	e_bl_tl.instance();
	DDLSEdge e_tl_bl;
	e_tl_bl.instance();
	DDLSEdge e_tr_bl;
	e_tr_bl.instance();
	DDLSEdge e_bl_tr;
	e_bl_tr.instance();
	DDLSEdge e_tl_br;
	e_tl_br.instance();
	DDLSEdge e_br_tl;
	e_br_tl.instance();

	DDLSFace f_tl_bl_tr;
	f_tl_bl_tr.instance();
	DDLSFace f_tr_bl_br;
	f_tr_bl_br.instance();
	DDLSFace f_tl_br_bl;
	f_tl_br_bl.instance();
	DDLSFace f_tl_tr_br;
	f_tl_tr_br.instance();

	DDLSConstraintShape bound_shape;
	bound_shape.instance();
	DDLSConstraintSegment seg_top;
	seg_top.instance();
	DDLSConstraintSegment seg_right;
	seg_right.instance();
	DDLSConstraintSegment seg_bot;
	seg_bot.instance();
	DDLSConstraintSegment seg_left;
	seg_left.instance();

	DDLSMesh mesh;
	mesh.instance();
	mesh->set_size(p_width, p_height);

	//

	const real_t offset = DDLS::EPSILON * 1000;
	v_tl->set_pos(0 - offset, 0 - offset);
	v_tr->set_pos(p_width + offset, 0 - offset);
	v_br->set_pos(p_width + offset, p_height + offset);
	v_bl->set_pos(0 - offset, p_height + offset);

	v_tl->set_datas(e_tl_tr);
	v_tr->set_datas(e_tr_br);
	v_br->set_datas(e_br_bl);
	v_bl->set_datas(e_bl_tl);

	e_tl_tr->set_datas(v_tl, e_tr_tl, e_tr_br, f_tl_tr_br, true, true);
	e_tr_tl->set_datas(v_tr, e_tl_tr, e_tl_bl, f_tl_bl_tr, true, true);
	e_tr_br->set_datas(v_tr, e_br_tr, e_br_tl, f_tl_tr_br, true, true);
	e_br_tr->set_datas(v_br, e_tr_br, e_tr_bl, f_tr_bl_br, true, true);
	e_br_bl->set_datas(v_br, e_bl_br, e_bl_tl, f_tl_br_bl, true, true);
	e_bl_br->set_datas(v_bl, e_br_bl, e_br_tr, f_tr_bl_br, true, true);
	e_bl_tl->set_datas(v_bl, e_tl_bl, e_tl_br, f_tl_br_bl, true, true);
	e_tl_bl->set_datas(v_tl, e_bl_tl, e_bl_tr, f_tl_bl_tr, true, true);
	e_tr_bl->set_datas(v_tr, e_bl_tr, e_bl_br, f_tr_bl_br, true, false); // diagonal edge
	e_bl_tr->set_datas(v_bl, e_tr_bl, e_tr_tl, f_tl_bl_tr, true, false); // diagonal edge
	e_tl_br->set_datas(v_tl, e_br_tl, e_br_bl, f_tl_br_bl, false, false); // imaginary edge
	e_br_tl->set_datas(v_br, e_tl_br, e_tl_tr, f_tl_tr_br, false, false); // imaginary edge

	f_tl_bl_tr->set_datas(e_bl_tr);
	f_tr_bl_br->set_datas(e_tr_bl);
	f_tl_br_bl->set_datas(e_br_bl, false);
	f_tl_tr_br->set_datas(e_tr_br, false);

	// constraint relations datas
	v_tl->add_from_constraint_segment(seg_top, seg_left);
	v_tr->add_from_constraint_segment(seg_top, seg_right);
	v_br->add_from_constraint_segment(seg_right, seg_bot);
	v_bl->add_from_constraint_segment(seg_bot, seg_left);

	e_tl_tr->add_from_constraint_segment(seg_top);
	e_tr_tl->add_from_constraint_segment(seg_top);
	e_tr_br->add_from_constraint_segment(seg_right);
	e_br_tr->add_from_constraint_segment(seg_right);
	e_br_bl->add_from_constraint_segment(seg_bot);
	e_bl_br->add_from_constraint_segment(seg_bot);
	e_bl_tl->add_from_constraint_segment(seg_left);
	e_tl_bl->add_from_constraint_segment(seg_left);

	seg_top->add_edge(e_tl_tr);
	seg_right->add_edge(e_tr_br);
	seg_bot->add_edge(e_br_bl);
	seg_left->add_edge(e_bl_tl);
	seg_top->set_from_shape(bound_shape);
	seg_right->set_from_shape(bound_shape);
	seg_bot->set_from_shape(bound_shape);
	seg_left->set_from_shape(bound_shape);
	bound_shape->add_segment(seg_top, seg_right, seg_bot, seg_left);

	mesh->add_vertex(v_tl, v_tr, v_br, v_bl);
	mesh->add_edge(e_tl_tr, e_tr_tl, e_tr_br, e_br_tr);
	mesh->add_edge(e_br_bl, e_bl_br, e_bl_tl, e_tl_bl);
	mesh->add_edge(e_tr_bl, e_bl_tr, e_tl_br, e_br_tl);
	mesh->add_face(f_tl_bl_tr, f_tr_bl_br, f_tl_br_bl, f_tl_tr_br);
	mesh->add_constraint_shape(bound_shape);

	Vector<Point2> security_rect;
	security_rect.push_back(Point2(0, 0), Point2(p_width, 0));
	security_rect.push_back(Point2(p_width, 0), Point2(p_width, p_height));
	security_rect.push_back(Point2(p_width, p_height), Point2(0, p_height));
	security_rect.push_back(Point2(0, p_height), Point2(0, 0));
	mesh->set_clipping(false);
	mesh->insert_constraint_shape(security_rect);
	mesh->set_clipping(true);

	return mesh;
}
} // namespace DDLSRectMeshFactory

namespace DDLSBitmapObjectFactory {

DDLSObject build_from_bmp_data(Ref<Image> bmp_data, Ref<Image> p_debug_bmp, CanvasItem *p_debug_shape) {
	// OUTLINES STEP-LIKE SHAPES GENERATION
	Vector<Vector<Point2>> shapes = DDLSPotrace::build_shapes(bmp_data, p_debug_bmp, p_debug_shape);

	// GRAPHS OF POTENTIAL SEGMENTS GENERATION
	Vector<DDLSGraph> graphs;
	for (int i = 0; i < shapes.size(); i++) {
		graphs.push_back(DDLSPotrace::build_graph(shapes[i]));
	}

	// OPTIMIZED POLYGONS GENERATION
	Vector<Vector<Point2>> polygons;
	for (int i = 0; i < graphs.size(); i++) {
		polygons.push_back(DDLSPotrace::build_polygon(graphs[i], p_debug_shape));
	}

	// OBJECT GENERATION
	DDLSObject obj;
	obj.instance();
	for (int i = 0; i < polygons.size(); i++) {
		for (int j = 0; j < polygons[i].size() - 1; j++) {
			obj->add_coordinates(polygons[i][j], polygons[i][j + 1]);
		}
		obj->add_coordinates(polygons[i][0], polygons[i][polygons[i].size() - 1]);
	}

	return obj;
}
} // namespace DDLSBitmapObjectFactory

namespace DDLSBitmapMeshFactory {
DDLSMesh build_from_bmp_data(Ref<Image> p_bmp_data, Ref<Image> p_debug_bmp, CanvasItem *p_debug_shape) {
	ERR_FAIL_NULL_V(p_bmp_data, DDLSMesh());

	// OUTLINES STEP-LIKE SHAPES GENERATION
	Vector<Vector<Point2>> shapes = DDLSPotrace::build_shapes(p_bmp_data, p_debug_bmp, p_debug_shape);

	// GRAPHS OF POTENTIAL SEGMENTS GENERATION
	Vector<DDLSGraph> graphs;
	for (int i = 0; i < shapes.size(); i++) {
		graphs.push_back(DDLSPotrace::build_graph(shapes[i]));
	}

	// OPTIMIZED POLYGONS GENERATION
	Vector<Vector<Point2>> polygons;
	for (int i = 0; i < graphs.size(); i++) {
		polygons.push_back(DDLSPotrace::build_polygon(graphs[i], p_debug_shape));
	}

	// MESH GENERATION
	DDLSMesh mesh = DDLSRectMeshFactory::build_rectangle(p_bmp_data->get_width(), p_bmp_data->get_height());
	for (int i = 0; i < polygons.size(); i++) {
		for (int j = 0; j < polygons[i].size() - 1; j++) {
			mesh->insert_constraint_segment(polygons[i][j], polygons[i][j + 1]);
		}
		mesh->insert_constraint_segment(polygons[i][0], polygons[i][polygons[i].size() - 1]);
	}

	return mesh;
}
} // namespace DDLSBitmapMeshFactory

/// DDLSSimpleView

namespace {
const Color color_edges = Color::from_abgr(0x999999);
const Color color_constraints = Color::from_abgr(0xFF0000);
const Color color_vertices = Color::from_abgr(0x0000FF);
const Color color_paths = Color::from_abgr(0xFF00FF);
const Color color_entities = Color::from_abgr(0x00FF00);
} //namespace

void DDLSSimpleView::draw_mesh(DDLSMesh p_mesh) {
	VS::get_singleton()->canvas_item_clear(_surface);
	VS::get_singleton()->canvas_item_clear(_edges);
	VS::get_singleton()->canvas_item_clear(_constraints);
	VS::get_singleton()->canvas_item_clear(_vertices);

	VS::get_singleton()->canvas_item_add_rect(_surface, Rect2(0, 0, p_mesh->get_width(), p_mesh->get_height()), Color::from_abgr(0xFF0000));

	IteratorFromMeshToVertices iter_vertices = IteratorFromMeshToVertices().set_from_mesh(p_mesh);

	IteratorFromVertexToIncomingEdges iter_edges;
	Map<DDLSVertex, bool> dict_vertices_done;

	while (DDLSVertex vertex = iter_vertices.next()) {
		dict_vertices_done[vertex] = true;
		if (!vertex_is_inside_aabb(vertex, p_mesh)) {
			continue;
		}
		VS::get_singleton()->canvas_item_add_circle(_vertices, vertex->get_pos(), 0.5, color_vertices);

		if (_show_vertices_indices) {
			_debug_font->draw(_vertices, vertex->get_pos() + Vector2(5, 5), itos(vertex->get_id()));
		}

		iter_edges.set_from_vertex(vertex);
		while (DDLSEdge incoming_edge = iter_edges.next()) {
			if (!dict_vertices_done[incoming_edge->get_origin_vertex()]) {
				if (incoming_edge->if_is_constrained()) {
					VS::get_singleton()->canvas_item_add_line(_constraints, incoming_edge->get_origin_vertex()->get_pos(), incoming_edge->get_destination_vertex()->get_pos(), color_constraints, 2);
				} else {
					VS::get_singleton()->canvas_item_add_line(_edges, incoming_edge->get_origin_vertex()->get_pos(), incoming_edge->get_destination_vertex()->get_pos(), color_edges);
				}
			}
		}
	}
}

void DDLSSimpleView::draw_entity(DDLSEntityAI p_entity, bool p_clean_before) {
	if (p_clean_before) {
		VS::get_singleton()->canvas_item_clear(_entities);
	}
	VS::get_singleton()->canvas_item_add_circle(_entities, p_entity->get_pos(), p_entity->get_radius(), color_entities);
	if (p_entity->get_angle_fov() > 0 && p_entity->get_radius_fov() > 0) {
		const real_t dir_angle = Math::atan2(p_entity->get_dir_norm().y, p_entity->get_dir_norm().x);
		const Vector2 left_field = { Math::cos(dir_angle - p_entity->get_angle_fov() / 2), Math::sin(dir_angle - p_entity->get_angle_fov() / 2) };
		VS::get_singleton()->canvas_item_add_line(_entities, p_entity->get_pos(), p_entity->get_pos() + left_field * p_entity->get_radius_fov(), color_entities);
		const Vector2 right_field = { Math::cos(dir_angle + p_entity->get_angle_fov() / 2), Math::sin(dir_angle + p_entity->get_angle_fov() / 2) };
		VS::get_singleton()->canvas_item_add_line(_entities, p_entity->get_pos(), p_entity->get_pos() + right_field * p_entity->get_radius_fov(), color_entities);
	}
}

void DDLSSimpleView::draw_entities(const Vector<DDLSEntityAI> &p_entities, bool p_clean_before) {
	if (p_clean_before) {
		VS::get_singleton()->canvas_item_clear(_entities);
	}

	for (int i = 0; i < p_entities.size(); i++) {
		draw_entity(p_entities[i], false);
	}
}

void DDLSSimpleView::draw_path(const Vector<Point2> &p_path, bool p_clean_before) {
	if (p_clean_before) {
		VS::get_singleton()->canvas_item_clear(_paths);
	}
	if (p_path.size() == 0) {
		return;
	}
	VS::get_singleton()->canvas_item_add_polygon(_paths, p_path, make_vector(color_paths));
}

bool DDLSSimpleView::vertex_is_inside_aabb(DDLSVertex p_vertex, DDLSMesh p_mesh) {
	if (p_vertex->get_pos().x < 0 || p_vertex->get_pos().x > p_mesh->get_width() || p_vertex->get_pos().y < 0 || p_vertex->get_pos().y > p_mesh->get_height()) {
		return false;
	} else {
		return true;
	}
}

#define _InitItem(ci)                                          \
	ci = RID_PRIME(VS::get_singleton()->canvas_item_create()); \
	VS::get_singleton()->canvas_item_set_parent(ci, get_canvas())

void DDLSSimpleView::_notification(int p_notification) {
	if (p_notification == NOTIFICATION_READY) {
		_InitItem(_paths);
		_InitItem(_edges);
		_InitItem(_surface);
		_InitItem(_vertices);
		_InitItem(_constraints);
		_InitItem(_entities);
	}
}

void DDLSSimpleView::_bind_methods() {
	ClassDB::bind_method(D_METHOD("draw_ddls_mesh", "mesh"), &DDLSSimpleView::draw_mesh);
	ClassDB::bind_method(D_METHOD("draw_ddls_entity", "entity", "clear"), &DDLSSimpleView::draw_entity, DEFVAL(false));
}

DDLSSimpleView::DDLSSimpleView() {
	_show_vertices_indices = false;
}
