/*************************************************************************/
/*  TorusMesh.hpp                                                        */
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

// Copyright 2015 Markus Ilmola
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

#ifndef GENERATOR_TORUS_HPP
#define GENERATOR_TORUS_HPP

#include "AxisSwapMesh.hpp"
#include "CircleShape.hpp"
#include "LatheMesh.hpp"
#include "TranslateShape.hpp"

namespace generator {

/// Torus centered at origin on the xy-plane.
/// @image html TorusMesh.svg
class TorusMesh {
private:
	using Impl = AxisSwapMesh<LatheMesh<TranslateShape<CircleShape> > >;
	Impl axisSwapMesh_;

public:
	/// @param minor Radius of the minor (inner) ring
	/// @param major Radius of the major (outer) ring
	/// @param slices Subdivisions around the minor ring
	/// @param segments Subdivisions around the major ring
	/// @param minorStart Counterclockwise angle relative to the xy-plane.
	/// @param minorSweep Counterclockwise angle around the circle.
	/// @param majorStart Counterclockwise angle around the z-axis relative to the x-axis.
	/// @param majorSweep Counterclockwise angle around the z-axis.
	TorusMesh(
			double minor = 0.25,
			double major = 1.0,
			int slices = 16,
			int segments = 32,
			double minorStart = 0.0,
			double minorSweep = gml::radians(360.0),
			double majorStart = 0.0,
			double majorSweep = gml::radians(360.0));

	using Triangles = typename Impl::Triangles;

	Triangles triangles() const noexcept { return axisSwapMesh_.triangles(); }

	using Vertices = typename Impl::Vertices;

	Vertices vertices() const noexcept { return axisSwapMesh_.vertices(); }
};

} // namespace generator

#endif
