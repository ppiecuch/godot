/**************************************************************************/
/*  plane_flatten.cpp                                                     */
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

#include "plane_flatten.h"

#include "box_uv.h"
#include "rock_header.h"

#include "core/math/geometry.h"
#include "servers/visual_server.h"

// Moved verbatim from generators/procrockgen/procrockgen.cpp -- see that file's own
// pipeline_cutplane_* feature, still the other caller of this function.
ClippedMesh clip_and_cap(const Vector<Vector3> &p_vertices, const Vector<int> &p_indices, const Plane &p_plane) {
	ClippedMesh out;
	Vector<Vector3> cap_points;
	const real_t on_plane_epsilon = real_t(CMP_EPSILON) * 100;

	for (int i = 0; i + 2 < p_indices.size(); i += 3) {
		Vector<Vector3> tri;
		tri.push_back(p_vertices[p_indices[i]]);
		tri.push_back(p_vertices[p_indices[i + 1]]);
		tri.push_back(p_vertices[p_indices[i + 2]]);

		Vector<Vector3> clipped = Geometry::clip_polygon(tri, p_plane);
		if (clipped.size() < 3) {
			continue;
		}

		int base = out.vertices.size();
		for (int v = 0; v < clipped.size(); v++) {
			out.vertices.push_back(clipped[v]);
			if (Math::abs(p_plane.distance_to(clipped[v])) <= on_plane_epsilon) {
				cap_points.push_back(clipped[v]);
			}
		}
		for (int v = 1; v + 1 < clipped.size(); v++) {
			out.indices.push_back(base);
			out.indices.push_back(base + v);
			out.indices.push_back(base + v + 1);
		}
	}

	out.cap_start_index = out.indices.size();

	// Weld near-duplicate cap boundary points before fan-triangulating: each original
	// triangle is clipped independently, so a boundary edge shared by two adjacent kept
	// triangles contributes its own on-plane point twice (once per side, at the same
	// physical position up to floating-point noise). Left unwelded, the angular-sort fan
	// below connects these near-identical points into degenerate, effectively-zero-area
	// triangles whose ComputeNormal() direction is numerically unreliable (normalizing a
	// near-zero vector) -- confirmed directly: this produced a cap whose triangles were
	// NOT uniformly oriented (mixed roughly 20/80 split checked against the same convention
	// that correctly classifies 100% of the kept region), rather than a simple, uniform
	// flip. The same class of defect item 30 (memo.md) already fixed for RockCluster's own
	// boundary tracer via distance-based welding.
	{
		const real_t weld_epsilon = on_plane_epsilon * 10;
		Vector<Vector3> welded;
		for (int i = 0; i < cap_points.size(); i++) {
			bool dup = false;
			for (int j = 0; j < welded.size(); j++) {
				if (cap_points[i].distance_squared_to(welded[j]) <= weld_epsilon * weld_epsilon) {
					dup = true;
					break;
				}
			}
			if (!dup) {
				welded.push_back(cap_points[i]);
			}
		}
		cap_points = welded;
	}

	if (cap_points.size() >= 3) {
		Vector3 centroid;
		for (int i = 0; i < cap_points.size(); i++) {
			centroid += cap_points[i];
		}
		centroid /= cap_points.size();

		Vector3 normal = p_plane.normal;
		Vector3 up = Math::abs(normal.dot(Vector3(0, 1, 0))) < 0.99 ? Vector3(0, 1, 0) : Vector3(1, 0, 0);
		Vector3 tangent = up.cross(normal).normalized();
		Vector3 bitangent = normal.cross(tangent);

		Vector<real_t> angles;
		angles.resize(cap_points.size());
		for (int i = 0; i < cap_points.size(); i++) {
			Vector3 d = cap_points[i] - centroid;
			angles.write[i] = Math::atan2(d.dot(bitangent), d.dot(tangent));
		}

		Vector<int> order;
		order.resize(cap_points.size());
		for (int i = 0; i < order.size(); i++) {
			order.write[i] = i;
		}
		for (int i = 1; i < order.size(); i++) {
			int key = order[i];
			real_t key_angle = angles[key];
			int j = i - 1;
			while (j >= 0 && angles[order[j]] > key_angle) {
				order.write[j + 1] = order[j];
				j--;
			}
			order.write[j + 1] = key;
		}

		int base = out.vertices.size();
		for (int i = 0; i < order.size(); i++) {
			out.vertices.push_back(cap_points[order[i]]);
		}
		for (int i = 1; i + 1 < order.size(); i++) {
			int a = base, b = base + i, c = base + i + 1;
			// Orient the cap so ComputeNormal() (Godot's actual front-face convention,
			// confirmed directly against CubeMesh -- see memo.md's "Bugs Fixed" for why an
			// earlier attempt at this got the direction backwards) ends up matching
			// p_plane.normal -- i.e. facing away from the kept/remaining solid, toward
			// where the removed material used to be. Verified by hand against a real
			// exported triangle: this condition produces a downward-facing cap for
			// flatten_base's plane (0,-1,0), correctly invisible from any camera above it.
			// Making this double-sided (duplicate reverse-wound copy, tried in an earlier
			// pass) was reverted: it introduced a worse, more confusing symptom (the cap
			// reportedly drew on top of the dome regardless of actual depth, not simple
			// Z-fighting) than the single-sided "occasionally see-through from a grazing
			// angle" issue it was meant to fix. See memo.md's "Bugs Fixed" for the full
			// account. Single-sided remains correct for the primary, verified fix: the cap
			// is invisible from any camera above it, exactly as backface culling should do.
			Vector3 n = ComputeNormal(out.vertices[a], out.vertices[b], out.vertices[c]);
			if (n.dot(p_plane.normal) < 0) {
				out.indices.push_back(a);
				out.indices.push_back(c);
				out.indices.push_back(b);
			} else {
				out.indices.push_back(a);
				out.indices.push_back(b);
				out.indices.push_back(c);
			}
		}
	}

	return out;
}

