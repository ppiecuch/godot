/*************************************************************************/
/*  RepeatMesh.hpp                                                       */
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

#ifndef UUID_34107193C252FF8CC2A084B47CFB15D4
#define UUID_34107193C252FF8CC2A084B47CFB15D4

#include "MeshVertex.hpp"
#include "Triangle.hpp"
#include "utils.hpp"

namespace generator {

/**
 * Repeats the same mesh a given number of time at given intervals.
 */
template <typename Mesh>
class RepeatMesh {
public:
	class Triangles {
	public:
		bool done() const noexcept {
			return mIndex >= mRepeatMesh->mInstances;
		}

		Triangle generate() const {
			Triangle temp = mTriangles.generate();
			temp.vertices += mDelta;
			return temp;
		}

		void next() noexcept {
			mTriangles.next();

			if (mTriangles.done()) {
				++mIndex;
				mDelta += mRepeatMesh->mVertexCount;
				mTriangles = mRepeatMesh->mMesh.triangles();
			}
		}

	private:
		const RepeatMesh *mRepeatMesh;

		typename TriangleGeneratorType<Mesh>::Type mTriangles;

		int mIndex;

		int mDelta;

		explicit Triangles(const RepeatMesh *repeatMesh) noexcept :
				mRepeatMesh{ repeatMesh },
				mTriangles{ repeatMesh->mMesh.triangles() },
				mIndex{ repeatMesh->mVertexCount > 0 ? 0 : repeatMesh->mInstances },
				mDelta{ 0 } {}

		int countTriangles() const noexcept {
			if (mRepeatMesh->mInstances < 1)
				return 0;

			return count(mRepeatMesh->mMesh.triangles()) *
						   (mRepeatMesh->mInstances - mIndex - 1) +
				   count(mTriangles);
		}

		friend int count(const Triangles &generator) noexcept {
			return generator.countTriangles();
		}

		friend class RepeatMesh;
	};

	class Vertices {
	public:
		bool done() const noexcept {
			return mIndex >= mRepeatMesh->mInstances;
		}

		MeshVertex generate() const {
			MeshVertex temp = mVertices.generate();
			temp.position += mDelta;
			return temp;
		}

		void next() {
			mVertices.next();

			if (mVertices.done()) {
				++mIndex;
				mDelta += mRepeatMesh->mDelta;
				mVertices = mRepeatMesh->mMesh.vertices();
			}
		}

	private:
		explicit Vertices(const RepeatMesh *repeatMesh) :
				mRepeatMesh{ repeatMesh },
				mVertices{ repeatMesh->mMesh.vertices() },
				mIndex{ repeatMesh->mVertexCount > 0 ? 0 : repeatMesh->mInstances },
				mDelta{} {}

		const RepeatMesh *mRepeatMesh;

		typename VertexGeneratorType<Mesh>::Type mVertices;

		int mIndex;

		gml::dvec3 mDelta;

		int countVertices() const noexcept {
			if (mRepeatMesh->mInstances < 1)
				return 0;

			return mRepeatMesh->mVertexCount *
						   (mRepeatMesh->mInstances - mIndex - 1) +
				   count(mVertices);
		}

		friend int count(const Vertices &generator) noexcept {
			return generator.countVertices();
		}

		friend class RepeatMesh;
	};

	/// @param mesh The mesh to repeat.
	/// @param instances Number of times to repeat. If <1 an empty mesh results.
	/// @param delta An offset aplied to each copy.
	explicit RepeatMesh(Mesh mesh, int instances, const gml::dvec3 &delta) noexcept :
			mMesh{ std::move(mesh) },
			mInstances{ instances },
			mDelta{ delta },
			mVertexCount{ count(mMesh.vertices()) } {}

	Triangles triangles() const noexcept {
		return Triangles{ this };
	}

	Vertices vertices() const noexcept {
		return Vertices{ this };
	}

private:
	Mesh mMesh;

	int mInstances;

	gml::dvec3 mDelta;

	int mVertexCount;
};

template <typename Mesh>
RepeatMesh<Mesh> repeatMesh(Mesh mesh, int instances, const gml::dvec3 &delta) noexcept {
	return RepeatMesh<Mesh>{ std::move(mesh), instances, delta };
}

} // namespace generator

#endif
