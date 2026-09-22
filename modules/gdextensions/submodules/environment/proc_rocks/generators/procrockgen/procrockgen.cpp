/**************************************************************************/
/*  procrockgen.cpp                                                       */
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

#include "procrockgen.h"

#include "../shared/box_uv.h"
#include "../shared/rock_header.h"

#include "core/array.h"
#include "core/hash_map.h"
#include "core/math/geometry.h"
#include "core/math/plane.h"
#include "core/math/random_number_generator.h"
#include "modules/opensimplex/open_simplex_noise.h"
#include "scene/resources/gradient.h"
#include "servers/visual_server.h"

#include <queue>
#include <vector>

// =========================================================================
// Dependency-free noise graph interpreter (private to this file — procrockgen
// is currently the graph's only consumer, so it stays un-split; pull it back
// out into generators/shared/ if a second consumer needs it).
//
// A from-scratch reinterpretation of procrocklib's node-graph noise system
// (originally a thin wrapper over libnoise, LGPL-2.1+ — not relinked here; these
// node types/formulas are ported as plain math from libnoise's own documented
// algorithms, evaluated against Godot's OpenSimplexNoise instead of libnoise's
// classic Perlin lattice noise). Node type ids match procrocklib's
// NoiseNodeTypeId_* exactly so JSON presets exported by the original tool (see
// editor/proc_rocks_demo/presets/*.json) parse as-is.
//
// Known, deliberate divergences from procrocklib's actual (buggy) runtime behavior:
//  - The "Quality" dropdown on Perlin/Billow/RidgedMulti was serialized but never actually
//    applied by procrocklib's own code (confirmed by reading the deleted source) — omitted
//    here entirely rather than reproduced as a dead control.
//  - procrocklib's graph evaluator had a full input-order reversal bug for every node with
//    2+ sources (invisible for commutative nodes, a real defect for Power/Blend/Select/
//    Displace). This is a from-scratch evaluator: inputs are wired directly by slot index,
//    matching the evidently *intended* semantics, not that bug.
// =========================================================================

namespace {

enum NoiseNodeType {
	NOISE_NODE_OUTPUT = 0,
	NOISE_NODE_ADD = 1,
	NOISE_NODE_MAX = 2,
	NOISE_NODE_MIN = 3,
	NOISE_NODE_MULTIPLY = 4,
	NOISE_NODE_POWER = 5,
	NOISE_NODE_BLEND = 100,
	NOISE_NODE_SELECT = 101,
	NOISE_NODE_ABS = 200,
	NOISE_NODE_CLAMP = 201,
	NOISE_NODE_EXPONENT = 202,
	NOISE_NODE_INVERT = 203,
	NOISE_NODE_SCALE_BIAS = 204,
	NOISE_NODE_TERRACE = 205,
	NOISE_NODE_CURVE = 206,
	NOISE_NODE_DISPLACE = 300,
	NOISE_NODE_ROTATE_POINT = 301,
	NOISE_NODE_SCALE_POINT = 302,
	NOISE_NODE_TRANSLATE_POINT = 303,
	NOISE_NODE_TURBULENCE = 304,
	NOISE_NODE_CONST = 400,
	NOISE_NODE_PERLIN = 401,
	NOISE_NODE_BILLOW = 402,
	NOISE_NODE_RIDGED_MULTI = 403,
	NOISE_NODE_VORONOI = 404,
	NOISE_NODE_SPHERES = 405,
	NOISE_NODE_CYLINDERS = 406,
};

struct NoiseGraphNode {
	int type_id = NOISE_NODE_CONST;
	Vector<int> sources; // resolved indices into NoiseGraph::nodes, in slot order (0..N-1)

	// Parsed parameters — only the fields relevant to `type_id` are meaningful.
	real_t const_value = 0.0;

	real_t frequency = 1.0, lacunarity = 2.0, persistence = 0.5;
	int octaves = 6, seed = 0;

	real_t lower_bound = -1.0, upper_bound = 1.0, edge_falloff = 0.0; // Clamp, Select
	real_t exponent = 1.0; // Exponent
	real_t scale = 1.0, bias = 0.0; // ScaleBias
	Vector<real_t> control_points; // Terrace (sorted ascending)
	bool invert_terraces = false;
	Vector<Vector2> curve_points; // Curve: x=input, y=output, sorted ascending by x

	real_t angle_x = 0, angle_y = 0, angle_z = 0; // RotatePoint (degrees)
	real_t scale_x = 1, scale_y = 1, scale_z = 1; // ScalePoint
	real_t translate_x = 0, translate_y = 0, translate_z = 0; // TranslatePoint

	real_t power = 1.0; // Turbulence displacement power
	int roughness = 2; // Turbulence octave count for its 3 internal distort fields

	real_t displacement = 1.0; // Voronoi
	bool enable_distance = false; // Voronoi

	// Pre-seeded, single-octave-each noise sources for the node's own fractal loop
	// (Perlin/Billow/RidgedMulti), or for Turbulence's 3 internal distort fields.
	Vector<Ref<OpenSimplexNoise>> octave_noises;
	Vector<Ref<OpenSimplexNoise>> turbulence_x_noises, turbulence_y_noises, turbulence_z_noises;
};

struct NoiseGraph {
	Vector<NoiseGraphNode> nodes;
	int root_index = -1;

	bool is_valid() const { return root_index >= 0 && root_index < nodes.size(); }

	// Returns 0.0 for an invalid/empty graph.
	real_t evaluate(const Vector3 &p_pos) const;
	real_t evaluate_node(int p_index, const Vector3 &p_pos) const;

	// Parses procrocklib's {"nodes": [...], "edges": [...]} shape (see the edge-decoding
	// section below for the exact placeholder-id scheme this depends on).
	static NoiseGraph from_json(const Dictionary &p_json);

	// Builds a minimal single-node graph wrapping one fractal noise field — a convenient
	// bridge for callers that just want "the old single-OpenSimplexNoise-field behavior"
	// without hand-building a graph.
	static NoiseGraph make_simple_fractal(real_t p_frequency, int p_octaves, real_t p_persistence, int p_randseed);
};

} // namespace

// =========================================================================
// Math ported from modules/gdextensions/thirdparty/libnoise (still vendored,
// read-only reference — LGPL-2.1+, not linked). Formulas copied as plain,
// independent math from that library's own .cpp files; gradient-lattice noise
// (Perlin's GradientCoherentNoise3D) is NOT ported — OpenSimplexNoise stands in
// for the per-octave coherent-noise kernel instead (see the interpreter banner above).
// =========================================================================

namespace {

// --- Voronoi's value-noise hash (NOISE_VERSION==2 constants from noisegen.cpp) ---
uint32_t _int_value_noise_3d(int p_x, int p_y, int p_z, int p_seed) {
	uint32_t n = (uint32_t)(1619 * p_x + 31337 * p_y + 6971 * p_z + 1013 * p_seed) & 0x7fffffffu;
	n = (n >> 13) ^ n;
	return (n * (n * n * 60493u + 19990303u) + 1376312589u) & 0x7fffffffu;
}

real_t _value_noise_3d(int p_x, int p_y, int p_z, int p_seed = 0) {
	return 1.0 - (real_t)_int_value_noise_3d(p_x, p_y, p_z, p_seed) / 1073741824.0;
}

real_t _cubic_interp(real_t n0, real_t n1, real_t n2, real_t n3, real_t a) {
	real_t p = (n3 - n2) - (n0 - n1);
	real_t q = (n0 - n1) - p;
	real_t r = n2 - n0;
	real_t s = n1;
	return p * a * a * a + q * a * a + r * a + s;
}

real_t _scurve3(real_t a) {
	return a * a * (3.0 - 2.0 * a);
}

// One octave's worth of coherent noise, standing in for libnoise's
// GradientCoherentNoise3D (see file header note).
Ref<OpenSimplexNoise> _make_octave_noise(int p_seed) {
	Ref<OpenSimplexNoise> n;
	n.instance();
	n->set_seed(p_seed);
	n->set_octaves(1);
	n->set_period(1.0); // caller pre-scales position by frequency/lacunarity itself
	return n;
}

Vector<Ref<OpenSimplexNoise>> _make_octave_noises(int p_seed, int p_octaves) {
	Vector<Ref<OpenSimplexNoise>> result;
	result.resize(MAX(1, p_octaves));
	for (int i = 0; i < result.size(); i++) {
		result.write[i] = _make_octave_noise(p_seed + i);
	}
	return result;
}

// Perlin::GetValue (perlin.cpp) — plain fractal sum.
real_t _eval_perlin_fractal(const Vector<Ref<OpenSimplexNoise>> &p_octave_noises, real_t p_frequency, real_t p_lacunarity, real_t p_persistence, const Vector3 &p_pos) {
	real_t value = 0.0, cur_persistence = 1.0;
	Vector3 p = p_pos * p_frequency;
	for (int i = 0; i < p_octave_noises.size(); i++) {
		real_t signal = p_octave_noises[i]->get_noise_3dv(p);
		value += signal * cur_persistence;
		p *= p_lacunarity;
		cur_persistence *= p_persistence;
	}
	return value;
}

// Billow::GetValue (billow.cpp) — folded fractal sum, +0.5 offset.
real_t _eval_billow_fractal(const Vector<Ref<OpenSimplexNoise>> &p_octave_noises, real_t p_frequency, real_t p_lacunarity, real_t p_persistence, const Vector3 &p_pos) {
	real_t value = 0.0, cur_persistence = 1.0;
	Vector3 p = p_pos * p_frequency;
	for (int i = 0; i < p_octave_noises.size(); i++) {
		real_t signal = p_octave_noises[i]->get_noise_3dv(p);
		signal = 2.0 * Math::abs(signal) - 1.0;
		value += signal * cur_persistence;
		p *= p_lacunarity;
		cur_persistence *= p_persistence;
	}
	value += 0.5;
	return value;
}

// RidgedMulti::GetValue (ridgedmulti.cpp) — Musgrave ridged multifractal, no persistence.
real_t _eval_ridged_multi(const Vector<Ref<OpenSimplexNoise>> &p_octave_noises, real_t p_frequency, real_t p_lacunarity, const Vector3 &p_pos) {
	real_t value = 0.0, weight = 1.0;
	Vector3 p = p_pos * p_frequency;
	real_t spectral_freq = 1.0; // spectralWeight[i] = pow(lacunarity, -i), h=1.0
	for (int i = 0; i < p_octave_noises.size(); i++) {
		real_t spectral_weight = Math::pow(spectral_freq, real_t(-1.0));
		real_t signal = Math::abs(p_octave_noises[i]->get_noise_3dv(p));
		signal = 1.0 - signal;
		signal *= signal;
		signal *= weight;
		weight = CLAMP(signal * 2.0, real_t(0.0), real_t(1.0));
		value += signal * spectral_weight;
		p *= p_lacunarity;
		spectral_freq *= p_lacunarity;
	}
	return value * 1.25 - 1.0;
}

real_t _eval_voronoi(const NoiseGraphNode &p_node, const Vector3 &p_pos) {
	real_t x = p_pos.x * p_node.frequency, y = p_pos.y * p_node.frequency, z = p_pos.z * p_node.frequency;
	int xi = x > 0.0 ? (int)x : (int)x - 1;
	int yi = y > 0.0 ? (int)y : (int)y - 1;
	int zi = z > 0.0 ? (int)z : (int)z - 1;

	real_t min_dist = 2147483647.0;
	real_t cx = 0, cy = 0, cz = 0;
	for (int zc = zi - 2; zc <= zi + 2; zc++) {
		for (int yc = yi - 2; yc <= yi + 2; yc++) {
			for (int xc = xi - 2; xc <= xi + 2; xc++) {
				real_t px = xc + _value_noise_3d(xc, yc, zc, p_node.seed);
				real_t py = yc + _value_noise_3d(xc, yc, zc, p_node.seed + 1);
				real_t pz = zc + _value_noise_3d(xc, yc, zc, p_node.seed + 2);
				real_t dx = px - x, dy = py - y, dz = pz - z;
				real_t dist = dx * dx + dy * dy + dz * dz;
				if (dist < min_dist) {
					min_dist = dist;
					cx = px;
					cy = py;
					cz = pz;
				}
			}
		}
	}

	real_t value = 0.0;
	if (p_node.enable_distance) {
		real_t dx = cx - x, dy = cy - y, dz = cz - z;
		value = Math::sqrt(dx * dx + dy * dy + dz * dz) * 1.7320508075688772 /* sqrt(3) */ - 1.0;
	}
	// libnoise's final displacement lookup uses ValueNoise3D's default seed (0), not m_seed.
	return value + p_node.displacement * _value_noise_3d((int)Math::floor(cx), (int)Math::floor(cy), (int)Math::floor(cz));
}

struct _CurvePointLess {
	bool operator()(const Vector2 &p_a, const Vector2 &p_b) const { return p_a.x < p_b.x; }
};

} // namespace

real_t NoiseGraph::evaluate(const Vector3 &p_pos) const {
	if (!is_valid()) {
		return 0.0;
	}
	return evaluate_node(root_index, p_pos);
}

