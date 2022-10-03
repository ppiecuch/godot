/*************************************************************************/
/*  gd_geomfonts.h                                                       */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef GD_GEOMFONTS_H
#define GD_GEOMFONTS_H

#include "core/color.h"
#include "core/pair.h"
#include "core/math/transform_2d.h"
#include "core/reference.h"
#include "scene/2d/canvas_item.h"
#include "scene/resources/mesh.h"

class GdGeomFonts : public Reference {
	GDCLASS(GdGeomFonts, Reference);

	struct DrawItems {
		RID r1, r2;
		void clear();
		bool set_transform(const Transform2D &p_xform);
		bool set_modulate_color(const Color &p_color);
		DrawItems() { }
		DrawItems(RID r1) : r1(r1) { }
		DrawItems(RID r1, RID r2) : r1(r1), r2(r2) { }
	};

	RID canvas;
	Vector<DrawItems> items;

	Dictionary _cache;
	Ref<ArrayMesh> _mesh;
	Ref<CanvasItemMaterial> _mat_trnsp;

	bool _is_ready();
	RID _new_item();

protected:
	static void _bind_methods();

private:
	void set_transform(int p_index, const Transform2D &p_xform);
	void set_modulate_color(int p_index, const Color &p_color);

	void bob_font_add_text(const String &p_text, const Point3 &p_pos = Point3(), real_t p_size = 1, bool p_wire = true);
	void bob_font_add_text_xform(const String &p_text, const Transform &p_xform, const Point3 &p_pos = Point3(), real_t p_size = 1, bool p_wire = true);

	void easy_font_add_text(const String &p_text, const Point2 &p_pos = Point2(), real_t p_spacing = 0);
	void easy_font_add_text_xform(const String &p_text, const Transform &p_xform, const Point2 &p_pos = Point2(), real_t p_spacing = 0);
	Size2 easy_font_text_size(const String &p_text);

	int font_add_finish();
	void clear();

	int bitmap_font_draw_text(const String &p_text, const Point2 &p_pos = Point2());
	int lcd_font_draw_text(const String &p_text, const Point2 &p_pos = Point2());
	int square_font_draw_text(const String &p_text, const Point2 &p_pos = Point2());

	GdGeomFonts();
	~GdGeomFonts();
};

#endif // GD_GEOMFONTS_H
