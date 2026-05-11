// MSAInterpolator - Catmull-Rom / linear spline interpolation
//
// Usage:
//   msa::InterpolatorT<real_t>   myInterp1D;
//   msa::InterpolatorT<Vector2>  myInterp2D;
//   msa::InterpolatorT<Vector3>  myInterp3D;
//
// or use the preset typedefs:
//   msa::Interpolator1D  interp1;   // real_t spline
//   msa::Interpolator2D  interp2;   // Vector2 spline
//   msa::Interpolator3D  interp3;   // Vector3 spline
//
//   interp.push_back(data);          // append a control point
//   interp.sampleAt(t);              // sample along 0..1 of the spline
//   interp.setInterpolation(i);      // kInterpolationLinear or kInterpolationCubic
//   interp.setUseLength(true);       // uniform arc-length parameterization
//   interp.getLength();              // total arc length (only when useLength is true)

#ifndef MSAINTERPOLATOR_H
#define MSAINTERPOLATOR_H

#include "core/math/math_defs.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"

#include <vector>

namespace msa {

// Forward declarations so two-phase template lookup finds them at definition time.
_FORCE_INLINE_ real_t lengthOf(real_t f);
inline real_t lengthOf(const Vector2 &v);
_FORCE_INLINE_ real_t lengthOf(const Vector3 &v);

typedef enum {
	kInterpolationLinear,
	kInterpolationCubic,
} InterpolationType;

template <typename T>
class InterpolatorT {
protected:
	InterpolationType _interpolationMethod;
	bool _useLength;
	int _lengthSubdivisions;
	std::vector<T> _data;
	std::vector<real_t> _dist; // cumulative arc lengths at each control point

	real_t calcSegmentLength(int i);
	void updateAllLengths();
	void findPosition(real_t t, int &leftIndex, real_t &mu);

	T linearInterpolate(const T &y1, const T &y2, real_t mu);

	// Catmull-Rom cubic: http://local.wasp.uwa.edu.au/~pbourke/miscellaneous/interpolation/
	T cubicInterpolate(const T &y0, const T &y1, const T &y2, const T &y3, real_t mu);

public:
	bool verbose;

	T sampleAt(real_t t);

	void setInterpolation(InterpolationType i = kInterpolationCubic);
	int getInterpolation();

	void setUseLength(bool b);
	bool getUseLength();

	// Returns cumulative length up to data point i, or total length if i == -1.
	// Only valid when setUseLength(true) has been called.
	const real_t getLength(int i = -1);

	void setLengthSubdivisions(int i = 100);
	int getLengthSubdivisions();

	// Container wrappers
	void push_back(const T &newData);
	int size();
	void reserve(int i);
	void clear();
	const T &at(int i);
	std::vector<T> getData();

