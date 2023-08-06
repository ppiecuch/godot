/**************************************************************************/
/*  gd_plotter.h                                                          */
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

#include "core/color.h"
#include "core/int_types.h"
#include "core/math/math_funcs.h"
#include "core/math/vector2.h"
#include "scene/2d/canvas_item.h"

#include <float.h>

enum PlotType {
	PlotType_Lines,
	PlotType_Histogram,
};

const Color GuiColor_PlotLines = Color(0.61, 0.61, 0.61, 1.00);
const Color GuiColor_PlotLinesHovered = Color(1.00, 0.43, 0.35, 1.00);
const Color GuiColor_PlotHistogram = Color(0.90, 0.70, 0.00, 1.00);
const Color GuiColor_PlotHistogramHovered = Color(1.00, 0.60, 0.00, 1.00);

const Size2 GuiStyle_FramePadding = Size2(2, 2);
const Size2 GuiStyle_ItemInnerSpacing = Size2(2, 2);

static inline Vector2 LERP(const Vector2 &a, const Vector2 &b, real_t t) { return Vector2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t); }
static inline Vector2 LERP(const Vector2 &a, const Vector2 &b, const Vector2 &t) { return Vector2(a.x + (b.x - a.x) * t.x, a.y + (b.y - a.y) * t.y); }
static inline real_t SATURATE(real_t f) { return (f < 0 ? 0 : (f > 1 ? 1 : f)); }

