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

#include <algorithm>
#include <cstring>
#include <vector>

#include "core/image.h"

/* of your interest:

1. rect_xywhf - structure representing your rectangle object
	members:
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

For description how to tune the algorithm and how it actually works see the .cpp file.


*/

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

/*

For every sorting function, algorithm will perform packing attempts beginning with a bin with width and height equal to max_side,
and decreasing its dimensions if it finds out that rectangles did actually fit, increasing otherwise.
Although, it's doing that in sort of binary search manner, so for every comparing function it will perform at most log2(max_side) packing attempts looking for the smallest possible bin size.
discard_step = 128 means that the algorithm will break of the searching loop if the rectangles fit but "it may be possible to fit them in a bin smaller by 128"
the bigger the value, the sooner the algorithm will finish but the rectangles will be packed less tightly.
use discard_step = 1 for maximum tightness.

the algorithm was based on http://www.blackpawn.com/texts/lightmaps/default.html
the algorithm reuses the node tree so it doesn't reallocate them between searching attempts

*/

/*************************************************************************** CHAOS BEGINS HERE */

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
		rc = rect_ltrb(0, 0, r.w, r.h);
		delcheck();
	}

	node *insert(rect_xywhf &img, bool allowFlip) {
		if (c[0].pn && c[0].fill) {
			if (auto newn = c[0].pn->insert(img, allowFlip))
				return newn;
			return c[1].pn->insert(img, allowFlip);
		}

		if (id)
			return 0;
		int f = img.fits(rect_xywh(rc), allowFlip);

		switch (f) {
			case 0:
				return 0;
			case 1:
				img.flipped = false;
				break;
			case 2:
				img.flipped = true;
				break;
			case 3:
				id = true;
				img.flipped = false;
				return this;
			case 4:
				id = true;
				img.flipped = true;
				return this;
		}

		int iw = (img.flipped ? img.h : img.w), ih = (img.flipped ? img.w : img.h);

		if (rc.w() - iw > rc.h() - ih) {
			c[0].set(rc.l, rc.t, rc.l + iw, rc.b);
			c[1].set(rc.l + iw, rc.t, rc.r, rc.b);
		} else {
			c[0].set(rc.l, rc.t, rc.r, rc.t + ih);
			c[1].set(rc.l, rc.t + ih, rc.r, rc.b);
		}

		return c[0].pn->insert(img, allowFlip);
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
		if (c[0].pn)
			delete c[0].pn;
		if (c[1].pn)
			delete c[1].pn;
	}
};