	InterpolatorT();
};

//----------------------------------------------------------------------------

template <typename T>
InterpolatorT<T>::InterpolatorT() {
	setInterpolation();
	setUseLength(false);
	setLengthSubdivisions();
	verbose = false;
}

template <typename T>
T InterpolatorT<T>::sampleAt(real_t t) {
	int numItems = size();
	if (numItems == 0) {
		return T();
	}

	t = CLAMP(t, 0.0f, 1.0f);

	int i0, i1, i2, i3;
	real_t mu;

	findPosition(t, i1, mu);

	InterpolationType it = _interpolationMethod;
	if (numItems < 4)
		it = kInterpolationLinear;

	switch (it) {
		case kInterpolationCubic:
			i0 = i1 - 1;
			i2 = i1 + 1;
			i3 = i2 + 1;

			if (i0 < 0)
				i0 = 0;
			if (i3 >= numItems)
				i3 = numItems - 1;

			return cubicInterpolate(at(i0), at(i1), at(i2), at(i3), mu);

		case kInterpolationLinear:
			i2 = i1 + 1;
			if (i2 >= numItems)
				i2 = numItems - 1;
			return linearInterpolate(at(i1), at(i2), mu);
	}
	return T();
}

template <typename T>
void InterpolatorT<T>::setInterpolation(InterpolationType i) {
	_interpolationMethod = i;
	updateAllLengths();
}

template <typename T>
int InterpolatorT<T>::getInterpolation() { return _interpolationMethod; }

template <typename T>
void InterpolatorT<T>::setUseLength(bool b) {
	_useLength = b;
	if (_useLength) {
		updateAllLengths();
	} else {
		_dist.clear();
	}
}

template <typename T>
bool InterpolatorT<T>::getUseLength() { return _useLength; }

template <typename T>
const real_t InterpolatorT<T>::getLength(int i) {
	if (_useLength) {
		return i < 0 ? _dist[_dist.size() - 1] : _dist.at(i);
	} else {
		return 0;
	}
}

template <typename T>
void InterpolatorT<T>::setLengthSubdivisions(int i) { _lengthSubdivisions = i; }

template <typename T>
int InterpolatorT<T>::getLengthSubdivisions() { return _lengthSubdivisions; }

template <typename T>
void InterpolatorT<T>::push_back(const T &newData) {
	_data.push_back(newData);

	if (getUseLength()) {
		real_t segmentLength;
		real_t totalLength;

		if (size() > 1) {
			segmentLength = calcSegmentLength(size() - 1);
			totalLength = segmentLength + _dist.at(size() - 2);
		} else {
			segmentLength = 0;
			totalLength = 0;
		}

		_dist.push_back(totalLength);
	}
}

template <typename T>
int InterpolatorT<T>::size() { return _data.size(); }

template <typename T>
void InterpolatorT<T>::reserve(int i) {
	_data.reserve(i);
	_dist.reserve(i);
}

template <typename T>
void InterpolatorT<T>::clear() {
	_data.clear();
	_dist.clear();
}

template <typename T>
const T &InterpolatorT<T>::at(int i) { return _data.at(CLAMP(i, 0, size() - 1)); }

template <typename T>
std::vector<T> InterpolatorT<T>::getData() { return _data; }

template <typename T>
real_t InterpolatorT<T>::calcSegmentLength(int i) {
	int numItems = size();

	if (numItems < 2 || i < 1 || i >= numItems)
		return 0;

	bool saveUseLength = _useLength;
	_useLength = false;

	real_t startPerc = (i - 1) * 1.0f / (numItems - 1);
	real_t endPerc = (i) * 1.0f / (numItems - 1);
	real_t incPerc = (endPerc - startPerc) / _lengthSubdivisions;

	T prev = sampleAt(startPerc);
	T cur;

	real_t segmentLength = 0;
	for (real_t f = startPerc; f <= endPerc; f += incPerc) {
		cur = sampleAt(f);
		segmentLength += lengthOf(cur - prev);
		prev = cur;
	}

	_useLength = saveUseLength;

	return segmentLength;
}

template <typename T>
void InterpolatorT<T>::updateAllLengths() {
	_dist.clear();

	real_t curTotal = 0;

	for (int i = 0; i < size(); i++) {
		curTotal += calcSegmentLength(i);
		_dist.push_back(curTotal);
	}
}

template <typename T>
void InterpolatorT<T>::findPosition(real_t t, int &leftIndex, real_t &mu) {
	int numItems = size();

	switch (numItems) {
		case 0:
		case 1:
			leftIndex = 0;
			mu = 0;
			break;

		case 2:
			leftIndex = 0;
			mu = t;
			break;

		default:
			if (_useLength) {
				real_t totalLength = _dist.at(numItems - 1);
				real_t tDist = totalLength * t;
				int i1 = CLAMP((int)(t * (numItems - 1)), 0, numItems - 1);
				int limitLeft = 0;
				int limitRight = numItems - 1;

				for (int iter = 0; iter < 100; iter++) {
					real_t distAt1 = _dist.at(i1);
					if (distAt1 <= tDist) {
						real_t distAt2 = _dist.at(CLAMP(i1 + 1, 0, (int)_dist.size() - 1));
						if (distAt2 > tDist) {
							leftIndex = i1;
							mu = (tDist - distAt1) / (distAt2 - distAt1);
							return;
						} else {
							limitLeft = i1;
						}
					} else {
						limitRight = i1;
					}
					i1 = (limitLeft + limitRight) >> 1;
				}

			} else {
				real_t actT = t * (numItems - 1);
				leftIndex = (int)Math::floor(actT);
				mu = actT - leftIndex;
			}
	}
}

template <typename T>
T InterpolatorT<T>::linearInterpolate(const T &y1, const T &y2, real_t mu) { return (y2 - y1) * mu + y1; }

template <typename T>
T InterpolatorT<T>::cubicInterpolate(const T &y0, const T &y1, const T &y2, const T &y3, real_t mu) {
	real_t mu2 = mu * mu;
	T a0 = y3 - y2 - y0 + y1;
	T a1 = y0 - y1 - a0;
	T a2 = y2 - y0;
	T a3 = y1;

	return (a0 * mu * mu2 + a1 * mu2 + a2 * mu + a3);
}

// ============================================================
// Interpolator1D (real_t)
// ============================================================

_FORCE_INLINE_ real_t lengthOf(real_t f) { return f; }

typedef InterpolatorT<real_t> Interpolator1D;

// ============================================================
// Interpolator2D (Vector2)
// ============================================================

typedef InterpolatorT<Vector2> Interpolator2D;

inline real_t lengthOf(const Vector2 &v) {
	return v.length();
}

// ============================================================
// Interpolator3D (Vector3)
// ============================================================

typedef InterpolatorT<Vector3> Interpolator3D;

_FORCE_INLINE_ real_t lengthOf(const Vector3 &v) {
	return v.length();
}

} // namespace msa

#endif // MSAINTERPOLATOR_H
