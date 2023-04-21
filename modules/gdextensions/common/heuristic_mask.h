
#include "core/image,h"

enum {
	DiffuseAlphaDither,
	OrderedAlphaDither,
	ThresholdDither,
} MaskOptions;

const uint8_t _bayer_matrix[16][16] = {
    { 0x1, 0xc0, 0x30, 0xf0, 0xc, 0xcc, 0x3c, 0xfc,
      0x3, 0xc3, 0x33, 0xf3, 0xf, 0xcf, 0x3f, 0xff},
    { 0x80, 0x40, 0xb0, 0x70, 0x8c, 0x4c, 0xbc, 0x7c,
      0x83, 0x43, 0xb3, 0x73, 0x8f, 0x4f, 0xbf, 0x7f},
    { 0x20, 0xe0, 0x10, 0xd0, 0x2c, 0xec, 0x1c, 0xdc,
      0x23, 0xe3, 0x13, 0xd3, 0x2f, 0xef, 0x1f, 0xdf},
    { 0xa0, 0x60, 0x90, 0x50, 0xac, 0x6c, 0x9c, 0x5c,
      0xa3, 0x63, 0x93, 0x53, 0xaf, 0x6f, 0x9f, 0x5f},
    { 0x8, 0xc8, 0x38, 0xf8, 0x4, 0xc4, 0x34, 0xf4,
      0xb, 0xcb, 0x3b, 0xfb, 0x7, 0xc7, 0x37, 0xf7},
    { 0x88, 0x48, 0xb8, 0x78, 0x84, 0x44, 0xb4, 0x74,
      0x8b, 0x4b, 0xbb, 0x7b, 0x87, 0x47, 0xb7, 0x77},
    { 0x28, 0xe8, 0x18, 0xd8, 0x24, 0xe4, 0x14, 0xd4,
      0x2b, 0xeb, 0x1b, 0xdb, 0x27, 0xe7, 0x17, 0xd7},
    { 0xa8, 0x68, 0x98, 0x58, 0xa4, 0x64, 0x94, 0x54,
      0xab, 0x6b, 0x9b, 0x5b, 0xa7, 0x67, 0x97, 0x57},
    { 0x2, 0xc2, 0x32, 0xf2, 0xe, 0xce, 0x3e, 0xfe,
      0x1, 0xc1, 0x31, 0xf1, 0xd, 0xcd, 0x3d, 0xfd},
    { 0x82, 0x42, 0xb2, 0x72, 0x8e, 0x4e, 0xbe, 0x7e,
      0x81, 0x41, 0xb1, 0x71, 0x8d, 0x4d, 0xbd, 0x7d},
    { 0x22, 0xe2, 0x12, 0xd2, 0x2e, 0xee, 0x1e, 0xde,
      0x21, 0xe1, 0x11, 0xd1, 0x2d, 0xed, 0x1d, 0xdd},
    { 0xa2, 0x62, 0x92, 0x52, 0xae, 0x6e, 0x9e, 0x5e,
      0xa1, 0x61, 0x91, 0x51, 0xad, 0x6d, 0x9d, 0x5d},
    { 0xa, 0xca, 0x3a, 0xfa, 0x6, 0xc6, 0x36, 0xf6,
      0x9, 0xc9, 0x39, 0xf9, 0x5, 0xc5, 0x35, 0xf5},
    { 0x8a, 0x4a, 0xba, 0x7a, 0x86, 0x46, 0xb6, 0x76,
      0x89, 0x49, 0xb9, 0x79, 0x85, 0x45, 0xb5, 0x75},
    { 0x2a, 0xea, 0x1a, 0xda, 0x26, 0xe6, 0x16, 0xd6,
      0x29, 0xe9, 0x19, 0xd9, 0x25, 0xe5, 0x15, 0xd5},
    { 0xaa, 0x6a, 0x9a, 0x5a, 0xa6, 0x66, 0x96, 0x56,
      0xa9, 0x69, 0x99, 0x59, 0xa5, 0x65, 0x95, 0x55}
};

typedef unsigned int rgb_t;

_FORCE_INLINE_ uint8_t to_gray(uint8_t r, uint8_t g, uint8_t b) { return (r*11 + g*16 + b*5) / 32; } // convert r,g,b to gray 0..255

// Creates and returns a 1-bpp heuristic mask for this image.
// The function works by selecting a color from one of the corners,
// then chipping away pixels of that color starting at all the edges.
// The four corners vote for which color is to be masked away. In
// case of a draw (this generally means that this function is not
// applicable to the image), the result is arbitrary.
// If 'p_clip_tight' is true (the default) the mask is just large
// enough to cover the pixels; otherwise, the mask is larger than the
// data pixels.
// Note that this function disregards the alpha buffer.

