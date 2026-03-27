/**************************************************************************/
/*  texture_region.cpp                                                    */
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

#include "texture_region.h"

#include "core/error_macros.h"
#include "core/math/math_funcs.h"

// Sets the texture and sets the coordinates to the size of the specified texture->
void TextureRegion::set_region(const Ref<Texture> &texture) {
	ERR_FAIL_NULL_MSG(texture, "Texture cannot be null.");
	source_texture = texture;
	set_region(0, 0, texture->get_width(), texture->get_height());
}

void TextureRegion::set_region(int x, int y, int width, int height) {
	ERR_FAIL_COND_MSG(source_texture.is_null(), "TextureRegion: source texture not set.");
	ERR_FAIL_COND_MSG(source_texture->get_width() == 0 || source_texture->get_height() == 0, "TextureRegion: texture has zero dimensions.");
	const real_t inv_tex_width = 1.0 / source_texture->get_width();
	const real_t inv_tex_height = 1.0 / source_texture->get_height();
	set_region(x * inv_tex_width, y * inv_tex_height, (x + width) * inv_tex_width, (y + height) * inv_tex_height);
	region_width = Math::abs(width);
	region_height = Math::abs(height);
}

void TextureRegion::set_region(real_t u, real_t v, real_t u2, real_t v2) {
	const int tex_width = source_texture->get_width(), tex_height = source_texture->get_height();
	region_width = Math::round(Math::abs(u2 - u) * tex_width);
	region_height = Math::round(Math::abs(v2 - v) * tex_height);

	// For a 1x1 region, adjust UVs toward pixel center to avoid filtering artifacts on AMD GPUs when drawing very stretched.
	if (region_width == 1 && region_height == 1) {
		const real_t adjust_x = 0.25 / tex_width;
		u += adjust_x;
		opposite.u -= adjust_x;
		const real_t adjust_y = 0.25 / tex_height;
		v += adjust_y;
		opposite.v -= adjust_y;
	}

	origin.u = u;
	origin.v = v;
	opposite.u = u2;
	opposite.v = v2;
}

// Sets the texture to that of the specified region and sets the coordinates relative to the specified region.
void TextureRegion::set_region(const TextureRegion &region, int x, int y, int width, int height) {
	source_texture = region.source_texture;
	set_region(region.get_region_x() + x, region.get_region_y() + y, width, height);
}

Ref<Texture> TextureRegion::get_texture() const {
	return source_texture;
}

void TextureRegion::set_texture(const Ref<Texture> &texture) {
	source_texture = texture;
}

real_t TextureRegion::get_u() const {
	return origin.u;
}

void TextureRegion::set_u(real_t u) {
	origin.u = u;
	region_width = Math::round(Math::abs(opposite.u - u) * source_texture->get_width());
}

real_t TextureRegion::get_v() const {
	return origin.v;
}

void TextureRegion::set_v(real_t v) {
	origin.v = v;
	region_height = Math::round(Math::abs(opposite.v - v) * source_texture->get_height());
}

real_t TextureRegion::get_u2() const {
	return opposite.u;
}

void TextureRegion::set_u2(real_t u2) {
	opposite.u = u2;
	region_width = Math::round(Math::abs(u2 - origin.u) * source_texture->get_width());
}

real_t TextureRegion::get_v2() const {
	return opposite.v;
}

void TextureRegion::set_v2(real_t v2) {
	opposite.v = v2;
	region_height = Math::round(Math::abs(v2 - origin.v) * source_texture->get_height());
}

int TextureRegion::get_region_x() const {
	return Math::round(origin.u * source_texture->get_width());
}

void TextureRegion::set_region_x(int x) {
	set_u(x / (real_t)source_texture->get_width());
}

int TextureRegion::get_region_y() const {
	return Math::round(origin.v * source_texture->get_height());
}

void TextureRegion::set_region_y(int y) {
	set_v(y / (real_t)source_texture->get_height());
}

int TextureRegion::get_region_width() const {
	return region_width;
}

void TextureRegion::set_region_width(int width) {
	if (is_flip_x()) {
		set_u(opposite.u + width / (real_t)source_texture->get_width());
	} else {
		set_u2(origin.u + width / (real_t)source_texture->get_width());
	}
}

int TextureRegion::get_region_height() const {
	return region_height;
}

void TextureRegion::set_region_height(int height) {
	if (is_flip_y()) {
		set_v(opposite.v + height / (real_t)source_texture->get_height());
	} else {
		set_v2(origin.v + height / (real_t)source_texture->get_height());
	}
}

