/**************************************************************************/
/*  atlas_info_editor_plugin.cpp                                          */
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

#ifdef TOOLS_ENABLED

#include "atlas_info_editor_plugin.h"

#include "editor/editor_node.h"

// ---------------------------------------------------------------------------
// AtlasInfoPreview
// ---------------------------------------------------------------------------

void AtlasInfoPreview::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_item_selected", "index"), &AtlasInfoPreview::_on_item_selected);
	ClassDB::bind_method(D_METHOD("_on_hull_toggled", "pressed"), &AtlasInfoPreview::_on_hull_toggled);
	ClassDB::bind_method(D_METHOD("_on_tri_toggled", "pressed"), &AtlasInfoPreview::_on_tri_toggled);
	ClassDB::bind_method(D_METHOD("_draw_preview"), &AtlasInfoPreview::_draw_preview);
}

void AtlasInfoPreview::_notification(int p_what) {
}

void AtlasInfoPreview::edit(const Ref<AtlasInfo> &p_info) {
	atlas_info = p_info;
	selected_item = -1;

	item_combo->clear();
	if (atlas_info.is_valid()) {
		for (int i = 0; i < atlas_info->get_item_count(); ++i) {
			String name = atlas_info->get_item_name(i);
			if (name.empty())
				name = "item_" + itos(i);
			item_combo->add_item(name, i);
		}
		if (atlas_info->get_item_count() > 0) {
			item_combo->select(0);
			selected_item = 0;
		}
	}
	preview_canvas->update();
}

void AtlasInfoPreview::_on_item_selected(int p_index) {
	selected_item = item_combo->get_item_id(p_index);
	preview_canvas->update();
}

void AtlasInfoPreview::_on_hull_toggled(bool p_pressed) {
	preview_canvas->update();
}

void AtlasInfoPreview::_on_tri_toggled(bool p_pressed) {
	preview_canvas->update();
}

void AtlasInfoPreview::_draw_preview() {
	if (atlas_info.is_null() || selected_item < 0)
		return;

	Size2 canvas_size = preview_canvas->get_size();
	if (canvas_size.x <= 0 || canvas_size.y <= 0)
		return;

	Ref<Texture> tex = override_texture.is_valid() ? override_texture : atlas_info->get_default_texture();

	// Determine the atlas page dimensions from texture or item rects
	Size2 atlas_size;
	if (tex.is_valid()) {
		atlas_size = tex->get_size();
	} else {
		// Estimate from item rects
		for (int i = 0; i < atlas_info->get_item_count(); ++i) {
			Rect2 r = atlas_info->get_item_rect(i);
			atlas_size.x = MAX(atlas_size.x, r.position.x + r.size.x);
			atlas_size.y = MAX(atlas_size.y, r.position.y + r.size.y);
		}
	}
	if (atlas_size.x <= 0 || atlas_size.y <= 0)
		return;

	// Scale to fit canvas with aspect ratio preserved
	float scale = MIN(canvas_size.x / atlas_size.x, canvas_size.y / atlas_size.y);
	Size2 draw_size = atlas_size * scale;
	Point2 offset = (canvas_size - draw_size) * 0.5f;

	// Draw background texture
	if (tex.is_valid()) {
		preview_canvas->draw_texture_rect(tex, Rect2(offset, draw_size));
	} else {
		preview_canvas->draw_rect(Rect2(offset, draw_size), Color(0.15, 0.15, 0.15, 1.0));
	}

	// Draw selected item rect
	Rect2 item_rect = atlas_info->get_item_rect(selected_item);
	Rect2 draw_rect(
			offset + item_rect.position * scale,
			item_rect.size * scale);

	preview_canvas->draw_rect(draw_rect, Color(1, 1, 0, 0.8), false, 2.0);

	// Draw hull outline
	if (show_hull_outline->is_pressed() && atlas_info->item_has_hull(selected_item)) {
		PoolVector2Array hull = atlas_info->get_item_hull(selected_item);
		int n = hull.size();
		PoolVector2Array::Read rd = hull.read();
		for (int i = 0; i < n; ++i) {
			int j = (i + 1) % n;
			Point2 p0 = draw_rect.position + Vector2(rd[i].x * draw_rect.size.x, rd[i].y * draw_rect.size.y);
			Point2 p1 = draw_rect.position + Vector2(rd[j].x * draw_rect.size.x, rd[j].y * draw_rect.size.y);
			preview_canvas->draw_line(p0, p1, Color(0, 1, 0, 1), 2.0);
		}
	}

	// Draw hull triangulation
	if (show_hull_triangulation->is_pressed() && atlas_info->item_has_hull(selected_item)) {
		PoolVector2Array hull = atlas_info->get_item_hull(selected_item);
		PoolIntArray indices = atlas_info->get_item_hull_indices(selected_item);
		PoolVector2Array::Read vrd = hull.read();
		PoolIntArray::Read ird = indices.read();
		int tri_count = indices.size() / 3;
		for (int t = 0; t < tri_count; ++t) {
			for (int e = 0; e < 3; ++e) {
				int i0 = ird[t * 3 + e];
				int i1 = ird[t * 3 + (e + 1) % 3];
				Point2 p0 = draw_rect.position + Vector2(vrd[i0].x * draw_rect.size.x, vrd[i0].y * draw_rect.size.y);
				Point2 p1 = draw_rect.position + Vector2(vrd[i1].x * draw_rect.size.x, vrd[i1].y * draw_rect.size.y);
				preview_canvas->draw_line(p0, p1, Color(0, 1, 1, 0.7), 1.0);
			}
		}
	}
}

