#include "painter_path.h"

// Approximation of sdl-painter's Path::SegmentsForCurve curvature-bound formula: pick a
// segment count from the control polygon length, clamped to a sane range.
static int _segments_for_length(real_t p_length) {
	const real_t flatness = 0.25; // px, matches sdl-painter's kDefaultFlatness
	int segments = int(Math::ceil(Math::sqrt(double(p_length) / double(flatness))));
	return CLAMP(segments, 4, 64);
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

void PainterPath::quad_to(const Vector2 &p_control, const Vector2 &p_end) {
	_ensure_subpath();
	Vector2 p0 = current_point;
	real_t poly_len = p0.distance_to(p_control) + p_control.distance_to(p_end);
	int segments = _segments_for_length(poly_len);
	PoolVector2Array &points = subpaths.write[subpaths.size() - 1].points;
	for (int i = 1; i <= segments; i++) {
		real_t t = real_t(i) / real_t(segments);
		real_t mt = 1.0 - t;
		Vector2 point = p0 * (mt * mt) + p_control * (2.0 * mt * t) + p_end * (t * t);
		points.append(point);
	}
	current_point = p_end;
}

void PainterPath::cubic_to(const Vector2 &p_control1, const Vector2 &p_control2, const Vector2 &p_end) {
	_ensure_subpath();
	Vector2 p0 = current_point;
	real_t poly_len = p0.distance_to(p_control1) + p_control1.distance_to(p_control2) + p_control2.distance_to(p_end);
	int segments = _segments_for_length(poly_len);
	PoolVector2Array &points = subpaths.write[subpaths.size() - 1].points;
	for (int i = 1; i <= segments; i++) {
		real_t t = real_t(i) / real_t(segments);
		real_t mt = 1.0 - t;
		Vector2 point = p0 * (mt * mt * mt) + p_control1 * (3.0 * mt * mt * t) + p_control2 * (3.0 * mt * t * t) + p_end * (t * t * t);
		points.append(point);
	}
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