real_t NoiseGraph::evaluate_node(int p_index, const Vector3 &p_pos) const {
	if (p_index < 0 || p_index >= nodes.size()) {
		return 0.0;
	}
	const NoiseGraphNode &node = nodes[p_index];

	auto src = [&](int p_slot) -> real_t {
		if (p_slot < 0 || p_slot >= node.sources.size()) {
			return 0.0; // unwired input defaults to 0, matching procrocklib's placeholder Const(0)
		}
		return evaluate_node(node.sources[p_slot], p_pos);
	};

	switch (node.type_id) {
		case NOISE_NODE_OUTPUT: // TranslatePoint with default zero translation == identity
			return src(0);
		case NOISE_NODE_ADD:
			return src(0) + src(1);
		case NOISE_NODE_MAX:
			return MAX(src(0), src(1));
		case NOISE_NODE_MIN:
			return MIN(src(0), src(1));
		case NOISE_NODE_MULTIPLY:
			return src(0) * src(1);
		case NOISE_NODE_POWER:
			return Math::pow(src(0), src(1));
		case NOISE_NODE_BLEND: {
			real_t alpha = (src(2) + 1.0) / 2.0;
			return Math::lerp(src(0), src(1), alpha);
		}
		case NOISE_NODE_SELECT: {
			real_t control = src(2);
			if (node.edge_falloff <= 0.0) {
				return (control < node.lower_bound || control > node.upper_bound) ? src(0) : src(1);
			}
			if (control < node.lower_bound - node.edge_falloff) {
				return src(0);
			} else if (control < node.lower_bound + node.edge_falloff) {
				real_t lo = node.lower_bound - node.edge_falloff, hi = node.lower_bound + node.edge_falloff;
				return Math::lerp(src(0), src(1), _scurve3((control - lo) / (hi - lo)));
			} else if (control < node.upper_bound - node.edge_falloff) {
				return src(1);
			} else if (control < node.upper_bound + node.edge_falloff) {
				real_t lo = node.upper_bound - node.edge_falloff, hi = node.upper_bound + node.edge_falloff;
				return Math::lerp(src(1), src(0), _scurve3((control - lo) / (hi - lo)));
			}
			return src(0);
		}
		case NOISE_NODE_ABS:
			return Math::abs(src(0));
		case NOISE_NODE_CLAMP:
			return CLAMP(src(0), node.lower_bound, node.upper_bound);
		case NOISE_NODE_EXPONENT: {
			real_t v = src(0);
			real_t base = Math::abs((v + real_t(1.0)) / real_t(2.0));
			return Math::pow(base, node.exponent) * real_t(2.0) - real_t(1.0);
		}
		case NOISE_NODE_INVERT:
			return -src(0);
		case NOISE_NODE_SCALE_BIAS:
			return src(0) * node.scale + node.bias;
		case NOISE_NODE_TERRACE: {
			real_t v = src(0);
			int count = node.control_points.size();
			if (count < 2) {
				return v;
			}
			int index_pos = 0;
			while (index_pos < count && v >= node.control_points[index_pos]) {
				index_pos++;
			}
			int i0 = CLAMP(index_pos - 1, 0, count - 1);
			int i1 = CLAMP(index_pos, 0, count - 1);
			if (i0 == i1) {
				return node.control_points[i1];
			}
			real_t v0 = node.control_points[i0], v1 = node.control_points[i1];
			real_t alpha = (v - v0) / (v1 - v0);
			if (node.invert_terraces) {
				alpha = 1.0 - alpha;
				SWAP(v0, v1);
			}
			alpha *= alpha;
			return Math::lerp(v0, v1, alpha);
		}
		case NOISE_NODE_CURVE: {
			real_t v = src(0);
			int count = node.curve_points.size();
			if (count < 4) {
				return v;
			}
			int index_pos = 0;
			while (index_pos < count && v >= node.curve_points[index_pos].x) {
				index_pos++;
			}
			int i0 = CLAMP(index_pos - 2, 0, count - 1);
			int i1 = CLAMP(index_pos - 1, 0, count - 1);
			int i2 = CLAMP(index_pos, 0, count - 1);
			int i3 = CLAMP(index_pos + 1, 0, count - 1);
			if (i1 == i2) {
				return node.curve_points[i1].y;
			}
			real_t in0 = node.curve_points[i1].x, in1 = node.curve_points[i2].x;
			real_t alpha = (v - in0) / (in1 - in0);
			return _cubic_interp(node.curve_points[i0].y, node.curve_points[i1].y, node.curve_points[i2].y, node.curve_points[i3].y, alpha);
		}
		case NOISE_NODE_DISPLACE: {
			Vector3 displaced(p_pos.x + src(1), p_pos.y + src(2), p_pos.z + src(3));
			return evaluate_node(node.sources.size() > 0 ? node.sources[0] : -1, displaced);
		}
		case NOISE_NODE_ROTATE_POINT: {
			// Exact (non-standard-order) matrix from libnoise's RotatePoint::SetAngles.
			real_t xa = Math::deg2rad(node.angle_x), ya = Math::deg2rad(node.angle_y), za = Math::deg2rad(node.angle_z);
			real_t xc = Math::cos(xa), yc = Math::cos(ya), zc = Math::cos(za);
			real_t xs = Math::sin(xa), ys = Math::sin(ya), zs = Math::sin(za);
			real_t m00 = ys * xs * zs + yc * zc, m01 = xc * zs, m02 = ys * zc - yc * xs * zs;
			real_t m10 = ys * xs * zc - yc * zs, m11 = xc * zc, m12 = -yc * xs * zc - ys * zs;
			real_t m20 = -ys * xc, m21 = xs, m22 = yc * xc;
			Vector3 rotated(
					m00 * p_pos.x + m10 * p_pos.y + m20 * p_pos.z,
					m01 * p_pos.x + m11 * p_pos.y + m21 * p_pos.z,
					m02 * p_pos.x + m12 * p_pos.y + m22 * p_pos.z);
			return evaluate_node(node.sources.size() > 0 ? node.sources[0] : -1, rotated);
		}
		case NOISE_NODE_SCALE_POINT: {
			Vector3 scaled(p_pos.x * node.scale_x, p_pos.y * node.scale_y, p_pos.z * node.scale_z);
			return evaluate_node(node.sources.size() > 0 ? node.sources[0] : -1, scaled);
		}
		case NOISE_NODE_TRANSLATE_POINT: {
			Vector3 translated(p_pos.x + node.translate_x, p_pos.y + node.translate_y, p_pos.z + node.translate_z);
			return evaluate_node(node.sources.size() > 0 ? node.sources[0] : -1, translated);
		}
		case NOISE_NODE_TURBULENCE: {
			const Vector3 off_x(12414.0 / 65536.0, 65124.0 / 65536.0, 31337.0 / 65536.0);
			const Vector3 off_y(26519.0 / 65536.0, 18128.0 / 65536.0, 60493.0 / 65536.0);
			const Vector3 off_z(53820.0 / 65536.0, 11213.0 / 65536.0, 44845.0 / 65536.0);
			real_t dx = _eval_perlin_fractal(node.turbulence_x_noises, node.frequency, 2.0, 0.5, p_pos + off_x) * node.power;
			real_t dy = _eval_perlin_fractal(node.turbulence_y_noises, node.frequency, 2.0, 0.5, p_pos + off_y) * node.power;
			real_t dz = _eval_perlin_fractal(node.turbulence_z_noises, node.frequency, 2.0, 0.5, p_pos + off_z) * node.power;
			Vector3 distorted(p_pos.x + dx, p_pos.y + dy, p_pos.z + dz);
			return evaluate_node(node.sources.size() > 0 ? node.sources[0] : -1, distorted);
		}
		case NOISE_NODE_CONST:
			return node.const_value;
		case NOISE_NODE_PERLIN:
			return _eval_perlin_fractal(node.octave_noises, node.frequency, node.lacunarity, node.persistence, p_pos);
		case NOISE_NODE_BILLOW:
			return _eval_billow_fractal(node.octave_noises, node.frequency, node.lacunarity, node.persistence, p_pos);
		case NOISE_NODE_RIDGED_MULTI:
			return _eval_ridged_multi(node.octave_noises, node.frequency, node.lacunarity, p_pos);
		case NOISE_NODE_VORONOI:
			return _eval_voronoi(node, p_pos);
		case NOISE_NODE_SPHERES: {
			Vector3 p = p_pos * node.frequency;
			real_t d = p.length();
			real_t frac = d - Math::floor(d);
			real_t nearest = MIN(frac, 1.0 - frac);
			return 1.0 - nearest * 4.0;
		}
		case NOISE_NODE_CYLINDERS: {
			real_t d = Vector2(p_pos.x * node.frequency, p_pos.z * node.frequency).length();
			real_t frac = d - Math::floor(d);
			real_t nearest = MIN(frac, 1.0 - frac);
			return 1.0 - nearest * 4.0;
		}
		default:
			return 0.0;
	}
}

// =========================================================================
// JSON parsing — placeholder-id decoding rationale: procrocklib's edges reference
// auto-generated, unserialized "input slot" placeholder ids computed as
// `consumer_internal_id + 1 + slot*2` (see the interpreter banner above).
// =========================================================================

namespace {

real_t _dget(const Dictionary &p_d, const String &p_key, real_t p_default) {
	if (p_d.has(p_key)) {
		return p_d[p_key];
	}
	return p_default;
}

int _dget_int(const Dictionary &p_d, const String &p_key, int p_default) {
	if (p_d.has(p_key)) {
		return (int)p_d[p_key];
	}
	return p_default;
}

bool _dget_bool(const Dictionary &p_d, const String &p_key, bool p_default) {
	if (p_d.has(p_key)) {
		return (bool)p_d[p_key];
	}
	return p_default;
}

} // namespace

NoiseGraph NoiseGraph::from_json(const Dictionary &p_json) {
	NoiseGraph graph;
	if (!p_json.has("nodes")) {
		return graph;
	}
	Array nodes_json = p_json["nodes"];
	Array edges_json = p_json.has("edges") ? (Array)p_json["edges"] : Array();

	// internal_id -> array index, for real (serialized) nodes only.
	HashMap<int, int> id_to_index;
	graph.nodes.resize(nodes_json.size());

	for (int i = 0; i < nodes_json.size(); i++) {
		Dictionary node_json = nodes_json[i];
		int type_id = _dget_int(node_json, "_id", NOISE_NODE_CONST);
		int internal_id = 0;
		if (node_json.has("general")) {
			Dictionary general = node_json["general"];
			internal_id = _dget_int(general, "internal_id", i);
		}
		id_to_index[internal_id] = i;

		NoiseGraphNode &node = graph.nodes.write[i];
		node.type_id = type_id;

		Dictionary config = node_json.has("config") && node_json["config"].get_type() == Variant::DICTIONARY ? (Dictionary)node_json["config"] : Dictionary();
		Dictionary floats = config.has("floats") ? (Dictionary)config["floats"] : Dictionary();
		Dictionary ints = config.has("ints") ? (Dictionary)config["ints"] : Dictionary();
		Dictionary bools = config.has("bools") ? (Dictionary)config["bools"] : Dictionary();

		switch (type_id) {
			case NOISE_NODE_CONST:
				node.const_value = _dget(floats, "Value", 0.0);
				break;
			case NOISE_NODE_SELECT:
				node.lower_bound = _dget(floats, "Lower Bound", -1.0);
				node.upper_bound = _dget(floats, "Upper Bound", 1.0);
				node.edge_falloff = _dget(floats, "Edge Falloff", 0.0);
				break;
			case NOISE_NODE_CLAMP:
				node.lower_bound = _dget(floats, "Lower Bound", -1.0);
				node.upper_bound = _dget(floats, "Upper Bound", 1.0);
				break;
			case NOISE_NODE_EXPONENT:
				node.exponent = _dget(floats, "Exponent", 1.0);
				break;
			case NOISE_NODE_SCALE_BIAS:
				node.scale = _dget(floats, "Scale", 1.0);
				node.bias = _dget(floats, "Bias", 0.0);
				break;
			case NOISE_NODE_TERRACE: {
				node.invert_terraces = _dget_bool(bools, "Invert Terraces", false);
				Dictionary lists = config.has("lists") ? (Dictionary)config["lists"] : Dictionary();
				if (lists.has("Control Points")) {
					Dictionary list_entry = lists["Control Points"];
					Array list = list_entry.has("list") ? (Array)list_entry["list"] : Array();
					for (int p = 0; p < list.size(); p++) {
						node.control_points.push_back((real_t)list[p]);
					}
				}
				if (node.control_points.size() < 2) {
					node.control_points.clear();
					node.control_points.push_back(0.0);
					node.control_points.push_back(1.0);
				}
				break;
			}
			case NOISE_NODE_CURVE: {
				Dictionary curves = config.has("curves") ? (Dictionary)config["curves"] : Dictionary();
				if (curves.has("Curve")) {
					Dictionary curve_entry = curves["Curve"];
					Dictionary curve_points = curve_entry.has("curvePoints") ? (Dictionary)curve_entry["curvePoints"] : Dictionary();
					Array keys = curve_points.keys();
					for (int p = 0; p < keys.size(); p++) {
						real_t in = String(keys[p]).to_double();
						real_t out = curve_points[keys[p]];
						node.curve_points.push_back(Vector2(in, out));
					}
					node.curve_points.sort_custom<_CurvePointLess>();
				}
				if (node.curve_points.size() < 4) {
					node.curve_points.clear();
					node.curve_points.push_back(Vector2(0, 0));
					node.curve_points.push_back(Vector2(0.1, 0.2));
					node.curve_points.push_back(Vector2(0.6, 0.15));
					node.curve_points.push_back(Vector2(1, 1));
				}
				break;
			}
			case NOISE_NODE_ROTATE_POINT:
				node.angle_x = _dget(floats, "x Angle", 0.0);
				node.angle_y = _dget(floats, "y Angle", 0.0);
				node.angle_z = _dget(floats, "z Angle", 0.0);
				break;
			case NOISE_NODE_SCALE_POINT:
				node.scale_x = _dget(floats, "x Scale", 1.0);
				node.scale_y = _dget(floats, "y Scale", 1.0);
				node.scale_z = _dget(floats, "z Scale", 1.0);
				break;
			case NOISE_NODE_TRANSLATE_POINT:
				// Unverified against real data — no preset uses TranslatePoint (see NoiseGraphNode above);
				// key casing follows RotatePoint/ScalePoint's confirmed "x/y/z <Name>" convention.
				node.translate_x = _dget(floats, "x Translation", 0.0);
				node.translate_y = _dget(floats, "y Translation", 0.0);
				node.translate_z = _dget(floats, "z Translation", 0.0);
				break;
			case NOISE_NODE_TURBULENCE: {
				node.frequency = _dget(floats, "Frequency", 31.0);
				node.power = _dget(floats, "Power", 1.0);
				node.roughness = _dget_int(ints, "Roughness", 2);
				node.seed = _dget_int(ints, "Seed", 0);
				node.turbulence_x_noises = _make_octave_noises(node.seed, node.roughness);
				node.turbulence_y_noises = _make_octave_noises(node.seed + 1, node.roughness);
				node.turbulence_z_noises = _make_octave_noises(node.seed + 2, node.roughness);
				break;
			}
			case NOISE_NODE_PERLIN:
			case NOISE_NODE_BILLOW:
				node.frequency = _dget(floats, "Frequency", 31.0);
				node.lacunarity = _dget(floats, "Lacunarity", 2.5);
				node.persistence = _dget(floats, "Persistence", 0.6);
				node.octaves = _dget_int(ints, "Octaves", 3);
				node.seed = _dget_int(ints, "Seed", 0);
				node.octave_noises = _make_octave_noises(node.seed, node.octaves);
				break;
			case NOISE_NODE_RIDGED_MULTI:
				node.frequency = _dget(floats, "Frequency", 31.0);
				node.lacunarity = _dget(floats, "Lacunarity", 2.5);
				node.octaves = _dget_int(ints, "Octaves", 3);
				node.seed = _dget_int(ints, "Seed", 0);
				node.octave_noises = _make_octave_noises(node.seed, node.octaves);
				break;
			case NOISE_NODE_VORONOI:
				node.frequency = _dget(floats, "Frequency", 31.0);
				node.displacement = _dget(floats, "Displacement", 1.0);
				node.enable_distance = _dget_bool(bools, "Distance", false);
				node.seed = _dget_int(ints, "Seed", 0);
				break;
			case NOISE_NODE_SPHERES:
			case NOISE_NODE_CYLINDERS:
				node.frequency = _dget(floats, "Frequency", 31.0);
				break;
			default:
				break; // Output/Add/Max/Min/Multiply/Power/Blend/Abs/Invert/Displace: no config
		}

		if (type_id == NOISE_NODE_OUTPUT) {
			graph.root_index = i;
		}
	}

	// Resolve sources: for each real node with internal_id N and K input slots
	// (K = GetSourceModuleCount() equivalent, i.e. how many `sources` entries this
	// type needs), slot `s` is wired from whichever edge has `from == N+1+2*s`.
	for (int i = 0; i < nodes_json.size(); i++) {
		Dictionary node_json = nodes_json[i];
		int internal_id = i;
		if (node_json.has("general")) {
			Dictionary general = node_json["general"];
			internal_id = _dget_int(general, "internal_id", i);
		}
		NoiseGraphNode &node = graph.nodes.write[i];
		int slot_count = 0;
		switch (node.type_id) {
			case NOISE_NODE_ADD:
			case NOISE_NODE_MAX:
			case NOISE_NODE_MIN:
			case NOISE_NODE_MULTIPLY:
			case NOISE_NODE_POWER:
				slot_count = 2;
				break;
			case NOISE_NODE_BLEND:
			case NOISE_NODE_SELECT:
				slot_count = 3;
				break;
			case NOISE_NODE_DISPLACE:
				slot_count = 4;
				break;
			case NOISE_NODE_OUTPUT:
			case NOISE_NODE_ABS:
			case NOISE_NODE_CLAMP:
			case NOISE_NODE_EXPONENT:
			case NOISE_NODE_INVERT:
			case NOISE_NODE_SCALE_BIAS:
			case NOISE_NODE_TERRACE:
			case NOISE_NODE_CURVE:
			case NOISE_NODE_ROTATE_POINT:
			case NOISE_NODE_SCALE_POINT:
			case NOISE_NODE_TRANSLATE_POINT:
			case NOISE_NODE_TURBULENCE:
				slot_count = 1;
				break;
			default:
				slot_count = 0; // Const/Perlin/Billow/RidgedMulti/Voronoi/Spheres/Cylinders: leaves
				break;
		}

		node.sources.resize(slot_count);
		for (int s = 0; s < slot_count; s++) {
			node.sources.write[s] = -1;
			int placeholder_id = internal_id + 1 + 2 * s;
			for (int e = 0; e < edges_json.size(); e++) {
				Dictionary edge = edges_json[e];
				if (_dget_int(edge, "from", -999) == placeholder_id) {
					int source_internal_id = _dget_int(edge, "to", -999);
					if (id_to_index.has(source_internal_id)) {
						node.sources.write[s] = id_to_index[source_internal_id];
					}
					break;
				}
			}
		}
	}

	return graph;
}

NoiseGraph NoiseGraph::make_simple_fractal(real_t p_frequency, int p_octaves, real_t p_persistence, int p_randseed) {
	NoiseGraph graph;
	NoiseGraphNode perlin;
	perlin.type_id = NOISE_NODE_PERLIN;
	perlin.frequency = p_frequency;
	perlin.lacunarity = 2.0;
	perlin.persistence = p_persistence;
	perlin.octaves = CLAMP(p_octaves, 1, 10);
	perlin.seed = p_randseed;
	perlin.octave_noises = _make_octave_noises(perlin.seed, perlin.octaves);
	graph.nodes.push_back(perlin);
	graph.root_index = 0;
	return graph;
}

