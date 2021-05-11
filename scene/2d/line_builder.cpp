/*************************************************************************/
/*  line_builder.cpp                                                     */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "line_builder.h"

//----------------------------------------------------------------------------
// Util
//----------------------------------------------------------------------------

#define PUSH_BACK_IF(nm, op)                             \
	template <typename T>                                \
	void push_back_if_##nm(Vector<T> &p_arr, T p_elem) { \
		if (p_elem op) p_arr.push_back(p_elem);          \
	}

PUSH_BACK_IF(gtzero, > 0);
PUSH_BACK_IF(nonzero, != 0);
PUSH_BACK_IF(lezero, < 0);

enum SegmentIntersectionResult {
	SEGMENT_PARALLEL = 0,
	SEGMENT_NO_INTERSECT = 1,
	SEGMENT_INTERSECT = 2
};

static SegmentIntersectionResult segment_intersection(
		Vector2 a, Vector2 b, Vector2 c, Vector2 d,
		Vector2 *out_intersection) {
	// http://paulbourke.net/geometry/pointlineplane/ <-- Good stuff
	Vector2 cd = d - c;
	Vector2 ab = b - a;
	float div = cd.y * ab.x - cd.x * ab.y;

	if (Math::abs(div) > 0.001f) {
		float ua = (cd.x * (a.y - c.y) - cd.y * (a.x - c.x)) / div;
		float ub = (ab.x * (a.y - c.y) - ab.y * (a.x - c.x)) / div;
		*out_intersection = a + ua * ab;
		if (ua >= 0.f && ua <= 1.f &&
				ub >= 0.f && ub <= 1.f) {
			return SEGMENT_INTERSECT;
		}
		return SEGMENT_NO_INTERSECT;
	}

	return SEGMENT_PARALLEL;
}

static Vector2 Failed(NAN, NAN);

static Vector2 find_intersection(const Vector2 &p0, const Vector2 &p1, const Vector2 &p2, const Vector2 &p3) {
	const real_t s10_x = p1.x - p0.x;
	const real_t s10_y = p1.y - p0.y;
	const real_t s32_x = p3.x - p2.x;
	const real_t s32_y = p3.y - p2.y;

	const real_t denom = s10_x * s32_y - s32_x * s10_y;

	if (denom == 0) return Failed; // collinear

	const bool denom_is_positive = denom > 0;

	const real_t s02_x = p0.x - p2.x;
	const real_t s02_y = p0.y - p2.y;

	const real_t s_numer = s10_x * s02_y - s10_y * s02_x;

	if ((s_numer < 0) == denom_is_positive) return Failed; // no collision

	const real_t t_numer = s32_x * s02_y - s32_y * s02_x;

	if ((t_numer < 0) == denom_is_positive) return Failed; // no collision
	if ((s_numer > denom) == denom_is_positive || (t_numer > denom) == denom_is_positive) return Failed; // no collision

	// collision detected

	const real_t t = t_numer / denom;
	return Vector2(p0.x + (t * s10_x), p0.y + (t * s10_y));
}

// TODO I'm pretty sure there is an even faster way to swap things
template <typename T>
static inline void swap(T &a, T &b) {
	T tmp = a;
	a = b;
	b = tmp;
}

static float calculate_total_distance(const Vector<Vector2> &points) {
	float d = 0.f;
	for (int i = 1; i < points.size(); ++i) {
		d += points[i].distance_to(points[i - 1]);
	}
	return d;
}

static inline Vector2 rotate90(const Vector2 &v) {
	// Note: the 2D referential is X-right, Y-down
	return Vector2(v.y, -v.x);
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
	sharp_limit = 2.f;
	round_precision = 8;
	begin_cap_mode = Line2D::LINE_CAP_NONE;
	end_cap_mode = Line2D::LINE_CAP_NONE;
	tile_aspect = 1.f;
	tile_region = Rect2(0, 0, 1.f, 1.f);

	_interpolate_color = false;
	_last_index[0] = 0;
	_last_index[1] = 0;
}

