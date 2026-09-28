/**************************************************************************/
/*  rockcluster.cpp                                                       */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/**************************************************************************/

#include "rockcluster.h"

#include "rockcluster_cells_data.gen.h"

#include "../shared/box_uv.h"
#include "core/math/basis.h"
#include "core/math/quat.h"
#include "core/math/random_number_generator.h"
#include "core/math/vector3.h"
#include "modules/opensimplex/open_simplex_noise.h"
#include "servers/visual_server.h"

#include <manifold/manifold.h>
#include <cfloat>

using namespace rockcluster_data;

namespace {

real_t _lerp_clamped(real_t p_a, real_t p_b, real_t p_t) {
	return p_a + (p_b - p_a) * CLAMP(p_t, (real_t)0.0, (real_t)1.0);
}

real_t _inverse_lerp_clamped(real_t p_a, real_t p_b, real_t p_v) {
	if (p_a == p_b) {
		return 0.0;
	}
	return CLAMP((p_v - p_a) / (p_b - p_a), (real_t)0.0, (real_t)1.0);
}

// Builds a basis whose local +Z axis points along `forward`, with `up` used
// to resolve the remaining degree of freedom (not Godot Camera's -Z "looking
// at" convention, so built directly rather than reusing Basis::looking_at()).
Basis _look_rotation(const Vector3 &p_forward, const Vector3 &p_up) {
	Vector3 z = p_forward.normalized();
	Vector3 x = p_up.cross(z);
	if (x.length_squared() < CMP_EPSILON) {
		x = Vector3(1, 0, 0); // forward parallel to up — arbitrary but stable fallback
	}
	x.normalize();
	Vector3 y = z.cross(x);
	Basis b;
	b.set_axis(0, x);
	b.set_axis(1, y);
	b.set_axis(2, z);
	return b;
}

// One placed instance of a cell mesh. Deliberately excludes per-vertex
// normals — those are recomputed smooth over the *combined* mesh afterward
// instead, via _recompute_smooth_normals() below.
struct PlacedCell {
	const CellData *cell = nullptr;
	Vector3 position;
	Quat rotation;
	Vector3 scale = Vector3(1, 1, 1);
};

// One cell's own vertex/color/(optionally indexed) triangle data, kept
// separate from the combined cluster buffer so it can be clipped+capped
// individually — see _clip_and_cap_cell() for why that has to happen per
// cell rather than once on the whole combined mesh.
struct CellMesh {
	Vector<Vector3> vertices;
	Vector<Color> colors;
	Vector<int> indices; // empty means non-indexed (every 3 vertices is a triangle)
};

// Transforms one cell instance's own vertices/colors/indices into world space.
CellMesh _transform_cell(const PlacedCell &p_placed) {
	CellMesh out;
	if (p_placed.scale.length_squared() == 0.0 || p_placed.cell == nullptr) {
		return out;
	}
	const CellData &cell = *p_placed.cell;
	for (int i = 0; i < cell.vertex_count; i++) {
		Vector3 v(cell.positions[i * 3 + 0], cell.positions[i * 3 + 1], cell.positions[i * 3 + 2]);
		v *= p_placed.scale;
		v = p_placed.rotation.xform(v);
		v += p_placed.position;
		out.vertices.push_back(v);
		out.colors.push_back(Color(cell.colors[i * 4 + 0], cell.colors[i * 4 + 1], cell.colors[i * 4 + 2], cell.colors[i * 4 + 3]));
	}
	for (int i = 0; i < cell.index_count; i++) {
		out.indices.push_back(cell.indices[i]);
	}
	return out;
}

// RockCluster's own minimal per-vertex property layout for Manifold's MeshGL64
// (see _cell_to_manifold()/_manifold_to_cellmesh() below) -- position (required,
// always properties 0-2 per MeshGL64's own convention) plus this generator's
// vertex color. No material/UV/smooth/invert: RockCluster doesn't use them,
// unlike Godot 4's own modules/csg integration (which this is otherwise modeled
// on) which carries those for general-purpose CSGShape3D support.
enum {
	MANIFOLD_PROPERTY_POSITION_X,
	MANIFOLD_PROPERTY_POSITION_Y,
	MANIFOLD_PROPERTY_POSITION_Z,
	MANIFOLD_PROPERTY_COLOR_R,
	MANIFOLD_PROPERTY_COLOR_G,
	MANIFOLD_PROPERTY_COLOR_B,
	MANIFOLD_PROPERTY_COLOR_A,
	MANIFOLD_PROPERTY_MAX,
};

// Builds a Manifold from one already-transformed cell's own triangle soup
// (indexed or not), carrying its per-vertex color as extra MeshGL64 properties
// (Manifold interpolates these internally through every split/merge, same
// guarantee it gives position -- no hand-written interpolation needed, unlike
// modules/csg's own uv/color threading). Returns an invalid (default-constructed)
// Manifold on failure -- see _union_cells()'s handling of manifold.Status().
manifold::Manifold _cell_to_manifold(const CellMesh &p_cell) {
	manifold::MeshGL64 mesh;
	mesh.numProp = MANIFOLD_PROPERTY_MAX;
	int tri_count = p_cell.indices.size() > 0 ? p_cell.indices.size() : p_cell.vertices.size();
	mesh.vertProperties.reserve(tri_count * MANIFOLD_PROPERTY_MAX);
	mesh.triVerts.reserve(tri_count);
	// One vertProperties/triVerts entry per triangle-corner instance (not
	// deduplicated here) -- mesh.Merge() below welds coincident positions within
	// tolerance, same as Godot 4's own _pack_manifold() relies on.
	for (int i = 0; i < tri_count; i++) {
		int idx = p_cell.indices.size() > 0 ? p_cell.indices[i] : i;
		mesh.triVerts.push_back(mesh.vertProperties.size() / MANIFOLD_PROPERTY_MAX);
		size_t begin = mesh.vertProperties.size();
		mesh.vertProperties.resize(begin + MANIFOLD_PROPERTY_MAX);
		double *vert = &mesh.vertProperties[begin];
		const Vector3 &v = p_cell.vertices[idx];
		const Color &c = p_cell.colors[idx];
		vert[MANIFOLD_PROPERTY_POSITION_X] = v.x;
		vert[MANIFOLD_PROPERTY_POSITION_Y] = v.y;
		vert[MANIFOLD_PROPERTY_POSITION_Z] = v.z;
		vert[MANIFOLD_PROPERTY_COLOR_R] = c.r;
		vert[MANIFOLD_PROPERTY_COLOR_G] = c.g;
		vert[MANIFOLD_PROPERTY_COLOR_B] = c.b;
		vert[MANIFOLD_PROPERTY_COLOR_A] = c.a;
	}
	// Godot 4's own tolerance choice (modules/csg/csg_shape.cpp) -- a small,
	// proven default rather than a guess.
	mesh.tolerance = 2 * FLT_EPSILON;
	mesh.Merge();
	return manifold::Manifold(mesh);
}

// Flattens one cell's own (possibly indexed) triangles into a plain non-indexed
// (vertices, colors) pair, matching _union_cells()'s own output shape.
void _flatten_cell(const CellMesh &p_cell, Vector<Vector3> &r_vertices, Vector<Color> &r_colors) {
	int tri_count = p_cell.indices.size() > 0 ? p_cell.indices.size() : p_cell.vertices.size();
	for (int i = 0; i + 2 < tri_count; i += 3) {
		int i0 = p_cell.indices.size() > 0 ? p_cell.indices[i] : i;
		int i1 = p_cell.indices.size() > 0 ? p_cell.indices[i + 1] : i + 1;
		int i2 = p_cell.indices.size() > 0 ? p_cell.indices[i + 2] : i + 2;
		r_vertices.push_back(p_cell.vertices[i0]);
		r_vertices.push_back(p_cell.vertices[i1]);
		r_vertices.push_back(p_cell.vertices[i2]);
		r_colors.push_back(p_cell.colors[i0]);
		r_colors.push_back(p_cell.colors[i1]);
		r_colors.push_back(p_cell.colors[i2]);
	}
}

// Reads a (union result) Manifold's own mesh back into flat (vertices, colors),
// appending to r_vertices/r_colors. No material-run bookkeeping needed, unlike
// Godot 4's own _unpack_manifold(): RockCluster has no materials.
void _manifold_to_cellmesh(const manifold::Manifold &p_manifold, Vector<Vector3> &r_vertices, Vector<Color> &r_colors) {
	manifold::MeshGL64 mesh = p_manifold.GetMeshGL64();
	for (size_t i = 0; i + 2 < mesh.triVerts.size(); i += 3) {
		for (int k = 0; k < 3; k++) {
			uint64_t vi = mesh.triVerts[i + k];
			const double *vert = &mesh.vertProperties[vi * mesh.numProp];
			r_vertices.push_back(Vector3(vert[MANIFOLD_PROPERTY_POSITION_X], vert[MANIFOLD_PROPERTY_POSITION_Y], vert[MANIFOLD_PROPERTY_POSITION_Z]));
			r_colors.push_back(Color(vert[MANIFOLD_PROPERTY_COLOR_R], vert[MANIFOLD_PROPERTY_COLOR_G], vert[MANIFOLD_PROPERTY_COLOR_B], vert[MANIFOLD_PROPERTY_COLOR_A]));
		}
	}
}

AABB _cell_aabb(const CellMesh &p_cell) {
	AABB aabb;
	if (p_cell.vertices.size() == 0) {
		return aabb;
	}
	aabb.position = p_cell.vertices[0];
	for (int i = 1; i < p_cell.vertices.size(); i++) {
		aabb.expand_to(p_cell.vertices[i]);
	}
	return aabb;
}

// Unions multiple already-transformed, non-empty cell meshes into one combined,
// non-indexed (vertices+colors) mesh via a real CSG boolean union -- this
// generator scatters cells with no collision avoidance (faithfully matching the
// original source algorithm, see memo.md item 27: the original Unity asset
// never does any mesh-combine/boolean operation either -- it just relies on
// separate opaque GameObjects and the GPU's own depth test, which Godot's
// single-combined-ArrayMesh architecture doesn't get for free), so overlapping
// cells would otherwise leave genuinely overlapping/interpenetrating geometry in
// the final output: z-fighting where two unrelated surfaces coincide, or one
// cell reading as "visible through" another once depth ordering flips with
// viewing angle.
//
// Uses Manifold (thirdparty/manifold, the same library Godot 4 replaced its own
// modules/csg with) rather than this engine's own modules/csg
// (CSGBrushOperation): that was tried first, including native per-vertex color
// support added to it and a genuine pre-existing engine bug fixed along the way
// (see memo.md) -- but measured directly to be unreliable on this generator's
// complex, faceted, non-convex cell geometry. Its mark_inside_faces() (a
// raycast-parity inside/outside classifier) spuriously marked real, unmodified
// faces as "inside" the other brush even when two cells' actual surfaces never
// touched at all (confirmed with cells placed far enough apart, radius=10, that
// none could ever truly intersect: 16/60 seeds still showed a genuine mesh gap
// from misclassification alone), and at this generator's actual default
// settings (cells deliberately close together to form a cluster) the same
// misclassification produced significant gaps in the majority of generations --
// not fixable by tuning vertex_snap. Manifold's own explicit design goal is
// "guaranteed manifold output without caveats or edge cases," directly
// targeting this exact failure mode.
//
// Only cells whose AABBs actually overlap (grouped transitively -- a chain of
// pairwise-overlapping AABBs forms one group) are ever passed through the
// boolean op together; every other cell bypasses it entirely and is flattened
// straight into the output. Free correctness + performance for the common case
// of cells nowhere near each other, independent of which boolean backend is
// used underneath.
CellMesh _union_cells(const Vector<CellMesh> &p_cells) {
	Vector<const CellMesh *> nonempty;
	Vector<AABB> aabbs;
	for (int i = 0; i < p_cells.size(); i++) {
		if (p_cells[i].vertices.size() > 0) {
			nonempty.push_back(&p_cells[i]);
			aabbs.push_back(_cell_aabb(p_cells[i]));
		}
	}

	CellMesh out;
	if (nonempty.size() == 0) {
		return out;
	}
	if (nonempty.size() == 1) {
		return *nonempty[0];
	}

	// Union-find over pairwise AABB overlap.
	Vector<int> parent;
	for (int i = 0; i < nonempty.size(); i++) {
		parent.push_back(i);
	}
	auto find = [&](int x) {
		while (parent[x] != x) {
			parent.write[x] = parent[parent[x]];
			x = parent[x];
		}
		return x;
	};
	for (int i = 0; i < nonempty.size(); i++) {
		for (int j = i + 1; j < nonempty.size(); j++) {
			if (aabbs[i].intersects_inclusive(aabbs[j])) {
				int ri = find(i), rj = find(j);
				if (ri != rj) {
					parent.write[ri] = rj;
				}
			}
		}
	}

	Map<int, Vector<int>> groups;
	for (int i = 0; i < nonempty.size(); i++) {
		int root = find(i);
		Map<int, Vector<int>>::Element *e = groups.find(root);
		if (e) {
			e->get().push_back(i);
		} else {
			Vector<int> v;
			v.push_back(i);
			groups.insert(root, v);
		}
	}

	for (Map<int, Vector<int>>::Element *e = groups.front(); e; e = e->next()) {
		const Vector<int> &members = e->get();
		if (members.size() == 1) {
			_flatten_cell(*nonempty[members[0]], out.vertices, out.colors);
			continue;
		}
		std::vector<manifold::Manifold> group_manifolds;
		for (int k = 0; k < members.size(); k++) {
			manifold::Manifold m = _cell_to_manifold(*nonempty[members[k]]);
			if (m.Status() != manifold::Manifold::Error::NoError) {
				// Rare in practice (measured: ~2 in several thousand cell-unions
				// across this generator's own regression sweeps) but real: Manifold
				// requires input that's already a proper 2-manifold, and rejects it
				// (here, specifically Error::NotManifold) otherwise -- a genuine,
				// pre-existing defect in that specific cell's own source data this
				// generator's earlier duplicate/unmatched-edge checks (see memo.md
				// items 24/26) didn't happen to catch, not something introduced by
				// this union step. Falling back to this cell's own flattened
				// geometry (skipping the boolean op for it specifically, not the
				// whole cluster) can leave a hairline seam if it genuinely
				// overlaps a sibling in this group, but that's strictly better than
				// dropping the cell or failing the whole generation over it.
				_flatten_cell(*nonempty[members[k]], out.vertices, out.colors);
				continue;
			}
			group_manifolds.push_back(m);
		}
		if (group_manifolds.empty()) {
			continue;
		}
		manifold::Manifold merged = group_manifolds.size() == 1 ? group_manifolds[0] : manifold::Manifold::BatchBoolean(group_manifolds, manifold::OpType::Add);
		_manifold_to_cellmesh(merged, out.vertices, out.colors);
	}
	return out;
}

// Transforms and appends one cell instance's vertices/colors/indices into
// the shared output buffers. Used by Crystal, which has no plane clip (crystal
// clusters aren't meant to embed in terrain the way boulders are) so cells
// can be appended straight into one shared indexed mesh.
void _write_cell(const PlacedCell &p_placed, Vector<Vector3> &r_vertices, Vector<Color> &r_colors, Vector<int> &r_indices) {
	if (p_placed.scale.length_squared() == 0.0 || p_placed.cell == nullptr) {
		return;
	}
	const CellData &cell = *p_placed.cell;
	int base = r_vertices.size();
	for (int i = 0; i < cell.vertex_count; i++) {
		Vector3 v(cell.positions[i * 3 + 0], cell.positions[i * 3 + 1], cell.positions[i * 3 + 2]);
		v *= p_placed.scale;
		v = p_placed.rotation.xform(v);
		v += p_placed.position;
		r_vertices.push_back(v);
		r_colors.push_back(Color(cell.colors[i * 4 + 0], cell.colors[i * 4 + 1], cell.colors[i * 4 + 2], cell.colors[i * 4 + 3]));
	}
	for (int i = 0; i < cell.index_count; i++) {
		r_indices.push_back(base + cell.indices[i]);
	}
}

// Returns indices into p_points forming their 2D convex hull in the XZ plane
// (Andrew's monotone chain; p_points' Y is ignored). p_points.size() < 3
// returns every index as-is.
Vector<int> _convex_hull_xz(const Vector<Vector3> &p_points) {
	int n = p_points.size();
	Vector<int> order;
	order.resize(n);
	for (int i = 0; i < n; i++) {
		order.write[i] = i;
	}
	if (n < 3) {
		return order;
	}
	// Insertion sort by (x, z) -- n is small (a handful of cut points per cell).
	for (int i = 1; i < n; i++) {
		int key = order[i];
		int j = i - 1;
		while (j >= 0 && (p_points[order[j]].x > p_points[key].x || (p_points[order[j]].x == p_points[key].x && p_points[order[j]].z > p_points[key].z))) {
			order.write[j + 1] = order[j];
			j--;
		}
		order.write[j + 1] = key;
	}
	auto cross = [&](int o, int a, int b) -> real_t {
		real_t ax = p_points[a].x - p_points[o].x, az = p_points[a].z - p_points[o].z;
		real_t bx = p_points[b].x - p_points[o].x, bz = p_points[b].z - p_points[o].z;
		return ax * bz - az * bx;
	};
	Vector<int> lower;
	for (int i = 0; i < n; i++) {
		int p = order[i];
		while (lower.size() >= 2 && cross(lower[lower.size() - 2], lower[lower.size() - 1], p) <= 0) {
			lower.resize(lower.size() - 1);
		}
		lower.push_back(p);
	}
	Vector<int> upper;
	for (int i = n - 1; i >= 0; i--) {
		int p = order[i];
		while (upper.size() >= 2 && cross(upper[upper.size() - 2], upper[upper.size() - 1], p) <= 0) {
			upper.resize(upper.size() - 1);
		}
		upper.push_back(p);
	}
	lower.resize(lower.size() - 1);
	upper.resize(upper.size() - 1);
	for (int i = 0; i < upper.size(); i++) {
		lower.push_back(upper[i]);
	}
	return lower;
}

// Traces the boundary of a clip cross-section into one or more closed loops.
// `p_cap_points` is implicitly paired: each straddling triangle in the clip loop
// contributes exactly one segment, points (2k, 2k+1) -- the two ends of that
// triangle's own cut. Adjacent straddling triangles share a cut point at the
// edge between them (both compute the same position via lerp_at_plane on that
// shared edge), so segments chain into closed loops by welding coincident
// endpoints. Returns false (leaving r_loops in an unspecified state) if any
// welded position doesn't have exactly two segment-endpoints meeting there, or
// a trace doesn't close -- both signal a non-manifold or otherwise malformed
// cut that this simple tracer can't resolve; the caller should fall back to the
// convex-hull cap in that case.
bool _trace_boundary_loops(const Vector<Vector3> &p_cap_points, Vector<Vector<int>> &r_loops) {
	int point_count = p_cap_points.size();
	if (point_count < 6) {
		return false;
	}
	// Group points by actual 3D distance rather than a rounded position-grid key:
	// two lerp_at_plane() calls for what is geometrically the same cut point (from
	// two triangles sharing the crossed edge) can each round their t slightly
	// differently, landing a few ULPs apart -- and a hard rounding grid has a
	// straddling failure mode where such a pair rounds to two *different* grid
	// cells right when the true value sits close to a cell boundary (the same
	// class of false positive documented for this generator's own test sweeps in
	// memo.md). Plain distance clustering has no boundary to straddle. `eps` is
	// comfortably above observed floating-point noise between two such
	// computations (~1e-6) and comfortably below the smallest real distinct cut
	// points seen in practice (~2e-4).
	const real_t eps = (real_t)2e-6;
	Vector<int> cluster_of;
	cluster_of.resize(point_count);
	Vector<Vector3> cluster_pos;
	Vector<Vector<int>> cluster_members;
	for (int i = 0; i < point_count; i++) {
		int found_cluster = -1;
		for (int c = 0; c < cluster_pos.size(); c++) {
			if (p_cap_points[i].distance_to(cluster_pos[c]) <= eps) {
				found_cluster = c;
				break;
			}
		}
		if (found_cluster < 0) {
			found_cluster = cluster_pos.size();
			cluster_pos.push_back(p_cap_points[i]);
			cluster_members.push_back(Vector<int>());
		}
		cluster_of.write[i] = found_cluster;
		cluster_members.write[found_cluster].push_back(i);
	}
	// A cluster with more than 2 members is a genuine "pinch point" -- several
	// separate cut segments meeting at (or extremely near) the same position,
	// which happens whenever the cut cross-section passes through an original
	// mesh vertex that several straddling triangles fan around. This is a valid
	// boundary shape (a figure-eight-like touch, not a data error), not the same
	// thing as the near-duplicate-point noise the distance clustering above
	// already accounts for -- resolving it requires deciding which pairs of
	// segments continue into which loop, done below by angular order.
	auto other_endpoint = [](int idx) { return (idx % 2 == 0) ? idx + 1 : idx - 1; };

	Vector<Vector<int>> sorted_members;
	sorted_members.resize(cluster_members.size());
	for (int c = 0; c < cluster_members.size(); c++) {
		Vector<int> members = cluster_members[c];
		int m = members.size();
		if (m % 2 != 0 || m == 0) {
			return false; // an odd number of segment-ends meeting at one point can't pair up
		}
		Vector<real_t> angles;
		angles.resize(m);
		for (int i = 0; i < m; i++) {
			Vector3 dir = p_cap_points[other_endpoint(members[i])] - cluster_pos[c];
			angles.write[i] = Math::atan2((double)dir.z, (double)dir.x);
		}
		// Insertion sort by angle -- m is small (a handful of segments per pinch point).
		for (int i = 1; i < m; i++) {
			int key_idx = members[i];
			real_t key_angle = angles[i];
			int j = i - 1;
			while (j >= 0 && angles[j] > key_angle) {
				members.write[j + 1] = members[j];
				angles.write[j + 1] = angles[j];
				j--;
			}
			members.write[j + 1] = key_idx;
			angles.write[j + 1] = key_angle;
		}
		sorted_members.write[c] = members;
	}
	// At the node reached via `arrived_at` (the cap_point index just walked to),
	// continue via the immediately-preceding segment-end in that node's angular
	// order -- the standard "extract simple faces from a planar graph" turning
	// rule, which reduces to "the other point" for an ordinary degree-2 node and
	// correctly splits a higher-degree pinch point into separate, non-crossing
	// loop passes instead of one loop self-intersecting through it.
	auto next_at_node = [&](int arrived_at) -> int {
		const Vector<int> &members = sorted_members[cluster_of[arrived_at]];
		int m = members.size();
		int pos = 0;
		for (int i = 0; i < m; i++) {
			if (members[i] == arrived_at) {
				pos = i;
				break;
			}
		}
		return members[(pos - 1 + m) % m];
	};

	int seg_count = point_count / 2;
	Vector<bool> used_seg;
	used_seg.resize(seg_count);
	for (int i = 0; i < seg_count; i++) {
		used_seg.write[i] = false;
	}

	for (int start_seg = 0; start_seg < seg_count; start_seg++) {
		if (used_seg[start_seg]) {
			continue;
		}
		Vector<int> loop;
		int start_point = start_seg * 2;
		loop.push_back(start_point);
		used_seg.write[start_seg] = true;
		int cur = start_point;
		int guard = seg_count * 2 + 4;
		while (true) {
			int seg_end = other_endpoint(cur);
			loop.push_back(seg_end);
			int next = next_at_node(seg_end);
			// Closing can arrive back at the starting segment via EITHER of its two
			// points -- next == start_point is only one of the two valid closures;
			// arriving at other_endpoint(start_point) (the segment's far end, via
			// its own node pairing) closes it just as validly, from the other
			// side. Checking only the exact index missed that case, letting the
			// walk push a spurious duplicate of start_point and continue instead
			// of stopping, corrupting the loop.
			if (next / 2 == start_seg) {
				break; // closed
			}
			int next_seg = next / 2;
			if (used_seg[next_seg]) {
				return false; // malformed -- revisits a segment without closing
			}
			used_seg.write[next_seg] = true;
			cur = next;
			if (--guard <= 0) {
				return false; // safety valve
			}
		}
		// The loop can legitimately close via a segment whose far end lands in the
		// SAME cluster as an already-recorded point (e.g. the very first point, if
		// the closing segment's node-partner is the starting point itself rather
		// than its far end) -- leaving a redundant, near-zero-length edge between
		// two adjacent loop entries. Left in, this confuses ear-clipping (a
		// near-zero-area "ear" candidate has a numerically unstable convexity/
		// inside-triangle test), so drop any point within eps of its immediate
		// (circular) predecessor.
		Vector<int> deduped;
		for (int i = 0; i < loop.size(); i++) {
			if (deduped.size() > 0 && p_cap_points[loop[i]].distance_to(p_cap_points[deduped[deduped.size() - 1]]) <= eps) {
				continue;
			}
			deduped.push_back(loop[i]);
		}
		while (deduped.size() > 1 && p_cap_points[deduped[deduped.size() - 1]].distance_to(p_cap_points[deduped[0]]) <= eps) {
			deduped.resize(deduped.size() - 1);
		}
		loop = deduped;
		if (loop.size() < 3) {
			return false;
		}
		r_loops.push_back(loop);
	}
	return true;
}

// Ear-clipping triangulation of a simple (possibly non-convex) polygon in the XZ
// plane (Y is assumed constant -- true for a set of plane-cut points). Returns
// triangle index triples into p_loop in p_loop's own rotational order, or an
// empty result if no valid ear could be found (a self-intersecting or otherwise
// non-simple polygon this basic algorithm can't handle) -- callers should fall
// back to a hull-based cap in that case.
Vector<int> _ear_clip_xz(const Vector<Vector3> &p_loop) {
	int n = p_loop.size();
	if (n < 3) {
		return Vector<int>();
	}
	real_t signed_area2 = 0;
	for (int i = 0; i < n; i++) {
		const Vector3 &a = p_loop[i];
		const Vector3 &b = p_loop[(i + 1) % n];
		signed_area2 += a.x * b.z - b.x * a.z;
	}
	if (Math::abs(signed_area2) < (real_t)1e-12) {
		return Vector<int>();
	}
	bool ccw = signed_area2 > 0;

	auto point_in_tri = [](const Vector3 &p, const Vector3 &a, const Vector3 &b, const Vector3 &c) -> bool {
		real_t d1 = (p.x - b.x) * (a.z - b.z) - (a.x - b.x) * (p.z - b.z);
		real_t d2 = (p.x - c.x) * (b.z - c.z) - (b.x - c.x) * (p.z - c.z);
		real_t d3 = (p.x - a.x) * (c.z - a.z) - (c.x - a.x) * (p.z - a.z);
		bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
		bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);
		return !(has_neg && has_pos);
	};