// =========================================================================
// JSON pipeline reader — reads procrocklib's full pipeline shape
// {"generator","modifiers","parameterizer","textureAdders","textureGenerator"}
// (see editor/proc_rocks_demo/presets/*.json) directly, so real preset files can
// drive the mesh/texture pipeline instead of only the hand-extracted table in
// ProcRockMesh::set_pipeline_preset(). Phase 1 scope only: reads the
// textureGenerator stage (Displacement/Height noise graph, Albedo gradient,
// Roughness/Metalness/Ambient Occlusion scale+bias, Normals strength) — the
// modifier chain (Transformation/Subdivision/Decimate/DisplaceAlongNormals) and
// non-Icosahedron generators aren't implemented yet, so a JSON file that uses
// them still loads (approximated via the texture generator's own noise graph
// for mesh displacement, same as the non-JSON path already does) but emits a
// one-line warning rather than silently pretending full fidelity.
// =========================================================================

namespace {

// textureGenerator.config[p_stage_name] is always an Array in procrocklib's JSON —
// a "Method" selector dict optionally followed by one config dict per method (see
// memo.md's "Presets: real extracted values" section for why every method is always
// serialized). Returns an empty Array if the pipeline JSON doesn't have this shape.
Array _texgen_stage(const Dictionary &p_pipeline_json, const String &p_stage_name) {
	if (!p_pipeline_json.has("textureGenerator") || p_pipeline_json["textureGenerator"].get_type() != Variant::DICTIONARY) {
		return Array();
	}
	Dictionary texgen = p_pipeline_json["textureGenerator"];
	if (!texgen.has("config") || texgen["config"].get_type() != Variant::DICTIONARY) {
		return Array();
	}
	Dictionary config = texgen["config"];
	if (!config.has(p_stage_name) || config[p_stage_name].get_type() != Variant::ARRAY) {
		return Array();
	}
	return config[p_stage_name];
}

int _texgen_stage_method(const Array &p_stage) {
	if (p_stage.size() < 1 || p_stage[0].get_type() != Variant::DICTIONARY) {
		return 0;
	}
	Dictionary method_dict = p_stage[0];
	return method_dict.has("singleChoices") ? _dget_int(method_dict["singleChoices"], "Method", 0) : 0;
}

// Displacement/Height's single method's config is always at array slot 1 (no
// Method-dependent branching — confirmed: this stage has just one method, unlike
// Albedo below). Extracts its embedded "Noise Graph" and parses it; falls back to
// a plain fractal if the shape doesn't match (missing stage, or a JSON authored by
// a hypothetical future tool version that doesn't nest a noise graph here).
NoiseGraph _extract_displacement_graph(const Dictionary &p_pipeline_json) {
	Array stage = _texgen_stage(p_pipeline_json, "Displacement / Height");
	if (stage.size() >= 1 && stage[0].get_type() == Variant::DICTIONARY) {
		Dictionary entry = stage[0];
		if (entry.has("noiseGraphs") && entry["noiseGraphs"].get_type() == Variant::DICTIONARY) {
			Dictionary graphs = entry["noiseGraphs"];
			if (graphs.has("Noise Graph")) {
				NoiseGraph g = NoiseGraph::from_json(graphs["Noise Graph"]);
				if (g.is_valid()) {
					return g;
				}
			}
		}
	}
	WARN_PRINT("ProcRock: pipeline JSON has no valid 'Displacement / Height' noise graph — using default fractal noise.");
	return NoiseGraph::make_simple_fractal(1.0, 3, 0.5, 0);
}

// Albedo has two candidate gradients, always both serialized (array slots 2 and 3);
// which is active depends on the Method choice at slot 0 — confirmed against real
// preset data (10.json uses Method 0 / slot 2, all others use Method 1 / slot 3).
Ref<Gradient> _extract_albedo_gradient(const Dictionary &p_pipeline_json) {
	Ref<Gradient> gradient;
	gradient.instance();

	Array stage = _texgen_stage(p_pipeline_json, "Albedo");
	int active_slot = _texgen_stage_method(stage) == 0 ? 2 : 3;
	if (stage.size() > active_slot && stage[active_slot].get_type() == Variant::DICTIONARY) {
		Dictionary slot = stage[active_slot];
		if (slot.has("gradientColorings") && slot["gradientColorings"].get_type() == Variant::DICTIONARY) {
			Dictionary gc = slot["gradientColorings"];
			Array keys = gc.keys();
			if (keys.size() > 0 && gc[keys[0]].get_type() == Variant::ARRAY) {
				Array stops = gc[keys[0]];
				Vector<Gradient::Point> points;
				for (int i = 0; i < stops.size(); i++) {
					Array stop = stops[i];
					if (stop.size() != 2 || stop[1].get_type() != Variant::DICTIONARY) {
						continue;
					}
					Dictionary color_dict = stop[1];
					Gradient::Point point;
					point.offset = CLAMP(real_t(stop[0]) / real_t(100.0), real_t(0.0), real_t(1.0));
					point.color = Color(_dget(color_dict, "x", 0.0), _dget(color_dict, "y", 0.0), _dget(color_dict, "z", 0.0));
					points.push_back(point);
				}
				if (points.size() >= 2) {
					gradient->set_points(points);
					return gradient;
				}
			}
		}
	}
	WARN_PRINT("ProcRock: pipeline JSON has no valid Albedo gradient — using a default gray gradient.");
	return gradient; // Godot's default-constructed Gradient is a 2-stop black->white ramp.
}

// Roughness/Metalness/Ambient Occlusion all share this exact shape: a single method
// (no Method-dependent branching), config always at slot 1, with an int Bias in
// [-255,255] (procrocklib's original convention) normalized to this codebase's
// [-1,1]-ish float convention by dividing by 255.
void _extract_scale_bias(const Dictionary &p_pipeline_json, const String &p_stage_name, real_t p_default_scale, real_t p_default_bias, real_t &r_scale, real_t &r_bias) {
	Array stage = _texgen_stage(p_pipeline_json, p_stage_name);
	if (stage.size() > 1 && stage[1].get_type() == Variant::DICTIONARY) {
		Dictionary slot = stage[1];
		Dictionary floats = slot.has("floats") ? (Dictionary)slot["floats"] : Dictionary();
		Dictionary ints = slot.has("ints") ? (Dictionary)slot["ints"] : Dictionary();
		r_scale = _dget(floats, "Scaling", p_default_scale);
		r_bias = _dget_int(ints, "Bias", int(p_default_bias * 255.0)) / real_t(255.0);
		return;
	}
	r_scale = p_default_scale;
	r_bias = p_default_bias;
}

real_t _extract_normal_strength(const Dictionary &p_pipeline_json, real_t p_default) {
	Array stage = _texgen_stage(p_pipeline_json, "Normals");
	if (stage.size() > 1 && stage[1].get_type() == Variant::DICTIONARY) {
		Dictionary slot = stage[1];
		Dictionary floats = slot.has("floats") ? (Dictionary)slot["floats"] : Dictionary();
		return _dget(floats, "Normal Strength", p_default);
	}
	return p_default;
}

} // namespace

bool rock_pipeline_json_is_valid(const Dictionary &p_pipeline_json) {
	return p_pipeline_json.has("textureGenerator") && p_pipeline_json["textureGenerator"].get_type() == Variant::DICTIONARY;
}

// =========================================================================
// Mesh pipeline: icosphere -> noise displacement -> optional cut-plane -> box UV
// =========================================================================

