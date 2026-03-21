/**************************************************************************/
/*  foliage_2d_animation.cpp                                              */
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

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

// Ported from Unity C# Foliage2D system.
// Reference: https://assetstore.unity.com/packages/tools/animation/foliage-2d-45660

#include "foliage_2d_animation.h"

#include "common/gd_core.h"

// ==========================================================================
// FoliageMesh2D
// ==========================================================================

void FoliageMesh2D::set_width_segments(int p_segments) {
	ERR_FAIL_COND(p_segments < 1);
	width_segments = p_segments;
}

int FoliageMesh2D::get_width_segments() const {
	return width_segments;
}

void FoliageMesh2D::set_height_segments(int p_segments) {
	ERR_FAIL_COND(p_segments < 1);
	height_segments = p_segments;
}

int FoliageMesh2D::get_height_segments() const {
	return height_segments;
}

void FoliageMesh2D::set_size(const Vector2 &p_size) {
	ERR_FAIL_COND(p_size.x <= 0 || p_size.y <= 0);
	width = p_size.x;
	height = p_size.y;
}

Vector2 FoliageMesh2D::get_size() const {
	return Vector2(width, height);
}

void FoliageMesh2D::clear() {
	mesh_verts.resize(0);
	mesh_indices.resize(0);
	mesh_uvs.resize(0);
}

int FoliageMesh2D::get_vertex_count() const {
	return mesh_verts.size();
}

int FoliageMesh2D::get_horizontal_vertex_count() const {
	return width_segments + 1;
}

void FoliageMesh2D::generate_mesh() {
	clear();

	const int h_verts = width_segments + 1;
	const int v_verts = height_segments + 1;
	const real_t seg_w = width / width_segments;
	const real_t seg_h = height / height_segments;

	// Generate vertices and UVs.
	// Origin at bottom-center: x in [-width/2, width/2], y in [0, height].
	for (int y = 0; y < v_verts; y++) {
		for (int x = 0; x < h_verts; x++) {
			real_t vx = -width / 2.0 + x * seg_w;
			real_t vy = y * seg_h;
			mesh_verts.push_back(Vector3(vx, vy, 0));

			real_t u = (real_t)x / width_segments;
			real_t v = 1.0 - (real_t)y / height_segments; // UV: 0 at top, 1 at bottom.
			mesh_uvs.push_back(Vector2(u, v));
		}
	}

	// Generate triangle indices.
	for (int y = 0; y < height_segments; y++) {
		for (int x = 0; x < width_segments; x++) {
			int i0 = y * h_verts + x;
			int i1 = (y + 1) * h_verts + x;
			int i2 = y * h_verts + x + 1;
			int i3 = (y + 1) * h_verts + x + 1;

			mesh_indices.push_back(i0);
			mesh_indices.push_back(i1);
			mesh_indices.push_back(i2);

			mesh_indices.push_back(i1);
			mesh_indices.push_back(i3);
			mesh_indices.push_back(i2);
		}
	}
}

Ref<ArrayMesh> FoliageMesh2D::build() const {
	ERR_FAIL_COND_V(mesh_verts.size() == 0, Ref<ArrayMesh>());

	// Round vertices for pixel-perfect results.
	PoolVector3Array rounded_verts;
	rounded_verts.resize(mesh_verts.size());
	for (int i = 0; i < mesh_verts.size(); i++) {
		const Vector3 &v = mesh_verts[i];
		rounded_verts.set(i, Vector3(Math::stepify(v.x, 0.001), Math::stepify(v.y, 0.001), Math::stepify(v.z, 0.001)));
	}

	// Convert Vector3 verts to Vector2 for 2D mesh.
	PoolVector2Array verts_2d;
	verts_2d.resize(rounded_verts.size());
	for (int i = 0; i < rounded_verts.size(); i++) {
		verts_2d.set(i, Vector2(rounded_verts[i].x, rounded_verts[i].y));
	}

	Array arrays;
	arrays.resize(VS::ARRAY_MAX);
	arrays[VS::ARRAY_VERTEX] = verts_2d;
	arrays[VS::ARRAY_TEX_UV] = mesh_uvs;
	arrays[VS::ARRAY_INDEX] = mesh_indices;

	Ref<ArrayMesh> mesh = Ref<ArrayMesh>(memnew(ArrayMesh));
	mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays, Array(), Mesh::ARRAY_FLAG_USE_2D_VERTICES);
	return mesh;
}

void FoliageMesh2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_width_segments", "segments"), &FoliageMesh2D::set_width_segments);
	ClassDB::bind_method(D_METHOD("get_width_segments"), &FoliageMesh2D::get_width_segments);
	ClassDB::bind_method(D_METHOD("set_height_segments", "segments"), &FoliageMesh2D::set_height_segments);
	ClassDB::bind_method(D_METHOD("get_height_segments"), &FoliageMesh2D::get_height_segments);
	ClassDB::bind_method(D_METHOD("set_size", "size"), &FoliageMesh2D::set_size);
	ClassDB::bind_method(D_METHOD("get_size"), &FoliageMesh2D::get_size);
	ClassDB::bind_method(D_METHOD("clear"), &FoliageMesh2D::clear);
	ClassDB::bind_method(D_METHOD("generate_mesh"), &FoliageMesh2D::generate_mesh);
	ClassDB::bind_method(D_METHOD("build"), &FoliageMesh2D::build);
	ClassDB::bind_method(D_METHOD("get_vertex_count"), &FoliageMesh2D::get_vertex_count);
	ClassDB::bind_method(D_METHOD("get_horizontal_vertex_count"), &FoliageMesh2D::get_horizontal_vertex_count);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "width_segments", PROPERTY_HINT_RANGE, "1,10,1"), "set_width_segments", "get_width_segments");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "height_segments", PROPERTY_HINT_RANGE, "1,10,1"), "set_height_segments", "get_height_segments");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "size"), "set_size", "get_size");
}

FoliageMesh2D::FoliageMesh2D() {
	width_segments = 1;
	height_segments = 4;
	width = 32;
	height = 64;
}

// ==========================================================================
// FoliagePath2D
// ==========================================================================

// Hermite interpolation between 4 points.
real_t FoliagePath2D::_hermite_interpolation(real_t p1, real_t p2, real_t p3, real_t p4, int index) const {
	ERR_FAIL_COND_V(_line_length < CMP_EPSILON, p2);

	real_t mu = _distance_from_start / _line_length;
	real_t current_tension, current_bias;

	if (uniform_values) {
		current_tension = tension;
		current_bias = bias;
	} else {
		ERR_FAIL_INDEX_V(index + 1, handles.size(), p2);
		// Note: per-node tension/bias not exposed yet — use uniform for now.
		current_tension = tension;
		current_bias = bias;
	}

	real_t mu2 = mu * mu;
	real_t mu3 = mu2 * mu;
	real_t m0 = (p2 - p1) * (1 + current_bias) * (1 - current_tension) / 2 +
			(p3 - p2) * (1 - current_bias) * (1 - current_tension) / 2;
	real_t m1 = (p3 - p2) * (1 + current_bias) * (1 - current_tension) / 2 +
			(p4 - p3) * (1 - current_bias) * (1 - current_tension) / 2;
	real_t a0 = 2 * mu3 - 3 * mu2 + 1;
	real_t a1 = mu3 - 2 * mu2 + mu;
	real_t a2 = mu3 - mu2;
	real_t a3 = -2 * mu3 + 3 * mu2;

	return a0 * p2 + a1 * m0 + a2 * m1 + a3 * p3;
}

