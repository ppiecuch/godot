/*************************************************************************/
/*  TTFTypes.cpp                                                         */
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

#include <algorithm>
#include <cstdint>
#include <exception>
#include <vector>

#include "TTFExceptions.h"
#include "TTFMath.h"
#include "TTFTypes.h"

using namespace TTFCore;

// ---------------------------------------------------------------------------------------------------------------------------
//	ContourPoint
// ---------------------------------------------------------------------------------------------------------------------------

// ----- ContourPoint -----
ContourPoint::ContourPoint() {
	end_point = false;
	flags = 0;
	pos = vec2t(0, 0);
}

bool ContourPoint::OnCurve() const {
	return (flags & 1) != 0;
}

bool ContourPoint::XShortVector() const {
	return (flags & 2) != 0;
}

bool ContourPoint::YShortVector() const {
	return (flags & 4) != 0;
}

bool ContourPoint::XIsSame() const {
	return (flags & 16) != 0;
}

bool ContourPoint::XIsDifferent() const {
	return (flags & 16) == 0;
}

bool ContourPoint::XIsPositive() const {
	return (flags & 16) != 0;
}

bool ContourPoint::XIsNegative() const {
	return (flags & 16) == 0;
}

bool ContourPoint::YIsSame() const {
	return (flags & 32) != 0;
}

bool ContourPoint::YIsDifferent() const {
	return (flags & 32) == 0;
}

bool ContourPoint::YIsPositive() const {
	return (flags & 32) != 0;
}

bool ContourPoint::YIsNegative() const {
	return (flags & 32) == 0;
}

// ---------------------------------------------------------------------------------------------------------------------------
//	TriangulatorImpl internal types
// ---------------------------------------------------------------------------------------------------------------------------

// ----- Edge -----
Edge::Edge(size_t i0_, size_t i1_) {
	i0 = i0_;
	i1 = i1_;
}

Edge::Edge(size_t i0_, size_t i1_, size_t pe_, size_t ne_) {
	i0 = i0_;
	i1 = i1_;
	pe = pe_;
	ne = ne_;
}

// ----- LineSegment -----
LineSegment::LineSegment(size_t i0_, size_t i1_, int32_t length_) {
	i0 = i0_;
	i1 = i1_;
	length = length_;
	inscribe = 0;
}

LineSegment::LineSegment(size_t i0_, size_t i1_, int32_t length_, int32_t inscribe_) {
	i0 = i0_;
	i1 = i1_;
	length = length_;
	inscribe = inscribe_;
}

// ----- Bound -----
Bound::Bound(size_t i0_, size_t i1_) :
		i0(i0_), i1(i1_) {}

// ----- TriEdge -----
TriEdge::TriEdge(size_t i0_, size_t i1_) :
		i0(i0_), i1(i1_), in_use(true) {}

// ---------------------------------------------------------------------------------------------------------------------------
//	TriangulatorFlags
// ---------------------------------------------------------------------------------------------------------------------------

TriangulatorFlags TTFCore::operator~(TriangulatorFlags f) {
	return static_cast<TriangulatorFlags>(~static_cast<uint8_t>(f));
}

TriangulatorFlags TTFCore::operator&(TriangulatorFlags f0, TriangulatorFlags f1) {
	return static_cast<TriangulatorFlags>(static_cast<uint8_t>(f0) & static_cast<uint8_t>(f1));
}

TriangulatorFlags TTFCore::operator|(TriangulatorFlags f0, TriangulatorFlags f1) {
	return static_cast<TriangulatorFlags>(static_cast<uint8_t>(f0) | static_cast<uint8_t>(f1));
}

TriangulatorFlags TTFCore::operator^(TriangulatorFlags f0, TriangulatorFlags f1) {
	return static_cast<TriangulatorFlags>(static_cast<uint8_t>(f0) ^ static_cast<uint8_t>(f1));
}

TriangulatorFlags &TTFCore::operator&=(TriangulatorFlags &f0, TriangulatorFlags f1) {
	f0 = static_cast<TriangulatorFlags>(static_cast<uint8_t>(f0) & static_cast<uint8_t>(f1));
	return f0;
}

TriangulatorFlags &TTFCore::operator|=(TriangulatorFlags &f0, TriangulatorFlags f1) {
	f0 = static_cast<TriangulatorFlags>(static_cast<uint8_t>(f0) | static_cast<uint8_t>(f1));
	return f0;
}

TriangulatorFlags &TTFCore::operator^=(TriangulatorFlags &f0, TriangulatorFlags f1) {
	f0 = static_cast<TriangulatorFlags>(static_cast<uint8_t>(f0) ^ static_cast<uint8_t>(f1));
	return f0;
}
