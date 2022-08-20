/*************************************************************************/
/*  poly_geometry.cpp                                                    */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "core/math/math_funcs.h"
#include "core/math/vector2.h"

#include "poly_geometry.h"

// Reference:
// ----------
// 1. https://github.com/maksmakuta/CProcessing/blob/dev/lib/PStroker.h

static const real_t miterMinAngle = 0.349066;
static const real_t roundMinAngle = 0.174533;

//
// (a)--- line ---(b)
//
struct LineSegment {
	Point2 a, b;

	_FORCE_INLINE_ LineSegment operator+(const Point2 &to_add) const {
		return { a + to_add, b + to_add };
	}
	_FORCE_INLINE_ LineSegment operator-(const Point2 &to_remove) const {
		return { a - to_remove, b - to_remove };
	}
	_FORCE_INLINE_ Vector2 normal() const {
		auto dir = direction();
		return { -dir.y, dir.x };
	}
	_FORCE_INLINE_ Vector2 direction(bool normalized = true) const {
		auto vec = b - a;
		return normalized ? vec.normalized() : vec;
	}

	static Variant intersection(const LineSegment &a, const LineSegment &b, bool infinite_lines) {
		// calculate un-normalized direction vectors
		const auto r = a.direction(false);
		const auto s = b.direction(false);

		const auto origin_dist = b.a - a.a;

		const auto numerator = origin_dist.cross(r);
		const auto denominator = r.cross(s);

		if (Math::abs(denominator) < 0.0001) {
			return Variant(); // The lines are parallel
		}
		// solve the intersection positions
		auto u = numerator / denominator;
		auto t = origin_dist.cross(s) / denominator;

		if (!infinite_lines && (t < 0 || t > 1 || u < 0 || u > 1)) {
			return Variant(); // the intersection lies outside of the line segments
		}
		return a.a + (r * t); // calculate the intersection point
	}

	static Point2 intersection(const LineSegment &a, const LineSegment &b, const Point2 &def, bool infinite_lines) {
		if (const Variant sec = intersection(a, b, infinite_lines)) {
			return sec;
		} else {
			return def;
		}
	}

	LineSegment(const Point2 &a, const Point2 &b) :
			a(a), b(b) {}
	LineSegment() {}
};

//
// +----edge1----+
// |             | thickness
// +--- center --+
// |             | thickness
// +----edge2----+
//
struct PolySegment {
	LineSegment center, edge1, edge2;
	PolySegment(const LineSegment &center, real_t thickness) :
			center(center),
			edge1(center + (center.normal() * thickness)),
			edge2(center - (center.normal() * thickness)) {}
	PolySegment() {}
};

static void create_triangle_fan(PolyGeometry::Results &state, const Point2 &connect_to, const Point2 &origin, const Point2 &start, const Point2 &end, bool antialiased, bool clockwise) {
	const auto point1 = start - origin;
	const auto point2 = end - origin;

	// calculate the angle between the two points
	auto angle1 = Math::atan2(point1.y, point1.x);
	auto angle2 = Math::atan2(point2.y, point2.x);

	// ensure the outer angle is calculated
	if (clockwise) {
		if (angle2 > angle1) {
			angle2 = angle2 - 2 * Math_PI;
		}
	} else {
		if (angle1 > angle2) {
			angle1 = angle1 - 2 * Math_PI;
		}
	}

	const auto joint_angle = angle2 - angle1;
	const auto num_triangles = MAX(1, (int)Math::floor(Math::abs(joint_angle) / roundMinAngle)); // calculate the amount of triangles to use for the joint
	const auto tri_angle = joint_angle / num_triangles; // calculate the angle of each triangle

	Point2 start_point = start, end_point;
	for (int t = 0; t < num_triangles; t++) {
		if (t + 1 == num_triangles) {
			// it's the last triangle - ensure it perfectly
			// connects to the next line
			end_point = end;
		} else {
			auto rot = (t + 1) * tri_angle;

			// rotate the original point around the origin
			end_point.x = Math::cos(rot) * point1.x - Math::sin(rot) * point1.y;
			end_point.y = Math::sin(rot) * point1.x + Math::cos(rot) * point1.y;

			end_point = end_point + origin; // re-add the rotation origin to the target point
		}
		state.tris.push_back(start_point, end_point, connect_to); // emit the triangle
		if (antialiased) {
			state.lines.push_back(start_point, end_point);
		}
		start_point = end_point;
	}
}

