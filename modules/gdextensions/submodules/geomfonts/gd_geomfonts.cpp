/*************************************************************************/
/*  gd_geomfonts.cpp                                                     */
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

#include "gd_geomfonts.h"

#include "bluntfont/blutfont.cpp"
#include "bob3d/bob_font.cpp"
#include "easyfont/stb_easy_font.h"
#include "fbdigitalfont/fb.cpp"
#include "hershey/hershey_render.h"
#include "leonsans/leonsans.h"
#include "polyfonts/polygodot.cpp"
#include "simplevector/asteroids.c"
#include "simplevector/hp1345.c"
#include "ttftriangulator/ttftriangulator.cpp"

#include "common/gd_core.h"
#include "scene/main/scene_tree.h"
#include "scene/main/viewport.h"
#include "scene/resources/mesh.h"
#include "servers/visual_server.h"

void GdGeomFonts::DrawItems::clear() {
	if (auto *vs = VS::get_singleton()) {
		for (RID &rid : res) {
			if (rid.is_valid()) {
				vs->free(rid);
			}
		}
	}
	res.clear();
}

void GdGeomFonts::DrawItems::set_transform(const Transform2D &p_xform) {
	for (RID &rid : res) {
		if (rid.is_valid()) {
			VisualServer::get_singleton()->canvas_item_set_transform(rid, p_xform);
		}
	}
}

void GdGeomFonts::DrawItems::set_modulate_color(const Color &p_color) {
	for (RID &rid : res) {
		if (rid.is_valid()) {
			VisualServer::get_singleton()->canvas_item_set_modulate(rid, p_color);
		}
	}
}

GdGeomFonts::DrawItems::DrawItems(RID p_canvas, int p_num_res) {
	ERR_FAIL_COND(!p_canvas.is_valid());
	auto *vs = VS::get_singleton();
	for (int r = 0; r < p_num_res; r++) {
		RID rid = vs->canvas_item_create();
		if (rid.is_valid()) {
			vs->canvas_item_set_parent(rid, p_canvas);
		}
		res.push_back(rid);
	}
}

GdGeomFonts::DrawItems::DrawItems() {
}

void GdGeomFonts::group_add_bob_font_text(Ref<ArrayMesh> p_mesh, const String &p_text, const Point3 &p_pos, real_t p_size, bool p_wire) {
	bob_font_draw_string(p_mesh, p_text.ascii().c_str(), p_pos, p_size, p_wire);
}

void GdGeomFonts::group_add_bob_font_text_xform(Ref<ArrayMesh> p_mesh, const String &p_text, const Transform &p_xform, real_t p_size, bool p_wire) {
	bob_font_draw_string(p_mesh, p_text.ascii().c_str(), p_xform, p_size, p_wire);
}

void GdGeomFonts::group_add_easy_font_text(Ref<ArrayMesh> p_mesh, const String &p_text, const Point2 &p_pos, real_t p_spacing) {
	stb_easy_font_spacing(p_spacing);
	stb_easy_font_print_string(p_mesh, p_pos, p_text.ascii().c_str());
}

void GdGeomFonts::group_add_easy_font_text_xform(Ref<ArrayMesh> p_mesh, const String &p_text, const Transform &p_xform, real_t p_spacing) {
	stb_easy_font_spacing(p_spacing);
	stb_easy_font_print_string_xform(p_mesh, p_xform, p_text.ascii().c_str());
}

int GdGeomFonts::group_add_finish(Ref<ArrayMesh> p_mesh) {
	int id = items.size();
	items.push_back({ canvas, 1 });
	VisualServer::get_singleton()->canvas_item_add_mesh(items.last().get_res(0), p_mesh->get_rid());
	return id;
}

int GdGeomFonts::easy_font_draw_text(const String &p_text, const Point2 &p_pos, real_t p_spacing) {
	int id = items.size();
	return id;
}

int GdGeomFonts::easy_font_draw_text_xform(const String &p_text, const Transform &p_xform, real_t p_spacing) {
	int id = items.size();
	return id;
}