PoolByteArray create_heuristic_mask(const Ref<Image> &p_image, bool p_clip_tight) {
	if (p_image->get_format() != Image::FORMAT_RGBA8) {
		Ref<Image> img32 = p_image->converted(Image::FORMAT_RGBA8);
		return img32->create_heuristic_mask(p_clip_tight);
	}

	const int bytes_per_line = p_image->get_width() * 4;

#define SCAN_LINE(y) (bytes_per_line * y)
#define PIX(x, y)  (*((rgb_t*)SCAN_LINE(y)+x) & 0x00ffffff)

	const int w = p_image.get_width();
	const int h = p_image.get_height();
	PoolByteArray m;
	const int m_bytes_per_line = (p_image.get_width() + 7) / 8;
	m.resize(m_bytes_per_line * h);
	m.fill(0xff);

	rgb_t background = PIX(0, 0);
	if (background != PIX(w-1, 0) &&
		background != PIX(0, h-1) &&
		background != PIX(w-1, h-1)) {
		background = PIX(w-1, 0);
		if (background != PIX(w-1, h-1) &&
			background != PIX(0, h-1) &&
			PIX(0, h-1) == PIX(w-1, h-1)) {
			background = PIX(w-1, h-1);
		}
	}

	int x, y;
	bool done = false;
	uint8_t *ypp, *ypc, *ypn;
	while (!done) {
		done = true;
		ypn = m.scanLine(0);
		ypc = 0;
		for (y = 0; y < h; y++) {
			ypp = ypc;
			ypc = ypn;
			ypn = (y == h-1) ? 0 : m.scanLine(y+1);
			_rgb_t *p = (_rgb_t *)SCAN_LINE(y);
			for (x = 0; x < w; x++) {
				// slowness here - it's possible to do six of these tests
				// together in one go. oh well.
				if ((x == 0 || y == 0 || x == w-1 || y == h-1 ||
					!(*(ypc + ((x-1) >> 3)) & (1 << ((x-1) & 7))) ||
					!(*(ypc + ((x+1) >> 3)) & (1 << ((x+1) & 7))) ||
					!(*(ypp + (x     >> 3)) & (1 << (x     & 7))) ||
					!(*(ypn + (x     >> 3)) & (1 << (x     & 7)))) &&
					((*(ypc + (x     >> 3)) & (1 << (x     & 7)))) &&
					((*p & 0x00ffffff) == background)) {
					done = false;
					*(ypc + (x >> 3)) &= ~(1 << (x & 7));
				}
				p++;
			}
		}
	}

	if (!p_clip_tight) {
		ypn = m.scanLine(0);
		ypc = 0;
		for (y = 0; y < h; y++) {
			ypp = ypc;
			ypc = ypn;
			ypn = (y == h-1) ? 0 : m.scanLine(y+1);
			rgb_t *p = (rgb_t *)SCAN_LINE(y);
			for (x = 0; x < w; x++) {
				if ((*p & 0x00ffffff) != background) {
					if (x > 0) {
						*(ypc + ((x-1) >> 3)) |= (1 << ((x-1) & 7));
					}
					if (x < w-1) {
						*(ypc + ((x+1) >> 3)) |= (1 << ((x+1) & 7));
					}
					if (y > 0) {
						*(ypp + (x >> 3)) |= (1 << (x & 7));
					}
					if (y < h-1) {
						*(ypn + (x >> 3)) |= (1 << (x & 7));
					}
				}
				p++;
			}
		}
	}

#undef PIX

	return m;
}

// Builds and returns a 1-bpp mask from the alpha buffer in this
// image. Returns a null image if the image's format is RGB32.
// The 'flags' argument controls the conversion
// process. Passing 0 for flags sets all the default options.
// The returned image has little-endian bit order (LSB), which you can
// convert to big-endian (MSB) using the convert_to_format()
// function.

PoolByteArray create_alpha_mask(const Ref<Image> &p_image, MaskOptions p_flags) {
	if (!d || d->format == QImage::Format_RGB32) {
		return Ref<Image>();
	}

	if (d->depth == 1) {
		// A monochrome pixmap, with alpha channels on those two colors.
		// Pretty unlikely, so use less efficient solution.
		return convert_to_format(Format_Indexed8, flags).create_alpha_mask(flags);
	}

	if (p_image->is_valid()) {
		return dither_to_mono(p_image, flags, true, true);
	}
	return PoolByteArray();
}

