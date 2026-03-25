/**************************************************************************/
/*  gd_slug.cpp                                                           */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#include "gd_slug.h"
#include "gd_slug_shader.h"

#include "core/os/file_access.h"
#include "scene/resources/material.h"

using namespace Terathon;
using namespace Terathon::Slug;

// =========================================================================
// SlugFont
// =========================================================================

SlugFont::SlugFont() :
		_header(nullptr), _workspace(nullptr) {
	_workspace = new Workspace;
}

SlugFont::~SlugFont() {
	delete _workspace;
}

Error SlugFont::load_from_file(const String &p_path) {
	FileAccess *f = FileAccess::open(p_path, FileAccess::READ);
	ERR_FAIL_COND_V_MSG(!f, ERR_FILE_NOT_FOUND, "SlugFont: cannot open " + p_path);

	int64_t len = f->get_len();
	ERR_FAIL_COND_V_MSG(len < (int64_t)sizeof(FontHeader), ERR_FILE_CORRUPT, "SlugFont: file too small");

	_font_data.resize(len);
	{
		PoolByteArray::Write w = _font_data.write();
		f->get_buffer(w.ptr(), len);
	}
	memdelete(f);

	_header = GetFontHeader(_font_data.read().ptr());
	ERR_FAIL_COND_V_MSG(!_header, ERR_FILE_CORRUPT, "SlugFont: invalid font header");

	_extract_textures();
	return OK;
}

Error SlugFont::load_from_buffer(const PoolByteArray &p_data) {
	ERR_FAIL_COND_V(p_data.size() < (int)sizeof(FontHeader), ERR_INVALID_DATA);

	_font_data = p_data;
	_header = GetFontHeader(_font_data.read().ptr());
	ERR_FAIL_COND_V_MSG(!_header, ERR_FILE_CORRUPT, "SlugFont: invalid font header");

	_extract_textures();
	return OK;
}

bool SlugFont::is_valid() const {
	return _header != nullptr && _curve_texture.is_valid();
}

void SlugFont::_extract_textures() {
	if (!_header)
		return;

	int32 curve_w = _header->curveTextureSize.x;
	int32 curve_h = _header->curveTextureSize.y;
	int32 band_w = _header->bandTextureSize.x;
	int32 band_h = _header->bandTextureSize.y;

	// Curve texture: half-float RGBA (2 bytes per channel = 8 bytes per texel)
	int curve_size = curve_w * curve_h * 8;
	PoolByteArray curve_data;
	curve_data.resize(curve_size);

	// Band texture: uint16 RGBA (2 bytes per channel = 8 bytes per texel)
	int band_size = band_w * band_h * 8;
	PoolByteArray band_raw;
	band_raw.resize(band_size);

	{
		PoolByteArray::Write cw = curve_data.write();
		PoolByteArray::Write bw = band_raw.write();
		ExtractFontTextures(_header, cw.ptr(), bw.ptr());
	}

	// Create curve texture as FORMAT_RGBAH (half-float)
	{
		Ref<Image> img;
		img.instance();
		img->create(curve_w, curve_h, false, Image::FORMAT_RGBAH, curve_data);
		_curve_texture.instance();
		_curve_texture->create_from_image(img, 0); // no filter, no mipmap
	}

	// Convert band texture from uint16[4] to float[4] for shader access
	{
		PoolByteArray float_data;
		int pixel_count = band_w * band_h;
		float_data.resize(pixel_count * 4 * sizeof(float));

		{
			PoolByteArray::Read br = band_raw.read();
			PoolByteArray::Write fw = float_data.write();
			const uint16_t *src = reinterpret_cast<const uint16_t *>(br.ptr());
			float *dst = reinterpret_cast<float *>(fw.ptr());
			for (int i = 0; i < pixel_count * 4; i++) {
				dst[i] = static_cast<float>(src[i]);
			}
		}

		Ref<Image> img;
		img.instance();
		img->create(band_w, band_h, false, Image::FORMAT_RGBAF, float_data);
		_band_texture.instance();
		_band_texture->create_from_image(img, 0);
	}
}

