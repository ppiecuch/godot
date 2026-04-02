/**************************************************************************/
/*  ssfn_font.cpp                                                         */
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

#include "ssfn_font.h"

#include "core/io/resource_loader.h"
#include "core/os/file_access.h"
#include "servers/visual_server.h"

#include <climits>
#include <cstdlib>
#include <cstring>

// Provide standard library functions to SSFN before including it,
// to prevent it from declaring its own conflicting prototypes.
#define SSFN_memcmp memcmp
#define SSFN_memset memset
#define SSFN_realloc realloc
#define SSFN_free free

#define SSFN_IMPLEMENTATION
#include "thirdparty/misc/ssfn.h"

// --- ShelfPackTexture ---

SSFNFont::FontTexturePosition SSFNFont::ShelfPackTexture::pack_rect(int32_t p_id, int32_t p_h, int32_t p_w) {
	int32_t y = 0;
	int32_t waste = 0;
	List<Shelf>::Element *best_shelf = nullptr;
	int32_t best_waste = INT_MAX;

	for (List<Shelf>::Element *E = shelves.front(); E; E = E->next()) {
		y += E->get().h;
		if (p_w > E->get().w) {
			continue;
		}
		if (p_h == E->get().h) {
			return E->get().alloc_shelf(p_id, p_w, p_h);
		}
		if (p_h > E->get().h) {
			continue;
		}
		if (p_h < E->get().h) {
			waste = (E->get().h - p_h) * p_w;
			if (waste < best_waste) {
				best_waste = waste;
				best_shelf = E;
			}
		}
	}
	if (best_shelf) {
		return best_shelf->get().alloc_shelf(p_id, p_w, p_h);
	}
	if (p_h <= (texture_size - y) && p_w <= texture_size) {
		List<Shelf>::Element *E = shelves.push_back(Shelf(0, y, texture_size, p_h));
		return E->get().alloc_shelf(p_id, p_w, p_h);
	}
	return FontTexturePosition(-1, 0, 0);
}

// --- SSFNFont ---

void SSFNFont::_init_ssfn() {
	if (!ssfn_ctx) {
		ssfn_ctx = memalloc(sizeof(ssfn_t));
		memset(ssfn_ctx, 0, sizeof(ssfn_t));
	}
}

void SSFNFont::_free_ssfn() {
	if (ssfn_ctx) {
		ssfn_free((ssfn_t *)ssfn_ctx);
		memfree(ssfn_ctx);
		ssfn_ctx = nullptr;
	}
}

void SSFNFont::_clear_cache() const {
	char_map.clear();
	textures.clear();
}

void SSFNFont::_update_metrics() {
	if (!valid || !ssfn_ctx) {
		_ascent = 0;
		_descent = 0;
		_height = 0;
		return;
	}

	ssfn_t *ctx = (ssfn_t *)ssfn_ctx;

	// Select the font face for current size.
	int ret = ssfn_select(ctx, SSFN_FAMILY_ANY, nullptr, font_style, font_size);
	if (ret != SSFN_OK) {
		_ascent = font_size * 0.8f;
		_descent = font_size * 0.2f;
		_height = (float)font_size;
		return;
	}

	// Render a reference glyph to get metrics.
	ssfn_buf_t buf;
	memset(&buf, 0, sizeof(ssfn_buf_t));

	// Allocate a small temp buffer for rendering.
	int buf_size = font_size * 2;
	Vector<uint8_t> tmp;
	tmp.resize(buf_size * buf_size * 4);
	memset(tmp.ptrw(), 0, tmp.size());

	buf.ptr = tmp.ptrw();
	buf.w = buf_size;
	buf.h = buf_size;
	buf.p = buf_size * 4;
	buf.x = 0;
	buf.y = font_size;
	buf.fg = 0xFFFFFFFF;

	ret = ssfn_render(ctx, &buf, "A");
	if (ret == SSFN_OK && ctx->g) {
		_ascent = (float)ctx->g->a;
		_descent = (float)ctx->g->d;
		_height = _ascent + _descent;
	} else {
		// Fallback metrics from font header.
		const ssfn_font_t *fnt = ctx->f ? ctx->f : (ctx->s ? ctx->s : nullptr);
		if (fnt) {
			float scale = (float)font_size / (float)fnt->height;
			_ascent = (float)fnt->baseline * scale;
			_descent = (float)(fnt->height - fnt->baseline) * scale;
			_height = (float)font_size;
		} else {
			_ascent = font_size * 0.8f;
			_descent = font_size * 0.2f;
			_height = (float)font_size;
		}
	}

	if (_height < 1.0f) {
		_height = (float)font_size;
	}
	if (_ascent < 1.0f) {
		_ascent = _height * 0.8f;
	}
	if (_descent < 0.0f) {
		_descent = 0.0f;
	}
}

