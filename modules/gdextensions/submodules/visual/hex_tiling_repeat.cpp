/**************************************************************************/
/*  hex_tiling_repeat.cpp                                                 */
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
#include "doctest/doctest_godot.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

#include "hex_tiling_repeat.h"

// Godot 3.x canvas_item shader — ported and fixed from the original GameMaker source.
//
// Algorithm overview (see paper: https://jcgt.org/published/0011/03/05/):
//   1. Transform UV into a skewed hex-grid coordinate system.
//   2. Identify which of the three nearest hex-cell vertices dominates (via
//      the fractional barycentric coords inside the triangle).
//   3. For each of the three surrounding hex cells, sample the texture with
//      a random rotation/scale/offset derived from a per-cell hash.
//   4. Blend the three samples with power-weighted barycentric weights so
//      the contribution is smoothest at cell centers.
//
// Shader-to-Godot translation notes:
//   - v_vTexcoord  → UV          (Godot canvas_item built-in)
//   - gm_BaseTexture → albedo_texture (explicit uniform sampler2D)
//   - gl_FragColor → COLOR       (Godot fragment output)
//   - texture2D()  → texture()   (GLSL 3 / Godot convention)
//   - void main()  → void fragment()
//   - varying declarations removed (Godot provides UV/COLOR automatically)
//   - Added: named constant for 1/sin(60°), coldot zero-guard, hint annotations
static const char *_hex_tiling_shader_code = R"(
shader_type canvas_item;

uniform vec2  tex_repeat  = vec2(1.0, 1.0);
uniform float sharpness   : hint_range(0.1, 20.0, 0.1) = 6.0;
uniform float hex_size    : hint_range(0.01, 10.0, 0.01) = 1.0;
uniform sampler2D albedo_texture : hint_albedo;

// 1 / sin(60°) = 2/sqrt(3) ≈ 1.15470053838
const float INV_SIN60 = 1.15470053838;

vec3 Round3(vec3 v) {
	return floor(v + vec3(0.5));
}

// Rotate UV around the tiling center (vec2(0.5) * tex_repeat).
vec2 Rotate(vec2 UV, float angle) {
	vec2 center = vec2(0.5) * tex_repeat;
	UV -= center;
	float c = cos(angle);
	float s = sin(angle);
	return vec2(c * UV.x + s * UV.y, c * UV.y - s * UV.x) + center;
}

// 3-component hash from a 2D seed — output in [0,1)^3.
vec3 Hash2(vec2 UV) {
	return fract(sin(vec3(
		dot(vec3(UV.x, UV.y, UV.x), vec3(127.09,  311.7,   74.69)),
		dot(vec3(UV.y, UV.x, UV.x), vec3(269.5,   183.3,  246.1 )),
		dot(vec3(UV.x, UV.y, UV.y), vec3(113.5,   271.89, 124.59))
	)) * 43758.5453);
}

// Apply a random rotation, uniform scale, and translation to UV.
// The transform parameters are derived from the hash of seed.
vec2 RandomTransform(vec2 UV, vec2 seed) {
	vec3 hash = Hash2(seed);
	float rot = mix(-3.14159265, 3.14159265, fract(hash.b * 16.0));
	float scl = mix(0.8, 1.2, hash.b);
	return Rotate(UV, rot) * scl + hash.xy;
}

