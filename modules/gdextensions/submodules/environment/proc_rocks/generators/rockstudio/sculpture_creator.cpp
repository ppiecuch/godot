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

	MeshFilter meshFilter = (MeshFilter)rock.AddComponent(typeof(MeshFilter));  //Add a mesh filter.

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

	MeshFilter meshFilter = (MeshFilter)rock.AddComponent(typeof(MeshFilter));  //Add a mesh filter.
	renderer.material = rockmat;
	meshFilter.sharedMesh = CreateMesh(Vertices);
	meshFilter.sharedMesh.name = "Temporary Sculpture";

	Mesh lowpolymesh = MakeLowPoly(rock.transform, "Temporary Sculpture");
	BoxUV(lowpolymesh, rock.transform);
}
