/**************************************************************************/
/*  goost_geometry_2d_bind.h                                              */
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

#include "core/object.h"

class _GoostGeometry2D : public Object {
	GDCLASS(_GoostGeometry2D, Object);

private:
	static _GoostGeometry2D *singleton;

protected:
	static void _bind_methods();

public:
	static _GoostGeometry2D *get_singleton() { return singleton; }

public:
	Array merge_polygons(const Vector<Point2> &p_polygon_a, const Vector<Point2> &p_polygon_b) const;
	Array clip_polygons(const Vector<Point2> &p_polygon_a, const Vector<Point2> &p_polygon_b) const;
	Array intersect_polygons(const Vector<Point2> &p_polygon_a, const Vector<Point2> &p_polygon_b) const;
	Array exclude_polygons(const Vector<Point2> &p_polygon_a, const Vector<Point2> &p_polygon_b) const;
	Array clip_polyline_with_polygon(const Vector<Point2> &p_polyline, const Vector<Point2> &p_polygon) const;
	Array intersect_polyline_with_polygon(const Vector<Point2> &p_polyline, const Vector<Point2> &p_polygon) const;

	Array inflate_polygon(const Vector<Point2> &p_polygon, real_t p_delta) const;
	Array deflate_polygon(const Vector<Point2> &p_polygon, real_t p_delta) const;
	Array deflate_polyline(const Vector<Point2> &p_polyline, real_t p_delta) const;

	Array triangulate_polygon(const Vector<Point2> &p_polygon) const;
	Array decompose_polygon(const Vector<Point2> &p_polygon) const;

	Vector<Point2> smooth_polygon(const Vector<Point2> &p_polygon, float p_density, float p_alpha = 0.5f) const;
	Vector<Point2> smooth_polyline(const Vector<Point2> &p_polyline, float p_density, float p_alpha = 0.5f) const;
	Vector<Point2> smooth_polygon_approx(const Vector<Point2> &p_polygon, int p_iterations = 1, float cut_distance = 0.25f) const;
	Vector<Point2> smooth_polyline_approx(const Vector<Point2> &p_polyline, int p_iterations = 1, float cut_distance = 0.25f) const;
	Vector<Point2> simplify_polyline(const Vector<Point2> &p_polyline, real_t p_epsilon) const;

	Vector2 polygon_centroid(const Vector<Vector2> &p_polygon) const;
	real_t polygon_area(const Vector<Vector2> &p_polygon) const;
	real_t polygon_perimeter(const Vector<Vector2> &p_polygon) const;
	real_t polyline_length(const Vector<Vector2> &p_polyline) const;
	Rect2 bounding_rect(const Vector<Point2> &p_points) const;

	int point_in_polygon(const Point2 &p_point, const Vector<Point2> &p_polygon) const;

	Vector<Point2> rectangle(const Vector2 &p_extents) const;
	Vector<Point2> circle(real_t p_radius, real_t p_max_error) const;
	Vector<Point2> ellipse(real_t p_width, real_t p_height, real_t p_max_error) const;
	Vector<Point2> capsule(real_t p_radius, real_t p_height, real_t p_max_error) const;
	Vector<Point2> regular_polygon(int p_edge_count, real_t p_size) const;

	Vector<Point2> pixel_line(const Point2 &p_start, const Point2 &p_end) const;
	Vector<Point2> pixel_circle(int p_radius, const Point2 &p_origin = Point2(0, 0)) const;
	Vector<Point2> polyline_to_pixels(const Vector<Point2> &p_points) const;
	Vector<Point2> polygon_to_pixels(const Vector<Point2> &p_points) const;

	_GoostGeometry2D();
};
