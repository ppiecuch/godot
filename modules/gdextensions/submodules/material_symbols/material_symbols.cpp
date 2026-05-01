/**************************************************************************/
/*  material_symbols.cpp                                                  */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

#include "material_symbols.h"

#include "material_symbols_data.gen.h"
#include "material_symbols_renderer.h"

MaterialSymbols::MaterialSymbols() {
	_image_cache_cap = 256;
	_texture_cache_cap = 64;
}

MaterialSymbols::~MaterialSymbols() {
}

int MaterialSymbols::get_count(Style s) const {
	switch (s) {
		case STYLE_OUTLINED:
			return material_symbols_data::entries_outlined_count;
		case STYLE_ROUNDED:
			return material_symbols_data::entries_rounded_count;
		case STYLE_SHARP:
			return material_symbols_data::entries_sharp_count;
		default:
			return 0;
	}
}

uint32_t MaterialSymbols::_lookup_codepoint(int style, const String &name) const {
	const material_symbols_data::Entry *table = nullptr;
	int count = 0;
	switch (style) {
		case STYLE_OUTLINED:
			table = material_symbols_data::entries_outlined;
			count = material_symbols_data::entries_outlined_count;
			break;
		case STYLE_ROUNDED:
			table = material_symbols_data::entries_rounded;
			count = material_symbols_data::entries_rounded_count;
			break;
		case STYLE_SHARP:
			table = material_symbols_data::entries_sharp;
			count = material_symbols_data::entries_sharp_count;
			break;
		default:
			return 0;
	}
	CharString needle = name.utf8();
	int lo = 0, hi = count - 1;
	while (lo <= hi) {
		int mid = (lo + hi) >> 1;
		int cmp = strcmp(needle.get_data(), table[mid].name);
		if (cmp == 0) {
			return table[mid].codepoint;
		}
		if (cmp < 0) {
			hi = mid - 1;
		} else {
			lo = mid + 1;
		}
	}
	return 0;
}

