/*************************************************************************/
/*  TransformMesh.hpp                                                    */
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

#ifndef GENERATOR_TRANSLATOR_HPP
#define GENERATOR_TRANSLATOR_HPP

#include <functional>

#include "MeshVertex.hpp"
#include "utils.hpp"

namespace generator {

/// Apply a mutator function to each vertex.
template <typename Mesh>
class TransformMesh {
private:
	using Impl = Mesh;
	Impl mesh_;

public:
	class Vertices {
	public:
		MeshVertex generate() const {
			auto vertex = vertices_.generate();
			mesh_->mutate_(vertex);
			return vertex;
		}

		bool done() const noexcept { return vertices_.done(); }

		void next() { vertices_.next(); }

	private:
		const TransformMesh *mesh_;

		typename VertexGeneratorType<Mesh>::Type vertices_;

		explicit Vertices(const TransformMesh &mesh) :
				mesh_{ &mesh },
				vertices_{ mesh.mesh_.vertices() } {}

		friend class TransformMesh;
	};

	/// @param mesh Source data mesh.
	/// @param mutate Callback function that gets called once per vertex.
	explicit TransformMesh(Mesh mesh, std::function<void(MeshVertex &)> mutate) :
			mesh_{ std::move(mesh) },
			mutate_{ std::move(mutate) } {}

	using Triangles = typename Impl::Triangles;

	Triangles triangles() const noexcept { return mesh_.triangles(); }

	Vertices vertices() const noexcept { return Vertices{ *this }; }

private:
	std::function<void(MeshVertex &)> mutate_;
};

template <typename Mesh>
TransformMesh<Mesh> transformMesh(
		Mesh mesh, std::function<void(MeshVertex &)> mutate) {
	return TransformMesh<Mesh>{ std::move(mesh), std::move(mutate) };
}

} // namespace generator

#endif
