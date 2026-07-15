/**************************************************************************/
/*  font_engine_3d.cpp                                                    */
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

#include "font_engine_3d.h"

#include "core/io/json.h"
#include "core/io/resource_loader.h"
#include "core/os/file_access.h"
#include "scene/resources/material.h"

static const char *_special_filenames[][2] = {
	{ "-", "minus" },
	{ "+", "plus" },
	{ "/", "slash" },
	{ "*", "star" },
	{ "?", "ques" },
	{ ".", "dot" },
	{ "!", "excl" },
	{ ",", "comma" },
	{ nullptr, nullptr },
};

// ---------------------------------------------------------------------------
// FontEngine3D
// ---------------------------------------------------------------------------

FontEngine3D *FontEngine3D::singleton = nullptr;

FontEngine3D *FontEngine3D::get_singleton() { return singleton; }

String FontEngine3D::_char_to_filename(CharType c) const {
	if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
		return String::chr(c);
	}
	if (c >= 'a' && c <= 'z') {
		return String::chr(c - 32);
	}
	for (int i = 0; _special_filenames[i][0] != nullptr; i++) {
		if (c == _special_filenames[i][0][0]) {
			return _special_filenames[i][1];
		}
	}
	return String();
}

const FontEngine3D::FontInfo *FontEngine3D::_get_font_info(int p_style) const {
	if (p_style < 0 || p_style >= _fonts.size()) {
		return nullptr;
	}
	return &_fonts[p_style];
}

void FontEngine3D::set_catalog_path(const String &p_path) {
	if (_catalog_path != p_path) {
		_catalog_path = p_path;
		_catalog_loaded = false;
		_fonts.clear();
		_cache.clear();
	}
}

String FontEngine3D::get_catalog_path() const { return _catalog_path; }

bool FontEngine3D::load_catalog(const String &p_path) {
	_catalog_path = p_path;
	_fonts.clear();
	_cache.clear();
	_catalog_loaded = false;

	FileAccess *f = FileAccess::open(p_path, FileAccess::READ);
	if (!f) {
		return false;
	}

	String json_text = f->get_as_utf8_string();
	memdelete(f);

	Variant result;
	String err_str;
	int err_line;
	Error err = JSON::parse(json_text, result, err_str, err_line);
	if (err != OK) {
		ERR_PRINT("FontEngine3D: Failed to parse catalog: " + err_str + " at line " + itos(err_line));
		return false;
	}

	Dictionary catalog = result;
	if (!catalog.has("fonts")) {
		ERR_PRINT("FontEngine3D: catalog.json missing 'fonts' key");
		return false;
	}

	Dictionary fonts_dict = catalog["fonts"];
	Array keys = fonts_dict.keys();
	for (int i = 0; i < keys.size(); i++) {
		String font_name = keys[i];
		Dictionary font_data = fonts_dict[font_name];

		FontInfo info;
		info.name = font_name;
		info.path = font_data.has("path") ? String(font_data["path"]) : "";
		info.has_materials = font_data.has("has_materials") ? bool(font_data["has_materials"]) : false;

		if (font_data.has("glyphs")) {
			Array glyphs_arr = font_data["glyphs"];
			for (int g = 0; g < glyphs_arr.size(); g++) {
				info.glyphs.push_back(glyphs_arr[g]);
			}
		}

		if (font_data.has("textures")) {
			Array tex_arr = font_data["textures"];
			for (int t = 0; t < tex_arr.size(); t++) {
				info.textures.push_back(tex_arr[t]);
			}
		}

		_fonts.push_back(info);
	}

	_catalog_loaded = true;
	return true;
}

void FontEngine3D::_load_catalog() {
	if (_catalog_loaded || _catalog_path.empty()) {
		return;
	}
	load_catalog(_catalog_path);
}

int FontEngine3D::get_font_count() const {
	return _fonts.size();
}

String FontEngine3D::get_font_name(int p_style) const {
	const FontInfo *info = _get_font_info(p_style);
	return info ? info->name : String();
}

PoolStringArray FontEngine3D::get_font_names() const {
	PoolStringArray names;
	for (int i = 0; i < _fonts.size(); i++) {
		names.push_back(_fonts[i].name);
	}
	return names;
}

