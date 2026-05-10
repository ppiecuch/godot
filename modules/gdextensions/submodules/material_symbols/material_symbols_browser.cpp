/**************************************************************************/
/*  material_symbols_browser.cpp                                          */
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

#include "material_symbols_browser.h"

#include "core/os/file_access.h"
#include "core/os/os.h"
#include "editor/editor_node.h"
#include "editor/editor_settings.h"
#include "scene/gui/separator.h"

// =========================================================================
// MaterialSymbolsBrowser
// =========================================================================

MaterialSymbolsBrowser::MaterialSymbolsBrowser() {
	ms.instance();
	// Browser-side cache is large because the icon list can shows hundreds
	// of icons after a broad search.
	ms->set_image_cache_capacity(MAX_VISIBLE + 64);
	ms->set_texture_cache_capacity(MAX_VISIBLE + 64);
	render_cursor = -1;

	add_constant_override("separation", 4);

	// ── Single dense top row ──────────────────────────────────────────────
	HBoxContainer *top = memnew(HBoxContainer);
	top->add_constant_override("separation", 6);
	add_child(top);

	Label *search_label = memnew(Label);
	search_label->set_text("Search:");
	top->add_child(search_label);

	search_field = memnew(LineEdit);
	search_field->set_custom_minimum_size(Size2(180, 0));
	search_field->set_placeholder("e.g. play, folder");
	search_field->connect("text_changed", this, "_on_search_changed");
	top->add_child(search_field);

	Label *style_label = memnew(Label);
	style_label->set_text("Style:");
	top->add_child(style_label);

	style_picker = memnew(OptionButton);
	style_picker->add_item("Outlined", MaterialSymbols::STYLE_OUTLINED);
	style_picker->add_item("Rounded", MaterialSymbols::STYLE_ROUNDED);
	style_picker->add_item("Sharp", MaterialSymbols::STYLE_SHARP);
	style_picker->select(1); // Rounded
	style_picker->connect("item_selected", this, "_on_style_changed");
	top->add_child(style_picker);

	// Discrete-axis combos (id == real axis value, so get_selected_id() reads it).
	auto add_combo = [&](OptionButton *&r_combo, const String &p_label, const String &p_tooltip,
							 const Vector<int> &p_values, const Vector<String> &p_labels, int p_default_id) {
		Label *lbl = memnew(Label);
		lbl->set_text(p_label);
		top->add_child(lbl);
		r_combo = memnew(OptionButton);
		r_combo->set_tooltip(p_tooltip);
		for (int i = 0; i < p_values.size(); i++) {
			r_combo->add_item(p_labels[i], p_values[i]);
		}
		for (int i = 0; i < r_combo->get_item_count(); i++) {
			if (r_combo->get_item_id(i) == p_default_id) {
				r_combo->select(i);
				break;
			}
		}
		r_combo->connect("item_selected", this, "_on_axis_changed");
		top->add_child(r_combo);
	};

	Vector<int> wv;
	wv.push_back(100);
	wv.push_back(200);
	wv.push_back(300);
	wv.push_back(400);
	wv.push_back(500);
	wv.push_back(600);
	wv.push_back(700);
	Vector<String> wl;
	wl.push_back("100");
	wl.push_back("200");
	wl.push_back("300");
	wl.push_back("400");
	wl.push_back("500");
	wl.push_back("600");
	wl.push_back("700");
	add_combo(weight_picker, "Weight:", "wght axis", wv, wl, 400);

	Vector<int> gv;
	gv.push_back(-25);
	gv.push_back(0);
	gv.push_back(200);
	Vector<String> gl;
	gl.push_back("-25");
	gl.push_back("0");
	gl.push_back("200");
	add_combo(grade_picker, "Grade:", "GRAD axis", gv, gl, 0);

	Vector<int> ov;
	ov.push_back(20);
	ov.push_back(24);
	ov.push_back(40);
	ov.push_back(48);
	Vector<String> ol;
	ol.push_back("20");
	ol.push_back("24");
	ol.push_back("40");
	ol.push_back("48");
	add_combo(opsz_picker, "Opsz:", "opsz axis", ov, ol, 24);

	// Fill is logically continuous; reduce to three useful steps for the UI.
	// Combo id encodes percent (0, 50, 100) so get_selected_id()/100.0 → 0.0/0.5/1.0.
	Vector<int> fv;
	fv.push_back(0);
	fv.push_back(50);
	fv.push_back(100);
	Vector<String> fl;
	fl.push_back("Outline");
	fl.push_back("Half");
	fl.push_back("Filled");
	add_combo(fill_picker, "Fill:", "FILL axis", fv, fl, 0);

	Label *size_label = memnew(Label);
	size_label->set_text("Size:");
	top->add_child(size_label);

	preview_size = memnew(SpinBox);
	preview_size->set_min(12);
	preview_size->set_max(96);
	preview_size->set_step(2);
	preview_size->set_value(32);
	preview_size->connect("value_changed", this, "_on_size_changed");
	top->add_child(preview_size);

	color_picker = memnew(ColorPickerButton);
	color_picker->set_pick_color(Color(0.85, 0.85, 0.85));
	color_picker->set_custom_minimum_size(Size2(60, 0));
	color_picker->connect("color_changed", this, "_on_color_changed");
	top->add_child(color_picker);

	// ── Icon list (icon-only, dense; tooltip carries the name) ────────────
	icon_list = memnew(ItemList);
	icon_list->set_v_size_flags(SIZE_EXPAND_FILL);
	icon_list->set_max_columns(0); // auto-fit
	icon_list->set_same_column_width(true);
	icon_list->set_icon_mode(ItemList::ICON_MODE_TOP);
	icon_list->set_fixed_icon_size(Size2(48, 48));
	icon_list->set_select_mode(ItemList::SELECT_SINGLE);
	icon_list->connect("item_selected", this, "_on_item_selected");
	add_child(icon_list);

	// ── Bottom row: status | copy name | copy info | tag | manifest | view + clear
	HBoxContainer *bottom = memnew(HBoxContainer);
	bottom->add_constant_override("separation", 6);
	add_child(bottom);

	status_label = memnew(Label);
	status_label->set_h_size_flags(SIZE_EXPAND_FILL);
	bottom->add_child(status_label);

	copy_button = memnew(Button);
	copy_button->set_text("Copy name");
	copy_button->set_tooltip("Copy the selected icon's name to the clipboard");
	copy_button->connect("pressed", this, "_on_copy_pressed");
	bottom->add_child(copy_button);

	copy_info_button = memnew(Button);
	copy_info_button->set_text("Copy info");
	copy_info_button->set_tooltip("Copy the full manifest line for the selected icon to the clipboard");
	copy_info_button->connect("pressed", this, "_on_copy_info_pressed");
	bottom->add_child(copy_info_button);

	Label *tag_lbl = memnew(Label);
	tag_lbl->set_text("Tag:");
	bottom->add_child(tag_lbl);

	tag_field = memnew(LineEdit);
	tag_field->set_text("default");
	tag_field->set_custom_minimum_size(Size2(120, 0));
	tag_field->connect("text_changed", this, "_on_tag_changed");
	bottom->add_child(tag_field);

	Label *manifest_lbl = memnew(Label);
	manifest_lbl->set_text("Manifest:");
	bottom->add_child(manifest_lbl);

	manifest_path_field = memnew(LineEdit);
	manifest_path_field->set_custom_minimum_size(Size2(260, 0));
	manifest_path_field->set_placeholder("res://material_symbols.subset");
	bottom->add_child(manifest_path_field);

	manifest_view_button = memnew(Button);
	manifest_view_button->set_text(String::utf8("\xe2\x86\x91")); // ↑
	manifest_view_button->set_tooltip("View manifest contents");
	manifest_view_button->connect("pressed", this, "_on_manifest_view_pressed");
	bottom->add_child(manifest_view_button);

	add_to_subset_button = memnew(Button);
	add_to_subset_button->set_text("+");
	add_to_subset_button->set_tooltip("Append the selected icon to the manifest");
	add_to_subset_button->connect("pressed", this, "_on_add_to_subset_pressed");
	bottom->add_child(add_to_subset_button);

	clear_manifest_button = memnew(Button);
	clear_manifest_button->set_text("Clear");
	clear_manifest_button->set_tooltip("Clear the manifest (delete all entries)");
	clear_manifest_button->connect("pressed", this, "_on_manifest_clear_pressed");
	bottom->add_child(clear_manifest_button);

	// "Edit" — opens the manifest in the OS default editor. Useful for bulk
	// hand-edits (reorder lines, mass-remove, comment things out) that would
	// be tedious through the in-dock viewer.
	Button *edit_button = memnew(Button);
	edit_button->set_text("Edit");
	edit_button->set_tooltip("Open the manifest in the OS default editor");
	edit_button->connect("pressed", this, "_on_manifest_edit_pressed");
	bottom->add_child(edit_button);

	Button *reveal_button = memnew(Button);
	reveal_button->set_text("Reveal");
	reveal_button->set_tooltip("Open the manifest's containing folder");
	reveal_button->connect("pressed", this, "_on_manifest_reveal_pressed");
	bottom->add_child(reveal_button);

	Button *copy_path_button = memnew(Button);
	copy_path_button->set_text("Copy path");
	copy_path_button->set_tooltip("Copy the manifest's absolute path to the clipboard");
	copy_path_button->connect("pressed", this, "_on_manifest_copy_path_pressed");
	bottom->add_child(copy_path_button);

	// ── Manifest viewer popup (built once, opened on demand) ──────────────
	manifest_viewer = memnew(WindowDialog);
	manifest_viewer->set_title("material_symbols.subset");
	manifest_viewer->set_custom_minimum_size(Size2(520, 420));
	add_child(manifest_viewer);

	VBoxContainer *viewer_root = memnew(VBoxContainer);
	viewer_root->set_anchors_and_margins_preset(Control::PRESET_WIDE, Control::PRESET_MODE_MINSIZE, 8);
	manifest_viewer->add_child(viewer_root);

	ScrollContainer *viewer_scroll = memnew(ScrollContainer);
	viewer_scroll->set_v_size_flags(SIZE_EXPAND_FILL);
	viewer_root->add_child(viewer_scroll);

	manifest_rows = memnew(VBoxContainer);
	manifest_rows->set_h_size_flags(SIZE_EXPAND_FILL);
	manifest_rows->add_constant_override("separation", 4);
	viewer_scroll->add_child(manifest_rows);

	// Footer: Close | spacer | Copy | Save (Save-As)
	HBoxContainer *viewer_footer = memnew(HBoxContainer);
	viewer_footer->add_constant_override("separation", 8);
	viewer_root->add_child(viewer_footer);

	Button *close_button = memnew(Button);
	close_button->set_text("Close");
	close_button->set_tooltip("Close this dialog");
	close_button->connect("pressed", this, "_on_viewer_close_pressed");
	viewer_footer->add_child(close_button);

	Control *spacer = memnew(Control);
	spacer->set_h_size_flags(SIZE_EXPAND_FILL);
	viewer_footer->add_child(spacer);

	Button *vcopy_button = memnew(Button);
	vcopy_button->set_text("Copy");
	vcopy_button->set_tooltip("Copy the manifest's full text contents to the clipboard");
	vcopy_button->connect("pressed", this, "_on_viewer_copy_pressed");
	viewer_footer->add_child(vcopy_button);

	Button *vsave_button = memnew(Button);
	vsave_button->set_text("Save...");
	vsave_button->set_tooltip("Save the manifest to a different location");
	vsave_button->connect("pressed", this, "_on_viewer_save_pressed");
	viewer_footer->add_child(vsave_button);

	manifest_save_dialog = memnew(EditorFileDialog);
	manifest_save_dialog->set_mode(EditorFileDialog::MODE_SAVE_FILE);
	manifest_save_dialog->set_access(EditorFileDialog::ACCESS_FILESYSTEM);
	manifest_save_dialog->add_filter("*.subset");
	manifest_save_dialog->add_filter("*.txt");
	manifest_save_dialog->add_filter("*");
	manifest_save_dialog->connect("file_selected", this, "_on_viewer_save_path_selected");
	manifest_viewer->add_child(manifest_save_dialog);

	_refresh_filter();
}

