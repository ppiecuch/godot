/**************************************************************************/
/*  line_builder.cpp                                                      */
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

#include "core/math/geometry.h"
#include "core/math/math_defs.h"
#include "core/math/math_funcs.h"
#include "line_builder.h"
#include "scene/resources/curve.h"

//----------------------------------------------------------------------------
// Util
//----------------------------------------------------------------------------

#define PUSH_BACK_IF(nm, op)                             \
	template <typename T>                                \
	void push_back_if_##nm(Vector<T> &p_arr, T p_elem) { \
		if (p_elem op)                                   \
			p_arr.push_back(p_elem);                     \
	}

PUSH_BACK_IF(gtzero, > 0);
PUSH_BACK_IF(nonzero, != 0);
PUSH_BACK_IF(lezero, < 0);

// Utility method.

// TODO I'm pretty sure there is an even faster way to swap things
template <typename T>
static inline void swap(T &a, T &b) {
	T tmp = a;
	a = b;
	b = tmp;
}

static inline Vector2 interpolate(const Rect2 &r, const Vector2 &v) {
	return Vector2(
			Math::lerp(r.position.x, r.position.x + r.get_size().x, v.x),
			Math::lerp(r.position.y, r.position.y + r.get_size().y, v.y));
}

//----------------------------------------------------------------------------
// LineBuilder
//----------------------------------------------------------------------------

LineBuilder::LineBuilder() {
	joint_mode = Line2D::LINE_JOINT_SHARP;
	width = 10;
	curve = nullptr;
	default_color = Color(0.4, 0.5, 1);
	gradient = nullptr;
	texture_mode = Line2D::LINE_TEXTURE_NONE;
	sharp_limit = 2.f;
	round_precision = 8;
	begin_cap_mode = Line2D::LINE_CAP_NONE;
	end_cap_mode = Line2D::LINE_CAP_NONE;
	tile_aspect = 1.f;
	tile_region = Rect2(0, 0, 1, 1);

	_interpolate_color = false;
	_last_index[0] = 0;
	_last_index[1] = 0;
}

