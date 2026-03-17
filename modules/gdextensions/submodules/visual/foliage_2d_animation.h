#include "core/list.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"

class Foliage2D_Mesh {
	List<Vector3> meshVerts; // List of vertices to build a mesh from.
	List<int> meshIndices; // List of vertex indices that form triangles.
	List<Vector2> meshUVs; // List of UV coordinates for each vertex.

public:
	// Clears all verices, indices, uvs from this mesh.
	void Clear() {
		meshVerts.Clear();
		meshIndices.Clear();
		meshUVs.Clear();
	}

	// Clears out the mesh, fills in the data, and recalculates normals and bounds.
	// mesh: An already existing mesh to fill out.
	void Build(ref Mesh mesh) {
		// round off a few decimal points to try and get better pixel-perfect results
		for (int i = 0; i < meshVerts.Count; i += 1)
			meshVerts[i] = Vector3(
					(float)Math::round(meshVerts[i].x, 3),
					(float)Math::round(meshVerts[i].y, 3),
					(float)Math::round(meshVerts[i].z, 3));

		mesh.Clear();
		mesh.vertices = meshVerts.ToArray();
		mesh.uv = meshUVs.ToArray();
		mesh.triangles = meshIndices.ToArray();

		mesh.RecalculateBounds();
		mesh.RecalculateNormals();
	}

	// Generates triangles from a list of vertices.
	// widthSegments: The number of horizontal segments.
	// heightSegments: The number of vertical segments.
	// hVertices: The number of horizontal vertices.
	void GenerateTriangles(int widthSegments, int heightSegments, int hVertices) {
		for (int y = 0; y < heightSegments; y++) {
			for (int x = 0; x < widthSegments; x++) {
				meshIndices.Add((y * hVertices) + x);
				meshIndices.Add(((y + 1) * hVertices) + x);
				meshIndices.Add((y * hVertices) + x + 1);

				meshIndices.Add(((y + 1) * hVertices) + x);
				meshIndices.Add(((y + 1) * hVertices) + x + 1);
				meshIndices.Add((y * hVertices) + x + 1);
			}
		}
	}

	// Adds a vertex to the meshVerts list and a UV point to the meshUVs list.
	// vertexPoss">The position of a vertex.
	// z: The position of a vertex on the Z axis.
	// UV: The UV coordinate of the current vertex.
	void AddVertex(Vector2 vertexPoss, float z, Vector2 UV) {
		meshVerts.Add(new Vector3(vertexPoss.x, vertexPoss.y, z));
		meshUVs.Add(UV);
	}

	Foliage2D_Mesh() {}
}

// Describes how the foliage objects should be arranged on the path lines.
enum FOLIAGE2D_PATTERN {
	// When a new object must be instantiated, a random value between 0
	// and the max number of foliage prefabs, is generated. This value is
	// used to decide which prefab to instantiate.
	Random,
	// The objects are instantiated and arranged in a consecutive order.
	Consecutive,
}

// Describes the overlapping type. The value of the overlapping factor determines which part of a
// foliage object mesh will be placed to the left of the right edge of the previous foliage object mesh.
enum FOLIAGE2D_OVERLAPPINGTYPE {
	Fixed, // The overlapping value is constant for all objects.
	Random, // The overlapping value is a random value between a max and min for every object.
}

// Describes the path type.
enum FOLIAGE2D_PATHTYPE {
	Linear, // The foliage objects are arranged in straight line and bottom aligned to the path lines.
	Smooth, // The foliage objects are arranged on a smoothed line.
}

class Foliage2DPath {
	Foliage2D foliage2D; // Instace to a Foliage2D object script.
	Vector2 posOffset; // Foliage object position offset. This is what places the foliage object on top of the path line.
	Vector2 lineNormal; // The normal of a path segment.
	Vector2 pointOnTheLine; // A point on the line described by 2 path nodes.
	Vector2 line; // A line described by 2 path nodes.
	float foliagePrefabWidth; // The mesh width of a foliage object.
	float distanceFromStart = 0; // How far from the start point of a line described by 2 path nodes, the current object should be placed.
	float prevDistanceFromStart = 0; // How far from the start point of a line described by 2 path nodes, the previous object was placed.
	float previousWidth = 0; // The mesh width of the previous foliage object.
	float lineLength; // The length of the line described by 2 path nodes.
	float angleInRad; // The angle in radians of a foliage object.
	float angleInDeg; // The angle in degrees of a foliage object.
	int foliageCount; // The total number of foliage objects that are placed on the current path.
	int objectIndex;
	/ The index of the current foliage object.int prevIndex; // The index of the previous foliage object.
	int maxCount; // The total number of foliage prefabs.
	int prefabIndex = -1; // The index of a foliage prefab. It is used to decide which prefab to instantiate.
	bool updatedPrefabIndex = false; // It is used to determine if the "prefabIndex" value changed.

