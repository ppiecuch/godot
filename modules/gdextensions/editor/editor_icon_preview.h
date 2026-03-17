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

class EditorFileDialog;
class GridContainer;
class HSlider;
class Label;
class LineEdit;
class ScrollContainer;
class TextureRect;

class EditorIconPreviewDialog : public Reference {
	GDCLASS(EditorIconPreviewDialog, Reference)

	AcceptDialog *dlg;
	Ref<Script> dlg_script;

	// Cached node pointers (resolved in _cache_nodes)
	LineEdit *search_box;
	LineEdit *search_box_count_label;
	GridContainer *previews_container;
	ScrollContainer *previews_scroll;
	Label *icon_info_label;
	HSlider *icon_preview_size_range;
	Label *icon_size_label;
	TextureRect *icon_preview;
	Label *icon_copied_label;
	Label *icon_preview_size;

	int icon_size;
	String filter;
	bool _update_queued;

	EditorFileDialog *file_dialog;

	void _cache_nodes();
	void _queue_update();
	void _update_icons();

#ifdef TOOLS_ENABLED
	void _on_window_about_to_show();
	void _on_window_popup_hide();
	void _on_window_resized();
	void _on_window_visibility_changed();
	void _on_search_text_changed(String text);
	void _on_size_changed(float pixels);
	void _on_container_mouse_exited();
	void _on_save_pressed();
	void _on_file_selected(const String &p_path);
	void _icon_gui_input(Ref<InputEvent> event, Node *icon);
#endif

protected:
	static void _bind_methods();

public:
	AcceptDialog *load_ui();
	void open_ui();
	void set_file_dialog(EditorFileDialog *p_dialog);

	void add_icon(Ref<Texture> p_icon, const String &p_name);
	void clear();
	void display();

	EditorIconPreviewDialog();
};

/// Godot editor plugin

class EditorIconPreview : public EditorPlugin {
	GDCLASS(EditorIconPreview, EditorPlugin)

	EditorNode *editor;

	Ref<EditorIconPreviewDialog> dialog;

	void add_icons_menu_item(const String &p_name, const String &p_callback);
	void remove_icons_menu_item(const String &p_name);

	void _on_show_editor_icons_pressed(Variant p_null);
	void _on_update_requested();
	void _populate_icons();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	EditorIconPreview(EditorNode *p_node);
};

#endif // EDITOR_ICON_PREVIEW_H
