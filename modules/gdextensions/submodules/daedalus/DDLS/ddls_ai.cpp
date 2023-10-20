/**************************************************************************/
/*  ddls_ai.cpp                                                           */
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

#include "ai/ai_astar.h"
#include "ai/ai_entity_ai.h"
#include "ai/ai_funnel.h"
#include "ai/ai_path_finder.h"
#include "data/ddls_face.h"
#include "data/ddls_object.h"
#include "data/math/ddls_geom2d.h"

#include "core/error_macros.h"
#include "core/math/math_funcs.h"
#include "core/vector.h"
#include "ddls_fwd.h"

/// DDLS_EntityAI

void DDLS_EntityAI::build_approximation() {
	approximate_object.instance();
	approximate_object->set_matrix(Transform2D().translated(pos));

	if (radius == 0) {
		return;
	}

	Vector<Point2> coordinates;
	for (int i = 0; i < NUM_SEGMENTS; i++) {
		coordinates.push_back(radius * Point2(Math::cos(2 * Math_PI * i / NUM_SEGMENTS), Math::sin(2 * Math_PI * i / NUM_SEGMENTS)));
		coordinates.push_back(radius * Point2(Math::cos(2 * Math_PI * (i + 1) / NUM_SEGMENTS), Math::sin(2 * Math_PI * (i + 1) / NUM_SEGMENTS)));
	}
	approximate_object->set_coordinates(coordinates);
}

DDLSObject DDLS_EntityAI::get_approximate_object() {
	Transform2D matrix;
	matrix.translate(pos);
	approximate_object->set_matrix(matrix);
	return approximate_object;
}

DDLS_EntityAI::DDLS_EntityAI() {
	radius = 10;
	dir_norm = { 1, 0 };
	angle_fov = 60;
}

/// DDLS_PathFinder

DDLSEntityAI DDLS_PathFinder::get_entity() const { return entity; }
void DDLS_PathFinder::set_entity(DDLSEntityAI p_entity) { entity = p_entity; }

DDLSMesh DDLS_PathFinder::get_mesh() const { return mesh; }
void DDLS_PathFinder::set_mesh(DDLSMesh p_mesh) {
	mesh = p_mesh;
	astar->set_mesh(mesh);
}

void DDLS_PathFinder::find_path(const Point2 &p_to, Vector<Point2> &p_result_path) {
	ERR_FAIL_NULL(mesh);
	ERR_FAIL_NULL(entity);

	p_result_path.clear();
	if (DDLSGeom2D::is_circle_intersecting_any_constraint(p_to, entity->get_radius(), mesh)) {
		return;
	}

	astar->set_radius(entity->get_radius());
	funnel->set_radius(entity->get_radius());

	Vector<DDLSFace> list_faces;
	Vector<DDLSEdge> list_edges;
	astar->find_path(entity->get_pos(), p_to, list_faces, list_edges);
	if (list_faces.empty()) {
		print_verbose("(find_path) empty _list_faces");
		return;
	}
	funnel->find_path(entity->get_pos(), p_to, list_faces, list_edges, p_result_path);
}

DDLS_PathFinder::DDLS_PathFinder() {}

/// DDLS_AStar

struct _FaceDefaultComparator {
	DDLS_AStar *owner = nullptr;
	_FORCE_INLINE_ bool operator()(const DDLSFace &a, const DDLSFace &b) const { return owner->sorting_faces(a, b) > 0; }
};

void DDLS_AStar::set_mesh(DDLSMesh p_mesh) {
	mesh = p_mesh;
}