SSFNFont::FontTexturePosition SSFNFont::_find_texture_pos_for_glyph(int p_width, int p_height) const {
	// Try to fit in existing textures.
	for (int i = 0; i < textures.size(); i++) {
		FontTexturePosition pos = textures.write[i].pack_rect(i, p_height, p_width);
		if (pos.index != -1) {
			return pos;
		}
	}

	// Create a new texture.
	int tex_size = MAX(font_size * 8, 256);
	tex_size = MIN(tex_size, 4096);
	// Round to power of 2.
	tex_size = next_power_of_2(tex_size);

	ShelfPackTexture tex(tex_size);
	tex.imgdata.resize(tex_size * tex_size * 2); // LA8 = 2 bytes/pixel

	{
		PoolVector<uint8_t>::Write w = tex.imgdata.write();
		for (int i = 0; i < tex_size * tex_size * 2; i += 2) {
			w[i + 0] = 255; // Luminance
			w[i + 1] = 0; // Alpha (transparent)
		}
	}

	tex.texture.instance();
	Ref<Image> img = memnew(Image(tex_size, tex_size, false, Image::FORMAT_LA8, tex.imgdata));
	tex.texture->create_from_image(img, 0); // No flags = no filter/repeat
	tex.dirty = false;

	int new_idx = textures.size();
	textures.push_back(tex);

	FontTexturePosition pos = textures.write[new_idx].pack_rect(new_idx, p_height, p_width);
	return pos;
}

