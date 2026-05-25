/**************************************************************************/
/*  gd_pack.h                                                             */
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

#ifndef GD_PACK_H
#define GD_PACK_H

#include <algorithm>
#include <cstring>
#include <vector>

#include "core/image.h"

// Of your interest:
// -----------------
// 1. rect_xywhf - structure representing your rectangle object members:
//     int x, y, w, h;
//     bool flipped;
// 2. bin - structure representing resultant bin object
// 3. bool pack_rects(rect_xywhf* const * v, int n, int max_side, std::vector<bin>& bins) - actual packing function
//    Arguments:
//     input/output: v - pointer to array of pointers to your rectangles (const here means that the pointers will point to the same rectangles after the call)
//     input: n - rectangles count
//     input: max_side - maximum bins' side - algorithm works with square bins (in the end it may trim them to rectangular form).
//            for the algorithm to finish faster, pass a reasonable value (unreasonable would be passing 1 000 000 000 for packing 4 50x50 rectangles).
//     output: bins - vector to which the function will push_back() created bins, each of them containing vector to pointers of rectangles from "v"
//             belonging to that particular bin. Every bin also keeps information about its width and height of course, none of the dimensions
//             is bigger than max_side.
//   returns true on success, false if one of the rectangles' dimension was bigger than max_side
// 4. Dictionary merge_images(Vector<Ref<Image>> images, Vector<String> names, const TextureMergeOptions &options = TextureMergeOptions()) - Godot packing function
//
// You want to your rectangles representing your textures/glyph objects with GL_MAX_TEXTURE_SIZE as max_side,
// then for each bin iterate through its rectangles, typecast each one to your own structure (or manually add userdata)
// and then memcpy its pixel contents (rotated by 90 degrees if "flipped" rect_xywhf's member is true)
// to the array representing your texture atlas to the place specified by the rectangle, then finally upload it with glTexImage2D.
//
// Algorithm doesn't create any new rectangles.
// You just pass an array of pointers - rectangles' x/y/w/h/flipped are modified in place.
// There is a vector of pointers for every resultant bin to let you know which ones belong to the particular bin.
// The algorithm may swap the w and h fields for the sake of better fitting, the flag "flipped" will be set to true whenever this occurs.
//
// For description how to tune the algorithm and how it actually works see the .cpp file.

struct HullVert {
	float u, v; // normalized [0..1] sprite-local UV
};

struct HullMesh {
	std::vector<HullVert> verts; // CCW polygon (3-8 verts)
	std::vector<int> indices; // fan triangle indices (3*(N-2))
	bool empty() const { return verts.empty(); }
	int vert_count() const { return (int)verts.size(); }
	int tri_count() const { return (int)indices.size() / 3; }
};

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
			_w(rr.w()), _h(rr.h()) {}
	rect_wh(int w = 0, int h = 0, float scale = 1) :
			_w(w), _h(h), scale(scale) {}
	int _w, _h;
	float scale;
	int area() const { return w() * h(); }
	int perimeter() const { return 2 * w() + 2 * h(); }
	int fits(const rect_wh &bigger, bool allow_flip) const { // 0 - no, 1 - yes, 2 - flipped, 3 - perfectly, 4 perfectly flipped
		if (w() == bigger.w() && h() == bigger.h()) {
			return 3;
		}
		if (allow_flip && h() == bigger.w() && w() == bigger.h()) {
			return 4;
		}
		if (w() <= bigger.w() && h() <= bigger.h()) {
			return 1;
		}
		if (allow_flip && h() <= bigger.w() && w() <= bigger.h()) {
			return 2;
		}
		return 0;
	}
	int w() const { return _w * scale; }
	int h() const { return _h * scale; }
	rect_wh scaled() const { return rect_wh(w(), h()); }
	Size2i size() const { return Size2i(w(), h()); }
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
		rr.w(w());
		rr.h(h());
		return rr;
	}
	int x, y;
	int r() const { return x + w(); }
	int b() const { return y + h(); }
	void r(int right) { _w = right - x; }
	void b(int bottom) { _h = bottom - y; }
};

struct rect_xywhf : public rect_xywh {
	rect_xywhf(const rect_ltrb &rr) :
			rect_xywh(rr), flipped(false), bin(0), trim_l(0), trim_r(0), trim_t(0), trim_b(0) {}
	rect_xywhf(int x, int y, int w, int h) :
			rect_xywh(x, y, w, h), flipped(false), bin(0), trim_l(0), trim_r(0), trim_t(0), trim_b(0) {}
	rect_xywhf() :
			flipped(false), bin(0), trim_l(0), trim_r(0), trim_t(0), trim_b(0) {}
	void flip() {
		flipped = !flipped;
		std::swap(_w, _h);
	}
	bool flipped;
	int bin;
	Ref<Image> atlas_image;
	Ref<Image> original_image;
	// Trim margins: pixels removed from each side of the original image before packing.
	// All zeros when trim_alpha is disabled.
	int trim_l, trim_r, trim_t, trim_b;
	HullMesh hull_mesh; // populated when hull_compute is enabled
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
	return std::max(a->w(), a->h()) > std::max(b->w(), b->h());
}
static inline bool max_width(rect_xywhf *a, rect_xywhf *b) {
	return a->w() > b->w();
}
static inline bool max_height(rect_xywhf *a, rect_xywhf *b) {
	return a->h() > b->h();
}