bool FontEngine3D::has_glyph(int p_style, CharType c) const {
	const FontInfo *info = _get_font_info(p_style);
	if (!info) {
		return false;
	}
	String fname = _char_to_filename(c);
	if (fname.empty()) {
		return false;
	}
	for (int i = 0; i < info->glyphs.size(); i++) {
		if (info->glyphs[i] == fname) {
			return true;
		}
	}
	return false;
}

String FontEngine3D::get_glyph_resource_path(int p_style, CharType c) const {
	const FontInfo *info = _get_font_info(p_style);
	if (!info || info->path.empty()) {
		return String();
	}
	String fname = _char_to_filename(c);
	if (fname.empty()) {
		return String();
	}
	return String("res://").plus_file(info->path).plus_file(fname + ".tres");
}

Ref<ArrayMesh> FontEngine3D::get_glyph_mesh(int p_style, CharType c) {
	CharType uc = c;
	if (uc >= 'a' && uc <= 'z') {
		uc = uc - 32;
	}

	const FontInfo *info = _get_font_info(p_style);
	if (!info) {
		return Ref<ArrayMesh>();
	}

	if (_cache.has(info->name) && _cache[info->name].has(uc)) {
		return _cache[info->name][uc];
	}

	String path = get_glyph_resource_path(p_style, uc);
	if (path.empty()) {
		return Ref<ArrayMesh>();
	}

	Ref<ArrayMesh> mesh = ResourceLoader::load(path, "ArrayMesh");
	if (mesh.is_valid()) {
		_cache[info->name][uc] = mesh;
	}
	return mesh;
}

float FontEngine3D::get_glyph_advance(int p_style, CharType c) {
	Ref<ArrayMesh> mesh = get_glyph_mesh(p_style, c);
	if (mesh.is_null()) {
		return 1.0f;
	}
	AABB aabb = mesh->get_aabb();
	return aabb.size.x;
}

float FontEngine3D::get_text_width(const String &p_text, int p_style, float p_spacing) const {
	if (p_text.empty()) {
		return 0.0f;
	}
	float width = 0.0f;
	int valid_count = 0;
	for (int i = 0; i < p_text.length(); i++) {
		CharType c = p_text[i];
		if (c == ' ') {
			width += 1.0f;
			valid_count++;
			continue;
		}
		if (!has_glyph(p_style, c)) {
			continue;
		}
		width += 1.0f;
		valid_count++;
	}
	if (valid_count > 1) {
		width += p_spacing * (valid_count - 1);
	}
	return width;
}

int FontEngine3D::get_supported_glyph_count(int p_style) const {
	const FontInfo *info = _get_font_info(p_style);
	return info ? info->glyphs.size() : 0;
}

String FontEngine3D::get_supported_chars(int p_style) const {
	const FontInfo *info = _get_font_info(p_style);
	if (!info) {
		return String();
	}
	String result;
	for (int i = 0; i < info->glyphs.size(); i++) {
		result += info->glyphs[i] + ",";
	}
	return result;
}

void FontEngine3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_catalog_path", "path"), &FontEngine3D::set_catalog_path);
	ClassDB::bind_method(D_METHOD("get_catalog_path"), &FontEngine3D::get_catalog_path);
	ClassDB::bind_method(D_METHOD("load_catalog", "path"), &FontEngine3D::load_catalog);
	ClassDB::bind_method(D_METHOD("get_font_count"), &FontEngine3D::get_font_count);
	ClassDB::bind_method(D_METHOD("get_font_name", "style"), &FontEngine3D::get_font_name);
	ClassDB::bind_method(D_METHOD("get_font_names"), &FontEngine3D::get_font_names);
	ClassDB::bind_method(D_METHOD("has_glyph", "style", "character"), &FontEngine3D::has_glyph);
	ClassDB::bind_method(D_METHOD("get_glyph_resource_path", "style", "character"), &FontEngine3D::get_glyph_resource_path);
	ClassDB::bind_method(D_METHOD("get_glyph_advance", "style", "character"), &FontEngine3D::get_glyph_advance);
	ClassDB::bind_method(D_METHOD("get_text_width", "text", "style", "spacing"), &FontEngine3D::get_text_width);
	ClassDB::bind_method(D_METHOD("get_supported_glyph_count", "style"), &FontEngine3D::get_supported_glyph_count);
	ClassDB::bind_method(D_METHOD("get_supported_chars", "style"), &FontEngine3D::get_supported_chars);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "catalog_path", PROPERTY_HINT_FILE, "*.json"), "set_catalog_path", "get_catalog_path");

	BIND_ENUM_CONSTANT(HALIGN_LEFT);
	BIND_ENUM_CONSTANT(HALIGN_CENTER);
	BIND_ENUM_CONSTANT(HALIGN_RIGHT);
}

