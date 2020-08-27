/*************************************************************************/
/*  AxisSwapPath.hpp                                                     */
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

// Copyright 2015 Markus Ilmola
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

#ifndef GENERATOR_AXISSWAPPATH_HPP
#define GENERATOR_AXISSWAPPATH_HPP

#include "Axis.hpp"
#include "TransformPath.hpp"

namespace generator {

/// Swaps axis in path.
template <typename Path>
class AxisSwapPath {
private:
	using Impl = TransformPath<Path>;
	Impl transformPath_;

public:
	/// @param path Source data path
	/// @param x Axis to use as the X-axis
	/// @param y Axis to use as the Y-axis
	/// @param z Axis to use as the Z-axis
	AxisSwapPath(Path path, Axis x, Axis y, Axis z) :
			transformPath_{
				std::move(path),
				[x, y, z](PathVertex &vertex) {
					vertex.position = gml::dvec3{
						vertex.position[static_cast<int>(x)],
						vertex.position[static_cast<int>(y)],
						vertex.position[static_cast<int>(z)]
					};
					vertex.tangent = gml::dvec3{
						vertex.tangent[static_cast<int>(x)],
						vertex.tangent[static_cast<int>(y)],
						vertex.tangent[static_cast<int>(z)]
					};
					vertex.normal = gml::dvec3{
						vertex.normal[static_cast<int>(x)],
						vertex.normal[static_cast<int>(y)],
						vertex.normal[static_cast<int>(z)]
					};
				}
			} {}

	using Edges = typename Impl::Edges;

	Edges edges() const noexcept { return transformPath_.edges(); }

	using Vertices = typename Impl::Vertices;

	Vertices vertices() const noexcept { return transformPath_.vertices(); }
};

template <typename Path>
AxisSwapPath<Path> axisSwapPath(Path path, Axis x, Axis y, Axis z) {
	return AxisSwapPath<Path>{ std::move(path), x, y, z };
}

} // namespace generator

#endif
