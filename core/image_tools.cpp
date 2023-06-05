#include "core/image_tools.h"
#include "bind/core_bind.h"
#include "core/image.h"

/**
 * Moore Neighbor Tracing
 * An explanation of the algorithm can be found here:
 * http://www.imageprocessingplace.com/downloads_V3/root_downloads/tutorials/contour_tracing_Abeer_George_Ghuneim/moore.html
 */
Ref<Image> ImageTools::neighbor_tracing(const Image *p_src) {
	const int WHITE = 255;
	const int BLACK = 0;

	ERR_FAIL_NULL_V(p_src, Ref<Image>());

	const int width = p_src->width;
	const int height = p_src->height;

	Ref<Image> src_padded = p_src->expanded(width + 2, height + 2, Color(1, 1, 1)); // need to start by padding the image by 1 pixel
	src_padded->convert(Image::FORMAT_L8);
	const PoolVector<uint8_t> padded_image = src_padded->get_data();

	PoolVector<uint8_t> border_image_data; // allocate new image
	ERR_FAIL_COND_V(border_image_data.resize((height + 2) * (width + 2)) != OK, Ref<Image>());
	border_image_data.fill(WHITE); // set entire image to WHITE

	PoolVector<uint8_t>::Write border_image = border_image_data.write();

	bool inside = false;
	for (int y = 0; y < height + 2; y++) {
		for (int x = 0; x < width + 2; x++) {
			int pos = x + y * (width + 2);

			// scan for BLACK pixel
			if (border_image[pos] == BLACK && !inside) { // entering an already discovered border
				inside = true;
			} else if (padded_image[pos] == BLACK && inside) { // already discovered border point
				continue;
			} else if (padded_image[pos] == WHITE && inside) { // leaving a border
				inside = false;
			} else if (padded_image[pos] == BLACK && !inside) { // undiscovered border point
				border_image[pos] = BLACK; // mark the start pixel
				int check_location_nr = 1; // the neighbor number of the location we want to check for a new border point
				int check_position; // the corresponding absolute array address of check_location_nr
				int new_check_location_nr; // variable that holds the neighborhood position we want to check if we find a new border at check_location_nr
				int start_pos = pos; // set start position
				int counter = 0; // counter is used for the jacobi stop criterion
				int counter2 = 0; // counter2 is used to determine if the point we have discovered is one single point

				// defines the neighborhood offset position from current position and the neighborhood
				// position we want to check next if we find a new border at check_location_nr
				int neighborhood[8][2] = {
					{ -1, 7 },
					{ -3 - width, 7 },
					{ -width - 2, 1 },
					{ -1 - width, 1 },
					{ 1, 3 },
					{ 3 + width, 3 },
					{ width + 2, 5 },
					{ 1 + width, 5 },
				};
				// trace around the neighborhood
				while (true) {
					check_position = pos + neighborhood[check_location_nr - 1][0];
					new_check_location_nr = neighborhood[check_location_nr - 1][1];

					if (padded_image[check_position] == BLACK) { // next border point found
						if (check_position == start_pos) {
							counter++;

							// stopping criterion (jacob)
							if (new_check_location_nr == 1 || counter >= 3) {
								inside = true; // close loop - since we are starting the search at were we first started we must set inside to true
								break;
							}
						}

						check_location_nr = new_check_location_nr; // update which neighborhood position we should check next
						pos = check_position;
						counter2 = 0; // reset the counter that keeps track of how many neighbors we have visited
						border_image[check_position] = BLACK; // set the border pixel
					} else {
						// rotate clockwise in the neighborhood
						check_location_nr = 1 + (check_location_nr % 8);
						if (counter2 > 8) {
							// If counter2 is above 8 we have traced around the neighborhood and
							// therefor the border is a single black pixel and we can exit
							counter2 = 0;
							break;
						} else {
							counter2++;
						}
					}
				}
			}
		}
	}

	border_image.release();

	Ref<Image> dst = memnew(Image(width + 2, height + 2, false, Image::FORMAT_L8, border_image_data));
	dst->crop_from_point(1, 1, width, height);
	return dst;
}

