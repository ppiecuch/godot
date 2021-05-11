/*************************************************************************/
/*  GridShape.hpp                                                        */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef UUID_D7746018E901B4EE35CEDC635307152F
#define UUID_D7746018E901B4EE35CEDC635307152F

#include "Edge.hpp"
#include "LineShape.hpp"
#include "MergeShape.hpp"
#include "RepeatShape.hpp"
#include "ShapeVertex.hpp"

namespace generator {

/**
 * A 2d regular grid.
 */
class GridShape {
private:
	using Impl = MergeShape<RepeatShape<LineShape>, RepeatShape<LineShape>>;

	Impl mImpl;

public:
	using Edges = Impl::Edges;

	using Vertices = Impl::Vertices;

	/// @param size A half of the side length of the grid.
	/// @param segments The Number of cells in the grid.
	/// If <1 an empty shape results.
	/// @param subSegments The number of segment along each cell edge.
	/// If <1 an empty shape results.
	explicit GridShape(
			const gml::dvec2 &size = { 1.0, 1.0 },
			const gml::ivec2 &segments = { 4, 4 },
			const gml::ivec2 &subSegments = { 2, 2 }) noexcept;

	Edges edges() const noexcept;

	Vertices vertices() const noexcept;
};

} // namespace generator

#endif
