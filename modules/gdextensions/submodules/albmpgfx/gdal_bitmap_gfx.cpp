/**************************************************************************/
/*  gdal_bitmap_gfx.cpp                                                   */
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

#include "gdal_bitmap_gfx.h"

#include "core/math/math_defs.h"
#include "core/os/os.h"
#include "scene/resources/texture.h"

#include "al_gfx/al_gfx.h"

#include <cmath>

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

// ============================================================================
// GdAlRleSprite
// ============================================================================

GdAlRleSprite::GdAlRleSprite() :
		rle(nullptr) {}

GdAlRleSprite::~GdAlRleSprite() {
	if (rle) {
		::destroy_rle_sprite(rle);
		rle = nullptr;
	}
}

void GdAlRleSprite::_init_from_rle(RLE_SPRITE *p_rle) {
	if (rle) {
		::destroy_rle_sprite(rle);
	}
	rle = p_rle;
}

int GdAlRleSprite::get_width() const {
	return rle ? rle->w : 0;
}

int GdAlRleSprite::get_height() const {
	return rle ? rle->h : 0;
}

int GdAlRleSprite::get_color_depth() const {
	return rle ? rle->color_depth : 0;
}

int GdAlRleSprite::get_size() const {
	return rle ? rle->size : 0;
}

bool GdAlRleSprite::is_valid() const {
	return rle != nullptr;
}

void GdAlRleSprite::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_width"), &GdAlRleSprite::get_width);
	ClassDB::bind_method(D_METHOD("get_height"), &GdAlRleSprite::get_height);
	ClassDB::bind_method(D_METHOD("get_color_depth"), &GdAlRleSprite::get_color_depth);
	ClassDB::bind_method(D_METHOD("get_size"), &GdAlRleSprite::get_size);
	ClassDB::bind_method(D_METHOD("is_valid"), &GdAlRleSprite::is_valid);
}

// ============================================================================
// GdAlFont
// ============================================================================

GdAlFont::GdAlFont() :
		fnt(nullptr) {}

GdAlFont::~GdAlFont() {
	if (fnt) {
		::destroy_font(fnt);
		fnt = nullptr;
	}
}

void GdAlFont::_init_from_font(FONT *p_font) {
	if (fnt) {
		::destroy_font(fnt);
	}
	fnt = p_font;
}

int GdAlFont::get_height() const {
	return fnt ? ::text_height(fnt) : 0;
}

int GdAlFont::get_length(const String &str) const {
	if (!fnt)
		return 0;
	CharString cs = str.ascii();
	return ::text_length(fnt, cs.get_data());
}

bool GdAlFont::is_valid() const {
	return fnt != nullptr;
}

bool GdAlFont::is_mono() const {
	return fnt ? ::is_mono_font(fnt) : false;
}

bool GdAlFont::is_color() const {
	return fnt ? ::is_color_font(fnt) : false;
}

void GdAlFont::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_height"), &GdAlFont::get_height);
	ClassDB::bind_method(D_METHOD("get_length", "str"), &GdAlFont::get_length);
	ClassDB::bind_method(D_METHOD("is_valid"), &GdAlFont::is_valid);
	ClassDB::bind_method(D_METHOD("is_mono"), &GdAlFont::is_mono);
	ClassDB::bind_method(D_METHOD("is_color"), &GdAlFont::is_color);
}

// ============================================================================
// GdAlBitmapGfx
// ============================================================================

int GdAlBitmapGfx::_color_from_godot(const Color &c) const {
	ERR_FAIL_COND_V(!bmp, 0);
	int depth = bmp->vtable->color_depth;
	int r = CLAMP((int)(c.r * 255), 0, 255);
	int g = CLAMP((int)(c.g * 255), 0, 255);
	int b = CLAMP((int)(c.b * 255), 0, 255);
	return makecol_depth(depth, r, g, b);
}

Color GdAlBitmapGfx::_color_to_godot(int c) const {
	ERR_FAIL_COND_V(!bmp, Color());
	int depth = bmp->vtable->color_depth;
	float r = getr_depth(depth, c) / 255.0f;
	float g = getg_depth(depth, c) / 255.0f;
	float b = getb_depth(depth, c) / 255.0f;
	return Color(r, g, b, 1.0f);
}

int GdAlBitmapGfx::get_width() const {
	return bmp ? bmp->w : 0;
}

int GdAlBitmapGfx::get_height() const {
	return bmp ? bmp->h : 0;
}

int GdAlBitmapGfx::get_color_depth() const {
	return bmp ? bitmap_color_depth(bmp) : 0;
}

bool GdAlBitmapGfx::is_valid() const {
	return bmp != nullptr;
}

void GdAlBitmapGfx::clear(const Color &p_color) {
	ERR_FAIL_COND(!bmp);
	clear_to_color(bmp, _color_from_godot(p_color));
}

void GdAlBitmapGfx::putpixel(int x, int y, const Color &color) {
	ERR_FAIL_COND(!bmp);
	::putpixel(bmp, x, y, _color_from_godot(color));
}

Color GdAlBitmapGfx::getpixel(int x, int y) const {
	ERR_FAIL_COND_V(!bmp, Color());
	return _color_to_godot(::getpixel(bmp, x, y));
}

void GdAlBitmapGfx::hline(int x1, int y, int x2, const Color &color) {
	ERR_FAIL_COND(!bmp);
	_allegro_hline(bmp, x1, y, x2, _color_from_godot(color));
}

void GdAlBitmapGfx::vline(int x, int y1, int y2, const Color &color) {
	ERR_FAIL_COND(!bmp);
	_allegro_vline(bmp, x, y1, y2, _color_from_godot(color));
}

void GdAlBitmapGfx::line(int x1, int y1, int x2, int y2, const Color &color) {
	ERR_FAIL_COND(!bmp);
	::line(bmp, x1, y1, x2, y2, _color_from_godot(color));
}

void GdAlBitmapGfx::rect(int x1, int y1, int x2, int y2, const Color &color) {
	ERR_FAIL_COND(!bmp);
	::rect(bmp, x1, y1, x2, y2, _color_from_godot(color));
}

void GdAlBitmapGfx::rectfill(int x1, int y1, int x2, int y2, const Color &color) {
	ERR_FAIL_COND(!bmp);
	::rectfill(bmp, x1, y1, x2, y2, _color_from_godot(color));
}

void GdAlBitmapGfx::circle(int x, int y, int radius, const Color &color) {
	ERR_FAIL_COND(!bmp);
	::circle(bmp, x, y, radius, _color_from_godot(color));
}

void GdAlBitmapGfx::circlefill(int x, int y, int radius, const Color &color) {
	ERR_FAIL_COND(!bmp);
	::circlefill(bmp, x, y, radius, _color_from_godot(color));
}

void GdAlBitmapGfx::ellipse(int x, int y, int rx, int ry, const Color &color) {
	ERR_FAIL_COND(!bmp);
	::ellipse(bmp, x, y, rx, ry, _color_from_godot(color));
}

void GdAlBitmapGfx::ellipsefill(int x, int y, int rx, int ry, const Color &color) {
	ERR_FAIL_COND(!bmp);
	::ellipsefill(bmp, x, y, rx, ry, _color_from_godot(color));
}

void GdAlBitmapGfx::arc(const Vector2 &center, float ang1_deg, float ang2_deg, int radius, const Color &color) {
	ERR_FAIL_COND(!bmp);
	// Allegro uses 256 = full circle. Convert degrees: deg * 256 / 360
	fixed a1 = itofix((int)(ang1_deg * 256.0f / 360.0f));
	fixed a2 = itofix((int)(ang2_deg * 256.0f / 360.0f));
	::arc(bmp, (int)center.x, (int)center.y, a1, a2, radius, _color_from_godot(color));
}

void GdAlBitmapGfx::triangle(const Vector2 &v1, const Vector2 &v2, const Vector2 &v3, const Color &color) {
	ERR_FAIL_COND(!bmp);
	::triangle(bmp, (int)v1.x, (int)v1.y, (int)v2.x, (int)v2.y, (int)v3.x, (int)v3.y, _color_from_godot(color));
}

void GdAlBitmapGfx::polygon(const PoolVector2Array &vertices, const Color &color) {
	ERR_FAIL_COND(!bmp);
	int count = vertices.size();
	ERR_FAIL_COND(count < 3);

	Vector<int> points;
	points.resize(count * 2);
	PoolVector2Array::Read r = vertices.read();
	for (int i = 0; i < count; i++) {
		points.write[i * 2] = (int)r[i].x;
		points.write[i * 2 + 1] = (int)r[i].y;
	}
	::polygon(bmp, count, points.ptr(), _color_from_godot(color));
}

void GdAlBitmapGfx::floodfill(int x, int y, const Color &color) {
	ERR_FAIL_COND(!bmp);
	::floodfill(bmp, x, y, _color_from_godot(color));
}

void GdAlBitmapGfx::spline(const PoolVector2Array &points, const Color &color) {
	ERR_FAIL_COND(!bmp);
	ERR_FAIL_COND(points.size() != 4);

	int pts[8];
	PoolVector2Array::Read r = points.read();
	for (int i = 0; i < 4; i++) {
		pts[i * 2] = (int)r[i].x;
		pts[i * 2 + 1] = (int)r[i].y;
	}
	::spline(bmp, pts, _color_from_godot(color));
}

FONT *GdAlBitmapGfx::_active_font() const {
	if (current_font.is_valid() && current_font->_get_font()) {
		return current_font->_get_font();
	}
	return font; // built-in 8x8
}

void GdAlBitmapGfx::set_font(Ref<GdAlFont> p_font) {
	current_font = p_font;
}

Ref<GdAlFont> GdAlBitmapGfx::get_font() const {
	return current_font;
}

void GdAlBitmapGfx::text(const String &str, int x, int y, const Color &fg, const Color &bg) {
	ERR_FAIL_COND(!bmp);
	int bg_col = (bg.r < 0) ? -1 : _color_from_godot(bg);
	CharString cs = str.ascii();
	textout_ex(bmp, _active_font(), cs.get_data(), x, y, _color_from_godot(fg), bg_col);
}

void GdAlBitmapGfx::text_centered(const String &str, int x, int y, const Color &fg, const Color &bg) {
	ERR_FAIL_COND(!bmp);
	int bg_col = (bg.r < 0) ? -1 : _color_from_godot(bg);
	CharString cs = str.ascii();
	textout_centre_ex(bmp, _active_font(), cs.get_data(), x, y, _color_from_godot(fg), bg_col);
}

void GdAlBitmapGfx::text_right(const String &str, int x, int y, const Color &fg, const Color &bg) {
	ERR_FAIL_COND(!bmp);
	int bg_col = (bg.r < 0) ? -1 : _color_from_godot(bg);
	CharString cs = str.ascii();
	textout_right_ex(bmp, _active_font(), cs.get_data(), x, y, _color_from_godot(fg), bg_col);
}

int GdAlBitmapGfx::get_text_length(const String &str) const {
	ERR_FAIL_COND_V(!bmp, 0);
	CharString cs = str.ascii();
	return ::text_length(_active_font(), cs.get_data());
}

int GdAlBitmapGfx::get_text_height() const {
	return ::text_height(_active_font());
}

void GdAlBitmapGfx::blit_from(Ref<GdAlBitmapGfx> source, const Rect2 &src_rect, const Vector2 &dest_pos) {
	ERR_FAIL_COND(!bmp);
	ERR_FAIL_COND(source.is_null() || !source->bmp);
	::blit(source->bmp, bmp, (int)src_rect.position.x, (int)src_rect.position.y,
			(int)dest_pos.x, (int)dest_pos.y, (int)src_rect.size.x, (int)src_rect.size.y);
}

void GdAlBitmapGfx::stretch_blit_from(Ref<GdAlBitmapGfx> source, const Rect2 &src_rect, const Rect2 &dest_rect) {
	ERR_FAIL_COND(!bmp);
	ERR_FAIL_COND(source.is_null() || !source->bmp);
	::stretch_blit(source->bmp, bmp,
			(int)src_rect.position.x, (int)src_rect.position.y, (int)src_rect.size.x, (int)src_rect.size.y,
			(int)dest_rect.position.x, (int)dest_rect.position.y, (int)dest_rect.size.x, (int)dest_rect.size.y);
}

void GdAlBitmapGfx::masked_blit_from(Ref<GdAlBitmapGfx> source, const Rect2 &src_rect, const Vector2 &dest_pos) {
	ERR_FAIL_COND(!bmp);
	ERR_FAIL_COND(source.is_null() || !source->bmp);
	::masked_blit(source->bmp, bmp, (int)src_rect.position.x, (int)src_rect.position.y,
			(int)dest_pos.x, (int)dest_pos.y, (int)src_rect.size.x, (int)src_rect.size.y);
}

void GdAlBitmapGfx::draw_sprite(Ref<GdAlBitmapGfx> sprite, int x, int y) {
	ERR_FAIL_COND(!bmp);
	ERR_FAIL_COND(sprite.is_null() || !sprite->bmp);
	::draw_sprite(bmp, sprite->bmp, x, y);
}

void GdAlBitmapGfx::draw_sprite_h_flip(Ref<GdAlBitmapGfx> sprite, int x, int y) {
	ERR_FAIL_COND(!bmp);
	ERR_FAIL_COND(sprite.is_null() || !sprite->bmp);
	::draw_sprite_h_flip(bmp, sprite->bmp, x, y);
}

void GdAlBitmapGfx::draw_sprite_v_flip(Ref<GdAlBitmapGfx> sprite, int x, int y) {
	ERR_FAIL_COND(!bmp);
	ERR_FAIL_COND(sprite.is_null() || !sprite->bmp);
	::draw_sprite_v_flip(bmp, sprite->bmp, x, y);
}

void GdAlBitmapGfx::draw_sprite_vh_flip(Ref<GdAlBitmapGfx> sprite, int x, int y) {
	ERR_FAIL_COND(!bmp);
	ERR_FAIL_COND(sprite.is_null() || !sprite->bmp);
	::draw_sprite_vh_flip(bmp, sprite->bmp, x, y);
}

void GdAlBitmapGfx::draw_trans_sprite(Ref<GdAlBitmapGfx> sprite, int x, int y) {
	ERR_FAIL_COND(!bmp);
	ERR_FAIL_COND(sprite.is_null() || !sprite->bmp);
	::draw_trans_sprite(bmp, sprite->bmp, x, y);
}

void GdAlBitmapGfx::draw_lit_sprite(Ref<GdAlBitmapGfx> sprite, int x, int y, int color) {
	ERR_FAIL_COND(!bmp);
	ERR_FAIL_COND(sprite.is_null() || !sprite->bmp);
	::draw_lit_sprite(bmp, sprite->bmp, x, y, color);
}

Ref<GdAlRleSprite> GdAlBitmapGfx::get_rle_sprite() const {
	ERR_FAIL_COND_V(!bmp, Ref<GdAlRleSprite>());
	RLE_SPRITE *rle = ::get_rle_sprite(bmp);
	ERR_FAIL_COND_V(!rle, Ref<GdAlRleSprite>());
	Ref<GdAlRleSprite> result;
	result.instance();
	result->_init_from_rle(rle);
	return result;
}

void GdAlBitmapGfx::draw_rle_sprite(Ref<GdAlRleSprite> sprite, int x, int y) {
	ERR_FAIL_COND(!bmp);
	ERR_FAIL_COND(sprite.is_null() || !sprite->_get_rle());
	::draw_rle_sprite(bmp, sprite->_get_rle(), x, y);
}

void GdAlBitmapGfx::rotate_sprite(Ref<GdAlBitmapGfx> sprite, const Vector2 &pos, float angle_deg) {
	ERR_FAIL_COND(!bmp);
	ERR_FAIL_COND(sprite.is_null() || !sprite->bmp);
	fixed angle = ftofix(angle_deg * 256.0 / 360.0);
	::rotate_sprite(bmp, sprite->bmp, (int)pos.x, (int)pos.y, angle);
}

void GdAlBitmapGfx::rotate_scaled_sprite(Ref<GdAlBitmapGfx> sprite, const Vector2 &pos, float angle_deg, float scale) {
	ERR_FAIL_COND(!bmp);
	ERR_FAIL_COND(sprite.is_null() || !sprite->bmp);
	fixed angle = ftofix(angle_deg * 256.0 / 360.0);
	fixed fscale = ftofix(scale);
	::rotate_scaled_sprite(bmp, sprite->bmp, (int)pos.x, (int)pos.y, angle, fscale);
}

void GdAlBitmapGfx::pivot_sprite(Ref<GdAlBitmapGfx> sprite, const Vector2 &pos, const Vector2 &pivot, float angle_deg) {
	ERR_FAIL_COND(!bmp);
	ERR_FAIL_COND(sprite.is_null() || !sprite->bmp);
	fixed angle = ftofix(angle_deg * 256.0 / 360.0);
	::pivot_sprite(bmp, sprite->bmp, (int)pos.x, (int)pos.y, (int)pivot.x, (int)pivot.y, angle);
}

void GdAlBitmapGfx::pivot_sprite_v_flip(Ref<GdAlBitmapGfx> sprite, const Vector2 &pos, const Vector2 &pivot, float angle_deg) {
	ERR_FAIL_COND(!bmp);
	ERR_FAIL_COND(sprite.is_null() || !sprite->bmp);
	fixed angle = ftofix(angle_deg * 256.0 / 360.0);
	::pivot_sprite_v_flip(bmp, sprite->bmp, (int)pos.x, (int)pos.y, (int)pivot.x, (int)pivot.y, angle);
}

void GdAlBitmapGfx::pivot_scaled_sprite(Ref<GdAlBitmapGfx> sprite, const Vector2 &pos, const Vector2 &pivot, float angle_deg, float scale) {
	ERR_FAIL_COND(!bmp);
	ERR_FAIL_COND(sprite.is_null() || !sprite->bmp);
	fixed angle = ftofix(angle_deg * 256.0 / 360.0);
	fixed fscale = ftofix(scale);
	::pivot_scaled_sprite(bmp, sprite->bmp, (int)pos.x, (int)pos.y, (int)pivot.x, (int)pivot.y, angle, fscale);
}

void GdAlBitmapGfx::pivot_scaled_sprite_v_flip(Ref<GdAlBitmapGfx> sprite, const Vector2 &pos, const Vector2 &pivot, float angle_deg, float scale) {
	ERR_FAIL_COND(!bmp);
	ERR_FAIL_COND(sprite.is_null() || !sprite->bmp);
	fixed angle = ftofix(angle_deg * 256.0 / 360.0);
	fixed fscale = ftofix(scale);
	::pivot_scaled_sprite_v_flip(bmp, sprite->bmp, (int)pos.x, (int)pos.y, (int)pivot.x, (int)pivot.y, angle, fscale);
}

