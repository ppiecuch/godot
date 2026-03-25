/**************************************************************************/
/*  rock_studio.cpp                                                       */
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

#include "rock_studio.h"

#include "core/math/quick_hull.h"
#include "servers/visual_server.h"

// =========================================================================
// Point generators
// =========================================================================

Vector<Vector3> rock_studio_points_cube(int p_count, real_t p_width, real_t p_height, real_t p_depth) {
	Vector<Vector3> points;
	points.resize(p_count);
	for (int i = 0; i < p_count; i++) {
		points.write[i] = Vector3(
				Math::random(-p_depth * 0.5, p_depth * 0.5),
				Math::random(-p_height * 0.5, p_height * 0.5),
				Math::random(-p_width * 0.5, p_width * 0.5));
	}
	return points;
}

Vector<Vector3> rock_studio_points_sphere(int p_count, real_t p_radius) {
	Vector<Vector3> points;
	points.resize(p_count);
	for (int i = 0; i < p_count; i++) {
		// Uniform random point inside unit sphere via rejection sampling
		Vector3 p;
		do {
			p = Vector3(
					Math::random(-1.0, 1.0),
					Math::random(-1.0, 1.0),
					Math::random(-1.0, 1.0));
		} while (p.length_squared() > 1.0);
		points.write[i] = p * p_radius;
	}
	return points;
}

Vector<Vector3> rock_studio_points_crystal(int p_count, bool p_tetragonal, bool p_one_sided, real_t p_base_width, real_t p_base_height, real_t p_tip_protrusion, real_t p_tip_flatness) {
	Vector<Vector3> points;

	// Base points
	if (p_tetragonal) {
		for (int i = 0; i < p_count; i++) {
			points.push_back(Vector3(
					Math::random(-p_base_height, p_base_height),
					Math::random(-p_base_width, p_base_width),
					Math::random(-p_base_width, p_base_width)));
		}
	} else {
		for (int i = 0; i < p_count; i++) {
			points.push_back(Vector3(
					Math::random(-p_base_height, p_base_height),
					Math::random(-p_base_width * 1.732, p_base_width * 1.732),
					Math::random(-p_base_width, p_base_width)));
		}
		for (int i = 0; i < p_count; i++) {
			points.push_back(Vector3(
					Math::random(-p_base_height, p_base_height),
					0,
					Math::random(-p_base_width * 1.732, p_base_width * 1.732)));
		}
	}

	// Tip points
	real_t flat = p_tip_flatness / 10.0;
	if (p_one_sided) {
		for (int i = 0; i < p_count; i++) {
			points.push_back(Vector3(
					Math::random(-p_base_height, p_base_height + p_tip_protrusion),
					Math::random(-p_base_width * flat, p_base_width * flat),
					Math::random(-p_base_width * 1.732 * flat, p_base_width * 1.732 * flat)));
		}
	} else {
		for (int i = 0; i < p_count; i++) {
			points.push_back(Vector3(
					Math::random(-(p_base_height - p_tip_protrusion), p_base_height - p_tip_protrusion),
					Math::random(-p_base_width * flat, p_base_width * flat),
					Math::random(-p_base_width * 1.732 * flat, p_base_width * 1.732 * flat)));
		}
	}

	return points;
}

// =========================================================================
// Mesh creation — convex hull from random points
// =========================================================================

Ref<ArrayMesh> rock_studio_create_mesh(const Vector<Vector3> &p_points) {
	// Use Godot's built-in convex hull via QuickHull
	Geometry::MeshData mesh_data;
	Error err = QuickHull::build(p_points, mesh_data);

	if (err != OK || mesh_data.faces.size() == 0) {
		WARN_PRINT("rock_studio: convex hull generation failed.");
		return Ref<ArrayMesh>();
	}

	// Convert MeshData to ArrayMesh
	Vector<Vector3> vertices;
	Vector<Vector3> normals;
	Vector<int> indices;

	for (int f = 0; f < mesh_data.faces.size(); f++) {
		const Geometry::MeshData::Face &face = mesh_data.faces[f];
		Vector3 normal = face.plane.normal;

		// Fan triangulation of face polygon
		for (int j = 1; j + 1 < face.indices.size(); j++) {
			int i0 = face.indices[0];
			int i1 = face.indices[j];
			int i2 = face.indices[j + 1];

			int base = vertices.size();
			vertices.push_back(mesh_data.vertices[i0]);
			vertices.push_back(mesh_data.vertices[i1]);
			vertices.push_back(mesh_data.vertices[i2]);
			normals.push_back(normal);
			normals.push_back(normal);
			normals.push_back(normal);
			indices.push_back(base);
			indices.push_back(base + 1);
			indices.push_back(base + 2);
		}
	}

	Array arrays;
	arrays.resize(VS::ARRAY_MAX);
	arrays[VS::ARRAY_VERTEX] = vertices;
	arrays[VS::ARRAY_NORMAL] = normals;
	arrays[VS::ARRAY_INDEX] = indices;

	Ref<ArrayMesh> mesh;
	mesh.instance();
	mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
	return mesh;
}

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
