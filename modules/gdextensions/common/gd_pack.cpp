/**************************************************************************/
/*  gd_pack.cpp                                                           */
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

#include "gd_pack.h"

#include <cmath>
#include <limits>

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

static int _next_pow2(int v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	return v + 1;
}

// just add another comparing function name to cmpf to perform another packing attempt
// more functions == slower but probably more efficient cases covered and hence less area wasted

static bool (*cmpf[])(rect_xywhf *, rect_xywhf *) = {
	area,
	perimeter,
	max_side,
	max_width,
	max_height
};

// if you find the algorithm running too slow you may double this factor to increase speed but also decrease efficiency
// 1 == most efficient, slowest
// efficiency may be still satisfying at 64 or even 256 with nice speedup

static const int discard_step = 128;

// For every sorting function, algorithm will perform packing attempts beginning with a bin with width and height equal to max_side,
// and decreasing its dimensions if it finds out that rectangles did actually fit, increasing otherwise.
// Although, it's doing that in sort of binary search manner, so for every comparing function it will perform at most log2(max_side) packing attempts looking for the smallest possible bin size.
// discard_step = 128 means that the algorithm will break of the searching loop if the rectangles fit but "it may be possible to fit them in a bin smaller by 128"
// the bigger the value, the sooner the algorithm will finish but the rectangles will be packed less tightly.
// use discard_step = 1 for maximum tightness.
//
// the algorithm was based on http://www.blackpawn.com/texts/lightmaps/default.html
// the algorithm reuses the node tree so it doesn't reallocate them between searching attempts

struct node {
	struct pnode {
		node *pn = nullptr;
		bool fill = false;

		void set(int l, int t, int r, int b) {
			if (!pn)
				pn = new node(rect_ltrb(l, t, r, b));
			else {
				(*pn).rc = rect_ltrb(l, t, r, b);
				(*pn).id = false;
			}
			fill = true;
		}
	};

	pnode c[2];
	rect_ltrb rc;
	bool id = false;
	node(rect_ltrb rc = rect_ltrb()) :
			rc(rc) {}

	void reset(const rect_wh &r) {
		id = false;
		rc = rect_ltrb(0, 0, r.w(), r.h());
		delcheck();
	}

	node *insert(rect_xywhf &img, bool allow_flip) {
		if (c[0].pn && c[0].fill) {
			if (auto newn = c[0].pn->insert(img, allow_flip)) {
				return newn;
			}
			return c[1].pn->insert(img, allow_flip);
		}

		if (id) {
			return 0;
		}
		const int f = img.fits(rect_xywh(rc), allow_flip);

		switch (f) {
			case 0:
				return 0;
			case 1: {
				img.flipped = false;
			} break;
			case 2: {
				img.flipped = true;
			} break;
			case 3:
				id = true;
				img.flipped = false;
				return this;
			case 4:
				id = true;
				img.flipped = true;
				return this;
		}

		int iw = (img.flipped ? img.h() : img.w()), ih = (img.flipped ? img.w() : img.h());

		if (rc.w() - iw > rc.h() - ih) {
			c[0].set(rc.l, rc.t, rc.l + iw, rc.b);
			c[1].set(rc.l + iw, rc.t, rc.r, rc.b);
		} else {
			c[0].set(rc.l, rc.t, rc.r, rc.t + ih);
			c[1].set(rc.l, rc.t + ih, rc.r, rc.b);
		}

		return c[0].pn->insert(img, allow_flip);
	}

	void delcheck() {
		if (c[0].pn) {
			c[0].fill = false;
			c[0].pn->delcheck();
		}
		if (c[1].pn) {
			c[1].fill = false;
			c[1].pn->delcheck();
		}
	}

	~node() {
		if (c[0].pn) {
			delete c[0].pn;
		}
		if (c[1].pn) {
			delete c[1].pn;
		}
	}
};

static rect_wh _rect_2d(rect_xywhf *const *v, int n, int max_s, bool allow_flip, std::vector<rect_xywhf *> &succ, std::vector<rect_xywhf *> &unsucc) {
	node root;

	const int funcs = (sizeof(cmpf) / sizeof(bool (*)(rect_xywhf *, rect_xywhf *)));

	rect_xywhf **order[funcs];

	for (int f = 0; f < funcs; f++) {
		order[f] = new rect_xywhf *[n];
		std::memcpy(order[f], v, sizeof(rect_xywhf *) * n);
		std::sort(order[f], order[f] + n, cmpf[f]);
	}

	rect_wh min_bin = rect_wh(max_s, max_s);
	int min_func = -1, best_func = 0, best_area = 0, _area = 0, step, fit;

	bool fail = false;

	for (int f = 0; f < funcs; ++f) {
		v = order[f];
		step = min_bin.w() / 2;
		root.reset(min_bin);

		while (true) {
			if (root.rc.w() > min_bin.w()) {
				if (min_func > -1) {
					break;
				}
				_area = 0;

				root.reset(min_bin);
				for (int i = 0; i < n; ++i) {
					if (root.insert(*v[i], allow_flip)) {
						_area += v[i]->area();
					}
				}
				fail = true;
				break;
			}

			fit = -1;

			for (int i = 0; i < n; ++i) {
				if (!root.insert(*v[i], allow_flip)) {
					fit = 1;
					break;
				}
			}

			if (fit == -1 && step <= discard_step) {
				break;
			}

			root.reset(rect_wh(root.rc.w() + fit * step, root.rc.h() + fit * step));

			step /= 2;
			if (!step) {
				step = 1;
			}
		}

		if (!fail && (min_bin.area() >= root.rc.area())) {
			min_bin = rect_wh(root.rc);
			min_func = f;
		} else if (fail && (_area > best_area)) {
			best_area = _area;
			best_func = f;
		}
		fail = false;
	}

	v = order[min_func == -1 ? best_func : min_func];

	int clip_x = 0, clip_y = 0;

	root.reset(min_bin);

	for (int i = 0; i < n; ++i) {
		if (auto ret = root.insert(*v[i], allow_flip)) {
			v[i]->x = ret->rc.l;
			v[i]->y = ret->rc.t;

			if (v[i]->flipped) {
				v[i]->flipped = false;
				v[i]->flip();
			}

			clip_x = std::max(clip_x, ret->rc.r);
			clip_y = std::max(clip_y, ret->rc.b);

			succ.push_back(v[i]);
		} else {
			unsucc.push_back(v[i]);

			v[i]->flipped = false;
		}
	}

	for (int f = 0; f < funcs; ++f) {
		delete[] order[f];
	}

	return rect_wh(clip_x, clip_y);
}

static rect_wh _try_rects_2d(rect_xywhf *const *v, int n, bool allow_flip) {
	int max_side = 0, min_side = 32;

	// start from biggest side
	for (int i = 0; i < n; i++) {
		if (v[i]->w() > max_side) {
			max_side = v[i]->w();
		}
		if (v[i]->h() > max_side) {
			max_side = v[i]->h();
		}
		if (v[i]->w() > 0 && v[i]->w() < min_side) {
			min_side = v[i]->w();
		}
		if (v[i]->h() > 0 && v[i]->h() < min_side) {
			min_side = v[i]->h();
		}
	}

	while (true) {
		rect_wh _rect(max_side, max_side);

		std::vector<rect_xywhf *> vec[2], *p[2] = { vec, vec + 1 }, rects;
		vec[0].resize(n);
		vec[1].clear();
		std::memcpy(&vec[0][0], v, sizeof(rect_xywhf *) * n);

		rect_wh size = _rect_2d(&((*p[0])[0]), static_cast<int>(p[0]->size()), max_side, allow_flip, rects, *p[1]);
		if (!p[1]->size()) { // no unfitted items - finish
			return size; // pack size
		}

		max_side += min_side;
	}

	return { 0, 0 };
}

static bool _pack_rects_free(rect_xywhf *const *v, int n, int max_side, bool single_page, bool allow_flip, PackingAlgorithm algo, std::vector<bin> &bins); // forward decl

static bool _pack_rects(rect_xywhf *const *v, int n, int max_side, bool single_page, bool allow_flip, PackingAlgorithm algo, std::vector<bin> &bins) {
	if (algo != PACK_BSP) {
		return _pack_rects_free(v, n, max_side, single_page, allow_flip, algo, bins);
	}
	// --- BSP path (original) ---
	real_t req_max_side = max_side;
	if (max_side <= 0 || single_page) {
		const rect_wh rc = _try_rects_2d(v, n, allow_flip);
		max_side = rc.w();
		if (max_side <= 0) {
			return false;
		}
		print_verbose(vformat("Autofit packing success: %dx%d", rc.w(), rc.h()));
	}

	if (req_max_side <= 0) {
		req_max_side = max_side;
	}

	if (single_page && req_max_side > 0 && max_side > req_max_side) {
		// find scale to fit in max_side
		int max_area = 0;
		for (int i = 0; i < n; i++) {
			const int area = v[i]->area();
			if (area > max_area) {
				max_area = area;
			}
		}
		const real_t step = 0.01;
		real_t base_scale = 2;
		int last_max_side = 0;
		do {
			for (int i = 0; i < n; i++) {
				const real_t area = v[i]->_w * v[i]->_h;
				v[i]->scale = base_scale - (area / max_area);
				ERR_FAIL_COND_V(v[i]->scale <= 0, false);
			}
			max_side = _try_rects_2d(v, n, allow_flip).w();
			print_verbose(vformat("Autoscaling iteration: scale %0.2f -> side %d", base_scale, max_side));
			if (last_max_side && last_max_side - max_side != 0) {
				const real_t diff = (last_max_side - max_side) / real_t(req_max_side); // % difference between iterations
				const real_t need = (req_max_side - max_side) / real_t(req_max_side);
				const int iters = Math::ceil(need / diff);
				if (Math::abs(iters) > 2) { // if estimation shows more iterations are needed increase step one-time
					base_scale += step * iters;
				} else {
					base_scale += step * SIGN2(iters);
				}
			} else {
				base_scale -= step;
			}
			last_max_side = max_side;
		} while (max_side > req_max_side);
	}

	rect_wh _rect(max_side, max_side);
	for (int i = 0; i < n; i++) {
		if (!v[i]->fits(_rect, allow_flip)) {
			return false;
		}
	}

	std::vector<rect_xywhf *> vec[2], *p[2] = { vec, vec + 1 };
	vec[0].resize(n);
	vec[1].clear();
	std::memcpy(&vec[0][0], v, sizeof(rect_xywhf *) * n);

	while (true) {
		bins.push_back(bin());
		bin *b = &bins[bins.size() - 1];

		b->size = _rect_2d(&((*p[0])[0]), static_cast<int>(p[0]->size()), max_side, allow_flip, b->rects, *p[1]);
		p[0]->clear();

		if (!p[1]->size()) { // no unfitted items - finish
			break;
		}

		std::swap(p[0], p[1]); // continue with new bin
	}

	return true;
}

// ---------------------------------------------------------------------------
// Guillotine bin packer
// Adapted from texpack/src/rectpack.cpp (GuillotineBinPack).
// Uses Best-Short-Side-Fit placement + Shorter-Leftover-Axis split + free-list merge.
// ---------------------------------------------------------------------------

struct _fr { // internal free rectangle
	int x, y, w, h;
	_fr() :
			x(0), y(0), w(0), h(0) {}
	_fr(int x, int y, int w, int h) :
			x(x), y(y), w(w), h(h) {}
	int area() const { return w * h; }
};

class GuillotinePacker {
	std::vector<_fr> free_rects;

	static int _score(int w, int h, const _fr &r) {
		return std::min(std::abs(r.w - w), std::abs(r.h - h));
	}

	_fr _find(int w, int h, int *idx) const {
		_fr best;
		int best_score = std::numeric_limits<int>::max();
		*idx = -1;
		for (int i = 0; i < (int)free_rects.size(); ++i) {
			if (w <= free_rects[i].w && h <= free_rects[i].h) {
				int s = _score(w, h, free_rects[i]);
				if (s < best_score) {
					best = _fr(free_rects[i].x, free_rects[i].y, w, h);
					best_score = s;
					*idx = i;
				}
			}
		}
		return best;
	}

