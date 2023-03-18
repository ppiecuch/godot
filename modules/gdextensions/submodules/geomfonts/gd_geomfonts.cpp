/**************************************************************************/
/*  gd_geomfonts.cpp                                                      */
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

#include "fbdigitalfont/fb_font_symbol.h"
#define _CRT_SECURE_NO_WARNINGS

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

std::pair<RID, int> GdGeomFonts::_next_item(RID canvas) {
	RID rid = RID_PRIME(VS::get_singleton()->canvas_item_create());
	const int handle = items.insert(rid).value;
	if (rid.is_valid()) {
		ERR_FAIL_COND_V(!canvas.is_valid(), std::make_pair(RID(), -1));
		VS::get_singleton()->canvas_item_set_parent(rid, canvas);
	}
	return std::make_pair(rid, handle);
}

RID GdGeomFonts::_from_handle(int hrid) const {
	const Id_T t = make_handle(hrid);
	if (items.is_valid(t)) {
		return items[t];
	} else {
		return RID();
	}
}

bool GdGeomFonts::_valid_handle(int hrid) const {
	const Id_T t = make_handle(hrid);
	return items.is_valid(t);
}

void GdGeomFonts::mesh_add_aster_font_text(Ref<ArrayMesh> p_mesh, const String &p_text, const Point2 &p_pos, const Vector2 &p_size) {
	ERR_FAIL_NULL(p_mesh);
	_draw_asteroid_text(p_mesh, p_pos, p_size, p_text.ascii().c_str());
}

void GdGeomFonts::mesh_add_bob_font_text(Ref<ArrayMesh> p_mesh, const String &p_text, const Point3 &p_pos, real_t p_size, bool p_wire) {
	ERR_FAIL_NULL(p_mesh);
	bob_font_draw_string(p_mesh, p_text.ascii().c_str(), p_pos, p_size, p_wire);
}

void GdGeomFonts::mesh_add_bob_font_text_xform(Ref<ArrayMesh> p_mesh, const String &p_text, const Transform &p_xform, real_t p_size, bool p_wire) {
	ERR_FAIL_NULL(p_mesh);
	bob_font_draw_string(p_mesh, p_text.ascii().c_str(), p_xform, p_size, p_wire);
}

void GdGeomFonts::mesh_add_easy_font_text(Ref<ArrayMesh> p_mesh, const String &p_text, const Point2 &p_pos, real_t p_spacing) {
	ERR_FAIL_NULL(p_mesh);
	stb_easy_font_spacing(p_spacing);
	stb_easy_font_print_string(p_mesh, p_pos, p_text.ascii().c_str());
}

void GdGeomFonts::mesh_add_easy_font_text_xform(Ref<ArrayMesh> p_mesh, const String &p_text, const Transform &p_xform, real_t p_spacing) {
	ERR_FAIL_NULL(p_mesh);
	stb_easy_font_spacing(p_spacing);
	stb_easy_font_print_string_xform(p_mesh, p_xform, p_text.ascii().c_str());
}

void GdGeomFonts::mesh_add_hp_font_text(Ref<ArrayMesh> p_mesh, const String &p_text, const Point2 &p_pos, const Size2 &p_scale) {
	ERR_FAIL_NULL(p_mesh);
	_draw_hp1345_text(p_mesh, p_text, p_pos, p_scale);
}

void GdGeomFonts::mesh_add_hp_font_text_xform(Ref<ArrayMesh> p_mesh, const String &p_text, const Transform &p_xform) {
}

int GdGeomFonts::mesh_add_finish(RID p_canvas, Ref<ArrayMesh> p_mesh) {
	ERR_FAIL_COND_V(!p_canvas.is_valid(), -1);
	ERR_FAIL_NULL_V(p_mesh, -1);
	const auto item = _next_item(p_canvas);
	VisualServer::get_singleton()->canvas_item_add_mesh(item.first, p_mesh->get_rid());
	return item.second;
}

int GdGeomFonts::easy_font_text(RID p_canvas, const String &p_text, const Point2 &p_pos, real_t p_spacing) {
	ERR_FAIL_COND_V(!p_canvas.is_valid(), -1);
	const auto item = _next_item(p_canvas);
	Ref<ArrayMesh> mesh = newref(ArrayMesh);
	stb_easy_font_spacing(p_spacing);
	stb_easy_font_print_string(mesh, p_pos, p_text.ascii().c_str());
	VisualServer::get_singleton()->canvas_item_add_mesh(item.first, mesh->get_rid());
	return item.second;
}

