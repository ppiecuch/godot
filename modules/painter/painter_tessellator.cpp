#include "painter_tessellator.h"

#include "core/math/geometry.h"

int PainterTessellator::_circle_segments(real_t p_radius) {
	int segments = int(Math::ceil(double(p_radius) * 0.5));
	return CLAMP(segments, 12, 128);
}

static real_t _polygon_signed_area(const Vector<Point2> &p_points) {
	real_t area = 0.0;
	int n = p_points.size();
	for (int i = 0; i < n; i++) {
		const Point2 &a = p_points[i];
		const Point2 &b = p_points[(i + 1) % n];
		area += a.x * b.y - b.x * a.y;
	}
	return area * 0.5;
}

// Geometry::offset_polyline_2d(..., END_JOINED) strokes a closed loop by returning the outer
// boundary and the inner-hole boundary of the resulting ring as two separate contours (Clipper
// gives holes opposite winding from their containing outer contour). Godot's ear-clipping
// triangulator, like sdl-painter's own, has no native hole support, so triangulating each
// contour independently and filling both would double-fill the ring solid instead of leaving
// it hollow. This bridges every non-largest contour into the largest one at its closest vertex
// pair with a zero-width double-back edge — the standard "keyhole" technique — producing one
// simple polygon boundary that a plain ear-clipper triangulates as "outer minus holes".
// Only correct when the smaller contours are actually holes nested in the largest one (true for
// any single-loop offset_polyline_2d result); a self-intersecting stroke wide enough to split
// into disjoint (non-nested) pieces could bridge two unrelated islands together instead.
static Vector<Point2> _bridge_contours(const Vector<Vector<Point2>> &p_contours) {
	if (p_contours.size() == 0) {
		return Vector<Point2>();
	}

	int outer_index = 0;
	real_t outer_area = Math::abs(_polygon_signed_area(p_contours[0]));
	for (int i = 1; i < p_contours.size(); i++) {
		real_t area = Math::abs(_polygon_signed_area(p_contours[i]));
		if (area > outer_area) {
			outer_area = area;
			outer_index = i;
		}
	}

	Vector<Point2> outline = p_contours[outer_index];
	for (int c = 0; c < p_contours.size(); c++) {
		if (c == outer_index || p_contours[c].size() == 0) {
			continue;
		}
		const Vector<Point2> &hole = p_contours[c];

		int best_outline_i = 0;
		int best_hole_i = 0;
		real_t best_dist = -1.0;
		for (int i = 0; i < outline.size(); i++) {
			for (int j = 0; j < hole.size(); j++) {
				real_t d = outline[i].distance_squared_to(hole[j]);
				if (best_dist < 0.0 || d < best_dist) {
					best_dist = d;
					best_outline_i = i;
					best_hole_i = j;
				}
			}
		}

		Vector<Point2> merged;
		for (int i = 0; i <= best_outline_i; i++) {
			merged.push_back(outline[i]);
		}
		for (int j = 0; j < hole.size(); j++) {
			merged.push_back(hole[(best_hole_i + j) % hole.size()]);
		}
		merged.push_back(hole[best_hole_i]);
		for (int i = best_outline_i; i < outline.size(); i++) {
			merged.push_back(outline[i]);
		}
		outline = merged;
	}
	return outline;
}

Vector<Vector2> PainterTessellator::arc_points(const Point2 &p_center, real_t p_rx, real_t p_ry, real_t p_start_deg, real_t p_sweep_deg) {
	Vector<Vector2> points;
	int full_segments = _circle_segments(MAX(p_rx, p_ry));
	int segments = MAX(2, int(Math::ceil(full_segments * Math::abs(p_sweep_deg) / 360.0)));
	for (int i = 0; i <= segments; i++) {
		real_t t = Math::deg2rad(p_start_deg + p_sweep_deg * (real_t(i) / real_t(segments)));
		points.push_back(p_center + Vector2(Math::cos(t) * p_rx, Math::sin(t) * p_ry));
	}
	return points;
}

