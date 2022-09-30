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
		if (r1.is_valid()) {
			vs->free(r1);
		}
		if (r2.is_valid()) {
			vs->free(r2);
		}
	}
}

bool GdGeomFonts::DrawItems::set_transform(const Transform2D &p_xform) {
	if (r1.is_valid()) {
		VisualServer::get_singleton()->canvas_item_set_transform(r1, p_xform);
	}
	if (r2.is_valid()) {
		VisualServer::get_singleton()->canvas_item_set_transform(r2, p_xform);
	}
	return r1.is_valid() || r2.is_valid();
}

bool GdGeomFonts::DrawItems::set_modulate_color(const Color &p_color) {
	if (r1.is_valid()) {
		VisualServer::get_singleton()->canvas_item_set_modulate(r1, p_color);
	}
	if (r2.is_valid()) {
		VisualServer::get_singleton()->canvas_item_set_modulate(r2, p_color);
	}
	return r1.is_valid() || r2.is_valid();
}

bool GdGeomFonts::_is_ready() {
	if (!canvas.is_valid()) {
		auto *st = SceneTree::get_singleton();
		ERR_FAIL_NULL_V(st, false);
		auto *viewport = st->get_root()->get_viewport();
		ERR_FAIL_NULL_V(viewport, false);
		auto *vs = VS::get_singleton();
		canvas = vs->canvas_create();
		ERR_FAIL_COND_V(!canvas.is_valid(), false);
		vs->viewport_attach_canvas(viewport->get_viewport_rid(), canvas);
		vs->viewport_set_canvas_stacking(viewport->get_viewport_rid(), canvas, (~0U) >> 1, (~0U) >> 1);
	}
	return canvas.is_valid();
}

RID GdGeomFonts::_new_item() {
	ERR_FAIL_COND_V(!canvas.is_valid(), RID());
	auto *vs = VS::get_singleton();
	RID rid = vs->canvas_item_create();
	ERR_FAIL_COND_V(!rid.is_valid(), RID());
	vs->canvas_item_set_parent(rid, canvas);
	return rid;
}

void GdGeomFonts::bob_font_add_text(const String &p_text, const Point3 &p_pos, real_t p_size, bool p_wire) {
	if (_is_ready()) {
		bob_font_draw_string(_mesh, p_text.ascii().c_str(), p_pos, p_size, p_wire);
	}
}

void GdGeomFonts::bob_font_add_text_xform(const String &p_text, const Transform &p_xform, const Point3 &p_pos, real_t p_size, bool p_wire) {
	if (_is_ready()) {
		bob_font_draw_string(_mesh, p_text.ascii().c_str(), p_xform, p_pos, p_size, p_wire);
	}
}

void GdGeomFonts::easy_font_add_text(const String &p_text, const Point2 &p_pos, real_t p_spacing) {
	if (_is_ready()) {
		stb_easy_font_spacing(p_spacing);
		stb_easy_font_print_string(_mesh, p_pos, p_text.ascii().c_str());
	}
}

void GdGeomFonts::easy_font_add_text_xform(const String &p_text, const Transform &p_xform, const Point2 &p_pos, real_t p_spacing) {
	if (_is_ready()) {
		stb_easy_font_spacing(p_spacing);
		stb_easy_font_print_string_xform(_mesh, p_xform, p_pos, p_text.ascii().c_str());
	}
}

Size2 GdGeomFonts::easy_font_text_size(const String &p_text) {
	const char *ptr = p_text.ascii().c_str();
	return Size2(stb_easy_font_width(ptr), stb_easy_font_height(ptr));
}

int GdGeomFonts::bitmap_font_draw_text(const String &p_text, const Point2 &p_pos) {
	if (_is_ready()) {
		int id = items.size();
		items.push_back({_new_item(), _new_item()});
		FBBitmapFontView fnt(items.last().r1, items.last().r2, _cache);
		fnt.set_text(p_text);
		fnt.draw();
		return id;
	}
	return -1;
}

void GdGeomFonts::set_transform(int p_index, const Transform2D &p_xform) {
	ERR_FAIL_INDEX(p_index, items.size());
	if (items.get(p_index).set_transform(p_xform)) {
		emit_signal("changed", array(p_index));
	}
}

void GdGeomFonts::set_modulate_color(int p_index, const Color &p_color) {
	ERR_FAIL_INDEX(p_index, items.size());
	if (items.get(p_index).set_modulate_color(p_color)) {
		emit_signal("changed", array(p_index));
	}
}

void GdGeomFonts::clear() {
	_mesh->clear_mesh();
	for (DrawItems &rid : items) {
		rid.clear();
	}
	items.clear();
}

int GdGeomFonts::font_add_finish() {
	RID item = _new_item();
	ERR_FAIL_COND_V(!item.is_valid(), -1);
	ERR_FAIL_COND_V(_mesh.is_null(), -1);
	VisualServer::get_singleton()->canvas_item_add_mesh(item, _mesh->get_rid());
	int id = items.size();
	items.push_back({ item });
	return id;
}

void GdGeomFonts::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_transform", "index", "xform"), &GdGeomFonts::set_transform);
	ClassDB::bind_method(D_METHOD("set_modulate_color", "index", "color"), &GdGeomFonts::set_modulate_color);

	ClassDB::bind_method(D_METHOD("bob_font_add_text", "text", "pos", "size", "wire"), &GdGeomFonts::bob_font_add_text, DEFVAL(Point2()), DEFVAL(1), DEFVAL(true));
	ClassDB::bind_method(D_METHOD("bob_font_add_text_xform", "text", "xform", "pos", "size", "wire"), &GdGeomFonts::bob_font_add_text_xform, DEFVAL(Point2()), DEFVAL(1), DEFVAL(true));

	ClassDB::bind_method(D_METHOD("easy_font_add_text", "text", "pos", "spacing"), &GdGeomFonts::easy_font_add_text, DEFVAL(Point2()), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("easy_font_add_text_xform", "text", "xform", "pos", "spacing"), &GdGeomFonts::easy_font_add_text_xform, DEFVAL(Point2()), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("easy_font_text_size", "text"), &GdGeomFonts::easy_font_text_size);

	ClassDB::bind_method(D_METHOD("font_add_finish"), &GdGeomFonts::font_add_finish);
	ClassDB::bind_method(D_METHOD("clear"), &GdGeomFonts::clear);

	ClassDB::bind_method(D_METHOD("bitmap_font_draw_text", "text", "pos"), &GdGeomFonts::bitmap_font_draw_text, DEFVAL(Point2()));

	ADD_SIGNAL(MethodInfo("changed"));
}

GdGeomFonts::GdGeomFonts() {
	_mesh = newref(ArrayMesh);
	_mat_trnsp = newref(CanvasItemMaterial);
	_mat_trnsp->set_blend_mode(CanvasItemMaterial::BLEND_MODE_MIX);
	_cache["mat_trnsp"] = _mat_trnsp;
}

GdGeomFonts::~GdGeomFonts() {
	if (auto *vs = VS::get_singleton()) {
		if (canvas.is_valid()) {
			vs->free(canvas);
		}
		items.clear();
	}
}
