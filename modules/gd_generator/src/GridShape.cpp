/*************************************************************************/
/*  GridShape.cpp                                                        */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "generator/GridShape.hpp"

using namespace generator;

GridShape::GridShape(
		const gml::dvec2 &size,
		const gml::ivec2 &segments, const gml::ivec2 &subSegments) noexcept : mImpl{
	// Horizontal lines
	RepeatShape<LineShape>{
			LineShape{
					gml::dvec2{ -size[0], -size[1] }, gml::dvec2{ size[0], -size[1] },
					segments[0] * subSegments[0] },
			segments[1] < 1 ? 0 : segments[1] + 1,
			gml::dvec2{ 0.0, 2.0 * size[1] / std::max(segments[1], 1) } },
	// Vertical lines
	RepeatShape<LineShape>{
			LineShape{
					gml::dvec2{ -size[0], -size[1] }, gml::dvec2{ -size[0], size[1] },
					segments[1] * subSegments[1] },
			segments[0] < 1 ? 0 : segments[0] + 1,
			gml::dvec2{ 2.0 * size[0] / std::max(segments[0], 1), 0.0 } },
} {
	//
}

GridShape::Edges GridShape::edges() const noexcept {
	return mImpl.edges();
}

GridShape::Vertices GridShape::vertices() const noexcept {
	return mImpl.vertices();
}
