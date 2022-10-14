/*************************************************************************/
/*  gd_plotter.h                                                         */
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

#include "core/int_types.h"
#include "core/math/math_funcs.h"
#include "core/math/vector2.h"

enum PlotType {
	PlotType_Lines,
	PlotType_Histogram,
};

static inline Vector2 LERP(const Vector2 &a, const Vector2 &b, real_t t) { return Vector2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t); }
static inline Vector2 LERP(const Vector2 &a, const Vector2 &b, const Vector2 &t) { return Vector2(a.x + (b.x - a.x) * t.x, a.y + (b.y - a.y) * t.y); }
static inline real_t SATURATE(real_t f) { return (f < 0) ? 0 : (f > 1) ? 1
																	   : f; }

void PlotFlame(const char *label, void (*values_getter)(real_t *start, real_t *end, uint8_t *level, const char **caption, const void *data, int idx), const void *data, int values_count, int values_offset, const char *overlay_text, real_t scale_min, real_t scale_max, Size2 graph_size) {
	// Find the maximum depth
	uint8_t maxDepth = 0;
	for (int i = values_offset; i < values_count; ++i) {
		uint8_t depth;
		values_getter(nullptr, nullptr, &depth, nullptr, data, i);
		maxDepth = MAX(maxDepth, depth);
	}

	const auto blockHeight = ImGui::GetTextLineHeight() + (style.FramePadding.y * 2);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	const Rect2 frame_bb(window->DC.CursorPos, window->DC.CursorPos + graph_size);
	const Rect2 inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
	const Rect2 total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0 ? style.ItemInnerSpacing.x + label_size.x : 0, 0));
	ImGui::ItemSize(total_bb, style.FramePadding.y);
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
			if (v_start == v_start) // Check non-NaN values
				v_min = ImMin(v_min, v_start);
			if (v_end == v_end) // Check non-NaN values
				v_max = ImMax(v_max, v_end);
		}
		if (scale_min == FLT_MAX)
			scale_min = v_min;
		if (scale_max == FLT_MAX)
			scale_max = v_max;
	}

	ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

	bool any_hovered = false;
	if (values_count - values_offset >= 1) {
		const uint32_t col_base = ImGui::GetColorU32(ImGuiCol_PlotHistogram) & 0x77FFFFFF;
		const uint32_t col_hovered = ImGui::GetColorU32(ImGuiCol_PlotHistogramHovered) & 0x77FFFFFF;
		const uint32_t col_outline_base = ImGui::GetColorU32(ImGuiCol_PlotHistogram) & 0x7FFFFFFF;
		const uint32_t col_outline_hovered = ImGui::GetColorU32(ImGuiCol_PlotHistogramHovered) & 0x7FFFFFFF;

		for (int i = values_offset; i < values_count; ++i) {
			real_t stageStart, stageEnd;
			uint8_t depth;
			const char *caption;

			values_getter(&stageStart, &stageEnd, &depth, &caption, data, i);

			auto duration = scale_max - scale_min;
			if (duration == 0) {
				return;
			}

			auto start = stageStart - scale_min;
			auto end = stageEnd - scale_min;

			auto startX = static_cast<real_t>(start / (double)duration);
			auto endX = static_cast<real_t>(end / (double)duration);

			real_t width = inner_bb.Max.x - inner_bb.Min.x;
			real_t height = blockHeight * (maxDepth - depth + 1) - style.FramePadding.y;

			auto pos0 = inner_bb.Min + Vector2(startX * width, height);
			auto pos1 = inner_bb.Min + Vector2(endX * width, height + blockHeight);

			bool v_hovered = false;
			if (ImGui::IsMouseHoveringRect(pos0, pos1)) {
				ImGui::SetTooltip("%s: %8.4g", caption, stageEnd - stageStart);
				v_hovered = true;
				any_hovered = v_hovered;
			}

			window->DrawList->AddRectFilled(pos0, pos1, v_hovered ? col_hovered : col_base);
			window->DrawList->AddRect(pos0, pos1, v_hovered ? col_outline_hovered : col_outline_base);
			auto textSize = ImGui::CalcTextSize(caption);
			auto boxSize = (pos1 - pos0);
			auto textOffset = Vector2(0, 0);
			if (textSize.x < boxSize.x) {
				textOffset = Vector2(0.5, 0.5) * (boxSize - textSize);
				ImGui::RenderText(pos0 + textOffset, caption);
			}
		}

		// Text overlay
		if (overlay_text) {
			ImGui::RenderTextClipped(Vector2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), frame_bb.Max, overlay_text, nullptr, nullptr, Size2(0.5, 0.0));
		}

		if (label_size.x > 0) {
			ImGui::RenderText(Vector2(frame_bb.Max.x + style.ItemInnerSpacing.x, inner_bb.Min.y), label);
		}
	}

	if (!any_hovered && ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Total: %8.4g", scale_max - scale_min);
	}
}

