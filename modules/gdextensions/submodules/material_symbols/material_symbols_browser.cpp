/**************************************************************************/
/*  material_symbols_browser.cpp                                          */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
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

	// ── Top row: search | style | size | color ────────────────────────────
	HBoxContainer *top = memnew(HBoxContainer);
	top->add_constant_override("separation", 8);
	add_child(top);

	Label *search_label = memnew(Label);
	search_label->set_text("Search:");
	top->add_child(search_label);

	search_field = memnew(LineEdit);
	search_field->set_h_size_flags(SIZE_EXPAND_FILL);
	search_field->set_placeholder("substring match — e.g. \"play\", \"folder\"");
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

	// ── Axis sliders ──────────────────────────────────────────────────────
	HBoxContainer *axes = memnew(HBoxContainer);
	axes->add_constant_override("separation", 8);
	add_child(axes);

	auto add_axis = [&](const String &p_label, HSlider *&r_slider, Label *&r_value,
							double p_min, double p_max, double p_step, double p_default) {
		VBoxContainer *col = memnew(VBoxContainer);
		col->set_h_size_flags(SIZE_EXPAND_FILL);
		HBoxContainer *hb = memnew(HBoxContainer);
		Label *l = memnew(Label);
		l->set_text(p_label);
		hb->add_child(l);
		r_value = memnew(Label);
		r_value->set_text(rtos(p_default));
		r_value->set_h_size_flags(SIZE_EXPAND_FILL);
		r_value->set_align(Label::ALIGN_RIGHT);
		hb->add_child(r_value);
		col->add_child(hb);
		r_slider = memnew(HSlider);
		r_slider->set_min(p_min);
		r_slider->set_max(p_max);
		r_slider->set_step(p_step);
		r_slider->set_value(p_default);
		r_slider->set_h_size_flags(SIZE_EXPAND_FILL);
		r_slider->connect("value_changed", this, "_on_axis_changed");
		col->add_child(r_slider);
		axes->add_child(col);
	};
	add_axis("Weight", weight_slider, weight_value, 100, 700, 100, 400);
	add_axis("Grade", grade_slider, grade_value, -25, 200, 25, 0);
	add_axis("OptSize", opsz_slider, opsz_value, 20, 48, 4, 24);
	add_axis("Fill", fill_slider, fill_value, 0.0, 1.0, 0.05, 0.0);

	// ── Icon list (icon mode + lazy render) ───────────────────────────────
	icon_list = memnew(ItemList);
	icon_list->set_v_size_flags(SIZE_EXPAND_FILL);
	icon_list->set_max_columns(0); // auto-fit
	icon_list->set_same_column_width(true);
	icon_list->set_icon_mode(ItemList::ICON_MODE_TOP);
	icon_list->set_fixed_icon_size(Size2(48, 48));
	icon_list->set_select_mode(ItemList::SELECT_SINGLE);
	add_child(icon_list);

	// ── Bottom row: status | tag | actions ────────────────────────────────
	HBoxContainer *bottom = memnew(HBoxContainer);
	bottom->add_constant_override("separation", 8);
	add_child(bottom);

	status_label = memnew(Label);
	status_label->set_h_size_flags(SIZE_EXPAND_FILL);
	bottom->add_child(status_label);

	Label *tag_lbl = memnew(Label);
	tag_lbl->set_text("Tag:");
	bottom->add_child(tag_lbl);

	tag_field = memnew(LineEdit);
	tag_field->set_text("default");
	tag_field->set_custom_minimum_size(Size2(120, 0));
	bottom->add_child(tag_field);

	Label *manifest_lbl = memnew(Label);
	manifest_lbl->set_text("Manifest:");
	bottom->add_child(manifest_lbl);

	manifest_path_field = memnew(LineEdit);
	manifest_path_field->set_custom_minimum_size(Size2(280, 0));
	manifest_path_field->set_placeholder("res://material_symbols.subset");
	bottom->add_child(manifest_path_field);

	copy_button = memnew(Button);
	copy_button->set_text("Copy name");
	copy_button->connect("pressed", this, "_on_copy_pressed");
	bottom->add_child(copy_button);

	add_to_subset_button = memnew(Button);
	add_to_subset_button->set_text("Add to subset");
	add_to_subset_button->connect("pressed", this, "_on_add_to_subset_pressed");
	bottom->add_child(add_to_subset_button);

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
	d["weight"] = (int)weight_slider->get_value();
	d["grade"] = (int)grade_slider->get_value();
	d["opsz"] = (int)opsz_slider->get_value();
	d["fill"] = (float)fill_slider->get_value();
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
		icon_list->add_item(filter_results[i]);
	}
	render_cursor = 0;
	set_process(true); // start lazy render
	_refresh_status();
}

