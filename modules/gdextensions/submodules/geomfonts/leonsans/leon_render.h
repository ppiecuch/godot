/**************************************************************************/
/*  leon_render.h                                                         */
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

#ifndef LEON_RENDER_H
#define LEON_RENDER_H

#include "leonsans.h"

#include "core/math/vector2.h"
#include "core/pool_vector.h"
#include "core/ustring.h"

#include <vector>

// --- Character lookup ---

inline static const FontData *leon_lookup_char(CharType c) {
	char32_t ch = (char32_t)c;
	auto it = SPECIAL.find(ch);
	if (it != SPECIAL.end())
		return &it->second;
	if (ch >= 'A' && ch <= 'Z') {
		it = UPPER.find(ch);
		if (it != UPPER.end())
			return &it->second;
	}
	if (ch >= 'a' && ch <= 'z') {
		it = LOWER.find(ch);
		if (it != LOWER.end())
			return &it->second;
	}
	if (ch >= '0' && ch <= '9') {
		it = NUMBER.find(ch);
		if (it != NUMBER.end())
			return &it->second;
	}
	it = LATIN.find(ch);
	if (it != LATIN.end())
		return &it->second;
	it = SPECIAL.find(char32_t(TOFU));
	if (it != SPECIAL.end())
		return &it->second;
	return nullptr;
}

// --- Cubic bezier evaluation (De Casteljau) ---

inline static Vector2 cubic_bezier_point(
		real_t x0, real_t y0,
		real_t x1, real_t y1,
		real_t x2, real_t y2,
		real_t x3, real_t y3,
		real_t t) {
	const real_t u = 1.0f - t;
	const real_t tt = t * t;
	const real_t uu = u * u;
	const real_t uuu = uu * u;
	const real_t ttt = tt * t;
	return Vector2(
			uuu * x0 + 3.0f * uu * t * x1 + 3.0f * u * tt * x2 + ttt * x3,
			uuu * y0 + 3.0f * uu * t * y1 + 3.0f * u * tt * y2 + ttt * y3);
}

// --- Weight system ---

inline static real_t leon_weight_to_offset(real_t weight) {
	return (CLAMP(weight, 1.0f, 900.0f) - 1.0f) / 899.0f * 70.0f;
}

// Left/right ratio selection uses x <= 0 because coordinates are pre-centered by setCenter().
inline static void leon_apply_weight_to_point(
		real_t &x, real_t &y,
		const FontPathSeg &seg,
		const FontData &glyph,
		real_t fontW) {
	auto fit = seg.info.find('f');
	if (fit != seg.info.end() && int(fit->second) == 1)
		return;

	real_t rx, ry;
	auto xit = seg.info.find('x');
	if (xit != seg.info.end()) {
		rx = real_t(xit->second);
	} else {
		rx = (x <= 0) ? glyph.ratio.x1 : glyph.ratio.x2;
	}

	auto yit = seg.info.find('y');
	if (yit != seg.info.end()) {
		ry = real_t(yit->second);
	} else {
		ry = (y <= 0) ? glyph.ratio.y1 : glyph.ratio.y2;
	}

	x += fontW * rx;
	y += fontW * ry;
}

// --- Text metrics ---

inline static real_t leon_text_width(const String &text, real_t size) {
	const real_t scale = size / FONT_HEIGHT;
	real_t total_w = 0;
	for (int i = 0; i < text.length(); i++) {
		const FontData *fd = leon_lookup_char(text[i]);
		if (fd) {
			total_w += fd->rect.w * scale;
		}
	}
	return total_w;
}

// --- Line mesh generation for PRIMITIVE_LINES ---

