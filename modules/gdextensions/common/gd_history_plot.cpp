/*************************************************************************/
/*  gd_history_plot.cpp                                                  */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "gd_history_plot.h"

#include "core/math/transform_2d.h"
#include "core/ustring.h"
#include "core/variant.h"
#include "scene/gui/label.h"
#include "scene/resources/dynamic_font.h"
#include "scene/resources/theme.h"

#include <float.h>

#define DEFAULT_SIZE Size2(200, 30)
#define DEFAULT_LOWEST 0
#define DEFAULT_HIGHEST 1

#define VECFONT_ASCENT 18
#define VECFONT_DESCENT 8
#define VECFONT_ASPECT 0.75
#define VECFONT_HEIGHT (VECFONT_ASCENT + VECFONT_DESCENT)
#define VECFONT_WIDTH (VECFONT_HEIGHT * VECFONT_ASPECT)

void GdHistoryPlot::_recalc_low_high() {
	real_t nlowest = FLT_MAX;
	real_t nhighest = -FLT_MIN;
	for (size_t i = 0; i < values.size(); i++) {
		const real_t val = values[i];
		if (val > nhighest) {
			nhighest = val;
		}
		if (val < nlowest) {
			nlowest = val;
		}
	}
	if (nlowest == FLT_MAX) {
		nlowest = DEFAULT_LOWEST;
	}
	if (nhighest == -FLT_MIN) {
		nhighest = DEFAULT_HIGHEST;
	}
	if (shrink_back_in_auto_range) {
		lowest = nlowest;
		highest = nhighest;
	} else { // never shrink range
		if (lowest < nlowest) {
			lowest = nlowest;
		}
		if (highest > nhighest) {
			highest = nhighest;
		}
	}
}

void GdHistoryPlot::set_max_history(int p_max) {
	max_history = p_max;
	update();
}

int GdHistoryPlot::get_max_history() const {
	return max_history;
}

void GdHistoryPlot::reset() {
	values.clear();
	smooth_values.clear();
	count = 0;
	lowest = -1;
	highest = 1;
}

void GdHistoryPlot::add_sample(real_t p_new_val) {
	if (count < 1) {
		smooth_value = p_new_val;
	}

	count++;

	if (p_new_val > highest) {
		highest = p_new_val;
	}
	if (p_new_val < lowest) {
		lowest = p_new_val;
	}

	values.push_back(p_new_val);

	if (show_smoothed_plot) {
		smooth_value = p_new_val * smooth_factor + smooth_value * (1 - smooth_factor);
		smooth_values.push_back(smooth_value);
	}

	bool needs_range_recalc = false;

	while (values.size() > max_history) {
		real_t val_to_delete = *values.begin();
		if (val_to_delete <= lowest || val_to_delete >= highest) { // we are deleting values from plot, were theey max or min?
			needs_range_recalc = true;
		}
		values.erase(values.begin());
	}
	if (needs_range_recalc || count % auto_recalc_interval == 0) {
		_recalc_low_high();
	}
	if (show_smoothed_plot) {
		while (smooth_values.size() > max_history) {
			smooth_values.erase(smooth_values.begin());
		}
	}
	plot_needs_refresh = true;
	update();
}

void GdHistoryPlot::refill_grid_mesh(const Rect2 &p_frame) {
	grid_mesh.clear();
	const real_t x = p_frame.position.x, y = p_frame.position.y, w = p_frame.size.width, h = p_frame.size.height;
	const int grid_h = grid_unit;
	const int num_lines_h = Math::floor(h / grid_h);
	const int offset_v = (h - (num_lines_h * grid_h)) * 0.5;
	PoolVector2Array data;
	for (int i = 0; i < num_lines_h + 1; i++) {
		data.push_back({ x, y + grid_h * i + offset_v });
		data.push_back({ x + w, y + grid_h * i + offset_v });
	}
	const int grid_w = grid_unit;
	const int num_lines_w = Math::floor(w / grid_w);
	const int offset_w = (w - (num_lines_w * grid_w)) * 0.5;
	for (int i = 0; i < num_lines_w + 1; i++) {
		data.push_back(Vector2(x + grid_w * i + offset_w, y));
		data.push_back(Vector2(x + grid_w * i + offset_w, y + h));
	}
	Array mesh_array;
	mesh_array.resize(VS::ARRAY_MAX);
	mesh_array[VS::ARRAY_VERTEX] = data;
	grid_mesh.m->add_surface_from_arrays(Mesh::PRIMITIVE_LINES, mesh_array);
	grid_mesh._dirty = false;
}

void GdHistoryPlot::refill_plot_mesh(MeshInfo &mesh, std::deque<real_t> &vals) {
	mesh.clear();
	PoolVector2Array data;
	for (size_t i = 0; i < vals.size(); i += draw_skip) {
		data.push_back(Vector2(i, vals[i]));
	}
	Array mesh_array;
	mesh_array.resize(VS::ARRAY_MAX);
	mesh_array[VS::ARRAY_VERTEX] = data;
	mesh.m->add_surface_from_arrays(Mesh::PRIMITIVE_LINE_STRIP, mesh_array);
	mesh._dirty = false;
}

void GdHistoryPlot::add_horizontal_guide(real_t yval, const Color &c) {
	horizontal_guides.push_back(yval);
	horizontal_guide_colors.push_back(c);
	plot_needs_refresh = true;
}

void GdHistoryPlot::clear_horizontal_guides() {
	horizontal_guides.clear();
	horizontal_guide_colors.clear();
	plot_needs_refresh = true;
}

void GdHistoryPlot::draw(const Rect2 &p_frame) {
	Rect2 rc = p_frame;
	if (draw_background) {
		draw_style_box(bg, rc);
		rc.grow_by(-1);
	}

	bool needs_mesh = false;
	if (rc != prev_rect || auto_update || plot_needs_refresh) {
		needs_mesh = true;
		plot_needs_refresh = false;
	}

	real_t plot_low = 0, plot_high = 0;

	switch (range_mode) {
		case RANGE_MANUAL: {
			plot_low = manual_lowest;
			plot_high = manual_highest;
		} break;
		case RANGE_LOWER_FIXED: {
			plot_low = manual_lowest;
			plot_high = highest;
		} break;
		case RANGE_AUTOMATIC: {
			plot_low = lowest;
			plot_high = highest;
		} break;
	}

	const bool needs_grid = (rc != prev_rect);
	const bool have_data = values.size() > 0;

	if (auto_update) {
		update();
	}
	if (draw_grid) {
		if (needs_grid || grid_mesh._dirty) {
			refill_grid_mesh(rc);
		}
		draw_mesh(grid_mesh.m, Ref<Texture>(), Ref<Texture>(), Ref<Texture>(), Transform2D(), grid_color);
	}
	if (draw_header) {
		text_mesh.clear();
		if (!title_label.empty()) {
			_draw_vec_text(title_label, Point2(rc.position.x, VECFONT_HEIGHT * text_scale.y), text_scale);
		}
		if (draw_numerical_info) {
			const real_t cval = have_data ? *values.rbegin() : 0;
			String text = have_data ? vformat("%s \x12%s \x14%s", humanize_string ? String::humanize_size(cval) : String::num(cval, precision), humanize_string ? String::humanize_size(plot_high) : String::num(plot_high, precision), humanize_string ? String::humanize_size(plot_low) : String::num(plot_low, precision))
									: vformat("\x12%s \x14%s", humanize_string ? String::humanize_size(plot_high) : String::num(plot_high, precision), humanize_string ? String::humanize_size(plot_low) : String::num(plot_low, precision));
			Size2 text_size = _size_vec_text(text, text_scale);
			_draw_vec_text(text, Point2(rc.position.x + rc.size.width - text_size.width, VECFONT_HEIGHT * text_scale.y), text_scale);
		}
	}
	for (size_t i = 0; i < horizontal_guides.size(); i++) {
		const real_t myy = horizontal_guides[i];
		if (myy > plot_low && myy < plot_high) { //TODO negative!
			const real_t yy = Math::map2(myy, plot_low, plot_high, 0, rc.size.height, true);
			if (draw_guide_values) {
				_draw_vec_text(String::num(horizontal_guides[i], precision), Point2(10 + rc.position.x, rc.position.y + rc.size.height - yy + 10), horizontal_guide_colors[i].with_alpha(0.2), text_scale);
			}
			draw_line(Point2(rc.position.x, rc.position.y + rc.size.height - yy), Point2(rc.position.x + rc.size.width, rc.position.y + rc.size.height - yy), horizontal_guide_colors[i].with_alpha(0.25));
		}
	}

	if (draw_header || draw_guide_values) {
		draw_mesh(text_mesh.m, Ref<Texture>(), Ref<Texture>(), Ref<Texture>(), Transform2D(), text_color);
	}

	prev_rect = rc;

	if (have_data) {
		if (needs_mesh) {
			refill_plot_mesh(plot_mesh, values);
			if (show_smoothed_plot) {
				refill_plot_mesh(smooth_plot_mesh, smooth_values);
			}
		}
		if (respect_borders && draw_header) {
			rc.size.height -= (VECFONT_HEIGHT * text_scale.y);
		}
		Transform2D draw_xform;
		const real_t plot_values_range = plot_high - plot_low;
		const real_t yscale = (rc.size.height - 1) / plot_values_range;
		const real_t xscale = rc.size.width / max_history;
		if (draw_from_right) {
			draw_xform.translate(rc.size.width, 0);
			draw_xform.scale(Vector2(-1, 1));
		}
		draw_xform.scale(Vector2(xscale, -yscale));
		draw_xform.translate(rc.position.x / xscale, (rc.position.y - (rc.size.height + (respect_borders ? (VECFONT_HEIGHT * text_scale.y) : 0))) / yscale); // bottom-left origin
		draw_xform.translate(0, -plot_low);
		if (show_smoothed_plot) {
			draw_mesh(plot_mesh.m, Ref<Texture>(), Ref<Texture>(), Ref<Texture>(), draw_xform, Color(line_color.r * 0.25, line_color.g * 0.25, line_color.b * 0.25, line_color.a));
			draw_mesh(smooth_plot_mesh.m, Ref<Texture>(), Ref<Texture>(), Ref<Texture>(), draw_xform, line_color);
		} else {
			draw_mesh(plot_mesh.m, Ref<Texture>(), Ref<Texture>(), Ref<Texture>(), draw_xform, line_color);
		}
	}
}

