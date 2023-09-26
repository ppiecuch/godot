
class RockStudio : EditorWindow {
	// Generation Settings
	private float numberOfVerticesFloat = 25f;
	private const float maximumResolution = 1000f;
	public bool generateWithRigidbody;
	public enum MethodOfSeed
	{
		Random,
		Custom
	}
	public static MethodOfSeed SeedMethod;
	public static int Seed;
	public static int SeedSize = 5000;

	private bool showGenerationInfo;

	// Rock Information
	private Material rockMaterial;
	private Material[] rockMaterials = new Material[4];
	private int materialPickerIndex;
	private int numberOfVerticesInt = 25;
	private float rockYPositionOffset;
	private int rockOrientation; // 0 = X, 1 = Y, 2 = Z
	private string rockName = "ExampleRock";
	private GameObject rockGroup;
	private string rockGroupName = "RockGroup";
	public List<Vector3> points;

	// Placement Tool
	private RaycastHit mouseHitPoint;
	private static bool currentlyPlacingRock;
	private Transform placementToolLocation;
	private Event currentEvent;
	private Vector3 currentMousePosition = Vector3.zero;
	private float placementToolSize = 1f;
	private Color outerColor = new Color(0.15f, 0.75f, 1f);
	private Color innerColor = new Color(0.15f, 0.75f, 1f, 0.1f);

	// GUIStyles and GUIContents
	GUIStyle helpbox;
	private GUIStyle centeredLabel;
	private static Texture2D[] materialPreviews;
	private GUIStyle[] materialPickers;
	private GUIContent multipleMaterialsIcon;
	private GUIContent removeSourceMeshIcon;
	private GUIContent meshCountIcon;
	private GUIContent vertexCountIcon;
	GUIContent basicMenuHeader;
	GUIContent composeMenuHeader;
	GUIContent sculptMenuHeader;
	GUIContent wearMenuHeader;
	GUIContent combineMenuHeader;
	GUIContent exportMenuHeader;
	GUIContent settingsMenuHeader;
	GUIContent orientationX;
	GUIContent orientationY;
	GUIContent orientationZ;
	private GUIStyle smallButton;
	private GUIStyle toolBarButton;
	private GUIContent addrigidbody;
	bool StylesNotLoaded = true;

	// UI Elements
	public int toolBarIndex = 0;
	int rowWidth = 76; // row width = (number of columns * 25) + 1
	int columnHeight = 36; // column height = (6 / number of columns) * 18
	private Texture[] toolbarIcons;
	int heightOffset = 2;
	public static Color lineDividerColor = new Color(0.6f, 0.6f, 0.6f);

	// Basic Settings
	string[] rockTypes = { "Cubic", "Boulder", "Quartz", "Custom" };
	int typeOfrock;
	float edgeWidth = 1f;
	float edgeHeight = 1f;
	float edgeDepth = 1f;
	float boulderRadius = 1f;
	private float tipProtrusion = 2f;
	private float tipFlatness = 1f;
	private float baseHeight = 2f;
	private float baseWidth = 2f;
	private bool tetragonal;
	private bool oneSided;
	private Mesh meshVolume;

	// Sculpt Settings
	string[] compositionModes = { "Fill", "Outline" };
	int modeOfComposition;
	string sculptureName;

	// Export Settings
	GameObject ExportObject;
	string[] exportOptions = { ".fbx" };
	int exportType;
	public static string DefaultPath = "Assets/";

	// Combine Settings
	GameObject parentObject;
	public bool multipleMaterials;
	public bool addRigidBody;
	public bool removeSourceMesh;
	public string newMeshName;
	string[] colliderOptions = { "Box Collider", "Mesh Collider", "None" };
	int colliderType;

	[MenuItem("Tools/RockStudio/RockStudio")]
	static void Init()
	{
		RockStudio window = (RockStudio)GetWindow(typeof(RockStudio));
		window.Show();

		window.titleContent = new GUIContent("Rocks");
		window.titleContent.tooltip = "RockStudio v3.0.0";
		window.position = new Rect(500, 500, 90, 500);
	}