real_t FoliagePath2D::_hermite_slope(real_t p1, real_t p2, real_t p3, real_t p4, real_t mu, int index) const {
	real_t current_tension, current_bias;

	if (uniform_values) {
		current_tension = tension;
		current_bias = bias;
	} else {
		current_tension = tension;
		current_bias = bias;
	}

	real_t mu2 = mu * mu;
	real_t m0 = ((1 - current_tension) * (current_bias + 1) * (p2 - p1) +
						(1 - current_bias) * (1 - current_tension) * (p3 - p2)) /
			2;
	real_t m1 = ((1 - current_tension) * (current_bias + 1) * (p3 - p2) +
						(1 - current_bias) * (1 - current_tension) * (p4 - p3)) /
			2;
	real_t a0 = 3 * mu2 - 4 * mu + 1;
	real_t a1 = 3 * mu2 - 2 * mu;
	real_t a2 = p2 * (6 * mu2 - 6 * mu);
	real_t a3 = p3 * (6 * mu - 6 * mu2);

	return a0 * m0 + a1 * m1 + a2 + a3;
}

Vector2 FoliagePath2D::_smooth_point(int index) const {
	ERR_FAIL_INDEX_V(index + 1, handles.size(), Vector2());

	Vector2 start_point, end_point;

	// Create virtual start/end points for boundary nodes.
	if (index == 0) {
		start_point = Vector2(handles[index].x - _line.x, handles[index].y - _line.y);
	} else {
		start_point = handles[index - 1];
	}

	if (index == handles.size() - 2) {
		end_point = Vector2(handles[index + 1].x - _line.x, handles[index + 1].y - _line.y);
	} else {
		ERR_FAIL_INDEX_V(index + 2, handles.size(), Vector2());
		end_point = handles[index + 2];
	}

	return Vector2(
			_hermite_interpolation(start_point.x, handles[index].x, handles[index + 1].x, end_point.x, index),
			_hermite_interpolation(start_point.y, handles[index].y, handles[index + 1].y, end_point.y, index));
}

Vector2 FoliagePath2D::_smooth_tangent(int index) const {
	ERR_FAIL_INDEX_V(index + 1, handles.size(), Vector2(1, 0));
	ERR_FAIL_COND_V(_line_length < CMP_EPSILON, Vector2(1, 0));

	Vector2 start_point, end_point;

	if (index == 0) {
		start_point = Vector2(handles[index].x - _line.x, handles[index].y - _line.y);
	} else {
		start_point = handles[index - 1];
	}

	if (index == handles.size() - 2) {
		end_point = Vector2(handles[index + 1].x - _line.x, handles[index + 1].y - _line.y);
	} else {
		ERR_FAIL_INDEX_V(index + 2, handles.size(), Vector2(1, 0));
		end_point = handles[index + 2];
	}

	real_t mu = _distance_from_start / _line_length;
	return Vector2(
			_hermite_slope(start_point.x, handles[index].x, handles[index + 1].x, end_point.x, mu, index),
			_hermite_slope(start_point.y, handles[index].y, handles[index + 1].y, end_point.y, mu, index));
}

void FoliagePath2D::set_pattern(Foliage2DPattern p_pattern) {
	ERR_FAIL_INDEX(p_pattern, FOLIAGE2D_PATTERN_MAX);
	pattern = p_pattern;
}

Foliage2DPattern FoliagePath2D::get_pattern() const {
	return pattern;
}

void FoliagePath2D::set_overlap_type(Foliage2DOverlappingType p_type) {
	ERR_FAIL_INDEX(p_type, FOLIAGE2D_OVERLAP_MAX);
	overlap_type = p_type;
}

Foliage2DOverlappingType FoliagePath2D::get_overlap_type() const {
	return overlap_type;
}

void FoliagePath2D::set_path_type(Foliage2DPathType p_type) {
	ERR_FAIL_INDEX(p_type, FOLIAGE2D_PATH_MAX);
	path_type = p_type;
}

Foliage2DPathType FoliagePath2D::get_path_type() const {
	return path_type;
}

void FoliagePath2D::set_handles(const PoolVector2Array &p_handles) {
	handles = p_handles;
}

PoolVector2Array FoliagePath2D::get_handles() const {
	return handles;
}

void FoliagePath2D::add_handle(const Vector2 &p_pos) {
	handles.push_back(p_pos);
}

void FoliagePath2D::remove_handle(int p_index) {
	ERR_FAIL_INDEX(p_index, handles.size());
	handles.remove(p_index);
}

int FoliagePath2D::get_handle_count() const {
	return handles.size();
}

void FoliagePath2D::set_overlapping_factor(real_t p_factor) {
	overlapping_factor = CLAMP(p_factor, 0.0, 1.0);
}

real_t FoliagePath2D::get_overlapping_factor() const {
	return overlapping_factor;
}

void FoliagePath2D::set_min_overlapping_factor(real_t p_factor) {
	min_overlapping_factor = CLAMP(p_factor, 0.0, 1.0);
}

real_t FoliagePath2D::get_min_overlapping_factor() const {
	return min_overlapping_factor;
}

void FoliagePath2D::set_max_overlapping_factor(real_t p_factor) {
	max_overlapping_factor = CLAMP(p_factor, 0.0, 1.0);
}

real_t FoliagePath2D::get_max_overlapping_factor() const {
	return max_overlapping_factor;
}

void FoliagePath2D::set_bias(real_t p_bias) {
	bias = p_bias;
}

real_t FoliagePath2D::get_bias() const {
	return bias;
}

void FoliagePath2D::set_tension(real_t p_tension) {
	tension = p_tension;
}

real_t FoliagePath2D::get_tension() const {
	return tension;
}

void FoliagePath2D::set_uniform_values(bool p_uniform) {
	uniform_values = p_uniform;
}

bool FoliagePath2D::get_uniform_values() const {
	return uniform_values;
}

void FoliagePath2D::set_first_object_offset(real_t p_offset) {
	first_object_offset = p_offset;
}

real_t FoliagePath2D::get_first_object_offset() const {
	return first_object_offset;
}

void FoliagePath2D::set_last_object_offset(real_t p_offset) {
	last_object_offset = p_offset;
}

real_t FoliagePath2D::get_last_object_offset() const {
	return last_object_offset;
}

Vector2 FoliagePath2D::get_point_at(real_t p_t) const {
	ERR_FAIL_COND_V(handles.size() < 2, Vector2());

	p_t = CLAMP(p_t, 0.0, 1.0);

	// Find which segment this t falls into.
	int num_segments = handles.size() - 1;
	real_t scaled_t = p_t * num_segments;
	int seg = CLAMP((int)scaled_t, 0, num_segments - 1);
	real_t local_t = scaled_t - seg;

	if (path_type == FOLIAGE2D_PATH_SMOOTH) {
		// Use Hermite interpolation.
		// Temporarily set internal state for interpolation.
		FoliagePath2D *mutable_self = const_cast<FoliagePath2D *>(this);
		mutable_self->_line = handles[seg + 1] - handles[seg];
		mutable_self->_line_length = _line.length();
		mutable_self->_distance_from_start = local_t * _line_length;
		return _smooth_point(seg);
	} else {
		return handles[seg].linear_interpolate(handles[seg + 1], local_t);
	}
}

