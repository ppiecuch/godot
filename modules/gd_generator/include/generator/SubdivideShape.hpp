/*************************************************************************/
/*  SubdivideShape.hpp                                                   */
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

#ifndef GENERATOR_SUBDIVIDESHAPE_HPP
#define GENERATOR_SUBDIVIDESHAPE_HPP

#include "Edge.hpp"
#include "ShapeVertex.hpp"
#include "utils.hpp"

namespace generator {

/// Cuts each edge in half
template <typename Shape>
class SubdivideShape {
public:
	class Edges {
	public:
		bool done() const noexcept { return edges_.done(); }

		Edge generate() const {
			Edge edge_ = edges_.generate();

			if (i_ % 2 == 0) return Edge{
				edge_.vertices[0],
				static_cast<int>(shape_->vertexCache_.size()) + i_ / 2
			};

			return Edge{
				static_cast<int>(shape_->vertexCache_.size()) + i_ / 2,
				edge_.vertices[1]
			};
		}

		void next() {
			++i_;
			if (i_ % 2 == 0) edges_.next();
		}

	private:
		const SubdivideShape *shape_;

		typename EdgeGeneratorType<Shape>::Type edges_;

		int i_;

		Edges(const SubdivideShape &shape) :
				shape_{ &shape },
				edges_{ shape.shape_.edges() },
				i_{ 0 } {}

		friend class SubdivideShape;
	};

	class Vertices {
	public:
		bool done() const noexcept {
			return vertexIndex_ == shape_->vertexCache_.size() && edges_.done();
		}

		ShapeVertex generate() const {
			if (vertexIndex_ < shape_->vertexCache_.size())
				return shape_->vertexCache_[vertexIndex_];

			const Edge edge = edges_.generate();
			const ShapeVertex &v1 = shape_->vertexCache_[edge.vertices[0]];
			const ShapeVertex &v2 = shape_->vertexCache_[edge.vertices[1]];

			ShapeVertex vertex;
			vertex.position = gml::mix(v1.position, v2.position, 0.5);
			vertex.tangent = gml::normalize(gml::mix(v1.tangent, v2.tangent, 0.5));
			vertex.texCoord = 0.5 * v1.texCoord + 0.5 * v2.texCoord;
			return vertex;
		}

		void next() {
			if (vertexIndex_ < shape_->vertexCache_.size())
				++vertexIndex_;
			else
				edges_.next();
		}

	private:
		const SubdivideShape *shape_;

		int vertexIndex_;

		typename EdgeGeneratorType<Shape>::Type edges_;

		Vertices(const SubdivideShape &shape) :
				shape_{ &shape },
				vertexIndex_{ 0 },
				edges_{ shape.shape_.edges() } {}

		friend class SubdivideShape;
	};

	SubdivideShape(Shape shape) :
			shape_(std::move(shape)),
			vertexCache_{} {
		for (const ShapeVertex &vertex : shape_.vertices()) {
			vertexCache_.push_back(vertex);
		}
	}

	Edges edges() const noexcept { return *this; }

	Vertices vertices() const noexcept { return *this; }

private:
	Shape shape_;

	std::vector<ShapeVertex> vertexCache_;
};

} // namespace generator

#endif
