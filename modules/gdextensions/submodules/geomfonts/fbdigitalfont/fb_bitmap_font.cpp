/**************************************************************************/
/*  fb_bitmap_font.cpp                                                    */
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

#include "common/gd_core.h"
#include "common/resources_cache.h"
#include "core/image.h"
#include "core/math/math_defs.h"
#include "core/math/rect2.h"
#include "core/variant.h"
#include "scene/resources/texture.h"
#include "servers/visual_server.h"

#include "fb_font_draw.h"

#include "lights.h"

#include <vector>

static std::vector<std::vector<bool>> coord_for_symbol(FBFontSymbolType symbol);

// some convenience constants for the texture dimensions:
#define phf 16 // the point half-width in px. must be a power of two <= 256.
#define psz (phf << 1) // point size
#define psm (psz - 1) // size minus one
#define phs (phf * phf) // half size squared
#define pdb (psz << 1) // double size
#define pct (psz >> 1) // point center
#define prs ((phf - 1) * (phf - 1)) // radius squared

#define frsqrtes(v) (1 / Math::sqrt(v)) // reciprocal square root

static std::vector<real_t> logtbl; // for sphere lookups

static real_t ifun(real_t x, real_t y, real_t F) { // compute falloff at x,y with exponent F [-1..1]
	real_t S = x * x + y;
	real_t L;
	if (S) { // estimate sqrt, accurate to about 1/4096
		L = frsqrtes(S);
		L = L * S;
	} else {
		L = 0;
	}
	if (F == 0) { // intensity: [-1..0..1] = bloom..normal..fuzzy
		S = 0;
	} else if (F > 0) {
		S = Math::exp(logtbl[(int)(L * 4)] / F); // this is just pow(L/phf, F) but with a lookup table
	} else {
		// this is a bit more complex - logarithm ramp up to a solid dot with slight S blend at cusp
		S = L < (1 + F) * phf ? 0 : Math::exp((1 + F * 0.9) / (L / phf - 1 - F) * logtbl[(int)(L * 4)]);
	}
	return 1 - S;
}

template <int dim>
Point2i disc_to_square(int x, int y) { // mapping a circular disc to a square region
	const real_t unit = 2.0 / dim;
	const real_t half = dim / 2.0;
	const real_t squared = 0.4; // 0.1 .. 0.5
	const real_t nx = x * unit, ny = y * unit; // -1 .. 1
	const real_t u = nx * Math::sqrt(1 - squared * ny * ny);
	const real_t v = ny * Math::sqrt(1 - squared * nx * nx);
	return Point2i(Math::round(u * half), Math::round(v * half));
}

#define default_circle_squared 0.3

// 8-way symmetric macro
#define putpixel8(x, y, I)                                                                                       \
	{                                                                                                            \
		const uint8_t c = 255 * (inv ? 1 - I : I);                                                               \
		buffer[(pct + x) + (pct + y) * psz] =                                                                    \
				buffer[(pct - 1 - x) + (pct + y) * psz] =                                                        \
						buffer[(pct + x) + (pct - 1 - y) * psz] =                                                \
								buffer[(pct - 1 - x) + (pct - 1 - y) * psz] =                                    \
										buffer[(pct + y) + (pct + x) * psz] =                                    \
												buffer[(pct - 1 - y) + (pct + x) * psz] =                        \
														buffer[(pct + y) + (pct - 1 - x) * psz] =                \
																buffer[(pct - 1 - y) + (pct - 1 - x) * psz] = c; \
	}

// 4-way symmetric macro
#define putpixel4(x, y, c)                                               \
	{                                                                    \
		buffer[(pct + x) + (pct + y) * psz] =                            \
				buffer[(pct + x) + (pct - y) * psz] =                    \
						buffer[(pct - x) + (pct + y) * psz] =            \
								buffer[(pct - x) + (pct - y) * psz] = c; \
	}