MaterialSymbolsBrowser::~MaterialSymbolsBrowser() {
}

int MaterialSymbolsBrowser::_current_style() const {
	int sel = style_picker->get_selected_id();
	if (sel < 0) {
		sel = MaterialSymbols::STYLE_ROUNDED;
	}
	return sel;
}

Dictionary MaterialSymbolsBrowser::_current_axes() const {
	Dictionary d;
	d["color"] = color_picker->get_pick_color();
	d["weight"] = weight_picker->get_selected_id();
	d["grade"] = grade_picker->get_selected_id();
	d["opsz"] = opsz_picker->get_selected_id();
	d["fill"] = (float)fill_picker->get_selected_id() / 100.0f;
	return d;
}

void MaterialSymbolsBrowser::_refresh_filter() {
	const int style = _current_style();
	const String query = search_field->get_text().to_lower().strip_edges();

	PoolStringArray all = ms->get_names((MaterialSymbols::Style)style);
	filter_results.clear();
	for (int i = 0; i < all.size(); i++) {
		const String &n = all[i];
		if (query.empty() || n.findn(query) != -1) {
			filter_results.push_back(n);
			if (filter_results.size() >= MAX_VISIBLE) {
				break;
			}
		}
	}

	icon_list->clear();
	for (int i = 0; i < filter_results.size(); i++) {
		// Icon-only — no text label below the tile. Hover tooltip shows the name.
		icon_list->add_item(String());
		icon_list->set_item_tooltip(i, filter_results[i]);
	}
	render_cursor = 0;
	set_process(true); // start lazy render
	_refresh_status();
}