int GdGeomFonts::easy_font_text_xform(RID p_canvas, const String &p_text, const Transform &p_xform, real_t p_spacing) {
	const int id = items.size();
	return id;
}

Size2 GdGeomFonts::easy_font_text_size(const String &p_text) const {
	const char *ptr = p_text.ascii().c_str();
	return Size2(stb_easy_font_width(ptr), stb_easy_font_height(ptr));
}

int GdGeomFonts::bob_font_text(RID p_canvas, const String &p_text, const Transform &p_xform, real_t p_size, bool p_wire) {
	const int id = items.size();
	return id;
}

int GdGeomFonts::bob_font_text_xform(RID p_canvas, const String &p_text, const Transform &p_xform, real_t p_size, bool p_wire) {
	const int id = items.size();
	return id;
}

Size2 GdGeomFonts::bob_font_text_size(const String &p_text) const {
	return Size2();
}

int GdGeomFonts::hp_font_text(RID p_canvas, const String &p_text, const Point2 &p_pos, const Size2 &p_scale) {
	ERR_FAIL_COND_V(!p_canvas.is_valid(), -1);
	const auto item = _next_item(p_canvas);
	Ref<ArrayMesh> mesh = newref(ArrayMesh);
	_draw_hp1345_text(mesh, p_text, p_pos, p_scale);
	VisualServer::get_singleton()->canvas_item_add_mesh(item.first, mesh->get_rid());
	return item.second;
}

int GdGeomFonts::hp_font_text_xform(RID p_canvas, const String &p_text, const Transform &p_xform) {
	const int id = items.size();
	return id;
}

Size2 GdGeomFonts::hp_font_text_size(const String &p_text, const Size2 &p_scale) const {
	return _size_hp1345_text(p_text, p_scale);
}

void GdGeomFonts::init_dot_textures(real_t p_fall_off) {
	init_bitmap_symbol(_cache, p_fall_off);
}