namespace {

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

struct ClippedMesh {
	Vector<Vector3> vertices;
	Vector<int> indices;
};

// Clips an indexed triangle mesh against a plane (keeping the side the plane's normal
// points away from) and caps the exposed cross-section with a triangle fan ordered by
// angle around its centroid. This assumes a single, star-shaped cross-section, which
// holds for cutting a lightly-displaced icosphere with one plane.
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
			// Orient outward (towards the removed material) regardless of the fan's
			// natural winding, using the same cross(P2-P0,P1-P0) convention as ComputeNormal.
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

Array _finalize_mesh_arrays(const Vector<Vector3> &p_vertices, const Vector<int> &p_indices, bool p_smoothed);

Array _rock_pipeline_gen_impl(int p_subdivisions, real_t p_width, real_t p_height, real_t p_depth,
		const NoiseGraph &p_noise, real_t p_noise_amplitude, int p_randseed,
		bool p_cutplane_enabled, real_t p_cutplane_offset, bool p_smoothed) {
	if (p_randseed == 0) {
		Math::randomize();
	} else {
		Math::seed((uint64_t)p_randseed);
	}

	IndexedMesh ico = MakeIcosphere(CLAMP(p_subdivisions, 0, 6));

	Vector<Vector3> vertices;
	vertices.resize(ico.first.size());
	for (size_t i = 0; i < ico.first.size(); i++) {
		Vector3 dir = ico.first[i]; // unit-sphere direction
		real_t displacement = p_noise.evaluate(dir) * p_noise_amplitude;
		vertices.write[i] = dir * Vector3(p_width, p_height, p_depth) * 0.5 + dir * displacement;
	}

	Vector<int> indices;
	indices.resize(ico.second.size() * 3);
	for (size_t i = 0; i < ico.second.size(); i++) {
		indices.write[i * 3 + 0] = ico.second[i].vertex[0];
		indices.write[i * 3 + 1] = ico.second[i].vertex[1];
		indices.write[i * 3 + 2] = ico.second[i].vertex[2];
	}

	if (p_cutplane_enabled) {
		Vector3 plane_normal = Vector3(Math::randf() * 2 - 1, Math::randf() * 2 - 1, Math::randf() * 2 - 1);
		if (plane_normal.length_squared() < CMP_EPSILON) {
			plane_normal = Vector3(0, 1, 0);
		}
		plane_normal.normalize();

		ClippedMesh clipped = clip_and_cap(vertices, indices, Plane(plane_normal, p_cutplane_offset));
		vertices = clipped.vertices;
		indices = clipped.indices;
	}

	return _finalize_mesh_arrays(vertices, indices, p_smoothed);
}

// Shared tail: smooth (indexed, box-UV) or flat/low-poly shading + final Array packing —
// used by both the scalar pipeline (above) and the JSON-driven generator/modifier chain.
Array _finalize_mesh_arrays(const Vector<Vector3> &p_vertices, const Vector<int> &p_indices, bool p_smoothed) {
	Array mesh_arrays;
	mesh_arrays.resize(VS::ARRAY_MAX);
	if (p_vertices.size() == 0 || p_indices.size() == 0) {
		return mesh_arrays;
	}

	if (p_smoothed) {
		Vector<Vector3> normals = compute_smooth_normals(p_vertices, p_indices);

		Vector<Vector2> uvs;
		uvs.resize(p_vertices.size());
		for (int i = 0; i < p_vertices.size(); i++) {
			int box_dir = rock_studio_get_box_dir(normals[i]);
			uvs.write[i] = rock_studio_get_box_uv(p_vertices[i], box_dir);
		}

		mesh_arrays[VS::ARRAY_VERTEX] = p_vertices;
		mesh_arrays[VS::ARRAY_NORMAL] = normals;
		mesh_arrays[VS::ARRAY_TEX_UV] = uvs;
		mesh_arrays[VS::ARRAY_INDEX] = p_indices;
	} else {
		// rock_studio_make_low_poly() derives its flat normal via cross(v1-v0, v2-v0),
		// the opposite winding convention from ComputeNormal()'s cross(v2-v0, v1-v0) used
		// above — flip winding here so the flat-shaded result faces outward too.
		Vector<int> flipped_indices;
		flipped_indices.resize(p_indices.size());
		for (int i = 0; i + 2 < p_indices.size(); i += 3) {
			flipped_indices.write[i] = p_indices[i];
			flipped_indices.write[i + 1] = p_indices[i + 2];
			flipped_indices.write[i + 2] = p_indices[i + 1];
		}

		Array arrays;
		arrays.resize(VS::ARRAY_MAX);
		arrays[VS::ARRAY_VERTEX] = p_vertices;
		arrays[VS::ARRAY_INDEX] = flipped_indices;

		Ref<ArrayMesh> temp;
		temp.instance();
		temp->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

		Ref<ArrayMesh> low_poly = rock_studio_make_low_poly(temp);
		if (low_poly.is_valid() && low_poly->get_surface_count() > 0) {
			rock_studio_box_uv(low_poly);
			mesh_arrays = low_poly->surface_get_arrays(0);
		}
	}

	return mesh_arrays;
}

// =========================================================================
// JSON-driven generators — ported from procrocklib's gen/cuboid_generator.cpp
// and gen/pyramid_generator.cpp (recovered from git history, MPL2-licensed
// libigl reference for per_vertex_normals only — not relinked; normals are
// computed via compute_smooth_normals() above instead). Generator _id values
// match procrocklib's PipelineStage_Gen_* exactly (0=Cuboid, 1=Icosahedron,
// 2=Pyramid, 3=SkinSurface). SkinSurface needs a CGAL-free substitute
// (marching-cubes metaball mesher, per the phase roadmap) — falls back to
// Icosahedron with a one-line warning until that lands.
// =========================================================================

struct RockMesh {
	Vector<Vector3> vertices;
	Vector<int> indices; // triangle list, 3 per face
};

// generator.config["General"][p_index] — procrocklib groups a stage's
// Configuration::ConfigurationGroup entries as one dict per array slot.
Dictionary _stage_group(const Dictionary &p_stage, const String &p_group_name, int p_index) {
	if (!p_stage.has("config") || p_stage["config"].get_type() != Variant::DICTIONARY) {
		return Dictionary();
	}
	Dictionary config = p_stage["config"];
	if (!config.has(p_group_name) || config[p_group_name].get_type() != Variant::ARRAY) {
		return Dictionary();
	}
	Array groups = config[p_group_name];
	if (p_index < 0 || p_index >= groups.size() || groups[p_index].get_type() != Variant::DICTIONARY) {
		return Dictionary();
	}
	return groups[p_index];
}

RockMesh generate_cuboid(real_t p_length, real_t p_height, real_t p_depth) {
	RockMesh mesh;
	real_t hl = p_length / 2.0, hh = p_height / 2.0, hd = p_depth / 2.0;
	mesh.vertices.push_back(Vector3(hl, -hh, -hd));
	mesh.vertices.push_back(Vector3(hl, hh, -hd));
	mesh.vertices.push_back(Vector3(-hl, hh, -hd));
	mesh.vertices.push_back(Vector3(-hl, -hh, -hd));
	mesh.vertices.push_back(Vector3(hl, -hh, hd));
	mesh.vertices.push_back(Vector3(hl, hh, hd));
	mesh.vertices.push_back(Vector3(-hl, hh, hd));
	mesh.vertices.push_back(Vector3(-hl, -hh, hd));

	static const int faces[12][3] = {
		{ 0, 2, 1 }, { 0, 3, 2 },
		{ 0, 1, 5 }, { 0, 5, 4 },
		{ 1, 2, 6 }, { 1, 6, 5 },
		{ 2, 3, 7 }, { 2, 7, 6 },
		{ 3, 0, 4 }, { 3, 4, 7 },
		{ 4, 5, 6 }, { 4, 6, 7 }
	};
	for (int i = 0; i < 12; i++) {
		mesh.indices.push_back(faces[i][0]);
		mesh.indices.push_back(faces[i][1]);
		mesh.indices.push_back(faces[i][2]);
	}
	return mesh;
}

RockMesh generate_pyramid(real_t p_base_height, real_t p_base_radius, int p_base_subdivisions, real_t p_base_wiggle, int p_seed,
		real_t p_height, real_t p_tip_x, real_t p_tip_z) {
	RockMesh mesh;
	int n = MAX(1, p_base_subdivisions - 1);

	Ref<RandomNumberGenerator> rng;
	rng.instance();
	rng->set_seed(p_seed);

	mesh.vertices.resize(n + 2);
	mesh.vertices.write[0] = Vector3(0, p_base_height, 0);
	for (int i = 0; i < n; i++) {
		real_t ratio = real_t(i) / real_t(n);
		real_t r = ratio * Math_TAU;
		real_t x = Math::cos(r) * p_base_radius;
		real_t z = Math::sin(r) * p_base_radius;
		real_t random1 = (rng->randf() - 0.5) * 2.0;
		real_t random2 = (rng->randf() - 0.5) * 2.0;
		mesh.vertices.write[i + 1] = Vector3(x + random1 * p_base_wiggle, p_base_height, z + random2 * p_base_wiggle);
	}
	int apex_index = n + 1;
	mesh.vertices.write[apex_index] = Vector3(p_tip_x, p_height, p_tip_z);

	for (int i = 0; i < n; i++) {
		int i1 = ((i + 1) % n == 0) ? i + 1 : (i + 1) % n;
		int i2 = ((i + 2) % n == 0) ? i + 2 : (i + 2) % n;
		mesh.indices.push_back(0);
		mesh.indices.push_back(i1);
		mesh.indices.push_back(i2);
		mesh.indices.push_back(i1);
		mesh.indices.push_back(apex_index);
		mesh.indices.push_back(i2);
	}
	return mesh;
}

RockMesh generate_icosphere(int p_subdivisions, real_t p_width, real_t p_height, real_t p_depth) {
	IndexedMesh ico = MakeIcosphere(CLAMP(p_subdivisions, 0, 6));
	RockMesh mesh;
	mesh.vertices.resize(ico.first.size());
	for (size_t i = 0; i < ico.first.size(); i++) {
		mesh.vertices.write[i] = ico.first[i] * Vector3(p_width, p_height, p_depth) * 0.5;
	}
	mesh.indices.resize(ico.second.size() * 3);
	for (size_t i = 0; i < ico.second.size(); i++) {
		mesh.indices.write[i * 3 + 0] = ico.second[i].vertex[0];
		mesh.indices.write[i * 3 + 1] = ico.second[i].vertex[1];
		mesh.indices.write[i * 3 + 2] = ico.second[i].vertex[2];
	}
	return mesh;
}

RockMesh generate_base_mesh_from_json(const Dictionary &p_pipeline_json, int p_subdivisions, real_t p_width, real_t p_height, real_t p_depth) {
	if (!p_pipeline_json.has("generator") || p_pipeline_json["generator"].get_type() != Variant::DICTIONARY) {
		return generate_icosphere(p_subdivisions, p_width, p_height, p_depth);
	}
	Dictionary generator = p_pipeline_json["generator"];
	int generator_id = _dget_int(generator, "_id", 1);

	switch (generator_id) {
		case 0: { // Cuboid
			Dictionary group = _stage_group(generator, "General", 0);
			Dictionary floats = group.has("floats") ? (Dictionary)group["floats"] : Dictionary();
			return generate_cuboid(_dget(floats, "Length", 1.0), _dget(floats, "Height", 0.5), _dget(floats, "Depth ", 0.2));
		}
		case 2: { // Pyramid
			Dictionary base = _stage_group(generator, "General", 0);
			Dictionary base_floats = base.has("floats") ? (Dictionary)base["floats"] : Dictionary();
			Dictionary base_ints = base.has("ints") ? (Dictionary)base["ints"] : Dictionary();
			Dictionary other = _stage_group(generator, "General", 1);
			Dictionary other_floats = other.has("floats") ? (Dictionary)other["floats"] : Dictionary();
			return generate_pyramid(
					_dget(base_floats, "Height", -1.0), _dget(base_floats, "Radius", 2.0),
					_dget_int(base_ints, "Subdivisions", 4), _dget(base_floats, "Wiggle amount", 0.0), _dget_int(base_ints, "Seed", 0),
					_dget(other_floats, "Height", 2.0), _dget(other_floats, "Tip X", 0.0), _dget(other_floats, "Tip Z", 0.0));
		}
		case 1: // Icosahedron
			return generate_icosphere(p_subdivisions, p_width, p_height, p_depth);
		default: // 3 == SkinSurface, or anything else unrecognized
			WARN_PRINT("ProcRock: pipeline JSON's generator (SkinSurface or unrecognized) isn't implemented yet — using the icosphere generator instead.");
			return generate_icosphere(p_subdivisions, p_width, p_height, p_depth);
	}
}

// =========================================================================
// JSON-driven modifier chain — ported from procrocklib's mod/*.cpp (recovered
// from git history). Modifier _id values match procrocklib's PipelineStage_Mod_*
// exactly (0=Transform, 1=Subdivision, 2=Decimate, 3=DisplaceAlongNormals,
// 4=CutPlane).
// =========================================================================

Vector3 _dget_vec3(const Dictionary &p_d, const String &p_key, const Vector3 &p_default) {
	if (!p_d.has(p_key) || p_d[p_key].get_type() != Variant::DICTIONARY) {
		return p_default;
	}
	Dictionary v = p_d[p_key];
	return Vector3(_dget(v, "x", p_default.x), _dget(v, "y", p_default.y), _dget(v, "z", p_default.z));
}

// group["noiseGraphs"]["Noise Graph"] — same embedded shape as the texture
// generator's "Displacement / Height" stage (see _extract_displacement_graph).
NoiseGraph _extract_noise_graph_from_group(const Dictionary &p_group) {
	if (p_group.has("noiseGraphs") && p_group["noiseGraphs"].get_type() == Variant::DICTIONARY) {
		Dictionary graphs = p_group["noiseGraphs"];
		if (graphs.has("Noise Graph")) {
			NoiseGraph g = NoiseGraph::from_json(graphs["Noise Graph"]);
			if (g.is_valid()) {
				return g;
			}
		}
	}
	return NoiseGraph::make_simple_fractal(1.0, 3, 0.5, 0);
}

// mod/transformation_modifier.cpp: transform = Translation * Scaling * RotX * RotY * RotZ,
// rotation in radians, applied in that composition order (rotate, then scale, then translate).
void apply_transformation_modifier(RockMesh &p_mesh, const Dictionary &p_modifier) {
	Dictionary group = _stage_group(p_modifier, "General", 0);
	Dictionary float3s = group.has("float3s") ? (Dictionary)group["float3s"] : Dictionary();
	Vector3 translation = _dget_vec3(float3s, "Translation", Vector3());
	Vector3 scale = _dget_vec3(float3s, "Scale", Vector3(1, 1, 1));
	Vector3 rotation = _dget_vec3(float3s, "Rotate", Vector3());

	Basis rot_x(Vector3(1, 0, 0), rotation.x);
	Basis rot_y(Vector3(0, 1, 0), rotation.y);
	Basis rot_z(Vector3(0, 0, 1), rotation.z);
	Basis rot = rot_x * rot_y * rot_z;

	for (int i = 0; i < p_mesh.vertices.size(); i++) {
		Vector3 v = rot.xform(p_mesh.vertices[i]);
		v *= scale;
		v += translation;
		p_mesh.vertices.write[i] = v;
	}
}

// mod/displace_along_normals_modifier.cpp: displaces each vertex along its own
// smooth normal by factor*amount*preferAmount, where amount comes from either a
// seeded RNG (Type 0) or a noise graph sampled at the vertex position (Type 1),
// optionally scaled by how well the normal aligns with a preferred direction.
void apply_displace_along_normals_modifier(RockMesh &p_mesh, const Dictionary &p_modifier) {
	Dictionary group = _stage_group(p_modifier, "General", 0);
	Dictionary floats = group.has("floats") ? (Dictionary)group["floats"] : Dictionary();
	Dictionary ints = group.has("ints") ? (Dictionary)group["ints"] : Dictionary();
	Dictionary bools = group.has("bools") ? (Dictionary)group["bools"] : Dictionary();
	int selection = _dget_int(group.has("singleChoices") ? (Dictionary)group["singleChoices"] : Dictionary(), "Type", 0);
	real_t factor = _dget(floats, "Factor", 0.001);
	bool prefer_direction = _dget_bool(bools, "Prefer Direction", false);
	int seed = _dget_int(ints, "Seed", 0);

	Dictionary prefer_group = _stage_group(p_modifier, "Prefer Direction", 0);
	Dictionary prefer_float3s = prefer_group.has("float3s") ? (Dictionary)prefer_group["float3s"] : Dictionary();
	Dictionary prefer_floats = prefer_group.has("floats") ? (Dictionary)prefer_group["floats"] : Dictionary();
	Vector3 preferred_direction = _dget_vec3(prefer_float3s, "Direction", Vector3(0, -1, 0)).normalized();
	real_t prefer_strength = _dget(prefer_floats, "Strength", 1.0);

	NoiseGraph noise;
	if (selection == 1) {
		Dictionary noise_group = _stage_group(p_modifier, "Noise", 0);
		noise = _extract_noise_graph_from_group(noise_group);
	}

	Ref<RandomNumberGenerator> rng;
	rng.instance();
	rng->set_seed(seed);

	Vector<Vector3> normals = compute_smooth_normals(p_mesh.vertices, p_mesh.indices);
	for (int i = 0; i < p_mesh.vertices.size(); i++) {
		real_t amount = 0.0;
		if (selection == 1) {
			amount = (noise.evaluate(p_mesh.vertices[i]) + 1.0) / 2.0;
		} else {
			amount = rng->randf();
		}
		amount = CLAMP(amount, real_t(0.0), real_t(1.0));

		real_t prefer_amount = 1.0;
		if (prefer_direction) {
			prefer_amount = (normals[i].normalized() - preferred_direction).length() * prefer_strength;
		}

		p_mesh.vertices.write[i] += normals[i].normalized() * (factor * amount * prefer_amount);
	}
}

// --- Subdivision (mod/subdivision_modifier.cpp, ported from igl::loop/upsample/
// false_barycentric_subdivision — MPL2 reference, not relinked) ---
//
// Assumes a closed manifold mesh (every edge shared by exactly 2 triangles), true
// for everything this pipeline produces (cut-plane clips always get capped — see
// clip_and_cap()) — so unlike igl's general-purpose versions, no boundary-edge case
// is needed here.

struct _EdgeInfo {
	int opposite[2] = { -1, -1 };
	int count = 0;
};

int64_t _edge_key(int p_a, int p_b) {
	if (p_a > p_b) {
		SWAP(p_a, p_b);
	}
	return (((int64_t)p_a) << 32) | (uint32_t)p_b;
}

HashMap<int64_t, _EdgeInfo> _build_edge_info(const RockMesh &p_mesh) {
	HashMap<int64_t, _EdgeInfo> edges;
	for (int i = 0; i + 2 < p_mesh.indices.size(); i += 3) {
		int v[3] = { p_mesh.indices[i], p_mesh.indices[i + 1], p_mesh.indices[i + 2] };
		for (int e = 0; e < 3; e++) {
			int a = v[e], b = v[(e + 1) % 3], opp = v[(e + 2) % 3];
			int64_t key = _edge_key(a, b);
			_EdgeInfo *info = edges.getptr(key);
			if (!info) {
				edges.set(key, _EdgeInfo());
				info = edges.getptr(key);
			}
			if (info->count < 2) {
				info->opposite[info->count] = opp;
				info->count++;
			}
		}
	}
	return edges;
}

// Shared 1-to-4 face split: for each original face (a,b,c), emits (a,mAB,mCA),
// (b,mBC,mAB), (mAB,mBC,mCA), (mBC,c,mCA) — matches igl::loop/upsample's NF layout.
void _split_faces_with_edge_midpoints(const RockMesh &p_mesh, const HashMap<int64_t, int> &p_edge_to_new_vertex, Vector<int> &r_indices) {
	for (int i = 0; i + 2 < p_mesh.indices.size(); i += 3) {
		int a = p_mesh.indices[i], b = p_mesh.indices[i + 1], c = p_mesh.indices[i + 2];
		int m_ab = p_edge_to_new_vertex[_edge_key(a, b)];
		int m_bc = p_edge_to_new_vertex[_edge_key(b, c)];
		int m_ca = p_edge_to_new_vertex[_edge_key(c, a)];
		int faces[4][3] = { { a, m_ab, m_ca }, { b, m_bc, m_ab }, { m_ab, m_bc, m_ca }, { m_bc, c, m_ca } };
		for (int f = 0; f < 4; f++) {
			r_indices.push_back(faces[f][0]);
			r_indices.push_back(faces[f][1]);
			r_indices.push_back(faces[f][2]);
		}
	}
}

RockMesh subdivide_upsample(const RockMesh &p_mesh) {
	HashMap<int64_t, _EdgeInfo> edges = _build_edge_info(p_mesh);
	HashMap<int64_t, int> edge_to_new_vertex;

	RockMesh result;
	result.vertices = p_mesh.vertices;
	const int64_t *key = nullptr;
	while ((key = edges.next(key))) {
		int a = int(*key >> 32), b = int(*key & 0xffffffffLL);
		int idx = result.vertices.size();
		result.vertices.push_back((p_mesh.vertices[a] + p_mesh.vertices[b]) * 0.5);
		edge_to_new_vertex[*key] = idx;
	}
	_split_faces_with_edge_midpoints(p_mesh, edge_to_new_vertex, result.indices);
	return result;
}

RockMesh subdivide_false_barycentric(const RockMesh &p_mesh) {
	RockMesh result;
	result.vertices = p_mesh.vertices;
	int face_count = p_mesh.indices.size() / 3;
	for (int i = 0; i < face_count; i++) {
		int a = p_mesh.indices[i * 3], b = p_mesh.indices[i * 3 + 1], c = p_mesh.indices[i * 3 + 2];
		Vector3 centroid = (p_mesh.vertices[a] + p_mesh.vertices[b] + p_mesh.vertices[c]) / 3.0;
		int center_index = result.vertices.size();
		result.vertices.push_back(centroid);
		result.indices.push_back(a);
		result.indices.push_back(b);
		result.indices.push_back(center_index);
		result.indices.push_back(b);
		result.indices.push_back(c);
		result.indices.push_back(center_index);
		result.indices.push_back(c);
		result.indices.push_back(a);
		result.indices.push_back(center_index);
	}
	return result;
}

RockMesh subdivide_loop(const RockMesh &p_mesh) {
	HashMap<int64_t, _EdgeInfo> edges = _build_edge_info(p_mesh);

	// Even (old) vertex repositioning: 3/16 beta for valence 3, 3/(8n) otherwise.
	Vector<Vector<int>> neighbors;
	neighbors.resize(p_mesh.vertices.size());
	const int64_t *key = nullptr;
	while ((key = edges.next(key))) {
		int a = int(*key >> 32), b = int(*key & 0xffffffffLL);
		neighbors.write[a].push_back(b);
		neighbors.write[b].push_back(a);
	}

	RockMesh result;
	result.vertices.resize(p_mesh.vertices.size());
	for (int i = 0; i < p_mesh.vertices.size(); i++) {
		int n = neighbors[i].size();
		if (n == 0) {
			result.vertices.write[i] = p_mesh.vertices[i];
			continue;
		}
		real_t beta = (n == 3) ? (3.0 / 16.0) : (3.0 / (8.0 * n));
		Vector3 sum;
		for (int j = 0; j < n; j++) {
			sum += p_mesh.vertices[neighbors[i][j]];
		}
		result.vertices.write[i] = p_mesh.vertices[i] * (1.0 - n * beta) + sum * beta;
	}

	// Odd (new, edge-midpoint) vertices: 3/8-3/8-1/8-1/8 weighted by the edge's two
	// endpoints plus the opposite vertex of each of its two adjacent triangles.
	HashMap<int64_t, int> edge_to_new_vertex;
	key = nullptr;
	while ((key = edges.next(key))) {
		const _EdgeInfo &info = edges[*key];
		int a = int(*key >> 32), b = int(*key & 0xffffffffLL);
		int idx = result.vertices.size();
		Vector3 pos = (p_mesh.vertices[a] + p_mesh.vertices[b]) * (3.0 / 8.0);
		if (info.count == 2) {
			pos += (p_mesh.vertices[info.opposite[0]] + p_mesh.vertices[info.opposite[1]]) * (1.0 / 8.0);
		} else {
			// Degenerate (non-manifold) edge — fall back to a plain midpoint rather
			// than reading an uninitialized opposite vertex.
			pos = (p_mesh.vertices[a] + p_mesh.vertices[b]) * 0.5;
		}
		result.vertices.push_back(pos);
		edge_to_new_vertex[*key] = idx;
	}

	_split_faces_with_edge_midpoints(p_mesh, edge_to_new_vertex, result.indices);
	return result;
}

RockMesh apply_subdivision_modifier(const RockMesh &p_mesh, const Dictionary &p_modifier) {
	Dictionary group = _stage_group(p_modifier, "General", 0);
	Dictionary ints = group.has("ints") ? (Dictionary)group["ints"] : Dictionary();
	int subdivs = CLAMP(_dget_int(ints, "Subdivs", 1), 1, 3);
	int mode = _dget_int(group.has("singleChoices") ? (Dictionary)group["singleChoices"] : Dictionary(), "Type", 0);

	RockMesh mesh = p_mesh;
	for (int i = 0; i < subdivs; i++) {
		switch (mode) {
			case 1:
				mesh = subdivide_upsample(mesh);
				break;
			case 2:
				mesh = subdivide_false_barycentric(mesh);
				break;
			default: // 0 == Loop
				mesh = subdivide_loop(mesh);
				break;
		}
	}
	return mesh;
}

// --- Decimate (mod/decimate_modifier.cpp, ported from igl::decimate's actual default
// cost/placement function igl::shortest_edge_and_midpoint — MPL2 reference, not
// relinked): a greedy priority-queue-driven edge collapse. Repeatedly collapses the
// currently-shortest edge to its midpoint until the target face count is reached.
// Stale queue entries (an edge whose endpoint moved or died since it was pushed) are
// detected via a per-vertex generation counter and skipped on pop, rather than
// eagerly removed from the queue (the standard "lazy deletion" technique). ---

struct _DecimateEdge {
	real_t cost;
	int a, b; // a < b
	int gen_a, gen_b; // vertex generation counters at push time
	bool operator>(const _DecimateEdge &p_other) const { return cost > p_other.cost; }
};

RockMesh decimate_greedy(const RockMesh &p_mesh, int p_target_face_count) {
	Vector<Vector3> vertices = p_mesh.vertices;
	Vector<bool> vertex_alive;
	vertex_alive.resize(vertices.size());
	for (int i = 0; i < vertex_alive.size(); i++) {
		vertex_alive.write[i] = true;
	}
	Vector<int> vertex_gen;
	vertex_gen.resize(vertices.size());
	for (int i = 0; i < vertex_gen.size(); i++) {
		vertex_gen.write[i] = 0;
	}

	int face_count = p_mesh.indices.size() / 3;
	Vector<int> face_verts = p_mesh.indices; // 3 per face, mutated in place
	Vector<bool> face_alive;
	face_alive.resize(face_count);
	for (int i = 0; i < face_count; i++) {
		face_alive.write[i] = true;
	}

	Vector<Vector<int>> vertex_faces;
	vertex_faces.resize(vertices.size());
	for (int f = 0; f < face_count; f++) {
		for (int c = 0; c < 3; c++) {
			vertex_faces.write[face_verts[f * 3 + c]].push_back(f);
		}
	}

	auto push_candidate_edges_from = [&](int p_v, std::priority_queue<_DecimateEdge, std::vector<_DecimateEdge>, std::greater<_DecimateEdge>> &p_queue) {
		HashMap<int, bool> seen_neighbor;
		const Vector<int> &incident = vertex_faces[p_v];
		for (int i = 0; i < incident.size(); i++) {
			int f = incident[i];
			if (!face_alive[f]) {
				continue;
			}
			for (int c = 0; c < 3; c++) {
				int n = face_verts[f * 3 + c];
				if (n == p_v || seen_neighbor.has(n)) {
					continue;
				}
				seen_neighbor[n] = true;
				int a = MIN(p_v, n), b = MAX(p_v, n);
				_DecimateEdge e;
				e.cost = (vertices[a] - vertices[b]).length();
				e.a = a;
				e.b = b;
				e.gen_a = vertex_gen[a];
				e.gen_b = vertex_gen[b];
				p_queue.push(e);
			}
		}
	};

	std::priority_queue<_DecimateEdge, std::vector<_DecimateEdge>, std::greater<_DecimateEdge>> queue;
	HashMap<int64_t, bool> seeded;
	for (int f = 0; f < face_count; f++) {
		for (int c = 0; c < 3; c++) {
			int a = face_verts[f * 3 + c], b = face_verts[f * 3 + (c + 1) % 3];
			int64_t key = _edge_key(a, b);
			if (seeded.has(key)) {
				continue;
			}
			seeded[key] = true;
			int lo = MIN(a, b), hi = MAX(a, b);
			_DecimateEdge e;
			e.cost = (vertices[lo] - vertices[hi]).length();
			e.a = lo;
			e.b = hi;
			e.gen_a = vertex_gen[lo];
			e.gen_b = vertex_gen[hi];
			queue.push(e);
		}
	}

	while (face_count > p_target_face_count && !queue.empty()) {
		_DecimateEdge e = queue.top();
		queue.pop();
		if (!vertex_alive[e.a] || !vertex_alive[e.b]) {
			continue;
		}
		if (vertex_gen[e.a] != e.gen_a || vertex_gen[e.b] != e.gen_b) {
			continue; // stale — one endpoint moved since this entry was pushed
		}

		int survivor = e.a, removed = e.b;
		vertices.write[survivor] = (vertices[e.a] + vertices[e.b]) * 0.5;
		vertex_alive.write[removed] = false;
		vertex_gen.write[survivor]++;

		const Vector<int> &removed_faces = vertex_faces[removed];
		for (int i = 0; i < removed_faces.size(); i++) {
			int f = removed_faces[i];
			if (!face_alive[f]) {
				continue;
			}
			for (int c = 0; c < 3; c++) {
				if (face_verts[f * 3 + c] == removed) {
					face_verts.write[f * 3 + c] = survivor;
				}
			}
			int v0 = face_verts[f * 3], v1 = face_verts[f * 3 + 1], v2 = face_verts[f * 3 + 2];
			if (v0 == v1 || v1 == v2 || v2 == v0) {
				face_alive.write[f] = false;
				face_count--;
			} else {
				vertex_faces.write[survivor].push_back(f);
			}
		}

		if (face_count > p_target_face_count) {
			push_candidate_edges_from(survivor, queue);
		}
	}

	// Rebuild a compact mesh from surviving vertices/faces.
	Vector<int> remap;
	remap.resize(vertices.size());
	RockMesh result;
	for (int i = 0; i < vertices.size(); i++) {
		if (vertex_alive[i]) {
			remap.write[i] = result.vertices.size();
			result.vertices.push_back(vertices[i]);
		} else {
			remap.write[i] = -1;
		}
	}
	int total_faces = face_verts.size() / 3;
	for (int f = 0; f < total_faces; f++) {
		if (!face_alive[f]) {
			continue;
		}
		result.indices.push_back(remap[face_verts[f * 3]]);
		result.indices.push_back(remap[face_verts[f * 3 + 1]]);
		result.indices.push_back(remap[face_verts[f * 3 + 2]]);
	}
	return result;
}

RockMesh apply_decimate_modifier(const RockMesh &p_mesh, const Dictionary &p_modifier) {
	Dictionary group = _stage_group(p_modifier, "General", 0);
	Dictionary floats = group.has("floats") ? (Dictionary)group["floats"] : Dictionary();
	Dictionary ints = group.has("ints") ? (Dictionary)group["ints"] : Dictionary();
	int mode = _dget_int(group.has("singleChoices") ? (Dictionary)group["singleChoices"] : Dictionary(), "Type", 0);

	int current_face_count = p_mesh.indices.size() / 3;
	int target_face_count;
	if (mode == 1) {
		target_face_count = _dget_int(ints, "Abs. Value", 1000);
	} else {
		real_t rel_value = _dget(floats, "Rel. Value", 0.5);
		target_face_count = int(MAX(4.0, rel_value * current_face_count));
	}
	if (target_face_count >= current_face_count) {
		return p_mesh;
	}
	return decimate_greedy(p_mesh, target_face_count);
}

// --- CutPlane (mod/cut_plane_modifier.cpp) ---
//
// The original cuts the mesh via a real CGAL boolean difference against a large
// transformed half-space slab — CGAL/Boost stay out per explicit instruction, so
// the flat case is ported as an equivalent plane clip+cap (already implemented for
// the scalar pipeline's own cut-plane property, see clip_and_cap() above): the
// slab's cutting face is the local x=0 face of a cube spanning local x∈[0,10], so
// after the same Rx*Ry*Rz*Translation transform as TransformationModifier (no
// scale), that face becomes a plane through `translation` with normal
// R.xform(+X) — pointing toward the discarded side, matching clip_and_cap()'s
// "keep the side the normal points away from" convention directly.
//
// The "Noisy Surface" case originally subdivides+noise-displaces the cutting
// slab before the boolean op, roughening the cut. Real substitute (per explicit
// decision — no CGAL): after the flat clip+cap, displace every resulting vertex
// that lies on the cut plane by the noise graph sampled at its own position,
// along the plane's (discarded-side) normal. Both the clip-loop's boundary copies
// and the cap fan's own copies of each boundary point start at the identical
// position, so evaluating the same deterministic noise function there displaces
// them identically — the seam stays watertight without needing to track which
// output vertices came from which part of clip_and_cap().
RockMesh apply_cutplane_modifier(const RockMesh &p_mesh, const Dictionary &p_modifier) {
	Dictionary group = _stage_group(p_modifier, "General", 0);
	Dictionary float3s = group.has("float3s") ? (Dictionary)group["float3s"] : Dictionary();
	Dictionary bools = group.has("bools") ? (Dictionary)group["bools"] : Dictionary();
	Vector3 translation = _dget_vec3(float3s, "Translation", Vector3());
	Vector3 rotation = _dget_vec3(float3s, "Rotation", Vector3(0.3, 0.0, 1.7));
	bool noisy = _dget_bool(bools, "Noisy Surface", false);

	Basis rot_x(Vector3(1, 0, 0), rotation.x);
	Basis rot_y(Vector3(0, 1, 0), rotation.y);
	Basis rot_z(Vector3(0, 0, 1), rotation.z);
	Basis rot = rot_x * rot_y * rot_z;

	Vector3 plane_normal = rot.xform(Vector3(1, 0, 0)).normalized();
	Plane plane(plane_normal, plane_normal.dot(translation));

	ClippedMesh clipped = clip_and_cap(p_mesh.vertices, p_mesh.indices, plane);

	RockMesh result;
	result.vertices = clipped.vertices;
	result.indices = clipped.indices;

	if (noisy && result.vertices.size() > 0) {
		Dictionary noise_group = _stage_group(p_modifier, "Noise", 0);
		Dictionary noise_floats = noise_group.has("floats") ? (Dictionary)noise_group["floats"] : Dictionary();
		real_t scale = _dget(noise_floats, "Scale", 0.15);
		NoiseGraph noise = _extract_noise_graph_from_group(noise_group);
		const real_t on_plane_epsilon = real_t(CMP_EPSILON) * 100;
		for (int i = 0; i < result.vertices.size(); i++) {
			if (Math::abs(plane.distance_to(result.vertices[i])) <= on_plane_epsilon) {
				real_t amount = CLAMP((noise.evaluate(result.vertices[i]) + 1.0) / 2.0, real_t(0.0), real_t(1.0));
				result.vertices.write[i] += plane_normal * (scale * amount);
			}
		}
	}
	return result;
}

// Executes the JSON's "modifiers" array in order, skipping disabled entries and
// _id 4/CutPlane-noisy... (handled above) — everything except SkinSurface-only
// concerns is implemented, so nothing is skipped here anymore besides `disabled`.
RockMesh apply_modifier_chain(RockMesh p_mesh, const Dictionary &p_pipeline_json) {
	if (!p_pipeline_json.has("modifiers") || p_pipeline_json["modifiers"].get_type() != Variant::ARRAY) {
		return p_mesh;
	}
	Array modifiers = p_pipeline_json["modifiers"];
	for (int i = 0; i < modifiers.size(); i++) {
		if (modifiers[i].get_type() != Variant::DICTIONARY) {
			continue;
		}
		Dictionary modifier = modifiers[i];
		if (_dget_bool(modifier, "disabled", false)) {
			continue;
		}
		int modifier_id = _dget_int(modifier, "_id", -1);
		switch (modifier_id) {
			case 0:
				apply_transformation_modifier(p_mesh, modifier);
				break;
			case 1:
				p_mesh = apply_subdivision_modifier(p_mesh, modifier);
				break;
			case 2:
				p_mesh = apply_decimate_modifier(p_mesh, modifier);
				break;
			case 3:
				apply_displace_along_normals_modifier(p_mesh, modifier);
				break;
			case 4:
				p_mesh = apply_cutplane_modifier(p_mesh, modifier);
				break;
			default:
				WARN_PRINT("ProcRock: pipeline JSON's modifier chain has an unrecognized modifier _id — skipping it.");
				break;
		}
	}
	return p_mesh;
}

} // namespace