static void create_joint(PolyGeometry::Results &state, const PolySegment &segment1, const PolySegment &segment2, int joint_style, Point2 &end1, Point2 &end2, Point2 &next_start1, Point2 &next_start2, bool antialiased, bool allow_overlap) {
	// calculate the angle between the two line segments
	auto dir1 = segment1.center.direction();
	auto dir2 = segment2.center.direction();

	const auto angle = dir1.angle_to(dir2);

	// wrap the angle around the 180° mark if it exceeds 90°
	// for minimum angle detection
	auto wrapped_angle = angle;
	if (wrapped_angle > Math_PI / 2) {
		wrapped_angle = Math_PI - wrapped_angle;
	}

	if (joint_style == PolyGeometry::LINE_JOIN_MITTER && wrapped_angle < miterMinAngle) {
		joint_style = PolyGeometry::LINE_JOIN_BEVEL;
	}

	if (joint_style == PolyGeometry::LINE_JOIN_MITTER) {
		// calculate each edge's intersection point
		// with the next segment's central line
		auto sec1 = LineSegment::intersection(segment1.edge1, segment2.edge1, true);
		auto sec2 = LineSegment::intersection(segment1.edge2, segment2.edge2, true);

		end1 = sec1 ? (Point2)sec1 : segment1.edge1.b;
		end2 = sec2 ? (Point2)sec2 : segment1.edge2.b;

		next_start1 = end1;
		next_start2 = end2;

	} else {
		// joint style is either BEVEL or ROUND

		// find out which are the inner edges for this joint
		const auto x1 = dir1.x;
		const auto x2 = dir2.x;
		const auto y1 = dir1.y;
		const auto y2 = dir2.y;

		const auto clockwise = x1 * y2 - x2 * y1 < 0;

		const LineSegment *inner1, *inner2, *outer1, *outer2;

		// as the normal vector is rotated counter-clockwise,
		// the first edge lies to the left
		// from the central line's perspective,
		// and the second one to the right.
		if (clockwise) {
			outer1 = &segment1.edge1;
			outer2 = &segment2.edge1;
			inner1 = &segment1.edge2;
			inner2 = &segment2.edge2;
		} else {
			outer1 = &segment1.edge2;
			outer2 = &segment2.edge2;
			inner1 = &segment1.edge1;
			inner2 = &segment2.edge1;
		}

		// calculate the intersection point of the inner edges
		const auto inner_sec_opt = LineSegment::intersection(*inner1, *inner2, allow_overlap);

		auto inner_sec = inner_sec_opt
				? (Point2)inner_sec_opt
				// for parallel lines, simply connect them directly
				: inner1->b;

		// if there's no inner intersection, flip
		// the next start position for near-180° turns
		Vector2 inner_start;
		if (inner_sec_opt) {
			inner_start = inner_sec;
		} else if (angle > Math_PI / 2) {
			inner_start = outer1->b;
		} else {
			inner_start = inner1->b;
		}

		if (clockwise) {
			end1 = outer1->b;
			end2 = inner_sec;

			next_start1 = outer2->a;
			next_start2 = inner_start;
		} else {
			end1 = inner_sec;
			end2 = outer1->b;

			next_start1 = inner_start;
			next_start2 = outer2->a;
		}

		// connect the intersection points according to the joint style

		if (joint_style == PolyGeometry::LINE_JOIN_BEVEL) {
			state.tris.push_back(outer1->b, outer2->a, inner_sec); // simply connect the intersection points
			if (antialiased) {
				state.lines.push_back(outer1->b, outer2->a);
			}
		} else if (joint_style == PolyGeometry::LINE_JOIN_ROUND) {
			// draw a circle between the ends of the outer edges, centered at
			// the actual point with half the line thickness as the radius
			create_triangle_fan(state, inner_sec, segment1.center.b, outer1->b, outer2->a, antialiased, clockwise);
		}
	}
}

