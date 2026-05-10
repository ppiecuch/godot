/**************************************************************************/
/*  material_symbols.h                                                    */
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

// MaterialSymbols — runtime icon source backed by Google's Material Symbols
// variable fonts (Outlined / Rounded / Sharp). Glyphs are rasterised on
// demand by FreeType's variable-axis API (FILL, GRAD, opsz, wght) and cached
// LRU. ~3000 icons per style.
//
//   var ms := MaterialSymbols.new()
//   $icon.texture = ms.get_texture(MaterialSymbols.STYLE_ROUNDED, "search", 24)
//   $alt.texture  = ms.get_texture(MaterialSymbols.STYLE_ROUNDED, "play_arrow", 32,
//                                  Color.gold, 600, 0, 24, 1.0)  # heavy filled

#ifndef MATERIAL_SYMBOLS_H
#define MATERIAL_SYMBOLS_H

#include "core/hash_map.h"
#include "core/list.h"
#include "core/reference.h"
#include "scene/resources/texture.h"

class MaterialSymbols : public Reference {
	GDCLASS(MaterialSymbols, Reference);

public:
	enum Style {
		STYLE_OUTLINED = 0,
		STYLE_ROUNDED = 1,
		STYLE_SHARP = 2,
		STYLE_COUNT = 3,
	};

private:
	struct CacheKey {
		int style;
		String name;
		int size_px;
		uint32_t color_rgba8;
		uint16_t weight;
		int16_t grade;
		uint8_t optical_size;
		uint8_t fill_q;
		bool operator==(const CacheKey &o) const {
			return style == o.style && size_px == o.size_px && color_rgba8 == o.color_rgba8 &&
					weight == o.weight && grade == o.grade && optical_size == o.optical_size &&
					fill_q == o.fill_q && name == o.name;
		}
	};
	struct CacheKeyHasher {
		static _FORCE_INLINE_ uint32_t hash(const CacheKey &k) {
			uint32_t h = (uint32_t)k.style * 2654435761u;
			h ^= k.name.hash() + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= (uint32_t)k.size_px + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= k.color_rgba8 + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= ((uint32_t)k.weight << 16) ^ (uint32_t)(uint16_t)k.grade;
			h ^= ((uint32_t)k.optical_size << 8) ^ (uint32_t)k.fill_q;
			return h;
		}
	};

	HashMap<CacheKey, Ref<Image>, CacheKeyHasher> _image_cache;
	HashMap<CacheKey, Ref<ImageTexture>, CacheKeyHasher> _texture_cache;
	List<CacheKey> _image_lru, _texture_lru;
	int _image_cache_cap;
	int _texture_cache_cap;

	static uint32_t _quantize_color(const Color &c);
	static int _quantize_weight(int w);
	static int _quantize_grade(int g);
	static int _quantize_opsz(int o);
	static uint8_t _quantize_fill(float f);
	void _evict_image();
	void _evict_texture();
	void _touch_image(const CacheKey &k);
	void _touch_texture(const CacheKey &k);

	uint32_t _lookup_codepoint(int style, const String &name) const;

protected:
	static void _bind_methods();

public:
	int get_count(Style s) const;
	PoolStringArray get_names(Style s) const;
	bool has_symbol(Style s, const String &name) const;
	uint32_t get_codepoint(Style s, const String &name) const;

	// Axis tweaks are passed via a Dictionary `opts` to keep the bound signature
	// within Godot 3.x's 5-arg bind_method ceiling. Recognised keys (all optional):
	//   color: Color (default white)
	//   weight: int   (100..700, default 400)
	//   grade: int    (-25/0/200, default 0)
	//   opsz: int     (20/24/40/48, default 24)
	//   fill: float   (0..1, default 0)
	Ref<Image> get_image(Style s, const String &name, int size_px, const Dictionary &opts = Dictionary());
	Ref<ImageTexture> get_texture(Style s, const String &name, int size_px, const Dictionary &opts = Dictionary());

	void clear_cache();
	void set_image_cache_capacity(int cap);
	void set_texture_cache_capacity(int cap);
	int get_image_cache_size() const;
	int get_texture_cache_size() const;

	MaterialSymbols();
	~MaterialSymbols();
};

VARIANT_ENUM_CAST(MaterialSymbols::Style);

#endif // MATERIAL_SYMBOLS_H