inline static int leon_make_lines(
		const String &text,
		real_t size,
		real_t weight,
		PoolVector2Array &out_verts) {
	const real_t scale = size / FONT_HEIGHT;
	const real_t fontW = leon_weight_to_offset(weight);
	real_t cursor_x = 0;
	const int BEZIER_STEPS = 20;
	const int ARC_STEPS = 12;
	const real_t DOT_RADIUS = 4.0f;

	for (int ci = 0; ci < text.length(); ci++) {
		const FontData *fd = leon_lookup_char(text[ci]);
		if (!fd)
			continue;

		for (size_t pi = 0; pi < fd->p.size(); pi++) {
			const FontPath &path = fd->p[pi];
			Vector2 cur(0, 0);
			bool has_cur = false;

			for (size_t si = 0; si < path.v.size(); si++) {
				const FontPathSeg &seg = path.v[si];

				switch (seg.op) {
					case 'm': {
						real_t px = seg._1, py = seg._2;
						leon_apply_weight_to_point(px, py, seg, *fd, fontW);
						cur = Vector2(px, py);
						has_cur = true;
					} break;

					case 'l': {
						real_t px = seg._1, py = seg._2;
						leon_apply_weight_to_point(px, py, seg, *fd, fontW);
						Vector2 target(px, py);
						if (has_cur) {
							out_verts.push_back(Vector2(
									(cur.x + cursor_x) * scale,
									cur.y * scale));
							out_verts.push_back(Vector2(
									(target.x + cursor_x) * scale,
									target.y * scale));
						}
						cur = target;
						has_cur = true;
					} break;

					case 'b': {
						real_t cp1x = seg._1, cp1y = seg._2;
						real_t cp2x = seg._3, cp2y = seg._4;
						real_t endx = seg._5, endy = seg._6;
						leon_apply_weight_to_point(cp1x, cp1y, seg, *fd, fontW);
						leon_apply_weight_to_point(cp2x, cp2y, seg, *fd, fontW);
						leon_apply_weight_to_point(endx, endy, seg, *fd, fontW);

						if (has_cur) {
							Vector2 prev = cur;
							for (int s = 1; s <= BEZIER_STEPS; s++) {
								real_t t = real_t(s) / BEZIER_STEPS;
								Vector2 pt = cubic_bezier_point(
										cur.x, cur.y,
										cp1x, cp1y, cp2x, cp2y,
										endx, endy, t);
								out_verts.push_back(Vector2(
										(prev.x + cursor_x) * scale,
										prev.y * scale));
								out_verts.push_back(Vector2(
										(pt.x + cursor_x) * scale,
										pt.y * scale));
								prev = pt;
							}
						}
						cur = Vector2(endx, endy);
						has_cur = true;
					} break;

					case 'a': {
						real_t cx = seg._1, cy = seg._2;
						leon_apply_weight_to_point(cx, cy, seg, *fd, fontW);
						real_t r = DOT_RADIUS + fontW * 0.1f;
						for (int s = 0; s < ARC_STEPS; s++) {
							real_t a0 = Math_TAU * s / ARC_STEPS;
							real_t a1 = Math_TAU * (s + 1) / ARC_STEPS;
							out_verts.push_back(Vector2(
									(cx + Math::cos(a0) * r + cursor_x) * scale,
									(cy + Math::sin(a0) * r) * scale));
							out_verts.push_back(Vector2(
									(cx + Math::cos(a1) * r + cursor_x) * scale,
									(cy + Math::sin(a1) * r) * scale));
						}
						cur = Vector2(cx, cy);
						has_cur = true;
					} break;
				}
			}
		}
		cursor_x += fd->rect.w;
	}
	return out_verts.size();
}

// --- Path sampling for effects (metaball, plants, morphing) ---

struct LeonPathPoint {
	real_t x, y;
	char type;
	real_t rotation;
	bool start;
};

