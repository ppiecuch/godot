/*************************************************************************/
/*  AxisSwapMesh.hpp                                                     */
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

#ifndef GENERATOR_AXISSWAPMESH_HPP
#define GENERATOR_AXISSWAPMESH_HPP

#include "Axis.hpp"
#include "TransformMesh.hpp"
#include "Triangle.hpp"

namespace generator {

/// Swaps any number of axis in the mesh.
template <typename Mesh>
class AxisSwapMesh {
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
		const AxisSwapMesh *mesh_;

		typename TriangleGeneratorType<TransformMesh<Mesh>>::Type triangles_;

		Triangles(const AxisSwapMesh &mesh) :
				mesh_{ &mesh },
				triangles_{ mesh.transformMesh_.triangles() } {}

		friend class AxisSwapMesh;
	};

	///@param mesh Source data mesh.
	///@param x Axis to be used as the x-axis
	///@param y Axis to be used as the y-axis
	///@param z Axis to be used as the z-axis
	AxisSwapMesh(Mesh mesh, Axis x, Axis y, Axis z) :
			transformMesh_{
				std::move(mesh),
				[x, y, z](MeshVertex &vertex) {
					vertex.position = gml::dvec3{
						vertex.position[static_cast<int>(x)],
						vertex.position[static_cast<int>(y)],
						vertex.position[static_cast<int>(z)]
					};
					vertex.normal = gml::dvec3{
						vertex.normal[static_cast<int>(x)],
						vertex.normal[static_cast<int>(y)],
						vertex.normal[static_cast<int>(z)]
					};
				}
			},
			flip_{ true } {
		if (x != Axis::X)
			flip_ = !flip_;
		if (y != Axis::Y)
			flip_ = !flip_;
		if (z != Axis::Z)
			flip_ = !flip_;
	}

	Triangles triangles() const noexcept { return { *this }; }

	using Vertices = typename Impl::Vertices;

	Vertices vertices() const noexcept { return transformMesh_.vertices(); }

private:
	bool flip_;
};

template <typename Mesh>
AxisSwapMesh<Mesh> axisSwapMesh(Mesh mesh, Axis x, Axis y, Axis z) {
	return AxisSwapMesh<Mesh>{ std::move(mesh), x, y, z };
}

} // namespace generator

#endif
