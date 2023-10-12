/**************************************************************************/
/*  rock_creation.cpp                                                     */
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

#include "core/vector.h"

#include <igl/copyleft/cgal/convex_hull.h>

#include "core/math/math_defs.h"

MeshInstance *CreateRock(const Vector<Vector3> &vertices, Vector3 pos, Vector3 nor, int rockOrientation, Ref<Material> rockMaterial, string rockName, bool rigidBody, int colliderType, Node *parent) {
	GameObject rock = new GameObject(RockName);

	rock.transform.position = pos;
	rock.transform.LookAt(pos - nor);
	rock.transform.parent = parent.transform;

	switch (rockOrientation) {
		case 0:
			rock.transform.localEulerAngles = Vector3(0, 0, 0);
		break case 1:
			rock.transform.localEulerAngles = Vector3(0, 0, 90);
			break;
		case 2:
			rock.transform.localEulerAngles = Vector3(0, 90, 0);
			break;
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
	for (int i = 0; i < V.rows(); i++) {
		points[i] = Point_3(V(i, 0), V(i, 1), V(i, 2));
	}

	Eigen::MatrixX3f CvxV; // result hull
	Eigen::MatrixX3i CvxE; // result tri. faces
	igl::copyleft::cgal::convex_hull(V, CvxV, CvxE);

	var result = MIConvexHull.ConvexHull.Create(vertices);
	mesh.vertices = result.Points.Select(x = > x.ToVec()).ToArray();
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
