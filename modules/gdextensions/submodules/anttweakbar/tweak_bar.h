/**************************************************************************/
/*  tweak_bar.h                                                           */
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

#ifndef TWEAK_BAR_H
#define TWEAK_BAR_H

#include "core/reference.h"
#include "scene/2d/node_2d.h"

#include <cfloat>
#include <deque>

class TweakBar : public Node2D {
	GDCLASS(TweakBar, Node2D);

	bool initialized;

public:
	// Chart data for histogram, line chart, and flame graph widgets
	struct ChartData {
		enum ChartType { HISTOGRAM,
			LINE_CHART,
			FLAME_GRAPH } type;
		// Histogram & line chart: rolling buffer
		std::deque<float> values;
		int max_history;
		float scale_min, scale_max; // FLT_MAX = auto-range
		// Flame graph entries
		struct FlameEntry {
			float start, end;
			int level;
			String caption;
		};
		Vector<FlameEntry> flame_entries;

		ChartData() :
				type(HISTOGRAM), max_history(128), scale_min(FLT_MAX), scale_max(FLT_MAX) {}
	};

private:
	struct VarData {
		enum Type { FLOAT,
			INT,
			BOOL,
			STRING,
			COLOR3,
			COLOR4,
			DIR3 } type;
		float f;
		int i;
		bool b;
		float c3[3];
		float c4[4];
		float d3[3];
	};

	struct VarCBInfo {
		TweakBar *self;
		String bar_name;
		String var_name;
		VarData::Type type;
	};

	// Property binding: binds a TweakBar variable to a Godot object's property
	struct PropertyBinding {
		ObjectID object_id; // safe reference (doesn't prevent GC)
		String property;
		String bar_name;
		String var_name;
		VarData::Type type;
		TweakBar *self;
	};

	Map<String, VarData> m_vars;
	Map<String, String> m_string_vars;
	Map<String, PropertyBinding *> m_bindings; // var_key → binding
	Map<String, ChartData *> m_charts; // var_key → chart data
	Vector<VarCBInfo *> m_cb_infos;
	Vector<VarCBInfo *> m_btn_infos;

	static void _set_cb(const void *value, void *clientData);
	static void _get_cb(void *value, void *clientData);
	static void _btn_cb(void *clientData);

	// Property binding callbacks
	static void _prop_set_cb(const void *value, void *clientData);
	static void _prop_get_cb(void *value, void *clientData);
	VarData::Type _variant_type_to_var_type(Variant::Type p_type) const;

	void _ensure_init();
	String _var_key(const String &bar, const String &var) const;
	void _auto_discover_configs();
	void _apply_config(Object *p_object, const Dictionary &p_config);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	// Bar management
	bool new_bar(const String &p_name);
	void delete_bar(const String &p_name);
	int get_bar_count() const;
	void define(const String &p_def);

	// Variables — bar name, variable name, AntTweakBar definition string
	bool add_float(const String &p_bar, const String &p_name, const String &p_def);
	bool add_int(const String &p_bar, const String &p_name, const String &p_def);
	bool add_bool(const String &p_bar, const String &p_name, const String &p_def);
	bool add_string(const String &p_bar, const String &p_name, const String &p_def);
	bool add_color3(const String &p_bar, const String &p_name, const String &p_def);
	bool add_color4(const String &p_bar, const String &p_name, const String &p_def);
	bool add_direction(const String &p_bar, const String &p_name, const String &p_def);
	bool add_button(const String &p_bar, const String &p_name, const String &p_def);
	bool add_separator(const String &p_bar, const String &p_name);
	bool remove_var(const String &p_bar, const String &p_name);
	bool remove_all_vars(const String &p_bar);

	// Get/set variable values
	Variant get_value(const String &p_bar, const String &p_name) const;
	void set_value(const String &p_bar, const String &p_name, const Variant &p_value);

	// Property binding: observe a Godot object property
	// Automatically detects type and creates the right TweakBar variable.
	// The bar variable stays synced with the object property bidirectionally.
	bool bind_property(const String &p_bar, const String &p_name, Object *p_object, const String &p_property, const String &p_def = "");

	// Variant-based: auto-detect type and add appropriate variable
	bool add_variant(const String &p_bar, const String &p_name, const Variant &p_value, const String &p_def = "");

	// Chart widgets
	bool add_histogram(const String &p_bar, const String &p_name, const String &p_def);
	bool add_line_chart(const String &p_bar, const String &p_name, const String &p_def);
	bool add_flame_graph(const String &p_bar, const String &p_name, const String &p_def);
	void chart_push_value(const String &p_bar, const String &p_name, float p_value);
	void chart_set_values(const String &p_bar, const String &p_name, const PoolRealArray &p_values);
	void chart_add_flame_entry(const String &p_bar, const String &p_name, const Vector2 &p_range, int p_level, const String &p_caption);
	void chart_clear(const String &p_bar, const String &p_name);

	// Input forwarding
	void _handle_input(const Ref<InputEvent> &p_event);

	// Info
	String get_last_error() const;
	void refresh();

	TweakBar();
	~TweakBar();
};

#endif // TWEAK_BAR_H
