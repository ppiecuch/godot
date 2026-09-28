/**************************************************************************/
/*  gen_rock.cpp                                                          */
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

#include "gen_rock.h"

#include "../shared/box_uv.h"
#include "../shared/rock_header.h"
#include "common/gd_core.h"
#include "core/math/math_funcs.h"

GenRock::GenRock(real_t width, real_t height, real_t depth, int steps) :
		m_Width(width),
		m_Height(height),
		m_Depth(depth),
		m_Steps(steps),
		m_NumVertices(0),
		m_NumIndices(0) {}

GenRock::~GenRock(void) {
	m_VecGeom.clear();
	m_VecIndices.clear();
}

// Build icosphere
void GenRock::BuildIco() {
	int subdivisions = m_Steps;
	auto lists = MakeIcosphere(subdivisions);
	auto vertices = lists.first;
	auto indices = lists.second;

	// Subdivide triangles + add new vertices to buffer
	for (int i = 0; i < vertices.size(); i++) {
		auto vertice = vertices[i];
		auto vertVector = vertice.normalized();
		auto vert = vertVector;

		// real_t r = Math::sqrt(vertice.x * vertice.x + vertice.y * vertice.y + vertice.z * vertice.z);
		// real_t theta = Math::acos(vertice.y / r); // lat
		// real_t phi = Math::atan(vertice.x / vertice.z); // long

		auto newVert = vert * Vector3(m_Width, m_Height, m_Depth);

		auto normalVector = newVert.normalized();
		auto normal = normalVector.normalized();

		auto texcoord = UVFromVector3(newVert);
		if (texcoord.y == 0) {
			m_NorthIdx.insert(i);
		}
		if (texcoord.y == 1) {
			m_SouthIdx.insert(i);
		}
		VertexRock base;
		base.Position = newVert;
		base.Normal = normal;
		base.Tangent = Vector3(0, 0, 0);
		base.TexCoord = texcoord;

		m_VecGeom.push_back(base);
	}
	m_NumVertices = m_VecGeom.Position.size();

	for (const auto &indice : indices) { // Set indices
		m_VecIndices.push_back(indice.vertex[0]);
		m_VecIndices.push_back(indice.vertex[1]);
		m_VecIndices.push_back(indice.vertex[2]);
	}
	m_NumIndices = m_VecIndices.size();
}

// Convert icosphere into 'rock'
void GenRock::BuildRock() {
	// Influence radius for each plane's falloff: how far (in world units) from a plane's
	// own contact point its flattening effect reaches. Reuses the same formula Expand()
	// computes one function down -- a direct, correct stand-in for what the original
	// (buggy) antipodal-point "diameter" computation was trying to approximate; see
	// memo.md's "Bugs Fixed" for why that computation never actually measured the
	// ellipsoid's diameter.
	const real_t averageRadius = (m_Width + m_Height + m_Depth) / 3.0;

	// Flatten by 'planes'
	for (uint32_t plane = 0; plane < m_MaxPlanes; plane++) {
		// Random point on the unit sphere -- rejection sampling avoids the corner-bias a
		// naive (theta, phi) parametrization would have, and needs no degree/radian
		// conversion (unlike the angle-based approach this replaces).
		Vector3 unitDir;
		do {
			unitDir = Vector3(
					Math::randf() * 2.0 - 1.0,
					Math::randf() * 2.0 - 1.0,
					Math::randf() * 2.0 - 1.0);
		} while (unitDir.length_squared() < 0.0001 || unitDir.length_squared() > 1.0);
		unitDir.normalize();

		// Plane origin: a point on the mesh's own ellipsoid surface in that direction,
		// pulled inward by up to m_MaxOffsetPercent% toward the center -- same "how deep
		// can a cut reach" knob as before, unchanged.
		const real_t offset = Math::rand() % MAX(1, int(m_MaxOffsetPercent));
		Vector3 originPlane = unitDir * Vector3(m_Width, m_Height, m_Depth) * ((100.0 - offset) / 100.0);
		Vector3 normal = unitDir;

		// Flatten vertices onto plane
		for (uint32_t i = 0; i < m_NumVertices; i++) {
			// Check if vertice is in front of the plane
			auto vertice = m_VecGeom[i];
			auto point = vertice.Position;
			auto vecP = point - originPlane;
			auto dist = vec3_dot(vecP, normal); // signed distance above the plane
			if (dist < 0) { // dont proceed this one if dist is negative == more then 90 degree
				continue;
			}
			// Project on plane
			const auto projectedPoint = point - dist * normal;

			// Falloff: 1 at the plane's own contact point (pulls the vertex all the way
			// onto the plane), fading to 0 at the influence radius (vertex untouched) --
			// unlike the previous strength formula, this only ever pulls the vertex
			// *toward* the plane, never past it, so it produces a real flat facet instead
			// of a smooth outward bump.
			const auto distToCenter = LengthBetweenPoints(projectedPoint, originPlane);
			const auto falloff = CLAMP(1.0 - distToCenter / averageRadius, 0.0, 1.0);

			// Update vertice
			m_VecGeom.Position[i] = point - falloff * dist * normal;
			m_VecGeom.Normal[i] = normal;
		}
	}
}