void GdHistoryPlot::set_title_label(String p_label) {
	title_label = p_label;
	update();
}

String GdHistoryPlot::get_title_label() const {
	return title_label;
}

void GdHistoryPlot::set_range_auto() {
	range_mode = RANGE_AUTOMATIC;
}

void GdHistoryPlot::set_range(real_t p_low, real_t p_high) {
	range_mode = RANGE_MANUAL;
	manual_lowest = p_low;
	manual_highest = p_high;
}

void GdHistoryPlot::set_auto_recalc_interval(int p_interval) {
	ERR_FAIL_COND(p_interval <= 1);
	auto_recalc_interval = p_interval;
}

int GdHistoryPlot::get_auto_recalc_interval() const {
	return auto_recalc_interval;
}

void GdHistoryPlot::set_draw_from_right(bool p_val) {
	draw_from_right = p_val;
	update();
}

bool GdHistoryPlot::get_draw_from_right() const {
	return draw_from_right;
}

real_t GdHistoryPlot::get_lower_range() const {
	switch (range_mode) {
		case RANGE_MANUAL:
			return manual_lowest;
		case RANGE_LOWER_FIXED:
			return manual_lowest;
		case RANGE_AUTOMATIC:
			return lowest;
	}
	return -1.0;
}

real_t GdHistoryPlot::get_higer_range() const {
	switch (range_mode) {
		case RANGE_MANUAL:
			return manual_highest;
		case RANGE_LOWER_FIXED:
			return highest;
		case RANGE_AUTOMATIC:
			return highest;
	}
	return 1.0;
}

void GdHistoryPlot::set_lower_range(real_t p_low) {
	range_mode = RANGE_LOWER_FIXED;
	manual_lowest = p_low;
}

real_t GdHistoryPlot::get_lowest_value() const {
	return lowest;
}

real_t GdHistoryPlot::get_highest_value() const {
	return highest;
}

void GdHistoryPlot::set_auto_range_shrinks_back(bool p_shrink) {
	shrink_back_in_auto_range = p_shrink;
};

GdHistoryPlot::RangeMode GdHistoryPlot::get_range_mode() const {
	return range_mode;
}

void GdHistoryPlot::set_draw_guide_values(bool p_state) {
	draw_guide_values = p_state;
}

void GdHistoryPlot::set_precision(int p_prec) {
	precision = CLAMP(p_prec, 0, 15);
	update();
}

int GdHistoryPlot::get_precision() const {
	return precision;
}

void GdHistoryPlot::set_text_color(const Color &p_color) {
	text_color = p_color;
	update();
}

Color GdHistoryPlot::get_text_color() const {
	return text_color;
}

void GdHistoryPlot::set_line_color(const Color &p_color) {
	line_color = p_color;
	update();
}

Color GdHistoryPlot::get_line_color() const {
	return line_color;
}

void GdHistoryPlot::set_background_color(const Color &p_color) {
	bg_color = p_color;
	bg->set_border_color(bg_color.lightened(0.3));
	bg->set_bg_color(bg_color);
	update();
}

Color GdHistoryPlot::get_background_color() const {
	return bg_color;
}

void GdHistoryPlot::show_grid(bool p_state) {
	draw_grid = p_state;
	update();
}

bool GdHistoryPlot::is_grid() const {
	return draw_grid;
}

void GdHistoryPlot::set_grid_color(const Color &p_color) {
	grid_color = p_color;
}

Color GdHistoryPlot::get_grid_color() const {
	return grid_color;
}

void GdHistoryPlot::set_grid_unit(real_t p_unit) {
	grid_unit = p_unit;
}

void GdHistoryPlot::show_numerical_info(bool p_show) {
	draw_numerical_info = p_show;
	update();
}

bool GdHistoryPlot::is_numerical_info() const {
	return draw_numerical_info;
}

void GdHistoryPlot::set_respect_borders(bool p_respect) {
	respect_borders = p_respect;
}

void GdHistoryPlot::set_draw_skip_val(int p_skip) {
	draw_skip = p_skip;
	if (draw_skip < 1) {
		draw_skip = 1;
	}
}

void GdHistoryPlot::show_background(bool p_draw) {
	draw_background = p_draw;
	update();
}

bool GdHistoryPlot::is_background() const {
	return draw_background;
}

void GdHistoryPlot::show_text_header(bool p_draw) {
	draw_header = p_draw;
	update();
}

bool GdHistoryPlot::is_text_header() const {
	return draw_header;
}

void GdHistoryPlot::set_humanize_value(bool p_set) {
	humanize_string = p_set;
	update();
}

bool GdHistoryPlot::is_humanize_value() const {
	return humanize_string;
}

void GdHistoryPlot::set_show_smoothed_curve(bool p_show) {
	show_smoothed_plot = p_show;
}

void GdHistoryPlot::set_smooth_filter(real_t p_filter) {
	smooth_factor = p_filter;
};

std::deque<real_t> &GdHistoryPlot::get_values() {
	return values;
}

#define _rgb(C) Color(((C & 0xff0000) >> 16) / 255., ((C & 0xff00) >> 8) / 255., (C & 0xff) / 255.)