	// This method is called only when the foliage pattern is set to consecutive.
	// Restores the prefab index when an object is deleted or this is the last object
	// on the line.
	void RestorePrefabIndex() {
		prefabIndex--;
		if (prefabIndex == -1) {
			prefabIndex = maxCount - 1;
		}
	}

	// Calculates the distance from the start of the line, described by 2 path nodes,
	// where the current object must be placed
	void DistanceFromStart() {
		// This segment of code is executed if this is the first object on the path
		// or the first on the line described by 2 path nodes.
		if (objectIndex == 1 || distanceFromStart == 0) {
			distanceFromStart = foliage2D.width / 2f + firstObjectOffset;

			// If the value of "firstObjectOffset" is negative and its absolute value is equal to
			// half the width of the mesh, we would get stuck in an infinite loop, so we make sure
			// that "distanceFromStart" never has the value of 0.
			if (distanceFromStart == 0) {
				distanceFromStart = 0.0001;
			}
			// The distance from the start point of the current line and mesh width of
			// the current object are saved so that we can determine where the next object
			// should be placed on the line in relation to current object.
			prevDistanceFromStart = distanceFromStart;
			previousWidth = foliage2D.width;
			// This makes sure that no matter the height of the mesh, all foliage objects are
			// bottom aligned to the foliage path.
			posOffset = lineNormal * (foliage2D.height / 2);
		} else {
			float currentOverlappingFactor;

			if (foliageOverlapType == Foliage2D_OverlappingType.Fixed) :
				currentOverlappingFactor = overlappingFactor;
		}
		else {
			currentOverlappingFactor = Random.Range(minOverlappingFactor, maxOverlappingFactor);
		}
		// The position on the line for the current object is calculated based on
		// the value of "currentOverlappingFactor" and the position on the line
		// and mesh width of the previous object.
		if (currentOverlappingFactor <= 0.5) {
			distanceFromStart = prevDistanceFromStart + (previousWidth / 2) + ((foliage2D.width / 2) - (foliage2D.width * currentOverlappingFactor));
		} else {
			distanceFromStart = prevDistanceFromStart + (previousWidth / 2) - ((foliage2D.width * currentOverlappingFactor) - (foliage2D.width / 2));
		}
		prevDistanceFromStart = distanceFromStart;
		previousWidth = foliage2D.width;
		posOffset = lineNormal * (foliage2D.height / 2);
	}
}

// Smooths a path segment.
// index: Index of the path node.
void
SmoothPath(int index) {
	Vector2 startPoint = Vector2.zero; // The first of the 4 path points.
	Vector2 endPoint = Vector2.zero; // The last of the 4 path points.
	Vector2 tanget;

	// If this is the first path node, an imaginary node is created that has the same slope as the start node.
	if (index == 0) {
		startPoint = new Vector2(handlesPosition[index].x - 1 * line.x, handlesPosition[index].y - 1 * line.y);
	} else {
		startPoint = handlesPosition[index - 1];
	}
	// If this is the last path node minus 2, an imaginary node is created that has the same slope as the last node.
	if (index == handlesPosition.Count - 2) {
		endPoint = new Vector2(handlesPosition[index + 1].x - 1 * line.x, handlesPosition[index + 1].y - 1 * line.y);
	} else {
		endPoint = handlesPosition[index + 2];
	}
	// The smoothed position of the object is calculated using the hermite interpolation method.
	pointOnTheLine = new Vector2(
			HermiteInterpolation(startPoint.x, handlesPosition[index].x, handlesPosition[index + 1].x, endPoint.x, index),
			HermiteInterpolation(startPoint.y, handlesPosition[index].y, handlesPosition[index + 1].y, endPoint.y, index));

	tanget = GetTangent(index, startPoint, endPoint);
	angleInRad = Math::Atan2(tanget.y, tanget.x);
	angleInDeg = angleInRad * Math::Rad2Deg;
}