static PoolByteArray generate_circle_aa_data(real_t fall_off, bool inv) {
	if (logtbl.empty()) {
		logtbl.resize(pdb);
		for (int i = 1; i <= pdb; i++) {
			logtbl[i - 1] = Math::log(real_t(i) / pdb); // lookup table has 4x the bitmap resolution
		}
	}
	int x = phf - 1, y = 0;
	real_t T = 0;
	PoolByteArray data;
	data.resize(psz * psz);
	data.fill(0);
	auto buffer = data.write();
	while (x > y) {
		const real_t ys = y * y;
		const real_t L = Math::sqrt(prs - ys);
		const real_t D = Math::ceil(L) - L;
		if (D < T) {
			x--;
		}
		for (int ax = y; ax < x; ax++) {
			putpixel8(ax, y, ifun(ax, ys, fall_off)); // fill wedge
		}
		putpixel8(x, y, (1 - D) * ifun(x, ys, fall_off)); // outer edge
		T = D;
		y++;
	}
	buffer.release();
	return data;
}

static Ref<Texture> generate_circle_aa_texture(const PoolByteArray &data) {
	Ref<Image> image = newref(Image);
	image->create(psz, psz, false, Image::FORMAT_A8, data);
	Ref<ImageTexture> texture = newref(ImageTexture);
	texture->create_from_image(image);
#ifdef DEBUG_ENABLED
	texture->set_name("aa32_circle");
#endif
	return texture;
}

static Ref<Texture> generate_squircle_aa_texture(const PoolByteArray &data) {
	PoolByteArray squircle;
	squircle.resize(data.size());
	squircle.fill(0);
	auto buffer = squircle.write();
	for (int y = 0; y < phf; y++) {
		for (int x = 0; x < phf; x++) {
			const Point2i index = disc_to_square<psz>(x, y);
			const uint8_t c = data[(pct + index.x) + (pct + index.y) * psz];
			putpixel4(x, y, c);
		}
	}
	buffer.release();
	Ref<Image> image = newref(Image);
	image->create(psz, psz, false, Image::FORMAT_A8, squircle);
	Ref<ImageTexture> texture = newref(ImageTexture);
	texture->create_from_image(image);
#ifdef DEBUG_ENABLED
	texture->set_name("aa32_square");
#endif
	return texture;
}

static Ref<Texture> generate_atlas_texture(const uint8_t *data, size_t data_size) {
	Ref<Image> image = newref(Image, data, data_size);
	image->fix_alpha_edges(); // cleanup alpha borders
	Ref<ImageTexture> texture = newref(ImageTexture);
	texture->create_from_image(image, Texture::FLAG_FILTER); // only FILTER, no REPEAT, MIPMAPS
	return texture;
}

void init_bitmap_symbol(real_t fall_off) {
	PoolByteArray data = generate_circle_aa_data(fall_off, false);
	PoolByteArray outer = generate_circle_aa_data(1.25, false);
	PoolByteArray glow = generate_circle_aa_data(0.7, true);
	if (!_CACHE_HAS("aa32_square")) {
		_CACHE_ADD("aa32_square", generate_squircle_aa_texture(data));
	}
	if (!_CACHE_HAS("aa32_circle")) {
		_CACHE_ADD("aa32_circle", generate_circle_aa_texture(data));
	}
	if (!_CACHE_HAS("aa32_circle_glow")) {
		_CACHE_ADD("aa32_circle_glow", generate_circle_aa_texture(glow));
	}
	if (!_CACHE_HAS("aa32_square_glow")) {
		_CACHE_ADD("aa32_square_glow", generate_squircle_aa_texture(glow));
	}
	if (!_CACHE_HAS("aa32_outer_glow")) {
		_CACHE_ADD("aa32_outer_glow", generate_squircle_aa_texture(outer));
	}
	if (!_CACHE_HAS("light_texture")) {
		Ref<Texture> lights = generate_atlas_texture(lights_png, lights_png_size);
		Ref<AtlasTexture> light_on = newref(AtlasTexture);
		light_on->set_atlas(lights);
		light_on->set_region(light_on_rect);
#ifdef DEBUG_ENABLED
		light_on->set_name("light_on");
#endif
		Ref<AtlasTexture> light_off = newref(AtlasTexture);
		light_off->set_atlas(lights);
		light_off->set_region(light_off_rect);
#ifdef DEBUG_ENABLED
		light_off->set_name("light_off");
#endif
		_CACHE_ADD("light_texture", lights);
		_CACHE_ADD("light_texture_on", light_on);
		_CACHE_ADD("light_texture_off", light_off);
	}
	if (!_CACHE_HAS("square_texture")) {
		Ref<Texture> squares = generate_atlas_texture(squares_png, squares_png_size);
		Ref<AtlasTexture> square_on = newref(AtlasTexture);
		square_on->set_atlas(squares);
		square_on->set_region(square_on_rect);
#ifdef DEBUG_ENABLED
		square_on->set_name("square_on");
#endif
		Ref<AtlasTexture> square_off = newref(AtlasTexture);
		square_off->set_atlas(squares);
		square_off->set_region(square_off_rect);
#ifdef DEBUG_ENABLED
		square_off->set_name("square_off");
#endif
		_CACHE_ADD("square_texture", squares);
		_CACHE_ADD("square_texture_on", square_on);
		_CACHE_ADD("square_texture_off", square_off);
	}
}

