#ifndef ROCK_STUDIO_H
#define ROCK_STUDIO_H

#include "core/vector.h"
#include "core/math/vector3.h"

MeshInstance CreateRock(const Vector<Vector3> &vertices, Vector3 pos, Vector3 nor, int rockOrientation, Ref<Material> rockMaterial, const String &rockName, bool rigidbody, int colliderType, Node *parent = nullptr);
Ref<Mesh> CreateMesh(const Vector<Vector3> &vertices);

Vector3 GetRandomPointOnMesh(const Ref<Mesh> &mesh);
Vector<Vector3> GetRandomPointsWithinCube(int numberOfPoints, real_t width, real_t height, real_t depth);
Vector<Vector3> GetRandomPointsWithinSphere(int numberOfPoints, real_t radius);
Vector<Vector3> GetRandomPointsWithinCrystal(int numberOfPoints, bool tetragonal, bool oneSided, real_t baseWidth, real_t baseHeight, real_t tipProtrusion, real_t tipFlatness);
Vector<Vector3> GetRandomPointsOnMesh(int numberOfPoints, Ref<Mesh> mesh);

void GenerateComposition(int numberOfVertices, Vector3 pos, Vector3 nor, Ref<Material> rockmat);
void CreateSculpture(Vector<Vector3> vertices, Vector3 pos, Vector3 nor, Vector3 rot, Vector3 scale, Ref<Material> rockmat);
void GenerateCompositionFill(int numberOfVertices, Vector3 pos, Vector3 nor, Ref<Material> rockmat);
void CreateSculptureFill(const Vector<Vector3> &vertices, Vector3 pos, Vector3 nor, Ref<Material> rockmat);

Ref<Mesh> MakeLowPoly(Transform transform, String name);
void BoxUV(Ref<Mesh> mesh, Transform tform);
Vector2 GetBoxUV(const Vector3 &vertex, int boxDir);
int GetBoxDir(const Vector3 &v);

#endif // ROCK_STUDIO_H