// get color from palette: warm, cool or neon.
PoolColorArray GdHistoryPlot::get_color_from_palette(int pal, int num_colors) {
	static Color warm[][12] = {
		{ _rgb(0xfdb25f) },
		{ _rgb(0xffc96b), _rgb(0xf47942) },
		{ _rgb(0xffc96b), _rgb(0xf47942), _rgb(0xab412c) },
		{ _rgb(0xffd773), _rgb(0xf99851), _rgb(0xef5833), _rgb(0x923e2d) },
		{ _rgb(0xffe67a), _rgb(0xfcaf5e), _rgb(0xf47942), _rgb(0xda492d), _rgb(0x773a2d) },
		{ _rgb(0xffe77e), _rgb(0xfebd65), _rgb(0xf78f4d), _rgb(0xf16137), _rgb(0xc1452c), _rgb(0x6f382e) },
		{ _rgb(0xffe782), _rgb(0xffc76a), _rgb(0xfaa055), _rgb(0xf47942), _rgb(0xef512f), _rgb(0xae422c), _rgb(0x67382e) },
		{ _rgb(0xffe886), _rgb(0xffd06f), _rgb(0xfbad5d), _rgb(0xf68a4b), _rgb(0xf16739), _rgb(0xde4a2d), _rgb(0xa0402c), _rgb(0x5f362f) },
		{ _rgb(0xffe98b), _rgb(0xffd873), _rgb(0xfeb862), _rgb(0xf99851), _rgb(0xf47942), _rgb(0xef5833), _rgb(0xca462d), _rgb(0x913d2d), _rgb(0x573530) },
		{ _rgb(0xffe98e), _rgb(0xffdf76), _rgb(0xffc267), _rgb(0xfaa458), _rgb(0xf68749), _rgb(0xf26a3a), _rgb(0xee4c2d), _rgb(0xb9442c), _rgb(0x843c2d), _rgb(0x4f3330) },
		{ _rgb(0xffe98e), _rgb(0xffe278), _rgb(0xffc76a), _rgb(0xfbad5d), _rgb(0xf8934f), _rgb(0xf47942), _rgb(0xf05d35), _rgb(0xde4a2d), _rgb(0xae422c), _rgb(0x7f3b2d), _rgb(0x4f3330) },
		{ _rgb(0xffe98e), _rgb(0xffe479), _rgb(0xffe479), _rgb(0xfeb460), _rgb(0xfa9c53), _rgb(0xf58547), _rgb(0xf58547), _rgb(0xee5531), _rgb(0xd1492d), _rgb(0xa5402c), _rgb(0x7a3a2d), _rgb(0x4f3330) },
	};
	static Color cool[][12] = {
		{ _rgb(0x41bed1) },
		{ _rgb(0x78cccf), _rgb(0x0b88b4) },
		{ _rgb(0x78cccf), _rgb(0x0b88b4), _rgb(0x00357e) },
		{ _rgb(0x96d5cd), _rgb(0x0caac9), _rgb(0x07669e), _rgb(0x002c71) },
		{ _rgb(0xb2dfcc), _rgb(0x3bbcd0), _rgb(0x0b88b4), _rgb(0x044f8f), _rgb(0x042561) },
		{ _rgb(0xbae0cd), _rgb(0x5cc5cf), _rgb(0x0da0c3), _rgb(0x096fa4), _rgb(0x044186), _rgb(0x05255c) },
		{ _rgb(0xc3e2cc), _rgb(0x75cbcf), _rgb(0x10b1ce), _rgb(0x0b88b4), _rgb(0x0b88b4), _rgb(0x01377f), _rgb(0x052257) },
		{ _rgb(0xcce5cc), _rgb(0x86d1ce), _rgb(0x33bbd1), _rgb(0x0c9ac0), _rgb(0x0a75a7), _rgb(0x065090), _rgb(0x002f79), _rgb(0x051f53) },
		{ _rgb(0xd4e8cc), _rgb(0x96d5cd), _rgb(0x52c2d0), _rgb(0x0caac9), _rgb(0x0b88b4), _rgb(0x07669e), _rgb(0x034589), _rgb(0x012b70), _rgb(0x061d4e) },
		{ _rgb(0xdcebcb), _rgb(0xa3dacd), _rgb(0x67c7cf), _rgb(0x0db6d1), _rgb(0x0b97be), _rgb(0x0b78a9), _rgb(0x065a96), _rgb(0x023d83), _rgb(0x042868), _rgb(0x061c49) },
		{ _rgb(0xdcebcb), _rgb(0xa9dccd), _rgb(0x75cbcf), _rgb(0x33bbd1), _rgb(0x33bbd1), _rgb(0x33bbd1), _rgb(0x096ba2), _rgb(0x065090), _rgb(0x01377f), _rgb(0x032766), _rgb(0x032766) },
		{ _rgb(0xdcebcb), _rgb(0xaeddcd), _rgb(0x7ecece), _rgb(0x48c0d1), _rgb(0x10adcc), _rgb(0x0b94bc), _rgb(0x0b7bab), _rgb(0x09639b), _rgb(0x034a8b), _rgb(0x00337c), _rgb(0x032663), _rgb(0x061c49) },
	};
	static Color neon[][12] = {
		{ _rgb(0xf3648b) },
		{ _rgb(0xf78495), _rgb(0xad3e8c) },
		{ _rgb(0xf78495), _rgb(0xad3e8c), _rgb(0x502370) },
		{ _rgb(0xf9989b), _rgb(0xde4c86), _rgb(0x7d3392), _rgb(0x461e63) },
		{ _rgb(0xfcaca4), _rgb(0xf2618a), _rgb(0xad3e8c), _rgb(0x612b8a), _rgb(0x3c1955) },
		{ _rgb(0xfdb3a5), _rgb(0xf5738f), _rgb(0xd04888), _rgb(0x8a3590), _rgb(0x58277c), _rgb(0x3a1951) },
		{ _rgb(0xfeb9a7), _rgb(0xf68294), _rgb(0xea5084), _rgb(0xad3e8c), _rgb(0x703093), _rgb(0x512472), _rgb(0x37164c) },
		{ _rgb(0xffbfaa), _rgb(0xf88f98), _rgb(0xf25e89), _rgb(0xc84688), _rgb(0x93378f), _rgb(0x632c8c), _rgb(0x4b2069), _rgb(0x331548) },
		{ _rgb(0xffc5ac), _rgb(0xf9999c), _rgb(0xf46e8d), _rgb(0xde4c86), _rgb(0xad3e8c), _rgb(0x7c3292), _rgb(0x5b2981), _rgb(0x451e62), _rgb(0x301443) },
		{ _rgb(0xffcbae), _rgb(0xfaa2a0), _rgb(0xf57a91), _rgb(0xf05384), _rgb(0xc44488), _rgb(0x97388e), _rgb(0x692f94), _rgb(0x552578), _rgb(0x411c5c), _rgb(0x2d133f) },
		{ _rgb(0xffcbae), _rgb(0xfba6a2), _rgb(0xf68294), _rgb(0xf25e89), _rgb(0xd64a87), _rgb(0xad3e8c), _rgb(0x853491), _rgb(0x632c8c), _rgb(0x512472), _rgb(0x3f1b59), _rgb(0x2d133f) },
		{ _rgb(0xffcbae), _rgb(0xfbaaa3), _rgb(0xf78997), _rgb(0xf3688c), _rgb(0xe54f85), _rgb(0xbf4489), _rgb(0x9c398d), _rgb(0x753193), _rgb(0x5e2985), _rgb(0x4d226d), _rgb(0x3d1a57), _rgb(0x2d133f) },
	};
	auto pool_from_data = [](Color *buf_ptr, size_t buf_size) {
		PoolColorArray data;
		data.resize(buf_size);
		memcpy(data.write().ptr(), buf_ptr, buf_size * sizeof(Color));
		return data;
	};
	switch (num_colors) {
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12: {
			switch (pal) {
				case PAL_WARM: return pool_from_data(warm[num_colors - 1], num_colors);
				case PAL_COOL: return pool_from_data(cool[num_colors - 1], num_colors);
				case PAL_NEON: return pool_from_data(neon[num_colors - 1], num_colors);
				default: {
					WARN_PRINT("Undefined palette");
					return PoolColorArray();
				}
			}
		} break;
		default: {
			WARN_PRINT("Undefined palette");
			return PoolColorArray();
		}
	}
}

void GdHistoryPlot::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			bg->set_border_width_all(1);
			bg->set_corner_radius_all(2);
		} break;
		case NOTIFICATION_ENTER_TREE: {
		} break;
		case NOTIFICATION_EXIT_TREE: {
		} break;
		case NOTIFICATION_DRAW: {
			draw(Rect2(Point2(), get_size()));
		} break;
		case NOTIFICATION_THEME_CHANGED: {
		}
	}
}

void GdHistoryPlot::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_max_history", "size"), &GdHistoryPlot::set_max_history);
	ClassDB::bind_method(D_METHOD("get_max_history"), &GdHistoryPlot::get_max_history);
	ClassDB::bind_method(D_METHOD("set_precision", "prec"), &GdHistoryPlot::set_precision);
	ClassDB::bind_method(D_METHOD("get_precision"), &GdHistoryPlot::get_precision);
	ClassDB::bind_method(D_METHOD("set_humanize_value", "humanize"), &GdHistoryPlot::set_humanize_value);
	ClassDB::bind_method(D_METHOD("is_humanize_value"), &GdHistoryPlot::is_humanize_value);
	ClassDB::bind_method(D_METHOD("set_background_color", "prec"), &GdHistoryPlot::set_background_color);
	ClassDB::bind_method(D_METHOD("get_background_color"), &GdHistoryPlot::get_background_color);
	ClassDB::bind_method(D_METHOD("set_text_color", "prec"), &GdHistoryPlot::set_line_color);
	ClassDB::bind_method(D_METHOD("get_text_color"), &GdHistoryPlot::get_line_color);
	ClassDB::bind_method(D_METHOD("set_line_color", "prec"), &GdHistoryPlot::set_line_color);
	ClassDB::bind_method(D_METHOD("get_line_color"), &GdHistoryPlot::get_line_color);
	ClassDB::bind_method(D_METHOD("set_grid_color", "prec"), &GdHistoryPlot::set_grid_color);
	ClassDB::bind_method(D_METHOD("get_grid_color"), &GdHistoryPlot::get_grid_color);
	ClassDB::bind_method(D_METHOD("set_title_label", "prec"), &GdHistoryPlot::set_title_label);
	ClassDB::bind_method(D_METHOD("get_title_label"), &GdHistoryPlot::get_title_label);
	ClassDB::bind_method(D_METHOD("show_grid", "prec"), &GdHistoryPlot::show_grid);
	ClassDB::bind_method(D_METHOD("is_grid"), &GdHistoryPlot::is_grid);
	ClassDB::bind_method(D_METHOD("show_text_header", "prec"), &GdHistoryPlot::show_text_header);
	ClassDB::bind_method(D_METHOD("is_text_header"), &GdHistoryPlot::is_text_header);
	ClassDB::bind_method(D_METHOD("show_background", "prec"), &GdHistoryPlot::show_background);
	ClassDB::bind_method(D_METHOD("is_background"), &GdHistoryPlot::is_background);
	ClassDB::bind_method(D_METHOD("set_range", "low", "high"), &GdHistoryPlot::set_range);
	ClassDB::bind_method(D_METHOD("set_lower_range", "lower"), &GdHistoryPlot::set_lower_range);
	ClassDB::bind_method(D_METHOD("set_range_auto"), &GdHistoryPlot::set_range_auto);
	ClassDB::bind_method(D_METHOD("get_lower_range"), &GdHistoryPlot::get_lower_range);
	ClassDB::bind_method(D_METHOD("get_higer_range"), &GdHistoryPlot::get_higer_range);
	ClassDB::bind_method(D_METHOD("set_auto_range_shrinks_back", "shrink"), &GdHistoryPlot::set_auto_range_shrinks_back);
	ClassDB::bind_method(D_METHOD("get_range_mode"), &GdHistoryPlot::get_range_mode);
	ClassDB::bind_method(D_METHOD("get_color_from_palette", "palette", "color"), &GdHistoryPlot::get_color_from_palette);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "text_header"), "show_text_header", "is_text_header");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "title_label"), "set_title_label", "get_title_label");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "text_color"), "set_text_color", "get_text_color");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "history_size"), "set_max_history", "get_max_history");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "display_precision"), "set_precision", "get_precision");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "humanize_value"), "set_humanize_value", "is_humanize_value");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "line_color"), "set_line_color", "get_line_color");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "grid"), "show_grid", "is_grid");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "grid_color"), "set_grid_color", "get_grid_color");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "background"), "show_background", "is_background");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "background_color"), "set_background_color", "get_background_color");

	BIND_ENUM_CONSTANT(PAL_WARM);
	BIND_ENUM_CONSTANT(PAL_COOL);
	BIND_ENUM_CONSTANT(PAL_NEON);

	BIND_ENUM_CONSTANT(RANGE_AUTOMATIC);
	BIND_ENUM_CONSTANT(RANGE_MANUAL);
	BIND_ENUM_CONSTANT(RANGE_LOWER_FIXED);
}