Ref<Image> create_mask_from_color(const Ref<Image> &p_image, const Color &p_color, bool p_mask_out_color) {
	ERR_FAIL_NULL(p_image);
	PoolByteArray mask_data;
	mask_data.resize(p_image->get_width() * p_image->get_height());
	mask_data.fill(0);
	uint8_t *s = mask_data.bits();
	const int mask_data_bytes_per_line = (p_image->get_width() + 7) / 8;

	if (depth() == 32) {
		for (int h = 0; h < p_image->get_height(); h++) {
			const uint32_t *sl = (uint32_t *) scanLine(h);
			for (int w = 0; w < p_image->get_width(); w++) {
				if (sl[w] == color) {
					*(s + (w >> 3)) |= (1 << (w & 7));
				}
			}
			s += mask_data.bytesPerLine();
		}
	} else {
		for (int h = 0; h < p_image->get_height(); h++) {
			for (int w = 0; w < p_image->get_width(); w++) {
				if (p_image->pixel(w, h) == color) {
					*(s + (w >> 3)) |= (1 << (w & 7));
				}
			}
			s += mask_data.bytesPerLine();
		}
	}
	if  (p_mask_out_color) {
		invert_pixels(mask_data);
	}
	return memnew(Image(p_image->get_width(), p_image->get_height(), false, Image::FORMAT_L8, convert_mono_to_indexed8(mask_data, PoolColorArray())));
}


/// Internal functions


// Inverts all pixel values in the image.
// The default mode leaves the alpha channel unchanged.
// If the 'p_invert_alpha' is true, the alpha
// bits are also inverted.
// Inverting an 8-bit image means to replace all pixels using color
// index 'i' with a pixel using color index 255 minus 'i'. The same
// is the case for a 1-bit image. Note that the color table is not
// changed.

static void invert_pixels(PoolByteArray &p_data, int p_depth, bool  p_invert_alpha) {
	if (p_depth != 32) {
		// number of used bytes pr line
		int bpl = (d->width * d->depth + 7) / 8;
		int pad = d->bytes_per_line - bpl;
		uint8_t *sl = p_data.write().ptr();
		for (int y=0; y<d->height; ++y) {
			for (int x=0; x<bpl; ++x) {
				*sl++ ^= 0xff;
			}
			sl += pad;
		}
	} else {
		uint8_t *data = p_data.write().ptr();
		uint32_t *p = (uint32_t*)data;
		uint32_t *end = (uint32_t*)(data + p_data.size());
		unit32_t xorbits = p_invert_alpha ? 0xffffffff : 0x00ffffff;
		while (p < end) {
			*p++ ^= xorbits;
		}
	}
}