Size2 GdGeomFonts::easy_font_text_size(const String &p_text) {
	const char *ptr = p_text.ascii().c_str();
	return Size2(stb_easy_font_width(ptr), stb_easy_font_height(ptr));
}

int GdGeomFonts::bitmap_font_draw_text(const String &p_text, const Point2 &p_pos) {
	int id = items.size();
	items.push_back({ canvas, 2 });
	FBBitmapFontView fnt(items.last().get_res(0), items.last().get_res(1), _cache);
	fnt.set_text(p_text);
	fnt.draw();
	return id;
}

void GdGeomFonts::set_transform(int p_index, const Transform2D &p_xform) {
	ERR_FAIL_INDEX(p_index, items.size());
	items.get(p_index).set_transform(p_xform);
	emit_signal("changed", array(p_index));
}

void GdGeomFonts::set_modulate_color(int p_index, const Color &p_color) {
	ERR_FAIL_INDEX(p_index, items.size());
	items.get(p_index).set_modulate_color(p_color);
	emit_signal("changed", array(p_index));
}

void GdGeomFonts::clear_all() {
	for (DrawItems &rid : items) {
		rid.clear();
	}
	items.clear();
}

void GdGeomFonts::_bind_methods() {
	ClassDB::bind_method(D_METHOD("group_add_bob_font_text", "mesh", "text", "pos", "size", "wire"), &GdGeomFonts::group_add_bob_font_text, DEFVAL(Point2()), DEFVAL(1), DEFVAL(true));
	ClassDB::bind_method(D_METHOD("group_add_bob_font_xform", "mesh", "text", "xform", "size", "wire"), &GdGeomFonts::group_add_bob_font_text_xform, DEFVAL(1), DEFVAL(true));

	ClassDB::bind_method(D_METHOD("group_add_easy_font_text", "mesh", "text", "pos", "spacing"), &GdGeomFonts::group_add_easy_font_text, DEFVAL(Point2()), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("group_add_easy_font_text_xform", "mesh", "text", "xform", "spacing"), &GdGeomFonts::group_add_easy_font_text_xform, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("group_add_finish", "mesh"), &GdGeomFonts::group_add_finish);

	ClassDB::bind_method(D_METHOD("easy_font_draw_text", "text", "pos", "spacing"), &GdGeomFonts::easy_font_draw_text, DEFVAL(Point2()), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("easy_font_draw_text_xform", "text", "xform", "spacing"), &GdGeomFonts::easy_font_draw_text_xform, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("easy_font_text_size", "text"), &GdGeomFonts::easy_font_text_size);

	ClassDB::bind_method(D_METHOD("bitmap_font_draw_text", "text", "pos"), &GdGeomFonts::bitmap_font_draw_text, DEFVAL(Point2()));

	ClassDB::bind_method(D_METHOD("set_transform", "index", "xform"), &GdGeomFonts::set_transform);
	ClassDB::bind_method(D_METHOD("set_modulate_color", "index", "color"), &GdGeomFonts::set_modulate_color);
	ClassDB::bind_method(D_METHOD("clear_all"), &GdGeomFonts::clear_all);

	ADD_SIGNAL(MethodInfo("changed"));
}

GdGeomFonts::GdGeomFonts() {
	_mat_trnsp = newref(CanvasItemMaterial);
	_mat_trnsp->set_blend_mode(CanvasItemMaterial::BLEND_MODE_MIX);
	_cache["mat_trnsp"] = _mat_trnsp;

	auto *st = SceneTree::get_singleton();
	ERR_FAIL_NULL(st);
	auto *viewport = st->get_root()->get_viewport();
	ERR_FAIL_NULL(viewport);
	auto *vs = VS::get_singleton();
	canvas = vs->canvas_create();
	ERR_FAIL_COND(!canvas.is_valid());
	vs->viewport_attach_canvas(viewport->get_viewport_rid(), canvas);
	vs->viewport_set_canvas_stacking(viewport->get_viewport_rid(), canvas, (~0U) >> 1, (~0U) >> 1);
}

GdGeomFonts::~GdGeomFonts() {
	if (auto *vs = VS::get_singleton()) {
		if (canvas.is_valid()) {
			vs->free(canvas);
		}
		items.clear();
	}
}