// Push vertices outwards to counter overlap
void GenRock::Expand() {
	real_t averageRadius = (m_Width + m_Height + m_Depth) / 3.0;
	for (uint32_t i = 0; i < m_NumIndices; i += 3) {
		const auto &idx0 = m_VecIndices[(i + 0) % m_NumIndices];
		const auto &idx1 = m_VecIndices[(i + 1) % m_NumIndices];
		const auto &idx2 = m_VecIndices[(i + 2) % m_NumIndices];

		const auto &v0 = m_VecGeom[idx0];
		const auto &v1 = m_VecGeom[idx1];
		const auto &v2 = m_VecGeom[idx2];

		const auto normal = ComputeNormal(v0.Position, v1.Position, v2.Position); // Push all vertices out by every plane

		m_VecGeom.Position[idx0] = v0.Position + normal * averageRadius / 100.;
		m_VecGeom.Position[idx1] = v1.Position + normal * averageRadius / 100.;
		m_VecGeom.Position[idx2] = v2.Position + normal * averageRadius / 100.;
	}
}

// Build normals
void GenRock::BuildNormals() {
	for (size_t idx = 0; idx + 2 < m_VecIndices.size(); idx += 3) {
		const int &idx0 = m_VecIndices[idx + 0];
		const int &idx1 = m_VecIndices[idx + 1];
		const int &idx2 = m_VecIndices[idx + 2];

		const auto normal = ComputeNormal(m_VecGeom.Position[idx0], m_VecGeom.Position[idx1], m_VecGeom.Position[idx2]);

		m_VecGeom.Normal[idx0] = m_VecGeom.Normal[idx0] + normal;
		m_VecGeom.Normal[idx1] = m_VecGeom.Normal[idx1] + normal;
		m_VecGeom.Normal[idx2] = m_VecGeom.Normal[idx2] + normal;
	}
	for (size_t i = 0; i < m_VecGeom.Normal.size(); i++) {
		m_VecGeom.Normal[i] = m_VecGeom.Normal[i].normalized();
	}
}

