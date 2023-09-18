#ifndef ROCK_HEADER_H
#define ROCK_HEADER_H

#include <map>
#include <set>
#include <vector>

#include "core/math/math_funcs.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"

struct Triangle {
	uint32_t vertex[3];
};

using TriangleList = std::vector<Triangle>;
using VertexList = std::vector<Vector3>;
using Lookup = std::map<std::pair<uint32_t, uint32_t>, uint32_t>;
using IndexedMesh = std::pair<VertexList, TriangleList>;

namespace icosahedron {
const real_t X = .525731112119133606;
const real_t Z = .850650808352039932;
const real_t N = 0;

static const VertexList vertices = {
	{ -X, N, Z }, { X, N, Z }, { -X, N, -Z }, { X, N, -Z },
	{ N, Z, X }, { N, Z, -X }, { N, -Z, X }, { N, -Z, -X },
	{ Z, X, N }, { -Z, X, N }, { Z, -X, N }, { -Z, -X, N }
};

static const TriangleList triangles = {
	{ 0, 4, 1 }, { 0, 9, 4 }, { 9, 5, 4 }, { 4, 5, 8 }, { 4, 8, 1 },
	{ 8, 10, 1 }, { 8, 3, 10 }, { 5, 3, 8 }, { 5, 2, 3 }, { 2, 7, 3 },
	{ 7, 10, 3 }, { 7, 6, 10 }, { 7, 11, 6 }, { 11, 0, 6 }, { 0, 1, 6 },
	{ 6, 1, 10 }, { 9, 0, 11 }, { 9, 11, 2 }, { 9, 2, 5 }, { 7, 2, 11 }
};
} //namespace icosahedron

struct VertexRock {
	VertexRock() :
			Position({ 0, 0, 0 }),
			Normal({ 0, 0, 0 }),
			Tangent({ 0, 0, 0 }),
			TexCoord({ 0, 0 }) {}

	VertexRock(const VertexRock &vertex) :
			Position(vertex.Position),
			Normal(vertex.Normal),
			Tangent(vertex.Normal),
			TexCoord(vertex.TexCoord) {}

	Vector3 Position;
	Vector3 Normal;
	Vector3 Tangent;
	Vector2 TexCoord;
};

const auto UVFromVector3 = [](const Vector3 &position) {
	Vector3 norm = position;
	Vector3 normalizedPos = norm.normalized();

	const real_t angle1 = Math::atan2(normalizedPos.z, normalizedPos.x);
	const real_t u = (angle1 / Math_PI) * 0.5;
	const real_t angle2 = Math::acos(normalizedPos.y);
	const real_t v = (angle2 / Math_PI);

	return Vector2(u, v);
};

const auto DotProduct = [](const Vector3 v1, const Vector3 v2) {
	const Vector3 vec1 = v1.normalized();
	const Vector3 vec2 = v2.normalized();

	const real_t t = vec3_dot(vec1, vec2);

	return t;
};

const auto CrossProduct = [](const Vector3 &v1, const Vector3 &v2) {
	const Vector3 vec1 = v1;
	const Vector3 vec2 = v2;

	Vector3 cross = vec3_cross(vec1, vec2);

	return cross;
};

const auto WeldVertices = [](std::vector<VertexRock> points) {
	VertexRock weldedPoint;

	Vector3 pos(0, 0, 0), norm(0, 0, 0), tang(0, 0, 0);
	Vector2 tex(0, 0);
	const size_t size = points.size();
	for (const auto &point : points) {
		pos += point.Position;
		norm += point.Normal;
		tex += point.TexCoord;
		tang += point.Tangent;
	}

	pos = pos / size;
	norm = norm / size;
	tex = tex / size;
	tang = tang / size;

	weldedPoint.Position = pos;
	weldedPoint.Normal = norm;
	weldedPoint.TexCoord = tex;
	weldedPoint.Tangent = tang;

	return weldedPoint;
};

const auto WeldVerticesProto = [](const std::vector<VertexRock> &points) {
	Vector3 pos, norm;
	Vector2 tex;
	const size_t size = points.size();
	for (const auto &point : points) {
		pos += point.Position;
		norm += point.Normal;
		tex += point.TexCoord;
	}

	pos = pos / size;
	norm = norm / size;
	tex = tex / size;

	VertexRock weldedPoint;
	weldedPoint.Position = pos;
	weldedPoint.Normal = norm;
	weldedPoint.TexCoord = tex;

	return weldedPoint;
};