inline static void leon_sample_paths(
		const String &text,
		real_t size,
		real_t weight,
		real_t path_gap,
		std::vector<LeonPathPoint> &out_points) {
	const real_t scale = size / FONT_HEIGHT;
	const real_t fontW = leon_weight_to_offset(weight);
	real_t cursor_x = 0;

	for (int ci = 0; ci < text.length(); ci++) {
		const FontData *fd = leon_lookup_char(text[ci]);
		if (!fd)
			continue;

		for (size_t pi = 0; pi < fd->p.size(); pi++) {
			const FontPath &path = fd->p[pi];
			Vector2 cur(0, 0);
			bool has_cur = false;
			bool first_in_path = true;

			for (size_t si = 0; si < path.v.size(); si++) {
				const FontPathSeg &seg = path.v[si];

				switch (seg.op) {
					case 'm': {
						real_t px = seg._1, py = seg._2;
						leon_apply_weight_to_point(px, py, seg, *fd, fontW);
						cur = Vector2(px, py);
						has_cur = true;

						real_t rot = ROTATE_NONE;
						auto rit = seg.info.find('r');
						if (rit != seg.info.end())
							rot = real_t(rit->second);

						LeonPathPoint lp;
						lp.x = (cur.x + cursor_x) * scale;
						lp.y = cur.y * scale;
						lp.type = 'm';
						lp.rotation = rot;
						lp.start = first_in_path;
						out_points.push_back(lp);
						first_in_path = false;
					} break;

					case 'l': {
						real_t px = seg._1, py = seg._2;
						leon_apply_weight_to_point(px, py, seg, *fd, fontW);
						Vector2 target(px, py);

						if (has_cur) {
							Vector2 from_sc((cur.x + cursor_x) * scale, cur.y * scale);
							Vector2 to_sc((target.x + cursor_x) * scale, target.y * scale);
							real_t seg_len = (to_sc - from_sc).length();
							int steps = MAX(1, int(seg_len / path_gap));
							real_t rot = Math::atan2(to_sc.y - from_sc.y, to_sc.x - from_sc.x);

							for (int s = 1; s <= steps; s++) {
								real_t t = real_t(s) / steps;
								Vector2 pt = from_sc.linear_interpolate(to_sc, t);
								LeonPathPoint lp;
								lp.x = pt.x;
								lp.y = pt.y;
								lp.type = 'l';
								lp.rotation = rot;
								lp.start = false;
								out_points.push_back(lp);
							}
						}
						cur = target;
						has_cur = true;
					} break;

					case 'b': {
						real_t cp1x = seg._1, cp1y = seg._2;
						real_t cp2x = seg._3, cp2y = seg._4;
						real_t endx = seg._5, endy = seg._6;
						leon_apply_weight_to_point(cp1x, cp1y, seg, *fd, fontW);
						leon_apply_weight_to_point(cp2x, cp2y, seg, *fd, fontW);
						leon_apply_weight_to_point(endx, endy, seg, *fd, fontW);

						if (has_cur) {
							const int LEN_SAMPLES = 20;
							real_t approx_len = 0;
							Vector2 prev_pt(cur.x, cur.y);
							for (int s = 1; s <= LEN_SAMPLES; s++) {
								real_t t = real_t(s) / LEN_SAMPLES;
								Vector2 pt = cubic_bezier_point(
										cur.x, cur.y,
										cp1x, cp1y, cp2x, cp2y,
										endx, endy, t);
								approx_len += (pt - prev_pt).length();
								prev_pt = pt;
							}
							approx_len *= scale;

							int steps = MAX(2, int(approx_len / path_gap));
							for (int s = 1; s <= steps; s++) {
								real_t t = real_t(s) / steps;
								Vector2 pt = cubic_bezier_point(
										cur.x, cur.y,
										cp1x, cp1y, cp2x, cp2y,
										endx, endy, t);
								real_t rot = -Math::atan2(
										bezierTangent(cur.x, cp1x, cp2x, endx, t),
										bezierTangent(cur.y, cp1y, cp2y, endy, t));
								LeonPathPoint lp;
								lp.x = (pt.x + cursor_x) * scale;
								lp.y = pt.y * scale;
								lp.type = 'b';
								lp.rotation = rot;
								lp.start = false;
								out_points.push_back(lp);
							}
						}
						cur = Vector2(endx, endy);
						has_cur = true;
					} break;

					case 'a': {
						real_t cx = seg._1, cy = seg._2;
						leon_apply_weight_to_point(cx, cy, seg, *fd, fontW);
						LeonPathPoint lp;
						lp.x = (cx + cursor_x) * scale;
						lp.y = cy * scale;
						lp.type = 'a';
						lp.rotation = 0;
						lp.start = first_in_path;
						out_points.push_back(lp);
						cur = Vector2(cx, cy);
						has_cur = true;
						first_in_path = false;
					} break;
				}
			}
		}
		cursor_x += fd->rect.w;
	}
}

#endif // LEON_RENDER_H