void ImageTools::fix_alpha_edges(Image *p_src) {
	ERR_FAIL_NULL(p_src);
	ERR_FAIL_COND(!p_src->_can_modify(p_src->format));
	ERR_FAIL_COND_MSG(p_src->write_lock.ptr(), "Cannot modify image when it is locked.");

	if (p_src->data.size() == 0) {
		return;
	}

	if (p_src->format != Image::FORMAT_RGBA8) {
		return; // not needed
	}

	PoolVector<uint8_t> dcopy = p_src->data;
	PoolVector<uint8_t>::Read rp = dcopy.read();
	const uint8_t *srcptr = rp.ptr();

	PoolVector<uint8_t>::Write wp = p_src->data.write();
	uint8_t *data_ptr = wp.ptr();

	const int max_radius = 4;
	const int alpha_threshold = 20;
	const int max_dist = 0x7FFFFFFF;

	const int width = p_src->width;
	const int height = p_src->height;

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			const uint8_t *rptr = &srcptr[(i * width + j) * 4];
			uint8_t *wptr = &data_ptr[(i * width + j) * 4];

			if (rptr[3] >= alpha_threshold) {
				continue;
			}

			int closest_dist = max_dist;
			uint8_t closest_color[3];

			int from_x = MAX(0, j - max_radius);
			int to_x = MIN(width - 1, j + max_radius);
			int from_y = MAX(0, i - max_radius);
			int to_y = MIN(height - 1, i + max_radius);

			for (int k = from_y; k <= to_y; k++) {
				for (int l = from_x; l <= to_x; l++) {
					int dy = i - k;
					int dx = j - l;
					int dist = dy * dy + dx * dx;
					if (dist >= closest_dist) {
						continue;
					}

					const uint8_t *rp2 = &srcptr[(k * width + l) << 2];

					if (rp2[3] < alpha_threshold) {
						continue;
					}

					closest_dist = dist;
					closest_color[0] = rp2[0];
					closest_color[1] = rp2[1];
					closest_color[2] = rp2[2];
				}
			}

			if (closest_dist != max_dist) {
				wptr[0] = closest_color[0];
				wptr[1] = closest_color[1];
				wptr[2] = closest_color[2];
			}
		}
	}
}

void ImageTools::fix_tex_bleed(Image *p_src) {
	ERR_FAIL_NULL(p_src);
	ERR_FAIL_COND(!p_src->_can_modify(p_src->format));
	ERR_FAIL_COND_MSG(p_src->write_lock.ptr(), "Cannot modify image when it is locked.");

	if (p_src->data.size() == 0) {
		return;
	}

	if (p_src->format != Image::FORMAT_RGBA8) {
		return; // not needed
	}

	struct point_t {
		short dx, dy;
	};

	// Given an RGBA texture, finds all pixels where alpha==0 and fills in a suitable RGB color for it.
	static auto _bleedcompare = [](point_t *p, int gstride, int offsetx, int offsety) {
		point_t other = p[offsety * gstride + offsetx];
		other.dx += offsetx;
		other.dy += offsety;

		const int odist = other.dx * other.dx + other.dy * other.dy;
		const int pdist = p->dx * p->dx + p->dy * p->dy;
		if (odist < pdist) {
			*p = other;
		}
	};

	const int BLEED_THRESHOLD = 20; // we search for pixels with alpha greater than this to bleed outwards from.
	const int MAX_DIST = 0x7FFF;

	const int width = p_src->width;
	const int height = p_src->height;

	const int pixstride = p_src->get_format_pixel_size(p_src->format);
	const int rowstride = width * pixstride;
	const int gstride = width + 2;
	const int cellcount = gstride * (height + 2);
	point_t *storage = (point_t *)memalloc(cellcount * sizeof(point_t));
	ERR_FAIL_NULL(storage);
	point_t *grid = storage + gstride + 1;

	// Initialize to empty.
	for (int n = 0; n < cellcount; n++) {
		storage[n].dx = storage[n].dy = MAX_DIST;
	}

	uint8_t *pixels = p_src->data.write().ptr();
	const int ac = 3;

	// Fill in the solid pixels.
	bool any = false;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			const uint8_t *pix = pixels + y * rowstride + x * pixstride;
			if (pix[ac] > BLEED_THRESHOLD) {
				point_t *p = &grid[y * gstride + x];
				p->dx = p->dy = 0;
				any = true;
			}
		}
	}
	if (any) {
		// Distance field sweep - Pass 0
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				point_t *p = &grid[y * gstride + x];
				_bleedcompare(p, gstride, -1, 0);
				_bleedcompare(p, gstride, 0, -1);
				_bleedcompare(p, gstride, -1, -1);
				_bleedcompare(p, gstride, 1, -1);
			}
			for (int x = width - 1; x >= 0; x--) {
				point_t *p = &grid[y * gstride + x];
				_bleedcompare(p, gstride, 1, 0);
			}
		}

		// Distance field sweep - Pass 1
		for (int y = height - 1; y >= 0; y--) {
			for (int x = width - 1; x >= 0; x--) {
				point_t *p = &grid[y * gstride + x];
				_bleedcompare(p, gstride, 1, 0);
				_bleedcompare(p, gstride, 0, 1);
				_bleedcompare(p, gstride, -1, 1);
				_bleedcompare(p, gstride, 1, 1);
			}
			for (int x = 0; x < width; x++) {
				point_t *p = &grid[y * gstride + x];
				_bleedcompare(p, gstride, -1, 0);
			}
		}

		// Read back the nearest pixels.
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				point_t *p = &grid[y * gstride + x];
				const int sx = x + p->dx;
				const int sy = y + p->dy;
				// Copy the RGB over.
				const uint8_t *src = pixels + sy * rowstride + sx * pixstride;
				uint8_t *dst = pixels + y * rowstride + x * pixstride;
				if (dst[ac] == 0) {
					for (int n = 0; n < pixstride; n++) {
						dst[n] = src[n];
					}
					dst[ac] = 0;
				}
			}
		}
	}
	memfree(storage);
}

