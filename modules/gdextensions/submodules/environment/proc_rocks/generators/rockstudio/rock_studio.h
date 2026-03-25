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

#include "core/math/geometry.h"
#include "core/math/math_funcs.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"
#include "core/vector.h"
#include "scene/resources/mesh.h"
#include "scene/resources/surface_tool.h"

// =========================================================================
// Point generators — create random point clouds for convex hull
// =========================================================================

Vector<Vector3> rock_studio_points_cube(int p_count, real_t p_width, real_t p_height, real_t p_depth);
Vector<Vector3> rock_studio_points_sphere(int p_count, real_t p_radius);
Vector<Vector3> rock_studio_points_crystal(int p_count, bool p_tetragonal, bool p_one_sided, real_t p_base_width, real_t p_base_height, real_t p_tip_protrusion, real_t p_tip_flatness);

// =========================================================================
// Mesh creation — convex hull + low-poly + box UV
// =========================================================================

Ref<ArrayMesh> rock_studio_create_mesh(const Vector<Vector3> &p_points);
Ref<ArrayMesh> rock_studio_make_low_poly(const Ref<ArrayMesh> &p_mesh);
void rock_studio_box_uv(Ref<ArrayMesh> p_mesh);

// =========================================================================
// UV helpers
// =========================================================================

int rock_studio_get_box_dir(const Vector3 &p_normal);
Vector2 rock_studio_get_box_uv(const Vector3 &p_vertex, int p_box_dir);

#endif // ROCK_STUDIO_H