void LineBuilder::build() {
	// Need at least 2 points to draw a line
	if (points.size() < 2) {
		vertices.clear();
		colors.clear();
		indices.clear();
		uvs.clear();
		return;
	}

	ERR_FAIL_COND(tile_aspect <= 0);

	const float hw = width / 2;
	const float hw_sq = hw * hw;
	const float sharp_limit_sq = sharp_limit * sharp_limit;

	const int point_count = points.size();
	const bool wrap_around = closed && point_count > 2;
	const bool retrieve_curve = curve != nullptr;
	const bool distance_required = _interpolate_color || retrieve_curve || texture_mode == Line2D::LINE_TEXTURE_TILE || texture_mode == Line2D::LINE_TEXTURE_STRETCH;

	_interpolate_color = gradient != nullptr;
	_repeat_segment = (tile_region != Rect2(0, 0, 1, 1));
	_last_uvx = 0;

	// Initial values

	Vector2 pos0 = points[0];
	Vector2 pos1 = points[1];
	Vector2 f0 = (pos1 - pos0).normalized();
	Vector2 u0 = f0.orthogonal();

	Color color0;
	Color color1;

	float current_distance0 = 0.f;
	float current_distance1 = 0.f;
	float total_distance = 0.f;
	float width_factor = 1.f;
	float modified_hw = hw;
	if (retrieve_curve) {
		width_factor = curve->sample_baked(0);
		modified_hw = hw * width_factor;
	}

	// The strip's starting edge must already be offset to either side of pos0;
	// leaving it collapsed on pos0 produced a degenerate first quad/cap.
	Vector2 pos_up0 = pos0 + u0 * modified_hw;
	Vector2 pos_down0 = pos0 - u0 * modified_hw;
	if (distance_required) {
		// Calculate the total distance.
		for (int i = 1; i < point_count; ++i) {
			total_distance += points[i].distance_to(points[i - 1]);
		}
		if (wrap_around) {
			total_distance += points[point_count - 1].distance_to(pos0);
		} else {
			// Adjust the total distance. The line's outer length may be a little higher due to the end caps.
			if (begin_cap_mode == Line2D::LINE_CAP_BOX || begin_cap_mode == Line2D::LINE_CAP_ROUND) {
				total_distance += modified_hw;
			}
			if (end_cap_mode == Line2D::LINE_CAP_BOX || end_cap_mode == Line2D::LINE_CAP_ROUND) {
				if (retrieve_curve) {
					total_distance += hw * curve->sample_baked(1.f);
				} else {
					total_distance += hw;
				}
			}
		}
	}
	if (_interpolate_color) {
		color0 = gradient->get_color(0);
	} else {
		colors.push_back(default_color);
	}

	float uvx0 = 0.f;
	float uvx1 = 0.f;

	// Begin cap
	if (!wrap_around) {
		if (begin_cap_mode == Line2D::LINE_CAP_BOX) {
			// Push back first vertices a little bit.
			pos_up0 -= f0 * modified_hw;
			pos_down0 -= f0 * modified_hw;

			current_distance0 += modified_hw;
			current_distance1 = current_distance0;
		} else if (begin_cap_mode == Line2D::LINE_CAP_ROUND) {
			if (texture_mode == Line2D::LINE_TEXTURE_TILE) {
				uvx0 = width_factor * 0.5 / tile_aspect;
			} else if (texture_mode == Line2D::LINE_TEXTURE_STRETCH) {
				uvx0 = width * width_factor / total_distance;
			}
			new_arc(pos0, pos_up0 - pos0, -Math_PI, color0, Rect2(0, 0, uvx0 * 2, 1));
			current_distance0 += modified_hw;
			current_distance1 = current_distance0;
		}
		strip_begin(pos_up0, pos_down0, color0, uvx0);
	}

	/*
	 *  pos_up0 ------------- pos_up1 --------------------
	 *     |                     |
	 *   pos0 - - - - - - - - - pos1 - - - - - - - - - pos2
	 *     |                     |
	 * pos_down0 ------------ pos_down1 ------------------
	 *
	 *   i-1                     i                      i+1
	 */

	// http://labs.hyperandroid.com/tag/opengl-lines
	// (not the same implementation but visuals help a lot)

	// If the polyline wraps around, then draw two more segments with joints:
	// The last one, which should normally end with an end cap, and the one that matches the end and the beginning.
	int segments_count = wrap_around ? point_count : (point_count - 2);
	// The wraparound case starts with a "fake walk" from the end of the polyline
	// to its beginning, so that its first joint is correct, without drawing anything.
	int first_point = wrap_around ? -1 : 1;

	// If the line wraps around, these variables will be used for the final segment.
	Vector2 first_pos_up, first_pos_down;
	bool is_first_joint_sharp = false;

	// For each additional segment
	for (int i = first_point; i <= segments_count; ++i) {
		pos1 = points[(i == -1) ? point_count - 1 : i % point_count]; // First point.
		Vector2 pos2 = points[(i + 1) % point_count]; // Second point.

		Vector2 f1 = (pos2 - pos1).normalized();
		Vector2 u1 = f1.orthogonal();

		// Determine joint orientation
		const float dp = u0.dot(f1);
		const Orientation orientation = (dp > 0 ? UP : DOWN);

		if (distance_required && i >= 1) {
			current_distance1 += pos0.distance_to(pos1);
		}
		if (_interpolate_color) {
			color1 = gradient->get_color_at_offset(current_distance1 / total_distance);
		}
		if (retrieve_curve) {
			width_factor = curve->interpolate_baked(current_distance1 / total_distance);
			modified_hw = hw * width_factor;
		}

		Vector2 inner_normal0 = u0 * modified_hw;
		Vector2 inner_normal1 = u1 * modified_hw;
		if (orientation == DOWN) {
			inner_normal0 = -inner_normal0;
			inner_normal1 = -inner_normal1;
		}

		/*
		 * ---------------------------
		 *                        /
		 * 0                     /    1
		 *                      /          /
		 * --------------------x------    /
		 *                    /          /    (here shown with orientation == DOWN)
		 *                   /          /
		 *                  /          /
		 *                 /          /
		 *                     2     /
		 *                          /
		 */

		// Find inner intersection at the joint
		Vector2 corner_pos_in, corner_pos_out;
		bool is_intersecting = Geometry::segment_intersects_segment_2d(
				pos0 + inner_normal0, pos1 + inner_normal0,
				pos1 + inner_normal1, pos2 + inner_normal1,
				&corner_pos_in);

		if (is_intersecting) {
			// Inner parts of the segments intersect
			corner_pos_out = 2.f * pos1 - corner_pos_in;
		} else {
			// No intersection, segments are either parallel or too sharp
			corner_pos_in = pos1 + inner_normal0;
			corner_pos_out = pos1 - inner_normal0;
		}

		Vector2 corner_pos_up, corner_pos_down;
		if (orientation == UP) {
			corner_pos_up = corner_pos_in;
			corner_pos_down = corner_pos_out;
		} else {
			corner_pos_up = corner_pos_out;
			corner_pos_down = corner_pos_in;
		}

		Line2D::LineJointMode current_joint_mode = joint_mode;

		Vector2 pos_up1, pos_down1;
		if (is_intersecting) {
			// Fallback on bevel if sharp angle is too high (because it would produce very long miters)
			float width_factor_sq = width_factor * width_factor;
			if (current_joint_mode == Line2D::LINE_JOINT_SHARP && corner_pos_out.distance_squared_to(pos1) / (hw_sq * width_factor_sq) > sharp_limit_sq) {
				current_joint_mode = Line2D::LINE_JOINT_BEVEL;
			}
			if (current_joint_mode == Line2D::LINE_JOINT_SHARP) {
				// In this case, we won't create joint geometry,
				// The previous and next line quads will directly share an edge.
				pos_up1 = corner_pos_up;
				pos_down1 = corner_pos_down;
			} else {
				// Bevel or round
				if (orientation == UP) {
					pos_up1 = corner_pos_up;
					pos_down1 = pos1 - u0 * modified_hw;
				} else {
					pos_up1 = pos1 + u0 * modified_hw;
					pos_down1 = corner_pos_down;
				}
			}
		} else {
			// No intersection: fallback
			if (current_joint_mode == Line2D::LINE_JOINT_SHARP) {
				// There is no fallback implementation for LINE_JOINT_SHARP so switch to the LINE_JOINT_BEVEL
				current_joint_mode = Line2D::LINE_JOINT_BEVEL;
			}
			pos_up1 = corner_pos_up;
			pos_down1 = corner_pos_down;
		}

		// Triangles are clockwise
		if (texture_mode == Line2D::LINE_TEXTURE_TILE) {
			uvx1 = current_distance1 / (width * tile_aspect);
		} else if (texture_mode == Line2D::LINE_TEXTURE_STRETCH) {
			uvx1 = current_distance1 / total_distance;
		}

		// Swap vars for use in the next line.
		color0 = color1;
		u0 = u1;
		f0 = f1;
		pos0 = pos1;
		if (is_intersecting) {
			if (current_joint_mode == Line2D::LINE_JOINT_SHARP) {
				pos_up0 = pos_up1;
				pos_down0 = pos_down1;
			} else {
				if (orientation == UP) {
					pos_up0 = corner_pos_up;
					pos_down0 = pos1 - u1 * modified_hw;
				} else {
					pos_up0 = pos1 + u1 * modified_hw;
					pos_down0 = corner_pos_down;
				}
			}
		} else {
			pos_up0 = pos1 + u1 * modified_hw;
			pos_down0 = pos1 - u1 * modified_hw;
		}

		// End the "fake pass" in the closed line case before the drawing subroutine.
		// Seed the strip with the wrap joint's corner (just computed above) so the
		// first real quad connects to it instead of to stale/unset indices.
		if (i == -1) {
			strip_begin(pos_up0, pos_down0, color0, uvx0);
			continue;
		}

		// For wrap-around polylines, store some kind of start positions of the first joint for the final connection.
		if (wrap_around && i == 0) {
			const Vector2 first_pos_center = (pos_up1 + pos_down1) / 2;
			const float lerp_factor = 1.0 / width_factor;
			first_pos_up = first_pos_center.lerp(pos_up1, lerp_factor);
			first_pos_down = first_pos_center.lerp(pos_down1, lerp_factor);
			is_first_joint_sharp = current_joint_mode == Line2D::LINE_JOINT_SHARP;
		}

		// Add current line body quad.
		if (wrap_around && retrieve_curve && !is_first_joint_sharp && i == segments_count) {
			// If the width curve is not seamless, we might need to fetch the line's start points to use them for the final connection.
			Vector2 first_pos_center = (first_pos_up + first_pos_down) / 2;
			strip_add_quad(first_pos_center.lerp(first_pos_up, width_factor), first_pos_center.lerp(first_pos_down, width_factor), color1, uvx1);
			break;
		} else {
			strip_add_quad(pos_up1, pos_down1, color1, uvx1);
		}

		// From this point, bu0 and bd0 concern the next segment.
		// Add joint geometry.

		if (current_joint_mode != Line2D::LINE_JOINT_SHARP) {
			/* ________________ cbegin
			 *               / \
			 *              /   \
			 * ____________/_ _ _\ cend
			 *             |     |
			 *             |     |
			 *             |     |
			 */

			Vector2 cbegin, cend;
			if (orientation == UP) {
				cbegin = pos_down1;
				cend = pos_down0;
			} else {
				cbegin = pos_up1;
				cend = pos_up0;
			}

			if (current_joint_mode == Line2D::LINE_JOINT_BEVEL && !(wrap_around && i == segments_count)) {
				strip_add_tri(cend, orientation);
			} else if (current_joint_mode == Line2D::LINE_JOINT_ROUND && !(wrap_around && i == segments_count)) {
				Vector2 vbegin = cbegin - pos1;
				Vector2 vend = cend - pos1;
				strip_add_arc(pos1, vbegin.angle_to(vend), orientation);
			}

			if (is_intersecting) {
				// In this case the joint is too corrputed to be re-used,
				// start again the strip with fallback points
				strip_begin(pos_up0, pos_down0, color1, uvx1);
			}
		}
	}

	// Draw the last (or only) segment, with its end cap logic.
	if (!wrap_around) {
		pos1 = points[point_count - 1];

		if (distance_required) {
			current_distance1 += pos0.distance_to(pos1);
		}
		if (_interpolate_color) {
			color1 = gradient->get_color(gradient->get_points_count() - 1);
		}
		if (retrieve_curve) {
			width_factor = curve->sample_baked(1);
			modified_hw = hw * width_factor;
		}

		Vector2 pos_up1 = pos1 + u0 * modified_hw;
		Vector2 pos_down1 = pos1 - u0 * modified_hw;

		// Add extra distance for a box end cap.
		if (end_cap_mode == Line2D::LINE_CAP_BOX) {
			pos_up1 += f0 * modified_hw;
			pos_down1 += f0 * modified_hw;

			current_distance1 += modified_hw;
		}

		if (texture_mode == Line2D::LINE_TEXTURE_TILE) {
			uvx1 = current_distance1 / (width * tile_aspect);
		} else if (texture_mode == Line2D::LINE_TEXTURE_STRETCH) {
			uvx1 = current_distance1 / total_distance;
		}

		strip_add_quad(pos_up1, pos_down1, color1, uvx1);

		// Custom drawing for a round end cap.
		if (end_cap_mode == Line2D::LINE_CAP_ROUND) {
			// Note: color is not used in case we don't interpolate.
			Color color = _interpolate_color ? gradient->get_color(gradient->get_points_count() - 1) : Color(0, 0, 0);
			float dist = 0;
			if (texture_mode == Line2D::LINE_TEXTURE_TILE) {
				dist = width_factor / tile_aspect;
			} else if (texture_mode == Line2D::LINE_TEXTURE_STRETCH) {
				dist = width * width_factor / total_distance;
			}
			new_arc(pos1, pos_up1 - pos1, Math_PI, color, Rect2(uvx1 - 0.5 * dist, 0, dist, 1));
		}
	}

	if (!_repeat_segment) {
		return;
	}

	const real_t sx = tile_region.position.x;
	const real_t sy = tile_region.position.y;
	const real_t sw = tile_region.size.x;
	const real_t sh = tile_region.size.y;
	// recalculate uvs values
	for (int i = 0; i < uvs.size(); i++) {
		uvs.write[i] = Vector2(sx + uvs[i].x * sw, sy + uvs[i].y * sh);
	}
}

