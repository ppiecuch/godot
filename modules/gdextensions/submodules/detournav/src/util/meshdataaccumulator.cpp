/*************************************************************************/
/*  meshdataaccumulator.cpp                                              */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "meshdataaccumulator.h"

#include "godotgeometryparser.h"

#include "core/variant.h"
#include "core/print_string.h"
#include "core/os/file_access.h"
#include "scene/resources/material.h"
#include "scene/resources/mesh.h"
#include "scene/resources/mesh_data_tool.h"

#define MDA_SAVE_VERSION 1

MeshDataAccumulator::MeshDataAccumulator(MeshInstance *meshInstance) {
	GodotGeometryParser parser;
	parser.getNodeVerticesAndIndices(meshInstance, _vertices, _triangles);

	print_verbose("Got vertices and triangles...");

	// Copy normals (we can't just copy them from the MeshDataTool since we operate on transformed values)
	// Code below mostly taken from recastnavigation sample
	size_t indexCount = _triangles.size();
	_normals.resize(indexCount);
	for (int j = 0; j < indexCount; j += 3) {
		const float *v0 = &_vertices[_triangles[j + 0] * 3];
		const float *v1 = &_vertices[_triangles[j + 1] * 3];
		const float *v2 = &_vertices[_triangles[j + 2] * 3];
		float e0[3], e1[3];
		for (int k = 0; k < 3; ++k) {
			e0[k] = v1[k] - v0[k];
			e1[k] = v2[k] - v0[k];
		}
		float *n = &_normals.data()[j];
		n[0] = e0[1] * e1[2] - e0[2] * e1[1];
		n[1] = e0[2] * e1[0] - e0[0] * e1[2];
		n[2] = e0[0] * e1[1] - e0[1] * e1[0];
		float d = Math::sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
		if (d > 0) {
			d = 1.0 / d;
			n[0] *= d;
			n[1] *= d;
			n[2] *= d;
		}
	}
	print_verbose("Got normals...");
}

MeshDataAccumulator::MeshDataAccumulator() { }

MeshDataAccumulator::~MeshDataAccumulator() { }

void MeshDataAccumulator::save(FileAccessRef &targetFile) {
	// Store version
	targetFile->store_16(MDA_SAVE_VERSION);
	// Store vertices
	targetFile->store_32(_vertices.size());
	for (int i = 0; i < _vertices.size(); ++i) {
		targetFile->store_float(_vertices[i]);
	}
	// Store triangles
	targetFile->store_32(_triangles.size());
	for (int i = 0; i < _triangles.size(); ++i) {
		targetFile->store_32(_triangles[i]);
	}
	// Store normals
	targetFile->store_32(_normals.size());
	for (int i = 0; i < _normals.size(); ++i) {
		targetFile->store_float(_normals[i]);
	}
}

bool MeshDataAccumulator::load(FileAccessRef &sourceFile) {
	// Load version
	int version = sourceFile->get_16();

	// Newest version
	if (version == MDA_SAVE_VERSION) {
		// Vertices
		int size = sourceFile->get_32();
		_vertices.resize(size);
		for (int i = 0; i < size; ++i) {
			_vertices[i] = sourceFile->get_float();
		}

		// Triangles
		size = sourceFile->get_32();
		_triangles.resize(size);
		for (int i = 0; i < size; ++i) {
			_triangles[i] = sourceFile->get_32();
		}

		// Normals
		size = sourceFile->get_32();
		_normals.resize(size);
		for (int i = 0; i < size; ++i) {
			_normals[i] = sourceFile->get_float();
		}
	} else {
		ERR_PRINT(vformat("MeshDataAccumulator: Unknown save version: %d", version));
		return false;
	}

	return true;
}