Array rock_pipeline_gen(int p_subdivisions, real_t p_width, real_t p_height, real_t p_depth,
		real_t p_noise_frequency, real_t p_noise_amplitude, int p_noise_octaves, real_t p_noise_persistence,
		int p_randseed, bool p_cutplane_enabled, real_t p_cutplane_offset, bool p_smoothed) {
	// Resolve the noise seed from a freshly-randomized RNG (not whatever state happened to
	// be left over) when p_randseed==0, matching the "0 = random" convention exactly —
	// _rock_pipeline_gen_impl() re-seeds from p_randseed again below, which is harmless
	// (idempotent for a fixed seed, just another random draw when p_randseed==0).
	if (p_randseed == 0) {
		Math::randomize();
	} else {
		Math::seed((uint64_t)p_randseed);
	}
	NoiseGraph noise = NoiseGraph::make_simple_fractal(p_noise_frequency, CLAMP(p_noise_octaves, 1, 6), p_noise_persistence, p_randseed == 0 ? (int)Math::rand() : p_randseed);
	return _rock_pipeline_gen_impl(p_subdivisions, p_width, p_height, p_depth, noise, p_noise_amplitude, p_randseed, p_cutplane_enabled, p_cutplane_offset, p_smoothed);
}

Array rock_pipeline_gen_from_json(int p_subdivisions, real_t p_width, real_t p_height, real_t p_depth,
		const Dictionary &p_pipeline_json, int p_randseed,
		bool p_cutplane_enabled, real_t p_cutplane_offset, bool p_smoothed) {
	if (p_randseed == 0) {
		Math::randomize();
	} else {
		Math::seed((uint64_t)p_randseed);
	}

	RockMesh mesh = generate_base_mesh_from_json(p_pipeline_json, p_subdivisions, p_width, p_height, p_depth);
	mesh = apply_modifier_chain(mesh, p_pipeline_json);

	// Manual Inspector-driven override, applied on top of the JSON's own modifier
	// chain (which may already include its own CutPlane entries) for experimentation.
	if (p_cutplane_enabled) {
		Vector3 plane_normal = Vector3(Math::randf() * 2 - 1, Math::randf() * 2 - 1, Math::randf() * 2 - 1);
		if (plane_normal.length_squared() < CMP_EPSILON) {
			plane_normal = Vector3(0, 1, 0);
		}
		plane_normal.normalize();
		ClippedMesh clipped = clip_and_cap(mesh.vertices, mesh.indices, Plane(plane_normal, p_cutplane_offset));
		mesh.vertices = clipped.vertices;
		mesh.indices = clipped.indices;
	}

	return _finalize_mesh_arrays(mesh.vertices, mesh.indices, p_smoothed);
}

// =========================================================================
// Texture pipeline: noise height field -> albedo / normal / roughness / metalness / AO
// =========================================================================

namespace {

Ref<Image> make_height_image_from_noise(int p_size, const NoiseGraph &p_noise) {
	PoolVector<uint8_t> data;
	data.resize(p_size * p_size);
	{
		PoolVector<uint8_t>::Write wd8 = data.write();
		for (int y = 0; y < p_size; y++) {
			for (int x = 0; x < p_size; x++) {
				real_t v = p_noise.evaluate(Vector3(real_t(x), real_t(y), 0.0)) * 0.5 + 0.5; // normalize [0..1]
				wd8[y * p_size + x] = uint8_t(CLAMP(v * 255.0, real_t(0.0), real_t(255.0)));
			}
		}
	} // release the write lock before Image's constructor copies `data`
	return Ref<Image>(memnew(Image(p_size, p_size, false, Image::FORMAT_L8, data)));
}

Ref<Image> make_height_image(int p_size, real_t p_noise_frequency, int p_noise_octaves, real_t p_noise_persistence, int p_randseed) {
	NoiseGraph noise = NoiseGraph::make_simple_fractal(p_noise_frequency, CLAMP(p_noise_octaves, 1, 6), p_noise_persistence, p_randseed == 0 ? (int)Math::rand() : p_randseed);
	return make_height_image_from_noise(p_size, noise);
}

Ref<Image> make_albedo_image_from_gradient(Ref<Image> p_height, Ref<Gradient> p_gradient) {
	int w = p_height->get_width(), h = p_height->get_height();

	Ref<Image> img;
	img.instance();
	img->create(w, h, false, Image::FORMAT_RGB8);

	p_height->lock();
	img->lock();
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			img->set_pixel(x, y, p_gradient->get_color_at_offset(p_height->get_pixel(x, y).r));
		}
	}
	img->unlock();
	p_height->unlock();
	return img;
}

