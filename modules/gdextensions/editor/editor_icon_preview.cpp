/**************************************************************************/
/*  editor_icon_preview.cpp                                               */
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

#include "editor_icon_preview.h"

#include "common/gd_core.h"
#include "core/os/input_event.h"
#include "core/os/os.h"
#include "core/print_string.h"
#include "core/script_language.h"
#include "editor/editor_file_dialog.h"
#include "scene/gui/grid_container.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/scroll_container.h"
#include "scene/gui/slider.h"
#include "scene/gui/texture_rect.h"
#include "scene/resources/resource_format_text.h"

const String SELECT_ICON_MSG = "Select any icon.";
const String ICON_SIZE_MSG = "Icon size: ";
const String NUMBER_ICONS_MSG = "Found: ";
const String SNIPPET_TEMPLATE = "get_icon(\"%s\", \"EditorIcons\")";

const int MIN_ICON_SIZE = 16;
const int MAX_ICON_SIZE = 128;

// UI description
static const char *_ui = R"(
[gd_scene load_steps=2 format=2]

[node name="window" type="AcceptDialog"]
margin_right = 300.0
margin_bottom = 240.0
rect_min_size = Vector2( 600, 480 )
focus_mode = 2
window_title = "Editor Icons"
resizable = true

[node name="body" type="VBoxContainer" parent="."]
margin_left = 8.0
margin_top = 8.0
margin_right = 592.0
margin_bottom = 444.0
size_flags_horizontal = 3
size_flags_vertical = 3
custom_constants/separation = 16
__meta__ = {
"_edit_use_anchors_": false
}

[node name="search" type="HBoxContainer" parent="body"]
margin_right = 584.0
margin_bottom = 24.0

[node name="box" type="LineEdit" parent="body/search"]
margin_right = 484.0
margin_bottom = 24.0
size_flags_horizontal = 3
placeholder_text = "Search icons..."
caret_blink = true
caret_blink_speed = 0.5

[node name="found" type="LineEdit" parent="body/search"]
margin_left = 488.0
margin_right = 584.0
margin_bottom = 24.0
rect_min_size = Vector2( 96, 0 )
focus_mode = 0
text = "Found: 0"
editable = false

[node name="icons" type="HSplitContainer" parent="body"]
margin_top = 40.0
margin_right = 584.0
margin_bottom = 456.0
size_flags_horizontal = 3
size_flags_vertical = 3
dragger_visibility = 1

[node name="previews" type="ScrollContainer" parent="body/icons"]
margin_right = 396.0
margin_bottom = 416.0
size_flags_horizontal = 3
size_flags_vertical = 3
__meta__ = {
"_edit_use_anchors_": false
}

[node name="container" type="GridContainer" parent="body/icons/previews"]
margin_right = 396.0
margin_bottom = 416.0
size_flags_horizontal = 3
size_flags_vertical = 3
columns = 18

[node name="info" type="PanelContainer" parent="body/icons"]
self_modulate = Color( 0.615686, 0.615686, 0.615686, 1 )
margin_left = 408.0
margin_right = 584.0
margin_bottom = 416.0
rect_min_size = Vector2( 176, 0 )
size_flags_vertical = 3

[node name="icon" type="VBoxContainer" parent="body/icons/info"]
margin_left = 7.0
margin_top = 7.0
margin_right = 169.0
margin_bottom = 409.0
size_flags_horizontal = 3
size_flags_vertical = 3
size_flags_stretch_ratio = 0.25

[node name="label" type="Label" parent="body/icons/info/icon"]
margin_right = 162.0
margin_bottom = 14.0
text = "Select any icon."
autowrap = true
__meta__ = {
"_edit_use_anchors_": false
}

[node name="preview" type="TextureRect" parent="body/icons/info/icon"]
margin_top = 18.0
margin_right = 128.0
margin_bottom = 146.0
rect_min_size = Vector2( 128, 128 )
size_flags_horizontal = 0
size_flags_vertical = 0
size_flags_stretch_ratio = 0.25
expand = true
stretch_mode = 5