	Vector<int> idx;
	idx.resize(n);
	for (int i = 0; i < n; i++) {
		idx.write[i] = i;
	}
	Vector<int> triangles;
	int guard = n * n + 8;
	while (idx.size() > 3) {
		if (--guard <= 0) {
			return Vector<int>();
		}
		bool found_ear = false;
		int m = idx.size();
		for (int i = 0; i < m; i++) {
			int iprev = (i - 1 + m) % m;
			int inext = (i + 1) % m;
			const Vector3 &a = p_loop[idx[iprev]];
			const Vector3 &b = p_loop[idx[i]];
			const Vector3 &c = p_loop[idx[inext]];
			real_t cross = (b.x - a.x) * (c.z - a.z) - (b.z - a.z) * (c.x - a.x);
			bool convex = ccw ? (cross > 0) : (cross < 0);
			if (!convex) {
				continue;
			}
			bool any_inside = false;
			for (int k = 0; k < m; k++) {
				if (k == iprev || k == i || k == inext) {
					continue;
				}
				if (point_in_tri(p_loop[idx[k]], a, b, c)) {
					any_inside = true;
					break;
				}
			}
			if (any_inside) {
				continue;
			}
			triangles.push_back(idx[iprev]);
			triangles.push_back(idx[i]);
			triangles.push_back(idx[inext]);
			idx.remove(i);
			found_ear = true;
			break;
		}
		if (!found_ear) {
			return Vector<int>();
		}
	}
	triangles.push_back(idx[0]);
	triangles.push_back(idx[1]);
	triangles.push_back(idx[2]);
	return triangles;
}

