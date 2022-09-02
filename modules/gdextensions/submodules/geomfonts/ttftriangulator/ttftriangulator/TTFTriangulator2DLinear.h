/*************************************************************************/
/*  TTFTriangulator2DLinear.h                                            */
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

#pragma once

#include <algorithm>
#include <cstdint>
#include <exception>
#include <vector>

#include "TTFExceptions.h"
#include "TTFMath.h"
#include "TTFTriangulator2D.h"
#include "TTFTypes.h"

namespace TTFCore {

template <typename TVert, typename TTri>
class Triangulator2DLinear : public Triangulator2D<TVert, TTri> {
	friend class TFont;

protected:
	// Triangulator2DLinear specific functions
	void AddCurvatureEdges(TVert pp, TVert ip, TVert pc, size_t vbo);
	void TraceContourHelper(ContourPoint cp, TVert &p0, TVert &p1, bool &ppc, size_t vbo);
	void TraceContour(CItr begin, CItr end);
	void TraceContour();

	// Triangulator private interface
	std::vector<ContourPoint> &GetContours(); // returns a vector to the contour vector
	void AppendTriangulation(); // triangulates and appends any contour data into the mesh data

	virtual size_t GetCurveDivideCount(TVert p0, TVert p1, TVert p2) = 0; // returns the number of edge sub-divisions for a given curve (0 = straight line, 1 = 2 edges, 2 = 3, ect...)

public:
	Triangulator2DLinear();
	Triangulator2DLinear(TriangulatorFlags);
};

template <typename TVert, typename TTri>
Triangulator2DLinear<TVert, TTri>::Triangulator2DLinear() :
		Triangulator2D<TVert, TTri>(TriangulatorFlags::none) {
}

template <typename TVert, typename TTri>
Triangulator2DLinear<TVert, TTri>::Triangulator2DLinear(TriangulatorFlags flags_) :
		Triangulator2D<TVert, TTri>(flags_) {
}

template <typename TVert, typename TTri>
void Triangulator2DLinear<TVert, TTri>::AddCurvatureEdges(TVert p0, TVert p1, TVert p2, size_t vbo) {
	// assume pp has been pushed onto the vertex 'stack' and is the last vertex

	// sanity checks
	if (p0 == p1 || p1 == p2 || p0 == p2)
		return;

	// get point count for curve
	size_t j = std::min<size_t>(GetCurveDivideCount(p0, p1, p2), 0xfffe);
	uint16_t k = static_cast<uint16_t>(0xffff) / static_cast<uint16_t>(j + 1);

	// get start/end index
	size_t i0 = AddVertex(vbo, p0);
	size_t i2 = AddVertex(vbo, p2);

	// create edges
	uint16_t f = k;
	size_t ai = i0;
	for (size_t i = 0; i < j; ++i, f += k) {
		TVert b = quad_lerp(p0, p1, p2, f); // new interpolated point
		size_t bi = AddVertex(vbo, b);
		this->AddEdge(ai, bi);
		ai = bi;
	}
	this->AddEdge(ai, i2);
}

template <typename TVert, typename TTri>
void Triangulator2DLinear<TVert, TTri>::TraceContourHelper(ContourPoint cp, TVert &p0, TVert &p1, bool &ppc, size_t vbo) {
	// intialize point variables
	TVert p2 = cp.pos; // point current/under consideration
	bool cpc = cp.OnCurve(); // current point on curve

	// determine action
	if (ppc && cpc) {
		size_t i0 = AddVertex(vbo, p0);
		size_t i2 = AddVertex(vbo, p2);
		this->AddEdge(i0, i2);
		p0 = p2;
	} else if (ppc && !cpc) {
		p1 = p2;
		ppc = false;
	} else if (!ppc && cpc) {
		AddCurvatureEdges(p0, p1, p2, vbo);
		p0 = p2;
		ppc = true;
	} else if (!ppc && !cpc) {
		TVert pn = MidPoint(p1, p2);
		AddCurvatureEdges(p0, p1, pn, vbo);
		p0 = pn;
		p1 = p2;
	}
}

template <typename TVert, typename TTri>
void Triangulator2DLinear<TVert, TTri>::TraceContour(CItr begin, CItr end) {
	// sanity checks
	if (end - begin < 2)
		return;

	// init variables
	size_t vbo = this->verts.size(); // store offset of 1st vertex
	size_t ebo = this->edges.size(); // store offset of 1st edge

	// trace contour
	if (begin->OnCurve()) {
		TVert p0 = begin->pos; // previous point on curve
		TVert p1; // intermediate (off curve) point/control point
		bool ppc = true; // previous point was on the curve

		for (auto i = begin + 1; i != end; ++i)
			TraceContourHelper(*i, p0, p1, ppc, vbo);
		TraceContourHelper(*end, p0, p1, ppc, vbo);
		TraceContourHelper(*begin, p0, p1, ppc, vbo);
	} else if ((begin + 1)->OnCurve()) {
		TVert p0 = (begin + 1)->pos;
		TVert p1;
		bool ppc = true;

		for (auto i = begin + 2; i != end; ++i)
			TraceContourHelper(*i, p0, p1, ppc, vbo);
		TraceContourHelper(*end, p0, p1, ppc, vbo);
		TraceContourHelper(*begin, p0, p1, ppc, vbo);
		TraceContourHelper(*(begin + 1), p0, p1, ppc, vbo);
	} else {
		// neither begin nor begin + 1 is OnCurve
		// we have to create a new ContourPoint to start the contour
		ContourPoint cp;
		cp.end_point = false;
		cp.flags = 1; // on curve
		cp.pos = MidPoint(begin->pos, (begin + 1)->pos);

		TVert p0 = cp.pos;
		TVert p1;
		bool ppc = true;

		for (auto i = begin + 1; i != end; ++i)
			TraceContourHelper(*i, p0, p1, ppc, vbo);
		TraceContourHelper(*end, p0, p1, ppc, vbo);
		TraceContourHelper(*begin, p0, p1, ppc, vbo);
		TraceContourHelper(cp, p0, p1, ppc, vbo);
	}

	// create edge linked list
	size_t eeo = this->edges.size() - 1;
	this->edges[ebo].pe = eeo;
	this->edges[ebo].ne = ebo + 1;
	for (size_t i = ebo + 1; i < eeo; ++i) {
		this->edges[i].pe = i - 1;
		this->edges[i].ne = i + 1;
	}
	this->edges[eeo].pe = eeo - 1;
	this->edges[eeo].ne = ebo;
}

template <typename TVert, typename TTri>
void Triangulator2DLinear<TVert, TTri>::TraceContour() {
	CItr begin = this->contours.begin();
	CItr end = this->contours.end();

	if (end - begin == 0)
		return;
	CItr contour_begin = begin;
	for (auto i = begin + 1; i != end; ++i) {
		if (i->end_point) {
			TraceContour(contour_begin, i);
			contour_begin = i + 1;
		}
	}
}

// ----- Triangulator private interface -----
template <typename TVert, typename TTri>
std::vector<ContourPoint> &Triangulator2DLinear<TVert, TTri>::GetContours() {
	return this->contours;
}

template <typename TVert, typename TTri>
void Triangulator2DLinear<TVert, TTri>::AppendTriangulation() {
	// Clear() has been called,  GetContours() called and the contour points filled

	TraceContour();
	TriangulateEdges((this->flags & TriangulatorFlags::use_cdt) == TriangulatorFlags::use_cdt);
	this->CreateTris();

	if (this->bold_offset != 0)
		this->ApplyBold();
	if (this->italic_offset_x != 0 && this->italic_offset_y != 0)
		this->ApplyItalic();

	if ((this->flags & TriangulatorFlags::remove_unused_verts) == TriangulatorFlags::remove_unused_verts) {
		this->RemoveUnusedVerts();
	}

	// clear temporary data (if the triangulator is copied, this does not need to be copied with it)
	this->contours.clear();
	this->edges.clear();
	this->segs.clear();
	this->bounds.clear();
	this->tri_edges.clear();
}

} //namespace TTFCore
