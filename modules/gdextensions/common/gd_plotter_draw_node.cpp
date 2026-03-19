/**************************************************************************/
/*  gd_plotter_draw_node.cpp                                              */
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

#include "gd_plotter_draw_node.h"

#include "gd_plotter_draw.h"
#include "scene/resources/dynamic_font.h"

#include <float.h>

static real_t _deque_values_getter(void *data, int idx) {
	std::deque<real_t> *vals = (std::deque<real_t> *)data;
	if (idx >= 0 && idx < (int)vals->size()) {
		return (*vals)[idx];
	}
	return 0;
}

struct FlameGetterData {
	const Vector<GdPlotterDraw::FlameEntry> *entries;
};

static void _flame_series_getter(real_t *start, real_t *end, uint8_t *level, const String &caption_out, const void *data, int idx) {
	const FlameGetterData *fdata = (const FlameGetterData *)data;
	const GdPlotterDraw::FlameEntry &entry = fdata->entries->get(idx);
	if (start) {
		*start = entry.start;
	}
	if (end) {
		*end = entry.end;
	}
	if (level) {
		*level = entry.level;
	}
	// caption_out is const ref so we can't assign to it from here;
	// the flame graph will use empty captions for the getter-based path.
}

void GdPlotterDraw::set_mode(PlotMode p_mode) {
	mode = p_mode;
	dirty = true;
	update();
}

GdPlotterDraw::PlotMode GdPlotterDraw::get_mode() const {
	return mode;
}

void GdPlotterDraw::set_label(const String &p_label) {
	label = p_label;
	dirty = true;
	update();
}

String GdPlotterDraw::get_label() const {
	return label;
}

void GdPlotterDraw::set_overlay_text(const String &p_text) {
	overlay_text = p_text;
	dirty = true;
	update();
}

String GdPlotterDraw::get_overlay_text() const {
	return overlay_text;
}

void GdPlotterDraw::set_scale_min(real_t p_min) {
	scale_min = p_min;
	dirty = true;
	update();
}

real_t GdPlotterDraw::get_scale_min() const {
	return scale_min;
}

void GdPlotterDraw::set_scale_max(real_t p_max) {
	scale_max = p_max;
	dirty = true;
	update();
}

real_t GdPlotterDraw::get_scale_max() const {
	return scale_max;
}

void GdPlotterDraw::set_max_history(int p_max) {
	max_history = p_max;
	update();
}

int GdPlotterDraw::get_max_history() const {
	return max_history;
}

void GdPlotterDraw::set_background_color(const Color &p_color) {
	bg_color = p_color;
	update();
}

Color GdPlotterDraw::get_background_color() const {
	return bg_color;
}

void GdPlotterDraw::set_draw_background(bool p_draw) {
	draw_background = p_draw;
	update();
}

bool GdPlotterDraw::get_draw_background() const {
	return draw_background;
}

void GdPlotterDraw::add_sample(real_t p_value) {
	values.push_back(p_value);
	while ((int)values.size() > max_history) {
		values.pop_front();
	}
	dirty = true;
	update();
}

void GdPlotterDraw::add_flame_entry(real_t p_start, real_t p_end, int p_level, const String &p_caption) {
	FlameEntry entry;
	entry.start = p_start;
	entry.end = p_end;
	entry.level = (uint8_t)CLAMP(p_level, 0, 255);
	entry.caption = p_caption;
	flame_entries.push_back(entry);
	dirty = true;
	update();
}

void GdPlotterDraw::clear_flame_entries() {
	flame_entries.clear();
	dirty = true;
	update();
}

void GdPlotterDraw::reset() {
	values.clear();
	flame_entries.clear();
	dirty = true;
	update();
}

