/**************************************************************************/
/*  gd_plotter_draw.cpp                                                   */
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

#include "gd_plotter_draw.h"

#include "common/gd_core.h"
#include "gd_core.h"

// Reference:
// ----------
// 1. https://github.com/bwrsandman/imgui-flame-graph

enum PlotType {
	PlotType_Lines,
	PlotType_Histogram,
};

const Color GuiColor_PlotLines = Color(0.61, 0.61, 0.61, 1.00);
const Color GuiColor_PlotLinesHovered = Color(1.00, 0.43, 0.35, 1.00);
const Color GuiColor_PlotHistogram = Color(0.90, 0.70, 0.00, 1.00);
const Color GuiColor_PlotHistogramHovered = Color(1.00, 0.60, 0.00, 1.00);

const int GuiStyle_FramePadding = 2;
const int GuiStyle_ItemInnerSpacing = 2;

static _ALWAYS_INLINE_ Vector2 LERP(const Vector2 &a, const Vector2 &b, real_t t) { return Vector2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t); }
static _ALWAYS_INLINE_ Vector2 LERP(const Vector2 &a, const Vector2 &b, const Vector2 &t) { return Vector2(a.x + (b.x - a.x) * t.x, a.y + (b.y - a.y) * t.y); }
static _ALWAYS_INLINE_ real_t SATURATE(real_t f) { return (f < 0 ? 0 : (f > 1 ? 1 : f)); }

static void PlotFlame(CanvasItem *canvas, Ref<Font> &text_font, const String &label, void (*values_getter)(real_t *start, real_t *end, uint8_t *level, const String &caption, const void *data, int idx), const void *data, int values_count, int values_offset, const String &overlay_text, real_t scale_min, real_t scale_max, const Rect2 &frame_rect, const Point2 *tooltip_pos = nullptr) {
	ERR_FAIL_NULL(canvas);
	ERR_FAIL_NULL(text_font);

	// Find the maximum depth
	uint8_t max_depth = 0;
	for (int i = values_offset; i < values_count; ++i) {
		uint8_t depth;
		values_getter(nullptr, nullptr, &depth, String(), data, i);
		max_depth = MAX(max_depth, depth);
	}

	const real_t block_height = text_font->get_height() + (GuiStyle_FramePadding * 2);
	const String label_text = string_ellipsis(text_font, label, frame_rect.size.width);
	const Size2 label_size = text_font->get_string_size(label_text);

	const Rect2 frame_bb = Rect2(frame_rect.position, frame_rect.size - Size2(0, label_size.y)); // label on the bottom
	const Rect2 inner_bb(frame_bb.shrink(GuiStyle_FramePadding));
	const Rect2 total_bb = frame_rect;

	// determine scale from values if not specified
	if (scale_min == FLT_MAX || scale_max == FLT_MAX) {
		real_t v_min = FLT_MAX;
		real_t v_max = -FLT_MAX;
		for (int i = values_offset; i < values_count; i++) {
			real_t v_start, v_end;
			values_getter(&v_start, &v_end, nullptr, String(), data, i);
			if (v_start == v_start) { // check non-NaN values
				v_min = MIN(v_min, v_start);
			}
			if (v_end == v_end) { // check non-NaN values
				v_max = MAX(v_max, v_end);
			}
		}
		if (scale_min == FLT_MAX) {
			scale_min = v_min;
		}
		if (scale_max == FLT_MAX) {
			scale_max = v_max;
		}
	}

	bool any_hovered = false;
	if (values_count - values_offset >= 1) {
		const Color col_base = GuiColor_PlotHistogram.lightened(0.75);
		const Color col_hovered = GuiColor_PlotHistogramHovered.lightened(0.75);
		const Color col_outline_base = GuiColor_PlotHistogram.lightened(0.75);
		const Color col_outline_hovered = GuiColor_PlotHistogramHovered.lightened(0.75);

		String tooltip;

		for (int i = values_offset; i < values_count; ++i) {
			real_t stage_start, stage_end;
			uint8_t depth;
			String caption;

			values_getter(&stage_start, &stage_end, &depth, caption, data, i);

			const real_t duration = scale_max - scale_min;
			if (duration == 0) {
				return;
			}

			const real_t start = stage_start - scale_min;
			const real_t end = stage_end - scale_min;

			const real_t start_x = start / duration;
			const real_t end_x = end / duration;

			const real_t width = inner_bb.size.width;
			const real_t height = block_height * (max_depth - depth + 1) - GuiStyle_FramePadding;

			const Point2 pos0 = inner_bb.position + Vector2(start_x * width, height);
			const Point2 pos1 = inner_bb.position + Vector2(end_x * width, height + block_height);
			const Rect2 rc = Rect2(pos0, pos1 - pos0);

			bool v_hovered = false;
			if (tooltip_pos) {
				if (rc.has_point(*tooltip_pos)) {
					tooltip += string_format("%s: %8.4g", caption.c_str(), stage_end - stage_start);
					v_hovered = true;
					any_hovered = v_hovered;
				}
			}

			canvas->draw_rect(rc, v_hovered ? col_hovered : col_base);
			canvas->draw_rect(rc, v_hovered ? col_outline_hovered : col_outline_base, false);
			const Size2 text_size = text_font->get_string_size(caption);
			const Size2 box_size = rc.size;
			Vector2 text_offset;
			if (text_size.x < box_size.x) {
				text_offset = Vector2(0.5, 0.5) * (box_size - text_size);
				canvas->draw_string(text_font, pos0 + text_offset, caption);
			}
		}

		if (!overlay_text.empty()) {
			const String text = string_ellipsis(text_font, overlay_text, frame_bb.size.width);
			const Size2 size = text_font->get_string_size(text);
			canvas->draw_string(text_font, frame_bb.position - Vector2(size.x / 2, -GuiStyle_FramePadding), overlay_text); // center
		}

		if (label_size.x > 0) {
			canvas->draw_string(text_font, Vector2(frame_bb.position.x + GuiStyle_ItemInnerSpacing, inner_bb.position.y), label_text);
		}

		if (!tooltip.empty()) {
			canvas->draw_string(text_font, *tooltip_pos, tooltip);
		}
	}

	if (tooltip_pos) {
		if (!any_hovered && frame_rect.has_point(*tooltip_pos)) {
			const String tooltip = string_format("Total: %8.4g", scale_max - scale_min);
			canvas->draw_string(text_font, *tooltip_pos, tooltip);
		}
	}
}

