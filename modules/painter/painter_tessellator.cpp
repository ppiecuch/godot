#include "painter_tessellator.h"

#include "core/math/geometry.h"

int PainterTessellator::_circle_segments(real_t p_radius) {
	int segments = int(Math::ceil(double(p_radius) * 0.5));
	return CLAMP(segments, 12, 128);
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
	if (p_points.size() < 3) {
		return mesh;
	}
	mesh.vertices = p_points;
	mesh.indices = Geometry::triangulate_polygon(Span<Vector2>(p_points.ptr(), p_points.size()));
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

void PainterTessellator::_append_round_disc(PainterMesh &p_mesh, const Vector2 &p_center, real_t p_radius) {
	p_mesh.append(filled_circle(p_center, p_radius));
}

void PainterTessellator::_append_segment_quad(PainterMesh &p_mesh, const Vector2 &p_a, const Vector2 &p_b, const Vector2 &p_normal) {
	int base = p_mesh.vertices.size();
	p_mesh.vertices.push_back(p_a + p_normal);
	p_mesh.vertices.push_back(p_b + p_normal);
	p_mesh.vertices.push_back(p_b - p_normal);
	p_mesh.vertices.push_back(p_a - p_normal);
	p_mesh.indices.push_back(base + 0);
	p_mesh.indices.push_back(base + 1);
	p_mesh.indices.push_back(base + 2);
	p_mesh.indices.push_back(base + 0);
	p_mesh.indices.push_back(base + 2);
	p_mesh.indices.push_back(base + 3);
}

// Picks the outer (convex) side of a joint between two travel directions and, for round joins,
// simply drops a filled disc over the joint — cheap, and equivalent to an exact round join once
// filled with an opaque color. Miter falls back to bevel past the miter limit, same as upstream.
void PainterTessellator::_append_join(PainterMesh &p_mesh, const Vector2 &p_joint, const Vector2 &p_dir_in, const Vector2 &p_dir_out, real_t p_half_width, PainterPen::LineJoin p_join) {
	if (p_join == PainterPen::JOIN_ROUND) {
		_append_round_disc(p_mesh, p_joint, p_half_width);
		return;
	}

	Vector2 n_in(-p_dir_in.y, p_dir_in.x);
	Vector2 n_out(-p_dir_out.y, p_dir_out.x);
	real_t turn = p_dir_in.cross(p_dir_out);
	Vector2 m_in = (turn < 0) ? n_in : -n_in;
	Vector2 m_out = (turn < 0) ? n_out : -n_out;

	Vector2 outer_in = p_joint + m_in * p_half_width;
	Vector2 outer_out = p_joint + m_out * p_half_width;

	if (p_join == PainterPen::JOIN_MITER) {
		Vector2 bisector = m_in + m_out;
		real_t bisector_len = bisector.length();
		if (bisector_len > CMP_EPSILON) {
			bisector /= bisector_len;
			real_t cos_half_angle = bisector.dot(m_in);
			const real_t miter_limit = 4.0;
			if (cos_half_angle > 0.05) {
				real_t miter_len = p_half_width / cos_half_angle;
				if (miter_len <= p_half_width * miter_limit) {
					Vector2 miter_point = p_joint + bisector * miter_len;
					int base = p_mesh.vertices.size();
					p_mesh.vertices.push_back(p_joint);
					p_mesh.vertices.push_back(outer_in);
					p_mesh.vertices.push_back(miter_point);
					p_mesh.vertices.push_back(outer_out);
					p_mesh.indices.push_back(base + 0);
					p_mesh.indices.push_back(base + 1);
					p_mesh.indices.push_back(base + 2);
					p_mesh.indices.push_back(base + 0);
					p_mesh.indices.push_back(base + 2);
					p_mesh.indices.push_back(base + 3);
					return;
				}
			}
		}
		// Falls through to bevel when the miter would be too long or the turn is degenerate.
	}

	int base = p_mesh.vertices.size();
	p_mesh.vertices.push_back(p_joint);
	p_mesh.vertices.push_back(outer_in);
	p_mesh.vertices.push_back(outer_out);
	p_mesh.indices.push_back(base + 0);
	p_mesh.indices.push_back(base + 1);
	p_mesh.indices.push_back(base + 2);
}

void PainterTessellator::_append_cap(PainterMesh &p_mesh, const Vector2 &p_end, const Vector2 &p_dir_outward, real_t p_half_width, PainterPen::LineCap p_cap) {
	if (p_cap == PainterPen::CAP_BUTT) {
		return;
	}
	if (p_cap == PainterPen::CAP_ROUND) {
		_append_round_disc(p_mesh, p_end, p_half_width);
		return;
	}

	// CAP_SQUARE: extend a half-width rectangle beyond the endpoint.
	Vector2 normal(-p_dir_outward.y, p_dir_outward.x);
	Vector2 extended = p_end + p_dir_outward * p_half_width;
	int base = p_mesh.vertices.size();
	p_mesh.vertices.push_back(p_end + normal * p_half_width);
	p_mesh.vertices.push_back(extended + normal * p_half_width);
	p_mesh.vertices.push_back(extended - normal * p_half_width);
	p_mesh.vertices.push_back(p_end - normal * p_half_width);
	p_mesh.indices.push_back(base + 0);
	p_mesh.indices.push_back(base + 1);
	p_mesh.indices.push_back(base + 2);
	p_mesh.indices.push_back(base + 0);
	p_mesh.indices.push_back(base + 2);
	p_mesh.indices.push_back(base + 3);
}

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

	int n = pts.size();
	if (n < 2 || p_width <= 0.0) {
		return mesh;
	}

	real_t half_width = p_width * 0.5;
	int segment_count = p_closed ? n : n - 1;

	Vector<Vector2> dirs;
	dirs.resize(segment_count);
	for (int i = 0; i < segment_count; i++) {
		dirs.write[i] = (pts[(i + 1) % n] - pts[i]).normalized();
	}

	for (int i = 0; i < segment_count; i++) {
		Vector2 normal(-dirs[i].y, dirs[i].x);
		_append_segment_quad(mesh, pts[i], pts[(i + 1) % n], normal * half_width);
	}

	if (p_closed) {
		for (int j = 0; j < n; j++) {
			int seg_in = (j - 1 + n) % n;
			_append_join(mesh, pts[j], dirs[seg_in], dirs[j], half_width, p_join);
		}
	} else {
		for (int j = 1; j < n - 1; j++) {
			_append_join(mesh, pts[j], dirs[j - 1], dirs[j], half_width, p_join);
		}
		_append_cap(mesh, pts[0], -dirs[0], half_width, p_cap);
		_append_cap(mesh, pts[n - 1], dirs[segment_count - 1], half_width, p_cap);
	}

	return mesh;
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