FontEngine3D::FontEngine3D() {
	_catalog_loaded = false;
	if (!singleton) {
		singleton = this;
	}
}

FontEngine3D::~FontEngine3D() {
	if (singleton == this) {
		singleton = nullptr;
	}
}

// ---------------------------------------------------------------------------
// Font3DLabel
// ---------------------------------------------------------------------------

void Font3DLabel::set_text(const String &p_text) {
	if (_text != p_text) {
		_text = p_text;
		_dirty = true;
		_update_mesh();
	}
}
String Font3DLabel::get_text() const { return _text; }

void Font3DLabel::set_font_style(int p_style) {
	FontEngine3D *engine = FontEngine3D::get_singleton();
	int max_style = engine ? engine->get_font_count() : 1;
	p_style = CLAMP(p_style, 0, MAX(max_style - 1, 0));
	if (_font_style != p_style) {
		_font_style = p_style;
		_dirty = true;
		_update_mesh();
	}
}
int Font3DLabel::get_font_style() const { return _font_style; }

void Font3DLabel::set_font_color(const Color &p_color) {
	if (_font_color != p_color) {
		_font_color = p_color;
		for (int i = 0; i < _glyph_nodes.size(); i++) {
			MeshInstance *mi = _glyph_nodes[i];
			if (mi && mi->get_surface_material_count() > 0) {
				Ref<SpatialMaterial> mat = mi->get_surface_material(0);
				if (mat.is_valid()) {
					mat->set_albedo(p_color);
				}
			}
		}
	}
}
Color Font3DLabel::get_font_color() const { return _font_color; }

void Font3DLabel::set_letter_spacing(float p_spacing) {
	if (_letter_spacing != p_spacing) {
		_letter_spacing = p_spacing;
		_dirty = true;
		_update_mesh();
	}
}
float Font3DLabel::get_letter_spacing() const { return _letter_spacing; }

void Font3DLabel::set_font_size(float p_size) {
	p_size = MAX(p_size, 0.01f);
	if (_font_size != p_size) {
		_font_size = p_size;
		_dirty = true;
		_update_mesh();
	}
}
float Font3DLabel::get_font_size() const { return _font_size; }

void Font3DLabel::set_horizontal_align(FontEngine3D::HAlign p_align) {
	if (_h_align != p_align) {
		_h_align = p_align;
		_dirty = true;
		_update_mesh();
	}
}
FontEngine3D::HAlign Font3DLabel::get_horizontal_align() const { return _h_align; }

float Font3DLabel::get_text_width() const {
	FontEngine3D *engine = FontEngine3D::get_singleton();
	if (!engine) {
		return 0.0f;
	}
	return engine->get_text_width(_text, _font_style, _letter_spacing) * _font_size;
}

void Font3DLabel::_clear_glyphs() {
	for (int i = 0; i < _glyph_nodes.size(); i++) {
		if (_glyph_nodes[i]) {
			_glyph_nodes[i]->queue_delete();
		}
	}
	_glyph_nodes.clear();
}

void Font3DLabel::_update_mesh() {
	if (!is_inside_tree() || !_dirty) {
		return;
	}
	_dirty = false;
	_clear_glyphs();

	if (_text.empty()) {
		return;
	}

	FontEngine3D *engine = FontEngine3D::get_singleton();
	if (!engine || engine->get_font_count() == 0) {
		return;
	}

	float cursor_x = 0.0f;

	float total_width = engine->get_text_width(_text, _font_style, _letter_spacing);
	float align_offset = 0.0f;
	switch (_h_align) {
		case FontEngine3D::HALIGN_CENTER:
			align_offset = -total_width * _font_size * 0.5f;
			break;
		case FontEngine3D::HALIGN_RIGHT:
			align_offset = -total_width * _font_size;
			break;
		default:
			break;
	}

	for (int i = 0; i < _text.length(); i++) {
		CharType c = _text[i];

		if (c == ' ') {
			cursor_x += _font_size * (1.0f + _letter_spacing);
			continue;
		}

		if (!engine->has_glyph(_font_style, c)) {
			continue;
		}

		Ref<ArrayMesh> glyph = engine->get_glyph_mesh(_font_style, c);
		if (glyph.is_null()) {
			cursor_x += _font_size * (1.0f + _letter_spacing);
			continue;
		}

		MeshInstance *mi = memnew(MeshInstance);
		mi->set_mesh(glyph);

		Transform t;
		t.origin.x = align_offset + cursor_x;
		t.basis.scale(Vector3(_font_size, _font_size, _font_size));
		mi->set_transform(t);

		Ref<SpatialMaterial> mat;
		mat.instance();
		mat->set_albedo(_font_color);
		mi->set_surface_material(0, mat);

		add_child(mi);
		_glyph_nodes.push_back(mi);

		AABB aabb = glyph->get_aabb();
		cursor_x += (aabb.size.x * _font_size) + (_letter_spacing * _font_size);
	}
}

