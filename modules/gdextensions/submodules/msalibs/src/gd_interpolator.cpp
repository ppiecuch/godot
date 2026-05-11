/**************************************************************************/
/*  gd_interpolator.cpp                                                   */
/**************************************************************************/

#include "gd_interpolator.h"

MSASpline2D::MSASpline2D() {}

void MSASpline2D::push(Vector2 p) {
	_interp.push_back(p);
}

void MSASpline2D::clear() {
	_interp.clear();
}

void MSASpline2D::reserve(int n) {
	_interp.reserve(n);
}

int MSASpline2D::get_size() const {
	return _interp.size();
}

Vector2 MSASpline2D::get_control_point(int i) const {
	ERR_FAIL_INDEX_V(i, _interp.size(), Vector2());
	return _interp.at(i);
}

void MSASpline2D::set_cubic(bool b) {
	_interp.setInterpolation(b ? msa::kInterpolationCubic : msa::kInterpolationLinear);
}

bool MSASpline2D::get_cubic() const {
	return _interp.getInterpolation() == msa::kInterpolationCubic;
}

void MSASpline2D::set_use_length(bool b) {
	_interp.setUseLength(b);
}

bool MSASpline2D::get_use_length() const {
	return _interp.getUseLength();
}

void MSASpline2D::set_length_subdivisions(int n) {
	_interp.setLengthSubdivisions(n);
}

int MSASpline2D::get_length_subdivisions() const {
	return _interp.getLengthSubdivisions();
}

float MSASpline2D::get_length() const {
	return _interp.getLength();
}

Vector2 MSASpline2D::sample(float t) {
	return _interp.sampleAt(t);
}

PoolVector2Array MSASpline2D::sample_array(int count) {
	PoolVector2Array out;
	if (count < 1 || _interp.size() == 0)
		return out;
	out.resize(count);
	PoolVector2Array::Write w = out.write();
	for (int i = 0; i < count; i++) {
		w[i] = _interp.sampleAt((float)i / (float)(count - 1 > 0 ? count - 1 : 1));
	}
	return out;
}

void MSASpline2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("push", "point"), &MSASpline2D::push);
	ClassDB::bind_method(D_METHOD("clear"), &MSASpline2D::clear);
	ClassDB::bind_method(D_METHOD("reserve", "n"), &MSASpline2D::reserve);
	ClassDB::bind_method(D_METHOD("get_size"), &MSASpline2D::get_size);
	ClassDB::bind_method(D_METHOD("get_control_point", "i"), &MSASpline2D::get_control_point);
	ClassDB::bind_method(D_METHOD("sample", "t"), &MSASpline2D::sample);
	ClassDB::bind_method(D_METHOD("sample_array", "count"), &MSASpline2D::sample_array);
	ClassDB::bind_method(D_METHOD("get_length"), &MSASpline2D::get_length);

	ClassDB::bind_method(D_METHOD("set_cubic", "enabled"), &MSASpline2D::set_cubic);
	ClassDB::bind_method(D_METHOD("get_cubic"), &MSASpline2D::get_cubic);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "cubic"), "set_cubic", "get_cubic");

	ClassDB::bind_method(D_METHOD("set_use_length", "enabled"), &MSASpline2D::set_use_length);
	ClassDB::bind_method(D_METHOD("get_use_length"), &MSASpline2D::get_use_length);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_length"), "set_use_length", "get_use_length");

	ClassDB::bind_method(D_METHOD("set_length_subdivisions", "n"), &MSASpline2D::set_length_subdivisions);
	ClassDB::bind_method(D_METHOD("get_length_subdivisions"), &MSASpline2D::get_length_subdivisions);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "length_subdivisions"), "set_length_subdivisions", "get_length_subdivisions");
}

// ============================================================
// Doctests
// ============================================================

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_CASE("[msalibs][MSASpline2D] linear interpolation returns endpoints") {
	MSASpline2D s;
	s.set_cubic(false);
	s.push(Vector2(0, 0));
	s.push(Vector2(100, 0));
	Vector2 at0 = s.sample(0.0f);
	Vector2 at1 = s.sample(1.0f);
	CHECK(Math::abs(at0.x) < 0.01f);
	CHECK(Math::abs(at1.x - 100.0f) < 0.01f);
}

TEST_CASE("[msalibs][MSASpline2D] cubic interpolation passes through control points") {
	MSASpline2D s;
	s.set_cubic(true);
	s.push(Vector2(0, 0));
	s.push(Vector2(10, 0));
	s.push(Vector2(20, 0));
	s.push(Vector2(30, 0));
	Vector2 at0 = s.sample(0.0f);
	Vector2 at1 = s.sample(1.0f);
	CHECK(Math::abs(at0.x) < 0.5f);
	CHECK(Math::abs(at1.x - 30.0f) < 0.5f);
}

TEST_CASE("[msalibs][MSASpline2D] sample_array returns requested count") {
	MSASpline2D s;
	s.push(Vector2(0, 0));
	s.push(Vector2(1, 0));
	s.push(Vector2(2, 0));
	s.push(Vector2(3, 0));
	PoolVector2Array pts = s.sample_array(10);
	CHECK(pts.size() == 10);
}

TEST_CASE("[msalibs][MSASpline2D] arc-length parameterization reports nonzero length") {
	MSASpline2D s;
	s.set_use_length(true);
	s.push(Vector2(0, 0));
	s.push(Vector2(100, 0));
	s.push(Vector2(200, 0));
	s.push(Vector2(300, 0));
	CHECK(s.get_length() > 0.0f);
}
#endif // DOCTEST
