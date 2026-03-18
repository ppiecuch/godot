/**************************************************************************/
/*  geom_font_2d.h                                                        */
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

#ifndef GEOM_FONT_2D_H
#define GEOM_FONT_2D_H

#include "scene/2d/node_2d.h"
#include "scene/resources/mesh.h"

class GeomFont2D : public Node2D {
	GDCLASS(GeomFont2D, Node2D);

public:
	enum GeomFontType {
		GEOM_FONT_EASY,
		GEOM_FONT_ASTEROIDS,
		GEOM_FONT_HP1345,
		GEOM_FONT_BOB3D,
		GEOM_FONT_HERSHEY,
		GEOM_FONT_LOWPOLY,
		GEOM_FONT_BITMAP_DOT,
		GEOM_FONT_LCD,
		GEOM_FONT_SQUARE,
		GEOM_FONT_MAX
	};

	enum HersheyFont {
		HERSHEY_FUTURAL,
		HERSHEY_FUTURAM,
		HERSHEY_ROWMANS,
		HERSHEY_ROWMAND,
		HERSHEY_ROWMANT,
		HERSHEY_SCRIPTS,
		HERSHEY_SCRIPTC,
		HERSHEY_CURSIVE,
		HERSHEY_GOTHICENG,
		HERSHEY_GOTHICGER,
		HERSHEY_GOTHICITA,
		HERSHEY_GOTHGBT,
		HERSHEY_GOTHGRT,
		HERSHEY_GOTHITT,
		HERSHEY_TIMESI,
		HERSHEY_TIMESR,
		HERSHEY_TIMESIB,
		HERSHEY_TIMESRB,
		HERSHEY_TIMESG,
		HERSHEY_CYRILLIC,
		HERSHEY_CYRILC_1,
		HERSHEY_GREEK,
		HERSHEY_GREEKC,
		HERSHEY_GREEKS,
		HERSHEY_JAPANESE,
		HERSHEY_SYMBOLIC,
		HERSHEY_MUSIC,
		HERSHEY_MATHLOW,
		HERSHEY_MATHUPP,
		HERSHEY_ASTROLOGY,
		HERSHEY_METEOROLOGY,
		HERSHEY_MARKERS,
		HERSHEY_MAX
	};

	enum BitmapDotStyle {
		DOT_FLAT_CIRCLE,
		DOT_FLAT_SQUARE,
		DOT_TEXTURE_CIRCLE,
		DOT_TEXTURE_SQUARE,
		DOT_TEXTURE_3D_1,
		DOT_TEXTURE_3D_2,
	};

private:
	String _text;
	GeomFontType _font_type;
	HersheyFont _hershey_font;
	Color _font_color;
	Vector2 _font_scale;
	Transform _font_transform;
	real_t _line_width;
	real_t _letter_spacing;
	bool _bbcode_enabled;
	bool _bob3d_wireframe;
	int _bitmap_dot_style;

	bool _dirty;
	Ref<ArrayMesh> _mesh;
	Size2 _text_rect_size;

	// BBCode parser
	struct TextSpan {
		String text;
		Color color;
		Vector2 scale;
	};
	Vector<TextSpan> _parse_bbcode(const String &p_text) const;

	// Per-font mesh builders
	void _build_easy_font_mesh();
	void _build_asteroids_mesh();
	void _build_hp1345_mesh();
	void _build_bob3d_mesh();
	void _build_hershey_mesh();
	void _build_lowpoly_mesh();

	// FB digital font builders (canvas-item based)
	void _build_bitmap_dot(RID p_canvas);
	void _build_lcd(RID p_canvas);
	void _build_square(RID p_canvas);

	void _rebuild();
	void _mark_dirty();

	// Hershey data lookup
	struct HersheyData {
		const char **font_data;
		const int *font_data_size;
		const char *font_width;
		int font_height;
	};
	HersheyData _get_hershey_data() const;

protected:
	void _notification(int p_what);
	static void _bind_methods();
	void _validate_property(PropertyInfo &property) const;

public:
	void set_text(const String &p_text);
	String get_text() const;

	void set_font_type(GeomFontType p_type);
	GeomFontType get_font_type() const;

	void set_hershey_font(HersheyFont p_font);
	HersheyFont get_hershey_font() const;

	void set_font_color(const Color &p_color);
	Color get_font_color() const;

	void set_font_scale(const Vector2 &p_scale);
	Vector2 get_font_scale() const;

	void set_font_transform(const Transform &p_xform);
	Transform get_font_transform() const;

	void set_line_width(real_t p_width);
	real_t get_line_width() const;

	void set_letter_spacing(real_t p_spacing);
	real_t get_letter_spacing() const;

	void set_bbcode_enabled(bool p_enabled);
	bool is_bbcode_enabled() const;

	void set_bob3d_wireframe(bool p_wire);
	bool is_bob3d_wireframe() const;

	void set_bitmap_dot_style(int p_style);
	int get_bitmap_dot_style() const;

	Size2 get_text_size() const;

#ifdef TOOLS_ENABLED
	bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;
	Rect2 _edit_get_rect() const;
	bool _edit_use_rect() const;
#endif

	GeomFont2D();
};

VARIANT_ENUM_CAST(GeomFont2D::GeomFontType);
VARIANT_ENUM_CAST(GeomFont2D::HersheyFont);
VARIANT_ENUM_CAST(GeomFont2D::BitmapDotStyle);

#endif // GEOM_FONT_2D_H