static void PlotGraph(CanvasItem *canvas, Ref<Font> &text_font, PlotType plot_type, const String &label, ValuesGetter values_getter, void *data, int values_count, int values_offset, const String &overlay_text, real_t scale_min, real_t scale_max, const Rect2 &frame_rect, const Point2 *tooltip_pos = nullptr) {
	const String label_text = string_ellipsis(text_font, label, frame_rect.size.width);
	const Size2 label_size = text_font->get_string_size(label_text);

	const Rect2 frame_bb = Rect2(frame_rect.position, frame_rect.size - Size2(0, label_size.y)); // label on the bottom
	const Rect2 inner_bb(frame_bb.shrink(GuiStyle_FramePadding));
	const Rect2 total_bb = frame_rect;

	// Determine scale from values if not specified
	if (scale_min == FLT_MAX || scale_max == FLT_MAX) {
		real_t v_min = FLT_MAX;
		real_t v_max = -FLT_MAX;
		for (int i = 0; i < values_count; i++) {
			const real_t v = values_getter(data, i);
			if (v != v) { // Ignore NaN values
				continue;
			}
			v_min = MIN(v_min, v);
			v_max = MAX(v_max, v);
		}
		if (scale_min == FLT_MAX) {
			scale_min = v_min;
		}
		if (scale_max == FLT_MAX) {
			scale_max = v_max;
		}
	}

	const int values_count_min = (plot_type == PlotType_Lines) ? 2 : 1;
	String tooltip;
	int idx_hovered = -1;
	if (values_count >= values_count_min) {
		const int res_w = MIN((int)inner_bb.size.width, values_count) + ((plot_type == PlotType_Lines) ? -1 : 0);
		const int item_count = values_count + ((plot_type == PlotType_Lines) ? -1 : 0);

		// Tooltip on hover
		if (tooltip_pos && inner_bb.has_point(*tooltip_pos)) {
			const real_t t = CLAMP((tooltip_pos->x - inner_bb.position.x) / inner_bb.size.width, 0, 0.9999);
			const int v_idx = t * item_count;

			ERR_FAIL_COND(v_idx < 0 || v_idx >= values_count);

			const real_t v0 = values_getter(data, (v_idx + values_offset) % values_count);
			const real_t v1 = values_getter(data, (v_idx + 1 + values_offset) % values_count);
			if (plot_type == PlotType_Lines) {
				tooltip + string_format("%d: %8.4g\n%d: %8.4g", v_idx, v0, v_idx + 1, v1);
			} else if (plot_type == PlotType_Histogram) {
				tooltip + string_format("%d: %8.4g", v_idx, v0);
			}
			idx_hovered = v_idx;
		}

		const real_t t_step = 1.0 / res_w;
		const real_t inv_scale = (scale_min == scale_max) ? 0 : (1 / (scale_max - scale_min));

		real_t v0 = values_getter(data, (0 + values_offset) % values_count);
		real_t t0 = 0;
		Vector2 tp0 = Vector2(t0, 1 - SATURATE((v0 - scale_min) * inv_scale)); // point in the normalized space of our target rectangle
		const real_t zero_line = (scale_min * scale_max < 0) ? (1 + scale_min * inv_scale) : (scale_min < 0 ? 0 : 1); // where does the zero line stands

		const Color col_base = (plot_type == PlotType_Lines) ? GuiColor_PlotLines : GuiColor_PlotHistogram;
		const Color col_hovered = (plot_type == PlotType_Lines) ? GuiColor_PlotLinesHovered : GuiColor_PlotHistogramHovered;

		for (int n = 0; n < res_w; n++) {
			const real_t t1 = t0 + t_step;
			const int v1_idx = (int)(t0 * item_count + 0.5);
			ERR_FAIL_COND(v1_idx < 0 || v1_idx > values_count);
			const real_t v1 = values_getter(data, (v1_idx + values_offset + 1) % values_count);
			const Vector2 tp1 = Vector2(t1, 1 - SATURATE((v1 - scale_min) * inv_scale));

			Vector2 pos0 = LERP(inner_bb.min(), inner_bb.max(), tp0);
			Vector2 pos1 = LERP(inner_bb.min(), inner_bb.max(), (plot_type == PlotType_Lines) ? tp1 : Vector2(tp1.x, zero_line));
			if (plot_type == PlotType_Lines) {
				canvas->draw_line(pos0, pos1, idx_hovered == v1_idx ? col_hovered : col_base);
			} else if (plot_type == PlotType_Histogram) {
				if (pos1.x >= pos0.x + 2) {
					pos1.x -= 1;
				}
				canvas->draw_rect(Rect2(pos0, pos1 - pos0), idx_hovered == v1_idx ? col_hovered : col_base);
			}

			t0 = t1;
			tp0 = tp1;
		}
	}

	if (!overlay_text.empty()) {
		const String text = string_ellipsis(text_font, overlay_text, frame_bb.size.width);
		const Size2 size = text_font->get_string_size(text);
		canvas->draw_string(text_font, frame_bb.position - Vector2(size.x / 2, -GuiStyle_FramePadding), overlay_text); // center
	}

	if (label_size.x > 0) {
		canvas->draw_string(text_font, Vector2(frame_bb.position.x + GuiStyle_ItemInnerSpacing, inner_bb.position.y), label_text);
	}

	if (!tooltip.empty()) {
		canvas->draw_string(text_font, *tooltip_pos, tooltip);
	}
}

