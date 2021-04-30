/*************************************************************************/
/*  DodecahedronMesh.cpp                                                 */
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

#include "generator/DodecahedronMesh.hpp"
#include "generator/Iterator.hpp"

#include <array>
#include <memory>

using namespace generator;

static const double a = (1.0 + std::sqrt(5.0)) / 2.0;

static const std::array<gml::dvec3, 20> vertices{ { { 1.0, 1.0, -1.0 },
		{ -1.0, 1.0, -1.0 },
		{ 1.0, -1.0, -1.0 },
		{ -1.0, -1.0, -1.0 },
		{ 1.0, 1.0, 1.0 },
		{ -1.0, 1.0, 1.0 },
		{ 1.0, -1.0, 1.0 },
		{ -1.0, -1.0, 1.0 },

		{ 0.0, 1.0 / a, -a },
		{ 0.0, -1.0 / a, -a },
		{ 0.0, 1.0 / a, a },
		{ 0.0, -1.0 / a, a },

		{ 1.0 / a, a, 0.0 },
		{ -1.0 / a, a, 0.0 },
		{ 1.0 / a, -a, 0.0 },
		{ -1.0 / a, -a, 0.0 },

		{ a, 0.0, -1.0 / a },
		{ -a, 0.0, -1.0 / a },
		{ a, 0.0, 1.0 / a },
		{ -a, 0.0, 1.0 / a } } };

static const std::array<std::array<int, 5>, 12> polygons{ {
		{ { 0, 12, 4, 18, 16 } }, // 0
		{ { 18, 6, 14, 2, 16 } }, // 1
		{ { 4, 10, 11, 6, 18 } }, // 2
		{ { 11, 7, 15, 14, 6 } }, // 3
		{ { 7, 11, 10, 5, 19 } }, // 4
		{ { 17, 3, 15, 7, 19 } }, // 5
		{ { 9, 2, 14, 15, 3 } }, // 6
		{ { 1, 8, 9, 3, 17 } }, // 7
		{ { 10, 4, 12, 13, 5 } }, // 8
		{ { 5, 13, 1, 17, 19 } }, // 9
		{ { 0, 8, 1, 13, 12 } }, // 10
		{ { 0, 16, 2, 9, 8 } } // 11
} };

static std::vector<gml::dvec3> makeVertices(int faceIndex) noexcept {
	std::vector<gml::dvec3> result(5);
	for (int i = 0; i < 5; ++i) {
		result.at(i) = gml::normalize(vertices.at(polygons.at(faceIndex)[i]));
	}
	return result;
}

DodecahedronMesh::Triangles::Triangles(const DodecahedronMesh &mesh) noexcept : mMesh{ &mesh },
																				mFaceIndex{ 0 },
																				mFaceMesh{ std::make_shared<ConvexPolygonMesh>(makeVertices(0), mMesh->mSegments, mMesh->mRings) },
																				mTriangles{ mFaceMesh->triangles() } {}

bool DodecahedronMesh::Triangles::done() const noexcept {
	return mFaceIndex == ::polygons.size();
}

Triangle DodecahedronMesh::Triangles::generate() const {
	if (done()) throw std::out_of_range("Done!");

	Triangle triangle = mTriangles.generate();

	const int base = mFaceIndex * mMesh->mFaceVertexCount;

	triangle.vertices[0] += base;
	triangle.vertices[1] += base;
	triangle.vertices[2] += base;

	return triangle;
}

void DodecahedronMesh::Triangles::next() {
	if (done()) throw std::out_of_range("Done!");

	mTriangles.next();

	if (mTriangles.done()) {
		++mFaceIndex;

		if (!done()) {
			mFaceMesh = std::make_shared<ConvexPolygonMesh>(
					makeVertices(mFaceIndex),
					mMesh->mSegments);

			mTriangles = mFaceMesh->triangles();
		}
	}
}

DodecahedronMesh::Vertices::Vertices(const DodecahedronMesh &mesh) noexcept : mMesh{ &mesh },
																			  mFaceIndex{ 0 },
																			  mFaceMesh{ std::make_shared<ConvexPolygonMesh>(
																					  makeVertices(0),
																					  mMesh->mSegments,
																					  mMesh->mRings) },
																			  mVertices{ mFaceMesh->vertices() } {
}

bool DodecahedronMesh::Vertices::done() const noexcept {
	return mFaceIndex == ::polygons.size();
}

MeshVertex DodecahedronMesh::Vertices::generate() const {

	MeshVertex vertex = mVertices.generate();

	vertex.position *= mMesh->mRadius;

	return vertex;
}

void DodecahedronMesh::Vertices::next() {
	mVertices.next();

	if (mVertices.done()) {
		++mFaceIndex;

		if (!done()) {
			mFaceMesh = std::make_shared<ConvexPolygonMesh>(
					makeVertices(mFaceIndex),
					mMesh->mSegments,
					mMesh->mRings);

			mVertices = mFaceMesh->vertices();
		}
	}
}

DodecahedronMesh::DodecahedronMesh(double radius, int segments, int rings) noexcept : mRadius{ radius },
																					  mSegments{ segments },
																					  mRings{ rings },
																					  mFaceVertexCount{ count(ConvexPolygonMesh{ 1.0, 5u, segments, rings }.vertices()) } {
}

DodecahedronMesh::Triangles DodecahedronMesh::triangles() const noexcept {
	return Triangles{ *this };
}

DodecahedronMesh::Vertices DodecahedronMesh::vertices() const noexcept {
	return Vertices{ *this };
}