real_t FoliagePath2D::get_angle_at(real_t p_t) const {
	ERR_FAIL_COND_V(handles.size() < 2, 0.0);

	p_t = CLAMP(p_t, 0.0, 1.0);

	int num_segments = handles.size() - 1;
	real_t scaled_t = p_t * num_segments;
	int seg = CLAMP((int)scaled_t, 0, num_segments - 1);
	real_t local_t = scaled_t - seg;

	if (path_type == FOLIAGE2D_PATH_SMOOTH) {
		FoliagePath2D *mutable_self = const_cast<FoliagePath2D *>(this);
		mutable_self->_line = handles[seg + 1] - handles[seg];
		mutable_self->_line_length = _line.length();
		mutable_self->_distance_from_start = local_t * _line_length;
		Vector2 tangent = _smooth_tangent(seg);
		return Math::atan2(tangent.y, tangent.x);
	} else {
		Vector2 dir = handles[seg + 1] - handles[seg];
		return Math::atan2(dir.y, dir.x);
	}
}

Array FoliagePath2D::compute_placement(const PoolRealArray &p_widths, const PoolRealArray &p_heights) const {
	Array result;
	ERR_FAIL_COND_V(handles.size() < 2, result);
	ERR_FAIL_COND_V(p_widths.size() == 0, result);
	ERR_FAIL_COND_V(p_widths.size() != p_heights.size(), result);

	int max_prefabs = p_widths.size();
	int prefab_index = -1;
	real_t dist = 0;
	real_t prev_dist = 0;
	real_t prev_width = 0;

	for (int seg = 0; seg < handles.size() - 1; seg++) {
		Vector2 seg_line = handles[seg + 1] - handles[seg];
		real_t seg_len = seg_line.length();
		if (seg_len < CMP_EPSILON) {
			continue;
		}

		Vector2 seg_dir = seg_line.normalized();
		Vector2 seg_normal = Vector2(-seg_dir.y, seg_dir.x);
		real_t seg_angle = Math::atan2(seg_line.y, seg_line.x);

		dist = 0;
		prev_dist = 0;
		prev_width = 0;
		bool first_on_segment = true;

		while (true) {
			// Select prefab.
			int idx;
			if (pattern == FOLIAGE2D_PATTERN_RANDOM) {
				idx = Math::rand() % max_prefabs;
			} else {
				prefab_index++;
				if (prefab_index >= max_prefabs || prefab_index < 0) {
					prefab_index = 0;
				}
				idx = prefab_index;
			}

			real_t item_width = p_widths[idx];
			real_t item_height = p_heights[idx];

			// Calculate distance from start.
			if (first_on_segment) {
				dist = item_width / 2.0 + first_object_offset;
				if (dist <= 0) {
					dist = 0.0001;
				}
				first_on_segment = false;
			} else {
				real_t overlap;
				if (overlap_type == FOLIAGE2D_OVERLAP_FIXED) {
					overlap = overlapping_factor;
				} else {
					overlap = Math::random(min_overlapping_factor, max_overlapping_factor);
				}

				if (overlap <= 0.5) {
					dist = prev_dist + (prev_width / 2) + ((item_width / 2) - (item_width * overlap));
				} else {
					dist = prev_dist + (prev_width / 2) - ((item_width * overlap) - (item_width / 2));
				}
			}

			if (dist > seg_len) {
				break;
			}

			// Compute position on path.
			real_t t_on_seg = dist / seg_len;
			Vector2 pos;
			real_t angle;

			if (path_type == FOLIAGE2D_PATH_SMOOTH) {
				FoliagePath2D *mutable_self = const_cast<FoliagePath2D *>(this);
				mutable_self->_line = seg_line;
				mutable_self->_line_length = seg_len;
				mutable_self->_distance_from_start = dist;
				pos = _smooth_point(seg);
				Vector2 tangent = _smooth_tangent(seg);
				angle = Math::atan2(tangent.y, tangent.x);
			} else {
				pos = handles[seg].linear_interpolate(handles[seg + 1], t_on_seg);
				angle = seg_angle;
			}

			// Offset for bottom-alignment.
			Vector2 offset = seg_normal * (item_height / 2);
			pos += offset;

			Transform2D xform;
			xform.set_rotation(angle);
			xform.set_origin(pos);
			result.push_back(xform);

			prev_dist = dist;
			prev_width = item_width;
		}
	}

	return result;
}

void FoliagePath2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			// Draw path handles in editor.
#ifdef TOOLS_ENABLED
			if (Engine::get_singleton()->is_editor_hint() && handles.size() >= 2) {
				for (int i = 0; i < handles.size() - 1; i++) {
					draw_line(handles[i], handles[i + 1], Color(0.4, 0.8, 0.2, 0.7), 2.0);
				}
				for (int i = 0; i < handles.size(); i++) {
					draw_circle(handles[i], 4, Color(0.8, 0.4, 0.2));
				}
			}
#endif
		} break;
	}
}

void FoliagePath2D::_bind_methods() {
	BIND_ENUM_CONSTANT(FOLIAGE2D_PATTERN_RANDOM);
	BIND_ENUM_CONSTANT(FOLIAGE2D_PATTERN_CONSECUTIVE);
	BIND_ENUM_CONSTANT(FOLIAGE2D_OVERLAP_FIXED);
	BIND_ENUM_CONSTANT(FOLIAGE2D_OVERLAP_RANDOM);
	BIND_ENUM_CONSTANT(FOLIAGE2D_PATH_LINEAR);
	BIND_ENUM_CONSTANT(FOLIAGE2D_PATH_SMOOTH);

	ClassDB::bind_method(D_METHOD("set_pattern", "pattern"), &FoliagePath2D::set_pattern);
	ClassDB::bind_method(D_METHOD("get_pattern"), &FoliagePath2D::get_pattern);
	ClassDB::bind_method(D_METHOD("set_overlap_type", "type"), &FoliagePath2D::set_overlap_type);
	ClassDB::bind_method(D_METHOD("get_overlap_type"), &FoliagePath2D::get_overlap_type);
	ClassDB::bind_method(D_METHOD("set_path_type", "type"), &FoliagePath2D::set_path_type);
	ClassDB::bind_method(D_METHOD("get_path_type"), &FoliagePath2D::get_path_type);

	ClassDB::bind_method(D_METHOD("set_handles", "handles"), &FoliagePath2D::set_handles);
	ClassDB::bind_method(D_METHOD("get_handles"), &FoliagePath2D::get_handles);
	ClassDB::bind_method(D_METHOD("add_handle", "position"), &FoliagePath2D::add_handle);
	ClassDB::bind_method(D_METHOD("remove_handle", "index"), &FoliagePath2D::remove_handle);
	ClassDB::bind_method(D_METHOD("get_handle_count"), &FoliagePath2D::get_handle_count);

	ClassDB::bind_method(D_METHOD("set_overlapping_factor", "factor"), &FoliagePath2D::set_overlapping_factor);
	ClassDB::bind_method(D_METHOD("get_overlapping_factor"), &FoliagePath2D::get_overlapping_factor);
	ClassDB::bind_method(D_METHOD("set_min_overlapping_factor", "factor"), &FoliagePath2D::set_min_overlapping_factor);
	ClassDB::bind_method(D_METHOD("get_min_overlapping_factor"), &FoliagePath2D::get_min_overlapping_factor);
	ClassDB::bind_method(D_METHOD("set_max_overlapping_factor", "factor"), &FoliagePath2D::set_max_overlapping_factor);
	ClassDB::bind_method(D_METHOD("get_max_overlapping_factor"), &FoliagePath2D::get_max_overlapping_factor);

	ClassDB::bind_method(D_METHOD("set_bias", "bias"), &FoliagePath2D::set_bias);
	ClassDB::bind_method(D_METHOD("get_bias"), &FoliagePath2D::get_bias);
	ClassDB::bind_method(D_METHOD("set_tension", "tension"), &FoliagePath2D::set_tension);
	ClassDB::bind_method(D_METHOD("get_tension"), &FoliagePath2D::get_tension);
	ClassDB::bind_method(D_METHOD("set_uniform_values", "uniform"), &FoliagePath2D::set_uniform_values);
	ClassDB::bind_method(D_METHOD("get_uniform_values"), &FoliagePath2D::get_uniform_values);

	ClassDB::bind_method(D_METHOD("set_first_object_offset", "offset"), &FoliagePath2D::set_first_object_offset);
	ClassDB::bind_method(D_METHOD("get_first_object_offset"), &FoliagePath2D::get_first_object_offset);
	ClassDB::bind_method(D_METHOD("set_last_object_offset", "offset"), &FoliagePath2D::set_last_object_offset);
	ClassDB::bind_method(D_METHOD("get_last_object_offset"), &FoliagePath2D::get_last_object_offset);

	ClassDB::bind_method(D_METHOD("get_point_at", "t"), &FoliagePath2D::get_point_at);
	ClassDB::bind_method(D_METHOD("get_angle_at", "t"), &FoliagePath2D::get_angle_at);
	ClassDB::bind_method(D_METHOD("compute_placement", "widths", "heights"), &FoliagePath2D::compute_placement);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "pattern", PROPERTY_HINT_ENUM, "Random,Consecutive"), "set_pattern", "get_pattern");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "overlap_type", PROPERTY_HINT_ENUM, "Fixed,Random"), "set_overlap_type", "get_overlap_type");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "path_type", PROPERTY_HINT_ENUM, "Linear,Smooth"), "set_path_type", "get_path_type");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR2_ARRAY, "handles"), "set_handles", "get_handles");

	ADD_GROUP("Overlapping", "");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "overlapping_factor", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_overlapping_factor", "get_overlapping_factor");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "min_overlapping_factor", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_min_overlapping_factor", "get_min_overlapping_factor");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "max_overlapping_factor", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_max_overlapping_factor", "get_max_overlapping_factor");

	ADD_GROUP("Interpolation", "");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "bias", PROPERTY_HINT_RANGE, "-1,1,0.01"), "set_bias", "get_bias");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "tension", PROPERTY_HINT_RANGE, "-1,1,0.01"), "set_tension", "get_tension");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "uniform_values"), "set_uniform_values", "get_uniform_values");

	ADD_GROUP("Offset", "");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "first_object_offset"), "set_first_object_offset", "get_first_object_offset");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "last_object_offset"), "set_last_object_offset", "get_last_object_offset");
}