float SlugFont::get_ascent() const {
	if (!_header)
		return 0;
	// Cap height is in FontHeightData, accessed via key data
	const FontHeightData *hd = static_cast<const FontHeightData *>(
			GetFontKeyData(_header, kFontKeyHeight));
	return hd ? hd->fontCapHeight : 0.75f;
}

float SlugFont::get_descent() const {
	return 0; // Slug baseline is at 0
}

float SlugFont::get_em_size() const {
	// Em size is always 1.0 in Slug's em-space coordinate system
	return 1.0f;
}

int SlugFont::get_glyph_count() const {
	if (!_header)
		return 0;
	return _header->glyphCount;
}

float SlugFont::get_glyph_advance(int p_unicode) const {
	if (!_header)
		return 0;
	const GlyphData *glyph = GetGlyphData(_header, p_unicode);
	if (!glyph)
		return 0;
	return glyph->advanceWidth;
}

float SlugFont::measure_text(const String &p_text, float p_font_size) const {
	if (!_header || p_text.empty())
		return 0;

	LayoutData layout;
	SetDefaultLayoutData(&layout);
	layout.fontSize = p_font_size;

	CharString utf8 = p_text.utf8();
	float width = MeasureSlug(_header, &layout, utf8.get_data(), -1,
			0, nullptr, nullptr, nullptr, _workspace);
	return width;
}

Dictionary SlugFont::build_text_mesh(const String &p_text, float p_font_size,
		const Vector2 &p_position, const Color &p_color) const {
	Dictionary result;
	result["vertices"] = PoolVector2Array();
	result["uvs"] = PoolVector2Array();
	result["uv2s"] = PoolVector2Array();
	result["colors"] = PoolColorArray();
	result["indices"] = PoolIntArray();
	result["banding"] = PoolRealArray();
	result["bounds"] = Rect2();

	if (!_header || p_text.empty())
		return result;

	LayoutData layout;
	SetDefaultLayoutData(&layout);
	layout.fontSize = p_font_size;
	layout.textColor.color[0].red = (uint8)(p_color.r * 255);
	layout.textColor.color[0].green = (uint8)(p_color.g * 255);
	layout.textColor.color[0].blue = (uint8)(p_color.b * 255);
	layout.textColor.color[0].alpha = (uint8)(p_color.a * 255);

	CharString utf8 = p_text.utf8();

	// Count vertices and triangles
	int32 vert_count = 0, tri_count = 0;
	CountSlug(_header, &layout, utf8.get_data(), -1,
			&vert_count, &tri_count, nullptr, _workspace);

	if (vert_count == 0 || tri_count == 0)
		return result;

	// Allocate buffers
	GlyphVertex *verts = memnew_arr(GlyphVertex, vert_count);
	GlyphTriangle *tris = memnew_arr(GlyphTriangle, tri_count);

	GlyphBuffer buffer;
	buffer.glyphVertex = verts;
	buffer.glyphTriangle = tris;
	buffer.vertexIndex = 0;

	Terathon::Point2D pos;
	pos.x = p_position.x;
	pos.y = p_position.y;
	Box2D text_box;

	BuildSlug(_header, &layout, utf8.get_data(), -1,
			pos, &buffer, &text_box, nullptr, _workspace);

	// Convert to Godot arrays
	PoolVector2Array positions;
	PoolVector2Array uvs;
	PoolVector2Array uv2s;
	PoolColorArray colors;
	PoolIntArray indices;
	PoolRealArray banding;

	positions.resize(vert_count);
	uvs.resize(vert_count);
	uv2s.resize(vert_count);
	colors.resize(vert_count);
	banding.resize(vert_count * 4);

	{
		PoolVector2Array::Write pw = positions.write();
		PoolVector2Array::Write uw = uvs.write();
		PoolVector2Array::Write u2w = uv2s.write();
		PoolColorArray::Write cw = colors.write();
		PoolRealArray::Write bw = banding.write();

		for (int i = 0; i < vert_count; i++) {
			const GlyphVertex &v = verts[i];
			pw[i] = Vector2(v.position.x, v.position.y);
			uw[i] = Vector2(v.texcoord.x, v.texcoord.y);
			uv2s.write()[i] = Vector2(v.glyph.x, v.glyph.y); // raw float bits preserved
			cw[i] = Color(v.color.red / 255.0f, v.color.green / 255.0f,
					v.color.blue / 255.0f, v.color.alpha / 255.0f);
			bw[i * 4 + 0] = v.banding.x;
			bw[i * 4 + 1] = v.banding.y;
			bw[i * 4 + 2] = v.banding.z;
			bw[i * 4 + 3] = v.banding.w;
		}
	}

	indices.resize(tri_count * 3);
	{
		PoolIntArray::Write iw = indices.write();
		for (int i = 0; i < tri_count; i++) {
			iw[i * 3 + 0] = tris[i].index[0];
			iw[i * 3 + 1] = tris[i].index[1];
			iw[i * 3 + 2] = tris[i].index[2];
		}
	}

	memdelete_arr(verts);
	memdelete_arr(tris);

	result["vertices"] = positions;
	result["uvs"] = uvs;
	result["uv2s"] = uv2s;
	result["colors"] = colors;
	result["indices"] = indices;
	result["banding"] = banding;
	result["bounds"] = Rect2(text_box.min.x, text_box.min.y,
			text_box.max.x - text_box.min.x, text_box.max.y - text_box.min.y);

	return result;
}

