/*************************************************************************/
/*  IcosahedronMesh.hpp                                                  */
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

#ifndef GENERATOR_ICOSAHEDRONMESH_HPP
#define GENERATOR_ICOSAHEDRONMESH_HPP

#include <array>
#include <memory>

#include "MeshVertex.hpp"
#include "Triangle.hpp"
#include "TriangleMesh.hpp"
#include "utils.hpp"

namespace generator {

/// Regular icosahedron centered at origin with given radius.
/// @image html IcosahedronMesh.svg
class IcosahedronMesh {
public:
	class Triangles {
	public:
		bool done() const noexcept;
		Triangle generate() const;
		void next();

	private:
		const IcosahedronMesh *mesh_;

		int i_;

		// Needs be a shared_ptr in order to make copy/move not to mess up the
		// internal pointer in triangles_.
		std::shared_ptr<const TriangleMesh> triangleMesh_;

		typename TriangleGeneratorType<TriangleMesh>::Type triangles_;

		Triangles(const IcosahedronMesh &mesh);

		friend class IcosahedronMesh;
	};

	class Vertices {
	public:
		bool done() const noexcept;
		MeshVertex generate() const;
		void next();

	private:
		const IcosahedronMesh *mesh_;

		int i_;

		// Needs be a shared_ptr in order to make copy/move not to mess up the
		// internal pointer in triangles_.
		std::shared_ptr<const TriangleMesh> triangleMesh_;

		typename VertexGeneratorType<TriangleMesh>::Type vertices_;

		Vertices(const IcosahedronMesh &mesh);

		friend class IcosahedronMesh;
	};

private:
	double radius_;

	int segments_;

	int faceVertexCount_;

public:
	/// @param radius The radius of the enclosing sphere.
	/// @param segments The number segments along each edge. Must be >= 1.
	IcosahedronMesh(double radius = 1.0, int segments = 1);

	Triangles triangles() const noexcept;

	Vertices vertices() const noexcept;
};

} // namespace generator

#endif