void DDLS_AStar::find_path(const Point2 &p_from, const Point2 &p_to, Vector<DDLSFace> &r_result_list_faces, Vector<DDLSEdge> &p_result_list_edges) {
	Variant loc = DDLSGeom2D::locate_position(p_from, mesh);
	if (DDLSVertex loc_vertex = loc) {
		return; // vertex are always in constraint, so we abort
	} else if (DDLSEdge loc_edge = loc) {
		// if the vertex lies on a constrained edge, we abort
		if (loc_edge->if_is_constrained()) {
			return;
		}
		_from_face = loc_edge->get_left_face();
	} else {
		_from_face = loc;
	}

	loc = DDLSGeom2D::locate_position(p_to, mesh);
	if (DDLSVertex loc_vertex = loc) {
		_to_face = loc_vertex->get_edge()->get_left_face();
	} else if (DDLSEdge loc_edge = loc) {
		_to_face = loc_edge->get_left_face();
	} else {
		_to_face = loc;
	}

	sorted_opened_faces.push_back(_from_face);
	entry_edges[_from_face] = nullptr;
	entry_points[_from_face] = p_from;
	score_g[_from_face] = 0;
	score_h[_from_face] = (p_to - p_from).length();
	score_f[_from_face] = score_h[_from_face] + score_g[_from_face];

	Point2 entry_point;
	while (true) {
		// no path found
		if (sorted_opened_faces.size() == 0) {
			print_verbose("(find_path) no path found");
			_cur_face = nullptr;
			break;
		}

		// we reached the target face
		_cur_face = sorted_opened_faces.pop();
		if (_cur_face == _to_face) {
			break;
		}

		// we continue the search
		IteratorFromFaceToInnerEdges _iter_edge = IteratorFromFaceToInnerEdges().set_from_face(_cur_face);
		while (DDLSEdge inner_edge = _iter_edge.next()) {
			if (inner_edge->if_is_constrained()) {
				continue;
			}
			DDLSFace neighbour_face = inner_edge->get_right_face();
			if (!closed_faces[neighbour_face]) {
				if (_cur_face != _from_face && radius > 0 && !is_walkable_by_radius(entry_edges[_cur_face], _cur_face, inner_edge)) {
					// printv_verbose("- NOT WALKABLE -");
					// printv_verbose( "from, entry_edges[_cur_face]->get_origin_vertex()->get_id(), entry_edges[_cur_face]->get_destination_vertex()->get_id());
					// printv_verbose( "to", inner_edge->get_origin_vertex()->get_id(), inner_edge->get_destination_vertex()->get_id() );
					continue;
				}

				const Point2 from_point = entry_points[_cur_face];

				// entry_point will be the direct point of intersection between p_from_point and p_to if the edge inner_edge intersects it
				Point2 vw1 = inner_edge->get_origin_vertex()->get_pos();
				Point2 vw2 = inner_edge->get_destination_vertex()->get_pos();
				if (!DDLSGeom2D::intersections2segments(from_point, p_to, vw1, vw2, &entry_point)) {
					entry_point = p_to;
					const real_t vst = vw1.distance_squared_to(from_point) + vw1.distance_squared_to(entry_point);
					const real_t wst = vw2.distance_squared_to(from_point) + vw2.distance_squared_to(entry_point);
					entry_point.x = vst <= wst ? vw1.x : vw2.x;
					entry_point.y = vst <= wst ? vw1.y : vw2.y;
				}

				Point2 distance_point = entry_point - p_to;
				real_t h = distance_point.length();
				distance_point = from_point - entry_point;
				real_t g = score_g[_cur_face] + distance_point.length();
				real_t f = h + g;
				bool fill_datas = false;
				if (!opened_faces[neighbour_face]) {
					sorted_opened_faces.push_back(neighbour_face);
					opened_faces[neighbour_face] = true;
					fill_datas = true;
				} else if (score_f[neighbour_face] > f) {
					fill_datas = true;
				}
				if (fill_datas) {
					entry_edges[neighbour_face] = inner_edge;
					entry_points[neighbour_face] = entry_point;
					score_f[neighbour_face] = f;
					score_g[neighbour_face] = g;
					score_h[neighbour_face] = h;
					predecessor[neighbour_face] = _cur_face;
				}
			}
		}

		opened_faces[_cur_face] = nullptr;
		closed_faces[_cur_face] = true;

		if (!sorted_opened_faces.empty()) {
			SortArray<DDLSFace, _FaceDefaultComparator> sorter;
			sorter.compare.owner = this;
			sorter.sort(sorted_opened_faces.ptrw(), sorted_opened_faces.size());
		}
	}

	// if we didn't find a path
	if (!_cur_face) {
		return;
	}

	// else we build the path
	r_result_list_faces.push_back(_cur_face);
	while (_cur_face != _from_face) {
		r_result_list_faces.insert(entry_edges[_cur_face]);
		_cur_face = predecessor[_cur_face];
		r_result_list_faces.insert(_cur_face);
	}
}

// faces with low distance value are at the end of the array
int DDLS_AStar::sorting_faces(DDLSFace a, DDLSFace b) {
	if (score_f[a] == score_f[b]) {
		return 0;
	} else if (score_f[a] < score_f[b]) {
		return 1;
	} else {
		return -1;
	}
}

