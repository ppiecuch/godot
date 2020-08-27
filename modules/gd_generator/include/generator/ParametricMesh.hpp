/*************************************************************************/
/*  ParametricMesh.hpp                                                   */
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

#ifndef GENERATOR_PARAMETRICMESH_HPP
#define GENERATOR_PARAMETRICMESH_HPP

#include <functional>

#include "MeshVertex.hpp"
#include "Triangle.hpp"

namespace generator {

/// A mesh with values evaluated using a callback function.
class ParametricMesh {
public:
	class Triangles {
	public:
		Triangle generate() const;
		bool done() const noexcept;
		void next();

	private:
		Triangles(const ParametricMesh &mesh);

		const ParametricMesh *mesh_;

		gml::ivec2 i_;

		bool even_;

		friend class ParametricMesh;
	};

	class Vertices {
	public:
		MeshVertex generate() const;
		bool done() const noexcept;
		void next();

	private:
		Vertices(const ParametricMesh &mesh);

		const ParametricMesh *mesh_;

		gml::ivec2 i_;

		friend class ParametricMesh;
	};

	/// @param eval A callback that returns a MeshVertex for a given value.
	/// @param segments The number of segments along the surface.
	/// Both should be >= 1. If either is zero an empty mesh is generated.
	explicit ParametricMesh(
			std::function<MeshVertex(const gml::dvec2 &t)> eval,
			const gml::ivec2 &segments = { 16, 16 }) noexcept;

	Triangles triangles() const noexcept;

	Vertices vertices() const noexcept;

private:
	std::function<MeshVertex(const gml::dvec2 &t)> eval_;

	gml::ivec2 segments_;

	gml::dvec2 delta_;
};

} // namespace generator

#endif