GdHistoryPlot::GdHistoryPlot() {
	auto_update = false;
	lowest = DEFAULT_LOWEST;
	highest = DEFAULT_HIGHEST;
	max_history = 100;
	range_mode = RANGE_AUTOMATIC;
	auto_recalc_interval = 60;
	count = 0;
	precision = 2;
	draw_skip = 1;
	draw_numerical_info = true;
	humanize_string = false;
	respect_borders = true;
	draw_header = true;
	draw_background = true;
	bg_color = Color(0.2, 0.2, 0.2);
	grid_color = Color(1, 1, 1, 0.25);
	text_color = Color(0, 1, 0);
	text_scale = Size2(0.35, 0.35);
	draw_grid = true;
	shrink_back_in_auto_range = false;
	plot_needs_refresh = true;
	grid_unit = 10;
	smooth_factor = 0.1;
	smooth_value = 0;
	show_smoothed_plot = false;
	draw_guide_values = false;
	line_color = Color(1, 0, 0);
	draw_from_right = false;
	bg = newref(StyleBoxFlat);
	bg->set_border_color(bg_color.lightened(0.3));
	bg->set_bg_color(bg_color);
	set_size(DEFAULT_SIZE);
}

/// Simple vector font

//  +-------+  (18)
//  |       |
//  |       |
// -+-------+- (0)
//  |       |
//  +-------+  (-8)
// (0)     (18)

#define F false
#define T true

static bool vecfont_ops[] = { F, T, T, T, T, T, F, T, T, T, T, T, F, T, F, T, T, T, F, T, T, T, T, T, T, F, T, T, T, T, T, T, T, F, T, T, T, T, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, T, T, T, T, T, T, T, F, T, T, F, T, F, T, T, F, T, F, T, T, F, T, F, T, T, F, T, F, T, T, T, F, T, F, T, F, T, T, T, F, T, T, T, F, T, F, T, T, T, F, T, F, T, T, T, T, T, T, T, T, F, T, T, T, T, T, T, T, T, T, F, T, T, T, T, T, T, T, T, F, T, F, T, F, T, T, F, T, T, T, T, T, T, T, T, T, T, F, T, F, T, F, T, T, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, T, T, T, T, T, T, T, T, T, T, F, T, F, T, F, T, T, T, T, F, T, T, T, T, F, T, T, T, T, T, T, T, T, F, T, T, F, T, T, T, F, T, T, T, F, T, F, T, F, T, F, T, F, T, F, T, T, F, T, F, T, T, F, T, F, T, F, T, T, T, T, T, T, T, T, F, T, F, T, T, F, T, T, T, T, T, T, T, F, T, T, T, T, T, T, F, T, T, T, T, T, F, T, T, T, F, T, T, T, T, T, T, T, T, T, F, T, T, T, T, T, T, T, T, T, T, F, T, T, F, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, F, T, T, T, T, T, T, T, T, T, T, F, T, F, T, F, T, T, F, T, F, T, T, F, T, F, T, F, T, T, F, T, T, T, T, T, T, F, T, F, T, T, T, T, T, T, T, T, T, T, T, F, T, T, F, T, F, T, T, T, T, T, T, F, T, T, T, T, F, T, T, T, T, T, T, T, F, T, T, T, T, T, T, F, T, T, F, T, F, T, F, T, T, F, T, F, T, T, T, T, T, T, T, T, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, T, T, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, T, T, T, F, T, T, T, F, T, T, T, T, T, T, T, T, F, T, T, T, T, T, T, F, T, T, T, T, T, T, T, T, F, T, F, T, T, T, T, T, T, F, T, F, T, T, T, T, T, T, T, T, T, T, T, F, T, F, T, F, T, T, T, T, T, F, T, T, F, T, T, T, T, F, T, F, T, F, T, T, F, T, F, T, T, F, T, F, T, T, T, F, T, F, T, T, T, F, T, T, F, T, F, T, T, F, T, T, T, T, T, T, T, T, F, T, F, T, F, T, T, T, T, T, F, T, T, T, T, T, F, T, T, T, T, T, F, T, F, T, T, T, T, T, T, T, T, F, T, T, T, F, T, F, T, T, T, T, T, F, T, T, T, F, T, F, T, T, T, F, T, T, F, T, F, T, T, T, F, T, F, T, F, T, F, T, F, T, F, T, T, F, T, F, T, T, T, F, T, T, T, F, T, F, T, T, T, F, T, T, T, T, T, T, F, T, F, T, T, T, T, T, F, T, T, T, T, T, F, T, T, F, T, F, T, T, F, T, T, T, T, T, T, T, F, T, T, T, F, T, F, T, T, T, T, F, T, T, F, T, T, T, T, F, T, F, T, F, T, F, T, F, T, T, T, F, T, T, T, T, T, T, F, T, F, T, F, T, T, T, T, T, T, F, T, T, T, T, F, T, T, T, F, T, T, T, F, T, T, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, T, T, T, F, T, F, T, T, T, F, T, T, F, T, T, T, T, T, T, F, T, F, T, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, T, F, T, F, T, F, T, T, T, F, T, T, T, T, T, F, T, T, T, F, T, F, T, F, T, T, T, T, T, F, T, F, T, F, T, T, T, T, T, T, T, T, T, T, F, T, T, T, T, F, T, T, T, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, T, T, T, T, T, T, F, T, T, T, T, F, T, F, T, F, T, T, T, T, T, T, T, T, T, T, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, F, T, T, T, T, T, T, T, T, F, T, F, T, F, T, F, T, T, T, F, T, F, T, T, T, T, T, F, T, F, T, F, T, T, T, T, T, F, T, T, T };

#undef F
#undef T