void MaterialSymbolsBrowser::_refresh_status() {
	const int total = ms->get_count((MaterialSymbols::Style)_current_style());
	Vector<int> sel = icon_list->get_selected_items();
	if (sel.size() > 0 && sel[0] < filter_results.size()) {
		const String name = filter_results[sel[0]];
		const String tag = tag_field->get_text().strip_edges();
		const String preview_line = _build_manifest_line(name);
		String suffix;
		if (_is_in_manifest(name, tag.empty() ? String("default") : tag)) {
			suffix = "  (already in manifest)";
		}
		status_label->set_text("[ + ] " + preview_line + suffix);
	} else {
		status_label->set_text(itos(filter_results.size()) + " / " + itos(total) + " shown -- click an icon for details");
	}
}

void MaterialSymbolsBrowser::_on_item_selected(int /*p_idx*/) {
	_refresh_status();
}

void MaterialSymbolsBrowser::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_PROCESS: {
			if (render_cursor < 0 || render_cursor >= filter_results.size()) {
				set_process(false);
				return;
			}
			const int size_px = (int)preview_size->get_value();
			const int style = _current_style();
			const Dictionary axes = _current_axes();
			const int end = MIN(render_cursor + RENDER_BATCH, filter_results.size());
			for (int i = render_cursor; i < end; i++) {
				Ref<ImageTexture> tex = ms->get_texture((MaterialSymbols::Style)style, filter_results[i], size_px, axes);
				if (tex.is_valid()) {
					icon_list->set_item_icon(i, tex);
				}
			}
			render_cursor = end;
			if (render_cursor >= filter_results.size()) {
				set_process(false);
				_refresh_status();
			}
		} break;
		case NOTIFICATION_VISIBILITY_CHANGED: {
			if (is_visible_in_tree()) {
				_refresh_filter();
			}
		} break;
	}
}