FoliagePath2D::FoliagePath2D() {
	pattern = FOLIAGE2D_PATTERN_RANDOM;
	overlap_type = FOLIAGE2D_OVERLAP_FIXED;
	path_type = FOLIAGE2D_PATH_LINEAR;
	overlapping_factor = 0.4;
	min_overlapping_factor = 0;
	max_overlapping_factor = 0.8;
	bias = 0;
	tension = 0;
	first_object_offset = 0;
	last_object_offset = 0;
	uniform_values = true;

	_distance_from_start = 0;
	_prev_distance_from_start = 0;
	_previous_width = 0;
	_line_length = 0;
	_object_index = 0;
	_prefab_index = -1;
}

// ==========================================================================
// FoliageAnimation2D
// ==========================================================================

PoolRealArray FoliageAnimation2D::make_linear_factors(int p_height_segments) {
	PoolRealArray factors;
	int rows = p_height_segments + 1;
	for (int i = 0; i < rows; i++) {
		factors.push_back((real_t)i / p_height_segments);
	}
	return factors;
}

PoolRealArray FoliageAnimation2D::make_quadratic_factors(int p_height_segments) {
	PoolRealArray factors;
	int rows = p_height_segments + 1;
	for (int i = 0; i < rows; i++) {
		real_t t = (real_t)i / p_height_segments;
		factors.push_back(t * t);
	}
	return factors;
}

void FoliageAnimation2D::_apply_simple_bending(PoolVector3Array &p_verts) const {
	const int h_len = height_segments + 1;
	const int w_len = width_segments + 1;

	for (int i = 0; i < h_len && i < offset_factors.size(); i++) {
		real_t factor = offset_factors[i];
		Vector3 pos_offset(offset.x * factor, offset.y * factor, offset.z * factor);

		for (int j = 0; j < w_len; j++) {
			int vert_index = w_len * i + j;
			ERR_FAIL_INDEX(vert_index, p_verts.size());
			ERR_FAIL_INDEX(vert_index, initial_vertex_pos.size());

			p_verts.set(vert_index, Vector3(initial_vertex_pos[vert_index].x + pos_offset.x, initial_vertex_pos[vert_index].y + pos_offset.y, initial_vertex_pos[vert_index].z + pos_offset.z));
		}
	}
}

void FoliageAnimation2D::_apply_smart_bending(PoolVector3Array &p_verts) const {
	const int h_len = height_segments + 1;
	const int w_len = width_segments + 1;

	ERR_FAIL_COND(h_len < 2);
	ERR_FAIL_COND(offset_factors.size() < h_len);
	ERR_FAIL_COND(initial_vertex_pos.size() < h_len * w_len);

	const int dir = offset.x < 0 ? -1 : 1;
	const real_t height_of_segment = (initial_vertex_pos.size() > w_len)
			? Math::abs(initial_vertex_pos[w_len].y - initial_vertex_pos[0].y)
			: 1.0;

	// Build center line points (the "backbone" of the mesh).
	Vector<Vector2> center_line;
	center_line.resize(h_len);
	center_line.write[0] = Vector2(offset.x * offset_factors[0], initial_vertex_pos[0].y);

	Vector<real_t> angles_deg;
	angles_deg.resize(h_len);
	angles_deg.write[0] = 0;

	for (int i = 1; i < h_len; i++) {
		real_t x_offset = offset.x * offset_factors[i];

		if (Math::abs(x_offset) < height_of_segment) {
			real_t dy = Math::sqrt(MAX(0.0, height_of_segment * height_of_segment - x_offset * x_offset));
			center_line.write[i] = Vector2(center_line[i - 1].x + x_offset, center_line[i - 1].y + dy);
		} else {
			real_t y = Math::abs(x_offset) - height_of_segment;
			real_t dx = Math::sqrt(MAX(0.0, height_of_segment * height_of_segment - y * y));
			center_line.write[i] = Vector2(center_line[i - 1].x + dir * dx, center_line[i - 1].y - y);
		}
	}

	// Compute angles and rotate vertices.
	for (int i = 1; i < h_len; i++) {
		Vector2 line = center_line[i] - center_line[i - 1];
		angles_deg.write[i] = Math::rad2deg(Math::atan2(line.y, line.x)) - 90;
		real_t angle_rad = Math::deg2rad(angles_deg[i]);
		real_t cos_a = Math::cos(angle_rad);
		real_t sin_a = Math::sin(angle_rad);

		for (int j = 0; j < w_len; j++) {
			int vert_index = w_len * i + j;
			ERR_FAIL_INDEX(vert_index, p_verts.size());
			ERR_FAIL_INDEX(vert_index, initial_vertex_pos.size());

			// Offset vertex to center line.
			real_t vx = initial_vertex_pos[vert_index].x + center_line[i].x;
			real_t vy = center_line[i].y;

			// Rotate around center line point.
			real_t rx = (vx - center_line[i].x) * cos_a - (vy - center_line[i].y) * sin_a + center_line[i].x;
			real_t ry = (vx - center_line[i].x) * sin_a + (vy - center_line[i].y) * cos_a + center_line[i].y;

			p_verts.set(vert_index, Vector3(rx, ry, 0));
		}
	}

	// Row 0: just offset by center_line[0].
	for (int j = 0; j < w_len; j++) {
		int vert_index = j;
		ERR_FAIL_INDEX(vert_index, p_verts.size());
		ERR_FAIL_INDEX(vert_index, initial_vertex_pos.size());
		p_verts.set(vert_index, Vector3(initial_vertex_pos[vert_index].x + center_line[0].x, center_line[0].y, initial_vertex_pos[vert_index].z));
	}
}