static rect_wh _rect2D(rect_xywhf *const *v, int n, int max_s, bool allowFlip, std::vector<rect_xywhf *> &succ, std::vector<rect_xywhf *> &unsucc) {
	node root;

	const int funcs = (sizeof(cmpf) / sizeof(bool (*)(rect_xywhf *, rect_xywhf *)));

	rect_xywhf **order[funcs];

	for (int f = 0; f < funcs; ++f) {
		order[f] = new rect_xywhf *[n];
		std::memcpy(order[f], v, sizeof(rect_xywhf *) * n);
		std::sort(order[f], order[f] + n, cmpf[f]);
	}

	rect_wh min_bin = rect_wh(max_s, max_s);
	int min_func = -1, best_func = 0, best_area = 0, _area = 0, step, fit, i;

	bool fail = false;

	for (int f = 0; f < funcs; ++f) {
		v = order[f];
		step = min_bin.w / 2;
		root.reset(min_bin);

		while (true) {
			if (root.rc.w() > min_bin.w) {
				if (min_func > -1)
					break;
				_area = 0;

				root.reset(min_bin);
				for (i = 0; i < n; ++i)
					if (root.insert(*v[i], allowFlip))
						_area += v[i]->area();

				fail = true;
				break;
			}

			fit = -1;

			for (i = 0; i < n; ++i)
				if (!root.insert(*v[i], allowFlip)) {
					fit = 1;
					break;
				}

			if (fit == -1 && step <= discard_step)
				break;

			root.reset(rect_wh(root.rc.w() + fit * step, root.rc.h() + fit * step));

			step /= 2;
			if (!step)
				step = 1;
		}

		if (!fail && (min_bin.area() >= root.rc.area())) {
			min_bin = rect_wh(root.rc);
			min_func = f;
		}

		else if (fail && (_area > best_area)) {
			best_area = _area;
			best_func = f;
		}
		fail = false;
	}

	v = order[min_func == -1 ? best_func : min_func];

	int clip_x = 0, clip_y = 0;

	root.reset(min_bin);

	for (i = 0; i < n; ++i) {
		if (auto ret = root.insert(*v[i], allowFlip)) {
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

	for (int f = 0; f < funcs; ++f)
		delete[] order[f];

	return rect_wh(clip_x, clip_y);
}

static bool _pack_rects(rect_xywhf *const *v, int n, int max_s, bool allowFlip, std::vector<bin> &bins) {
	rect_wh _rect(max_s, max_s);

	for (int i = 0; i < n; ++i)
		if (!v[i]->fits(_rect, allowFlip))
			return false;

	std::vector<rect_xywhf *> vec[2], *p[2] = { vec, vec + 1 };
	vec[0].resize(n);
	vec[1].clear();
	std::memcpy(&vec[0][0], v, sizeof(rect_xywhf *) * n);

	bin *b = 0;

	while (true) {
		bins.push_back(bin());
		b = &bins[bins.size() - 1];

		b->size = _rect2D(&((*p[0])[0]), static_cast<int>(p[0]->size()), max_s, allowFlip, b->rects, *p[1]);
		p[0]->clear();

		if (!p[1]->size())
			break;

		std::swap(p[0], p[1]);
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
		case Image::FORMAT_L8:
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
	for (int k = 0; k < by; k++)
		form->blit_rect(topb, get_rect(topb), Point2(x_border, y_border - k - 1)); // top
	for (int k = 0; k < by; k++)
		form->blit_rect(botb, get_rect(botb), Point2(x_border, rc.height - y_border + k)); // bottom
	for (int k = 0; k < bx; k++)
		form->blit_rect(leftb, get_rect(leftb), Point2(x_border - k - 1, y_border)); // left
	for (int k = 0; k < bx; k++)
		form->blit_rect(rightb, get_rect(rightb), Point2(rc.width - x_border + k, y_border)); // right

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

struct TextureMergeOptions {
	int max_atlas_size = 512;
	Color background_color = Color(0, 0, 0, 0);
	int margin = 2;
	bool power_of_two = false;

	TextureMergeOptions() {}
	TextureMergeOptions(int max_atlas_size) :
			max_atlas_size(max_atlas_size) {}
};

static Dictionary merge_images(Vector<Ref<Image>> images, Vector<String> names, const TextureMergeOptions &options = TextureMergeOptions()) {
	ERR_FAIL_COND_V(images.size() != names.size(), Dictionary());

	const int margin = options.margin;
	const Color background_color = options.background_color;

	Array generated_images;
	std::vector<bin> bins;

	// NOTICE: atlas texture can be 3 or 4 channels only
	int atlas_channels = 3;

	const int n = images.size();
	Vector<rect_xywhf> data;
	data.resize(n);
	Vector<rect_xywhf *> rects;
	rects.resize(n);
	for (int i = 0; i < images.size(); ++i) {
		Ref<Image> image = images[i];
		if (margin > 0) {
			image = _mirror_borders(image, margin, margin);
		}
		data.write[i].original_image = image;
		data.write[i].x = 0;
		data.write[i].y = 0;
		data.write[i].w = image->get_size().x;
		data.write[i].h = image->get_size().y;
		rects.write[i] = &data.write[i];
		if (image->get_format() == Image::FORMAT_RGBA8 || image->get_format() == Image::FORMAT_LA8) {
			// only if we have a real alpha values in the channel
			if (image->detect_alpha() == Image::ALPHA_BLEND) {
				atlas_channels = 4;
			}
		}
	}

	ERR_FAIL_COND_V(atlas_channels < 3, Dictionary());

	Dictionary ret;
	if (_pack_rects(rects.ptr(), rects.size(), options.max_atlas_size, false, bins)) {
		generated_images.clear();
		generated_images.resize(bins.size());

		for (uint32_t i = 0; i < bins.size(); ++i) {
			bin b = bins[i];

			PoolByteArray atlas_data;
			atlas_data.resize(b.size.w * b.size.h * atlas_channels);

			// Setup background color
			const uint8_t cr = background_color.r * 255.0;
			const uint8_t cg = background_color.g * 255.0;
			const uint8_t cb = background_color.b * 255.0;
			const uint8_t ca = background_color.a * 255.0;

			for (int j = 0; j < atlas_data.size(); j += atlas_channels) {
				atlas_data.set(j, cr);
				atlas_data.set(j + 1, cg);
				atlas_data.set(j + 2, cb);
				if (atlas_channels == 4)
					atlas_data.set(j + 3, ca);
			}

			Ref<Image> atlas;
			atlas.instance();

			// Process rects
			for (uint32_t j = 0; j < b.rects.size(); ++j) {
				rect_xywhf *r = b.rects[j];

				r->bin = i;
				r->atlas_image = atlas;

				int rect_pos_x = 0;
				int rect_pos_y = 0;

				Ref<Image> img = r->original_image;

				ERR_CONTINUE(!img.is_valid());

				int img_width = img->get_width();
				PoolByteArray image_data = img->get_data();

				int input_format_offset = _get_offset_for_format(img->get_format());

				ERR_CONTINUE_MSG(input_format_offset == 0, "Image format is not supported, Skipping!");

				for (int y = 0; y < r->h; ++y) {
					const int orig_img_indx = (rect_pos_y + y) * img_width * input_format_offset + rect_pos_x * input_format_offset;
					const int start_indx = (r->y + y) * b.size.w * atlas_channels + r->x * atlas_channels;

					for (int x = 0; x < r->w; ++x) {
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
									if (sx == 3 && atlas_channels == 4)
										atlas_data.set(start_indx + (x * atlas_channels) + sx, image_data[orig_img_indx + 1 + (x * input_format_offset)]);
									else
										atlas_data.set(start_indx + (x * atlas_channels) + sx, image_data[orig_img_indx + 0 + (x * input_format_offset)]);
								}
							} break;
						}
					}
				}
			}

			atlas->create(b.size.w, b.size.h, false, atlas_channels == 4 ? Image::FORMAT_RGBA8 : Image::FORMAT_RGB8, atlas_data);

			// dump generated atlas:
			// atlas->save_png(vformat("atlas_%d.png", i));

			generated_images.set(i, atlas);
		}

		Dictionary atlas_rects;
		for (int r = 0; r < data.size(); ++r) {
			const rect_xywhf &rc = data[r];
			Dictionary entry;
			entry["rect"] = Rect2(rc.x + margin, rc.y + margin, rc.w - 2 * margin, rc.h - 2 * margin);
			entry["atlas_page"] = rc.bin;
			entry["atlas"] = rc.atlas_image;
			atlas_rects[names[r]] = entry;
		}

		ret["_rects"] = atlas_rects;
		ret["_generated_images"] = generated_images;
	}

	return ret;
}
