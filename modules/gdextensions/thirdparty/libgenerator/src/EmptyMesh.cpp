
// Copyright 2015 Markus Ilmola
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

#include "generator/EmptyMesh.hpp"

#include "common/gd_core.h"

using namespace generator;

EmptyMesh::Triangles::Triangles() {}

Triangle EmptyMesh::Triangles::generate() const {
	ERR_THROW_V(std::out_of_range("Called generate on an EmptyMesh!"), Triangle());
}

bool EmptyMesh::Triangles::done() const noexcept {
	return true;
}

void EmptyMesh::Triangles::next() {
	ERR_THROW(std::out_of_range("Called next on an EmptyMesh!"));
}

EmptyMesh::Vertices::Vertices() {}

MeshVertex EmptyMesh::Vertices::generate() const {
	ERR_THROW_V(std::out_of_range("Called generate on an EmptyMesh!"), MeshVertex());
}

bool EmptyMesh::Vertices::done() const noexcept {
	return true;
}

void EmptyMesh::Vertices::next() {
	ERR_THROW(std::out_of_range("Called next on an EmptyMesh!"));
}

EmptyMesh::EmptyMesh() {}

EmptyMesh::Triangles EmptyMesh::triangles() const noexcept {
	return {};
}

EmptyMesh::Vertices EmptyMesh::vertices() const noexcept {
	return {};
}
