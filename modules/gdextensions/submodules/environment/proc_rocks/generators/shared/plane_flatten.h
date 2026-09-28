/**************************************************************************/
/*  plane_flatten.h                                                       */
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

#ifndef PROC_ROCKS_SHARED_PLANE_FLATTEN_H
#define PROC_ROCKS_SHARED_PLANE_FLATTEN_H

#include "core/array.h"
#include "core/math/plane.h"
#include "core/math/vector3.h"

// Plane-clip-and-cap + smooth-normal recompute — shared between the noise pipeline
// (Method 3's own arbitrary-angle cutplane) and the generic cross-generator
// "flatten_base" property (see proc_rocks.cpp's ProcRockMesh::_apply_flatten_base()).
// Originally lived entirely in generators/procrockgen/procrockgen.cpp; moved here,
// unchanged, so any generator can reuse it without depending on procrockgen.cpp
// (regression-pinned via a fixed-seed checksum doctest in proc_rocks.cpp so the move
// is provably behavior-preserving).

struct ClippedMesh {
	Vector<Vector3> vertices;
	Vector<int> indices;
	// Index into `indices` where the cap fan's triangles begin (indices before this are
	// the clipped "kept" geometry, carried over from the input mesh's own winding;
	// indices from here on are the newly-generated cap). Purely additional metadata —
	// does not change vertices/indices' own content, so existing callers reading only
	// those two fields are unaffected. Added so callers can verify/correct the cap's
	// winding against the kept region's own (already correct, since it's untouched)
	// winding without assuming a fixed absolute handedness convention — see
	// apply_flatten_base()'s own comment for why that assumption doesn't always hold.
	int cap_start_index = 0;
};

// Clips an indexed triangle mesh against a plane (keeping the side the plane's normal
// points away from) and caps the exposed cross-section with a triangle fan ordered by
// angle around its centroid. This assumes a single, star-shaped cross-section, which
// holds for cutting a lightly-displaced icosphere/hull with one plane -- NOT valid for a
// mesh combined from multiple disjoint pieces (e.g. RockCluster's scattered cells, which
// use their own per-cell convex-hull cap instead — see rockcluster.cpp).
ClippedMesh clip_and_cap(const Vector<Vector3> &p_vertices, const Vector<int> &p_indices, const Plane &p_plane);

// Smooth per-vertex normals: accumulate each triangle's own normalized face normal
// (ComputeNormal(), rock_header.h) into its 3 vertices, then normalize — equal-weighted
// per triangle, not area-weighted. NOTE: this is deliberately kept separate from the
// similar-looking normal-recompute helpers in rockcluster.cpp/gen_rock.cpp, which use a
// different (area-weighted) accumulation and the opposite face-winding convention —
// confirmed non-equivalent by inspection, so consolidating them would silently change
// those callers' output; only this one (already paired with clip_and_cap() above) is
// shared.
Vector<Vector3> compute_smooth_normals(const Vector<Vector3> &p_vertices, const Vector<int> &p_indices);

// Clips mesh_arrays flat at a horizontal plane (normal +Y, offset p_offset from the
// origin), keeping the upper half and capping the cut, so a generator's usually
// free-floating blob gets a flat base it can sit flush on a surface with. Regenerates
// normals (compute_smooth_normals() above) and UV (box projection) on the result since
// clipping changes the vertex set. No-ops (returns p_mesh_arrays unchanged, with a
// WARN_PRINT_ONCE) if the cut would remove the entire mesh.
Array apply_flatten_base(const Array &p_mesh_arrays, real_t p_offset);

#endif // PROC_ROCKS_SHARED_PLANE_FLATTEN_H