// Calculates the position of an object using the hermite interpolation method.
// p1: The position on the x or y axis of the first node.
// p2: The position on the x or y axis of the second node.
// p3: The position on the x or y axis of the third node.
// p4: The position on the x or y axis of the fourth node.
// index: Index of the main path node.
// returns: The interpolated position of an object on the X or Y axis.
float HermiteInterpolation(float p1, float p2, float p3, float p4, int index) {
	float m0, m1, mu, mu2, mu3;
	float a0, a1, a2, a3;
	float currentTension;
	float currentBias;

	mu = distanceFromStart / lineLength;

	if (uniformValues) {
		currentTension = tension;
		currentBias = bias;
	} else {
		currentTension = Math::Lerp(nodeTension[index], nodeTension[index + 1], mu);
		currentBias = Math::Lerp(nodeBias[index], nodeBias[index + 1], mu);
	}

	mu2 = mu * mu;
	mu3 = mu2 * mu;
	m0 = (p2 - p1) * (1 + currentBias) * (1 - currentTension) / 2 + (p3 - p2) * (1 - currentBias) * (1 - currentTension) / 2;
	m1 = (p3 - p2) * (1 + currentBias) * (1 - currentTension) / 2 + (p4 - p3) * (1 - currentBias) * (1 - currentTension) / 2;
	a0 = 2 * mu3 - 3 * mu2 + 1;
	a1 = mu3 - 2 * mu2 + mu;
	a2 = mu3 - mu2;
	a3 = -2 * mu3 + 3 * mu2;

	return (a0 * p2 + a1 * m0 + a2 * m1 + a3 * p3);
}

Vector2 GetTangent(int index, Vector2 startPoint, Vector2 endPoint) {
	float mu = distanceFromStart / lineLength;
	return new Vector2(
			HermiteSlope(startPoint.x, handlesPosition[index].x, handlesPosition[index + 1].x, endPoint.x, mu, index),
			HermiteSlope(startPoint.y, handlesPosition[index].y, handlesPosition[index + 1].y, endPoint.y, mu, index));
}

float HermiteSlope(float p1, float p2, float p3, float p4, float mu, int index) {
	float m0, m1, mu2;
	float a0, a1, a2, a3;
	float currentTension;
	float currentBias;

	if (uniformValues) {
		currentTension = tension;
		currentBias = bias;
	} else {
		currentTension = Math::Lerp(nodeTension[index], nodeTension[index + 1], mu);
		currentBias = Math::Lerp(nodeBias[index], nodeBias[index + 1], mu);
	}

	mu2 = mu * mu;
	m0 = ((1 - currentTension) * (currentBias + 1) * (p2 - p1) + (1 - currentBias) * (1 - currentTension) * (p3 - p2)) / 2;
	m1 = ((1 - currentTension) * (currentBias + 1) * (p3 - p2) + (1 - currentBias) * (1 - currentTension) * (p4 - p3)) / 2;
	a0 = (3 * mu2 - 4 * mu + 1);
	a1 = (3 * mu2 - 2 * mu);
	a2 = p2 * (6 * mu2 - 6 * mu);
	a3 = p3 * (6 * mu - 6 * mu2);

	return (a0 * m0 + a1 * m1 + a2 + a3);
}

public:
Foliage2D_Pattern foliagePattern = Foliage2D_Pattern.Random;
Foliage2D_OverlappingType foliageOverlapType = Foliage2D_OverlappingType.Fixed;
Foliage2D_PathType foliagePathType = Foliage2D_PathType.Linear;
List<Vector2> handlesPosition = new List<Vector2>(); // The local positions of the path handles.
List<Vector2> handleControlsPos = new List<Vector2>(); // The local positions of the handles that control the tension and bias of a path node.
List<GameObject> foliageOnPath = new List<GameObject>(); // The list of foliage objects that are placed on the current path.
List<GameObject> foliagePrefabs = new List<GameObject>(); // The list of foliage prefabs.
List<float> nodeTension = new List<float>(); // The list of tensions for each path node.
List<float> nodeBias = new List<float>(); // The list of bias for each path node.
// The value of the overlapping factor determines which part of a foliage object mesh
// will be placed to the left of the right edge of the previous foliage object mesh.
float overlappingFactor = 0.4;
float minOverlappingFactor = 0; // The min value that the "overlappingFactor" should have.
float maxOverlappingFactor = 0.8; // The max value that the "overlappingFactor" should have.
float bias = 0; // The bias of every path node.
float tension = 0; // The tension of every path node.
float firstObjectOffset = 0; // The offset for the first object on a path line.
float lastObjectOffset = 0; // The offset for the last object on a path line.
float biasScale = 4; // The bias scale.
float tensionScale = 4; // The tension scale.
float zOffset = 0.01; // The offset on the Z axis for a foliage object.
bool uniformValues = true; // When set to true the tension and bias will be the same for all path nodes.
int foliagePrefabListSize = 1; // The size of the list that contains the foliage prefabs.