	void _split(const _fr &free, const _fr &placed) {
		const int rw = free.w - placed.w;
		const int rh = free.h - placed.h;
		const bool horiz = (rw <= rh); // ShorterLeftoverAxis
		_fr bottom(free.x, free.y + placed.h, horiz ? free.w : placed.w, rh);
		_fr right(free.x + placed.w, free.y, rw, horiz ? placed.h : free.h);
		if (bottom.w > 0 && bottom.h > 0)
			free_rects.push_back(bottom);
		if (right.w > 0 && right.h > 0)
			free_rects.push_back(right);
	}

	void _merge() {
		for (int i = 0; i < (int)free_rects.size(); ++i) {
			for (int j = i + 1; j < (int)free_rects.size(); ++j) {
				if (free_rects[i].w == free_rects[j].w && free_rects[i].x == free_rects[j].x) {
					if (free_rects[i].y == free_rects[j].y + free_rects[j].h) {
						free_rects[i].y -= free_rects[j].h;
						free_rects[i].h += free_rects[j].h;
						free_rects.erase(free_rects.begin() + j--);
					} else if (free_rects[i].y + free_rects[i].h == free_rects[j].y) {
						free_rects[i].h += free_rects[j].h;
						free_rects.erase(free_rects.begin() + j--);
					}
				} else if (free_rects[i].h == free_rects[j].h && free_rects[i].y == free_rects[j].y) {
					if (free_rects[i].x == free_rects[j].x + free_rects[j].w) {
						free_rects[i].x -= free_rects[j].w;
						free_rects[i].w += free_rects[j].w;
						free_rects.erase(free_rects.begin() + j--);
					} else if (free_rects[i].x + free_rects[i].w == free_rects[j].x) {
						free_rects[i].w += free_rects[j].w;
						free_rects.erase(free_rects.begin() + j--);
					}
				}
			}
		}
	}

public:
	GuillotinePacker(int w, int h) {
		free_rects.push_back(_fr(0, 0, w, h));
	}

	// Returns placed _fr (w>0 on success). Sets r->x, r->y, and flips if needed.
	_fr insert(rect_xywhf *r, bool allow_flip) {
		int idx = -1;
		_fr placed = _find(r->w(), r->h(), &idx);
		if (idx == -1 && allow_flip) {
			placed = _find(r->h(), r->w(), &idx);
			if (idx != -1)
				r->flip();
		}
		if (idx == -1)
			return _fr();
		r->x = placed.x;
		r->y = placed.y;
		_split(free_rects[idx], placed);
		free_rects.erase(free_rects.begin() + idx);
		_merge();
		return placed;
	}
};

// ---------------------------------------------------------------------------
// MaxRects bin packer
// Adapted from texpack/src/rectpack.cpp (MaxRectsBinPack).
// Uses Best-Short-Side-Fit heuristic. Supports optional rotation.
// ---------------------------------------------------------------------------

class MaxRectsPacker {
	std::vector<_fr> used_rects;
	std::vector<_fr> free_rects;

	static bool _contained(const _fr &a, const _fr &b) {
		return a.x >= b.x && a.y >= b.y &&
				a.x + a.w <= b.x + b.w &&
				a.y + a.h <= b.y + b.h;
	}

	_fr _find_bssf(int w, int h, bool allow_flip, bool &flipped, int &s1, int &s2) const {
		_fr best;
		s1 = s2 = std::numeric_limits<int>::max();
		flipped = false;
		for (const _fr &f : free_rects) {
			if (w <= f.w && h <= f.h) {
				int lo = std::min(std::abs(f.w - w), std::abs(f.h - h));
				int hi = std::max(std::abs(f.w - w), std::abs(f.h - h));
				if (lo < s1 || (lo == s1 && hi < s2)) {
					best = _fr(f.x, f.y, w, h);
					s1 = lo;
					s2 = hi;
					flipped = false;
				}
			}
			if (allow_flip && h <= f.w && w <= f.h) {
				int lo = std::min(std::abs(f.w - h), std::abs(f.h - w));
				int hi = std::max(std::abs(f.w - h), std::abs(f.h - w));
				if (lo < s1 || (lo == s1 && hi < s2)) {
					best = _fr(f.x, f.y, h, w);
					s1 = lo;
					s2 = hi;
					flipped = true;
				}
			}
		}
		return best;
	}

	bool _split_free(_fr free, const _fr &used) {
		if (used.x >= free.x + free.w || used.x + used.w <= free.x ||
				used.y >= free.y + free.h || used.y + used.h <= free.y)
			return false;
		if (used.x < free.x + free.w && used.x + used.w > free.x) {
			if (used.y > free.y)
				free_rects.push_back(_fr(free.x, free.y, free.w, used.y - free.y));
			if (used.y + used.h < free.y + free.h)
				free_rects.push_back(_fr(free.x, used.y + used.h, free.w, free.y + free.h - (used.y + used.h)));
		}
		if (used.y < free.y + free.h && used.y + used.h > free.y) {
			if (used.x > free.x)
				free_rects.push_back(_fr(free.x, free.y, used.x - free.x, free.h));
			if (used.x + used.w < free.x + free.w)
				free_rects.push_back(_fr(used.x + used.w, free.y, free.x + free.w - (used.x + used.w), free.h));
		}
		return true;
	}

	void _place(const _fr &node) {
		int n = (int)free_rects.size();
		for (int i = 0; i < n; ++i) {
			if (_split_free(free_rects[i], node)) {
				free_rects.erase(free_rects.begin() + i);
				--i;
				--n;
			}
		}
		// prune free rects that are contained by another
		for (int i = 0; i < (int)free_rects.size(); ++i)
			for (int j = i + 1; j < (int)free_rects.size(); ++j) {
				if (_contained(free_rects[i], free_rects[j])) {
					free_rects.erase(free_rects.begin() + i--);
					break;
				}
				if (_contained(free_rects[j], free_rects[i]))
					free_rects.erase(free_rects.begin() + j--);
			}
		used_rects.push_back(node);
	}

public:
	MaxRectsPacker(int w, int h) {
		free_rects.push_back(_fr(0, 0, w, h));
	}

	// Returns placed _fr (h>0 on success). Sets r->x, r->y, and flips if needed.
	_fr insert(rect_xywhf *r, bool allow_flip) {
		bool flipped = false;
		int s1, s2;
		_fr node = _find_bssf(r->w(), r->h(), allow_flip, flipped, s1, s2);
		if (s1 == std::numeric_limits<int>::max())
			return _fr();
		r->x = node.x;
		r->y = node.y;
		if (flipped)
			r->flip();
		_place(node);
		return node;
	}
};

// Pack rects using Guillotine or MaxRects. Called from _pack_rects when algo != PACK_BSP.
static bool _pack_rects_free(rect_xywhf *const *v, int n, int max_side, bool single_page, bool allow_flip, PackingAlgorithm algo, std::vector<bin> &bins) {
	// Compute an initial square bin side large enough to hold all rects.
	long long total_area = 0;
	int max_dim = 0;
	for (int i = 0; i < n; ++i) {
		total_area += (long long)v[i]->area();
		max_dim = MAX(max_dim, MAX(v[i]->w(), v[i]->h()));
	}
	int bin_side;
	if (max_side > 0) {
		bin_side = max_side;
	} else {
		bin_side = MAX(max_dim, (int)ceil(sqrt((double)total_area)));
		bin_side = MAX(bin_side, 64);
	}

	// Sort by area descending for better initial placement.
	std::vector<rect_xywhf *> remaining(v, v + n);
	std::sort(remaining.begin(), remaining.end(), [](rect_xywhf *a, rect_xywhf *b) {
		return a->area() > b->area();
	});

	while (!remaining.empty()) {
		bins.push_back(bin());
		bin *b = &bins.back();
		const int bin_idx = (int)bins.size() - 1;
		int clip_x = 0, clip_y = 0;
		std::vector<rect_xywhf *> unpacked;

		if (algo == PACK_GUILLOTINE) {
			GuillotinePacker packer(bin_side, bin_side);
			for (rect_xywhf *r : remaining) {
				_fr placed = packer.insert(r, allow_flip);
				if (placed.h > 0) {
					r->bin = bin_idx;
					b->rects.push_back(r);
					clip_x = MAX(clip_x, r->x + r->w());
					clip_y = MAX(clip_y, r->y + r->h());
				} else {
					unpacked.push_back(r);
				}
			}
		} else { // PACK_MAXRECTS
			MaxRectsPacker packer(bin_side, bin_side);
			for (rect_xywhf *r : remaining) {
				_fr placed = packer.insert(r, allow_flip);
				if (placed.h > 0) {
					r->bin = bin_idx;
					b->rects.push_back(r);
					clip_x = MAX(clip_x, r->x + r->w());
					clip_y = MAX(clip_y, r->y + r->h());
				} else {
					unpacked.push_back(r);
				}
			}
		}

		if (b->rects.empty()) {
			bins.pop_back();
			WARN_PRINT("Pack: could not place remaining " + itos((int)remaining.size()) + " image(s) - bin_side may be too small.");
			break;
		}

		b->size = rect_wh(clip_x, clip_y);
		remaining = unpacked;

		if (single_page)
			break;
	}

	return true;
}

static int _get_offset_for_format(Image::Format format) {
	switch (format) {
		case Image::FORMAT_RGB8:
			return 3;
		case Image::FORMAT_RGBA8:
			return 4;
		case Image::FORMAT_LA8:
			return 2;
		case Image::FORMAT_A8:
			return 1;
		case Image::FORMAT_L8:
			return 1;
		case Image::FORMAT_R8:
		case Image::FORMAT_RG8:
		case Image::FORMAT_RGBA4444:
		case Image::FORMAT_RF:
		case Image::FORMAT_RGF:
		case Image::FORMAT_RGBF:
		case Image::FORMAT_RGBAF:
		case Image::FORMAT_RH:
		case Image::FORMAT_RGH:
		case Image::FORMAT_RGBH:
		case Image::FORMAT_RGBAH:
		case Image::FORMAT_RGBE9995:
		case Image::FORMAT_DXT1:
		case Image::FORMAT_DXT3:
		case Image::FORMAT_DXT5:
		case Image::FORMAT_RGTC_R:
		case Image::FORMAT_RGTC_RG:
		case Image::FORMAT_BPTC_RGBA:
		case Image::FORMAT_BPTC_RGBF:
		case Image::FORMAT_BPTC_RGBFU:
		case Image::FORMAT_PVRTC2:
		case Image::FORMAT_PVRTC2A:
		case Image::FORMAT_PVRTC4:
		case Image::FORMAT_PVRTC4A:
		case Image::FORMAT_ETC:
		case Image::FORMAT_ETC2_R11:
		case Image::FORMAT_ETC2_R11S:
		case Image::FORMAT_ETC2_RG11:
		case Image::FORMAT_ETC2_RG11S:
		case Image::FORMAT_ETC2_RGB8:
		case Image::FORMAT_ETC2_RGBA8:
		case Image::FORMAT_ETC2_RGB8A1:
#if VERSION_MAJOR >= 4
		case Image::FORMAT_RGB565:
		case Image::FORMAT_ETC2_RA_AS_RG:
		case Image::FORMAT_DXT5_RA_AS_RG:
#else
		case Image::FORMAT_RGBA5551:
#endif
		case Image::FORMAT_MAX:
			return 0;
	}

	return 0;
}