void LineBuilder::strip_begin(Vector2 up, Vector2 down, Color color, float uvx) {
	int vi = vertices.size();

	vertices.push_back(up, down);

	if (_interpolate_color) {
		colors.push_back(color, color);
	}

	if (texture_mode != Line2D::LINE_TEXTURE_NONE) {
		uvs.push_back(Vector2(uvx, 0), Vector2(uvx, 1));
	}

	_last_index[UP] = vi;
	_last_index[DOWN] = vi + 1;
}

void LineBuilder::strip_new_quad(Vector2 up, Vector2 down, Color color, float uvx) {
	int vi = vertices.size();

	vertices.push_back(vertices[_last_index[UP]], vertices[_last_index[DOWN]]);
	vertices.push_back(up, down);

	if (_interpolate_color) {
		colors.push_multi(4, color);
	}

	if (texture_mode != Line2D::LINE_TEXTURE_NONE) {
		uvs.push_back(uvs[_last_index[UP]], uvs[_last_index[DOWN]]);
		uvs.push_back(Vector2(uvx, UP), Vector2(uvx, DOWN));
	}

	indices.push_back(vi, vi + 3, vi + 1);
	indices.push_back(vi, vi + 2, vi + 3);

	_last_index[UP] = vi + 2;
	_last_index[DOWN] = vi + 3;
}

