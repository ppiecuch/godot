/**************************************************************************/
/*  foliage_2d_animation.h                                                */
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

// Ported from Unity C# Foliage2D system:
//   Foliage2D_Mesh  -> FoliageMesh2D (Resource)
//   Foliage2DPath   -> FoliagePath2D (Node2D)
//   Foliage2DAnimation -> FoliageAnimation2D (Node2D)

#ifndef FOLIAGE_2D_ANIMATION_H
#define FOLIAGE_2D_ANIMATION_H

#include "core/reference.h"
#include "scene/2d/mesh_instance_2d.h"
#include "scene/2d/node_2d.h"
#include "scene/resources/mesh.h"

// Foliage placement pattern.
// Describes how foliage objects are arranged on path lines.
enum Foliage2DPattern {
	FOLIAGE2D_PATTERN_RANDOM, // Random prefab selection.
	FOLIAGE2D_PATTERN_CONSECUTIVE, // Sequential prefab selection.
	FOLIAGE2D_PATTERN_MAX,
};

// Foliage overlapping type.
// Controls spacing overlap between adjacent foliage objects.
enum Foliage2DOverlappingType {
	FOLIAGE2D_OVERLAP_FIXED, // Constant overlap factor.
	FOLIAGE2D_OVERLAP_RANDOM, // Random overlap between min/max.
	FOLIAGE2D_OVERLAP_MAX,
};

// Foliage path interpolation type.
enum Foliage2DPathType {
	FOLIAGE2D_PATH_LINEAR, // Straight line between nodes.
	FOLIAGE2D_PATH_SMOOTH, // Hermite-interpolated curve.
	FOLIAGE2D_PATH_MAX,
};

// Foliage mesh bending mode.
enum Foliage2DBendMode {
	FOLIAGE2D_BEND_SIMPLE, // Simple offset-based bending.
	FOLIAGE2D_BEND_SMART, // Realistic arc-based bending with rotation.
	FOLIAGE2D_BEND_MAX,
};

VARIANT_ENUM_CAST(Foliage2DPattern);
VARIANT_ENUM_CAST(Foliage2DOverlappingType);
VARIANT_ENUM_CAST(Foliage2DPathType);
VARIANT_ENUM_CAST(Foliage2DBendMode);

// FoliageMesh2D: Builds subdivided quad meshes for foliage deformation.
// Creates an ArrayMesh with configurable width/height segments for
// vertex-level animation (wind, bending).
class FoliageMesh2D : public Reference {
	GDCLASS(FoliageMesh2D, Reference)

private:
	PoolVector3Array mesh_verts;
	PoolIntArray mesh_indices;
	PoolVector2Array mesh_uvs;

	int width_segments;
	int height_segments;
	real_t width;
	real_t height;

protected:
	static void _bind_methods();

public:
	void set_width_segments(int p_segments);
	int get_width_segments() const;
	void set_height_segments(int p_segments);
	int get_height_segments() const;
	void set_size(const Vector2 &p_size);
	Vector2 get_size() const;

	void clear();
	void generate_mesh();
	Ref<ArrayMesh> build() const;

	int get_vertex_count() const;
	int get_horizontal_vertex_count() const;

	FoliageMesh2D();
};

// FoliagePath2D: Places child Node2D instances along a path defined
// by handle positions. Supports linear and Hermite-smoothed paths,
// overlapping control, and random/consecutive placement patterns.
class FoliagePath2D : public Node2D {
	GDCLASS(FoliagePath2D, Node2D)

private:
	Foliage2DPattern pattern;
	Foliage2DOverlappingType overlap_type;
	Foliage2DPathType path_type;

	PoolVector2Array handles;

	real_t overlapping_factor;
	real_t min_overlapping_factor;
	real_t max_overlapping_factor;
	real_t bias;
	real_t tension;
	real_t first_object_offset;
	real_t last_object_offset;
	bool uniform_values;