void MaterialSymbolsBrowser::_on_search_changed(const String & /*p_q*/) {
	_refresh_filter();
}

void MaterialSymbolsBrowser::_on_style_changed(int /*p_idx*/) {
	_refresh_filter();
}

void MaterialSymbolsBrowser::_on_size_changed(double /*p_v*/) {
	// Re-render at new size; cache key includes size so we don't lose hits.
	render_cursor = 0;
	set_process(true);
	_refresh_status();
}

void MaterialSymbolsBrowser::_on_color_changed(const Color & /*p_c*/) {
	render_cursor = 0;
	set_process(true);
	_refresh_status();
}

void MaterialSymbolsBrowser::_on_tag_changed(const String & /*p_t*/) {
	_refresh_status();
}

void MaterialSymbolsBrowser::_on_axis_changed(int /*p_idx*/) {
	render_cursor = 0;
	set_process(true);
	_refresh_status();
}

void MaterialSymbolsBrowser::_on_copy_pressed() {
	int sel = -1;
	Vector<int> selected = icon_list->get_selected_items();
	if (selected.size() > 0) {
		sel = selected[0];
	}
	if (sel < 0 || sel >= filter_results.size()) {
		return;
	}
	OS::get_singleton()->set_clipboard(filter_results[sel]);
}

String MaterialSymbolsBrowser::_resolve_manifest_path() const {
	String p = manifest_path_field->get_text().strip_edges();
	if (p.empty()) {
		// Default: project-root sibling. Use globalized res:// if available.
		String res = ProjectSettings::get_singleton()->globalize_path("res://material_symbols.subset");
		return res;
	}
	if (p.begins_with("res://")) {
		return ProjectSettings::get_singleton()->globalize_path(p);
	}
	return p;
}