PolyGeometry::Results PolyGeometry::strokify_polyline(const Vector<Point2> &p_contour, const Vector<Color> &p_colors, real_t p_width, LineDrawMode p_line_join, LineDrawMode p_line_cap, bool p_loop, bool p_antialiased, bool p_allow_overlap) {
	const real_t w = p_width / 2; // operate on half the thickness to make our lives easier

	// create poly segments from the points
	Vector<PolySegment> segments;
	for (size_t i = 0; i + 1 < p_contour.size(); i++) {
		const auto p1 = p_contour[i];
		const auto p2 = p_contour[i + 1];

		// to avoid division-by-zero errors,
		// only create a line segment for non-identical points
		if (!p1.is_equal_approx(p2)) {
			segments.push_back({ { p1, p2 }, p_width });
		}
	}

	if (segments.empty()) {
		return Results(); // handle the case of insufficient input points
	}

	Results r;

	if (p_colors.size() == 0) {
		r.tri_colors.push_back(Color(1, 1, 1, 1));
		if (p_antialiased) {
			r.line_colors.push_back(Color(1, 1, 1, 1));
		}
	} else if (p_colors.size() == 1) {
		r.tri_colors = p_colors;
		r.line_colors = p_colors;
	} else {
		if (p_colors.size() != p_contour.size()) {
			r.tri_colors.push_back(p_colors[0]);
			r.line_colors.push_back(p_colors[0]);
		}
	}

	Point2 next_start1, next_start2;
	Point2 start1, start2;
	Point2 end1, end2;

	// calculate the path's global start and end points
	const auto &first_segment = segments[0];
	const auto &last_segment = segments[segments.size() - 1];

	auto path_start1 = first_segment.edge1.a;
	auto path_start2 = first_segment.edge2.a;
	auto path_end1 = last_segment.edge1.b;
	auto path_end2 = last_segment.edge2.b;

	if (p_loop) {
		create_joint(r, last_segment, first_segment, p_line_join, path_end1, path_end2, path_start1, path_start2, p_antialiased, p_allow_overlap);
	} else {
		if (p_line_cap == LINE_CAP_SQUARE) {
			if (p_antialiased) {
				r.lines.push_back(path_start1, path_start2);
				r.lines.push_back(path_end1, path_end2);
			}
		} else if (p_line_cap == LINE_CAP_PROJECT) {
			path_start1 = path_start1 - (first_segment.edge1.direction() * w);
			path_start2 = path_start2 - (first_segment.edge2.direction() * w);
			path_end1 = path_end1 + (last_segment.edge1.direction() * w);
			path_end2 = path_end2 + (last_segment.edge2.direction() * w);
			if (p_antialiased) {
				r.lines.push_back(path_start1, path_start2);
				r.lines.push_back(path_end1, path_end2);
			}
		} else if (p_line_cap == LINE_CAP_ROUND) {
			create_triangle_fan(r, first_segment.center.a, first_segment.center.a, first_segment.edge1.a, first_segment.edge2.a, p_antialiased, false);
			create_triangle_fan(r, last_segment.center.b, last_segment.center.b, last_segment.edge1.b, last_segment.edge2.b, p_antialiased, true);
		}
	}

	// generate mesh data for path segments
	for (size_t i = 0; i < segments.size(); i++) {
		const auto &segment = segments[i];
		// calculate start
		if (i == 0) {
			// this is the first segment
			start1 = path_start1;
			start2 = path_start2;
		}
		if (i + 1 == segments.size()) { // this is the last segment
			end1 = path_end1;
			end2 = path_end2;
		} else {
			create_joint(r, segment, segments[i + 1], p_line_join, end1, end2, next_start1, next_start2, p_antialiased, p_allow_overlap);
		}

		// emit triangles
		r.tris.push_back(start1, start2, end1);
		r.tris.push_back(end1, start2, end2);

		// emit outlines
		if (p_antialiased) {
			r.lines.push_back(start1, end1);
			r.lines.push_back(start2, end2);
		}

		start1 = next_start1;
		start2 = next_start2;
	}

	return r;
}

