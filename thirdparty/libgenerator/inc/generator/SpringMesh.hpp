/*************************************************************************/
/*  SpringMesh.hpp                                                       */
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

#ifndef GENERATOR_SPRINGMESH_HPP
#define GENERATOR_SPRINGMESH_HPP

#include "CircleShape.hpp"
#include "ExtrudeMesh.hpp"
#include "HelixPath.hpp"

namespace generator {

/// A spring aligned along the z-axis winding counterclockwise
/// @image html SpringMesh.svg
class SpringMesh {
private:
	using Impl = ExtrudeMesh<CircleShape, HelixPath>;
	Impl extrudeMesh_;

public:
	/// @param minor Radius of the spring it self.
	/// @param major Radius from the z-axis
	/// @param size Half of the length along the z-axis.
	/// @param slices Subdivisions around the spring.
	/// @param segments Subdivisions along the path.
	/// @param majorStart Counterclockwise angle around the z-axis relative to the x-axis.
	/// @param majorSweep Counterclockwise angle arounf the z-axis.
	SpringMesh(
			double minor = 0.25,
			double major = 1.0,
			double size = 1.0,
			int slices = 8,
			int segments = 32,
			double minorStart = 0.0,
			double minorSweep = gml::radians(360.0),
			double majorStart = 0.0,
			double majorSweep = gml::radians(720.0));

	using Triangles = typename Impl::Triangles;

	Triangles triangles() const noexcept { return extrudeMesh_.triangles(); }

	using Vertices = typename Impl::Vertices;

	Vertices vertices() const noexcept { return extrudeMesh_.vertices(); }
};

} // namespace generator

#endif
