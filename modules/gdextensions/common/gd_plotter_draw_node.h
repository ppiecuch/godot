/**************************************************************************/
/*  gd_plotter_draw_node.h                                                */
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

#ifndef GD_PLOTTER_DRAW_NODE_H
#define GD_PLOTTER_DRAW_NODE_H

#include "scene/gui/control.h"
#include "scene/resources/font.h"

#include <deque>

class GdPlotterDraw : public Control {
	GDCLASS(GdPlotterDraw, Control);

public:
	enum PlotMode {
		MODE_LINES,
		MODE_HISTOGRAM,
		MODE_FLAME,
	};

	struct FlameEntry {
		real_t start;
		real_t end;
		uint8_t level;
		String caption;
	};

	static const int MAX_SERIES = 8;

private:
	PlotMode mode;
	String label;
	String overlay_text;
	real_t scale_min;
	real_t scale_max;
	int max_history;
	Color bg_color;
	bool draw_background;

	Ref<Font> font;
	int series_count;
	std::deque<real_t> series_values[MAX_SERIES];
	Color series_colors[MAX_SERIES];
	Vector<FlameEntry> flame_entries;
	bool dirty;

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_mode(PlotMode p_mode);
	PlotMode get_mode() const;

	void set_label(const String &p_label);
	String get_label() const;

	void set_overlay_text(const String &p_text);
	String get_overlay_text() const;

	void set_scale_min(real_t p_min);
	real_t get_scale_min() const;
	void set_scale_max(real_t p_max);
	real_t get_scale_max() const;

	void set_max_history(int p_max);
	int get_max_history() const;

	void set_background_color(const Color &p_color);
	Color get_background_color() const;
	void set_draw_background(bool p_draw);
	bool get_draw_background() const;

	void set_series_count(int p_count);
	int get_series_count() const;
	void set_series_color(int p_series, const Color &p_color);
	Color get_series_color(int p_series) const;

	void add_sample(real_t p_value);
	void add_sample_to_series(int p_series, real_t p_value);
	void add_flame_entry(real_t p_start, real_t p_end, int p_level, const String &p_caption);
	void clear_flame_entries();
	void reset();

	GdPlotterDraw();
};

VARIANT_ENUM_CAST(GdPlotterDraw::PlotMode);

#endif // GD_PLOTTER_DRAW_NODE_H
