/**************************************************************************/
/*  material_symbols_baked.cpp                                            */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

// Runtime (non-tools) implementation of MaterialSymbols. Backed by a
// pre-rasterised PNG table emitted at build time by bake_material_symbols.py
// from the manifest passed via gdext_material_symbols_subset.
//
// The class declaration in material_symbols.h is shared with the tools build;
// only this .cpp (or material_symbols.cpp under TOOLS_ENABLED) is compiled,
// never both at the same time. Axis args on get_image()/get_texture() are
// quantised and used only to look up the pre-baked variant — no FreeType
// runtime rasterisation happens here.

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

#include "material_symbols.h"

#include "material_symbols_baked.gen.h"

#include "core/io/image_loader.h"
#include "core/os/memory.h"

MaterialSymbols::MaterialSymbols() {
	_image_cache_cap = 256;
	_texture_cache_cap = 64;
}

MaterialSymbols::~MaterialSymbols() {
}

// In runtime mode we don't ship the full per-style codepoint catalogue —
// the only "names we know" are those that survived the bake. get_count()
// reports the bake count for any style (best we can do), and get_names()
// returns the union of all baked names.

int MaterialSymbols::get_count(Style /*s*/) const {
	return material_symbols_baked::entries_count;
}

PoolStringArray MaterialSymbols::get_names(Style /*s*/) const {
	PoolStringArray out;
	out.resize(material_symbols_baked::entries_count);
	PoolStringArray::Write w = out.write();
	for (int i = 0; i < material_symbols_baked::entries_count; i++) {
		w[i] = String::utf8(material_symbols_baked::entries[i].name);
	}
	return out;
}

bool MaterialSymbols::has_symbol(Style /*s*/, const String &name) const {
	CharString needle = name.utf8();
	int lo = 0, hi = material_symbols_baked::entries_count - 1;
	while (lo <= hi) {
		int mid = (lo + hi) >> 1;
		int cmp = strcmp(needle.get_data(), material_symbols_baked::entries[mid].name);
		if (cmp == 0) {
			return true;
		}
		if (cmp < 0) {
			hi = mid - 1;
		} else {
			lo = mid + 1;
		}
	}
	return false;
}

uint32_t MaterialSymbols::get_codepoint(Style /*s*/, const String & /*name*/) const {
	// Codepoints aren't shipped in non-tools builds. Callers use get_image/get_texture.
	return 0;
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
	return ((w + 50) / 100) * 100;
}

int MaterialSymbols::_quantize_grade(int g) {
	if (g <= -12)
		return -25;
	if (g >= 100)
		return 200;
	return 0;
}

int MaterialSymbols::_quantize_opsz(int o) {
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

// Lookup: tag is taken from opts["tag"] (default "default"). Style is ignored
// at runtime — bake-time decided which style the entry's PNG was rendered in.
static const material_symbols_baked::Entry *_find_baked(const String &name, const String &tag) {
	CharString name_utf8 = name.utf8();
	CharString tag_utf8 = tag.utf8();
	int lo = 0, hi = material_symbols_baked::entries_count - 1;
	while (lo <= hi) {
		int mid = (lo + hi) >> 1;
		const material_symbols_baked::Entry &e = material_symbols_baked::entries[mid];
		int cmp = strcmp(name_utf8.get_data(), e.name);
		if (cmp == 0) {
			cmp = strcmp(tag_utf8.get_data(), e.tag);
		}
		if (cmp == 0) {
			return &e;
		}
		if (cmp < 0) {
			hi = mid - 1;
		} else {
			lo = mid + 1;
		}
	}
	return nullptr;
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
	const String tag = opts.has("tag") ? (String)opts["tag"] : String("default");
	const int qw = _quantize_weight(weight);
	const int qg = _quantize_grade(grade);
	const int qo = _quantize_opsz(optical_size);
	const uint8_t qf = _quantize_fill(fill);

	// Cache key uses the visible identity (name + tag + axes). The size_px
	// and color are ignored by the lookup but kept in the key to allow
	// per-call cache hits without re-decoding.
	CacheKey key{ (int)s, name + "@" + tag, size_px, _quantize_color(color),
		(uint16_t)qw, (int16_t)qg, (uint8_t)qo, qf };
	if (Ref<Image> *hit = _image_cache.getptr(key)) {
		_touch_image(key);
		return *hit;
	}

	const material_symbols_baked::Entry *e = _find_baked(name, tag);
	if (!e) {
		WARN_PRINT_ONCE(vformat("MaterialSymbols: '%s' (tag '%s') is not in the baked manifest.", name, tag));
		return Ref<Image>();
	}
	Ref<Image> img;
	img.instance();
	Vector<uint8_t> buf;
	buf.resize((int)e->png_len);
	memcpy(buf.ptrw(), e->png, e->png_len);
	Error err = img->load_png_from_buffer(buf);
	if (err != OK) {
		WARN_PRINT(vformat("MaterialSymbols: PNG decode failed for '%s'.", name));
		return Ref<Image>();
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
	const String tag = opts.has("tag") ? (String)opts["tag"] : String("default");
	const int qw = _quantize_weight(weight);
	const int qg = _quantize_grade(grade);
	const int qo = _quantize_opsz(optical_size);
	const uint8_t qf = _quantize_fill(fill);
	CacheKey key{ (int)s, name + "@" + tag, size_px, _quantize_color(color),
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