void GdAlBitmapGfx::set_drawing_mode(int mode, Ref<GdAlBitmapGfx> pattern, int x_anchor, int y_anchor) {
	BITMAP *pat = (pattern.is_valid() && pattern->bmp) ? pattern->bmp : nullptr;
	ERR_FAIL_COND(mode >= DRAW_MODE_COPY_PATTERN && mode <= DRAW_MODE_MASKED_PATTERN && !pat);
	::drawing_mode(mode, pat, x_anchor, y_anchor);
}

void GdAlBitmapGfx::solid_mode() {
	::solid_mode();
}

void GdAlBitmapGfx::set_xor_mode(bool enabled) {
	xor_mode(enabled ? TRUE : FALSE);
}

void GdAlBitmapGfx::set_trans_blender(int r, int g, int b, int a) {
	::set_trans_blender(r, g, b, a);
}

void GdAlBitmapGfx::set_add_blender(int r, int g, int b, int a) {
	::set_add_blender(r, g, b, a);
}

void GdAlBitmapGfx::set_alpha_blender() {
	::set_alpha_blender();
}

void GdAlBitmapGfx::set_clip_rect(int x1, int y1, int x2, int y2) {
	ERR_FAIL_COND(!bmp);
	::set_clip_rect(bmp, x1, y1, x2, y2);
}

Rect2 GdAlBitmapGfx::get_clip_rect() const {
	ERR_FAIL_COND_V(!bmp, Rect2());
	int x1, y1, x2, y2;
	::get_clip_rect(bmp, &x1, &y1, &x2, &y2);
	return Rect2((real_t)x1, (real_t)y1, (real_t)(x2 - x1), (real_t)(y2 - y1));
}

Ref<GdAlBitmapGfx> GdAlBitmapGfx::create_sub_bitmap(int x, int y, int w, int h) {
	ERR_FAIL_COND_V(!bmp, Ref<GdAlBitmapGfx>());
	BITMAP *sub = ::create_sub_bitmap(bmp, x, y, w, h);
	ERR_FAIL_COND_V(!sub, Ref<GdAlBitmapGfx>());

	Ref<GdAlBitmapGfx> result;
	result.instance();
	result->_init_from_bitmap(sub, true);
	return result;
}

Ref<Image> GdAlBitmapGfx::get_image() const {
	ERR_FAIL_COND_V(!bmp, Ref<Image>());

	int w = bmp->w;
	int h = bmp->h;
	int depth = bitmap_color_depth(bmp);

	PoolByteArray data;
	data.resize(w * h * 4);
	PoolByteArray::Write wd = data.write();

	if (depth == 32 && _rgb_r_shift_32 == 0 && _rgb_g_shift_32 == 8 &&
			_rgb_b_shift_32 == 16 && _rgb_a_shift_32 == 24) {
		// Fast path: pixel layout matches RGBA8 byte order — direct scanline copy
		for (int y = 0; y < h; y++) {
			memcpy(&wd[y * w * 4], bmp->line[y], w * 4);
		}
	} else {
		// General path: per-scanline with depth-specific unpacking
		for (int y = 0; y < h; y++) {
			unsigned char *sl = bmp->line[y];
			uint8_t *dst = &wd[y * w * 4];
			for (int x = 0; x < w; x++) {
				int c;
				switch (depth) {
					case 32:
						c = ((int *)sl)[x];
						dst[0] = getr32(c);
						dst[1] = getg32(c);
						dst[2] = getb32(c);
						dst[3] = geta32(c);
						break;
					case 24:
						c = ((sl[x * 3 + 2] << 16) | (sl[x * 3 + 1] << 8) | sl[x * 3]);
						dst[0] = getr24(c);
						dst[1] = getg24(c);
						dst[2] = getb24(c);
						dst[3] = 255;
						break;
					case 16:
						c = ((unsigned short *)sl)[x];
						dst[0] = getr16(c);
						dst[1] = getg16(c);
						dst[2] = getb16(c);
						dst[3] = 255;
						break;
					case 15:
						c = ((unsigned short *)sl)[x];
						dst[0] = getr15(c);
						dst[1] = getg15(c);
						dst[2] = getb15(c);
						dst[3] = 255;
						break;
					default: // 8-bit fallback via vtable
						c = ::getpixel(bmp, x, y);
						dst[0] = getr_depth(depth, c);
						dst[1] = getg_depth(depth, c);
						dst[2] = getb_depth(depth, c);
						dst[3] = 255;
						break;
				}
				dst += 4;
			}
		}
	}

	Ref<Image> img;
	img.instance();
	img->create(w, h, false, Image::FORMAT_RGBA8, data);
	return img;
}

void GdAlBitmapGfx::triangle3d_flat(const Vector3 &v1, const Vector3 &v2, const Vector3 &v3, const Color &color) {
	ERR_FAIL_COND(!bmp);
	int c = _color_from_godot(color);
	int type = POLYTYPE_FLAT;
	if (zbuf) {
		type |= POLYTYPE_ZBUF;
		::set_zbuffer(zbuf);
	}

	V3D_f a, b, d;
	a.x = v1.x;
	a.y = v1.y;
	a.z = v1.z;
	a.u = 0;
	a.v = 0;
	a.c = c;
	b.x = v2.x;
	b.y = v2.y;
	b.z = v2.z;
	b.u = 0;
	b.v = 0;
	b.c = c;
	d.x = v3.x;
	d.y = v3.y;
	d.z = v3.z;
	d.u = 0;
	d.v = 0;
	d.c = c;
	triangle3d_f(bmp, type, nullptr, &a, &b, &d);
}

void GdAlBitmapGfx::triangle3d_gouraud(const PoolVector3Array &vertices, const PoolColorArray &colors) {
	ERR_FAIL_COND(!bmp);
	ERR_FAIL_COND(vertices.size() != 3);
	ERR_FAIL_COND(colors.size() != 3);
	int type = POLYTYPE_GCOL;
	if (zbuf) {
		type |= POLYTYPE_ZBUF;
		::set_zbuffer(zbuf);
	}

	PoolVector3Array::Read vr = vertices.read();
	PoolColorArray::Read cr = colors.read();

	V3D_f a, b, d;
	a.x = vr[0].x;
	a.y = vr[0].y;
	a.z = vr[0].z;
	a.u = 0;
	a.v = 0;
	a.c = _color_from_godot(cr[0]);
	b.x = vr[1].x;
	b.y = vr[1].y;
	b.z = vr[1].z;
	b.u = 0;
	b.v = 0;
	b.c = _color_from_godot(cr[1]);
	d.x = vr[2].x;
	d.y = vr[2].y;
	d.z = vr[2].z;
	d.u = 0;
	d.v = 0;
	d.c = _color_from_godot(cr[2]);
	triangle3d_f(bmp, type, nullptr, &a, &b, &d);
}

void GdAlBitmapGfx::triangle3d(int polytype, const PoolVector3Array &vertices, const PoolColorArray &colors,
		Ref<GdAlBitmapGfx> texture) {
	ERR_FAIL_COND(!bmp);
	ERR_FAIL_COND(vertices.size() != 3);
	ERR_FAIL_COND(colors.size() != 3);
	int type = CLAMP(polytype, 0, POLYTYPE_MAX - 1);
	if (zbuf) {
		type |= POLYTYPE_ZBUF;
		::set_zbuffer(zbuf);
	}

	BITMAP *tex = texture.is_valid() ? texture->_get_bitmap() : nullptr;

	PoolVector3Array::Read vr = vertices.read();
	PoolColorArray::Read cr = colors.read();

	V3D_f a, b, d;
	a.x = vr[0].x;
	a.y = vr[0].y;
	a.z = vr[0].z;
	a.u = 0;
	a.v = 0;
	a.c = _color_from_godot(cr[0]);
	b.x = vr[1].x;
	b.y = vr[1].y;
	b.z = vr[1].z;
	b.u = 0;
	b.v = 0;
	b.c = _color_from_godot(cr[1]);
	d.x = vr[2].x;
	d.y = vr[2].y;
	d.z = vr[2].z;
	d.u = 0;
	d.v = 0;
	d.c = _color_from_godot(cr[2]);
	triangle3d_f(bmp, type, tex, &a, &b, &d);
}

void GdAlBitmapGfx::quad3d_flat(const PoolVector3Array &vertices, const Color &color) {
	ERR_FAIL_COND(!bmp);
	ERR_FAIL_COND(vertices.size() != 4);
	int c = _color_from_godot(color);
	int type = POLYTYPE_FLAT;
	if (zbuf) {
		type |= POLYTYPE_ZBUF;
		::set_zbuffer(zbuf);
	}

	PoolVector3Array::Read vr = vertices.read();

	V3D_f a, b, d, e;
	a.x = vr[0].x;
	a.y = vr[0].y;
	a.z = vr[0].z;
	a.u = 0;
	a.v = 0;
	a.c = c;
	b.x = vr[1].x;
	b.y = vr[1].y;
	b.z = vr[1].z;
	b.u = 0;
	b.v = 0;
	b.c = c;
	d.x = vr[2].x;
	d.y = vr[2].y;
	d.z = vr[2].z;
	d.u = 0;
	d.v = 0;
	d.c = c;
	e.x = vr[3].x;
	e.y = vr[3].y;
	e.z = vr[3].z;
	e.u = 0;
	e.v = 0;
	e.c = c;
	quad3d_f(bmp, type, nullptr, &a, &b, &d, &e);
}

void GdAlBitmapGfx::quad3d_gouraud(const PoolVector3Array &vertices, const PoolColorArray &colors) {
	ERR_FAIL_COND(!bmp);
	ERR_FAIL_COND(vertices.size() != 4);
	ERR_FAIL_COND(colors.size() != 4);
	int type = POLYTYPE_GCOL;
	if (zbuf) {
		type |= POLYTYPE_ZBUF;
		::set_zbuffer(zbuf);
	}

	PoolVector3Array::Read vr = vertices.read();
	PoolColorArray::Read cr = colors.read();

	V3D_f a, b, d, e;
	a.x = vr[0].x;
	a.y = vr[0].y;
	a.z = vr[0].z;
	a.u = 0;
	a.v = 0;
	a.c = _color_from_godot(cr[0]);
	b.x = vr[1].x;
	b.y = vr[1].y;
	b.z = vr[1].z;
	b.u = 0;
	b.v = 0;
	b.c = _color_from_godot(cr[1]);
	d.x = vr[2].x;
	d.y = vr[2].y;
	d.z = vr[2].z;
	d.u = 0;
	d.v = 0;
	d.c = _color_from_godot(cr[2]);
	e.x = vr[3].x;
	e.y = vr[3].y;
	e.z = vr[3].z;
	e.u = 0;
	e.v = 0;
	e.c = _color_from_godot(cr[3]);
	quad3d_f(bmp, type, nullptr, &a, &b, &d, &e);
}

// Z-buffer management

void GdAlBitmapGfx::create_zbuffer() {
	ERR_FAIL_COND(!bmp);
	if (zbuf) {
		::destroy_zbuffer(zbuf);
	}
	zbuf = ::create_zbuffer(bmp);
	ERR_FAIL_COND_MSG(!zbuf, "Failed to create Z-buffer");
	::set_zbuffer(zbuf);
}

void GdAlBitmapGfx::clear_zbuffer(float z) {
	ERR_FAIL_COND(!zbuf);
	::clear_zbuffer(zbuf, z);
}

void GdAlBitmapGfx::enable_zbuffer() {
	ERR_FAIL_COND(!zbuf);
	::set_zbuffer(zbuf);
}

void GdAlBitmapGfx::disable_zbuffer() {
	// Set global _zbuffer to NULL — disables Z-buffer testing in polygon rasterizer.
	// Can't use set_zbuffer(NULL) because it has ASSERT(zbuf).
	_zbuffer = nullptr;
}

bool GdAlBitmapGfx::has_zbuffer() const {
	return zbuf != nullptr;
}

void GdAlBitmapGfx::destroy_zbuffer() {
	if (zbuf) {
		::destroy_zbuffer(zbuf);
		zbuf = nullptr;
	}
}

// ============================================================================
// Demo rendering — reimplementation of demo.c as a method
// ============================================================================

// Demo grid configuration
#define DEMO_COLS 4
#define DEMO_ROWS 4
#define DEMO_CELL_W 200
#define DEMO_CELL_H 150
#define DEMO_PADDING 2
#define DEMO_SCREEN_W (DEMO_COLS * DEMO_CELL_W)
#define DEMO_SCREEN_H (DEMO_ROWS * DEMO_CELL_H + 30)

#define DEMO_CELL_X(col) ((col)*DEMO_CELL_W)
#define DEMO_CELL_Y(row) ((row)*DEMO_CELL_H + 30)

static void _demo_draw_cell(BITMAP *b, int col, int row, const char *label) {
	int x = DEMO_CELL_X(col);
	int y = DEMO_CELL_Y(row);
	int border = makecol(80, 80, 80);
	int bg = makecol(20, 20, 30);
	::rectfill(b, x + DEMO_PADDING, y + DEMO_PADDING,
			x + DEMO_CELL_W - DEMO_PADDING - 1, y + DEMO_CELL_H - DEMO_PADDING - 1, bg);
	::rect(b, x + DEMO_PADDING, y + DEMO_PADDING,
			x + DEMO_CELL_W - DEMO_PADDING - 1, y + DEMO_CELL_H - DEMO_PADDING - 1, border);
	textout_ex(b, font, label, x + 5, y + 5, makecol(255, 255, 0), -1);
}

static void _demo_primitives(BITMAP *b, int col, int row) {
	_demo_draw_cell(b, col, row, "Primitives");
	int x = DEMO_CELL_X(col);
	int y = DEMO_CELL_Y(row);
	int cx = x + DEMO_CELL_W / 2;
	int cy = y + DEMO_CELL_H / 2;

	srand(42);
	for (int i = 0; i < 100; i++) {
		int px = x + 10 + (rand() % 80);
		int py = y + 20 + (rand() % 100);
		int c = makecol(rand() % 256, rand() % 256, rand() % 256);
		::putpixel(b, px, py, c);
	}
	for (int i = 0; i < 16; i++) {
		float angle = i * AL_PI / 8.0f;
		int x2 = cx + 50 + (int)(60 * cosf(angle));
		int y2 = cy + (int)(50 * sinf(angle));
		int c = makecol(128 + i * 8, 64, 255 - i * 8);
		::line(b, cx + 50, cy, x2, y2, c);
	}
	_allegro_hline(b, x + 100, y + DEMO_CELL_H - 20, x + DEMO_CELL_W - 10, makecol(0, 255, 0));
	_allegro_vline(b, x + DEMO_CELL_W - 20, y + 20, y + DEMO_CELL_H - 10, makecol(0, 255, 255));
}

static void _demo_rectangles(BITMAP *b, int col, int row) {
	_demo_draw_cell(b, col, row, "Rectangles");
	int x = DEMO_CELL_X(col);
	int y = DEMO_CELL_Y(row);

	for (int i = 0; i < 5; i++) {
		int c = makecol(50 + i * 40, 100, 200 - i * 30);
		::rect(b, x + 10 + i * 8, y + 25 + i * 8, x + 90 - i * 8, y + 105 - i * 8, c);
	}
	for (int i = 0; i < 8; i++) {
		int c = makecol(255 - i * 20, i * 30, 128);
		::rectfill(b, x + 100 + i * 10, y + 30 + i * 10, x + 120 + i * 10, y + 50 + i * 10, c);
	}
}

static void _demo_circles(BITMAP *b, int col, int row) {
	_demo_draw_cell(b, col, row, "Circles/Ellipses");
	int x = DEMO_CELL_X(col);
	int y = DEMO_CELL_Y(row);
	int cx = x + 50;
	int cy = y + 75;

	for (int r = 5; r <= 40; r += 5) {
		int c = makecol(r * 6, 255 - r * 5, 128);
		::circle(b, cx, cy, r, c);
	}
	::circlefill(b, x + 120, y + 50, 25, makecol(200, 100, 50));
	::circle(b, x + 120, y + 50, 25, makecol(255, 200, 100));
	::ellipse(b, x + 150, y + 100, 40, 20, makecol(100, 200, 255));
	::ellipsefill(b, x + 120, y + 110, 25, 15, makecol(150, 50, 200));
}

static void _demo_polygons(BITMAP *b, int col, int row) {
	_demo_draw_cell(b, col, row, "Triangles/Polygons");
	int x = DEMO_CELL_X(col);
	int y = DEMO_CELL_Y(row);

	::triangle(b, x + 30, y + 110, x + 70, y + 30, x + 110, y + 110, makecol(255, 100, 100));

	int pentagon[10];
	int pcx = x + 155, pcy = y + 70;
	for (int i = 0; i < 5; i++) {
		float angle = -AL_PI / 2 + i * 2 * AL_PI / 5;
		pentagon[i * 2] = pcx + (int)(35 * cosf(angle));
		pentagon[i * 2 + 1] = pcy + (int)(35 * sinf(angle));
	}
	::polygon(b, 5, pentagon, makecol(100, 200, 100));

	int tx = x + 60, ty = y + 65;
	::line(b, tx, ty - 20, tx - 25, ty + 20, makecol(255, 255, 0));
	::line(b, tx - 25, ty + 20, tx + 25, ty + 20, makecol(255, 255, 0));
	::line(b, tx + 25, ty + 20, tx, ty - 20, makecol(255, 255, 0));
}

static void _demo_arcs(BITMAP *b, int col, int row) {
	_demo_draw_cell(b, col, row, "Arcs/Splines");
	int x = DEMO_CELL_X(col);
	int y = DEMO_CELL_Y(row);

	::arc(b, x + 50, y + 70, itofix(0), itofix(64), 30, makecol(255, 100, 100));
	::arc(b, x + 50, y + 70, itofix(64), itofix(128), 35, makecol(100, 255, 100));
	::arc(b, x + 50, y + 70, itofix(128), itofix(192), 40, makecol(100, 100, 255));
	::arc(b, x + 50, y + 70, itofix(192), itofix(256), 45, makecol(255, 255, 100));

	int points[8] = {
		x + 100, y + 100,
		x + 120, y + 30,
		x + 160, y + 120,
		x + 185, y + 50
	};
	::spline(b, points, makecol(255, 128, 255));
	for (int i = 0; i < 4; i++) {
		::circlefill(b, points[i * 2], points[i * 2 + 1], 3, makecol(255, 255, 255));
	}
}