// Fills the foliage path with objects.
void RecreateFoliage() {
	foliageCount = foliageOnPath.Count;
	distanceFromStart = 0;
	prevDistanceFromStart = 0;
	maxCount = 0;
	float ZAxisOffset = zOffset;

	List<GameObject> foliageP = new List<GameObject>();
	int len = foliagePrefabs.Count;

	foliageP.Clear();
	// The foliage prefabs are placed in a new list to make sure that there are no null fields.
	for (int k = 0; k < len; k++) {
		if (foliagePrefabs[k] != null) {
			foliageP.Add(foliagePrefabs[k]);
		}
	}
	maxCount = foliageP.Count;

	// If the prefab list is empty return.
	if (maxCount == 0) {
		return;
	}
	objectIndex = 1;

	// Foliage objects are placed on the line described by the path nodes.
	for (int i = 0; i < handlesPosition.Count - 1; i++) {
		line = handlesPosition[i + 1] - handlesPosition[i];
		angleInRad = Math::Atan2(line.y, line.x);
		angleInDeg = angleInRad * Math::Rad2Deg;
		lineLength = line.magnitude;
		// The vector is normalized so that we can calculate its normal correctly.
		line.Normalize();
		lineNormal = new Vector2(-line.y, line.x);
		distanceFromStart = 0;
		prevDistanceFromStart = 0;

		// This loop is executed until the distance between the start of the line and a
		// foliage object is smaller than the length of the line.
		while (true) {
			updatedPrefabIndex = false;

			// If a new object must be instantiated, updates the "prefabIndex" value.
			// The value of "prefabIndex" determines which prefab will be instantiated
			// in the current iteration.
			if (foliageCount < objectIndex) {
				if (foliagePattern == Foliage2D_Pattern.Random) {
					// Get a random index.
					prefabIndex = Random.Range(0, maxCount);
				} else {
					// The value of "prefabIndex" is incremented by 1 so that new objects are instantiated consecutively.
					prefabIndex++;
					// If the value of "prefab Index" is bigger than "maxCount" or smaller than 0, we reset it to 0.
					if (prefabIndex >= maxCount || prefabIndex < 0) {
						prefabIndex = 0;
					}
				}

				updatedPrefabIndex = true;
				// We get the Foliage2D component of the prefab with the index of "prefabIndex"
				// so that we have access to the width of the correct foliage object when we
				// calculate the distance from the start of the line where this object will be placed.
				foliage2D = foliageP[prefabIndex].GetComponent<Foliage2D>();
				DistanceFromStart();
			} else {
				foliage2D = foliageOnPath[objectIndex - 1].GetComponent<Foliage2D>();
				DistanceFromStart();
			}

			// If the distance from the start of the line where the last object was placed is
			// bigger than the line length, the position of the object is reset and the execution
			// of the while loop is terminated.
			if (distanceFromStart > lineLength) {
				foliage2D = foliageOnPath[prevIndex].GetComponent<Foliage2D>();
				distanceFromStart = lineLength - foliage2D.width / 2 + lastObjectOffset;

				if (foliagePathType == Foliage2D_PathType.Smooth) {
					SmoothPath(i);
				} else {
					pointOnTheLine = Vector2.Lerp(handlesPosition[i], handlesPosition[i + 1], distanceFromStart / lineLength);
				}
				posOffset = lineNormal * (foliage2D.height / 2);
				foliageOnPath[prevIndex].transform.position = transform.TransformPoint(new Vector3(pointOnTheLine.x + posOffset.x, pointOnTheLine.y + posOffset.y, ZAxisOffset));

				if (updatedPrefabIndex && foliagePattern == Foliage2D_Pattern.Consecutive) {
					RestorePrefabIndex();
				}
				break;
			}

			if (foliagePathType == Foliage2D_PathType.Smooth) {
				SmoothPath(i);
			} else {
				pointOnTheLine = Vector2.Lerp(handlesPosition[i], handlesPosition[i + 1], distanceFromStart / lineLength);
			}
			if (foliageCount < objectIndex) {
				// If the index of the current object is bigger than the total number of
				// instantiated objects, a new object is instantiated.
				ZAxisOffset *= -1;
				Vector3 pos = pointOnTheLine + posOffset;
				pos.z = zOffset;
				GameObject obj = Instantiate(foliageP[prefabIndex], transform.TransformPoint(pos), Quaternion.Euler(new Vector3(0, 0, angleInDeg))) as GameObject;
				obj.transform.parent = transform;
				foliageOnPath.Add(obj);
				prevIndex = foliageOnPath.Count - 1;
				foliageCount = foliageOnPath.Count;
				Foliage2D objFoliage = obj.GetComponent<Foliage2D>();
				// The object mesh is recreated so that we don't have 2 objects with the same mesh instance.
				objFoliage.RebuildMesh();
				objectIndex++;
			} else {
				ZAxisOffset *= -1;
				prevIndex = objectIndex - 1;
				foliageOnPath[objectIndex - 1].transform.position = transform.TransformPoint(new Vector3(pointOnTheLine.x + posOffset.x, pointOnTheLine.y + posOffset.y, ZAxisOffset));
				foliageOnPath[objectIndex - 1].transform.rotation = Quaternion.Euler(new Vector3(0, 0, angleInDeg));
				objectIndex++;
			}
		}
	}

	// If the path shrinked and there is an excess of foliage objects, delete them.
	if (foliageCount + 1 > objectIndex) {
		int lenD = foliageCount + 1 - objectIndex;

		for (int i = 0; i < lenD; i++) {
			int last = foliageOnPath.Count - 1;
			DestroyImmediate(foliageOnPath[last]);
			foliageOnPath.RemoveAt(last);

			if (foliagePattern == Foliage2D_Pattern.Consecutive) {
				RestorePrefabIndex();
			}
		}
	}
}

