/*************************************************************************/
/*  ScaleMesh.hpp                                                        */
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

#ifndef GENERATOR_SCALEMESH_HPP
#define GENERATOR_SCALEMESH_HPP

#include "TransformMesh.hpp"

namespace generator {

/// Scales a mesh (keeps normals unit length)
template <typename Mesh>
class ScaleMesh {
private:
	using Impl = TransformMesh<Mesh>;
	Impl transformMesh_;

public:
	/// @param mesh Source data mesh.
	/// @param scale Scale factors. Must not be zero!
	ScaleMesh(Mesh mesh, const gml::dvec3 &scale) :
			transformMesh_{
				std::move(mesh),
				[scale](MeshVertex &value) {
					value.position *= scale;
					value.normal = normalize(scale * value.normal);
				}
			} {}

	using Triangles = typename Impl::Triangles;

	Triangles triangles() const noexcept { return transformMesh_.triangles(); }

	using Vertices = typename Impl::Vertices;

	Vertices vertices() const noexcept { return transformMesh_.vertices(); }
};

template <typename Mesh>
ScaleMesh<Mesh> scaleMesh(Mesh mesh, const gml::dvec3 &delta) {
	return ScaleMesh<Mesh>{ std::move(mesh), delta };
}

} // namespace generator

#endif