	void OnEnable() {
		materialPreviews = new Texture2D[4];
		for (int i = 0; i < 4; i++) {
			materialPreviews[i] = new Texture2D(1, 1);
		}

		materialPickers = new GUIStyle[4];
		for (int i = 0; i < 4; i++) {
			materialPickers[i] = new GUIStyle();
		}

		minSize = new Vector2(90, 350);
		maxSize = new Vector2(150, 550);

		if (EditorGUIUtility.isProSkin) {
			toolBarIndex = 0;
			toolbarIcons = new Texture[6];
			toolbarIcons[0] = EditorGUIUtility.Load("Assets/Ameye/RockStudio/Icons/basic16.png") as Texture2D;
			toolbarIcons[1] = EditorGUIUtility.Load("Assets/Ameye/RockStudio/Icons/sculpting16.png") as Texture2D;
			toolbarIcons[2] = EditorGUIUtility.Load("Assets/Ameye/RockStudio/Icons/wear16.png") as Texture2D;
			toolbarIcons[3] = EditorGUIUtility.Load("Assets/Ameye/RockStudio/Icons/combine16.png") as Texture2D;
			toolbarIcons[4] = EditorGUIUtility.Load("Assets/Ameye/RockStudio/Icons/save16.png") as Texture2D;
			toolbarIcons[5] = EditorGUIUtility.Load("Assets/Ameye/RockStudio/Icons/settings16.png") as Texture2D;

			basicMenuHeader = new GUIContent("   <b>Basic</b>",
				EditorGUIUtility.Load("Assets/Ameye/RockStudio/Icons/" + "basic16" + ".png") as Texture2D);
			combineMenuHeader = new GUIContent(" <b>Combine</b>",
				EditorGUIUtility.Load("Assets/Ameye/RockStudio/Icons/" + "combine16" + ".png") as Texture2D);
			sculptMenuHeader = new GUIContent("  <b>Sculpt</b>",
				EditorGUIUtility.Load("Assets/Ameye/RockStudio/Icons/" + "sculpting16" + ".png") as Texture2D);
			wearMenuHeader = new GUIContent("  <b> Wear</b>",
				EditorGUIUtility.Load("Assets/Ameye/RockStudio/Icons/" + "wear16" + ".png") as Texture2D);
			settingsMenuHeader = new GUIContent(" <b>Settings</b>",
				EditorGUIUtility.Load("Assets/Ameye/RockStudio/Icons/" + "settings16" + ".png") as Texture2D);
			exportMenuHeader = new GUIContent("  <b>Save</b>",
				EditorGUIUtility.Load("Assets/Ameye/RockStudio/Icons/" + "save16" + ".png") as Texture2D);
		} else {
			toolBarIndex = 0;
			toolbarIcons = new Texture[6];
			toolbarIcons[0] = EditorGUIUtility.Load("Assets/Ameye/RockStudio/Icons/basic16dark.png") as Texture2D;
			toolbarIcons[1] = EditorGUIUtility.Load("Assets/Ameye/RockStudio/Icons/sculpting16dark.png") as Texture2D;
			toolbarIcons[2] = EditorGUIUtility.Load("Assets/Ameye/RockStudio/Icons/wear16dark.png") as Texture2D;
			toolbarIcons[3] = EditorGUIUtility.Load("Assets/Ameye/RockStudio/Icons/combine16dark.png") as Texture2D;
			toolbarIcons[4] = EditorGUIUtility.Load("Assets/Ameye/RockStudio/Icons/save16dark.png") as Texture2D;
			toolbarIcons[5] = EditorGUIUtility.Load("Assets/Ameye/RockStudio/Icons/settings16dark.png") as Texture2D;

			basicMenuHeader = new GUIContent("   <b>Basic</b>", EditorGUIUtility.Load("Assets/Ameye/RockStudio/Icons/" + "basic16dark" + ".png") as Texture2D);
			combineMenuHeader = new GUIContent(" <b>Combine</b>", EditorGUIUtility.Load("Assets/Ameye/RockStudio/Icons/" + "combine16dark" + ".png") as Texture2D);
			sculptMenuHeader = new GUIContent("  <b>Sculpt</b>", EditorGUIUtility.Load("Assets/Ameye/RockStudio/Icons/" + "sculpting16dark" + ".png") as Texture2D);
			wearMenuHeader = new GUIContent("  <b> Wear</b>", EditorGUIUtility.Load("Assets/Ameye/RockStudio/Icons/" + "wear16dark" + ".png") as Texture2D);
			settingsMenuHeader = new GUIContent(" <b>Settings</b>", EditorGUIUtility.Load("Assets/Ameye/RockStudio/Icons/" + "settings16dark" + ".png") as Texture2D);
			exportMenuHeader = new GUIContent("  <b>Save</b>", EditorGUIUtility.Load("Assets/Ameye/RockStudio/Icons/" + "save16dark" + ".png") as Texture2D);
		}

		orientationX = new GUIContent("<b>X</b>", "The orientation of the rock when placing.");
		orientationY = new GUIContent("<b>Y</b>", "The orientation of the rock when placing.");
		orientationZ = new GUIContent("<b>Z</b>", "The orientation of the rock when placing.");

		meshCountIcon = new GUIContent("", EditorGUIUtility.ObjectContent(null, typeof(MeshFilter)).image, "The number of meshes that will be combined.");
		vertexCountIcon = new GUIContent("",
			EditorGUIUtility.Load("Assets/Ameye/RockStudio/Icons/vertex_count_small.png") as Texture2D,
			"The number of vertices that will be combined.");
		addrigidbody = new GUIContent("", EditorGUIUtility.IconContent("Rigidbody2D Icon").image,
			"Add Rigidbody component.");
		multipleMaterialsIcon = new GUIContent("", EditorGUIUtility.IconContent("PreTextureRGB").image,
			"Preserve multiple materials.");
		removeSourceMeshIcon = new GUIContent("", EditorGUIUtility.IconContent("vcs_delete").image,
		"Remove source mesh.");

		rockMaterial = AssetDatabase.GetBuiltinExtraResource<Material>("Default-Diffuse.mat");

		for (int i = 0; i < 4; i++) {
			rockMaterials[i] = rockMaterial;
		}
	}