// Moves the object location to the center of the handles. Also offsets the handles locations to match.
void ReCenterPivotPoint() {
	Vector2 center = Vector2.zero;

	for (int i = 0; i < handlesPosition.Count; i++) {
		center += handlesPosition[i];
	}

	center = center / handlesPosition.Count + new Vector2(transform.position.x, transform.position.y);
	Vector2 offset = center - new Vector2(gameObject.transform.position.x, gameObject.transform.position.y);

	for (int i = 0; i < handlesPosition.Count; i++) {
		handlesPosition[i] -= offset;
	}

	gameObject.transform.position = new Vector3(center.x, center.y, gameObject.transform.position.z);
	RecreateFoliage();
}

// Deletes all objects on the current path and instantiates new objects.
void ClearList() {
	int len = foliageOnPath.Count - 1;

	for (int i = len; i >= 0; i--) {
		DestroyImmediate(foliageOnPath[i]);
		foliageOnPath.RemoveAt(i);
	}
	prefabIndex = -1;
	foliageOnPath.Clear();
	RecreateFoliage();
}

// Adds a node to the foliage path.
// hP: Node local position.
void AddPathPoint(Vector2 hP) {
	handlesPosition.Add(hP);
	handleControlsPos.Add(new Vector2(hP.x + 2, hP.y));
	nodeTension.Add(0);
	nodeBias.Add(0);
}
}

class Foliage2DAnimation {
	Vector3[] posOffset; // The amount by which to change the initial position of a vertex. The change in position for the vertices on the same row is the same.
	Vector2[] centerLinePoints; // A series of points around which to rotate the mesh vertices.
	Foliage2D foliage2D; // Reference to the Foliage2D component of this object.
	bool isBending = false; // Did an object bend this foliage object?.
	int horizontalVerts; // The number of verices a horizontal row has.
	float[] anglesInDeg; // The amount in degrees by which to rotate a vertex around a point.

public:
	List<float> offsetFactor = new List<float>(); // A list of values that define which part of the "offset" value should be used to calculate the new vertex position.
	Vector3 offset; // This is the value that should be changed to animate the mesh.
	bool changeAnimationSpeed = false; // Should we change the animation speed at the start?.