String MaterialSymbolsBrowser::_build_manifest_line(const String &p_name) const {
	const String tag = tag_field->get_text().strip_edges();
	const int style = _current_style();
	const char *style_str = "rounded";
	if (style == MaterialSymbols::STYLE_OUTLINED) {
		style_str = "outlined";
	} else if (style == MaterialSymbols::STYLE_SHARP) {
		style_str = "sharp";
	}
	const Color c = color_picker->get_pick_color();

	String line = p_name;
	if (!tag.empty() && tag != "default") {
		line += " tag=" + tag;
	}
	line += " style=";
	line += style_str;
	int sz = (int)preview_size->get_value();
	if (sz != 24) {
		line += " size=" + itos(sz);
	}
	int w = weight_picker->get_selected_id();
	if (w != 400) {
		line += " weight=" + itos(w);
	}
	int g = grade_picker->get_selected_id();
	if (g != 0) {
		line += " grade=" + itos(g);
	}
	int o = opsz_picker->get_selected_id();
	if (o != 24) {
		line += " opsz=" + itos(o);
	}
	int f_pct = fill_picker->get_selected_id();
	if (f_pct > 0) {
		float f = (float)f_pct / 100.0f;
		line += " fill=" + rtos(f).pad_decimals(2);
	}
	uint8_t cr = (uint8_t)CLAMP((int)round(c.r * 255.0f), 0, 255);
	uint8_t cg = (uint8_t)CLAMP((int)round(c.g * 255.0f), 0, 255);
	uint8_t cb = (uint8_t)CLAMP((int)round(c.b * 255.0f), 0, 255);
	if (cr != 0xff || cg != 0xff || cb != 0xff) {
		line += vformat(" color=%02x%02x%02x", cr, cg, cb);
	}
	return line;
}

void MaterialSymbolsBrowser::_on_copy_info_pressed() {
	int sel = -1;
	Vector<int> selected = icon_list->get_selected_items();
	if (selected.size() > 0) {
		sel = selected[0];
	}
	if (sel < 0 || sel >= filter_results.size()) {
		return;
	}
	OS::get_singleton()->set_clipboard(_build_manifest_line(filter_results[sel]));
}

bool MaterialSymbolsBrowser::_is_in_manifest(const String &p_name, const String &p_tag) const {
	const String path = _resolve_manifest_path();
	if (!FileAccess::exists(path)) {
		return false;
	}
	FileAccess *fa = FileAccess::open(path, FileAccess::READ);
	if (!fa) {
		return false;
	}
	String text = fa->get_as_utf8_string();
	memdelete(fa);
	Vector<String> lines = text.split("\n");
	for (int i = 0; i < lines.size(); i++) {
		String body = lines[i].split("#", 1)[0].strip_edges();
		if (body.empty()) {
			continue;
		}
		Vector<String> tok = body.split_spaces();
		if (tok.size() == 0 || tok[0] != p_name) {
			continue;
		}
		String el_tag = "default";
		for (int t = 1; t < tok.size(); t++) {
			if (tok[t].begins_with("tag=")) {
				el_tag = tok[t].substr(4);
				break;
			}
		}
		if (el_tag == p_tag) {
			return true;
		}
	}
	return false;
}

void MaterialSymbolsBrowser::_on_manifest_edit_pressed() {
	const String path = _resolve_manifest_path();
	// Create the file if it doesn't exist yet so the editor has something to open.
	if (!FileAccess::exists(path)) {
		FileAccess *fa = FileAccess::open(path, FileAccess::WRITE);
		if (fa) {
			fa->store_string("# Material Symbols subset\n");
			memdelete(fa);
		}
	}
	OS::get_singleton()->shell_open(path);
}

void MaterialSymbolsBrowser::_on_manifest_reveal_pressed() {
	const String path = _resolve_manifest_path();
	const String dir = path.get_base_dir();
	if (dir.empty()) {
		status_label->set_text("Cannot resolve manifest directory.");
		return;
	}
	OS::get_singleton()->shell_open(dir);
}

void MaterialSymbolsBrowser::_on_manifest_copy_path_pressed() {
	const String path = _resolve_manifest_path();
	OS::get_singleton()->set_clipboard(path);
	status_label->set_text("Copied path: " + path);
}

void MaterialSymbolsBrowser::_on_viewer_close_pressed() {
	manifest_viewer->hide();
}

void MaterialSymbolsBrowser::_on_viewer_copy_pressed() {
	const String path = _resolve_manifest_path();
	if (!FileAccess::exists(path)) {
		return;
	}
	FileAccess *fa = FileAccess::open(path, FileAccess::READ);
	if (!fa) {
		return;
	}
	OS::get_singleton()->set_clipboard(fa->get_as_utf8_string());
	memdelete(fa);
}

void MaterialSymbolsBrowser::_on_viewer_save_pressed() {
	manifest_save_dialog->set_current_file("material_symbols.subset");
	manifest_save_dialog->popup_centered_ratio();
}

