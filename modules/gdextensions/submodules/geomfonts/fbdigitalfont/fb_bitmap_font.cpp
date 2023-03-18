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

#define default_circle_squared 0.4

// 8-way symmetric macro
#define putpixel8(x, y, I)                                                                                       \
	{                                                                                                            \
		const uint8_t c = I * 255;                                                                               \
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

static PoolByteArray generate_circle_aa_data(real_t fall_off) {
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

static Ref<Texture> generate_light_texture() {
	Ref<Image> image = newref(Image, lights2_png, lights2_png_size);
	Ref<ImageTexture> texture = newref(ImageTexture);
	texture->create_from_image(image);
	return texture;
}

void init_bitmap_symbol(Dictionary &cache, real_t fall_off) {
	PoolByteArray data = generate_circle_aa_data(fall_off);
	if (!cache.has("aa32_square")) {
		cache["aa32_square"] = generate_squircle_aa_texture(data);
	}
	if (!cache.has("aa32_circle")) {
		cache["aa32_circle"] = generate_circle_aa_texture(data);
	}
	if (!cache.has("light_texture")) {
		Ref<Texture> lights = generate_light_texture();
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
		light_off->set_name("light_of");
#endif
		cache["light_texture"] = lights;
		cache["light_texture_on"] = light_on;
		cache["light_texture_off"] = light_off;
		cache["light_texture_off_scale"] = 0.6;
	}
}

void draw_padding_with_dot_style(
		const RID &canvas_item,
		Dictionary &cache,
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
			texture = cache["aa32_square"];
		} break;
		case FBFontDotStyleTextureCircle: {
			texture = cache["aa32_circle"];
		} break;
		case FBFontDotStyleTexture3D: {
			texture = cache["light_texture_off"];
			const real_t texture_off_scale = cache["light_texture_off_scale"];
			const real_t texture_offset = edge_length * (1 - texture_off_scale) / 2;
			start_point += Vector2(texture_offset, texture_offset);
			edge_length *= texture_off_scale;
			color = Color(1, 1, 1); // ignore color
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
		Dictionary &cache,
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
			texture = cache["aa32_square"];
		} break;
		case FBFontDotStyleTextureCircle: {
			texture = cache["aa32_circle"];
		} break;
		case FBFontDotStyleTexture3D: {
			texture = cache["light_texture_off"];
			const real_t texture_off_scale = cache["light_texture_off_scale"];
			const real_t texture_offset = edge_length * (1 - texture_off_scale) / 2;
			start_point += Vector2(texture_offset, texture_offset);
			edge_length *= texture_off_scale;
			color = Color(1, 1, 1); // ignore color
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
		Dictionary &cache,
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

	Ref<Texture> texture;
	struct {
		Ref<Texture> texture;
		Size2 rc;
		Vector2 start;
	} texture_off;
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
			texture = cache["aa32_square"];
		} break;
		case FBFontDotStyleTextureCircle: {
			texture = cache["aa32_circle"];
		} break;
		case FBFontDotStyleTexture3D: {
			texture = cache["light_texture_on"];
			texture_off.texture = cache["light_texture_off"];
			const real_t texture_off_scale = cache["light_texture_off_scale"];
			const real_t texture_offset = edge_length * (1 - texture_off_scale) / 2;
			texture_off.start = Vector2(texture_offset, texture_offset);
			texture_off.rc = rc * texture_off_scale;
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
				if (texture_off.texture) {
					texture_off.texture->draw_rect(canvas_item, { xy + texture_off.start, texture_off.rc }, false, color);
				}
			}
		}
	}
}

void draw_bitmap_symbol_with_padding(
		const RID &canvas_item,
		Dictionary &cache,
		FBFontSymbolType symbol,
		FBFontDotStyle dot_style,
		Color color,
		Color off_color,
		int edge_length,
		int margin,
		const CharPadding &padding,
		const Color &glow_color,
		real_t glow_size,
		const Color &inner_glow_color,
		real_t inner_glow_size,
		const Point2 &start_point) {
	const int l = edge_length + margin;
	const auto &coord = coord_for_symbol(symbol);

	Ref<Texture> texture;
	struct {
		Ref<Texture> texture;
		Size2 rc;
		Vector2 start;
	} texture_off;
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
			texture = cache["aa32_square"];
		} break;
		case FBFontDotStyleTextureCircle: {
			texture = cache["aa32_circle"];
		} break;
		case FBFontDotStyleTexture3D: {
			texture = cache["light_texture_on"];
			texture_off.texture = cache["light_texture_off"];
			const real_t texture_off_scale = cache["light_texture_off_scale"];
			const real_t texture_offset = edge_length * (1 - texture_off_scale) / 2;
			texture_off.start = Vector2(texture_offset, texture_offset);
			texture_off.rc = rc * texture_off_scale;
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
					if (texture_off.texture) {
						if (active) {
							texture->draw_rect(canvas_item, { xy, rc }, false, color);
						} else {
							texture_off.texture->draw_rect(canvas_item, { xy + texture_off.start, texture_off.rc }, false, off_color);
						}
					} else {
						if (texture) {
							texture->draw_rect(canvas_item, { xy, rc }, false, active ? color : off_color);
						}
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