	private void OnFocus() {
		SceneView.onSceneGUIDelegate -= OnSceneGUI;
		SceneView.onSceneGUIDelegate += OnSceneGUI;
	}

	void LoadStyles() {
		for (int i = 0; i < 4; i++) {
			if (rockMaterials[i].mainTexture != null) {
				materialPreviews[i] = rockMaterials[i].mainTexture as Texture2D;
			} else {
				materialPreviews[i].SetPixel(0, 0, rockMaterials[i].color);
				materialPreviews[i].Apply();
			}

			materialPickers[i].normal.background = materialPreviews[i];
			materialPickers[i].margin = new RectOffset(5, 5, 5, 5);
		}

		helpbox = new GUIStyle(EditorStyles.helpBox)
		{
			alignment = TextAnchor.MiddleLeft,
			fontSize = 10,
			richText = true,
			contentOffset = new Vector2(5f, 0f),
		};

		centeredLabel = new GUIStyle(EditorStyles.label)
		{
			alignment = TextAnchor.MiddleCenter
		};

		smallButton = new GUIStyle(GUI.skin.button)
		{
			alignment = TextAnchor.MiddleCenter,
			margin = new RectOffset(4, 4, 2, 2),
			richText = true,
			padding = new RectOffset(1, 1, 1, 1)
		};

		toolBarButton = new GUIStyle(GUI.skin.button)
		{
			alignment = TextAnchor.MiddleCenter,
			margin = new RectOffset(4, 4, 2, 2),
			richText = true,
			padding = new RectOffset(2, 2, 2, 2)
		};
	}