void LineBuilder::strip_add_quad(Vector2 up, Vector2 down, Color color, float uvx) {
	int vi = vertices.size();

	if (uvx > 1 && _repeat_segment && vertices.size() && texture_mode == Line2D::LINE_TEXTURE_TILE) {
		const float last_remainings = Math::ceil(_last_uvx) - _last_uvx; // remainings of the last tile
		const float dist = uvx - _last_uvx;

		// split dist (excluding remaining part of the last tile) on texture tile, eg:
		// [0,1] ................ [2,2] ................ [3,3]
		// [0,1] .. 1|0 .. 1|0 .. [0,2] .. 1|0 .. 1|0 .. [0.3]
		Vector2 prev_down = vertices.last(0);
		Vector2 prev_up = vertices.last(1);
		Vector2 step_down = (down - prev_down) / dist;
		Vector2 step_up = (up - prev_up) / dist;
		Color prev_color = _interpolate_color ? colors.last() : Color();

		const bool is_last_remains = last_remainings > 0;
		const int segs = Math::floor(dist - last_remainings) + is_last_remains;
		const Vector2 full_quad[] = { Vector2(1, 0), Vector2(1, 1), Vector2(0, 0), Vector2(0, 1) };

		for (int s = 0; s < segs; s++) {
			if (s == 0 && is_last_remains) {
				prev_up += step_up * last_remainings;
				prev_down += step_down * last_remainings;
			} else {
				prev_up += step_up;
				prev_down += step_down;
			}
			vertices.push_back(prev_up, prev_down, prev_up, prev_down);
			if (_interpolate_color) {
				const float t = s / dist;
				Color curr_color = prev_color.linear_interpolate(color, t);
				colors.push_multi(4, curr_color);
			}

			uvs.append_array(4, full_quad);

			indices.push_back(_last_index[UP], vi + 1, _last_index[DOWN]);
			indices.push_back(_last_index[UP], vi, vi + 1);

			_last_index[UP] = vi;
			_last_index[DOWN] = vi + 1;

			vi += 2;

			indices.push_back(_last_index[UP], vi + 1, _last_index[DOWN]);
			indices.push_back(_last_index[UP], vi, vi + 1);

			_last_index[UP] = vi;
			_last_index[DOWN] = vi + 1;

			vi += 2;
		}
		_last_uvx = uvx;

		// remaining part
		uvx -= Math::floor(uvx);
		// nothing left to drawn
		if (uvx == 0) {
			return;
		}
	}

	vertices.push_back(up, down);

	if (_interpolate_color) {
		colors.push_multi(2, color);
	}

	if (texture_mode != Line2D::LINE_TEXTURE_NONE) {
		uvs.push_back(Vector2(uvx, 0), Vector2(uvx, 1));
	}

	indices.push_back(_last_index[UP], vi + 1, _last_index[DOWN]);
	indices.push_back(_last_index[UP], vi, vi + 1);

	_last_index[UP] = vi;
	_last_index[DOWN] = vi + 1;
}

void LineBuilder::strip_add_tri(Vector2 up, Orientation orientation) {
	int vi = vertices.size();

	vertices.push_back(up);

	if (_interpolate_color) {
		colors.push_back(colors[colors.size() - 1]);
	}

	Orientation opposite_orientation = orientation == UP ? DOWN : UP;

	if (texture_mode != Line2D::LINE_TEXTURE_NONE) {
		// UVs are just one slice of the texture all along
		// (otherwise we can't share the bottom vertice)
		uvs.push_back(uvs[_last_index[opposite_orientation]]);
	}

	indices.push_back(_last_index[opposite_orientation], vi, _last_index[orientation]);

	_last_index[opposite_orientation] = vi;
}

void LineBuilder::strip_add_arc(Vector2 center, float angle_delta, Orientation orientation) {
	// Take the two last vertices and extrude an arc made of triangles
	// that all share one of the initial vertices

	Orientation opposite_orientation = orientation == UP ? DOWN : UP;
	Vector2 vbegin = vertices[_last_index[opposite_orientation]] - center;
	float radius = vbegin.length();
	float angle_step = Math_PI / static_cast<float>(round_precision);
	float steps = Math::abs(angle_delta) / angle_step;

	if (angle_delta < 0.f) {
		angle_step = -angle_step;
	}

	float t = Vector2(1, 0).angle_to(vbegin);
	float end_angle = t + angle_delta;
	Vector2 rpos(0, 0);

	// Arc vertices
	for (int ti = 0; ti < steps; ++ti, t += angle_step) {
		rpos = center + Vector2(Math::cos(t), Math::sin(t)) * radius;
		strip_add_tri(rpos, orientation);
	}

	// Last arc vertice
	rpos = center + Vector2(Math::cos(end_angle), Math::sin(end_angle)) * radius;
	strip_add_tri(rpos, orientation);
}

