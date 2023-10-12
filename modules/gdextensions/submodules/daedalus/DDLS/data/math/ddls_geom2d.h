/**************************************************************************/
/*  ddls_geom2d.h                                                         */
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

#include "core/math/vector2.h"
#include "core/vector.h"

#include "data/ddls_edge.h"
#include "data/ddls_mesh.h"
#include "data/ddls_vertex.h"
#include "data/math/ddls_rand_generator.h"

namespace DDLSGeom2D {
// return one the following, in priority order:
// - an existant vertex (if (x, y) lies on this vertex)
// or
// - an existant edge (if (x, y) lies on this edge )
// or
// - an existant face (if (x, y) lies on this face )
// or
// - null if outside mesh
// YOU SHOULD USE THIS FUNCTION ONLY FOR COORDINATES INSIDE SAFE AREA
Variant locate_position(const Point2 &p_pos, DDLSMesh p_mesh);
bool is_circle_intersecting_any_constraint(const Point2 &p_pos, real_t p_radius, DDLSMesh p_mesh);
// return the relative direction from (x1,y1), to (x3,y3) through (x2, y2)
// the function returns:
//  0 if the path is a straight line
//  1 if the path goes to the left
// -1 if the path goes to the right
int get_direction(const Point2 &p_1, const Point2 &p_2, const Point2 &p_3);
// second version of get_direction. More accurate and safer version
// return the relative direction from (x1,y1), to (x3,y3) through (x2, y2)
// the function returns:
//  0 if the path is a straight line
//  1 if the path goes to the left
// -1 if the path goes to the right
int get_direction2(const Point2 &p_1, const Point2 &p_2, const Point2 &p_3);
// e_up seen as an infinite line splits the 2D space in 2 parts (left and right),
// the function returns:
//   0 if the (x, y) lies on the line
//   1 if the (x, y) lies at left
//  -1 if the (x, y) lies at right
int get_relative_position(const Point2 &p_pos, DDLSEdge p_e_up);
int get_relative_position2(const Point2 &p_pos, DDLSEdge p_e_up);
// the function checks by priority:
// - if the (x, y) lies on a vertex of the polygon, it will return this vertex
// - if the (x, y) lies on a edge of the polygon, it will return this edge
// - if the (x, y) lies inside the polygon, it will return the polygon
// - if the (x, y) lies outside the polygon, it will return null
Variant is_in_face(const Point2 &p_pos, DDLSFace p_polygon);
// return:
// - true if the segment is totally or partially in the triangle
// - false if the segment is totally outside the triangle
bool clip_segment_by_triangle(const Point2 &s1, const Point2 &s2, const Point2 &t1, const Point2 &t2, const Point2 &t3, Point2 &r_result1, Point2 &r_result2);
// test if the segment intersects or lies inside the triangle
bool is_segment_intersecting_triangle(const Point2 &s1, const Point2 &s2, const Point2 &t1, const Point2 &t2, const Point2 &t3);
bool is_delaunay(DDLSEdge edge);
Point2 get_circumcenter(const Point2 &p1, const Point2 &p2, const Point2 &p3);
bool intersections2segments(const Point2 &s1p1, const Point2 &s1p2, const Point2 &s2p1, const Point2 &s2p2, Point2 *r_pos_intersection = nullptr, Vector<real_t> *r_param_intersection = nullptr, bool infinite_line_mode = false);
bool intersections2edges(DDLSEdge edge1, DDLSEdge edge2, Point2 *r_pos_intersection = nullptr, Vector<real_t> *r_param_intersection = nullptr, bool infinite_line_mode = false);
// a edge is convex if the polygon formed by the 2 faces at left and right of this edge is convex
bool is_convex(DDLSEdge edge);
Point2 project_orthogonaly(const Point2 &p_vertex_pos, DDLSEdge p_edge);
Point2 project_orthogonaly_on_segment(const Point2 &p_pos, const Point2 &p_sp1, const Point2 &p_sp2);
// fill the result vector with 2 elements, with the form:
//   [intersect0, intersect1]
// empty if no intersection
bool intersections2circles(const Point2 &c1, real_t r1, const Point2 &c2, real_t r2, Vector<Point2> *r_result = nullptr);
bool intersections_segment_circle(const Point2 &p0, const Point2 &p1, const Point2 &cc, real_t r, Vector<real_t> *p_result = nullptr);
bool intersections_line_circle(const Point2 &p0, const Point2 &p1, const Point2 &cc, real_t r, Vector<real_t> *r_result = nullptr);
// based on intersections2circles method
//   fill the result vector with 2 elements, with the form:
//   [point_tangent1, point_tangent2]
// empty if no tangent
bool tangents_point_to_circle(const Point2 &p, const Point2 &cc, real_t r, Vector<Point2> *r_result = nullptr);
// <!!!> CIRCLES MUST HAVE SAME RADIUS
bool tangents_cross_circle_to_circle(real_t r, const Point2 &c1, const Point2 &c2, Vector<Point2> *r_result = nullptr);
// <!!!> CIRCLES MUST HAVE SAME RADIUS
Vector<Point2> tangents_paral_circle_to_circle(real_t r, const Point2 &c1, const Point2 &c2);
// squared distance from point p to infinite line (a, b)
real_t distance_squared_point_to_line(const Point2 &p, const Point2 &a, const Point2 &b);
// squared distance from point p to finite segment [a, b]
real_t distance_squared_point_to_segment(const Point2 &p, const Point2 &a, const Point2 &b);
real_t distance_squared_vertex_to_edge(DDLSVertex vertex, DDLSEdge edge);
real_t path_length(Vector<real_t> p_path);
} // namespace DDLSGeom2D