static void _demo_text(BITMAP *b, int col, int row) {
	_demo_draw_cell(b, col, row, "Text Rendering");
	int x = DEMO_CELL_X(col);
	int y = DEMO_CELL_Y(row);

	textout_ex(b, font, "Left aligned", x + 10, y + 30, makecol(255, 255, 255), -1);
	textout_centre_ex(b, font, "Centered", x + DEMO_CELL_W / 2, y + 50, makecol(255, 200, 100), -1);
	textout_right_ex(b, font, "Right aligned", x + DEMO_CELL_W - 10, y + 70, makecol(100, 200, 255), -1);
	textout_ex(b, font, "With BG", x + 10, y + 90, makecol(0, 0, 0), makecol(200, 200, 0));
	textprintf_ex(b, font, x + 10, y + 110, makecol(200, 255, 200), -1, "Value: %d", 42);
	textprintf_ex(b, font, x + 10, y + 125, makecol(200, 200, 255), -1, "Pi: %.2f", 3.14159);
}

static void _demo_blitting(BITMAP *b, int col, int row) {
	_demo_draw_cell(b, col, row, "Blit/Sprites");
	int x = DEMO_CELL_X(col);
	int y = DEMO_CELL_Y(row);

	BITMAP *sprite = create_bitmap(24, 24);
	clear_to_color(sprite, makecol(255, 0, 255));
	::circlefill(sprite, 12, 12, 10, makecol(255, 220, 180));
	::circle(sprite, 12, 12, 10, makecol(0, 0, 0));
	::putpixel(sprite, 8, 9, makecol(0, 0, 0));
	::putpixel(sprite, 16, 9, makecol(0, 0, 0));
	::arc(sprite, 12, 14, itofix(64), itofix(192), 4, makecol(200, 50, 50));

	::blit(sprite, b, 0, 0, x + 20, y + 40, 24, 24);
	textout_ex(b, font, "blit", x + 15, y + 70, makecol(180, 180, 180), -1);

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			::blit(sprite, b, 0, 0, x + 55 + i * 26, y + 35 + j * 26, 24, 24);
		}
	}
	textout_ex(b, font, "tiled", x + 65, y + 95, makecol(180, 180, 180), -1);

	::draw_sprite(b, sprite, x + 140, y + 40);
	::draw_sprite_h_flip(b, sprite, x + 166, y + 40);
	::draw_sprite_v_flip(b, sprite, x + 140, y + 66);
	::draw_sprite_vh_flip(b, sprite, x + 166, y + 66);
	textout_ex(b, font, "flips", x + 145, y + 100, makecol(180, 180, 180), -1);

	destroy_bitmap(sprite);
}

static void _demo_rotation(BITMAP *b, int col, int row) {
	_demo_draw_cell(b, col, row, "Rotate/Scale");
	int x = DEMO_CELL_X(col);
	int y = DEMO_CELL_Y(row);

	BITMAP *arrow = create_bitmap(20, 20);
	clear_to_color(arrow, makecol(255, 0, 255));
	::triangle(arrow, 10, 2, 2, 17, 18, 17, makecol(0, 200, 255));
	::line(arrow, 10, 2, 10, 17, makecol(255, 255, 255));

	for (int i = 0; i < 8; i++) {
		fixed angle = itofix(i * 32);
		int px = x + 30 + (i % 4) * 40;
		int py = y + 45 + (i / 4) * 50;
		::rotate_sprite(b, arrow, px, py, angle);
	}
	::rotate_scaled_sprite(b, arrow, x + 140, y + 80, itofix(32), ftofix(2.0));

	destroy_bitmap(arrow);
}

static void _demo_gradients(BITMAP *b, int col, int row) {
	_demo_draw_cell(b, col, row, "Color Gradients");
	int x = DEMO_CELL_X(col);
	int y = DEMO_CELL_Y(row);

	for (int i = 0; i < 180; i++) {
		int r = i * 255 / 180;
		int g = 128;
		int bl = 255 - i * 255 / 180;
		_allegro_vline(b, x + 10 + i, y + 25, y + 55, makecol(r, g, bl));
	}
	for (int i = 0; i < 60; i++) {
		int r = 255;
		int g = i * 255 / 60;
		_allegro_hline(b, x + 10, y + 65 + i, x + 90, makecol(r, g, 0));
	}
	for (int r = 40; r > 0; r--) {
		int intensity = 255 - r * 5;
		int c = makecol(intensity, intensity / 2, intensity);
		::circlefill(b, x + 145, y + 95, r, c);
	}
}

static void _demo_3d_poly(BITMAP *b, int col, int row) {
	_demo_draw_cell(b, col, row, "3D Polygons");
	int x = DEMO_CELL_X(col);
	int y = DEMO_CELL_Y(row);

	V3D_f v1, v2, v3;
	v1.x = x + 50;
	v1.y = y + 35;
	v1.z = 0;
	v1.c = makecol(200, 50, 50);
	v2.x = x + 20;
	v2.y = y + 95;
	v2.z = 0;
	v2.c = makecol(200, 50, 50);
	v3.x = x + 80;
	v3.y = y + 95;
	v3.z = 0;
	v3.c = makecol(200, 50, 50);
	triangle3d_f(b, POLYTYPE_FLAT, nullptr, &v1, &v2, &v3);
	textout_ex(b, font, "flat", x + 35, y + 105, makecol(180, 180, 180), -1);

	V3D_f g1, g2, g3;
	g1.x = x + 145;
	g1.y = y + 35;
	g1.z = 0;
	g1.c = makecol(255, 0, 0);
	g2.x = x + 110;
	g2.y = y + 95;
	g2.z = 0;
	g2.c = makecol(0, 255, 0);
	g3.x = x + 180;
	g3.y = y + 95;
	g3.z = 0;
	g3.c = makecol(0, 0, 255);
	triangle3d_f(b, POLYTYPE_GCOL, nullptr, &g1, &g2, &g3);
	textout_ex(b, font, "gouraud", x + 120, y + 105, makecol(180, 180, 180), -1);
}

static void _demo_blending(BITMAP *b, int col, int row) {
	_demo_draw_cell(b, col, row, "Drawing Modes");
	int x = DEMO_CELL_X(col);
	int y = DEMO_CELL_Y(row);

	textout_ex(b, font, "SOLID:", x + 10, y + 25, makecol(200, 200, 200), -1);
	::rectfill(b, x + 10, y + 35, x + 60, y + 65, makecol(255, 0, 0));
	::rectfill(b, x + 40, y + 45, x + 90, y + 75, makecol(0, 0, 255));

	textout_ex(b, font, "XOR:", x + 100, y + 25, makecol(200, 200, 200), -1);
	::rectfill(b, x + 100, y + 35, x + 150, y + 65, makecol(255, 0, 0));
	xor_mode(TRUE);
	::rectfill(b, x + 130, y + 45, x + 180, y + 75, makecol(0, 0, 255));
	xor_mode(FALSE);

	textout_ex(b, font, "Masked Sprite:", x + 10, y + 85, makecol(200, 200, 200), -1);
	BITMAP *sprite = create_bitmap(40, 30);
	clear_to_color(sprite, makecol(255, 0, 255));
	::circlefill(sprite, 20, 15, 12, makecol(0, 255, 255));
	::circle(sprite, 20, 15, 12, makecol(255, 255, 0));
	for (int i = 0; i < 8; i++) {
		::rectfill(b, x + 10 + i * 20, y + 95, x + 20 + i * 20, y + 130, makecol(100 + i * 15, 50, 50));
	}
	::masked_blit(sprite, b, 0, 0, x + 50, y + 97, 40, 30);
	::masked_blit(sprite, b, 0, 0, x + 100, y + 97, 40, 30);
	destroy_bitmap(sprite);
}

static void _demo_floodfill(BITMAP *b, int col, int row) {
	_demo_draw_cell(b, col, row, "Flood Fill");
	int x = DEMO_CELL_X(col);
	int y = DEMO_CELL_Y(row);

	int star[20];
	int scx = x + 50, scy = y + 70;
	for (int i = 0; i < 10; i++) {
		float angle = -AL_PI / 2 + i * AL_PI / 5;
		int r = (i % 2 == 0) ? 35 : 15;
		star[i * 2] = scx + (int)(r * cosf(angle));
		star[i * 2 + 1] = scy + (int)(r * sinf(angle));
	}
	for (int i = 0; i < 10; i++) {
		int next = (i + 1) % 10;
		::line(b, star[i * 2], star[i * 2 + 1], star[next * 2], star[next * 2 + 1], makecol(255, 255, 255));
	}
	::floodfill(b, scx, scy, makecol(255, 200, 0));

	::rect(b, x + 110, y + 35, x + 180, y + 105, makecol(255, 255, 255));
	::circle(b, x + 145, y + 70, 20, makecol(255, 255, 255));
	::floodfill(b, x + 115, y + 40, makecol(100, 150, 255));
}

static void _demo_xor_mode(BITMAP *b, int col, int row) {
	_demo_draw_cell(b, col, row, "XOR Mode");
	int x = DEMO_CELL_X(col);
	int y = DEMO_CELL_Y(row);

	for (int i = 0; i < 6; i++) {
		::rectfill(b, x + 15 + i * 30, y + 30, x + 35 + i * 30, y + 110, makecol(200, 100, 50 + i * 30));
	}
	xor_mode(TRUE);
	::rectfill(b, x + 30, y + 50, x + 170, y + 90, makecol(255, 255, 255));
	::circlefill(b, x + 100, y + 70, 30, makecol(255, 255, 255));
	xor_mode(FALSE);
}

static void _demo_subbitmaps(BITMAP *b, int col, int row) {
	_demo_draw_cell(b, col, row, "Sub-Bitmaps");
	int x = DEMO_CELL_X(col);
	int y = DEMO_CELL_Y(row);

	BITMAP *parent = create_bitmap(100, 80);
	clear_to_color(parent, makecol(50, 50, 80));

	BITMAP *sub = ::create_sub_bitmap(parent, 10, 10, 40, 40);
	clear_to_color(sub, makecol(255, 100, 100));
	::circle(sub, 20, 20, 15, makecol(255, 255, 0));
	textout_ex(sub, font, "SUB", 5, 15, makecol(0, 0, 0), -1);

	::rect(parent, 8, 8, 52, 52, makecol(0, 255, 0));
	textout_ex(parent, font, "Parent", 55, 35, makecol(200, 200, 200), -1);

	::blit(parent, b, 0, 0, x + 50, y + 40, 100, 80);

	destroy_bitmap(sub);
	destroy_bitmap(parent);
}

static void _demo_colors(BITMAP *b, int col, int row) {
	_demo_draw_cell(b, col, row, "Color Depth");
	int x = DEMO_CELL_X(col);
	int y = DEMO_CELL_Y(row);

	textprintf_ex(b, font, x + 10, y + 25, makecol(255, 255, 255), -1,
			"Depth: %d-bit", bitmap_color_depth(b));

	for (int i = 0; i < 64; i++) {
		_allegro_vline(b, x + 10 + i * 2, y + 45, y + 55, makecol(i * 4, 0, 0));
		_allegro_vline(b, x + 10 + i * 2 + 1, y + 45, y + 55, makecol(i * 4, 0, 0));
	}
	for (int i = 0; i < 64; i++) {
		_allegro_vline(b, x + 10 + i * 2, y + 60, y + 70, makecol(0, i * 4, 0));
		_allegro_vline(b, x + 10 + i * 2 + 1, y + 60, y + 70, makecol(0, i * 4, 0));
	}
	for (int i = 0; i < 64; i++) {
		_allegro_vline(b, x + 10 + i * 2, y + 75, y + 85, makecol(0, 0, i * 4));
		_allegro_vline(b, x + 10 + i * 2 + 1, y + 75, y + 85, makecol(0, 0, i * 4));
	}
	for (int i = 0; i < 128; i++) {
		int g = i * 2;
		_allegro_vline(b, x + 10 + i, y + 95, y + 105, makecol(g, g, g));
	}
	textout_ex(b, font, "R G B Gray", x + 145, y + 70, makecol(180, 180, 180), -1);
}

static void _demo_fixedpoint(BITMAP *b, int col, int row) {
	_demo_draw_cell(b, col, row, "Fixed-Point Math");
	int x = DEMO_CELL_X(col);
	int y = DEMO_CELL_Y(row);
	int cy = y + 75;

	int prev_sy = cy;
	for (int i = 0; i < 180; i++) {
		fixed angle = itofix(i);
		fixed sin_val = fixsin(angle);
		int sy = cy - fixtoi(fixmul(sin_val, itofix(35)));
		if (i > 0) {
			::line(b, x + 9 + i, prev_sy, x + 10 + i, sy, makecol(100, 255, 100));
		}
		prev_sy = sy;
	}
	prev_sy = cy;
	for (int i = 0; i < 180; i++) {
		fixed angle = itofix(i);
		fixed cos_val = fixcos(angle);
		int sy = cy - fixtoi(fixmul(cos_val, itofix(35)));
		if (i > 0) {
			::line(b, x + 9 + i, prev_sy, x + 10 + i, sy, makecol(255, 100, 100));
		}
		prev_sy = sy;
	}
	_allegro_hline(b, x + 10, cy, x + 190, makecol(80, 80, 80));
	textout_ex(b, font, "sin", x + 10, y + 25, makecol(100, 255, 100), -1);
	textout_ex(b, font, "cos", x + 40, y + 25, makecol(255, 100, 100), -1);
}

void GdAlBitmapGfx::render_demo() {
	ERR_FAIL_COND(!bmp);
	ERR_FAIL_COND_MSG(bmp->w < DEMO_SCREEN_W || bmp->h < DEMO_SCREEN_H,
			vformat("Bitmap too small for demo. Need at least %dx%d, got %dx%d.",
					DEMO_SCREEN_W, DEMO_SCREEN_H, bmp->w, bmp->h));

	clear_to_color(bmp, makecol(30, 30, 40));
	textout_centre_ex(bmp, font, "al_gfx Library Demo - Allegro 4.4.3 Amalgamation",
			DEMO_SCREEN_W / 2, 10, makecol(255, 255, 255), -1);

	acquire_bitmap(bmp);

	_demo_primitives(bmp, 0, 0);
	_demo_rectangles(bmp, 1, 0);
	_demo_circles(bmp, 2, 0);
	_demo_polygons(bmp, 3, 0);

	_demo_arcs(bmp, 0, 1);
	_demo_text(bmp, 1, 1);
	_demo_blitting(bmp, 2, 1);
	_demo_rotation(bmp, 3, 1);

	_demo_gradients(bmp, 0, 2);
	_demo_3d_poly(bmp, 1, 2);
	_demo_blending(bmp, 2, 2);
	_demo_floodfill(bmp, 3, 2);

	_demo_xor_mode(bmp, 0, 3);
	_demo_subbitmaps(bmp, 1, 3);
	_demo_colors(bmp, 2, 3);
	_demo_fixedpoint(bmp, 3, 3);

	release_bitmap(bmp);
}

// ============================================================================
// Internal
// ============================================================================

void GdAlBitmapGfx::_init_from_bitmap(BITMAP *p_bmp, bool p_owns) {
	if (zbuf) {
		::destroy_zbuffer(zbuf);
		zbuf = nullptr;
	}
	if (bmp && owns_bitmap) {
		destroy_bitmap(bmp);
	}
	bmp = p_bmp;
	owns_bitmap = p_owns;
}

GdAlBitmapGfx::GdAlBitmapGfx() {
	bmp = nullptr;
	owns_bitmap = false;
	zbuf = nullptr;
}

GdAlBitmapGfx::~GdAlBitmapGfx() {
	if (zbuf) {
		::destroy_zbuffer(zbuf);
		zbuf = nullptr;
	}
	if (bmp && owns_bitmap) {
		destroy_bitmap(bmp);
	}
}

