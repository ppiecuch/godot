/*************************************************************************/
/*  CappedConeMesh.hpp                                                   */
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

#ifndef GENERATOR_CAPPEDCONEMESH_HPP
#define GENERATOR_CAPPEDCONEMESH_HPP

#include "ConeMesh.hpp"
#include "DiskMesh.hpp"
#include "FlipMesh.hpp"
#include "MergeMesh.hpp"
#include "TranslateMesh.hpp"
#include "UvFlipMesh.hpp"

namespace generator {

/// A cone with a cap centered at origin pointing towards positive z-axis.
/// @image html CappedConeMesh.svg
class CappedConeMesh {
private:
	using Impl = MergeMesh<
			ConeMesh,
			UvFlipMesh<FlipMesh<TranslateMesh<DiskMesh> > > >;
	Impl mergeMesh_;

public:
	/// @param radius Radius of the flat (negative z) end along the xy-plane.
	/// @param size Half of the length of the cylinder along the z-axis.
	/// @param slices Number of subdivisions around the z-axis.
	/// @param segments Number of subdivisions along the z-axis.
	/// @param rings Number of subdivisions of the cap.
	/// @param start Counterclockwise angle around the z-axis relative to the
	/// positive x-axis.
	/// @param sweep Counterclockwise angle around the z-axis.
	CappedConeMesh(
			double radius = 1.0,
			double size = 1.0,
			int slices = 32,
			int segments = 8,
			int rings = 4,
			double start = 0.0,
			double sweep = gml::radians(360.0));

	using Triangles = typename Impl::Triangles;

	Triangles triangles() const noexcept { return mergeMesh_.triangles(); }

	using Vertices = typename Impl::Vertices;

	Vertices vertices() const noexcept { return mergeMesh_.vertices(); }
};

} // namespace generator

#endif
