/*************************************************************************/
/*  godotgeometryparser.h                                                */
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

#ifndef GDGEOMETRYPARSER_H
#define GDGEOMETRYPARSER_H

#include "core/variant.h"
#include "core/math/vector2.h"
#include "core/math/transform.h"
#include "scene/resources/mesh.h"
#include "scene/3d/mesh_instance.h"

#include <vector>

/// Will parse a passed MeshInstance
class GodotGeometryParser {
	void addVertex(const Vector3 &p_vec3, std::vector<float> &_verticies);
	void addMesh(const Ref<ArrayMesh> &p_mesh, const Transform &p_xform, std::vector<float> &p_vertices, std::vector<int> &p_indices);
	void parseGeometry(MeshInstance *meshInstance, std::vector<float> &p_vertices, std::vector<int> &p_indices);

public:
	void getNodeVerticesAndIndices(MeshInstance *meshInstance, std::vector<float> &outVertices, std::vector<int> &outIndices);

	GodotGeometryParser();
	~GodotGeometryParser();
};

inline void GodotGeometryParser::addVertex(const Vector3 &p_vec3, std::vector<float> &p_vertices) {
	p_vertices.emplace_back(p_vec3.x);
	p_vertices.emplace_back(p_vec3.y);
	p_vertices.emplace_back(p_vec3.z);
}

#endif // GDGEOMETRYPARSER_H