void SSFNFont::_ensure_glyph(int32_t p_char) const {
	if (char_map.has(p_char)) {
		return;
	}

	if (!valid || !ssfn_ctx) {
		return;
	}

	ssfn_t *ctx = (ssfn_t *)ssfn_ctx;

	// Select font for rendering.
	int ret = ssfn_select(ctx, SSFN_FAMILY_ANY, nullptr, font_style, font_size);
	if (ret != SSFN_OK) {
		return;
	}

	// Encode codepoint as UTF-8.
	char utf8[5] = {};
	if (p_char < 0x80) {
		utf8[0] = (char)p_char;
	} else if (p_char < 0x800) {
		utf8[0] = (char)(0xC0 | (p_char >> 6));
		utf8[1] = (char)(0x80 | (p_char & 0x3F));
	} else if (p_char < 0x10000) {
		utf8[0] = (char)(0xE0 | (p_char >> 12));
		utf8[1] = (char)(0x80 | ((p_char >> 6) & 0x3F));
		utf8[2] = (char)(0x80 | (p_char & 0x3F));
	} else {
		utf8[0] = (char)(0xF0 | (p_char >> 18));
		utf8[1] = (char)(0x80 | ((p_char >> 12) & 0x3F));
		utf8[2] = (char)(0x80 | ((p_char >> 6) & 0x3F));
		utf8[3] = (char)(0x80 | (p_char & 0x3F));
	}

	// Render to a temp buffer to get the glyph.
	int buf_size = font_size * 2;
	if (buf_size < 64) {
		buf_size = 64;
	}

	Vector<uint8_t> tmp;
	tmp.resize(buf_size * buf_size * 4);
	memset(tmp.ptrw(), 0, tmp.size());

	ssfn_buf_t buf;
	memset(&buf, 0, sizeof(ssfn_buf_t));
	buf.ptr = tmp.ptrw();
	buf.w = buf_size;
	buf.h = buf_size;
	buf.p = buf_size * 4;
	buf.x = 0;
	buf.y = (int)_ascent;
	buf.fg = 0xFFFFFFFF;

	ret = ssfn_render(ctx, &buf, utf8);
	if (ret != SSFN_OK || !ctx->g) {
		// Store an empty character for this codepoint.
		Character ch;
		ch.texture_idx = -1;
		ch.advance = font_size * 0.5f;
		char_map[p_char] = ch;
		return;
	}

	ssfn_glyph_t *g = ctx->g;
	int glyph_w = (int)g->p; // Pitch = width for alpha bitmaps.
	int glyph_h = (int)g->h;
	float advance_x = (float)g->x;
	float glyph_ascender = (float)g->a;
	float glyph_overlap = (float)g->o;

	if (glyph_w <= 0 || glyph_h <= 0) {
		// Whitespace or empty glyph — store metrics only.
		Character ch;
		ch.texture_idx = -1;
		ch.advance = advance_x;
		ch.h_align = -glyph_overlap;
		ch.v_align = _ascent - glyph_ascender;
		char_map[p_char] = ch;
		return;
	}

	// Find atlas space.
	FontTexturePosition pos = _find_texture_pos_for_glyph(glyph_w, glyph_h);
	if (pos.index == -1) {
		// Could not allocate. Store empty.
		Character ch;
		ch.texture_idx = -1;
		ch.advance = advance_x;
		char_map[p_char] = ch;
		return;
	}

	// Copy glyph bitmap into atlas as LA8.
	ShelfPackTexture &atlas = textures.write[pos.index];
	{
		PoolVector<uint8_t>::Write w = atlas.imgdata.write();
		for (int row = 0; row < glyph_h; row++) {
			for (int col = 0; col < glyph_w; col++) {
				int atlas_x = pos.x + col;
				int atlas_y = pos.y + row;
				int atlas_idx = (atlas_y * atlas.texture_size + atlas_x) * 2;
				// SSFN glyph data is alpha values.
				uint8_t alpha = g->data[row * glyph_w + col];
				w[atlas_idx + 0] = 255; // Luminance
				w[atlas_idx + 1] = alpha;
			}
		}
	}
	atlas.dirty = true;

	// Update texture on GPU.
	Ref<Image> img = memnew(Image(atlas.texture_size, atlas.texture_size, false, Image::FORMAT_LA8, atlas.imgdata));
	atlas.texture->set_data(img);
	atlas.dirty = false;

	// Store character.
	Character ch;
	ch.texture_idx = pos.index;
	ch.rect = Rect2(pos.x, pos.y, glyph_w, glyph_h);
	ch.h_align = -glyph_overlap;
	ch.v_align = _ascent - glyph_ascender;
	ch.advance = advance_x;
	char_map[p_char] = ch;
}

// --- Public API ---

Error SSFNFont::load_from_file(const String &p_path) {
	FileAccess *f = FileAccess::open(p_path, FileAccess::READ);
	ERR_FAIL_COND_V_MSG(!f, ERR_FILE_NOT_FOUND, "Cannot open font file: " + p_path);

	int len = f->get_len();
	PoolByteArray data;
	data.resize(len);
	{
		PoolByteArray::Write w = data.write();
		f->get_buffer(w.ptr(), len);
	}
	f->close();
	memdelete(f);

	font_path = p_path;
	return load_from_data(data);
}

Error SSFNFont::load_from_data(const PoolByteArray &p_data) {
	_free_ssfn();
	_clear_cache();
	valid = false;

	if (p_data.size() < (int)sizeof(ssfn_font_t)) {
		ERR_FAIL_V_MSG(ERR_INVALID_DATA, "Font data too small to be a valid SSFN font.");
	}

	// Store font data — SSFN needs the pointer to remain valid.
	font_data_storage.resize(p_data.size());
	{
		PoolByteArray::Read r = p_data.read();
		memcpy(font_data_storage.ptrw(), r.ptr(), p_data.size());
	}

	_init_ssfn();
	ssfn_t *ctx = (ssfn_t *)ssfn_ctx;

	int ret = ssfn_load(ctx, font_data_storage.ptr());
	if (ret != SSFN_OK) {
		_free_ssfn();
		font_data_storage.clear();
		ERR_FAIL_V_MSG(ERR_INVALID_DATA, String("SSFN load error: ") + ssfn_error(ret));
	}

	valid = true;
	_update_metrics();
	_clear_cache();
	emit_changed();

	return OK;
}

void SSFNFont::set_size(int p_size) {
	p_size = CLAMP(p_size, 8, SSFN_SIZE_MAX);
	if (font_size == p_size) {
		return;
	}
	font_size = p_size;
	_clear_cache();
	if (valid) {
		_update_metrics();
	}
	emit_changed();
}