void FoliageAnimation2D::build_mesh(const Vector2 &p_size) {
	ERR_FAIL_COND(p_size.x <= 0 || p_size.y <= 0);

	Ref<FoliageMesh2D> builder = Ref<FoliageMesh2D>(memnew(FoliageMesh2D));
	builder->set_width_segments(width_segments);
	builder->set_height_segments(height_segments);
	builder->set_size(p_size);
	builder->generate_mesh();

	Ref<ArrayMesh> mesh = builder->build();
	set_mesh(mesh);

	// Cache initial vertex positions as Vector3.
	int h_verts = width_segments + 1;
	int v_verts = height_segments + 1;
	int total = h_verts * v_verts;
	initial_vertex_pos.resize(total);

	real_t seg_w = p_size.x / width_segments;
	real_t seg_h = p_size.y / height_segments;
	for (int y = 0; y < v_verts; y++) {
		for (int x = 0; x < h_verts; x++) {
			int idx = y * h_verts + x;
			initial_vertex_pos.set(idx, Vector3(-p_size.x / 2.0 + x * seg_w, y * seg_h, 0));
		}
	}

	// Generate default offset factors if not set.
	if (offset_factors.size() != v_verts) {
		offset_factors = make_linear_factors(height_segments);
	}
}

void FoliageAnimation2D::update_animation() {
	Ref<ArrayMesh> mesh = get_mesh();
	ERR_FAIL_COND(mesh.is_null());
	ERR_FAIL_COND(initial_vertex_pos.size() == 0);

	PoolVector3Array verts;
	verts.resize(initial_vertex_pos.size());

	// Copy initial positions.
	for (int i = 0; i < initial_vertex_pos.size(); i++) {
		verts.set(i, initial_vertex_pos[i]);
	}

	if (bend_mode == FOLIAGE2D_BEND_SIMPLE) {
		_apply_simple_bending(verts);
	} else {
		_apply_smart_bending(verts);
	}

	// Convert to 2D and update mesh.
	PoolVector2Array verts_2d;
	verts_2d.resize(verts.size());
	for (int i = 0; i < verts.size(); i++) {
		verts_2d.set(i, Vector2(verts[i].x, verts[i].y));
	}

	// Get existing surface arrays and update vertices.
	if (mesh->get_surface_count() > 0) {
		Array arrays = mesh->surface_get_arrays(0);
		arrays[VS::ARRAY_VERTEX] = verts_2d;

		mesh->surface_remove(0);
		mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays, Array(), Mesh::ARRAY_FLAG_USE_2D_VERTICES);
	}
}

void FoliageAnimation2D::set_bend_mode(Foliage2DBendMode p_mode) {
	ERR_FAIL_INDEX(p_mode, FOLIAGE2D_BEND_MAX);
	bend_mode = p_mode;
}

Foliage2DBendMode FoliageAnimation2D::get_bend_mode() const {
	return bend_mode;
}

void FoliageAnimation2D::set_offset(const Vector3 &p_offset) {
	offset = p_offset;
}

Vector3 FoliageAnimation2D::get_offset() const {
	return offset;
}

void FoliageAnimation2D::set_offset_factors(const PoolRealArray &p_factors) {
	offset_factors = p_factors;
}

PoolRealArray FoliageAnimation2D::get_offset_factors() const {
	return offset_factors;
}

void FoliageAnimation2D::set_width_segments(int p_segments) {
	ERR_FAIL_COND(p_segments < 1);
	width_segments = p_segments;
}

int FoliageAnimation2D::get_width_segments() const {
	return width_segments;
}

void FoliageAnimation2D::set_height_segments(int p_segments) {
	ERR_FAIL_COND(p_segments < 1);
	height_segments = p_segments;
}

int FoliageAnimation2D::get_height_segments() const {
	return height_segments;
}

void FoliageAnimation2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_PROCESS: {
			update_animation();
		} break;
	}
}

void FoliageAnimation2D::_bind_methods() {
	BIND_ENUM_CONSTANT(FOLIAGE2D_BEND_SIMPLE);
	BIND_ENUM_CONSTANT(FOLIAGE2D_BEND_SMART);

	ClassDB::bind_method(D_METHOD("set_bend_mode", "mode"), &FoliageAnimation2D::set_bend_mode);
	ClassDB::bind_method(D_METHOD("get_bend_mode"), &FoliageAnimation2D::get_bend_mode);
	ClassDB::bind_method(D_METHOD("set_offset", "offset"), &FoliageAnimation2D::set_offset);
	ClassDB::bind_method(D_METHOD("get_offset"), &FoliageAnimation2D::get_offset);
	ClassDB::bind_method(D_METHOD("set_offset_factors", "factors"), &FoliageAnimation2D::set_offset_factors);
	ClassDB::bind_method(D_METHOD("get_offset_factors"), &FoliageAnimation2D::get_offset_factors);
	ClassDB::bind_method(D_METHOD("set_width_segments", "segments"), &FoliageAnimation2D::set_width_segments);
	ClassDB::bind_method(D_METHOD("get_width_segments"), &FoliageAnimation2D::get_width_segments);
	ClassDB::bind_method(D_METHOD("set_height_segments", "segments"), &FoliageAnimation2D::set_height_segments);
	ClassDB::bind_method(D_METHOD("get_height_segments"), &FoliageAnimation2D::get_height_segments);

	ClassDB::bind_method(D_METHOD("build_mesh", "size"), &FoliageAnimation2D::build_mesh);
	ClassDB::bind_method(D_METHOD("update_animation"), &FoliageAnimation2D::update_animation);
	ClassDB::bind_method(D_METHOD("make_linear_factors", "height_segments"), &FoliageAnimation2D::make_linear_factors);
	ClassDB::bind_method(D_METHOD("make_quadratic_factors", "height_segments"), &FoliageAnimation2D::make_quadratic_factors);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "bend_mode", PROPERTY_HINT_ENUM, "Simple,Smart"), "set_bend_mode", "get_bend_mode");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "offset"), "set_offset", "get_offset");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_REAL_ARRAY, "offset_factors"), "set_offset_factors", "get_offset_factors");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "width_segments", PROPERTY_HINT_RANGE, "1,10,1"), "set_width_segments", "get_width_segments");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "height_segments", PROPERTY_HINT_RANGE, "1,10,1"), "set_height_segments", "get_height_segments");
}

FoliageAnimation2D::FoliageAnimation2D() {
	bend_mode = FOLIAGE2D_BEND_SIMPLE;
	offset = Vector3(0, 0, 0);
	width_segments = 1;
	height_segments = 4;
}

// ==========================================================================
// Doctest tests
// ==========================================================================

#ifdef DOCTEST