#define _set_prop(D, P) \
	if (D.has(#P)) { \
		const int val = D[#P]; \
		if (val >= 0) { \
			fnt.set_##P(val); \
		} \
	}

#define _set_color(D, P) \
	if (D.has(#P)) { \
		const Color val = D[#P]; \
		fnt.set_##P(val); \
	}

int GdGeomFonts::canvas_add_bitmap_font_text(RID p_canvas, const String &p_text, const Point2 &p_pos, int p_dot_style, const Dictionary &p_style) {
	ERR_FAIL_COND_V(!p_canvas.is_valid(), -1);
	const auto item = _next_item(p_canvas);
	FBBitmapFontView fnt(item.first, _cache);
	fnt.set_style(FBFontDotStyle(p_dot_style));
	fnt.set_text(p_text);
	_set_prop(p_style, margin);
	_set_prop(p_style, edge_length);
	_set_color(p_style, on_color);
	_set_color(p_style, off_color);
	fnt.draw(p_pos);

	return item.second;
}

void GdGeomFonts::set_transform(int p_index, const Transform2D &p_xform) {
	ERR_FAIL_COND(!_valid_handle(p_index));
	VS::get_singleton()->canvas_item_set_transform(_from_handle(p_index), p_xform);
	emit_signal("changed", array(p_index));
}

void GdGeomFonts::set_modulate_color(int p_index, const Color &p_color) {
	ERR_FAIL_COND(!_valid_handle(p_index));
	VS::get_singleton()->canvas_item_set_modulate(_from_handle(p_index), p_color);
	emit_signal("changed", array(p_index));
}

void GdGeomFonts::clear_item(int p_index) {
	ERR_FAIL_COND(!_valid_handle(p_index));
	VS::get_singleton()->free(_from_handle(p_index));
}

void GdGeomFonts::clear_all_items() {
	for (const RID &rid : items) {
		VS::get_singleton()->free(rid);
	}
	items.reset();
}

void GdGeomFonts::_bind_methods() {
	ClassDB::bind_method(D_METHOD("mesh_add_aster_font_text", "mesh", "text", "pos", "size"), &GdGeomFonts::mesh_add_bob_font_text, DEFVAL(Point2()), DEFVAL(Vector2(1, 1)));
	ClassDB::bind_method(D_METHOD("mesh_add_bob_font_text", "mesh", "text", "pos", "size", "wire"), &GdGeomFonts::mesh_add_bob_font_text, DEFVAL(Point2()), DEFVAL(1), DEFVAL(true));
	ClassDB::bind_method(D_METHOD("mesh_add_bob_font_text_xform", "mesh", "text", "xform", "size", "wire"), &GdGeomFonts::mesh_add_bob_font_text_xform, DEFVAL(1), DEFVAL(true));
	ClassDB::bind_method(D_METHOD("mesh_add_easy_font_text", "mesh", "text", "pos", "spacing"), &GdGeomFonts::mesh_add_easy_font_text, DEFVAL(Point2()), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("mesh_add_easy_font_text_xform", "mesh", "text", "xform", "spacing"), &GdGeomFonts::mesh_add_easy_font_text_xform, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("mesh_add_hp_font_text", "mesh", "text", "pos", "scale"), &GdGeomFonts::mesh_add_hp_font_text, DEFVAL(Point2()), DEFVAL(Size2(10, 10)));
	ClassDB::bind_method(D_METHOD("mesh_add_hp_font_text_xform", "mesh", "text", "xform"), &GdGeomFonts::mesh_add_hp_font_text_xform);
	ClassDB::bind_method(D_METHOD("mesh_add_finish", "canvas", "mesh"), &GdGeomFonts::mesh_add_finish);

	ClassDB::bind_method(D_METHOD("easy_font_text", "canvas", "text", "pos", "spacing"), &GdGeomFonts::easy_font_text, DEFVAL(Point2()), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("easy_font_text_xform", "canvas", "text", "xform", "spacing"), &GdGeomFonts::easy_font_text_xform, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("easy_font_text_size", "text"), &GdGeomFonts::easy_font_text_size);
	ClassDB::bind_method(D_METHOD("bob_font_text", "canvas", "text", "pos", "spacing"), &GdGeomFonts::bob_font_text, DEFVAL(Point2()), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("bob_font_text_xform", "canvas", "text", "xform", "spacing"), &GdGeomFonts::bob_font_text_xform, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("bob_font_text_size", "text"), &GdGeomFonts::bob_font_text_size);
	ClassDB::bind_method(D_METHOD("hp_font_text", "canvas", "text", "pos", "scale"), &GdGeomFonts::hp_font_text, DEFVAL(Point2()), DEFVAL(Size2(10, 10)));
	ClassDB::bind_method(D_METHOD("hp_font_text_xform", "canvas", "text", "xform"), &GdGeomFonts::hp_font_text_xform);
	ClassDB::bind_method(D_METHOD("hp_font_text_size", "text", "scale"), &GdGeomFonts::hp_font_text_size, DEFVAL(Size2(10, 10)));

	ClassDB::bind_method(D_METHOD("init_dot_textures", "fall_off"), &GdGeomFonts::init_dot_textures, DEFVAL(0.3));
	ClassDB::bind_method(D_METHOD("canvas_add_bitmap_font_text", "canvas", "text", "pos", "dot_style", "style"), &GdGeomFonts::canvas_add_bitmap_font_text, DEFVAL(Point2()), DEFVAL(BITMAP_FONT_FLAT_CIRCLE), DEFVAL(Dictionary()));

	ClassDB::bind_method(D_METHOD("set_transform", "index", "xform"), &GdGeomFonts::set_transform);
	ClassDB::bind_method(D_METHOD("set_modulate_color", "index", "color"), &GdGeomFonts::set_modulate_color);
	ClassDB::bind_method(D_METHOD("clear_item", "index"), &GdGeomFonts::clear_item);
	ClassDB::bind_method(D_METHOD("clear_all_items"), &GdGeomFonts::clear_all_items);

	BIND_ENUM_CONSTANT(BITMAP_FONT_FLAT_CIRCLE);
	BIND_ENUM_CONSTANT(BITMAP_FONT_FLAT_SQUARE);
	BIND_ENUM_CONSTANT(BITMAP_FONT_TEXTURE_CIRCLE);
	BIND_ENUM_CONSTANT(BITMAP_FONT_TEXTURE_SQUARE);
	BIND_ENUM_CONSTANT(BITMAP_FONT_TEXTURE_3D);

	ADD_SIGNAL(MethodInfo("changed"));
}

GdGeomFonts::GdGeomFonts() : items(1, 32) {
	init_bitmap_symbol(_cache, DOT_TEXTURE_FALL_OUT);
}

GdGeomFonts::~GdGeomFonts() {
	for (const RID &rid : items) {
		VS::get_singleton()->free(rid);
	}
	items.reset();
}
