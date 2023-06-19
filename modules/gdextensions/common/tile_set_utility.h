/**************************************************************************/
/*  tile_set_utility.h                                                    */
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

#pragma once

#include <stdint.h>
#include <stdio.h>

#include <utility>
#include <vector>

#include "core/int_types.h"
#include "core/print_string.h"

struct TileCoord {
	int x1, y1, x2, y2;
};

struct TileTextureCoord {
	real_t x1, y1, x2, y2;
};

typedef std::pair<int, int> TileRegions;
typedef std::vector<TileRegions> ArrayRegions;

struct BufferedImage {
	typedef uint8_t BufferData;

	const BufferData *data;
	const int width, height;

	int get_width() const { return width; }
	int get_height() const { return height; }
	template <int PIXEL_SIZE>
	uint32_t get_rgb(int x, int y) const;

	BufferedImage(BufferData *data, int width, int height) :
			data(data), width(width), height(height) {}
};

template <>
uint32_t BufferedImage::get_rgb<1>(int x, int y) const {
	return data[y * width + x];
}
template <>
uint32_t BufferedImage::get_rgb<2>(int x, int y) const {
	return *(uint16_t *)(data + (y * (width << 1) + (x << 1)));
}
template <>
uint32_t BufferedImage::get_rgb<3>(int x, int y) const {
	const int pixel_size = width + (width << 1);
	return (*(uint32_t *)(data + (y * pixel_size + x * pixel_size))) & 0x00ffffff;
}
template <>
uint32_t BufferedImage::get_rgb<4>(int x, int y) const {
	return (*(uint32_t *)(data + (y * (width << 2) + (x << 2)))) & 0x00ffffff;
}

template <int PIXEL_SIZE>
struct TileSetUtility {
	// Divide the tile into tiles based on the number of cols & rows
	// supplied.  Exclude any images that are a solid color.
	static std::vector<TileCoord> getTiles(const BufferedImage &tile, const ArrayRegions &cols, const ArrayRegions &rows, bool solid = true) {
		const int w = tile.get_width();
		const int h = tile.get_height();

		print_verbose(vformat("Processing regions of %d columns and %d rows with border color 0x%08x.", cols.size(), rows.size(), tile.getRGB<PIXEL_SIZE>(0, 0)));

		std::vector<TileCoord> tiles;
		for (int y = 0; y < rows.size(); y++) {
			for (int x = 0; x < cols.size(); x++) {
				TileCoord i = (TileCoord){ cols[x].first, rows[y].first, cols[x].second, rows[y].second };
				if (solid || !is_image_solid_color(tile, i)) {
					tiles.push_back(i);
				}
			}
		}
		return tiles;
	}

	// Takes an image that represents tiles of a tile set, and infers the
	// number of columns based on the assumption that the color at 0x0 in the
	// image represents a border color or frame for the contained tiles.
	static ArrayRegions infer_number_columns(const BufferedImage &img) {
		std::vector<bool> column_clear;

		// after this loop, we should have a series of contiguous regions
		// of 'true' in the array.
		column_clear.resize(img.getWidth());
		for (int ii = 0; ii < column_clear.size(); ii++) {
			column_clear[ii] = is_line_empty(img, ii, false);
		}
		return count_contiguous_regions(column_clear);
	}

	// Takes an image that represents tiles of a tile set, and infers the
	// number of rows based on the assumption that the color at 0x0 in the
	// image represents a border color or frame for the contained tiles.
	static ArrayRegions infer_number_rows(const BufferedImage &img) {
		std::vector<bool> column_clear;
		// after this loop, we should have a series of contiguous regions
		// of 'true' in the array.
		column_clear.resize(img.getHeight());
		for (int ii = 0; ii < column_clear.size(); ii++) {
			column_clear[ii] = is_line_empty(img, ii, true);
		}
		return count_contiguous_regions(column_clear);
	}

	// Count the number of contiguous regions of 'true'
	static ArrayRegions count_contiguous_regions(const std::vector<bool> &array) {
		bool new_region = false;
		int begin_region = -1, end_region = -1;
		ArrayRegions regs;
		int count = 0;
		for (int index = 0; index < array.size(); ++index) {
			const bool b = array[index];
			if (b) {
				if (new_region) {
					count++;
					end_region = index - 1;
					regs.push_back(std::pair<int, int>(begin_region, end_region));
					begin_region = end_region = -1;
				}
				new_region = false;
			} else {
				new_region = true;
				if (begin_region == -1) {
					begin_region = index;
				}
			}
		}
		print_verbose(vformat("Found %d regions.", count));
		return regs;
	}

	// Determine if this entire column or row of image pixels is empty.
	static bool is_line_empty(const BufferedImage &img, int pos, bool row) {
		if (!row) {
			for (int y = 0; y < img.get_height(); y++) {
				if (img.get_rgb<PIXEL_SIZE>(pos, y) != img.get_rgb<PIXEL_SIZE>(0, 0)) {
					return false;
				}
			}
		} else {
			for (int x = 0; x < img.get_width(); x++) {
				if (img.get_rgb<PIXEL_SIZE>(x, pos) != img.get_rgb<PIXEL_SIZE>(0, 0)) {
					return false;
				}
			}
		}
		return true;
	}

	// Determine if this image is one solid color (implies redundant tile)
	static bool is_image_solid_color(const BufferedImage &img, const TileCoord &coord) {
		const int c = img.get_rgb<PIXEL_SIZE>(0, 0);
		for (int x = coord.x1; x < coord.x2; x++) {
			for (int y = coord.y1; y < coord.y2; y++) {
				if (c != img.get_rgb<PIXEL_SIZE>(x, y)) {
					return false;
				}
			}
		}
		return true;
	}

	// Determine if this image is one solid color (implies redundant tile)
	static bool is_image_solid_color(const BufferedImage &img) {
		const TileCoord coord = (TileCoord){ 0, 0, img.get_width() + 1, img.get_height() + 1 };
		return is_image_solid_color(img, coord);
	}
};

typedef TileSetUtility<4> TileSetUtilityRGBA;
typedef TileSetUtility<3> TileSetUtilityRGB;