Vector<Vector2> PainterTessellator::rounded_rect_points(const Rect2 &p_rect, real_t p_radius) {
	Vector<Vector2> points;
	real_t r = MIN(p_radius, MIN(p_rect.size.width, p_rect.size.height) * 0.5);
	real_t left = p_rect.position.x;
	real_t top = p_rect.position.y;
	real_t right = p_rect.position.x + p_rect.size.width;
	real_t bottom = p_rect.position.y + p_rect.size.height;

	if (r <= CMP_EPSILON) {
		points.push_back(Vector2(left, top));
		points.push_back(Vector2(right, top));
		points.push_back(Vector2(right, bottom));
		points.push_back(Vector2(left, bottom));
		return points;
	}

	const Vector2 corner_centers[4] = {
		Vector2(right - r, top + r),
		Vector2(right - r, bottom - r),
		Vector2(left + r, bottom - r),
		Vector2(left + r, top + r),
	};
	const real_t corner_start_deg[4] = { -90, 0, 90, 180 };

	for (int c = 0; c < 4; c++) {
		Vector<Vector2> corner = arc_points(corner_centers[c], r, r, corner_start_deg[c], 90);
		for (int i = 0; i < corner.size(); i++) {
			points.push_back(corner[i]);
		}
	}
	return points;
}

PainterMesh PainterTessellator::filled_rect(const Rect2 &p_rect) {
	PainterMesh mesh;
	Vector2 a = p_rect.position;
	Vector2 b = Vector2(p_rect.position.x + p_rect.size.width, p_rect.position.y);
	Vector2 c = p_rect.position + p_rect.size;
	Vector2 d = Vector2(p_rect.position.x, p_rect.position.y + p_rect.size.height);
	mesh.vertices.push_back(a);
	mesh.vertices.push_back(b);
	mesh.vertices.push_back(c);
	mesh.vertices.push_back(d);
	mesh.indices.push_back(0);
	mesh.indices.push_back(1);
	mesh.indices.push_back(2);
	mesh.indices.push_back(0);
	mesh.indices.push_back(2);
	mesh.indices.push_back(3);
	return mesh;
}

PainterMesh PainterTessellator::filled_circle(const Point2 &p_center, real_t p_radius) {
	return filled_ellipse(p_center, p_radius, p_radius);
}

PainterMesh PainterTessellator::filled_ellipse(const Point2 &p_center, real_t p_rx, real_t p_ry) {
	PainterMesh mesh;
	int segments = _circle_segments(MAX(p_rx, p_ry));
	mesh.vertices.push_back(p_center);
	for (int i = 0; i < segments; i++) {
		real_t t = Math::deg2rad(real_t(i) * 360.0 / real_t(segments));
		mesh.vertices.push_back(p_center + Vector2(Math::cos(t) * p_rx, Math::sin(t) * p_ry));
	}
	for (int i = 0; i < segments; i++) {
		int a = 1 + i;
		int b = 1 + ((i + 1) % segments);
		mesh.indices.push_back(0);
		mesh.indices.push_back(a);
		mesh.indices.push_back(b);
	}
	return mesh;
}

PainterMesh PainterTessellator::filled_polygon(const Vector<Vector2> &p_points) {
	PainterMesh mesh;

	// Matches sdl-painter's RemoveDuplicatePoints pre-pass: drop consecutive duplicates and,
	// for a closed contour, a trailing point that coincides with the first. Godot's own
	// Triangulate::triangulate has no such pre-pass, so a caller-supplied closed-loop point
	// list (e.g. a PainterPath subpath, which repeats the start point) would otherwise feed
	// the ear-clipper a zero-area "ear" and force its relaxed-mode fallback.
	Vector<Vector2> points;
	for (int i = 0; i < p_points.size(); i++) {
		if (points.size() == 0 || points[points.size() - 1].distance_squared_to(p_points[i]) > CMP_EPSILON) {
			points.push_back(p_points[i]);
		}
	}
	if (points.size() > 1 && points[0].distance_squared_to(points[points.size() - 1]) <= CMP_EPSILON) {
		points.remove(points.size() - 1);
	}

	if (points.size() < 3) {
		return mesh;
	}
	mesh.vertices = points;
	mesh.indices = Geometry::triangulate_polygon(Span<Vector2>(points.ptr(), points.size()));
	return mesh;
}

