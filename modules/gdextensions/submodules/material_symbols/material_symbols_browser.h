/**************************************************************************/
/*  material_symbols_browser.h                                            */
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

// Editor-only dock that lets you browse the full Material Symbols catalogue
// at design time: search, switch style (Outlined / Rounded / Sharp), tweak
// FILL/wght/GRAD/opsz, recolour, copy a name to the clipboard, or append the
// current selection to the project's `material_symbols.subset` manifest.

#ifndef MATERIAL_SYMBOLS_BROWSER_H
#define MATERIAL_SYMBOLS_BROWSER_H

#ifdef TOOLS_ENABLED

#include "core/reference.h"
#include "editor/editor_file_dialog.h"
#include "editor/editor_plugin.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/color_picker.h"
#include "scene/gui/dialogs.h"
#include "scene/gui/item_list.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/option_button.h"
#include "scene/gui/scroll_container.h"
#include "scene/gui/slider.h"
#include "scene/gui/spin_box.h"

#include "material_symbols.h"

class MaterialSymbolsBrowser : public VBoxContainer {
	GDCLASS(MaterialSymbolsBrowser, VBoxContainer);

	// Single dense top row: search (compact) + style + axes + size + color.
	LineEdit *search_field;
	OptionButton *style_picker;
	OptionButton *weight_picker;
	OptionButton *grade_picker;
	OptionButton *opsz_picker;
	OptionButton *fill_picker;
	SpinBox *preview_size;
	ColorPickerButton *color_picker;

	// Icon list
	ItemList *icon_list;

	// Bottom row: status | copy name | copy info | tag | manifest | view | + | clear
	Label *status_label;
	LineEdit *tag_field;
	Button *copy_button;
	Button *copy_info_button;
	LineEdit *manifest_path_field;
	Button *manifest_view_button;
	Button *add_to_subset_button;
	Button *clear_manifest_button;

	// Manifest viewer popup
	WindowDialog *manifest_viewer;
	VBoxContainer *manifest_rows;
	EditorFileDialog *manifest_save_dialog;

	Ref<MaterialSymbols> ms;

	// Render queue (filtered names rendered one batch per frame)
	Vector<String> filter_results;
	int render_cursor;
	static const int MAX_VISIBLE = 600;
	static const int RENDER_BATCH = 24;

	int _current_style() const;
	Dictionary _current_axes() const;
	void _refresh_filter();
	void _refresh_status();
	void _on_search_changed(const String &p_q);
	void _on_axis_changed(int p_idx);
	void _on_style_changed(int p_idx);
	void _on_size_changed(double p_v);
	void _on_color_changed(const Color &p_c);
	void _on_copy_pressed();
	void _on_copy_info_pressed();
	void _on_add_to_subset_pressed();
	void _on_item_selected(int p_idx);
	void _on_manifest_view_pressed();
	void _on_manifest_remove(int p_line_index);
	void _on_manifest_clear_pressed();
	void _on_manifest_edit_pressed();
	void _on_manifest_reveal_pressed();
	void _on_manifest_copy_path_pressed();
	void _on_viewer_close_pressed();
	void _on_viewer_save_pressed();
	void _on_viewer_save_path_selected(const String &p_path);
	void _on_viewer_copy_pressed();
	void _on_tag_changed(const String &p_t);
	void _rebuild_manifest_view();
	String _build_manifest_line(const String &p_name) const;
	bool _is_in_manifest(const String &p_name, const String &p_tag) const;
	String _resolve_manifest_path() const;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	MaterialSymbolsBrowser();
	~MaterialSymbolsBrowser();
};

class MaterialSymbolsBrowserPlugin : public EditorPlugin {
	GDCLASS(MaterialSymbolsBrowserPlugin, EditorPlugin);

	MaterialSymbolsBrowser *dock;

public:
	MaterialSymbolsBrowserPlugin(EditorNode *p_node);
	~MaterialSymbolsBrowserPlugin();
};

#endif // TOOLS_ENABLED

#endif // MATERIAL_SYMBOLS_BROWSER_H