void TextureRegion::scroll(real_t x_amount, real_t y_amount) {
	ERR_FAIL_COND_MSG(source_texture.is_null(), "TextureRegion: source texture not set.");
	if (x_amount != 0) {
		const real_t width = (opposite.u - origin.u) * source_texture->get_width();
		origin.u = Math::fposmod(origin.u + x_amount, 1);
		opposite.u = origin.u + width / source_texture->get_width();
	}
	if (y_amount != 0) {
		const real_t height = (opposite.v - origin.v) * source_texture->get_height();
		origin.v = Math::fposmod(origin.v + y_amount, 1);
		opposite.v = origin.v + height / source_texture->get_height();
	}
}

TextureRegion::TextureRegionArray TextureRegion::split(int tile_width, int tile_height) const {
	ERR_FAIL_COND_V_MSG(tile_width <= 0 || tile_height <= 0, TextureRegionArray(), "TextureRegion: tile dimensions must be positive.");
	ERR_FAIL_COND_V_MSG(source_texture.is_null(), TextureRegionArray(), "TextureRegion: source texture not set.");

	int x = get_region_x();
	int y = get_region_y();
	const int width = region_width;
	const int height = region_height;

	const int rows = height / tile_height;
	const int cols = width / tile_width;

	const int start_x = x;
	TextureRegionArray tiles;
	for (int row = 0; row < rows; row++, y += tile_height) {
		Vector<TextureRegion> r;
		ERR_FAIL_COND_V(r.resize(cols) != OK, TextureRegionArray());
		x = start_x;
		for (int col = 0; col < cols; col++, x += tile_width) {
			r.write[col] = TextureRegion(source_texture, x, y, tile_width, tile_height);
		}
		tiles.push_back(r);
	}

	return tiles;
}

TextureRegion::TextureRegionArray TextureRegion::split(const Ref<Texture> &texture, int tile_width, int tile_height) {
	TextureRegion region(texture);
	return region.split(tile_width, tile_height);
}

TextureRegion::TextureRegion(const Ref<Texture> &texture) {
	ERR_FAIL_NULL_MSG(texture, "Texture cannot be null.");
	source_texture = texture;
	set_region(0, 0, texture->get_width(), texture->get_height());
}

TextureRegion::TextureRegion(const Ref<Texture> &texture, int width, int height) {
	source_texture = texture;
	set_region(0, 0, width, height);
}

TextureRegion::TextureRegion(const Ref<Texture> &texture, int x, int y, int width, int height) {
	source_texture = texture;
	set_region(x, y, width, height);
}

TextureRegion::TextureRegion(const Ref<Texture> &texture, real_t u, real_t v, real_t u2, real_t v2) {
	source_texture = texture;
	set_region(u, v, u2, v2);
}

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"
#include "doctest/doctest_godot.h"
#include "tile_set_utility.h"

TEST_SUITE("[[texture_region]] TextureRegion") {
	TEST_CASE("[texture_region] default constructor zeroes members") {
		TextureRegion r;
		CHECK(r.get_texture().is_null());
		CHECK(r.get_u() == doctest::Approx(0));
		CHECK(r.get_v() == doctest::Approx(0));
		CHECK(r.get_u2() == doctest::Approx(0));
		CHECK(r.get_v2() == doctest::Approx(0));
		CHECK(r.get_region_width() == 0);
		CHECK(r.get_region_height() == 0);
	}

	TEST_CASE("[texture_region] flip detection on default is false") {
		TextureRegion r;
		CHECK_FALSE(r.is_flip_x());
		CHECK_FALSE(r.is_flip_y());
	}

	TEST_CASE("[texture_region] set_region(int) requires texture") {
		TextureRegion r;
		SUPPRESS_OUTPUT(r.set_region(0, 0, 32, 32));
		// Should not crash, just error
		CHECK(r.get_region_width() == 0);
	}

	TEST_CASE("[texture_region] scroll requires texture") {
		TextureRegion r;
		SUPPRESS_OUTPUT(r.scroll(0.1, 0.1));
	}

	TEST_CASE("[texture_region] split requires positive tile size") {
		TextureRegion r;
		SUPPRESS_OUTPUT({
			auto tiles = r.split(0, 32);
			CHECK(tiles.size() == 0);
		});
		SUPPRESS_OUTPUT({
			auto tiles = r.split(32, -1);
			CHECK(tiles.size() == 0);
		});
	}
}

