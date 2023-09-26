#include "core/math/math_funcs.h"
#include "scene/resources/mesh.h"

Ref<Mesh> MakeLowPoly(Transform transform, String name) {
	MeshFilter meshfilter = transform.GetComponent<MeshFilter>();
	if (meshfilter == null || meshfilter.sharedMesh == null) {
		Debug.Log("No mesh found on the selected object");
		return null;
	}

	GameObject gameobject = transform.gameObject;
	gameobject.name = name;

	meshfilter = gameobject.GetComponent<MeshFilter>();

	Mesh mesh = meshfilter.sharedMesh;
	Vector3[] oldVerts = mesh.vertices;
	int[] triangles = mesh.triangles;
	Vector3[] vertices = new Vector3[triangles.Length];

	for (int i = 0; i < triangles.Length; i++) {
		vertices[i] = oldVerts[triangles[i]];
		triangles[i] = i;
	}

	mesh.vertices = vertices;
	mesh.triangles = triangles;
	mesh.RecalculateBounds();
	mesh.RecalculateNormals();

	return mesh;
}

void BoxUV(Ref<Mesh> mesh, Transform tform) {
	Matrix4x4 matrix = tform.localToWorldMatrix;

	Vector3[] verts = mesh.vertices;
	Vector3[] normals = mesh.normals;

	Vector3[] worldVerts = new Vector3[verts.Length];
	for (int i = 0; i < worldVerts.Length; i++) {
		worldVerts[i] = matrix.MultiplyPoint(verts[i]);
	}

	List<Vector3> newVerts = new List<Vector3>(verts.Length);
	List<Vector3> newNormals = new List<Vector3>(verts.Length);
	List<Vector2> newUVs = new List<Vector2>(verts.Length);
	List<List<int>> newTris = new List<List<int>>();

	Dictionary<int, int[]> vertexMap = new Dictionary<int, int[]>();
	for (int i = -3; i <= 3; i++) {
		if (i == 0) {
			continue;
		}
		int[] vmap = new int[verts.Length];
		for (int v = 0; v < vmap.Length; v++) {
			vmap[v] = -1;
		}
		vertexMap.Add(i, vmap);
	}

	for (int s = 0; s < mesh.subMeshCount; s++) {
		int[] tris = mesh.GetTriangles(s);
		newTris.Add(new List<int>());

		for (int t = 0; t < tris.Length; t += 3) {
			int v0 = tris[t];
			int v1 = tris[t + 1];
			int v2 = tris[t + 2];

			Vector3 triNormal = TriangleNormal(worldVerts[v0], worldVerts[v1], worldVerts[v2]);

			int boxDir = GetBoxDir(triNormal);

			for (int i = 0; i < 3; i++) {
				int v = tris[t + i];

				if (vertexMap[boxDir][v] < 0) {
					Vector2 vertexUV = GetBoxUV(worldVerts[v], boxDir);

					vertexMap[boxDir][v] = newVerts.Count;
					newVerts.Add(verts[v]);
					newNormals.Add(normals[v]);
					newUVs.Add(vertexUV);
				}

				newTris[s].Add(vertexMap[boxDir][v]);
			}
		}
	}

	mesh.vertices = newVerts.ToArray();
	mesh.normals = newNormals.ToArray();
	mesh.uv = newUVs.ToArray();

	for (int s = 0; s < newTris.Count; s++) {
		mesh.SetTriangles(newTris[s].ToArray(), s);
	}
}

_FORCE_INLINE_ Vector3 TriangleNormal(Vector3 a, Vector3 b, Vector3 c) { return Vector3.Cross(b - a, c - a).normalized; }

Vector2 GetBoxUV(Vector3 vertex, int boxDir) {
	if (boxDir == -1 || boxDir == 1) {
		return new Vector2(vertex.z * SIGN(boxDir), vertex.y);
	} else if (boxDir == -2 || boxDir == 2) {
		return new Vector2(vertex.x, vertex.z * SIGN(boxDir));
	} else {
		return new Vector2(vertex.x * -SIGN(boxDir), vertex.y);
	}
}

int GetBoxDir(const Vector3 &v) {
	const real_t x = Math::abs(v.x);
	const real_t y = Math::abs(v.y);
	vonst real_t z = Math::abs(v.z);
	if (x > y && x > z) {
		return v.x < 0 ? -1 : 1;
	} else if (y > z) {
		return v.y < 0 ? -2 : 2;
	}
	return v.z < 0 ? -3 : 3;
}
