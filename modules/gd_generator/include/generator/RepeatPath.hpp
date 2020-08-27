/*************************************************************************/
/*  RepeatPath.hpp                                                       */
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

#ifndef UUID_580A4434BBE9A17A6302CC90C22DE898
#define UUID_580A4434BBE9A17A6302CC90C22DE898

#include "Edge.hpp"
#include "PathVertex.hpp"
#include "utils.hpp"

namespace generator {

/**
 * Repeats the same path a given number of time at given intervals.
 */
template <typename Path>
class RepeatPath {
public:
	class Edges {
	public:
		bool done() const noexcept {
			return mIndex >= mRepeatPath->mInstances;
		}

		Edge generate() const {
			Edge temp = mEdges.generate();
			temp.vertices += mDelta;
			return temp;
		}

		void next() noexcept {
			mEdges.next();

			if (mEdges.done()) {
				++mIndex;
				mDelta += mRepeatPath->mVertexCount;
				mEdges = mRepeatPath->mPath.edges();
			}
		}

	private:
		const RepeatPath *mRepeatPath;

		typename EdgeGeneratorType<Path>::Type mEdges;

		int mIndex;

		int mDelta;

		explicit Edges(const RepeatPath *repeatPath) noexcept : mRepeatPath{ repeatPath },
																mEdges{ repeatPath->mPath.edges() },
																mIndex{ repeatPath->mVertexCount > 0 ? 0 : repeatPath->mInstances },
																mDelta{ 0 } {
		}

		friend class RepeatPath;
	};

	class Vertices {
	public:
		bool done() const noexcept {
			return mIndex >= mRepeatPath->mInstances;
		}

		PathVertex generate() const {
			PathVertex temp = mVertices.generate();
			temp.position += mDelta;
			return temp;
		}

		void next() {
			mVertices.next();

			if (mVertices.done()) {
				++mIndex;
				mDelta += mRepeatPath->mDelta;
				mVertices = mRepeatPath->mPath.vertices();
			}
		}

	private:
		explicit Vertices(const RepeatPath *repeatPath) :
				mRepeatPath{ repeatPath },
				mVertices{ repeatPath->mPath.vertices() },
				mIndex{ repeatPath->mVertexCount > 0 ? 0 : repeatPath->mInstances },
				mDelta{} {}

		const RepeatPath *mRepeatPath;

		typename VertexGeneratorType<Path>::Type mVertices;

		int mIndex;

		gml::dvec3 mDelta;

		int countVertices() const noexcept {
			if (mRepeatPath->mInstances < 1) return 0;

			return mRepeatPath->mVertexCount *
						   (mRepeatPath->mInstances - mIndex - 1) +
				   count(mVertices);
		}

		friend int count(const Vertices &generator) noexcept {
			return generator.countVertices();
		}

		friend class RepeatPath;
	};

	/// @param path The path to repeat.
	/// @param instances Number of times to repeat. If <1 an empty path results.
	/// @param delta An offset aplied to each copy.
	explicit RepeatPath(Path path, int instances, const gml::dvec3 &delta) noexcept : mPath{ std::move(path) },
																					  mInstances{ instances },
																					  mDelta{ delta },
																					  mVertexCount{ count(mPath.vertices()) } {}

	Edges edges() const noexcept {
		return Edges{ this };
	}

	Vertices vertices() const noexcept {
		return Vertices{ this };
	}

private:
	Path mPath;

	int mInstances;

	gml::dvec3 mDelta;

	int mVertexCount;
};

template <typename Path>
RepeatPath<Path> repeatPath(Path path, int instances, const gml::dvec3 &delta) noexcept {
	return RepeatPath<Path>{ std::move(path), instances, delta };
}

} // namespace generator

#endif