static struct {
	int16_t x, y;
} vecfont_points[] = { { 19, 0 }, { 3, 0 }, { 0, 3 }, { 0, 24 }, { 3, 27 }, { 14, 27 }, { 20, 27 }, { 42, 27 }, { 45, 24 }, { 45, 3 }, { 42, 0 }, { 25, 0 }, { 13, 9 }, { 17, 27 }, { 15, 18 }, { 19, 18 }, { 21, 16 }, { 20, 9 }, { 22, 0 }, { 26, 18 }, { 30, 18 }, { 32, 16 }, { 31, 11 }, { 29, 9 }, { 24, 9 }, { 0, -7 }, { 1, 7 }, { 3, 16 }, { 7, 18 }, { 12, 16 }, { 12, 10 }, { 8, 8 }, { 2, 8 }, { 8, 8 }, { 11, 7 }, { 12, 3 }, { 9, 0 }, { 5, 0 }, { 1, 3 }, { 0, 0 }, { 0, 4 }, { 0, 0 }, { 0, -4 }, { 0, 0 }, { -4, 0 }, { 0, 0 }, { 4, 0 }, { -4, 0 }, { 4, 0 }, { 0, 4 }, { 0, -4 }, { 4, 4 }, { -4, -4 }, { 0, -5 }, { 0, 5 }, { -4, 4 }, { 4, -4 }, { 5, 0 }, { -5, 0 }, { -2, -5 }, { -5, -2 }, { -5, 2 }, { -2, 5 }, { 2, 5 }, { 5, 2 }, { 5, -2 }, { 2, -5 }, { -2, -5 }, { 0, 10 }, { 6, 18 }, { 12, 10 }, { 6, 18 }, { 6, 0 }, { 6, 3 }, { 0, 9 }, { 6, 15 }, { 0, 9 }, { 12, 9 }, { 0, 8 }, { 6, 0 }, { 12, 8 }, { 6, 0 }, { 6, 18 }, { 6, 3 }, { 12, 9 }, { 6, 15 }, { 0, 9 }, { 12, 9 }, { 0, 3 }, { 3, 0 }, { 6, 20 }, { 13, 20 }, { 3, 0 }, { 4, 12 }, { 9, 0 }, { 9, 12 }, { 0, 10 }, { 4, 12 }, { 9, 12 }, { 12, 14 }, { 0, 0 }, { 6, 15 }, { 12, 0 }, { 0, 0 }, { 0, -7 }, { 2, 11 }, { 1, 2 }, { 6, 0 }, { 10, 2 }, { 11, 11 }, { 10, 2 }, { 13, 0 }, { 6, 16 }, { 4, 18 }, { 4, 21 }, { 6, 23 }, { 9, 23 }, { 11, 21 }, { 11, 18 }, { 9, 16 }, { 6, 16 }, { 0, 0 }, { 4, 0 }, { 1, 7 }, { 1, 12 }, { 4, 16 }, { 9, 16 }, { 12, 12 }, { 12, 7 }, { 9, 0 }, { 13, 0 }, { 0, -7 }, { 3, 9 }, { 7, 12 }, { 11, 11 }, { 13, 8 }, { 13, 4 }, { 10, 0 }, { 5, 0 }, { 2, 3 }, { 0, 0 }, { 4, 0 }, { 2, 0 }, { 2, 18 }, { 0, 18 }, { 12, 18 }, { 12, 14 }, { 7, 0 }, { 2, 0 }, { 0, 4 }, { 0, 10 }, { 2, 15 }, { 5, 18 }, { 10, 18 }, { 12, 14 }, { 12, 8 }, { 10, 3 }, { 7, 0 }, { 0, 9 }, { 12, 9 }, { 0, 0 }, { 6, 10 }, { 0, 17 }, { 3, 18 }, { 9, 2 }, { 12, 0 }, { 6, 0 }, { 6, 0 }, { 6, 5 }, { 6, 18 }, { 3, 14 }, { 4, 18 }, { 7, 14 }, { 8, 18 }, { 2, 0 }, { 4, 18 }, { 8, 0 }, { 10, 18 }, { 0, 13 }, { 12, 13 }, { 0, 5 }, { 12, 5 }, { 0, 3 }, { 3, 1 }, { 9, 1 }, { 12, 3 }, { 12, 7 }, { 9, 9 }, { 3, 9 }, { 0, 11 }, { 0, 15 }, { 3, 17 }, { 9, 17 }, { 12, 15 }, { 6, 19 }, { 6, -1 }, { 0, 0 }, { 12, 18 }, { 6, 14 }, { 3, 10 }, { 0, 14 }, { 3, 18 }, { 6, 14 }, { 9, 8 }, { 12, 4 }, { 9, 0 }, { 6, 4 }, { 9, 8 }, { 12, 5 }, { 8, 0 }, { 2, 0 }, { 0, 4 }, { 9, 14 }, { 7, 18 }, { 3, 18 }, { 1, 14 }, { 12, 0 }, { 5, 14 }, { 7, 18 }, { 7, 18 }, { 12, -2 }, { 6, 4 }, { 6, 14 }, { 12, 20 }, { 0, -2 }, { 6, 4 }, { 6, 14 }, { 0, 20 }, { 3, 2 }, { 9, 16 }, { 3, 16 }, { 9, 2 }, { 0, 9 }, { 12, 9 }, { 6, 2 }, { 6, 16 }, { 0, 9 }, { 12, 9 }, { 4, -4 }, { 6, 1 }, { 6, 1 }, { 0, 9 }, { 12, 9 }, { 6, 0 }, { 6, 0 }, { 6, 0 }, { 0, 0 }, { 12, 18 }, { 1, 2 }, { 11, 16 }, { 12, 12 }, { 12, 6 }, { 9, 0 }, { 3, 0 }, { 0, 6 }, { 0, 12 }, { 3, 18 }, { 9, 18 }, { 12, 12 }, { 3, 0 }, { 9, 0 }, { 6, 0 }, { 6, 18 }, { 3, 15 }, { 0, 15 }, { 3, 18 }, { 9, 18 }, { 12, 15 }, { 12, 11 }, { 2, 5 }, { 0, 0 }, { 12, 0 }, { 0, 16 }, { 3, 18 }, { 9, 18 }, { 12, 15 }, { 12, 11 }, { 9, 9 }, { 3, 9 }, { 9, 9 }, { 12, 7 }, { 12, 3 }, { 9, 0 }, { 3, 0 }, { 0, 2 }, { 9, 0 }, { 9, 18 }, { 0, 6 }, { 12, 6 }, { 0, 2 }, { 3, 0 }, { 9, 0 }, { 12, 2 }, { 12, 8 }, { 9, 10 }, { 3, 10 }, { 0, 9 }, { 2, 18 }, { 12, 18 }, { 0, 7 }, { 3, 10 }, { 9, 10 }, { 12, 7 }, { 12, 3 }, { 9, 0 }, { 3, 0 }, { 0, 3 }, { 0, 10 }, { 3, 15 }, { 7, 18 }, { 0, 18 }, { 12, 18 }, { 4, 0 }, { 3, 10 }, { 0, 13 }, { 0, 16 }, { 3, 19 }, { 9, 19 }, { 12, 16 }, { 12, 13 }, { 9, 10 }, { 3, 10 }, { 0, 7 }, { 0, 3 }, { 3, 0 }, { 9, 0 }, { 12, 3 }, { 12, 7 }, { 9, 10 }, { 5, 0 }, { 9, 3 }, { 12, 8 }, { 12, 15 }, { 9, 18 }, { 3, 18 }, { 0, 15 }, { 0, 11 }, { 3, 8 }, { 9, 8 }, { 12, 11 }, { 6, 4 }, { 6, 4 }, { 6, 14 }, { 6, 14 }, { 5, -4 }, { 7, 0 }, { 7, 0 }, { 7, 10 }, { 7, 10 }, { 12, 0 }, { 0, 9 }, { 12, 18 }, { 0, 4 }, { 12, 4 }, { 0, 14 }, { 12, 14 }, { 0, 0 }, { 12, 9 }, { 0, 18 }, { 0, 15 }, { 3, 18 }, { 9, 18 }, { 12, 15 }, { 12, 11 }, { 6, 7 }, { 6, 4 }, { 6, 0 }, { 6, 0 }, { 12, 2 }, { 10, 0 }, { 3, 0 }, { 0, 3 }, { 0, 15 }, { 3, 18 }, { 9, 18 }, { 12, 15 }, { 12, 6 }, { 5, 6 }, { 5, 13 }, { 12, 13 }, { 0, 0 }, { 6, 18 }, { 12, 0 }, { 3, 9 }, { 9, 9 }, { 0, 0 }, { 0, 18 }, { 9, 18 }, { 12, 15 }, { 12, 12 }, { 9, 9 }, { 0, 9 }, { 9, 9 }, { 12, 6 }, { 12, 3 }, { 9, 0 }, { 0, 0 }, { 12, 3 }, { 9, 0 }, { 3, 0 }, { 0, 3 }, { 0, 15 }, { 3, 18 }, { 9, 18 }, { 12, 15 }, { 0, 0 }, { 0, 18 }, { 9, 18 }, { 12, 15 }, { 12, 3 }, { 9, 0 }, { 0, 0 }, { 0, 0 }, { 0, 18 }, { 12, 18 }, { 0, 9 }, { 9, 9 }, { 0, 0 }, { 12, 0 }, { 0, 0 }, { 0, 18 }, { 12, 18 }, { 0, 9 }, { 9, 9 }, { 12, 15 }, { 9, 18 }, { 3, 18 }, { 0, 15 }, { 0, 3 }, { 3, 0 }, { 9, 0 }, { 12, 3 }, { 12, 8 }, { 5, 8 }, { 0, 0 }, { 0, 18 }, { 12, 0 }, { 12, 18 }, { 0, 9 }, { 12, 9 }, { 2, 0 }, { 10, 0 }, { 6, 0 }, { 6, 18 }, { 2, 18 }, { 10, 18 }, { 0, 2 }, { 3, 0 }, { 5, 0 }, { 8, 2 }, { 8, 18 }, { 4, 18 }, { 12, 18 }, { 0, 0 }, { 0, 18 }, { 12, 18 }, { 0, 6 }, { 3, 9 }, { 12, 0 }, { 0, 0 }, { 0, 18 }, { 0, 0 }, { 12, 0 }, { 0, 0 }, { 0, 18 }, { 6, 5 }, { 12, 18 }, { 12, 0 }, { 0, 0 }, { 0, 18 }, { 12, 0 }, { 12, 18 }, { 3, 0 }, { 0, 3 }, { 0, 15 }, { 3, 18 }, { 9, 18 }, { 12, 15 }, { 12, 3 }, { 9, 0 }, { 3, 0 }, { 0, 0 }, { 0, 18 }, { 9, 18 }, { 12, 15 }, { 12, 11 }, { 9, 8 }, { 0, 8 }, { 3, 0 }, { 0, 3 }, { 0, 15 }, { 3, 18 }, { 9, 18 }, { 12, 15 }, { 12, 3 }, { 9, 0 }, { 3, 0 }, { 7, 5 }, { 14, -2 }, { 0, 0 }, { 0, 18 }, { 9, 18 }, { 12, 15 }, { 12, 11 }, { 9, 8 }, { 0, 8 }, { 7, 8 }, { 12, 0 }, { 0, 2 }, { 3, 0 }, { 9, 0 }, { 12, 3 }, { 12, 6 }, { 9, 9 }, { 3, 9 }, { 0, 12 }, { 0, 15 }, { 3, 18 }, { 9, 18 }, { 12, 16 }, { 6, 0 }, { 6, 18 }, { 0, 18 }, { 12, 18 }, { 0, 18 }, { 0, 3 }, { 3, 0 }, { 9, 0 }, { 12, 3 }, { 12, 18 }, { 0, 18 }, { 6, 0 }, { 12, 18 }, { 0, 18 }, { 3, 0 }, { 6, 14 }, { 9, 0 }, { 12, 18 }, { 0, 0 }, { 12, 18 }, { 0, 18 }, { 12, 0 }, { 6, 0 }, { 6, 7 }, { 0, 18 }, { 6, 7 }, { 12, 18 }, { 0, 0 }, { 12, 18 }, { 0, 18 }, { 12, 0 }, { 0, 0 }, { 12, 20 }, { 6, 20 }, { 6, -2 }, { 12, -2 }, { 0, 18 }, { 12, 0 }, { 0, -2 }, { 6, -2 }, { 6, 20 }, { 0, 20 }, { 0, 7 }, { 6, 16 }, { 12, 7 }, { -18, -5 }, { 0, -5 }, { 5, 18 }, { 5, 18 }, { 7, 14 }, { 0, 10 }, { 5, 12 }, { 11, 10 }, { 11, 2 }, { 8, 0 }, { 4, 0 }, { 0, 2 }, { 0, 5 }, { 11, 6 }, { 11, 2 }, { 13, 0 }, { 0, 0 }, { 0, 18 }, { 0, 9 }, { 6, 11 }, { 12, 9 }, { 12, 2 }, { 6, 0 }, { 0, 2 }, { 11, 9 }, { 6, 11 }, { 0, 9 }, { 0, 2 }, { 6, 0 }, { 11, 2 }, { 12, 2 }, { 6, 0 }, { 0, 2 }, { 0, 9 }, { 6, 11 }, { 12, 9 }, { 12, 18 }, { 12, 0 }, { 0, 6 }, { 12, 7 }, { 9, 12 }, { 3, 12 }, { 0, 9 }, { 0, 2 }, { 3, 0 }, { 9, 0 }, { 12, 2 }, { 4, 0 }, { 4, 16 }, { 8, 18 }, { 12, 16 }, { 0, 9 }, { 8, 9 }, { 11, 2 }, { 6, 0 }, { 0, 2 }, { 0, 9 }, { 6, 11 }, { 11, 9 }, { 11, 11 }, { 11, -5 }, { 6, -7 }, { 0, -5 }, { 0, 0 }, { 0, 18 }, { 0, 9 }, { 6, 11 }, { 12, 9 }, { 12, 0 }, { 7, 0 }, { 7, 11 }, { 4, 11 }, { 7, 18 }, { 7, 18 }, { 0, -5 }, { 4, -7 }, { 8, -5 }, { 8, 11 }, { 8, 18 }, { 8, 18 }, { 0, 0 }, { 0, 18 }, { 0, 5 }, { 12, 11 }, { 4, 7 }, { 12, 0 }, { 3, 0 }, { 9, 0 }, { 6, 0 }, { 6, 18 }, { 3, 18 }, { 0, 0 }, { 0, 12 }, { 0, 9 }, { 4, 12 }, { 6, 9 }, { 6, 0 }, { 6, 9 }, { 10, 12 }, { 12, 9 }, { 12, 0 }, { 0, 0 }, { 0, 11 }, { 0, 8 }, { 6, 11 }, { 12, 8 }, { 12, 0 }, { 6, 0 }, { 0, 2 }, { 0, 9 }, { 6, 11 }, { 12, 9 }, { 12, 2 }, { 6, 0 }, { 0, -7 }, { 0, 11 }, { 0, 9 }, { 6, 11 }, { 12, 9 }, { 12, 2 }, { 6, 0 }, { 0, 2 }, { 11, 2 }, { 6, 0 }, { 0, 2 }, { 0, 9 }, { 6, 11 }, { 11, 9 }, { 11, 11 }, { 11, -6 }, { 13, -8 }, { 0, 0 }, { 0, 11 }, { 0, 8 }, { 6, 11 }, { 12, 8 }, { 0, 2 }, { 6, 0 }, { 12, 2 }, { 12, 5 }, { 0, 7 }, { 0, 10 }, { 6, 12 }, { 12, 10 }, { 12, 2 }, { 8, 0 }, { 4, 2 }, { 4, 18 }, { 0, 11 }, { 8, 11 }, { 0, 11 }, { 0, 2 }, { 6, 0 }, { 12, 2 }, { 12, 11 }, { 0, 11 }, { 6, 0 }, { 12, 11 }, { 0, 11 }, { 3, 0 }, { 6, 8 }, { 9, 0 }, { 12, 11 }, { 0, 0 }, { 11, 11 }, { 0, 11 }, { 11, 0 }, { 0, 11 }, { 7, 1 }, { 3, -7 }, { 12, 11 }, { 0, 11 }, { 12, 11 }, { 0, 0 }, { 12, 0 }, { 12, -2 }, { 7, 1 }, { 7, 6 }, { 4, 9 }, { 7, 12 }, { 7, 17 }, { 12, 20 }, { 6, 0 }, { 6, 6 }, { 6, 12 }, { 6, 18 }, { 0, -2 }, { 5, 1 }, { 5, 6 }, { 8, 9 }, { 5, 12 }, { 5, 17 }, { 0, 20 }, { 0, 0 }, { 0, 53 }, { 53, 53 }, { 53, 0 }, { 0, 0 }, { 0, 0 }, { 0, 18 }, { 12, 9 }, { 0, 0 }, { 0, 3 }, { 4, 3 }, { 4, 15 }, { 0, 15 }, { 0, 6 }, { 8, 6 }, { 8, 12 }, { 0, 12 }, { 0, 9 }, { 12, 9 }, { 0, 0 }, { -48, 0 }, { 0, -11 }, { -48, -11 }, { 0, -22 }, { -48, -22 }, { 0, -33 }, { -48, -33 }, { 0, -44 }, { -48, -44 }, { 0, -55 }, { -48, -55 }, { 0, 0 }, { 0, 9 }, { 0, 0 }, { 0, -9 }, { 0, 0 }, { -9, 0 }, { 0, 0 }, { 9, 0 }, { -9, 0 }, { 9, 0 }, { 0, 9 }, { 0, -9 }, { 0, 0 }, { 0, 39 }, { 53, 39 }, { 53, 0 }, { 0, 0 }, { 0, 0 }, { 0, 39 }, { 0, 40 }, { 0, 80 }, { 58, 80 }, { 58, 41 }, { 58, 40 }, { 58, 0 }, { 0, 0 }, { 5, 7 }, { 5, 40 }, { 5, 73 }, { 53, 73 }, { 53, 40 }, { 53, 7 }, { 5, 7 }, { 5, 40 }, { 53, 40 }, { 29, 73 }, { 29, 40 }, { 29, 7 }, { 31, 40 }, { 31, 7 }, { 33, 40 }, { 33, 7 }, { 35, 40 }, { 35, 7 }, { 37, 40 }, { 37, 7 }, { 39, 40 }, { 39, 7 }, { 41, 40 }, { 41, 7 }, { 43, 40 }, { 43, 7 }, { 45, 40 }, { 45, 7 }, { 47, 40 }, { 47, 7 }, { 49, 40 }, { 49, 7 }, { 51, 40 }, { 51, 7 }, { 0, -7 }, { 6, 0 }, { 0, -14 }, { 12, 0 }, { 0, -21 }, { 18, 0 }, { 0, -28 }, { 24, 0 }, { 0, -35 }, { 30, 0 }, { 0, -42 }, { 36, 0 }, { 0, -49 }, { 42, 0 }, { 0, -56 }, { 48, 0 }, { 0, -63 }, { 48, -7 }, { 3, -66 }, { 48, -14 }, { 9, -66 }, { 48, -21 }, { 15, -66 }, { 48, -28 }, { 21, -66 }, { 48, -35 }, { 27, -66 }, { 48, -42 }, { 33, -66 }, { 48, -49 }, { 39, -66 }, { 48, -56 }, { 45, -66 }, { 48, -63 }, { 54, 0 }, { 48, 7 }, { 60, 0 }, { 48, 14 }, { 66, 0 }, { 48, 21 }, { 72, 0 }, { 48, 28 }, { 78, 0 }, { 48, 35 }, { 84, 0 }, { 48, 42 }, { 90, 0 }, { 48, 49 }, { 96, 0 }, { 48, 56 }, { 96, 7 }, { 48, 63 }, { 96, 14 }, { 51, 66 }, { 96, 21 }, { 57, 66 }, { 96, 28 }, { 63, 66 }, { 96, 35 }, { 69, 66 }, { 96, 42 }, { 75, 66 }, { 96, 49 }, { 81, 66 }, { 96, 56 }, { 87, 66 }, { 96, 63 }, { 93, 66 }, { 48, 55 }, { 0, 55 }, { 48, 44 }, { 0, 44 }, { 48, 33 }, { 0, 33 }, { 48, 22 }, { 0, 22 }, { 48, 11 }, { 0, 11 }, { 6, 13 }, { 6, 18 }, { -17, 15 }, { -8, 18 }, { -17, 19 }, { -7, 15 }, { -17, 15 }, { -12, 18 }, { -7, 15 }, { -15, 17 }, { -15, 17 }, { -9, 17 }, { -9, 17 }, { -18, 23 }, { -15, 26 }, { -9, 23 }, { -6, 26 }, { 11, 9 }, { 5, 11 }, { 0, 9 }, { 0, 2 }, { 5, 0 }, { 11, 2 }, { 5, 0 }, { 5, -4 }, { 8, -7 }, { 5, -10 }, { 12, 0 }, { 12, 13 }, { 12, 18 }, { 12, 18 }, { 12, 3 }, { 6, 0 }, { 0, 3 }, { 0, 7 }, { 6, 10 }, { 6, 14 }, { 6, 18 }, { 6, 18 }, { 3, 9 }, { 9, 9 }, { 12, 17 }, { 8, 18 }, { 6, 15 }, { 6, 2 }, { 3, 0 }, { 0, 0 }, { 0, 2 }, { 3, 2 }, { 6, 0 }, { 9, 0 }, { 12, 2 }, { 0, 0 }, { 0, 58 }, { 58, 58 }, { 58, 0 }, { 0, 0 }, { 5, 5 }, { 5, 53 }, { 53, 53 }, { 53, 5 }, { 5, 5 }, { 5, 29 }, { 53, 29 }, { 29, 53 }, { 29, 5 }, { 31, 29 }, { 31, 5 }, { 33, 29 }, { 33, 5 }, { 35, 29 }, { 35, 5 }, { 37, 29 }, { 37, 5 }, { 39, 29 }, { 39, 5 }, { 41, 29 }, { 41, 5 }, { 43, 29 }, { 43, 5 }, { 45, 29 }, { 45, 5 }, { 47, 29 }, { 47, 5 }, { 49, 29 }, { 49, 5 }, { 51, 29 }, { 51, 5 }, { 0, 3 }, { 0, 15 }, { 6, 18 }, { 12, 15 }, { 12, 3 }, { 6, 0 }, { 0, 3 }, { 12, 15 }, { 0, 0 }, { 4, 18 }, { 7, 18 }, { 7, 0 }, { 12, 0 }, { 12, 9 }, { 2, 9 }, { 7, 18 }, { 12, 18 }, { 0, 11 }, { 6, 11 }, { 6, 0 }, { 0, 0 }, { 2, 5 }, { 13, 5 }, { 13, 8 }, { 10, 11 }, { 7, 8 }, { 7, 2 }, { 9, 0 }, { 13, 0 }, { 0, -5 }, { 18, -5 }, { 0, -6 }, { 5, 0 }, { 0, -12 }, { 10, 0 }, { 0, -18 }, { 15, 0 }, { 0, -24 }, { 20, 0 }, { 0, -30 }, { 25, 0 }, { 0, -36 }, { 30, 0 }, { 0, -42 }, { 35, 0 }, { 0, -48 }, { 40, 0 }, { 5, -48 }, { 45, 0 }, { 10, -48 }, { 48, -3 }, { 15, -48 }, { 48, -9 }, { 20, -48 }, { 48, -15 }, { 25, -48 }, { 48, -21 }, { 30, -48 }, { 48, -27 }, { 35, -48 }, { 48, -33 }, { 40, -48 }, { 48, -39 }, { 45, -48 }, { 48, -45 }, { 53, 0 }, { 48, 6 }, { 58, 0 }, { 48, 12 }, { 63, 0 }, { 48, 18 }, { 68, 0 }, { 48, 24 }, { 73, 0 }, { 48, 30 }, { 78, 0 }, { 48, 36 }, { 83, 0 }, { 48, 42 }, { 88, 0 }, { 48, 48 }, { 93, 0 }, { 53, 48 }, { 96, 3 }, { 58, 48 }, { 96, 9 }, { 63, 48 }, { 96, 15 }, { 68, 48 }, { 96, 21 }, { 73, 48 }, { 96, 27 }, { 78, 48 }, { 96, 33 }, { 83, 48 }, { 96, 39 }, { 88, 48 }, { 96, 45 }, { 93, 48 }, { 48, 44 }, { 0, 44 }, { 48, 40 }, { 0, 40 }, { 48, 36 }, { 0, 36 }, { 48, 32 }, { 0, 32 }, { 48, 28 }, { 0, 28 }, { 48, 24 }, { 0, 24 }, { 48, 20 }, { 0, 20 }, { 48, 16 }, { 0, 16 }, { 48, 12 }, { 0, 12 }, { 48, 8 }, { 0, 8 }, { 48, 4 }, { 0, 4 }, { 0, 0 }, { 12, 10 }, { 12, 8 }, { 9, 10 }, { 3, 10 }, { 0, 8 }, { 0, 2 }, { 3, 0 }, { 9, 0 }, { 12, 2 }, { 12, 8 }, { -15, 23 }, { -15, 23 }, { -9, 23 }, { -9, 23 }, { -12, 23 }, { -12, 23 }, { -18, 16 }, { -15, 18 }, { -9, 16 }, { -6, 18 }, { -12, 17 }, { -12, 17 }, { -36, 23 }, { -30, 26 }, { -24, 26 }, { -18, 23 }, { -12, 23 }, { -6, 26 }, { 0, 0 }, { 0, 18 }, { 0, 9 }, { 12, 9 }, { -36, 16 }, { -30, 18 }, { -24, 18 }, { -18, 16 }, { -12, 16 }, { -6, 18 }, { 0, 16 }, { 3, 18 }, { 9, 16 }, { 12, 18 } };

