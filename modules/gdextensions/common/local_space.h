/*************************************************************************/
/*  local_space.h                                                        */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef LOCAL_SPACE_H
#define LOCAL_SPACE_H

#include "core/math/vector2.h"
#include "core/math/vector3.h"

// LocalSpace: a local coordinate system for 2d/3d space
//
// Provide functionality such as transforming from local space to global
// space and vice versa.  Also regenerates a valid space from a perturbed
// "forward vector" which is the basis of abstract object turning.
//
// These are comparable to a 4x4 homogeneous transformation matrix where the
// 3x3 (R) portion is constrained to be a pure rotation (no shear or scale).
// The rows of the 3x3 R matrix are the basis vectors of the space.  They are
// all constrained to be mutually perpendicular and of unit length.  The top
// ("x") row is called "side", the middle ("y") row is called "up" and the
// bottom ("z") row is called forward.  The translation vector is called
// "position".  Finally the "homogeneous column" is always [0 0 0 1].
//
//     [ R R R  0 ]      [ Sx Sy Sz  0 ]
//     [ R R R  0 ]      [ Ux Uy Uz  0 ]
//     [ R R R  0 ]  ->  [ Fx Fy Fz  0 ]
//     [          ]      [             ]
//     [ T T T  1 ]      [ Tx Ty Tz  1 ]
//

class LocalSpace {
public:
	// transformation as three orthonormal unit basis vectors and the
	// origin of the local space.  These correspond to the "rows" of
	// a 3x4 transformation matrix with [0 0 0 1] as the final column
	LocalSpace() { set_to_identity(); }

	// reset transform: set local space to its identity state, equivalent to a
	// 4x4 homogeneous transform like this:
	// ------------------------------------------------------------------------
	//     [ X 0 0 0 ]
	//     [ 0 1 0 0 ]
	//     [ 0 0 1 0 ]
	//     [ 0 0 0 1 ]
	//
	// where X is 1 for a left-handed system and -1 for a right-handed system.
	void set_to_identity() {
		forward = Vector3(0, 0, 1);
		side = local_rotate_forward_to_side(forward);
		up = Vector3(0, 1, 0);
	}

	bool right_handed(void) const { return true; }

	Vector3 globalize_position(const Point3 &pos, const Vector3 &local) const { return Vector3(pos.x, pos.y, pos.z) + globalize_direction(local); }
	Vector3 globalize_direction(const Vector3 &local) const { return side * local.x + up * local.y + forward * local.z; }
	Vector3 localize_direction(const Vector3 &global) const { return Vector3(global.dot(side), global.dot(up), global.dot(forward)); }
	Vector3 localize_position(const Point3 &pos, const Vector3 &global) const { return localize_direction(global - Vector3(pos.x, pos.y, pos.z)); }
	Vector3 local_rotate_forward_to_side(const Vector3 &v) const { return Vector3(right_handed() ? (-v.z) : (+v.z), v.y, v.x); }

	Vector3 side, forward, up;
};

class LocalSpace2 {
public:
	LocalSpace2() { set_to_identity(); }

	void set_to_identity() {
		side = Vector2(1, 0);
		up = Vector2(0, 1);
	}

	Vector2 globalize_position(const Point2 &pos, const Vector2 &local) const { return Vector2(pos.x, pos.y) + globalize_direction(local); }
	Vector2 globalize_direction(const Vector2 &local) const { return side * local.x + up * local.y; }
	Vector2 localize_position(const Point2 &pos, const Vector2 &global) const { return localize_direction(global - Vector2(pos.x, pos.y)); }
	Vector2 localize_direction(const Vector2 &global) const { return Vector2(global.dot(side), global.dot(up)); }
	Vector2 local_rotate_forward_to_side(const Vector2 &v) const { return Vector2(v.y, v.x); }

	Vector2 side, up;
};

#endif // LOCAL_SPACE_H
