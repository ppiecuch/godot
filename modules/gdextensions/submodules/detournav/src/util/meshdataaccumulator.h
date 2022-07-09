/*************************************************************************/
/*  meshdataaccumulator.h                                                */
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

#ifndef MESHDATAACCUMULATOR_H
#define MESHDATAACCUMULATOR_H

#include "core/math/transform.h"
#include "core/os/file_access.h"
#include "scene/resources/mesh.h"
#include "scene/3d/mesh_instance.h"

#include <vector>

// Gets all the vertices, faces, etc. from an ArrayMesh and combines it into a single set of data (vertices, indices, ...).
class MeshDataAccumulator {
	std::vector<float> _vertices;
	std::vector<int> _triangles;
	std::vector<float> _normals;

public:
	const float *getVerts() const; // Returns a pointer to all the vertices' floats.

	int getVertCount() const; // Returns the number of vertices.

	const int *getTris(); // Returns a pointer to all the triangles' indices.

	int getTriCount(); // Returns the number of triangles.

	const float *getNormals(); // Returns a pointer to all the triangles' normals.

	void save(FileAccessRef targetFile); // Store the mesh to the target file.
	bool load(FileAccessRef sourceFile); // Load the mesh from the source file.

	MeshDataAccumulator(MeshInstance *meshInstance);
	MeshDataAccumulator();
	~MeshDataAccumulator();
};

// ------------------------------------------------------------
inline const float *MeshDataAccumulator::getVerts() const { return _vertices.data(); }

inline int MeshDataAccumulator::getVertCount() const { return _vertices.size() / 3; }

inline const int *MeshDataAccumulator::getTris() { return _triangles.data(); }

inline int MeshDataAccumulator::getTriCount() { return _triangles.size() / 3; }

inline const float *MeshDataAccumulator::getNormals() { return _normals.data(); }

#endif // MESHDATAACCUMULATOR_H