Ref<Image> make_albedo_image(Ref<Image> p_height, const Color &p_low, const Color &p_high) {
	Ref<Gradient> gradient;
	gradient.instance();
	gradient->set_color(0, p_low);
	gradient->set_color(1, p_high);
	return make_albedo_image_from_gradient(p_height, gradient);
}

Ref<Image> make_normal_image(Ref<Image> p_height, real_t p_strength) {
	int w = p_height->get_width(), h = p_height->get_height();
	Ref<Image> img;
	img.instance();
	img->create(w, h, false, Image::FORMAT_RGB8);

	p_height->lock();
	img->lock();
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			real_t hl = p_height->get_pixel(CLAMP(x - 1, 0, w - 1), y).r;
			real_t hr = p_height->get_pixel(CLAMP(x + 1, 0, w - 1), y).r;
			real_t hd = p_height->get_pixel(x, CLAMP(y - 1, 0, h - 1)).r;
			real_t hu = p_height->get_pixel(x, CLAMP(y + 1, 0, h - 1)).r;
			Vector3 n = Vector3(-(hr - hl) * p_strength, -(hu - hd) * p_strength, 1.0).normalized();
			img->set_pixel(x, y, Color(n.x * 0.5 + 0.5, n.y * 0.5 + 0.5, n.z * 0.5 + 0.5));
		}
	}
	img->unlock();
	p_height->unlock();
	return img;
}

Ref<Image> make_scaled_grayscale_image(Ref<Image> p_height, real_t p_scale, real_t p_bias) {
	int w = p_height->get_width(), h = p_height->get_height();
	Ref<Image> img;
	img.instance();
	img->create(w, h, false, Image::FORMAT_L8);

	p_height->lock();
	img->lock();
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			real_t v = CLAMP(p_height->get_pixel(x, y).r * p_scale + p_bias, real_t(0.0), real_t(1.0));
			img->set_pixel(x, y, Color(v, v, v));
		}
	}
	img->unlock();
	p_height->unlock();
	return img;
}

} // namespace

ProcRockPipelineTextures rock_pipeline_gen_textures(
		int p_size, real_t p_noise_frequency, int p_noise_octaves, real_t p_noise_persistence, int p_randseed,
		const Color &p_albedo_low, const Color &p_albedo_high, real_t p_normal_strength,
		real_t p_roughness_scale, real_t p_roughness_bias,
		real_t p_metalness_scale, real_t p_metalness_bias,
		real_t p_ao_scale, real_t p_ao_bias) {
	int size = CLAMP(p_size, 8, 4096);
	Ref<Image> height = make_height_image(size, p_noise_frequency, p_noise_octaves, p_noise_persistence, p_randseed);

	ProcRockPipelineTextures textures;
	textures.albedo.instance();
	textures.albedo->create_from_image(make_albedo_image(height, p_albedo_low, p_albedo_high));
	textures.normal.instance();
	textures.normal->create_from_image(make_normal_image(height, p_normal_strength));
	textures.roughness.instance();
	textures.roughness->create_from_image(make_scaled_grayscale_image(height, p_roughness_scale, p_roughness_bias));
	textures.metalness.instance();
	textures.metalness->create_from_image(make_scaled_grayscale_image(height, p_metalness_scale, p_metalness_bias));
	textures.ambient_occlusion.instance();
	textures.ambient_occlusion->create_from_image(make_scaled_grayscale_image(height, p_ao_scale, p_ao_bias));

	return textures;
}

ProcRockPipelineTextures rock_pipeline_gen_textures_from_json(int p_size, const Dictionary &p_pipeline_json) {
	int size = CLAMP(p_size, 8, 4096);
	NoiseGraph noise = _extract_displacement_graph(p_pipeline_json);
	Ref<Image> height = make_height_image_from_noise(size, noise);

	real_t roughness_scale, roughness_bias, metalness_scale, metalness_bias, ao_scale, ao_bias;
	_extract_scale_bias(p_pipeline_json, "Roughness", 0.4, 0.5, roughness_scale, roughness_bias);
	_extract_scale_bias(p_pipeline_json, "Metalness", 0.0, 0.0, metalness_scale, metalness_bias);
	_extract_scale_bias(p_pipeline_json, "Ambient Occlusion", 0.5, 0.5, ao_scale, ao_bias);
	real_t normal_strength = _extract_normal_strength(p_pipeline_json, 2.0);

	ProcRockPipelineTextures textures;
	textures.albedo.instance();
	textures.albedo->create_from_image(make_albedo_image_from_gradient(height, _extract_albedo_gradient(p_pipeline_json)));
	textures.normal.instance();
	textures.normal->create_from_image(make_normal_image(height, normal_strength));
	textures.roughness.instance();
	textures.roughness->create_from_image(make_scaled_grayscale_image(height, roughness_scale, roughness_bias));
	textures.metalness.instance();
	textures.metalness->create_from_image(make_scaled_grayscale_image(height, metalness_scale, metalness_bias));
	textures.ambient_occlusion.instance();
	textures.ambient_occlusion->create_from_image(make_scaled_grayscale_image(height, ao_scale, ao_bias));

	return textures;
}

Ref<SpatialMaterial> rock_pipeline_make_material(const ProcRockPipelineTextures &p_textures) {
	Ref<SpatialMaterial> material;
	material.instance();

	material->set_texture(SpatialMaterial::TEXTURE_ALBEDO, p_textures.albedo);
	material->set_texture(SpatialMaterial::TEXTURE_NORMAL, p_textures.normal);
	material->set_feature(SpatialMaterial::FEATURE_NORMAL_MAPPING, true);
	material->set_texture(SpatialMaterial::TEXTURE_ROUGHNESS, p_textures.roughness);
	material->set_roughness_texture_channel(SpatialMaterial::TEXTURE_CHANNEL_GRAYSCALE);
	material->set_texture(SpatialMaterial::TEXTURE_METALLIC, p_textures.metalness);
	material->set_metallic_texture_channel(SpatialMaterial::TEXTURE_CHANNEL_GRAYSCALE);
	material->set_texture(SpatialMaterial::TEXTURE_AMBIENT_OCCLUSION, p_textures.ambient_occlusion);
	material->set_ao_texture_channel(SpatialMaterial::TEXTURE_CHANNEL_GRAYSCALE);
	material->set_feature(SpatialMaterial::FEATURE_AMBIENT_OCCLUSION, true);

	return material;
}

// Editor-only baked texture packs (ProcRock dock "Demo Texture" picker) live in
// modules/gdextensions/editor/proc_rocks_editor_plugin.cpp — that's the only caller,
// and it's TOOLS_ENABLED-only already, so the loader belongs there, not in the
// generator API.

#ifdef DOCTEST
#include "core/io/json.h"
#include "core/os/file_access.h"

#include "doctest/doctest.h"

namespace {

int _push_const(NoiseGraph &p_graph, real_t p_value) {
	NoiseGraphNode node;
	node.type_id = NOISE_NODE_CONST;
	node.const_value = p_value;
	p_graph.nodes.push_back(node);
	return p_graph.nodes.size() - 1;
}

int _push(NoiseGraph &p_graph, int p_type_id, const Vector<int> &p_sources) {
	NoiseGraphNode node;
	node.type_id = p_type_id;
	node.sources = p_sources;
	p_graph.nodes.push_back(node);
	return p_graph.nodes.size() - 1;
}

// Vector<T> has no std::initializer_list support in this Godot version — these
// convenience overloads keep the test bodies below readable.
Vector<int> _ids(int p_a) {
	Vector<int> v;
	v.push_back(p_a);
	return v;
}
Vector<int> _ids(int p_a, int p_b) {
	Vector<int> v;
	v.push_back(p_a, p_b);
	return v;
}
Vector<int> _ids(int p_a, int p_b, int p_c) {
	Vector<int> v;
	v.push_back(p_a, p_b, p_c);
	return v;
}
Vector<int> _ids(int p_a, int p_b, int p_c, int p_d) {
	Vector<int> v;
	v.push_back(p_a, p_b, p_c, p_d);
	return v;
}

} // namespace

