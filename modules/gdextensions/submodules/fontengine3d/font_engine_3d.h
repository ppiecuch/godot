/**************************************************************************/
/*  font_engine_3d.h                                                      */
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

#ifndef FONTENGINE_3D_H
#define FONTENGINE_3D_H

#include "core/hash_map.h"
#include "core/reference.h"
#include "core/ustring.h"
#include "core/variant.h"
#include "scene/3d/mesh_instance.h"
#include "scene/3d/spatial.h"
#include "scene/resources/mesh.h"

class FontEngine3D : public Reference {
	GDCLASS(FontEngine3D, Reference);

public:
	enum HAlign {
		HALIGN_LEFT = 0,
		HALIGN_CENTER,
		HALIGN_RIGHT,
	};

	struct FontInfo {
		String name;
		String path;
		Vector<String> glyphs;
		bool has_materials;
		Vector<String> textures;
	};

private:
	String _catalog_path;
	Vector<FontInfo> _fonts;
	HashMap<String, HashMap<CharType, Ref<ArrayMesh>>> _cache;
	bool _catalog_loaded;

	static FontEngine3D *singleton;

	void _load_catalog();
	String _char_to_filename(CharType c) const;
	const FontInfo *_get_font_info(int p_style) const;

protected:
	static void _bind_methods();

public:
	static FontEngine3D *get_singleton();

	void set_catalog_path(const String &p_path);
	String get_catalog_path() const;
	bool load_catalog(const String &p_path);

	int get_font_count() const;
	String get_font_name(int p_style) const;
	PoolStringArray get_font_names() const;

	bool has_glyph(int p_style, CharType c) const;
	String get_glyph_resource_path(int p_style, CharType c) const;
	Ref<ArrayMesh> get_glyph_mesh(int p_style, CharType c);
	float get_glyph_advance(int p_style, CharType c);
	float get_text_width(const String &p_text, int p_style, float p_spacing) const;
	int get_supported_glyph_count(int p_style) const;
	String get_supported_chars(int p_style) const;

	FontEngine3D();
	~FontEngine3D();
};

class Font3DLabel : public Spatial {
	GDCLASS(Font3DLabel, Spatial);

	String _text;
	int _font_style;
	Color _font_color;
	float _letter_spacing;
	float _font_size;
	FontEngine3D::HAlign _h_align;

	bool _dirty;
	Vector<MeshInstance *> _glyph_nodes;

	void _update_mesh();
	void _clear_glyphs();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_text(const String &p_text);
	String get_text() const;

	void set_font_style(int p_style);
	int get_font_style() const;

	void set_font_color(const Color &p_color);
	Color get_font_color() const;

	void set_letter_spacing(float p_spacing);
	float get_letter_spacing() const;

	void set_font_size(float p_size);
	float get_font_size() const;

	void set_horizontal_align(FontEngine3D::HAlign p_align);
	FontEngine3D::HAlign get_horizontal_align() const;

	float get_text_width() const;

	Font3DLabel();
	~Font3DLabel();
};

VARIANT_ENUM_CAST(FontEngine3D::HAlign);

#endif // FONTENGINE_3D_H