[node name="size" type="Label" parent="body/icons/info/icon"]
margin_top = 150.0
margin_right = 162.0
margin_bottom = 164.0
autowrap = true
__meta__ = {
"_edit_use_anchors_": false
}

[node name="copied" type="Label" parent="body/icons/info/icon"]
visible = false
margin_top = 3.0
margin_right = 223.0
margin_bottom = 28.0
text = "Copied to clipboard!"
autowrap = true

[node name="space" type="Control" parent="body/icons/info/icon"]
margin_top = 168.0
margin_right = 162.0
margin_bottom = 340.0
size_flags_horizontal = 3
size_flags_vertical = 3

[node name="params" type="VBoxContainer" parent="body/icons/info/icon"]
margin_top = 344.0
margin_right = 162.0
margin_bottom = 378.0

[node name="icon" type="Label" parent="body/icons/info/icon/params"]
margin_right = 162.0
margin_bottom = 14.0
text = "Icons preview size"
__meta__ = {
"_edit_use_anchors_": false
}

[node name="size" type="HBoxContainer" parent="body/icons/info/icon/params"]
margin_top = 18.0
margin_right = 162.0
margin_bottom = 34.0

[node name="range" type="HSlider" parent="body/icons/info/icon/params/size"]
margin_right = 138.0
margin_bottom = 16.0
focus_mode = 0
size_flags_horizontal = 3
min_value = 16.0
max_value = 128.0
value = 16.0
__meta__ = {
"_edit_use_anchors_": false
}

[node name="pixels" type="Label" parent="body/icons/info/icon/params/size"]
margin_left = 142.0
margin_top = 1.0
margin_right = 162.0
margin_bottom = 15.0
text = "16 px"
__meta__ = {
"_edit_use_anchors_": false
}

[node name="save" type="Button" parent="body/icons/info/icon"]
margin_top = 382.0
margin_right = 162.0
margin_bottom = 402.0
text = "Save"

[connection signal="about_to_show" from="." to="." method="_on_window_about_to_show"]
[connection signal="popup_hide" from="." to="." method="_on_window_popup_hide"]
[connection signal="resized" from="." to="." method="_on_window_resized"]
[connection signal="visibility_changed" from="." to="." method="_on_window_visibility_changed"]
[connection signal="text_changed" from="body/search/box" to="." method="_on_search_text_changed"]
[connection signal="mouse_exited" from="body/icons/previews/container" to="." method="_on_container_mouse_exited"]
[connection signal="value_changed" from="body/icons/info/icon/params/size/range" to="." method="_on_size_changed"]
[connection signal="pressed" from="body/icons/info/icon/save" to="." method="_on_save_pressed"]
)";

// BEGIN PassthroughScript for EditorIconPreviewDialog

typedef PassthroughScript<AcceptDialog, EditorIconPreviewDialog> EditorIconPreviewUIScriptInstanceBase;

class EditorIconPreviewUIScript : public EditorIconPreviewUIScriptInstanceBase {
	GDCLASS(EditorIconPreviewUIScript, EditorIconPreviewUIScriptInstanceBase)

	_THREAD_SAFE_CLASS_

public:
	EditorIconPreviewUIScript(EditorIconPreviewDialog *p_recv) {
		set_receiver(p_recv);
	}
};

// END PassthroughScript

// BEGIN EditorIconPreviewDialog

EditorIconPreviewDialog::EditorIconPreviewDialog() {
	icon_size = MIN_ICON_SIZE;
	filter = "";
	_update_queued = false;
	dlg = nullptr;
	search_box = nullptr;
	search_box_count_label = nullptr;
	previews_container = nullptr;
	previews_scroll = nullptr;
	icon_info_label = nullptr;
	icon_preview_size_range = nullptr;
	icon_size_label = nullptr;
	icon_preview = nullptr;
	icon_copied_label = nullptr;
	icon_preview_size = nullptr;
	file_dialog = nullptr;
}