void LineBuilder::new_arc(Vector2 center, Vector2 vbegin, float angle_delta, Color color, Rect2 uv_rect) {
	// Make a standalone arc that doesn't use existing vertices,
	// with undistorted UVs from within a square section

	float radius = vbegin.length();
	float angle_step = Math_PI / static_cast<float>(round_precision);
	float steps = Math::abs(angle_delta) / angle_step;

	if (angle_delta < 0.f) {
		angle_step = -angle_step;
	}

	float t = Vector2(1, 0).angle_to(vbegin);
	float end_angle = t + angle_delta;
	Vector2 rpos(0, 0);
	float tt_begin = -Math_PI / 2.f;
	float tt = tt_begin;

	// Center vertice
	int vi = vertices.size();
	vertices.push_back(center);
	if (_interpolate_color) {
		colors.push_back(color);
	}

	// Texture is mapping from the center
	// of the uv_rect:
	// +-------+
	// |  /--\ |
	// | | x | |
	// | \--/  |
	// +-------+
	// x = (0.5, 0.5)

	if (_repeat_segment && texture_mode == Line2D::LINE_TEXTURE_TILE) {
		// we donot support texture repeat on arcs,
		// so make sure that texture is not tiling
		// (but might be distorted)
		Vector2 mid = uv_rect.get_center();
		real_t dist = uv_rect.size.width / 2;
		mid.x -= Math::floor(mid.x);
		if (mid.x + dist > 1) {
			dist = 1 - mid.x;
			uv_rect = Rect2(mid.x - dist, uv_rect.position.y, dist * 2, uv_rect.size.height);
		}
	}
	if (texture_mode != Line2D::LINE_TEXTURE_NONE) {
		uvs.push_back(interpolate(uv_rect, Vector2(0.5, 0.5)));
	}

	// Arc vertices
	for (int ti = 0; ti < steps; ++ti, t += angle_step) {
		Vector2 sc = Vector2(Math::cos(t), Math::sin(t));
		rpos = center + sc * radius;

		vertices.push_back(rpos);
		if (_interpolate_color) {
			colors.push_back(color);
		}
		if (texture_mode != Line2D::LINE_TEXTURE_NONE) {
			Vector2 tsc = Vector2(Math::cos(tt), Math::sin(tt));
			uvs.push_back(interpolate(uv_rect, 0.5f * (tsc + Vector2(1.f, 1.f))));
			tt += angle_step;
		}
	}

	// Last arc vertice
	Vector2 sc = Vector2(Math::cos(end_angle), Math::sin(end_angle));
	rpos = center + sc * radius;
	vertices.push_back(rpos);
	if (_interpolate_color) {
		colors.push_back(color);
	}
	if (texture_mode != Line2D::LINE_TEXTURE_NONE) {
		tt = tt_begin + angle_delta;
		Vector2 tsc = Vector2(Math::cos(tt), Math::sin(tt));
		uvs.push_back(interpolate(uv_rect, 0.5f * (tsc + Vector2(1.f, 1.f))));
	}

	// Make up triangles
	int vi0 = vi;
	for (int ti = 0; ti < steps; ++ti) {
		indices.push_back(vi0);
		indices.push_back(++vi);
		indices.push_back(vi + 1);
	}
}

// -- Tests --

#ifdef DOCTEST

namespace {

bool has_nan_or_inf(const Vector<Vector2> &p_vectors) {
	for (int i = 0; i < p_vectors.size(); ++i) {
		if (Math::is_nan(p_vectors[i].x) || Math::is_nan(p_vectors[i].y) ||
				Math::is_inf(p_vectors[i].x) || Math::is_inf(p_vectors[i].y)) {
			return true;
		}
	}
	return false;
}

bool has_nan_or_inf(const Vector<Color> &p_colors) {
	for (int i = 0; i < p_colors.size(); ++i) {
		const Color &c = p_colors[i];
		if (Math::is_nan(c.r) || Math::is_nan(c.g) || Math::is_nan(c.b) || Math::is_nan(c.a) ||
				Math::is_inf(c.r) || Math::is_inf(c.g) || Math::is_inf(c.b) || Math::is_inf(c.a)) {
			return true;
		}
	}
	return false;
}

// Distance from `p_vertex` to the closest of the two given polyline points,
// used to check that a strip vertex sits at (roughly) half-width from the centerline.
float min_distance_to(const Vector2 &p_vertex, const Vector2 &p_a, const Vector2 &p_b) {
	return MIN(p_vertex.distance_to(p_a), p_vertex.distance_to(p_b));
}

int count_degenerate_triangles(const Vector<int> &p_indices, const Vector<Vector2> &p_vertices) {
	int count = 0;
	for (int i = 0; i + 2 < p_indices.size(); i += 3) {
		const Vector2 &a = p_vertices[p_indices[i]];
		const Vector2 &b = p_vertices[p_indices[i + 1]];
		const Vector2 &c = p_vertices[p_indices[i + 2]];
		const real_t area2 = Math::abs((b - a).cross(c - a));
		if (area2 <= CMP_EPSILON) {
			count++;
		}
	}
	return count;
}

} // namespace

