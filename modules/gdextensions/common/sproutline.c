/*************************************************************************/
/*  sproutline.c                                                         */
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

/* sproutline - v0.10 - public domain sprite outline detector - http://github.org/ands/sproutline
						no warranty implied; use at your own risk

License:
   This software is in the public domain. Where that dedication is not
   recognized, you are granted a perpetual, irrevocable license to copy
   and modify this file however you want.

*/

#include "sproutline.h"

#include <math.h> // sqrtf, abs

#ifndef S2O_MALLOC
#include <stdlib.h> // malloc
#define S2O_MALLOC(sz) malloc(sz)
#endif

///////////////////////////////////////////////
//
//  locally used types

typedef int s2o_bool;

// 2d point type helpers
#define S2O_POINT_ADD(result, a, b) \
	{                               \
		(result).x = (a).x + (b).x; \
		(result).y = (a).y + (b).y; \
	}
#define S2O_POINT_SUB(result, a, b) \
	{                               \
		(result).x = (a).x - (b).x; \
		(result).y = (a).y - (b).y; \
	}
#define S2O_POINT_IS_INSIDE(a, w, h) ((a).x >= 0 && (a).y >= 0 && (a).x < (w) && (a).y < (h))
#define S2O_POINT_IS_NEXT_TO(a, b) ((a).x - (b).x <= 1 && (a).x - (b).x >= -1 && (a).y - (b).y <= 1 && (a).y - (b).y >= -1)

// direction type
typedef int s2o_direction; // 8 cw directions: >, _|, v, |_, <, |", ^, "|
#define S2O_DIRECTION_OPPOSITE(dir) ((dir + 4) & 7)
static const s2o_point s2o_direction_to_pixel_offset[] = { { 1, 0 }, { 1, -1 }, { 0, -1 }, { -1, -1 }, { -1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 } };

// image manipulation functions
S2ODEF s2o_uc *s2o_rgba_to_alpha(const s2o_uc *data, int w, int h) {
	s2o_uc *result = (s2o_uc *)S2O_MALLOC(w * h);
	int x, y;
	for (y = 0; y < h; y++)
		for (x = 0; x < w; x++)
			result[y * w + x] = data[(y * w + x) * 4 + 3];
	return result;
}

S2ODEF s2o_uc *s2o_alpha_to_thresholded(const s2o_uc *data, int w, int h, s2o_uc threshold) {
	s2o_uc *result = (s2o_uc *)S2O_MALLOC(w * h);
	int x, y;
	for (y = 0; y < h; y++)
		for (x = 0; x < w; x++)
			result[y * w + x] = data[y * w + x] >= threshold ? 255 : 0;
	return result;
}

S2ODEF s2o_uc *s2o_dilate_thresholded(const s2o_uc *data, int w, int h) {
	int x, y, dx, dy, cx, cy;
	s2o_uc *result = (s2o_uc *)S2O_MALLOC(w * h);
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			result[y * w + x] = 0;
			for (dy = -1; dy <= 1; dy++) {
				for (dx = -1; dx <= 1; dx++) {
					cx = x + dx;
					cy = y + dy;
					if (cx >= 0 && cx < w && cy >= 0 && cy < h) {
						if (data[cy * w + cx]) {
							result[y * w + x] = 255;
							dy = 1;
							break;
						}
					}
				}
			}
		}
	}
	return result;
}

S2ODEF s2o_uc *s2o_thresholded_to_outlined(const s2o_uc *data, int w, int h) {
	s2o_uc *result = (s2o_uc *)S2O_MALLOC(w * h);
	int x, y;
	for (x = 0; x < w; x++) {
		result[x] = data[x];
		result[(h - 1) * w + x] = data[(h - 1) * w + x];
	}
	for (y = 1; y < h - 1; y++) {
		result[y * w] = data[y * w];
		for (x = 1; x < w - 1; x++) {
			if (data[y * w + x] &&
					(!data[y * w + x - 1] ||
							!data[y * w + x + 1] ||
							!data[y * w + x - w] ||
							!data[y * w + x + w])) {
				result[y * w + x] = 255;
			} else {
				result[y * w + x] = 0;
			}
		}
		result[y * w + w - 1] = data[y * w + w - 1];
	}
	return result;
}

