#pragma once

#include <exception>
#include <vector>
#include <cstdint>
#include <algorithm>

#include "TTFExceptions.h"
#include "TTFMath.h"
#include "TTFTypes.h"
#include "TTFTriangulator2D.h"
#include "TTFTriangulator2DLinear.h"

namespace TTFCore {

// ---------------------------------------------------------------------------------------------------------------------------
// Triangulator3D
//  - extrude is the distance to extrude the 3d mesh
//  - extrude should probably be positive for a LH coordinate system
// ---------------------------------------------------------------------------------------------------------------------------

template <typename TVert, typename TTri> class Triangulator3D : public Triangulator2DLinear<TVert,TTri> {
	friend class Font;

    protected:
        using TCoord = typename Triangulator2DLinear<TVert,TTri>::TCoord;

	protected:
		// Triangulator 3D data/functions
		TCoord extrude;
		void ExtrudeMesh();

		// Triangulator private interface
		std::vector<ContourPoint>& GetContours(); // returns a vector to the contour vector
		void AppendTriangulation(); // triangulates and appends any contour data into the mesh data

	public:
		Triangulator3D(TCoord extrude);
		Triangulator3D(TriangulatorFlags, TCoord extrude);

		// extrude access
		TCoord GetExtrude() const;
		void SetExtrude(TCoord);
};

template <typename TVert, typename TTri> Triangulator3D<TVert, TTri>::Triangulator3D(TCoord extrude_) :
	Triangulator2DLinear<TVert,TTri>(TriangulatorFlags::none) {
	this->extrude = extrude_;
}

template <typename TVert, typename TTri> Triangulator3D<TVert, TTri>::Triangulator3D(TriangulatorFlags flags_, TCoord extrude_) :
	Triangulator2DLinear<TVert,TTri>(flags_) {
	this->extrude = extrude_;
}

template <typename TVert, typename TTri> void Triangulator3D<TVert, TTri>::ExtrudeMesh() {
	
	// copy and duplicate vertices
	size_t n = this->verts.size();
	this->verts.reserve(n * 2);
	for (size_t i = 0; i < n; ++i) {
		TVert v = this->verts[i];
		this->verts.push_back(TVert(v.x, v.y, this->extrude));
	}

	// duplicate tris
	size_t m = this->tris.size();
	for (size_t i = 0; i < m; ++i) {
		TTri t = this->tris[i];
		this->tris.push_back(TTri(t.i0 + n, t.i2 + n, t.i1 + n, 0)); // duplicate triangle and reverse winding order
	}

	// create edges
	for (auto i = this->edges.begin(); i != this->edges.end(); ++i) {
		size_t v0 = i->i0;
		size_t v1 = i->i1;
		size_t v2 = i->i0 + n;
		size_t v3 = i->i1 + n;
		this->tris.push_back(TTri(v0,v2,v1,0));
		this->tris.push_back(TTri(v1,v2,v3,0));
	}
}

template <typename TVert, typename TTri> std::vector<ContourPoint>& Triangulator3D<TVert, TTri>::GetContours() {
	return this->contours;
}

template <typename TVert, typename TTri> void Triangulator3D<TVert, TTri>::AppendTriangulation() {
	// Clear() has been called,  GetContours() called and the contour points filled

	this->TraceContour();
	TriangulateEdges((this->flags & TriangulatorFlags::use_cdt) == TriangulatorFlags::use_cdt);
	this->CreateTris();
	if (this->bold_offset != 0) this->ApplyBold();
	if (this->italic_offset_x != 0 && this->italic_offset_y != 0) this->ApplyItalic();

	ExtrudeMesh();
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


// ----- extrude access -----
template <typename TVert, typename TTri> typename Triangulator3D<TVert, TTri>::TCoord Triangulator3D<TVert, TTri>::GetExtrude() const {
	return this->extrude;
}

template <typename TVert, typename TTri> void Triangulator3D<TVert, TTri>::SetExtrude(TCoord extrude_) {
	this->extrude = extrude_;
}

} // namespace