Ref<ImageTexture> SlugFont::get_curve_texture() const { return _curve_texture; }
Ref<ImageTexture> SlugFont::get_band_texture() const { return _band_texture; }

void SlugFont::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load_from_file", "path"), &SlugFont::load_from_file);
	ClassDB::bind_method(D_METHOD("load_from_buffer", "data"), &SlugFont::load_from_buffer);
	ClassDB::bind_method(D_METHOD("is_valid"), &SlugFont::is_valid);
	ClassDB::bind_method(D_METHOD("get_ascent"), &SlugFont::get_ascent);
	ClassDB::bind_method(D_METHOD("get_descent"), &SlugFont::get_descent);
	ClassDB::bind_method(D_METHOD("get_em_size"), &SlugFont::get_em_size);
	ClassDB::bind_method(D_METHOD("get_glyph_count"), &SlugFont::get_glyph_count);
	ClassDB::bind_method(D_METHOD("get_glyph_advance", "unicode"), &SlugFont::get_glyph_advance);
	ClassDB::bind_method(D_METHOD("measure_text", "text", "font_size"), &SlugFont::measure_text);
	ClassDB::bind_method(D_METHOD("build_text_mesh", "text", "font_size", "position", "color"),
			&SlugFont::build_text_mesh, DEFVAL(Vector2()), DEFVAL(Color(1, 1, 1, 1)));
	ClassDB::bind_method(D_METHOD("get_curve_texture"), &SlugFont::get_curve_texture);
	ClassDB::bind_method(D_METHOD("get_band_texture"), &SlugFont::get_band_texture);
}

// =========================================================================
// SlugMaterial
// =========================================================================

SlugMaterial::SlugMaterial() {
	_create_shader();
}

SlugMaterial::~SlugMaterial() {}

void SlugMaterial::_create_shader() {
	_shader.instance();
	_shader->set_code(slug_shader_code);

	Ref<ShaderMaterial> mat;
	mat.instance();
	mat->set_shader(_shader);
	_material = mat;
}

void SlugMaterial::set_font(const Ref<SlugFont> &p_font) {
	_font = p_font;
	_update_uniforms();
}

Ref<SlugFont> SlugMaterial::get_font() const { return _font; }

void SlugMaterial::_update_uniforms() {
	if (!_material.is_valid())
		return;

	if (_font.is_valid() && _font->is_valid()) {
		_material->set_shader_param("curve_texture", _font->get_curve_texture());
		_material->set_shader_param("band_texture", _font->get_band_texture());
	}
}

