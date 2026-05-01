/**************************************************************************/
/*  opensymbols.cpp                                                       */
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

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

#include "opensymbols.h"

#include "../thorvg/image_loader_thor_svg.h"
#include "opensymbols_data.gen.h"

OpenSymbols::OpenSymbols() {
	_image_cache_cap = 256;
	_texture_cache_cap = 64;
}

OpenSymbols::~OpenSymbols() {
}

int OpenSymbols::get_count(Family v) const {
	switch (v) {
		case DEEPIN_1:
			return opensymbols_data::variant_1_count;
		case DEEPIN_2:
			return opensymbols_data::variant_2_count;
		case DEEPIN_3:
			return opensymbols_data::variant_3_count;
		default:
			return 0;
	}
}

const char *OpenSymbols::_lookup_svg(int variant, const String &name, size_t *r_len) const {
	const opensymbols_data::Entry *table = nullptr;
	int count = 0;
	switch (variant) {
		case DEEPIN_1:
			table = opensymbols_data::variant_1;
			count = opensymbols_data::variant_1_count;
			break;
		case DEEPIN_2:
			table = opensymbols_data::variant_2;
			count = opensymbols_data::variant_2_count;
			break;
		case DEEPIN_3:
			table = opensymbols_data::variant_3;
			count = opensymbols_data::variant_3_count;
			break;
		default:
			return nullptr;
	}
	// Tables are sorted by name at codegen time — binary search.
	CharString needle = name.utf8();
	int lo = 0, hi = count - 1;
	while (lo <= hi) {
		int mid = (lo + hi) >> 1;
		int cmp = strcmp(needle.get_data(), table[mid].name);
		if (cmp == 0) {
			if (r_len) {
				*r_len = table[mid].svg_len;
			}
			return table[mid].svg;
		}
		if (cmp < 0) {
			hi = mid - 1;
		} else {
			lo = mid + 1;
		}
	}
	return nullptr;
}

PoolStringArray OpenSymbols::get_names(Family v) const {
	PoolStringArray out;
	const opensymbols_data::Entry *table = nullptr;
	int count = 0;
	switch (v) {
		case DEEPIN_1:
			table = opensymbols_data::variant_1;
			count = opensymbols_data::variant_1_count;
			break;
		case DEEPIN_2:
			table = opensymbols_data::variant_2;
			count = opensymbols_data::variant_2_count;
			break;
		case DEEPIN_3:
			table = opensymbols_data::variant_3;
			count = opensymbols_data::variant_3_count;
			break;
		default:
			return out;
	}
	out.resize(count);
	PoolStringArray::Write w = out.write();
	for (int i = 0; i < count; i++) {
		w[i] = String::utf8(table[i].name);
	}
	return out;
}

bool OpenSymbols::has_symbol(Family v, const String &name) const {
	return _lookup_svg(v, name, nullptr) != nullptr;
}

String OpenSymbols::get_svg(Family v, const String &name) const {
	size_t len = 0;
	const char *svg = _lookup_svg(v, name, &len);
	if (!svg) {
		return String();
	}
	return String::utf8(svg, (int)len);
}

uint32_t OpenSymbols::_quantize_color(const Color &c) {
	uint32_t r = (uint32_t)CLAMP((int)round(c.r * 255.0f), 0, 255);
	uint32_t g = (uint32_t)CLAMP((int)round(c.g * 255.0f), 0, 255);
	uint32_t b = (uint32_t)CLAMP((int)round(c.b * 255.0f), 0, 255);
	uint32_t a = (uint32_t)CLAMP((int)round(c.a * 255.0f), 0, 255);
	return (a << 24) | (r << 16) | (g << 8) | b;
}

void OpenSymbols::_touch_image(const CacheKey &k) {
	for (List<CacheKey>::Element *E = _image_lru.front(); E; E = E->next()) {
		if (E->get() == k) {
			_image_lru.erase(E);
			break;
		}
	}
	_image_lru.push_front(k);
}

