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

const Color GuiColor_PlotLines = Color(0.61, 0.61, 0.61, 1.00);
const Color GuiColor_PlotLinesHovered = Color(1.00, 0.43, 0.35, 1.00);
const Color GuiColor_PlotHistogram = Color(0.90, 0.70, 0.00, 1.00);
const Color GuiColor_PlotHistogramHovered = Color(1.00, 0.60, 0.00, 1.00);

const Size2 GuiStyle_FramePadding = Size2(2, 2);
const Size2 GuiStyle_ItemInnerSpacing = Size2(2, 2);

static inline Vector2 LERP(const Vector2 &a, const Vector2 &b, real_t t) { return Vector2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t); }
static inline Vector2 LERP(const Vector2 &a, const Vector2 &b, const Vector2 &t) { return Vector2(a.x + (b.x - a.x) * t.x, a.y + (b.y - a.y) * t.y); }
static inline real_t SATURATE(real_t f) { return (f < 0 ? 0 : (f > 1 ? 1 : f)); }
