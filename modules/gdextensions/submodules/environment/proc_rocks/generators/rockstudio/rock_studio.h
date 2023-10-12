/**************************************************************************/
/*  rock_studio.h                                                         */
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

#ifndef ROCK_STUDIO_H
#define ROCK_STUDIO_H

#include "core/math/vector3.h"
#include "core/vector.h"

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