// Packing algorithm selection.
// PACK_BSP      - binary space partitioning (original); performs bin-size search, good all-rounder.
// PACK_GUILLOTINE - guillotine free-rect split; fast, slightly less efficient than MaxRects.
// PACK_MAXRECTS   - maximum rectangles; best packing quality, slightly slower.
enum PackingAlgorithm {
	PACK_BSP = 0,
	PACK_GUILLOTINE = 1,
	PACK_MAXRECTS = 2,
};

// Border fill mode for the per-sprite margin area.
// BORDER_EMPTY  - leave margin filled with background_color.
// BORDER_MIRROR - mirror edge pixels into the margin (default, prevents texture bleeding).
// BORDER_BLUR   - mirror then box-blur the margin ring (smoother fades at edges).
enum BorderMode {
	BORDER_EMPTY = 0,
	BORDER_MIRROR = 1,
	BORDER_BLUR = 2,
};

struct ImageMergeOptions {
	int max_atlas_size = 0; // default: autofit
	bool force_single_page_atlas = true; // default: rescale to fit (BSP) or single bin
	int margin = 2;
	int force_atlas_channels = 0; // default: autodetect

	Color background_color = Color(0, 0, 0, 0);

	// --- Algorithm & rotation ---
	PackingAlgorithm algorithm = PACK_BSP;
	bool allow_rotation = false; // allow 90° sprite rotation for tighter packing

	// --- Trimming ---
	bool trim_alpha = false; // crop fully-transparent borders before packing
	int trim_alpha_threshold = 0; // pixels with alpha <= threshold are trimmed (0 = fully transparent only)

	// --- Image quality ---
	bool fix_halo = false; // repair Photoshop white-fringe: fill alpha=0 pixel RGB from neighbours
	BorderMode border_mode = BORDER_MIRROR; // fill mode for the margin ring

	// --- Atlas constraints ---
	bool power_of_two = false; // round atlas dimensions up to next power of two
	bool square_atlas = false; // force atlas width == height (max of both)

	// --- Color border trimming ---
	bool trim_color = false; // crop solid-color borders (detected from corner pixels)
	float trim_color_threshold = 1.0f; // CIE94 delta-E threshold (1.0 = just-noticeable difference)

	// --- Debug ---
	bool debug_borders = false; // draw dashed sprite outlines on atlas (for debugging UV placement)
	Color debug_border_color = Color(1, 0, 1, 1); // magenta by default

	// --- Alpha separation ---
	bool separate_alpha = false; // pack opaque and alpha-bearing sprites into separate atlas pages

	// --- Hull mesh ---
	bool hull_compute = false; // compute convex hull polygon per sprite
	int hull_vertex_count = 4; // target polygon vertex count [3..8]
	int hull_alpha_threshold = 0; // alpha threshold for boundary detection
	int hull_max_size = 50; // max hull vertices before simplification
	int hull_sub_pixel = 16; // sub-pixel sampling subdivisions

	// --- Hull debug ---
	bool debug_hull_outline = false;
	bool debug_hull_triangulation = false;
	Color debug_hull_color = Color(0, 1, 0, 1); // green
	Color debug_hull_tri_color = Color(1, 1, 0, 1); // yellow

	ImageMergeOptions &set_max_size(int v) {
		max_atlas_size = v;
		return *this;
	}
	ImageMergeOptions &set_single_page(bool v) {
		force_single_page_atlas = v;
		return *this;
	}
	ImageMergeOptions &set_margin(int v) {
		margin = v;
		return *this;
	}
	ImageMergeOptions &set_algorithm(PackingAlgorithm v) {
		algorithm = v;
		return *this;
	}
};

// Merge images from 'images' and returns dictionary with
// atlas description and atlas images.
// ---------------------------------------------------------
// Return dictionary:
//    "_generated_images" -> array of generated atlas images
//    "_bins_size" -> size of working area of atlas
//    "_rects" -> array of atlas rects, eg:
//       "_rects[0..images]:
//          "rect"       -> Rect2 of image rect on atlas (content area, no margin)
//          "rrect"      -> Rect2 normalized (0-1) texture coordinates
//          "atlas_page" -> atlas page index (of '_generated_images')
//          "atlas"      -> atlas image reference
//          "flipped"    -> bool: true if sprite was rotated 90° CW
//          "trim_l/r/t/b" -> pixels cropped from original before packing
//          "hull"       -> PoolVector2Array of hull polygon verts (normalized UV, when hull_compute=true)
//          "hull_indices" -> PoolIntArray of fan triangle indices (when hull_compute=true)
Dictionary merge_images(const Vector<Ref<Image>> &images, const ImageMergeOptions &options = ImageMergeOptions());

#endif // GD_PACK_H
