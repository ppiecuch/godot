/*************************************************************************/
/*  MergePath.hpp                                                        */
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

#ifndef GENERATOR_MERGEPath_HPP
#define GENERATOR_MERGEPath_HPP

#include "EmptyPath.hpp"
#include "PathVertex.hpp"
#include "utils.hpp"

namespace generator {

/// Merges (concatenates) multiple paths together.
template <typename... Path>
class MergePath; // undefined

template <>
class MergePath<> : public EmptyPath {};

template <typename Head, typename... Tail>
class MergePath<Head, Tail...> {
public:
	class Edges {
	public:
		Edge generate() const {
			if (!head_.done()) return head_.generate();
			return tail_.generate() + head_VertexCount;
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
		typename EdgeGeneratorType<Head>::type head_;
		typename EdgeGeneratorType<MergePath<Tail...> >::type tail_;

		int head_VertexCount;

		bool mAllDone;

		Edges(const MergePath &path) :
				head_{ path.head_.triangles() },
				tail_(path.tail_.triangles()),
				head_VertexCount{ count(path.head_.vertices()) },
				mAllDone{ tail_.done() && head_.done() } {}

		friend class MergePath<Head, Tail...>;
	};

	class Vertices {
	public:
		PathVertex generate() const {
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
		typename VertexGeneratorType<Head>::type head_;
		typename VertexGeneratorType<MergePath<Tail...> >::type tail_;
		bool mAllDone;

		Vertices(const MergePath &path) :
				head_{ path.head_.vertices() },
				tail_(path.tail_.vertices()),
				mAllDone{ tail_.done() && head_.done() } {}

		friend class MergePath<Head, Tail...>;
	};

	MergePath(Head head, Tail... tail) :
			head_{ head },
			tail_{ tail... } {}

	Edges edges() const noexcept { return *this; }

	Vertices vertices() const noexcept { return *this; }

private:
	Head head_;
	MergePath<Tail...> tail_;
};

template <typename... Path>
MergePath<Path...> mergePath(Path... paths) {
	return MergePath<Path...>{ std::move(paths)... };
}

} // namespace generator

#endif