TEST_CASE("[LineBuilder] build() with fewer than 2 points clears the output") {
	LineBuilder lb;
	lb.width = 10;

	// Pre-fill the output arrays to make sure build() actually clears them
	// rather than just leaving them untouched.
	lb.vertices.push_back(Vector2());
	lb.colors.push_back(Color());
	lb.indices.push_back(0);
	lb.uvs.push_back(Vector2());

	SUBCASE("zero points") {
		lb.build();
	}
	SUBCASE("one point") {
		lb.points.push_back(Vector2(1, 2));
		lb.build();
	}

	CHECK(lb.vertices.size() == 0);
	CHECK(lb.colors.size() == 0);
	CHECK(lb.indices.size() == 0);
	CHECK(lb.uvs.size() == 0);
}

TEST_CASE("[LineBuilder] straight open line with no texture, no cap") {
	LineBuilder lb;
	lb.points.push_back(Vector2(0, 0));
	lb.points.push_back(Vector2(100, 0));
	lb.width = 10;
	lb.texture_mode = Line2D::LINE_TEXTURE_NONE;
	lb.begin_cap_mode = Line2D::LINE_CAP_NONE;
	lb.end_cap_mode = Line2D::LINE_CAP_NONE;
	lb.build();

	const float hw = 5.f;

	REQUIRE(lb.vertices.size() == 4);
	CHECK(lb.indices.size() == 6);
	CHECK(lb.uvs.size() == 0);
	// A single flat color for the whole primitive rather than one per vertex.
	CHECK(lb.colors.size() == 1);

	const Vector2 f0 = Vector2(1, 0);
	for (int i = 0; i < lb.vertices.size(); ++i) {
		const Vector2 &v = lb.vertices[i];
		const Vector2 &nearest = (v.distance_to(Vector2(0, 0)) < v.distance_to(Vector2(100, 0))) ? Vector2(0, 0) : Vector2(100, 0);
		const Vector2 offset = v - nearest;
		CHECK(offset.length() == doctest::Approx(hw).epsilon(0.001));
		// The offset must be perpendicular to the line direction (no leftover
		// tangential component), i.e. the strip's starting edge is a proper
		// perpendicular edge and not collapsed onto the centerline point.
		CHECK(offset.dot(f0) == doctest::Approx(0.f).epsilon(0.001));
	}
	CHECK_FALSE(has_nan_or_inf(lb.vertices));
}

TEST_CASE("[LineBuilder] gradient assigns per-vertex colors matching the endpoints") {
	Gradient *grad = memnew(Gradient); // Default: black at 0, white at 1.

	LineBuilder lb;
	lb.points.push_back(Vector2(0, 0));
	lb.points.push_back(Vector2(100, 0));
	lb.width = 10;
	lb.gradient = grad;
	lb.build();

	REQUIRE(lb.vertices.size() == 4);
	REQUIRE(lb.colors.size() == 4);
	CHECK(lb.colors[0] == Color(0, 0, 0, 1));
	CHECK(lb.colors[1] == Color(0, 0, 0, 1));
	CHECK(lb.colors[2] == Color(1, 1, 1, 1));
	CHECK(lb.colors[3] == Color(1, 1, 1, 1));

	memdelete(grad);
}

TEST_CASE("[LineBuilder] width curve tapers the half-width along the line") {
	Curve *curve = memnew(Curve);
	curve->add_point(Vector2(0, 1.0));
	curve->add_point(Vector2(1, 0.2));

	LineBuilder lb;
	lb.points.push_back(Vector2(0, 0));
	lb.points.push_back(Vector2(100, 0));
	lb.width = 10;
	lb.curve = curve;
	lb.build();

	REQUIRE(lb.vertices.size() == 4);
	// First two vertices (start edge) should be offset by hw * curve(0) == 5.
	CHECK(min_distance_to(lb.vertices[0], Vector2(0, 0), Vector2(100, 0)) == doctest::Approx(5.0).epsilon(0.02));
	CHECK(min_distance_to(lb.vertices[1], Vector2(0, 0), Vector2(100, 0)) == doctest::Approx(5.0).epsilon(0.02));
	// Last two vertices (end edge) should be offset by hw * curve(1) == 1.
	CHECK(min_distance_to(lb.vertices[2], Vector2(0, 0), Vector2(100, 0)) == doctest::Approx(1.0).epsilon(0.02));
	CHECK(min_distance_to(lb.vertices[3], Vector2(0, 0), Vector2(100, 0)) == doctest::Approx(1.0).epsilon(0.02));

	memdelete(curve);
}

TEST_CASE("[LineBuilder] begin/end box caps extend the endpoints along the tangent") {
	auto build_with_caps = [](Line2D::LineCapMode p_begin, Line2D::LineCapMode p_end) {
		LineBuilder lb;
		lb.points.push_back(Vector2(0, 0));
		lb.points.push_back(Vector2(100, 0));
		lb.width = 10;
		lb.begin_cap_mode = p_begin;
		lb.end_cap_mode = p_end;
		lb.build();
		return lb;
	};

	LineBuilder no_cap = build_with_caps(Line2D::LINE_CAP_NONE, Line2D::LINE_CAP_NONE);
	LineBuilder box_cap = build_with_caps(Line2D::LINE_CAP_BOX, Line2D::LINE_CAP_BOX);

	REQUIRE(no_cap.vertices.size() == 4);
	REQUIRE(box_cap.vertices.size() == 4);

	const Vector2 f0 = Vector2(1, 0);
	const float hw = 5.f;

	// The begin edge (vertices 0/1) must be pushed back by hw along -f0
	// compared to the uncapped version, while staying hw away perpendicularly.
	for (int i = 0; i < 2; ++i) {
		const Vector2 delta = box_cap.vertices[i] - no_cap.vertices[i];
		CHECK(delta.dot(f0) == doctest::Approx(-hw).epsilon(0.01));
	}
	// The end edge (vertices 2/3) must be pushed forward by hw along +f0.
	for (int i = 2; i < 4; ++i) {
		const Vector2 delta = box_cap.vertices[i] - no_cap.vertices[i];
		CHECK(delta.dot(f0) == doctest::Approx(hw).epsilon(0.01));
	}
}