Ref<Texture> create_aa_circle_texture(real_t fall_off, bool invert) {
	return generate_circle_aa_texture(generate_circle_aa_data(fall_off, invert));
}

Ref<Texture> create_aa_squircle_texture(real_t fall_off, bool invert) {
	return generate_squircle_aa_texture(generate_circle_aa_data(fall_off, invert));
}

void draw_padding_with_dot_style(
		const RID &canvas_item,
		FBFontDotStyle dot_style,
		Point2 start_point,
		Color color,
		int edge_length,
		int margin,
		const Size2i &character,
		const CharPadding &padding) {
	const int l = edge_length + margin;

	Ref<Texture> texture;
	real_t circle_squared = 0;
	switch (dot_style) {
		case FBFontDotStyleFlatSquare: {
			if (edge_length > 5) {
				dot_style = FBFontDotStyleFlatCircle;
				edge_length /= 2;
				circle_squared = default_circle_squared;
			}
		} break;
		case FBFontDotStyleFlatCircle: {
			edge_length /= 2;
		} break;
		case FBFontDotStyleTextureSquare: {
			texture = _CACHE_GET("aa32_square");
		} break;
		case FBFontDotStyleTextureCircle: {
			texture = _CACHE_GET("aa32_circle");
		} break;
		case FBFontDotStyleTexture3D_1: {
			texture = _CACHE_GET("light_texture_off");
		} break;
		case FBFontDotStyleTexture3D_2: {
			texture = _CACHE_GET("square_texture_off");
		} break;
		default: {
			// not a texture style
		}
	}

	const Size2 rc = Size2(edge_length, edge_length);
	const Rect2i character_rc({ padding.number_of_left_dot, padding.number_of_top_dot }, character);
	const int horizontal_amount = character.width + padding.number_of_left_dot + padding.number_of_right_dot + padding.number_of_between_dot;
	const int vertical_amount = character.height + padding.number_of_top_dot + padding.number_of_bottom_dot;

	for (int i = 0; i < vertical_amount; i++) {
		for (int j = 0; j < horizontal_amount; j++) {
			if (!character_rc.has_point({ j, i })) {
				const Point2 xy = start_point + Point2(j, i) * l;
				switch (dot_style) {
					case FBFontDotStyleFlatSquare: {
						VS::get_singleton()->canvas_item_add_rect(canvas_item, { xy, rc }, color);
					} break;
					case FBFontDotStyleFlatCircle: {
						VS::get_singleton()->canvas_item_add_circle(canvas_item, xy + rc, edge_length, color, circle_squared);
					} break;
					default: {
						if (texture) {
							texture->draw_rect(canvas_item, { xy, rc }, false, color);
						}
					} break;
				}
			}
		}
	}
}

