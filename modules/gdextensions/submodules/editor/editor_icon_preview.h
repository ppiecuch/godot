/**************************************************************************/
/*  editor_icon_preview.h                                                 */
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

#ifndef EDITOR_ICON_PREVIEW_H
#define EDITOR_ICON_PREVIEW_H

#include "editor/editor_plugin.h"
#include "scene/gui/dialogs.h"

class EditorIconPreview : public EditorPlugin {
	Ref<PackedScene> icon_window;
	bool suppress_warnings;

	void _enter_tree() {
		icon_window = preload("editor_icon_window.tscn").instance();
		var dialog : = EditorFileDialog.new();
		icon_window.file_dialog = dialog;
		icon_window.add_child(dialog);
		dialog.add_filter("*.png");
		dialog.mode = EditorFileDialog.MODE_SAVE_FILE;
		dialog.access = EditorFileDialog.ACCESS_RESOURCES;
		get_editor_interface().get_base_control().add_child(icon_window);
		icon_window.connect("update_request", self, "_on_update_requested");

		add_icons_menu_item("Show Editor Icons", "_on_show_editor_icons_pressed");
	}

	void _exit_tree() {
		if (icon_window) {
			icon_window.queue_free();
			remove_icons_menu_item("Show Editor Icons");
		}
	}

	void _on_show_editor_icons_pressed(_data) {
		icon_window.display();
	}

	void _on_update_requested() {
		_populate_icons();
	}

	void _populate_icons() {
		icon_window.clear();

		var godot_theme = get_editor_interface().get_base_control().theme;

		var list = Array(godot_theme.get_icon_list("EditorIcons"));
		list.sort() // alphabetically

				var no_name = [];

		for (icon_name in list) {
			var icon_tex = godot_theme.get_icon(icon_name, "EditorIcons");

			if (icon_name.empty()) {
				no_name.append(icon_tex);
				continue;
			}

			icon_window.add_icon(icon_tex, icon_name);
		}

		if (!suppress_warnings) {
			if (no_name.size() > 0) {
				push_warning(vformat("EditorIconsPreviewer: detected %s icons with no name set, skipping.", no_name.size()));
			}
		}
	}

	void add_icons_menu_item(const String &p_name, const String &p_callback) {
		if (Eint(Engine::get_singleton()->get_version_info()["hex"]) >= 0x030100) {
			add_tool_menu_item(p_name, this, p_callback);
		}
	}

	void remove_icons_menu_item(const String &p_name) {
		if (int(Engine::get_singleton()->get_version_info()["hex"]) >= 0x030100) {
			remove_tool_menu_item(p_name);
		}
	}

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("_enter_tree"));
		ClassDB::bind_method(D_METHOD("_exit_tree"));
		ClassDB::bind_method(D_METHOD("_on_update_requested"));
		ClassDB::bind_method(D_METHOD("_on_show_editor_icons_pressed"));
		ClassDB::bind_method(D_METHOD("_on_update_requested"));
	}
	void _notification(int p_what) {
	}

public:
	EditorIconPreviewer(EditorNode *p_node) {
		suppress_warnings = false;
	}
};

class EditorIconPreviewDialog : public AcceptDialog {
	GDCLASS(EditorIconPreviewDialog, AcceptDialog);

	Node *search_box = $body / search / box Node *search_box_count_label = $body / search / found

																									Node *icons_control = $body / icons Node *previews_container = icons_control.get_node("previews/container")
																																										   Node *previews_scroll = icons_control.get_node("previews")
																																																		   Node *icon_info = icons_control.get_node("info/icon")

																																																									 Node *icon_preview_size_range = icon_info.get_node("params/size/range")
																																																																			 Node *icon_info_label = icon_info.get_node("label")
																																																																											 Node *icon_preview = icon_info.get_node("preview")
																																																																																		  Node *icon_copied_label = icon_info.get_node("copied")
																																																																																											Node *icon_size_label = icon_info.get_node("size")
																																																																																																			Node *icon_preview_size = icon_info.get_node("params/size/pixels")

																																																																																																											  int icon_size;
	String filter;

	bool _update_queued;

	Ref<EditorFileDialog> file_dialog;

	void _ready() {
		file_dialog.connect("file_selected", this, "_on_file_selected");

		icon_info_label.text = SELECT_ICON_MSG;

		icon_preview_size_range.min_value = MIN_ICON_SIZE;
		icon_preview_size_range.max_value = MAX_ICON_SIZE;

		icon_preview.rect_min_size = Vector2(MAX_ICON_SIZE, MAX_ICON_SIZE);

		if (has_color("success_color", "Editor")) {
			Color color = get_color("success_color", "Editor")
								  icon_copied_label.add_color_override("font_color", color);
		}

		get_ok().hide(); // give more space for icons

		_queue_update();
	}

