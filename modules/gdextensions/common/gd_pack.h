/*************************************************************************/
/*  gd_pack.h                                                            */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef GD_PACK_H
#define GD_PACK_H

#include <algorithm>
#include <cstring>
#include <vector>

#include "core/image.h"

/* Of your interest:

1. rect_xywhf - structure representing your rectangle object members:
	int x, y, w, h;
	bool flipped;

2. bin - structure representing resultant bin object
3. bool pack(rect_xywhf* const * v, int n, int max_side, std::vector<bin>& bins) - actual packing function
   Arguments:
	input/output: v - pointer to array of pointers to your rectangles (const here means that the pointers will point to the same rectangles after the call)
	input: n - rectangles count

	input: max_side - maximum bins' side - algorithm works with square bins (in the end it may trim them to rectangular form).
	for the algorithm to finish faster, pass a reasonable value (unreasonable would be passing 1 000 000 000 for packing 4 50x50 rectangles).
	output: bins - vector to which the function will push_back() created bins, each of them containing vector to pointers of rectangles from "v" belonging to that particular bin.
	Every bin also keeps information about its width and height of course, none of the dimensions is bigger than max_side.

	returns true on success, false if one of the rectangles' dimension was bigger than max_side

You want to your rectangles representing your textures/glyph objects with GL_MAX_TEXTURE_SIZE as max_side,
then for each bin iterate through its rectangles, typecast each one to your own structure (or manually add userdata) and then memcpy its pixel contents (rotated by 90 degrees if "flipped" rect_xywhf's member is true)
to the array representing your texture atlas to the place specified by the rectangle, then finally upload it with glTexImage2D.

Algorithm doesn't create any new rectangles.
You just pass an array of pointers - rectangles' x/y/w/h/flipped are modified in place.
There is a vector of pointers for every resultant bin to let you know which ones belong to the particular bin.
The algorithm may swap the w and h fields for the sake of better fitting, the flag "flipped" will be set to true whenever this occurs.

For description how to tune the algorithm and how it actually works see the .cpp file. */

struct rect_ltrb {
	rect_ltrb() :
			l(0), t(0), r(0), b(0) {}
	rect_ltrb(int l, int t, int r, int b) :
			l(l), t(t), r(r), b(b) {}
	int l, t, r, b;
	int w() const { return r - l; }
	int h() const { return b - t; }
	int area() const { return w() * h(); }
	int perimeter() const { return 2 * w() + 2 * h(); }
	void w(int ww) { r = l + ww; }
	void h(int hh) { b = t + hh; }
};

struct rect_wh {
	rect_wh(const rect_ltrb &rr) :
			w(rr.w()), h(rr.h()) {}
	rect_wh(int w = 0, int h = 0) :
			w(w), h(h) {}
	int w, h;
	int area() const { return w * h; }
	int perimeter() const { return 2 * w + 2 * h; }
	int fits(const rect_wh &bigger, bool allowFlip) const // 0 - no, 1 - yes, 2 - flipped, 3 - perfectly, 4 perfectly flipped
	{
		if (w == bigger.w && h == bigger.h)
			return 3;
		if (allowFlip && h == bigger.w && w == bigger.h)
			return 4;
		if (w <= bigger.w && h <= bigger.h)
			return 1;
		if (allowFlip && h <= bigger.w && w <= bigger.h)
			return 2;
		return 0;
	}
	Size2i size() const { return Size2i(w, h); }
};

struct rect_xywh : public rect_wh {
	rect_xywh() :
			x(0), y(0) {}
	rect_xywh(const rect_ltrb &rc) :
			x(rc.l), y(rc.t) {
		b(rc.b);
		r(rc.r);
	}
	rect_xywh(int x, int y, int w, int h) :
			rect_wh(w, h), x(x), y(y) {}
	operator rect_ltrb() {
		rect_ltrb rr(x, y, 0, 0);
		rr.w(w);
		rr.h(h);
		return rr;
	}
	int x, y;
	int r() const { return x + w; }
	int b() const { return y + h; }
	void r(int right) { w = right - x; }
	void b(int bottom) { h = bottom - y; }
};

struct rect_xywhf : public rect_xywh {
	rect_xywhf(const rect_ltrb &rr) :
			rect_xywh(rr), flipped(false) {}
	rect_xywhf(int x, int y, int w, int h) :
			rect_xywh(x, y, w, h), flipped(false) {}
	rect_xywhf() :
			flipped(false) {}
	void flip() {
		flipped = !flipped;
		std::swap(w, h);
	}
	bool flipped;
	int bin;
	Ref<Image> atlas_image;
	Ref<Image> original_image;
};

struct bin {
	rect_wh size;
	std::vector<rect_xywhf *> rects;
};

static inline bool area(rect_xywhf *a, rect_xywhf *b) {
	return a->area() > b->area();
}
static inline bool perimeter(rect_xywhf *a, rect_xywhf *b) {
	return a->perimeter() > b->perimeter();
}
static inline bool max_side(rect_xywhf *a, rect_xywhf *b) {
	return std::max(a->w, a->h) > std::max(b->w, b->h);
}
static inline bool max_width(rect_xywhf *a, rect_xywhf *b) {
	return a->w > b->w;
}
static inline bool max_height(rect_xywhf *a, rect_xywhf *b) {
	return a->h > b->h;
}

struct TextureMergeOptions {
	int max_atlas_size = 512;
	Color background_color = Color(0, 0, 0, 0);
	int margin = 2;
	bool power_of_two = false;

	TextureMergeOptions() {}
	TextureMergeOptions(int max_atlas_size) :
			max_atlas_size(max_atlas_size) {}
};

// Merge images from 'images' array using `names` as unique
// identifier for each image.
// --------------------------------------------------------
// Return dictionary:
//    "_generated_images" -> array of generated atlas images
//    "_rects" -> dictionary of atlas rects, eg:
//       "_rects[<image name>]:
//          "rect" -> Rect2 of image rect on atlas
//          "atlas_page" -> atlas page index (of '_generated_images')
//          "atlas" -> atlas image reference
Dictionary merge_images(Vector<Ref<Image>> images, Vector<String> names, const TextureMergeOptions &options = TextureMergeOptions());

#endif // GD_PACK_H