static struct {
	int16_t start, num;
	int16_t min_x, max_x, min_y, max_y;
} vecfont_meta[] = { { 0, 0, 0, 0, 0, 0 }, { 0, 25, 0, 45, 0, 27 }, { 25, 14, 0, 12, -7, 18 }, { 0, 0, 0, 0, 0, 0 }, { 39, 2, 0, 0, 0, 4 }, { 41, 2, 0, 0, -4, 0 }, { 43, 2, -4, 0, 0, 0 }, { 45, 2, 0, 4, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 47, 2, -4, 4, 0, 0 }, { 49, 2, 0, 0, -4, 4 }, { 51, 8, -5, 5, -5, 5 }, { 59, 9, -5, 5, -5, 5 }, { 68, 5, 0, 12, 0, 18 }, { 73, 5, 0, 12, 3, 15 }, { 78, 5, 0, 12, 0, 18 }, { 83, 5, 0, 12, 3, 15 }, { 88, 4, 0, 13, 0, 20 }, { 92, 8, 0, 12, 0, 14 }, { 100, 4, 0, 12, 0, 15 }, { 104, 8, 0, 13, -7, 11 }, { 112, 9, 4, 11, 16, 23 }, { 121, 10, 0, 13, 0, 16 }, { 131, 9, 0, 13, -7, 12 }, { 140, 7, 0, 12, 0, 18 }, { 147, 13, 0, 12, 0, 18 }, { 160, 6, 0, 12, 0, 18 }, { 0, 0, 0, 12, 0, 18 }, { 166, 4, 6, 6, 0, 18 }, { 170, 4, 3, 8, 14, 18 }, { 174, 8, 0, 12, 0, 18 }, { 182, 14, 0, 12, -1, 19 }, { 196, 12, 0, 12, 0, 18 }, { 208, 9, 0, 12, 0, 18 }, { 217, 3, 5, 7, 14, 18 }, { 220, 4, 6, 12, -2, 20 }, { 224, 4, 0, 6, -2, 20 }, { 228, 6, 0, 12, 2, 16 }, { 234, 4, 0, 12, 2, 16 }, { 238, 3, 4, 6, -4, 1 }, { 241, 2, 0, 12, 9, 9 }, { 243, 3, 6, 6, 0, 0 }, { 246, 2, 0, 12, 0, 18 }, { 248, 11, 0, 12, 0, 18 }, { 259, 5, 3, 9, 0, 18 }, { 264, 8, 0, 12, 0, 18 }, { 272, 13, 0, 12, 0, 18 }, { 285, 4, 0, 12, 0, 18 }, { 289, 10, 0, 12, 0, 18 }, { 299, 11, 0, 12, 0, 18 }, { 310, 3, 0, 12, 0, 18 }, { 313, 16, 0, 12, 0, 19 }, { 329, 11, 0, 12, 0, 18 }, { 340, 4, 6, 6, 4, 14 }, { 344, 5, 5, 7, -4, 10 }, { 349, 3, 0, 12, 0, 18 }, { 352, 4, 0, 12, 4, 14 }, { 356, 3, 0, 12, 0, 18 }, { 359, 9, 0, 12, 0, 18 }, { 368, 12, 0, 12, 0, 18 }, { 380, 5, 0, 12, 0, 18 }, { 385, 12, 0, 12, 0, 18 }, { 397, 8, 0, 12, 0, 18 }, { 405, 7, 0, 12, 0, 18 }, { 412, 7, 0, 12, 0, 18 }, { 419, 5, 0, 12, 0, 18 }, { 424, 10, 0, 12, 0, 18 }, { 434, 6, 0, 12, 0, 18 }, { 440, 6, 2, 10, 0, 18 }, { 446, 7, 0, 12, 0, 18 }, { 453, 6, 0, 12, 0, 18 }, { 459, 4, 0, 12, 0, 18 }, { 463, 5, 0, 12, 0, 18 }, { 468, 4, 0, 12, 0, 18 }, { 472, 9, 0, 12, 0, 18 }, { 481, 7, 0, 12, 0, 18 }, { 488, 11, 0, 14, -2, 18 }, { 499, 9, 0, 12, 0, 18 }, { 508, 12, 0, 12, 0, 18 }, { 520, 4, 0, 12, 0, 18 }, { 524, 6, 0, 12, 0, 18 }, { 530, 3, 0, 12, 0, 18 }, { 533, 5, 0, 12, 0, 18 }, { 538, 4, 0, 12, 0, 18 }, { 542, 5, 0, 12, 0, 18 }, { 547, 5, 0, 12, 0, 18 }, { 552, 4, 6, 12, -2, 20 }, { 556, 2, 0, 12, 0, 18 }, { 558, 4, 0, 6, -2, 20 }, { 562, 3, 0, 12, 7, 16 }, { 565, 2, -18, 0, -5, -5 }, { 567, 3, 5, 7, 14, 18 }, { 570, 11, 0, 13, 0, 12 }, { 581, 8, 0, 12, 0, 18 }, { 589, 6, 0, 11, 0, 11 }, { 595, 8, 0, 12, 0, 18 }, { 603, 9, 0, 12, 0, 12 }, { 612, 6, 0, 12, 0, 18 }, { 618, 10, 0, 11, -7, 11 }, { 628, 6, 0, 12, 0, 18 }, { 634, 5, 4, 7, 0, 18 }, { 639, 6, 0, 8, -7, 18 }, { 645, 6, 0, 12, 0, 18 }, { 651, 5, 3, 9, 0, 18 }, { 656, 10, 0, 12, 0, 12 }, { 666, 6, 0, 12, 0, 11 }, { 672, 7, 0, 12, 0, 11 }, { 679, 8, 0, 12, -7, 11 }, { 687, 9, 0, 13, -8, 11 }, { 696, 5, 0, 12, 0, 11 }, { 701, 8, 0, 12, 0, 12 }, { 709, 6, 0, 12, 0, 18 }, { 715, 5, 0, 12, 0, 11 }, { 720, 3, 0, 12, 0, 11 }, { 723, 5, 0, 12, 0, 11 }, { 728, 4, 0, 11, 0, 11 }, { 732, 4, 0, 12, -7, 11 }, { 736, 4, 0, 12, 0, 11 }, { 740, 7, 4, 12, -2, 20 }, { 747, 4, 6, 6, 0, 18 }, { 751, 7, 0, 8, -2, 20 }, { 758, 5, 0, 53, 0, 53 }, { 763, 14, 0, 12, 0, 18 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 777, 12, -48, 0, -55, 0 }, { 0, 0, 0, 0, 0, 0 }, { 789, 2, 0, 0, 0, 9 }, { 791, 2, 0, 0, -9, 0 }, { 793, 2, -9, 0, 0, 0 }, { 795, 2, 0, 9, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 797, 2, -9, 9, 0, 0 }, { 799, 2, 0, 0, -9, 9 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 801, 5, 0, 53, 0, 39 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 806, 43, 0, 58, 0, 80 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 849, 78, 0, 96, -66, 66 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 927, 2, 6, 6, 13, 18 }, { 929, 2, -17, -8, 15, 18 }, { 931, 2, -17, -7, 15, 19 }, { 933, 3, -17, -7, 15, 18 }, { 936, 4, -15, -9, 17, 17 }, { 940, 4, -18, -6, 23, 26 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 944, 10, 0, 11, -10, 11 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 954, 4, 12, 12, 0, 18 }, { 958, 8, 0, 12, 0, 18 }, { 0, 0, 0, 0, 0, 0 }, { 966, 13, 0, 12, 0, 18 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 979, 36, 0, 58, 0, 58 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 1015, 8, 0, 12, 0, 18 }, { 1023, 9, 0, 12, 0, 18 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 1032, 12, 0, 13, 0, 11 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 1044, 2, 0, 18, -5, -5 }, { 0, 0, 0, 0, 0, 0 }, { 1046, 90, 0, 96, -48, 48 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 1136, 11, 0, 12, 0, 10 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 }, { 1147, 4, -15, -9, 23, 23 }, { 0, 0, 0, 0, 0, 0 }, { 1151, 2, -12, -12, 23, 23 }, { 0, 0, 0, 0, 0, 0 }, { 1153, 4, -18, -6, 16, 18 }, { 0, 0, 0, 0, 0, 0 }, { 1157, 2, -12, -12, 17, 17 }, { 0, 0, 0, 0, 0, 0 }, { 1159, 6, -36, -6, 23, 26 }, { 1165, 4, 0, 12, 0, 18 }, { 1169, 6, -36, -6, 16, 18 }, { 1175, 4, 0, 12, 16, 18 }, { 0, 0, 0, 0, 0, 0 } };

