/**************************************************************************/
/*  ai_funnel.h                                                           */
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

#include "core/map.h"
#include "core/reference.h"
#include "core/vector.h"

#include "ddls_fwd.h"

class CanvasItem;

class DDLS_Funnel : public Reference {
	real_t radius;
	real_t radius_squared;
	int num_samples_circle;
	Vector<Point2> sample_circle;
	real_t sample_circle_distance_squared;

	CanvasItem *debug_surface;

	void adjust_with_tangents(const Point2 &p_1, bool p_apply_radius_to_p1, const Point2 &p_2, bool p_apply_radius_to_p2, const Map<Point2, int> &p_point_sides, const Map<Point2, Point2> &p_point_successor, Vector<Point2> &r_new_path, Vector<Point2> &r_adjusted_points);
	void check_adjusted_path(Vector<Point2> &p_new_path, Vector<Point2> &p_adjusted_points, const Map<Point2, int> &p_point_sides);
	void smooth_angle(const Point2 &p_prev_point, const Point2 &p_point_to_smooth, const Point2 &p_next_point, int p_side, Vector<Point2> &p_encircle_points);

public:
	real_t get_radius();
	void set_radius(real_t value);

	void find_path(Point2 p_from, Point2 p_to, Vector<DDLSFace> &p_list_faces, Vector<DDLSEdge> &p_list_edges, Vector<Point2> &r_result_path);

	DDLS_Funnel();
};