void GdAlBitmapGfx::_bind_methods() {
	// Info
	ClassDB::bind_method(D_METHOD("get_width"), &GdAlBitmapGfx::get_width);
	ClassDB::bind_method(D_METHOD("get_height"), &GdAlBitmapGfx::get_height);
	ClassDB::bind_method(D_METHOD("get_color_depth"), &GdAlBitmapGfx::get_color_depth);
	ClassDB::bind_method(D_METHOD("is_valid"), &GdAlBitmapGfx::is_valid);

	// Clear
	ClassDB::bind_method(D_METHOD("clear", "color"), &GdAlBitmapGfx::clear);

	// Pixel
	ClassDB::bind_method(D_METHOD("putpixel", "x", "y", "color"), &GdAlBitmapGfx::putpixel);
	ClassDB::bind_method(D_METHOD("getpixel", "x", "y"), &GdAlBitmapGfx::getpixel);

	// Lines
	ClassDB::bind_method(D_METHOD("hline", "x1", "y", "x2", "color"), &GdAlBitmapGfx::hline);
	ClassDB::bind_method(D_METHOD("vline", "x", "y1", "y2", "color"), &GdAlBitmapGfx::vline);
	ClassDB::bind_method(D_METHOD("line", "x1", "y1", "x2", "y2", "color"), &GdAlBitmapGfx::line);

	// Rectangles
	ClassDB::bind_method(D_METHOD("rect", "x1", "y1", "x2", "y2", "color"), &GdAlBitmapGfx::rect);
	ClassDB::bind_method(D_METHOD("rectfill", "x1", "y1", "x2", "y2", "color"), &GdAlBitmapGfx::rectfill);

	// Circles / ellipses
	ClassDB::bind_method(D_METHOD("circle", "x", "y", "radius", "color"), &GdAlBitmapGfx::circle);
	ClassDB::bind_method(D_METHOD("circlefill", "x", "y", "radius", "color"), &GdAlBitmapGfx::circlefill);
	ClassDB::bind_method(D_METHOD("ellipse", "x", "y", "rx", "ry", "color"), &GdAlBitmapGfx::ellipse);
	ClassDB::bind_method(D_METHOD("ellipsefill", "x", "y", "rx", "ry", "color"), &GdAlBitmapGfx::ellipsefill);

	// Arc
	ClassDB::bind_method(D_METHOD("arc", "center", "ang1_deg", "ang2_deg", "radius", "color"), &GdAlBitmapGfx::arc);

	// Triangle
	ClassDB::bind_method(D_METHOD("triangle", "v1", "v2", "v3", "color"), &GdAlBitmapGfx::triangle);

	// Polygon
	ClassDB::bind_method(D_METHOD("polygon", "vertices", "color"), &GdAlBitmapGfx::polygon);

	// Flood fill
	ClassDB::bind_method(D_METHOD("floodfill", "x", "y", "color"), &GdAlBitmapGfx::floodfill);

	// Spline
	ClassDB::bind_method(D_METHOD("spline", "points", "color"), &GdAlBitmapGfx::spline);

	// Text / font
	ClassDB::bind_method(D_METHOD("set_font", "font"), &GdAlBitmapGfx::set_font);
	ClassDB::bind_method(D_METHOD("get_font"), &GdAlBitmapGfx::get_font);
	ClassDB::bind_method(D_METHOD("text", "str", "x", "y", "fg", "bg"), &GdAlBitmapGfx::text, DEFVAL(Color(-1, -1, -1, -1)));
	ClassDB::bind_method(D_METHOD("text_centered", "str", "x", "y", "fg", "bg"), &GdAlBitmapGfx::text_centered, DEFVAL(Color(-1, -1, -1, -1)));
	ClassDB::bind_method(D_METHOD("text_right", "str", "x", "y", "fg", "bg"), &GdAlBitmapGfx::text_right, DEFVAL(Color(-1, -1, -1, -1)));
	ClassDB::bind_method(D_METHOD("get_text_length", "str"), &GdAlBitmapGfx::get_text_length);
	ClassDB::bind_method(D_METHOD("get_text_height"), &GdAlBitmapGfx::get_text_height);

	// Blitting
	ClassDB::bind_method(D_METHOD("blit_from", "source", "src_rect", "dest_pos"), &GdAlBitmapGfx::blit_from);
	ClassDB::bind_method(D_METHOD("stretch_blit_from", "source", "src_rect", "dest_rect"), &GdAlBitmapGfx::stretch_blit_from);
	ClassDB::bind_method(D_METHOD("masked_blit_from", "source", "src_rect", "dest_pos"), &GdAlBitmapGfx::masked_blit_from);

	// Sprites
	ClassDB::bind_method(D_METHOD("draw_sprite", "sprite", "x", "y"), &GdAlBitmapGfx::draw_sprite);
	ClassDB::bind_method(D_METHOD("draw_sprite_h_flip", "sprite", "x", "y"), &GdAlBitmapGfx::draw_sprite_h_flip);
	ClassDB::bind_method(D_METHOD("draw_sprite_v_flip", "sprite", "x", "y"), &GdAlBitmapGfx::draw_sprite_v_flip);
	ClassDB::bind_method(D_METHOD("draw_sprite_vh_flip", "sprite", "x", "y"), &GdAlBitmapGfx::draw_sprite_vh_flip);
	ClassDB::bind_method(D_METHOD("draw_trans_sprite", "sprite", "x", "y"), &GdAlBitmapGfx::draw_trans_sprite);
	ClassDB::bind_method(D_METHOD("draw_lit_sprite", "sprite", "x", "y", "color"), &GdAlBitmapGfx::draw_lit_sprite);
	ClassDB::bind_method(D_METHOD("get_rle_sprite"), &GdAlBitmapGfx::get_rle_sprite);
	ClassDB::bind_method(D_METHOD("draw_rle_sprite", "sprite", "x", "y"), &GdAlBitmapGfx::draw_rle_sprite);

	// Rotation / scaling
	ClassDB::bind_method(D_METHOD("rotate_sprite", "sprite", "pos", "angle_deg"), &GdAlBitmapGfx::rotate_sprite);
	ClassDB::bind_method(D_METHOD("rotate_scaled_sprite", "sprite", "pos", "angle_deg", "scale"), &GdAlBitmapGfx::rotate_scaled_sprite);
	ClassDB::bind_method(D_METHOD("pivot_sprite", "sprite", "pos", "pivot", "angle_deg"), &GdAlBitmapGfx::pivot_sprite);
	ClassDB::bind_method(D_METHOD("pivot_sprite_v_flip", "sprite", "pos", "pivot", "angle_deg"), &GdAlBitmapGfx::pivot_sprite_v_flip);
	ClassDB::bind_method(D_METHOD("pivot_scaled_sprite", "sprite", "pos", "pivot", "angle_deg", "scale"), &GdAlBitmapGfx::pivot_scaled_sprite);
	ClassDB::bind_method(D_METHOD("pivot_scaled_sprite_v_flip", "sprite", "pos", "pivot", "angle_deg", "scale"), &GdAlBitmapGfx::pivot_scaled_sprite_v_flip);

	// Drawing modes
	ClassDB::bind_method(D_METHOD("set_drawing_mode", "mode", "pattern", "x_anchor", "y_anchor"), &GdAlBitmapGfx::set_drawing_mode, DEFVAL(Ref<GdAlBitmapGfx>()), DEFVAL(0), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("solid_mode"), &GdAlBitmapGfx::solid_mode);
	ClassDB::bind_method(D_METHOD("set_xor_mode", "enabled"), &GdAlBitmapGfx::set_xor_mode);
	ClassDB::bind_method(D_METHOD("set_trans_blender", "r", "g", "b", "a"), &GdAlBitmapGfx::set_trans_blender);
	ClassDB::bind_method(D_METHOD("set_add_blender", "r", "g", "b", "a"), &GdAlBitmapGfx::set_add_blender);
	ClassDB::bind_method(D_METHOD("set_alpha_blender"), &GdAlBitmapGfx::set_alpha_blender);

	BIND_ENUM_CONSTANT(MODE_SOLID);
	BIND_ENUM_CONSTANT(MODE_XOR);
	BIND_ENUM_CONSTANT(MODE_COPY_PATTERN);
	BIND_ENUM_CONSTANT(MODE_SOLID_PATTERN);
	BIND_ENUM_CONSTANT(MODE_MASKED_PATTERN);
	BIND_ENUM_CONSTANT(MODE_TRANS);

	// Clipping
	ClassDB::bind_method(D_METHOD("set_clip_rect", "x1", "y1", "x2", "y2"), &GdAlBitmapGfx::set_clip_rect);
	ClassDB::bind_method(D_METHOD("get_clip_rect"), &GdAlBitmapGfx::get_clip_rect);

	// Sub-bitmap
	ClassDB::bind_method(D_METHOD("create_sub_bitmap", "x", "y", "w", "h"), &GdAlBitmapGfx::create_sub_bitmap);

	// Conversion
	ClassDB::bind_method(D_METHOD("get_image"), &GdAlBitmapGfx::get_image);

	// 3D triangles and quads
	ClassDB::bind_method(D_METHOD("triangle3d_flat", "v1", "v2", "v3", "color"), &GdAlBitmapGfx::triangle3d_flat);
	ClassDB::bind_method(D_METHOD("triangle3d_gouraud", "vertices", "colors"), &GdAlBitmapGfx::triangle3d_gouraud);
	ClassDB::bind_method(D_METHOD("triangle3d", "polytype", "vertices", "colors", "texture"), &GdAlBitmapGfx::triangle3d, DEFVAL(Ref<GdAlBitmapGfx>()));
	ClassDB::bind_method(D_METHOD("quad3d_flat", "vertices", "color"), &GdAlBitmapGfx::quad3d_flat);
	ClassDB::bind_method(D_METHOD("quad3d_gouraud", "vertices", "colors"), &GdAlBitmapGfx::quad3d_gouraud);

	BIND_ENUM_CONSTANT(POLY_FLAT);
	BIND_ENUM_CONSTANT(POLY_GCOL);
	BIND_ENUM_CONSTANT(POLY_GRGB);
	BIND_ENUM_CONSTANT(POLY_ATEX);
	BIND_ENUM_CONSTANT(POLY_PTEX);
	BIND_ENUM_CONSTANT(POLY_ATEX_MASK);
	BIND_ENUM_CONSTANT(POLY_PTEX_MASK);
	BIND_ENUM_CONSTANT(POLY_ATEX_LIT);
	BIND_ENUM_CONSTANT(POLY_PTEX_LIT);
	BIND_ENUM_CONSTANT(POLY_ATEX_MASK_LIT);
	BIND_ENUM_CONSTANT(POLY_PTEX_MASK_LIT);
	BIND_ENUM_CONSTANT(POLY_ATEX_TRANS);
	BIND_ENUM_CONSTANT(POLY_PTEX_TRANS);
	BIND_ENUM_CONSTANT(POLY_ATEX_MASK_TRANS);
	BIND_ENUM_CONSTANT(POLY_PTEX_MASK_TRANS);

	// Z-buffer
	ClassDB::bind_method(D_METHOD("create_zbuffer"), &GdAlBitmapGfx::create_zbuffer);
	ClassDB::bind_method(D_METHOD("clear_zbuffer", "z"), &GdAlBitmapGfx::clear_zbuffer, DEFVAL(0.0f));
	ClassDB::bind_method(D_METHOD("enable_zbuffer"), &GdAlBitmapGfx::enable_zbuffer);
	ClassDB::bind_method(D_METHOD("disable_zbuffer"), &GdAlBitmapGfx::disable_zbuffer);
	ClassDB::bind_method(D_METHOD("has_zbuffer"), &GdAlBitmapGfx::has_zbuffer);
	ClassDB::bind_method(D_METHOD("destroy_zbuffer"), &GdAlBitmapGfx::destroy_zbuffer);

	// Demo
	ClassDB::bind_method(D_METHOD("render_demo"), &GdAlBitmapGfx::render_demo);

	// Properties
	ADD_PROPERTY(PropertyInfo(Variant::INT, "width"), "", "get_width");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "height"), "", "get_height");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "color_depth"), "", "get_color_depth");
}

// ============================================================================
// AlBitmapGfx singleton
// ============================================================================

AlBitmapGfx *AlBitmapGfx::singleton = nullptr;

Ref<GdAlBitmapGfx> AlBitmapGfx::create_bitmap(int width, int height) {
	ERR_FAIL_COND_V(width <= 0 || height <= 0, Ref<GdAlBitmapGfx>());

	BITMAP *b = ::create_bitmap(width, height);
	ERR_FAIL_COND_V(!b, Ref<GdAlBitmapGfx>());

	Ref<GdAlBitmapGfx> result;
	result.instance();
	result->_init_from_bitmap(b, true);
	return result;
}

Ref<GdAlBitmapGfx> AlBitmapGfx::create_bitmap_ex(int color_depth, int width, int height) {
	ERR_FAIL_COND_V(width <= 0 || height <= 0, Ref<GdAlBitmapGfx>());

	BITMAP *b = ::create_bitmap_ex(color_depth, width, height);
	ERR_FAIL_COND_V(!b, Ref<GdAlBitmapGfx>());

	Ref<GdAlBitmapGfx> result;
	result.instance();
	result->_init_from_bitmap(b, true);
	return result;
}

void AlBitmapGfx::set_color_depth(int depth) {
	::set_color_depth(depth);
}

int AlBitmapGfx::get_color_depth() const {
	return ::get_color_depth();
}

Ref<GdAlBitmapGfx> AlBitmapGfx::load_tga(const String &path) {
	CharString cs = path.ascii();
	BITMAP *b = ::load_tga(cs.get_data(), nullptr);
	ERR_FAIL_COND_V_MSG(!b, Ref<GdAlBitmapGfx>(), "Failed to load TGA: " + path);

	Ref<GdAlBitmapGfx> result;
	result.instance();
	result->_init_from_bitmap(b, true);
	return result;
}

Error AlBitmapGfx::save_tga(const String &path, Ref<GdAlBitmapGfx> bmp) {
	ERR_FAIL_COND_V(bmp.is_null() || !bmp->_get_bitmap(), ERR_INVALID_PARAMETER);
	CharString cs = path.ascii();
	int ret = ::save_tga(cs.get_data(), bmp->_get_bitmap(), nullptr);
	return ret == 0 ? OK : FAILED;
}

Ref<GdAlBitmapGfx> AlBitmapGfx::from_image(Ref<Image> image) {
	ERR_FAIL_COND_V(image.is_null(), Ref<GdAlBitmapGfx>());

	int w = image->get_width();
	int h = image->get_height();
	ERR_FAIL_COND_V(w <= 0 || h <= 0, Ref<GdAlBitmapGfx>());

	// Convert to RGBA8 if needed
	Ref<Image> src = image;
	if (src->get_format() != Image::FORMAT_RGBA8) {
		src = src->duplicate();
		src->convert(Image::FORMAT_RGBA8);
	}

	BITMAP *b = ::create_bitmap_ex(32, w, h);
	ERR_FAIL_COND_V(!b, Ref<GdAlBitmapGfx>());

	PoolByteArray data = src->get_data();
	PoolByteArray::Read rd = data.read();

	if (_rgb_r_shift_32 == 0 && _rgb_g_shift_32 == 8 &&
			_rgb_b_shift_32 == 16 && _rgb_a_shift_32 == 24) {
		// Fast path: RGBA8 byte order matches pixel layout — direct scanline copy
		for (int y = 0; y < h; y++) {
			memcpy(b->line[y], &rd[y * w * 4], w * 4);
		}
	} else {
		// General path: pack each pixel with correct shifts
		for (int y = 0; y < h; y++) {
			const uint8_t *src_row = &rd[y * w * 4];
			int *dst_row = (int *)b->line[y];
			for (int x = 0; x < w; x++) {
				dst_row[x] = makeacol32(src_row[0], src_row[1], src_row[2], src_row[3]);
				src_row += 4;
			}
		}
	}

	Ref<GdAlBitmapGfx> result;
	result.instance();
	result->_init_from_bitmap(b, true);
	return result;
}

Ref<GdAlFont> AlBitmapGfx::load_bitmap_font(const String &path) {
	Ref<Image> img;
	img.instance();
	Error err = img->load(path);
	ERR_FAIL_COND_V_MSG(err != OK, Ref<GdAlFont>(), "Failed to load font image: " + path);

	// Convert image to al_gfx BITMAP
	Ref<GdAlBitmapGfx> bmp_wrapper = from_image(img);
	ERR_FAIL_COND_V(bmp_wrapper.is_null(), Ref<GdAlFont>());

	return font_from_bitmap(bmp_wrapper);
}

Ref<GdAlFont> AlBitmapGfx::font_from_bitmap(Ref<GdAlBitmapGfx> bitmap) {
	ERR_FAIL_COND_V(bitmap.is_null() || !bitmap->_get_bitmap(), Ref<GdAlFont>());

	FONT *f = ::grab_font_from_bitmap(bitmap->_get_bitmap());
	ERR_FAIL_COND_V_MSG(!f, Ref<GdAlFont>(), "Failed to extract font from bitmap");

	Ref<GdAlFont> result;
	result.instance();
	result->_init_from_font(f);
	return result;
}

AlBitmapGfx::AlBitmapGfx() {
	ERR_FAIL_COND_MSG(singleton, "AlBitmapGfx singleton already exists.");
	singleton = this;
	initialized = false;

	// Initialize al_gfx global state
	install_error(nullptr);
	::set_color_depth(32);
	set_palette(desktop_palette);
	initialized = true;
}

AlBitmapGfx::~AlBitmapGfx() {
	singleton = nullptr;
}

void AlBitmapGfx::_bind_methods() {
	// Factory
	ClassDB::bind_method(D_METHOD("create_bitmap", "width", "height"), &AlBitmapGfx::create_bitmap);
	ClassDB::bind_method(D_METHOD("create_bitmap_ex", "color_depth", "width", "height"), &AlBitmapGfx::create_bitmap_ex);

	// Color depth
	ClassDB::bind_method(D_METHOD("set_color_depth", "depth"), &AlBitmapGfx::set_color_depth);
	ClassDB::bind_method(D_METHOD("get_color_depth"), &AlBitmapGfx::get_color_depth);

	// I/O
	ClassDB::bind_method(D_METHOD("load_tga", "path"), &AlBitmapGfx::load_tga);
	ClassDB::bind_method(D_METHOD("save_tga", "path", "bitmap"), &AlBitmapGfx::save_tga);

	// Image conversion
	ClassDB::bind_method(D_METHOD("from_image", "image"), &AlBitmapGfx::from_image);

	// Font loading
	ClassDB::bind_method(D_METHOD("load_bitmap_font", "path"), &AlBitmapGfx::load_bitmap_font);
	ClassDB::bind_method(D_METHOD("font_from_bitmap", "bitmap"), &AlBitmapGfx::font_from_bitmap);
}

// ============================================================================
// AlBitmapGfxNode
// ============================================================================

AlBitmapGfxNode::AlBitmapGfxNode() :
		centered(true),
		dirty(true),
		auto_update(true),
		tex_flags(Texture::FLAG_FILTER) {
	set_process(true);
}

void AlBitmapGfxNode::set_bitmap(Ref<GdAlBitmapGfx> p_bitmap) {
	bitmap = p_bitmap;
	// Reset cached texture/image — will be recreated on next update
	texture.unref();
	image.unref();
	dirty = true;
}

Ref<GdAlBitmapGfx> AlBitmapGfxNode::get_bitmap() const {
	return bitmap;
}

void AlBitmapGfxNode::set_centered(bool p_centered) {
	if (centered == p_centered)
		return;
	centered = p_centered;
	update();
}

bool AlBitmapGfxNode::is_centered() const {
	return centered;
}

void AlBitmapGfxNode::set_auto_update(bool p_auto) {
	auto_update = p_auto;
	set_process(auto_update);
}

bool AlBitmapGfxNode::is_auto_update() const {
	return auto_update;
}

void AlBitmapGfxNode::set_texture_flags(int p_flags) {
	if (tex_flags == p_flags)
		return;
	tex_flags = p_flags;
	// Force texture recreation with new flags
	texture.unref();
	dirty = true;
}

int AlBitmapGfxNode::get_texture_flags() const {
	return tex_flags;
}

void AlBitmapGfxNode::mark_dirty() {
	dirty = true;
}

Ref<ImageTexture> AlBitmapGfxNode::get_texture() const {
	return texture;
}

void AlBitmapGfxNode::_update_texture() {
	if (bitmap.is_null() || !bitmap->is_valid())
		return;

	BITMAP *bmp = bitmap->_get_bitmap();
	int w = bmp->w;
	int h = bmp->h;
	int depth = bitmap_color_depth(bmp);

	// Allocate persistent Image if needed (or if bitmap size changed)
	if (image.is_null() || image->get_width() != w || image->get_height() != h) {
		image.instance();
		image->create(w, h, false, Image::FORMAT_RGBA8);
		// Force texture recreation
		texture.unref();
	}

	// Write scanlines directly into the persistent Image's data
	PoolByteArray data;
	data.resize(w * h * 4);
	{
		PoolByteArray::Write wd = data.write();
		if (depth == 32 && _rgb_r_shift_32 == 0 && _rgb_g_shift_32 == 8 &&
				_rgb_b_shift_32 == 16 && _rgb_a_shift_32 == 24) {
			for (int y = 0; y < h; y++) {
				memcpy(&wd[y * w * 4], bmp->line[y], w * 4);
			}
		} else {
			for (int y = 0; y < h; y++) {
				unsigned char *sl = bmp->line[y];
				uint8_t *dst = &wd[y * w * 4];
				for (int x = 0; x < w; x++) {
					int c;
					switch (depth) {
						case 32:
							c = ((int *)sl)[x];
							dst[0] = getr32(c);
							dst[1] = getg32(c);
							dst[2] = getb32(c);
							dst[3] = geta32(c);
							break;
						case 24:
							c = ((sl[x * 3 + 2] << 16) | (sl[x * 3 + 1] << 8) | sl[x * 3]);
							dst[0] = getr24(c);
							dst[1] = getg24(c);
							dst[2] = getb24(c);
							dst[3] = 255;
							break;
						case 16:
							c = ((unsigned short *)sl)[x];
							dst[0] = getr16(c);
							dst[1] = getg16(c);
							dst[2] = getb16(c);
							dst[3] = 255;
							break;
						case 15:
							c = ((unsigned short *)sl)[x];
							dst[0] = getr15(c);
							dst[1] = getg15(c);
							dst[2] = getb15(c);
							dst[3] = 255;
							break;
						default:
							c = ::getpixel(bmp, x, y);
							dst[0] = getr_depth(depth, c);
							dst[1] = getg_depth(depth, c);
							dst[2] = getb_depth(depth, c);
							dst[3] = 255;
							break;
					}
					dst += 4;
				}
			}
		}
	}
	image->create(w, h, false, Image::FORMAT_RGBA8, data);

	// Create or update texture
	if (texture.is_null()) {
		texture.instance();
		texture->create_from_image(image, tex_flags);
	} else {
		texture->set_data(image);
	}
}

