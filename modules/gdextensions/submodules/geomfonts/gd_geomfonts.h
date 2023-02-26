/**************************************************************************/
/*  gd_geomfonts.h                                                        */
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

#ifndef GD_GEOMFONTS_H
#define GD_GEOMFONTS_H

#include "core/color.h"
#include "core/math/transform_2d.h"
#include "core/reference.h"
#include "scene/2d/node_2d.h"
#include "scene/resources/mesh.h"

#include "misc/c++/handle_map.h"

#include <utility>

class GdGeomFonts : public Reference {
	GDCLASS(GdGeomFonts, Reference);

	handle_map<RID> items;

	Dictionary _cache;

	std::pair<RID, int> _next_item(RID canvas);
	RID _from_handle(int hrid) const;
	bool _valid_handle(int hrid) const;

protected:
	static void _bind_methods();

public:
	// creating multi-text instance
	void mesh_add_bob_font_text(Ref<ArrayMesh> p_mesh, const String &p_text, const Point3 &p_pos = Point3(), real_t p_size = 1, bool p_wire = true);
	void mesh_add_bob_font_text_xform(Ref<ArrayMesh> p_mesh, const String &p_text, const Transform &p_xform, real_t p_size = 1, bool p_wire = true);
	void mesh_add_easy_font_text(Ref<ArrayMesh> p_mesh, const String &p_text, const Point2 &p_pos = Point2(), real_t p_spacing = 0);
	void mesh_add_easy_font_text_xform(Ref<ArrayMesh> p_mesh, const String &p_text, const Transform &p_xform, real_t p_spacing = 0);
	void mesh_add_hp_font_text(Ref<ArrayMesh> p_mesh, const String &p_text, const Point2 &p_pos = Point2(), const Size2 &p_scale = Size2(10, 10));
	void mesh_add_hp_font_text_xform(Ref<ArrayMesh> p_mesh, const String &p_text, const Transform &p_xform);
	int mesh_add_finish(RID p_canvas, Ref<ArrayMesh> p_mesh);

	// direct drawing functions
	int easy_font_text(RID p_canvas, const String &p_text, const Point2 &p_pos = Point2(), real_t p_spacing = 0);
	int easy_font_text_xform(RID p_canvas, const String &p_text, const Transform &p_xform, real_t p_spacing = 0);
	Size2 easy_font_text_size(const String &p_text) const;

	int bob_font_text(RID p_canvas, const String &p_text, const Transform &p_xform, real_t p_size = 1, bool p_wire = true);
	int bob_font_text_xform(RID p_canvas, const String &p_text, const Transform &p_xform, real_t p_size = 1, bool p_wire = true);
	Size2 bob_font_text_size(const String &p_text) const;

	int hp_font_text(RID p_canvas, const String &p_text, const Point2 &p_pos = Point2(), const Size2 &p_scale = Size2(10, 10));
	int hp_font_text_xform(RID p_canvas, const String &p_text, const Transform &p_xform);
	Size2 hp_font_text_size(const String &p_text, const Size2 &p_scale) const;

	int aster_font_text(RID p_canvas, const String &p_text, const Point2 &p_pos = Point2(), const Size2 &p_scale = Size2(10, 10));
	int aster_font_text_xform(RID p_canvas, const String &p_text, const Transform &p_xform);
	Size2 aster_font_text_size(const String &p_text, const Size2 &p_scale) const;

	int bitmap_font_text(RID p_canvas, const String &p_text, const Point2 &p_pos = Point2());
	int lcd_font_text(RID p_canvas, const String &p_text, const Point2 &p_pos = Point2());
	int square_font_text(RID p_canvas, const String &p_text, const Point2 &p_pos = Point2());

	// manage text instance
	void set_transform(int p_index, const Transform2D &p_xform);
	void set_modulate_color(int p_index, const Color &p_color);

	void clear_all_items();
	void clear_item(int p_index);

	GdGeomFonts();
	~GdGeomFonts();
};

#endif // GD_GEOMFONTS_H