// Clips one already-transformed cell to the y>=0 half-space, splitting any
// straddling triangle and lerping color at the cut, then fills the resulting
// cross-section by tracing the cut's own exact boundary loop(s) and
// ear-clip-triangulating each one (see _trace_boundary_loops()/_ear_clip_xz()).
//
// History: this used to always cap with the 2D convex hull of every cut point
// instead of the exact boundary, because this submodule's cell library (source:
// a paid Unity asset) originally had widespread non-manifold overlaps in its
// Boulder/"Cubic" cells specifically -- verified by scanning every cell's own
// raw triangle data for position-matched edge/winding consistency: 28 of 30
// Boulder cells had unmatched or duplicate-directed edges (Sharp/Crystal were
// always clean). Cutting such a mesh didn't reliably produce one simple
// traceable boundary loop back then. Those Boulder cells have since been
// repaired at the source (see gen_rockcluster_cells_data.py's
// repair_non_manifold_triangles() and this file's own memo.md) -- 0/30 Boulder
// cells now have a duplicated or unmatched edge in their raw data. With a clean
// source, the hull approach's own real cost became visible instead: whenever a
// cut cross-section is *concave*, the hull "shortcuts" across the concavity,
// which both over-fills a sliver of removed material AND, more seriously,
// leaves the true cut boundary's segments in that region without a matching
// reverse edge from the cap -- a genuine, sometimes large (0.1+ unit) gap
// between the cap and the side walls, not a hairline artifact. Measured
// directly: sweeping 300 random seeds at production defaults found *significant*
// (>0.01 unit) unmatched boundary edges in roughly half of them. This directly
// explains reports of a rock reading as "inside-out" / "material on the wrong
// side" -- backface culling makes the true (uncapped) boundary segments in a
// concave region simply not render from outside, an actual hole, not a lighting
// or winding-convention issue. The exact trace has no such gap by construction
// (it always caps precisely what was cut, convex or not) and is used whenever
// it succeeds; _trace_boundary_loops() falls back to signaling failure (and this
// function falls back to the old hull cap) only if the cut's own topology isn't
// a clean set of closed loops -- a defense-in-depth for cell data this
// generator hasn't already verified clean, not the expected path anymore.
// Capping happens per cell, before combining, because the *combined*
// multi-cell cluster's cut is several disjoint per-cell caps, not one shared
// polygon. Outputs a non-indexed triangle list.
CellMesh _clip_and_cap_cell(const CellMesh &p_in) {
	CellMesh out;

	auto lerp_at_plane = [](const Vector3 &p1, const Vector3 &p2, const Color &c1, const Color &c2, Color &r_c) -> Vector3 {
		real_t denom = p2.y - p1.y;
		real_t t = Math::is_zero_approx(denom) ? 0.0 : (-p1.y) / denom;
		t = CLAMP(t, (real_t)0.0, (real_t)1.0);
		r_c = c1.linear_interpolate(c2, t);
		return p1 + (p2 - p1) * t;
	};
	auto emit = [&](const Vector3 &a, const Vector3 &b, const Vector3 &c, const Color &ca, const Color &cb, const Color &cc) {
		out.vertices.push_back(a);
		out.vertices.push_back(b);
		out.vertices.push_back(c);
		out.colors.push_back(ca);
		out.colors.push_back(cb);
		out.colors.push_back(cc);
	};

	Vector<Vector3> cap_points;
	Vector<Color> cap_colors;
	auto add_cap_point = [&](const Vector3 &p, const Color &c) {
		cap_points.push_back(p);
		cap_colors.push_back(c);
	};

	int tri_count = p_in.indices.size() > 0 ? p_in.indices.size() : p_in.vertices.size();
	for (int i = 0; i + 2 < tri_count; i += 3) {
		int i0 = p_in.indices.size() > 0 ? p_in.indices[i] : i;
		int i1 = p_in.indices.size() > 0 ? p_in.indices[i + 1] : i + 1;
		int i2 = p_in.indices.size() > 0 ? p_in.indices[i + 2] : i + 2;
		const Vector3 &p1 = p_in.vertices[i0];
		const Vector3 &p2 = p_in.vertices[i1];
		const Vector3 &p3 = p_in.vertices[i2];
		const Color &c1 = p_in.colors[i0];
		const Color &c2 = p_in.colors[i1];
		const Color &c3 = p_in.colors[i2];
		bool s1 = p1.y >= 0.0, s2 = p2.y >= 0.0, s3 = p3.y >= 0.0;

		if (s1 == s2 && s1 == s3) {
			if (s1) {
				emit(p1, p2, p3, c1, c2, c3);
			}
			continue;
		}

		Color ic1, ic2;
		if (s1 == s2) {
			Vector3 j1 = lerp_at_plane(p2, p3, c2, c3, ic1);
			Vector3 j2 = lerp_at_plane(p3, p1, c3, c1, ic2);
			add_cap_point(j1, ic1);
			add_cap_point(j2, ic2);
			if (s1) {
				emit(p1, p2, j1, c1, c2, ic1);
				emit(p1, j1, j2, c1, ic1, ic2);
			} else {
				emit(j1, p3, j2, ic1, c3, ic2);
			}
		} else if (s1 == s3) {
			Vector3 j1 = lerp_at_plane(p1, p2, c1, c2, ic1);
			Vector3 j2 = lerp_at_plane(p2, p3, c2, c3, ic2);
			add_cap_point(j1, ic1);
			add_cap_point(j2, ic2);
			if (s1) {
				emit(p1, j1, p3, c1, ic1, c3);
				emit(j1, j2, p3, ic1, ic2, c3);
			} else {
				emit(j1, p2, j2, ic1, c2, ic2);
			}
		} else {
			Vector3 j1 = lerp_at_plane(p1, p2, c1, c2, ic1);
			Vector3 j2 = lerp_at_plane(p1, p3, c1, c3, ic2);
			add_cap_point(j1, ic1);
			add_cap_point(j2, ic2);
			if (s1) {
				emit(p1, j1, j2, c1, ic1, ic2);
			} else {
				emit(j1, p2, p3, ic1, c2, c3);
				emit(j1, p3, j2, ic1, c3, ic2);
			}
		}
	}

	// Decide a loop/fan's winding ONCE, from the aggregate of every triangle's own
	// cross product, rather than per-triangle: a single thin/near-collinear
	// triangle (common on messy source data with near-duplicate boundary points)
	// can have a cross product whose Y-sign is dominated by floating-point noise
	// rather than the true orientation, which would flip just that one triangle
	// relative to its correctly-wound neighbors if decided independently.
	auto emit_oriented = [&](const Vector3 &a, const Vector3 &b, const Vector3 &c,
								 const Color &ca, const Color &cb, const Color &cc, bool p_flip) {
		if (!p_flip) {
			emit(a, b, c, ca, cb, cc);
		} else {
			emit(a, c, b, ca, cc, cb);
		}
	};

	Vector<Vector<int>> loops;
	bool traced = _trace_boundary_loops(cap_points, loops);
	bool exact_cap_ok = traced;
	Vector<Vector<int>> loop_triangles;
	if (traced) {
		for (int li = 0; li < loops.size(); li++) {
			Vector<Vector3> loop_pts;
			for (int k = 0; k < loops[li].size(); k++) {
				loop_pts.push_back(cap_points[loops[li][k]]);
			}
			Vector<int> tris = _ear_clip_xz(loop_pts);
			if (tris.size() == 0) {
				exact_cap_ok = false;
				break;
			}
			loop_triangles.push_back(tris);
		}
	}

	if (exact_cap_ok) {
		for (int li = 0; li < loops.size(); li++) {
			const Vector<int> &loop = loops[li];
			const Vector<int> &tris = loop_triangles[li];
			Vector3 aggregate_n;
			for (int t = 0; t + 2 < tris.size(); t += 3) {
				const Vector3 &a = cap_points[loop[tris[t]]];
				const Vector3 &b = cap_points[loop[tris[t + 1]]];
				const Vector3 &c = cap_points[loop[tris[t + 2]]];
				aggregate_n += (b - a).cross(c - a);
			}
			// Orient toward the removed (below-plane) material -- same convention as
			// procrockgen.cpp's clip_and_cap(): normal should point opposite the
			// plane normal (0,1,0).
			bool flip = aggregate_n.dot(Vector3(0, 1, 0)) >= 0;
			for (int t = 0; t + 2 < tris.size(); t += 3) {
				int ia = loop[tris[t]], ib = loop[tris[t + 1]], ic = loop[tris[t + 2]];
				emit_oriented(cap_points[ia], cap_points[ib], cap_points[ic],
						cap_colors[ia], cap_colors[ib], cap_colors[ic], flip);
			}
		}
		return out;
	}

	// Fallback: fan cap over the convex hull of every cut point. Always valid,
	// non-self-intersecting geometry regardless of how messy the cross-section
	// is, but (see this function's own history above) can over-fill a concavity
	// or leave a real gap at the cap/wall seam -- only reached if the cut's own
	// topology isn't a clean set of closed loops the exact tracer can resolve.
	Vector<int> hull = _convex_hull_xz(cap_points);
	if (hull.size() >= 3) {
		Vector3 centroid;
		Color avg_color;
		for (int i = 0; i < hull.size(); i++) {
			centroid += cap_points[hull[i]];
			avg_color += cap_colors[hull[i]];
		}
		centroid /= hull.size();
		avg_color = avg_color / (real_t)hull.size();

		Vector3 aggregate_n;
		for (int i = 0; i < hull.size(); i++) {
			int next = (i + 1) % hull.size();
			aggregate_n += (cap_points[hull[i]] - centroid).cross(cap_points[hull[next]] - centroid);
		}
		bool flip = aggregate_n.dot(Vector3(0, 1, 0)) >= 0;

		for (int i = 0; i < hull.size(); i++) {
			int next = (i + 1) % hull.size();
			const Vector3 &va = cap_points[hull[i]];
			const Vector3 &vb = cap_points[hull[next]];
			Vector3 n = (va - centroid).cross(vb - centroid);
			// Skip near-degenerate fan wedges: a triangle with negligible area
			// contributes negligible real surface but an unreliable normal
			// direction, which read as small, oddly bright/white sliver faces once
			// _recompute_smooth_normals() started genuinely blending normals
			// across neighbors.
			if (n.length_squared() < (real_t)1e-8) {
				continue;
			}
			emit_oriented(centroid, va, vb, avg_color, cap_colors[hull[i]], cap_colors[hull[next]], flip);
		}
	}

	return out;
}

