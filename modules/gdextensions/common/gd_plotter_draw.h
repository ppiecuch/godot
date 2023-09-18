/**************************************************************************/
/*  gd_plotter_draw.h                                                     */
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

#ifndef GD_PLOTTER_DRAW_H
#define GD_PLOTTER_DRAW_H

#include "core/math/vector2.h"
#include "scene/2d/canvas_item.h"
#include "scene/resources/font.h"

typedef real_t (*ValuesGetter)(void *data, int idx);
typedef void (*SeriesGetter)(real_t *start, real_t *end, uint8_t *level, const String &caption, const void *data, int idx);

void PlotLines(CanvasItem *canvas, Ref<Font> &text_font, const String &label, const real_t *values, int values_count, int values_offset, const String &overlay_text, real_t scale_min, real_t scale_max, const Rect2 &frame_rect, int stride);
void PlotLines(CanvasItem *canvas, Ref<Font> &text_font, const String &label, ValuesGetter values_getter, void *data, int values_count, int values_offset, const char *overlay_text, real_t scale_min, real_t scale_max, const Rect2 &frame_rect);

void PlotHistogram(CanvasItem *canvas, Ref<Font> &text_font, const String &label, const real_t *values, int values_count, int values_offset, const String &overlay_text, real_t scale_min, real_t scale_max, const Rect2 &frame_rect, int stride);
void PlotHistogram(CanvasItem *canvas, Ref<Font> &text_font, const String &label, ValuesGetter values_getter, void *data, int values_count, int values_offset, const char *overlay_text, real_t scale_min, real_t scale_max, const Rect2 &frame_rect);

void PlotFlame(CanvasItem *canvas, Ref<Font> &text_font, const String &label, const real_t *values, int values_count, int values_offset, const String &overlay_text, real_t scale_min, real_t scale_max, const Rect2 &frame_rect, int stride);
void PlotFlame(CanvasItem *canvas, Ref<Font> &text_font, const String &label, SeriesGetter values_getter, void *data, int values_count, int values_offset, const char *overlay_text, real_t scale_min, real_t scale_max, const Rect2 &frame_rect);

#endif // GD_PLOTTER_DRAW_H
