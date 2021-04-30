/*************************************************************************/
/*  CapsuleMesh.hpp                                                      */
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

#ifndef GENERATOR_CAPSULEMESH_HPP
#define GENERATOR_CAPSULEMESH_HPP

#include "CylinderMesh.hpp"
#include "MergeMesh.hpp"
#include "SphereMesh.hpp"
#include "TranslateMesh.hpp"

namespace generator {

/// Capsule (cylinder with spherical caps) centered at origin aligned along z-axis.
/// @image html CapsuleMesh.svg
class CapsuleMesh {
private:
	using Impl = MergeMesh<
			CylinderMesh,
			TranslateMesh<SphereMesh>,
			TranslateMesh<SphereMesh> >;
	Impl mergeMesh_;

public:
	/// @param radius Radius of the capsule on the xy-plane.
	/// @param size Half of the length between centers of the caps along the z-axis.
	/// @param slices Number of subdivisions around the z-axis.
	/// @param rings Number of radial subdivisions in the caps.
	/// @param start Counterclockwise angle relative to the x-axis.
	/// @param sweep Counterclockwise angle.
	CapsuleMesh(
			double radius = 1.0,
			double size = 0.5,
			int slices = 32,
			int segments = 4,
			int rings = 8,
			double start = 0.0,
			double sweep = gml::radians(360.0));

	using Triangles = typename Impl::Triangles;

	Triangles triangles() const noexcept { return mergeMesh_.triangles(); }

	using Vertices = typename Impl::Vertices;

	Vertices vertices() const noexcept { return mergeMesh_.vertices(); }
};

} // namespace generator

#endif
