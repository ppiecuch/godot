/**************************************************************************/
/*  ddls_potrace.h                                                        */
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

#pragma once

#include "ddls_fwd.h"

#include "core/image.h"
#include "core/map.h"
#include "core/math/vector2.h"
#include "core/vector.h"

class CanvasItem;

namespace DDLSPotrace {
Vector<Vector<Point2>> build_shapes(Ref<Image> p_bmp_data, Ref<Image> p_debug_bmp = nullptr, CanvasItem *p_debug_shape = nullptr);
Vector<Point2> build_shape(Ref<Image> p_bmp_data, int p_from_pixel_row, int p_from_pixel_col, Map<String, bool> p_dict_pixels_done, Ref<Image> p_debug_bmp = nullptr, CanvasItem *p_debug_shape = nullptr);
DDLSGraph build_graph(Vector<Point2> p_shape);
Vector<Point2> build_polygon(DDLSGraph p_graph, CanvasItem *p_debug_shape = nullptr);
} //namespace DDLSPotrace