int PlotEx(PlotType plot_type, const char *label, real_t (*values_getter)(void *data, int idx), void *data, int values_count, int values_offset, const char *overlay_text, real_t scale_min, real_t scale_max, Size2 frame_size) {
	const ImGuiStyle &style = g.Style;
	const ImGuiID id = window->GetID(label);

	const Vector2 label_size = CalcTextSize(label, NULL, true);

	const Rect2 frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
	const Rect2 inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
	const Rect2 total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0, 0));
	ItemSize(total_bb, style.FramePadding.y);
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
			if (v != v) // Ignore NaN values
				continue;
			v_min = ImMin(v_min, v);
			v_max = ImMax(v_max, v);
		}
		if (scale_min == FLT_MAX) {
			scale_min = v_min;
		}
		if (scale_max == FLT_MAX) {
			scale_max = v_max;
		}
	}

	RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

	const int values_count_min = (plot_type == PlotType_Lines) ? 2 : 1;
	int idx_hovered = -1;
	if (values_count >= values_count_min) {
		int res_w = ImMin((int)frame_size.x, values_count) + ((plot_type == PlotType_Lines) ? -1 : 0);
		int item_count = values_count + ((plot_type == PlotType_Lines) ? -1 : 0);

		// Tooltip on hover
		if (hovered && inner_bb.Contains(g.IO.MousePos)) {
			const real_t t = ImClamp((g.IO.MousePos.x - inner_bb.Min.x) / (inner_bb.Max.x - inner_bb.Min.x), 0.0f, 0.9999f);
			const int v_idx = (int)(t * item_count);
			IM_ASSERT(v_idx >= 0 && v_idx < values_count);

			const real_t v0 = values_getter(data, (v_idx + values_offset) % values_count);
			const real_t v1 = values_getter(data, (v_idx + 1 + values_offset) % values_count);
			if (plot_type == PlotType_Lines) {
				SetTooltip("%d: %8.4g\n%d: %8.4g", v_idx, v0, v_idx + 1, v1);
			} else if (plot_type == PlotType_Histogram) {
				SetTooltip("%d: %8.4g", v_idx, v0);
			}
			idx_hovered = v_idx;
		}

		const real_t t_step = 1 / (real_t)res_w;
		const real_t inv_scale = (scale_min == scale_max) ? 0 : (1 / (scale_max - scale_min));

		real_t v0 = values_getter(data, (0 + values_offset) % values_count);
		real_t t0 = 0;
		Vector2 tp0 = Vector2(t0, 1 - SATURATE((v0 - scale_min) * inv_scale)); // point in the normalized space of our target rectangle
		real_t histogram_zero_line_t = (scale_min * scale_max < 0) ? (1 + scale_min * inv_scale) : (scale_min < 0 ? 0 : 1); // where does the zero line stands

		const uint32_t col_base = GetColorU32((plot_type == PlotType_Lines) ? ImGuiCol_PlotLines : ImGuiCol_PlotHistogram);
		const uint32_t col_hovered = GetColorU32((plot_type == PlotType_Lines) ? ImGuiCol_PlotLinesHovered : ImGuiCol_PlotHistogramHovered);

		for (int n = 0; n < res_w; n++) {
			const real_t t1 = t0 + t_step;
			const int v1_idx = (int)(t0 * item_count + 0.5);
			IM_ASSERT(v1_idx >= 0 && v1_idx < values_count);
			const real_t v1 = values_getter(data, (v1_idx + values_offset + 1) % values_count);
			const Vector2 tp1 = Vector2(t1, 1 - SATURATE((v1 - scale_min) * inv_scale));

			Vector2 pos0 = LERP(inner_bb.Min, inner_bb.Max, tp0);
			Vector2 pos1 = LERP(inner_bb.Min, inner_bb.Max, (plot_type == PlotType_Lines) ? tp1 : Vector2(tp1.x, histogram_zero_line_t));
			if (plot_type == PlotType_Lines) {
				window->DrawList->AddLine(pos0, pos1, idx_hovered == v1_idx ? col_hovered : col_base);
			} else if (plot_type == PlotType_Histogram) {
				if (pos1.x >= pos0.x + 2) {
					pos1.x -= 1;
				}
				window->DrawList->AddRectFilled(pos0, pos1, idx_hovered == v1_idx ? col_hovered : col_base);
			}

			t0 = t1;
			tp0 = tp1;
		}
	}

	// Text overlay
	if (overlay_text) {
		draw_text(Point2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), frame_bb.Max, overlay_text, nullptr, nullptr, Size2(0.5, 0.0));
	}

	if (label_size.x > 0) {
		draw_text(Point2(frame_bb.Max.x + style.ItemInnerSpacing.x, inner_bb.Min.y), label);
	}

	// Return hovered index or -1 if none are hovered.
	// This is currently not exposed in the public API because we need a larger redesign of the whole thing, but in the short-term we are making it available in PlotEx().
	return idx_hovered;
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

void PlotLines(const char *label, const real_t *values, int values_count, int values_offset, const char *overlay_text, real_t scale_min, real_t scale_max, Size2 graph_size, int stride) {
	PlotArrayGetterData data(values, stride);
	PlotEx(PlotType_Lines, label, &Plot_ArrayGetter, (void *)&data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void PlotLines(const char *label, real_t (*values_getter)(void *data, int idx), void *data, int values_count, int values_offset, const char *overlay_text, real_t scale_min, real_t scale_max, Size2 graph_size) {
	PlotEx(PlotType_Lines, label, values_getter, data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void PlotHistogram(const char *label, const real_t *values, int values_count, int values_offset, const char *overlay_text, real_t scale_min, real_t scale_max, Size2 graph_size, int stride) {
	PlotArrayGetterData data(values, stride);
	PlotEx(PlotType_Histogram, label, &Plot_ArrayGetter, (void *)&data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void PlotHistogram(const char *label, real_t (*values_getter)(void *data, int idx), void *data, int values_count, int values_offset, const char *overlay_text, real_t scale_min, real_t scale_max, Size2 graph_size) {
	PlotEx(PlotType_Histogram, label, values_getter, data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}
