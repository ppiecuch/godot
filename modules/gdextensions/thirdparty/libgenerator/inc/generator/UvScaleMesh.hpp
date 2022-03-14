
// Copyright 2015 Markus Ilmola
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

#ifndef GENERATOR_UVSCALEMESH_HPP
#define GENERATOR_UVSCALEMESH_HPP

#include "TransformMesh.hpp"

namespace generator {

/// Flips texture coordinate axis directions.
template <typename Mesh>
class UvScaleMesh {
private:
	using Impl = TransformMesh<Mesh>;
	Impl transformMesh_;

public:
	/// @param mesh Passthrought source data mesh.
	UvScaleMesh(Mesh mesh) : transformMesh_{ std::move(mesh) } {}

	/// @param mesh Source data mesh.
	/// @param u Flip u
	/// @param v Flip v
	UvScaleMesh(Mesh mesh, const gml::dvec2 &delta) :
		transformMesh_{
			std::move(mesh),
			[delta](MeshVertex &vertex) {
				vertex.texCoord *= delta;
			}
		} {}

	using Triangles = typename Impl::Triangles;

	Triangles triangles() const noexcept { return transformMesh_.triangles(); }

	using Vertices = typename Impl::Vertices;

	Vertices vertices() const noexcept { return transformMesh_.vertices(); }
};

template <typename Mesh>
UvScaleMesh<Mesh> uvScaleMesh(Mesh mesh, const gml::dvec2 &delta) {
	return UvScaleMesh<Mesh>{ std::move(mesh), delta };
}

} // namespace generator

#endif