	void OnGUI() {
		if (StylesNotLoaded) LoadStyles();

		EditorGUILayout.BeginHorizontal(helpbox);
		toolBarIndex = GUILayout.SelectionGrid(toolBarIndex, toolbarIcons, 3, toolBarButton,
			GUILayout.Height(columnHeight));
		EditorGUILayout.EndHorizontal();

		switch (toolBarIndex) {
			case 0: // Basic Creation
				rockYPositionOffset = 0;
				GUILayout.Label(basicMenuHeader, helpbox, GUILayout.Height(columnHeight - heightOffset));
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				GUILayout.Label("Rock", centeredLabel);
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				EditorGUILayout.Space();
				typeOfrock = EditorGUILayout.Popup(typeOfrock, rockTypes);
				EditorGUILayout.Space();

				switch (typeOfrock) {
					// Cubic
					case 0:
						GUILayout.Label("Dimensions");
						EditorGUILayout.BeginHorizontal();
						GUILayout.Label("X");
						edgeWidth = EditorGUILayout.FloatField(edgeWidth);
						EditorGUILayout.EndHorizontal();
						EditorGUILayout.BeginHorizontal();
						GUILayout.Label("Y");
						edgeHeight = EditorGUILayout.FloatField(edgeHeight);
						EditorGUILayout.EndHorizontal();
						EditorGUILayout.BeginHorizontal();
						GUILayout.Label("Z");
						edgeDepth = EditorGUILayout.FloatField(edgeDepth);
						EditorGUILayout.EndHorizontal();
						break;

					// Boulder
					case 1:
						GUILayout.Label("Radius");
						boulderRadius = EditorGUILayout.FloatField(boulderRadius);
						break;

					// Quartz
					case 2:
						GUILayout.Label("Protrusion");
						tipProtrusion = EditorGUILayout.FloatField(tipProtrusion);
						GUILayout.Label("Flat Tip " + Math.Round(tipFlatness * 10) + "%");
						tipFlatness = GUILayout.HorizontalSlider(tipFlatness, 0.1f, 10f);
						GUILayout.Label("Height");
						baseHeight = EditorGUILayout.FloatField(baseHeight);
						GUILayout.Label("Width");
						baseWidth = EditorGUILayout.FloatField(baseWidth);
						EditorGUILayout.Space();
						tetragonal = GUILayout.Toggle(tetragonal, " Tetragonal");
						oneSided = GUILayout.Toggle(oneSided, " One Sided");
						break;

					// Custom
					case 3:
						GUILayout.Label("Mesh");
						meshVolume = (Mesh)EditorGUILayout.ObjectField(meshVolume, typeof(Mesh), true);
						EditorGUILayout.Space();
						GUILayout.Label( "Do not use complex meshes!", helpbox );
						break;
				}

				EditorGUILayout.Space();
				GUILayout.Label("Materials");
				EditorGUILayout.BeginHorizontal();
				if (GUILayout.Button(GUIContent.none, materialPickers[0], GUILayout.Height(17))) {
					materialPickerIndex = 1;
					EditorGUIUtility.ShowObjectPicker<Material>(null, true, "", materialPickerIndex);
				}

				if (GUILayout.Button(GUIContent.none, materialPickers[1], GUILayout.Height(17))) {
					materialPickerIndex = 2;
					EditorGUIUtility.ShowObjectPicker<Material>(null, true, "", materialPickerIndex);
				}

				if (GUILayout.Button(GUIContent.none, materialPickers[2], GUILayout.Height(17))) {
					materialPickerIndex = 3;
					EditorGUIUtility.ShowObjectPicker<Material>(null, true, "", materialPickerIndex);
				}

				if (GUILayout.Button(GUIContent.none, materialPickers[3], GUILayout.Height(17))) {
					materialPickerIndex = 4;
					EditorGUIUtility.ShowObjectPicker<Material>(null, true, "", materialPickerIndex);
				}

				if (Event.current.commandName == "ObjectSelectorUpdated") {
					currentlyPlacingRock = false;
				}

				if (Event.current.commandName == "ObjectSelectorClosed") {
					switch (materialPickerIndex) {
						case 1:
							if (EditorGUIUtility.GetObjectPickerObject() != null)
								rockMaterials[0] = (Material)EditorGUIUtility.GetObjectPickerObject();
							break;
						case 2:
							if (EditorGUIUtility.GetObjectPickerObject() != null)
								rockMaterials[1] = (Material)EditorGUIUtility.GetObjectPickerObject();
							break;
						case 3:
							if (EditorGUIUtility.GetObjectPickerObject() != null)
								rockMaterials[2] = (Material)EditorGUIUtility.GetObjectPickerObject();
							break;
						case 4:
							if (EditorGUIUtility.GetObjectPickerObject() != null)
								rockMaterials[3] = (Material)EditorGUIUtility.GetObjectPickerObject();
							break;
					}
				}

				EditorGUILayout.EndHorizontal();
				EditorGUILayout.Space();
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				GUILayout.Label("Saving", centeredLabel);
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				GUILayout.Label("Name");
				rockName = EditorGUILayout.TextField("", rockName);
				EditorGUILayout.Space();
				GUILayout.Label("Collider");

				if (typeOfrock != 2) {
					rockOrientation = 2;
				}

				EditorGUILayout.BeginHorizontal();
				if (typeOfrock == 2 || typeOfrock == 3) {
					colliderType = EditorGUILayout.Popup(colliderType, colliderOptions,
						GUILayout.Height(columnHeight / 2f));
				} else {
					colliderType = EditorGUILayout.Popup(colliderType, colliderOptions,
						GUILayout.Height(columnHeight / 2f));
				}

				if (rockOrientation == 0 && typeOfrock == 2 || typeOfrock == 3) {
					smallButton.normal.textColor = new Color(0.8588f, 0.2431f, 0.1137f);
					if (GUILayout.Button(orientationX, smallButton, GUILayout.Width(rowWidth / 4f),
						GUILayout.Height(columnHeight / 2 - 3)))
						rockOrientation = (rockOrientation + 1) % 3;
				} else if (rockOrientation == 1 && typeOfrock == 2 || typeOfrock == 3) {
					smallButton.normal.textColor = new Color(0.18f, 0.77f, 0.2f);
					if (GUILayout.Button(orientationY, smallButton, GUILayout.Width(rowWidth / 4f),
						GUILayout.Height(columnHeight / 2 - 3)))
						rockOrientation = (rockOrientation + 1) % 3;
				} else if (rockOrientation == 2 && typeOfrock == 2 || typeOfrock == 3) {
					smallButton.normal.textColor = new Color(0f, 0.04f, 0.97f);
					if (GUILayout.Button(orientationZ, smallButton, GUILayout.Width(rowWidth / 4f),
						GUILayout.Height(columnHeight / 2 - 3)))
						rockOrientation = (rockOrientation + 1) % 3;
				}
				EditorGUILayout.EndHorizontal();

				if (!currentlyPlacingRock) {
					if (GUILayout.Button("Generate")) {
						MeshUtilities.RemoveGameObject("_TemporaryRock");
						currentlyPlacingRock = !currentlyPlacingRock;
					}
				} else {
					if (GUILayout.Button("Cancel")) CancelPlacement();
				}

				EditorGUILayout.Space();
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				EditorGUILayout.Space();

				if (rockName == "") {
					GUILayout.Label("Rock name was left blank.", helpbox);
				}
				break;

			case 1: // Sculpting
				CancelPlacement();
				GUILayout.Label(sculptMenuHeader, helpbox, GUILayout.Height(columnHeight - heightOffset));
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				GUILayout.Label("Sculpture", centeredLabel);
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				EditorGUILayout.Space();
				GUILayout.Label("Mode");
				modeOfComposition = EditorGUILayout.Popup(modeOfComposition, compositionModes);
				EditorGUILayout.Space();

				GUILayout.Label("Sculpture");
				if (GUILayout.Button("New")) {
					if (GameObject.Find("Sculpture in progress") != null) {
						DestroyImmediate(GameObject.Find("Sculpture in progress"));
					}
					NewSculpture();
					AddShape();
					SceneView.lastActiveSceneView.FrameSelected();
				}

				if (GUILayout.Button("Add Shape")) {
					if (GameObject.Find("Sculpture in progress") != null) {
						AddShape();
					} else {
						Debug.Log("<color=cyan>[RockStudio] </color>No sculpture was found. Start a new sculpture first.");
					}
				}

				if (GUILayout.Button("Generate")) {
					if (GameObject.Find("Sculpture in progress") != null) {
						if (modeOfComposition == 1)
							SculptureCreator.GenerateComposition(numberOfVerticesInt, Vector3.zero, Vector3.zero, rockMaterial);
						else
							SculptureCreator.GenerateCompositionFill(numberOfVerticesInt, Vector3.zero, Vector3.zero, rockMaterial);

						DestroyImmediate(GameObject.Find("Sculpture in progress"));
						Debug.Log("<color=cyan>[RockStudio] </color>Sculpture was generated. Don't forget to save it.");
					}
					else Debug.Log("<color=cyan>[RockStudio] </color>No sculpture was found. Start a new sculpture first.");
				}

				EditorGUILayout.Space();
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				GUILayout.Label("Saving", centeredLabel);
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				EditorGUILayout.Space();
				GUILayout.Label("Name");
				sculptureName =
					EditorGUILayout.TextField("", sculptureName);
				EditorGUILayout.Space();
				GUILayout.Label("Collider");
				colliderType = EditorGUILayout.Popup(colliderType, colliderOptions,
					GUILayout.Height(columnHeight / 2f));

				if (GUILayout.Button("Save"))
				{
					if (GameObject.Find("Sculpture") != null)
					{
						GameObject oldStructure = GameObject.Find("Sculpture");
						if (sculptureName == "") Debug.Log("<color=cyan>[RockStudio] </color>Sculpture name was left empty.");
						else
						{
							GameObject newStructure = MeshCombiner.Combine(sculptureName, oldStructure, true);

							MeshRenderer renderer = newStructure.GetComponent(typeof(MeshRenderer)) as MeshRenderer;
							renderer.material = rockMaterial;

							if (addRigidBody) newStructure.AddComponent(typeof(Rigidbody));

							if (colliderType == 0) {
								newStructure.AddComponent(typeof(BoxCollider));
							}
							if (colliderType == 1) {
								MeshCollider meshcollider =
									newStructure.AddComponent(typeof(MeshCollider)) as MeshCollider;
								meshcollider.convex = true;
								meshcollider.sharedMesh = newStructure.GetComponent<MeshFilter>().sharedMesh;
							}

							DestroyImmediate(GameObject.Find("Sculpture in progress"));
							DestroyImmediate(GameObject.Find("Sculpture"));
							Debug.Log("<color=cyan>[RockStudio] </color>No sculpture was found. Start a new sculpture first.");
						}
					}
					else
						Debug.Log("<color=cyan>[RockStudio] </color>No mesh was found to save. Generate a sculpture mesh first.");
				}

				EditorGUILayout.Space();
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				EditorGUILayout.Space();
				break;

			case 2: // Wear
				CancelPlacement();
				GUILayout.Label(wearMenuHeader, helpbox,
					GUILayout.Height(columnHeight - heightOffset));
				GUI.color = Color.white;
				EditorGUILayout.Space();
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				EditorGUILayout.Space();
				GUILayout.Label("Coming in future updates.", helpbox
				);
				EditorGUILayout.Space();
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				break;

			case 3: // MESH COMBINING
				CancelPlacement();
				GUILayout.Label(combineMenuHeader, helpbox,
					GUILayout.Height(columnHeight - heightOffset));
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				GUILayout.Label("Objects", centeredLabel);
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				EditorGUILayout.Space();
				GUILayout.Label("Parent Object");
				parentObject = (GameObject)EditorGUILayout.ObjectField(parentObject, typeof(GameObject), true
				);
				EditorGUILayout.Space();
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				GUILayout.Label("Saving", centeredLabel);
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				EditorGUILayout.Space();
				GUILayout.Label("Name");
				newMeshName = EditorGUILayout.TextField("", newMeshName);
				EditorGUILayout.Space();
				GUILayout.Label("Options");
				colliderType = EditorGUILayout.Popup(colliderType, colliderOptions
				);
				EditorGUILayout.BeginHorizontal();
				multipleMaterials = GUILayout.Toggle(multipleMaterials, multipleMaterialsIcon, toolBarButton,
					GUILayout.Width(rowWidth / 3f), GUILayout.Height(columnHeight / 2f));
				addRigidBody = GUILayout.Toggle(addRigidBody, addrigidbody, toolBarButton, GUILayout.Width(rowWidth / 3),
					GUILayout.Height(columnHeight / 2f));
				removeSourceMesh = GUILayout.Toggle(removeSourceMesh, removeSourceMeshIcon, toolBarButton,
					GUILayout.Width(rowWidth / 3f), GUILayout.Height(columnHeight / 2f));
				EditorGUILayout.EndHorizontal();
				if (GUILayout.Button("Combine"))
				{
					if (parentObject != null && MeshUtilities.ChildrenAllGameObjects(parentObject))
					{
						GameObject combinedGameObject =
							MeshCombiner.Combine(newMeshName, parentObject, multipleMaterials);
						if (removeSourceMesh) Undo.DestroyObjectImmediate(parentObject);
						if (addRigidBody) combinedGameObject.AddComponent(typeof(Rigidbody));
						if (colliderType == 0) combinedGameObject.AddComponent(typeof(BoxCollider));
						if (colliderType == 1)
						{
							MeshCollider meshcollider =
								combinedGameObject.AddComponent(typeof(MeshCollider)) as MeshCollider;
							meshcollider.convex = true;
							meshcollider.sharedMesh = combinedGameObject.GetComponent<MeshFilter>().sharedMesh;
						}
						Selection.activeGameObject = combinedGameObject;
					}

					else Debug.Log("<color=cyan>[RockStudio] </color>Parent object field was left empty.");
				}
				EditorGUILayout.Space();
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				if (parentObject && MeshUtilities.ChildrenAllGameObjects(parentObject))
				{
					if (parentObject.transform.childCount != 0)
					{
						EditorGUILayout.BeginHorizontal();
						GUILayout.Label(meshCountIcon, GUILayout.Width(rowWidth / 3),
							GUILayout.Height(columnHeight / 2));
						GUILayout.Label(parentObject.transform.childCount.ToString());
						EditorGUILayout.EndHorizontal();

						EditorGUILayout.BeginHorizontal();
						GUILayout.Label(vertexCountIcon, GUILayout.Width(rowWidth / 3),
							GUILayout.Height(columnHeight / 2));
						GUILayout.Label(MeshUtilities.GetVerts(parentObject).ToString());
						EditorGUILayout.EndHorizontal();
						EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
					}

					else
					{
						EditorGUILayout.BeginHorizontal();
						GUILayout.Label(meshCountIcon, GUILayout.Width(rowWidth / 3),
							GUILayout.Height(columnHeight / 2));
						GUILayout.Label("1");
						EditorGUILayout.EndHorizontal();

						EditorGUILayout.BeginHorizontal();
						GUILayout.Label(vertexCountIcon, GUILayout.Width(rowWidth / 3),
							GUILayout.Height(columnHeight / 2));
						GUILayout.Label(parentObject.transform.GetComponent<MeshFilter>().sharedMesh.vertexCount
							.ToString());
						EditorGUILayout.EndHorizontal();
						EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
					}
				}
				EditorGUILayout.Space();
				GUILayout.Label("Add a parent that contains the objects you want to combine.", helpbox
				);
				break;

			case 4: // Exporting
				CancelPlacement();
				GUILayout.Label(exportMenuHeader, helpbox,
					GUILayout.Height(columnHeight - heightOffset));
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				GUILayout.Label("Objects", centeredLabel);
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				EditorGUILayout.Space();
				GUILayout.Label("Type");
				exportType = EditorGUILayout.Popup(exportType, exportOptions,
					GUILayout.Height(columnHeight / 2f));
				EditorGUILayout.Space();
				GUILayout.Label("Object");
				ExportObject = (GameObject)EditorGUILayout.ObjectField(ExportObject, typeof(GameObject), true);
				EditorGUILayout.Space();
				if (GUILayout.Button("Export"))
				{
					Exporter.ExportGameObject(ExportObject, false, false);
					Debug.Log("<color=cyan>[RockStudio] </color>Mesh exported.");
				}
				EditorGUILayout.Space();
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				break;

			case 5: // Settings
				CancelPlacement();
				GUILayout.Label(settingsMenuHeader, helpbox,
					GUILayout.Height(columnHeight - heightOffset));
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				GUILayout.Label("Generation", centeredLabel);
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				GUILayout.Label("Resolution");
				numberOfVerticesFloat = GUILayout.HorizontalSlider(numberOfVerticesFloat, 10f, maximumResolution
				);
				numberOfVerticesInt = Mathf.RoundToInt(numberOfVerticesFloat);
				EditorGUILayout.BeginHorizontal();
				GUILayout.Label(vertexCountIcon, GUILayout.Width(rowWidth / 3),
					GUILayout.Height(columnHeight / 2));
				GUILayout.Label(numberOfVerticesInt.ToString());
				EditorGUILayout.EndHorizontal();
				EditorGUILayout.Space();
				GUILayout.Label("Rigidbody");
				generateWithRigidbody = GUILayout.Toggle(generateWithRigidbody, " add");
				EditorGUILayout.Space();
				GUILayout.Label("Group Name");
				rockGroupName =
					EditorGUILayout.TextField("", rockGroupName);
				EditorGUILayout.Space();
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				GUILayout.Label("Seeds", centeredLabel);
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				EditorGUILayout.Space();
				GUILayout.Label("Generation");
				SeedMethod = (MethodOfSeed)EditorGUILayout.EnumPopup("", SeedMethod);
				EditorGUILayout.Space();

				if (SeedMethod == MethodOfSeed.Custom)
				{
					GUILayout.Label("Value");
					Seed = EditorGUILayout.IntField("", Seed);
				}

				if (SeedMethod == MethodOfSeed.Random)
				{
					GUILayout.Label("Max Value");
					SeedSize = EditorGUILayout.IntField("", SeedSize);
				}
				EditorGUILayout.Space();
				GUILayout.Label("Debug Info");
				showGenerationInfo = GUILayout.Toggle(showGenerationInfo, " show");
				EditorGUILayout.Space();
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				GUILayout.Label("Saving", centeredLabel);
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				EditorGUILayout.Space();
				EditorGUILayout.LabelField("Folder:");
				string[] splitString = DefaultPath.Split(new string[] { "/" }, StringSplitOptions.None);
				EditorGUILayout.LabelField(splitString[splitString.Length - 1] + "/");
				if (GUILayout.Button("Change")) DefaultPath = Exporter.ChangePath(DefaultPath);
				EditorGUILayout.Space();
				EditorUtilities.DrawUILine(lineDividerColor, 2, 0);
				break;
		}
	}