struct PlotArrayGetterData {
	const real_t *values;
	int stride;

	PlotArrayGetterData(const real_t *p_values, int p_stride) {
		values = p_values;
		stride = p_stride;
	}
};

static real_t Plot_ArrayGetter(void *data, int idx) {
	PlotArrayGetterData *plot_data = (PlotArrayGetterData *)data;
	const real_t v = *(const real_t *)(const void *)((const unsigned char *)plot_data->values + (size_t)idx * plot_data->stride);
	return v;
}

/// Public interface

void PlotLines(CanvasItem *canvas, Ref<Font> &text_font, const String &label, const real_t *values, int values_count, int values_offset, const String &overlay_text, real_t scale_min, real_t scale_max, const Rect2 &frame_rect, int stride) {
	PlotArrayGetterData data(values, stride);
	PlotGraph(canvas, text_font, PlotType_Lines, label, &Plot_ArrayGetter, (void *)&data, values_count, values_offset, overlay_text, scale_min, scale_max, frame_rect);
}

void PlotLines(CanvasItem *canvas, Ref<Font> &text_font, const String &label, ValuesGetter values_getter, void *data, int values_count, int values_offset, const String &overlay_text, real_t scale_min, real_t scale_max, const Rect2 &frame_rect) {
	PlotGraph(canvas, text_font, PlotType_Lines, label, values_getter, data, values_count, values_offset, overlay_text, scale_min, scale_max, frame_rect);
}

void PlotHistogram(CanvasItem *canvas, Ref<Font> &text_font, const String &label, const real_t *values, int values_count, int values_offset, const String &overlay_text, real_t scale_min, real_t scale_max, const Rect2 &frame_rect, int stride) {
	PlotArrayGetterData data(values, stride);
	PlotGraph(canvas, text_font, PlotType_Histogram, label, &Plot_ArrayGetter, (void *)&data, values_count, values_offset, overlay_text, scale_min, scale_max, frame_rect);
}

void PlotHistogram(CanvasItem *canvas, Ref<Font> &text_font, const String &label, ValuesGetter values_getter, void *data, int values_count, int values_offset, const String &overlay_text, real_t scale_min, real_t scale_max, const Rect2 &frame_rect) {
	PlotGraph(canvas, text_font, PlotType_Histogram, label, values_getter, data, values_count, values_offset, overlay_text, scale_min, scale_max, frame_rect);
}