TEST_CASE("[LineBuilder] round begin cap adds an arc fan sized by round_precision") {
	auto build_with_round_cap = [](int p_round_precision) {
		LineBuilder lb;
		lb.points.push_back(Vector2(0, 0));
		lb.points.push_back(Vector2(100, 0));
		lb.width = 10;
		lb.begin_cap_mode = Line2D::LINE_CAP_ROUND;
		lb.end_cap_mode = Line2D::LINE_CAP_NONE;
		lb.round_precision = p_round_precision;
		lb.build();
		return lb;
	};

	LineBuilder lb8 = build_with_round_cap(8);
	// Strip alone would be 4 vertices / 6 indices; the arc fan adds one center
	// vertex plus (round_precision + 1) rim vertices, and round_precision triangles.
	CHECK(lb8.vertices.size() == 4 + (8 + 2));
	CHECK(lb8.indices.size() == 6 + 8 * 3);
	CHECK_FALSE(has_nan_or_inf(lb8.vertices));

	LineBuilder lb4 = build_with_round_cap(4);
	LineBuilder lb16 = build_with_round_cap(16);
	CHECK(lb4.vertices.size() < lb16.vertices.size());
	CHECK(lb4.indices.size() < lb16.indices.size());
}

TEST_CASE("[LineBuilder] joint modes at a right-angle corner") {
	auto build_with_joint = [](Line2D::LineJointMode p_mode) {
		LineBuilder lb;
		lb.points.push_back(Vector2(0, 0));
		lb.points.push_back(Vector2(100, 0));
		lb.points.push_back(Vector2(100, 100));
		lb.width = 10;
		lb.joint_mode = p_mode;
		lb.build();
		return lb;
	};

	LineBuilder sharp = build_with_joint(Line2D::LINE_JOINT_SHARP);
	LineBuilder bevel = build_with_joint(Line2D::LINE_JOINT_BEVEL);
	LineBuilder round = build_with_joint(Line2D::LINE_JOINT_ROUND);

	const float hw = 5.f;
	const Vector2 p0(0, 0), p1(100, 0), p2(100, 100);

	for (const LineBuilder *lb : { &sharp, &bevel, &round }) {
		CHECK_FALSE(has_nan_or_inf(lb->vertices));
		REQUIRE(lb->indices.size() % 3 == 0);
		REQUIRE(lb->indices.size() > 0);
		// Every emitted vertex must stay close to the polyline: none of the
		// joint strategies should be able to fling a corner vertex far away.
		for (int i = 0; i < lb->vertices.size(); ++i) {
			const Vector2 &v = lb->vertices[i];
			float best = MIN(v.distance_to(p0), MIN(v.distance_to(p1), v.distance_to(p2)));
			CHECK(best <= hw * 3.f);
		}
		// All index references must land inside the vertex buffer.
		for (int i = 0; i < lb->indices.size(); ++i) {
			CHECK(lb->indices[i] >= 0);
			CHECK(lb->indices[i] < lb->vertices.size());
		}
	}

	// ROUND approximates the same corner as BEVEL with more, smaller triangles.
	CHECK(round.indices.size() >= bevel.indices.size());
}

TEST_CASE("[LineBuilder] sharp_limit falls back to bevel instead of producing a runaway miter") {
	LineBuilder lb;
	// A near-reversal: the polyline almost folds back onto itself at (100, 0),
	// which would normally produce an extremely long sharp miter.
	lb.points.push_back(Vector2(0, 0));
	lb.points.push_back(Vector2(100, 0));
	lb.points.push_back(Vector2(0, 1));
	lb.width = 10;
	lb.joint_mode = Line2D::LINE_JOINT_SHARP;
	lb.sharp_limit = 2.f;
	lb.build();

	CHECK_FALSE(has_nan_or_inf(lb.vertices));

	// Every vertex must stay close to its source polyline point; none of them
	// should have been pulled out to the (unbounded) miter tip.
	const float hw = 5.f;
	for (int i = 0; i < lb.vertices.size(); ++i) {
		const Vector2 &v = lb.vertices[i];
		float best = 1e30f;
		best = MIN(best, v.distance_to(Vector2(0, 0)));
		best = MIN(best, v.distance_to(Vector2(100, 0)));
		best = MIN(best, v.distance_to(Vector2(0, 1)));
		CHECK(best < hw * 3.f);
	}
}

TEST_CASE("[LineBuilder] closed polyline produces a seamless ring with no degenerate triangles") {
	LineBuilder lb;
	lb.points.push_back(Vector2(0, 0));
	lb.points.push_back(Vector2(100, 0));
	lb.points.push_back(Vector2(100, 100));
	lb.points.push_back(Vector2(0, 100));
	lb.width = 10;
	lb.closed = true;
	lb.joint_mode = Line2D::LINE_JOINT_SHARP;
	lb.build();

	REQUIRE(lb.indices.size() % 3 == 0);
	REQUIRE(lb.indices.size() > 0);
	CHECK_FALSE(has_nan_or_inf(lb.vertices));

	// The wrap-around seam must connect to real geometry, not to stale/unset
	// indices, so no triangle in the ring should have (near) zero area.
	CHECK(count_degenerate_triangles(lb.indices, lb.vertices) == 0);

	// Begin/end cap settings must be ignored entirely for a closed polyline
	// (caps only make sense for the two loose ends of an open line).
	LineBuilder with_caps_requested;
	with_caps_requested.points = lb.points;
	with_caps_requested.width = 10;
	with_caps_requested.closed = true;
	with_caps_requested.joint_mode = Line2D::LINE_JOINT_SHARP;
	with_caps_requested.begin_cap_mode = Line2D::LINE_CAP_ROUND;
	with_caps_requested.end_cap_mode = Line2D::LINE_CAP_ROUND;
	with_caps_requested.build();
	CHECK(lb.vertices.size() == with_caps_requested.vertices.size());
	CHECK(lb.indices.size() == with_caps_requested.indices.size());
}