// Smooth per-vertex normals, accumulated from adjacent triangle face normals
// then normalized — same technique as GenRock::BuildNormals() elsewhere in
// this submodule.
//
// For non-indexed input (p_indices empty — rock_cluster_gen_boulder()'s combined
// clip+cap output) this welds vertices by position first: without that, every
// triangle's 3 vertices occupy their own private array slot with nothing else
// referencing it, so "accumulate into shared slots, then normalize" degenerates into
// each vertex just keeping its own triangle's single face normal -- flat shading
// silently disguised as smooth. That mattered visually, not just cosmetically: flat
// shading means each facet's brightness depends only on *its own* normal with no
// blending from neighbors, and this generator's angular, low-poly cells have enough
// distinct facet directions that some inevitably end up facing away from both of the
// preview dock's lights -- verified numerically, the resulting ambient-only color
// for such a facet (~0.28,0.26,0.24) sits only ~0.1 away from the dock's own
// background color (0.2,0.2,0.24), easily misread as "the background showing
// through" even though the facet is solid and correctly wound. True smoothing
// (blending each vertex's normal with its actual neighbors, weld duplicates or not)
// directly reduces how often any single point on the surface is that isolated.
Vector<Vector3> _recompute_smooth_normals(const Vector<Vector3> &p_vertices, const Vector<int> &p_indices) {
	Vector<Vector3> normals;
	normals.resize(p_vertices.size());
	for (int i = 0; i < normals.size(); i++) {
		normals.write[i] = Vector3();
	}

	// Faces are blended into a shared vertex normal only when their direction agrees
	// with each other within this angle (~60 degrees) -- otherwise summing raw face
	// normals across a real hard edge (e.g. a cap's flat rim meeting a steep side
	// wall) can (a) land near zero when contributions happen to nearly cancel,
	// previously falling back to an arbitrary hardcoded Vector3(0,1,0) that matched
	// neither surface, or (b) land on some meaningless blend direction even when it
	// doesn't cancel -- both read as wrong shading or a visible seam right at the
	// edge instead of the crisp, correctly-lit crease a real hard edge should have.
	// Found via a real generated cell: two adjacent, correctly-wound, consistently
	// downward-facing cap-fan triangles had their shared vertex normal reported as
	// straight up, traced to exactly this cancellation.
	const real_t kSmoothingCosThreshold = 0.5;
	// CMP_EPSILON (1e-5) is tuned for *normalized* comparisons; these cells are tiny
	// (~0.01-0.1 units), so a raw, un-normalized cross product's squared length --
	// which scales with the *fourth power* of edge length -- routinely lands well
	// under 1e-5 for a perfectly valid, non-degenerate triangle. Using CMP_EPSILON
	// here misfired as "degenerate" on ordinary small triangles, hard-defaulting
	// their direction to (0,1,0) almost everywhere. This only needs to catch a truly
	// zero-area triangle (coincident/collinear points), so use a much smaller floor.
	const real_t kDegenerateLenSq = (real_t)1e-18;

	if (p_indices.size() == 0) {
		// Weld by quantized position so triangles that happen to share a vertex
		// position (but not an array slot) contribute to the same accumulated normal.
		auto weld_key = [](const Vector3 &p) {
			const real_t q = 100000.0;
			return String::num_int64((int64_t)Math::round((double)p.x * q)) + "," +
					String::num_int64((int64_t)Math::round((double)p.y * q)) + "," +
					String::num_int64((int64_t)Math::round((double)p.z * q));
		};
		int tri_count = p_vertices.size() / 3;
		Vector<Vector3> face_normals;
		face_normals.resize(tri_count);
		Map<String, Vector<int>> group_faces; // weld key -> triangle indices touching it
		for (int t = 0; t < tri_count; t++) {
			int i = t * 3;
			const Vector3 &p0 = p_vertices[i], &p1 = p_vertices[i + 1], &p2 = p_vertices[i + 2];
			Vector3 n = (p1 - p0).cross(p2 - p0);
			face_normals.write[t] = n;
			for (const Vector3 &p : { p0, p1, p2 }) {
				String k = weld_key(p);
				Map<String, Vector<int>>::Element *e = group_faces.find(k);
				if (e) {
					e->get().push_back(t);
				} else {
					Vector<int> v;
					v.push_back(t);
					group_faces.insert(k, v);
				}
			}
		}
		for (int i = 0; i < p_vertices.size(); i++) {
			int self_t = i / 3;
			Vector3 self_n = face_normals[self_t];
			Vector3 self_dir = self_n.length_squared() > kDegenerateLenSq ? self_n.normalized() : Vector3(0, 1, 0);
			Vector3 accum;
			const Vector<int> &touching = group_faces.find(weld_key(p_vertices[i]))->get();
			for (int j = 0; j < touching.size(); j++) {
				Vector3 fn = face_normals[touching[j]];
				if (fn.length_squared() < kDegenerateLenSq) {
					continue;
				}
				if (fn.normalized().dot(self_dir) >= kSmoothingCosThreshold) {
					accum += fn;
				}
			}
			normals.write[i] = accum.length_squared() > kDegenerateLenSq ? accum.normalized() : self_dir;
		}
		return normals;
	}

	// Same angle-thresholded blending as the non-indexed path above, keyed by shared
	// index instead of welded position (this path's input is already a real indexed
	// mesh -- see _write_cell()'s caller) -- a genuinely indexed mesh can still have
	// a hard edge where two very differently-angled faces share a vertex, with the
	// same cancellation risk if blended unconditionally.
	int vert_count = p_vertices.size();
	Vector<Vector<int>> vertex_faces; // per vertex, indices into face_normals
	vertex_faces.resize(vert_count);
	int tri_count = p_indices.size() / 3;
	Vector<Vector3> face_normals;
	face_normals.resize(tri_count);
	for (int t = 0; t < tri_count; t++) {
		int i = t * 3;
		int i0 = p_indices[i], i1 = p_indices[i + 1], i2 = p_indices[i + 2];
		Vector3 n = (p_vertices[i1] - p_vertices[i0]).cross(p_vertices[i2] - p_vertices[i0]);
		face_normals.write[t] = n;
		vertex_faces.write[i0].push_back(t);
		vertex_faces.write[i1].push_back(t);
		vertex_faces.write[i2].push_back(t);
	}
	for (int i = 0; i + 2 < p_indices.size(); i += 3) {
		int idx[3] = { p_indices[i], p_indices[i + 1], p_indices[i + 2] };
		int t = i / 3;
		Vector3 self_n = face_normals[t];
		Vector3 self_dir = self_n.length_squared() > kDegenerateLenSq ? self_n.normalized() : Vector3(0, 1, 0);
		for (int k = 0; k < 3; k++) {
			int vi = idx[k];
			if (normals[vi].length_squared() > CMP_EPSILON) {
				continue; // already computed for this vertex index -- checks the final, normalized (unit-length) result, so CMP_EPSILON is fine here
			}
			Vector3 accum;
			const Vector<int> &touching = vertex_faces[vi];
			for (int j = 0; j < touching.size(); j++) {
				Vector3 fn = face_normals[touching[j]];
				if (fn.length_squared() < kDegenerateLenSq) {
					continue;
				}
				if (fn.normalized().dot(self_dir) >= kSmoothingCosThreshold) {
					accum += fn;
				}
			}
			normals.write[vi] = accum.length_squared() > kDegenerateLenSq ? accum.normalized() : self_dir;
		}
	}
	return normals;
}

