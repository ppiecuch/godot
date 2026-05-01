/**************************************************************************/
/*  opensymbols.h                                                         */
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

// OpenSymbols — runtime icon source backed by Deepin OpenSymbol fonts.
// 649 vector glyphs across 3 variants. Rasterised on demand via ThorVG and
// cached (LRU) for repeat lookups.
//
// Typical use:
//   var os := OpenSymbols.new()
//   $icon.texture = os.get_texture(OpenSymbols.DEEPIN_1, "folder", 32, Color.gold)

#ifndef OPENSYMBOLS_H
#define OPENSYMBOLS_H

#include "core/hash_map.h"
#include "core/list.h"
#include "core/reference.h"
#include "scene/resources/texture.h"

class OpenSymbols : public Reference {
	GDCLASS(OpenSymbols, Reference);

public:
	enum Family {
		DEEPIN_1 = 0,
		DEEPIN_2 = 1,
		DEEPIN_3 = 2,
		FAMILY_COUNT = 3,
	};

private:
	struct CacheKey {
		int variant;
		String name;
		int size_px;
		uint32_t color_rgba8;
		bool operator==(const CacheKey &o) const {
			return variant == o.variant && size_px == o.size_px && color_rgba8 == o.color_rgba8 && name == o.name;
		}
	};
	struct CacheKeyHasher {
		static _FORCE_INLINE_ uint32_t hash(const CacheKey &k) {
			uint32_t h = (uint32_t)k.variant * 2654435761u;
			h ^= k.name.hash() + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= (uint32_t)k.size_px + 0x9e3779b9 + (h << 6) + (h >> 2);
			h ^= k.color_rgba8 + 0x9e3779b9 + (h << 6) + (h >> 2);
			return h;
		}
	};

	HashMap<CacheKey, Ref<Image>, CacheKeyHasher> _image_cache;
	HashMap<CacheKey, Ref<ImageTexture>, CacheKeyHasher> _texture_cache;
	List<CacheKey> _image_lru, _texture_lru;
	int _image_cache_cap;
	int _texture_cache_cap;

	static uint32_t _quantize_color(const Color &c);
	void _evict_if_needed_image();
	void _evict_if_needed_texture();
	void _touch_image(const CacheKey &k);
	void _touch_texture(const CacheKey &k);

	// Internal lookup against the generated tables.
	const char *_lookup_svg(int variant, const String &name, size_t *r_len) const;

protected:
	static void _bind_methods();

public:
	int get_count(Family v) const;
	PoolStringArray get_names(Family v) const;
	bool has_symbol(Family v, const String &name) const;

	String get_svg(Family v, const String &name) const;

	Ref<Image> get_image(Family v, const String &name, int size_px, const Color &color = Color(1, 1, 1, 1));
	Ref<ImageTexture> get_texture(Family v, const String &name, int size_px, const Color &color = Color(1, 1, 1, 1));

	void clear_cache();
	void set_image_cache_capacity(int cap);
	void set_texture_cache_capacity(int cap);
	int get_image_cache_size() const;
	int get_texture_cache_size() const;

	OpenSymbols();
	~OpenSymbols();
};

VARIANT_ENUM_CAST(OpenSymbols::Family);

#endif // OPENSYMBOLS_H