bool DDLS_AStar::is_walkable_by_radius(DDLSEdge p_from_edge, DDLSFace p_through_face, DDLSEdge p_to_edge) {
	DDLSVertex va; // the vertex on from_edge not on to_edge
	DDLSVertex vb; // the vertex on to_edge not on from_edge
	DDLSVertex vc; // the common vertex of the 2 edges (pivot)

	// we identify the points
	if (p_from_edge->get_origin_vertex() == p_to_edge->get_origin_vertex()) {
		va = p_from_edge->get_destination_vertex();
		vb = p_to_edge->get_destination_vertex();
		vc = p_from_edge->get_origin_vertex();
	} else if (p_from_edge->get_destination_vertex() == p_to_edge->get_destination_vertex()) {
		va = p_from_edge->get_origin_vertex();
		vb = p_to_edge->get_origin_vertex();
		vc = p_from_edge->get_destination_vertex();
	} else if (p_from_edge->get_origin_vertex() == p_to_edge->get_destination_vertex()) {
		va = p_from_edge->get_destination_vertex();
		vb = p_to_edge->get_origin_vertex();
		vc = p_from_edge->get_origin_vertex();
	} else if (p_from_edge->get_destination_vertex() == p_to_edge->get_origin_vertex()) {
		va = p_from_edge->get_origin_vertex();
		vb = p_to_edge->get_destination_vertex();
		vc = p_from_edge->get_destination_vertex();
	}

	// if we have a right or obtuse angle on CAB
	real_t dot = (vc->get_pos().x - va->get_pos().x) * (vb->get_pos().x - va->get_pos().x) + (vc->get_pos().y - va->get_pos().y) * (vb->get_pos().y - va->get_pos().y);
	if (dot <= 0) {
		// we compare length of AC with radius
		const real_t dist_squared = (vc->get_pos() - va->get_pos()).length_squared();
		if (dist_squared >= diameter_squared) {
			return true;
		} else {
			return false;
		}
	}

	// if we have a right or obtuse angle on CBA
	dot = (vc->get_pos().x - vb->get_pos().x) * (va->get_pos().x - vb->get_pos().x) + (vc->get_pos().y - vb->get_pos().y) * (va->get_pos().y - vb->get_pos().y);
	if (dot <= 0) {
		// we compare length of BC with radius
		const real_t dist_squared = (vc->get_pos() - vb->get_pos()).length_squared();
		if (dist_squared >= diameter_squared)
			return true;
		else
			return false;
	}

	// we identify the adjacent edge (facing pivot vertex)
	DDLSEdge adj_edge;
	if (p_through_face->get_edge() != p_from_edge && p_through_face->get_edge()->get_opposite_edge() != p_from_edge && p_through_face->get_edge() != p_to_edge && p_through_face->get_edge()->get_opposite_edge() != p_to_edge) {
		adj_edge = p_through_face->get_edge();
	} else if (p_through_face->get_edge()->get_next_left_edge() != p_from_edge && p_through_face->get_edge()->get_next_left_edge()->get_opposite_edge() != p_from_edge && p_through_face->get_edge()->get_next_left_edge() != p_to_edge && p_through_face->get_edge()->get_next_left_edge()->get_opposite_edge() != p_to_edge) {
		adj_edge = p_through_face->get_edge()->get_next_left_edge();
	} else {
		adj_edge = p_through_face->get_edge()->get_prev_left_edge();
	}

	// if the adjacent edge is constrained, we check the distance of orthognaly projected
	if (adj_edge->if_is_constrained()) {
		Point2 proj = DDLSGeom2D::project_orthogonaly(vc->get_pos(), adj_edge);
		const real_t dist_squared = (proj - vc->get_pos()).length_squared();
		if (dist_squared >= diameter_squared) {
			return true;
		} else {
			return false;
		}
	} else { // if the adjacent is not constrained
		const real_t dist_squared_a = (vc->get_pos() - va->get_pos()).length_squared();
		const real_t dist_squared_b = (vc->get_pos() - vb->get_pos()).length_squared();
		if (dist_squared_a < diameter_squared || dist_squared_b < diameter_squared) {
			return false;
		} else {
			Vector<DDLSFace> v_face_to_check;
			Vector<DDLSEdge> v_face_is_from_edge;
			Map<DDLSFace, bool> faces_done;
			v_face_is_from_edge.push_back(adj_edge);
			if (adj_edge->get_left_face() == p_through_face) {
				v_face_to_check.push_back(adj_edge->get_right_face());
				faces_done[adj_edge->get_right_face()] = true;
			} else {
				v_face_to_check.push_back(adj_edge->get_left_face());
				faces_done[adj_edge->get_left_face()] = true;
			}

			while (v_face_to_check.size() > 0) {
				DDLSFace curr_face = v_face_to_check.shift();
				DDLSEdge face_from_edge = v_face_is_from_edge.shift();

				DDLSEdge curr_edge_a;
				DDLSFace next_face_a;
				DDLSEdge curr_edge_b;
				DDLSFace next_face_b;

				// we identify the 2 edges to evaluate
				if (curr_face->get_edge() == face_from_edge || curr_face->get_edge() == face_from_edge->get_opposite_edge()) {
					curr_edge_a = curr_face->get_edge()->get_next_left_edge();
					curr_edge_b = curr_face->get_edge()->get_next_left_edge()->get_next_left_edge();
				} else if (curr_face->get_edge()->get_next_left_edge() == face_from_edge || curr_face->get_edge()->get_next_left_edge() == face_from_edge->get_opposite_edge()) {
					curr_edge_a = curr_face->get_edge();
					curr_edge_b = curr_face->get_edge()->get_next_left_edge()->get_next_left_edge();
				} else {
					curr_edge_a = curr_face->get_edge();
					curr_edge_b = curr_face->get_edge()->get_next_left_edge();
				}

				// we identify the faces related to the 2 edges
				if (curr_edge_a->get_left_face() == curr_face) {
					next_face_a = curr_edge_a->get_right_face();
				} else {
					next_face_a = curr_edge_a->get_left_face();
				}
				if (curr_edge_b->get_left_face() == curr_face) {
					next_face_b = curr_edge_b->get_right_face();
				} else {
					next_face_b = curr_edge_b->get_left_face();
				}

				// we check if the next face is not already in pipe
				// and if the edge A is close to pivot vertex
				if (!faces_done[next_face_a] && DDLSGeom2D::distance_squared_vertex_to_edge(vc, curr_edge_a) < diameter_squared) {
					// if the edge is constrained
					if (curr_edge_a->if_is_constrained()) {
						// so it is not walkable
						return false;
					} else {
						// if the edge is not constrained, we continue the search
						v_face_to_check.push_back(next_face_a);
						v_face_is_from_edge.push_back(curr_edge_a);
						faces_done[next_face_a] = true;
					}
				}

				// we check if the next face is not already in pipe
				// and if the edge B is close to pivot vertex
				if (!faces_done[next_face_b] && DDLSGeom2D::distance_squared_vertex_to_edge(vc, curr_edge_b) < diameter_squared) {
					// if the edge is constrained
					if (curr_edge_b->if_is_constrained()) {
						return false; // so it is not walkable
					} else {
						// if the edge is not constrained, we continue the search
						v_face_to_check.push_back(next_face_b);
						v_face_is_from_edge.push_back(curr_edge_b);
						faces_done[next_face_b] = true;
					}
				}
			}
			return true; // if we didn't previously meet a constrained edge
		}
	}
	return true;
}

