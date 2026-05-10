/**************************************************************************/
/*  poly_offset_clipper10.cpp                                             */
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

#include "poly_offset_clipper10.h"
#include "core/math/geometry/2d/poly/utils/godot_clipper10_path_convert.h"

Vector<Vector<Point2>> PolyOffset2DClipper10::offset_polypaths(const Vector<Vector<Point2>> &p_polypaths, real_t p_delta) {
	clipperlib::ClipperOffset clp = configure(parameters);

	clipperlib::Paths subject;
	GodotClipperUtils::scale_up_polypaths(p_polypaths, subject);
	clp.AddPaths(subject, join_type, end_type);

	clipperlib::Paths solution;
	clp.Execute(solution, p_delta * SCALE_FACTOR);

	Vector<Vector<Point2>> ret;
	GodotClipperUtils::scale_down_polypaths(solution, ret);

	return ret;
}

clipperlib::ClipperOffset PolyOffset2DClipper10::configure(const Ref<PolyOffsetParameters2D> &p_parameters) {
	using namespace clipperlib;

	switch (p_parameters->join_type) {
		case PolyOffsetParameters2D::JOIN_SQUARE:
			join_type = kSquare;
			break;
		case PolyOffsetParameters2D::JOIN_ROUND:
			join_type = kRound;
			break;
		case PolyOffsetParameters2D::JOIN_MITER:
			join_type = kMiter;
			break;
	}
	switch (p_parameters->end_type) {
		case PolyOffsetParameters2D::END_POLYGON:
			end_type = kPolygon;
			break;
		case PolyOffsetParameters2D::END_JOINED:
			end_type = kOpenJoined;
			break;
		case PolyOffsetParameters2D::END_BUTT:
			end_type = kOpenButt;
			break;
		case PolyOffsetParameters2D::END_SQUARE:
			end_type = kOpenSquare;
			break;
		case PolyOffsetParameters2D::END_ROUND:
			end_type = kOpenRound;
			break;
	}
	return ClipperOffset(p_parameters->miter_limit, p_parameters->arc_tolerance * SCALE_FACTOR);
}