int SSFNFont::get_size() const {
	return font_size;
}

void SSFNFont::set_style(int p_style) {
	if (font_style == p_style) {
		return;
	}
	font_style = p_style;
	_clear_cache();
	if (valid) {
		_update_metrics();
	}
	emit_changed();
}

int SSFNFont::get_style() const {
	return font_style;
}

void SSFNFont::set_extra_spacing_char(int p_spacing) {
	if (extra_spacing_char == p_spacing) {
		return;
	}
	extra_spacing_char = p_spacing;
	emit_changed();
}

int SSFNFont::get_extra_spacing_char() const {
	return extra_spacing_char;
}

void SSFNFont::set_fallback(const Ref<SSFNFont> &p_fallback) {
	// Prevent circular references.
	Ref<SSFNFont> f = p_fallback;
	while (f.is_valid()) {
		ERR_FAIL_COND_MSG(f.ptr() == this, "Can't set as fallback one of its parents to prevent crashes due to recursive loop.");
		f = f->get_fallback();
	}
	fallback = p_fallback;
	emit_changed();
}

Ref<SSFNFont> SSFNFont::get_fallback() const {
	return fallback;
}

void SSFNFont::set_font_path(const String &p_path) {
	if (font_path == p_path) {
		return;
	}
	load_from_file(p_path);
}

String SSFNFont::get_font_path() const {
	return font_path;
}

bool SSFNFont::is_valid() const {
	return valid;
}

void SSFNFont::_set_font_data(const PoolByteArray &p_data) {
	if (p_data.size() > 0) {
		load_from_data(p_data);
	}
}

PoolByteArray SSFNFont::_get_font_data() const {
	PoolByteArray data;
	if (font_data_storage.size() > 0) {
		data.resize(font_data_storage.size());
		PoolByteArray::Write w = data.write();
		memcpy(w.ptr(), font_data_storage.ptr(), font_data_storage.size());
	}
	return data;
}

// --- Font interface ---

float SSFNFont::get_height() const {
	return _height;
}

float SSFNFont::get_ascent() const {
	return _ascent;
}

float SSFNFont::get_descent() const {
	return _descent;
}

int SSFNFont::get_spacing_char() const {
	return extra_spacing_char;
}

static int32_t _decode_char(CharType p_char, CharType p_next) {
	int32_t ch = p_char;
	if (((p_char & 0xfffffc00) == 0xd800) && (p_next & 0xfffffc00) == 0xdc00) {
		ch = (p_char << 10UL) + p_next - ((0xd800 << 10UL) + 0xdc00 - 0x10000);
	}
	return ch;
}

static bool _is_trail_surrogate(CharType p_char) {
	return (p_char & 0xfffffc00) == 0xdc00;
}

Size2 SSFNFont::get_char_size(CharType p_char, CharType p_next) const {
	if (_is_trail_surrogate(p_char)) {
		return Size2();
	}
	int32_t ch = _decode_char(p_char, p_next);

	_ensure_glyph(ch);
	const Character *c = char_map.getptr(ch);

	if (!c) {
		if (fallback.is_valid()) {
			return fallback->get_char_size(p_char, p_next);
		}
		return Size2();
	}

	Size2 ret(c->advance + extra_spacing_char, _height);
	return ret;
}

