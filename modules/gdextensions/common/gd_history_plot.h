/**************************************************************************/
/*  gd_history_plot.h                                                     */
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

#ifndef GD_HISTORY_PLOT_H
#define GD_HISTORY_PLOT_H

#include "common/gd_core.h"
#include "core/color.h"
#include "core/math/math_defs.h"
#include "scene/gui/control.h"
#include "scene/resources/font.h"
#include "scene/resources/mesh.h"

#include <deque>
#include <limits>
#include <vector>

class GdHistoryPlot : public Control {
	GDCLASS(GdHistoryPlot, Control);

public:
	enum RangeMode {
		RANGE_MANUAL,
		RANGE_LOWER_FIXED,
		RANGE_AUTOMATIC,
	};

	enum PaletteColor {
		PAL_WARM,
		PAL_COOL,
		PAL_NEON,
	};

private:
	typedef struct _MeshInfo {
		Ref<ArrayMesh> m;
		bool _dirty;
		void clear() {
			m->clear_mesh();
			_dirty = true;
		}
		_MeshInfo() {
			m = newref(ArrayMesh);
			_dirty = true;
		}
	} MeshInfo;

	void refill_grid_mesh(const Rect2 &p_frame);
	void refill_plot_mesh(MeshInfo &p_mesh, std::deque<real_t> &p_vals);

	String title_label;
	std::deque<real_t> values;
	std::deque<real_t> smooth_values;

	std::vector<real_t> horizontal_guides;
	bool draw_guide_values;
	std::vector<Color> horizontal_guide_colors;

	real_t lowest, highest; // auto-updated range of all the current data values
	real_t manual_lowest, manual_highest;

	RangeMode range_mode;

	bool draw_background;
	Ref<StyleBoxFlat> bg;

	bool auto_update;
	bool draw_numerical_info;
	bool humanize_string;
	bool respect_borders;
	bool draw_grid;
	bool shrink_back_in_auto_range;
	bool draw_from_right; // begin drawing graph from right instead of left
	bool draw_header;

	int max_history;
	int precision;

	size_t count;
	int auto_recalc_interval;

	Color line_color;
	Color bg_color;
	Color grid_color;
	Color text_color;
	Size2 text_scale;

	int draw_skip;
	real_t grid_unit;

	bool show_smoothed_plot;
	real_t smooth_value; // average of the last plotted vals
	real_t smooth_factor; // (0 1.0] >> 1.0 means no smoothing

	bool plot_needs_refresh;

	MeshInfo grid_mesh, plot_mesh, smooth_plot_mesh, text_mesh;

	Rect2 prev_rect;

	void _recalc_low_high();
	bool _prep_vec_glyph(PoolVector2Array &p_data, uint8_t p_idx, const Point2 &p_pos, const Size2 &p_scale = Size2(1, 1));
	void _draw_vec_text(const String &p_text, const Point2 &p_pos, const Color &p_color, const Size2 &p_scale = Size2(1, 1));
	void _draw_vec_text(const String &p_text, const Point2 &p_pos, const Size2 &p_scale = Size2(1, 1));
	Size2 _size_vec_text(const String &p_text, const Size2 &p_scale);

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	// adds in the plot current value of the specified var,
	// usually you would call this once per frame if not auto_update_ == false for the graph to update
	void add_sample(real_t p_val);

	void draw(const Rect2 &p_frame); // draws a box

	void reset(); // clears out history

	void set_title_label(String p_label);
	String get_title_label() const;

	void set_range(real_t p_low, real_t p_high); // plot range is manually set
	void set_lower_range(real_t p_low); // low is fixed, high is auto
	void set_range_auto(); // low and high range of the plot is auto-calculated
	real_t get_lower_range() const;
	real_t get_higer_range() const;
	void set_auto_range_shrinks_back(bool p_shrink); // is the range allowed to shrink?
	RangeMode get_range_mode() const;
	void set_auto_recalc_interval(int p_interval);
	int get_auto_recalc_interval() const;

	void add_horizontal_guide(real_t p_yval, const Color &p_color);
	void clear_horizontal_guides();
	void set_draw_guide_values(bool p_draw);

	void set_max_history(int p_max); // how many samples to keep at max (older samples deleted if more are added)
	int get_max_history() const;
	void set_precision(int p_prec); // number of decimals to show
	int get_precision() const;

	void set_text_color(const Color &p_color);
	Color get_text_color() const;
	void set_line_color(const Color &p_color);
	Color get_line_color() const;
	void set_background_color(const Color &p_color);
	Color get_background_color() const;

	void show_grid(bool p_draw);
	bool is_grid() const;
	void set_grid_color(const Color &p_color);
	Color get_grid_color() const;
	void set_grid_unit(real_t p_unit); // pixels
	real_t get_grid_unit() const;

	void show_numerical_info(bool p_show);
	bool is_numerical_info() const;
	void set_respect_borders(bool p_respect);
	void set_draw_skip_val(int p_skip); // draw evey n samples, might speed up drawing
	void show_background(bool p_show);
	bool is_background() const;
	void show_text_header(bool p_draw);
	bool is_text_header() const;
	void set_humanize_value(bool p_set);
	bool is_humanize_value() const;

	void set_draw_from_right(bool p_val); // begin drawing graph from right instead of left
	bool get_draw_from_right() const;

	real_t get_lowest_value() const;
	real_t get_highest_value() const;

	void set_show_smoothed_curve(bool p_show);
	void set_smooth_filter(real_t p_filter);

	PoolColorArray get_color_from_palette(int pal, int num_colors);

	std::deque<real_t> &get_values();

	GdHistoryPlot();
};

VARIANT_ENUM_CAST(GdHistoryPlot::RangeMode);
VARIANT_ENUM_CAST(GdHistoryPlot::PaletteColor);

#endif // GD_HISTORY_PLOT_H