// Moved verbatim from generators/procrockgen/procrockgen.cpp.
Vector<Vector3> compute_smooth_normals(const Vector<Vector3> &p_vertices, const Vector<int> &p_indices) {
	Vector<Vector3> normals;
	normals.resize(p_vertices.size());
	for (int i = 0; i < normals.size(); i++) {
		normals.write[i] = Vector3();
	}
	for (int i = 0; i + 2 < p_indices.size(); i += 3) {
		int i0 = p_indices[i], i1 = p_indices[i + 1], i2 = p_indices[i + 2];
		Vector3 n = ComputeNormal(p_vertices[i0], p_vertices[i1], p_vertices[i2]);
		normals.write[i0] += n;
		normals.write[i1] += n;
		normals.write[i2] += n;
	}
	for (int i = 0; i < normals.size(); i++) {
		Vector3 n = normals[i];
		normals.write[i] = n.length_squared() > CMP_EPSILON ? n.normalized() : Vector3(0, 1, 0);
	}
	return normals;
}

Array apply_flatten_base(const Array &p_mesh_arrays, real_t p_offset) {
	if (p_mesh_arrays.size() != VS::ARRAY_MAX) {
		return p_mesh_arrays;
	}
	Vector<Vector3> vertices = p_mesh_arrays[VS::ARRAY_VERTEX];
	Vector<int> indices = p_mesh_arrays[VS::ARRAY_INDEX];
	if (vertices.size() == 0) {
		return p_mesh_arrays;
	}
	if (indices.size() == 0) {
		// Flat-shaded generator output (e.g. RockGen/IcoRock's default rockgen_smoothed=
		// false / rockgeneration_smoothed=false) is unindexed -- every 3 consecutive
		// vertices already form one triangle with no vertex sharing, so a trivial
		// sequential index buffer represents exactly the same mesh, just in the form
		// clip_and_cap() needs.
		indices.resize(vertices.size());
		for (int i = 0; i < indices.size(); i++) {
			indices.write[i] = i;
		}
	}

	// clip_and_cap() keeps the side the plane's *normal points away from* (see its own
	// doc comment / Geometry::clip_polygon()'s LOC_INSIDE convention: kept when
	// distance_to(point) < 0). To keep the upper half (y > p_offset) the normal has to
	// point down, with the distance negated to match.
	ClippedMesh clipped = clip_and_cap(vertices, indices, Plane(Vector3(0, -1, 0), -p_offset));
	if (clipped.vertices.size() == 0 || clipped.indices.size() == 0) {
		WARN_PRINT_ONCE("apply_flatten_base(): flatten_base_offset cut away the entire mesh -- returning it unchanged instead of an empty surface.");
		return p_mesh_arrays;
	}

	// clip_and_cap()'s cap-fan winding is derived directly from the plane's normal (see
	// its own comment) -- previously this was cross-checked against a signed-volume
	// (divergence theorem) comparison between the kept region and the cap, self-correcting
	// the cap's winding when the two volumes' signs disagreed. That check itself turned out
	// to be unreliable: it reported the cap as "consistent" with the kept region for every
	// tested seed (a 60-seed sweep, 0/60 flagged), yet live rendering showed the resulting
	// cap was still visible from outside the mesh -- the wrong side -- for every one of
	// them. Root-caused via direct, controlled live-render comparison (isolated cap alone,
	// isolated kept-region alone, then the combined mesh with the cap's winding manually
	// flipped, which fixed it): the divergence-theorem contribution from cap triangles
	// (degenerate/planar, all lying exactly on the cut plane) isn't a reliable proxy for
	// their true visible-side orientation the way it is for the kept region's fully 3D
	// triangles. Fixed at the source instead -- clip_and_cap()'s own fan-orientation check
	// (see its comment) now derives the correct winding directly, verified by the same live
	// rendering test, with no downstream volume-based correction needed or applied.

	Vector<Vector3> normals = compute_smooth_normals(clipped.vertices, clipped.indices);
	Vector<Vector2> uvs;
	uvs.resize(clipped.vertices.size());
	for (int i = 0; i < clipped.vertices.size(); i++) {
		int box_dir = rock_studio_get_box_dir(normals[i]);
		uvs.write[i] = rock_studio_get_box_uv(clipped.vertices[i], box_dir);
	}

	Array out;
	out.resize(VS::ARRAY_MAX);
	out[VS::ARRAY_VERTEX] = clipped.vertices;
	out[VS::ARRAY_NORMAL] = normals;
	out[VS::ARRAY_TEX_UV] = uvs;
	out[VS::ARRAY_INDEX] = clipped.indices;
	return out;
}