// mirror borders to avoid leaking outside pixels when filtering
static Ref<Image> _mirror_borders(Ref<Image> image, int x_border, int y_border) {
	ERR_FAIL_COND_V(image.is_null(), Ref<Image>());

	int bx = MAX(0, x_border - 1);
	int by = MAX(0, y_border - 1);
	Size2 rc = image->get_size();

	Ref<Image> form = memnew(Image);
	form->create(image->get_size().width + 2 * x_border, image->get_size().height + 2 * y_border, false, image->get_format());

	auto get_rect = [](Ref<Image> img) {
		return Rect2(Point2(0, 0), img->get_size());
	};

	// copy borders:
	const auto topb = image->get_rect(Rect2(0, 0, rc.width, 1));
	const auto botb = image->get_rect(Rect2(0, rc.height - 1, rc.width, 1));
	const auto leftb = image->get_rect(Rect2(0, 0, 1, rc.height));
	const auto rightb = image->get_rect(Rect2(rc.width - 1, 0, 1, rc.height));

	// copy corner pixels:
	image->lock();
	const auto topp = image->get_pixel(0, 0);
	const auto botp = image->get_pixel(0, rc.height - 1);
	const auto leftp = image->get_pixel(rc.width - 1, rc.height - 1);
	const auto rightp = image->get_pixel(rc.width - 1, 0);
	image->unlock();

	// place image:
	form->blit_rect(image, get_rect(image), Point2(x_border, y_border));

	// duplicate borders around the image:
	for (int k = 0; k < by; k++) {
		form->blit_rect(topb, get_rect(topb), Point2(x_border, y_border - k - 1)); // top
	}
	for (int k = 0; k < by; k++) {
		form->blit_rect(botb, get_rect(botb), Point2(x_border, rc.height - y_border + k)); // bottom
	}
	for (int k = 0; k < bx; k++) {
		form->blit_rect(leftb, get_rect(leftb), Point2(x_border - k - 1, y_border)); // left
	}
	for (int k = 0; k < bx; k++) {
		form->blit_rect(rightb, get_rect(rightb), Point2(rc.width - x_border + k, y_border)); // right
	}

	form->lock();
	// fill up corners:
	for (int k = 0; k < by; k++) {
		for (int m = 0; m < bx; m++) {
			form->set_pixel(x_border - m - 1, y_border - k - 1, topp);
			form->set_pixel(x_border - m - 1, rc.height - y_border + k, botp);
			form->set_pixel(rc.width - x_border + m, rc.height - y_border + k, leftp);
			form->set_pixel(rc.width - x_border + m, y_border - k - 1, rightp);
		}
	}
	form->unlock();

	return form;
}

// Crop fully-transparent borders from 'src'. Fills trim_* with how many pixels were removed
// from each side. Returns the cropped image in the original format, or src unchanged if nothing trimmed.
static Ref<Image> _trim_alpha_border(const Ref<Image> &src, int threshold,
		int &trim_l, int &trim_r, int &trim_t, int &trim_b) {
	trim_l = trim_r = trim_t = trim_b = 0;
	ERR_FAIL_COND_V(!src.is_valid(), src);

	// Use RGBA8 copy for alpha inspection, keep original format for crop.
	Ref<Image> rgba = src->duplicate();
	if (rgba->get_format() != Image::FORMAT_RGBA8) {
		rgba->convert(Image::FORMAT_RGBA8);
	}

	const int w = rgba->get_width();
	const int h = rgba->get_height();
	PoolByteArray data = rgba->get_data();
	PoolByteArray::Read rd = data.read();
	const uint8_t *ptr = rd.ptr();

	auto alpha_at = [ptr, w](int x, int y) -> int {
		return ptr[(y * w + x) * 4 + 3];
	};
	auto has_opaque_row = [&](int y) {
		for (int x = 0; x < w; ++x)
			if (alpha_at(x, y) > threshold)
				return true;
		return false;
	};
	auto has_opaque_col = [&](int x) {
		for (int y = 0; y < h; ++y)
			if (alpha_at(x, y) > threshold)
				return true;
		return false;
	};

	while (trim_t < h && !has_opaque_row(trim_t))
		++trim_t;
	while (trim_b < h - trim_t && !has_opaque_row(h - 1 - trim_b))
		++trim_b;
	while (trim_l < w && !has_opaque_col(trim_l))
		++trim_l;
	while (trim_r < w - trim_l && !has_opaque_col(w - 1 - trim_r))
		++trim_r;

	const int nw = w - trim_l - trim_r;
	const int nh = h - trim_t - trim_b;
	if (nw <= 0 || nh <= 0 || (trim_l == 0 && trim_r == 0 && trim_t == 0 && trim_b == 0)) {
		trim_l = trim_r = trim_t = trim_b = 0;
		return src;
	}
	return src->get_rect(Rect2(trim_l, trim_t, nw, nh));
}

// Fix Photoshop/tool halo: alpha=0 pixels often have their RGB set to white.
// Replaces the RGB of each alpha=0 pixel with a weighted average of its non-transparent
// 8-neighbours, so the GPU fades into the correct colour rather than white.
static void _fix_halo(Ref<Image> &img) {
	ERR_FAIL_COND(!img.is_valid());

	Ref<Image> rgba = img;
	if (rgba->get_format() != Image::FORMAT_RGBA8) {
		rgba = img->duplicate();
		rgba->convert(Image::FORMAT_RGBA8);
	}

	const int w = rgba->get_width();
	const int h = rgba->get_height();

	rgba->lock();
	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			Color c = rgba->get_pixel(x, y);
			if (c.a > 0)
				continue;
			float rs = 0, gs = 0, bs = 0;
			int count = 0;
			for (int dy = -1; dy <= 1; ++dy) {
				for (int dx = -1; dx <= 1; ++dx) {
					int nx = x + dx, ny = y + dy;
					if (nx < 0 || ny < 0 || nx >= w || ny >= h)
						continue;
					Color nc = rgba->get_pixel(nx, ny);
					if (nc.a <= 0)
						continue;
					rs += nc.r * nc.a;
					gs += nc.g * nc.a;
					bs += nc.b * nc.a;
					count++;
				}
			}
			if (count > 0) {
				rgba->set_pixel(x, y, Color(rs / count, gs / count, bs / count, 0));
			}
		}
	}
	rgba->unlock();

	if (img->get_format() != Image::FORMAT_RGBA8) {
		rgba->convert(img->get_format());
	}
	img = rgba;
}

// Build border-extended image with box-blurred margin ring.
// The interior content area is identical to _mirror_borders; only the outer ring is blurred.
static Ref<Image> _blur_border(Ref<Image> image, int border) {
	Ref<Image> extended = _mirror_borders(image, border, border);
	ERR_FAIL_COND_V(!extended.is_valid(), extended);
	if (border <= 1)
		return extended;

	const int ew = extended->get_width();
	const int eh = extended->get_height();
	Ref<Image> result = extended->duplicate();

	extended->lock();
	result->lock();

	for (int y = 0; y < eh; ++y) {
		for (int x = 0; x < ew; ++x) {
			// Only process the outer border ring.
			if (x >= border && x < ew - border && y >= border && y < eh - border)
				continue;
			float r = 0, g = 0, b = 0, a = 0;
			int cnt = 0;
			for (int dy = -1; dy <= 1; ++dy) {
				for (int dx = -1; dx <= 1; ++dx) {
					int nx = CLAMP(x + dx, 0, ew - 1);
					int ny = CLAMP(y + dy, 0, eh - 1);
					Color c = extended->get_pixel(nx, ny);
					r += c.r;
					g += c.g;
					b += c.b;
					a += c.a;
					cnt++;
				}
			}
			result->set_pixel(x, y, Color(r / cnt, g / cnt, b / cnt, a / cnt));
		}
	}

	result->unlock();
	extended->unlock();
	return result;
}

// ---------------------------------------------------------------------------
// Color border trimming
// Detects a background color from the image corners using CIE94 delta-E, then
// trims rows and columns that match it within the given perceptual threshold.
// threshold is a CIE94 delta-E value; 1.0 ≈ just-noticeable difference.
// ---------------------------------------------------------------------------

static void _rgb2lab(float r, float g, float b, float &L, float &A, float &B) {
	// sRGB → linear
	auto lin = [](float c) -> float {
		return c > 0.04045f ? powf((c + 0.055f) / 1.055f, 2.4f) : c / 12.92f;
	};
	r = lin(r) * 100.f;
	g = lin(g) * 100.f;
	b = lin(b) * 100.f;
	// linear → XYZ (D65)
	float x = r * 0.4124f + g * 0.3576f + b * 0.1805f;
	float y = r * 0.2126f + g * 0.7152f + b * 0.0722f;
	float z = r * 0.0193f + g * 0.1192f + b * 0.9505f;
	// XYZ → Lab
	auto f = [](float v) -> float {
		return v > 0.008856f ? cbrtf(v) : 7.787f * v + 0.137931f;
	};
	x = f(x / 95.047f);
	y = f(y / 100.000f);
	z = f(z / 108.883f);
	L = 116.f * y - 16.f;
	A = 500.f * (x - y);
	B = 200.f * (y - z);
}

static float _delta_e(const Color &c1, const Color &c2) {
	float l1, a1, b1, l2, a2, b2;
	_rgb2lab(c1.r, c1.g, c1.b, l1, a1, b1);
	_rgb2lab(c2.r, c2.g, c2.b, l2, a2, b2);
	const float dL = l1 - l2;
	const float C1 = sqrtf(a1 * a1 + b1 * b1);
	const float C2 = sqrtf(a2 * a2 + b2 * b2);
	const float dC = C1 - C2;
	const float da = a1 - a2;
	const float db = b1 - b2;
	const float dH2 = da * da + db * db - dC * dC;
	const float dH = dH2 > 0 ? sqrtf(dH2) : 0;
	const float q1 = dL / 2.f;
	const float q2 = dC / (1.f + 0.048f * C1);
	const float q3 = dH / (1.f + 0.014f * C1);
	return sqrtf(q1 * q1 + q2 * q2 + q3 * q3);
}

// Trim rows/cols whose pixels all match a corner-sampled background within delta-E threshold.
// Returns cropped image. Updates trim_l/r/t/b cumulatively (adds to existing values).
static Ref<Image> _trim_color_border(const Ref<Image> &src, float threshold,
		int &trim_l, int &trim_r, int &trim_t, int &trim_b) {
	ERR_FAIL_COND_V(!src.is_valid(), src);
	Ref<Image> rgba = src;
	if (rgba->get_format() != Image::FORMAT_RGBA8) {
		rgba = src->duplicate();
		rgba->convert(Image::FORMAT_RGBA8);
	}

	const int W = rgba->get_width();
	const int H = rgba->get_height();
	if (W <= 1 || H <= 1)
		return src;

	rgba->lock();

	// Sample 4 corners as candidate background colors.
	Color corners[4] = {
		rgba->get_pixel(0, 0),
		rgba->get_pixel(W - 1, 0),
		rgba->get_pixel(W - 1, H - 1),
		rgba->get_pixel(0, H - 1),
	};

	// Find the corner color that best represents the background:
	// pick the one with most other corners within threshold.
	int best_idx = 0, best_score = -1;
	for (int i = 0; i < 4; ++i) {
		int score = 0;
		for (int j = 0; j < 4; ++j)
			if (_delta_e(corners[i], corners[j]) <= threshold)
				++score;
		if (score > best_score) {
			best_score = score;
			best_idx = i;
		}
	}
	const Color bg = corners[best_idx];

	// Helper: does every pixel in this row match the background?
	auto row_matches = [&](int y, int x0, int x1) -> bool {
		for (int x = x0; x < x1; ++x)
			if (_delta_e(rgba->get_pixel(x, y), bg) > threshold)
				return false;
		return true;
	};
	auto col_matches = [&](int x, int y0, int y1) -> bool {
		for (int y = y0; y < y1; ++y)
			if (_delta_e(rgba->get_pixel(x, y), bg) > threshold)
				return false;
		return true;
	};

	int x0 = 0, y0 = 0, x1 = W, y1 = H;
	while (y0 < y1 && row_matches(y0, x0, x1))
		++y0;
	while (y1 > y0 && row_matches(y1 - 1, x0, x1))
		--y1;
	while (x0 < x1 && col_matches(x0, y0, y1))
		++x0;
	while (x1 > x0 && col_matches(x1 - 1, y0, y1))
		--x1;

	rgba->unlock();

	const int nw = x1 - x0;
	const int nh = y1 - y0;
	if (nw <= 0 || nh <= 0 || (x0 == 0 && y0 == 0 && nw == W && nh == H))
		return src;

	trim_l += x0;
	trim_r += W - x1;
	trim_t += y0;
	trim_b += H - y1;
	return src->get_rect(Rect2(x0, y0, nw, nh));
}