PainterMesh PainterTessellator::filled_pie(const Point2 &p_center, real_t p_rx, real_t p_ry, real_t p_start_deg, real_t p_sweep_deg) {
	PainterMesh mesh;
	Vector<Vector2> arc = arc_points(p_center, p_rx, p_ry, p_start_deg, p_sweep_deg);
	if (arc.size() < 2) {
		return mesh;
	}
	mesh.vertices.push_back(p_center);
	for (int i = 0; i < arc.size(); i++) {
		mesh.vertices.push_back(arc[i]);
	}
	for (int i = 1; i < arc.size(); i++) {
		mesh.indices.push_back(0);
		mesh.indices.push_back(i);
		mesh.indices.push_back(i + 1);
	}
	return mesh;
}

PainterMesh PainterTessellator::filled_chord(const Point2 &p_center, real_t p_rx, real_t p_ry, real_t p_start_deg, real_t p_sweep_deg) {
	return filled_polygon(arc_points(p_center, p_rx, p_ry, p_start_deg, p_sweep_deg));
}

PainterMesh PainterTessellator::filled_rounded_rect(const Rect2 &p_rect, real_t p_radius) {
	return filled_polygon(rounded_rect_points(p_rect, p_radius));
}

static Geometry::PolyJoinType _to_poly_join_type(PainterPen::LineJoin p_join) {
	switch (p_join) {
		case PainterPen::JOIN_MITER:
			return Geometry::JOIN_MITER;
		case PainterPen::JOIN_BEVEL:
			// Clipper's "square" join is the flat-cut corner other libraries call "bevel" —
			// there's no separate bevel join type in Geometry::PolyJoinType.
			return Geometry::JOIN_SQUARE;
		case PainterPen::JOIN_ROUND:
		default:
			return Geometry::JOIN_ROUND;
	}
}

static Geometry::PolyEndType _to_poly_end_type(PainterPen::LineCap p_cap) {
	switch (p_cap) {
		case PainterPen::CAP_SQUARE:
			return Geometry::END_SQUARE;
		case PainterPen::CAP_ROUND:
			return Geometry::END_ROUND;
		case PainterPen::CAP_BUTT:
		default:
			return Geometry::END_BUTT;
	}
}

// Strokes a polyline by computing a single continuous offset-outline polygon via Godot's own
// Clipper-backed Geometry::offset_polyline_2d, then triangulating that outline — rather than
// building one independent quad per segment plus separate join/cap patches. The quad-per-
// segment approach always leaves a small overlap on the inner (concave) side of every joint,
// regardless of join style, which double-blends visibly for semi-transparent pens; a single
// offset outline has no such overlap since it's one simple polygon. Note: Clipper's default
// miter limit is 2.0 (ClipperOffset's own default, not configurable through this wrapper),
// vs. upstream sdl-painter's 4.0 — miters fall back to the join type a little sooner here.
PainterMesh PainterTessellator::thick_polyline(const Vector<Vector2> &p_points, real_t p_width, PainterPen::LineCap p_cap, PainterPen::LineJoin p_join, bool p_closed) {
	PainterMesh mesh;

	Vector<Vector2> pts;
	for (int i = 0; i < p_points.size(); i++) {
		if (pts.size() == 0 || pts[pts.size() - 1].distance_squared_to(p_points[i]) > CMP_EPSILON) {
			pts.push_back(p_points[i]);
		}
	}
	if (p_closed && pts.size() > 1 && pts[0].distance_squared_to(pts[pts.size() - 1]) <= CMP_EPSILON) {
		pts.remove(pts.size() - 1);
	}
	if (pts.size() < 2 || p_width <= 0.0) {
		return mesh;
	}
	// A 2-point "closed" line has no interior to join; treat it as open so it gets caps.
	bool closed = p_closed && pts.size() > 2;

	Geometry::PolyJoinType join_type = _to_poly_join_type(p_join);
	Geometry::PolyEndType end_type = closed ? Geometry::END_JOINED : _to_poly_end_type(p_cap);

	Vector<Vector<Point2>> contours = Geometry::offset_polyline_2d(Span<Vector2>(pts.ptr(), pts.size()), p_width * 0.5, join_type, end_type);
	return filled_polygon(_bridge_contours(contours));
}

