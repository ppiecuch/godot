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