DDLS_AStar::DDLS_AStar() {}

/// DDLS_Funnel

void DDLS_Funnel::adjust_with_tangents(const Point2 &p_1, bool p_apply_radius_to_p1, const Point2 &p_2, bool p_apply_radius_to_p2, const Map<Point2, int> &p_point_sides, const Map<Point2, Point2> &p_point_successor, Vector<Point2> &r_new_path, Vector<Point2> &r_adjusted_points) {
	// we find the tangent T between the points path_points[i] - path_points[i+1]
	// then we check the unused intermediate points between path_points[i] and path_points[i+1]
	// if a point P is too close from the segment, we replace T by 2 tangents T1, T2, between the points path_points[i] P and P - path_points[i+1]

	Vector<Point2> tangents_result;

	const int side1 = p_point_sides[p_1];
	const int side2 = p_point_sides[p_2];

	Point2 p_tangent1;
	Point2 p_tangent2;

	// if no radius application
	if (!p_apply_radius_to_p1 && !p_apply_radius_to_p2) {
		p_tangent1 = p_1;
		p_tangent2 = p_2;
	} else if (!p_apply_radius_to_p1) { // we apply radius to p2 only
		DDLSGeom2D::tangents_point_to_circle(p_1, p_2, radius, &tangents_result);
		if (side2 == 1) { // p2 lies on the left funnel
			p_tangent1 = p_1;
			p_tangent2 = tangents_result[1];
		} else { // p2 lies on the right funnel
			p_tangent1 = p_1;
			p_tangent2 = tangents_result[0];
		}
	} else if (!p_apply_radius_to_p2) { // we apply radius to p1 only
		DDLSGeom2D::tangents_point_to_circle(p_2, p_1, radius, &tangents_result);
		if (side1 == 1) { // p1 lies on the left funnel
			p_tangent1 = tangents_result[0];
			p_tangent2 = p_2;
		} else { // p1 lies on the right funnel
			p_tangent1 = tangents_result[1];
			p_tangent2 = p_2;
		}
	} else { // we apply radius to both points
		// both points lie on left funnel
		if (side1 == 1 && side2 == 1) {
			tangents_result = DDLSGeom2D::tangents_paral_circle_to_circle(radius, p_1, p_2);
			// we keep the points of the right tangent
			p_tangent1 = tangents_result[1];
			p_tangent2 = tangents_result[2];
		} else if (side1 == -1 && side2 == -1) { // both points lie on right funnel
			tangents_result = DDLSGeom2D::tangents_paral_circle_to_circle(radius, p_1, p_2);
			// we keep the points of the left tangent
			p_tangent1 = tangents_result[0];
			p_tangent2 = tangents_result[3];
		} else if (side1 == 1 && side2 == -1) { // 1st point lies on left funnel, 2nd on right funnel
			if (DDLSGeom2D::tangents_cross_circle_to_circle(radius, p_1, p_2, &tangents_result)) {
				// we keep the points of the right-left tangent
				p_tangent1 = tangents_result[1];
				p_tangent2 = tangents_result[3];
			} else {
				// NO TANGENT BECAUSE POINTS TOO CLOSE - A* MUST CHECK THAT !
				print_verbose("WARN: No tangent - points are too close for radius");
				return;
			}
		} else { // 1st point lies on right funnel, 2nd on left funnel
			if (DDLSGeom2D::tangents_cross_circle_to_circle(radius, p_1, p_2, &tangents_result)) {
				// we keep the points of the left-right tangent
				p_tangent1 = tangents_result[0];
				p_tangent2 = tangents_result[2];
			} else {
				// NO TANGENT BECAUSE POINTS TOO CLOSE - A* MUST CHECK THAT !
				print_verbose("WARN: No tangent - points are too close for radius");
				return;
			}
		}
	}

	Point2 successor = p_point_successor[p_1];
	while (successor != p_2) {
		real_t distance = DDLSGeom2D::distance_squared_point_to_segment(successor, p_tangent1, p_tangent2);
		if (distance < radius_squared) {
			adjust_with_tangents(p_1, p_apply_radius_to_p1, successor, true, p_point_sides, p_point_successor, r_new_path, r_adjusted_points);
			adjust_with_tangents(successor, true, p_2, p_apply_radius_to_p2, p_point_sides, p_point_successor, r_new_path, r_adjusted_points);
			return;
		} else {
			successor = p_point_successor[successor];
		}
	}

	// we check distance in order to remove useless close points due to straight line subdivision
	// if ( adjusted_points.length > 0 ) {
	//   real_t distance_squared;
	//   Point2 last_point = p_adjusted_points[adjusted_points.size() - 1];
	//   distance_squared = (last_point - p_tangent1).length_squared();
	//   if (distance_squared <= EPSILON_SQUARED) {
	//     p_adjusted_points.pop();
	//     p_adjusted_points.push_back(p_tangent2);
	//     return;
	//   }
	// }
	r_adjusted_points.push_back(p_tangent1, p_tangent2);
	r_new_path.push_back(p_1);
}