PolyGeometry::Results PolyGeometry::strokify_multiline(const Vector<Point2> &p_lines, const Vector<Color> &p_colors, real_t p_width, LineDrawMode p_line_cap, bool p_antialiased, bool p_allow_overlap) {
	const real_t w = p_width / 2; // operate on half the thickness to make our lives easier

	// create poly segments from the points
	Vector<PolySegment> segments;
	for (size_t i = 0; i + 1 < p_lines.size(); i += 2) {
		const auto p1 = p_lines[i];
		const auto p2 = p_lines[i + 1];

		// to avoid division-by-zero errors,
		// only create a line segment for non-identical points
		if (!p1.is_equal_approx(p2)) {
			segments.push_back({ { p1, p2 }, w });
		}
	}

	if (segments.empty()) {
		return Results(); // handle the case of insufficient input points
	}

	Results r;

	if (p_colors.size() == 0) {
		r.tri_colors.push_back(Color(1, 1, 1, 1));
		if (p_antialiased) {
			r.line_colors.push_back(Color(1, 1, 1, 1));
		}
	} else if (p_colors.size() == 1) {
		r.tri_colors = p_colors;
		r.line_colors.push_back(Color(1, 0, 0, 1));
	} else {
		if (p_colors.size() != p_lines.size()) {
			r.tri_colors.push_back(p_colors[0]);
			r.line_colors.push_back(p_colors[0]);
		}
	}

	Point2 next_start1, next_start2;
	Point2 start1, start2;
	Point2 end1, end2;

	// generate mesh data for path segments
	for (size_t i = 0; i < segments.size(); i++) {
		const auto &segment = segments[i];

		auto path_start1 = segment.edge1.a;
		auto path_start2 = segment.edge2.a;
		auto path_end1 = segment.edge1.b;
		auto path_end2 = segment.edge2.b;

		if (p_line_cap == LINE_CAP_SQUARE) {
			if (p_antialiased) {
				r.lines.push_back(path_start1, path_start2);
				r.lines.push_back(path_end1, path_end2);
			}
		} else if (p_line_cap == LINE_CAP_PROJECT) {
			path_start1 = path_start1 - (segment.edge1.direction() * w);
			path_start2 = path_start2 - (segment.edge2.direction() * w);
			path_end1 = path_end1 + (segment.edge1.direction() * w);
			path_end2 = path_end2 + (segment.edge2.direction() * w);
			if (p_antialiased) {
				r.lines.push_back(path_start1, path_start2);
				r.lines.push_back(path_end1, path_end2);
			}
		} else if (p_line_cap == LINE_CAP_ROUND) {
			create_triangle_fan(r, segment.center.a, segment.center.a, segment.edge1.a, segment.edge2.a, p_antialiased, false);
			create_triangle_fan(r, segment.center.b, segment.center.b, segment.edge1.b, segment.edge2.b, p_antialiased, true);
		}

		// emit triangles
		r.tris.push_back(start1, start2, end1);
		r.tris.push_back(end1, start2, end2);

		// emit outlines
		if (p_antialiased) {
			r.lines.push_back(start1, end1);
			r.lines.push_back(start2, end2);
		}

		start1 = next_start1;
		start2 = next_start2;
	}

	return r;
}
