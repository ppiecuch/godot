/*************************************************************************/
/*  RepeatShape.hpp                                                      */
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

#ifndef UUID_768F8730AFFB92752C688F5439A49976
#define UUID_768F8730AFFB92752C688F5439A49976

#include "Edge.hpp"
#include "ShapeVertex.hpp"
#include "utils.hpp"

namespace generator {

/**
 * Repeats the same shape a given number of time at given intervals.
 */
template <typename Shape>
class RepeatShape {
public:
	class Edges {
	public:
		bool done() const noexcept {
			return mIndex >= mRepeatShape->mInstances;
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
				mDelta += mRepeatShape->mVertexCount;
				mEdges = mRepeatShape->mShape.edges();
			}
		}

	private:
		const RepeatShape *mRepeatShape;

		typename EdgeGeneratorType<Shape>::Type mEdges;

		int mIndex;

		int mDelta;

		explicit Edges(const RepeatShape *repeatShape) noexcept : mRepeatShape{ repeatShape },
																  mEdges{ repeatShape->mShape.edges() },
																  mIndex{ repeatShape->mVertexCount > 0 ? 0 : repeatShape->mInstances },
																  mDelta{ 0 } {}

		int countEdges() const noexcept {
			if (mRepeatShape->mInstances < 1) return 0;

			return count(mRepeatShape->mShape.edges()) *
						   (mRepeatShape->mInstances - mIndex - 1) +
				   count(mEdges);
		}

		friend int count(const Edges &generator) noexcept {
			return generator.countEdges();
		}

		friend class RepeatShape;
	};

	class Vertices {
	public:
		bool done() const noexcept {
			return mIndex >= mRepeatShape->mInstances;
		}

		ShapeVertex generate() const {
			ShapeVertex temp = mVertices.generate();
			temp.position += mDelta;
			return temp;
		}

		void next() {
			mVertices.next();

			if (mVertices.done()) {
				++mIndex;
				mDelta += mRepeatShape->mDelta;
				mVertices = mRepeatShape->mShape.vertices();
			}
		}

	private:
		explicit Vertices(const RepeatShape *repeatShape) :
				mRepeatShape{ repeatShape },
				mVertices{ repeatShape->mShape.vertices() },
				mIndex{ repeatShape->mVertexCount > 0 ? 0 : repeatShape->mInstances },
				mDelta{} {}

		const RepeatShape *mRepeatShape;

		typename VertexGeneratorType<Shape>::Type mVertices;

		int mIndex;

		gml::dvec2 mDelta;

		int countVertices() const noexcept {
			if (mRepeatShape->mInstances < 1) return 0;

			return mRepeatShape->mVertexCount *
						   (mRepeatShape->mInstances - mIndex - 1) +
				   count(mVertices);
		}

		friend int count(const Vertices &generator) noexcept {
			return generator.countVertices();
		}

		friend class RepeatShape;
	};

	/// @param shape The shape to repeat.
	/// @param instances Number of times to repeat. If <1 an empty shape results.
	/// @param delta An offset aplied to each copy.
	explicit RepeatShape(Shape shape, int instances, const gml::dvec2 &delta) noexcept : mShape{ std::move(shape) },
																						 mInstances{ instances },
																						 mDelta{ delta },
																						 mVertexCount{ count(mShape.vertices()) } {}

	Edges edges() const noexcept {
		return Edges{ this };
	}

	Vertices vertices() const noexcept {
		return Vertices{ this };
	}

private:
	Shape mShape;

	int mInstances;

	gml::dvec2 mDelta;

	int mVertexCount;
};

template <typename Shape>
RepeatShape<Shape> repeatShape(Shape shape, int instances, const gml::dvec2 &delta) noexcept {
	return RepeatShape<Shape>{ std::move(shape), instances, delta };
}

} // namespace generator

#endif