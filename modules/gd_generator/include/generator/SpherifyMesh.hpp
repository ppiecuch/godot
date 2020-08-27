/*************************************************************************/
/*  SpherifyMesh.hpp                                                     */
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

#ifndef SPHERIFYMESH_HPP
#define SPHERIFYMESH_HPP

#include "TransformMesh.hpp"

namespace generator {

/// Projects vertices on a sphere centered at origin.
template <typename Mesh>
class SpherifyMesh {
private:
	using Impl = TransformMesh<Mesh>;
	Impl transformMesh_;

public:
	SpherifyMesh(Mesh mesh, double radius, double factor) :
			transformMesh_{
				std::move(mesh),
				[radius, factor](MeshVertex &value) {
					value.position = gml::mix(
							value.position,
							radius * gml::normalize(value.position),
							factor);
					value.normal = gml::normalize(gml::mix(
							value.normal,
							gml::normalize(value.position),
							factor));
				}
			} {}

	using Triangles = typename Impl::Triangles;

	Triangles triangles() const noexcept { return transformMesh_.triangles(); }

	using Vertices = typename Impl::Vertices;

	Vertices vertices() const noexcept { return transformMesh_.vertices(); }
};

template <typename Mesh>
SpherifyMesh<Mesh> spherifyMesh(Mesh mesh, double radius, double factor) {
	return SpherifyMesh<Mesh>{ std::move(mesh), radius, factor };
}

} // namespace generator

#endif
