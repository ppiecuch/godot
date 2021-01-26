/*************************************************************************/
/*  FlipPath.hpp                                                         */
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

#ifndef GENERATOR_FLIPPATH_HPP
#define GENERATOR_FLIPPATH_HPP

#include "Edge.hpp"
#include "TransformPath.hpp"

namespace generator {

/// Flips mesh inside out. Reverses triangles and normals.
template <typename Path>
class FlipPath {
private:
	using Impl = TransformPath<Path>;
	Impl transformPath_;

public:
	class Edges {
	public:
		Edge generate() const {
			Edge edge = edges_.generate();
			std::swap(edge.vertices[0], edge.vertices[1]);
			return edge;
		}

		bool done() const noexcept { return edges_.done(); }

		void next() { edges_.next(); }

	private:
		typename EdgeGeneratorType<TransformPath<Path> >::Type edges_;

		Edges(const TransformPath<Path> &path) :
				edges_{ path.edges() } {}

		friend class FlipPath;
	};

	/// @param path Source data path.
	FlipPath(Path path) :
			transformPath_{
				std::move(path),
				[](PathVertex &vertex) {
					vertex.tangent *= -1.0;
					vertex.normal *= -1.0;
				}
			} {}

	Edges edges() const noexcept { return { *this }; }

	using Vertices = typename Impl::Vertices;

	Vertices vertices() const noexcept { return transformPath_.vertices(); }
};

template <typename Path>
FlipPath<Path> flipPath(Path path) {
	return FlipPath<Path>{ std::move(path) };
}

} // namespace generator

#endif