void draw_background_with_dot_style(
		const RID &canvas_item,
		FBFontDotStyle dot_style,
		Point2 start_point,
		Color color,
		int edge_length,
		int margin,
		int horizontal_amount,
		int vertical_amount) {
	const int l = edge_length + margin;

	Ref<Texture> texture;
	real_t circle_squared = 0;
	switch (dot_style) {
		case FBFontDotStyleFlatSquare: {
			if (edge_length > 5) {
				dot_style = FBFontDotStyleFlatCircle;
				edge_length /= 2;
				circle_squared = default_circle_squared;
			}
		} break;
		case FBFontDotStyleFlatCircle: {
			edge_length /= 2;
		} break;
		case FBFontDotStyleTextureSquare: {
			texture = _CACHE_GET("aa32_square");
		} break;
		case FBFontDotStyleTextureCircle: {
			texture = _CACHE_GET("aa32_circle");
		} break;
		case FBFontDotStyleTexture3D_1: {
			texture = _CACHE_GET("light_texture_off");
		} break;
		case FBFontDotStyleTexture3D_2: {
			texture = _CACHE_GET("square_texture_off");
		} break;
		default: {
			// not a texture style
		}
	}

	const Size2 rc = Size2(edge_length, edge_length);

	for (int i = 0; i < vertical_amount; i++) {
		for (int j = 0; j < horizontal_amount; j++) {
			const Point2 xy = start_point + Point2(j, i) * l;
			switch (dot_style) {
				case FBFontDotStyleFlatSquare: {
					VS::get_singleton()->canvas_item_add_rect(canvas_item, { xy, rc }, color);
				} break;
				case FBFontDotStyleFlatCircle: {
					VS::get_singleton()->canvas_item_add_circle(canvas_item, xy + rc, edge_length, color, circle_squared);
				} break;
				default: {
					if (texture) {
						texture->draw_rect(canvas_item, { xy, rc }, false, color);
					}
				} break;
			}
		}
	}
}

void draw_bitmap_symbol(
		const RID &canvas_item,
		FBFontSymbolType symbol,
		FBFontDotStyle dot_style,
		Color color,
		int edge_length,
		int margin,
		const Color &glow_color,
		real_t glow_size,
		const Color &inner_glow_color,
		real_t inner_glow_size,
		const Point2 &start_point) {
	const int l = edge_length + margin;
	const auto &coord = coord_for_symbol(symbol);

	Ref<Texture> texture, texture_off;
	Size2 rc(edge_length, edge_length);
	real_t circle_squared = 0;
	switch (dot_style) {
		case FBFontDotStyleFlatSquare: {
			if (edge_length > 5) {
				dot_style = FBFontDotStyleFlatCircle;
				rc /= 2;
				edge_length /= 2;
				circle_squared = default_circle_squared;
			}
		} break;
		case FBFontDotStyleFlatCircle: {
			rc /= 2;
			edge_length /= 2;
		} break;
		case FBFontDotStyleTextureSquare: {
			texture = _CACHE_GET("aa32_square");
		} break;
		case FBFontDotStyleTextureCircle: {
			texture = _CACHE_GET("aa32_circle");
		} break;
		case FBFontDotStyleTexture3D_1: {
			texture = _CACHE_GET("light_texture_on");
			texture_off = _CACHE_GET("light_texture_off");
		} break;
		case FBFontDotStyleTexture3D_2: {
			texture = _CACHE_GET("square_texture_on");
			texture_off = _CACHE_GET("square_texture_off");
		} break;
		default: {
			// not a texture mode
		}
	}

	for (int r = 0; r < coord.size(); r++) {
		const auto &column = coord[r];
		for (int c = 0; c < column.size(); c++) {
			const Point2 xy = start_point + Vector2(c, r) * l;
			if (column[c]) {
				switch (dot_style) {
					case FBFontDotStyleFlatSquare: {
						VS::get_singleton()->canvas_item_add_rect(canvas_item, { xy, rc }, color);
					} break;
					case FBFontDotStyleFlatCircle: {
						VS::get_singleton()->canvas_item_add_circle(canvas_item, xy + rc, edge_length, color, circle_squared);
					} break;
					default: {
						if (texture) {
							texture->draw_rect(canvas_item, { xy, rc }, false, color);
						}
					}
				}
			} else {
				if (texture_off) {
					texture_off->draw_rect(canvas_item, { xy, rc }, false, color);
				}
			}
		}
	}
}