// outline path procedures
static s2o_bool s2o_find_first_filled_pixel(const s2o_uc *data, int w, int h, s2o_point *first) {
	int x, y;
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			if (data[y * w + x]) {
				first->x = (short)x;
				first->y = (short)y;
				return 1;
			}
		}
	}
	return 0;
}

static s2o_bool s2o_find_next_filled_pixel(const s2o_uc *data, int w, int h, s2o_point current, s2o_direction *dir, s2o_point *next) {
	// turn around 180°, then make a clockwise scan for a filled pixel
	*dir = S2O_DIRECTION_OPPOSITE(*dir);
	int i;
	for (i = 0; i < 8; i++) {
		S2O_POINT_ADD(*next, current, s2o_direction_to_pixel_offset[*dir]);

		if (S2O_POINT_IS_INSIDE(*next, w, h) && data[next->y * w + next->x])
			return 1;

		// move to next angle (clockwise)
		*dir = *dir - 1;
		if (*dir < 0)
			*dir = 7;
	}
	return 0;
}

S2ODEF s2o_point *s2o_extract_outline_path(s2o_uc *data, int w, int h, int *point_count, s2o_point *reusable_outline) {
	s2o_point *outline = reusable_outline;
	if (!outline)
		outline = (s2o_point *)S2O_MALLOC(w * h * sizeof(s2o_point));

	s2o_point current, next;

restart:
	if (!s2o_find_first_filled_pixel(data, w, h, &current)) {
		*point_count = 0;
		return outline;
	}

	int count = 0;
	s2o_direction dir = 0;

	while (S2O_POINT_IS_INSIDE(current, w, h)) {
		data[current.y * w + current.x] = 0; // clear the visited path
		outline[count++] = current; // add our current point to the outline
		if (!s2o_find_next_filled_pixel(data, w, h, current, &dir, &next)) {
			// find loop connection
			s2o_bool found = 0;
			int i;
			for (i = 0; i < count / 2; i++) // only allow big loops
			{
				if (S2O_POINT_IS_NEXT_TO(current, outline[i])) {
					found = 1;
					break;
				}
			}

			if (found) {
				break;
			} else {
				// go backwards until we see outline pixels again
				dir = S2O_DIRECTION_OPPOSITE(dir);
				count--; // back up
				int prev;
				for (prev = count; prev >= 0; prev--) {
					current = outline[prev];
					outline[count++] = current; // add our current point to the outline again
					if (s2o_find_next_filled_pixel(data, w, h, current, &dir, &next))
						break;
				}
			}
		}
		current = next;
	}

	if (count <= 2) // too small, discard and try again!
		goto restart;
	*point_count = count;
	return outline;
}

S2ODEF void s2o_distance_based_path_simplification(s2o_point *outline, int *outline_length, float distance_threshold) {
	int length = *outline_length;
	int l;
	for (l = length / 2 /*length - 1*/; l > 1; l--) {
		int a, b = l;
		for (a = 0; a < length; a++) {
			s2o_point ab;
			S2O_POINT_SUB(ab, outline[b], outline[a]);
			float lab = sqrtf((float)(ab.x * ab.x + ab.y * ab.y));
			float ilab = 1.0f / lab;
			float abnx = ab.x * ilab, abny = ab.y * ilab;

			if (lab != 0.0f) {
				s2o_bool found = 1;
				int i = (a + 1) % length;
				while (i != b) {
					s2o_point ai;
					S2O_POINT_SUB(ai, outline[i], outline[a]);
					float t = (abnx * ai.x + abny * ai.y) * ilab;
					float distance = -abny * ai.x + abnx * ai.y;
					if (t < 0.0f || t > 1.0f || distance > distance_threshold || -distance > distance_threshold) {
						found = 0;
						break;
					}

					if (++i == length)
						i = 0;
				}

				if (found) {
					int i;
					if (a < b) {
						for (i = 0; i < length - b; i++)
							outline[a + i + 1] = outline[b + i];
						length -= b - a - 1;
					} else {
						length = a - b + 1;
						for (i = 0; i < length; i++)
							outline[i] = outline[b + i];
					}
					if (l >= length)
						l = length - 1;
				}
			}

			if (++b >= length)
				b = 0;
		}
	}
	*outline_length = length;
}