TEST_CASE("[LineBuilder] stretch texture spans the whole line from 0 to 1") {
	LineBuilder lb;
	lb.points.push_back(Vector2(0, 0));
	lb.points.push_back(Vector2(100, 0));
	lb.width = 10;
	lb.texture_mode = Line2D::LINE_TEXTURE_STRETCH;
	lb.build();

	REQUIRE(lb.uvs.size() == 4);
	CHECK(lb.uvs[0].x == doctest::Approx(0.f));
	CHECK(lb.uvs[1].x == doctest::Approx(0.f));
	CHECK(lb.uvs[2].x == doctest::Approx(1.f));
	CHECK(lb.uvs[3].x == doctest::Approx(1.f));
}

TEST_CASE("[LineBuilder] tile texture without an atlas region grows unbounded") {
	LineBuilder lb;
	lb.points.push_back(Vector2(0, 0));
	lb.points.push_back(Vector2(1000, 0));
	lb.width = 10;
	lb.texture_mode = Line2D::LINE_TEXTURE_TILE;
	lb.tile_aspect = 1.f;
	// tile_region left at the default full rect => _repeat_segment is false,
	// so the atlas-safe slicing in strip_add_quad must never trigger.
	lb.build();

	REQUIRE(lb.vertices.size() == 4);
	REQUIRE(lb.uvs.size() == 4);
	CHECK(lb.uvs[2].x == doctest::Approx(100.f)); // distance / (width * aspect)
	CHECK(lb.uvs[3].x == doctest::Approx(100.f));
}

TEST_CASE("[LineBuilder] tile texture with an atlas sub-region stays inside the region bounds") {
	LineBuilder lb;
	lb.points.push_back(Vector2(0, 0));
	lb.points.push_back(Vector2(35, 0));
	lb.width = 10;
	lb.texture_mode = Line2D::LINE_TEXTURE_TILE;
	lb.tile_aspect = 1.f;
	// Mimics Line2D::_draw()'s AtlasTexture handling: a proper sub-rect of the
	// atlas, not the full [0,1]x[0,1] rect, so _repeat_segment kicks in.
	lb.tile_region = Rect2(0.25, 0.5, 0.25, 0.25);
	lb.build();

	REQUIRE(lb.uvs.size() > 0);
	CHECK_FALSE(has_nan_or_inf(lb.uvs));

	// The raw (pre-atlas) uvx for this line is 35/(10*1) == 3.5, i.e. it spans
	// more than 3 tile repeats. Without per-tile slicing, uvs would run past
	// the atlas sub-region and sample neighboring frames; every emitted UV
	// must instead stay clamped inside the region.
	const float eps = 0.0001f;
	for (int i = 0; i < lb.uvs.size(); ++i) {
		CHECK(lb.uvs[i].x >= lb.tile_region.position.x - eps);
		CHECK(lb.uvs[i].x <= lb.tile_region.position.x + lb.tile_region.size.x + eps);
		CHECK(lb.uvs[i].y >= lb.tile_region.position.y - eps);
		CHECK(lb.uvs[i].y <= lb.tile_region.position.y + lb.tile_region.size.y + eps);
	}

	// Compare against the same line without an atlas region: slicing must
	// have actually inserted extra geometry rather than being a no-op.
	LineBuilder no_atlas;
	no_atlas.points = lb.points;
	no_atlas.width = lb.width;
	no_atlas.texture_mode = Line2D::LINE_TEXTURE_TILE;
	no_atlas.tile_aspect = 1.f;
	no_atlas.build();
	CHECK(lb.vertices.size() > no_atlas.vertices.size());
}

TEST_CASE("[LineBuilder] coincident points do not produce NaN/Inf output") {
	Gradient *grad = memnew(Gradient);

	auto build_degenerate = [&](bool p_three_points) {
		LineBuilder lb;
		lb.points.push_back(Vector2(50, 50));
		lb.points.push_back(Vector2(50, 50));
		if (p_three_points) {
			lb.points.push_back(Vector2(50, 50));
		}
		lb.width = 10;
		lb.gradient = grad;
		lb.texture_mode = Line2D::LINE_TEXTURE_TILE;
		lb.tile_region = Rect2(0.25, 0.5, 0.25, 0.25);
		lb.build();
		return lb;
	};

	SUBCASE("two coincident points") {
		LineBuilder lb = build_degenerate(false);
		CHECK_FALSE(has_nan_or_inf(lb.vertices));
		CHECK_FALSE(has_nan_or_inf(lb.colors));
		CHECK_FALSE(has_nan_or_inf(lb.uvs));
	}
	SUBCASE("three coincident points") {
		LineBuilder lb = build_degenerate(true);
		CHECK_FALSE(has_nan_or_inf(lb.vertices));
		CHECK_FALSE(has_nan_or_inf(lb.colors));
		CHECK_FALSE(has_nan_or_inf(lb.uvs));
	}

	memdelete(grad);
}

#endif // DOCTEST
