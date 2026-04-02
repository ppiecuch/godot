/**************************************************************************/
/*  gd_slug.h                                                             */
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

#ifndef GD_SLUG_H
#define GD_SLUG_H

#include "core/reference.h"
#include "scene/2d/node_2d.h"
#include "scene/resources/mesh.h"
#include "scene/resources/shader.h"
#include "scene/resources/texture.h"

#include "ts_slug.h"

// =========================================================================
// SlugFont — Resource that loads a .slug font file
// =========================================================================

class SlugFont : public Resource {
	GDCLASS(SlugFont, Resource);

	PoolByteArray _font_data;
	const Terathon::Slug::FontHeader *_header;
	Ref<ImageTexture> _curve_texture;
	Ref<ImageTexture> _band_texture;
	Terathon::Slug::Workspace *_workspace;

	void _extract_textures();

protected:
	static void _bind_methods();

public:
	Error load_from_file(const String &p_path);
	Error load_from_buffer(const PoolByteArray &p_data);
	bool is_valid() const;

	// Font metrics
	float get_ascent() const;
	float get_descent() const;
	float get_em_size() const;
	int get_glyph_count() const;
	float get_glyph_advance(int p_unicode) const;

	// Text measurement
	float measure_text(const String &p_text, float p_font_size) const;

	// Text mesh generation
	// Returns Dictionary with: vertices, uvs, uv2s, colors, indices, banding, bounds
	Dictionary build_text_mesh(const String &p_text, float p_font_size,
			const Vector2 &p_position = Vector2(),
			const Color &p_color = Color(1, 1, 1, 1)) const;

	// Texture accessors
	Ref<ImageTexture> get_curve_texture() const;
	Ref<ImageTexture> get_band_texture() const;

	// Internal
	const Terathon::Slug::FontHeader *get_header() const { return _header; }
	Terathon::Slug::Workspace *get_workspace() const { return _workspace; }

	SlugFont();
	~SlugFont();
};

// =========================================================================
// SlugMaterial — Manages the Godot shader + font texture uniforms
// =========================================================================

class SlugMaterial : public Reference {
	GDCLASS(SlugMaterial, Reference);

	Ref<ShaderMaterial> _material;
	Ref<Shader> _shader;
	Ref<SlugFont> _font;
	Ref<ImageTexture> _banding_texture;

	void _create_shader();
	void _update_uniforms();

protected:
	static void _bind_methods();

public:
	void set_font(const Ref<SlugFont> &p_font);
	Ref<SlugFont> get_font() const;

	void update_banding_data(const PoolRealArray &p_data, int p_vertex_count);

	Ref<ShaderMaterial> get_material() const;

	SlugMaterial();
	~SlugMaterial();
};

// =========================================================================
// SlugLabel — 2D text rendering node using Slug GPU font rendering
// =========================================================================

class SlugLabel : public Node2D {
	GDCLASS(SlugLabel, Node2D);

	Ref<SlugFont> _font;
	String _text;
	float _font_size;
	Color _font_color;
	float _max_width;

	Ref<SlugMaterial> _slug_material;
	Ref<ArrayMesh> _mesh;
	Rect2 _text_bounds;
	bool _dirty;

	void _rebuild_mesh();
	void _ensure_material();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_font(const Ref<SlugFont> &p_font);
	Ref<SlugFont> get_font() const;

	void set_text(const String &p_text);
	String get_text() const;

	void set_font_size(float p_size);
	float get_font_size() const;

	void set_font_color(const Color &p_color);
	Color get_font_color() const;

	void set_max_width(float p_width);
	float get_max_width() const;

	float get_text_width() const;
	float get_text_height() const;
	Rect2 get_text_bounds() const;

	Rect2 _edit_get_rect() const;
	bool _edit_use_rect() const;
	String get_configuration_warning() const;

	SlugLabel();
	~SlugLabel();
};

#endif // GD_SLUG_H