	void _queue_update() {
		if (!is_inside_tree()) {
			return;
		}

		if (_update_queued) {
			return;
		}

		_update_queued = true;

		call_deferred("_update_icons");
	}

	void add_icon(p_icon, p_name) {
		var icon = TextureRect.new();
		icon.expand = true;
		icon.texture = p_icon;
		icon.rect_min_size = Vector2(icon_size, icon_size);
		icon.hint_tooltip = p_name;
		icon.name = p_name;

		icon->connect("gui_input", this, "_icon_gui_input", array(icon));

		previews_container.add_child(icon);
	}

	void _icon_gui_input(event, icon) {
		if (event is InputEventMouseButton && event.pressed) {
			if (event.button_index == BUTTON_LEFT) {
				// Copy raw icon's name into the clipboard
				OS.clipboard = icon.hint_tooltip;
				icon_copied_label.show();
			} else if (event.button_index == BUTTON_RIGHT) {
				// Copy icon's name with embedded code into the clipboard
				var snippet = SNIPPET_TEMPLATE % [icon.hint_tooltip];
				OS.clipboard = snippet;
				icon_copied_label.show();
			}
			icon_info_label.text = icon.hint_tooltip;
			icon_preview.texture = icon.texture;
			icon_size_label.text = ICON_SIZE_MSG + str(icon.texture.get_size());
		}
	}

	void _input(event) {
		if (event is InputEventKey && event.is_pressed() && !event.echo) {
			if (event.alt and event.scancode == KEY_I) {
				if (!visible) {
					display();
				} else {
					hide();
				}
			}
		}

		void display() {
			if (previews_container.get_child_count() == 0) {
				// First time, request to create previews by the plugin
				emit_signal("update_request");
				call_deferred("popup_centered_ratio", 0.75);
			} else {
				popup_centered_ratio(0.75);
			}
		}

		void clear() {
			for (idx in previews_container.get_child_count()) {
				previews_container.get_child(idx).queue_free();
			}
		}

		void _on_size_changed(pixels) {
			icon_size = int(clamp(pixels, MIN_ICON_SIZE, MAX_ICON_SIZE));
			_queue_update();
		}

		void _update_icons() {
			int number = 0;

			for (idx in previews_container.get_child_count()) {
				var icon = previews_container.get_child(idx);

				if (!filter.is_subsequence_ofi(icon.hint_tooltip)) {
					icon.visible = false;
				} else {
					icon.visible = true;
					number += 1;
				}

				icon.rect_min_size = Vector2(icon_size, icon_size);
				icon.rect_size = icon.rect_min_size;
			}

			var sep = previews_container.get_constant("hseparation")
							  var cols = int(previews_scroll.rect_size.x / (icon_size + sep))

												 previews_container.columns = cols - 1 icon_preview_size.text = str(icon_size) + " px"

																																 search_box_count_label.text = NUMBER_ICONS_MSG + str(number)

																																														  _update_queued = false
		}

		void _on_window_visibility_changed() {
			if (visible) {
				_queue_update();
			}
		}

		void _on_window_resized() {
			_queue_update();
		}

		void _on_search_text_changed(text) {
			filter = text;
			_queue_update();
		}

		void _on_container_mouse_exited() {
			icon_copied_label->hide();
		}

		void _on_window_about_to_show() {
			// For some reason can't get proper rect size, so need to wait
			yield(previews_container, "sort_children");
			search_box.grab_focus();
			_queue_update();
		}

		void _on_window_popup_hide() {
			// Reset
			filter = '';
			icon_size = MIN_ICON_SIZE;

			search_box.text = filter;
			icon_preview_size_range.value = icon_size;
		}

		void _on_save_pressed() {
			file_dialog.popup_centered_ratio();
		}

		void _on_file_selected(const String &p_path) {
			Ref<Texture> texture = icon_preview.get_texture();
			ResourceSaver::get_instance()->save(p_path, texture);
		}

	protected:
		static void _bind_methods() {
			ADD_SIGNAL("update_request");
		}

		void _notification(int p_what) {
			switch (p_what) {
				case NOTIFICATION_THEME_CHANGED: {
					emit_signal("update_request");
				}
			}
		}

	public:
		EditorIconPreviewDialog() {
			icon_size = MIN_ICON_SIZE;
			filter = "";
			_update_queued = false;
		}
	};

#endif // EDITOR_ICON_PREVIEW_H