// Correct uv seams
void GenRock::CorrectUV() {
	// Find seam vertices
	uint32_t countExtraVerts = 0;
	Set<uint32_t> duplicatesIdx;
	for (int i = 0; i < m_NumIndices; i += 3) {
		// Data
		const auto &idx0 = m_VecIndices[i % m_NumIndices];
		const auto &idx1 = m_VecIndices[(i + 1) % m_NumIndices];
		const auto &idx2 = m_VecIndices[(i + 2) % m_NumIndices];

		const auto &v0 = m_VecGeom[idx0];
		const auto &v1 = m_VecGeom[idx1];
		const auto &v2 = m_VecGeom[idx2];

		Vector3 tex0 = Vector3(v0.TexCoord.x, v0.TexCoord.y, 0);
		Vector3 tex1 = Vector3(v1.TexCoord.x, v1.TexCoord.y, 0);
		Vector3 tex2 = Vector3(v2.TexCoord.x, v2.TexCoord.y, 0);

		Vector3 texNormal = vec3_cross(tex1 - tex0, tex2 - tex0);

		// Check uv to determine if new triangles are needed

		// Sides
		if (texNormal.z > 0) {
			if (tex0.x < 0.1) {
				if (auto e = duplicatesIdx.find(idx0)) {
					m_VecIndices[i % m_NumIndices] = e->get();
				} else {
					auto newV0 = v0;
					newV0.TexCoord.x += 1;
					m_VecGeom.push_back(newV0);
					m_VecIndices[i % m_NumIndices] = countExtraVerts + m_NumVertices;

					duplicatesIdx.insert(countExtraVerts + m_NumVertices);
					countExtraVerts++;
				}
			}

			if (tex1.x < 0.1) {
				if (auto e = duplicatesIdx.find(idx1)) {
					m_VecIndices[(i + 1) % m_NumIndices] = e->get();
				} else {
					auto newV1 = v1;
					newV1.TexCoord.x += 1;
					m_VecGeom.push_back(newV1);
					m_VecIndices[(i + 1) % m_NumIndices] = countExtraVerts + m_NumVertices;

					duplicatesIdx.insert(countExtraVerts + m_NumVertices);
					countExtraVerts++;
				}
			}

			if (tex2.x < 0.1) {
				if (auto e = duplicatesIdx.find(idx2)) {
					m_VecIndices[(i + 2) % m_NumIndices] = e->get();
				} else {
					auto newV2 = v2;
					newV2.TexCoord.x += 1;
					m_VecGeom.push_back(newV2);
					m_VecIndices[(i + 2) % m_NumIndices] = countExtraVerts + m_NumVertices;

					duplicatesIdx.insert(countExtraVerts + m_NumVertices);
					countExtraVerts++;
				}
			}
		}

		// Poles
		if (m_NorthIdx.has(idx0) || m_SouthIdx.has(idx0)) {
			auto newV0 = v0;
			newV0.TexCoord.x = (v1.TexCoord.x + v2.TexCoord.x) / 2.0;
			m_VecGeom.push_back(newV0);
			m_VecIndices[(i) % m_NumIndices] = countExtraVerts + m_NumVertices;
			countExtraVerts++;
		} else if (m_NorthIdx.has(idx1) || m_SouthIdx.has(idx1)) {
			auto newV1 = v0;
			newV1.TexCoord.x = (v0.TexCoord.x + v2.TexCoord.x) / 2.0;
			m_VecGeom.push_back(newV1);
			m_VecIndices[(i + 1) % m_NumIndices] = countExtraVerts + m_NumVertices;
			countExtraVerts++;
		} else if (m_NorthIdx.has(idx2) || m_SouthIdx.has(idx2)) {
			auto newV2 = v0;
			newV2.TexCoord.x = (v0.TexCoord.x + v1.TexCoord.x) / 2.0;
			m_VecGeom.push_back(newV2);
			m_VecIndices[(i + 2) % m_NumIndices] = countExtraVerts + m_NumVertices;
			countExtraVerts++;
		}
	}

	m_NumVertices = m_VecGeom.Position.size();
	m_NumIndices = m_VecIndices.size();
}

