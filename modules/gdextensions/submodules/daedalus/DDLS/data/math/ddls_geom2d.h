#pragma once

#include "core/vector.h"
#include "core/math/vector2.h"

#include "data/ddls_edge.h"
#include "data/ddls_vertex.h"
#include "data/ddls_mesh.h"
#include "data/math/ddls_rand_generator.h"

namespace DDLSGeom2D {
	static DDLSRandGenerator rand_gen;
	static Vector<DDLSVertex> samples;
	static Point2 _circumcenter;

	// return one the following, in priority order:
	// - an existant vertex (if (x, y) lies on this vertex)
	// or
	// - an existant edge (if (x, y) lies on this edge )
	// or
	// - an existant face (if (x, y) lies on this face )
	// or
	// - null if outside mesh
	// YOU SHOULD USE THIS FUNCTION ONLY FOR COORDINATES INSIDE SAFE AREA
	Variant locate_position(real_t x, real_t y, DDLSMesh mesh);
	bool is_circle_intersecting_any_constraint(real_t x, real_t y, real_t radius, DDLSMesh mesh);
	// return the relative direction from (x1,y1), to (x3,y3) through (x2, y2)
	// the function returns:
	//  0 if the path is a straight line
	//  1 if the path goes to the left
	// -1 if the path goes to the right
	int get_direction(real_t x1, real_t y1, real_t x2, real_t y2, real_t x3, real_t y3);
	// second version of getDirection. More accurate and safer version
	// return the relative direction from (x1,y1), to (x3,y3) through (x2, y2)
	// the function returns:
	//  0 if the path is a straight line
	//  1 if the path goes to the left
	// -1 if the path goes to the right
	int get_direction2(real_t x1, real_t y1, real_t x2, real_t y2, real_t x3, real_t y3);
	// eUp seen as an infinite line splits the 2D space in 2 parts (left and right),
	// the function returns:
	//   0 if the (x, y) lies on the line
	//   1 if the (x, y) lies at left
	//  -1 if the (x, y) lies at right
	int get_relative_position(real_t x, real_t y, DDLSEdge e_up);
	int get_relative_position(const Point2 &xy, DDLSEdge e_up);
	int get_relative_position2(real_t x, real_t y, DDLSEdge e_up);
	int get_relative_position2(const Point2 &xy, DDLSEdge e_up);
	// the function checks by priority:
	// - if the (x, y) lies on a vertex of the polygon, it will return this vertex
	// - if the (x, y) lies on a edge of the polygon, it will return this edge
	// - if the (x, y) lies inside the polygon, it will return the polygon
	// - if the (x, y) lies outside the polygon, it will return null
	Variant is_in_face(real_t x, real_t y, DDLSFace polygon);
	// return:
	// - true if the segment is totally or partially in the triangle
	// - false if the segment is totally outside the triangle
	bool clip_segment_by_triangle(real_t s1x, real_t s1y, real_t s2x, real_t s2y, real_t t1x, real_t t1y, real_t t2x, real_t t2y, real_t t3x, real_t t3y, Point2 *p_result1 = nullptr, Point2 *p_result2 = nullptr);
	// test if the segment intersects or lies inside the triangle
	bool is_segment_intersecting_triangle(real_t s1x, real_t s1y, real_t s2x, real_t s2y, real_t t1x, real_t t1y, real_t t2x, real_t t2y, real_t t3x, real_t t3y);
	bool is_delaunay(DDLSEdge edge);
	Point2 get_circumcenter(real_t x1, real_t y1, real_t x2, real_t y2, real_t x3, real_t y3);
	Point2 get_circumcenter(const Point2 xy1, const Point2 xy2, const Point2 xy3);
	bool intersections2segments(real_t s1p1x, real_t s1p1y, real_t s1p2x, real_t s1p2y, real_t s2p1x, real_t s2p1y, real_t s2p2x, real_t s2p2y, Point2 *pos_intersection = nullptr, Vector<real_t> *param_intersection = nullptr, bool infinite_line_mode = false);
	bool intersections2edges(DDLSEdge edge1, DDLSEdge edge2, Point2 &pos_intersection, Vector<real_t> *param_intersection = nullptr, bool infinite_line_mode = false);
	// a edge is convex if the polygon formed by the 2 faces at left and right of this edge is convex
	bool is_convex(DDLSEdge edge);
	void project_orthogonaly(Point2 vertexPos, DDLSEdge edge);
	void project_orthogonaly_on_segment(real_t px, real_t py, real_t sp1x, real_t sp1y, real_t sp2x, real_t sp2y, Point2 &result);
	// fill the result vector with 4 elements, with the form:
	//   [intersect0.x, intersect0.y, intersect1.x, intersect1.y]
	// empty if no intersection
	bool intersections2circles(real_t cx1, real_t cy1, real_t r1, real_t cx2, real_t cy2, real_t r2, Vector<real_t> *result = nullptr);
	bool intersections_segment_circle(real_t p0x, real_t p0y, real_t p1x, real_t p1y, real_t cx, real_t cy, real_t r, Vector<real_t> *result = nullptr);
	bool intersections_line_circle(real_t p0x, real_t p0y, real_t p1x, real_t p1y, real_t cx, real_t cy, real_t r, Vector<real_t> &result);
	// based on intersections2Circles method
	//   fill the result vector with 4 elements, with the form:
	//   [point_tangent1.x, point_tangent1.y, point_tangent2.x, point_tangent2.y]
	// empty if no tangent
	void tangents_point_to_circle(real_t px, real_t py, real_t cx, real_t cy, real_t r, Vector<real_t> &result);
	// <!!!> CIRCLES MUST HAVE SAME RADIUS
	bool tangents_cross_circleToCircle(real_t r, real_t c1x, real_t c1y, real_t c2x, real_t c2y, Vector<real_t> &result);
	// <!!!> CIRCLES MUST HAVE SAME RADIUS
	void tangents_paral_circle_to_circle(real_t r, real_t c1x, real_t c1y, real_t c2x, real_t c2y, Vector<real_t> &result);
	// squared distance from point p to infinite line (a, b)
	real_t distance_squared_point_to_line(real_t px, real_t py, real_t ax, real_t ay, real_t bx, real_t by);
	// squared distance from point p to finite segment [a, b]
	real_t distance_squared_point_to_segment(real_t px, real_t py, real_t ax, real_t ay, real_t bx, real_t by);
	real_t distance_squared_vertex_to_edge(DDLSVertex vertex, DDLSEdge edge);
	real_t path_length(Vector<real_t> path);
} // namespace DDLSGeom2D