	// Internal state during placement.
	real_t _distance_from_start;
	real_t _prev_distance_from_start;
	real_t _previous_width;
	real_t _line_length;
	int _object_index;
	int _prefab_index;
	Vector2 _line;
	Vector2 _line_normal;

	// Hermite interpolation helpers.
	real_t _hermite_interpolation(real_t p1, real_t p2, real_t p3, real_t p4, int index) const;
	real_t _hermite_slope(real_t p1, real_t p2, real_t p3, real_t p4, real_t mu, int index) const;
	Vector2 _smooth_point(int index) const;
	Vector2 _smooth_tangent(int index) const;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_pattern(Foliage2DPattern p_pattern);
	Foliage2DPattern get_pattern() const;
	void set_overlap_type(Foliage2DOverlappingType p_type);
	Foliage2DOverlappingType get_overlap_type() const;
	void set_path_type(Foliage2DPathType p_type);
	Foliage2DPathType get_path_type() const;

	void set_handles(const PoolVector2Array &p_handles);
	PoolVector2Array get_handles() const;
	void add_handle(const Vector2 &p_pos);
	void remove_handle(int p_index);
	int get_handle_count() const;

	void set_overlapping_factor(real_t p_factor);
	real_t get_overlapping_factor() const;
	void set_min_overlapping_factor(real_t p_factor);
	real_t get_min_overlapping_factor() const;
	void set_max_overlapping_factor(real_t p_factor);
	real_t get_max_overlapping_factor() const;

	void set_bias(real_t p_bias);
	real_t get_bias() const;
	void set_tension(real_t p_tension);
	real_t get_tension() const;
	void set_uniform_values(bool p_uniform);
	bool get_uniform_values() const;

	void set_first_object_offset(real_t p_offset);
	real_t get_first_object_offset() const;
	void set_last_object_offset(real_t p_offset);
	real_t get_last_object_offset() const;

	// Compute a point on the path at parameter t (0..1) across the full path.
	Vector2 get_point_at(real_t p_t) const;
	// Compute the angle at parameter t.
	real_t get_angle_at(real_t p_t) const;

	// Distribute positions along the path for items of given widths/heights.
	// Returns array of Transform2D for each placed item.
	Array compute_placement(const PoolRealArray &p_widths, const PoolRealArray &p_heights) const;

	FoliagePath2D();
};

// FoliageAnimation2D: Animates mesh vertices for wind/bend effects.
// Works with a FoliageMesh2D to produce vertex-animated foliage.
// Supports simple offset-based bending and smart arc-based bending.
class FoliageAnimation2D : public MeshInstance2D {
	GDCLASS(FoliageAnimation2D, MeshInstance2D)

private:
	Foliage2DBendMode bend_mode;
	Vector3 offset; // Animation offset (x=horizontal bend, y=vertical, z=depth).
	PoolRealArray offset_factors; // Per-row factor (0=base, 1=tip).

	int width_segments;
	int height_segments;

	// Cached initial vertex positions (set when mesh is built).
	PoolVector3Array initial_vertex_pos;

	void _apply_simple_bending(PoolVector3Array &p_verts) const;
	void _apply_smart_bending(PoolVector3Array &p_verts) const;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_bend_mode(Foliage2DBendMode p_mode);
	Foliage2DBendMode get_bend_mode() const;

	void set_offset(const Vector3 &p_offset);
	Vector3 get_offset() const;

	void set_offset_factors(const PoolRealArray &p_factors);
	PoolRealArray get_offset_factors() const;

	void set_width_segments(int p_segments);
	int get_width_segments() const;
	void set_height_segments(int p_segments);
	int get_height_segments() const;

	void build_mesh(const Vector2 &p_size);
	void update_animation();

	// Generate default linear offset factors for n rows (0 at base, 1 at tip).
	static PoolRealArray make_linear_factors(int p_height_segments);
	// Generate quadratic offset factors (more movement at tip).
	static PoolRealArray make_quadratic_factors(int p_height_segments);

	FoliageAnimation2D();
};

#endif // FOLIAGE_2D_ANIMATION_H