float SSFNFont::draw_char_ex(RID p_canvas_item, const Point2 &p_pos, CharType p_char, CharType p_next, const Color &p_modulate, bool p_outline, MultiRect *p_multirect, const CharTransform *p_char_xform) const {
	if (_is_trail_surrogate(p_char)) {
		return 0;
	}
	int32_t ch = _decode_char(p_char, p_next);

	_ensure_glyph(ch);
	const Character *c = char_map.getptr(ch);

	if (!c) {
		if (fallback.is_valid()) {
			return fallback->draw_char(p_canvas_item, p_pos, p_char, p_next, p_modulate, p_outline);
		}
		return 0;
	}

	if (c->texture_idx < 0 || c->texture_idx >= textures.size()) {
		return c->advance + extra_spacing_char;
	}

	if (!p_outline) {
		Point2 cpos = p_pos;
		cpos.x += c->h_align;
		cpos.y += c->v_align - _ascent;

		if (p_char_xform) {
			if (!p_char_xform->hidden) {
				const Rect2 rc = p_char_xform->xform_dest(Rect2(cpos, c->rect.size));
				real_t valign = 0;
				if (p_char_xform->vertical_align) {
					const real_t rotation_base = p_pos.y - _ascent / 2.0;
					const real_t t = p_char_xform->progress;
					valign = (rotation_base - rc.get_center().y) * t * t * t * t * t;
				}
				if (p_multirect) {
					p_multirect->add_rect(p_canvas_item, rc.move_by(Point2(0, valign)), textures[c->texture_idx].texture->get_rid(), p_char_xform->xform_tex(c->rect), p_modulate, false, RID(), RID(), false);
				} else {
					VisualServer::get_singleton()->canvas_item_add_texture_rect_region(p_canvas_item,
							rc.move_by(Point2(0, valign)),
							textures[c->texture_idx].texture->get_rid(),
							p_char_xform->xform_tex(c->rect), p_modulate, false, RID(), RID(), false);
				}
			}
		} else {
			if (p_multirect) {
				p_multirect->add_rect(p_canvas_item, Rect2(cpos, c->rect.size), textures[c->texture_idx].texture->get_rid(), c->rect, p_modulate, false, RID(), RID(), false);
			} else {
				VisualServer::get_singleton()->canvas_item_add_texture_rect_region(p_canvas_item,
						Rect2(cpos, c->rect.size),
						textures[c->texture_idx].texture->get_rid(),
						c->rect, p_modulate, false, RID(), RID(), false);
			}
		}
	}

	return c->advance + extra_spacing_char;
}

RID SSFNFont::get_char_texture(CharType p_char, CharType p_next, bool p_outline) const {
	if (_is_trail_surrogate(p_char)) {
		return RID();
	}
	int32_t ch = _decode_char(p_char, p_next);

	_ensure_glyph(ch);
	const Character *c = char_map.getptr(ch);

	if (!c) {
		if (fallback.is_valid()) {
			return fallback->get_char_texture(p_char, p_next, p_outline);
		}
		return RID();
	}

	if (!p_outline && c->texture_idx >= 0 && c->texture_idx < textures.size()) {
		return textures[c->texture_idx].texture->get_rid();
	}
	return RID();
}

Size2 SSFNFont::get_char_texture_size(CharType p_char, CharType p_next, bool p_outline) const {
	if (_is_trail_surrogate(p_char)) {
		return Size2();
	}
	int32_t ch = _decode_char(p_char, p_next);

	_ensure_glyph(ch);
	const Character *c = char_map.getptr(ch);

	if (!c) {
		if (fallback.is_valid()) {
			return fallback->get_char_texture_size(p_char, p_next, p_outline);
		}
		return Size2();
	}

	if (!p_outline && c->texture_idx >= 0 && c->texture_idx < textures.size()) {
		return textures[c->texture_idx].texture->get_size();
	}
	return Size2();
}

Vector2 SSFNFont::get_char_tx_offset(CharType p_char, CharType p_next, bool p_outline) const {
	if (_is_trail_surrogate(p_char)) {
		return Vector2();
	}
	int32_t ch = _decode_char(p_char, p_next);

	_ensure_glyph(ch);
	const Character *c = char_map.getptr(ch);

	if (!c) {
		if (fallback.is_valid()) {
			return fallback->get_char_tx_offset(p_char, p_next, p_outline);
		}
		return Vector2();
	}

	if (!p_outline && c->texture_idx >= 0 && c->texture_idx < textures.size()) {
		Point2 cpos;
		cpos.x += c->h_align;
		cpos.y -= _ascent;
		cpos.y += c->v_align;
		return cpos;
	}
	return Vector2();
}

Size2 SSFNFont::get_char_tx_size(CharType p_char, CharType p_next, bool p_outline) const {
	if (_is_trail_surrogate(p_char)) {
		return Size2();
	}
	int32_t ch = _decode_char(p_char, p_next);

	_ensure_glyph(ch);
	const Character *c = char_map.getptr(ch);

	if (!c) {
		if (fallback.is_valid()) {
			return fallback->get_char_tx_size(p_char, p_next, p_outline);
		}
		return Size2();
	}

	if (!p_outline && c->texture_idx >= 0 && c->texture_idx < textures.size()) {
		return c->rect.size;
	}
	return Size2();
}