void OpenSymbols::_touch_texture(const CacheKey &k) {
	for (List<CacheKey>::Element *E = _texture_lru.front(); E; E = E->next()) {
		if (E->get() == k) {
			_texture_lru.erase(E);
			break;
		}
	}
	_texture_lru.push_front(k);
}

void OpenSymbols::_evict_if_needed_image() {
	while (_image_cache_cap > 0 && _image_cache.size() > _image_cache_cap && !_image_lru.empty()) {
		CacheKey old = _image_lru.back()->get();
		_image_lru.pop_back();
		_image_cache.erase(old);
	}
}

void OpenSymbols::_evict_if_needed_texture() {
	while (_texture_cache_cap > 0 && _texture_cache.size() > _texture_cache_cap && !_texture_lru.empty()) {
		CacheKey old = _texture_lru.back()->get();
		_texture_lru.pop_back();
		_texture_cache.erase(old);
	}
}

Ref<Image> OpenSymbols::get_image(Family v, const String &name, int size_px, const Color &color) {
	if (size_px <= 0) {
		return Ref<Image>();
	}
	CacheKey key{ (int)v, name, size_px, _quantize_color(color) };
	if (Ref<Image> *hit = _image_cache.getptr(key)) {
		_touch_image(key);
		return *hit;
	}

	String svg = get_svg(v, name);
	if (svg.empty()) {
		return Ref<Image>();
	}
	// SVG's `currentColor` (CSS-only) — ThorVG does not resolve it; substitute
	// the requested colour as a hex literal before rasterizing.
	String hex = vformat("#%02x%02x%02x", (int)round(color.r * 255.0f),
			(int)round(color.g * 255.0f), (int)round(color.b * 255.0f));
	svg = svg.replace("currentColor", hex);

	Ref<Image> img;
	img.instance();
	HashMap<Color, Color> empty_remap;
	ImageLoaderThorSVG loader;
	// size_px on the longest side; the other side is derived from aspect ratio.
	Error err = loader.create_image_sized_from_string(img, svg, size_px, 0, empty_remap);
	if (err != OK) {
		return Ref<Image>();
	}

	// Apply alpha component if caller passed semi-transparent colour.
	if (color.a < 1.0f) {
		PoolByteArray data = img->get_data();
		PoolByteArray::Write w = data.write();
		const int pixel_count = img->get_width() * img->get_height();
		uint8_t a_factor = (uint8_t)CLAMP((int)round(color.a * 255.0f), 0, 255);
		for (int i = 0; i < pixel_count; i++) {
			uint8_t &a = w[i * 4 + 3];
			a = (uint8_t)((a * a_factor) / 255);
		}
		img->create(img->get_width(), img->get_height(), false, Image::FORMAT_RGBA8, data);
	}

	_image_cache.set(key, img);
	_image_lru.push_front(key);
	_evict_if_needed_image();
	return img;
}

Ref<ImageTexture> OpenSymbols::get_texture(Family v, const String &name, int size_px, const Color &color) {
	CacheKey key{ (int)v, name, size_px, _quantize_color(color) };
	if (Ref<ImageTexture> *hit = _texture_cache.getptr(key)) {
		_touch_texture(key);
		return *hit;
	}
	Ref<Image> img = get_image(v, name, size_px, color);
	if (img.is_null()) {
		return Ref<ImageTexture>();
	}
	Ref<ImageTexture> tex;
	tex.instance();
	tex->create_from_image(img, Texture::FLAG_FILTER);
	_texture_cache.set(key, tex);
	_texture_lru.push_front(key);
	_evict_if_needed_texture();
	return tex;
}

void OpenSymbols::clear_cache() {
	_image_cache.clear();
	_image_lru.clear();
	_texture_cache.clear();
	_texture_lru.clear();
}

void OpenSymbols::set_image_cache_capacity(int cap) {
	_image_cache_cap = MAX(0, cap);
	_evict_if_needed_image();
}

void OpenSymbols::set_texture_cache_capacity(int cap) {
	_texture_cache_cap = MAX(0, cap);
	_evict_if_needed_texture();
}