static PoolByteArray dither_to_mono(const Ref<Image> &p_src, MaskOptions flags, bool p_from_alpha, bool p_mono_lsb) {
	dst->colortable.clear();
	dst->colortable.append(0xffffffff);
	dst->colortable.append(0xff000000);

	PoolByteArray conv;
	conv.resize();

	const size_t src_bytes_per_line = ;

	enum { Threshold, Ordered, Diffuse } dithermode;

	if (p_from_alpha) {
		if (flags & DiffuseAlphaDither) {
			dithermode = Diffuse;
		} else if (flags & OrderedAlphaDither) {
			dithermode = Ordered;
		} else {
			dithermode = Threshold;
		}
	} else {
		if (flags & ThresholdDither) {
			dithermode = Threshold;
		} else if (flags & OrderedDither) {
			dithermode = Ordered;
		} else {
			dithermode = Diffuse;
		}
	}

	int w = p_src->get_width();
	int h = p_src->get_height();
	int d = src->depth;
	uint8_t gray[256]; // gray map for 8 bit images
	bool  use_gray = (d == 8);
	if (use_gray) { // make gray map
		if (p_from_alpha) {
			// alpha 0x00 -> 0 pixels (white)
			// alpha 0xff -> 1 pixels (black)
			for (int i = 0; i < src->colortable.size(); i++) {
				gray[i] = (255 - (src->colortable.at(i) >> 24));
			}
		} else {
			// pixel 0x00 -> 1 pixels (black)
			// pixel 0xff -> 0 pixels (white)
			for (int i = 0; i < src->colortable.size(); i++) {
				gray[i] = to_gray(src->colortable.at(i));
			}
		}
	}

	uint8_t *dst_data = conv.write().ptr();
	int dst_bpl = dst_bytes_per_line;
	const uint8_t *src_data = p_src->data;
	int src_bpl = src->bytes_per_line;

	switch (dithermode) {
		case Diffuse: {
			PoolIntArray lineBuffer;
			lineBuffer.resize(w * 2);
			int *line1 = lineBuffer.data();
			int *line2 = lineBuffer.data() + w;
			int bmwidth = (w+7)/8;

			int *b1, *b2;
			int wbytes = w * (d/8);
			const uint8_t *p = src->data;
			const uint8_t *end = p + wbytes;
			b2 = line2;
			if (use_gray) { // 8 bit image
				while (p < end) {
					*b2++ = gray[*p++];
				}
			} else { // 32 bit image
				if (fromalpha) {
					while (p < end) {
						*b2++ = 255 - (*(uint32_t*)p >> 24);
						p += 4;
					}
				} else {
					while (p < end) {
						*b2++ = to_gray(*(uint32_t*)p);
						p += 4;
					}
				}
			}
			for (int y=0; y<h; y++) { // for each scan line...
				int *tmp = line1; line1 = line2; line2 = tmp;
				bool not_last_line = y < h - 1;
				if (not_last_line) { // calc. grayvals for next line
					p = src->data + (y+1)*src_bytes_per_line;
					end = p + wbytes;
					b2 = line2;
					if (use_gray) { // 8 bit image
						while (p < end) {
							*b2++ = gray[*p++];
						}
					} else { // 24 bit image
						if (fromalpha) {
							while (p < end) {
								*b2++ = 255 - (*(uint32_t*)p >> 24);
								p += 4;
							}
						} else {
							while (p < end) {
								*b2++ = GRAY(*(uint32_t*)p);
								p += 4;
							}
						}
					}
				}

				int err;
				uint8_t *p = dst->data + y*dst->bytes_per_line;
				memset(p, 0, bmwidth);
				b1 = line1;
				b2 = line2;
				int bit = 7;
				for (int x=1; x<=w; x++) {
					if (*b1 < 128) { // black pixel
						err = *b1++;
						*p |= 1 << bit;
					} else { // white pixel
						err = *b1++ - 255;
					}
					if (bit == 0) {
						p++;
						bit = 7;
					} else {
						bit--;
					}
					if (x < w) {
						*b1 += (err*7)>>4; // spread error to right pixel
					}
					if (not_last_line) {
						b2[0] += (err*5)>>4; // pixel below
						if (x > 1) {
							b2[-1] += (err*3)>>4; // pixel below left
						}
						if (x < w) {
							b2[1] += err>>4; // pixel below right
						}
					}
					b2++;
				}
			}
		} break;

		case Ordered: {
			memset(dst->data, 0, dst->nbytes);
			if (d == 32) {
				for (int i=0; i<h; i++) {
					const uint32_t *p = (const uint32_t *)src_data;
					const uint32_t *end = p + w;
					uchar *m = dst_data;
					int bit = 7;
					int j = 0;
					if (fromalpha) {
						while (p < end) {
							if ((*p++ >> 24) >= _bayer_matrix[j++&15][i&15])
								*m |= 1 << bit;
							if (bit == 0) {
								m++;
								bit = 7;
							} else {
								bit--;
							}
						}
					} else {
						while (p < end) {
							if ((uint32_t)to_gray(*p++) < _bayer_matrix[j++&15][i&15])
								*m |= 1 << bit;
							if (bit == 0) {
								m++;
								bit = 7;
							} else {
								bit--;
							}
						}
					}
					dst_data += dst_bpl;
					src_data += src_bpl;
				}
			} else
				/* (d == 8) */ {
				for (int i=0; i<h; i++) {
					const uint8_t *p = src_data;
					const uint8_t *end = p + w;
					uint8_t *m = dst_data;
					int bit = 7;
					int j = 0;
					while (p < end) {
						if ((uint32_t)gray[*p++] < _bayer_matrix[j++&15][i&15])
							*m |= 1 << bit;
						if (bit == 0) {
							m++;
							bit = 7;
						} else {
							bit--;
						}
					}
					dst_data += dst_bpl;
					src_data += src_bpl;
				}
			}
		} break;

		default: { // Threshold:
			memset(dst->data, 0, dst->nbytes);
			if (d == 32) {
				for (int i=0; i<h; i++) {
					const uint32_t *p = (const uint32_t *)src_data;
					const uint32_t *end = p + w;
					uint8_t *m = dst_data;
					int bit = 7;
					if (fromalpha) {
						while (p < end) {
							if ((*p++ >> 24) >= 128) {
								*m |= 1 << bit; // Set mask "on"
							}
							if (bit == 0) {
								m++;
								bit = 7;
							} else {
								bit--;
							}
						}
					} else {
						while (p < end) {
							if (to_gray(*p++) < 128)
								*m |= 1 << bit; // Set pixel "black"
							if (bit == 0) {
								m++;
								bit = 7;
							} else {
								bit--;
							}
						}
					}
					dst_data += dst_bpl;
					src_data += src_bpl;
				}
			} else {
				if (d == 8) {
					for (int i=0; i<h; i++) {
						const uint8_t *p = src_data;
						const uint8_t *end = p + w;
						uint8_t *m = dst_data;
						int bit = 7;
						while (p < end) {
							if (gray[*p++] < 128) {
								*m |= 1 << bit; // Set mask "on"/ pixel "black"
							}
							if (bit == 0) {
								m++;
								bit = 7;
							} else {
								bit--;
							}
						}
						dst_data += dst_bpl;
						src_data += src_bpl;
					}
				}
			}
		} break;
	}

	if (p_mono_lsb) {
		// need to swap bit order
		uint8_t *sl = dst->data;
		int bpl = (dst->width + 7) * dst->depth / 8;
		int pad = dst->bytes_per_line - bpl;
		for (int y=0; y<dst->height; ++y) {
			for (int x=0; x<bpl; ++x) {
				*sl = bitflip[*sl];
				++sl;
			}
			sl += pad;
		}
	}

	return conv;
}