void GdPlotterDraw::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			const Rect2 rc = Rect2(Point2(), get_size());

			if (draw_background) {
				draw_rect(rc, bg_color);
			}

			if (!font.is_valid()) {
				font = get_font("font", "Label");
			}
			if (!font.is_valid()) {
				return;
			}

			Ref<Font> f = font;

			// Draw compact label inside the graph area with separator
			Rect2 plot_rc = rc;
			if (!label.empty()) {
				const real_t font_scale = 0.55;
				const real_t label_h = f->get_height() * font_scale + 4;
				const Color label_color = Color(0.7, 0.7, 0.7);
				const Color sep_color = Color(0.4, 0.4, 0.4, 0.5);

				draw_set_transform(Point2(4, 2), 0, Size2(font_scale, font_scale));
				draw_string(f, Point2(0, f->get_ascent()), label, label_color);
				draw_set_transform(Point2(), 0, Size2(1, 1));
				draw_line(Point2(0, label_h), Point2(rc.size.width, label_h), sep_color);

				plot_rc = Rect2(Point2(0, label_h + 1), Size2(rc.size.width, rc.size.height - label_h - 1));
			}

			// Pass empty label to plot functions since we drew it ourselves
			const String empty_label;

			switch (mode) {
				case MODE_LINES: {
					if (values.size() >= 2) {
						plot_lines(this, f, empty_label, &_deque_values_getter, (void *)&values, (int)values.size(), 0, overlay_text, scale_min, scale_max, plot_rc);
					}
				} break;
				case MODE_HISTOGRAM: {
					if (values.size() >= 1) {
						plot_histogram(this, f, empty_label, &_deque_values_getter, (void *)&values, (int)values.size(), 0, overlay_text, scale_min, scale_max, plot_rc);
					}
				} break;
				case MODE_FLAME: {
					if (flame_entries.size() >= 1) {
						FlameGetterData fdata;
						fdata.entries = &flame_entries;
						plot_flame(this, f, empty_label, &_flame_series_getter, (void *)&fdata, flame_entries.size(), 0, overlay_text, scale_min, scale_max, plot_rc);
					}
				} break;
			}
			dirty = false;
		} break;
	}
}

void GdPlotterDraw::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_mode", "mode"), &GdPlotterDraw::set_mode);
	ClassDB::bind_method(D_METHOD("get_mode"), &GdPlotterDraw::get_mode);
	ClassDB::bind_method(D_METHOD("set_label", "label"), &GdPlotterDraw::set_label);
	ClassDB::bind_method(D_METHOD("get_label"), &GdPlotterDraw::get_label);
	ClassDB::bind_method(D_METHOD("set_overlay_text", "text"), &GdPlotterDraw::set_overlay_text);
	ClassDB::bind_method(D_METHOD("get_overlay_text"), &GdPlotterDraw::get_overlay_text);
	ClassDB::bind_method(D_METHOD("set_scale_min", "min"), &GdPlotterDraw::set_scale_min);
	ClassDB::bind_method(D_METHOD("get_scale_min"), &GdPlotterDraw::get_scale_min);
	ClassDB::bind_method(D_METHOD("set_scale_max", "max"), &GdPlotterDraw::set_scale_max);
	ClassDB::bind_method(D_METHOD("get_scale_max"), &GdPlotterDraw::get_scale_max);
	ClassDB::bind_method(D_METHOD("set_max_history", "max"), &GdPlotterDraw::set_max_history);
	ClassDB::bind_method(D_METHOD("get_max_history"), &GdPlotterDraw::get_max_history);
	ClassDB::bind_method(D_METHOD("set_background_color", "color"), &GdPlotterDraw::set_background_color);
	ClassDB::bind_method(D_METHOD("get_background_color"), &GdPlotterDraw::get_background_color);
	ClassDB::bind_method(D_METHOD("set_draw_background", "draw"), &GdPlotterDraw::set_draw_background);
	ClassDB::bind_method(D_METHOD("get_draw_background"), &GdPlotterDraw::get_draw_background);

	ClassDB::bind_method(D_METHOD("add_sample", "value"), &GdPlotterDraw::add_sample);
	ClassDB::bind_method(D_METHOD("add_flame_entry", "start", "end", "level", "caption"), &GdPlotterDraw::add_flame_entry);
	ClassDB::bind_method(D_METHOD("clear_flame_entries"), &GdPlotterDraw::clear_flame_entries);
	ClassDB::bind_method(D_METHOD("reset"), &GdPlotterDraw::reset);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "mode", PROPERTY_HINT_ENUM, "Lines,Histogram,Flame"), "set_mode", "get_mode");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "label"), "set_label", "get_label");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "overlay_text"), "set_overlay_text", "get_overlay_text");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "scale_min"), "set_scale_min", "get_scale_min");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "scale_max"), "set_scale_max", "get_scale_max");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_history"), "set_max_history", "get_max_history");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "background_color"), "set_background_color", "get_background_color");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "draw_background"), "set_draw_background", "get_draw_background");

	BIND_ENUM_CONSTANT(MODE_LINES);
	BIND_ENUM_CONSTANT(MODE_HISTOGRAM);
	BIND_ENUM_CONSTANT(MODE_FLAME);
}

GdPlotterDraw::GdPlotterDraw() {
	mode = MODE_LINES;
	scale_min = FLT_MAX;
	scale_max = FLT_MAX;
	max_history = 200;
	bg_color = Color(0.15, 0.15, 0.15);
	draw_background = true;
	dirty = true;
	set_size(Size2(250, 120));
}