void Font3DLabel::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			if (_dirty) {
				_update_mesh();
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			_clear_glyphs();
		} break;
	}
}

void Font3DLabel::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_text", "text"), &Font3DLabel::set_text);
	ClassDB::bind_method(D_METHOD("get_text"), &Font3DLabel::get_text);
	ClassDB::bind_method(D_METHOD("set_font_style", "style"), &Font3DLabel::set_font_style);
	ClassDB::bind_method(D_METHOD("get_font_style"), &Font3DLabel::get_font_style);
	ClassDB::bind_method(D_METHOD("set_font_color", "color"), &Font3DLabel::set_font_color);
	ClassDB::bind_method(D_METHOD("get_font_color"), &Font3DLabel::get_font_color);
	ClassDB::bind_method(D_METHOD("set_letter_spacing", "spacing"), &Font3DLabel::set_letter_spacing);
	ClassDB::bind_method(D_METHOD("get_letter_spacing"), &Font3DLabel::get_letter_spacing);
	ClassDB::bind_method(D_METHOD("set_font_size", "size"), &Font3DLabel::set_font_size);
	ClassDB::bind_method(D_METHOD("get_font_size"), &Font3DLabel::get_font_size);
	ClassDB::bind_method(D_METHOD("set_horizontal_align", "align"), &Font3DLabel::set_horizontal_align);
	ClassDB::bind_method(D_METHOD("get_horizontal_align"), &Font3DLabel::get_horizontal_align);
	ClassDB::bind_method(D_METHOD("get_text_width"), &Font3DLabel::get_text_width);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "text", PROPERTY_HINT_MULTILINE_TEXT), "set_text", "get_text");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "font_style"), "set_font_style", "get_font_style");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "font_color"), "set_font_color", "get_font_color");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "letter_spacing", PROPERTY_HINT_RANGE, "0,10,0.01"), "set_letter_spacing", "get_letter_spacing");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "font_size", PROPERTY_HINT_RANGE, "0.01,100,0.01"), "set_font_size", "get_font_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "horizontal_align", PROPERTY_HINT_ENUM, "Left,Center,Right"), "set_horizontal_align", "get_horizontal_align");
}

Font3DLabel::Font3DLabel() {
	_font_style = 0;
	_font_color = Color(1, 1, 1, 1);
	_letter_spacing = 0.1f;
	_font_size = 1.0f;
	_h_align = FontEngine3D::HALIGN_LEFT;
	_dirty = true;
}

Font3DLabel::~Font3DLabel() {
}

// ---------------------------------------------------------------------------
// Doctests
// ---------------------------------------------------------------------------

#ifdef DOCTEST
#include "doctest/doctest.h"
#include "doctest/doctest_godot.h"

static String _write_test_catalog() {
	_doctest_prepare_folder();
	String path = _doctest_get_folder() + "test_catalog.json";
	FileAccess *f = FileAccess::open(path, FileAccess::WRITE);
	ERR_FAIL_COND_V(!f, "");
	f->store_string(R"({
  "fonts": {
    "neon": {
      "glyphs": ["A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z","0","1","2","3","4","5","6","7","8","9"],
      "glyph_count": 36,
      "has_materials": true,
      "textures": ["albedo","normal","emissive","metallic","ao"],
      "path": "fontengine3d/neon"
    },
    "art_retro/low": {
      "glyphs": ["A","B","C","0","1","excl","ques","minus"],
      "glyph_count": 8,
      "has_materials": true,
      "textures": ["albedo"],
      "path": "fontengine3d/fonts_art/Retro/Low"
    },
    "toon_statics": {
      "glyphs": ["0","1","2","3","4","5","6","7","8","9","dot","comma","slash"],
      "glyph_count": 13,
      "has_materials": false,
      "textures": [],
      "path": "fontengine3d/fonts_toon/statics"
    }
  }
})");
	memdelete(f);
	return path;
}