PoolStringArray MaterialSymbols::get_names(Style s) const {
	PoolStringArray out;
	const material_symbols_data::Entry *table = nullptr;
	int count = 0;
	switch (s) {
		case STYLE_OUTLINED:
			table = material_symbols_data::entries_outlined;
			count = material_symbols_data::entries_outlined_count;
			break;
		case STYLE_ROUNDED:
			table = material_symbols_data::entries_rounded;
			count = material_symbols_data::entries_rounded_count;
			break;
		case STYLE_SHARP:
			table = material_symbols_data::entries_sharp;
			count = material_symbols_data::entries_sharp_count;
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

bool MaterialSymbols::has_symbol(Style s, const String &name) const {
	return _lookup_codepoint((int)s, name) != 0;
}

uint32_t MaterialSymbols::get_codepoint(Style s, const String &name) const {
	return _lookup_codepoint((int)s, name);
}

uint32_t MaterialSymbols::_quantize_color(const Color &c) {
	uint32_t r = (uint32_t)CLAMP((int)round(c.r * 255.0f), 0, 255);
	uint32_t g = (uint32_t)CLAMP((int)round(c.g * 255.0f), 0, 255);
	uint32_t b = (uint32_t)CLAMP((int)round(c.b * 255.0f), 0, 255);
	uint32_t a = (uint32_t)CLAMP((int)round(c.a * 255.0f), 0, 255);
	return (a << 24) | (r << 16) | (g << 8) | b;
}

int MaterialSymbols::_quantize_weight(int w) {
	w = CLAMP(w, 100, 700);
	return ((w + 50) / 100) * 100; // round to nearest 100
}

int MaterialSymbols::_quantize_grade(int g) {
	// Snap to {-25, 0, 200} (Material's documented grades).
	if (g <= -12) {
		return -25;
	}
	if (g >= 100) {
		return 200;
	}
	return 0;
}

int MaterialSymbols::_quantize_opsz(int o) {
	// Snap to nearest of {20, 24, 40, 48}.
	static const int allowed[] = { 20, 24, 40, 48 };
	int best = allowed[0];
	int best_d = ABS(o - allowed[0]);
	for (int i = 1; i < 4; i++) {
		int d = ABS(o - allowed[i]);
		if (d < best_d) {
			best_d = d;
			best = allowed[i];
		}
	}
	return best;
}

uint8_t MaterialSymbols::_quantize_fill(float f) {
	f = CLAMP(f, 0.0f, 1.0f);
	return (uint8_t)round(f * 255.0f);
}

void MaterialSymbols::_touch_image(const CacheKey &k) {
	for (List<CacheKey>::Element *E = _image_lru.front(); E; E = E->next()) {
		if (E->get() == k) {
			_image_lru.erase(E);
			break;
		}
	}
	_image_lru.push_front(k);
}

void MaterialSymbols::_touch_texture(const CacheKey &k) {
	for (List<CacheKey>::Element *E = _texture_lru.front(); E; E = E->next()) {
		if (E->get() == k) {
			_texture_lru.erase(E);
			break;
		}
	}
	_texture_lru.push_front(k);
}

void MaterialSymbols::_evict_image() {
	while (_image_cache_cap > 0 && _image_cache.size() > _image_cache_cap && !_image_lru.empty()) {
		CacheKey old = _image_lru.back()->get();
		_image_lru.pop_back();
		_image_cache.erase(old);
	}
}

void MaterialSymbols::_evict_texture() {
	while (_texture_cache_cap > 0 && _texture_cache.size() > _texture_cache_cap && !_texture_lru.empty()) {
		CacheKey old = _texture_lru.back()->get();
		_texture_lru.pop_back();
		_texture_cache.erase(old);
	}
}

Ref<Image> MaterialSymbols::get_image(Style s, const String &name, int size_px, const Dictionary &opts) {
	if (size_px <= 0) {
		return Ref<Image>();
	}
	const Color color = opts.has("color") ? (Color)opts["color"] : Color(1, 1, 1, 1);
	const int weight = opts.has("weight") ? (int)opts["weight"] : 400;
	const int grade = opts.has("grade") ? (int)opts["grade"] : 0;
	const int optical_size = opts.has("opsz") ? (int)opts["opsz"] : 24;
	const float fill = opts.has("fill") ? (float)opts["fill"] : 0.0f;
	const int qw = _quantize_weight(weight);
	const int qg = _quantize_grade(grade);
	const int qo = _quantize_opsz(optical_size);
	const uint8_t qf = _quantize_fill(fill);

	CacheKey key{ (int)s, name, size_px, _quantize_color(color),
		(uint16_t)qw, (int16_t)qg, (uint8_t)qo, qf };
	if (Ref<Image> *hit = _image_cache.getptr(key)) {
		_touch_image(key);
		return *hit;
	}

	uint32_t cp = _lookup_codepoint((int)s, name);
	if (cp == 0) {
		return Ref<Image>();
	}
	MaterialSymbolsRenderer::Axes axes;
	axes.weight = qw;
	axes.grade = qg;
	axes.optical_size = qo;
	axes.fill = (float)qf / 255.0f;
	Ref<Image> img = MaterialSymbolsRenderer::render((int)s, cp, size_px, color, axes);
	if (img.is_null()) {
		return img;
	}
	_image_cache.set(key, img);
	_image_lru.push_front(key);
	_evict_image();
	return img;
}

Ref<ImageTexture> MaterialSymbols::get_texture(Style s, const String &name, int size_px, const Dictionary &opts) {
	const Color color = opts.has("color") ? (Color)opts["color"] : Color(1, 1, 1, 1);
	const int weight = opts.has("weight") ? (int)opts["weight"] : 400;
	const int grade = opts.has("grade") ? (int)opts["grade"] : 0;
	const int optical_size = opts.has("opsz") ? (int)opts["opsz"] : 24;
	const float fill = opts.has("fill") ? (float)opts["fill"] : 0.0f;
	const int qw = _quantize_weight(weight);
	const int qg = _quantize_grade(grade);
	const int qo = _quantize_opsz(optical_size);
	const uint8_t qf = _quantize_fill(fill);
	CacheKey key{ (int)s, name, size_px, _quantize_color(color),
		(uint16_t)qw, (int16_t)qg, (uint8_t)qo, qf };
	if (Ref<ImageTexture> *hit = _texture_cache.getptr(key)) {
		_touch_texture(key);
		return *hit;
	}
	Ref<Image> img = get_image(s, name, size_px, opts);
	if (img.is_null()) {
		return Ref<ImageTexture>();
	}
	Ref<ImageTexture> tex;
	tex.instance();
	tex->create_from_image(img, Texture::FLAG_FILTER);
	_texture_cache.set(key, tex);
	_texture_lru.push_front(key);
	_evict_texture();
	return tex;
}

void MaterialSymbols::clear_cache() {
	_image_cache.clear();
	_image_lru.clear();
	_texture_cache.clear();
	_texture_lru.clear();
}

void MaterialSymbols::set_image_cache_capacity(int cap) {
	_image_cache_cap = MAX(0, cap);
	_evict_image();
}

void MaterialSymbols::set_texture_cache_capacity(int cap) {
	_texture_cache_cap = MAX(0, cap);
	_evict_texture();
}

int MaterialSymbols::get_image_cache_size() const {
	return _image_cache.size();
}

int MaterialSymbols::get_texture_cache_size() const {
	return _texture_cache.size();
}

void MaterialSymbols::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_count", "style"), &MaterialSymbols::get_count);
	ClassDB::bind_method(D_METHOD("get_names", "style"), &MaterialSymbols::get_names);
	ClassDB::bind_method(D_METHOD("has_symbol", "style", "name"), &MaterialSymbols::has_symbol);
	ClassDB::bind_method(D_METHOD("get_codepoint", "style", "name"), &MaterialSymbols::get_codepoint);
	ClassDB::bind_method(D_METHOD("get_image", "style", "name", "size_px", "opts"),
			&MaterialSymbols::get_image, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("get_texture", "style", "name", "size_px", "opts"),
			&MaterialSymbols::get_texture, DEFVAL(Dictionary()));

	ClassDB::bind_method(D_METHOD("clear_cache"), &MaterialSymbols::clear_cache);
	ClassDB::bind_method(D_METHOD("set_image_cache_capacity", "cap"), &MaterialSymbols::set_image_cache_capacity);
	ClassDB::bind_method(D_METHOD("set_texture_cache_capacity", "cap"), &MaterialSymbols::set_texture_cache_capacity);
	ClassDB::bind_method(D_METHOD("get_image_cache_size"), &MaterialSymbols::get_image_cache_size);
	ClassDB::bind_method(D_METHOD("get_texture_cache_size"), &MaterialSymbols::get_texture_cache_size);

	BIND_ENUM_CONSTANT(STYLE_OUTLINED);
	BIND_ENUM_CONSTANT(STYLE_ROUNDED);
	BIND_ENUM_CONSTANT(STYLE_SHARP);
	BIND_ENUM_CONSTANT(STYLE_COUNT);
}

#ifdef DOCTEST
TEST_CASE("[MaterialSymbols] catalogue counts") {
	MaterialSymbols ms;
	CHECK(ms.get_count(MaterialSymbols::STYLE_OUTLINED) > 3000);
	CHECK(ms.get_count(MaterialSymbols::STYLE_ROUNDED) > 3000);
	CHECK(ms.get_count(MaterialSymbols::STYLE_SHARP) > 3000);
}

TEST_CASE("[MaterialSymbols] codepoint roundtrip") {
	MaterialSymbols ms;
	for (int s = 0; s < MaterialSymbols::STYLE_COUNT; s++) {
		MaterialSymbols::Style style = (MaterialSymbols::Style)s;
		CHECK(ms.has_symbol(style, "search"));
		CHECK(ms.get_codepoint(style, "search") != 0);
		CHECK_FALSE(ms.has_symbol(style, "definitely_not_a_symbol"));
	}
}

TEST_CASE("[MaterialSymbols] axis quantisation") {
	MaterialSymbols ms;
	Dictionary o437;
	o437["weight"] = 437;
	Dictionary o400;
	o400["weight"] = 400;
	Ref<Image> a = ms.get_image(MaterialSymbols::STYLE_ROUNDED, "search", 24, o437);
	Ref<Image> b = ms.get_image(MaterialSymbols::STYLE_ROUNDED, "search", 24, o400);
	REQUIRE(a.is_valid());
	REQUIRE(b.is_valid());
	CHECK(a == b); // both quantise to weight=400, share the cache slot
}
#endif