void LineBuilder::clear_output() {
	vertices.clear();
	colors.clear();
	indices.clear();
	uvs.clear();
}

void LineBuilder::build() {
	// Need at least 2 points to draw a line
	if (points.size() < 2) {
		clear_output();
		return;
	}

	ERR_FAIL_COND(tile_aspect <= 0.f);

	const float hw = width / 2.f;
	const float hw_sq = hw * hw;
	const float sharp_limit_sq = sharp_limit * sharp_limit;
	const int len = points.size();

	// Initial values

	Vector2 pos0 = points[0];
	Vector2 pos1 = points[1];
	Vector2 f0 = (pos1 - pos0).normalized();
	Vector2 u0 = rotate90(f0);
	Vector2 pos_up0 = pos0;
	Vector2 pos_down0 = pos0;

	Color color0;
	Color color1;

	float current_distance0 = 0.f;
	float current_distance1 = 0.f;
	float total_distance = 0.f;
	float width_factor = 1.f;
	_interpolate_color = gradient != nullptr;
	_repeat_segment = tile_region != Rect2(0, 0, 1.f, 1.f);
	_last_uvx = 0;
	bool retrieve_curve = curve != nullptr;
	bool distance_required = _interpolate_color ||
							 retrieve_curve ||
							 texture_mode == Line2D::LINE_TEXTURE_TILE ||
							 texture_mode == Line2D::LINE_TEXTURE_STRETCH;
	if (distance_required) {
		total_distance = calculate_total_distance(points);
		// Adjust totalDistance.
		// The line's outer length will be a little higher due to begin and end caps
		if (begin_cap_mode == Line2D::LINE_CAP_BOX || begin_cap_mode == Line2D::LINE_CAP_ROUND) {
			if (retrieve_curve) {
				total_distance += width * curve->interpolate_baked(0.f) * 0.5f;
			} else {
				total_distance += width * 0.5f;
			}
		}
		if (end_cap_mode == Line2D::LINE_CAP_BOX || end_cap_mode == Line2D::LINE_CAP_ROUND) {
			if (retrieve_curve) {
				total_distance += width * curve->interpolate_baked(1.f) * 0.5f;
			} else {
				total_distance += width * 0.5f;
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

	if (retrieve_curve) {
		width_factor = curve->interpolate_baked(0.f);
	}

	pos_up0 += u0 * hw * width_factor;
	pos_down0 -= u0 * hw * width_factor;

	// Begin cap
	if (begin_cap_mode == Line2D::LINE_CAP_BOX) {
		// Push back first vertices a little bit
		pos_up0 -= f0 * hw * width_factor;
		pos_down0 -= f0 * hw * width_factor;

		current_distance0 += hw * width_factor;
		current_distance1 = current_distance0;
	} else if (begin_cap_mode == Line2D::LINE_CAP_ROUND) {
		if (texture_mode == Line2D::LINE_TEXTURE_TILE) {
			uvx0 = width_factor * 0.5f / tile_aspect;
		} else if (texture_mode == Line2D::LINE_TEXTURE_STRETCH) {
			uvx0 = width * width_factor / total_distance;
		}
		new_arc(pos0, pos_up0 - pos0, -Math_PI, color0, Rect2(0.f, 0.f, uvx0 * 2, 1.f));
		current_distance0 += hw * width_factor;
		current_distance1 = current_distance0;
	}

	strip_begin(pos_up0, pos_down0, color0, uvx0);

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

	// For each additional segment
	for (int i = 1; i < len - 1; ++i) {
		pos1 = points[i];
		Vector2 pos2 = points[i + 1];

		Vector2 f1 = (pos2 - pos1).normalized();
		Vector2 u1 = rotate90(f1);

		// Determine joint orientation
		const float dp = u0.dot(f1);
		const Orientation orientation = (dp > 0.f ? UP : DOWN);

		if (distance_required) {
			current_distance1 += pos0.distance_to(pos1);
		}
		if (_interpolate_color) {
			color1 = gradient->get_color_at_offset(current_distance1 / total_distance);
		}
		if (retrieve_curve) {
			width_factor = curve->interpolate_baked(current_distance1 / total_distance);
		}

		Vector2 inner_normal0, inner_normal1;
		if (orientation == UP) {
			inner_normal0 = u0 * hw * width_factor;
			inner_normal1 = u1 * hw * width_factor;
		} else {
			inner_normal0 = -u0 * hw * width_factor;
			inner_normal1 = -u1 * hw * width_factor;
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
		SegmentIntersectionResult intersection_result = segment_intersection(
				pos0 + inner_normal0, pos1 + inner_normal0,
				pos1 + inner_normal1, pos2 + inner_normal1,
				&corner_pos_in);

		if (intersection_result == SEGMENT_INTERSECT) {
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
		if (intersection_result == SEGMENT_INTERSECT) {
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
					pos_down1 = pos1 - u0 * hw * width_factor;
				} else {
					pos_up1 = pos1 + u0 * hw * width_factor;
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

		// Add current line body quad
		// Triangles are clockwise
		if (texture_mode == Line2D::LINE_TEXTURE_TILE) {
			uvx1 = current_distance1 / (width * tile_aspect);
		} else if (texture_mode == Line2D::LINE_TEXTURE_STRETCH) {
			uvx1 = current_distance1 / total_distance;
		}

		strip_add_quad(pos_up1, pos_down1, color1, uvx1);

		// Swap vars for use in the next line
		color0 = color1;
		u0 = u1;
		f0 = f1;
		pos0 = pos1;
		if (intersection_result == SEGMENT_INTERSECT) {
			if (current_joint_mode == Line2D::LINE_JOINT_SHARP) {
				pos_up0 = pos_up1;
				pos_down0 = pos_down1;
			} else {
				if (orientation == UP) {
					pos_up0 = corner_pos_up;
					pos_down0 = pos1 - u1 * hw * width_factor;
				} else {
					pos_up0 = pos1 + u1 * hw * width_factor;
					pos_down0 = corner_pos_down;
				}
			}
		} else {
			pos_up0 = pos1 + u1 * hw * width_factor;
			pos_down0 = pos1 - u1 * hw * width_factor;
		}
		// From this point, bu0 and bd0 concern the next segment

		// Add joint geometry
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

			if (current_joint_mode == Line2D::LINE_JOINT_BEVEL) {
				strip_add_tri(cend, orientation);
			} else if (current_joint_mode == Line2D::LINE_JOINT_ROUND) {
				Vector2 vbegin = cbegin - pos1;
				Vector2 vend = cend - pos1;
				strip_add_arc(pos1, vbegin.angle_to(vend), orientation);
			}

			if (intersection_result != SEGMENT_INTERSECT) {
				// In this case the joint is too corrputed to be re-used,
				// start again the strip with fallback points
				strip_begin(pos_up0, pos_down0, color1, uvx1);
			}
		}
	}
	// Last (or only) segment
	pos1 = points[points.size() - 1];

	if (distance_required) {
		current_distance1 += pos0.distance_to(pos1);
	}
	if (_interpolate_color) {
		color1 = gradient->get_color(gradient->get_points_count() - 1);
	}
	if (retrieve_curve) {
		width_factor = curve->interpolate_baked(1.f);
	}

	Vector2 pos_up1 = pos1 + u0 * hw * width_factor;
	Vector2 pos_down1 = pos1 - u0 * hw * width_factor;

	// End cap (box)
	if (end_cap_mode == Line2D::LINE_CAP_BOX) {
		pos_up1 += f0 * hw * width_factor;
		pos_down1 += f0 * hw * width_factor;
	}

	if (texture_mode == Line2D::LINE_TEXTURE_TILE) {
		uvx1 = current_distance1 / (width * tile_aspect);
	} else if (texture_mode == Line2D::LINE_TEXTURE_STRETCH) {
		uvx1 = current_distance1 / total_distance;
	}

	strip_add_quad(pos_up1, pos_down1, color1, uvx1);

	// End cap (round)
	if (end_cap_mode == Line2D::LINE_CAP_ROUND) {
		// Note: color is not used in case we don't interpolate...
		Color color = _interpolate_color ? gradient->get_color(gradient->get_points_count() - 1) : Color(0, 0, 0);
		float dist = 0;
		if (texture_mode == Line2D::LINE_TEXTURE_TILE) {
			dist = width_factor / tile_aspect;
		} else if (texture_mode == Line2D::LINE_TEXTURE_STRETCH) {
			dist = width * width_factor / total_distance;
		}
		new_arc(pos1, pos_up1 - pos1, Math_PI, color, Rect2(uvx1 - 0.5f * dist, 0.f, dist, 1.f));
	}

	if (!_repeat_segment)
		return;

	const real_t sx = tile_region.position.x;
	const real_t sy = tile_region.position.y;
	const real_t sw = tile_region.size.x;
	const real_t sh = tile_region.size.y;
	// rescale uvs values
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
		uvs.push_back(Vector2(uvx, 0.f), Vector2(uvx, 1.f));
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

		const float last_remainings = ceil(_last_uvx) - _last_uvx; // remainings of the last tile
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
		const int segs = floor(dist - last_remainings) + is_last_remains;
		const Vector2 full_quad[] = { Vector2(1.f, 0.f), Vector2(1.f, 1.f), Vector2(0.f, 0.f), Vector2(0.f, 1.f) };

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
		uvx -= floor(uvx);
		// nothing left to drawn
		if (uvx == 0)
			return;
	}

	vertices.push_back(up, down);

	if (_interpolate_color) {
		colors.push_multi(2, color);
	}

	if (texture_mode != Line2D::LINE_TEXTURE_NONE) {
		uvs.push_back(Vector2(uvx, 0.f), Vector2(uvx, 1.f));
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
		print_line(vformat("strip_add_tri uvs: %f", uvs[_last_index[opposite_orientation]]));
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
	if (_repeat_segment && texture_mode == Line2D::LINE_TEXTURE_TILE) {
		new_arc_tiled_geometry(center, vbegin, angle_delta, color, uv_rect);
	} else {
		new_arc_tiled_texture(center, vbegin, angle_delta, color, uv_rect);
	}
}

void LineBuilder::new_arc_tiled_texture(Vector2 center, Vector2 vbegin, float angle_delta, Color color, Rect2 uv_rect) {
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
	float tt_begin = -Math_PI / 2.f;
	float tt = tt_begin;

	// Center vertice
	int vi = vertices.size();
	vertices.push_back(center);
	if (_interpolate_color) {
		colors.push_back(color);
	}
	if (texture_mode != Line2D::LINE_TEXTURE_NONE) {
		uvs.push_back(interpolate(uv_rect, Vector2(0.5f, 0.5f)));
	}

	// Arc vertices
	for (int ti = 0; ti < steps; ++ti, t += angle_step) {
		Vector2 sc = Vector2(Math::cos(t), Math::sin(t));
		Vector2 rpos = center + sc * radius;

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
	Vector2 rpos = center + sc * radius;
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

void LineBuilder::new_arc_tiled_geometry(Vector2 center, Vector2 vbegin, float angle_delta, Color color, Rect2 uv_rect) {

	// Make a standalone arc that doesn't use existing vertices,
	// build with stripes not triangle fans,
	// with undistorted UVs from within a square section (possible spans across
	// multiple tiles)

	//               *  stop
	//              /|
	//             / |
	//            /  |
	//           +---+- 0
	//          +----+- 1
	//         /     |
	//        /      |
	//       /       |
	//      +--------+- 0
	//     +---------+- 1
	//    /          |
	//   /           |
	//  /            |
	// +-------------+- start
	// R             C

	float radius = vbegin.length();
	float angle_step = Math_PI / static_cast<float>(round_precision);
	float steps = Math::abs(angle_delta) / angle_step;

	if (angle_delta < 0.f)
		angle_step = -angle_step;

	float t = Vector2(1, 0).angle_to(vbegin);
	const float half_angle = angle_delta / 2;
	const float tt_begin = -Math_PI / 2.f;

	Vector<float> uv_segs;
	// get the number of repeated segments and
	// split texture for each segment
	const float uv0 = uv_rect.position.x;
	const float uv1 = uv_rect.position.x + uv_rect.size.width;
	push_back_if_gtzero(uv_segs, Math::ceil(uv0) - uv0);
	int uv = ceil(uv0);
	while (++uv <= uv1)
		uv_segs.push_back(1);
	push_back_if_gtzero(uv_segs, uv1 - Math::floor(uv1));

	print_line(vformat("new_arc uvs_rect: {%s}, {%s}", String(uv_rect), String(interpolate(uv_rect, Vector2(0.5f, 0.5f)))));
	for (int s = 0; s < uv_segs.size(); ++s) {
		print_line(vformat("%d uv_segs: %f", s, uv_segs[s]));
	}

	// Center vertice
	int vi = vertices.size();
	vertices.push_back(center);
	if (_interpolate_color)
		colors.push_back(color);
	if (texture_mode != Line2D::LINE_TEXTURE_NONE)
		uvs.push_back(interpolate(uv_rect, Vector2(0.5f, 0.5f)));

	// Radius weights
	Vector<float> segs;
	// divide radius for repeated segments + fractional begin/end:
	// | . | ... | ... | .. |
	const float step = radius / uv_rect.size.width;
	float dist = 0;
	for (int s = 0; s < uv_segs.size(); ++s) {
		dist += uv_segs[s];
		segs.push_back(dist * step);
	}
	for (int s = 0; s < segs.size(); ++s) {
		print_line(vformat("%d segs: %f", s, segs[s]));
	}

	auto add_vertex = [=](float tt, const Vector2 &v, const Color &vc) {
		vertices.push_back(v);
		if (_interpolate_color)
			colors.push_back(vc);
		if (texture_mode != Line2D::LINE_TEXTURE_NONE) {
			Vector2 tsc = Vector2(Math::cos(tt_begin + tt), Math::sin(tt_begin + tt));
			uvs.push_back(interpolate(uv_rect, 0.5f * (tsc + Vector2(1.f, 1.f))));
		}
	};

	const Vector2 ho = Vector2(cos(half_angle), sin(half_angle)); // half arc unit vector

	// First arc vertice
	Vector2 last_so = center + Vector2(Math::cos(t), Math::sin(t)) * radius;
	add_vertex(t, last_so, color);

	t += angle_step;

	const int sc = segs.size();
	const int seg_last = sc - 1;

	int seg_index = 0, seg_step = 1;
	Vector2 half_s = center + ho * segs[seg_index]; // band along the half arc
	for (int ti = 1; ti < steps + 1; ++ti, t += angle_step) {
		print_line(vformat("ti %d t %f seg_index %d seg_step %d", ti, t, seg_index, seg_step));
		const Vector2 so = center + Vector2(Math::cos(t), Math::sin(t)) * radius;
		const Vector2 ro(radius * seg_step, 0); // -radius or radius vector
		const Vector2 edge = find_intersection(half_s, half_s + ro, last_so, so);
		if (edge != Failed) {
			add_vertex(t, center + so * radius, color);
			// next band up or down
			seg_index += seg_step;
			if (seg_index == seg_last) {
				seg_step = -1;
				seg_index += seg_step;
			}
			half_s = center + ho * segs[seg_index];
		}
		add_vertex(t, half_s, color);
		add_vertex(t, half_s, color);
		last_so = so;
	}

	print_line(vformat("steps:%d vi0:%d, size:%d, new:%d", steps, vi, vertices.size(), vertices.size() - vi));

	const int vi0 = vi++;
	const int st = sc * 2 - 1;
	for (int ti = 0; ti < steps; ++ti, vi += st) {
		indices.push_back(vi0, vi, vi + st);
		for (int si = 0; si < sc - 1; ++si) {
			indices.push_back(vi + si + 1, vi + si + 2, vi + si + st + 2);
			indices.push_back(vi + si + 1, vi + si + st + 2, vi + si + st + 1);
		}
	}
}