	// Applies a simple bending to the mesh.
	void SimpleMeshBending() {
		const int hLen = foliage2D.heightSegments + 1;
		const int wLen = foliage2D.widthSegments + 1;

		// This for loop is responsible for the mesh animation.
		// The final position of the mesh vertices is calculated based on
		// their initial position plus the offset for a given horizontal row.
		for (int i = 0; i < hLen; i++) {
			posOffset[i] = Vector3(offset.x * offsetFactor[i], offset.y * offsetFactor[i], offset.z * offsetFactor[i]);

			for (int j = 0; j < wLen; j++) {
				// The index of the vertex whose final local position must be calculated.
				const int vertIndex = horizontalVerts * i + j;

				finalVertexPos[vertIndex] = Vector3(
						initialVertexPos[vertIndex].x + posOffset[i].x,
						initialVertexPos[vertIndex].y + posOffset[i].y,
						initialVertexPos[vertIndex].z + posOffset[i].z);
			}
		}
	}

	// Applies a more realistic bending to the mesh.
	void SmartMeshBending() {
		// The value of -1 means the vertex will be moved to the left of it's
		// initial position on the X axis while 1 to the right.
		const int dir = offset.x < 0 ? -1 : 1;
		// The height of a horizontal mesh segment.
		const float heightOfSegment = foliage2D.height / foliage2D.heightSegments;
		centerLinePoints[0] = new Vector2(offset.x * offsetFactor[0], initialVertexPos[0].y);

		const int hLen = foliage2D.heightSegments + 1;
		const int wLen = foliage2D.widthSegments + 1;

		// You could say this is the backbone of the mesh. Uncomment the segment below this loop to see it in the scene view.
		// Calculates the points around which to rotate the mesh vertices.
		for (int i = 1; i < hLen; i++) {
			float xOffset = offset.x * offsetFactor[i];

			if (Math::abs(xOffset) < heightOfSegment) {
				centerLinePoints[i] = new Vector2(centerLinePoints[i - 1].x + xOffset, centerLinePoints[i - 1].y + Math::sqrt((heightOfSegment * heightOfSegment) - (xOffset * xOffset)));
			} else {
				float y = Math::abs(xOffset) - heightOfSegment;
				centerLinePoints[i] = new Vector2(centerLinePoints[i - 1].x + dir * Math::sqrt((heightOfSegment * heightOfSegment) - (y * y)), centerLinePoints[i - 1].y - y);
			}
		}

		// Uncomment this segment to see the mesh center line in the scene view.
		//for (int i = 0; i < hLen - 1; i++)
		//{
		//    Debug.DrawLine(transform.TransformPoint(centerLinePoints[i]), transform.TransformPoint(centerLinePoints[i + 1]), Color.blue);
		//}

		for (int i = 1; i < hLen; i++) {
			Vector2 line = centerLinePoints[i] - centerLinePoints[i - 1];
			anglesInDeg[i] = Math::rad2deg(Math::atan2(line.y, line.x)) - 90;

			// The amount in radians by which to rotate a vertex around a point.
			const float angleInRadians = anglesInDeg[i] * Math::Deg2Rad;

			for (int j = 0; j < wLen; j++) {
				// The index of the vertex whose final local position must be calculated.
				int vertIndex = horizontalVerts * i + j;

				// The initial position of a vertex is offset based on the values of "centerLinePoints".
				finalVertexPos[vertIndex] = new Vector3(
						initialVertexPos[vertIndex].x + centerLinePoints[i].x,
						centerLinePoints[i].y,
						initialVertexPos[vertIndex].z);

				// The Current position of the vertex.
				Vector3 vertCurPos = finalVertexPos[vertIndex];

				// Calculates the final position of the vertex after it is rotated.
				finalVertexPos[vertIndex] = new Vector3(
						(vertCurPos.x - centerLinePoints[i].x) * Math::cos(angleInRadians) - (vertCurPos.y - centerLinePoints[i].y) * Math::sin(angleInRadians) + centerLinePoints[i].x,
						(vertCurPos.x - centerLinePoints[i].x) * Math::sin(angleInRadians) + (vertCurPos.y - centerLinePoints[i].y) * Math::cos(angleInRadians) + centerLinePoints[i].y,
						0);
			}
		}
	}
}