void DDLS_Funnel::check_adjusted_path(Vector<Point2> &p_new_path, Vector<Point2> &p_adjusted_points, const Map<Point2, int> &p_point_sides) {
	Vector<Point2> tangents_result;
	Point2 p_tangent1;
	Point2 p_tangent2;

	bool need_check = true;
	while (need_check) {
		need_check = false;
		for (int i = 2; i < p_new_path.size(); i++) {
			const Point2 point2 = p_new_path[i];
			const int point2side = p_point_sides[point2];
			const Point2 point1 = p_new_path[i - 1];
			const int point1side = p_point_sides[point1];
			const Point2 point0 = p_new_path[i - 2];
			const int point0side = p_point_sides[point0];

			if (point1side == point2side) {
				Point2 pt1 = p_adjusted_points[(i - 2) * 2];
				Point2 pt2 = p_adjusted_points[(i - 1) * 2 - 1];
				Point2 pt3 = p_adjusted_points[(i - 1) * 2];
				real_t dot = (pt1.x - pt2.x) * (pt3.x - pt2.x) + (pt1.y - pt2.y) * (pt3.y - pt2.y);
				if (dot > 0) {
					// need_check = true;
					// rework the tangent
					if (i == 2) {
						// tangent from start point
						DDLSGeom2D::tangents_point_to_circle(point0, point2, radius, &tangents_result);
						// p2 lies on the left funnel
						if (point2side == 1) {
							p_tangent1 = point0;
							p_tangent2 = tangents_result[1];
						} else {
							p_tangent1 = point0;
							p_tangent2 = tangents_result[0];
						}
					} else if (i == p_new_path.size() - 1) {
						// tangent to end point
						DDLSGeom2D::tangents_point_to_circle(point2, point0, radius, &tangents_result);
						// p1 lies on the left funnel
						if (point0side == 1) {
							p_tangent1 = tangents_result[0];
							p_tangent2 = point2;
						} else { // p1 lies on the right funnel
							p_tangent1 = tangents_result[1];
							p_tangent2 = point2;
						}
					} else {
						// 1st point lies on left funnel, 2nd on right funnel
						if (point0side == 1 && point2side == -1) {
							DDLSGeom2D::tangents_cross_circle_to_circle(radius, point0, point2, &tangents_result);
							// we keep the points of the right-left tangent
							p_tangent1 = tangents_result[1];
							p_tangent2 = tangents_result[3];
						} else if (point0side == -1 && point2side == 1) { // 1st point lies on right funnel, 2nd on left funnel
							DDLSGeom2D::tangents_cross_circle_to_circle(radius, point0, point2, &tangents_result);
							// we keep the points of the right-left tangent
							p_tangent1 = tangents_result[0];
							p_tangent2 = tangents_result[2];
						} else if (point0side == 1 && point2side == 1) { // both points lie on left funnel
							tangents_result = DDLSGeom2D::tangents_paral_circle_to_circle(radius, point0, point2);
							// we keep the points of the right tangent
							p_tangent1 = tangents_result[1];
							p_tangent2 = tangents_result[2];
						} else if (point0side == -1 && point2side == -1) { // both points lie on right funnel
							tangents_result = DDLSGeom2D::tangents_paral_circle_to_circle(radius, point0, point2);
							// we keep the points of the right tangent
							p_tangent1 = tangents_result[0];
							p_tangent2 = tangents_result[3];
						}
					}
					p_adjusted_points.write[(i - 2) * 2] = p_tangent1; // changed from splice to
					p_adjusted_points.write[i * 2 - 1] = p_tangent2; // direct assignment

					// delete useless point
					p_new_path.remove(i - 1);
					p_adjusted_points.remove((i - 1) * 2 - 1, 2);

					tangents_result.clear();
					i--;
				}
			}
		}
	}
}