void draw_bitmap_symbol_with_padding(
		const RID &canvas_item,
		FBFontSymbolType symbol,
		FBFontDotStyle dot_style,
		Color color,
		Color off_color,
		int edge_length,
		int margin,
		const CharPadding &padding,
		const Color &inner_glow_color,
		const Color &glow_color,
		const Point2 &start_point) {
	const int l = edge_length + margin;
	const auto &coord = coord_for_symbol(symbol);

	Ref<Texture> texture, texture_glow, texture_off;
	Size2 rc(edge_length, edge_length), rc_edge = rc;
	real_t circle_squared = 0;
	switch (dot_style) {
		case FBFontDotStyleFlatSquare: {
			if (edge_length > 5) {
				dot_style = FBFontDotStyleFlatCircle;
				rc /= 2;
				edge_length /= 2;
				circle_squared = default_circle_squared;
			}
			texture_glow = _CACHE_GET("aa32_square_glow");
		} break;
		case FBFontDotStyleFlatCircle: {
			rc /= 2;
			edge_length /= 2;
			texture_glow = _CACHE_GET("aa32_circle_glow");
		} break;
		case FBFontDotStyleTextureSquare: {
			texture = _CACHE_GET("aa32_square");
			texture_glow = _CACHE_GET("aa32_square_glow");
		} break;
		case FBFontDotStyleTextureCircle: {
			texture = _CACHE_GET("aa32_circle");
			texture_glow = _CACHE_GET("aa32_circle_glow");
		} break;
		case FBFontDotStyleTexture3D_1: {
			texture = _CACHE_GET("light_texture_on");
			texture_off = _CACHE_GET("light_texture_off");
			texture_glow = _CACHE_GET("aa32_square_glow");
		} break;
		case FBFontDotStyleTexture3D_2: {
			texture = _CACHE_GET("square_texture_on");
			texture_off = _CACHE_GET("square_texture_off");
			texture_glow = _CACHE_GET("aa32_square_glow");
		} break;
		default: {
			// not a texture mode
		}
	}

	const Rect2i character_rc({ padding.number_of_left_dot, padding.number_of_top_dot }, { int(coord.front().size()), int(coord.size()) });
	const int horizontal_amount = character_rc.size.width + padding.number_of_left_dot + padding.number_of_right_dot + padding.number_of_between_dot;
	const int vertical_amount = character_rc.size.height + padding.number_of_top_dot + padding.number_of_bottom_dot;

	for (int r = 0; r < vertical_amount; r++) {
		for (int c = 0; c < horizontal_amount; c++) {
			const Point2 xy = start_point + Vector2i(c, r) * l;
			const bool active = character_rc.has_point({ c, r }) && coord[r - character_rc.position.y][c - character_rc.position.x];
			switch (dot_style) {
				case FBFontDotStyleFlatSquare: {
					VS::get_singleton()->canvas_item_add_rect(canvas_item, { xy, rc }, active ? color : off_color);
				} break;
				case FBFontDotStyleFlatCircle: {
					VS::get_singleton()->canvas_item_add_circle(canvas_item, xy + rc, edge_length, active ? color : off_color, circle_squared);
				} break;
				default: {
					if (texture_off) {
						if (active) {
							texture->draw_rect(canvas_item, { xy, rc }, false, color);
						} else {
							texture_off->draw_rect(canvas_item, { xy, rc }, false, off_color);
						}
					} else {
						if (texture) {
							if (active) {
								texture->draw_rect(canvas_item, { xy - rc, rc * 3 }, false, glow_color);
							}
							texture->draw_rect(canvas_item, { xy, rc }, false, active ? color : off_color);
						}
					}
				}
			}
			if (texture_glow && active) {
				texture_glow->draw_rect(canvas_item, { xy, rc_edge }, false, inner_glow_color);
			}
		}
	}
}

