/**************************************************************************/
/*  ssfn_font.h                                                           */
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

#ifndef SSFN_FONT_H
#define SSFN_FONT_H

#include "scene/resources/font.h"
#include "scene/resources/texture.h"

class SSFNFont : public Font {
	GDCLASS(SSFNFont, Font);
	RES_BASE_EXTENSION("sfnfont");

public:
	struct Character {
		int texture_idx;
		Rect2 rect; // UV rect in atlas (pixels)
		float v_align;
		float h_align;
		float advance;

		Character() {
			texture_idx = 0;
			v_align = 0;
			h_align = 0;
			advance = 0;
		}
	};

private:
	struct FontTexturePosition {
		int32_t index = -1;
		int32_t x = 0;
		int32_t y = 0;

		FontTexturePosition() {}
		FontTexturePosition(int32_t p_id, int32_t p_x, int32_t p_y) :
				index(p_id), x(p_x), y(p_y) {}
	};

	struct Shelf {
		int32_t x = 0;
		int32_t y = 0;
		int32_t w = 0;
		int32_t h = 0;

		FontTexturePosition alloc_shelf(int32_t p_id, int32_t p_w, int32_t p_h) {
			if (p_w > w || p_h > h) {
				return FontTexturePosition(-1, 0, 0);
			}
			int32_t xx = x;
			x += p_w;
			w -= p_w;
			return FontTexturePosition(p_id, xx, y);
		}

		Shelf() {}
		Shelf(int32_t p_x, int32_t p_y, int32_t p_w, int32_t p_h) :
				x(p_x), y(p_y), w(p_w), h(p_h) {}
	};

	struct ShelfPackTexture {
		int32_t texture_size = 1024;
		PoolVector<uint8_t> imgdata;
		Ref<ImageTexture> texture;
		List<Shelf> shelves;
		bool dirty = true;

		FontTexturePosition pack_rect(int32_t p_id, int32_t p_h, int32_t p_w);

		ShelfPackTexture() {}
		ShelfPackTexture(int32_t p_size) :
				texture_size(p_size) {}
	};

	// SSFN context (opaque, allocated/freed in cpp)
	void *ssfn_ctx;
	bool valid;

	Vector<uint8_t> font_data_storage;
	String font_path;
	int font_size;
	int font_style;
	int extra_spacing_char;

	float _ascent;
	float _descent;
	float _height;

	mutable HashMap<int32_t, Character> char_map;
	mutable Vector<ShelfPackTexture> textures;

	Ref<SSFNFont> fallback;

	void _clear_cache() const;
	void _ensure_glyph(int32_t p_char) const;
	FontTexturePosition _find_texture_pos_for_glyph(int p_width, int p_height) const;
	void _update_metrics();
	void _init_ssfn();
	void _free_ssfn();

	void _set_font_data(const PoolByteArray &p_data);
	PoolByteArray _get_font_data() const;

protected:
	static void _bind_methods();

public:
	Error load_from_file(const String &p_path);
	Error load_from_data(const PoolByteArray &p_data);

	void set_size(int p_size);
	int get_size() const;

	void set_style(int p_style);
	int get_style() const;

	void set_extra_spacing_char(int p_spacing);
	int get_extra_spacing_char() const;

	void set_fallback(const Ref<SSFNFont> &p_fallback);
	Ref<SSFNFont> get_fallback() const;

	void set_font_path(const String &p_path);
	String get_font_path() const;

	bool is_valid() const;

	// Font interface
	float get_height() const override;
	float get_ascent() const override;
	float get_descent() const override;
	int get_spacing_char() const override;

	Size2 get_char_size(CharType p_char, CharType p_next = 0) const override;
	bool is_distance_field_hint() const override { return false; }

	float draw_char_ex(RID p_canvas_item, const Point2 &p_pos, CharType p_char, CharType p_next = 0, const Color &p_modulate = Color(1, 1, 1), bool p_outline = false, MultiRect *p_multirect = nullptr, const CharTransform *p_char_xform = nullptr) const override;

	RID get_char_texture(CharType p_char, CharType p_next, bool p_outline) const override;
	Size2 get_char_texture_size(CharType p_char, CharType p_next, bool p_outline) const override;

	Vector2 get_char_tx_offset(CharType p_char, CharType p_next, bool p_outline) const override;
	Size2 get_char_tx_size(CharType p_char, CharType p_next, bool p_outline) const override;
	Rect2 get_char_tx_uv_rect(CharType p_char, CharType p_next, bool p_outline) const override;

	SSFNFont();
	~SSFNFont();
};

class ResourceFormatLoaderSSFNFont : public ResourceFormatLoader {
public:
	virtual RES load(const String &p_path, const String &p_original_path = "", Error *r_error = nullptr, bool p_no_subresource_cache = false);
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual bool handles_type(const String &p_type) const;
	virtual String get_resource_type(const String &p_path) const;
};

#endif // SSFN_FONT_H