// Build tangents
void GenRock::BuildTangents() {
	for (uint32_t idx = 0; idx + 2 < m_VecIndices.size(); idx += 3) {
		const int idx0 = m_VecIndices[idx + 0];
		const int idx1 = m_VecIndices[idx + 1];
		const int idx2 = m_VecIndices[idx + 2];

		Vector3 tangent = ComputeTangent(
				m_VecGeom.Position[idx0], m_VecGeom.Position[idx1], m_VecGeom.Position[idx2],
				m_VecGeom.TexCoord[idx0], m_VecGeom.TexCoord[idx1], m_VecGeom.TexCoord[idx2]);

		m_VecGeom.Tangent[idx0] = m_VecGeom.Tangent[idx0] + tangent;
		m_VecGeom.Tangent[idx1] = m_VecGeom.Tangent[idx1] + tangent;
		m_VecGeom.Tangent[idx2] = m_VecGeom.Tangent[idx2] + tangent;
	}
	for (uint32_t i = 0; i < m_VecGeom.Tangent.size(); i++) {
		m_VecGeom[i].Tangent = m_VecGeom.Tangent[i].normalized();
	}
}

void GenRock::_update() {
	if (!mesh) {
		mesh = newref(ArrayMesh);
	}

	mesh->clear_mesh();

	if (m_PostInitialize) {
		m_VecGeom.clear(), m_NumVertices = 0;
		m_VecIndices.clear(), m_NumIndices = 0;

		BuildIco();
		BuildRock();
		Expand();
		BuildNormals();
		CorrectUV();
		BuildTangents();

		ERR_FAIL_COND(!m_VecGeom.valid(m_NumVertices));
		ERR_FAIL_COND(m_VecIndices.size() != m_NumIndices);

		if (m_Smoothed) {
			Array a;
			a.resize(VS::ARRAY_MAX);
			a[VS::ARRAY_VERTEX] = (Vector<Vector3>)m_VecGeom.Position;
			a[VS::ARRAY_NORMAL] = (Vector<Vector3>)m_VecGeom.Normal;
			// CorrectUV() above already computes a real per-vertex UV unwrap — it was just
			// never copied into the output array, so texture_source materials on this
			// generator always sampled UV (0,0) everywhere. (m_VecGeom.Tangent from
			// BuildTangents() is a plain Vector3, not Godot's 4-component tangent+handedness
			// format, so it's not usable here directly — proc_rocks.cpp's ensure_tangents()
			// generates a correctly-formatted tangent from this UV instead.)
			a[VS::ARRAY_TEX_UV] = (Vector<Vector2>)m_VecGeom.TexCoord;
			a[VS::ARRAY_INDEX] = (Vector<int>)m_VecIndices;

			mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, a);
		} else {
			// Flat/low-poly shading: smooth per-vertex normals (BuildNormals() above)
			// visually disguise BuildRock()'s flattened facets as a rounded surface --
			// same reasoning as Method 3's own smooth/flat choice
			// (generators/procrockgen/procrockgen.cpp). Reuses the same shared helpers
			// that path already uses. A previous version of this code flipped the index
			// order here, reasoning that rock_studio_make_low_poly()'s flat-normal
			// computation (a naive cross(v1-v0,v2-v0), used only for LIGHTING) being the
			// opposite convention from this file's own ComputeNormal() meant the WINDING
			// needed flipping too -- but the stored normal attribute has no effect on
			// backface culling at all; only the actual vertex winding order does, and
			// rock_studio_make_low_poly() preserves whatever winding it's given unchanged.
			// Re-checked with an assumption-free synthetic-distant-camera test (see
			// memo.md's "Bugs Fixed"): m_VecIndices as produced by BuildIco()/BuildRock()
			// is already correctly wound; the flip made it backwards. Use it directly.
			Array temp_arrays;
			temp_arrays.resize(VS::ARRAY_MAX);
			temp_arrays[VS::ARRAY_VERTEX] = (Vector<Vector3>)m_VecGeom.Position;
			temp_arrays[VS::ARRAY_INDEX] = (Vector<int>)m_VecIndices;

			Ref<ArrayMesh> temp;
			temp.instance();
			temp->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, temp_arrays);

			Ref<ArrayMesh> low_poly = rock_studio_make_low_poly(temp);
			if (low_poly.is_valid() && low_poly->get_surface_count() > 0) {
				rock_studio_box_uv(low_poly);
				mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, low_poly->surface_get_arrays(0));
			}
		}
		m_PostInitialize = false;

		print_verbose("Rock updated");
	}
}