void SlugMaterial::update_banding_data(const PoolRealArray &p_data, int p_vertex_count) {
	if (p_vertex_count <= 0)
		return;

	// Create texture dimensions (power-of-2 width for efficiency)
	int width = MIN(p_vertex_count, 256);
	int height = (p_vertex_count + width - 1) / width;

	PoolByteArray tex_data;
	int total_pixels = width * height;
	tex_data.resize(total_pixels * 4 * sizeof(float));

	{
		PoolByteArray::Write tw = tex_data.write();
		PoolRealArray::Read pr = p_data.read();
		float *dst = reinterpret_cast<float *>(tw.ptr());

		int provided = MIN(p_data.size(), total_pixels * 4);
		for (int i = 0; i < provided; i++) {
			dst[i] = pr[i];
		}
		// Zero-fill remaining
		for (int i = provided; i < total_pixels * 4; i++) {
			dst[i] = 0;
		}
	}

	Ref<Image> img;
	img.instance();
	img->create(width, height, false, Image::FORMAT_RGBAF, tex_data);

	if (_banding_texture.is_null()) {
		_banding_texture.instance();
	}
	_banding_texture->create_from_image(img, 0);

	if (_material.is_valid()) {
		_material->set_shader_param("banding_data", _banding_texture);
		_material->set_shader_param("banding_width", width);
	}
}

Ref<ShaderMaterial> SlugMaterial::get_material() const { return _material; }

void SlugMaterial::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_font", "font"), &SlugMaterial::set_font);
	ClassDB::bind_method(D_METHOD("get_font"), &SlugMaterial::get_font);
	ClassDB::bind_method(D_METHOD("update_banding_data", "data", "vertex_count"), &SlugMaterial::update_banding_data);
	ClassDB::bind_method(D_METHOD("get_material"), &SlugMaterial::get_material);
}

// =========================================================================
// SlugLabel
// =========================================================================

SlugLabel::SlugLabel() :
		_font_size(16.0f), _font_color(1, 1, 1, 1), _max_width(0), _dirty(true) {}

SlugLabel::~SlugLabel() {}

void SlugLabel::set_font(const Ref<SlugFont> &p_font) {
	_font = p_font;
	_dirty = true;
	update();
}

Ref<SlugFont> SlugLabel::get_font() const { return _font; }

void SlugLabel::set_text(const String &p_text) {
	if (_text == p_text)
		return;
	_text = p_text;
	_dirty = true;
	update();
}

String SlugLabel::get_text() const { return _text; }

void SlugLabel::set_font_size(float p_size) {
	if (_font_size == p_size)
		return;
	_font_size = MAX(p_size, 1.0f);
	_dirty = true;
	update();
}

float SlugLabel::get_font_size() const { return _font_size; }

void SlugLabel::set_font_color(const Color &p_color) {
	if (_font_color == p_color)
		return;
	_font_color = p_color;
	_dirty = true;
	update();
}

Color SlugLabel::get_font_color() const { return _font_color; }

void SlugLabel::set_max_width(float p_width) {
	if (_max_width == p_width)
		return;
	_max_width = MAX(p_width, 0.0f);
	_dirty = true;
	update();
}

float SlugLabel::get_max_width() const { return _max_width; }

float SlugLabel::get_text_width() const { return _text_bounds.size.x; }
float SlugLabel::get_text_height() const { return _text_bounds.size.y; }
Rect2 SlugLabel::get_text_bounds() const { return _text_bounds; }

void SlugLabel::_ensure_material() {
	if (_slug_material.is_null()) {
		_slug_material.instance();
	}
	if (_font.is_valid() && _slug_material->get_font() != _font) {
		_slug_material->set_font(_font);
	}
}

void SlugLabel::_rebuild_mesh() {
	_dirty = false;
	_mesh.unref();
	_text_bounds = Rect2();

	if (_font.is_null() || !_font->is_valid() || _text.empty())
		return;

	_ensure_material();

	Dictionary mesh_data = _font->build_text_mesh(_text, _font_size, Vector2(), _font_color);

	PoolVector2Array positions = mesh_data["vertices"];
	if (positions.size() == 0)
		return;

	PoolVector2Array uvs = mesh_data["uvs"];
	PoolVector2Array uv2s = mesh_data["uv2s"];
	PoolColorArray colors = mesh_data["colors"];
	PoolIntArray indices = mesh_data["indices"];
	PoolRealArray banding = mesh_data["banding"];
	_text_bounds = mesh_data["bounds"];

	// Build ArrayMesh
	Array arrays;
	arrays.resize(Mesh::ARRAY_MAX);
	arrays[Mesh::ARRAY_VERTEX] = positions;
	arrays[Mesh::ARRAY_TEX_UV] = uvs;
	arrays[Mesh::ARRAY_TEX_UV2] = uv2s;
	arrays[Mesh::ARRAY_COLOR] = colors;
	arrays[Mesh::ARRAY_INDEX] = indices;

	_mesh.instance();
	_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays, Array(),
			Mesh::ARRAY_FLAG_USE_2D_VERTICES);
	_mesh->surface_set_material(0, _slug_material->get_material());

	// Update banding data texture
	_slug_material->update_banding_data(banding, positions.size());
}