void DDLS_Funnel::smooth_angle(const Point2 &p_prev_point, const Point2 &p_point_to_smooth, const Point2 &p_next_point, int p_side, Vector<Point2> &p_encircle_points) {
	const int angle_type = DDLSGeom2D::get_direction(p_prev_point, p_point_to_smooth, p_next_point);

	// printv_verbose("smooth_angle:");
	// printv_verbose("  angle_type=", angle_type);
	// printv_verbose("  prev_point=", prev_point);
	// printv_verbose("  point_to_smooth=", point_to_smooth);
	// printv_verbose("  next_point=", next_point);

	const real_t distance_squared = (p_prev_point - p_next_point).length_squared();
	if (distance_squared <= sample_circle_distance_squared) {
		return;
	}
	int index = 0;
	for (int i = 0; i < num_samples_circle; i++) {
		bool point_in_area = false;
		Point2 to_check = p_point_to_smooth + sample_circle[i];
		const int side1 = DDLSGeom2D::get_direction(p_prev_point, p_point_to_smooth, to_check);
		const int side2 = DDLSGeom2D::get_direction(p_point_to_smooth, p_next_point, to_check);

		// if funnel left
		if (p_side == 1) {
			if (angle_type == -1) { // if angle is < 180
				if (side1 == -1 && side2 == -1)
					point_in_area = true;
			} else { // if angle is >= 180
				if (side1 == -1 || side2 == -1) {
					point_in_area = true;
				}
			}
		} else { // if funnel right
			if (angle_type == 1) { // if angle is < 180
				if (side1 == 1 && side2 == 1) {
					point_in_area = true;
				}
			} else { // if angle is >= 180
				if (side1 == 1 || side2 == 1) {
					point_in_area = true;
				}
			}
		}
		if (point_in_area) {
			p_encircle_points.insert(index, to_check);
			index++;
		} else {
			index = 0;
		}
	}
	if (p_side == -1) {
		// points in sample circle are CCW
		// so we inverse the order for right funnel
		p_encircle_points.invert();
	}
}

real_t DDLS_Funnel::get_radius() { return radius; }

void DDLS_Funnel::set_radius(real_t value) {
	radius = MAX(0, value);
	radius_squared = radius * radius;
	sample_circle.clear();
	if (radius == 0) {
		return;
	}
	for (int i = 0; i < num_samples_circle; i++) {
		sample_circle.push_back(Point2(radius * Math::cos(-2 * Math_PI * i / num_samples_circle), radius * Math::sin(-2 * Math_PI * i / num_samples_circle)));
	}
	sample_circle_distance_squared = (sample_circle[0] - sample_circle[1]).length_squared();
}

