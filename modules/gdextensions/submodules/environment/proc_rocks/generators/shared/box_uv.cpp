/**************************************************************************/
/*  box_uv.cpp                                                            */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "box_uv.h"

#include "servers/visual_server.h"

// =========================================================================
// Low-poly conversion — unindex mesh for flat shading
// =========================================================================

Ref<ArrayMesh> rock_studio_make_low_poly(const Ref<ArrayMesh> &p_mesh) {
	ERR_FAIL_COND_V(p_mesh.is_null() || p_mesh->get_surface_count() == 0, Ref<ArrayMesh>());

	Array src = p_mesh->surface_get_arrays(0);
	Vector<Vector3> old_verts = src[VS::ARRAY_VERTEX];
	Vector<int> old_indices = src[VS::ARRAY_INDEX];

	if (old_indices.size() == 0) {
		// Already unindexed
		return p_mesh;
	}

	Vector<Vector3> vertices;
	Vector<Vector3> normals;
	vertices.resize(old_indices.size());
	normals.resize(old_indices.size());

	for (int i = 0; i < old_indices.size(); i++) {
		vertices.write[i] = old_verts[old_indices[i]];
	}

	// Compute flat normals per triangle
	for (int i = 0; i + 2 < vertices.size(); i += 3) {
		Vector3 n = (vertices[i + 1] - vertices[i]).cross(vertices[i + 2] - vertices[i]).normalized();
		normals.write[i] = n;
		normals.write[i + 1] = n;
		normals.write[i + 2] = n;
	}

	Array arrays;
	arrays.resize(VS::ARRAY_MAX);
	arrays[VS::ARRAY_VERTEX] = vertices;
	arrays[VS::ARRAY_NORMAL] = normals;

	Ref<ArrayMesh> result;
	result.instance();
	result->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
	return result;
}

// =========================================================================
// Box UV projection
// =========================================================================

int rock_studio_get_box_dir(const Vector3 &p_normal) {
	real_t ax = Math::abs(p_normal.x);
	real_t ay = Math::abs(p_normal.y);
	real_t az = Math::abs(p_normal.z);
	if (ax > ay && ax > az) {
		return p_normal.x < 0 ? -1 : 1;
	} else if (ay > az) {
		return p_normal.y < 0 ? -2 : 2;
	}
	return p_normal.z < 0 ? -3 : 3;
}

Vector2 rock_studio_get_box_uv(const Vector3 &p_vertex, int p_box_dir) {
	real_t s = p_box_dir < 0 ? -1.0 : 1.0;
	switch (ABS(p_box_dir)) {
		case 1:
			return Vector2(p_vertex.z * s, p_vertex.y);
		case 2:
			return Vector2(p_vertex.x, p_vertex.z * s);
		case 3:
			return Vector2(p_vertex.x * -s, p_vertex.y);
	}
	return Vector2();
}

void rock_studio_box_uv(Ref<ArrayMesh> p_mesh) {
	ERR_FAIL_COND(p_mesh.is_null() || p_mesh->get_surface_count() == 0);

	Array src = p_mesh->surface_get_arrays(0);
	Vector<Vector3> vertices = src[VS::ARRAY_VERTEX];
	Vector<Vector3> normals = src[VS::ARRAY_NORMAL];

	if (vertices.size() == 0 || normals.size() == 0) {
		return;
	}

	Vector<Vector2> uvs;
	uvs.resize(vertices.size());

	// Assign UV per-triangle based on face normal direction
	for (int i = 0; i + 2 < vertices.size(); i += 3) {
		Vector3 face_normal = (vertices[i + 1] - vertices[i]).cross(vertices[i + 2] - vertices[i]).normalized();
		int box_dir = rock_studio_get_box_dir(face_normal);
		uvs.write[i] = rock_studio_get_box_uv(vertices[i], box_dir);
		uvs.write[i + 1] = rock_studio_get_box_uv(vertices[i + 1], box_dir);
		uvs.write[i + 2] = rock_studio_get_box_uv(vertices[i + 2], box_dir);
	}

	// Rebuild surface with UVs
	Array arrays;
	arrays.resize(VS::ARRAY_MAX);
	arrays[VS::ARRAY_VERTEX] = vertices;
	arrays[VS::ARRAY_NORMAL] = normals;
	arrays[VS::ARRAY_TEX_UV] = uvs;

	p_mesh->surface_remove(0);
	p_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
}