TEST_SUITE("[[fontengine3d]]") {
	TEST_CASE("[FontEngine3D] singleton") {
		Ref<FontEngine3D> engine;
		engine.instance();
		CHECK(engine.is_valid());
		CHECK(FontEngine3D::get_singleton() == engine.ptr());
	}

	TEST_CASE("[FontEngine3D] empty state before catalog") {
		Ref<FontEngine3D> engine;
		engine.instance();
		CHECK(engine->get_font_count() == 0);
		CHECK(engine->get_font_name(0).empty());
		CHECK(engine->get_font_names().size() == 0);
		CHECK(engine->get_supported_glyph_count(0) == 0);
		CHECK(engine->get_supported_chars(0).empty());
		CHECK_FALSE(engine->has_glyph(0, 'A'));
		CHECK(engine->get_glyph_resource_path(0, 'A').empty());
		CHECK(engine->get_text_width("HELLO", 0, 0.1f) == 0.0f);
	}

	TEST_CASE("[FontEngine3D] catalog_path property") {
		Ref<FontEngine3D> engine;
		engine.instance();
		CHECK(engine->get_catalog_path().empty());
		engine->set_catalog_path("res://fontengine3d/catalog.json");
		CHECK(engine->get_catalog_path() == "res://fontengine3d/catalog.json");
	}

	TEST_CASE("[FontEngine3D] load catalog from file") {
		String path = _write_test_catalog();
		REQUIRE_FALSE(path.empty());

		Ref<FontEngine3D> engine;
		engine.instance();
		bool ok = engine->load_catalog(path);
		CHECK(ok);
		CHECK(engine->get_font_count() == 3);
	}

	TEST_CASE("[FontEngine3D] load catalog - bad path") {
		Ref<FontEngine3D> engine;
		engine.instance();
		bool ok = false;
		EXPECT_ERROR(ok = engine->load_catalog("nonexistent.json"));
		CHECK_FALSE(ok);
		CHECK(engine->get_font_count() == 0);
	}

	TEST_CASE("[FontEngine3D] load catalog - invalid json") {
		_doctest_prepare_folder();
		String path = _doctest_get_folder() + "bad_catalog.json";
		FileAccess *f = FileAccess::open(path, FileAccess::WRITE);
		f->store_string("NOT JSON {{{");
		memdelete(f);

		Ref<FontEngine3D> engine;
		engine.instance();
		bool ok = false;
		EXPECT_ERROR(ok = engine->load_catalog(path));
		CHECK_FALSE(ok);
	}

	TEST_CASE("[FontEngine3D] load catalog - missing fonts key") {
		_doctest_prepare_folder();
		String path = _doctest_get_folder() + "no_fonts.json";
		FileAccess *f = FileAccess::open(path, FileAccess::WRITE);
		f->store_string("{\"version\": 1}");
		memdelete(f);

		Ref<FontEngine3D> engine;
		engine.instance();
		bool ok = false;
		EXPECT_ERROR(ok = engine->load_catalog(path));
		CHECK_FALSE(ok);
	}

	TEST_CASE("[FontEngine3D] font names from catalog") {
		String path = _write_test_catalog();
		Ref<FontEngine3D> engine;
		engine.instance();
		engine->load_catalog(path);

		PoolStringArray names = engine->get_font_names();
		CHECK(names.size() == 3);

		bool has_neon = false, has_retro = false, has_toon = false;
		for (int i = 0; i < names.size(); i++) {
			if (names[i] == "neon")
				has_neon = true;
			if (names[i] == "art_retro/low")
				has_retro = true;
			if (names[i] == "toon_statics")
				has_toon = true;
		}
		CHECK(has_neon);
		CHECK(has_retro);
		CHECK(has_toon);
	}

	TEST_CASE("[FontEngine3D] font name by index") {
		String path = _write_test_catalog();
		Ref<FontEngine3D> engine;
		engine.instance();
		engine->load_catalog(path);

		CHECK_FALSE(engine->get_font_name(0).empty());
		CHECK_FALSE(engine->get_font_name(1).empty());
		CHECK_FALSE(engine->get_font_name(2).empty());
		CHECK(engine->get_font_name(3).empty());
		CHECK(engine->get_font_name(-1).empty());
	}

	TEST_CASE("[FontEngine3D] glyph counts per font") {
		String path = _write_test_catalog();
		Ref<FontEngine3D> engine;
		engine.instance();
		engine->load_catalog(path);

		// Find neon index
		int neon_idx = -1, retro_idx = -1, toon_idx = -1;
		for (int i = 0; i < engine->get_font_count(); i++) {
			if (engine->get_font_name(i) == "neon")
				neon_idx = i;
			if (engine->get_font_name(i) == "art_retro/low")
				retro_idx = i;
			if (engine->get_font_name(i) == "toon_statics")
				toon_idx = i;
		}

		REQUIRE(neon_idx >= 0);
		CHECK(engine->get_supported_glyph_count(neon_idx) == 36);
		REQUIRE(retro_idx >= 0);
		CHECK(engine->get_supported_glyph_count(retro_idx) == 8);
		REQUIRE(toon_idx >= 0);
		CHECK(engine->get_supported_glyph_count(toon_idx) == 13);
		CHECK(engine->get_supported_glyph_count(99) == 0);
	}

	TEST_CASE("[FontEngine3D] has_glyph with catalog") {
		String path = _write_test_catalog();
		Ref<FontEngine3D> engine;
		engine.instance();
		engine->load_catalog(path);

		int neon_idx = -1, retro_idx = -1;
		for (int i = 0; i < engine->get_font_count(); i++) {
			if (engine->get_font_name(i) == "neon")
				neon_idx = i;
			if (engine->get_font_name(i) == "art_retro/low")
				retro_idx = i;
		}
		REQUIRE(neon_idx >= 0);
		REQUIRE(retro_idx >= 0);

		// Neon has A-Z, 0-9
		CHECK(engine->has_glyph(neon_idx, 'A'));
		CHECK(engine->has_glyph(neon_idx, 'Z'));
		CHECK(engine->has_glyph(neon_idx, '0'));
		CHECK(engine->has_glyph(neon_idx, '9'));
		// Lowercase maps to uppercase
		CHECK(engine->has_glyph(neon_idx, 'a'));
		// Neon doesn't have special chars in this test catalog
		CHECK_FALSE(engine->has_glyph(neon_idx, '!'));
		CHECK_FALSE(engine->has_glyph(neon_idx, '~'));

		// Retro has A,B,C,0,1 and special chars
		CHECK(engine->has_glyph(retro_idx, 'A'));
		CHECK(engine->has_glyph(retro_idx, '!'));
		CHECK(engine->has_glyph(retro_idx, '?'));
		CHECK(engine->has_glyph(retro_idx, '-'));
		CHECK_FALSE(engine->has_glyph(retro_idx, 'Z'));

		// Invalid style
		CHECK_FALSE(engine->has_glyph(99, 'A'));
	}

	TEST_CASE("[FontEngine3D] resource path generation from catalog") {
		String path = _write_test_catalog();
		Ref<FontEngine3D> engine;
		engine.instance();
		engine->load_catalog(path);

		int neon_idx = -1, retro_idx = -1;
		for (int i = 0; i < engine->get_font_count(); i++) {
			if (engine->get_font_name(i) == "neon")
				neon_idx = i;
			if (engine->get_font_name(i) == "art_retro/low")
				retro_idx = i;
		}
		REQUIRE(neon_idx >= 0);
		REQUIRE(retro_idx >= 0);

		CHECK(engine->get_glyph_resource_path(neon_idx, 'A') == "res://fontengine3d/neon/A.tres");
		CHECK(engine->get_glyph_resource_path(neon_idx, '0') == "res://fontengine3d/neon/0.tres");
		CHECK(engine->get_glyph_resource_path(neon_idx, 'a') == "res://fontengine3d/neon/A.tres");
		CHECK(engine->get_glyph_resource_path(retro_idx, '!') == "res://fontengine3d/fonts_art/Retro/Low/excl.tres");
		CHECK(engine->get_glyph_resource_path(retro_idx, '-') == "res://fontengine3d/fonts_art/Retro/Low/minus.tres");

		// Unsupported char
		CHECK(engine->get_glyph_resource_path(neon_idx, '~').empty());
		// Invalid style
		CHECK(engine->get_glyph_resource_path(99, 'A').empty());
	}

	TEST_CASE("[FontEngine3D] text width with catalog") {
		String path = _write_test_catalog();
		Ref<FontEngine3D> engine;
		engine.instance();
		engine->load_catalog(path);

		int neon_idx = -1;
		for (int i = 0; i < engine->get_font_count(); i++) {
			if (engine->get_font_name(i) == "neon")
				neon_idx = i;
		}
		REQUIRE(neon_idx >= 0);

		CHECK(engine->get_text_width("", neon_idx, 0.1f) == 0.0f);
		CHECK(engine->get_text_width("A", neon_idx, 0.1f) == 1.0f);
		CHECK(engine->get_text_width("AB", neon_idx, 0.1f) == doctest::Approx(2.1f));
		CHECK(engine->get_text_width("ABC", neon_idx, 0.1f) == doctest::Approx(3.2f));
		CHECK(engine->get_text_width("AB", neon_idx, 0.0f) == 2.0f);
		CHECK(engine->get_text_width("A B", neon_idx, 0.1f) == doctest::Approx(3.2f));
		// '~' not in neon, gets skipped
		CHECK(engine->get_text_width("A~B", neon_idx, 0.1f) == doctest::Approx(2.1f));
	}

	TEST_CASE("[FontEngine3D] text width - invalid font style") {
		String path = _write_test_catalog();
		Ref<FontEngine3D> engine;
		engine.instance();
		engine->load_catalog(path);
		CHECK(engine->get_text_width("ABC", 99, 0.1f) == 0.0f);
	}

	TEST_CASE("[FontEngine3D] reload catalog clears cache") {
		String path = _write_test_catalog();
		Ref<FontEngine3D> engine;
		engine.instance();
		engine->load_catalog(path);
		CHECK(engine->get_font_count() == 3);

		// Write a different catalog
		_doctest_prepare_folder();
		String path2 = _doctest_get_folder() + "catalog2.json";
		FileAccess *f = FileAccess::open(path2, FileAccess::WRITE);
		f->store_string(R"({"fonts": {"single": {"glyphs": ["A"], "glyph_count": 1, "has_materials": false, "textures": [], "path": "test"}}})");
		memdelete(f);

		engine->load_catalog(path2);
		CHECK(engine->get_font_count() == 1);
		CHECK(engine->get_font_name(0) == "single");
	}

	TEST_CASE("[FontEngine3D] set_catalog_path clears state") {
		String path = _write_test_catalog();
		Ref<FontEngine3D> engine;
		engine.instance();
		engine->load_catalog(path);
		CHECK(engine->get_font_count() == 3);

		engine->set_catalog_path("other.json");
		CHECK(engine->get_font_count() == 0);
	}

	TEST_CASE("[Font3DLabel] default values") {
		Font3DLabel *label = memnew(Font3DLabel);
		CHECK(label->get_text().empty());
		CHECK(label->get_font_style() == 0);
		CHECK(label->get_font_color() == Color(1, 1, 1, 1));
		CHECK(label->get_letter_spacing() == doctest::Approx(0.1f));
		CHECK(label->get_font_size() == doctest::Approx(1.0f));
		CHECK(label->get_horizontal_align() == FontEngine3D::HALIGN_LEFT);
		memdelete(label);
	}

	TEST_CASE("[Font3DLabel] property round-trips") {
		Font3DLabel *label = memnew(Font3DLabel);

		label->set_text("HELLO");
		CHECK(label->get_text() == "HELLO");

		label->set_font_style(0);
		CHECK(label->get_font_style() == 0);

		label->set_font_color(Color(1, 0, 0, 1));
		CHECK(label->get_font_color() == Color(1, 0, 0, 1));

		label->set_letter_spacing(0.5f);
		CHECK(label->get_letter_spacing() == doctest::Approx(0.5f));

		label->set_font_size(2.0f);
		CHECK(label->get_font_size() == doctest::Approx(2.0f));

		label->set_horizontal_align(FontEngine3D::HALIGN_CENTER);
		CHECK(label->get_horizontal_align() == FontEngine3D::HALIGN_CENTER);

		memdelete(label);
	}

	TEST_CASE("[Font3DLabel] font size clamped") {
		Font3DLabel *label = memnew(Font3DLabel);
		label->set_font_size(0.0f);
		CHECK(label->get_font_size() >= 0.01f);
		label->set_font_size(-5.0f);
		CHECK(label->get_font_size() >= 0.01f);
		memdelete(label);
	}
}

#endif // DOCTEST