void draw_bitmap_map(
		const RID &canvas_item,
		std::vector<std::vector<bool>> map,
		FBFontDotStyle dot_style,
		Color color,
		Color off_color,
		unsigned edge_length,
		unsigned margin,
		const CharPadding &padding,
		const Color &inner_glow_color,
		const Color &glow_color,
		real_t glow_size,
		const Point2 &start_point) {
	const int l = edge_length + margin;

	Ref<Texture> texture, texture_off, texture_glow, texture_outer = _CACHE_GET("aa32_outer_glow");
	Size2 rc(edge_length, edge_length), margins(margin, margin), rc_edge = rc, rc_glow = 2 * rc;
	real_t circle_squared = 0;
	switch (dot_style) {
		case FBFontDotStyleFlatSquare: {
			if (edge_length > 5) {
				dot_style = FBFontDotStyleFlatCircle;
				rc /= 2;
				edge_length /= 2;
				circle_squared = default_circle_squared;
			}
			texture_glow = _CACHE_GET("aa32_square_glow");
		} break;
		case FBFontDotStyleFlatCircle: {
			rc /= 2;
			edge_length /= 2;
			texture_glow = _CACHE_GET("aa32_circle_glow");
		} break;
		case FBFontDotStyleTextureSquare: {
			texture = _CACHE_GET("aa32_square");
			texture_glow = _CACHE_GET("aa32_square_glow");
		} break;
		case FBFontDotStyleTextureCircle: {
			texture = _CACHE_GET("aa32_circle");
			texture_glow = _CACHE_GET("aa32_circle_glow");
		} break;
		case FBFontDotStyleTexture3D_1: {
			texture = _CACHE_GET("light_texture_on");
			texture_off = _CACHE_GET("light_texture_off");
			texture_glow = _CACHE_GET("aa32_circle_glow");
		} break;
		case FBFontDotStyleTexture3D_2: {
			texture = _CACHE_GET("square_texture_on");
			//texture_glow = _CACHE_GET("aa32_square_glow");
			texture_off = _CACHE_GET("square_texture_off");
		} break;
		default: {
			// not a texture mode
		}
	}

	const Size2 md = (rc_glow - rc_edge) / 2, mmd = md - margins / 2;
	const Size2 mt = (texture_outer ? Size2(1, 1) : texture_outer->get_size()) * (mmd / rc_glow);

	for (int s = 0; s < 3; s++) { // 0.back, 1.glow, 2.front
		for (int r = 0; r < map.size(); r++) {
			const bool last_row = r == map.size() - 1;
			const auto &column = map[r];
			for (int c = 0; c < column.size(); c++) {
				const bool last_col = c == column.size() - 1;
				const bool active = map[r][c];
				const bool back = (s == 0 && !active);
				const bool glow = (s == 1 && active);
				const bool front = (s == 2 && active);
				const Point2 xy = start_point + Vector2i(c, r) * l;
				if (glow) {
					if (texture_outer) {
						const bool nleft = c ? map[r][c - 1] : false,
								   nright = last_col ? false : map[r][c + 1],
								   ntop = r ? map[r - 1][c] : false,
								   nbottom = last_row ? false : map[r + 1][c];
						const Vector2 change_position(nleft, ntop);
						const Vector2 change_size(nleft + nright, ntop + nbottom);
						Rect2 dest(xy - md + change_position * mmd, rc_glow - change_size * mmd);
						Rect2 src(Point2() + change_position * mt, texture_outer->get_size() - change_size * mt);
						//VS::get_singleton()->canvas_item_add_rect(canvas_item, dest, (c+r)%2 ? Color::named("red").with_alpha(0.5) : Color::named("green").with_alpha(0.5));
						texture_outer->draw_rect_region(canvas_item, dest, src, glow_color);
					}
				} else {
					switch (dot_style) {
						case FBFontDotStyleFlatSquare: {
							if (back || front) {
								VS::get_singleton()->canvas_item_add_rect(canvas_item, { xy, rc }, active ? color : off_color);
							}
						} break;
						case FBFontDotStyleFlatCircle: {
							if (back || front) {
								VS::get_singleton()->canvas_item_add_circle(canvas_item, xy + rc, edge_length, active ? color : off_color, circle_squared);
							}
						} break;
						default: {
							if (texture_off) {
								if (active) {
									texture->draw_rect(canvas_item, { xy, rc }, false, color);
								} else {
									texture_off->draw_rect(canvas_item, { xy, rc }, false, off_color);
								}
							} else {
								if (back || front) {
									if (texture) {
										texture->draw_rect(canvas_item, { xy, rc }, false, active ? color : off_color);
									}
								}
							}
						}
					}
					if (texture_glow && front) {
						texture_glow->draw_rect(canvas_item, { xy, rc_edge }, false, inner_glow_color);
					}
				}
			}
		}
	}
}