void ImageTools::normalmap_to_xy(Image *p_src) {
	ERR_FAIL_NULL(p_src);

	p_src->convert(Image::FORMAT_RGBA8);

	{
		const int len = p_src->data.size() / 4;
		PoolVector<uint8_t>::Write wp = p_src->data.write();
		unsigned char *data_ptr = wp.ptr();

		for (int i = 0; i < len; i++) {
			data_ptr[(i << 2) + 3] = data_ptr[(i << 2) + 0]; //x to w
			data_ptr[(i << 2) + 0] = data_ptr[(i << 2) + 1]; //y to xz
			data_ptr[(i << 2) + 2] = data_ptr[(i << 2) + 1];
		}
	}

	p_src->convert(Image::FORMAT_LA8);
}

void ImageTools::bumpmap_to_normalmap(Image *p_src, float bump_scale) {
	ERR_FAIL_NULL(p_src);
	ERR_FAIL_COND(!p_src->_can_modify(p_src->format));
	ERR_FAIL_COND_MSG(p_src->write_lock.ptr(), "Cannot modify image when it is locked.");
	p_src->clear_mipmaps();
	p_src->convert(Image::FORMAT_RF);

	const int width = p_src->width;
	const int height = p_src->height;

	PoolVector<uint8_t> result_image; // rgba output
	result_image.resize(width * height * 4);

	{
		PoolVector<uint8_t>::Read rp = p_src->data.read();
		PoolVector<uint8_t>::Write wp = result_image.write();

		ERR_FAIL_COND(!rp.ptr());

		unsigned char *write_ptr = wp.ptr();
		float *read_ptr = (float *)rp.ptr();

		for (int ty = 0; ty < height; ty++) {
			int py = ty + 1;
			if (py >= height) {
				py -= height;
			}

			for (int tx = 0; tx < width; tx++) {
				int px = tx + 1;
				if (px >= width) {
					px -= width;
				}
				float here = read_ptr[ty * width + tx];
				float to_right = read_ptr[ty * width + px];
				float above = read_ptr[py * width + tx];
				Vector3 up = Vector3(0, 1, (here - above) * bump_scale);
				Vector3 across = Vector3(1, 0, (to_right - here) * bump_scale);

				Vector3 normal = across.cross(up);
				normal.normalize();

				write_ptr[((ty * width + tx) << 2) + 0] = (127.5 + normal.x * 127.5);
				write_ptr[((ty * width + tx) << 2) + 1] = (127.5 + normal.y * 127.5);
				write_ptr[((ty * width + tx) << 2) + 2] = (127.5 + normal.z * 127.5);
				write_ptr[((ty * width + tx) << 2) + 3] = 255;
			}
		}
	}
	p_src->format = Image::FORMAT_RGBA8;
	p_src->data = result_image;
}

// Basic helper classes for unpack and seamless generator

union _byteword {
	uint8_t b[sizeof(uint32_t)];
	uint32_t w;
	_byteword() {}
	_byteword(uint32_t v) :
			w(v) {}
};

