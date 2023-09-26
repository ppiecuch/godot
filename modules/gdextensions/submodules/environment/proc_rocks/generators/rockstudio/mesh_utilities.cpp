#include "scene/resources/mesh.h"

Vector3 GetRandomPointOnMesh(Ref<Mesh> mesh) {
	int triangleCount = mesh.triangles.Length / 3;
	real_t[] sizes = new real_t[triangleCount];
	for (int i = 0; i < triangleCount; i++) {
		Vector3 va = mesh.vertices[mesh.triangles[i * 3 + 0]];
		Vector3 vb = mesh.vertices[mesh.triangles[i * 3 + 1]];
		Vector3 vc = mesh.vertices[mesh.triangles[i * 3 + 2]];

		sizes[i] = .5f * Vector3.Cross(vb - va, vc - va).magnitude;
	}

	real_t[] cumulativeSizes = new real_t[sizes.Length];
	real_t total = 0;

	for (int i = 0; i < sizes.Length; i++) {
		total += sizes[i];
		cumulativeSizes[i] = total;
	}

	const real_t randomsample = Math::randf() * total;

	int triIndex = -1;

	for (int i = 0; i < sizes.Length; i++) {
		if (randomsample <= cumulativeSizes[i])
		{
			triIndex = i;
			break;
		}
	}

	if (triIndex == -1) ERR_PRINT("triIndex should never be -1");

	Vector3 a = mesh.vertices[mesh.triangles[triIndex * 3 + 0]];
	Vector3 b = mesh.vertices[mesh.triangles[triIndex * 3 + 1]];
	Vector3 c = mesh.vertices[mesh.triangles[triIndex * 3 + 2]];

	real_t r = Math::randf();
	real_t s = Math::randf();

	if (r + s >= 1) {
		r = 1 - r;
		s = 1 - s;
	}

	const Vector3 pointOnMesh = a + r * (b - a) + s * (c - a);
	return pointOnMesh;
}

int GetVerts(GameObject obj) {
	int numberOfVerts = 0;

	for (Transform child : obj.transform) {
		numberOfVerts += child.gameObject.GetComponent<MeshFilter>().sharedMesh.vertexCount;

	}

	if (obj.gameObject.transform.childCount == 0 && obj.gameObject.GetComponent<MeshFilter>() != null) {
		numberOfVerts = obj.gameObject.GetComponent<MeshFilter>().sharedMesh.vertexCount;
	}

	return numberOfVerts;
}

Vector3<Vector3> GetRandomPointsWithinCube(int numberOfPoints, real_t width, real_t height, real_t depth) {
	Vector<Vector3> points;
#define RandRange(var) Math::random(-(var) * 0.5, (var) * 0.5)
	for (int i = 0; i < numberOfPoints; i++) {
		points.push_back(Vector3(RandRange(depth), RandRange(height), RandRange(width)));
	}
#undef RandRange
	return points;
}

List<Vector3> GetRandomPointsWithinSphere(int numberOfPoints, real_t radius) {
	List<Vector3> points = new List<Vector3>();
	for (int i = 0; i < numberOfPoints; i++) {
		points.push_back(Random.insideUnitSphere * radius);
	}
	return points;
}

Vector<Vector3> GetRandomPointsWithinCrystal(int numberOfPoints, bool tetragonal, bool oneSided, real_t baseWidth, real_t baseHeight, real_t tipProtrusion, real_t tipFlatness) {
	Vector<Vector3> points;
#define RandRange(var) Math::random(-(var), (var))
	if (tetragonal) {
		for (int i = 0; i < numberOfPoints; i++) {
			points.push_back(Vector3(RandRange(baseHeight), RandRange(baseWidth), RandRange(baseWidth)));
		}
	} else {
		for (int i = 0; i < numberOfPoints; i++) {
			points.push_back(Vector3(RandRange(baseHeight), RandRange(baseWidth * 1.732), Math::random(-baseWidth, baseWidth)));
		}
		for (int i = 0; i < numberOfPoints; i++) {
			points.push_back(Vector3(RandRange(baseHeight), 0, RandRange(baseWidth * 1.732)));
		}
	}

	if (oneSided) {
		for (int i = 0; i < numberOfPoints; i++) {
			points.push_back(Vector3(Math::random(-baseHeight, baseHeight + tipProtrusion), RandRange(baseWidth * (tipFlatness / 10))), RandRange(baseWidth * 1.732 * (tipFlatness / 10))));
		}
	} else {
		for (int i = 0; i < numberOfPoints; i++) {
			points.push_back(new Vector3(RandRange(baseHeight - tipProtrusion), RandRange(baseWidth * (tipFlatness / 10)), RandRange(baseWidth * 1.732f * (tipFlatness / 10))));
	}
#undef RandRange
	return points;
}

Vector<Vector3> GetRandomPointsOnMesh(int numberOfPoints, Ref<Mesh> mesh) {
	Vector<Vector3> points;
	for (int i = 0; i < numberOfPoints; i++) {
		Vector3 p = GetRandomPointOnMesh(mesh);

		const real_t min = 1;
		const real_t max = 1;

		p *= Math::random(min, max);
		points.push_back(p);
	}
	return points;
}