// ---------------------------------------------------------------------------
// Debug border overlay
// Draws a dashed rectangle at the inner edge of the margin for each packed sprite.
// Every 3rd pixel on the perimeter is coloured with debug_border_color.
// ---------------------------------------------------------------------------
static void _draw_debug_border(PoolByteArray &atlas_data, int atlas_w, int atlas_channels,
		int rx, int ry, int rw, int rh, int margin, const Color &col) {
	if (margin <= 0 || rw <= 2 * margin || rh <= 2 * margin)
		return;

	const int x1 = rx + margin;
	const int y1 = ry + margin;
	const int x2 = rx + rw - margin - 1;
	const int y2 = ry + rh - margin - 1;

	uint8_t cr = (uint8_t)(col.r * 255);
	uint8_t cg = (uint8_t)(col.g * 255);
	uint8_t cb = (uint8_t)(col.b * 255);
	uint8_t ca = (uint8_t)(col.a * 255);

	auto put = [&](int x, int y) {
		if (x < 0 || y < 0 || x >= atlas_w)
			return;
		const int idx = y * atlas_w * atlas_channels + x * atlas_channels;
		if (atlas_channels >= 1)
			atlas_data.set(idx + 0, cr);
		if (atlas_channels >= 2)
			atlas_data.set(idx + 1, cg);
		if (atlas_channels >= 3)
			atlas_data.set(idx + 2, cb);
		if (atlas_channels >= 4)
			atlas_data.set(idx + 3, ca);
	};

	int step = 0;
	for (int x = x1; x <= x2; ++x, ++step) {
		if (step % 3 == 0) {
			put(x, y1);
			put(x, y2);
		}
	}
	step = 0;
	for (int y = y1; y <= y2; ++y, ++step) {
		if (step % 3 == 0) {
			put(x1, y);
			put(x2, y);
		}
	}
	// Always draw corners.
	put(x1, y1);
	put(x2, y1);
	put(x1, y2);
	put(x2, y2);
}

Dictionary merge_images(const Vector<Ref<Image>> &images, const ImageMergeOptions &options) {
	if (images.empty()) {
		return Dictionary(); // exit early
	}

	const int margin = options.margin;
	const Color background_color = options.background_color;

	int atlas_channels = options.force_atlas_channels > 0 ? options.force_atlas_channels : 0;

	// --- Alpha separation: split into opaque and alpha-bearing groups ---
	// When separate_alpha=true, opaque images are packed first; alpha images follow on
	// additional atlas pages. Both groups share the same output data[] / rects[] arrays
	// (indexed by original image position) so all output keys remain consistent.
	Vector<int> opaque_indices, alpha_indices;
	for (int i = 0; i < images.size(); ++i) {
		bool has_alpha = false;
		if (options.separate_alpha) {
			Ref<Image> tmp = images[i];
			if (tmp->get_format() != Image::FORMAT_RGBA8 && tmp->get_format() != Image::FORMAT_LA8) {
				// Formats without alpha channel are always opaque.
				has_alpha = false;
			} else {
				has_alpha = (tmp->detect_alpha() != Image::ALPHA_NONE);
			}
		}
		if (has_alpha)
			alpha_indices.push_back(i);
		else
			opaque_indices.push_back(i);
	}
	// Build a packing order: opaque first, then alpha.
	// If separate_alpha is false, all images are in opaque_indices.
	Vector<int> pack_order;
	for (int i = 0; i < opaque_indices.size(); ++i)
		pack_order.push_back(opaque_indices[i]);
	for (int i = 0; i < alpha_indices.size(); ++i)
		pack_order.push_back(alpha_indices[i]);

	const int n = images.size();

	Vector<rect_xywhf> data;
	data.resize(n);
	Vector<rect_xywhf *> rects;
	rects.resize(n);

	for (int i = 0; i < images.size(); ++i) {
		Ref<Image> image = images[i];
		if (image->get_size().width == 0 || image->get_size().height == 0) {
			WARN_PRINT("Image " + itos(i) + " is empty");
		}

		// --- Halo fix (before trimming so transparent pixels are repaired first) ---
		if (options.fix_halo) {
			_fix_halo(image);
		}

		// --- Alpha border trimming ---
		int tl = 0, tr = 0, tt = 0, tb = 0;
		if (options.trim_alpha) {
			image = _trim_alpha_border(image, options.trim_alpha_threshold, tl, tr, tt, tb);
		}
		// --- Color border trimming (cumulative with alpha trim) ---
		if (options.trim_color) {
			image = _trim_color_border(image, options.trim_color_threshold, tl, tr, tt, tb);
		}
		data.write[i].trim_l = tl;
		data.write[i].trim_r = tr;
		data.write[i].trim_t = tt;
		data.write[i].trim_b = tb;

		// --- Margin / border fill ---
		if (margin > 0) {
			switch (options.border_mode) {
				case BORDER_MIRROR:
					image = _mirror_borders(image, margin, margin);
					break;
				case BORDER_BLUR:
					image = _blur_border(image, margin);
					break;
				case BORDER_EMPTY:
				default: {
					// Create a larger canvas; leave margin filled with background_color.
					Ref<Image> padded = memnew(Image);
					padded->create(image->get_width() + 2 * margin, image->get_height() + 2 * margin, false, image->get_format());
					padded->fill(background_color);
					padded->blit_rect(image, Rect2(0, 0, image->get_width(), image->get_height()), Point2(margin, margin));
					image = padded;
				} break;
			}
		}

		data.write[i].original_image = image;
		data.write[i].x = 0;
		data.write[i].y = 0;
		data.write[i]._w = image->get_size().width;
		data.write[i]._h = image->get_size().height;
		data.write[i].scale = 1;
		rects.write[i] = &data.write[i];

		if (options.force_atlas_channels == 0) {
			if (image->get_format() == Image::FORMAT_L8) {
				atlas_channels = MAX(1, atlas_channels);
			} else if (image->get_format() == Image::FORMAT_RGB8) {
				atlas_channels = MAX(3, atlas_channels);
			} else if (image->get_format() == Image::FORMAT_RGBA8 || image->get_format() == Image::FORMAT_LA8) {
				// only if we have a real alpha values in the channel
				if (image->detect_alpha() == Image::ALPHA_BLEND) {
					if (image->get_format() == Image::FORMAT_RGBA8) {
						atlas_channels = 4;
					}
					if (image->get_format() == Image::FORMAT_LA8) {
						atlas_channels = MAX(2, atlas_channels);
					}
				} else {
					if (image->get_format() == Image::FORMAT_RGBA8) {
						atlas_channels = MAX(3, atlas_channels);
					}
					if (image->get_format() == Image::FORMAT_LA8) {
						atlas_channels = MAX(2, atlas_channels);
					}
				}
			}
		}
	}

	ERR_FAIL_COND_V(atlas_channels < 1 || atlas_channels > 4, Dictionary());

	Image::Format atlas_format = Image::FORMAT_RGBA8;
	switch (atlas_channels) {
		case 1: {
			atlas_format = Image::FORMAT_L8;
		} break;
		case 2: {
			atlas_format = Image::FORMAT_LA8;
		} break;
		case 3: {
			atlas_format = Image::FORMAT_RGB8;
		} break;
	}

	Array generated_images;
	std::vector<bin> bins;
	Dictionary ret;

	// When separate_alpha is true: pack opaque group first, then alpha group.
	// The bin indices of the alpha group are offset by the number of opaque bins.
	bool pack_ok = false;
	if (options.separate_alpha && alpha_indices.size() > 0 && opaque_indices.size() > 0) {
		// Build sub-arrays of rects for each group.
		Vector<rect_xywhf *> opaque_rects, alpha_rects;
		for (int i = 0; i < opaque_indices.size(); ++i)
			opaque_rects.push_back(rects[opaque_indices[i]]);
		for (int i = 0; i < alpha_indices.size(); ++i)
			alpha_rects.push_back(rects[alpha_indices[i]]);

		std::vector<bin> alpha_bins;
		bool ok1 = _pack_rects(opaque_rects.ptr(), opaque_rects.size(), options.max_atlas_size, options.force_single_page_atlas, options.allow_rotation, options.algorithm, bins);
		bool ok2 = _pack_rects(alpha_rects.ptr(), alpha_rects.size(), options.max_atlas_size, options.force_single_page_atlas, options.allow_rotation, options.algorithm, alpha_bins);

		// Renumber alpha-group bin indices (offset by opaque bin count).
		const int offset = (int)bins.size();
		for (auto &b : alpha_bins) {
			for (rect_xywhf *r : b.rects)
				r->bin += offset;
			bins.push_back(b);
		}
		pack_ok = ok1 && ok2;
	} else {
		// Normal path: pack everything together (or only one group exists).
		pack_ok = _pack_rects(rects.ptr(), rects.size(), options.max_atlas_size, options.force_single_page_atlas, options.allow_rotation, options.algorithm, bins);
	}

	if (pack_ok) {
		// --- Post-process bin sizes: power-of-two and/or square ---
		if (options.power_of_two || options.square_atlas) {
			for (auto &b : bins) {
				int bw = b.size.w();
				int bh = b.size.h();
				if (options.square_atlas) {
					bw = bh = MAX(bw, bh);
				}
				if (options.power_of_two) {
					bw = _next_pow2(bw);
					bh = _next_pow2(bh);
				}
				b.size = rect_wh(bw, bh);
			}
		}

		generated_images.clear();
		generated_images.resize(bins.size());

		for (size_t i = 0; i < bins.size(); ++i) {
			const bin b = bins[i];

			const Size2 atlas_size = b.size.size();
			PoolByteArray atlas_data;
			atlas_data.resize(atlas_size.width * atlas_size.height * atlas_channels);

			// Setup background color
			const uint8_t cr = background_color.r * 255.0;
			const uint8_t cg = background_color.g * 255.0;
			const uint8_t cb = background_color.b * 255.0;
			const uint8_t ca = background_color.a * 255.0;

			for (int j = 0; j < atlas_data.size(); j += atlas_channels) {
				if (atlas_channels == 1) {
					static uint8_t c = (cr + cg + cb) / 3;
					atlas_data.set(j, c);
				} else {
					atlas_data.set(j, cr);
					atlas_data.set(j + 1, cg);
					atlas_data.set(j + 2, cb);
					if (atlas_channels == 4) {
						atlas_data.set(j + 3, ca);
					}
				}
			}

			Ref<Image> atlas;
			atlas.instance();

			// Process rects
			for (size_t j = 0; j < b.rects.size(); ++j) {
				rect_xywhf *r = b.rects[j];

				r->bin = i;
				r->atlas_image = atlas;

				Ref<Image> img = r->original_image;
				if (r->scale != 1) {
					// Note: scale is only used by BSP autoscaling (PACK_BSP, single_page).
					// When flipped, scale is not combined with rotation.
					img = img->resized(r->flipped ? r->h() : r->w(), r->flipped ? r->w() : r->h());
				}

				ERR_CONTINUE(!img.is_valid());

				// image_size is always the pre-rotation (original) dimensions.
				const int img_w = img->get_width();
				const int img_h = img->get_height();
				PoolByteArray image_data = img->get_data();

				int input_format_offset = _get_offset_for_format(img->get_format());
				ERR_CONTINUE_MSG(input_format_offset == 0, "Image format is not supported, skipping.");

				if (!r->flipped) {
					// --- Normal (non-rotated) blit ---
					for (int y = 0; y < r->h(); ++y) {
						const int orig_img_indx = y * img_w * input_format_offset;
						const int start_indx = (r->y + y) * (int)atlas_size.width * atlas_channels + r->x * atlas_channels;

						for (int x = 0; x < r->w(); ++x) {
							switch (input_format_offset) {
								case 4:
								case 3: {
									for (int sx = 0; sx < input_format_offset; ++sx) {
										atlas_data.set(start_indx + (x * atlas_channels) + sx, image_data[orig_img_indx + sx + (x * input_format_offset)]);
									}
								} break;
								case 2: {
									// grey + alpha
									for (int sx = 0; sx < 4; ++sx) {
										if (sx == 3 && atlas_channels == 4) {
											atlas_data.set(start_indx + (x * atlas_channels) + sx, image_data[orig_img_indx + 1 + (x * input_format_offset)]);
										} else {
											atlas_data.set(start_indx + (x * atlas_channels) + sx, image_data[orig_img_indx + 0 + (x * input_format_offset)]);
										}
									}
								} break;
								case 1: {
									// alpha / L8
									const uint8_t a = image_data[orig_img_indx + (x * input_format_offset)];
									if (atlas_channels == 1) {
										atlas_data.set(start_indx + x, a);
									} else {
										for (int sx = 0; sx < 4; ++sx) {
											if (sx == 3 && atlas_channels == 4) {
												atlas_data.set(start_indx + (x * atlas_channels) + sx, a);
											} else {
												if (atlas_channels == 4) {
													atlas_data.set(start_indx + (x * atlas_channels) + sx, 255);
												} else if (atlas_channels == 3) {
													atlas_data.set(start_indx + (x * atlas_channels) + sx, a);
												}
											}
										}
									}
								}
							}
						}
					}
				} else {
					// --- Rotated 90° CW blit ---
					// After flip(): r->w() = img_h (atlas width), r->h() = img_w (atlas height).
					// Mapping: src(sx, sy) -> atlas(r->x + (img_h-1-sy), r->y + sx)
					for (int sy = 0; sy < img_h; ++sy) {
						for (int sx = 0; sx < img_w; ++sx) {
							const int src_idx = sy * img_w * input_format_offset + sx * input_format_offset;
							const int atlas_x = r->x + (img_h - 1 - sy);
							const int atlas_y = r->y + sx;
							const int dst_idx = atlas_y * (int)atlas_size.width * atlas_channels + atlas_x * atlas_channels;

							switch (input_format_offset) {
								case 4:
								case 3: {
									for (int sx2 = 0; sx2 < input_format_offset; ++sx2) {
										atlas_data.set(dst_idx + sx2, image_data[src_idx + sx2]);
									}
								} break;
								case 2: {
									for (int sx2 = 0; sx2 < atlas_channels && sx2 < 4; ++sx2) {
										if (sx2 == 3 && atlas_channels == 4) {
											atlas_data.set(dst_idx + sx2, image_data[src_idx + 1]);
										} else {
											atlas_data.set(dst_idx + sx2, image_data[src_idx + 0]);
										}
									}
								} break;
								case 1: {
									const uint8_t a = image_data[src_idx];
									if (atlas_channels == 1) {
										atlas_data.set(dst_idx, a);
									} else {
										for (int sx2 = 0; sx2 < atlas_channels && sx2 < 4; ++sx2) {
											if (sx2 == 3 && atlas_channels == 4) {
												atlas_data.set(dst_idx + sx2, a);
											} else if (atlas_channels == 4) {
												atlas_data.set(dst_idx + sx2, 255);
											} else if (atlas_channels == 3) {
												atlas_data.set(dst_idx + sx2, a);
											}
										}
									}
								}
							}
						}
					}
				}
				// --- Debug border overlay ---
				if (options.debug_borders) {
					_draw_debug_border(atlas_data, (int)atlas_size.width, atlas_channels,
							r->x, r->y, r->w(), r->h(), margin, options.debug_border_color);
				}
			}

			atlas->create(atlas_size.width, atlas_size.height, false, atlas_format, atlas_data);
			generated_images.set(i, atlas);
		}

		Array atlas_rects;
		ERR_FAIL_COND_V(atlas_rects.resize(data.size()) != OK, Dictionary());
		for (int r = 0; r < data.size(); ++r) {
			const rect_xywhf &rc = data[r];
			Dictionary entry;
			// When flipped: rc.w() = img_h, rc.h() = img_w; content area excludes margin.
			const int content_w = rc.flipped ? (rc.w() - 2 * margin) : (rc.w() - 2 * margin);
			const int content_h = rc.flipped ? (rc.h() - 2 * margin) : (rc.h() - 2 * margin);
			entry["rect"] = Rect2(rc.x + margin, rc.y + margin, content_w, content_h);
			entry["rrect"] = Rect2(Point2(rc.x + margin, rc.y + margin) / rc.atlas_image->get_size(), Size2(content_w, content_h) / rc.atlas_image->get_size());
			entry["atlas_page"] = rc.bin;
			entry["atlas"] = rc.atlas_image;
			entry["flipped"] = rc.flipped;
			// trim_* are pixels removed from the original image before packing.
			// Callers reconstruct: original_rect = expand rect by (trim_l, trim_t, trim_r, trim_b)
			entry["trim_l"] = rc.trim_l;
			entry["trim_r"] = rc.trim_r;
			entry["trim_t"] = rc.trim_t;
			entry["trim_b"] = rc.trim_b;
			atlas_rects[r] = entry;
		}

		ret["_rects"] = atlas_rects;
		ret["_generated_images"] = generated_images;
		Array bins_size;
		ERR_FAIL_COND_V(bins_size.resize(bins.size()) != OK, Dictionary());
		for (size_t i = 0; i < bins.size(); ++i) {
			bins_size[i] = Size2(bins[i].size.w(), bins[i].size.h());
		}
		ret["_bins_size"] = bins_size;
	} else {
		WARN_PRINT("Packing of " + String::num(images.size()) + " images failed.");
	}

	return ret;
}