int OpenSymbols::get_image_cache_size() const {
	return _image_cache.size();
}

int OpenSymbols::get_texture_cache_size() const {
	return _texture_cache.size();
}

void OpenSymbols::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_count", "variant"), &OpenSymbols::get_count);
	ClassDB::bind_method(D_METHOD("get_names", "variant"), &OpenSymbols::get_names);
	ClassDB::bind_method(D_METHOD("has_symbol", "variant", "name"), &OpenSymbols::has_symbol);
	ClassDB::bind_method(D_METHOD("get_svg", "variant", "name"), &OpenSymbols::get_svg);
	ClassDB::bind_method(D_METHOD("get_image", "variant", "name", "size_px", "color"),
			&OpenSymbols::get_image, DEFVAL(Color(1, 1, 1, 1)));
	ClassDB::bind_method(D_METHOD("get_texture", "variant", "name", "size_px", "color"),
			&OpenSymbols::get_texture, DEFVAL(Color(1, 1, 1, 1)));

	ClassDB::bind_method(D_METHOD("clear_cache"), &OpenSymbols::clear_cache);
	ClassDB::bind_method(D_METHOD("set_image_cache_capacity", "cap"), &OpenSymbols::set_image_cache_capacity);
	ClassDB::bind_method(D_METHOD("set_texture_cache_capacity", "cap"), &OpenSymbols::set_texture_cache_capacity);
	ClassDB::bind_method(D_METHOD("get_image_cache_size"), &OpenSymbols::get_image_cache_size);
	ClassDB::bind_method(D_METHOD("get_texture_cache_size"), &OpenSymbols::get_texture_cache_size);

	BIND_ENUM_CONSTANT(DEEPIN_1);
	BIND_ENUM_CONSTANT(DEEPIN_2);
	BIND_ENUM_CONSTANT(DEEPIN_3);
	BIND_ENUM_CONSTANT(FAMILY_COUNT);
}

#ifdef DOCTEST
TEST_CASE("[OpenSymbols] catalogue counts") {
	OpenSymbols os;
	CHECK(os.get_count(OpenSymbols::DEEPIN_1) == 224);
	CHECK(os.get_count(OpenSymbols::DEEPIN_2) == 217);
	CHECK(os.get_count(OpenSymbols::DEEPIN_3) == 208);
}

TEST_CASE("[OpenSymbols] lookup roundtrip") {
	OpenSymbols os;
	CHECK(os.has_symbol(OpenSymbols::DEEPIN_1, "folder"));
	CHECK_FALSE(os.has_symbol(OpenSymbols::DEEPIN_1, "definitely-not-a-symbol"));
	String svg = os.get_svg(OpenSymbols::DEEPIN_1, "folder");
	CHECK(svg.length() > 0);
	CHECK(svg.begins_with("<svg"));
}

TEST_CASE("[OpenSymbols] cache hit returns same Image ref") {
	OpenSymbols os;
	Ref<Image> a = os.get_image(OpenSymbols::DEEPIN_1, "folder", 32, Color(1, 1, 1));
	REQUIRE(a.is_valid());
	Ref<Image> b = os.get_image(OpenSymbols::DEEPIN_1, "folder", 32, Color(1, 1, 1));
	CHECK(a == b);
	CHECK(os.get_image_cache_size() == 1);
	os.clear_cache();
	CHECK(os.get_image_cache_size() == 0);
}

TEST_CASE("[OpenSymbols] LRU eviction respects capacity") {
	OpenSymbols os;
	os.set_image_cache_capacity(2);
	(void)os.get_image(OpenSymbols::DEEPIN_1, "folder", 16, Color(1, 1, 1));
	(void)os.get_image(OpenSymbols::DEEPIN_1, "airplane", 16, Color(1, 1, 1));
	(void)os.get_image(OpenSymbols::DEEPIN_1, "scissors", 16, Color(1, 1, 1));
	CHECK(os.get_image_cache_size() == 2);
}
#endif