#define ERR_FAIL_RANGE_V(Val, Vmin, Vmax, Ret) ERR_FAIL_COND_V(Val <= Vmin || Val > Vmax, Ret)
#define MATH_LERP(From, To, Val) Math::lerp(real_t(From), real_t(To), real_t(Val))
#define MATH_INVLERP(From, To, Val) Math::inverse_lerp(real_t(From), real_t(To), real_t(Val))
#define MATH_SQRT(Val) Math::sqrt(real_t(Val))

/**
 * Unpack sprites regions on image
 * https://github.com/ForkandBeard/Alferd-Spritesheet-Unpacker
 *
 */

// Geometry helpers

struct _re_options {
	uint32_t background;
	uint8_t alpha_threshold;
	int distance_between_tiles;
};

static real_t _re_get_x_gap_between_rectangles(const Rect2 &rect_a, const Rect2 &rect_b) {
	real_t sng_return = 0;
	if (rect_a.right() >= rect_b.left()) {
		sng_return = rect_a.left() - rect_b.right();
	} else {
		sng_return = rect_b.left() - rect_a.right();
	}
	return MAX(0, sng_return);
}

static real_t _re_get_y_gap_between_rectangles(const Rect2 &rect_a, const Rect2 &rect_b) {
	real_t sng_return = 0;
	if (rect_a.bottom() >= rect_b.top()) {
		sng_return = rect_a.top() - rect_b.bottom();
	} else {
		sng_return = rect_b.top() - rect_a.bottom();
	}
	return MAX(0, sng_return);
}

static bool _re_is_background(const Image *image, int x, int y, const _re_options *opts) {
	const uint8_t *data_ptr = image->get_data().read().ptr();
	const uint32_t ofs = y * image->get_width() + x;
	_byteword color_at_xy;
	switch (image->get_format()) {
		case Image::FORMAT_RGBA8: {
			color_at_xy.w = ((uint32_t *)data_ptr)[ofs];
		} break;
		case Image::FORMAT_RGB8: {
			color_at_xy.b[0] = data_ptr[ofs + 0];
			color_at_xy.b[1] = data_ptr[ofs + 1];
			color_at_xy.b[2] = data_ptr[ofs + 2];
			color_at_xy.b[3] = 0xff;
		} break;
		default: {
			color_at_xy = image->get_pixel(x, y).to_rgba32(); // should not be called - we donot support other formats
		}
	}
	if (color_at_xy.b[3] < opts->alpha_threshold) {
		return true; // background
	} else {
		return (color_at_xy.w == opts->background);
	}
}

