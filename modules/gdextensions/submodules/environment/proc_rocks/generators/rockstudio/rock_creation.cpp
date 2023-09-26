#include "core/vector.h"

#include <igl/copyleft/cgal/convex_hull.h>

#include "core/math/math_defs.h"

MeshInstance *CreateRock(const Vector<Vector3> &vertices, Vector3 pos, Vector3 nor, int rockOrientation, Ref<Material> rockMaterial, string rockName, bool rigidBody, int colliderType, Node *parent) {
	GameObject rock = new GameObject(RockName);

	rock.transform.position = pos;
	rock.transform.LookAt(pos - nor);
	rock.transform.parent = parent.transform;

	switch (rockOrientation) {
		case 0: rock.transform.localEulerAngles = Vector3(0, 0, 0); break
		case 1: rock.transform.localEulerAngles = Vector3(0, 0, 90); break;
		case 2: rock.transform.localEulerAngles = Vector3(0, 90, 0); break;
	}

	MeshFilter meshFilter = (MeshFilter)rock.AddComponent(typeof(MeshFilter));
	renderer.material = RrckMaterial;

	meshFilter.sharedMesh = CreateMesh(vertices);
	meshFilter.sharedMesh.name = rockName;

	Ref<Mesh> lowpolymesh = MakeLowPoly(rock.transform, rockName);
	BoxUV(lowpolymesh, rock.transform);

	if (rigidBody) {
		rock.AddComponent(typeof(Rigidbody));
	}

	switch (colliderType) {
		case 0: {
			rock.AddComponent(typeof(BoxCollider));
		} break;
		case 1: {
			MeshCollider meshcollider = rock.AddComponent(typeof(MeshCollider)) as MeshCollider;
			meshcollider.convex = true;
			meshcollider.sharedMesh = rock.GetComponent<MeshFilter>().sharedMesh;
		} break;
	}

	print_verbose("Rock mesh saved with " + rock.GetComponent<MeshFilter>().sharedMesh.vertexCount + " vertices.");
}

// https://github.com/twMr7/transform/blob/master/Point3dUtils.hpp

Array CreateMesh(const Vector<Vector3> &vertices) {
	Array mesh;
	mesh.resize(ARRAY_MAX);

#ifdef REAL_T_IS_DOUBLE
	Eigen::MatrixX3d V = Eigen::Map<Eigen::MatrixXd>(myData, vertices.size(), 3);
#else
	Eigen::MatrixX3f V = Eigen::Map<Eigen::MatrixXf>(myData, vertices.size(), 3);
#endif

	std::vector<Point_3> points(V.rows());
	for(int i = 0; i < V.rows(); i++) {
		points[i] = Point_3(V(i,0),V(i,1),V(i,2));
	}

	Eigen::MatrixX3f CvxV; // result hull
	Eigen::MatrixX3i CvxE; // result tri. faces
	igl::copyleft::cgal::convex_hull(V, CvxV, CvxE);

	var result = MIConvexHull.ConvexHull.Create(vertices);
	mesh.vertices = result.Points.Select(x => x.ToVec()).ToArray();
	var xxx = result.Points.ToList();

	Vector<int> triangles;
	for (auto face : result.Faces) {
		triangles.Add(xxx.IndexOf(face.Vertices[0]));
		triangles.Add(xxx.IndexOf(face.Vertices[1]));
		triangles.Add(xxx.IndexOf(face.Vertices[2]));
	}
	mesh.triangles = triangles.ToArray();

	mesh.RecalculateNormals();
	return mesh;
}