static void PlotFlame(CanvasItem *canvas, Ref<Font> &text_font, const char *label, void (*values_getter)(real_t *start, real_t *end, uint8_t *level, const char **caption, const void *data, int idx), const void *data, int values_count, int values_offset, const char *overlay_text, real_t scale_min, real_t scale_max, Size2 graph_size) {
	// Find the maximum depth
	uint8_t max_depth = 0;
	for (int i = values_offset; i < values_count; ++i) {
		uint8_t depth;
		values_getter(nullptr, nullptr, &depth, nullptr, data, i);
		max_depth = MAX(max_depth, depth);
	}

	const real_t block_height = ImGui::GetTextLineHeight() + (GuiStyle_FramePadding.y * 2);
	const Vector2 label_size = text_font->get_string_size(label);

	const Rect2 frame_bb(window->DC.CursorPos, window->DC.CursorPos + graph_size);
	const Rect2 inner_bb(frame_bb.Min + GuiStyle_FramePadding, frame_bb.Max - GuiStyle_FramePadding);
	const Rect2 total_bb(frame_bb.Min, frame_bb.Max + Vector2(label_size.x > 0 ? GuiStyle_ItemInnerSpacing.x + label_size.x : 0, 0));
	ImGui::ItemSize(total_bb, GuiStyle_FramePadding.y);
	if (!ImGui::ItemAdd(total_bb, 0, &frame_bb)) {
		return;
	}
	// determine scale from values if not specified
	if (scale_min == FLT_MAX || scale_max == FLT_MAX) {
		real_t v_min = FLT_MAX;
		real_t v_max = -FLT_MAX;
		for (int i = values_offset; i < values_count; i++) {
			real_t v_start, v_end;
			values_getter(&v_start, &v_end, nullptr, nullptr, data, i);
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
		const Color col_base = GuiColor_PlotHistogram.lighter();
		const Color col_hovered = GuiColor_PlotHistogramHovered.lighter();
		const Color col_outline_base = GuiColor_PlotHistogram.lighter();
		const Color col_outline_hovered = GuiColor_PlotHistogramHovered.lighter();

		for (int i = values_offset; i < values_count; ++i) {
			real_t stageStart, stageEnd;
			uint8_t depth;
			const char *caption;

			values_getter(&stageStart, &stageEnd, &depth, &caption, data, i);

			real_t duration = scale_max - scale_min;
			if (duration == 0) {
				return;
			}

			const real_t start = stageStart - scale_min;
			const real_t end = stageEnd - scale_min;

			const real_t startX = static_cast<real_t>(start / (real_t)duration);
			const real_t endX = static_cast<real_t>(end / (real_t)duration);

			const real_t width = inner_bb.Max.x - inner_bb.Min.x;
			const real_t height = block_height * (max_depth - depth + 1) - GuiStyle_FramePadding.y;

			const real_t pos0 = inner_bb.Min + Vector2(startX * width, height);
			const real_t pos1 = inner_bb.Min + Vector2(endX * width, height + block_height);

			bool v_hovered = false;
			if (IsMouseHoveringRect(pos0, pos1)) {
				SetTooltip("%s: %8.4g", caption, stageEnd - stageStart);
				v_hovered = true;
				any_hovered = v_hovered;
			}

			canvas->draw_rect(pos0, pos1, v_hovered ? col_hovered : col_base);
			canvas->draw_rect(pos0, pos1, v_hovered ? col_outline_hovered : col_outline_base, false);
			Size2 textSize = text_font->get_string_size(label)(caption);
			Size2 boxSize = (pos1 - pos0);
			Vector2 textOffset = Vector2(0, 0);
			if (textSize.x < boxSize.x) {
				textOffset = Vector2(0.5, 0.5) * (boxSize - textSize);
				canvas->draw_text(pos0 + textOffset, caption);
			}
		}

		// Text overlay
		if (overlay_text) {
			canvas->draw_text(Vector2(frame_bb.Min.x, frame_bb.Min.y + GuiStyle_FramePadding.y), frame_bb.Max, overlay_text, nullptr, nullptr, Size2(0.5, 0.0));
		}

		if (label_size.x > 0) {
			canvas->draw_text(Vector2(frame_bb.Max.x + GuiStyle_ItemInnerSpacing.x, inner_bb.Min.y), label);
		}
	}

	if (!any_hovered && IsItemHovered()) {
		SetTooltip("Total: %8.4g", scale_max - scale_min);
	}
}

static void PlotGraph(CanvasItem *canvas, Ref<Font> &text_font, PlotType plot_type, const char *label, real_t (*values_getter)(void *data, int idx), void *data, int values_count, int values_offset, const char *overlay_text, real_t scale_min, real_t scale_max, const Size2 &frame_size) {
	const ImGuiID id = window->GetID(label);

	const Size2 label_size = font->text_rect(label, nullptr, true);

	const Rect2 frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
	const Rect2 inner_bb(frame_bb.Min + GuiStyle_FramePadding, frame_bb.Max - GuiStyle_FramePadding);
	const Rect2 total_bb(frame_bb.Min, frame_bb.Max + Vector2(label_size.x > 0 ? GuiStyle_ItemInnerSpacing.x + label_size.x : 0, 0));
	ItemSize(total_bb, GuiStyle_FramePadding.y);
	if (!ItemAdd(total_bb, 0, &frame_bb)) {
		return -1;
	}
	const bool hovered = ItemHoverable(frame_bb, id);

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
	int idx_hovered = -1;
	if (values_count >= values_count_min) {
		int res_w = MIN((int)frame_size.x, values_count) + ((plot_type == PlotType_Lines) ? -1 : 0);
		int item_count = values_count + ((plot_type == PlotType_Lines) ? -1 : 0);

		// Tooltip on hover
		if (hovered && inner_bb.contains(g.IO.MousePos)) {
			const real_t t = CLAMP((g.IO.MousePos.x - inner_bb.Min.x) / (inner_bb.Max.x - inner_bb.Min.x), 0, 0.9999);
			const int v_idx = (int)(t * item_count);
			ERR_FAIL_COND(v_idx < 0 || v_idx >= values_count);

			const real_t v0 = values_getter(data, (v_idx + values_offset) % values_count);
			const real_t v1 = values_getter(data, (v_idx + 1 + values_offset) % values_count);
			if (plot_type == PlotType_Lines) {
				SetTooltip("%d: %8.4g\n%d: %8.4g", v_idx, v0, v_idx + 1, v1);
			} else if (plot_type == PlotType_Histogram) {
				SetTooltip("%d: %8.4g", v_idx, v0);
			}
			idx_hovered = v_idx;
		}

		const real_t t_step = 1.0 / res_w;
		const real_t inv_scale = (scale_min == scale_max) ? 0 : (1 / (scale_max - scale_min));

		real_t v0 = values_getter(data, (0 + values_offset) % values_count);
		real_t t0 = 0;
		Vector2 tp0 = Vector2(t0, 1 - SATURATE((v0 - scale_min) * inv_scale)); // point in the normalized space of our target rectangle
		real_t histogram_zero_line_t = (scale_min * scale_max < 0) ? (1 + scale_min * inv_scale) : (scale_min < 0 ? 0 : 1); // where does the zero line stands

		const Color col_base = (plot_type == PlotType_Lines) ? GuiColor_PlotLines : GuiColor_PlotHistogram;
		const Color col_hovered = (plot_type == PlotType_Lines) ? GuiColor_PlotLinesHovered : GuiColor_PlotHistogramHovered;

		for (int n = 0; n < res_w; n++) {
			const real_t t1 = t0 + t_step;
			const int v1_idx = (int)(t0 * item_count + 0.5);
			ERR_FAIL_COND(v1_idx < 0 || v1_idx > values_count);
			const real_t v1 = values_getter(data, (v1_idx + values_offset + 1) % values_count);
			const Vector2 tp1 = Vector2(t1, 1 - SATURATE((v1 - scale_min) * inv_scale));

			Vector2 pos0 = LERP(inner_bb.Min, inner_bb.Max, tp0);
			Vector2 pos1 = LERP(inner_bb.Min, inner_bb.Max, (plot_type == PlotType_Lines) ? tp1 : Vector2(tp1.x, histogram_zero_line_t));
			if (plot_type == PlotType_Lines) {
				canvas->draw_Line(pos0, pos1, idx_hovered == v1_idx ? col_hovered : col_base);
			} else if (plot_type == PlotType_Histogram) {
				if (pos1.x >= pos0.x + 2) {
					pos1.x -= 1;
				}
				canvas->draw_rect(pos0, pos1, idx_hovered == v1_idx ? col_hovered : col_base);
			}

			t0 = t1;
			tp0 = tp1;
		}
	}

	// Text overlay
	if (overlay_text) {
		canvas->draw_text(Point2(frame_bb.Min.x, frame_bb.Min.y + GuiStyle_FramePadding.y), frame_bb.Max, overlay_text, nullptr, nullptr, Size2(0.5, 0.0));
	}

	if (label_size.x > 0) {
		canvas->draw_text(Point2(frame_bb.Max.x + GuiStyle_ItemInnerSpacing.x, inner_bb.Min.y), label);
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

void PlotLines(CanvasItem *canvas, const char *label, const real_t *values, int values_count, int values_offset, const char *overlay_text, real_t scale_min, real_t scale_max, const Size2 &graph_size, int stride) {
	PlotArrayGetterData data(values, stride);
	PlotGraph(PlotType_Lines, label, &Plot_ArrayGetter, (void *)&data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void PlotLines(CanvasItem *canvas, const char *label, real_t (*values_getter)(void *data, int idx), void *data, int values_count, int values_offset, const char *overlay_text, real_t scale_min, real_t scale_max, const Size2 &graph_size) {
	PlotGraph(PlotType_Lines, label, values_getter, data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void PlotHistogram(CanvasItem *canvas, const char *label, const real_t *values, int values_count, int values_offset, const char *overlay_text, real_t scale_min, real_t scale_max, const Size2 &graph_size, int stride) {
	PlotArrayGetterData data(values, stride);
	PlotGraph(PlotType_Histogram, label, &Plot_ArrayGetter, (void *)&data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void PlotHistogram(CanvasItem *canvas, const char *label, real_t (*values_getter)(void *data, int idx), void *data, int values_count, int values_offset, const char *overlay_text, real_t scale_min, real_t scale_max, const Size2 &graph_size) {
	PlotGraph(PlotType_Histogram, label, values_getter, data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}