static bool _re_do_boxes_contain_adjacent_or_overlapping_pixels(const Image *image, const Rect2 &box1, const Rect2 &box2, const _re_options *opts) {
	if (box1.intersects(box2)) {
		const Rect2 intersection = box1.clip(box2);
		for (int x = intersection.position.x; x <= intersection.right(); x++) {
			for (int y = intersection.position.y; y <= intersection.bottom(); y++) {
				if (!_re_is_background(image, x, y, opts)) {
					return true;
				}
			}
		}
	}
	if (_re_get_x_gap_between_rectangles(box1, box2) <= opts->distance_between_tiles) {
		for (int y = box1.position.y - opts->distance_between_tiles; y <= box1.bottom() + opts->distance_between_tiles; y++) {
			if (y >= box2.top() && y <= box2.bottom()) {
				if (box2.left() > box1.right()) {
					if (!_re_is_background(image, box1.right(), y, opts))
						if (!_re_is_background(image, box2.left(), y, opts)) {
							return true;
						}
				} else {
					if (!_re_is_background(image, box1.left(), y, opts)) {
						if (!_re_is_background(image, box2.right(), y, opts)) {
							return true;
						}
					}
				}
			}
		}
	}
	if (_re_get_y_gap_between_rectangles(box1, box2) <= opts->distance_between_tiles) {
		for (int x = box1.left() - opts->distance_between_tiles; x <= box1.right() + opts->distance_between_tiles; x++) {
			if (x >= box2.left() && x <= box2.right()) {
				if (box2.top() > box1.bottom()) {
					if (!_re_is_background(image, x, box1.bottom(), opts)) {
						if (!_re_is_background(image, x, box2.top(), opts)) {
							return true;
						}
					}
				} else {
					if (!_re_is_background(image, x, box1.top(), opts)) {
						if (!_re_is_background(image, x, box2.bottom(), opts)) {
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

static List<Rect2> _re_create_boxes(const Image *image, const Rect2 &region, const _re_options *opts) {
	List<Rect2> boxes;
	int x2 = 0, y2 = 0;
	for (int y = region.top(); y <= region.bottom(); y++) {
		for (int x = region.left(); x <= region.right(); x++) {
			if (x > 0 && x < image->get_width() && y > 0 && y < image->get_height()) {
				if (!_re_is_background(image, x, y, opts)) {
					Rect2 new_box = Rect2(x, y, 0, 0);
					x2 = x;
					while (x2 < (image->get_width() - 1) && !_re_is_background(image, x2, y, opts)) {
						x2 += 1;
						new_box = Rect2(new_box.position.x, new_box.position.y, new_box.size.width + 1, new_box.size.height);
					}
					y2 = y;
					while (y2 < (image->get_height() - 1) && !_re_is_background(image, x2, y2, opts)) {
						y2 += 1;
						new_box = Rect2(new_box.position.x, new_box.position.y, new_box.size.width, new_box.size.height + 1);
					}
					y2 = y + new_box.size.height;
					while (y2 < (image->get_height() - 1) && !_re_is_background(image, x, y2, opts)) {
						y2 += 1;
						new_box = Rect2(new_box.position.x, new_box.position.y, new_box.size.width, new_box.size.height + 1);
					}
					boxes.push_back(new_box);
					x += new_box.size.width + 1;
				}
			}
		}
	}
	return boxes;
}

static int _re_combine_first_overlapping_box(const Image *image, List<Rect2> &boxes, int start_index, const _re_options *opts) {
	List<Rect2> old_boxes;
	Rect2 new_box;
	int return_index = -1;

	for (int i = start_index; i < boxes.size(); i++) {
		Rect2 box = boxes[i];
		for (const Rect2 &collider : boxes) {
			if (box != collider) {
				if (_re_do_boxes_contain_adjacent_or_overlapping_pixels(image, box, collider, opts)) {
					new_box = box;
					return_index = i;
					if (collider.right() > new_box.right()) {
						new_box.size.width = collider.right() - new_box.left();
					}
					if (collider.left() < new_box.left()) {
						new_box.size.width = new_box.size.width + new_box.left() - collider.left();
					}
					if (collider.bottom() > new_box.bottom()) {
						new_box.size.height = collider.bottom() - new_box.top();
					}
					if (collider.top() < new_box.top()) {
						new_box.size.height = new_box.size.height + new_box.top() - collider.top();
					}
					new_box.position.x = MIN(new_box.position.x, collider.position.x);
					new_box.position.y = MIN(collider.position.y, new_box.position.y);

					old_boxes.push_back(box);
					old_boxes.push_back(collider);

					break; // TODO: might not be correct. Was: Exit For
				}
			}
		}
		if (!new_box.has_no_area()) {
			break; // TODO: might not be correct. Was: Exit For
		}
	}
	if (!new_box.has_no_area()) {
		for (const Rect2 &old_box : old_boxes) {
			boxes.erase(old_box);
		}
		boxes.push_back(new_box);
	}
	return return_index;
}

static List<Rect2> _re_combine_boxes(const Image *image, List<Rect2> boxes, const _re_options *opts) {
	int index = 0;
	do {
		index = _re_combine_first_overlapping_box(image, boxes, index, opts);
		// There is a bug here where -1 is returned even when boxes still need to be
		// combined so just a hack to try again even if index is -1. Keep trying.
		if (index == -1) {
			index = _re_combine_first_overlapping_box(image, boxes, index, opts);
		}
	} while (index != -1);
	return boxes;
}

Vector<Rect2> ImageTools::unpack_region(const Image *p_src, real_t p_distance_between_tiles_perc, real_t p_minimum_tile_area_to_save_perc, real_t p_alpha_threshold, Ref<Image> p_debug_image) {
	ERR_FAIL_NULL_V(p_src, Vector<Rect2>());

	if (p_src->data.size() == 0) {
		return Vector<Rect2>();
	}

	if (p_src->format != Image::FORMAT_RGBA8 && p_src->format != Image::FORMAT_RGB8) {
		WARN_PRINT("Unsupported image format");
		return Vector<Rect2>();
	}

	ERR_FAIL_RANGE_V(p_distance_between_tiles_perc, 0, 100, Vector<Rect2>());
	ERR_FAIL_RANGE_V(p_minimum_tile_area_to_save_perc, 0, 100, Vector<Rect2>());
	ERR_FAIL_RANGE_V(p_alpha_threshold, 0, 1, Vector<Rect2>());

	const int width = p_src->width;
	const int height = p_src->height;

	const int minimum_tile_area_to_save = p_minimum_tile_area_to_save_perc / 100 * p_minimum_tile_area_to_save_perc / 100 * width * height; // % of area
	const int distance_between_tiles = p_distance_between_tiles_perc / 100 * (width + height) / 2; // % of avg. size

	print_verbose("Unpacking image:");
	print_verbose(vformat("  distance between tiles: %d", distance_between_tiles));
	print_verbose(vformat("  minimum save area: %d", minimum_tile_area_to_save));

	const _re_options opts{ p_src->get_pixel(0, 0).to_rgba32(), uint8_t(p_alpha_threshold * 255), distance_between_tiles };

	List<Rect2> boxes = _re_combine_boxes(p_src, _re_create_boxes(p_src, Rect2(Point2(), p_src->get_size()), &opts), &opts);
	print_verbose("Unpacking " + itos(boxes.size()) + " boxes after combining");

	Vector<Rect2> ret;
	for (const List<Rect2>::Element *E = boxes.front(); E; E = E->next()) {
		ret.push_back(E->get());
	}

	if (p_debug_image.is_valid()) {
		ERR_FAIL_COND_V(p_debug_image->get_size() != p_src->get_size(), ret);
		const Color red(1, 0, 0);
		for (const Rect2 &rc : boxes) {
			p_debug_image->plot_rect(rc, red);
		}
	}

	return ret;
}

/**
 * Create seamless texture so you can tile them and it will be much less noticeable.
 * You can customize settings to create best fitting set for your texture.
 * Algorithm is supporting making seamless textures also for normal maps.
 */

static void _fe_rotate_square(Vector<_byteword> &tex, int width, int height, real_t phi) {
	const real_t sn = Math::sin(phi);
	const real_t cs = Math::cos(phi);
	LocalVector<_byteword> source_copy(tex.size(), tex.ptr());

	const int xc = width / 2;
	const int yc = height / 2;

	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			const int x = cs * (i - xc) + sn * (j - yc) + xc;
			const int y = -sn * (i - xc) + cs * (j - yc) + yc;
			if ((x > -1) && (x < width) && (y > -1) && (y < height)) {
				tex.write[j * width + i] = source_copy[y * width + x];
			} else {
				tex.write[j * width + i] = 0;
			}
		}
	}
}

static void _fe_rotate_image(Vector<_byteword> &tex, int width, int height, int angle) {
	const int x = 0, y = 0;
	LocalVector<_byteword> source_copy(tex.size());
	_fe_rotate_square(tex, width, height, (Math_PI / 180 * angle));
	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			source_copy[width / 2 - width / 2 + x + i + width * (height / 2 - height / 2 + j + y)] = tex[i + j * width];
		}
	}
	memcpy(tex.ptrw(), source_copy.ptr(), tex.size() * sizeof(_byteword));
}

// Getting dimensions of image by it's pixels count
static _FORCE_INLINE_ Vector2i _fe_get_square_dims(int array_length) {
	const int dim = Math::sqrt((real_t)array_length);
	return Vector2i(dim, dim);
}

// Texture small operations

// Getting index for position on texture
static int _fe_get_px(int x, int y, const Vector2i &dimensions) {
	if (y < 0) {
		y = 0;
	} else if (y >= dimensions.y) {
		y = dimensions.y - 1;
	}
	if (x < 0) {
		x = 0;
	} else if (x >= dimensions.x) {
		x = dimensions.x - 1;
	}
	return MIN(dimensions.x * dimensions.y - 1, y * dimensions.x + x);
}

// Getting index for position on texture looping if greater than dimensions
static int _fe_get_px_loop(int x, int y, const Vector2i &dimensions) {
	if (x < 0) {
		x += dimensions.x;
		x %= dimensions.x;
		if (x < 0) {
			x = -x;
		}
	} else if (x >= dimensions.x) {
		x -= dimensions.x;
		x %= dimensions.x;
	}
	if (y < 0) {
		y += dimensions.y;
		y %= dimensions.y;
		if (y < 0) {
			y = -y;
		}
	} else if (y >= dimensions.y) {
		y -= dimensions.y;
		y %= dimensions.y;
	}
	return MIN(dimensions.x * dimensions.y - 1, y * dimensions.x + x);
}

// Blending pixels using alpha channel
static _FORCE_INLINE_ uint32_t _fe_blend_pixel(uint32_t a, uint32_t b) {
	const _byteword pa = a, pb = b;
	const float w = pb.b[3] / 255.0;
	_byteword blended(0);
	for (int i = 0; i < 3; i++) {
		blended.b[i] = pa.b[i] + (pb.b[i] - pa.b[i]) * w;
	}
	blended.b[3] = pa.b[3];
	return blended.w;
}

// Pasting texture on another in certain place
static void _fe_paste_to(const Vector<_byteword> &to_paste, uint32_t *target, const Vector2i &origin, const Vector2i &to_paste_dim, const Vector2i &target_dim) {
	for (int x = 0; x < to_paste_dim.x; x++) {
		for (int y = 0; y < to_paste_dim.y; y++) {
			const int index = _fe_get_px_loop(origin.x - to_paste_dim.x / 2 + x, origin.y - to_paste_dim.y / 2 + y, target_dim);
			const int to_p = _fe_get_px(x, y, to_paste_dim);
			target[index] = _fe_blend_pixel(target[index], to_paste[to_p].w);
		}
	}
}

static Vector<_byteword> _fe_get_stamp(const Image *source, const uint32_t *source_pixels, real_t radius, real_t hardness, real_t randomize, real_t stamp_noise_mask, int stamp_rotate) {
	const real_t rnd = -0.2 + Math::randd() * 1.2;
	if (randomize > 0) {
		const int t_rad = radius * (1 + rnd * randomize);
		if (radius < source->get_width() && radius < source->get_height()) {
			radius = t_rad;
		}
	}

	const int radius2 = radius * 2;
	Vector<_byteword> stamp_pixels;
	ERR_FAIL_COND_V(stamp_pixels.resize(radius2 * radius2) != OK, Vector<_byteword>());

	const Vector2 origin = Vector2(Math::random(radius, source->get_width() - radius), Math::random(radius, source->get_height() - radius));
	const Vector2 stamp_dim = Vector2(radius2, radius2);

	const real_t random_off = Math::randf() * radius * 512;

	for (int x = 0; x < radius2; x++) {
		for (int y = 0; y < radius2; y++) {
			const int xx = -radius + x;
			const int yy = -radius + y;
			const int i = _fe_get_px(x, y, stamp_dim);
			stamp_pixels.write[i] = source_pixels[_fe_get_px(origin.x + xx, origin.y + yy, source->get_size())];

			const real_t distance = Vector2(xx, yy).distance_to(Vector2::ZERO);
			const real_t fade_mul = distance / (radius * 0.95);
			stamp_pixels.write[i].b[3] = MIN(255, MATH_LERP(255 + hardness * 215, 0, fade_mul));

			// Applying perlin noise to stamp
			if (stamp_noise_mask > 0) {
				if (stamp_pixels[i].b[3] < 235 + hardness * 15 + stamp_noise_mask * 10) {
					const real_t noise = Math::perlin(x / radius * 3 + random_off, y / radius * 3 + random_off);

					real_t spread_alpha = MATH_LERP(1, 0, MATH_INVLERP(255, 0, stamp_pixels[i].b[3]));

					real_t noise_mask = stamp_noise_mask;
					if (stamp_noise_mask > 1) {
						const real_t t_a = MATH_LERP(255 + MATH_LERP(hardness * 215, 0, noise_mask - 1), 0, fade_mul);
						spread_alpha = MATH_LERP(1, 0, MATH_INVLERP(255, 0, t_a));
						noise_mask -= 1;
					}

					real_t noise_alpha = MATH_LERP(1, noise, noise_mask * 0.95);
					noise_alpha = MATH_LERP(noise_alpha, noise_alpha * noise_alpha, noise_mask - 0.5);
					noise_alpha = MATH_LERP(1, noise_alpha, 1 - spread_alpha);

					stamp_pixels.write[i].b[3] = spread_alpha * (noise_alpha)*255;
				}
			}
		}
	}
	if (stamp_rotate >= 1) { // rotating
		_fe_rotate_image(stamp_pixels, radius2, radius2, Math::random(0, stamp_rotate));
	}
	return stamp_pixels;
}

Ref<Image> ImageTools::make_seamless(const Image *p_src, SeamlessStampMode p_stamp_mode, real_t p_stamper_radius, real_t p_stamp_density, real_t p_hardness, real_t p_stamp_noise_mask, real_t p_randomize, int p_stamp_rotate, SeamlessAxis p_to_loop) {
	ERR_FAIL_NULL_V(p_src, Ref<Image>());

	if (p_src->data.size() == 0) {
		return Ref<Image>();
	}

	if (p_src->format != Image::FORMAT_RGBA8 && p_src->format != Image::FORMAT_RGB8) {
		WARN_PRINT("Unsupported image format");
		return Ref<Image>();
	}

	if (p_stamp_mode == FE_STAMPING) {
		ERR_FAIL_RANGE_V(p_stamper_radius, 0, 1, Ref<Image>());
		ERR_FAIL_RANGE_V(p_stamp_density, 0, 1, Ref<Image>());
		ERR_FAIL_RANGE_V(p_hardness, 0, 1, Ref<Image>());
		ERR_FAIL_RANGE_V(p_stamp_noise_mask, 0, 2, Ref<Image>());
		ERR_FAIL_RANGE_V(p_randomize, 0, 0.5, Ref<Image>());
		ERR_FAIL_RANGE_V(p_stamp_rotate, 0, 360, Ref<Image>());
	} else {
		ERR_FAIL_RANGE_V(p_stamper_radius, 0, 1, Ref<Image>());
		ERR_FAIL_RANGE_V(p_stamp_density, -1, 1, Ref<Image>());
		ERR_FAIL_RANGE_V(p_hardness, 0, 2, Ref<Image>());
		ERR_FAIL_RANGE_V(p_stamp_noise_mask, 0, 3, Ref<Image>());
		ERR_FAIL_RANGE_V(p_randomize, 0, 1, Ref<Image>());
		ERR_FAIL_RANGE_V(p_stamp_rotate, 0, 360, Ref<Image>());
		if (p_to_loop != FE_XY) {
			p_to_loop = FE_XY;
			WARN_PRINT("Resetting p_to_loop to FE_XY");
		}
	}
	const int width = p_src->width;
	const int height = p_src->height;

	// Preparing variables to use down below

	print_verbose("Generating seamless texture / Preparing");

	PoolByteArray new_data = p_src->data;
	const uint32_t *source_pixels = (const uint32_t *)p_src->data.read().ptr();

	const bool do_x = (p_to_loop != FE_X);
	const bool do_y = (p_to_loop != FE_Y);

	// Stamping texture

	const float stamp_radius_x = width * MATH_LERP(0.05, .3, p_stamper_radius);
	const float stamp_radius_y = height * MATH_LERP(0.05, .3, p_stamper_radius);
	const float stamps_offset_x = stamp_radius_x * MATH_LERP(1.45, 0.45, p_stamp_density);
	const float stamps_offset_y = stamp_radius_y * MATH_LERP(1.45, 0.45, p_stamp_density);

	if (do_x) {
		const int stamps_count_x = width / stamps_offset_x;
		uint32_t *new_pixels = (uint32_t *)new_data.write().ptr();
		for (int x = 0; x <= stamps_count_x; x++) {
			Vector<_byteword> stamp = _fe_get_stamp(p_src, source_pixels, stamp_radius_x, p_hardness, p_randomize, p_stamp_noise_mask, p_stamp_rotate);
			const float boost = (p_stamp_mode == FE_SPLATMODE) ? (1 / (0.01 + p_stamper_radius)) : 1;
			Point2i paste_pos;
			paste_pos.x = x * stamps_offset_x;
			paste_pos.y = (stamp_radius_y * 2) * (-1 + Math::randf() * 2) * 0.5 * p_randomize * boost; // Randomize y
			_fe_paste_to(stamp, new_pixels, paste_pos, _fe_get_square_dims(stamp.size()), p_src->get_size());
		}
	}

	print_verbose("Generating seamless texture / Creating stamps");

	if (do_y) {
		const int stamps_count_y = width / (stamps_offset_y);
		uint32_t *new_pixels = (uint32_t *)new_data.write().ptr();
		for (int y = 0; y <= stamps_count_y; y++) {
			Vector<_byteword> stamp = _fe_get_stamp(p_src, source_pixels, stamp_radius_y, p_hardness, p_randomize, p_stamp_noise_mask, p_stamp_rotate);
			const float boost = (p_stamp_mode == FE_SPLATMODE) ? 1 / (0.01 + p_stamper_radius) : 1;
			Point2i paste_pos;
			paste_pos.x = (stamp_radius_x * 2) * (-1 + Math::randf() * 2) * 0.5 * p_randomize * boost; // Randomize x
			paste_pos.y = y * stamps_offset_y;
			_fe_paste_to(stamp, new_pixels, paste_pos, _fe_get_square_dims(stamp.size()), p_src->get_size());
		}
	}

	Ref<Image> target;
	target->create(width, height, false, Image::FORMAT_RGBA8, new_data);
	return target;
}