AcceptDialog *EditorIconPreviewDialog::load_ui() {
	if (!dlg) {
		ResourceFormatLoaderText rl;
		Ref<PackedScene> ui = rl.load_from_data(_ui, "editor_icon_preview_ui.tscn");
		if (ui.is_valid()) {
			dlg = cast_to<AcceptDialog>(ui->instance());
			dlg_script = newref(EditorIconPreviewUIScript, this);
			dlg->set_script(dlg_script.get_ref_ptr());
			// Resolve node pointers immediately. Without this, the first
			// display() emits `update_request` before _on_window_about_to_show
			// has run, so add_icon() trips its ERR_FAIL_NULL on
			// previews_container and silently drops every icon — the grid
			// then renders empty.
			_cache_nodes();
		}
	}
	return dlg;
}

void EditorIconPreviewDialog::open_ui() {
	display();
}

void EditorIconPreviewDialog::set_file_dialog(EditorFileDialog *p_dialog) {
	file_dialog = p_dialog;
}

void EditorIconPreviewDialog::_cache_nodes() {
	ERR_FAIL_NULL(dlg);

	search_box = cast_to<LineEdit>(dlg->get_node_or_null(NodePath("body/search/box")));
	search_box_count_label = cast_to<LineEdit>(dlg->get_node_or_null(NodePath("body/search/found")));
	previews_container = cast_to<GridContainer>(dlg->get_node_or_null(NodePath("body/icons/previews/container")));
	previews_scroll = cast_to<ScrollContainer>(dlg->get_node_or_null(NodePath("body/icons/previews")));
	icon_info_label = cast_to<Label>(dlg->get_node_or_null(NodePath("body/icons/info/icon/label")));
	icon_preview_size_range = cast_to<HSlider>(dlg->get_node_or_null(NodePath("body/icons/info/icon/params/size/range")));
	icon_size_label = cast_to<Label>(dlg->get_node_or_null(NodePath("body/icons/info/icon/size")));
	icon_preview = cast_to<TextureRect>(dlg->get_node_or_null(NodePath("body/icons/info/icon/preview")));
	icon_copied_label = cast_to<Label>(dlg->get_node_or_null(NodePath("body/icons/info/icon/copied")));
	icon_preview_size = cast_to<Label>(dlg->get_node_or_null(NodePath("body/icons/info/icon/params/size/pixels")));
}

void EditorIconPreviewDialog::display() {
	ERR_FAIL_NULL(dlg);

	if (!previews_container || previews_container->get_child_count() == 0) {
		// First time, request to create previews by the plugin
		emit_signal("update_request");
		dlg->call_deferred("popup_centered_ratio", 0.75);
	} else {
		dlg->popup_centered_ratio(0.75);
	}
}

void EditorIconPreviewDialog::clear() {
	ERR_FAIL_NULL(previews_container);

	for (int idx = 0; idx < previews_container->get_child_count(); idx++) {
		previews_container->get_child(idx)->queue_delete();
	}
}

void EditorIconPreviewDialog::add_icon(Ref<Texture> p_icon, const String &p_name) {
	ERR_FAIL_NULL(previews_container);

	TextureRect *icon = memnew(TextureRect);
	icon->set_expand(true);
	icon->set_texture(p_icon);
	icon->set_custom_minimum_size(Vector2(icon_size, icon_size));
	icon->set_tooltip(p_name);
	icon->set_name(p_name);

	icon->connect("gui_input", this, "_icon_gui_input", varray(icon));

	previews_container->add_child(icon);
}

#ifdef TOOLS_ENABLED
void EditorIconPreviewDialog::_icon_gui_input(Ref<InputEvent> event, Node *icon) {
	Ref<InputEventMouseButton> mb = event;
	if (mb.is_valid() && mb->is_pressed()) {
		Control *ctrl = cast_to<Control>(icon);
		ERR_FAIL_NULL(ctrl);
		String tooltip_text = ctrl->get_tooltip(Point2());

		if (mb->get_button_index() == BUTTON_LEFT) {
			// Copy raw icon name to clipboard
			OS::get_singleton()->set_clipboard(tooltip_text);
			if (icon_copied_label) {
				icon_copied_label->show();
			}
		} else if (mb->get_button_index() == BUTTON_RIGHT) {
			// Copy icon name with embedded code snippet to clipboard
			String snippet = vformat(SNIPPET_TEMPLATE, tooltip_text);
			OS::get_singleton()->set_clipboard(snippet);
			if (icon_copied_label) {
				icon_copied_label->show();
			}
		}
		if (icon_info_label) {
			icon_info_label->set_text(tooltip_text);
		}
		TextureRect *tex_rect = cast_to<TextureRect>(icon);
		if (icon_preview && tex_rect) {
			icon_preview->set_texture(tex_rect->get_texture());
		}
		if (icon_size_label && tex_rect && tex_rect->get_texture().is_valid()) {
			icon_size_label->set_text(ICON_SIZE_MSG + String(tex_rect->get_texture()->get_size()));
		}
	}
}

