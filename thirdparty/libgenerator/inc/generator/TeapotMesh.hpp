/*************************************************************************/
/*  TeapotMesh.hpp                                                       */
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

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

#ifndef UUID_88C28B4C2B304C399A34364D99D2EC26
#define UUID_88C28B4C2B304C399A34364D99D2EC26

#include <array>
#include <memory>

#include "BezierMesh.hpp"
#include "MeshVertex.hpp"
#include "Triangle.hpp"
#include "utils.hpp"

namespace generator {

/// The Utah Teapot.
/// https://en.wikipedia.org/wiki/Utah_teapot
/// @image html TeapotMesh.svg
class TeapotMesh {
public:
	class Triangles {
	public:
		bool done() const noexcept;
		Triangle generate() const;
		void next();

	private:
		const TeapotMesh *mMesh;

		int mIndex;

		std::shared_ptr<const BezierMesh<4, 4>> mPatchMesh;

		typename TriangleGeneratorType<BezierMesh<4, 4>>::Type mTriangles;

		explicit Triangles(const TeapotMesh &mesh) noexcept;

		friend class TeapotMesh;
	};

	class Vertices {
	public:
		bool done() const noexcept;
		MeshVertex generate() const;
		void next();

	private:
		const TeapotMesh *mMesh;

		int mIndex;

		// Needs be a shared_ptr in order to make copy/move not to mess up the
		// internal pointer in mTriangles.
		std::shared_ptr<const BezierMesh<4, 4>> mPatchMesh;

		typename VertexGeneratorType<BezierMesh<4, 4>>::Type mVertices;

		explicit Vertices(const TeapotMesh &mesh) noexcept;

		friend class TeapotMesh;
	};

	/// Generates the Utah teapot using the original data.
	/// The lid is pointing towards the z axis and the spout towards the x axis.
	/// @param segments The number segments along each patch. Should be >= 1.
	/// If zero empty mesh is generated.
	explicit TeapotMesh(int segments = 8) noexcept;

	Triangles triangles() const noexcept;

	Vertices vertices() const noexcept;

private:
	int mSegments;

	int mPatchVertexCount;
};

} // namespace generator

#endif