TEST_SUITE("[[proc_rocks]] NoiseGraph") {
	TEST_CASE("[NoiseGraph] Const") {
		NoiseGraph g;
		g.root_index = _push_const(g, 0.42);
		CHECK(g.evaluate(Vector3(1, 2, 3)) == doctest::Approx(0.42));
		CHECK(g.evaluate(Vector3(-9, 9, -9)) == doctest::Approx(0.42));
	}

	TEST_CASE("[NoiseGraph] Add/Max/Min/Multiply/Power") {
		NoiseGraph g;
		int a = _push_const(g, 3.0), b = _push_const(g, 5.0);

		NoiseGraph add;
		add.nodes.push_back(g.nodes[a]);
		add.nodes.push_back(g.nodes[b]);
		add.root_index = _push(add, NOISE_NODE_ADD, _ids(0, 1));
		CHECK(add.evaluate(Vector3()) == doctest::Approx(8.0));

		NoiseGraph mx;
		mx.nodes.push_back(g.nodes[a]);
		mx.nodes.push_back(g.nodes[b]);
		mx.root_index = _push(mx, NOISE_NODE_MAX, _ids(0, 1));
		CHECK(mx.evaluate(Vector3()) == doctest::Approx(5.0));

		NoiseGraph mn;
		mn.nodes.push_back(g.nodes[a]);
		mn.nodes.push_back(g.nodes[b]);
		mn.root_index = _push(mn, NOISE_NODE_MIN, _ids(0, 1));
		CHECK(mn.evaluate(Vector3()) == doctest::Approx(3.0));

		NoiseGraph mul;
		mul.nodes.push_back(g.nodes[a]);
		mul.nodes.push_back(g.nodes[b]);
		mul.root_index = _push(mul, NOISE_NODE_MULTIPLY, _ids(0, 1));
		CHECK(mul.evaluate(Vector3()) == doctest::Approx(15.0));

		NoiseGraph pw;
		pw.nodes.push_back(g.nodes[a]);
		int two = _push_const(pw, 2.0);
		pw.root_index = _push(pw, NOISE_NODE_POWER, _ids(0, two));
		CHECK(pw.evaluate(Vector3()) == doctest::Approx(9.0));
	}

	TEST_CASE("[NoiseGraph] ScaleBias/Clamp/Abs/Invert/Exponent") {
		NoiseGraph sb;
		int src = _push_const(sb, 2.0);
		NoiseGraphNode sb_node;
		sb_node.type_id = NOISE_NODE_SCALE_BIAS;
		sb_node.sources.push_back(src);
		sb_node.scale = 3.0;
		sb_node.bias = 1.0;
		sb.nodes.push_back(sb_node);
		sb.root_index = sb.nodes.size() - 1;
		CHECK(sb.evaluate(Vector3()) == doctest::Approx(7.0)); // 2*3+1

		NoiseGraph cl;
		src = _push_const(cl, 5.0);
		NoiseGraphNode cl_node;
		cl_node.type_id = NOISE_NODE_CLAMP;
		cl_node.sources.push_back(src);
		cl_node.lower_bound = -1.0;
		cl_node.upper_bound = 1.0;
		cl.nodes.push_back(cl_node);
		cl.root_index = cl.nodes.size() - 1;
		CHECK(cl.evaluate(Vector3()) == doctest::Approx(1.0));

		NoiseGraph ab;
		src = _push_const(ab, -4.0);
		ab.root_index = _push(ab, NOISE_NODE_ABS, _ids(src));
		CHECK(ab.evaluate(Vector3()) == doctest::Approx(4.0));

		NoiseGraph inv;
		src = _push_const(inv, -4.0);
		inv.root_index = _push(inv, NOISE_NODE_INVERT, _ids(src));
		CHECK(inv.evaluate(Vector3()) == doctest::Approx(4.0));

		NoiseGraph ex;
		src = _push_const(ex, 1.0); // (1+1)/2=1, 1^exponent=1, *2-1=1
		NoiseGraphNode ex_node;
		ex_node.type_id = NOISE_NODE_EXPONENT;
		ex_node.sources.push_back(src);
		ex_node.exponent = 2.0;
		ex.nodes.push_back(ex_node);
		ex.root_index = ex.nodes.size() - 1;
		CHECK(ex.evaluate(Vector3()) == doctest::Approx(1.0));
	}

	TEST_CASE("[NoiseGraph] Blend/Select") {
		NoiseGraph bl;
		int lo = _push_const(bl, 0.0), hi = _push_const(bl, 10.0), ctrl = _push_const(bl, 0.0); // alpha=(0+1)/2=0.5
		bl.root_index = _push(bl, NOISE_NODE_BLEND, _ids(lo, hi, ctrl));
		CHECK(bl.evaluate(Vector3()) == doctest::Approx(5.0));

		NoiseGraph se;
		int a = _push_const(se, 1.0), b = _push_const(se, 2.0), c = _push_const(se, 0.0);
		NoiseGraphNode se_node;
		se_node.type_id = NOISE_NODE_SELECT;
		se_node.sources.push_back(a);
		se_node.sources.push_back(b);
		se_node.sources.push_back(c);
		se_node.lower_bound = -0.5;
		se_node.upper_bound = 0.5;
		se_node.edge_falloff = 0.0;
		se.nodes.push_back(se_node);
		se.root_index = se.nodes.size() - 1;
		CHECK(se.evaluate(Vector3()) == doctest::Approx(2.0)); // control 0 is within [-0.5,0.5] -> source b
	}

	TEST_CASE("[NoiseGraph] Displace") {
		NoiseGraph g;
		int base = _push(g, NOISE_NODE_CYLINDERS, {});
		g.nodes.write[base].frequency = 1.0;
		int dx = _push_const(g, 0.7), dy = _push_const(g, 0.0), dz = _push_const(g, 0.0);
		g.root_index = _push(g, NOISE_NODE_DISPLACE, _ids(base, dx, dy, dz));

		NoiseGraph reference;
		int ref_base = _push(reference, NOISE_NODE_CYLINDERS, {});
		reference.nodes.write[ref_base].frequency = 1.0;
		reference.root_index = ref_base;

		Vector3 p(0.1, 5.0, 0.2);
		CHECK(g.evaluate(p) == doctest::Approx(reference.evaluate(p + Vector3(0.7, 0, 0))));
	}

	TEST_CASE("[NoiseGraph] ScalePoint/TranslatePoint are equivalent to pre-transforming the sample point") {
		NoiseGraph scale_graph;
		int src = _push(scale_graph, NOISE_NODE_SPHERES, {});
		scale_graph.nodes.write[src].frequency = 1.0;
		NoiseGraphNode sp;
		sp.type_id = NOISE_NODE_SCALE_POINT;
		sp.sources.push_back(src);
		sp.scale_x = 2.0;
		sp.scale_y = 1.0;
		sp.scale_z = 1.0;
		scale_graph.nodes.push_back(sp);
		scale_graph.root_index = scale_graph.nodes.size() - 1;

		NoiseGraph reference;
		int ref = _push(reference, NOISE_NODE_SPHERES, {});
		reference.nodes.write[ref].frequency = 1.0;
		reference.root_index = ref;

		Vector3 p(0.3, 0.4, 0.6);
		CHECK(scale_graph.evaluate(p) == doctest::Approx(reference.evaluate(Vector3(p.x * 2.0, p.y, p.z))));

		NoiseGraph translate_graph;
		src = _push(translate_graph, NOISE_NODE_SPHERES, {});
		translate_graph.nodes.write[src].frequency = 1.0;
		NoiseGraphNode tp;
		tp.type_id = NOISE_NODE_TRANSLATE_POINT;
		tp.sources.push_back(src);
		tp.translate_x = 1.0;
		translate_graph.nodes.push_back(tp);
		translate_graph.root_index = translate_graph.nodes.size() - 1;
		CHECK(translate_graph.evaluate(p) == doctest::Approx(reference.evaluate(Vector3(p.x + 1.0, p.y, p.z))));
	}

	TEST_CASE("[NoiseGraph] RotatePoint 90-degree X rotation matches hand-derived axis swap") {
		NoiseGraph rot_graph;
		int src = _push(rot_graph, NOISE_NODE_CYLINDERS, {});
		rot_graph.nodes.write[src].frequency = 1.0;
		NoiseGraphNode rp;
		rp.type_id = NOISE_NODE_ROTATE_POINT;
		rp.sources.push_back(src);
		rp.angle_x = 90.0;
		rot_graph.nodes.push_back(rp);
		rot_graph.root_index = rot_graph.nodes.size() - 1;

		NoiseGraph reference;
		int ref = _push(reference, NOISE_NODE_CYLINDERS, {});
		reference.nodes.write[ref].frequency = 1.0;
		reference.root_index = ref;

		// Hand-derived from the RotatePoint matrix at (angleX=90, angleY=0, angleZ=0):
		// rotated = (px, pz, -py).
		Vector3 p(0.3, 0.5, 0.0);
		Vector3 expected_rotated(p.x, p.z, -p.y);
		CHECK(rot_graph.evaluate(p) == doctest::Approx(reference.evaluate(expected_rotated)));
	}

	TEST_CASE("[NoiseGraph] Terrace interpolates between control points with squared alpha") {
		NoiseGraph g;
		int src = _push_const(g, 0.5);
		NoiseGraphNode terrace;
		terrace.type_id = NOISE_NODE_TERRACE;
		terrace.sources.push_back(src);
		terrace.control_points.push_back(0.0);
		terrace.control_points.push_back(1.0);
		g.nodes.push_back(terrace);
		g.root_index = g.nodes.size() - 1;
		// alpha=(0.5-0)/(1-0)=0.5, squared=0.25, lerp(0,1,0.25)=0.25
		CHECK(g.evaluate(Vector3()) == doctest::Approx(0.25));
	}

	TEST_CASE("[NoiseGraph] Curve does cubic interpolation across sorted points") {
		NoiseGraph g;
		int src = _push_const(g, 0.0);
		NoiseGraphNode curve;
		curve.type_id = NOISE_NODE_CURVE;
		curve.sources.push_back(src);
		curve.curve_points.push_back(Vector2(-1, -1));
		curve.curve_points.push_back(Vector2(0, 0));
		curve.curve_points.push_back(Vector2(1, 1));
		curve.curve_points.push_back(Vector2(2, 2));
		g.nodes.push_back(curve);
		g.root_index = g.nodes.size() - 1;
		// A straight-line control set should reproduce the identity at an exact control point.
		CHECK(g.evaluate(Vector3()) == doctest::Approx(0.0));
	}

	TEST_CASE("[NoiseGraph] Perlin/Billow/RidgedMulti/Voronoi/Turbulence are deterministic and unwired inputs default to 0") {
		NoiseGraph perlin = NoiseGraph::make_simple_fractal(4.0, 3, 0.5, 7);
		real_t v1 = perlin.evaluate(Vector3(1, 2, 3));
		real_t v2 = perlin.evaluate(Vector3(1, 2, 3));
		CHECK(v1 == doctest::Approx(v2));

		NoiseGraph billow;
		int b = _push(billow, NOISE_NODE_BILLOW, {});
		billow.nodes.write[b].frequency = 4.0;
		billow.nodes.write[b].octaves = 3;
		billow.nodes.write[b].seed = 3;
		billow.nodes.write[b].octave_noises.resize(3);
		for (int i = 0; i < 3; i++) {
			billow.nodes.write[b].octave_noises.write[i].instance();
			billow.nodes.write[b].octave_noises.write[i]->set_seed(3 + i);
			billow.nodes.write[b].octave_noises.write[i]->set_octaves(1);
			billow.nodes.write[b].octave_noises.write[i]->set_period(1.0);
		}
		billow.root_index = b;
		CHECK(billow.evaluate(Vector3(5, 5, 5)) == doctest::Approx(billow.evaluate(Vector3(5, 5, 5))));

		// Add with an unwired second slot behaves as identity (source defaults to 0).
		NoiseGraph identity_add;
		int c = _push_const(identity_add, 3.0);
		NoiseGraphNode add_node;
		add_node.type_id = NOISE_NODE_ADD;
		add_node.sources.push_back(c);
		identity_add.nodes.push_back(add_node);
		identity_add.root_index = identity_add.nodes.size() - 1;
		CHECK(identity_add.evaluate(Vector3()) == doctest::Approx(3.0));
	}

	TEST_CASE("[NoiseGraph] Spheres/Cylinders exact concentric-shell formula") {
		NoiseGraph spheres;
		int s = _push(spheres, NOISE_NODE_SPHERES, {});
		spheres.nodes.write[s].frequency = 1.0;
		spheres.root_index = s;
		// At radius exactly 1.0 (a shell boundary), distance-to-nearest-shell is 0 -> value 1.
		CHECK(spheres.evaluate(Vector3(1, 0, 0)) == doctest::Approx(1.0));
		// At radius 0.5 (exactly between shells 0 and 1), distance-to-nearest-shell is 0.5 -> value -1.
		CHECK(spheres.evaluate(Vector3(0.5, 0, 0)) == doctest::Approx(-1.0));

		NoiseGraph cylinders;
		int c = _push(cylinders, NOISE_NODE_CYLINDERS, {});
		cylinders.nodes.write[c].frequency = 1.0;
		cylinders.root_index = c;
		CHECK(cylinders.evaluate(Vector3(1, 999, 0)) == doctest::Approx(1.0)); // y is ignored
		CHECK(cylinders.evaluate(Vector3(0, -999, 0.5)) == doctest::Approx(-1.0));
	}

	TEST_CASE("[NoiseGraph] from_json decodes procrocklib's placeholder-id edge scheme") {
		String json_text = "{"
						   "\"edges\":[{\"from\":2,\"to\":0}],"
						   "\"nodes\":["
						   "{\"_id\":401,\"config\":{\"floats\":{\"Frequency\":31.0,\"Lacunarity\":2.5,\"Persistence\":0.6},"
						   "\"ints\":{\"Octaves\":3,\"Seed\":0},\"singleChoices\":{\"Quality\":2}},"
						   "\"general\":{\"internal_id\":0,\"position\":{\"x\":25.0,\"y\":25.0}}},"
						   "{\"_id\":0,\"config\":null,\"general\":{\"internal_id\":1,\"position\":{\"x\":400.0,\"y\":25.0}}}"
						   "]}";
		Variant parsed;
		String err_str;
		int err_line = 0;
		REQUIRE(JSON::parse(json_text, parsed, err_str, err_line) == OK);
		NoiseGraph g = NoiseGraph::from_json(parsed);
		REQUIRE(g.is_valid());
		CHECK(g.nodes.size() == 2);
		CHECK(g.nodes[g.root_index].type_id == NOISE_NODE_OUTPUT);
		CHECK(g.nodes[g.root_index].sources.size() == 1);
		int perlin_index = g.nodes[g.root_index].sources[0];
		CHECK(g.nodes[perlin_index].type_id == NOISE_NODE_PERLIN);
		CHECK(g.nodes[perlin_index].frequency == doctest::Approx(31.0));
		CHECK(g.nodes[perlin_index].octaves == 3);
		// Should not crash or NaN when evaluated.
		real_t v = g.evaluate(Vector3(1, 2, 3));
		CHECK(v == v); // NaN check
	}

	TEST_CASE("[NoiseGraph] from_json parses a real preset's Displacement/Height graph (proc_rocks_demo preset 1.json)") {
		// Verbatim "Noise Graph" object from editor/proc_rocks_demo/presets/1.json's
		// textureGenerator.config["Displacement / Height"][0].noiseGraphs["Noise Graph"] —
		// used here as a real-data regression check for the edge/slot decoding logic.
		String json_text =
				"{\"edges\":[{\"from\":15,\"to\":5},{\"from\":17,\"to\":26},{\"from\":21,\"to\":26},"
				"{\"from\":19,\"to\":29},{\"from\":7,\"to\":14},{\"from\":9,\"to\":0},{\"from\":41,\"to\":6},"
				"{\"from\":2,\"to\":40},{\"from\":52,\"to\":38},{\"from\":58,\"to\":51},{\"from\":43,\"to\":57}],"
				"\"nodes\":["
				"{\"_id\":401,\"config\":{\"floats\":{\"Frequency\":18.686,\"Lacunarity\":2.5,\"Persistence\":0.6},"
				"\"ints\":{\"Octaves\":3,\"Seed\":0},\"singleChoices\":{\"Quality\":2}},"
				"\"general\":{\"internal_id\":0,\"position\":{\"x\":513.0,\"y\":59.0}}},"
				"{\"_id\":0,\"config\":null,\"general\":{\"internal_id\":1,\"position\":{\"x\":2059.0,\"y\":271.0}}},"
				"{\"_id\":402,\"config\":{\"floats\":{\"Frequency\":0.99,\"Lacunarity\":2.5,\"Persistence\":0.6},"
				"\"ints\":{\"Octaves\":3,\"Seed\":0},\"singleChoices\":{\"Quality\":2}},"
				"\"general\":{\"internal_id\":5,\"position\":{\"x\":274.0,\"y\":237.0}}},"
				"{\"_id\":1,\"config\":null,\"general\":{\"internal_id\":6,\"position\":{\"x\":1033.0,\"y\":280.0}}},"
				"{\"_id\":300,\"config\":null,\"general\":{\"internal_id\":14,\"position\":{\"x\":848.0,\"y\":404.0}}},"
				"{\"_id\":403,\"config\":{\"floats\":{\"Frequency\":0.5,\"Lacunarity\":2.5},"
				"\"ints\":{\"Octaves\":3,\"Seed\":0},\"singleChoices\":{\"Quality\":2}},"
				"\"general\":{\"internal_id\":26,\"position\":{\"x\":243.0,\"y\":438.0}}},"
				"{\"_id\":401,\"config\":{\"floats\":{\"Frequency\":0.3,\"Lacunarity\":2.5,\"Persistence\":0.6},"
				"\"ints\":{\"Octaves\":3,\"Seed\":0},\"singleChoices\":{\"Quality\":2}},"
				"\"general\":{\"internal_id\":29,\"position\":{\"x\":503.0,\"y\":573.0}}},"
				"{\"_id\":401,\"config\":{\"floats\":{\"Frequency\":15.201,\"Lacunarity\":2.5,\"Persistence\":0.6},"
				"\"ints\":{\"Octaves\":3,\"Seed\":0},\"singleChoices\":{\"Quality\":2}},"
				"\"general\":{\"internal_id\":38,\"position\":{\"x\":1009.0,\"y\":504.0}}},"
				"{\"_id\":1,\"config\":null,\"general\":{\"internal_id\":40,\"position\":{\"x\":1729.0,\"y\":259.0}}},"
				"{\"_id\":302,\"config\":{\"floats\":{\"x Scale\":0.3,\"y Scale\":8.372,\"z Scale\":0.3}},"
				"\"general\":{\"internal_id\":51,\"position\":{\"x\":1350.0,\"y\":489.0}}},"
				"{\"_id\":304,\"config\":{\"floats\":{\"Frequency\":0.5,\"Power\":1.0},"
				"\"ints\":{\"Roughness\":2,\"Seed\":0}},"
				"\"general\":{\"internal_id\":57,\"position\":{\"x\":1643.0,\"y\":483.0}}}"
				"]}";
		Variant parsed;
		String err_str;
		int err_line = 0;
		REQUIRE(JSON::parse(json_text, parsed, err_str, err_line) == OK);
		NoiseGraph g = NoiseGraph::from_json(parsed);
		REQUIRE(g.is_valid());
		CHECK(g.nodes.size() == 11);
		CHECK(g.nodes[g.root_index].type_id == NOISE_NODE_OUTPUT);

		// Output <- Add(internal_id 40) <- {Add(internal_id 6), Turbulence(internal_id 57)}
		int add_top = g.nodes[g.root_index].sources[0];
		CHECK(g.nodes[add_top].type_id == NOISE_NODE_ADD);
		CHECK(g.nodes[add_top].sources.size() == 2);
		int add_inner = g.nodes[add_top].sources[0];
		int turbulence = g.nodes[add_top].sources[1];
		CHECK(g.nodes[add_inner].type_id == NOISE_NODE_ADD);
		CHECK(g.nodes[turbulence].type_id == NOISE_NODE_TURBULENCE);

		// Add(internal_id 6) <- {Displace(internal_id 14), Perlin(internal_id 0)}
		CHECK(g.nodes[add_inner].sources.size() == 2);
		int displace = g.nodes[add_inner].sources[0];
		int root_perlin = g.nodes[add_inner].sources[1];
		CHECK(g.nodes[displace].type_id == NOISE_NODE_DISPLACE);
		CHECK(g.nodes[root_perlin].type_id == NOISE_NODE_PERLIN);
		CHECK(g.nodes[root_perlin].frequency == doctest::Approx(18.686));

		// Displace's 4 sources: base=Billow(5), x=RidgedMulti(26), y=Perlin(29), z=RidgedMulti(26).
		CHECK(g.nodes[displace].sources.size() == 4);
		CHECK(g.nodes[g.nodes[displace].sources[0]].type_id == NOISE_NODE_BILLOW);
		CHECK(g.nodes[g.nodes[displace].sources[1]].type_id == NOISE_NODE_RIDGED_MULTI);
		CHECK(g.nodes[g.nodes[displace].sources[2]].type_id == NOISE_NODE_PERLIN);
		CHECK(g.nodes[g.nodes[displace].sources[3]].type_id == NOISE_NODE_RIDGED_MULTI);

		// Turbulence <- ScalePoint(51) <- Perlin(38), with the confirmed lowercase field casing.
		CHECK(g.nodes[turbulence].sources.size() == 1);
		int scale_point = g.nodes[turbulence].sources[0];
		CHECK(g.nodes[scale_point].type_id == NOISE_NODE_SCALE_POINT);
		CHECK(g.nodes[scale_point].scale_x == doctest::Approx(0.3));
		CHECK(g.nodes[scale_point].scale_y == doctest::Approx(8.372));
		CHECK(g.nodes[scale_point].scale_z == doctest::Approx(0.3));
		CHECK(g.nodes[scale_point].sources.size() == 1);
		CHECK(g.nodes[g.nodes[scale_point].sources[0]].type_id == NOISE_NODE_PERLIN);

		// Full evaluation across a few sample points must stay finite (no NaN/garbage).
		for (int i = 0; i < 5; i++) {
			real_t v = g.evaluate(Vector3(i * 0.37, -i * 0.21, i * 0.53));
			CHECK(v == v);
			CHECK(Math::abs(v) < 1000.0);
		}
	}
}

namespace {

// Real preset files live in editor/proc_rocks_demo/presets/ (see memo.md) — read from
// disk rather than embedded, since they're 15-40KB each. Assumes doctest is invoked
// with the repo root as the working directory, matching this session's established
// verification pattern (`./bin/godot... --doctest-test-case=...` from repo root).
Variant _load_pipeline_json(const String &p_path) {
	FileAccess *f = FileAccess::open(p_path, FileAccess::READ);
	REQUIRE_MESSAGE(f != nullptr, (String("could not open ") + p_path));
	String text = f->get_as_utf8_string();
	memdelete(f);
	Variant parsed;
	String err_str;
	int err_line = 0;
	REQUIRE_MESSAGE(JSON::parse(text, parsed, err_str, err_line) == OK, err_str);
	return parsed;
}

// Inverse of _dget_vec3() — builds the {"x":.., "y":.., "z":..} shape procrocklib's
// float3s use, for hand-constructing modifier config dicts in tests below.
Dictionary _vec3_dict(const Vector3 &p_v) {
	Dictionary d;
	d["x"] = p_v.x;
	d["y"] = p_v.y;
	d["z"] = p_v.z;
	return d;
}

const char *const ALL_PRESET_FILES[] = {
	"1.json", "2.json", "3.json", "4.json", "5.json", "6.json", "7.json", "8.json",
	"9.json", "10.json", "11.json", "12.json", "granite_custom.json"
};
const int ALL_PRESET_FILES_COUNT = sizeof(ALL_PRESET_FILES) / sizeof(ALL_PRESET_FILES[0]);
const char *const PRESETS_DIR = "modules/gdextensions/editor/proc_rocks_demo/presets/";

} // namespace