void MaterialSymbolsBrowser::_refresh_status() {
	const int total = ms->get_count((MaterialSymbols::Style)_current_style());
	status_label->set_text(vformat("%d / %d shown — Cache: %d", filter_results.size(), total, ms->get_image_cache_size()));
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
}

void MaterialSymbolsBrowser::_on_color_changed(const Color & /*p_c*/) {
	render_cursor = 0;
	set_process(true);
}

void MaterialSymbolsBrowser::_on_axis_changed(double /*p_v*/) {
	weight_value->set_text(itos((int)weight_slider->get_value()));
	grade_value->set_text(itos((int)grade_slider->get_value()));
	opsz_value->set_text(itos((int)opsz_slider->get_value()));
	fill_value->set_text(rtos(fill_slider->get_value()).pad_decimals(2));
	render_cursor = 0;
	set_process(true);
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
	const int style = _current_style();
	const char *style_str = "rounded";
	if (style == MaterialSymbols::STYLE_OUTLINED)
		style_str = "outlined";
	else if (style == MaterialSymbols::STYLE_SHARP)
		style_str = "sharp";
	const Color c = color_picker->get_pick_color();

	// Build a manifest line. Always include style; include axis tweaks only
	// when they differ from the defaults.
	String line = name;
	if (!tag.empty() && tag != "default") {
		line += " tag=" + tag;
	}
	line += " style=";
	line += style_str;
	int sz = (int)preview_size->get_value();
	if (sz != 24)
		line += " size=" + itos(sz);
	int w = (int)weight_slider->get_value();
	if (w != 400)
		line += " weight=" + itos(w);
	int g = (int)grade_slider->get_value();
	if (g != 0)
		line += " grade=" + itos(g);
	int o = (int)opsz_slider->get_value();
	if (o != 24)
		line += " opsz=" + itos(o);
	float f = fill_slider->get_value();
	if (f > 0.001f)
		line += " fill=" + rtos(f).pad_decimals(2);
	uint8_t cr = (uint8_t)CLAMP((int)round(c.r * 255.0f), 0, 255);
	uint8_t cg = (uint8_t)CLAMP((int)round(c.g * 255.0f), 0, 255);
	uint8_t cb = (uint8_t)CLAMP((int)round(c.b * 255.0f), 0, 255);
	if (cr != 0xff || cg != 0xff || cb != 0xff) {
		line += vformat(" color=%02x%02x%02x", cr, cg, cb);
	}

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

void MaterialSymbolsBrowser::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_search_changed", "q"), &MaterialSymbolsBrowser::_on_search_changed);
	ClassDB::bind_method(D_METHOD("_on_style_changed", "idx"), &MaterialSymbolsBrowser::_on_style_changed);
	ClassDB::bind_method(D_METHOD("_on_size_changed", "v"), &MaterialSymbolsBrowser::_on_size_changed);
	ClassDB::bind_method(D_METHOD("_on_color_changed", "c"), &MaterialSymbolsBrowser::_on_color_changed);
	ClassDB::bind_method(D_METHOD("_on_axis_changed", "v"), &MaterialSymbolsBrowser::_on_axis_changed);
	ClassDB::bind_method(D_METHOD("_on_copy_pressed"), &MaterialSymbolsBrowser::_on_copy_pressed);
	ClassDB::bind_method(D_METHOD("_on_add_to_subset_pressed"), &MaterialSymbolsBrowser::_on_add_to_subset_pressed);
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