	void OnSceneGUI(SceneView sceneView)
	{
		currentEvent = Event.current;
		sceneInput();
		updateMousePos(sceneView);
		drawGizmo();

		if (toolBarIndex != 0) return;

		Event current = Event.current;
		int controlID = GUIUtility.GetControlID(FocusType.Passive);

		switch (current.type)
		{
			case EventType.MouseUp:
				{
					if (currentlyPlacingRock && current.button == 0 && !current.alt)
					{
						GenerateRock(mouseHitPoint.point, mouseHitPoint.normal);
						Repaint();
					}
					break;
				}
			case EventType.Layout:
				if (currentlyPlacingRock) HandleUtility.AddDefaultControl(controlID);
				break;
		}
	}

	void NewSculpture()
	{
		GameObject newSculpture = new GameObject("Sculpture in progress");
		Undo.RegisterCompleteObjectUndo(newSculpture, "Create");
	}

	void AddShape()
	{
		GameObject NewRockShape = new GameObject("Sculpture point");
		Undo.RegisterCreatedObjectUndo(NewRockShape, "Create");

		NewRockShape.transform.parent = GameObject.Find("Sculpture in progress").transform;
		NewRockShape.AddComponent<Handle>();
		Selection.activeGameObject = NewRockShape;
	}

private:
	void GenerateRock(Vector3 pos, Vector3 nor) {
		points = new List<Vector3>(numberOfVerticesInt);
		points.Clear();
		Random.state = Random.state;

		if (SeedMethod == MethodOfSeed.Random) {
			Random.state = Random.state;
			Seed = Random.Range(0, SeedSize);
			Random.InitState(Seed);
		} else {
			Random.InitState(Seed);
		}

		rockMaterial = rockMaterials[Random.Range(0, 4)];

		switch (typeOfrock) {
			case 0: {
				points = MeshUtilities.GetRandomPointsWithinCube(numberOfVerticesInt, edgeWidth, edgeHeight, edgeDepth);
			} break;
			case 1: {
				points = MeshUtilities.GetRandomPointsWithinSphere(numberOfVerticesInt, boulderRadius);
			} break;
			case 2: {
				points = MeshUtilities.GetRandomPointsWithinCrystal(numberOfVerticesInt, tetragonal, oneSided, baseWidth, baseHeight, tipProtrusion, tipFlatness);
			} break;
			case 3: {
				points = MeshUtilities.GetRandomPointsOnMesh(numberOfVerticesInt, meshVolume);
			} break;
		}

		if (GameObject.Find(rockGroupName) == nullptr) {
			rockGroup = new GameObject(rockGroupName);
			Undo.RegisterCreatedObjectUndo(rockGroup, "Create");
			rockGroup.transform.position = pos;
		} else {
			rockGroup = GameObject.Find(rockGroupName);
		}

		RockCreation.CreateRock(points, pos + new Vector3(0, rockYPositionOffset, 0), nor, rockOrientation, rockMaterial, rockName, generateWithRigidbody, colliderType, rockGroup, showGenerationInfo);
	}
}