void SlugLabel::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			if (_dirty)
				_rebuild_mesh();
			if (_mesh.is_valid()) {
				draw_mesh(_mesh, Ref<Texture>(), Ref<Texture>());
			}
		} break;
	}
}

Rect2 SlugLabel::_edit_get_rect() const { return _text_bounds; }
bool SlugLabel::_edit_use_rect() const { return true; }

String SlugLabel::get_configuration_warning() const {
	if (_font.is_null()) {
		return "SlugLabel requires a SlugFont resource. Set the 'font' property.";
	}
	return String();
}

void SlugLabel::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_font", "font"), &SlugLabel::set_font);
	ClassDB::bind_method(D_METHOD("get_font"), &SlugLabel::get_font);
	ClassDB::bind_method(D_METHOD("set_text", "text"), &SlugLabel::set_text);
	ClassDB::bind_method(D_METHOD("get_text"), &SlugLabel::get_text);
	ClassDB::bind_method(D_METHOD("set_font_size", "size"), &SlugLabel::set_font_size);
	ClassDB::bind_method(D_METHOD("get_font_size"), &SlugLabel::get_font_size);
	ClassDB::bind_method(D_METHOD("set_font_color", "color"), &SlugLabel::set_font_color);
	ClassDB::bind_method(D_METHOD("get_font_color"), &SlugLabel::get_font_color);
	ClassDB::bind_method(D_METHOD("set_max_width", "width"), &SlugLabel::set_max_width);
	ClassDB::bind_method(D_METHOD("get_max_width"), &SlugLabel::get_max_width);
	ClassDB::bind_method(D_METHOD("get_text_width"), &SlugLabel::get_text_width);
	ClassDB::bind_method(D_METHOD("get_text_height"), &SlugLabel::get_text_height);
	ClassDB::bind_method(D_METHOD("get_text_bounds"), &SlugLabel::get_text_bounds);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "font", PROPERTY_HINT_RESOURCE_TYPE, "SlugFont"), "set_font", "get_font");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "text", PROPERTY_HINT_MULTILINE_TEXT), "set_text", "get_text");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "font_size", PROPERTY_HINT_RANGE, "1,256,0.5"), "set_font_size", "get_font_size");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "font_color"), "set_font_color", "get_font_color");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "max_width", PROPERTY_HINT_RANGE, "0,4096,1"), "set_max_width", "get_max_width");
}

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"
#include "doctest/doctest_godot.h"
#include "ts_simd_bench.h"

TEST_SUITE("[[slugfont]] SlugFont") {
	TEST_CASE("[slugfont] default state") {
		SlugFont font;
		CHECK_FALSE(font.is_valid());
		CHECK(font.get_glyph_count() == 0);
		CHECK(font.get_ascent() == doctest::Approx(0.0f));
		CHECK(font.get_curve_texture().is_null());
		CHECK(font.get_band_texture().is_null());
	}

	TEST_CASE("[slugfont] load from empty buffer") {
		SlugFont font;
		PoolByteArray empty;
		EXPECT_ERROR(CHECK(font.load_from_buffer(empty) != OK));
		CHECK_FALSE(font.is_valid());
	}

	TEST_CASE("[slugfont] load invalid file") {
		SlugFont font;
		EXPECT_ERROR(CHECK(font.load_from_file("res://nonexistent_test_font.slug") != OK));
		CHECK_FALSE(font.is_valid());
	}

	TEST_CASE("[slugfont] measure empty text") {
		SlugFont font;
		CHECK(font.measure_text("", 16.0f) == doctest::Approx(0.0f));
	}

	TEST_CASE("[slugfont] build empty text mesh") {
		SlugFont font;
		Dictionary mesh = font.build_text_mesh("", 16.0f);
		PoolVector2Array verts = mesh["vertices"];
		CHECK(verts.size() == 0);
	}

	TEST_CASE("[slugfont] glyph advance without font returns zero") {
		SlugFont font;
		CHECK(font.get_glyph_advance(0x41) == doctest::Approx(0.0f));
	}
}