Rect2 SSFNFont::get_char_tx_uv_rect(CharType p_char, CharType p_next, bool p_outline) const {
	if (_is_trail_surrogate(p_char)) {
		return Rect2();
	}
	int32_t ch = _decode_char(p_char, p_next);

	_ensure_glyph(ch);
	const Character *c = char_map.getptr(ch);

	if (!c) {
		if (fallback.is_valid()) {
			return fallback->get_char_tx_uv_rect(p_char, p_next, p_outline);
		}
		return Rect2();
	}

	if (!p_outline && c->texture_idx >= 0 && c->texture_idx < textures.size()) {
		return c->rect;
	}
	return Rect2();
}

// --- Bindings ---

void SSFNFont::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load_from_file", "path"), &SSFNFont::load_from_file);
	ClassDB::bind_method(D_METHOD("load_from_data", "data"), &SSFNFont::load_from_data);

	ClassDB::bind_method(D_METHOD("set_size", "size"), &SSFNFont::set_size);
	ClassDB::bind_method(D_METHOD("get_size"), &SSFNFont::get_size);

	ClassDB::bind_method(D_METHOD("set_style", "style"), &SSFNFont::set_style);
	ClassDB::bind_method(D_METHOD("get_style"), &SSFNFont::get_style);

	ClassDB::bind_method(D_METHOD("set_extra_spacing_char", "spacing"), &SSFNFont::set_extra_spacing_char);
	ClassDB::bind_method(D_METHOD("get_extra_spacing_char"), &SSFNFont::get_extra_spacing_char);

	ClassDB::bind_method(D_METHOD("set_fallback", "fallback"), &SSFNFont::set_fallback);
	ClassDB::bind_method(D_METHOD("get_fallback"), &SSFNFont::get_fallback);

	ClassDB::bind_method(D_METHOD("set_font_path", "path"), &SSFNFont::set_font_path);
	ClassDB::bind_method(D_METHOD("get_font_path"), &SSFNFont::get_font_path);

	ClassDB::bind_method(D_METHOD("is_valid"), &SSFNFont::is_valid);

	ClassDB::bind_method(D_METHOD("_set_font_data", "data"), &SSFNFont::_set_font_data);
	ClassDB::bind_method(D_METHOD("_get_font_data"), &SSFNFont::_get_font_data);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "font_path", PROPERTY_HINT_FILE, "*.sfn"), "set_font_path", "get_font_path");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "size", PROPERTY_HINT_RANGE, "8,192,1"), "set_size", "get_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "style"), "set_style", "get_style");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "extra_spacing_char"), "set_extra_spacing_char", "get_extra_spacing_char");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "fallback", PROPERTY_HINT_RESOURCE_TYPE, "SSFNFont"), "set_fallback", "get_fallback");
	ADD_PROPERTY(PropertyInfo(Variant::POOL_BYTE_ARRAY, "font_data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE), "_set_font_data", "_get_font_data");
}

SSFNFont::SSFNFont() {
	ssfn_ctx = nullptr;
	valid = false;
	font_size = 16;
	font_style = SSFN_STYLE_REGULAR;
	extra_spacing_char = 0;
	_ascent = 0;
	_descent = 0;
	_height = 0;
}

SSFNFont::~SSFNFont() {
	_clear_cache();
	_free_ssfn();
}

// --- ResourceFormatLoaderSSFNFont ---

RES ResourceFormatLoaderSSFNFont::load(const String &p_path, const String &p_original_path, Error *r_error, bool p_no_subresource_cache) {
	if (r_error) {
		*r_error = ERR_FILE_CANT_OPEN;
	}

	Ref<SSFNFont> font;
	font.instance();

	Error err = font->load_from_file(p_path);
	if (err) {
		if (r_error) {
			*r_error = err;
		}
		return RES();
	}

	if (r_error) {
		*r_error = OK;
	}
	return font;
}

void ResourceFormatLoaderSSFNFont::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("sfn");
}

bool ResourceFormatLoaderSSFNFont::handles_type(const String &p_type) const {
	return (p_type == "SSFNFont");
}

String ResourceFormatLoaderSSFNFont::get_resource_type(const String &p_path) const {
	String el = p_path.get_extension().to_lower();
	if (el == "sfn") {
		return "SSFNFont";
	}
	return "";
}