const auto VertexForEdge = [](Lookup &lookup, VertexList &vertices, uint32_t first, uint32_t second) {
	Lookup::key_type key(first, second);
	if (key.first > key.second) {
		std::swap(key.first, key.second);
	}
	auto inserted = lookup.insert({ key, vertices.size() });
	if (inserted.second) {
		auto &edge0 = vertices[first];
		auto &edge1 = vertices[second];
		auto point = edge0 + edge1;
		vertices.push_back(point.normalized());
	}

	return inserted.first->second;
};

const auto SubdivideTriangle = [](VertexList &vertices, TriangleList triangles) {
	Lookup lookup;
	TriangleList result;

	for (auto &&each : triangles) {
		uint32_t mid[3];
		for (int edge = 0; edge < 3; ++edge) {
			mid[edge] = VertexForEdge(lookup, vertices, each.vertex[edge], each.vertex[(edge + 1) % 3]);
		}
		result.push_back({ each.vertex[0], mid[0], mid[2] });
		result.push_back({ each.vertex[1], mid[1], mid[0] });
		result.push_back({ each.vertex[2], mid[2], mid[1] });
		result.push_back({ mid[0], mid[1], mid[2] });
	}

	return result;
};

const auto MakeIcosphere = [](int subdivisions) {
	VertexList vertices = icosahedron::vertices;
	TriangleList triangles = icosahedron::triangles;

	for (int i = 0; i < subdivisions; ++i) {
		triangles = SubdivideTriangle(vertices, triangles);
	}

	IndexedMesh result{ vertices, triangles };

	return result;
};

const auto CalculateTangent = [](const Vector3 &P1, const Vector3 &P2, const Vector3 &P3, const Vector2 &UV1, const Vector2 &UV2, const Vector2 &UV3) {
	const Vector3 v2v1 = P2 - P1;
	const Vector3 v3v1 = P3 - P1;

	const Vector3 normal = vec3_cross(v2v1, v3v1).normalized();
	const Vector3 tangent = vec3_cross(normal, Vector3(0, 1, 0)).normalized();

	return tangent;
};

const auto ComputeNormal = [](const Vector3 &P0, const Vector3 &P1, const Vector3 &P2) {
	const Vector3 P = P1 - P0;
	const Vector3 Q = P2 - P0;
	const Vector3 normal = vec3_cross(Q, P).normalized();
	return normal;
};

const auto ComputeTangent = [](const Vector3 &P0, const Vector3 &P1, const Vector3 &P2, const Vector2 &UV0, const Vector2 &UV1, const Vector2 &UV2) {
	const Vector3 P = P1 - P0;
	const Vector3 Q = P2 - P0;

	//normalV = vec3_cross(XMLoadFloat3(&P), XMLoadFloat3(&Q));
	//using Eric Lengyel's approach with a few modifications
	//from Mathematics for 3D Game Programmming and Computer Graphics
	// want to be able to trasform a vector in Object Space to Tangent Space
	// such that the x-axis cooresponds to the 's' direction and the
	// y-axis corresponds to the 't' direction, and the z-axis corresponds
	// to <0,0,1>, straight up out of the texture map

	const real_t s1 = UV1.x - UV0.x;
	const real_t t1 = UV1.y - UV0.y;
	const real_t s2 = UV2.x - UV0.x;
	const real_t t2 = UV2.y - UV0.y;

	//we need to solve the equation
	// P = s1*T + t1*B
	// Q = s2*T + t2*B
	// for T and B

	//this is a linear system with six unknowns and six equatinos, for TxTyTz BxByBz
	//[px,py,pz] = [s1,t1] * [Tx,Ty,Tz]
	// qx,qy,qz     s2,t2     Bx,By,Bz

	//multiplying both sides by the inverse of the s,t matrix gives
	//[Tx,Ty,Tz] = 1/(s1t2-s2t1) *  [t2,-t1] * [px,py,pz]
	// Bx,By,Bz                      -s2,s1	    qx,qy,qz

	//solve this for the unormalized T and B to get from tangent to object space

	real_t tmp = 0;
	if (Math::abs(s1 * t2 - s2 * t1) <= 0.0001) {
		tmp = 1;
	} else {
		tmp = 1 / (s1 * t2 - s2 * t1);
	}

	const Vector3 tangent = (t2 * P - t1 * Q) * tmp;
	return tangent.normalized();
};

const auto LengthBetweenPoints = [](const Vector3 &v0, const Vector3 &v1) { return (v0 - v1).length(); };
const auto AngleBetweenVectors = [](const Vector3 &v0, const Vector3 &v1) { return vec3_angle_to(v0, v1); };