void DDLS_Funnel::find_path(Point2 p_from, Point2 p_to, Vector<DDLSFace> &p_list_faces, Vector<DDLSEdge> &p_list_edges, Vector<Point2> &r_result_path) {
	// we check the start and goal
	if (radius > 0) {
		DDLSFace check_face = p_list_faces[0];
		Point2 p1 = check_face->get_edge()->get_origin_vertex()->get_pos();
		Point2 p2 = check_face->get_edge()->get_destination_vertex()->get_pos();
		Point2 p3 = check_face->get_edge()->get_next_left_edge()->get_destination_vertex()->get_pos();
		real_t distance_squared = (p1 - p_from).length_squared();
		if (distance_squared <= radius_squared) {
			const real_t distance = Math::sqrt(distance_squared);
			p_from = radius * 1.01 * ((p_from - p1) / distance) + p1;
		} else {
			distance_squared = (p2 - p_from).length_squared();
			if (distance_squared <= radius_squared) {
				const real_t distance = Math::sqrt(distance_squared);
				p_from = radius * 1.01 * ((p_from - p2) / distance) + p2;
			} else {
				distance_squared = (p3 - p_from).length_squared();
				if (distance_squared <= radius_squared) {
					const real_t distance = Math::sqrt(distance_squared);
					p_from = radius * 1.01 * ((p_from - p3) / distance) + p3;
				}
			}
		}
		//
		check_face = p_list_faces[p_list_faces.size() - 1];
		p1 = check_face->get_edge()->get_origin_vertex()->get_pos();
		p2 = check_face->get_edge()->get_destination_vertex()->get_pos();
		p3 = check_face->get_edge()->get_next_left_edge()->get_destination_vertex()->get_pos();
		distance_squared = (p1 - p_to).length_squared();
		if (distance_squared <= radius_squared) {
			const real_t distance = Math::sqrt(distance_squared);
			p_to = radius * 1.01 * ((p_to - p1) / distance) + p1;
		} else {
			distance_squared = (p2 - p_to).length_squared();
			if (distance_squared <= radius_squared) {
				const real_t distance = Math::sqrt(distance_squared);
				p_to = radius * 1.01 * ((p_to - p2) / distance) + p2;
			} else {
				const real_t distance_squared = (p3 - p_to).length_squared();
				if (distance_squared <= radius_squared) {
					const real_t distance = Math::sqrt(distance_squared);
					p_to = radius * 1.01 * ((p_to - p3) / distance) + p3;
				}
			}
		}
	}

	// we build starting and ending points
	Point2 start_point = p_from;
	Point2 end_point = p_to;

	if (p_list_faces.size() == 1) {
		r_result_path.push_back(start_point);
		r_result_path.push_back(end_point);
		return;
	}

	if (p_list_faces.size() > 1) {
		// first we skip the first face and first edge if the starting point lies on the first interior edge:
		if (p_list_edges[0] == DDLSGeom2D::is_in_face(p_from, p_list_faces[0])) {
			p_list_faces.shift();
			p_list_faces.shift();
		}
	}
	if (p_list_faces.size() == 1) {
		r_result_path.push_back(start_point);
		r_result_path.push_back(end_point);
		return;
	}

	// our funnels, inited with starting point
	Vector<Point2> funnel_left;
	Vector<Point2> funnel_right;
	funnel_left.push_back(start_point);
	funnel_right.push_back(start_point);

	// useful to keep track of done vertices and compare the sides
	Map<DDLSVertex, int> vertices_done_side;

	// we extract the vertices positions and sides from the edges list
	Vector<Point2> points_list;
	Map<Point2, int> point_sides;
	// we keep the successor relation in a dictionnary
	Map<Point2, Point2> point_successor;
	//
	point_sides[start_point] = 0;
	// we begin with the vertices in first edge
	DDLSEdge curr_edge = p_list_edges[0];
	int relativ_pos = DDLSGeom2D::get_relative_position2(p_from, curr_edge);
	Point2 new_point_a = curr_edge->get_destination_vertex()->get_pos();
	Point2 new_point_b = curr_edge->get_origin_vertex()->get_pos();

	points_list.push_back(new_point_a);
	points_list.push_back(new_point_b);
	point_successor[start_point] = new_point_a;
	point_successor[new_point_a] = new_point_b;
	Point2 prev_point = new_point_b;
	if (relativ_pos == 1) {
		point_sides[new_point_a] = 1;
		point_sides[new_point_b] = -1;
		vertices_done_side[curr_edge->get_destination_vertex()] = 1;
		vertices_done_side[curr_edge->get_origin_vertex()] = -1;
	} else if (relativ_pos == -1) {
		point_sides[new_point_a] = -1;
		point_sides[new_point_b] = 1;
		vertices_done_side[curr_edge->get_destination_vertex()] = -1;
		vertices_done_side[curr_edge->get_origin_vertex()] = 1;
	}

	// then we iterate through the edges
	DDLSVertex from_vertex = p_list_edges[0]->get_origin_vertex();
	DDLSVertex from_from_vertex = p_list_edges[0]->get_destination_vertex();
	for (int i = 1; i < p_list_edges.size(); i++) {
		DDLSVertex curr_vertex;
		// we identify the current vertex and his origin vertex
		DDLSEdge curr_edge = p_list_edges[i];
		if (curr_edge->get_origin_vertex() == from_vertex) {
			curr_vertex = curr_edge->get_destination_vertex();
		} else if (curr_edge->get_destination_vertex() == from_vertex) {
			curr_vertex = curr_edge->get_origin_vertex();
		} else if (curr_edge->get_origin_vertex() == from_from_vertex) {
			curr_vertex = curr_edge->get_destination_vertex();
			from_vertex = from_from_vertex;
		} else if (curr_edge->get_destination_vertex() == from_from_vertex) {
			curr_vertex = curr_edge->get_origin_vertex();
			from_vertex = from_from_vertex;
		} else {
			print_verbose("WARN: Impossible to identify the vertex!");
		}

		new_point_a = curr_vertex->get_pos();
		points_list.push_back(new_point_a);
		const int direction = -vertices_done_side[from_vertex];
		point_sides[new_point_a] = direction;
		point_successor[prev_point] = new_point_a;
		vertices_done_side[curr_vertex] = direction;
		prev_point = new_point_a;
		from_from_vertex = from_vertex;
		from_vertex = curr_vertex;
	}
	// we then we add the end point
	point_successor[prev_point] = end_point;
	point_sides[end_point] = 0;

	// Point2 ppp1 = start_point;
	// while (point_successor.has(ppp1)) {
	//   Point2 ppp2 = point_successor[ppp1];
	//   debug_surface->draw_line(ppp1 + {0, 2}, ppp2 + {0, 2}, Color::from_abgr(0x0000FF));
	//   debug_surface->draw_circle(ppp2, 3);
	//   ppp1 = ppp2;
	// }
	// for (int i = 1 ; i < points_list.size() ; i++) {
	//   debug_surface->draw_line(points_list[i-1] + {2, 0}, points_list[i] + {2, 0}, Color::from_abgr(0x00FF00));
	// }

	// we will keep the points and funnel sides of the optimized path
	Vector<Point2> path_points;
	Map<Point2, int> path_sides;
	path_points.push_back(start_point);
	path_sides[start_point] = 0;

	// now we process the points by order
	for (int i = 0; i < points_list.size(); i++) {
		Point2 curr_pos = points_list[i];

		// we identify the current vertex funnel's position by the position of his origin vertex
		if (point_sides[curr_pos] == -1) {
			// current vertex is at right
			for (int j = funnel_left.size() - 2; j >= 0; j--) {
				const int direction = DDLSGeom2D::get_direction(funnel_left[j], funnel_left[j + 1], curr_pos);
				if (direction != -1) {
					// print_verbose("(find_path) funnels are crossing");
					(void)funnel_left.shift();
					for (int k = 0; k <= j - 1; k++) {
						path_points.push_back(funnel_left[0]);
						path_sides[funnel_left[0]] = 1;
						(void)funnel_left.shift();
					}
					path_points.push_back(funnel_left[0]);
					path_sides[funnel_left[0]] = 1;
					funnel_right.clear();
					funnel_right.push_back(funnel_left[0], curr_pos);
					break;
				}
			}

			funnel_right.push_back(curr_pos);
			for (int j = funnel_right.size() - 3; j >= 0; j--) {
				const int direction = DDLSGeom2D::get_direction(funnel_right[j], funnel_right[j + 1], curr_pos);
				if (direction == -1) {
					break;
				} else {
					funnel_right.remove(j + 1);
				}
			}
		} else {
			// current vertex is at left
			for (int j = funnel_right.size() - 2; j >= 0; j--) {
				const int direction = DDLSGeom2D::get_direction(funnel_right[j], funnel_right[j + 1], curr_pos);
				if (direction != 1) {
					(void)funnel_right.shift();
					for (int k = 0; k <= j - 1; k++) {
						path_points.push_back(funnel_right[0]);
						path_sides[funnel_right[0]] = -1;
						(void)funnel_right.shift();
					}
					path_points.push_back(funnel_right[0]);
					path_sides[funnel_right[0]] = -1;
					funnel_left.clear();
					funnel_left.push_back(funnel_right[0], curr_pos);
					break;
				}
			}

			funnel_left.push_back(curr_pos);
			for (int j = funnel_left.size() - 3; j >= 0; j--) {
				const int direction = DDLSGeom2D::get_direction(funnel_left[j], funnel_left[j + 1], curr_pos);
				if (direction == 1) {
					break;
				} else {
					funnel_left.remove(j + 1);
				}
			}
		}
	}

	// check if the goal is blocked by one funnel's right vertex
	bool blocked = false;
	for (int j = funnel_right.size() - 2; j >= 0; j--) {
		const int direction = DDLSGeom2D::get_direction(funnel_right[j], funnel_right[j + 1], p_to);
		if (direction != 1) {
			// access blocked
			(void)funnel_right.shift();
			for (int k = 0; k <= j; k++) {
				path_points.push_back(funnel_right[0]);
				path_sides[funnel_right[0]] = -1;
				(void)funnel_right.shift();
			}
			path_points.push_back(end_point);
			path_sides[end_point] = 0;
			blocked = true;
			break;
		}
	}

	if (!blocked) {
		// check if the goal is blocked by one funnel's left vertex
		for (int j = funnel_left.size() - 2; j >= 0; j--) {
			const int direction = DDLSGeom2D::get_direction(funnel_left[j], funnel_left[j + 1], p_to);
			if (direction != -1) {
				// access blocked
				(void)funnel_left.shift();
				for (int k = 0; k <= j; k++) {
					path_points.push_back(funnel_left[0]);
					path_sides[funnel_left[0]] = 1;
					(void)funnel_left.shift();
				}
				path_points.push_back(end_point);
				path_sides[end_point] = 0;
				blocked = true;
				break;
			}
		}
	}

	// if not blocked, we consider the direct path
	if (!blocked) {
		path_points.push_back(end_point);
		path_sides[end_point] = 0;
		blocked = true;
	}

	Vector<Point2> adjusted_points;

	// if radius is non zero
	if (radius > 0) {
		Vector<Point2> new_path;

		if (path_points.size() == 2) {
			adjust_with_tangents(path_points[0], false, path_points[1], false, point_sides, point_successor, new_path, adjusted_points);
		} else if (path_points.size() > 2) {
			// tangent from start point to 2nd point
			adjust_with_tangents(path_points[0], false, path_points[1], true, point_sides, point_successor, new_path, adjusted_points);

			// tangents for intermediate points
			if (path_points.size() > 3) {
				for (int i = 1; i <= path_points.size() - 3; i++) {
					adjust_with_tangents(path_points[i], true, path_points[i + 1], true, point_sides, point_successor, new_path, adjusted_points);
				}
			}
			// tangent from last-1 point to end point
			const int pathLength = path_points.size();
			adjust_with_tangents(path_points[pathLength - 2], true, path_points[pathLength - 1], false, point_sides, point_successor, new_path, adjusted_points);
		}

		new_path.push_back(end_point);

		// adjusted path can have useless tangents, we check it
		check_adjusted_path(new_path, adjusted_points, point_sides);

		Vector<Point2> smooth_points;
		for (int i = new_path.size() - 2; i >= 1; i--) {
			smooth_angle(adjusted_points[i * 2 - 1], new_path[i], adjusted_points[i * 2], point_sides[new_path[i]], smooth_points);
			while (smooth_points.size()) {
				adjusted_points.insert(i * 2, smooth_points.pop());
			}
		}
	} else {
		adjusted_points = path_points;
	}

	// extract coordinates
	for (int i = 0; i < adjusted_points.size(); i++) {
		r_result_path.push_back(adjusted_points[i]);
	}
}

DDLS_Funnel::DDLS_Funnel() {
	radius = 0;
	radius_squared = 0;
	num_samples_circle = 16;
	debug_surface = nullptr;
}