int number_of_dots_wide_for_symbol(FBFontSymbolType symbol) {
	return coord_for_symbol(symbol).front().size();
}

/* clang-format off */
static std::vector<std::vector<bool>> coord_for_symbol(FBFontSymbolType symbol) {
	switch (symbol) {
		case FBFontSymbol0:
			return {
				{ 0, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 1, 1, 0 },
			};
		case FBFontSymbol1:
			return {
				{ 0, 0, 1, 0, 0 },
				{ 0, 1, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
			};
		case FBFontSymbol2:
			return {
				{ 0, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 0, 0, 0, 1 },
				{ 0, 0, 0, 1, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 1, 0, 0, 0 },
				{ 1, 1, 1, 1, 1 },
			};
		case FBFontSymbol3:
			return {
				{ 0, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 0, 0, 0, 1 },
				{ 0, 0, 1, 1, 0 },
				{ 0, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 1, 1, 0 },
			};
		case FBFontSymbol4:
			return {
				{ 0, 0, 0, 1, 0 },
				{ 0, 0, 1, 1, 0 },
				{ 0, 1, 0, 1, 0 },
				{ 1, 0, 0, 1, 0 },
				{ 1, 1, 1, 1, 1 },
				{ 0, 0, 0, 1, 0 },
				{ 0, 0, 0, 1, 0 },
			};
		case FBFontSymbol5:
			return {
				{ 1, 1, 1, 1, 1 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 1, 1, 1, 0 },
				{ 0, 0, 0, 0, 1 },
				{ 0, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 1, 1, 0 },
			};
		case FBFontSymbol6:
			return {
				{ 0, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 1, 1, 0 },
			};
		case FBFontSymbol7:
			return {
				{ 1, 1, 1, 1, 1 },
				{ 0, 0, 0, 0, 1 },
				{ 0, 0, 0, 1, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 1, 0, 0, 0 },
				{ 0, 1, 0, 0, 0 },
				{ 0, 1, 0, 0, 0 },
			};
		case FBFontSymbol8:
			return {
				{ 0, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 1, 1, 0 },
			};
		case FBFontSymbol9:
			return {
				{ 0, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 1, 1, 1 },
				{ 0, 0, 0, 0, 1 },
				{ 0, 0, 0, 0, 1 },
				{ 0, 1, 1, 1, 0 },
			};
		case FBFontSymbolA:
			return {
				{ 0, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 1, 1, 1, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
			};
		case FBFontSymbolB:
			return {
				{ 1, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 1, 1, 1, 0 },
			};
		case FBFontSymbolC:
			return {
				{ 0, 1, 1, 1, 1 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 0, 1, 1, 1, 1 },
			};
		case FBFontSymbolD:
			return {
				{ 1, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 1, 1, 1, 0 },
			};
		case FBFontSymbolE:
			return {
				{ 1, 1, 1, 1, 1 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 1, 1, 1, 1 },
			};
		case FBFontSymbolF:
			return {
				{ 1, 1, 1, 1, 1 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
			};
		case FBFontSymbolG:
			return {
				{ 0, 1, 1, 1, 1 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 1, 1, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 1, 1, 0 },
			};
		case FBFontSymbolH:
			return {
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 1, 1, 1, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
			};
		case FBFontSymbolI:
			return {
				{ 1, 1, 1, 1, 1 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 1, 1, 1, 1, 1 },
			};
		case FBFontSymbolJ:
			return {
				{ 0, 0, 0, 1, 0 },
				{ 0, 0, 0, 1, 0 },
				{ 0, 0, 0, 1, 0 },
				{ 0, 0, 0, 1, 0 },
				{ 1, 0, 0, 1, 0 },
				{ 1, 0, 0, 1, 0 },
				{ 0, 1, 1, 0, 0 },
			};
		case FBFontSymbolK:
			return {
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 1, 0 },
				{ 1, 0, 1, 0, 0 },
				{ 1, 1, 0, 0, 0 },
				{ 1, 0, 1, 0, 0 },
				{ 1, 0, 0, 1, 0 },
				{ 1, 0, 0, 0, 1 },
			};
		case FBFontSymbolL:
			return {
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 1, 1, 1, 1 },
			};
		case FBFontSymbolM:
			return {
				{ 1, 0, 0, 0, 1 },
				{ 1, 1, 0, 1, 1 },
				{ 1, 0, 1, 0, 1 },
				{ 1, 0, 1, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
			};
		case FBFontSymbolN:
			return {
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 1, 0, 0, 1 },
				{ 1, 0, 1, 0, 1 },
				{ 1, 0, 0, 1, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
			};
		case FBFontSymbolO:
			return {
				{ 0, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 1, 1, 0 },
			};
		case FBFontSymbolP:
			return {
				{ 1, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
			};
		case FBFontSymbolQ:
			return {
				{ 0, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 1, 0, 1 },
				{ 1, 0, 0, 1, 1 },
				{ 0, 1, 1, 1, 1 },
			};
		case FBFontSymbolR:
			return {
				{ 1, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 1, 1, 1, 0 },
				{ 1, 0, 1, 0, 0 },
				{ 1, 0, 0, 1, 0 },
				{ 1, 0, 0, 0, 1 },
			};
		case FBFontSymbolS:
			return {
				{ 0, 1, 1, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 0 },
				{ 0, 1, 1, 1, 0 },
				{ 0, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 1, 1, 0 },
			};
		case FBFontSymbolT:
			return {
				{ 1, 1, 1, 1, 1 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
			};
		case FBFontSymbolU:
			return {
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 1, 1, 0 },
			};
		case FBFontSymbolV:
			return {
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 0, 1, 0 },
				{ 0, 0, 1, 0, 0 },
			};
		case FBFontSymbolW:
			return {
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 1, 0, 1 },
				{ 1, 0, 1, 0, 1 },
				{ 1, 1, 0, 1, 1 },
				{ 1, 0, 0, 0, 1 },
			};
		case FBFontSymbolX:
			return { { 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 0, 1, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 1, 0, 1, 0 },
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 } };
		case FBFontSymbolY:
			return {
				{ 1, 0, 0, 0, 1 },
				{ 1, 0, 0, 0, 1 },
				{ 0, 1, 0, 1, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
			};
		case FBFontSymbolZ:
			return {
				{ 1, 1, 1, 1, 1 },
				{ 0, 0, 0, 0, 1 },
				{ 0, 0, 0, 1, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 1, 0, 0, 0 },
				{ 1, 0, 0, 0, 0 },
				{ 1, 1, 1, 1, 1 },
			};
		case FBFontSymbolArrowUp:
			return {
				{ 0, 0, 0, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 1, 1, 1, 0 },
				{ 1, 0, 1, 0, 1 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 0, 0, 0 },
			};
		case FBFontSymbolArrowDown:
			return {
				{ 0, 0, 0, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 1, 0, 1, 0, 1 },
				{ 0, 1, 1, 1, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 0, 0, 0 },
			};
		case FBFontSymbolArrowLeft:
			return {
				{ 0, 0, 0, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 1, 0, 0, 0 },
				{ 1, 1, 1, 1, 1 },
				{ 0, 1, 0, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 0, 0, 0 },
			};
		case FBFontSymbolArrowRight:
			return {
				{ 0, 0, 0, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 0, 1, 0 },
				{ 1, 1, 1, 1, 1 },
				{ 0, 0, 0, 1, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 0, 0, 0 },
			};
		case FBFontSymbolDash:
			return {
				{ 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0 },
				{ 1, 1, 1, 1, 1 },
				{ 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0 },
			};
		case FBFontSymbolSpace:
			return {
				{ 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0 },
				{ 0, 0, 0, 0, 0 },
			};
		case FBFontSymbolExclamationMark:
			return {
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 1, 0, 0 },
				{ 0, 0, 0, 0, 0 },
				{ 0, 0, 1, 0, 0 },
			};
		case FBFontSymbolColon:
			return {
				{ 0, 0, 0 },
				{ 0, 1, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 0, 0 },
				{ 0, 1, 0 },
				{ 0, 0, 0 },
			};
		default:
			return std::vector<std::vector<bool>>();
	}
}
/* clang-format on */
