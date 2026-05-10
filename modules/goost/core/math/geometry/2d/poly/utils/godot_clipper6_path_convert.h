/**************************************************************************/
/*  godot_clipper6_path_convert.h                                         */
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

#include "core/math/vector2.h"
#include "core/vector.h"
#include "thirdparty/misc/clipper.hpp"

// Note: we provide a complete type for LocalMinimum as Clipper 6.4.2 only
// forward-declares the struct, which leads to the compilation errors.
namespace ClipperLib {
struct LocalMinimum {
	cInt Y;
	TEdge *LeftBound;
	TEdge *RightBound;
};
} // namespace ClipperLib

namespace GodotClipperUtils {

using namespace ClipperLib;

void scale_up_polypaths(const Vector<Vector<Point2>> &p_polypaths_in, Paths &p_polypaths_out);
void scale_down_polypaths(const Paths &p_polypaths_in, Vector<Vector<Point2>> &p_polypaths_out);
void scale_up_polypath(const Vector<Point2> &p_polypath_in, Path &p_polypath_out);
void scale_down_polypath(const Path &p_polypath_in, Vector<Point2> &p_polypath_out);

} // namespace GodotClipperUtils