TEST_SUITE("[[slugfont]] SlugMaterial") {
	TEST_CASE("[slugfont] material creation") {
		SlugMaterial mat;
		CHECK(mat.get_material().is_valid());
		CHECK(mat.get_font().is_null());
	}

	TEST_CASE("[slugfont] banding data update") {
		SlugMaterial mat;
		PoolRealArray banding;
		for (int i = 0; i < 12; i++)
			banding.push_back(float(i));
		mat.update_banding_data(banding, 3);
		// Should not crash; internal texture created
	}

	TEST_CASE("[slugfont] banding data zero vertices") {
		SlugMaterial mat;
		PoolRealArray empty;
		mat.update_banding_data(empty, 0);
		// Should not crash
	}
}

TEST_SUITE("[[slugfont]] SlugLabel") {
	TEST_CASE("[slugfont] default state") {
		SlugLabel label;
		CHECK(label.get_text().empty());
		CHECK(label.get_font_size() == doctest::Approx(16.0f));
		CHECK(label.get_font().is_null());
		CHECK(label.get_font_color() == Color(1, 1, 1, 1));
		CHECK(label.get_max_width() == doctest::Approx(0.0f));
	}

	TEST_CASE("[slugfont] configuration warning without font") {
		SlugLabel label;
		label.set_text("Hello");
		CHECK_FALSE(label.get_configuration_warning().empty());
	}

	TEST_CASE("[slugfont] properties") {
		SlugLabel label;
		label.set_text("Test");
		CHECK(label.get_text() == "Test");

		label.set_font_size(24.0f);
		CHECK(label.get_font_size() == doctest::Approx(24.0f));

		label.set_font_color(Color(1, 0, 0, 1));
		CHECK(label.get_font_color() == Color(1, 0, 0, 1));

		label.set_max_width(200.0f);
		CHECK(label.get_max_width() == doctest::Approx(200.0f));
	}

	TEST_CASE("[slugfont] font size clamps to minimum") {
		SlugLabel label;
		label.set_font_size(0.0f);
		CHECK(label.get_font_size() >= 1.0f);
	}

	TEST_CASE("[slugfont] max width clamps to non-negative") {
		SlugLabel label;
		label.set_max_width(-10.0f);
		CHECK(label.get_max_width() >= 0.0f);
	}
}

TEST_SUITE("[[slugfont]] SIMD backend") {
	TEST_CASE("[slugfont] SIMD detection") {
		Terathon::SimdBenchResult result = Terathon::simd_detect();
		// At least one backend should be active on any supported platform
#if defined(TERATHON_SSE)
		CHECK(result.has_sse);
#elif defined(TERATHON_NEON)
		CHECK(result.has_neon);
#endif
		CHECK(result.backend_name != nullptr);
		CHECK(strlen(result.backend_name) > 0);
	}

	TEST_CASE("[slugfont] SIMD validation") {
		CHECK(Terathon::simd_validate());
	}

	TEST_CASE("[slugfont] Sqrt correctness") {
		CHECK(Terathon::Sqrt(4.0f) == doctest::Approx(2.0f).epsilon(0.001));
		CHECK(Terathon::Sqrt(9.0f) == doctest::Approx(3.0f).epsilon(0.001));
		CHECK(Terathon::Sqrt(1.0f) == doctest::Approx(1.0f).epsilon(0.001));
		CHECK(Terathon::Sqrt(0.0f) == doctest::Approx(0.0f).epsilon(0.001));
	}

	TEST_CASE("[slugfont] InverseSqrt correctness") {
		CHECK(Terathon::InverseSqrt(4.0f) == doctest::Approx(0.5f).epsilon(0.001));
		CHECK(Terathon::InverseSqrt(1.0f) == doctest::Approx(1.0f).epsilon(0.001));
		CHECK(Terathon::InverseSqrt(0.25f) == doctest::Approx(2.0f).epsilon(0.001));
	}
}

#endif // DOCTEST