void MaterialSymbolsBrowser::_on_viewer_save_path_selected(const String &p_path) {
	const String src = _resolve_manifest_path();
	String content;
	if (FileAccess::exists(src)) {
		FileAccess *fr = FileAccess::open(src, FileAccess::READ);
		if (fr) {
			content = fr->get_as_utf8_string();
			memdelete(fr);
		}
	}
	const String dst = p_path.begins_with("res://")
			? ProjectSettings::get_singleton()->globalize_path(p_path)
			: p_path;
	FileAccess *fw = FileAccess::open(dst, FileAccess::WRITE);
	if (!fw) {
		status_label->set_text("Cannot write " + dst);
		return;
	}
	fw->store_string(content);
	memdelete(fw);
	status_label->set_text("Saved manifest to " + dst);
}

void MaterialSymbolsBrowser::_on_manifest_clear_pressed() {
	const String path = _resolve_manifest_path();
	if (FileAccess::exists(path)) {
		// Truncate to empty (preserve the file so the build's mtime check still fires).
		FileAccess *fa = FileAccess::open(path, FileAccess::WRITE);
		if (fa) {
			memdelete(fa);
		}
	}
	status_label->set_text(vformat("Cleared manifest: %s", path));
	if (manifest_viewer->is_visible()) {
		_rebuild_manifest_view();
	}
}

void MaterialSymbolsBrowser::_on_add_to_subset_pressed() {
	int sel = -1;
	Vector<int> selected = icon_list->get_selected_items();
	if (selected.size() > 0) {
		sel = selected[0];
	}
	if (sel < 0 || sel >= filter_results.size()) {
		status_label->set_text("Select an icon first.");
		return;
	}

	const String name = filter_results[sel];
	const String tag = tag_field->get_text().strip_edges();
	const String line = _build_manifest_line(name);
	const String path = _resolve_manifest_path();

	// De-dupe: read existing file, skip if any line already starts with
	// "<name>" and has the same `tag=` value (or both lack `tag`).
	String existing;
	if (FileAccess::exists(path)) {
		FileAccess *fr = FileAccess::open(path, FileAccess::READ);
		if (fr) {
			existing = fr->get_as_utf8_string();
			memdelete(fr);
		}
	}
	const String dup_key = vformat("%s|%s", name, tag.empty() ? String("default") : tag);
	Vector<String> existing_lines = existing.split("\n");
	for (int i = 0; i < existing_lines.size(); i++) {
		String el = existing_lines[i].split("#", 1)[0].strip_edges();
		if (el.empty()) {
			continue;
		}
		Vector<String> tok = el.split_spaces();
		if (tok.size() == 0) {
			continue;
		}
		String el_name = tok[0];
		String el_tag = "default";
		for (int t = 1; t < tok.size(); t++) {
			if (tok[t].begins_with("tag=")) {
				el_tag = tok[t].substr(4);
			}
		}
		if (vformat("%s|%s", el_name, el_tag) == dup_key) {
			status_label->set_text(vformat("Already in manifest: %s", dup_key));
			return;
		}
	}

	FileAccess *fa = FileAccess::open(path, FileAccess::READ_WRITE);
	if (!fa) {
		fa = FileAccess::open(path, FileAccess::WRITE);
	}
	if (!fa) {
		status_label->set_text(vformat("Cannot write manifest at %s", path));
		return;
	}
	fa->seek_end();
	if (fa->get_position() > 0) {
		fa->store_8('\n');
	}
	fa->store_string(line);
	fa->store_8('\n');
	memdelete(fa);
	status_label->set_text(vformat("Appended: %s", line));
}

// =========================================================================
// Manifest viewer popup
// =========================================================================

void MaterialSymbolsBrowser::_on_manifest_view_pressed() {
	_rebuild_manifest_view();
	manifest_viewer->popup_centered();
}

namespace {
struct ManifestEntry {
	int line_index; // index in the original (unfiltered) line list
	String raw_line; // exact text of the line, for diff/remove
	String name;
	String tag;
	String style; // "outlined" | "rounded" | "sharp"
	int size_px;
	int weight;
	int grade;
	int opsz;
	float fill;
	Color color;
};

static bool _parse_manifest_line(const String &p_raw, int p_line_index, ManifestEntry &r) {
	String body = p_raw.split("#", 1)[0].strip_edges();
	if (body.empty()) {
		return false;
	}
	Vector<String> toks = body.split_spaces();
	if (toks.size() == 0) {
		return false;
	}
	r.line_index = p_line_index;
	r.raw_line = p_raw;
	r.name = toks[0];
	r.tag = "default";
	r.style = "rounded";
	r.size_px = 24;
	r.weight = 400;
	r.grade = 0;
	r.opsz = 24;
	r.fill = 0.0f;
	r.color = Color(1, 1, 1, 1);
	for (int i = 1; i < toks.size(); i++) {
		const String &t = toks[i];
		int eq = t.find("=");
		if (eq < 0) {
			continue;
		}
		String k = t.substr(0, eq);
		String v = t.substr(eq + 1);
		if (k == "tag") {
			r.tag = v;
		} else if (k == "style") {
			r.style = v;
		} else if (k == "size") {
			r.size_px = v.to_int();
		} else if (k == "weight") {
			r.weight = v.to_int();
		} else if (k == "grade") {
			r.grade = v.to_int();
		} else if (k == "opsz") {
			r.opsz = v.to_int();
		} else if (k == "fill") {
			r.fill = v.to_float();
		} else if (k == "color" && v.length() == 6) {
			int hex = v.hex_to_int();
			r.color = Color(((hex >> 16) & 0xff) / 255.0f, ((hex >> 8) & 0xff) / 255.0f, (hex & 0xff) / 255.0f, 1.0f);
		}
	}
	return true;
}
} // namespace

