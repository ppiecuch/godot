/*************************************************************************/
/*  TLFXVector2.h                                                        */
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

#ifdef _MSC_VER
#pragma once
#endif

#ifndef _TLFX_VECTOR2_H
#define _TLFX_VECTOR2_H

namespace TLFX {

class Vector2 {
public:
	float x, y;

	Vector2(float vx = 0, float vy = 0);

	/**
	 * Create a new vector with the given coordinates
	 * @return New #Vector2
	 */
	static Vector2 Create(float vx, float vy);

	/**
	 * Move a vector but the given x and y amount
	 */
	void Move(float distanceX, float distanceY);

	/**
	 * Move a vector buy the given vector
	 */
	void Move(const Vector2 &distance);

	/**
	 * Reposition the vector by the new x,y coordinates given
	 */
	void Set(float vx, float vy);

	/**
	 * Reposition the vector by the given vector
	 */
	void Set(const Vector2 &v);

	/**
	 * Subtract this Vector2 vector by another and return the result in a new vector
	 * @return A new #Vector2
	 */
	Vector2 Subtract(const Vector2 &v) const;

	/**
	 * Add this Vector2 vector to another and return the result in a new vector
	 * return A new #Vector2
	 */
	Vector2 Add(const Vector2 &v) const;

	/**
	 * multiply this vector with another and return the result
	 * @return New #Vector2
	 */
	Vector2 Multiply(const Vector2 &v) const;

	/**
	 * Scale the vector by the given amount and return the result in a new vector
	 * @return new scaled #Vector2
	 */
	Vector2 Scale(float scale) const;

	/**
	 * Get the length of the vector
	 * @return The length or magnitude of the vector.
	 */
	float Length() const;

	/**
	 * Get the unit vector of the vector
	 * @return New unit vector of this vector
	 */
	Vector2 Unit() const;

	/**
	 * get the normal of the vector
	 * @return New #Vector2 normal of this vector
	 */
	Vector2 Normal() const;
	Vector2 LeftNormal() const;

	/**
	 * Normalise the vector
	 */
	void Normalize();

	/**
	 * Get the dot product of the vector
	 * @return The dot product of the vector.
	 */
	float DotProduct(const Vector2 &v) const;

	static float GetDistance(float fromx, float fromy, float tox, float toy, bool fast = false);
	static float GetDirection(float fromx, float fromy, float tox, float toy);

protected:
};

} // namespace TLFX

#endif // _TLFX_VECTOR2_H
