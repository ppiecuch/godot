/**************************************************************************/
/*  texture_region.h                                                      */
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

#ifndef GDEXT_TEXTURE_REGION_H
#define GDEXT_TEXTURE_REGION_H

#include "core/vector.h"
#include "scene/resources/texture.h"

// Defines a rectangular area of a source texture. The coordinate system used has its origin
// in the upper left corner with the x-axis pointing to the right and the y axis pointing downwards.
//
// The width of the texture region may be negative to flip the sprite when drawn.
// The height of the texture region may be negative to flip the sprite when drawn.
class TextureRegion {
	Ref<Texture> source_texture;
	struct _uv_t {
		real_t u, v;
	} origin, opposite;
	int region_width, region_height;

public:
	typedef Vector<Vector<TextureRegion>> TextureRegionArray;

	// Sets the texture and sets the coordinates to the size of the specified texture.
	void set_region(const Ref<Texture> &texture);

	void set_region(int x, int y, int width, int height);
	void set_region(real_t u, real_t v, real_t u2, real_t v2);

	// Sets the texture to that of the specified region and sets the coordinates relative to the specified region.
	void set_region(const TextureRegion &region, int x, int y, int width, int height);

	Ref<Texture> get_texture() const;
	void set_texture(const Ref<Texture> &texture);

	real_t get_u() const;
	void set_u(real_t u);

	real_t get_v() const;
	void set_v(real_t v);

	real_t get_u2() const;
	void set_u2(real_t u2);

	real_t get_v2() const;
	void set_v2(real_t v2);

	int get_region_x() const;
	void set_region_x(int x);

	int get_region_y() const;
	void set_region_y(int y);

	int get_region_width() const;
	void set_region_width(int width);

	int get_region_height() const;
	void set_region_height(int height);

	_FORCE_INLINE_ void flip(bool x, bool y) {
		if (x) {
			std::swap(origin.u, opposite.u);
		}
		if (y) {
			std::swap(origin.v, opposite.v);
		}
	}

	_FORCE_INLINE_ bool is_flip_x() const {
		return origin.u > opposite.u;
	}

	_FORCE_INLINE_ bool is_flip_y() const {
		return origin.v > opposite.v;
	}

	// Offsets the region relative to the current region. Generally the region's size should be the entire size of the texture in
	// the direction(s) it is scrolled.
	// param x_amount: The percentage to offset horizontally.
	// param y_amount: The percentage to offset vertically. This is done in texture space, so up is negative.
	void scroll(real_t x_amount, real_t y_amount);

	// Helper method to create tiles out of this TextureRegion starting from the top left corner going to the right and ending at
	// the bottom right corner. Only complete tiles will be returned so if the region's width or height are not a multiple of the
	// tile width and height not all of the region will be used. This will not work on texture regions returned from a TextureAtlas
	// that either have whitespace removed or where flipped before the region is split.
	//
	// param tile_width: a tile's width in pixels
	// param tile_height: a tile's height in pixels
	// return: a 2D array of TextureRegions indexed by [row][column]
	TextureRegionArray split(int tile_width, int tile_height) const;

	// Helper method to create tiles out of the given {@link Texture} starting from the top left corner going to the right and
	// ending at the bottom right corner. Only complete tiles will be returned so if the texture's width or height are not a
	// multiple of the tile width and height not all of the texture will be used.
	//
	// param texture the Texture
	// param tile_width a tile's width in pixels
	// param tile_height a tile's height in pixels
	// return a 2D array of TextureRegions indexed by [row][column]
	static TextureRegionArray split(const Ref<Texture> &texture, int tile_width, int tile_height);

	// Constructs a region with the same texture as the specified region and sets the coordinates relative
	// to the specified region.
	TextureRegion(const TextureRegion &region, int x, int y, int width, int height) {
		set_region(region, x, y, width, height);
	}

	// Constructs a region the size of the specified texture.
	TextureRegion(const Ref<Texture> &texture);

	TextureRegion(const Ref<Texture> &texture, int width, int height);
	TextureRegion(const Ref<Texture> &texture, int x, int y, int width, int height);
	TextureRegion(const Ref<Texture> &texture, real_t u, real_t v, real_t u2, real_t v2);

	// Constructs a region that cannot be used until a texture and texture coordinates are set.
	TextureRegion() {
	}
};

#endif // GDEXT_TEXTURE_REGION_H