// Cell meshes carry no usable UV (Boulder/Sharp have none at all; Crystal's
// texcoord0 is an unusual 4-component attribute not worth chasing) — apply
// the same per-vertex box-UV RockGen uses for the same reason (see
// generators/rockgen/rockgen.cpp), reusing box_uv.h's shared helpers.
Vector<Vector2> _box_uv(const Vector<Vector3> &p_vertices, const Vector<Vector3> &p_normals) {
	Vector<Vector2> uvs;
	uvs.resize(p_vertices.size());
	for (int i = 0; i < p_vertices.size(); i++) {
		int box_dir = rock_studio_get_box_dir(p_normals[i]);
		uvs.write[i] = rock_studio_get_box_uv(p_vertices[i], box_dir);
	}
	return uvs;
}

} // namespace

Array rock_cluster_gen_boulder(bool p_sharp, int p_density, real_t p_radius, real_t p_asymmetry, real_t p_wave, real_t p_decentralize,
		real_t p_scale_local, const Ref<Curve> &p_scale_by_distance, real_t p_tallness, real_t p_flatness, real_t p_wideness,
		real_t p_rotation_degrees, real_t p_rotation_local_degrees, real_t p_rotation_rnd, int p_randseed) {
	Ref<RandomNumberGenerator> rng;
	rng.instance();
	if (p_randseed == 0) {
		rng->randomize();
	} else {
		rng->set_seed(p_randseed);
	}

	const CellData *cells = p_sharp ? kSharpCells : kBoulderCells;
	int cell_count = p_sharp ? kSharpCellCount : kBoulderCellCount;

	// The cull-cell-if-too-small threshold below scales with p_scale_local rather
	// than being a fixed absolute constant: it was originally an absolute 0.08
	// calibrated against a default scale_local of 2 — dividing scale_local by 4
	// without dividing this threshold too would silently cull ~4x as many cells
	// (any dist_curve between the old and new relative cutoff, which used to
	// produce a small-but-visible cell, now produces scale=0 instead), which is
	// exactly what happened when this generator's defaults were first tuned down.
	const real_t kScaleLimitMin = 0.08 * (p_scale_local / 2.0);

	Vector<CellMesh> placed_cells;

	for (int i = 0; i < p_density; i++) {
		PlacedCell placed;
		placed.cell = &cells[i % cell_count];

		// Disc-distributed position, biased toward/away from center by decentralize.
		real_t pow_exp = _lerp_clamped(0.66, 0.25, p_decentralize);
		real_t random_distance = rng->randf();
		real_t d = Math::pow(random_distance, pow_exp);
		real_t r = d * p_radius;
		real_t theta = rng->randf() * 2.0 * Math_PI;
		placed.position = Vector3(r * Math::cos(theta), 0, r * Math::sin(theta));

		// Base rotation plus distance-scaled random jitter (not bit-exact to any
		// particular Euler convention — not load-bearing for a randomized scatter
		// look, same accepted-approximation rationale as this submodule's noise graph).
		Vector3 rot_degrees(0, 0, p_rotation_degrees);
		if (p_rotation_rnd > 0.0) {
			Vector3 local_center(p_asymmetry * p_radius, 0, 0);
			real_t distance = (placed.position - local_center).length();
			distance = p_radius != 0 ? distance / p_radius : 0;
			if (distance > 0.5) {
				real_t rnd_rot_factor = p_rotation_rnd * 360.0 * distance;
				rot_degrees.x += rng->randf() * rnd_rot_factor;
				rot_degrees.y += rng->randf() * rnd_rot_factor;
				rot_degrees.z += rng->randf() * rnd_rot_factor;
			}
		}
		Basis rot_basis;
		rot_basis.rotate(Vector3(1, 0, 0), Math::deg2rad(rot_degrees.x));
		rot_basis.rotate(Vector3(0, 1, 0), Math::deg2rad(rot_degrees.y));
		rot_basis.rotate(Vector3(0, 0, 1), Math::deg2rad(rot_degrees.z));
		placed.rotation = rot_basis.get_rotation_quat();
		// Additional local-space spin around Y (post-multiplied).
		placed.rotation *= Quat(Vector3(0, 1, 0), Math::deg2rad(p_rotation_local_degrees));

		// Distance-based scale falloff plus tallness/flatness/wideness shaping.
		Vector3 local_center(p_asymmetry * p_radius, 0, 0);
		real_t distance = p_radius != 0 ? (placed.position - local_center).length() / p_radius : 0;
		real_t dist_curve = p_scale_by_distance->interpolate(1.0 - distance);
		real_t dist_curve_reverse = p_scale_by_distance->interpolate(distance);
		real_t local_scale = dist_curve * p_scale_local;

		real_t wave_factor = 0.0;
		if (distance > 0.3) {
			real_t wave_position = _lerp_clamped(p_radius, -p_radius, p_wave);
			real_t wave_dist = Math::abs(placed.position.x - wave_position);
			wave_factor = _inverse_lerp_clamped(p_radius / 4.0, 0.0, wave_dist) * (dist_curve_reverse * 4.0);
		}
		local_scale += local_scale * wave_factor;

		if (local_scale > kScaleLimitMin) {
			real_t m_tallness = _lerp_clamped(1.0, 3.0, dist_curve * p_tallness);
			real_t m_flatness = _lerp_clamped(0.0, 3.0, dist_curve_reverse * p_flatness);
			real_t m_wideness = _lerp_clamped(0.0, 3.0, dist_curve_reverse * p_wideness);
			real_t height = CLAMP(m_tallness - m_flatness, (real_t)0.1, (real_t)4.0);
			placed.scale = Vector3(1 + m_wideness, height, 1 + m_wideness) * local_scale;
		} else {
			placed.scale = Vector3(); // zero — _transform_cell() skips it, matching the original
		}

		placed_cells.push_back(_transform_cell(placed));
	}

	// Union all placed cells into one solid before clipping+capping, rather than
	// clip+cap each cell individually and simply concatenate the results: cells
	// are scattered with no collision avoidance (memo.md item 27), so without a
	// real boolean union, overlapping cells would leave genuinely interpenetrating
	// geometry (z-fighting, one cell "visible through" another) in the final
	// output -- see _union_cells() above. Unioning first, then clipping+capping
	// the combined solid exactly once, also means a single cut boundary/cap for
	// the cluster's true combined cross-section instead of one (potentially
	// overlapping-with-a-neighbor's) cap per individual cell.
	CellMesh unioned = _union_cells(placed_cells);
	CellMesh capped = _clip_and_cap_cell(unioned);

	Vector<Vector3> normals = _recompute_smooth_normals(capped.vertices, Vector<int>());

	Array arrays;
	arrays.resize(VS::ARRAY_MAX);
	if (capped.vertices.size() > 0) {
		arrays[VS::ARRAY_VERTEX] = capped.vertices;
		arrays[VS::ARRAY_NORMAL] = normals;
		arrays[VS::ARRAY_COLOR] = capped.colors;
		arrays[VS::ARRAY_TEX_UV] = _box_uv(capped.vertices, normals);
	}
	return arrays;
}