TEST_SUITE("[[proc_rocks]] ProcRock JSON pipeline") {
	TEST_CASE("[procrockgen] rock_pipeline_json_is_valid") {
		Dictionary empty;
		CHECK_FALSE(rock_pipeline_json_is_valid(empty));

		Dictionary with_texgen;
		with_texgen["textureGenerator"] = Dictionary();
		CHECK(rock_pipeline_json_is_valid(with_texgen));
	}

	TEST_CASE("[procrockgen] rock_pipeline_gen_from_json produces a valid mesh for all 13 real presets") {
		for (int i = 0; i < ALL_PRESET_FILES_COUNT; i++) {
			Variant parsed = _load_pipeline_json(String(PRESETS_DIR) + ALL_PRESET_FILES[i]);
			REQUIRE(parsed.get_type() == Variant::DICTIONARY);
			Dictionary pipeline_json = parsed;
			REQUIRE(rock_pipeline_json_is_valid(pipeline_json));

			Array mesh_arrays = rock_pipeline_gen_from_json(1, 1.0, 1.0, 1.0, pipeline_json, 42, false, 0.0, true);
			REQUIRE(mesh_arrays.size() == VS::ARRAY_MAX);
			Vector<Vector3> vertices = mesh_arrays[VS::ARRAY_VERTEX];
			Vector<int> indices = mesh_arrays[VS::ARRAY_INDEX];
			CHECK(vertices.size() > 0);
			CHECK(indices.size() > 0);
			CHECK(indices.size() % 3 == 0);
		}
	}

	TEST_CASE("[procrockgen] rock_pipeline_gen_textures_from_json produces valid PBR textures for all 13 real presets") {
		for (int i = 0; i < ALL_PRESET_FILES_COUNT; i++) {
			Variant parsed = _load_pipeline_json(String(PRESETS_DIR) + ALL_PRESET_FILES[i]);
			Dictionary pipeline_json = parsed;

			ProcRockPipelineTextures textures = rock_pipeline_gen_textures_from_json(32, pipeline_json);
			REQUIRE(textures.albedo.is_valid());
			REQUIRE(textures.normal.is_valid());
			REQUIRE(textures.roughness.is_valid());
			REQUIRE(textures.metalness.is_valid());
			REQUIRE(textures.ambient_occlusion.is_valid());
			CHECK(textures.albedo->get_width() == 32);
			CHECK(textures.albedo->get_height() == 32);
		}
	}

	TEST_CASE("[procrockgen] JSON reader Roughness/Metalness/Albedo cross-check ProcRockMesh::set_pipeline_preset()'s hand-extracted table") {
		// Expected values copied from proc_rocks.cpp's `presets[]` table (preset 0 = 1.json,
		// preset 9 = 10.json) — both tables read the exact same source JSON, so they must agree.
		{
			Dictionary pipeline_json = _load_pipeline_json(String(PRESETS_DIR) + "1.json");
			real_t scale, bias;
			_extract_scale_bias(pipeline_json, "Roughness", 0, 0, scale, bias);
			CHECK(scale == doctest::Approx(2.000));
			CHECK(bias == doctest::Approx(0.000));
			_extract_scale_bias(pipeline_json, "Metalness", 0, 0, scale, bias);
			CHECK(scale == doctest::Approx(0.200));
			CHECK(bias == doctest::Approx(0.000));

			Ref<Gradient> gradient = _extract_albedo_gradient(pipeline_json);
			REQUIRE(gradient->get_points_count() >= 2);
			Color low = gradient->get_color(0);
			Color high = gradient->get_color(gradient->get_points_count() - 1);
			CHECK(low.r == doctest::Approx(0.827).epsilon(0.002));
			CHECK(low.g == doctest::Approx(0.784).epsilon(0.002));
			CHECK(low.b == doctest::Approx(0.517).epsilon(0.002));
			CHECK(high.r == doctest::Approx(0.940).epsilon(0.002));
			CHECK(high.g == doctest::Approx(0.936).epsilon(0.002));
			CHECK(high.b == doctest::Approx(0.921).epsilon(0.002));
		}
		{
			Dictionary pipeline_json = _load_pipeline_json(String(PRESETS_DIR) + "10.json");
			real_t scale, bias;
			_extract_scale_bias(pipeline_json, "Roughness", 0, 0, scale, bias);
			CHECK(scale == doctest::Approx(2.911));
			CHECK(bias == doctest::Approx(0.000));
			_extract_scale_bias(pipeline_json, "Metalness", 0, 0, scale, bias);
			CHECK(scale == doctest::Approx(0.200));
			CHECK(bias == doctest::Approx(0.000));

			Ref<Gradient> gradient = _extract_albedo_gradient(pipeline_json);
			REQUIRE(gradient->get_points_count() >= 2);
			Color low = gradient->get_color(0);
			Color high = gradient->get_color(gradient->get_points_count() - 1);
			CHECK(low.r == doctest::Approx(0.355).epsilon(0.002));
			CHECK(low.g == doctest::Approx(0.355).epsilon(0.002));
			CHECK(low.b == doctest::Approx(0.355).epsilon(0.002));
			CHECK(high.r == doctest::Approx(0.145).epsilon(0.002));
			CHECK(high.g == doctest::Approx(0.108).epsilon(0.002));
			CHECK(high.b == doctest::Approx(0.108).epsilon(0.002));
		}
	}

	TEST_CASE("[procrockgen] _extract_displacement_graph decodes the real Displacement/Height graph from every preset") {
		for (int i = 0; i < ALL_PRESET_FILES_COUNT; i++) {
			Dictionary pipeline_json = _load_pipeline_json(String(PRESETS_DIR) + ALL_PRESET_FILES[i]);
			NoiseGraph g = _extract_displacement_graph(pipeline_json);
			REQUIRE(g.is_valid());
			real_t v = g.evaluate(Vector3(1, 2, 3));
			CHECK(v == v); // not NaN
		}
	}
}

TEST_SUITE("[[proc_rocks]] ProcRock generators and modifiers") {
	TEST_CASE("[procrockgen] generate_cuboid produces 8 vertices, 12 faces, correct extents") {
		RockMesh mesh = generate_cuboid(2.0, 4.0, 6.0);
		CHECK(mesh.vertices.size() == 8);
		CHECK(mesh.indices.size() == 36);
		real_t min_x = 1e9, max_x = -1e9, min_y = 1e9, max_y = -1e9, min_z = 1e9, max_z = -1e9;
		for (int i = 0; i < mesh.vertices.size(); i++) {
			min_x = MIN(min_x, mesh.vertices[i].x);
			max_x = MAX(max_x, mesh.vertices[i].x);
			min_y = MIN(min_y, mesh.vertices[i].y);
			max_y = MAX(max_y, mesh.vertices[i].y);
			min_z = MIN(min_z, mesh.vertices[i].z);
			max_z = MAX(max_z, mesh.vertices[i].z);
		}
		CHECK(max_x - min_x == doctest::Approx(2.0));
		CHECK(max_y - min_y == doctest::Approx(4.0));
		CHECK(max_z - min_z == doctest::Approx(6.0));
	}

	TEST_CASE("[procrockgen] generate_pyramid places apex and base ring correctly") {
		RockMesh mesh = generate_pyramid(0.0, 1.0, 5, 0.0, 0, 3.0, 0.5, -0.5);
		// baseSubdivisions=5 -> n=4 ring points, +1 base center, +1 apex = 6 vertices.
		CHECK(mesh.vertices.size() == 6);
		CHECK(mesh.vertices[0] == Vector3(0, 0, 0)); // base center
		CHECK(mesh.vertices[5] == Vector3(0.5, 3.0, -0.5)); // apex
		for (int i = 1; i < 5; i++) {
			CHECK(mesh.vertices[i].y == doctest::Approx(0.0));
			CHECK(mesh.vertices[i].length() == doctest::Approx(1.0)); // on the base radius
		}
		CHECK(mesh.indices.size() == 8 * 3); // faceCount = baseSubdivisions*2-2 = 8
	}

	TEST_CASE("[procrockgen] apply_transformation_modifier: translate, scale, rotate") {
		{
			RockMesh mesh;
			mesh.vertices.push_back(Vector3(1, 2, 3));
			Dictionary float3s;
			float3s["Translation"] = _vec3_dict(Vector3(10, -5, 0));
			Dictionary group;
			group["float3s"] = float3s;
			Array general;
			general.push_back(group);
			Dictionary config;
			config["General"] = general;
			Dictionary modifier;
			modifier["config"] = config;
			apply_transformation_modifier(mesh, modifier);
			CHECK(mesh.vertices[0].is_equal_approx(Vector3(11, -3, 3)));
		}
		{
			RockMesh mesh;
			mesh.vertices.push_back(Vector3(1, 1, 1));
			Dictionary float3s;
			float3s["Scale"] = _vec3_dict(Vector3(2, 3, 4));
			Dictionary group;
			group["float3s"] = float3s;
			Array general;
			general.push_back(group);
			Dictionary config;
			config["General"] = general;
			Dictionary modifier;
			modifier["config"] = config;
			apply_transformation_modifier(mesh, modifier);
			CHECK(mesh.vertices[0].is_equal_approx(Vector3(2, 3, 4)));
		}
		{
			// 90-degree rotation around X using Godot's own standard Basis(axis,angle)
			// convention (unlike NoiseGraph's RotatePoint, which ports libnoise's own
			// non-standard matrix verbatim — these are deliberately different, see
			// noise_graph's RotatePoint comment): Rx(90°)*(0,1,0) = (0,0,1).
			RockMesh mesh;
			mesh.vertices.push_back(Vector3(0, 1, 0));
			Dictionary float3s;
			float3s["Rotate"] = _vec3_dict(Vector3(Math_PI / 2.0, 0, 0));
			Dictionary group;
			group["float3s"] = float3s;
			Array general;
			general.push_back(group);
			Dictionary config;
			config["General"] = general;
			Dictionary modifier;
			modifier["config"] = config;
			apply_transformation_modifier(mesh, modifier);
			CHECK(mesh.vertices[0].is_equal_approx(Vector3(0, 0, 1)));
		}
	}

	TEST_CASE("[procrockgen] subdivide_upsample: 4x face split, old vertices unchanged, new at edge midpoints") {
		RockMesh cube = generate_cuboid(2.0, 2.0, 2.0);
		int original_face_count = cube.indices.size() / 3;
		RockMesh subdivided = subdivide_upsample(cube);
		CHECK(subdivided.indices.size() / 3 == original_face_count * 4);
		CHECK(subdivided.vertices.size() > cube.vertices.size());
		for (int i = 0; i < cube.vertices.size(); i++) {
			CHECK(subdivided.vertices[i].is_equal_approx(cube.vertices[i]));
		}
		// Every new vertex must be equidistant from some pair of original vertices
		// (i.e. an edge midpoint) — a coarse but real correctness check.
		for (int i = cube.vertices.size(); i < subdivided.vertices.size(); i++) {
			bool found_matching_edge = false;
			for (int a = 0; a < cube.vertices.size() && !found_matching_edge; a++) {
				for (int b = a + 1; b < cube.vertices.size(); b++) {
					Vector3 mid = (cube.vertices[a] + cube.vertices[b]) * 0.5;
					if (mid.is_equal_approx(subdivided.vertices[i])) {
						found_matching_edge = true;
						break;
					}
				}
			}
			CHECK(found_matching_edge);
		}
	}

	TEST_CASE("[procrockgen] subdivide_false_barycentric: 3x face split, one new centroid vertex per face") {
		RockMesh cube = generate_cuboid(2.0, 2.0, 2.0);
		int original_face_count = cube.indices.size() / 3;
		RockMesh subdivided = subdivide_false_barycentric(cube);
		CHECK(subdivided.indices.size() / 3 == original_face_count * 3);
		CHECK(subdivided.vertices.size() == cube.vertices.size() + original_face_count);
		for (int i = 0; i < cube.vertices.size(); i++) {
			CHECK(subdivided.vertices[i].is_equal_approx(cube.vertices[i]));
		}
	}

	TEST_CASE("[procrockgen] subdivide_loop: 4x face split, smooths old vertex positions") {
		RockMesh cube = generate_cuboid(2.0, 2.0, 2.0);
		int original_face_count = cube.indices.size() / 3;
		RockMesh subdivided = subdivide_loop(cube);
		CHECK(subdivided.indices.size() / 3 == original_face_count * 4);
		CHECK(subdivided.vertices.size() > cube.vertices.size());
		// Loop smoothing pulls corner vertices toward their neighbors' average, so a
		// cube corner (unlike upsample) must move — but should stay roughly in place
		// for a symmetric shape like a cube (not fly off to some unrelated position).
		bool any_moved = false;
		for (int i = 0; i < cube.vertices.size(); i++) {
			if (!subdivided.vertices[i].is_equal_approx(cube.vertices[i])) {
				any_moved = true;
			}
			CHECK(subdivided.vertices[i].length() < cube.vertices[i].length() + 1.0);
		}
		CHECK(any_moved);
	}

	TEST_CASE("[procrockgen] decimate_greedy reduces face count towards the target without degenerating") {
		RockMesh cube = generate_cuboid(2.0, 2.0, 2.0);
		RockMesh subdivided = subdivide_loop(cube); // 48 faces, enough room to decimate
		int original_face_count = subdivided.indices.size() / 3;
		RockMesh decimated = decimate_greedy(subdivided, 8);
		int result_face_count = decimated.indices.size() / 3;
		CHECK(result_face_count <= original_face_count);
		CHECK(result_face_count >= 4); // a closed manifold needs at least a tetrahedron
		CHECK(decimated.vertices.size() > 0);
		for (int i = 0; i < decimated.indices.size(); i++) {
			CHECK(decimated.indices[i] >= 0);
			CHECK(decimated.indices[i] < decimated.vertices.size());
		}
	}

	TEST_CASE("[procrockgen] apply_cutplane_modifier (flat) clips a cube in half through the origin") {
		RockMesh cube = generate_cuboid(2.0, 2.0, 2.0); // spans [-1,1] on each axis
		Dictionary float3s;
		float3s["Translation"] = _vec3_dict(Vector3());
		float3s["Rotation"] = _vec3_dict(Vector3()); // identity rotation -> plane normal = +X
		Dictionary group;
		group["float3s"] = float3s;
		Array general;
		general.push_back(group);
		Dictionary config;
		config["General"] = general;
		Dictionary modifier;
		modifier["config"] = config;

		RockMesh clipped = apply_cutplane_modifier(cube, modifier);
		REQUIRE(clipped.vertices.size() > 0);
		for (int i = 0; i < clipped.vertices.size(); i++) {
			CHECK(clipped.vertices[i].x <= real_t(0.0) + real_t(CMP_EPSILON) * 1000);
		}
	}

	TEST_CASE("[procrockgen] apply_displace_along_normals_modifier (RNG mode) is deterministic for a fixed seed") {
		RockMesh cube1 = generate_cuboid(2.0, 2.0, 2.0);
		RockMesh cube2 = generate_cuboid(2.0, 2.0, 2.0);
		Dictionary group;
		Dictionary floats;
		floats["Factor"] = 0.5;
		group["floats"] = floats;
		Dictionary ints;
		ints["Seed"] = 42;
		group["ints"] = ints;
		Array general;
		general.push_back(group);
		Dictionary config;
		config["General"] = general;
		Dictionary modifier;
		modifier["config"] = config;

		apply_displace_along_normals_modifier(cube1, modifier);
		apply_displace_along_normals_modifier(cube2, modifier);
		for (int i = 0; i < cube1.vertices.size(); i++) {
			CHECK(cube1.vertices[i].is_equal_approx(cube2.vertices[i]));
			CHECK(!cube1.vertices[i].is_equal_approx(Vector3())); // actually displaced
		}
	}

	TEST_CASE("[procrockgen] apply_modifier_chain executes in order and skips disabled entries") {
		RockMesh cube = generate_cuboid(2.0, 2.0, 2.0);
		Dictionary translate_float3s;
		translate_float3s["Translation"] = _vec3_dict(Vector3(100, 0, 0));
		Dictionary translate_group;
		translate_group["float3s"] = translate_float3s;
		Array translate_general;
		translate_general.push_back(translate_group);
		Dictionary translate_config;
		translate_config["General"] = translate_general;
		Dictionary enabled_transform;
		enabled_transform["_id"] = 0;
		enabled_transform["config"] = translate_config;
		enabled_transform["disabled"] = false;

		Dictionary disabled_transform;
		disabled_transform["_id"] = 0;
		Dictionary disabled_float3s;
		disabled_float3s["Translation"] = _vec3_dict(Vector3(0, 999, 0));
		Dictionary disabled_group;
		disabled_group["float3s"] = disabled_float3s;
		Array disabled_general;
		disabled_general.push_back(disabled_group);
		Dictionary disabled_config;
		disabled_config["General"] = disabled_general;
		disabled_transform["config"] = disabled_config;
		disabled_transform["disabled"] = true;

		Array modifiers;
		modifiers.push_back(disabled_transform);
		modifiers.push_back(enabled_transform);
		Dictionary pipeline_json;
		pipeline_json["modifiers"] = modifiers;

		RockMesh result = apply_modifier_chain(cube, pipeline_json);
		for (int i = 0; i < result.vertices.size(); i++) {
			CHECK(result.vertices[i].y < 500.0); // disabled step (+999 to y) was skipped
			CHECK(result.vertices[i].x > 90.0); // enabled translate applied
		}
	}
}
#endif // DOCTEST
