// Usage:
//   MSA::InterpolatorT<real_t>      myInterpolator1; // create spline of real_ts
//   MSA::InterpolatorT<myDataType> myInterpolator2; // create spline of custom data types (more info below)
//
// or use preset classes:
//
//   MSA::Interpolator1D myInterpolator1D; // create spline of real_ts (1D)
//   MSA::Interpolator2D myInterpolator2D; // create spline of Vec2f (2D)
//   MSA::Interpolator3D myInterpolator3D; // create spline of Vec3f (3D)
//
// splines wrap basic functionality of stl::vector:
//   myInterpolator.size();             // return number of data elements
//   myInterpolator.reserve(int count); // if you know how many elements up front it will improved performance when adding (you can still add more than this number of elements)
//   myInterpolator.at(int i);          // return data at i'th index
//   myInterpolator.clear();            // remove all elements
//   myInterpolator.push_back(data1);   // add some data to the spline
//   myInterpolator.push_back(data2);
//
//   myInterpolator.sampleAt(real_t t);  // (e.g. t:0.34 =>) samples along 34% of the whole spline using the current interpolation method and options
//
//   setInterpolation(i);               // set interpolation type, see MSAInterpolationTypes.h (currently cubic catmull rom and linear)
//   int getInterpolation();            // get interpolation type
//
//   setUseLength(bool b);              // whether to use Length or not. using Length is slightly slower than not using (depending on number of data points)
//   bool getUseLength();               // if useLength is true, sampleAt(0.57) means sample at 57% along the physical length of the spline (using the interpolated spline for Length calculation)
//                                      // if useLength is false, the %t refers to % along the data points. If data points are evenly spaced its no problem, but if they are randomly spaced, the interpolation will not be uniform
//
//   myInterpolator.drawRaw(int dotSize, int lineWidth);                  // draws raw data with dotSize and lineWidth (make either zero to not draw dots or lines)
//   myInterpolator.drawSmooth(int numSteps, int dotSize, int lineWidth); // draws smoothed data in  (make either zero to not draw dots or lines)
//
// Using custom data type:
//   MSA::InterpolatorT<myDataType> myInterpolator2; // create spline of custom data types (more info below)
//   myDataType has to be a scalar or class with the overloaded operators:
//      +  (myDataType&)
//      -  (myDataType&)
//      == (myDataType&)
//      =  (myDataType&)
//      *  (real_t)
//
// and also define the function lengthOf(myDataType&) to return a scalar real_t value depicting the 'magnitude' of the data type (used in calculating Length)


#ifndef MSAINTERPOLATOR_H
#define MSAINTERPOLATOR_H

#include "core/math_defs.h"

#include <vector>

namespace msa {

typedef enum {
	kInterpolationLinear,
	kInterpolationCubic,
} InterpolationType;

template <typename T>
class InterpolatorT {
protected:
	InterpolationType _interpolationMethod;
	bool _useLength;
	int _lengthSubdivisions; // number of subdivisions used for length calculation
	std::vector<T> _data; // vector of all data
	std::vector<real_t> _dist; // vector of cumulative Lengths from i'th data point to beginning of spline

	real_t calcSegmentLength(int i); // calculates length of segment prior to (leading up to) i'th point

	void updateAllLengths(); // update all Lengths in _dist array

	void findPosition(real_t t, int &leftIndex, real_t &mu); // given t(0...1) find the node index directly to the left of the point

	T linearInterpolate(const T &y1, const T &y2, real_t mu);

	// this function is from Paul Bourke's site
	// http://local.wasp.uwa.edu.au/~pbourke/miscellaneous/interpolation/
	T cubicInterpolate(const T &y0, const T &y1, const T &y2, const T &y3, real_t mu);

public:
	bool verbose;

	// interpolate and re-sample at t position along the spline
	// where t: 0....1 based on length of spline
	T sampleAt(real_t t);

	void setInterpolation(InterpolationType i = kInterpolationCubic);
	int getInterpolation();

	void setUseLength(bool b);
	bool getUseLength();

	// return length upto data point i
	// leave blank (-1) to return length of entire data set
	// only valid if setUseLength is true
	// uses current interpolation settings for lenth calculation
	// returns cached value, no calculations done in this function
	const real_t getLength(int i = -1);

	// set number of subdivisions used to calculation length of segment
	void setLengthSubdivisions(int i = 100);
	int getLengthSubdivisions();