#ifdef DOCTEST

#include "doctest/doctest_godot.h"

#include <vector>

// Test helper: create a solid RGBA8 image of given size/color.
static Ref<Image> _make_solid_rgba(int w, int h, const Color &c) {
	Ref<Image> img = memnew(Image);
	img->create(w, h, false, Image::FORMAT_RGBA8);
	img->fill(c);
	return img;
}

TEST_SUITE("[[gd_pack]] ImagePacker") {
	TEST_CASE("[gd_pack] packing functions") {
		_doctest_prepare_folder();

		// for s in 32 128 256 512 1024; do
		//   convert -size ${s}x$s xc:white test$s.png24
		//   pngcrush  -rem alla test$s.png24 test$s.png
		//   ls -l test$s* && ~/Private/Projekty/0.shared/common-dev-tools/res_tools/bin2c/bin2c -o /dev/stdin test$s.png
		//   rm test$s.*
		// done;
		static const struct {
			int image_size;
			int data_len;
			std::vector<uint8_t> png_data;
		} test_data[5] = {
			{ 32, 80,
					{ 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
							0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x08, 0x00, 0x00, 0x00, 0x00, 0x56, 0x11, 0x25,
							0x28, 0x00, 0x00, 0x00, 0x17, 0x49, 0x44, 0x41, 0x54, 0x38, 0xCB, 0x63, 0xFC, 0xCF, 0x80, 0x1F,
							0x30, 0x8E, 0x2A, 0x18, 0x55, 0x30, 0xAA, 0x60, 0xA4, 0x2A, 0x00, 0x00, 0xF8, 0x2D, 0x20, 0x01,
							0x4F, 0x2A, 0xA0, 0xAE, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82 } },
			{ 128, 148,
					{ 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
							0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80, 0x08, 0x00, 0x00, 0x00, 0x00, 0xE6, 0x55, 0x3E,
							0x17, 0x00, 0x00, 0x00, 0x5B, 0x49, 0x44, 0x41, 0x54, 0x78, 0x5E, 0xED, 0xCE, 0x31, 0x01, 0x00,
							0x00, 0x0C, 0x02, 0xA0, 0xD9, 0x3F, 0xF4, 0x8C, 0xE1, 0x03, 0x09, 0xC8, 0xDF, 0x56, 0x04, 0x04,
							0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
							0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
							0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
							0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0xD6, 0x81, 0x02,
							0xF4, 0xC0, 0x80, 0x01, 0xE0, 0x6A, 0x61, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44,
							0xAE, 0x42, 0x60, 0x82 } },
			{ 256, 369,
					{ 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
							0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x79, 0x19, 0xF7,
							0xBA, 0x00, 0x00, 0x01, 0x38, 0x49, 0x44, 0x41, 0x54, 0x78, 0xDA, 0xED, 0xD0, 0x01, 0x01, 0x00,
							0x00, 0x08, 0x02, 0x20, 0xFD, 0x3F, 0x3A, 0x87, 0x04, 0x13, 0xE8, 0xE5, 0xB7, 0x0A, 0x10, 0x20,
							0x40, 0x80, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x01, 0x02, 0x04, 0x08,
							0x10, 0x20, 0x40, 0x80, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x01, 0x02,
							0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00,
							0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40,
							0x80, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10,
							0x20, 0x40, 0x80, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x01, 0x02, 0x04,
							0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x01,
							0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
							0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20,
							0x40, 0x80, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x01, 0x02, 0x04, 0x08,
							0x10, 0x20, 0x40, 0x80, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x01, 0x02,
							0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00,
							0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40,
							0x80, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10,
							0x20, 0x40, 0x80, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x01, 0x02, 0x04,
							0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x01,
							0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
							0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x01, 0xC9, 0x00, 0x11, 0x78, 0x00,
							0x10, 0x9C, 0xDD, 0xB8, 0x38, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60,
							0x82 } },
			{ 512, 854,
					{ 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
							0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0xD1, 0x13, 0x8B,
							0x26, 0x00, 0x00, 0x03, 0x1D, 0x49, 0x44, 0x41, 0x54, 0x78, 0xDA, 0xED, 0xD2, 0x31, 0x01, 0x00,
							0x00, 0x08, 0x03, 0x20, 0xD7, 0x3F, 0xB4, 0xBB, 0xCD, 0x20, 0x64, 0x20, 0x3B, 0x7C, 0x16, 0x01,
							0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04,
							0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40,
							0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00,
							0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04,
							0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40,
							0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00,
							0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04,
							0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x10,
							0x40, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10,
							0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00,
							0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01,
							0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10,
							0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00,
							0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01,
							0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10,
							0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00,
							0x01, 0x04, 0x10, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40,
							0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00,
							0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04,
							0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40,
							0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00,
							0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04,
							0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40,
							0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00,
							0x04, 0x40, 0x00, 0x01, 0x04, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00,
							0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01,
							0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10,
							0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00,
							0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01,
							0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10,
							0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00,
							0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01,
							0x10, 0x00, 0x01, 0x10, 0x40, 0x00, 0x01, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00,
							0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04,
							0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40,
							0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00,
							0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04,
							0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40,
							0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00,
							0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x40, 0x00, 0x04,
							0x40, 0x00, 0x04, 0x40, 0x00, 0x04, 0x10, 0x40, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01,
							0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10,
							0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00,
							0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01,
							0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10,
							0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00,
							0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01,
							0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10, 0x00, 0x01, 0x10,
							0x80, 0xAB, 0x43, 0xB1, 0x00, 0x1F, 0xDE, 0xFC, 0x68, 0xA2, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45,
							0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82 } },
			{ 1024, 2389,
					{ 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
							0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x5A, 0x76, 0x74,
							0x5F, 0x00, 0x00, 0x09, 0x1C, 0x49, 0x44, 0x41, 0x54, 0x78, 0xDA, 0xED, 0xD4, 0x31, 0x01, 0x00,
							0x00, 0x08, 0xC3, 0x30, 0xE6, 0x5F, 0x34, 0x13, 0xC1, 0x49, 0x22, 0xA1, 0x47, 0xB3, 0x03, 0x7C,
							0x15, 0x03, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00,
							0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00,
							0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00,
							0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0,
							0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30,
							0x00, 0xC0, 0x00, 0xC0, 0x00, 0x34, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00,
							0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0,
							0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30,
							0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C,
							0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03,
							0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00,
							0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00,
							0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00,
							0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00,
							0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0,
							0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C,
							0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03,
							0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00,
							0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00,
							0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00,
							0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0xC0, 0x00, 0x00,
							0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0,
							0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30,
							0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C,
							0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03,
							0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00,
							0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00,
							0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00,
							0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00,
							0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0,
							0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30,
							0x00, 0xC0, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03,
							0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00,
							0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00,
							0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00,
							0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00,
							0x0C, 0x00, 0x30, 0x00, 0x30, 0x00, 0x03, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00,
							0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00,
							0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00,
							0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00,
							0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00,
							0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0xC0, 0x00, 0x34, 0x00, 0x03, 0x00, 0x0C, 0x00,
							0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00,
							0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00,
							0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0,
							0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30,
							0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0xC0, 0x00, 0x00, 0x03,
							0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00,
							0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00,
							0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00,
							0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00,
							0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0xC0,
							0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30,
							0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C,
							0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03,
							0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00,
							0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00,
							0xC0, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00,
							0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00,
							0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0,
							0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30,
							0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C,
							0x00, 0x30, 0x00, 0xC0, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00,
							0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00,
							0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00,
							0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00,
							0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00,
							0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30,
							0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C,
							0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03,
							0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00,
							0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00,
							0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0x30, 0x00, 0x03, 0x00, 0x03, 0x00, 0x0C,
							0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03,
							0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00,
							0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00,
							0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00,
							0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0xC0, 0x00, 0x34,
							0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00,
							0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00,
							0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00,
							0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00,
							0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0,
							0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C,
							0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03,
							0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00,
							0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00,
							0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00,
							0x30, 0x00, 0xC0, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00,
							0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0,
							0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30,
							0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C,
							0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03,
							0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00,
							0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00,
							0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00,
							0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00,
							0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0,
							0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C,
							0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03,
							0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00,
							0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00,
							0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00,
							0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0xC0, 0x00, 0x00,
							0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0,
							0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30,
							0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C,
							0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03,
							0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0x30, 0x00,
							0x03, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30,
							0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C,
							0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03,
							0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00,
							0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00,
							0xC0, 0x00, 0xC0, 0x00, 0x34, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03,
							0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00,
							0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00,
							0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00,
							0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00,
							0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0,
							0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30,
							0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C,
							0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03,
							0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00,
							0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00,
							0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00,
							0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00,
							0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0,
							0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30,
							0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0xC0, 0x00, 0x00, 0x03,
							0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00,
							0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00,
							0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00,
							0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00,
							0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0xC0,
							0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30,
							0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C,
							0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03,
							0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00,
							0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00,
							0xC0, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00,
							0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00,
							0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0,
							0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0x30, 0x00, 0xC0, 0x00, 0x00, 0x03, 0x00, 0x0C, 0x00, 0xB8,
							0x29, 0x15, 0x01, 0x00, 0x3D, 0x65, 0x1B, 0x47, 0xA5, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E,
							0x44, 0xAE, 0x42, 0x60, 0x82 } }
		};
		Vector<Ref<Image>> empty_set;
		Vector<Ref<Image>> test_set;
		for (int i = 0; i < 5; i++) {
			test_set.push_back(memnew(Image(test_data[i].png_data.data(), test_data[i].data_len)));
		}
		SUBCASE("empty set") {
			REQUIRE(merge_images(empty_set).empty());
		}
		SUBCASE("default packing") {
			const ImageMergeOptions options = ImageMergeOptions();
			Dictionary res;
			SUPPRESS_OUTPUT(res = merge_images(test_set, options));
			REQUIRE(!res.empty());
			REQUIRE(Array(res["_generated_images"]).size() == 1);
			Ref<Image> atlas = Array(res["_generated_images"])[0];
			Size2 area = Array(res["_bins_size"])[0];
			REQUIRE(atlas->get_size() == area);
			atlas->save_png(_doctest_get_folder() + "atlas_default_packing.png");
		}
		SUBCASE("packing to max. size 1000 with single page") {
			const ImageMergeOptions options = ImageMergeOptions().set_max_size(1000).set_single_page(true);
			Dictionary res;
			SUPPRESS_OUTPUT(res = merge_images(test_set, options));
			REQUIRE(!res.empty());
			REQUIRE(Array(res["_generated_images"]).size() == 1);
			Ref<Image> atlas = Array(res["_generated_images"])[0];
			atlas->save_png(_doctest_get_folder() + "atlas_max1000_with_single_page.png");
		}
		SUBCASE("packing big images to max. size 1000 with single page") {
			const ImageMergeOptions options = ImageMergeOptions().set_max_size(1000).set_single_page(true);
			Vector<Ref<Image>> test_set_1;
			test_set_1.push_back(test_set[3], test_set[4], test_set[4]);
			Dictionary res;
			SUPPRESS_OUTPUT(res = merge_images(test_set_1, options));
			REQUIRE(!res.empty());
			REQUIRE(Array(res["_generated_images"]).size() == 1);
			Ref<Image> atlas = Array(res["_generated_images"])[0];
			atlas->save_png(_doctest_get_folder() + "atlas_big_images_max1000_with_single_page.png");
		}
		SUBCASE("packing small images to max. size 1000 with single page") {
			const ImageMergeOptions options = ImageMergeOptions().set_max_size(1000).set_single_page(true);
			Vector<Ref<Image>> test_set_1;
			test_set_1.push_back(test_set[0], test_set[0], test_set[1]);
			Dictionary res;
			SUPPRESS_OUTPUT(res = merge_images(test_set_1, options));
			REQUIRE(!res.empty());
			REQUIRE(Array(res["_generated_images"]).size() == 1);
			Ref<Image> atlas = Array(res["_generated_images"])[0];
			atlas->save_png(_doctest_get_folder() + "atlas_small_images_max1000_with_single_page.png");
		}
	}

	// -----------------------------------------------------------------------
	// Algorithm selection tests
	// -----------------------------------------------------------------------
	TEST_CASE("[gd_pack] Guillotine algorithm produces valid atlas") {
		_doctest_prepare_folder();
		Ref<Image> img16 = _make_solid_rgba(16, 16, Color(1, 0, 0, 1));
		Ref<Image> img32 = _make_solid_rgba(32, 32, Color(0, 1, 0, 1));
		Ref<Image> img8 = _make_solid_rgba(8, 24, Color(0, 0, 1, 1));
		Vector<Ref<Image>> imgs;
		imgs.push_back(img16, img32, img8);

		ImageMergeOptions opts;
		opts.algorithm = PACK_GUILLOTINE;
		opts.margin = 0;

		Dictionary res;
		SUPPRESS_OUTPUT(res = merge_images(imgs, opts));
		REQUIRE(!res.empty());
		Array pages = res["_generated_images"];
		REQUIRE(pages.size() >= 1);
		Array rects = res["_rects"];
		REQUIRE(rects.size() == 3);

		// All rects must have non-zero size.
		for (int i = 0; i < rects.size(); ++i) {
			Dictionary entry = rects[i];
			Rect2 r = entry["rect"];
			CHECK(r.size.width > 0);
			CHECK(r.size.height > 0);
		}
		Ref<Image>(pages[0])->save_png(_doctest_get_folder() + "algo_guillotine.png");
	}

	TEST_CASE("[gd_pack] MaxRects algorithm produces valid atlas") {
		_doctest_prepare_folder();
		Ref<Image> img16 = _make_solid_rgba(16, 16, Color(1, 0, 0, 1));
		Ref<Image> img32 = _make_solid_rgba(32, 32, Color(0, 1, 0, 1));
		Ref<Image> img8 = _make_solid_rgba(8, 24, Color(0, 0, 1, 1));
		Vector<Ref<Image>> imgs;
		imgs.push_back(img16, img32, img8);

		ImageMergeOptions opts;
		opts.algorithm = PACK_MAXRECTS;
		opts.margin = 0;

		Dictionary res;
		SUPPRESS_OUTPUT(res = merge_images(imgs, opts));
		REQUIRE(!res.empty());
		Array pages = res["_generated_images"];
		REQUIRE(pages.size() >= 1);
		Array rects = res["_rects"];
		REQUIRE(rects.size() == 3);

		for (int i = 0; i < rects.size(); ++i) {
			Dictionary entry = rects[i];
			Rect2 r = entry["rect"];
			CHECK(r.size.width > 0);
			CHECK(r.size.height > 0);
		}
		Ref<Image>(pages[0])->save_png(_doctest_get_folder() + "algo_maxrects.png");
	}

	TEST_CASE("[gd_pack] all three algorithms pack same set to same total pixel count") {
		// BSP, Guillotine, MaxRects must all place every image (no data loss).
		Ref<Image> a = _make_solid_rgba(20, 10, Color(1, 0, 0, 1));
		Ref<Image> b = _make_solid_rgba(10, 30, Color(0, 1, 0, 1));
		Ref<Image> c = _make_solid_rgba(15, 15, Color(0, 0, 1, 1));
		Vector<Ref<Image>> imgs;
		imgs.push_back(a, b, c);

		for (int algo = 0; algo < 3; ++algo) {
			ImageMergeOptions opts;
			opts.algorithm = (PackingAlgorithm)algo;
			opts.margin = 0;
			Dictionary res;
			SUPPRESS_OUTPUT(res = merge_images(imgs, opts));
			INFO("algorithm=" << algo);
			REQUIRE(!res.empty());
			CHECK(Array(res["_rects"]).size() == 3);
		}
	}

	TEST_CASE("[gd_pack] multi-page packing with Guillotine") {
		// Force a tiny max_atlas_size so rects spill to multiple bins.
		Ref<Image> img = _make_solid_rgba(64, 64, Color(1, 1, 0, 1));
		Vector<Ref<Image>> imgs;
		imgs.push_back(img, img, img);

		ImageMergeOptions opts;
		opts.algorithm = PACK_GUILLOTINE;
		opts.max_atlas_size = 80; // too small for all three 64x64 in one page
		opts.force_single_page_atlas = false;
		opts.margin = 0;

		Dictionary res;
		SUPPRESS_OUTPUT(res = merge_images(imgs, opts));
		REQUIRE(!res.empty());
		CHECK(Array(res["_generated_images"]).size() > 1);
	}

	TEST_CASE("[gd_pack] multi-page packing with MaxRects") {
		Ref<Image> img = _make_solid_rgba(64, 64, Color(0, 1, 1, 1));
		Vector<Ref<Image>> imgs;
		imgs.push_back(img, img, img);

		ImageMergeOptions opts;
		opts.algorithm = PACK_MAXRECTS;
		opts.max_atlas_size = 80;
		opts.force_single_page_atlas = false;
		opts.margin = 0;

		Dictionary res;
		SUPPRESS_OUTPUT(res = merge_images(imgs, opts));
		REQUIRE(!res.empty());
		CHECK(Array(res["_generated_images"]).size() > 1);
	}

	// -----------------------------------------------------------------------
	// Rotation tests
	// -----------------------------------------------------------------------
	TEST_CASE("[gd_pack] allow_rotation flag is recorded in output") {
		_doctest_prepare_folder();
		// Use a very tall image so it's a candidate for rotation.
		Ref<Image> tall = _make_solid_rgba(4, 64, Color(1, 0.5, 0, 1));
		Ref<Image> wide = _make_solid_rgba(64, 4, Color(0.5, 1, 0, 1));
		Vector<Ref<Image>> imgs;
		imgs.push_back(tall, wide);

		ImageMergeOptions opts;
		opts.algorithm = PACK_MAXRECTS;
		opts.allow_rotation = true;
		opts.margin = 0;

		Dictionary res;
		SUPPRESS_OUTPUT(res = merge_images(imgs, opts));
		REQUIRE(!res.empty());
		Array rects = res["_rects"];
		REQUIRE(rects.size() == 2);

		// 'flipped' key must exist in every entry.
		for (int i = 0; i < rects.size(); ++i) {
			Dictionary entry = rects[i];
			CHECK(entry.has("flipped"));
		}

		// Atlas pixel dimensions should reflect any rotations applied.
		Ref<Image> atlas = Array(res["_generated_images"])[0];
		CHECK(atlas->get_width() > 0);
		CHECK(atlas->get_height() > 0);
		atlas->save_png(_doctest_get_folder() + "rotation_test.png");
	}

	TEST_CASE("[gd_pack] rotation=false never sets flipped=true") {
		Ref<Image> img = _make_solid_rgba(10, 40, Color(1, 0, 1, 1));
		Vector<Ref<Image>> imgs;
		imgs.push_back(img, img);

		ImageMergeOptions opts;
		opts.algorithm = PACK_MAXRECTS;
		opts.allow_rotation = false;
		opts.margin = 0;

		Dictionary res;
		SUPPRESS_OUTPUT(res = merge_images(imgs, opts));
		REQUIRE(!res.empty());
		Array rects = res["_rects"];
		for (int i = 0; i < rects.size(); ++i) {
			CHECK(!bool(Dictionary(rects[i])["flipped"]));
		}
	}

	// -----------------------------------------------------------------------
	// Alpha trimming tests
	// -----------------------------------------------------------------------
	TEST_CASE("[gd_pack] trim_alpha reduces packed size") {
		_doctest_prepare_folder();

		// 32x32 image with an 8-pixel transparent border (inner 16x16 is opaque).
		Ref<Image> img = memnew(Image);
		img->create(32, 32, false, Image::FORMAT_RGBA8);
		img->fill(Color(0, 0, 0, 0));
		img->lock();
		for (int y = 8; y < 24; ++y)
			for (int x = 8; x < 24; ++x)
				img->set_pixel(x, y, Color(1, 0, 0, 1));
		img->unlock();

		Vector<Ref<Image>> imgs;
		imgs.push_back(img);

		SUBCASE("without trim: rect is 32x32") {
			ImageMergeOptions opts;
			opts.trim_alpha = false;
			opts.margin = 0;
			Dictionary res;
			SUPPRESS_OUTPUT(res = merge_images(imgs, opts));
			REQUIRE(!res.empty());
			Rect2 r = Dictionary(Array(res["_rects"])[0])["rect"];
			CHECK(r.size.width == 32);
			CHECK(r.size.height == 32);
		}

		SUBCASE("with trim: rect is 16x16, trim margins = 8") {
			ImageMergeOptions opts;
			opts.trim_alpha = true;
			opts.trim_alpha_threshold = 0;
			opts.margin = 0;
			Dictionary res;
			SUPPRESS_OUTPUT(res = merge_images(imgs, opts));
			REQUIRE(!res.empty());
			Dictionary entry = Array(res["_rects"])[0];
			Rect2 r = entry["rect"];
			CHECK(r.size.width == 16);
			CHECK(r.size.height == 16);
			CHECK(int(entry["trim_l"]) == 8);
			CHECK(int(entry["trim_t"]) == 8);
			CHECK(int(entry["trim_r"]) == 8);
			CHECK(int(entry["trim_b"]) == 8);
		}

		SUBCASE("with trim: atlas is smaller than without trim") {
			ImageMergeOptions no_trim_opts, trim_opts;
			no_trim_opts.trim_alpha = false;
			no_trim_opts.margin = 0;
			trim_opts.trim_alpha = true;
			trim_opts.margin = 0;

			Dictionary res_no, res_trim;
			SUPPRESS_OUTPUT(res_no = merge_images(imgs, no_trim_opts));
			SUPPRESS_OUTPUT(res_trim = merge_images(imgs, trim_opts));

			Size2 s_no = Array(res_no["_bins_size"])[0];
			Size2 s_trim = Array(res_trim["_bins_size"])[0];
			CHECK(s_trim.width <= s_no.width);
			CHECK(s_trim.height <= s_no.height);
		}
	}

	TEST_CASE("[gd_pack] trim_alpha: fully-transparent image returns zero-size trim markers") {
		Ref<Image> fully_trans = memnew(Image);
		fully_trans->create(16, 16, false, Image::FORMAT_RGBA8);
		fully_trans->fill(Color(0, 0, 0, 0));

		int tl, tr, tt, tb;
		_trim_alpha_border(fully_trans, 0, tl, tr, tt, tb);
		// All zeros because nothing is opaque - we fall through to the guard.
		CHECK(tl == 0);
		CHECK(tr == 0);
		CHECK(tt == 0);
		CHECK(tb == 0);
	}

	TEST_CASE("[gd_pack] trim_alpha with threshold=127 trims semi-transparent pixels") {
		Ref<Image> img = memnew(Image);
		img->create(10, 10, false, Image::FORMAT_RGBA8);
		img->fill(Color(0, 0, 0, 0));
		img->lock();
		// Row 0 has alpha=64 (below threshold=127), row 5 has alpha=255.
		for (int x = 0; x < 10; ++x) {
			img->set_pixel(x, 0, Color(1, 1, 1, 64.0f / 255.0f));
			img->set_pixel(x, 5, Color(1, 0, 0, 1));
		}
		img->unlock();

		int tl, tr, tt, tb;
		// With threshold=127: row 0 (alpha=64) should be trimmed.
		_trim_alpha_border(img, 127, tl, tr, tt, tb);
		CHECK(tt >= 1); // at least the semi-transparent top row trimmed
	}

	TEST_CASE("[gd_pack] trim_alpha output contains trim keys in merge_images") {
		Ref<Image> img = memnew(Image);
		img->create(20, 20, false, Image::FORMAT_RGBA8);
		img->fill(Color(0, 0, 0, 0));
		img->lock();
		for (int y = 5; y < 15; ++y)
			for (int x = 5; x < 15; ++x)
				img->set_pixel(x, y, Color(0, 1, 0, 1));
		img->unlock();

		Vector<Ref<Image>> imgs;
		imgs.push_back(img);
		ImageMergeOptions opts;
		opts.trim_alpha = true;
		opts.margin = 0;

		Dictionary res;
		SUPPRESS_OUTPUT(res = merge_images(imgs, opts));
		REQUIRE(!res.empty());
		Dictionary entry = Array(res["_rects"])[0];
		CHECK(entry.has("trim_l"));
		CHECK(entry.has("trim_r"));
		CHECK(entry.has("trim_t"));
		CHECK(entry.has("trim_b"));
		CHECK(entry.has("flipped"));
	}

	// -----------------------------------------------------------------------
	// Halo fix tests
	// -----------------------------------------------------------------------
	TEST_CASE("[gd_pack] fix_halo repairs white fringe on adjacent transparent pixels") {
		// Build a 3x3 RGBA8 image: center pixel is red+opaque, all others alpha=0+white.
		Ref<Image> img = memnew(Image);
		img->create(3, 3, false, Image::FORMAT_RGBA8);
		img->lock();
		for (int y = 0; y < 3; ++y)
			for (int x = 0; x < 3; ++x)
				img->set_pixel(x, y, Color(1, 1, 1, 0)); // white, transparent
		img->set_pixel(1, 1, Color(1, 0, 0, 1)); // red, opaque center
		img->unlock();

		_fix_halo(img);

		img->lock();
		// The 8 neighbours of the center should now have reddish tint (not pure white).
		for (int dy = -1; dy <= 1; ++dy) {
			for (int dx = -1; dx <= 1; ++dx) {
				if (dx == 0 && dy == 0)
					continue;
				Color c = img->get_pixel(1 + dx, 1 + dy);
				CHECK(c.a == 0); // still transparent
				CHECK(c.r > 0); // repaired to red-ish
			}
		}
		img->unlock();
	}

	TEST_CASE("[gd_pack] fix_halo leaves opaque pixels unchanged") {
		Ref<Image> img = memnew(Image);
		img->create(4, 4, false, Image::FORMAT_RGBA8);
		img->fill(Color(0.2f, 0.5f, 0.8f, 1));
		Ref<Image> original = img->duplicate();

		_fix_halo(img);

		img->lock();
		original->lock();
		for (int y = 0; y < 4; ++y)
			for (int x = 0; x < 4; ++x)
				CHECK(img->get_pixel(x, y) == original->get_pixel(x, y));
		img->unlock();
		original->unlock();
	}

	TEST_CASE("[gd_pack] fix_halo isolated transparent pixel surrounded by transparent stays unchanged") {
		// All pixels transparent, no neighbours to sample from - pixel should remain as-is.
		Ref<Image> img = memnew(Image);
		img->create(3, 3, false, Image::FORMAT_RGBA8);
		img->fill(Color(1, 1, 1, 0));

		_fix_halo(img);

		img->lock();
		for (int y = 0; y < 3; ++y)
			for (int x = 0; x < 3; ++x) {
				Color c = img->get_pixel(x, y);
				// No opaque neighbours to bleed from; remains white/transparent.
				CHECK(c.a == 0);
				CHECK(c.r == 1.0f);
			}
		img->unlock();
	}

	// -----------------------------------------------------------------------
	// Border mode tests
	// -----------------------------------------------------------------------
	TEST_CASE("[gd_pack] BORDER_EMPTY creates padded atlas without mirroring") {
		_doctest_prepare_folder();
		Ref<Image> img = _make_solid_rgba(8, 8, Color(1, 0, 0, 1));
		Vector<Ref<Image>> imgs;
		imgs.push_back(img);

		ImageMergeOptions opts;
		opts.border_mode = BORDER_EMPTY;
		opts.margin = 4;
		opts.background_color = Color(0, 0, 0, 0);

		Dictionary res;
		SUPPRESS_OUTPUT(res = merge_images(imgs, opts));
		REQUIRE(!res.empty());
		// Atlas should exist and contain the image.
		Ref<Image> atlas = Array(res["_generated_images"])[0];
		CHECK(atlas->get_width() >= 16); // 8 + 4 + 4
		atlas->save_png(_doctest_get_folder() + "border_empty.png");
	}

	TEST_CASE("[gd_pack] BORDER_MIRROR and BORDER_BLUR both extend image correctly") {
		_doctest_prepare_folder();
		Ref<Image> img = _make_solid_rgba(16, 16, Color(0, 1, 0, 1));
		Vector<Ref<Image>> imgs_m, imgs_b;
		imgs_m.push_back(img);
		imgs_b.push_back(img);

		ImageMergeOptions mirror_opts, blur_opts;
		mirror_opts.border_mode = BORDER_MIRROR;
		mirror_opts.margin = 4;
		blur_opts.border_mode = BORDER_BLUR;
		blur_opts.margin = 4;

		Dictionary res_m, res_b;
		SUPPRESS_OUTPUT(res_m = merge_images(imgs_m, mirror_opts));
		SUPPRESS_OUTPUT(res_b = merge_images(imgs_b, blur_opts));

		REQUIRE(!res_m.empty());
		REQUIRE(!res_b.empty());
		Ref<Image>(Array(res_m["_generated_images"])[0])->save_png(_doctest_get_folder() + "border_mirror.png");
		Ref<Image>(Array(res_b["_generated_images"])[0])->save_png(_doctest_get_folder() + "border_blur.png");
	}

	// -----------------------------------------------------------------------
	// Power-of-two and square atlas tests
	// -----------------------------------------------------------------------
	TEST_CASE("[gd_pack] _next_pow2 rounds up correctly") {
		CHECK(_next_pow2(1) == 1);
		CHECK(_next_pow2(2) == 2);
		CHECK(_next_pow2(3) == 4);
		CHECK(_next_pow2(4) == 4);
		CHECK(_next_pow2(5) == 8);
		CHECK(_next_pow2(64) == 64);
		CHECK(_next_pow2(65) == 128);
		CHECK(_next_pow2(255) == 256);
		CHECK(_next_pow2(256) == 256);
		CHECK(_next_pow2(257) == 512);
		CHECK(_next_pow2(1000) == 1024);
	}

	TEST_CASE("[gd_pack] power_of_two atlas has pow2 dimensions") {
		_doctest_prepare_folder();
		Ref<Image> img = _make_solid_rgba(10, 10, Color(0.5f, 0.5f, 1, 1));
		Vector<Ref<Image>> imgs;
		imgs.push_back(img, img, img);

		ImageMergeOptions opts;
		opts.power_of_two = true;
		opts.margin = 0;

		Dictionary res;
		SUPPRESS_OUTPUT(res = merge_images(imgs, opts));
		REQUIRE(!res.empty());
		Ref<Image> atlas = Array(res["_generated_images"])[0];
		int w = atlas->get_width();
		int h = atlas->get_height();
		// Check power of two.
		CHECK((w & (w - 1)) == 0);
		CHECK((h & (h - 1)) == 0);
		atlas->save_png(_doctest_get_folder() + "pow2_atlas.png");
	}

	TEST_CASE("[gd_pack] square_atlas has equal width and height") {
		_doctest_prepare_folder();
		Ref<Image> wide = _make_solid_rgba(64, 8, Color(1, 1, 0, 1));
		Ref<Image> tall = _make_solid_rgba(8, 32, Color(0, 1, 1, 1));
		Vector<Ref<Image>> imgs;
		imgs.push_back(wide, tall);

		ImageMergeOptions opts;
		opts.square_atlas = true;
		opts.margin = 0;

		Dictionary res;
		SUPPRESS_OUTPUT(res = merge_images(imgs, opts));
		REQUIRE(!res.empty());
		Ref<Image> atlas = Array(res["_generated_images"])[0];
		CHECK(atlas->get_width() == atlas->get_height());
		atlas->save_png(_doctest_get_folder() + "square_atlas.png");
	}

	TEST_CASE("[gd_pack] power_of_two + square_atlas combined") {
		Ref<Image> img = _make_solid_rgba(12, 12, Color(1, 0.5f, 0, 1));
		Vector<Ref<Image>> imgs;
		imgs.push_back(img, img);

		ImageMergeOptions opts;
		opts.power_of_two = true;
		opts.square_atlas = true;
		opts.margin = 0;

		Dictionary res;
		SUPPRESS_OUTPUT(res = merge_images(imgs, opts));
		REQUIRE(!res.empty());
		Ref<Image> atlas = Array(res["_generated_images"])[0];
		int w = atlas->get_width();
		int h = atlas->get_height();
		CHECK(w == h);
		CHECK((w & (w - 1)) == 0);
	}

	// -----------------------------------------------------------------------
	// Regression: default options still work (backward compatibility)
	// -----------------------------------------------------------------------
	TEST_CASE("[gd_pack] default options: flipped=false, trim_* keys present") {
		Ref<Image> img = _make_solid_rgba(8, 8, Color(0, 0, 1, 1));
		Vector<Ref<Image>> imgs;
		imgs.push_back(img);

		Dictionary res;
		SUPPRESS_OUTPUT(res = merge_images(imgs));
		REQUIRE(!res.empty());
		Dictionary entry = Array(res["_rects"])[0];
		CHECK(entry.has("flipped"));
		CHECK(!bool(entry["flipped"]));
		CHECK(int(entry["trim_l"]) == 0);
		CHECK(int(entry["trim_r"]) == 0);
		CHECK(int(entry["trim_t"]) == 0);
		CHECK(int(entry["trim_b"]) == 0);
	}

	// -----------------------------------------------------------------------
	// Color trim tests
	// -----------------------------------------------------------------------
	TEST_CASE("[gd_pack] _delta_e identical colors returns 0") {
		Color red(1, 0, 0, 1);
		CHECK(_delta_e(red, red) == doctest::Approx(0.0f).epsilon(0.001f));
	}

	TEST_CASE("[gd_pack] _delta_e clearly different colors returns large value") {
		Color white(1, 1, 1, 1);
		Color black(0, 0, 0, 1);
		CHECK(_delta_e(white, black) > 10.0f);
	}

	TEST_CASE("[gd_pack] _trim_color_border trims solid red border") {
		// 20x20 image: 4-pixel red border, 12x12 blue center.
		Ref<Image> img = memnew(Image);
		img->create(20, 20, false, Image::FORMAT_RGBA8);
		img->fill(Color(1, 0, 0, 1)); // fill red
		img->lock();
		for (int y = 4; y < 16; ++y)
			for (int x = 4; x < 16; ++x)
				img->set_pixel(x, y, Color(0, 0, 1, 1)); // blue center
		img->unlock();

		int tl = 0, tr = 0, tt = 0, tb = 0;
		Ref<Image> cropped = _trim_color_border(img, 5.0f, tl, tr, tt, tb);

		CHECK(tl == 4);
		CHECK(tr == 4);
		CHECK(tt == 4);
		CHECK(tb == 4);
		CHECK(cropped->get_width() == 12);
		CHECK(cropped->get_height() == 12);
	}

	TEST_CASE("[gd_pack] _trim_color_border no trim when all pixels differ") {
		// Gradient image — no uniform border to trim.
		Ref<Image> img = memnew(Image);
		img->create(8, 8, false, Image::FORMAT_RGBA8);
		img->lock();
		for (int y = 0; y < 8; ++y)
			for (int x = 0; x < 8; ++x)
				img->set_pixel(x, y, Color(x / 7.0f, y / 7.0f, 0, 1));
		img->unlock();

		int tl = 0, tr = 0, tt = 0, tb = 0;
		Ref<Image> result = _trim_color_border(img, 1.0f, tl, tr, tt, tb);
		// No uniform border; image returned unchanged.
		CHECK(tl == 0);
		CHECK(tr == 0);
		CHECK(tt == 0);
		CHECK(tb == 0);
	}

	TEST_CASE("[gd_pack] trim_color option reduces rect size in merge_images") {
		_doctest_prepare_folder();
		// 16x16 with a 3px solid white border, 10x10 red center.
		Ref<Image> img = memnew(Image);
		img->create(16, 16, false, Image::FORMAT_RGBA8);
		img->fill(Color(1, 1, 1, 1));
		img->lock();
		for (int y = 3; y < 13; ++y)
			for (int x = 3; x < 13; ++x)
				img->set_pixel(x, y, Color(1, 0, 0, 1));
		img->unlock();

		Vector<Ref<Image>> imgs;
		imgs.push_back(img);

		SUBCASE("without color trim: rect is 16x16") {
			ImageMergeOptions opts;
			opts.trim_color = false;
			opts.margin = 0;
			Dictionary res;
			SUPPRESS_OUTPUT(res = merge_images(imgs, opts));
			REQUIRE(!res.empty());
			Rect2 r = Dictionary(Array(res["_rects"])[0])["rect"];
			CHECK(r.size.width == 16);
			CHECK(r.size.height == 16);
		}

		SUBCASE("with color trim: rect is ~10x10") {
			ImageMergeOptions opts;
			opts.trim_color = true;
			opts.trim_color_threshold = 5.0f; // generous threshold
			opts.margin = 0;
			Dictionary res;
			SUPPRESS_OUTPUT(res = merge_images(imgs, opts));
			REQUIRE(!res.empty());
			Dictionary entry = Array(res["_rects"])[0];
			Rect2 r = entry["rect"];
			CHECK(r.size.width <= 10);
			CHECK(r.size.height <= 10);
			CHECK(int(entry["trim_l"]) >= 3);
			CHECK(int(entry["trim_t"]) >= 3);
		}
	}

	TEST_CASE("[gd_pack] trim_color and trim_alpha together accumulate margins") {
		// Image: 2px transparent border, then 2px white border, then opaque center.
		Ref<Image> img = memnew(Image);
		img->create(14, 14, false, Image::FORMAT_RGBA8);
		img->fill(Color(0, 0, 0, 0)); // fully transparent background
		img->lock();
		// White ring at 2..11
		for (int y = 2; y < 12; ++y)
			for (int x = 2; x < 12; ++x)
				img->set_pixel(x, y, Color(1, 1, 1, 1));
		// Red center at 4..9
		for (int y = 4; y < 10; ++y)
			for (int x = 4; x < 10; ++x)
				img->set_pixel(x, y, Color(1, 0, 0, 1));
		img->unlock();

		Vector<Ref<Image>> imgs;
		imgs.push_back(img);

		ImageMergeOptions opts;
		opts.trim_alpha = true;
		opts.trim_alpha_threshold = 0;
		opts.trim_color = true;
		opts.trim_color_threshold = 5.0f;
		opts.margin = 0;

		Dictionary res;
		SUPPRESS_OUTPUT(res = merge_images(imgs, opts));
		REQUIRE(!res.empty());
		Dictionary entry = Array(res["_rects"])[0];
		// After alpha trim: 2px removed. After color trim: 2px more removed.
		CHECK(int(entry["trim_l"]) >= 4);
		CHECK(int(entry["trim_t"]) >= 4);
	}

	// -----------------------------------------------------------------------
	// Debug borders tests
	// -----------------------------------------------------------------------
	TEST_CASE("[gd_pack] debug_borders marks pixels on atlas") {
		_doctest_prepare_folder();
		Ref<Image> img = _make_solid_rgba(16, 16, Color(0, 0.5f, 1, 1));
		Vector<Ref<Image>> imgs;
		imgs.push_back(img);

		ImageMergeOptions opts;
		opts.debug_borders = true;
		opts.debug_border_color = Color(1, 0, 1, 1); // magenta
		opts.margin = 4;
		opts.border_mode = BORDER_EMPTY;
		opts.background_color = Color(0, 0, 0, 0);

		Dictionary res;
		SUPPRESS_OUTPUT(res = merge_images(imgs, opts));
		REQUIRE(!res.empty());
		Ref<Image> atlas = Array(res["_generated_images"])[0];
		atlas->save_png(_doctest_get_folder() + "debug_borders.png");

		// The corner pixel at (margin, margin) must be magenta.
		Dictionary entry = Array(res["_rects"])[0];
		Rect2 rect = entry["rect"];
		// rect.position is content area start; the debug corner is right there.
		atlas->lock();
		Color corner = atlas->get_pixel((int)rect.position.x, (int)rect.position.y);
		atlas->unlock();
		// Magenta: R=1,G=0,B=1. Allow small float conversion error.
		CHECK(corner.r > 0.9f);
		CHECK(corner.g < 0.1f);
		CHECK(corner.b > 0.9f);
	}

	TEST_CASE("[gd_pack] debug_borders=false does not change atlas pixel at corner") {
		Ref<Image> img = _make_solid_rgba(8, 8, Color(0, 1, 0, 1));
		Vector<Ref<Image>> imgs;
		imgs.push_back(img);

		ImageMergeOptions no_debug, with_debug;
		no_debug.debug_borders = false;
		no_debug.margin = 2;
		no_debug.border_mode = BORDER_EMPTY;
		no_debug.background_color = Color(0, 0, 0, 0);

		with_debug = no_debug;
		with_debug.debug_borders = true;
		with_debug.debug_border_color = Color(1, 0, 1, 1);

		Dictionary res_no, res_yes;
		SUPPRESS_OUTPUT(res_no = merge_images(imgs, no_debug));
		SUPPRESS_OUTPUT(res_yes = merge_images(imgs, with_debug));

		REQUIRE(!res_no.empty());
		REQUIRE(!res_yes.empty());

		// Atlases must differ (debug overlay changed at least one pixel).
		Ref<Image> a_no = Array(res_no["_generated_images"])[0];
		Ref<Image> a_yes = Array(res_yes["_generated_images"])[0];
		a_no->lock();
		a_yes->lock();
		bool found_diff = false;
		for (int y = 0; y < a_no->get_height() && !found_diff; ++y)
			for (int x = 0; x < a_no->get_width() && !found_diff; ++x)
				if (a_no->get_pixel(x, y) != a_yes->get_pixel(x, y))
					found_diff = true;
		a_no->unlock();
		a_yes->unlock();
		CHECK(found_diff);
	}

	// -----------------------------------------------------------------------
	// Alpha separation tests
	// -----------------------------------------------------------------------
	TEST_CASE("[gd_pack] separate_alpha: opaque and alpha images go to different pages") {
		_doctest_prepare_folder();

		// Opaque image (RGB, no alpha).
		Ref<Image> opaque = memnew(Image);
		opaque->create(16, 16, false, Image::FORMAT_RGB8);
		opaque->fill(Color(1, 0, 0));

		// Alpha image (RGBA with blend).
		Ref<Image> alpha_img = memnew(Image);
		alpha_img->create(16, 16, false, Image::FORMAT_RGBA8);
		alpha_img->fill(Color(0, 1, 0, 0)); // fully transparent green

		Vector<Ref<Image>> imgs;
		imgs.push_back(opaque, alpha_img);

		ImageMergeOptions opts;
		opts.separate_alpha = true;
		opts.margin = 0;

		Dictionary res;
		SUPPRESS_OUTPUT(res = merge_images(imgs, opts));
		REQUIRE(!res.empty());

		Array rects = res["_rects"];
		REQUIRE(rects.size() == 2);

		int page0 = Dictionary(rects[0])["atlas_page"];
		int page1 = Dictionary(rects[1])["atlas_page"];
		// The two images must be on different atlas pages.
		CHECK(page0 != page1);

		Array pages = res["_generated_images"];
		CHECK(pages.size() == 2);

		Ref<Image>(pages[0])->save_png(_doctest_get_folder() + "sep_alpha_opaque.png");
		Ref<Image>(pages[1])->save_png(_doctest_get_folder() + "sep_alpha_alpha.png");
	}

	TEST_CASE("[gd_pack] separate_alpha=false packs all images on same page") {
		Ref<Image> img1 = _make_solid_rgba(8, 8, Color(1, 0, 0, 1));
		Ref<Image> img2 = _make_solid_rgba(8, 8, Color(0, 1, 0, 0));
		Vector<Ref<Image>> imgs;
		imgs.push_back(img1, img2);

		ImageMergeOptions opts;
		opts.separate_alpha = false;
		opts.margin = 0;
		opts.max_atlas_size = 512;

		Dictionary res;
		SUPPRESS_OUTPUT(res = merge_images(imgs, opts));
		REQUIRE(!res.empty());
		Array rects = res["_rects"];
		REQUIRE(rects.size() == 2);
		// Both on page 0.
		CHECK(int(Dictionary(rects[0])["atlas_page"]) == int(Dictionary(rects[1])["atlas_page"]));
	}

	TEST_CASE("[gd_pack] separate_alpha: only opaque images produces single page") {
		Ref<Image> img1 = memnew(Image);
		img1->create(8, 8, false, Image::FORMAT_RGB8);
		img1->fill(Color(1, 0, 0));
		Ref<Image> img2 = memnew(Image);
		img2->create(8, 8, false, Image::FORMAT_RGB8);
		img2->fill(Color(0, 1, 0));

		Vector<Ref<Image>> imgs;
		imgs.push_back(img1, img2);

		ImageMergeOptions opts;
		opts.separate_alpha = true;
		opts.margin = 0;

		Dictionary res;
		SUPPRESS_OUTPUT(res = merge_images(imgs, opts));
		REQUIRE(!res.empty());
		// All opaque, so should still pack together.
		Array pages = res["_generated_images"];
		CHECK(pages.size() >= 1);
		Array rects = res["_rects"];
		REQUIRE(rects.size() == 2);
	}
} // TEST_SUITE
#endif
