void PlotFlame(const char* label, void (*values_getter)(real_t* start, real_t* end, ImU8* level, const char** caption, const void* data, int idx), const void* data, int values_count, int values_offset, const char* overlay_text, real_t scale_min, real_t scale_max, ImVec2 graph_size) {
    // Find the maximum depth
    ImU8 maxDepth = 0;
    for (int i = values_offset; i < values_count; ++i)
    {
        ImU8 depth;
        values_getter(nullptr, nullptr, &depth, nullptr, data, i);
        maxDepth = ImMax(maxDepth, depth);
    }

    const auto blockHeight = ImGui::GetTextLineHeight() + (style.FramePadding.y * 2);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    if (graph_size.x == 0.0f)
        graph_size.x = ImGui::CalcItemWidth();
    if (graph_size.y == 0.0f)
        graph_size.y = label_size.y + (style.FramePadding.y * 3) + blockHeight * (maxDepth + 1);

    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + graph_size);
    const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, 0, &frame_bb))
        return;

    // Determine scale from values if not specified
    if (scale_min == FLT_MAX || scale_max == FLT_MAX)
    {
        real_t v_min = FLT_MAX;
        real_t v_max = -FLT_MAX;
        for (int i = values_offset; i < values_count; i++)
        {
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
    if (values_count - values_offset >= 1)
    {
        const ImU32 col_base = ImGui::GetColorU32(ImGuiCol_PlotHistogram) & 0x77FFFFFF;
        const ImU32 col_hovered = ImGui::GetColorU32(ImGuiCol_PlotHistogramHovered) & 0x77FFFFFF;
        const ImU32 col_outline_base = ImGui::GetColorU32(ImGuiCol_PlotHistogram) & 0x7FFFFFFF;
        const ImU32 col_outline_hovered = ImGui::GetColorU32(ImGuiCol_PlotHistogramHovered) & 0x7FFFFFFF;

        for (int i = values_offset; i < values_count; ++i)
        {
            real_t stageStart, stageEnd;
            ImU8 depth;
            const char* caption;

            values_getter(&stageStart, &stageEnd, &depth, &caption, data, i);

            auto duration = scale_max - scale_min;
            if (duration == 0)
            {
                return;
            }

            auto start = stageStart - scale_min;
            auto end = stageEnd - scale_min;

            auto startX = static_cast<real_t>(start / (double)duration);
            auto endX = static_cast<real_t>(end / (double)duration);

            real_t width = inner_bb.Max.x - inner_bb.Min.x;
            real_t height = blockHeight * (maxDepth - depth + 1) - style.FramePadding.y;

            auto pos0 = inner_bb.Min + ImVec2(startX * width, height);
            auto pos1 = inner_bb.Min + ImVec2(endX * width, height + blockHeight);

            bool v_hovered = false;
            if (ImGui::IsMouseHoveringRect(pos0, pos1))
            {
                ImGui::SetTooltip("%s: %8.4g", caption, stageEnd - stageStart);
                v_hovered = true;
                any_hovered = v_hovered;
            }

            window->DrawList->AddRectFilled(pos0, pos1, v_hovered ? col_hovered : col_base);
            window->DrawList->AddRect(pos0, pos1, v_hovered ? col_outline_hovered : col_outline_base);
            auto textSize = ImGui::CalcTextSize(caption);
            auto boxSize = (pos1 - pos0);
            auto textOffset = ImVec2(0.0f, 0.0f);
            if (textSize.x < boxSize.x)
            {
                textOffset = ImVec2(0.5f, 0.5f) * (boxSize - textSize);
                ImGui::RenderText(pos0 + textOffset, caption);
            }
        }

        // Text overlay
        if (overlay_text)
            ImGui::RenderTextClipped(ImVec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), frame_bb.Max, overlay_text, NULL, NULL, ImVec2(0.5f,0.0f));

        if (label_size.x > 0.0f)
            ImGui::RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, inner_bb.Min.y), label);
    }

    if (!any_hovered && ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Total: %8.4g", scale_max - scale_min);
    }
}


