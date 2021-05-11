/*************************************************************************/
/*  FlipShape.hpp                                                        */
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

#ifndef GENERATOR_FLIPSHAPE_HPP
#define GENERATOR_FLIPSHAPE_HPP

#include "Edge.hpp"
#include "TransformShape.hpp"

namespace generator {

/// Flips shape direction. Reverses edges and tangents.
template <typename Shape>
class FlipShape {
private:
	using Impl = TransformShape<Shape>;
	Impl transformShape_;

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
		typename EdgeGeneratorType<TransformShape<Shape>>::Type edges_;

		Edges(const TransformShape<Shape> &shape) :
				edges_{ shape.edges() } {}

		friend class FlipShape;
	};

	/// @param shape Source data shape.
	FlipShape(Shape shape) :
			transformShape_{
				std::move(shape),
				[](ShapeVertex &vertex) {
					vertex.tangent *= -1.0;
				}
			} {}

	Edges edges() const noexcept { return { *this }; }

	using Vertices = typename Impl::Vertices;

	Vertices vertices() const noexcept { return transformShape_.vertices(); }
};

template <typename Shape>
FlipShape<Shape> flipShape(Shape shape) {
	return FlipShape<Shape>{ std::move(shape) };
}

} // namespace generator

#endif