void fragment() {
	// 1. Scale UVs by the repeat factor.
	vec2 base_uv = UV * tex_repeat;

	// 2. Transform to skewed hex-grid space.
	//    The skew matrix maps square UV to a hex lattice:
	//      u' = u - (0.5 * INV_SIN60) * v
	//      v' = INV_SIN60 * v
	vec2 uv = vec2(
		base_uv.x - (0.5 * INV_SIN60) * base_uv.y,
		INV_SIN60 * base_uv.y
	) / hex_size;

	// 3. Integer hex-cell coordinate and fractional position within it.
	vec2 coord = floor(uv);

	// 4. Compute barycentric-like hex vertex weights.
	//    color.rgb will hold three 0-or-1 flags identifying the dominant vertex.
	vec4 color = vec4(coord.x, coord.y, 0.0, 1.0);
	color.rgb = ((vec3(color.r - color.g) + vec3(0.0, 1.0, 2.0)) * 0.3333333) + (5.0 / 3.0);
	color.rgb = Round3(fract(color.rgb));

	// 5. Fractional position inside the hex triangle (signed, centered at 0).
	vec4 refcol = vec4(fract(uv), 1.0, 1.0);
	refcol.rgb  = vec3(refcol.g + refcol.r) - 1.0;
	vec4 abscol = vec4(abs(refcol.rgb), 1.0);

	// 6. Handle the "upper" triangle by flipping UVs.
	vec4 refswz  = vec4(fract(uv.yx), 1.0, 1.0);
	vec4 use_col = vec4(fract(uv),    1.0, 1.0);

	float flip_check = 0.0;
	if ((refcol.r + refcol.g + refcol.b) / 3.0 > 0.0) {
		use_col    = vec4(1.0 - refswz.x, 1.0 - refswz.y, refswz.b, refswz.a);
		flip_check = 1.0;
	}

	// 7. Power-weighted blend: high sharpness → hard cell edges.
	abscol.rgb = abs(vec3(abscol.r, use_col.r, use_col.g));
	use_col.rgb = vec3(
		pow(dot(abscol.rgb, vec3(color.z, color.x, color.y)), sharpness),
		pow(dot(abscol.rgb, vec3(color.y, color.z, color.x)), sharpness),
		pow(dot(abscol.rgb, color.rgb),                       sharpness)
	);

	// Normalize so weights sum to 1. Guard against all-zero (degenerate case).
	float coldot = max(dot(use_col.rgb, vec3(1.0)), 0.0001);
	use_col /= coldot;

	// 8. Build per-cell hash seeds from the dominant-vertex flags + cell coord.
	//    color.a is always 1.0 (constant offset to decorrelate the three seeds).
	//    color.z is always 0.0 after Round3(fract(0.0)).
	vec2 color_swiz1 = vec2(color.a, color.z); // (1, 0)  — constant offset
	vec2 color_swiz2 = vec2(color.z, color.x); // (0, color.r)
	vec2 color_swiz3 = vec2(color.x, color.a); // (color.r, 1)

	color.rgb *= flip_check;

	// 9. Sample the texture three times with random per-cell transforms, blend.
	vec4 ruv1 = texture(albedo_texture, RandomTransform(base_uv, color_swiz1 + vec2(color.r) + coord)) * vec4(vec3(use_col.r), 1.0);
	vec4 ruv2 = texture(albedo_texture, RandomTransform(base_uv, color_swiz2 + vec2(color.g) + coord)) * vec4(vec3(use_col.g), 1.0);
	vec4 ruv3 = texture(albedo_texture, RandomTransform(base_uv, color_swiz3 + vec2(color.b) + coord)) * vec4(vec3(use_col.b), 1.0);

	COLOR = ruv1 + ruv2 + ruv3;
}
)";

// ---------------------------------------------------------------------------

void HexTilingMaterial::set_tex_repeat(const Vector2 &p_repeat) {
	_tex_repeat = p_repeat;
	set_shader_param("tex_repeat", _tex_repeat);
}

Vector2 HexTilingMaterial::get_tex_repeat() const {
	return _tex_repeat;
}

void HexTilingMaterial::set_sharpness(float p_sharpness) {
	_sharpness = MAX(p_sharpness, 0.1f);
	set_shader_param("sharpness", _sharpness);
}

float HexTilingMaterial::get_sharpness() const {
	return _sharpness;
}

void HexTilingMaterial::set_hex_size(float p_size) {
	_hex_size = MAX(p_size, 0.01f);
	set_shader_param("hex_size", _hex_size);
}

float HexTilingMaterial::get_hex_size() const {
	return _hex_size;
}

void HexTilingMaterial::set_hex_texture(const Ref<Texture> &p_tex) {
	_hex_texture = p_tex;
	set_shader_param("albedo_texture", _hex_texture);
}

Ref<Texture> HexTilingMaterial::get_hex_texture() const {
	return _hex_texture;
}

void HexTilingMaterial::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_tex_repeat", "repeat"), &HexTilingMaterial::set_tex_repeat);
	ClassDB::bind_method(D_METHOD("get_tex_repeat"), &HexTilingMaterial::get_tex_repeat);

	ClassDB::bind_method(D_METHOD("set_sharpness", "sharpness"), &HexTilingMaterial::set_sharpness);
	ClassDB::bind_method(D_METHOD("get_sharpness"), &HexTilingMaterial::get_sharpness);

	ClassDB::bind_method(D_METHOD("set_hex_size", "size"), &HexTilingMaterial::set_hex_size);
	ClassDB::bind_method(D_METHOD("get_hex_size"), &HexTilingMaterial::get_hex_size);

	ClassDB::bind_method(D_METHOD("set_hex_texture", "texture"), &HexTilingMaterial::set_hex_texture);
	ClassDB::bind_method(D_METHOD("get_hex_texture"), &HexTilingMaterial::get_hex_texture);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "hex_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_hex_texture", "get_hex_texture");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "tex_repeat"), "set_tex_repeat", "get_tex_repeat");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "hex_size", PROPERTY_HINT_RANGE, "0.01,10.0,0.01"), "set_hex_size", "get_hex_size");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "sharpness", PROPERTY_HINT_RANGE, "0.1,20.0,0.1"), "set_sharpness", "get_sharpness");
}

HexTilingMaterial::HexTilingMaterial() :
		_tex_repeat(Vector2(1.0f, 1.0f)),
		_sharpness(6.0f),
		_hex_size(1.0f) {
	Ref<Shader> shader;
	shader.instance();
	shader->set_code(_hex_tiling_shader_code);
	set_shader(shader);

	set_shader_param("tex_repeat", _tex_repeat);
	set_shader_param("sharpness", _sharpness);
	set_shader_param("hex_size", _hex_size);
}

