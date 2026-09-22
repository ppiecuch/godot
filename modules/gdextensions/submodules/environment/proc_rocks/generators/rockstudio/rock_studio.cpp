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

// Low-poly conversion and box UV projection live in generators/shared/box_uv.*
// (shared with the Method 3 noise pipeline) — see rock_studio.h.