void MaterialSymbolsBrowser::_rebuild_manifest_view() {
	// Clear previous rows.
	while (manifest_rows->get_child_count()) {
		Node *c = manifest_rows->get_child(0);
		manifest_rows->remove_child(c);
		c->queue_delete();
	}

	const String path = _resolve_manifest_path();
	if (!FileAccess::exists(path)) {
		Label *empty = memnew(Label);
		empty->set_text(vformat("(no manifest at %s — \"Add to subset\" creates one)", path));
		manifest_rows->add_child(empty);
		manifest_viewer->set_title(vformat("material_symbols.subset (empty) — %s", path));
		return;
	}

	FileAccess *fr = FileAccess::open(path, FileAccess::READ);
	if (!fr) {
		Label *err = memnew(Label);
		err->set_text(vformat("Cannot open %s", path));
		manifest_rows->add_child(err);
		return;
	}
	String text = fr->get_as_utf8_string();
	memdelete(fr);

	Vector<String> lines = text.split("\n");
	int total_entries = 0;
	for (int i = 0; i < lines.size(); i++) {
		ManifestEntry e;
		if (!_parse_manifest_line(lines[i], i, e)) {
			continue;
		}
		total_entries++;

		HBoxContainer *row = memnew(HBoxContainer);
		row->add_constant_override("separation", 8);
		manifest_rows->add_child(row);

		// Live preview using the editor-side renderer at the entry's axes.
		MaterialSymbols::Style st = MaterialSymbols::STYLE_ROUNDED;
		if (e.style == "outlined") {
			st = MaterialSymbols::STYLE_OUTLINED;
		} else if (e.style == "sharp") {
			st = MaterialSymbols::STYLE_SHARP;
		}
		Dictionary axes;
		axes["color"] = e.color;
		axes["weight"] = e.weight;
		axes["grade"] = e.grade;
		axes["opsz"] = e.opsz;
		axes["fill"] = e.fill;
		Ref<ImageTexture> tex = ms->get_texture(st, e.name, e.size_px, axes);

		TextureRect *icon = memnew(TextureRect);
		if (tex.is_valid()) {
			icon->set_texture(tex);
		}
		icon->set_custom_minimum_size(Size2(40, 40));
		icon->set_stretch_mode(TextureRect::STRETCH_KEEP_CENTERED);
		row->add_child(icon);

		Label *summary = memnew(Label);
		String detail = e.name;
		if (e.tag != "default") {
			detail += "@" + e.tag;
		}
		detail += "  style=" + e.style;
		detail += " size=" + itos(e.size_px);
		detail += " w=" + itos(e.weight);
		detail += " g=" + itos(e.grade);
		detail += " opsz=" + itos(e.opsz);
		detail += " fill=" + rtos(e.fill).pad_decimals(2);
		uint8_t cr = (uint8_t)CLAMP((int)round(e.color.r * 255.0f), 0, 255);
		uint8_t cg = (uint8_t)CLAMP((int)round(e.color.g * 255.0f), 0, 255);
		uint8_t cb = (uint8_t)CLAMP((int)round(e.color.b * 255.0f), 0, 255);
		if (cr != 0xff || cg != 0xff || cb != 0xff) {
			detail += vformat(" color=%02x%02x%02x", cr, cg, cb);
		}
		summary->set_text(detail);
		summary->set_h_size_flags(SIZE_EXPAND_FILL);
		row->add_child(summary);

		Button *remove = memnew(Button);
		remove->set_text("X");
		remove->set_tooltip(vformat("Remove '%s' from manifest", e.name));
		remove->connect("pressed", this, "_on_manifest_remove", varray(e.line_index));
		row->add_child(remove);
	}
	manifest_viewer->set_title(itos(total_entries) + " entries -- " + path);
}

