/**************************************************************************/
/*  parallel_transport_frames.h                                           */
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

/*
 *  ParallelTransportFrames.h
 *
 *  Copyright (c) 2012, Neil Mendoza, http://www.neilmendoza.com
 *  All rights reserved.
 *
 */

#include "core/math/math_defs.h"
#include "core/math/transform.h"
#include "core/math/vector3.h"

#include <queue>

// TODO: add start_normal

class MultiMeshInstance;

class ParallelTransportFrames {
	unsigned max_points, max_frames;

	void first_frame();
	void next_frame();

	Vector3 start_normal;
	Vector3 prev_tangent, cur_tangent;

	std::deque<Vector3> points;
	std::deque<Transform> frames;

public:
	bool add_point(real_t x, real_t y, real_t z);
	bool add_point(const Vector3 &point);

	MultiMeshInstance *debug_draw_node(MultiMeshInstance *&multi_mesh, real_t axisSize = 10);

	Transform transform_matrix() const { return frames.back(); }
	Basis normal_matrix() const;

	unsigned frames_size() const { return frames.size(); }
	unsigned points_size() const { return points.size(); }

	std::deque<Transform> &get_frames() { return frames; }

	Transform frame_at(unsigned idx) const { return frames[idx]; }

	Vector3 get_start_normal() const { return start_normal; }
	Vector3 get_current_tangent() const { return cur_tangent; }

	Vector3 calc_current_normal() const;

	void clear();

	void set_max_frames(unsigned p_max_frames) { max_frames = p_max_frames; }

	ParallelTransportFrames();
};