void EditorIconPreviewDialog::_on_search_text_changed(String text) {
	filter = text;
	_queue_update();
}

void EditorIconPreviewDialog::_on_size_changed(float pixels) {
	icon_size = int(CLAMP(pixels, MIN_ICON_SIZE, MAX_ICON_SIZE));
	_queue_update();
}

void EditorIconPreviewDialog::_update_icons() {
	_update_queued = false;

	if (!previews_container || !previews_scroll) {
		return;
	}

	int number = 0;

	for (int idx = 0; idx < previews_container->get_child_count(); idx++) {
		Node *child = previews_container->get_child(idx);
		Control *icon = cast_to<Control>(child);
		if (!icon) {
			continue;
		}

		if (!filter.is_subsequence_ofi(icon->get_tooltip(Point2()))) {
			icon->set_visible(false);
		} else {
			icon->set_visible(true);
			number += 1;
		}

		icon->set_custom_minimum_size(Vector2(icon_size, icon_size));
		icon->set_size(icon->get_custom_minimum_size());
	}

	int sep = previews_container->get_constant("hseparation");
	int cols = int(previews_scroll->get_size().x / (icon_size + sep));
	if (cols < 1) {
		cols = 1;
	}

	previews_container->set_columns(cols);

	if (icon_preview_size) {
		icon_preview_size->set_text(itos(icon_size) + " px");
	}

	if (search_box_count_label) {
		search_box_count_label->set_text(NUMBER_ICONS_MSG + itos(number));
	}
}

void EditorIconPreviewDialog::_queue_update() {
	if (!dlg || !dlg->is_inside_tree()) {
		return;
	}

	if (_update_queued) {
		return;
	}

	_update_queued = true;

	call_deferred("_update_icons");
}

void EditorIconPreviewDialog::_on_window_about_to_show() {
	_cache_nodes();
	if (search_box) {
		search_box->grab_focus();
	}
	_queue_update();
}

void EditorIconPreviewDialog::_on_window_popup_hide() {
	// Reset
	filter = "";
	icon_size = MIN_ICON_SIZE;

	if (search_box) {
		search_box->set_text(filter);
	}
	if (icon_preview_size_range) {
		icon_preview_size_range->set_value(icon_size);
	}
}

void EditorIconPreviewDialog::_on_window_resized() {
	_queue_update();
}

void EditorIconPreviewDialog::_on_window_visibility_changed() {
	if (dlg && dlg->is_visible()) {
		_queue_update();
	}
}

void EditorIconPreviewDialog::_on_container_mouse_exited() {
	if (icon_copied_label) {
		icon_copied_label->hide();
	}
}

void EditorIconPreviewDialog::_on_save_pressed() {
	if (file_dialog) {
		file_dialog->popup_centered_ratio();
	}
}

void EditorIconPreviewDialog::_on_file_selected(const String &p_path) {
	if (icon_preview && icon_preview->get_texture().is_valid()) {
		ResourceSaver::save(p_path, icon_preview->get_texture());
	}
}
#endif