void AlBitmapGfxNode::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_PROCESS: {
			if (auto_update && bitmap.is_valid() && bitmap->is_valid()) {
				dirty = true;
			}
			if (dirty) {
				_update_texture();
				dirty = false;
				update(); // queue NOTIFICATION_DRAW
			}
		} break;

		case NOTIFICATION_DRAW: {
			if (texture.is_null())
				return;

			Point2 ofs;
			if (centered) {
				ofs = -Size2(texture->get_width(), texture->get_height()) / 2;
			}
			draw_texture(texture, ofs);
		} break;
	}
}

void AlBitmapGfxNode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_bitmap", "bitmap"), &AlBitmapGfxNode::set_bitmap);
	ClassDB::bind_method(D_METHOD("get_bitmap"), &AlBitmapGfxNode::get_bitmap);
	ClassDB::bind_method(D_METHOD("set_centered", "centered"), &AlBitmapGfxNode::set_centered);
	ClassDB::bind_method(D_METHOD("is_centered"), &AlBitmapGfxNode::is_centered);
	ClassDB::bind_method(D_METHOD("set_auto_update", "auto_update"), &AlBitmapGfxNode::set_auto_update);
	ClassDB::bind_method(D_METHOD("is_auto_update"), &AlBitmapGfxNode::is_auto_update);
	ClassDB::bind_method(D_METHOD("set_texture_flags", "flags"), &AlBitmapGfxNode::set_texture_flags);
	ClassDB::bind_method(D_METHOD("get_texture_flags"), &AlBitmapGfxNode::get_texture_flags);
	ClassDB::bind_method(D_METHOD("mark_dirty"), &AlBitmapGfxNode::mark_dirty);
	ClassDB::bind_method(D_METHOD("get_texture"), &AlBitmapGfxNode::get_texture);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "bitmap", PROPERTY_HINT_RESOURCE_TYPE, "GdAlBitmapGfx"), "set_bitmap", "get_bitmap");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "centered"), "set_centered", "is_centered");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_update"), "set_auto_update", "is_auto_update");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "texture_flags"), "set_texture_flags", "get_texture_flags");
}

// ============================================================================
// Tests
// ============================================================================

#ifdef DOCTEST

// Helper: ensure al_gfx is initialized for tests
static void _ensure_al_init() {
	static bool inited = false;
	if (!inited) {
		install_error(nullptr);
		::set_color_depth(32);
		set_palette(desktop_palette);
		inited = true;
	}
}

// Helper: create a raw BITMAP for low-level tests
static BITMAP *_test_bitmap(int w, int h) {
	_ensure_al_init();
	BITMAP *b = ::create_bitmap(w, h);
	clear_to_color(b, makecol(0, 0, 0));
	return b;
}