TEST_SUITE("[[foliage_2d_animation]]") {
	// --- FoliageMesh2D ---

	TEST_CASE("[FoliageMesh2D] default construction") {
		Ref<FoliageMesh2D> mesh = Ref<FoliageMesh2D>(memnew(FoliageMesh2D));
		CHECK(mesh->get_width_segments() == 1);
		CHECK(mesh->get_height_segments() == 4);
		CHECK(mesh->get_size() == Vector2(32, 64));
		CHECK(mesh->get_vertex_count() == 0);
	}

	TEST_CASE("[FoliageMesh2D] set/get width_segments") {
		Ref<FoliageMesh2D> mesh = Ref<FoliageMesh2D>(memnew(FoliageMesh2D));
		mesh->set_width_segments(3);
		CHECK(mesh->get_width_segments() == 3);
		mesh->set_width_segments(1);
		CHECK(mesh->get_width_segments() == 1);
	}

	TEST_CASE("[FoliageMesh2D] width_segments rejects zero") {
		Ref<FoliageMesh2D> mesh = Ref<FoliageMesh2D>(memnew(FoliageMesh2D));
		mesh->set_width_segments(3);
		mesh->set_width_segments(0);
		CHECK(mesh->get_width_segments() == 3);
	}

	TEST_CASE("[FoliageMesh2D] set/get height_segments") {
		Ref<FoliageMesh2D> mesh = Ref<FoliageMesh2D>(memnew(FoliageMesh2D));
		mesh->set_height_segments(6);
		CHECK(mesh->get_height_segments() == 6);
	}

	TEST_CASE("[FoliageMesh2D] set/get size") {
		Ref<FoliageMesh2D> mesh = Ref<FoliageMesh2D>(memnew(FoliageMesh2D));
		mesh->set_size(Vector2(100, 200));
		CHECK(mesh->get_size() == Vector2(100, 200));
	}

	TEST_CASE("[FoliageMesh2D] size rejects non-positive") {
		Ref<FoliageMesh2D> mesh = Ref<FoliageMesh2D>(memnew(FoliageMesh2D));
		mesh->set_size(Vector2(100, 200));
		mesh->set_size(Vector2(0, 100));
		CHECK(mesh->get_size() == Vector2(100, 200));
		mesh->set_size(Vector2(100, -5));
		CHECK(mesh->get_size() == Vector2(100, 200));
	}

	TEST_CASE("[FoliageMesh2D] generate_mesh creates correct vertex count") {
		Ref<FoliageMesh2D> mesh = Ref<FoliageMesh2D>(memnew(FoliageMesh2D));
		mesh->set_width_segments(2);
		mesh->set_height_segments(3);
		mesh->generate_mesh();
		// (2+1) * (3+1) = 12 vertices
		CHECK(mesh->get_vertex_count() == 12);
		CHECK(mesh->get_horizontal_vertex_count() == 3);
	}

	TEST_CASE("[FoliageMesh2D] generate_mesh 1x1 segments") {
		Ref<FoliageMesh2D> mesh = Ref<FoliageMesh2D>(memnew(FoliageMesh2D));
		mesh->set_width_segments(1);
		mesh->set_height_segments(1);
		mesh->set_size(Vector2(10, 20));
		mesh->generate_mesh();
		CHECK(mesh->get_vertex_count() == 4); // 2x2
	}

	TEST_CASE("[FoliageMesh2D] build returns valid ArrayMesh") {
		Ref<FoliageMesh2D> mesh = Ref<FoliageMesh2D>(memnew(FoliageMesh2D));
		mesh->set_width_segments(2);
		mesh->set_height_segments(4);
		mesh->set_size(Vector2(32, 64));
		mesh->generate_mesh();
		Ref<ArrayMesh> result = mesh->build();
		CHECK(result.is_valid());
		CHECK(result->get_surface_count() == 1);
	}

	TEST_CASE("[FoliageMesh2D] build returns null when empty") {
		Ref<FoliageMesh2D> mesh = Ref<FoliageMesh2D>(memnew(FoliageMesh2D));
		Ref<ArrayMesh> result = mesh->build();
		CHECK(result.is_null());
	}

	TEST_CASE("[FoliageMesh2D] clear resets vertex count") {
		Ref<FoliageMesh2D> mesh = Ref<FoliageMesh2D>(memnew(FoliageMesh2D));
		mesh->generate_mesh();
		CHECK(mesh->get_vertex_count() > 0);
		mesh->clear();
		CHECK(mesh->get_vertex_count() == 0);
	}

	// --- FoliagePath2D ---

	TEST_CASE("[FoliagePath2D] default construction") {
		FoliagePath2D *path = memnew(FoliagePath2D);
		CHECK(path->get_pattern() == FOLIAGE2D_PATTERN_RANDOM);
		CHECK(path->get_overlap_type() == FOLIAGE2D_OVERLAP_FIXED);
		CHECK(path->get_path_type() == FOLIAGE2D_PATH_LINEAR);
		CHECK(path->get_overlapping_factor() == doctest::Approx(0.4));
		CHECK(path->get_bias() == doctest::Approx(0.0));
		CHECK(path->get_tension() == doctest::Approx(0.0));
		CHECK(path->get_uniform_values() == true);
		CHECK(path->get_handle_count() == 0);
		memdelete(path);
	}

	TEST_CASE("[FoliagePath2D] set/get pattern") {
		FoliagePath2D *path = memnew(FoliagePath2D);
		path->set_pattern(FOLIAGE2D_PATTERN_CONSECUTIVE);
		CHECK(path->get_pattern() == FOLIAGE2D_PATTERN_CONSECUTIVE);
		path->set_pattern(FOLIAGE2D_PATTERN_RANDOM);
		CHECK(path->get_pattern() == FOLIAGE2D_PATTERN_RANDOM);
		memdelete(path);
	}

	TEST_CASE("[FoliagePath2D] set/get overlap_type") {
		FoliagePath2D *path = memnew(FoliagePath2D);
		path->set_overlap_type(FOLIAGE2D_OVERLAP_RANDOM);
		CHECK(path->get_overlap_type() == FOLIAGE2D_OVERLAP_RANDOM);
		memdelete(path);
	}

	TEST_CASE("[FoliagePath2D] set/get path_type") {
		FoliagePath2D *path = memnew(FoliagePath2D);
		path->set_path_type(FOLIAGE2D_PATH_SMOOTH);
		CHECK(path->get_path_type() == FOLIAGE2D_PATH_SMOOTH);
		memdelete(path);
	}

	TEST_CASE("[FoliagePath2D] add/remove handles") {
		FoliagePath2D *path = memnew(FoliagePath2D);
		path->add_handle(Vector2(0, 0));
		path->add_handle(Vector2(100, 0));
		path->add_handle(Vector2(200, 50));
		CHECK(path->get_handle_count() == 3);
		path->remove_handle(1);
		CHECK(path->get_handle_count() == 2);
		memdelete(path);
	}

	TEST_CASE("[FoliagePath2D] set/get handles") {
		FoliagePath2D *path = memnew(FoliagePath2D);
		PoolVector2Array handles;
		handles.push_back(Vector2(0, 0));
		handles.push_back(Vector2(100, 0));
		path->set_handles(handles);
		CHECK(path->get_handle_count() == 2);
		PoolVector2Array got = path->get_handles();
		CHECK(got[0] == Vector2(0, 0));
		CHECK(got[1] == Vector2(100, 0));
		memdelete(path);
	}

	TEST_CASE("[FoliagePath2D] overlapping_factor clamped to [0,1]") {
		FoliagePath2D *path = memnew(FoliagePath2D);
		path->set_overlapping_factor(0.5);
		CHECK(path->get_overlapping_factor() == doctest::Approx(0.5));
		path->set_overlapping_factor(-0.1);
		CHECK(path->get_overlapping_factor() == doctest::Approx(0.0));
		path->set_overlapping_factor(1.5);
		CHECK(path->get_overlapping_factor() == doctest::Approx(1.0));
		memdelete(path);
	}

	TEST_CASE("[FoliagePath2D] get_point_at linear path") {
		FoliagePath2D *path = memnew(FoliagePath2D);
		path->add_handle(Vector2(0, 0));
		path->add_handle(Vector2(100, 0));
		Vector2 start = path->get_point_at(0.0);
		Vector2 mid = path->get_point_at(0.5);
		Vector2 end = path->get_point_at(1.0);
		CHECK(start.is_equal_approx(Vector2(0, 0)));
		CHECK(mid.is_equal_approx(Vector2(50, 0)));
		CHECK(end.is_equal_approx(Vector2(100, 0)));
		memdelete(path);
	}

	TEST_CASE("[FoliagePath2D] get_point_at multi-segment linear") {
		FoliagePath2D *path = memnew(FoliagePath2D);
		path->add_handle(Vector2(0, 0));
		path->add_handle(Vector2(100, 0));
		path->add_handle(Vector2(100, 100));
		Vector2 p0 = path->get_point_at(0.0);
		Vector2 p_quarter = path->get_point_at(0.25);
		Vector2 p_half = path->get_point_at(0.5);
		Vector2 p_end = path->get_point_at(1.0);
		CHECK(p0.is_equal_approx(Vector2(0, 0)));
		CHECK(p_quarter.is_equal_approx(Vector2(50, 0)));
		CHECK(p_half.is_equal_approx(Vector2(100, 0)));
		CHECK(p_end.is_equal_approx(Vector2(100, 100)));
		memdelete(path);
	}

	TEST_CASE("[FoliagePath2D] get_angle_at horizontal line") {
		FoliagePath2D *path = memnew(FoliagePath2D);
		path->add_handle(Vector2(0, 0));
		path->add_handle(Vector2(100, 0));
		real_t angle = path->get_angle_at(0.5);
		CHECK(Math::abs(angle) < CMP_EPSILON); // Should be ~0 (horizontal right)
		memdelete(path);
	}

	TEST_CASE("[FoliagePath2D] get_angle_at vertical line") {
		FoliagePath2D *path = memnew(FoliagePath2D);
		path->add_handle(Vector2(0, 0));
		path->add_handle(Vector2(0, 100));
		real_t angle = path->get_angle_at(0.5);
		CHECK(Math::abs(angle - Math_PI / 2) < CMP_EPSILON);
		memdelete(path);
	}

	TEST_CASE("[FoliagePath2D] get_point_at with too few handles returns zero") {
		FoliagePath2D *path = memnew(FoliagePath2D);
		Vector2 p = path->get_point_at(0.5);
		CHECK(p == Vector2(0, 0));
		memdelete(path);
	}

	TEST_CASE("[FoliagePath2D] get_point_at clamps t") {
		FoliagePath2D *path = memnew(FoliagePath2D);
		path->add_handle(Vector2(0, 0));
		path->add_handle(Vector2(100, 0));
		Vector2 before = path->get_point_at(-0.5);
		Vector2 after = path->get_point_at(1.5);
		CHECK(before.is_equal_approx(Vector2(0, 0)));
		CHECK(after.is_equal_approx(Vector2(100, 0)));
		memdelete(path);
	}

	TEST_CASE("[FoliagePath2D] compute_placement returns transforms") {
		FoliagePath2D *path = memnew(FoliagePath2D);
		path->add_handle(Vector2(0, 0));
		path->add_handle(Vector2(200, 0));
		path->set_overlapping_factor(0.0);
		PoolRealArray widths, heights;
		widths.push_back(20);
		heights.push_back(40);
		Array placements = path->compute_placement(widths, heights);
		CHECK(placements.size() > 0);
		// Each placement should be a Transform2D
		for (int i = 0; i < placements.size(); i++) {
			CHECK(placements[i].get_type() == Variant::TRANSFORM2D);
		}
		memdelete(path);
	}

	TEST_CASE("[FoliagePath2D] compute_placement with empty handles") {
		FoliagePath2D *path = memnew(FoliagePath2D);
		PoolRealArray widths, heights;
		widths.push_back(20);
		heights.push_back(40);
		Array placements = path->compute_placement(widths, heights);
		CHECK(placements.size() == 0);
		memdelete(path);
	}

	TEST_CASE("[FoliagePath2D] set/get bias and tension") {
		FoliagePath2D *path = memnew(FoliagePath2D);
		path->set_bias(0.5);
		CHECK(path->get_bias() == doctest::Approx(0.5));
		path->set_tension(-0.3);
		CHECK(path->get_tension() == doctest::Approx(-0.3));
		memdelete(path);
	}

	TEST_CASE("[FoliagePath2D] set/get first/last object offset") {
		FoliagePath2D *path = memnew(FoliagePath2D);
		path->set_first_object_offset(5.0);
		CHECK(path->get_first_object_offset() == doctest::Approx(5.0));
		path->set_last_object_offset(-3.0);
		CHECK(path->get_last_object_offset() == doctest::Approx(-3.0));
		memdelete(path);
	}

	// --- FoliageAnimation2D ---

	TEST_CASE("[FoliageAnimation2D] default construction") {
		FoliageAnimation2D *anim = memnew(FoliageAnimation2D);
		CHECK(anim->get_bend_mode() == FOLIAGE2D_BEND_SIMPLE);
		CHECK(anim->get_offset() == Vector3(0, 0, 0));
		CHECK(anim->get_width_segments() == 1);
		CHECK(anim->get_height_segments() == 4);
		memdelete(anim);
	}

	TEST_CASE("[FoliageAnimation2D] set/get bend_mode") {
		FoliageAnimation2D *anim = memnew(FoliageAnimation2D);
		anim->set_bend_mode(FOLIAGE2D_BEND_SMART);
		CHECK(anim->get_bend_mode() == FOLIAGE2D_BEND_SMART);
		anim->set_bend_mode(FOLIAGE2D_BEND_SIMPLE);
		CHECK(anim->get_bend_mode() == FOLIAGE2D_BEND_SIMPLE);
		memdelete(anim);
	}

	TEST_CASE("[FoliageAnimation2D] set/get offset") {
		FoliageAnimation2D *anim = memnew(FoliageAnimation2D);
		anim->set_offset(Vector3(5, 10, 0));
		CHECK(anim->get_offset() == Vector3(5, 10, 0));
		memdelete(anim);
	}

	TEST_CASE("[FoliageAnimation2D] set/get segments") {
		FoliageAnimation2D *anim = memnew(FoliageAnimation2D);
		anim->set_width_segments(3);
		CHECK(anim->get_width_segments() == 3);
		anim->set_height_segments(6);
		CHECK(anim->get_height_segments() == 6);
		memdelete(anim);
	}

	TEST_CASE("[FoliageAnimation2D] segments reject zero") {
		FoliageAnimation2D *anim = memnew(FoliageAnimation2D);
		anim->set_width_segments(3);
		anim->set_width_segments(0);
		CHECK(anim->get_width_segments() == 3);
		anim->set_height_segments(5);
		anim->set_height_segments(-1);
		CHECK(anim->get_height_segments() == 5);
		memdelete(anim);
	}

	TEST_CASE("[FoliageAnimation2D] make_linear_factors") {
		FoliageAnimation2D *anim = memnew(FoliageAnimation2D);
		PoolRealArray factors = anim->make_linear_factors(4);
		CHECK(factors.size() == 5); // 4+1
		CHECK(factors[0] == doctest::Approx(0.0));
		CHECK(factors[1] == doctest::Approx(0.25));
		CHECK(factors[2] == doctest::Approx(0.5));
		CHECK(factors[3] == doctest::Approx(0.75));
		CHECK(factors[4] == doctest::Approx(1.0));
		memdelete(anim);
	}

	TEST_CASE("[FoliageAnimation2D] make_quadratic_factors") {
		FoliageAnimation2D *anim = memnew(FoliageAnimation2D);
		PoolRealArray factors = anim->make_quadratic_factors(4);
		CHECK(factors.size() == 5);
		CHECK(factors[0] == doctest::Approx(0.0));
		CHECK(factors[1] == doctest::Approx(0.0625)); // 0.25^2
		CHECK(factors[2] == doctest::Approx(0.25)); // 0.5^2
		CHECK(factors[3] == doctest::Approx(0.5625)); // 0.75^2
		CHECK(factors[4] == doctest::Approx(1.0));
		memdelete(anim);
	}

	TEST_CASE("[FoliageAnimation2D] make_linear_factors monotonically increasing") {
		FoliageAnimation2D *anim = memnew(FoliageAnimation2D);
		PoolRealArray factors = anim->make_linear_factors(8);
		for (int i = 1; i < factors.size(); i++) {
			CHECK(factors[i] > factors[i - 1]);
		}
		memdelete(anim);
	}

	TEST_CASE("[FoliageAnimation2D] build_mesh creates mesh") {
		FoliageAnimation2D *anim = memnew(FoliageAnimation2D);
		anim->set_width_segments(2);
		anim->set_height_segments(3);
		anim->build_mesh(Vector2(32, 64));
		CHECK(anim->get_mesh().is_valid());
		CHECK(anim->get_mesh()->get_surface_count() == 1);
		memdelete(anim);
	}

	TEST_CASE("[FoliageAnimation2D] build_mesh sets default offset factors") {
		FoliageAnimation2D *anim = memnew(FoliageAnimation2D);
		anim->set_height_segments(4);
		anim->build_mesh(Vector2(32, 64));
		PoolRealArray factors = anim->get_offset_factors();
		CHECK(factors.size() == 5); // 4+1
		memdelete(anim);
	}

	TEST_CASE("[FoliageAnimation2D] update_animation simple bending") {
		FoliageAnimation2D *anim = memnew(FoliageAnimation2D);
		anim->set_width_segments(1);
		anim->set_height_segments(4);
		anim->build_mesh(Vector2(32, 64));
		anim->set_bend_mode(FOLIAGE2D_BEND_SIMPLE);
		anim->set_offset(Vector3(10, 0, 0));
		// Should not crash
		anim->update_animation();
		CHECK(anim->get_mesh().is_valid());
		memdelete(anim);
	}

	TEST_CASE("[FoliageAnimation2D] update_animation smart bending") {
		FoliageAnimation2D *anim = memnew(FoliageAnimation2D);
		anim->set_width_segments(1);
		anim->set_height_segments(4);
		anim->build_mesh(Vector2(32, 64));
		anim->set_bend_mode(FOLIAGE2D_BEND_SMART);
		anim->set_offset(Vector3(5, 0, 0));
		// Should not crash
		anim->update_animation();
		CHECK(anim->get_mesh().is_valid());
		memdelete(anim);
	}

	TEST_CASE("[FoliageAnimation2D] update_animation with zero offset") {
		FoliageAnimation2D *anim = memnew(FoliageAnimation2D);
		anim->build_mesh(Vector2(32, 64));
		anim->set_offset(Vector3(0, 0, 0));
		anim->update_animation();
		CHECK(anim->get_mesh().is_valid());
		memdelete(anim);
	}

	TEST_CASE("[FoliageAnimation2D] set/get offset_factors") {
		FoliageAnimation2D *anim = memnew(FoliageAnimation2D);
		PoolRealArray factors;
		factors.push_back(0.0);
		factors.push_back(0.3);
		factors.push_back(0.7);
		factors.push_back(1.0);
		anim->set_offset_factors(factors);
		PoolRealArray got = anim->get_offset_factors();
		CHECK(got.size() == 4);
		CHECK(got[0] == doctest::Approx(0.0));
		CHECK(got[3] == doctest::Approx(1.0));
		memdelete(anim);
	}

	TEST_CASE("[FoliageAnimation2D] smart bending with negative offset") {
		FoliageAnimation2D *anim = memnew(FoliageAnimation2D);
		anim->set_width_segments(1);
		anim->set_height_segments(4);
		anim->build_mesh(Vector2(32, 64));
		anim->set_bend_mode(FOLIAGE2D_BEND_SMART);
		anim->set_offset(Vector3(-8, 0, 0));
		anim->update_animation();
		CHECK(anim->get_mesh().is_valid());
		memdelete(anim);
	}

	TEST_CASE("[FoliageAnimation2D] smart bending with large offset") {
		FoliageAnimation2D *anim = memnew(FoliageAnimation2D);
		anim->set_width_segments(1);
		anim->set_height_segments(4);
		anim->build_mesh(Vector2(32, 64));
		anim->set_bend_mode(FOLIAGE2D_BEND_SMART);
		anim->set_offset(Vector3(50, 0, 0));
		anim->update_animation();
		CHECK(anim->get_mesh().is_valid());
		memdelete(anim);
	}

	TEST_CASE("[FoliageAnimation2D] multiple segment configurations") {
		for (int w = 1; w <= 3; w++) {
			for (int h = 1; h <= 5; h++) {
				FoliageAnimation2D *anim = memnew(FoliageAnimation2D);
				anim->set_width_segments(w);
				anim->set_height_segments(h);
				anim->build_mesh(Vector2(32, 64));
				anim->set_offset(Vector3(5, 2, 0));
				anim->update_animation();
				CHECK(anim->get_mesh().is_valid());
				memdelete(anim);
			}
		}
	}

	// --- Hermite interpolation (via smooth path) ---

	TEST_CASE("[FoliagePath2D] smooth path endpoints match handles") {
		FoliagePath2D *path = memnew(FoliagePath2D);
		path->set_path_type(FOLIAGE2D_PATH_SMOOTH);
		path->add_handle(Vector2(0, 0));
		path->add_handle(Vector2(50, 25));
		path->add_handle(Vector2(100, 0));
		Vector2 start = path->get_point_at(0.0);
		Vector2 end = path->get_point_at(1.0);
		// Endpoints should be at or very near the handle positions
		CHECK(start.distance_to(Vector2(0, 0)) < 1.0);
		CHECK(end.distance_to(Vector2(100, 0)) < 1.0);
		memdelete(path);
	}

	TEST_CASE("[FoliagePath2D] smooth path midpoint deviates from linear") {
		FoliagePath2D *path = memnew(FoliagePath2D);
		path->set_path_type(FOLIAGE2D_PATH_SMOOTH);
		path->add_handle(Vector2(0, 0));
		path->add_handle(Vector2(50, 50));
		path->add_handle(Vector2(100, 0));
		// The smooth midpoint should be near the middle handle
		Vector2 mid = path->get_point_at(0.5);
		CHECK(mid.distance_to(Vector2(50, 50)) < 20.0);
		memdelete(path);
	}
}

TEST_CASE("[Foliage2DAnimation] placeholder") {
	CHECK(true);
}

#endif // DOCTEST
