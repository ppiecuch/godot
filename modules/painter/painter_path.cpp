#include "painter_path.h"

// Max deviation (px) a curve may have from its own chord before it's subdivided further —
// matches sdl-painter's kDefaultFlatness.
static const real_t FLATNESS_TOLERANCE = 0.25;
// Bounds worst-case recursion (2^20 segments) for degenerate/huge control polygons.
static const int MAX_BEZIER_DEPTH = 20;

// Perpendicular distance from p to the line through a-b (0 if a == b).
static real_t _point_line_distance(const Vector2 &p_point, const Vector2 &p_a, const Vector2 &p_b) {
	Vector2 ab = p_b - p_a;
	real_t len = ab.length();
	if (len <= CMP_EPSILON) {
		return p_point.distance_to(p_a);
	}
	Vector2 ap = p_point - p_a;
	return Math::abs(ab.x * ap.y - ab.y * ap.x) / len;
}

PainterPath::PainterPath() {
	has_open_subpath = false;
}

void PainterPath::_ensure_subpath() {
	if (!has_open_subpath) {
		SubPath sp;
		sp.points.append(current_point);
		subpaths.push_back(sp);
		subpath_start = current_point;
		has_open_subpath = true;
	}
}

void PainterPath::move_to(const Vector2 &p_point) {
	current_point = p_point;
	subpath_start = p_point;
	SubPath sp;
	sp.points.append(p_point);
	subpaths.push_back(sp);
	has_open_subpath = true;
}

void PainterPath::line_to(const Vector2 &p_point) {
	_ensure_subpath();
	subpaths.write[subpaths.size() - 1].points.append(p_point);
	current_point = p_point;
}

void PainterPath::_flatten_quad(const Vector2 &p0, const Vector2 &p1, const Vector2 &p2, int p_depth, PoolVector2Array &r_out) {
	if (p_depth >= MAX_BEZIER_DEPTH || _point_line_distance(p1, p0, p2) <= FLATNESS_TOLERANCE) {
		r_out.append(p2);
		return;
	}
	Vector2 p01 = p0.linear_interpolate(p1, 0.5);
	Vector2 p12 = p1.linear_interpolate(p2, 0.5);
	Vector2 p012 = p01.linear_interpolate(p12, 0.5);
	_flatten_quad(p0, p01, p012, p_depth + 1, r_out);
	_flatten_quad(p012, p12, p2, p_depth + 1, r_out);
}

void PainterPath::_flatten_cubic(const Vector2 &p0, const Vector2 &p1, const Vector2 &p2, const Vector2 &p3, int p_depth, PoolVector2Array &r_out) {
	real_t deviation = _point_line_distance(p1, p0, p3) + _point_line_distance(p2, p0, p3);
	if (p_depth >= MAX_BEZIER_DEPTH || deviation <= FLATNESS_TOLERANCE) {
		r_out.append(p3);
		return;
	}
	Vector2 p01 = p0.linear_interpolate(p1, 0.5);
	Vector2 p12 = p1.linear_interpolate(p2, 0.5);
	Vector2 p23 = p2.linear_interpolate(p3, 0.5);
	Vector2 p012 = p01.linear_interpolate(p12, 0.5);
	Vector2 p123 = p12.linear_interpolate(p23, 0.5);
	Vector2 p0123 = p012.linear_interpolate(p123, 0.5);
	_flatten_cubic(p0, p01, p012, p0123, p_depth + 1, r_out);
	_flatten_cubic(p0123, p123, p23, p3, p_depth + 1, r_out);
}

void PainterPath::quad_to(const Vector2 &p_control, const Vector2 &p_end) {
	_ensure_subpath();
	PoolVector2Array &points = subpaths.write[subpaths.size() - 1].points;
	_flatten_quad(current_point, p_control, p_end, 0, points);
	current_point = p_end;
}

void PainterPath::cubic_to(const Vector2 &p_control1, const Vector2 &p_control2, const Vector2 &p_end) {
	_ensure_subpath();
	PoolVector2Array &points = subpaths.write[subpaths.size() - 1].points;
	_flatten_cubic(current_point, p_control1, p_control2, p_end, 0, points);
	current_point = p_end;
}

void PainterPath::close() {
	if (has_open_subpath && subpaths.size() > 0) {
		subpaths.write[subpaths.size() - 1].closed = true;
		current_point = subpath_start;
	}
	has_open_subpath = false;
}

void PainterPath::clear() {
	subpaths.clear();
	current_point = Vector2();
	subpath_start = Vector2();
	has_open_subpath = false;
}

bool PainterPath::is_empty() const {
	return subpaths.size() == 0;
}

Vector2 PainterPath::get_current_point() const {
	return current_point;
}

int PainterPath::get_subpath_count() const {
	return subpaths.size();
}

PoolVector2Array PainterPath::get_subpath_points(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, subpaths.size(), PoolVector2Array());
	return subpaths[p_index].points;
}

bool PainterPath::is_subpath_closed(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, subpaths.size(), false);
	return subpaths[p_index].closed;
}

void PainterPath::_bind_methods() {
	ClassDB::bind_method(D_METHOD("move_to", "point"), &PainterPath::move_to);
	ClassDB::bind_method(D_METHOD("line_to", "point"), &PainterPath::line_to);
	ClassDB::bind_method(D_METHOD("quad_to", "control", "end"), &PainterPath::quad_to);
	ClassDB::bind_method(D_METHOD("cubic_to", "control1", "control2", "end"), &PainterPath::cubic_to);
	ClassDB::bind_method(D_METHOD("close"), &PainterPath::close);
	ClassDB::bind_method(D_METHOD("clear"), &PainterPath::clear);
	ClassDB::bind_method(D_METHOD("is_empty"), &PainterPath::is_empty);
	ClassDB::bind_method(D_METHOD("get_current_point"), &PainterPath::get_current_point);
	ClassDB::bind_method(D_METHOD("get_subpath_count"), &PainterPath::get_subpath_count);
	ClassDB::bind_method(D_METHOD("get_subpath_points", "index"), &PainterPath::get_subpath_points);
	ClassDB::bind_method(D_METHOD("is_subpath_closed", "index"), &PainterPath::is_subpath_closed);
}