bool GdHistoryPlot::_prep_vec_glyph(PoolVector2Array &p_data, uint8_t p_idx, const Point2 &p_pos, const Size2 &p_scale) {
	const int start = vecfont_meta[p_idx].start;
	const int num = vecfont_meta[p_idx].num;
	if (num == 0) {
		return false;
	}
	const int offset_x = vecfont_meta[p_idx].min_x;
	Point2 orig;
	for (int j = start; j < start + num; j++) {
		const Point2 xy = { p_pos.x + (vecfont_points[j].x - offset_x) * p_scale.x, p_pos.y - vecfont_points[j].y * p_scale.y };
		if (vecfont_ops[j]) {
			if (orig == xy) {
				// point
				p_data.push_back(xy + Vector2(-p_scale.x, 0));
				p_data.push_back(xy + Vector2(0, p_scale.y));
				p_data.push_back(xy + Vector2(0, p_scale.y));
				p_data.push_back(xy + Vector2(p_scale.x, 0));
				p_data.push_back(xy + Vector2(p_scale.x, 0));
				p_data.push_back(xy + Vector2(0, -p_scale.y));
				p_data.push_back(xy + Vector2(0, -p_scale.y));
				p_data.push_back(xy + Vector2(-p_scale.x, 0));
			} else {
				p_data.push_back(orig);
				p_data.push_back(xy);
			}
		};
		orig = xy;
	}
	return true;
}

