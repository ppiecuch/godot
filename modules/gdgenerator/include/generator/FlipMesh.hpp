/*************************************************************************/
/*  FlipMesh.hpp                                                         */
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

#ifndef GENERATOR_FLIPMESH_HPP
#define GENERATOR_FLIPMESH_HPP

#include "TransformMesh.hpp"
#include "Triangle.hpp"

namespace generator {

/// Flips mesh inside out. Reverses triangles and normals.
template <typename Mesh>
class FlipMesh {
private:
	using Impl = TransformMesh<Mesh>;
	Impl transformMesh_;

public:
	class Triangles {
	public:
		Triangle generate() const {
			Triangle triangle = triangles_.generate();
			std::swap(triangle.vertices[0], triangle.vertices[2]);
			return triangle;
		}

		bool done() const noexcept { return triangles_.done(); }

		void next() { triangles_.next(); }

	private:
		typename TriangleGeneratorType<TransformMesh<Mesh> >::Type triangles_;

		Triangles(const TransformMesh<Mesh> &mesh) :
				triangles_{ mesh.triangles() } {}

		friend class FlipMesh;
	};

	/// @param mesh Source data mesh.
	FlipMesh(Mesh mesh) :
			transformMesh_{ std::move(mesh), [](MeshVertex &vertex) {
							   vertex.normal *= -1.0;
						   } } {}

	Triangles triangles() const noexcept { return this->transformMesh_; }

	using Vertices = typename Impl::Vertices;

	Vertices vertices() const noexcept { return transformMesh_.vertices(); }
};

template <typename Mesh>
FlipMesh<Mesh> flipMesh(Mesh mesh) {
	return FlipMesh<Mesh>{ std::move(mesh) };
}

} // namespace generator

#endif