	// stl::container wrapper functions
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

// use catmull rom interpolation to re-sample At normT position along the spline
// where normT: 0....1 based on length of spline
template <typename T>
T InterpolatorT<T>::sampleAt(real_t t) {
	int numItems = size();
	if (numItems == 0) {
		// if (verbose) printf("InterpolatorT: not enough samples", t);
		return T();
	}

	if (t > 1) {
		t = 1;
	} else if (t < 0) {
		t = 0;
	}
	int i0, i1, i2, i3;
	real_t mu;

	findPosition(t, i1, mu);

	// if less than 4 data points, force linear interpolation
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
			break;

		case kInterpolationLinear:
			i2 = i1 + 1;
			if (i2 >= numItems)
				i2 = numItems - 1;
			return linearInterpolate(at(i1), at(i2), mu);
			break;
	}
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
	_data.push_back(newData); // add data

	if (getUseLength()) {
		real_t segmentLength;
		real_t totalLength;

		if (size() > 1) {
			// T distT = newData - _data.at(prevIndex); // get offset to previous node
			// real_t dist = lengthOf(distT); // actual Length to node

			segmentLength = calcSegmentLength(size() - 1);
			totalLength = segmentLength + _dist.at(size() - 2);
		} else {
			segmentLength = 0;
			totalLength = 0;
		}

		_dist.push_back(totalLength);

		// if (verbose) printf("segment length = %f | total length = %f\n", segmentLength, totalLength);
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
const T &InterpolatorT<T>::at(int i) { return _data.at(constrain(i, 0, size() - 1)); }

template <typename T>
vector<T> InterpolatorT<T>::getData() { return _data; }

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

	if (verbose) {
		printf("segment length for %i is %f\n", i, segmentLength);
	}

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
			leftIndex = 0;
			mu = 0;
			break;

		case 1:
			leftIndex = 0;
			mu = 0;
			break;

		case 2:
			leftIndex = 0;
			mu = t;
			break;

		default:
			if (_useLength) { // need to use
				real_t totalLengthOfInterpolator = _dist.at(numItems - 1);
				real_t tDist = totalLengthOfInterpolator * t; // the Length we want to be from the start
				int startIndex = floor(t * (numItems - 1)); // start approximation here
				int i1 = startIndex;
				int limitLeft = 0;
				int limitRight = numItems - 1;

				real_t distAt1, distAt2;
				for (int iterations = 0; iterations < 100; iterations++) { // limit iterations
					distAt1 = _dist.at(i1);
					if (distAt1 <= tDist) { // if Length at i1 is less than desired Length (this is good)
						distAt2 = _dist.at(constrain(i1 + 1, 0, (int)_dist.size() - 1));
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
				leftIndex = Math::floor(actT);
				mu = actT - leftIndex;
			}
	}
}

template <typename T>
T InterpolatorT<T>::linearInterpolate(const T &y1, const T &y2, real_t mu) { return (y2 - y1) * mu + y1; }

// this function is from Paul Bourke's site
// http://local.wasp.uwa.edu.au/~pbourke/miscellaneous/interpolation/
template <typename T>
T InterpolatorT<T>::cubicInterpolate(const T &y0, const T &y1, const T &y2, const T &y3, real_t mu) {
	real_t mu2 = mu * mu;
	T a0 = y3 - y2 - y0 + y1;
	T a1 = y0 - y1 - a0;
	T a2 = y2 - y0;
	T a3 = y1;

	return (a0 * mu * mu2 + a1 * mu2 + a2 * mu + a3);
}

// BEGIN Interpolator1D

_FORCE_INLINE_ real_t lengthOf(real_t f) { return f; }

typedef InterpolatorT<real_t> Interpolator1D;

// END Interpolator1D

// BEGIN Interpolator2D

typedef InterpolatorT<Vec2f> Interpolator2D;

inline real_t lengthOf(const Vec2f &v) {
	return v.length();
}

// OpenGL ES compatibility added by Rob Seward
// http://www.openframeworks.cc/forum/viewtopic.php?f=25&t=3767&p=19865

_FORCE_INLINE_ void drawInterpolatorRaw(Canvas *canvas, Interpolator2D &spline, int dotSize = 20, int lineWidth = 4) {
	int numItems = spline.size();

	if (lineWidth) {
		glLineWidth(lineWidth);
		GLfloat vertex[numItems * 2];
		for (int i = 0; i < numItems; i++) {
			vertex[i * 2] = spline.at(i).x;
			vertex[(i * 2) + 1] = spline.at(i).y;
		}
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, vertex);
		glDrawArrays(GL_LINE_STRIP, 0, numItems);
	}

	if (dotSize) {
		glPointSize(dotSize);
		GLfloat vertex[numItems * 2];
		for (int i = 0; i < numItems; i++) {
			vertex[i * 2] = spline.at(i).x;
			vertex[(i * 2) + 1] = spline.at(i).y;
		}
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, vertex);
		glDrawArrays(GL_POINTS, 0, numItems);
	}
}

_FORCE_INLINE_ void drawInterpolatorSmooth(Canvas *canvas, Interpolator2D &spline, int numSteps, int dotSize = 8, int lineWidth = 2) {
	real_t spacing = 1.0 / numSteps;
	if (lineWidth) {
		glLineWidth(lineWidth);

		GLfloat vertex[numSteps * 2];
		int i = 0;
		for (real_t f = 0; f < 1; f += spacing) {
			Vec2f v = spline.sampleAt(f);
			vertex[i * 2] = v.x;
			vertex[(i * 2) + 1] = v.y;
			i++;
		}
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, vertex);
		glDrawArrays(GL_LINE_STRIP, 0, numSteps);
	}

