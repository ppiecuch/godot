/**************************************************************************/
/*  TTFTriangulator3DBezel.h                                              */
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

#include <algorithm>
#include <cstdint>
#include <exception>
#include <vector>

#include "TTFExceptions.h"
#include "TTFMath.h"
#include "TTFTriangulator2D.h"
#include "TTFTriangulator2DLinear.h"
#include "TTFTypes.h"

namespace TTFCore {

// ---------------------------------------------------------------------------------------------------------------------------
// Triangulator3DBezel
//  - extrude is the distance to extrude the 3d mesh
//  - extrude should probably be positive for a LH coordinate system, negative for a right
// ---------------------------------------------------------------------------------------------------------------------------

template <typename TVert, typename TTri>
class Triangulator3DBezel : public Triangulator2DLinear<TVert, TTri> {
	friend class TFont;

protected:
	using TCoord = typename Triangulator2DLinear<TVert, TTri>::TCoord;

private:
	// hide Triangulator2D functions
	TCoord GetBold() const;
	void SetBold(TCoord);

protected:
	// Triangulator3DBezel data
	TCoord z1, z2; // z values of bezel extrude (z0 is implicitly 0)
	TCoord b0, b1, b2; // bold values each extruded layer
	std::vector<TVert> temp_verts; // temporary vertices

	// Triangulator3DBezel functions
	void BezelMesh();

	// Triangulator private interface
	std::vector<ContourPoint> &GetContours(); // returns a vector to the contour vector
	void AppendTriangulation(); // triangulates and appends any contour data into the mesh data

public:
	Triangulator3DBezel(TCoord z1, TCoord z2, TCoord b0, TCoord b1, TCoord b2);
	Triangulator3DBezel(TriangulatorFlags, TCoord z1, TCoord z2, TCoord b0, TCoord b1, TCoord b2);

	// bezel access
	void SetBezelValues(TCoord z1, TCoord z2, TCoord b0, TCoord b1, TCoord b2);
};

template <typename TVert, typename TTri>
Triangulator3DBezel<TVert, TTri>::Triangulator3DBezel(TCoord z1_, TCoord z2_, TCoord b0_, TCoord b1_, TCoord b2_) :
		Triangulator2DLinear<TVert, TTri>(TriangulatorFlags::none) {
	z1 = z1_;
	z2 = z2_;
	b0 = b0_;
	b1 = b1_;
	b2 = b2_;
}

template <typename TVert, typename TTri>
Triangulator3DBezel<TVert, TTri>::Triangulator3DBezel(TriangulatorFlags flags_, TCoord z1_, TCoord z2_, TCoord b0_, TCoord b1_, TCoord b2_) :
		Triangulator2DLinear<TVert, TTri>(flags_) {
	z1 = z1_;
	z2 = z2_;
	b0 = b0_;
	b1 = b1_;
	b2 = b2_;
}

template <typename TVert, typename TTri>
void Triangulator3DBezel<TVert, TTri>::BezelMesh() {
	temp_verts = this->verts;
	size_t n = this->verts.size();
	this->verts.reserve(n * 3);

	// construct back vertices
	this->bold_offset = b2;
	this->ApplyBold();
	for (size_t i = 0; i < n; ++i)
		this->verts[i].z = z2;

	// construct mid vertices
	this->verts.insert(this->verts.begin(), temp_verts.begin(), temp_verts.end());
	this->bold_offset = b1;
	this->ApplyBold();
	for (size_t i = 0; i < n; ++i)
		this->verts[i].z = z1;

	// construct front vertices
	this->verts.insert(this->verts.begin(), temp_verts.begin(), temp_verts.end());
	this->bold_offset = b0;
	this->ApplyBold();

	// construct back faces
	size_t m = this->tris.size();
	size_t n2 = n * 2;
	for (size_t i = 0; i < m; ++i) {
		TTri t = this->tris[i];
		this->tris.push_back(TTri(t.i0 + n2, t.i2 + n2, t.i1 + n2, 0)); // duplicate triangle and reverse winding order
	}

	// create edges
	for (Edge e : this->edges) {
		size_t v0 = e.i0;
		size_t v1 = e.i1;
		size_t v2 = e.i0 + n;
		size_t v3 = e.i1 + n;
		size_t v4 = e.i0 + n2;
		size_t v5 = e.i1 + n2;
		this->tris.push_back(TTri(v0, v2, v1, 0));
		this->tris.push_back(TTri(v1, v2, v3, 0));
		this->tris.push_back(TTri(v2, v4, v3, 0));
		this->tris.push_back(TTri(v3, v4, v5, 0));
	}

	// clean up
	this->bold_offset = 0;
	temp_verts.clear();
}

template <typename TVert, typename TTri>
std::vector<ContourPoint> &Triangulator3DBezel<TVert, TTri>::GetContours() {
	return this->contours;
}

template <typename TVert, typename TTri>
void Triangulator3DBezel<TVert, TTri>::AppendTriangulation() {
	// Clear() has been called,  GetContours() called and the contour points filled

	this->TraceContour();
	TriangulateEdges((this->flags & TriangulatorFlags::use_cdt) == TriangulatorFlags::use_cdt);
	this->CreateTris();
	if (this->italic_offset_x != 0 && this->italic_offset_y != 0)
		this->ApplyItalic();
	this->BezelMesh();

	if ((this->flags & TriangulatorFlags::remove_unused_verts) == TriangulatorFlags::remove_unused_verts) {
		this->RemoveUnusedVerts();
	}

	// clear temporary data
	this->contours.clear();
	this->edges.clear();
	this->segs.clear();
	this->bounds.clear();
	this->tri_edges.clear();
}

template <typename TVert, typename TTri>
void Triangulator3DBezel<TVert, TTri>::SetBezelValues(TCoord z1, TCoord z2, TCoord b0, TCoord b1, TCoord b2) {
	z1 = this->z1_;
	z2 = this->z2_;
	b0 = this->b0_;
	b1 = this->b1_;
	b2 = this->b2_;
}

} //namespace TTFCore
