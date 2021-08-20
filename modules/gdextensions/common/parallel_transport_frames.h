
/*
 *  ParallelTransportFrames.h
 *
 *  Copyright (c) 2012, Neil Mendoza, http://www.neilmendoza.com
 *  All rights reserved. 
 *  
 */

#include "core/math/math_defs.h"
#include "core/math/vector3.h"
#include "core/math/transform.h"
#include "scene/2d/canvas_item.h"

#include <queue>

class ParallelTransportFrames
{
public:
	// TODO: add start_normal

	ParallelTransportFrames();

	bool add_point(real_t x, real_t y, real_t z);
	bool add_point(const Vector3& point);
	void debug_draw(CanvasItem &canvas, real_t axisSize = 10);

	Transform transform_matrix() const { return frames.back(); }
	Basis normal_matrix() const;

	unsigned frames_size() const { return frames.size(); }
	unsigned points_size() const { return points.size(); }

	std::deque<Transform>& get_frames() { return frames; }

	Transform frame_at(unsigned idx) const { return frames[idx]; }

	Vector3 get_start_normal() const { return start_normal; }
	Vector3 get_current_tangent() const { return cur_tangent; }

	Vector3 calc_current_normal() const;

	void clear();

	void set_max_frames(unsigned p_max_frames) { max_frames = p_max_frames; }

private:
	unsigned max_points, max_frames;

	void first_frame();
	void next_frame();

	Vector3 start_normal;
	Vector3 prev_tangent, cur_tangent;

	std::deque<Vector3> points;
	std::deque<Transform> frames;
};
