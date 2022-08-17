#include "core/math/vector2.h"
#include "core/math/math_funcs.h"

#include "poly_geometry.h"

// Reference:
// ----------
// 1. https://github.com/maksmakuta/CProcessing/blob/dev/lib/PStroker.h

static const real_t miterMinAngle = 0.349066;
static const real_t roundMinAngle = 0.174533;

struct LineSegment {
	Point2 a, b;

	LineSegment operator+(const Point2 &to_add) const {
		return { a + to_add, b + to_add };
	}
	LineSegment operator-(const Point2 &to_remove) const {
		return { a - to_remove, b - to_remove };
	}
	Vector2 normal() const {
		auto dir = direction();
		return { -dir.y, dir.x };
	}
	Vector2 direction(bool normalized = true) const {
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

	LineSegment(const Point2 &a, const Point2 &b) :
			a(a), b(b) { }
	LineSegment() { }
};

struct PolySegment {
	LineSegment center, edge1, edge2;
	PolySegment(const LineSegment &center, real_t thickness) :
			center(center),
			edge1(center + (center.normal() * thickness)),
			edge2(center - (center.normal() * thickness)) { }
	PolySegment() { }
};


static PoolVector2Array create_triangle_fan(Point2 connect_to, Point2 origin, Point2 start, Point2 end, bool clockwise) {
	PoolVector2Array vertices;
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

	Vector2 start_point = start, end_point;
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

			// re-add the rotation origin to the target point
			end_point = end_point + origin;
		}

		// emit the triangle
		vertices.push_back(start_point);
		vertices.push_back(end_point);
		vertices.push_back(connect_to);

		start_point = end_point;
	}

	return vertices;
}

static PoolVector2Array create_joint(const PolySegment &segment1, const PolySegment &segment2, int joint_style, Point2 &end1, Point2 &end2, Point2 &next_start1, Point2 &next_start2, bool allow_overlap) {
	PoolVector2Array vertices;
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

	if (joint_style == VS::LINE_JOIN_MITTER && wrapped_angle < miterMinAngle) {
		joint_style = VS::LINE_JOIN_BEVEL;
	}

	if (joint_style == VS::LINE_JOIN_MITTER) {
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

		if (joint_style == VS::LINE_JOIN_BEVEL) {
			// simply connect the intersection points
			vertices.push_back(outer1->b);
			vertices.push_back(outer2->a);
			vertices.push_back(inner_sec);

		} else if (joint_style == VS::LINE_JOIN_ROUND) {
			// draw a circle between the ends of the outer edges, centered at
			// the actual point with half the line thickness as the radius
			vertices.append_array(create_triangle_fan(inner_sec, segment1.center.b, outer1->b, outer2->a, clockwise));
		}
	}

	return vertices;
}

PoolVector2Array strokify(const PoolVector2Array &contour, real_t w, VS::LineDrawMode cap, VS::LineDrawMode join, bool loop, bool allow_overlap = false) {
	PoolVector2Array data;
	w /= 2; // operate on half the thickness to make our lives easier

	// create poly segments from the points
	Vector<PolySegment> segments;
	for (size_t i = 0; i + 1 < contour.size(); i++) {
		const auto p1 = contour[i];
		const auto p2 = contour[i + 1];

		// to avoid division-by-zero errors,
		// only create a line segment for non-identical points
		if (!p1.is_equal_approx(p2)) {
			segments.push_back({ {p1, p2}, w });
		}
	}

	if (segments.empty()) {
		// handle the case of insufficient input points
		return contour;
	}

	Vector2 next_start1;
	Vector2 next_start2;
	Vector2 start1;
	Vector2 start2;
	Vector2 end1;
	Vector2 end2;

	// calculate the path's global start and end points
	auto &first_segment = segments[0];
	auto &last_segment = segments[segments.size() - 1];

	auto path_start1 = first_segment.edge1.a;
	auto path_start2 = first_segment.edge2.a;
	auto path_end1 = last_segment.edge1.b;
	auto path_end2 = last_segment.edge2.b;

	if (loop) {
		data.append_array(create_joint(last_segment, first_segment, join, path_end1, path_end2, path_start1, path_start2, allow_overlap));
	} else {
		if (cap == VS::LINE_CAP_SQUARE) {
			path_start1 = path_start1 - (first_segment.edge1.direction() * w);
			path_start2 = path_start2 - (first_segment.edge2.direction() * w);
			path_end1   = path_end1 + (last_segment.edge1.direction() * w);
			path_end2   = path_end2 + (last_segment.edge2.direction() * w);
		} else if (cap == VS::LINE_CAP_ROUND) {
			data.append_array(create_triangle_fan(first_segment.center.a, first_segment.center.a, first_segment.edge1.a, first_segment.edge2.a, false));
			data.append_array(create_triangle_fan(last_segment.center.b, last_segment.center.b, last_segment.edge1.b, last_segment.edge2.b, true));
		}
	}

	// generate mesh data for path segments
	for (size_t i = 0; i < segments.size(); i++) {
		auto &segment = segments[i];
		// calculate start
		if (i == 0) {
			// this is the first segment
			start1 = path_start1;
			start2 = path_start2;
		}
		if (i + 1 == segments.size()) {
			// this is the last segment
			end1 = path_end1;
			end2 = path_end2;
		} else {
			data.append_array(create_joint(segment, segments[i + 1], join, end1, end2, next_start1, next_start2, allow_overlap));
		}

		// emit vertices
		data.push_back(start1);
		data.push_back(start2);
		data.push_back(end1  );

		data.push_back(end1  );
		data.push_back(start2);
		data.push_back(end2  );

		start1 = next_start1;
		start2 = next_start2;
	}

	return data;
}
