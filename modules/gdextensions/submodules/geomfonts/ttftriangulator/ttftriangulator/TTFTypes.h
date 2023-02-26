/**************************************************************************/
/*  TTFTypes.h                                                            */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include <cstdint>
#include <vector>

#include "TTFExceptions.h"
#include "TTFMath.h"

namespace TTFCore {

// ---------------------------------------------------------------------------------------------------------------------------
// ContourPoint
//  - a point on the line that delineates the contours of the glyph
//  - flags come straight from the font file
//  - pretty much only pos and OnCurve() means anything outside of a TTFFont object
// ---------------------------------------------------------------------------------------------------------------------------

struct ContourPoint {
	vec2t pos;
	bool end_point;
	uint8_t flags;

	// helper functions
	ContourPoint();

	bool OnCurve() const; // returns true if this point is on curve, false if off curve (control point)
	bool XShortVector() const; // x coordinate is 1 byte
	bool YShortVector() const; // y coordinate is 1 byte

	bool XIsSame() const; // used when XShortVector() returns false
	bool XIsDifferent() const; // used when XShortVector() returns false
	bool XIsPositive() const; // used when XShortVector() returns true
	bool XIsNegative() const; // used when XShortVector() returns true

	bool YIsSame() const; // used when YShortVector() returns false
	bool YIsDifferent() const; // used when YShortVector() returns false
	bool YIsPositive() const; // used when YShortVector() returns true
	bool YIsNegative() const; // used when YShortVector() returns true
};

typedef std::vector<ContourPoint> ContourData;
typedef std::vector<ContourPoint>::const_iterator CItr;

// ---------------------------------------------------------------------------------------------------------------------------
// Internal types used for trinangulation
//  - Edge's form a linked list, delineating the contours of the glyph, ContourPoint's are converted to Edge's
//  - a LineSegment is an inner segment or line that connects the vertices of two different edges
//  - LineSegment's are 'candidate' edges of the triangulation
//  - a Bound is an Edge or LineSegment that cannot be crossed (used to cull LineSegments)
//  - a TriEdge is a final triangle edge and used for final triangle construction
//  - TriEdge's are directed (ie. they have an implied direction from i0 -> i1)
//  - not all Edge's are TriEdge's or vice-versa, but they are similar
// ---------------------------------------------------------------------------------------------------------------------------

struct Edge {
	size_t i0, i1;
	size_t pe, ne; // previous edge offset, next edge offset (forms a linked list)
	Edge(size_t i0, size_t i1);
	Edge(size_t i0, size_t i1, size_t pe, size_t ne);
};

struct LineSegment {
	size_t i0, i1;
	int32_t length, inscribe;
	LineSegment(size_t i0, size_t i1, int32_t length);
	LineSegment(size_t i0, size_t i1, int32_t length, int32_t inscribe);
};

struct Bound {
	size_t i0, i1;
	Bound(size_t i0, size_t i1);
};

struct TriEdge {
	size_t i0, i1; // vertex indices
	bool in_use; // start as true and become false when used in creating a tri
	TriEdge(size_t i0, size_t i1);
};

enum class TriangulatorFlags : uint8_t {
	none = 0, // no flags
	use_cdt = 1, // use a constrained delaunay triangulation
	remove_unused_verts, // remove any unused vertices
};

TriangulatorFlags operator~(TriangulatorFlags);
TriangulatorFlags operator&(TriangulatorFlags, TriangulatorFlags);
TriangulatorFlags operator|(TriangulatorFlags, TriangulatorFlags);
TriangulatorFlags operator^(TriangulatorFlags, TriangulatorFlags);
TriangulatorFlags &operator&=(TriangulatorFlags &, TriangulatorFlags);
TriangulatorFlags &operator|=(TriangulatorFlags &, TriangulatorFlags);
TriangulatorFlags &operator^=(TriangulatorFlags &, TriangulatorFlags);

} //namespace TTFCore
