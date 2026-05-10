/**************************************************************************/
/*  poly_decomp_clipper10.cpp                                             */
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

#include "poly_decomp_clipper10.h"
#include "core/math/geometry/2d/poly/utils/godot_clipper10_path_convert.h"

Vector<Vector<Point2>> PolyDecomp2DClipper10::triangulate_mono(const Vector<Vector<Point2>> &p_polygons) {
	using namespace clipperlib;

	ClipperTri clp = configure(parameters);

	Paths subject;
	GodotClipperUtils::scale_up_polypaths(p_polygons, subject);
	clp.AddPaths(subject, ptSubject);

	Paths triangles;
	clp.Execute(ctUnion, triangles, fill_rule);

	Vector<Vector<Point2>> ret;
	GodotClipperUtils::scale_down_polypaths(triangles, ret);

	return ret;
}

clipperlib::ClipperTri PolyDecomp2DClipper10::configure(const Ref<PolyDecompParameters2D> &p_parameters) {
	using namespace clipperlib;

	fill_rule = FillRule(p_parameters->fill_rule);

	return ClipperTri();
}
