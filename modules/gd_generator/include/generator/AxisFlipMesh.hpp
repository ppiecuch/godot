/*************************************************************************/
/*  AxisFlipMesh.hpp                                                     */
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

#ifndef GENERATOR_AXISFLIPMESH_HPP
#define GENERATOR_AXISFLIPMESH_HPP

#include "TransformMesh.hpp"
#include "Triangle.hpp"

namespace generator {

/// Flips (mirrors) the mesh along one or more axis.
/// Texture coordinates are not flipped.
/// Also reverses triangle vertex order if needed.
template <typename Mesh>
class AxisFlipMesh {
private:
	using Impl = TransformMesh<Mesh>;
	Impl transformMesh_;

public:
	class Triangles {
	public:
		Triangle generate() const {
			Triangle triangle = triangles_.generate();
			if (mesh_->flip_)
				std::swap(triangle.vertices[0], triangle.vertices[2]);
			return triangle;
		}

		bool done() const noexcept { return triangles_.done(); }

		void next() { triangles_.next(); }

	private:
		const AxisFlipMesh *mesh_;

		typename TriangleGeneratorType<TransformMesh<Mesh> >::Type triangles_;

		Triangles(const AxisFlipMesh &mesh) :
				mesh_{ &mesh },
				triangles_{ mesh.transformMesh_.triangles() } {}

		friend class AxisFlipMesh;
	};

	///@param mesh Source data mesh.
	///@param x Flip x
	///@param y Flip y
	///@param z Flip z
	AxisFlipMesh(Mesh mesh, bool x, bool y, bool z) :
			transformMesh_{
				std::move(mesh),
				[x, y, z](MeshVertex &vertex) {
					if (x) {
						vertex.position[0] *= -1.0;
						vertex.normal[0] *= -1.0;
					}

					if (y) {
						vertex.position[1] *= -1.0;
						vertex.normal[1] *= -1.0;
					}

					if (z) {
						vertex.position[2] *= -1.0;
						vertex.normal[2] *= -1.0;
					}
				}
			},
			flip_{ false } {
		if (x) flip_ = !flip_;
		if (y) flip_ = !flip_;
		if (z) flip_ = !flip_;
	}

	Triangles triangles() const noexcept { return { *this }; }

	using Vertices = typename Impl::Vertices;

	Vertices vertices() const noexcept { return transformMesh_.vertices(); }

private:
	bool flip_;
};

template <typename Mesh>
AxisFlipMesh<Mesh> axisFlipMesh(Mesh mesh, bool x, bool y, bool z) {
	return AxisFlipMesh<Mesh>{ std::move(mesh), x, y, z };
}

} // namespace generator

#endif