PainterMesh PainterTessellator::dashed_polyline(const Vector<Vector2> &p_points, real_t p_width, const PoolRealArray &p_dash, PainterPen::LineCap p_cap, PainterPen::LineJoin p_join, bool p_closed) {
	PainterMesh mesh;

	if (p_dash.size() == 0) {
		return thick_polyline(p_points, p_width, p_cap, p_join, p_closed);
	}

	Vector<Vector2> pts = p_points;
	if (p_closed && pts.size() > 1) {
		pts.push_back(pts[0]);
	}
	if (pts.size() < 2) {
		return mesh;
	}

	// Mirror an odd-length pattern over two passes, matching SVG/sdl-painter dash semantics.
	Vector<real_t> pattern;
	{
		PoolRealArray::Read r = p_dash.read();
		for (int i = 0; i < p_dash.size(); i++) {
			pattern.push_back(MAX(real_t(0.0), r[i]));
		}
	}
	if (pattern.size() % 2 == 1) {
		int count = pattern.size();
		for (int i = 0; i < count; i++) {
			pattern.push_back(pattern[i]);
		}
	}

	real_t pattern_total = 0.0;
	for (int i = 0; i < pattern.size(); i++) {
		pattern_total += pattern[i];
	}
	if (pattern_total <= CMP_EPSILON) {
		return thick_polyline(p_points, p_width, p_cap, p_join, p_closed);
	}

	int dash_index = 0;
	real_t dash_remaining = pattern[0];
	bool dash_on = true;

	Vector<Vector2> current_run;
	current_run.push_back(pts[0]);

	for (int i = 0; i < pts.size() - 1; i++) {
		Vector2 a = pts[i];
		Vector2 b = pts[i + 1];
		real_t seg_len = a.distance_to(b);
		real_t pos = 0.0;

		while (seg_len - pos > dash_remaining + real_t(CMP_EPSILON)) {
			pos += dash_remaining;
			Vector2 point = a.linear_interpolate(b, seg_len > CMP_EPSILON ? pos / seg_len : 0.0);

			if (dash_on) {
				// Closes off the run that was in progress: the transition point must be
				// appended before flushing, otherwise every "on" run except the very last
				// one gets flushed while still holding only its start point.
				current_run.push_back(point);
				if (current_run.size() >= 2) {
					mesh.append(thick_polyline(current_run, p_width, p_cap, p_join, false));
				}
				current_run.clear();
			} else {
				current_run.clear();
				current_run.push_back(point);
			}

			dash_on = !dash_on;
			dash_index = (dash_index + 1) % pattern.size();
			dash_remaining = pattern[dash_index];
		}

		dash_remaining -= (seg_len - pos);
		if (dash_on) {
			current_run.push_back(b);
		}
	}

	if (dash_on && current_run.size() >= 2) {
		mesh.append(thick_polyline(current_run, p_width, p_cap, p_join, false));
	}

	return mesh;
}