int ImGui::PlotEx(ImGuiPlotType plot_type, const char* label, real_t (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char* overlay_text, real_t scale_min, real_t scale_max, ImVec2 frame_size)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return -1;

    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    if (frame_size.x == 0.0f)
        frame_size.x = CalcItemWidth();
    if (frame_size.y == 0.0f)
        frame_size.y = label_size.y + (style.FramePadding.y * 2);

    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
    const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0));
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, 0, &frame_bb))
        return -1;
    const bool hovered = ItemHoverable(frame_bb, id);

    // Determine scale from values if not specified
    if (scale_min == FLT_MAX || scale_max == FLT_MAX)
    {
        real_t v_min = FLT_MAX;
        real_t v_max = -FLT_MAX;
        for (int i = 0; i < values_count; i++)
        {
            const real_t v = values_getter(data, i);
            if (v != v) // Ignore NaN values
                continue;
            v_min = ImMin(v_min, v);
            v_max = ImMax(v_max, v);
        }
        if (scale_min == FLT_MAX)
            scale_min = v_min;
        if (scale_max == FLT_MAX)
            scale_max = v_max;
    }

    RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

    const int values_count_min = (plot_type == ImGuiPlotType_Lines) ? 2 : 1;
    int idx_hovered = -1;
    if (values_count >= values_count_min)
    {
        int res_w = ImMin((int)frame_size.x, values_count) + ((plot_type == ImGuiPlotType_Lines) ? -1 : 0);
        int item_count = values_count + ((plot_type == ImGuiPlotType_Lines) ? -1 : 0);

        // Tooltip on hover
        if (hovered && inner_bb.Contains(g.IO.MousePos))
        {
            const real_t t = ImClamp((g.IO.MousePos.x - inner_bb.Min.x) / (inner_bb.Max.x - inner_bb.Min.x), 0.0f, 0.9999f);
            const int v_idx = (int)(t * item_count);
            IM_ASSERT(v_idx >= 0 && v_idx < values_count);

            const real_t v0 = values_getter(data, (v_idx + values_offset) % values_count);
            const real_t v1 = values_getter(data, (v_idx + 1 + values_offset) % values_count);
            if (plot_type == ImGuiPlotType_Lines)
                SetTooltip("%d: %8.4g\n%d: %8.4g", v_idx, v0, v_idx + 1, v1);
            else if (plot_type == ImGuiPlotType_Histogram)
                SetTooltip("%d: %8.4g", v_idx, v0);
            idx_hovered = v_idx;
        }

        const real_t t_step = 1.0f / (real_t)res_w;
        const real_t inv_scale = (scale_min == scale_max) ? 0.0f : (1.0f / (scale_max - scale_min));

        real_t v0 = values_getter(data, (0 + values_offset) % values_count);
        real_t t0 = 0.0f;
        ImVec2 tp0 = ImVec2( t0, 1.0f - ImSaturate((v0 - scale_min) * inv_scale) );                       // Point in the normalized space of our target rectangle
        real_t histogram_zero_line_t = (scale_min * scale_max < 0.0f) ? (1 + scale_min * inv_scale) : (scale_min < 0.0f ? 0.0f : 1.0f);   // Where does the zero line stands

        const ImU32 col_base = GetColorU32((plot_type == ImGuiPlotType_Lines) ? ImGuiCol_PlotLines : ImGuiCol_PlotHistogram);
        const ImU32 col_hovered = GetColorU32((plot_type == ImGuiPlotType_Lines) ? ImGuiCol_PlotLinesHovered : ImGuiCol_PlotHistogramHovered);

        for (int n = 0; n < res_w; n++)
        {
            const real_t t1 = t0 + t_step;
            const int v1_idx = (int)(t0 * item_count + 0.5f);
            IM_ASSERT(v1_idx >= 0 && v1_idx < values_count);
            const real_t v1 = values_getter(data, (v1_idx + values_offset + 1) % values_count);
            const ImVec2 tp1 = ImVec2( t1, 1.0f - ImSaturate((v1 - scale_min) * inv_scale) );

            // NB: Draw calls are merged together by the DrawList system. Still, we should render our batch are lower level to save a bit of CPU.
            ImVec2 pos0 = ImLerp(inner_bb.Min, inner_bb.Max, tp0);
            ImVec2 pos1 = ImLerp(inner_bb.Min, inner_bb.Max, (plot_type == ImGuiPlotType_Lines) ? tp1 : ImVec2(tp1.x, histogram_zero_line_t));
            if (plot_type == ImGuiPlotType_Lines)
            {
                window->DrawList->AddLine(pos0, pos1, idx_hovered == v1_idx ? col_hovered : col_base);
            }
            else if (plot_type == ImGuiPlotType_Histogram)
            {
                if (pos1.x >= pos0.x + 2.0f)
                    pos1.x -= 1.0f;
                window->DrawList->AddRectFilled(pos0, pos1, idx_hovered == v1_idx ? col_hovered : col_base);
            }

            t0 = t1;
            tp0 = tp1;
        }
    }

    // Text overlay
    if (overlay_text)
        RenderTextClipped(ImVec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), frame_bb.Max, overlay_text, NULL, NULL, ImVec2(0.5f, 0.0f));

    if (label_size.x > 0.0f)
        RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, inner_bb.Min.y), label);

    // Return hovered index or -1 if none are hovered.
    // This is currently not exposed in the public API because we need a larger redesign of the whole thing, but in the short-term we are making it available in PlotEx().
    return idx_hovered;
}

struct ImGuiPlotArrayGetterData
{
    const real_t* Values;
    int Stride;

    ImGuiPlotArrayGetterData(const real_t* values, int stride) { Values = values; Stride = stride; }
};

static real_t Plot_ArrayGetter(void* data, int idx)
{
    ImGuiPlotArrayGetterData* plot_data = (ImGuiPlotArrayGetterData*)data;
    const real_t v = *(const real_t*)(const void*)((const unsigned char*)plot_data->Values + (size_t)idx * plot_data->Stride);
    return v;
}

void ImGui::PlotLines(const char* label, const real_t* values, int values_count, int values_offset, const char* overlay_text, real_t scale_min, real_t scale_max, ImVec2 graph_size, int stride)
{
    ImGuiPlotArrayGetterData data(values, stride);
    PlotEx(ImGuiPlotType_Lines, label, &Plot_ArrayGetter, (void*)&data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void ImGui::PlotLines(const char* label, real_t (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char* overlay_text, real_t scale_min, real_t scale_max, ImVec2 graph_size)
{
    PlotEx(ImGuiPlotType_Lines, label, values_getter, data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void ImGui::PlotHistogram(const char* label, const real_t* values, int values_count, int values_offset, const char* overlay_text, real_t scale_min, real_t scale_max, ImVec2 graph_size, int stride)
{
    ImGuiPlotArrayGetterData data(values, stride);
    PlotEx(ImGuiPlotType_Histogram, label, &Plot_ArrayGetter, (void*)&data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void ImGui::PlotHistogram(const char* label, real_t (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char* overlay_text, real_t scale_min, real_t scale_max, ImVec2 graph_size)
{
    PlotEx(ImGuiPlotType_Histogram, label, values_getter, data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}