void MaterialSymbolsBrowser::_on_manifest_remove(int p_line_index) {
	const String path = _resolve_manifest_path();
	if (!FileAccess::exists(path)) {
		return;
	}
	FileAccess *fr = FileAccess::open(path, FileAccess::READ);
	if (!fr) {
		return;
	}
	String text = fr->get_as_utf8_string();
	memdelete(fr);
	Vector<String> lines = text.split("\n");
	if (p_line_index < 0 || p_line_index >= lines.size()) {
		return;
	}
	lines.remove(p_line_index);

	FileAccess *fw = FileAccess::open(path, FileAccess::WRITE);
	if (!fw) {
		status_label->set_text(vformat("Cannot write manifest at %s", path));
		return;
	}
	for (int i = 0; i < lines.size(); i++) {
		fw->store_string(lines[i]);
		if (i < lines.size() - 1) {
			fw->store_8('\n');
		}
	}
	memdelete(fw);
	_rebuild_manifest_view();
}

void MaterialSymbolsBrowser::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_search_changed", "q"), &MaterialSymbolsBrowser::_on_search_changed);
	ClassDB::bind_method(D_METHOD("_on_style_changed", "idx"), &MaterialSymbolsBrowser::_on_style_changed);
	ClassDB::bind_method(D_METHOD("_on_size_changed", "v"), &MaterialSymbolsBrowser::_on_size_changed);
	ClassDB::bind_method(D_METHOD("_on_color_changed", "c"), &MaterialSymbolsBrowser::_on_color_changed);
	ClassDB::bind_method(D_METHOD("_on_axis_changed", "v"), &MaterialSymbolsBrowser::_on_axis_changed);
	ClassDB::bind_method(D_METHOD("_on_copy_pressed"), &MaterialSymbolsBrowser::_on_copy_pressed);
	ClassDB::bind_method(D_METHOD("_on_copy_info_pressed"), &MaterialSymbolsBrowser::_on_copy_info_pressed);
	ClassDB::bind_method(D_METHOD("_on_add_to_subset_pressed"), &MaterialSymbolsBrowser::_on_add_to_subset_pressed);
	ClassDB::bind_method(D_METHOD("_on_item_selected", "idx"), &MaterialSymbolsBrowser::_on_item_selected);
	ClassDB::bind_method(D_METHOD("_on_manifest_view_pressed"), &MaterialSymbolsBrowser::_on_manifest_view_pressed);
	ClassDB::bind_method(D_METHOD("_on_manifest_remove", "line_index"), &MaterialSymbolsBrowser::_on_manifest_remove);
	ClassDB::bind_method(D_METHOD("_on_manifest_clear_pressed"), &MaterialSymbolsBrowser::_on_manifest_clear_pressed);
	ClassDB::bind_method(D_METHOD("_on_manifest_edit_pressed"), &MaterialSymbolsBrowser::_on_manifest_edit_pressed);
	ClassDB::bind_method(D_METHOD("_on_manifest_reveal_pressed"), &MaterialSymbolsBrowser::_on_manifest_reveal_pressed);
	ClassDB::bind_method(D_METHOD("_on_manifest_copy_path_pressed"), &MaterialSymbolsBrowser::_on_manifest_copy_path_pressed);
	ClassDB::bind_method(D_METHOD("_on_viewer_close_pressed"), &MaterialSymbolsBrowser::_on_viewer_close_pressed);
	ClassDB::bind_method(D_METHOD("_on_viewer_save_pressed"), &MaterialSymbolsBrowser::_on_viewer_save_pressed);
	ClassDB::bind_method(D_METHOD("_on_viewer_save_path_selected", "path"), &MaterialSymbolsBrowser::_on_viewer_save_path_selected);
	ClassDB::bind_method(D_METHOD("_on_viewer_copy_pressed"), &MaterialSymbolsBrowser::_on_viewer_copy_pressed);
	ClassDB::bind_method(D_METHOD("_on_tag_changed", "t"), &MaterialSymbolsBrowser::_on_tag_changed);
}

// =========================================================================
// MaterialSymbolsBrowserPlugin
// =========================================================================

MaterialSymbolsBrowserPlugin::MaterialSymbolsBrowserPlugin(EditorNode * /*p_node*/) {
	dock = memnew(MaterialSymbolsBrowser);
	dock->set_name("Material Symbols");
	add_control_to_bottom_panel(dock, "Material Symbols");
}

MaterialSymbolsBrowserPlugin::~MaterialSymbolsBrowserPlugin() {
}

#endif // TOOLS_ENABLED
