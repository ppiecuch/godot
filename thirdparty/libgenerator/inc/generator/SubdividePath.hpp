/*************************************************************************/
/*  SubdividePath.hpp                                                    */
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

#ifndef GENERATOR_SUBDIVIDEPATH_HPP
#define GENERATOR_SUBDIVIDEPATH_HPP

#include "Edge.hpp"
#include "PathVertex.hpp"
#include "ShapeVertex.hpp"
#include "utils.hpp"

namespace generator {

template <typename Path>
class SubdividePath {
public:
	class Edges {
	public:
		bool done() const noexcept { return edges_.done(); }

		Edge generate() const {
			Edge edge_ = edges_.generate();

			if (i_ % 2 == 0)
				return Edge{ { edge_.vertices[0],
						static_cast<int>(path_->vertexCache_.size()) + i_ / 2 } };

			return Edge{ { static_cast<int>(path_->vertexCache_.size()) + i_ / 2,
					edge_.vertices[1] } };
		}

		void next() {
			++i_;
			if (i_ % 2 == 0)
				edges_.next();
		}

	private:
		const SubdividePath *path_;

		typename EdgeGeneratorType<Path>::Type edges_;

		int i_;

		Edges(const SubdividePath &path) :
				path_{ &path },
				edges_{ path.path_.edges() },
				i_{ 0 } {}

		friend class SubdividePath;
	};

	class Vertices {
	public:
		bool done() const noexcept {
			return vertexIndex_ == path_->vertexCache_.size() && edges_.done();
		}

		PathVertex generate() const {
			if (vertexIndex_ < path_->vertexCache_.size())
				return path_->vertexCache_[vertexIndex_];

			const Edge edge = edges_.generate();
			const PathVertex &v1 = path_->vertexCache_[edge.vertices[0]];
			const PathVertex &v2 = path_->vertexCache_[edge.vertices[1]];

			PathVertex vertex;
			vertex.position = gml::mix(v1.position, v2.position, 0.5);
			vertex.tangent = gml::normalize(gml::mix(v1.tangent, v2.tangent, 0.5));
			vertex.normal = gml::normalize(gml::mix(v1.normal, v2.normal, 0.5));
			vertex.texCoord = 0.5 * v1.texCoord + 0.5 * v2.texCoord;
			return vertex;
		}

		void next() {
			if (vertexIndex_ < path_->vertexCache_.size())
				++vertexIndex_;
			else
				edges_.next();
		}

	private:
		const SubdividePath *path_;

		int vertexIndex_;

		typename EdgeGeneratorType<Path>::Type edges_;

		Vertices(const SubdividePath &path) :
				path_{ &path },
				vertexIndex_{ 0 },
				edges_{ path.path_.edges() } {}

		friend class SubdividePath;
	};

	SubdividePath(Path path) :
			path_(std::move(path)),
			vertexCache_{} {
		for (const PathVertex &vertex : path_.vertices()) {
			vertexCache_.push_back(vertex);
		}
	}

	Edges edges() const { return *this; }

	Vertices vertices() const { return *this; }

private:
	Path path_;

	std::vector<PathVertex> vertexCache_;
};

template <typename Path>
SubdividePath<Path> subdividePath(Path path) {
	return SubdividePath<Path>{ std::move(path) };
}

} // namespace generator

#endif
