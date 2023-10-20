/**************************************************************************/
/*  sculpture_creator.cpp                                                 */
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

void GenerateComposition(int NumberOfVertices, Vector3 pos, Vector3 nor, Ref<Material> rockmat) {
	if (GameObject.Find("Sculpture in progress") != null) {
		GameObject rockstructure = new GameObject("Sculpture");

		Random.state = Random.state;
		var seed = Random.Range(0, 5000);
		Random.InitState(seed);

#define RandOne() Math::random(real_t(-1), real_t(1))
		for (Transform child : GameObject.Find("Sculpture in progress").transform) {
			List<Vector3> VertexList = new List<Vector3>(NumberOfVertices);
			pos = child.localPosition;
			Vector3 scale = child.localScale;
			Vector3 rot = child.localEulerAngles;
			for (int i = 0; i < NumberOfVertices; i++) {
				VertexList.push_back((Vector3(RandOne(), RandOne(), RandOne()) * 0.5));
			}
			CreateSculpture(VertexList, pos, Vector3.zero, rot, scale, rockmat);
		}
	}
}

void CreateSculpture(Vector<Vector3> Vertices, Vector3 pos, Vector3 nor, Vector3 rot, Vector3 scale, Ref<Material> rockmat) {
	GameObject rock = new GameObject("Temporary Sculpture");

	rock.transform.position = pos;
	rock.transform.eulerAngles = rot;
	rock.transform.localScale = scale;
	rock.transform.LookAt(pos - nor);
	rock.transform.parent = GameObject.Find("Sculpture").transform;

	MeshFilter meshFilter = (MeshFilter)rock.AddComponent(typeof(MeshFilter)); //Add a mesh filter.

	renderer.material = rockmat;
	meshFilter.sharedMesh = RockCreation.CreateMesh(Vertices);
	meshFilter.sharedMesh.name = "Temporary Sculpture";

	Mesh lowpolymesh = MakeLowPoly(rock.transform, "Temporary Sculpture");
	BoxUV(lowpolymesh, rock.transform);
}

void GenerateCompositionFill(int NumberOfVertices, Vector3 pos, Vector3 nor, Ref<Material> rockmat) {
	Vector<Vector3> VertexList;
	VertexList.resize(NumberOfVertices);

	Random.state = Random.state;
	var seed = Math::random(0, 5000);
	Random.InitState(seed);

	if (GameObject.Find("Sculpture in progress") != null) {
		GameObject rockstructure = new GameObject();
		rockstructure.name = "Sculpture";

		for (Transform child : GameObject.Find("Sculpture in progress").transform) {
			Vector3 pos1 = child.localPosition;
			Vector3 scale = child.localScale;
			Vector3 rot = child.localEulerAngles;

			for (int i = 0; i < NumberOfVertices; i++) {
				VertexList.Add(new Vector3(Random.Range(-scale.x * 0.5, scale.x * 0.5) + pos1.x, Random.Range(-scale.y * 0.5, scale.y * 0.5) + pos1.y, Random.Range(-scale.z * 0.5, scale.z * 0.5) + pos1.z));
			}
		}
	}
	CreateSculptureFill(VertexList, pos, nor, rockmat); // Create the rock.
}

void CreateSculptureFill(Vector<Vector3> Vertices, Vector3 pos, Vector3 nor, Ref<Material> rockmat) {
	GameObject rock = new GameObject("Temporary Sculpture");
	rock.transform.position = pos;
	rock.transform.LookAt(pos - nor);
	rock.transform.parent = GameObject.Find("Sculpture").transform;

	MeshFilter meshFilter = (MeshFilter)rock.AddComponent(typeof(MeshFilter)); //Add a mesh filter.
	renderer.material = rockmat;
	meshFilter.sharedMesh = CreateMesh(Vertices);
	meshFilter.sharedMesh.name = "Temporary Sculpture";

	Mesh lowpolymesh = MakeLowPoly(rock.transform, "Temporary Sculpture");
	BoxUV(lowpolymesh, rock.transform);
}