void EditorIconPreviewDialog::_bind_methods() {
#ifdef TOOLS_ENABLED
	ClassDB::bind_method(D_METHOD("_update_icons"), &EditorIconPreviewDialog::_update_icons);
	ClassDB::bind_method(D_METHOD("_on_window_about_to_show"), &EditorIconPreviewDialog::_on_window_about_to_show);
	ClassDB::bind_method(D_METHOD("_on_window_popup_hide"), &EditorIconPreviewDialog::_on_window_popup_hide);
	ClassDB::bind_method(D_METHOD("_on_window_resized"), &EditorIconPreviewDialog::_on_window_resized);
	ClassDB::bind_method(D_METHOD("_on_window_visibility_changed"), &EditorIconPreviewDialog::_on_window_visibility_changed);
	ClassDB::bind_method(D_METHOD("_on_search_text_changed", "text"), &EditorIconPreviewDialog::_on_search_text_changed);
	ClassDB::bind_method(D_METHOD("_on_size_changed", "pixels"), &EditorIconPreviewDialog::_on_size_changed);
	ClassDB::bind_method(D_METHOD("_on_container_mouse_exited"), &EditorIconPreviewDialog::_on_container_mouse_exited);
	ClassDB::bind_method(D_METHOD("_on_save_pressed"), &EditorIconPreviewDialog::_on_save_pressed);
	ClassDB::bind_method(D_METHOD("_on_file_selected", "path"), &EditorIconPreviewDialog::_on_file_selected);
	ClassDB::bind_method(D_METHOD("_icon_gui_input", "event", "icon"), &EditorIconPreviewDialog::_icon_gui_input);
#endif

	ADD_SIGNAL(MethodInfo("update_request"));
}

// END EditorIconPreviewDialog

// BEGIN EditorIconPreview (EditorPlugin)

void EditorIconPreview::add_icons_menu_item(const String &p_name, const String &p_callback) {
	if (int(Engine::get_singleton()->get_version_info()["hex"]) >= 0x030100) {
		add_tool_menu_item(p_name, this, p_callback);
	}
}

void EditorIconPreview::remove_icons_menu_item(const String &p_name) {
	if (int(Engine::get_singleton()->get_version_info()["hex"]) >= 0x030100) {
		remove_tool_menu_item(p_name);
	}
}

void EditorIconPreview::_on_show_editor_icons_pressed(Variant p_null) {
	dialog->display();
}

void EditorIconPreview::_on_update_requested() {
	_populate_icons();
}

void EditorIconPreview::_populate_icons() {
	dialog->clear();

	Ref<Theme> godot_theme = get_editor_interface()->get_base_control()->get_theme();

	List<StringName> icon_list;
	godot_theme->get_icon_list("EditorIcons", &icon_list);

	// Sort alphabetically
	icon_list.sort();

	for (List<StringName>::Element *E = icon_list.front(); E; E = E->next()) {
		String icon_name = E->get();

		if (icon_name.empty()) {
			continue;
		}

		Ref<Texture> icon_tex = godot_theme->get_icon(icon_name, "EditorIcons");
		dialog->add_icon(icon_tex, icon_name);
	}
}

void EditorIconPreview::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			if (AcceptDialog *dlg = dialog->load_ui()) {
				get_editor_interface()->get_base_control()->add_child(dlg);

				// Create EditorFileDialog as child of the dialog window
				EditorFileDialog *fd = memnew(EditorFileDialog);
				fd->add_filter("*.png");
				fd->set_mode(EditorFileDialog::MODE_SAVE_FILE);
				fd->set_access(EditorFileDialog::ACCESS_RESOURCES);
				fd->connect("file_selected", dialog.ptr(), "_on_file_selected");
				dlg->add_child(fd);
				dialog->set_file_dialog(fd);
			}
			dialog->connect("update_request", this, "_on_update_requested");
		} break;
		case NOTIFICATION_ENTER_TREE: {
			add_icons_menu_item("Show Editor Icons", "_on_show_editor_icons_pressed");
		} break;
		case NOTIFICATION_EXIT_TREE: {
			remove_icons_menu_item("Show Editor Icons");
		} break;
	}
}

void EditorIconPreview::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_show_editor_icons_pressed"), &EditorIconPreview::_on_show_editor_icons_pressed);
	ClassDB::bind_method(D_METHOD("_on_update_requested"), &EditorIconPreview::_on_update_requested);
}

EditorIconPreview::EditorIconPreview(EditorNode *p_node) {
	editor = p_node;
	dialog = newref(EditorIconPreviewDialog);
}

// END EditorIconPreview