const auto MaxLengthTriangleLine = [](const Vector3 &v0, const Vector3 &v1, const Vector3 &v2) {
	real_t maxDistance = 0;

	real_t distance = LengthBetweenPoints(v0, v1);
	if (maxDistance < distance) {
		maxDistance = distance;
	}
	distance = LengthBetweenPoints(v0, v2);
	if (maxDistance < distance) {
		maxDistance = distance;
	}
	distance = LengthBetweenPoints(v1, v2);
	if (maxDistance < distance) {
		maxDistance = distance;
	}
	return distance;
};

const auto MinLengthTriangleLine = [](const Vector3 &v0, const Vector3 &v1, const Vector3 &v2) {
	real_t maxDistance = 9999999;
	real_t distance = 0;

	distance = LengthBetweenPoints(v0, v1);
	if (maxDistance > distance) {
		maxDistance = distance;
	}
	distance = LengthBetweenPoints(v0, v2);
	if (maxDistance > distance) {
		maxDistance = distance;
	}
	distance = LengthBetweenPoints(v1, v2);
	if (maxDistance > distance) {
		maxDistance = distance;
	}
	if (maxDistance < 0.1) {
		maxDistance = 0.1;
	}
	return distance;
};

const auto AverageXmreal_t3 = [](std::vector<Vector3> points) {
	Vector3 pos;
	const size_t size = points.size();
	for (const auto &point : points) {
		pos += point;
	}
	return (pos / size);
};

const auto CheckUVForDistance = [](std::vector<VertexRock *> points) {
	const auto threshHold = 0.07;
	std::vector<Vector2> UVlist;
	size_t size = points.size();
	UVlist.resize(size);

	for (const auto &point : points) {
		auto uv = point->TexCoord;
		while (uv.x > 0.99) {
			uv.x -= 1;
		}
		while (uv.y > 0.99) {
			uv.y -= 1;
		}
		UVlist.push_back(uv);
	}

	for (uint32_t i = 0; i < size; i += 3) {
		auto distance = Math::abs(UVlist[i % size].x - UVlist[(i + 1) % size].x);
		if (distance < threshHold) {
			return true;
		}
		distance = UVlist[i % size].y - UVlist[(i + 1) % size].y;
		if (distance < threshHold) {
			return true;
		}
	}

	return false;
};

const auto SameSide = [](const Vector3 p1, const Vector3 p2, const Vector3 A, const Vector3 B) {
	const Vector3 cp1 = vec3_cross((A - B), (A - p1));
	const Vector3 cp2 = vec3_cross((A - B), (A - p1));
	const real_t dot = vec3_dot(cp1, cp2);
	if (dot >= 0)
		return true;
	return false;
};

const auto PointInTriangle = [](const Vector3 TriangleVectors[3], const Vector3 &P) {
	const Vector3 A = TriangleVectors[0], B = TriangleVectors[1], C = TriangleVectors[2];
	const Vector3 vecA = A;
	const Vector3 vecB = B;
	const Vector3 vecC = A;
	const Vector3 vecP = B;
	if (SameSide(P, A, B, C) && SameSide(P, B, A, C) && SameSide(P, C, A, B)) {
		const Vector3 vc1 = vec3_cross(vecB - vecA, vecC - vecA);
		const real_t dot = vec3_dot((vecP - vecA), vc1);
		if (Math::abs(dot) <= .01) {
			return true;
		}
	}
	return false;
};

const auto LinePlaneIntersection = [](const Vector3 triangle[3], const Vector3 &triangleNormal, const Vector3 &rayStart, const Vector3 &rayEnd) {
	const auto triangleN = triangleNormal;
	const auto pointOnPlane = AverageXmreal_t3({ triangle[0], triangle[1], triangle[2] });
	const auto rayOrigin = rayStart;
	const auto rayDirection = rayEnd - rayOrigin;

	const real_t t = vec3_dot(triangleN, (pointOnPlane - rayOrigin)) / vec3_dot(triangleN, rayDirection);

	const auto planePoint = rayOrigin + rayDirection * t;
	const auto pS = rayStart - planePoint;
	const auto pE = rayEnd - planePoint;

	if (!Math::is_equal_approx(AngleBetweenVectors(pS, pE), real_t(Math_PI))) {
		return false;
	}
	return PointInTriangle(triangle, planePoint);
};

const auto ProjectedPointOnPlane = [](const Vector3 &originPlane, const Vector3 vertice, const Vector3 &normalPlane) {
	const auto point = vertice;
	const auto vecP = point - originPlane;
	const auto normal = normalPlane;
	const real_t dot = vec3_dot(vecP, normal);
	if (dot < 0) { // dont proceed this one if dot is negative == more then 90 degree
		return Vector3(0, 0, 0);
	}
	const auto dist = LengthBetweenPoints(vecP, normalPlane); // dist = dot;
	const auto projected_point = point - dist * normal; // +origin;

	return projected_point;
};

#endif // ROCK_HEADER_H