static PoolIntArray convert_mono_to_32(const PoolByteArray &p_src, const Size2 &p_size, const PoolColorArray &p_color_table, bool p_mono_lsb) {
	// Default to black / white colors
	PoolColorArray color_table = p_color_table;
	if (color_table.size() < 2) {
		if (color_table.size() == 0) {
			color_table.push_back(Color(0, 0, 0));
		}
		color_table.push_back(Color(1, 1, 1));
	}

	PoolByteArray conv;
	conv.resize(p_src.size() * 32);
	const uint8_t *src_data = p_src.ptr();
	const size_t src_bytes_per_line = (p_size.width + 7) / 8;
	uint8_t *dest_data = conv.write().ptr();
	const size_t dest_bytes_per_line = p_size.width * 4;
	if (p_mono_lsb) {
		for (int y = 0; y < dest->height; y++) {
			uint32_t *p = (uint32_t *)dest_data;
			for (int x = 0; x < dest->width; x++) {
				*p++ = color_table.get((src_data[x>>3] >> (x & 7)) & 1);
			}
			src_data += src_bytes_per_line;
			dest_data += dest_bytes_per_line;
		}
	} else {
		for (int y = 0; y < dest->height; y++) {
			uint *p = (uint *)dest_data;
			for (int x = 0; x < dest->width; x++) {
				*p++ = color_table.get((src_data[x>>3] >> (7 - (x & 7))) & 1);
			}
			src_data += src_bytes_per_line;
			dest_data += dest_bytes_per_line;
		}
	}
}


static PoolByteArray convert_mono_to_indexed8(const PoolByteArray &p_src, const Size2 &p_size, const PoolColorArray &p_color_table, bool p_mono_lsb) {
	PoolColorArray color_table = p_color_table;
	if (color_table.size() > 2) {
		color_table.resize(2);
	} else if (color_table.size() < 2) {
		if (color_table.size() == 0) {
			color_table.push_back(Color(0, 0, 0));
		}
		color_table.push_back(Color(1, 1, 1));
	}

	PoolByteArray conv;
	conv.resize(p_src.size() * 8);
	const uint8_t *src_data = p_src.ptr();
	const size_t src_bytes_per_line = (p_size.width + 7) / 8;
	uint8_t *dest_data = conv.write().ptr();
	const size_t dest_bytes_per_line = p_size.width * 1;
	struct {
	} const
	if (p_mono_lsb) {
		for (int y = 0; y < dest->height; y++) {
			uint8_t *p = dest_data;
			for (int x = 0; x < dest->width; x++) {
				*p++ = (src_data[x>>3] >> (x & 7)) & 1;
			}
			src_data += src_bytes_per_line;
			dest_data += dest_bytes_per_line;
		}
	} else {
		for (int y = 0; y < dest->height; y++) {
			uint8_t *p = dest_data;
			for (int x = 0; x < dest->width; x++) {
				*p++ = (src_data[x>>3] >> (7 - (x & 7))) & 1;
			}
			src_data += src_bytes_per_line;
			dest_data += dest_bytes_per_line;
		}
	}
	return conv;
}
