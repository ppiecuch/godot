/*************************************************************************/
/*  MergeMesh.hpp                                                        */
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

#ifndef GENERATOR_MERGEMESH_HPP
#define GENERATOR_MERGEMESH_HPP

#include "EmptyMesh.hpp"
#include "MeshVertex.hpp"
#include "utils.hpp"

namespace generator {

/// Merges (concatenates) multiple meshes to to together.
template <typename... Mesh>
class MergeMesh; // undefined

template <>
class MergeMesh<> : public EmptyMesh {};

template <typename Head, typename... Tail>
class MergeMesh<Head, Tail...> {
public:
	class Triangles {
	public:
		Triangle generate() const {
			if (!head_.done()) return head_.generate();

			Triangle triangle = tail_.generate();
			triangle.vertices += head_VertexCount;
			return triangle;
		}

		bool done() const noexcept { return mAllDone; }

		void next() {
			if (!head_.done())
				head_.next();
			else
				tail_.next();

			mAllDone = tail_.done() && head_.done();
		}

	private:
		typename TriangleGeneratorType<Head>::Type head_;
		typename TriangleGeneratorType<MergeMesh<Tail...> >::Type tail_;

		int head_VertexCount;

		bool mAllDone;

		Triangles(const MergeMesh &mesh) :
				head_{ mesh.head_.triangles() },
				tail_(mesh.tail_.triangles()),
				head_VertexCount{ count(mesh.head_.vertices()) },
				mAllDone{ tail_.done() && head_.done() } {}

		friend class MergeMesh<Head, Tail...>;
	};

	class Vertices {
	public:
		MeshVertex generate() const {
			if (!head_.done()) return head_.generate();
			return tail_.generate();
		}

		bool done() const noexcept { return mAllDone; }

		void next() {
			if (!head_.done())
				head_.next();
			else
				tail_.next();

			mAllDone = tail_.done() && head_.done();
		}

	private:
		typename VertexGeneratorType<Head>::Type head_;
		typename VertexGeneratorType<MergeMesh<Tail...> >::Type tail_;

		bool mAllDone;

		Vertices(const MergeMesh &mesh) :
				head_{ mesh.head_.vertices() },
				tail_(mesh.tail_.vertices()),
				mAllDone{ tail_.done() && head_.done() } {}

		friend class MergeMesh<Head, Tail...>;
	};

	MergeMesh(Head head, Tail... tail) :
			head_{ std::move(head) },
			tail_{ std::move(tail)... } {}

	Triangles triangles() const noexcept { return *this; }

	Vertices vertices() const noexcept { return *this; }

private:
	Head head_;
	MergeMesh<Tail...> tail_;
};

template <typename... Mesh>
MergeMesh<Mesh...> mergeMesh(Mesh... meshes) {
	return MergeMesh<Mesh...>{ std::move(meshes)... };
}

} // namespace generator

#endif