Array rock_cluster_gen_crystal(int p_density, real_t p_scale_local, real_t p_scale_by_angle, real_t p_scale_random_offset, real_t p_scale_bias, real_t p_bloom, int p_randseed) {
	Ref<RandomNumberGenerator> rng;
	rng.instance();
	if (p_randseed == 0) {
		rng->randomize();
	} else {
		rng->set_seed(p_randseed);
	}

	Ref<OpenSimplexNoise> noise;
	noise.instance();
	if (p_randseed != 0) {
		noise->set_seed(p_randseed);
	}

	static const real_t kSpacing = 0.3;
	static const real_t kScaleRandom = 0.65;

	int items_count = MAX(0, p_density - 1);
	int items_per_row = MAX(1, (int)Math::ceil(Math::sqrt((real_t)items_count)));
	Vector3 start_position(-(items_per_row - 1) * kSpacing * 0.5, 0, (items_per_row - 1) * kSpacing * 0.5);

	Vector<Vector3> vertices;
	Vector<Color> colors;
	Vector<int> indices;

	for (int i = 0; i < p_density; i++) {
		PlacedCell placed;
		placed.cell = &kCrystalCells[i % kCrystalCellCount];

		// Grid-cluster position around a center cell.
		if (i == 0) {
			placed.position = Vector3();
		} else {
			int adjusted = i - 1;
			int row = adjusted / items_per_row;
			int column = adjusted % items_per_row;
			placed.position = start_position + Vector3(column * kSpacing, 0, -row * kSpacing);
			real_t half_spacing = kSpacing / 2.0;
			placed.position += Vector3(rng->randf() * half_spacing, 0, rng->randf() * half_spacing);
		}

		// Orientation blend from "pointing away from center" to "pointing up".
		real_t distance_for_rot = CLAMP(placed.position.length(), (real_t)0.0, (real_t)0.9);
		Vector3 final_direction = (-placed.position.normalized()).linear_interpolate(Vector3(0, 1, 0), distance_for_rot * p_bloom);
		// Vector3::slerp requires normalized inputs and isn't defined for the zero
		// vector; linear_interpolate above is an accepted approximation for this
		// blend (only the resulting direction matters, not the interpolation
		// path taken to reach it, since it just feeds _look_rotation() below).
		if (final_direction.length_squared() > CMP_EPSILON) {
			placed.rotation = _look_rotation(final_direction, Vector3(0, 1, 0)).get_rotation_quat();
		} else {
			placed.rotation = Quat();
		}
		// Additional world-space spin around Y (pre-multiplied).
		placed.rotation = Quat(Vector3(0, 1, 0), Math::deg2rad(45.0 * rng->randf())) * placed.rotation;

		// Noise-driven scale with distance-based falloff.
		real_t noise_offset = p_scale_random_offset + rng->randf() * 3.0;
		real_t raw_noise = noise->get_noise_2d(noise_offset + placed.position.x, noise_offset + placed.position.z);
		real_t perlin = (raw_noise + 1.0) * 0.5; // remap OpenSimplexNoise's [-1,1] to [0,1]
		real_t noise_scale = _lerp_clamped(1.0, perlin, kScaleRandom);
		real_t distance_for_scale = placed.position.distance_to(Vector3(p_scale_bias * kSpacing * 0.75, 0, 0)) / ((items_per_row + 1) * kSpacing / 2.0);
		real_t f = MAX(1.0 - distance_for_scale, 0.15);
		real_t distanced_scale = _lerp_clamped(1.0, f, p_scale_by_angle);
		placed.scale = Vector3(1, 1, 1) * (distanced_scale * noise_scale * p_scale_local);

		_write_cell(placed, vertices, colors, indices);
	}

	Vector<Vector3> normals = _recompute_smooth_normals(vertices, indices);

	Array arrays;
	arrays.resize(VS::ARRAY_MAX);
	if (vertices.size() > 0) {
		arrays[VS::ARRAY_VERTEX] = vertices;
		arrays[VS::ARRAY_NORMAL] = normals;
		arrays[VS::ARRAY_COLOR] = colors;
		arrays[VS::ARRAY_INDEX] = indices;
		arrays[VS::ARRAY_TEX_UV] = _box_uv(vertices, normals);
	}
	return arrays;
}

Array rock_cluster_debug_clip_single_cell(int p_style, int p_cell_index) {
	const CellData *cell = nullptr;
	if (p_style == 0 && p_cell_index >= 0 && p_cell_index < kBoulderCellCount) {
		cell = &kBoulderCells[p_cell_index];
	} else if (p_style == 1 && p_cell_index >= 0 && p_cell_index < kSharpCellCount) {
		cell = &kSharpCells[p_cell_index];
	} else if (p_style == 2 && p_cell_index >= 0 && p_cell_index < kCrystalCellCount) {
		cell = &kCrystalCells[p_cell_index];
	}
	Array out;
	out.resize(VS::ARRAY_MAX);
	if (cell == nullptr) {
		return out;
	}

	PlacedCell placed;
	placed.cell = cell;
	placed.position = Vector3();
	placed.rotation = Quat();
	placed.scale = Vector3(1, 1, 1);

	CellMesh transformed = _transform_cell(placed);
	CellMesh result = p_style == 2 ? transformed : _clip_and_cap_cell(transformed);

	if (result.vertices.size() > 0) {
		out[VS::ARRAY_VERTEX] = result.vertices;
	}
	return out;
}