AtlasInfoPreview::AtlasInfoPreview() {
	selected_item = -1;

	HBoxContainer *toolbar = memnew(HBoxContainer);
	add_child(toolbar);

	item_combo = memnew(OptionButton);
	item_combo->set_h_size_flags(SIZE_EXPAND_FILL);
	item_combo->connect("item_selected", this, "_on_item_selected");
	toolbar->add_child(item_combo);

	show_hull_outline = memnew(CheckButton);
	show_hull_outline->set_text("Hull");
	show_hull_outline->set_pressed(true);
	show_hull_outline->connect("toggled", this, "_on_hull_toggled");
	toolbar->add_child(show_hull_outline);

	show_hull_triangulation = memnew(CheckButton);
	show_hull_triangulation->set_text("Tri");
	show_hull_triangulation->set_pressed(false);
	show_hull_triangulation->connect("toggled", this, "_on_tri_toggled");
	toolbar->add_child(show_hull_triangulation);

	preview_canvas = memnew(Control);
	preview_canvas->set_custom_minimum_size(Size2(0, 200));
	preview_canvas->set_h_size_flags(SIZE_EXPAND_FILL);
	preview_canvas->set_v_size_flags(SIZE_EXPAND_FILL);
	preview_canvas->connect("draw", this, "_draw_preview");
	add_child(preview_canvas);
}

// ---------------------------------------------------------------------------
// EditorInspectorPluginAtlasInfo
// ---------------------------------------------------------------------------

bool EditorInspectorPluginAtlasInfo::can_handle(Object *p_object) {
	return Object::cast_to<AtlasInfo>(p_object) != nullptr;
}

void EditorInspectorPluginAtlasInfo::parse_begin(Object *p_object) {
	AtlasInfo *info = Object::cast_to<AtlasInfo>(p_object);
	if (!info)
		return;

	AtlasInfoPreview *preview = memnew(AtlasInfoPreview);
	preview->edit(Ref<AtlasInfo>(info));
	add_custom_control(preview);
}

// ---------------------------------------------------------------------------
// AtlasInfoEditorPlugin
// ---------------------------------------------------------------------------

AtlasInfoEditorPlugin::AtlasInfoEditorPlugin(EditorNode *p_node) {
	inspector_plugin.instance();
	add_inspector_plugin(inspector_plugin);
}

#endif // TOOLS_ENABLED
