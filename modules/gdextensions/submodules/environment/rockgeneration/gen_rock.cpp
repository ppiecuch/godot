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

#include "common/gd_core.h"
#include "core/math/math_funcs.h"
#include "rock_header.h"

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

	// SUBDIVIDE TRIANGLES + ADD NEW VERTICES TO BUFFER
	for (int i = 0; i < vertices.size(); i++) {
		auto vertice = vertices[i];
		auto vertVector = vertice.normalized();
		auto vert = vertVector;

		real_t r = Math::sqrt(vertice.x * vertice.x + vertice.y * vertice.y + vertice.z * vertice.z);
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

	// Set indices
	for (const auto &indice : indices) {
		m_VecIndices.push_back(indice.vertex[0]);
		m_VecIndices.push_back(indice.vertex[1]);
		m_VecIndices.push_back(indice.vertex[2]);
	}
	m_NumIndices = m_VecIndices.size();
}

// Convert icosphere into 'rock'
void GenRock::BuildRock() {
	// Flatten by 'planes'
	for (uint32_t plane = 0; plane < m_MaxPlanes; plane++) {
		// Determine position of plane by angle on sphere
		Vector3 originPlane, radiusPlane;
		m_PrevAngles.x = m_PrevAngles.x + m_MinRandAngle / 180.0 * Math_PI;
		m_PrevAngles.y = m_PrevAngles.y + m_MinRandAngle / 180.0 * Math_PI;

		m_PrevAngles.x = Math::rand() % (int(m_MaxRandAngle) - int(m_MinRandAngle)) + int(m_MinRandAngle);
		m_PrevAngles.y = Math::rand() % (int(m_MaxRandAngle) - int(m_MinRandAngle)) + int(m_MinRandAngle);

		// Origin plane
		originPlane.x = m_Width * Math::cos(m_PrevAngles.x) * Math::cos(m_PrevAngles.y);
		originPlane.y = m_Height * Math::cos(m_PrevAngles.x) * Math::sin(m_PrevAngles.y);
		originPlane.z = m_Depth * Math::sin(m_PrevAngles.x);

		radiusPlane.x = m_Width * Math::cos(m_PrevAngles.x + Math_PI) * Math::cos(m_PrevAngles.y + Math_PI);
		radiusPlane.y = m_Height * Math::cos(m_PrevAngles.x + Math_PI) * Math::sin(m_PrevAngles.y + Math_PI);
		radiusPlane.z = m_Depth * Math::sin(m_PrevAngles.x + Math_PI);

		// Create plane
		const real_t offset = Math::rand() % int(m_MaxOffsetPercent);
		auto origin = originPlane;
		origin *= (100.0 - offset) / 100.0;
		originPlane = origin;
		auto normal = origin.normalized();
		auto normalPlane = normal;

		// Flatten vertices onto plane
		for (uint32_t i = 0; i < m_NumVertices; i++) {
			// Check if vertice is in front of the plane
			auto vertice = m_VecGeom[i];
			auto point = vertice.Position;
			auto vecP = point - originPlane;
			auto dot = vec3_dot(vecP, normal);
			if (dot < 0) { // dont proceed this one if dot is negative == more then 90 degree
				continue;
			}
			// Project on plane
			const auto vectorFromPoint = vecP;

			const auto dist = vectorFromPoint.x * normalPlane.x + vectorFromPoint.y * normalPlane.y + vectorFromPoint.z * normalPlane.z;
			const auto projectedPoint = point - dist * normal;

			// Create new vertice, make curved
			const auto distToCenter = LengthBetweenPoints(projectedPoint, originPlane);
			const auto diameter = LengthBetweenPoints(Vector3(0, 0, 0), radiusPlane) / 2.0;
			const auto strength = (1.0 / diameter) * distToCenter - 1.0;

			// Update vertice
			m_VecGeom.Position[i] = point - (dist / 2.0) * normal * strength;
			m_VecGeom.Normal[i] = normalPlane;
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
	std::set<uint32_t> duplicatesIdx;
	for (int i = 0; i < m_NumIndices; i += 3) {
// Data
#pragma region data
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
#pragma endregion

		// Sides
		if (texNormal.z > 0) {
			if (tex0.x < 0.1) {
				if (duplicatesIdx.find(idx0) != duplicatesIdx.end()) {
					m_VecIndices[i % m_NumIndices] = *duplicatesIdx.find(idx0);
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
				if (duplicatesIdx.find(idx1) != duplicatesIdx.end()) {
					m_VecIndices[(i + 1) % m_NumIndices] = *duplicatesIdx.find(idx1);
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
				if (duplicatesIdx.find(idx2) != duplicatesIdx.end()) {
					m_VecIndices[(i + 2) % m_NumIndices] = *duplicatesIdx.find(idx2);
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
		if (m_NorthIdx.find(idx0) != m_NorthIdx.end() || m_SouthIdx.find(idx0) != m_SouthIdx.end()) {
			auto newV0 = v0;
			newV0.TexCoord.x = (v1.TexCoord.x + v2.TexCoord.x) / 2.0;
			m_VecGeom.push_back(newV0);
			m_VecIndices[(i) % m_NumIndices] = countExtraVerts + m_NumVertices;
			countExtraVerts++;
		} else if (m_NorthIdx.find(idx1) != m_NorthIdx.end() || m_SouthIdx.find(idx1) != m_SouthIdx.end()) {
			auto newV1 = v0;
			newV1.TexCoord.x = (v0.TexCoord.x + v2.TexCoord.x) / 2.0;
			m_VecGeom.push_back(newV1);
			m_VecIndices[(i + 1) % m_NumIndices] = countExtraVerts + m_NumVertices;
			countExtraVerts++;
		} else if (m_NorthIdx.find(idx2) != m_NorthIdx.end() || m_SouthIdx.find(idx2) != m_SouthIdx.end()) {
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

	if (m_PostInitialize == true) {
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

		Array a;
		a.resize(VS::ARRAY_MAX);
		a[VS::ARRAY_VERTEX] = (Vector<Vector3>)m_VecGeom.Position;
		a[VS::ARRAY_NORMAL] = (Vector<Vector3>)m_VecGeom.Normal;
		a[VS::ARRAY_INDEX] = (Vector<int>)m_VecIndices;

		m_PostInitialize = false;

		print_verbose("Rock updated");
	}
}