	if (dotSize) {
		glPointSize(dotSize);
		GLfloat vertex[numSteps * 2];
		int i = 0;
		for (real_t f = 0; f < 1; f += spacing) {
			Vec2f v = spline.sampleAt(f);
			vertex[i * 2] = v.x;
			vertex[(i * 2) + 1] = v.y;
			i++;
		}
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, vertex);
		glDrawArrays(GL_POINTS, 0, numSteps);
	}
}

// END Interpolator2D

// BEGIN Interpolator3D

typedef InterpolatorT<Vec3f> Interpolator3D;

_FORCE_INLINE_ real_t lengthOf(const Vec3f &v) {
	return v.length();
}

// OpenGL ES compatibility added by Rob Seward
// http://www.openframeworks.cc/forum/viewtopic.php?f=25&t=3767&p=19865

_FORCE_INLINE_ void drawInterpolatorRaw(Canvas *canvas, Interpolator3D spline, int dotSize = 20, int lineWidth = 4) {
	int numItems = spline.size();

	if (lineWidth) {
		glLineWidth(lineWidth);
		GLfloat vertex[numItems * 3];
		for (int i = 0; i < numItems; i++) {
			vertex[i * 3] = spline.at(i).x;
			vertex[(i * 3) + 1] = spline.at(i).y;
			vertex[(i * 3) + 2] = spline.at(i).z;
		}
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, vertex);
		glDrawArrays(GL_LINE_STRIP, 0, numItems);
	}

	if (dotSize) {
		glPointSize(dotSize);
		GLfloat vertex[numItems * 3];
		for (int i = 0; i < numItems; i++) {
			vertex[i * 3] = spline.at(i).x;
			vertex[(i * 3) + 1] = spline.at(i).y;
			vertex[(i * 3) + 2] = spline.at(i).z;
		}
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, vertex);
		glDrawArrays(GL_POINTS, 0, numItems);
	}
}

_FORCE_INLINE_ void drawInterpolatorSmooth(Canvas *canvas, Interpolator3D spline, int numSteps, int dotSize = 8, int lineWidth = 2) {
	real_t spacing = 1.0 / numSteps;
	if (lineWidth) {
		glLineWidth(lineWidth);

		GLfloat vertex[numSteps * 3];
		int i = 0;
		for (real_t f = 0; f < 1; f += spacing) {
			Vec3f v = spline.sampleAt(f);
			vertex[i * 3] = v.x;
			vertex[(i * 3) + 1] = v.y;
			vertex[(i * 3) + 2] = v.z;
			i++;
		}
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, vertex);
		glDrawArrays(GL_LINE_STRIP, 0, numSteps);
	}

	if (dotSize) {
		glPointSize(dotSize);
		GLfloat vertex[numSteps * 3];
		int i = 0;
		for (real_t f = 0; f < 1; f += spacing) {
			Vec3f v = spline.sampleAt(f);
			vertex[i * 3] = v.x;
			vertex[(i * 3) + 1] = v.y;
			vertex[(i * 3) + 2] = v.z;
			i++;
		}
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, vertex);
		glDrawArrays(GL_POINTS, 0, numSteps);
	}
}

// END Interpolator3D

} // namespace msa

#endif // MSAINTERPOLATOR_H
