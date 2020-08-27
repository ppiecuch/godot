/*************************************************************************/
/*  BoxMesh.hpp                                                          */
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

#ifndef GENERATOR_BOXMESH_HPP
#define GENERATOR_BOXMESH_HPP

#include "AxisSwapMesh.hpp"
#include "FlipMesh.hpp"
#include "MergeMesh.hpp"
#include "PlaneMesh.hpp"
#include "TranslateMesh.hpp"
#include "UvFlipMesh.hpp"
#include "UvSwapMesh.hpp"

namespace generator {

namespace detail {

class BoxFace {
private:
	using Impl = TranslateMesh<PlaneMesh>;
	Impl translateMesh_;

public:
	BoxFace(const gml::dvec2 &size, const gml::ivec2 &segments, double delta);

	using Triangles = typename Impl::Triangles;

	Triangles triangles() const noexcept { return translateMesh_.triangles(); }

	using Vertices = typename Impl::Vertices;

	Vertices vertices() const noexcept { return translateMesh_.vertices(); }
};

class BoxFaces {
private:
	using Impl = MergeMesh<BoxFace, UvFlipMesh<FlipMesh<BoxFace> > >;
	Impl mergeMesh_;

public:
	BoxFaces(const gml::dvec2 &size, const gml::ivec2 &segments, double delta);

	using Triangles = typename Impl::Triangles;

	Triangles triangles() const noexcept { return mergeMesh_.triangles(); }

	using Vertices = typename Impl::Vertices;

	Vertices vertices() const noexcept { return mergeMesh_.vertices(); }
};

} // namespace detail

/// Rectangular box centered at origin aligned along the x, y and z axis.
/// @image html BoxMesh.svg
class BoxMesh {
private:
	using Impl = MergeMesh<
			AxisSwapMesh<detail::BoxFaces>,
			UvFlipMesh<AxisSwapMesh<detail::BoxFaces> >,
			detail::BoxFaces>;
	Impl mergeMesh_;

public:
	/// @param size Half of the side length in x (0), y (1) and z (2) direction.
	/// @param segments The number of segments in x (0), y (1) and z (2)
	/// directions. All should be >= 1. If any one is zero faces in that
	/// direction are not genereted. If more than one is zero the mesh is empty.
	explicit BoxMesh(
			const gml::dvec3 &size = { 1.0, 1.0, 1.0 },
			const gml::ivec3 &segments = { 8, 8, 8 }) noexcept;

	using Triangles = typename Impl::Triangles;

	Triangles triangles() const noexcept { return mergeMesh_.triangles(); }

	using Vertices = typename Impl::Vertices;

	Vertices vertices() const noexcept { return mergeMesh_.vertices(); }
};

} // namespace generator

#endif
