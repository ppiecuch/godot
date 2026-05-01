/**************************************************************************/
/*  material_symbols_browser.h                                            */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

// Editor-only dock that lets you browse the full Material Symbols catalogue
// at design time: search, switch style (Outlined / Rounded / Sharp), tweak
// FILL/wght/GRAD/opsz, recolour, copy a name to the clipboard, or append the
// current selection to the project's `material_symbols.subset` manifest.

#ifndef MATERIAL_SYMBOLS_BROWSER_H
#define MATERIAL_SYMBOLS_BROWSER_H

#ifdef TOOLS_ENABLED

#include "core/reference.h"
#include "editor/editor_plugin.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/color_picker.h"
#include "scene/gui/item_list.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/option_button.h"
#include "scene/gui/slider.h"
#include "scene/gui/spin_box.h"

#include "material_symbols.h"

class MaterialSymbolsBrowser : public VBoxContainer {
	GDCLASS(MaterialSymbolsBrowser, VBoxContainer);

	// Top row
	LineEdit *search_field;
	OptionButton *style_picker;
	SpinBox *preview_size;
	ColorPickerButton *color_picker;

	// Axis sliders row
	HSlider *weight_slider;
	Label *weight_value;
	HSlider *grade_slider;
	Label *grade_value;
	HSlider *opsz_slider;
	Label *opsz_value;
	HSlider *fill_slider;
	Label *fill_value;

	// Icon list
	ItemList *icon_list;

	// Bottom row
	Label *status_label;
	LineEdit *tag_field;
	Button *copy_button;
	Button *add_to_subset_button;
	LineEdit *manifest_path_field;

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
	void _on_axis_changed(double p_v);
	void _on_style_changed(int p_idx);
	void _on_size_changed(double p_v);
	void _on_color_changed(const Color &p_c);
	void _on_copy_pressed();
	void _on_add_to_subset_pressed();
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