TEST_SUITE("[[tile_set_utility]] TileSetUtility") {
	TEST_CASE("[tileset] BufferedImage basic access") {
		// 4x4 RGBA image, all red (255,0,0,255)
		uint8_t data[4 * 4 * 4];
		for (int i = 0; i < 4 * 4; i++) {
			data[i * 4 + 0] = 255;
			data[i * 4 + 1] = 0;
			data[i * 4 + 2] = 0;
			data[i * 4 + 3] = 255;
		}
		BufferedImage img = { data, 4, 4 };
		CHECK(img.get_width() == 4);
		CHECK(img.get_height() == 4);
		uint32_t pixel = img.get_rgb<4>(0, 0);
		CHECK(pixel != 0);
	}

	TEST_CASE("[tileset] solid color image detected") {
		// 8x8 solid blue
		uint8_t data[8 * 8 * 4];
		for (int i = 0; i < 8 * 8; i++) {
			data[i * 4 + 0] = 0;
			data[i * 4 + 1] = 0;
			data[i * 4 + 2] = 255;
			data[i * 4 + 3] = 255;
		}
		BufferedImage img = { data, 8, 8 };
		CHECK(TileSetUtilityRGBA::is_image_solid_color(img));
	}

	TEST_CASE("[tileset] non-solid image not detected as solid") {
		uint8_t data[4 * 4 * 4] = {};
		// Set pixel (2,2) to different color
		int idx = (2 * 4 + 2) * 4;
		data[idx] = 255;
		data[idx + 1] = 255;
		data[idx + 2] = 255;
		data[idx + 3] = 255;
		BufferedImage img = { data, 4, 4 };
		CHECK_FALSE(TileSetUtilityRGBA::is_image_solid_color(img));
	}

	TEST_CASE("[tileset] count_contiguous_regions finds blocks") {
		// Pattern: false false TRUE TRUE TRUE false TRUE TRUE false
		std::vector<bool> arr = { false, false, true, true, true, false, true, true, false };
		auto regions = TileSetUtilityRGBA::count_contiguous_regions(arr);
		CHECK(regions.size() == 2);
		CHECK(regions[0].first == 2);
		CHECK(regions[0].second == 5); // [2, 5)
		CHECK(regions[1].first == 6);
		CHECK(regions[1].second == 8); // [6, 8)
	}

	TEST_CASE("[tileset] count_contiguous_regions empty array") {
		std::vector<bool> arr;
		auto regions = TileSetUtilityRGBA::count_contiguous_regions(arr);
		CHECK(regions.size() == 0);
	}

	TEST_CASE("[tileset] count_contiguous_regions all true") {
		std::vector<bool> arr = { true, true, true };
		auto regions = TileSetUtilityRGBA::count_contiguous_regions(arr);
		CHECK(regions.size() == 1);
		CHECK(regions[0].first == 0);
	}

	TEST_CASE("[tileset] infer columns on grid image") {
		// 9x1 image: border(0) | tile(FF) tile(FF) | border(0) | tile(FF) tile(FF) | border(0)
		// Columns: [0]=border [1,2]=tile [3]=border [4,5]=tile [6]=border
		uint8_t data[9 * 4]; // 9 pixels, RGBA
		memset(data, 0, sizeof(data)); // all black (border)
		// Tile pixels at x=1,2 and x=4,5
		for (int x : { 1, 2, 4, 5, 7, 8 }) {
			int idx = x * 4;
			data[idx] = 255;
			data[idx + 1] = 255;
			data[idx + 2] = 255;
			data[idx + 3] = 255;
		}
		BufferedImage img = { data, 9, 1 };
		auto cols = TileSetUtilityRGBA::infer_number_columns(img);
		CHECK(cols.size() >= 2);
	}

	TEST_CASE("[tileset] is_line_empty on border") {
		// 3x3 image with black border, white center
		uint8_t data[3 * 3 * 4];
		memset(data, 0, sizeof(data));
		// Center pixel white
		int idx = (1 * 3 + 1) * 4;
		data[idx] = 255;
		data[idx + 1] = 255;
		data[idx + 2] = 255;
		data[idx + 3] = 255;
		BufferedImage img = { data, 3, 3 };
		CHECK(TileSetUtilityRGBA::is_line_empty(img, 0, false)); // column 0 = all black = matches border
		CHECK_FALSE(TileSetUtilityRGBA::is_line_empty(img, 1, false)); // column 1 has white center
		CHECK(TileSetUtilityRGBA::is_line_empty(img, 0, true)); // row 0 = all black
		CHECK_FALSE(TileSetUtilityRGBA::is_line_empty(img, 1, true)); // row 1 has white
	}
}

#endif // DOCTEST