TEST_SUITE("albmpgfx") {
	// -----------------------------------------------------------------------
	// Bitmap creation and properties
	// -----------------------------------------------------------------------

	TEST_CASE("al_gfx: create_bitmap returns valid bitmap") {
		BITMAP *b = _test_bitmap(64, 48);
		REQUIRE(b != nullptr);
		CHECK(b->w == 64);
		CHECK(b->h == 48);
		CHECK(bitmap_color_depth(b) == 32);
		destroy_bitmap(b);
	}

	TEST_CASE("al_gfx: create_bitmap_ex with different color depths") {
		_ensure_al_init();
		int depths[] = { 8, 15, 16, 24, 32 };
		for (int d : depths) {
			BITMAP *b = ::create_bitmap_ex(d, 32, 32);
			REQUIRE(b != nullptr);
			CHECK(bitmap_color_depth(b) == d);
			destroy_bitmap(b);
		}
	}

	TEST_CASE("al_gfx: clear_to_color fills entire bitmap") {
		BITMAP *b = _test_bitmap(16, 16);
		int fill = makecol(100, 150, 200);
		clear_to_color(b, fill);
		for (int y = 0; y < 16; y++) {
			for (int x = 0; x < 16; x++) {
				CHECK(::getpixel(b, x, y) == fill);
			}
		}
		destroy_bitmap(b);
	}

	// -----------------------------------------------------------------------
	// Pixel operations
	// -----------------------------------------------------------------------

	TEST_CASE("al_gfx: putpixel and getpixel round-trip") {
		BITMAP *b = _test_bitmap(8, 8);
		int c = makecol(255, 128, 64);
		::putpixel(b, 3, 5, c);
		CHECK(::getpixel(b, 3, 5) == c);
		// Other pixels remain black
		CHECK(::getpixel(b, 0, 0) == makecol(0, 0, 0));
		destroy_bitmap(b);
	}

	TEST_CASE("al_gfx: getpixel out-of-bounds returns -1") {
		BITMAP *b = _test_bitmap(8, 8);
		CHECK(::getpixel(b, -1, 0) == -1);
		CHECK(::getpixel(b, 8, 0) == -1);
		CHECK(::getpixel(b, 0, -1) == -1);
		CHECK(::getpixel(b, 0, 8) == -1);
		destroy_bitmap(b);
	}

	// -----------------------------------------------------------------------
	// Color functions
	// -----------------------------------------------------------------------

	TEST_CASE("al_gfx: makecol and getr/g/b round-trip at 32-bit") {
		_ensure_al_init();
		int c = makecol_depth(32, 200, 100, 50);
		CHECK(getr_depth(32, c) == 200);
		CHECK(getg_depth(32, c) == 100);
		CHECK(getb_depth(32, c) == 50);
	}

	TEST_CASE("al_gfx: makecol at 16-bit preserves approximate values") {
		_ensure_al_init();
		int c = makecol_depth(16, 255, 128, 0);
		// 16-bit has reduced precision: 5-6-5 bits
		int r = getr_depth(16, c);
		int g = getg_depth(16, c);
		int b = getb_depth(16, c);
		CHECK(r >= 248); // 5-bit: 255 -> 248..255
		CHECK(g >= 124); // 6-bit: 128 -> 124..131
		CHECK(g <= 131);
		CHECK(b == 0);
	}

	TEST_CASE("al_gfx: hsv_to_rgb and rgb_to_hsv round-trip") {
		_ensure_al_init();
		int r_out, g_out, b_out;
		hsv_to_rgb(120.0f, 1.0f, 1.0f, &r_out, &g_out, &b_out);
		CHECK(r_out == 0);
		CHECK(g_out == 255);
		CHECK(b_out == 0);

		float h, s, v;
		rgb_to_hsv(255, 0, 0, &h, &s, &v);
		CHECK(h == doctest::Approx(0.0f).epsilon(1.0f));
		CHECK(s == doctest::Approx(1.0f));
		CHECK(v == doctest::Approx(1.0f));
	}

	// -----------------------------------------------------------------------
	// Line drawing
	// -----------------------------------------------------------------------

	TEST_CASE("al_gfx: hline draws horizontal line") {
		BITMAP *b = _test_bitmap(32, 8);
		int c = makecol(255, 0, 0);
		_allegro_hline(b, 5, 3, 20, c);
		for (int x = 5; x <= 20; x++) {
			CHECK(::getpixel(b, x, 3) == c);
		}
		// Pixel before and after should be black
		CHECK(::getpixel(b, 4, 3) == makecol(0, 0, 0));
		CHECK(::getpixel(b, 21, 3) == makecol(0, 0, 0));
		destroy_bitmap(b);
	}

	TEST_CASE("al_gfx: vline draws vertical line") {
		BITMAP *b = _test_bitmap(8, 32);
		int c = makecol(0, 255, 0);
		_allegro_vline(b, 4, 2, 15, c);
		for (int y = 2; y <= 15; y++) {
			CHECK(::getpixel(b, 4, y) == c);
		}
		CHECK(::getpixel(b, 4, 1) == makecol(0, 0, 0));
		CHECK(::getpixel(b, 4, 16) == makecol(0, 0, 0));
		destroy_bitmap(b);
	}

	TEST_CASE("al_gfx: line draws diagonal") {
		BITMAP *b = _test_bitmap(16, 16);
		int c = makecol(0, 0, 255);
		::line(b, 0, 0, 15, 15, c);
		// Diagonal pixels should be colored
		CHECK(::getpixel(b, 0, 0) == c);
		CHECK(::getpixel(b, 7, 7) == c);
		CHECK(::getpixel(b, 15, 15) == c);
		destroy_bitmap(b);
	}

	// -----------------------------------------------------------------------
	// Rectangles
	// -----------------------------------------------------------------------

	TEST_CASE("al_gfx: rectfill fills area") {
		BITMAP *b = _test_bitmap(32, 32);
		int c = makecol(128, 64, 32);
		::rectfill(b, 5, 5, 10, 10, c);
		// Interior should be filled
		CHECK(::getpixel(b, 7, 7) == c);
		CHECK(::getpixel(b, 5, 5) == c);
		CHECK(::getpixel(b, 10, 10) == c);
		// Outside should be black
		CHECK(::getpixel(b, 4, 7) == makecol(0, 0, 0));
		CHECK(::getpixel(b, 11, 7) == makecol(0, 0, 0));
		destroy_bitmap(b);
	}

	TEST_CASE("al_gfx: rect draws outline only") {
		BITMAP *b = _test_bitmap(32, 32);
		int c = makecol(255, 255, 0);
		::rect(b, 5, 5, 15, 15, c);
		// Edges should be colored
		CHECK(::getpixel(b, 5, 5) == c);
		CHECK(::getpixel(b, 10, 5) == c);
		CHECK(::getpixel(b, 15, 5) == c);
		CHECK(::getpixel(b, 5, 10) == c);
		CHECK(::getpixel(b, 15, 10) == c);
		// Interior should be black
		CHECK(::getpixel(b, 10, 10) == makecol(0, 0, 0));
		destroy_bitmap(b);
	}

	// -----------------------------------------------------------------------
	// Circles and ellipses
	// -----------------------------------------------------------------------

	TEST_CASE("al_gfx: circlefill fills circle area") {
		BITMAP *b = _test_bitmap(64, 64);
		int c = makecol(0, 200, 100);
		::circlefill(b, 32, 32, 10, c);
		// Center should be filled
		CHECK(::getpixel(b, 32, 32) == c);
		// Points on cardinal directions within radius
		CHECK(::getpixel(b, 32, 23) == c); // top edge (radius 9 < 10)
		CHECK(::getpixel(b, 32, 41) == c); // bottom edge
		// Point well outside radius
		CHECK(::getpixel(b, 0, 0) == makecol(0, 0, 0));
		destroy_bitmap(b);
	}

	TEST_CASE("al_gfx: circle draws outline") {
		BITMAP *b = _test_bitmap(64, 64);
		int c = makecol(255, 128, 0);
		::circle(b, 32, 32, 15, c);
		// Center should be black (outline only)
		CHECK(::getpixel(b, 32, 32) == makecol(0, 0, 0));
		// Top of circle should be colored
		CHECK(::getpixel(b, 32, 17) == c);
		destroy_bitmap(b);
	}

	TEST_CASE("al_gfx: ellipsefill fills elliptical area") {
		BITMAP *b = _test_bitmap(64, 64);
		int c = makecol(200, 50, 150);
		::ellipsefill(b, 32, 32, 20, 10, c);
		// Center filled
		CHECK(::getpixel(b, 32, 32) == c);
		// Horizontal extent (within rx=20)
		CHECK(::getpixel(b, 50, 32) == c);
		// Vertical extent (within ry=10)
		CHECK(::getpixel(b, 32, 41) == c);
		// Outside ellipse
		CHECK(::getpixel(b, 32, 20) == makecol(0, 0, 0));
		destroy_bitmap(b);
	}

	// -----------------------------------------------------------------------
	// Triangle
	// -----------------------------------------------------------------------

	TEST_CASE("al_gfx: triangle fills triangular area") {
		BITMAP *b = _test_bitmap(64, 64);
		int c = makecol(255, 0, 128);
		::triangle(b, 10, 50, 32, 10, 54, 50, c);
		// Centroid area should be filled
		CHECK(::getpixel(b, 32, 35) == c);
		// Outside should be black
		CHECK(::getpixel(b, 0, 0) == makecol(0, 0, 0));
		destroy_bitmap(b);
	}

	// -----------------------------------------------------------------------
	// Polygon
	// -----------------------------------------------------------------------

	TEST_CASE("al_gfx: polygon fills convex polygon") {
		BITMAP *b = _test_bitmap(64, 64);
		int c = makecol(100, 200, 50);
		// Square polygon
		int pts[] = { 10, 10, 50, 10, 50, 50, 10, 50 };
		::polygon(b, 4, pts, c);
		// Interior should be filled
		CHECK(::getpixel(b, 30, 30) == c);
		// Outside
		CHECK(::getpixel(b, 5, 5) == makecol(0, 0, 0));
		destroy_bitmap(b);
	}

	// -----------------------------------------------------------------------
	// Flood fill
	// -----------------------------------------------------------------------

	TEST_CASE("al_gfx: floodfill fills bounded region") {
		BITMAP *b = _test_bitmap(32, 32);
		int border = makecol(255, 255, 255);
		int fill = makecol(255, 0, 0);

		// Draw a rectangle border
		::rect(b, 5, 5, 25, 25, border);
		// Flood fill inside
		::floodfill(b, 15, 15, fill);

		// Interior should be filled
		CHECK(::getpixel(b, 15, 15) == fill);
		CHECK(::getpixel(b, 6, 6) == fill);
		// Border should remain
		CHECK(::getpixel(b, 5, 5) == border);
		// Outside should be black
		CHECK(::getpixel(b, 0, 0) == makecol(0, 0, 0));
		destroy_bitmap(b);
	}

	// -----------------------------------------------------------------------
	// Blitting
	// -----------------------------------------------------------------------

	TEST_CASE("al_gfx: blit copies rectangular region") {
		BITMAP *src = _test_bitmap(16, 16);
		BITMAP *dst = _test_bitmap(32, 32);

		int c = makecol(111, 222, 33);
		::rectfill(src, 0, 0, 15, 15, c);

		::blit(src, dst, 0, 0, 8, 8, 16, 16);

		// Destination should have the copied pixels
		CHECK(::getpixel(dst, 8, 8) == c);
		CHECK(::getpixel(dst, 23, 23) == c);
		// Outside the blit region should be black
		CHECK(::getpixel(dst, 0, 0) == makecol(0, 0, 0));

		destroy_bitmap(src);
		destroy_bitmap(dst);
	}

	TEST_CASE("al_gfx: stretch_blit scales content" * doctest::skip(true)) {
		BITMAP *src = _test_bitmap(8, 8);
		BITMAP *dst = _test_bitmap(16, 16);

		int c = makecol(200, 100, 50);
		::rectfill(src, 0, 0, 7, 7, c);

		::stretch_blit(src, dst, 0, 0, 8, 8, 0, 0, 16, 16);

		// Entire destination should be filled (2x scale)
		CHECK(::getpixel(dst, 0, 0) == c);
		CHECK(::getpixel(dst, 15, 15) == c);

		destroy_bitmap(src);
		destroy_bitmap(dst);
	}

	TEST_CASE("al_gfx: masked_blit skips mask color") {
		BITMAP *src = _test_bitmap(8, 8);
		BITMAP *dst = _test_bitmap(16, 16);

		int bg = makecol(50, 50, 50);
		int obj = makecol(255, 255, 0);
		int mask = makecol(255, 0, 255); // magenta mask for 32-bit

		clear_to_color(src, mask);
		::rectfill(src, 2, 2, 5, 5, obj);

		clear_to_color(dst, bg);
		::masked_blit(src, dst, 0, 0, 4, 4, 8, 8);

		// Object pixels should be transferred
		CHECK(::getpixel(dst, 6, 6) == obj);
		// Mask pixels should leave background untouched
		CHECK(::getpixel(dst, 4, 4) == bg);

		destroy_bitmap(src);
		destroy_bitmap(dst);
	}

	// -----------------------------------------------------------------------
	// Sub-bitmaps
	// -----------------------------------------------------------------------

	TEST_CASE("al_gfx: sub-bitmap shares parent memory") {
		BITMAP *parent = _test_bitmap(32, 32);
		BITMAP *sub = ::create_sub_bitmap(parent, 8, 8, 16, 16);
		REQUIRE(sub != nullptr);
		CHECK(sub->w == 16);
		CHECK(sub->h == 16);
		CHECK(is_sub_bitmap(sub));

		// Draw on sub-bitmap
		int c = makecol(255, 128, 64);
		::putpixel(sub, 0, 0, c);

		// Should be visible on parent at offset (8,8)
		CHECK(::getpixel(parent, 8, 8) == c);

		destroy_bitmap(sub);
		destroy_bitmap(parent);
	}

	// -----------------------------------------------------------------------
	// Sprite drawing
	// -----------------------------------------------------------------------

	TEST_CASE("al_gfx: draw_sprite with masked transparency") {
		BITMAP *dst = _test_bitmap(32, 32);
		BITMAP *sprite = ::create_bitmap(8, 8);
		int mask = makecol(255, 0, 255);
		int obj = makecol(0, 255, 0);
		int bg = makecol(50, 50, 50);

		clear_to_color(sprite, mask);
		::rectfill(sprite, 2, 2, 5, 5, obj);
		clear_to_color(dst, bg);

		::draw_sprite(dst, sprite, 10, 10);

		// Object pixels transferred
		CHECK(::getpixel(dst, 12, 12) == obj);
		// Masked pixels leave background
		CHECK(::getpixel(dst, 10, 10) == bg);

		destroy_bitmap(sprite);
		destroy_bitmap(dst);
	}

	TEST_CASE("al_gfx: draw_trans_sprite with alpha blending") {
		_ensure_al_init();
		BITMAP *dst = ::create_bitmap_ex(32, 32, 32);
		BITMAP *sprite = ::create_bitmap_ex(32, 8, 8);

		// Fill destination with solid blue
		clear_to_color(dst, makeacol32(0, 0, 255, 255));

		// Fill sprite with semi-transparent red (alpha=128)
		clear_to_color(sprite, makeacol32(255, 0, 0, 128));

		// Set alpha blender for RGBA sprites
		::set_alpha_blender();

		::draw_trans_sprite(dst, sprite, 4, 4);

		// Pixel inside sprite area should be blended (not pure blue, not pure red)
		int c = ::getpixel(dst, 8, 8);
		int r = getr32(c);
		int b = getb32(c);
		CHECK(r > 50); // has some red from sprite
		CHECK(b > 50); // has some blue from destination
		CHECK(r < 230); // not fully red
		CHECK(b < 230); // not fully blue

		// Pixel outside sprite area should be unchanged (pure blue)
		int c2 = ::getpixel(dst, 0, 0);
		CHECK(getr32(c2) == 0);
		CHECK(getb32(c2) == 255);

		destroy_bitmap(sprite);
		destroy_bitmap(dst);
	}

	TEST_CASE("al_gfx: draw_lit_sprite tints pixels") {
		_ensure_al_init();
		BITMAP *dst = ::create_bitmap_ex(32, 32, 32);
		BITMAP *sprite = ::create_bitmap_ex(32, 8, 8);

		clear_to_color(dst, makecol32(0, 0, 0));
		// White sprite — lit_sprite will tint it
		clear_to_color(sprite, makecol32(255, 255, 255));

		// Set translucent blender with a red tint
		::set_trans_blender(255, 0, 0, 0);

		// color=128 means mid-level lighting
		::draw_lit_sprite(dst, sprite, 4, 4, 128);

		// Pixel inside sprite area should have been drawn (not black background)
		int c = ::getpixel(dst, 8, 8);
		int brightness = getr32(c) + getg32(c) + getb32(c);
		CHECK(brightness > 0); // something was drawn

		// Pixel outside sprite area should be unchanged (black)
		int c2 = ::getpixel(dst, 0, 0);
		CHECK(getr32(c2) == 0);
		CHECK(getg32(c2) == 0);
		CHECK(getb32(c2) == 0);

		destroy_bitmap(sprite);
		destroy_bitmap(dst);
	}

	TEST_CASE("GdAlBitmapGfx: draw_trans_sprite wrapper") {
		_ensure_al_init();
		Ref<GdAlBitmapGfx> dst;
		dst.instance();
		dst->_init_from_bitmap(::create_bitmap_ex(32, 32, 32), true);
		dst->clear(Color(0, 0, 1)); // blue

		Ref<GdAlBitmapGfx> sprite;
		sprite.instance();
		sprite->_init_from_bitmap(::create_bitmap_ex(32, 8, 8), true);
		// Semi-transparent red
		BITMAP *sb = sprite->_get_bitmap();
		clear_to_color(sb, makeacol32(255, 0, 0, 128));

		dst->set_alpha_blender();
		dst->draw_trans_sprite(sprite, 4, 4);

		// Verify blending occurred
		Color px = dst->getpixel(8, 8);
		CHECK(px.r > 0.2);
		CHECK(px.b > 0.2);
		CHECK(px.r < 0.9);
		CHECK(px.b < 0.9);
	}

	TEST_CASE("GdAlBitmapGfx: draw_lit_sprite wrapper") {
		_ensure_al_init();
		Ref<GdAlBitmapGfx> dst;
		dst.instance();
		dst->_init_from_bitmap(::create_bitmap_ex(32, 32, 32), true);
		dst->clear(Color(0, 0, 0)); // black

		Ref<GdAlBitmapGfx> sprite;
		sprite.instance();
		sprite->_init_from_bitmap(::create_bitmap_ex(32, 8, 8), true);
		sprite->clear(Color(1, 1, 1)); // white

		dst->set_trans_blender(255, 0, 0, 0);
		dst->draw_lit_sprite(sprite, 4, 4, 128);

		// Something was drawn at sprite location
		Color px = dst->getpixel(8, 8);
		float brightness = px.r + px.g + px.b;
		CHECK(brightness > 0.0);

		// Background untouched
		Color bg = dst->getpixel(0, 0);
		CHECK(bg.r == doctest::Approx(0.0).epsilon(0.02));
		CHECK(bg.g == doctest::Approx(0.0).epsilon(0.02));
		CHECK(bg.b == doctest::Approx(0.0).epsilon(0.02));
	}

	// -----------------------------------------------------------------------
	// XOR drawing mode
	// -----------------------------------------------------------------------

	TEST_CASE("al_gfx: xor_mode XORs pixels") {
		BITMAP *b = _test_bitmap(16, 16);
		int c1 = makecol(255, 0, 0);
		int c2 = makecol(0, 255, 0);

		::putpixel(b, 5, 5, c1);
		xor_mode(TRUE);
		::putpixel(b, 5, 5, c2);
		xor_mode(FALSE);

		int result = ::getpixel(b, 5, 5);
		// XOR of red and green should give yellow-ish
		CHECK(getr_depth(32, result) == 255);
		CHECK(getg_depth(32, result) == 255);
		CHECK(getb_depth(32, result) == 0);

		destroy_bitmap(b);
	}

	TEST_CASE("al_gfx: drawing_mode with COPY_PATTERN fills from pattern") {
		_ensure_al_init();
		// Create a 4x4 checkerboard pattern (power-of-two required)
		BITMAP *pattern = ::create_bitmap_ex(32, 4, 4);
		clear_to_color(pattern, makecol(0, 0, 0));
		// Top-left and bottom-right quadrants white
		::rectfill(pattern, 0, 0, 1, 1, makecol(255, 255, 255));
		::rectfill(pattern, 2, 2, 3, 3, makecol(255, 255, 255));

		BITMAP *dst = ::create_bitmap_ex(32, 16, 16);
		clear_to_color(dst, makecol(128, 128, 128));

		::drawing_mode(DRAW_MODE_COPY_PATTERN, pattern, 0, 0);
		// Draw a filled rect — should use pattern instead of solid color
		::rectfill(dst, 0, 0, 15, 15, makecol(255, 0, 0)); // color ignored in COPY_PATTERN
		::solid_mode();

		// Pixel at (0,0) should come from pattern (0,0) = white
		int c00 = ::getpixel(dst, 0, 0);
		CHECK(getr32(c00) == 255);
		CHECK(getg32(c00) == 255);

		// Pixel at (1,0) should come from pattern (1,0) = white
		// Pixel at (2,0) should come from pattern (2,0) = black
		int c20 = ::getpixel(dst, 2, 0);
		CHECK(getr32(c20) == 0);

		destroy_bitmap(pattern);
		destroy_bitmap(dst);
	}

	TEST_CASE("al_gfx: drawing_mode with MASKED_PATTERN skips mask-colored pixels") {
		_ensure_al_init();
		BITMAP *pattern = ::create_bitmap_ex(32, 4, 4);
		// Mask color (magenta) = skip; other colors = draw in specified color
		clear_to_color(pattern, bitmap_mask_color(pattern));
		::rectfill(pattern, 0, 0, 1, 1, makecol(255, 255, 255)); // non-mask = draw

		BITMAP *dst = ::create_bitmap_ex(32, 16, 16);
		clear_to_color(dst, makecol(0, 0, 0));

		::drawing_mode(DRAW_MODE_MASKED_PATTERN, pattern, 0, 0);
		::rectfill(dst, 0, 0, 15, 15, makecol(255, 0, 0)); // draws red where pattern is non-mask
		::solid_mode();

		// Where pattern is non-mask → red
		int c00 = ::getpixel(dst, 0, 0);
		CHECK(getr32(c00) == 255);
		CHECK(getb32(c00) == 0);

		// Where pattern is mask → untouched (black)
		int c20 = ::getpixel(dst, 2, 0);
		CHECK(getr32(c20) == 0);

		destroy_bitmap(pattern);
		destroy_bitmap(dst);
	}

	TEST_CASE("GdAlBitmapGfx: set_drawing_mode and solid_mode wrapper") {
		_ensure_al_init();
		Ref<GdAlBitmapGfx> pattern;
		pattern.instance();
		pattern->_init_from_bitmap(::create_bitmap_ex(32, 4, 4), true);
		pattern->clear(Color(0, 0, 0));
		pattern->rectfill(0, 0, 1, 1, Color(1, 1, 1));
		pattern->rectfill(2, 2, 3, 3, Color(1, 1, 1));

		Ref<GdAlBitmapGfx> dst;
		dst.instance();
		dst->_init_from_bitmap(::create_bitmap_ex(32, 16, 16), true);
		dst->clear(Color(0.5, 0.5, 0.5));

		dst->set_drawing_mode(GdAlBitmapGfx::MODE_COPY_PATTERN, pattern, 0, 0);
		dst->rectfill(0, 0, 15, 15, Color(1, 0, 0));
		dst->solid_mode();

		// Pattern white area
		Color c00 = dst->getpixel(0, 0);
		CHECK(c00.r > 0.9);
		CHECK(c00.g > 0.9);

		// Pattern black area
		Color c20 = dst->getpixel(2, 0);
		CHECK(c20.r < 0.1);
		CHECK(c20.g < 0.1);
	}

	// -----------------------------------------------------------------------
	// Clipping
	// -----------------------------------------------------------------------

	TEST_CASE("al_gfx: set_clip_rect limits drawing") {
		BITMAP *b = _test_bitmap(32, 32);
		::set_clip_rect(b, 10, 10, 20, 20);

		int c = makecol(255, 255, 255);
		// Draw a line that extends beyond clip rect
		_allegro_hline(b, 0, 15, 31, c);

		// Within clip: should be drawn
		CHECK(::getpixel(b, 15, 15) == c);
		// Outside clip: should be black
		CHECK(::getpixel(b, 5, 15) == makecol(0, 0, 0));
		CHECK(::getpixel(b, 25, 15) == makecol(0, 0, 0));

		destroy_bitmap(b);
	}

	TEST_CASE("al_gfx: get_clip_rect returns current clip") {
		BITMAP *b = _test_bitmap(64, 64);
		::set_clip_rect(b, 5, 10, 50, 40);
		int x1, y1, x2, y2;
		::get_clip_rect(b, &x1, &y1, &x2, &y2);
		CHECK(x1 == 5);
		CHECK(y1 == 10);
		CHECK(x2 == 50);
		CHECK(y2 == 40);
		destroy_bitmap(b);
	}

	// -----------------------------------------------------------------------
	// Text rendering
	// -----------------------------------------------------------------------

	TEST_CASE("al_gfx: textout_ex draws text pixels") {
		BITMAP *b = _test_bitmap(128, 16);
		int fg = makecol(255, 255, 255);
		textout_ex(b, font, "Hi", 0, 0, fg, -1);

		// At least some pixels should be white (text rendered)
		bool found_fg = false;
		for (int y = 0; y < 8 && !found_fg; y++) {
			for (int x = 0; x < 20 && !found_fg; x++) {
				if (::getpixel(b, x, y) == fg) {
					found_fg = true;
				}
			}
		}
		CHECK(found_fg);
		destroy_bitmap(b);
	}

	TEST_CASE("al_gfx: text_length returns positive for non-empty string") {
		_ensure_al_init();
		int len = text_length(font, "Hello");
		CHECK(len > 0);
		// Built-in font is 8px wide per char
		CHECK(len == 5 * 8);
	}

	TEST_CASE("al_gfx: text_height returns positive") {
		_ensure_al_init();
		int h = text_height(font);
		CHECK(h > 0);
		CHECK(h == 8); // Built-in font is 8px tall
	}

	// -----------------------------------------------------------------------
	// Fixed-point math
	// -----------------------------------------------------------------------

	TEST_CASE("al_gfx: itofix and fixtoi round-trip") {
		CHECK(fixtoi(itofix(42)) == 42);
		CHECK(fixtoi(itofix(0)) == 0);
		CHECK(fixtoi(itofix(-10)) == -10);
	}

	TEST_CASE("al_gfx: ftofix and fixtof round-trip") {
		CHECK(fixtof(ftofix(1.5)) == doctest::Approx(1.5).epsilon(0.001));
		CHECK(fixtof(ftofix(-3.25)) == doctest::Approx(-3.25).epsilon(0.001));
	}

	TEST_CASE("al_gfx: fixsin and fixcos produce correct values") {
		// 64 = 90 degrees in Allegro (256 = full circle)
		fixed sin90 = fixsin(itofix(64));
		fixed cos90 = fixcos(itofix(64));
		CHECK(fixtof(sin90) == doctest::Approx(1.0).epsilon(0.01));
		CHECK(fixtof(cos90) == doctest::Approx(0.0).epsilon(0.01));

		// 0 degrees
		fixed sin0 = fixsin(itofix(0));
		fixed cos0 = fixcos(itofix(0));
		CHECK(fixtof(sin0) == doctest::Approx(0.0).epsilon(0.01));
		CHECK(fixtof(cos0) == doctest::Approx(1.0).epsilon(0.01));
	}

	TEST_CASE("al_gfx: fixmul multiplies correctly") {
		fixed a = ftofix(3.0);
		fixed b = ftofix(4.0);
		fixed result = fixmul(a, b);
		CHECK(fixtof(result) == doctest::Approx(12.0).epsilon(0.01));
	}

	TEST_CASE("al_gfx: fixdiv divides correctly") {
		fixed a = ftofix(10.0);
		fixed b = ftofix(4.0);
		fixed result = fixdiv(a, b);
		CHECK(fixtof(result) == doctest::Approx(2.5).epsilon(0.01));
	}

	TEST_CASE("al_gfx: fixsqrt computes square root") {
		fixed val = ftofix(16.0);
		fixed result = fixsqrt(val);
		CHECK(fixtof(result) == doctest::Approx(4.0).epsilon(0.01));
	}

	// -----------------------------------------------------------------------
	// 3D triangles
	// -----------------------------------------------------------------------

	TEST_CASE("al_gfx: triangle3d_f flat-shaded fills pixels") {
		BITMAP *b = _test_bitmap(64, 64);
		int c = makecol(200, 50, 50);

		V3D_f v1, v2, v3;
		v1.x = 32;
		v1.y = 10;
		v1.z = 0;
		v1.c = c;
		v2.x = 10;
		v2.y = 50;
		v2.z = 0;
		v2.c = c;
		v3.x = 54;
		v3.y = 50;
		v3.z = 0;
		v3.c = c;
		triangle3d_f(b, POLYTYPE_FLAT, nullptr, &v1, &v2, &v3);

		// Centroid area should be filled
		CHECK(::getpixel(b, 32, 35) == c);
		// Outside should be black
		CHECK(::getpixel(b, 0, 0) == makecol(0, 0, 0));

		destroy_bitmap(b);
	}

	TEST_CASE("al_gfx: triangle3d_f gouraud produces gradient") {
		BITMAP *b = _test_bitmap(128, 128);
		V3D_f v1, v2, v3;
		v1.x = 64;
		v1.y = 10;
		v1.z = 0;
		v1.c = makecol(255, 0, 0);
		v2.x = 10;
		v2.y = 110;
		v2.z = 0;
		v2.c = makecol(0, 255, 0);
		v3.x = 118;
		v3.y = 110;
		v3.z = 0;
		v3.c = makecol(0, 0, 255);
		// POLYTYPE_GCOL extracts R/G/B via getr_depth (works with makecol).
		// POLYTYPE_GRGB expects raw (R<<16|G<<8|B) packing instead.
		triangle3d_f(b, POLYTYPE_GCOL, nullptr, &v1, &v2, &v3);

		// Near top vertex should be reddish
		int top = ::getpixel(b, 64, 20);
		CHECK(getr_depth(32, top) > 128);

		// Near bottom-left should be greenish
		int bl = ::getpixel(b, 30, 100);
		CHECK(getg_depth(32, bl) > 64);

		destroy_bitmap(b);
	}

	// -----------------------------------------------------------------------
	// Spline
	// -----------------------------------------------------------------------

	TEST_CASE("al_gfx: spline draws curve through control points") {
		BITMAP *b = _test_bitmap(128, 128);
		int c = makecol(255, 128, 255);
		int pts[8] = { 10, 64, 40, 10, 90, 118, 118, 64 };
		::spline(b, pts, c);

		// Start and end points should have colored pixels nearby
		bool found_start = false, found_end = false;
		for (int dy = -2; dy <= 2 && !found_start; dy++) {
			for (int dx = -2; dx <= 2 && !found_start; dx++) {
				if (::getpixel(b, 10 + dx, 64 + dy) == c)
					found_start = true;
			}
		}
		for (int dy = -2; dy <= 2 && !found_end; dy++) {
			for (int dx = -2; dx <= 2 && !found_end; dx++) {
				if (::getpixel(b, 118 + dx, 64 + dy) == c)
					found_end = true;
			}
		}
		CHECK(found_start);
		CHECK(found_end);

		destroy_bitmap(b);
	}

	// -----------------------------------------------------------------------
	// Arc
	// -----------------------------------------------------------------------

	TEST_CASE("al_gfx: arc draws partial circle") {
		BITMAP *b = _test_bitmap(64, 64);
		int c = makecol(0, 255, 128);
		// Draw arc from 0 to 64 (0 to 90 degrees), radius 20
		::arc(b, 32, 32, itofix(0), itofix(64), 20, c);

		// Should have pixels in first quadrant (right and above center)
		bool found = false;
		for (int x = 33; x <= 52 && !found; x++) {
			if (::getpixel(b, x, 32) == c)
				found = true;
		}
		CHECK(found);

		destroy_bitmap(b);
	}

	// -----------------------------------------------------------------------
	// Rotation
	// -----------------------------------------------------------------------

	TEST_CASE("al_gfx: rotate_sprite produces output") {
		BITMAP *sprite = _test_bitmap(16, 16);
		BITMAP *dst = _test_bitmap(64, 64);

		clear_to_color(sprite, makecol(255, 0, 255)); // mask
		::rectfill(sprite, 4, 4, 11, 11, makecol(0, 200, 255));

		::rotate_sprite(dst, sprite, 24, 24, itofix(32)); // 45 degrees

		// Some non-black pixels should exist around center
		bool found = false;
		for (int y = 20; y < 48 && !found; y++) {
			for (int x = 20; x < 48 && !found; x++) {
				if (::getpixel(dst, x, y) != makecol(0, 0, 0))
					found = true;
			}
		}
		CHECK(found);

		destroy_bitmap(sprite);
		destroy_bitmap(dst);
	}

	TEST_CASE("al_gfx: pivot_sprite rotates around pivot point") {
		_ensure_al_init();
		BITMAP *sprite = ::create_bitmap_ex(32, 16, 16);
		BITMAP *dst = ::create_bitmap_ex(32, 64, 64);

		clear_to_color(sprite, makecol(255, 0, 255)); // mask
		::rectfill(sprite, 0, 0, 15, 15, makecol(0, 200, 100));
		clear_to_color(dst, makecol(0, 0, 0));

		// Pivot at center of sprite (8,8), placed at (32,32), rotated 45 degrees
		::pivot_sprite(dst, sprite, 32, 32, 8, 8, itofix(32));

		// Some non-black pixels should appear around the placement point
		bool found = false;
		for (int y = 16; y < 48 && !found; y++) {
			for (int x = 16; x < 48 && !found; x++) {
				if (::getpixel(dst, x, y) != makecol(0, 0, 0))
					found = true;
			}
		}
		CHECK(found);

		destroy_bitmap(sprite);
		destroy_bitmap(dst);
	}

	TEST_CASE("al_gfx: pivot_scaled_sprite scales and rotates") {
		_ensure_al_init();
		BITMAP *sprite = ::create_bitmap_ex(32, 8, 8);
		BITMAP *dst = ::create_bitmap_ex(32, 64, 64);

		clear_to_color(sprite, makecol(255, 128, 0));
		clear_to_color(dst, makecol(0, 0, 0));

		// 2x scale, no rotation, pivot at sprite center
		::pivot_scaled_sprite(dst, sprite, 32, 32, 4, 4, itofix(0), ftofix(2.0));

		// At 2x scale an 8x8 sprite becomes 16x16, centered at (32,32)
		// Check that the scaled area has non-black pixels
		bool found = false;
		for (int y = 24; y < 40 && !found; y++) {
			for (int x = 24; x < 40 && !found; x++) {
				if (::getpixel(dst, x, y) != makecol(0, 0, 0))
					found = true;
			}
		}
		CHECK(found);

		// Far corner should still be black (not affected by 2x scale)
		CHECK(::getpixel(dst, 0, 0) == makecol(0, 0, 0));
		CHECK(::getpixel(dst, 63, 63) == makecol(0, 0, 0));

		destroy_bitmap(sprite);
		destroy_bitmap(dst);
	}

	TEST_CASE("GdAlBitmapGfx: pivot_sprite wrapper") {
		_ensure_al_init();
		Ref<GdAlBitmapGfx> dst;
		dst.instance();
		dst->_init_from_bitmap(::create_bitmap_ex(32, 64, 64), true);
		dst->clear(Color(0, 0, 0));

		Ref<GdAlBitmapGfx> sprite;
		sprite.instance();
		sprite->_init_from_bitmap(::create_bitmap_ex(32, 16, 16), true);
		sprite->clear(Color(0, 0.8, 0.4));

		// Place at (32,32), pivot at sprite center (8,8), rotate 90 degrees
		dst->pivot_sprite(sprite, Vector2(32, 32), Vector2(8, 8), 90.0);

		// Should have drawn something near center
		bool found = false;
		for (int y = 20; y < 44 && !found; y++) {
			for (int x = 20; x < 44 && !found; x++) {
				Color px = dst->getpixel(x, y);
				if (px.g > 0.3)
					found = true;
			}
		}
		CHECK(found);
	}

	TEST_CASE("GdAlBitmapGfx: pivot_scaled_sprite wrapper") {
		_ensure_al_init();
		Ref<GdAlBitmapGfx> dst;
		dst.instance();
		dst->_init_from_bitmap(::create_bitmap_ex(32, 64, 64), true);
		dst->clear(Color(0, 0, 0));

		Ref<GdAlBitmapGfx> sprite;
		sprite.instance();
		sprite->_init_from_bitmap(::create_bitmap_ex(32, 8, 8), true);
		sprite->clear(Color(1, 0.5, 0));

		// 2x scale, no rotation
		dst->pivot_scaled_sprite(sprite, Vector2(32, 32), Vector2(4, 4), 0.0, 2.0);

		// Should have orange pixels in center area
		Color center = dst->getpixel(32, 32);
		CHECK(center.r > 0.5);

		// Corners should be untouched
		Color corner = dst->getpixel(0, 0);
		CHECK(corner.r == doctest::Approx(0.0).epsilon(0.02));
	}

	// -----------------------------------------------------------------------
	// Godot wrapper: GdAlBitmapGfx
	// -----------------------------------------------------------------------

	TEST_CASE("GdAlBitmapGfx: default is invalid") {
		Ref<GdAlBitmapGfx> gfx;
		gfx.instance();
		CHECK(!gfx->is_valid());
		CHECK(gfx->get_width() == 0);
		CHECK(gfx->get_height() == 0);
	}

	TEST_CASE("GdAlBitmapGfx: init_from_bitmap makes it valid") {
		_ensure_al_init();
		BITMAP *b = ::create_bitmap(32, 24);
		REQUIRE(b != nullptr);

		Ref<GdAlBitmapGfx> gfx;
		gfx.instance();
		gfx->_init_from_bitmap(b, true);

		CHECK(gfx->is_valid());
		CHECK(gfx->get_width() == 32);
		CHECK(gfx->get_height() == 24);
		CHECK(gfx->get_color_depth() == 32);
	}

	TEST_CASE("GdAlBitmapGfx: clear and getpixel") {
		_ensure_al_init();
		BITMAP *b = ::create_bitmap(8, 8);
		Ref<GdAlBitmapGfx> gfx;
		gfx.instance();
		gfx->_init_from_bitmap(b, true);

		gfx->clear(Color(1, 0, 0));
		Color c = gfx->getpixel(4, 4);
		CHECK(c.r == doctest::Approx(1.0).epsilon(0.02));
		CHECK(c.g == doctest::Approx(0.0).epsilon(0.02));
		CHECK(c.b == doctest::Approx(0.0).epsilon(0.02));
	}

	TEST_CASE("GdAlBitmapGfx: putpixel and getpixel round-trip") {
		_ensure_al_init();
		BITMAP *b = ::create_bitmap(8, 8);
		clear_to_color(b, makecol(0, 0, 0));
		Ref<GdAlBitmapGfx> gfx;
		gfx.instance();
		gfx->_init_from_bitmap(b, true);

		Color input(0.5, 0.75, 0.25);
		gfx->putpixel(3, 3, input);
		Color output = gfx->getpixel(3, 3);
		// 8-bit per channel precision
		CHECK(output.r == doctest::Approx(input.r).epsilon(0.01));
		CHECK(output.g == doctest::Approx(input.g).epsilon(0.01));
		CHECK(output.b == doctest::Approx(input.b).epsilon(0.01));
	}

	TEST_CASE("GdAlBitmapGfx: line drawing") {
		_ensure_al_init();
		BITMAP *b = ::create_bitmap(32, 32);
		clear_to_color(b, makecol(0, 0, 0));
		Ref<GdAlBitmapGfx> gfx;
		gfx.instance();
		gfx->_init_from_bitmap(b, true);

		Color c(0, 1, 0);
		gfx->hline(0, 5, 31, c);
		gfx->vline(10, 0, 31, c);

		Color px1 = gfx->getpixel(15, 5);
		CHECK(px1.g == doctest::Approx(1.0).epsilon(0.02));

		Color px2 = gfx->getpixel(10, 20);
		CHECK(px2.g == doctest::Approx(1.0).epsilon(0.02));
	}

	TEST_CASE("GdAlBitmapGfx: get_image produces valid Image") {
		_ensure_al_init();
		BITMAP *b = ::create_bitmap(16, 16);
		clear_to_color(b, makecol(0, 0, 0));
		Ref<GdAlBitmapGfx> gfx;
		gfx.instance();
		gfx->_init_from_bitmap(b, true);

		// Draw a red rectangle
		gfx->rectfill(0, 0, 15, 15, Color(1, 0, 0));

		Ref<Image> img = gfx->get_image();
		REQUIRE(img.is_valid());
		CHECK(img->get_width() == 16);
		CHECK(img->get_height() == 16);
		CHECK(img->get_format() == Image::FORMAT_RGBA8);

		img->lock();
		Color px = img->get_pixel(8, 8);
		img->unlock();
		CHECK(px.r == doctest::Approx(1.0).epsilon(0.02));
		CHECK(px.g == doctest::Approx(0.0).epsilon(0.02));
		CHECK(px.b == doctest::Approx(0.0).epsilon(0.02));
	}

	TEST_CASE("GdAlBitmapGfx: create_sub_bitmap shares pixels") {
		_ensure_al_init();
		BITMAP *b = ::create_bitmap(32, 32);
		clear_to_color(b, makecol(0, 0, 0));
		Ref<GdAlBitmapGfx> parent;
		parent.instance();
		parent->_init_from_bitmap(b, true);

		Ref<GdAlBitmapGfx> sub = parent->create_sub_bitmap(8, 8, 16, 16);
		REQUIRE(sub.is_valid());
		CHECK(sub->get_width() == 16);
		CHECK(sub->get_height() == 16);

		// Draw on sub-bitmap
		sub->putpixel(0, 0, Color(1, 1, 0));

		// Should be visible on parent at offset
		Color px = parent->getpixel(8, 8);
		CHECK(px.r == doctest::Approx(1.0).epsilon(0.02));
		CHECK(px.g == doctest::Approx(1.0).epsilon(0.02));
	}

	TEST_CASE("GdAlBitmapGfx: polygon with PoolVector2Array") {
		_ensure_al_init();
		BITMAP *b = ::create_bitmap(64, 64);
		clear_to_color(b, makecol(0, 0, 0));
		Ref<GdAlBitmapGfx> gfx;
		gfx.instance();
		gfx->_init_from_bitmap(b, true);

		PoolVector2Array verts;
		verts.push_back(Vector2(10, 10));
		verts.push_back(Vector2(50, 10));
		verts.push_back(Vector2(50, 50));
		verts.push_back(Vector2(10, 50));
		gfx->polygon(verts, Color(0, 0, 1));

		Color px = gfx->getpixel(30, 30);
		CHECK(px.b == doctest::Approx(1.0).epsilon(0.02));
	}

	TEST_CASE("GdAlBitmapGfx: spline with PoolVector2Array") {
		_ensure_al_init();
		BITMAP *b = ::create_bitmap(128, 128);
		clear_to_color(b, makecol(0, 0, 0));
		Ref<GdAlBitmapGfx> gfx;
		gfx.instance();
		gfx->_init_from_bitmap(b, true);

		PoolVector2Array pts;
		pts.push_back(Vector2(10, 64));
		pts.push_back(Vector2(40, 10));
		pts.push_back(Vector2(90, 118));
		pts.push_back(Vector2(118, 64));
		gfx->spline(pts, Color(1, 0.5, 1));

		// Should have some colored pixels
		bool found = false;
		for (int y = 0; y < 128 && !found; y++) {
			for (int x = 0; x < 128 && !found; x++) {
				Color px = gfx->getpixel(x, y);
				if (px.r > 0.5 && px.b > 0.5)
					found = true;
			}
		}
		CHECK(found);
	}

	TEST_CASE("GdAlBitmapGfx: text rendering") {
		_ensure_al_init();
		BITMAP *b = ::create_bitmap(128, 16);
		clear_to_color(b, makecol(0, 0, 0));
		Ref<GdAlBitmapGfx> gfx;
		gfx.instance();
		gfx->_init_from_bitmap(b, true);

		gfx->text("AB", 0, 0, Color(1, 1, 1));

		bool found = false;
		for (int y = 0; y < 8 && !found; y++) {
			for (int x = 0; x < 16 && !found; x++) {
				Color px = gfx->getpixel(x, y);
				if (px.r > 0.9)
					found = true;
			}
		}
		CHECK(found);
		CHECK(gfx->get_text_length("AB") == 16);
		CHECK(gfx->get_text_height() == 8);
	}

	TEST_CASE("GdAlBitmapGfx: blit_from copies pixels") {
		_ensure_al_init();
		BITMAP *sb = ::create_bitmap(8, 8);
		BITMAP *db = ::create_bitmap(16, 16);
		clear_to_color(sb, makecol(0, 0, 0));
		clear_to_color(db, makecol(0, 0, 0));

		Ref<GdAlBitmapGfx> src, dst;
		src.instance();
		dst.instance();
		src->_init_from_bitmap(sb, true);
		dst->_init_from_bitmap(db, true);

		src->rectfill(0, 0, 7, 7, Color(0, 1, 1));
		dst->blit_from(src, Rect2(0, 0, 8, 8), Vector2(4, 4));

		Color px = dst->getpixel(6, 6);
		CHECK(px.g == doctest::Approx(1.0).epsilon(0.02));
		CHECK(px.b == doctest::Approx(1.0).epsilon(0.02));
	}

	TEST_CASE("GdAlBitmapGfx: triangle3d_flat draws filled triangle (Vector3)") {
		_ensure_al_init();
		BITMAP *b = ::create_bitmap(64, 64);
		clear_to_color(b, makecol(0, 0, 0));
		Ref<GdAlBitmapGfx> gfx;
		gfx.instance();
		gfx->_init_from_bitmap(b, true);

		gfx->triangle3d_flat(Vector3(32, 10, 0), Vector3(10, 50, 0), Vector3(54, 50, 0), Color(1, 0, 0));

		Color px = gfx->getpixel(32, 35);
		CHECK(px.r == doctest::Approx(1.0).epsilon(0.02));
	}

	TEST_CASE("GdAlBitmapGfx: triangle3d_gouraud produces color gradient (Vector3)") {
		_ensure_al_init();
		BITMAP *b = ::create_bitmap(128, 128);
		clear_to_color(b, makecol(0, 0, 0));
		Ref<GdAlBitmapGfx> gfx;
		gfx.instance();
		gfx->_init_from_bitmap(b, true);

		PoolVector3Array verts;
		verts.push_back(Vector3(64, 10, 0));
		verts.push_back(Vector3(10, 110, 0));
		verts.push_back(Vector3(118, 110, 0));
		PoolColorArray cols;
		cols.push_back(Color(1, 0, 0));
		cols.push_back(Color(0, 1, 0));
		cols.push_back(Color(0, 0, 1));
		gfx->triangle3d_gouraud(verts, cols);

		// Near top vertex: should be reddish
		Color top = gfx->getpixel(64, 20);
		CHECK(top.r > 0.5);
	}

	TEST_CASE("GdAlBitmapGfx: quad3d_flat draws filled quad") {
		_ensure_al_init();
		BITMAP *b = ::create_bitmap(64, 64);
		clear_to_color(b, makecol(0, 0, 0));
		Ref<GdAlBitmapGfx> gfx;
		gfx.instance();
		gfx->_init_from_bitmap(b, true);

		PoolVector3Array verts;
		verts.push_back(Vector3(10, 10, 0));
		verts.push_back(Vector3(50, 10, 0));
		verts.push_back(Vector3(50, 50, 0));
		verts.push_back(Vector3(10, 50, 0));
		gfx->quad3d_flat(verts, Color(0, 1, 0));

		Color center = gfx->getpixel(30, 30);
		CHECK(center.g == doctest::Approx(1.0).epsilon(0.02));

		// Outside the quad should be black
		Color outside = gfx->getpixel(5, 5);
		CHECK(outside.r == doctest::Approx(0.0).epsilon(0.02));
		CHECK(outside.g == doctest::Approx(0.0).epsilon(0.02));
	}

	TEST_CASE("GdAlBitmapGfx: quad3d_gouraud produces gradient") {
		_ensure_al_init();
		BITMAP *b = ::create_bitmap(64, 64);
		clear_to_color(b, makecol(0, 0, 0));
		Ref<GdAlBitmapGfx> gfx;
		gfx.instance();
		gfx->_init_from_bitmap(b, true);

		PoolVector3Array verts;
		verts.push_back(Vector3(10, 10, 0));
		verts.push_back(Vector3(50, 10, 0));
		verts.push_back(Vector3(50, 50, 0));
		verts.push_back(Vector3(10, 50, 0));
		PoolColorArray cols;
		cols.push_back(Color(1, 0, 0));
		cols.push_back(Color(0, 1, 0));
		cols.push_back(Color(0, 0, 1));
		cols.push_back(Color(1, 1, 0));
		gfx->quad3d_gouraud(verts, cols);

		// Center should have some mix of colors (not black)
		Color center = gfx->getpixel(30, 30);
		CHECK((center.r + center.g + center.b) > 0.3);
	}

	TEST_CASE("GdAlBitmapGfx: Z-buffer lifecycle") {
		_ensure_al_init();
		BITMAP *b = ::create_bitmap(32, 32);
		clear_to_color(b, makecol(0, 0, 0));
		Ref<GdAlBitmapGfx> gfx;
		gfx.instance();
		gfx->_init_from_bitmap(b, true);

		CHECK(!gfx->has_zbuffer());

		gfx->create_zbuffer();
		CHECK(gfx->has_zbuffer());

		gfx->clear_zbuffer(0.0f);
		gfx->enable_zbuffer();
		gfx->disable_zbuffer();

		gfx->destroy_zbuffer();
		CHECK(!gfx->has_zbuffer());
	}

	TEST_CASE("GdAlBitmapGfx: Z-buffer depth ordering") {
		_ensure_al_init();
		BITMAP *b = ::create_bitmap(64, 64);
		clear_to_color(b, makecol(0, 0, 0));
		Ref<GdAlBitmapGfx> gfx;
		gfx.instance();
		gfx->_init_from_bitmap(b, true);

		gfx->create_zbuffer();
		gfx->clear_zbuffer(0.0f);

		// Allegro Z-buffer stores 1/z: higher z = larger 1/z = closer.
		// Draw a red triangle at z=0.5 (closer, 1/z=2.0)
		gfx->triangle3d_flat(
				Vector3(10, 10, 0.5f), Vector3(50, 10, 0.5f), Vector3(30, 50, 0.5f),
				Color(1, 0, 0));

		// Draw a blue triangle at z=0.3 (even closer, 1/z=3.33) — should overwrite red
		gfx->triangle3d_flat(
				Vector3(10, 10, 0.3f), Vector3(50, 10, 0.3f), Vector3(30, 50, 0.3f),
				Color(0, 0, 1));

		// The overlapping area should be blue (higher 1/z wins)
		Color px = gfx->getpixel(30, 25);
		CHECK(px.b > 0.5);
		CHECK(px.r < 0.1);

		// Now draw a green triangle at z=0.1 (closest, 1/z=10) — should overwrite blue
		gfx->triangle3d_flat(
				Vector3(10, 10, 0.1f), Vector3(50, 10, 0.1f), Vector3(30, 50, 0.1f),
				Color(0, 1, 0));

		Color px2 = gfx->getpixel(30, 25);
		CHECK(px2.g > 0.5);

		// Draw yellow at z=0.8 (farther, 1/z=1.25) — should NOT overwrite
		gfx->triangle3d_flat(
				Vector3(10, 10, 0.8f), Vector3(50, 10, 0.8f), Vector3(30, 50, 0.8f),
				Color(1, 1, 0));

		Color px3 = gfx->getpixel(30, 25);
		CHECK(px3.g > 0.5); // still green
		CHECK(px3.r < 0.1); // no yellow

		gfx->destroy_zbuffer();
	}

	TEST_CASE("GdAlBitmapGfx: triangle3d generic with POLY_FLAT") {
		_ensure_al_init();
		BITMAP *b = ::create_bitmap(64, 64);
		clear_to_color(b, makecol(0, 0, 0));
		Ref<GdAlBitmapGfx> gfx;
		gfx.instance();
		gfx->_init_from_bitmap(b, true);

		PoolVector3Array verts;
		verts.push_back(Vector3(32, 5, 0));
		verts.push_back(Vector3(5, 55, 0));
		verts.push_back(Vector3(59, 55, 0));
		PoolColorArray cols;
		cols.push_back(Color(0, 1, 0));
		cols.push_back(Color(0, 1, 0));
		cols.push_back(Color(0, 1, 0));
		gfx->triangle3d(GdAlBitmapGfx::POLY_FLAT, verts, cols);

		Color px = gfx->getpixel(32, 30);
		CHECK(px.g == doctest::Approx(1.0).epsilon(0.02));
	}

	TEST_CASE("GdAlBitmapGfx: render_demo does not crash on large bitmap") {
		_ensure_al_init();
		// Demo requires at least 800x630
		BITMAP *b = ::create_bitmap(800, 630);
		REQUIRE(b != nullptr);
		clear_to_color(b, makecol(0, 0, 0));

		Ref<GdAlBitmapGfx> gfx;
		gfx.instance();
		gfx->_init_from_bitmap(b, true);

		// Should not crash
		gfx->render_demo();

		// Title area should have some non-black pixels
		bool found = false;
		for (int x = 300; x < 500 && !found; x++) {
			Color px = gfx->getpixel(x, 10);
			if (px.r > 0.5 || px.g > 0.5 || px.b > 0.5)
				found = true;
		}
		CHECK(found);
	}

	TEST_CASE("GdAlBitmapGfx: clip_rect limits drawing") {
		_ensure_al_init();
		BITMAP *b = ::create_bitmap(32, 32);
		clear_to_color(b, makecol(0, 0, 0));
		Ref<GdAlBitmapGfx> gfx;
		gfx.instance();
		gfx->_init_from_bitmap(b, true);

		gfx->set_clip_rect(10, 10, 20, 20);
		Rect2 cr = gfx->get_clip_rect();
		CHECK(cr.position.x == 10);
		CHECK(cr.position.y == 10);

		gfx->rectfill(0, 0, 31, 31, Color(1, 1, 1));

		// Inside clip: drawn
		Color inside = gfx->getpixel(15, 15);
		CHECK(inside.r == doctest::Approx(1.0).epsilon(0.02));

		// Outside clip: not drawn
		Color outside = gfx->getpixel(5, 5);
		CHECK(outside.r == doctest::Approx(0.0).epsilon(0.02));
	}

	TEST_CASE("GdAlBitmapGfx: get_image scanline optimization — 32-bit RGBA roundtrip") {
		_ensure_al_init();
		BITMAP *b = ::create_bitmap_ex(32, 4, 4);
		REQUIRE(b);

		// Write known pixels directly via scanline
		for (int y = 0; y < 4; y++) {
			int *row = (int *)b->line[y];
			for (int x = 0; x < 4; x++) {
				int r = (y * 4 + x) * 15;
				int g = 255 - r;
				int b_val = (x * 60) & 0xFF;
				int a = 200 + x;
				row[x] = makeacol32(r, g, b_val, a);
			}
		}

		Ref<GdAlBitmapGfx> gfx;
		gfx.instance();
		gfx->_init_from_bitmap(b, true);

		Ref<Image> img = gfx->get_image();
		REQUIRE(img.is_valid());
		CHECK(img->get_width() == 4);
		CHECK(img->get_height() == 4);

		// Verify every pixel matches
		img->lock();
		for (int y = 0; y < 4; y++) {
			for (int x = 0; x < 4; x++) {
				int r = (y * 4 + x) * 15;
				int g = 255 - r;
				int b_val = (x * 60) & 0xFF;
				int a = 200 + x;
				Color px = img->get_pixel(x, y);
				CHECK(px.r == doctest::Approx(r / 255.0).epsilon(0.02));
				CHECK(px.g == doctest::Approx(g / 255.0).epsilon(0.02));
				CHECK(px.b == doctest::Approx(b_val / 255.0).epsilon(0.02));
				CHECK(px.a == doctest::Approx(a / 255.0).epsilon(0.02));
			}
		}
		img->unlock();
	}

	TEST_CASE("AlBitmapGfx: from_image scanline optimization — roundtrip preserves pixels") {
		_ensure_al_init();

		// Create a Godot Image with known RGBA data
		int w = 8, h = 8;
		PoolByteArray data;
		data.resize(w * h * 4);
		{
			PoolByteArray::Write wd = data.write();
			for (int y = 0; y < h; y++) {
				for (int x = 0; x < w; x++) {
					int idx = (y * w + x) * 4;
					wd[idx + 0] = (x * 30) & 0xFF; // R
					wd[idx + 1] = (y * 30) & 0xFF; // G
					wd[idx + 2] = ((x + y) * 20) & 0xFF; // B
					wd[idx + 3] = 128 + (x & 0x7F); // A
				}
			}
		}

		Ref<Image> src_img;
		src_img.instance();
		src_img->create(w, h, false, Image::FORMAT_RGBA8, data);
		REQUIRE(src_img.is_valid());

		// Convert to al_gfx bitmap via from_image
		AlBitmapGfx *factory = AlBitmapGfx::get_singleton();
		REQUIRE(factory);
		Ref<GdAlBitmapGfx> gfx = factory->from_image(src_img);
		REQUIRE(gfx.is_valid());
		CHECK(gfx->get_width() == w);
		CHECK(gfx->get_height() == h);

		// Convert back via get_image
		Ref<Image> result = gfx->get_image();
		REQUIRE(result.is_valid());

		// Verify roundtrip pixel accuracy
		result->lock();
		src_img->lock();
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				Color orig = src_img->get_pixel(x, y);
				Color rt = result->get_pixel(x, y);
				CHECK(rt.r == doctest::Approx(orig.r).epsilon(0.02));
				CHECK(rt.g == doctest::Approx(orig.g).epsilon(0.02));
				CHECK(rt.b == doctest::Approx(orig.b).epsilon(0.02));
				CHECK(rt.a == doctest::Approx(orig.a).epsilon(0.02));
			}
		}
		result->unlock();
		src_img->unlock();
	}

	TEST_CASE("GdAlBitmapGfx: get_image works with non-32-bit depth") {
		_ensure_al_init();
		// Test 16-bit depth — uses the general scanline path
		BITMAP *b = ::create_bitmap_ex(16, 8, 8);
		REQUIRE(b);
		::clear_to_color(b, makecol_depth(16, 0, 0, 0));

		// Draw some colored pixels via putpixel
		::putpixel(b, 0, 0, makecol_depth(16, 255, 0, 0));
		::putpixel(b, 1, 0, makecol_depth(16, 0, 255, 0));
		::putpixel(b, 2, 0, makecol_depth(16, 0, 0, 255));

		Ref<GdAlBitmapGfx> gfx;
		gfx.instance();
		gfx->_init_from_bitmap(b, true);

		Ref<Image> img = gfx->get_image();
		REQUIRE(img.is_valid());
		CHECK(img->get_format() == Image::FORMAT_RGBA8);

		img->lock();
		// Red pixel (16-bit has lower precision, allow wider epsilon)
		Color r = img->get_pixel(0, 0);
		CHECK(r.r > 0.9);
		CHECK(r.g < 0.1);
		CHECK(r.b < 0.1);
		CHECK(r.a == doctest::Approx(1.0)); // non-32-bit always 255 alpha

		// Green pixel
		Color g = img->get_pixel(1, 0);
		CHECK(g.r < 0.1);
		CHECK(g.g > 0.9);
		CHECK(g.b < 0.1);

		// Blue pixel
		Color bl = img->get_pixel(2, 0);
		CHECK(bl.r < 0.1);
		CHECK(bl.g < 0.1);
		CHECK(bl.b > 0.9);
		img->unlock();
	}

	TEST_CASE("al_gfx: get_rle_sprite creates compressed sprite") {
		_ensure_al_init();
		BITMAP *src = ::create_bitmap_ex(32, 16, 16);
		// Fill with mask color (transparent) except a small rect
		clear_to_color(src, makecol(255, 0, 255));
		::rectfill(src, 4, 4, 11, 11, makecol(0, 200, 100));

		RLE_SPRITE *rle = ::get_rle_sprite(src);
		REQUIRE(rle != nullptr);
		CHECK(rle->w == 16);
		CHECK(rle->h == 16);
		CHECK(rle->color_depth == 32);
		CHECK(rle->size > 0);
		// RLE should be smaller than raw bitmap (16*16*4 = 1024 bytes)
		CHECK(rle->size < 1024);

		::destroy_rle_sprite(rle);
		::destroy_bitmap(src);
	}

	TEST_CASE("al_gfx: draw_rle_sprite renders correctly") {
		_ensure_al_init();
		BITMAP *src = ::create_bitmap_ex(32, 8, 8);
		clear_to_color(src, makecol(255, 0, 255)); // mask
		::rectfill(src, 2, 2, 5, 5, makecol(0, 255, 0)); // green center

		RLE_SPRITE *rle = ::get_rle_sprite(src);
		REQUIRE(rle);

		BITMAP *dst = ::create_bitmap_ex(32, 32, 32);
		clear_to_color(dst, makecol(0, 0, 0));

		::draw_rle_sprite(dst, rle, 10, 10);

		// Green pixels should appear in the drawn area
		int c = ::getpixel(dst, 12, 12);
		CHECK(getg32(c) == 255);
		CHECK(getr32(c) == 0);

		// Masked area should remain black
		int bg = ::getpixel(dst, 10, 10);
		CHECK(bg == makecol(0, 0, 0));

		// Outside sprite area should be black
		CHECK(::getpixel(dst, 0, 0) == makecol(0, 0, 0));

		::destroy_rle_sprite(rle);
		::destroy_bitmap(src);
		::destroy_bitmap(dst);
	}

	TEST_CASE("GdAlBitmapGfx: get_rle_sprite and draw_rle_sprite wrapper") {
		_ensure_al_init();
		Ref<GdAlBitmapGfx> src;
		src.instance();
		src->_init_from_bitmap(::create_bitmap_ex(32, 16, 16), true);

		// Fill with mask color, draw green rect
		BITMAP *sb = src->_get_bitmap();
		clear_to_color(sb, makecol(255, 0, 255));
		::rectfill(sb, 4, 4, 11, 11, makecol(0, 200, 100));

		Ref<GdAlRleSprite> rle = src->get_rle_sprite();
		REQUIRE(rle.is_valid());
		CHECK(rle->is_valid());
		CHECK(rle->get_width() == 16);
		CHECK(rle->get_height() == 16);
		CHECK(rle->get_color_depth() == 32);
		CHECK(rle->get_size() > 0);

		// Draw RLE sprite onto destination
		Ref<GdAlBitmapGfx> dst;
		dst.instance();
		dst->_init_from_bitmap(::create_bitmap_ex(32, 32, 32), true);
		dst->clear(Color(0, 0, 0));

		dst->draw_rle_sprite(rle, 4, 4);

		// Check drawn pixel
		Color px = dst->getpixel(8, 8);
		CHECK(px.g > 0.5);

		// Check masked area left background
		Color bg = dst->getpixel(4, 4);
		CHECK(bg.r == doctest::Approx(0.0).epsilon(0.02));
		CHECK(bg.g == doctest::Approx(0.0).epsilon(0.02));
		CHECK(bg.b == doctest::Approx(0.0).epsilon(0.02));
	}

	// -----------------------------------------------------------------------
	// Font from bitmap
	// -----------------------------------------------------------------------

	// Helper: create a minimal bitmap font with 2 characters ('A' and 'B').
	// Layout: yellow separator border around two 8x8 character cells.
	// For 32-bit truecolor, separator is yellow (255,255,0).
	//
	//  Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y
	//  Y . . . . . . . . Y . . . . . . . . Y
	//  Y . . . . . . . . Y . . . . . . . . Y
	//  Y . (A glyph) . . Y . (B glyph) . . Y
	//  ...
	//  Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y Y
	//
	static BITMAP *_make_test_font_bitmap() {
		// 2 chars, each 6x8, with 1px separator borders
		// Total: (1 + 6 + 1 + 6 + 1) x (1 + 8 + 1) = 15 x 10
		int cw = 6, ch = 8;
		int bw = 1 + cw + 1 + cw + 1; // 15
		int bh = 1 + ch + 1; // 10

		BITMAP *bmp = ::create_bitmap_ex(32, bw, bh);
		int yellow = makecol(255, 255, 0);
		int white = makecol(255, 255, 255);

		// Fill all yellow (separator)
		clear_to_color(bmp, yellow);

		// Clear character cells to black
		// Cell 0: x=1..6, y=1..8
		::rectfill(bmp, 1, 1, cw, ch, 0);
		// Cell 1: x=8..13, y=1..8
		::rectfill(bmp, cw + 2, 1, cw + 2 + cw - 1, ch, 0);

		// Draw a simple 'space' glyph (cell 0 — just black, since range starts at ' ')
		// Actually the first char IS space (ASCII 32). Let's put patterns in them
		// to verify font extraction works.

		// Cell 0 (space): leave black — no pixels
		// Cell 1 ('!'): draw a vertical line
		for (int y = 1; y <= 6; y++) {
			putpixel(bmp, cw + 2 + 2, y, white);
		}
		// Dot at bottom
		putpixel(bmp, cw + 2 + 2, 8, white);

		return bmp;
	}

	TEST_CASE("al_gfx: grab_font_from_bitmap creates font") {
		_ensure_al_init();
		BITMAP *fb = _make_test_font_bitmap();
		REQUIRE(fb);

		FONT *f = ::grab_font_from_bitmap(fb);
		REQUIRE(f);

		// 32-bit bitmaps always produce color fonts (mono detection uses
		// raw pixel value 255, which only works for 8-bit paletted bitmaps)
		CHECK(::is_color_font(f));

		// Height should match cell height (8)
		CHECK(::text_height(f) == 8);

		// Text length for "!" should equal the glyph width
		int len = ::text_length(f, "!");
		CHECK(len == 6);

		// Render with the font and verify pixels appear
		BITMAP *canvas = ::create_bitmap_ex(32, 32, 16);
		clear_to_color(canvas, 0);
		textout_ex(canvas, f, "!", 0, 0, makecol(255, 255, 255), -1);

		// Should have some white pixels from the '!' glyph
		bool found_white = false;
		for (int y = 0; y < 10 && !found_white; y++) {
			for (int x = 0; x < 10 && !found_white; x++) {
				if (getpixel(canvas, x, y) == makecol(255, 255, 255))
					found_white = true;
			}
		}
		CHECK(found_white);

		::destroy_font(f);
		::destroy_bitmap(canvas);
		::destroy_bitmap(fb);
	}

	TEST_CASE("GdAlFont: wrapper creation and queries") {
		_ensure_al_init();
		BITMAP *fb = _make_test_font_bitmap();
		REQUIRE(fb);

		FONT *f = ::grab_font_from_bitmap(fb);
		REQUIRE(f);

		Ref<GdAlFont> gf;
		gf.instance();
		gf->_init_from_font(f);

		CHECK(gf->is_valid());
		CHECK(!gf->is_mono()); // 32-bit bitmap → color font
		CHECK(gf->is_color());
		CHECK(gf->get_height() == 8);
		CHECK(gf->get_length("!") == 6);

		::destroy_bitmap(fb);
		// font is owned by GdAlFont — will be freed on destruction
	}

	TEST_CASE("GdAlBitmapGfx: set_font uses custom font for text rendering") {
		_ensure_al_init();
		BITMAP *fb = _make_test_font_bitmap();
		REQUIRE(fb);

		FONT *f = ::grab_font_from_bitmap(fb);
		REQUIRE(f);

		Ref<GdAlFont> gf;
		gf.instance();
		gf->_init_from_font(f);

		// Create canvas
		Ref<GdAlBitmapGfx> canvas;
		canvas.instance();
		canvas->_init_from_bitmap(::create_bitmap_ex(32, 64, 16), true);
		canvas->clear(Color(0, 0, 0));

		// Set custom font and draw
		canvas->set_font(gf);
		CHECK(canvas->get_font() == gf);

		// Text height should now reflect custom font
		CHECK(canvas->get_text_height() == 8);
		CHECK(canvas->get_text_length("!") == 6);

		canvas->text("!", 0, 0, Color(1, 1, 1));

		// Verify pixels were drawn
		bool found = false;
		for (int y = 0; y < 10 && !found; y++) {
			for (int x = 0; x < 10 && !found; x++) {
				Color px = canvas->getpixel(x, y);
				if (px.r > 0.5 && px.g > 0.5 && px.b > 0.5)
					found = true;
			}
		}
		CHECK(found);

		// Reset font to built-in
		canvas->set_font(Ref<GdAlFont>());
		CHECK(canvas->get_text_height() == 8); // built-in is also 8px

		::destroy_bitmap(fb);
	}

	TEST_CASE("AlBitmapGfx: font_from_bitmap creates GdAlFont from wrapper bitmap") {
		_ensure_al_init();

		// Create the test font bitmap via wrapper
		Ref<GdAlBitmapGfx> fb_wrap;
		fb_wrap.instance();
		fb_wrap->_init_from_bitmap(_make_test_font_bitmap(), true);

		// Use existing singleton registered by register_types
		AlBitmapGfx *factory = AlBitmapGfx::get_singleton();
		REQUIRE(factory);
		Ref<GdAlFont> gf = factory->font_from_bitmap(fb_wrap);

		REQUIRE(gf.is_valid());
		CHECK(gf->is_valid());
		CHECK(gf->is_color()); // 32-bit bitmap → color font
		CHECK(gf->get_height() == 8);
	}

} // TEST_SUITE("albmpgfx")

#endif // DOCTEST
