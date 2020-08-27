/*************************************************************************/
/*  DodecahedronMesh.hpp                                                 */
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

// Copyright 2016 Markus Ilmola
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

#ifndef UUID_8B1E88033CA8420EA606B29D704E10EC
#define UUID_8B1E88033CA8420EA606B29D704E10EC

#include <array>
#include <memory>

#include "ConvexPolygonMesh.hpp"
#include "MeshVertex.hpp"
#include "Triangle.hpp"
#include "utils.hpp"

namespace generator {

/// A regular dodecahedron centered at origin with given radius.
/// Each face optionally subdivided along edges and/or radius.
/// @image html DodecahedronMesh.svg
class DodecahedronMesh {
private:
	class Triangles {
	public:
		bool done() const noexcept;
		Triangle generate() const;
		void next();

	private:
		const DodecahedronMesh *mMesh;

		int mFaceIndex;

		// Needs be a shared_ptr in order to make copy/move not to mess up the
		// internal pointer in mTriangles.
		std::shared_ptr<const ConvexPolygonMesh> mFaceMesh;

		typename TriangleGeneratorType<ConvexPolygonMesh>::Type mTriangles;

		explicit Triangles(const DodecahedronMesh &mesh) noexcept;

		friend class DodecahedronMesh;
	};

	class Vertices {
	public:
		bool done() const noexcept;
		MeshVertex generate() const;
		void next();

	private:
		const DodecahedronMesh *mMesh;

		int mFaceIndex;

		// Needs be a shared_ptr in order to make copy/move not to mess up the
		// internal pointer in mTriangles.
		std::shared_ptr<const ConvexPolygonMesh> mFaceMesh;

		typename VertexGeneratorType<ConvexPolygonMesh>::Type mVertices;

		explicit Vertices(const DodecahedronMesh &mesh) noexcept;

		friend class DodecahedronMesh;
	};

	double mRadius;

	int mSegments;

	int mRings;

	int mFaceVertexCount;

public:
	/// @param radius The radius of the enclosing sphere.
	/// @param segments The number segments along each edge. Should be >= 1.
	/// If <1 empty mesh is generated.
	/// @param rings The number of radial segments on each face. Should be >= 1.
	/// If <1 an empty mesh is generated.
	explicit DodecahedronMesh(
			double radius = 1.0, int segments = 1, int rings = 1) noexcept;

	Triangles triangles() const noexcept;

	Vertices vertices() const noexcept;
};

} // namespace generator

#endif