void GdHistoryPlot::_draw_vec_text(const String &p_text, const Point2 &p_pos, const Color &p_color, const Size2 &p_scale) {
	if (p_text.empty()) {
		return;
	}
	PoolVector2Array data;
	Point2 pos = p_pos;
	for (int i = 0; i < p_text.length(); i++) {
		const CharType code = p_text[i];
		if (_prep_vec_glyph(data, code, pos, p_scale) || code == ' ') {
			const real_t adv = MAX(2, vecfont_meta[code].max_x - vecfont_meta[code].min_x) * p_scale.x;
			pos += Point2(adv + 2, 0);
		}
	}
	Array mesh_array;
	mesh_array.resize(VS::ARRAY_MAX);
	mesh_array[VS::ARRAY_VERTEX] = data;
	PoolColorArray colors;
	colors.resize(data.size());
	colors.fill(p_color);
	mesh_array[VS::ARRAY_COLOR] = colors;
	text_mesh.m->add_surface_from_arrays(Mesh::PRIMITIVE_LINES, mesh_array);
}

void GdHistoryPlot::_draw_vec_text(const String &p_text, const Point2 &p_pos, const Size2 &p_scale) {
	if (p_text.empty()) {
		return;
	}
	PoolVector2Array data;
	Point2 pos = p_pos;
	for (int i = 0; i < p_text.length(); i++) {
		const CharType code = p_text[i];
		if (_prep_vec_glyph(data, code, pos, p_scale) || code == ' ') {
			const real_t adv = MAX(2, vecfont_meta[code].max_x - vecfont_meta[code].min_x) * p_scale.x;
			pos += Point2(adv + 2, 0);
		}
	}
	Array mesh_array;
	mesh_array.resize(VS::ARRAY_MAX);
	mesh_array[VS::ARRAY_VERTEX] = data;
	text_mesh.m->add_surface_from_arrays(Mesh::PRIMITIVE_LINES, mesh_array);
}

Size2 GdHistoryPlot::_size_vec_text(const String &p_text, const Size2 &p_scale) {
	if (p_text.empty()) {
		return Size2();
	}
	real_t text_width = 0, min_y = 999, max_y = -999;
	for (int i = 0; i < p_text.length(); i++) {
		const CharType code = p_text[i];
		if (vecfont_meta[code].num == 0 && code != ' ') {
			continue;
		}
		text_width += 2 + MAX(2, vecfont_meta[code].max_x - vecfont_meta[code].min_x) * p_scale.x;
		if (vecfont_meta[code].min_y < min_y) {
			min_y = vecfont_meta[code].min_y;
		}
		if (vecfont_meta[code].max_y > max_y) {
			max_y = vecfont_meta[code].max_y;
		}
	}
	return Size2(text_width, p_scale.y * (max_y - min_y));
}
