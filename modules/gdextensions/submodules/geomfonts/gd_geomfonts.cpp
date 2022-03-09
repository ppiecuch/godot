#include "gd_geomfonts.h"

#include "bob3d/bob_font.cpp"
#include "easyfont/stb_easy_font.h"
#include "leonsans/leonsans.h"
#include "bluntfont/blutfont.cpp"
#include "polyfonts/polygodot.cpp"
#include "ttftriangulator/ttftriangulator.cpp"

#include "servers/visual_server.h"
#include "scene/main/scene_tree.h"
#include "scene/main/viewport.h"
#include "scene/resources/mesh.h"

bool GdGeomFonts::_is_ready() {
	if (!canvas_item.is_valid()) {
		auto *st = SceneTree::get_singleton();
		ERR_FAIL_NULL_V(st, false);
		auto *viewport = st->get_root()->get_viewport();
		ERR_FAIL_NULL_V(viewport, false);
		auto *vs = VS::get_singleton();
		canvas = vs->canvas_create();
		vs->viewport_attach_canvas(viewport->get_viewport_rid(), canvas);
		vs->viewport_set_canvas_stacking(viewport->get_viewport_rid(), canvas, (~0U) >> 1, (~0U) >> 1);
		canvas_item = vs->canvas_item_create();
		vs->canvas_item_set_parent(canvas_item, canvas);
	}
	return canvas_item.is_valid();
}

void GdGeomFonts::bob_font_add_text(const String &p_text, const Point3 &p_pos, real_t p_size, bool p_wire) {
	if (_is_ready()) {
		DrawBobString(_mesh, canvas_item, p_text.ascii().c_str(), p_pos, p_size, p_wire);
	}
}

void GdGeomFonts::easy_font_add_text(const String &p_text, const Point2 &p_pos, real_t p_spacing) {
	if (_is_ready()) {
		stb_easy_font_spacing(p_spacing);
		stb_easy_font_print_string(_mesh, canvas_item, p_pos, p_text.ascii().c_str());
	}
}

Size2 GdGeomFonts::easy_font_text_size(const String &p_text) {
	const char *ptr = p_text.ascii().c_str();
	return Size2(stb_easy_font_width(ptr), stb_easy_font_height(ptr));
}

void GdGeomFonts::set_transform(const Transform2D &p_xform) {
	xform = p_xform;
	emit_signal("changed");
}

Transform2D GdGeomFonts::get_transform() const { return xform; }

void GdGeomFonts::set_color(const Color &p_color) {
	modulate = p_color;
	emit_signal("changed");
}

Color GdGeomFonts::get_color() const { return modulate; }

void GdGeomFonts::clear() {
	_mesh->clear_mesh();
	if (canvas_item.is_valid()) {
		VisualServer::get_singleton()->canvas_item_clear(canvas_item);
	}
}

void GdGeomFonts::finish() {
	ERR_FAIL_COND(!canvas_item.is_valid());
	ERR_FAIL_COND(_mesh.is_null());
	VisualServer::get_singleton()->canvas_item_add_mesh(canvas_item, _mesh->get_rid(), xform, modulate, RID(), RID(), RID());
}

void GdGeomFonts::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_transform"), &GdGeomFonts::set_transform);
	ClassDB::bind_method(D_METHOD("get_transform"), &GdGeomFonts::get_transform);
	ClassDB::bind_method(D_METHOD("set_color"), &GdGeomFonts::set_color);
	ClassDB::bind_method(D_METHOD("get_color"), &GdGeomFonts::get_color);

	ClassDB::bind_method(D_METHOD("clear"), &GdGeomFonts::clear);
	ClassDB::bind_method(D_METHOD("finish"), &GdGeomFonts::finish);

	ClassDB::bind_method(D_METHOD("bob_font_add_text", "text", "pos", "size", "wire"), &GdGeomFonts::bob_font_add_text, DEFVAL(Point2()), DEFVAL(1), DEFVAL(true));

	ClassDB::bind_method(D_METHOD("easy_font_add_text", "text", "pos", "spacing"), &GdGeomFonts::easy_font_add_text, DEFVAL(Point2()), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("easy_font_text_size", "text"), &GdGeomFonts::easy_font_text_size);

	ADD_SIGNAL(MethodInfo("changed"));
}

GdGeomFonts::GdGeomFonts() {
	modulate = Color(1,1,1,1);
	xform = Transform2D();

	_mesh = newref(ArrayMesh);
}

GdGeomFonts::~GdGeomFonts() {
	if (auto *vs = VS::get_singleton()) {
		if (canvas.is_valid()) {
			vs->free(canvas);
		}
		if (canvas_item.is_valid()) {
			vs->free(canvas_item);
		}
	}
}