// -- Tests --

#ifdef DOCTEST

TEST_CASE("[HexTilingMaterial] default constructor values") {
	HexTilingMaterial mat;
	CHECK(mat.get_tex_repeat() == Vector2(1.0f, 1.0f));
	CHECK(mat.get_sharpness() == doctest::Approx(6.0f));
	CHECK(mat.get_hex_size() == doctest::Approx(1.0f));
	CHECK(mat.get_hex_texture().is_null());
	CHECK(mat.get_shader().is_valid());
}

TEST_CASE("[HexTilingMaterial] shader code is non-empty and contains canvas_item type") {
	HexTilingMaterial mat;
	String code = mat.get_shader()->get_code();
	CHECK(code.length() > 0);
	CHECK(code.find("shader_type canvas_item") != -1);
	CHECK(code.find("void fragment") != -1);
	CHECK(code.find("albedo_texture") != -1);
}

TEST_CASE("[HexTilingMaterial] tex_repeat round-trip") {
	HexTilingMaterial mat;
	mat.set_tex_repeat(Vector2(4.0f, 2.0f));
	CHECK(mat.get_tex_repeat().x == doctest::Approx(4.0f));
	CHECK(mat.get_tex_repeat().y == doctest::Approx(2.0f));
}

TEST_CASE("[HexTilingMaterial] sharpness clamps to minimum 0.1") {
	HexTilingMaterial mat;
	mat.set_sharpness(0.0f);
	CHECK(mat.get_sharpness() == doctest::Approx(0.1f));
	mat.set_sharpness(-5.0f);
	CHECK(mat.get_sharpness() == doctest::Approx(0.1f));
	mat.set_sharpness(12.0f);
	CHECK(mat.get_sharpness() == doctest::Approx(12.0f));
}

TEST_CASE("[HexTilingMaterial] hex_size clamps to minimum 0.01") {
	HexTilingMaterial mat;
	mat.set_hex_size(0.0f);
	CHECK(mat.get_hex_size() == doctest::Approx(0.01f));
	mat.set_hex_size(-1.0f);
	CHECK(mat.get_hex_size() == doctest::Approx(0.01f));
	mat.set_hex_size(3.5f);
	CHECK(mat.get_hex_size() == doctest::Approx(3.5f));
}

TEST_CASE("[HexTilingMaterial] shader param sync — sharpness written to shader") {
	HexTilingMaterial mat;
	mat.set_sharpness(15.0f);
	// set_shader_param stores in the ShaderMaterial param map
	Variant v = mat.get_shader_param("sharpness");
	CHECK(float(v) == doctest::Approx(15.0f));
}

TEST_CASE("[HexTilingMaterial] shader param sync — tex_repeat written to shader") {
	HexTilingMaterial mat;
	mat.set_tex_repeat(Vector2(3.0f, 3.0f));
	Variant v = mat.get_shader_param("tex_repeat");
	Vector2 stored = v;
	CHECK(stored.x == doctest::Approx(3.0f));
	CHECK(stored.y == doctest::Approx(3.0f));
}

TEST_CASE("[HexTilingMaterial] shader param sync — hex_size written to shader") {
	HexTilingMaterial mat;
	mat.set_hex_size(2.5f);
	Variant v = mat.get_shader_param("hex_size");
	CHECK(float(v) == doctest::Approx(2.5f));
}

TEST_CASE("[HexTilingMaterial] multiple instances share separate shader params") {
	HexTilingMaterial a, b;
	a.set_sharpness(2.0f);
	b.set_sharpness(18.0f);
	CHECK(a.get_sharpness() == doctest::Approx(2.0f));
	CHECK(b.get_sharpness() == doctest::Approx(18.0f));
	CHECK(float(a.get_shader_param("sharpness")) == doctest::Approx(2.0f));
	CHECK(float(b.get_shader_param("sharpness")) == doctest::Approx(18.0f));
}

TEST_CASE("[HexTilingMaterial] tex_repeat (1,1) — identity tiling") {
	HexTilingMaterial mat;
	mat.set_tex_repeat(Vector2(1.0f, 1.0f));
	Vector2 r = mat.get_tex_repeat();
	CHECK(r.x == doctest::Approx(1.0f));
	CHECK(r.y == doctest::Approx(1.0f));
}

TEST_CASE("[HexTilingMaterial] hex_texture null by default, set/get round-trip") {
	HexTilingMaterial mat;
	CHECK(mat.get_hex_texture().is_null());

	// After setting a valid texture the getter returns the same object.
	Ref<ImageTexture> tex;
	tex.instance();
	mat.set_hex_texture(tex);
	CHECK(mat.get_hex_texture() == tex);

	// Clearing to null is allowed.
	mat.set_hex_texture(Ref<Texture>());
	CHECK(mat.get_hex_texture().is_null());
}

#endif
