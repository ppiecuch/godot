/**************************************************************************/
/*  TLFXMatrix2.h                                                         */
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

#ifdef _MSC_VER
#pragma once
#endif

#ifndef _TLFX_MATRIX2_H
#define _TLFX_MATRIX2_H

namespace TLFX {

class Vector2;

/**
 * Matrix type which can be used to easily transform vectors
 * Matrices come in handy for transforming vectors such as scale and rotation
 */
class Matrix2 {
public:
	float aa, ab;
	float ba, bb;

	Matrix2();

	/**
	 * Create a new matrix
	 * @return New matrix type
	 */
	static Matrix2 Create(float aa_ = 1.0f, float ab_ = 0, float ba_ = 0, float bb_ = 1.0f);

	/**
	 * Set the matrix to a new set of values
	 * Use this to prepare the matrix for a new transform. For example if you wanted to to rotate a vector, then you could do
	 * &{<matrix.Set(cos(angle),sin(angle),-sin(angle),cos(angle))}
	 * and then transform the vector with
	 * &{matrix.TransformVector(vector)}
	 */
	void Set(float aa_ = 1.0f, float ab_ = 0, float _ba = 0, float _bb = 1.0f);

	/**
	 * Transpose the matrix
	 */
	void Transpose();

	/**
	 * Scale the matrix by a given amount
	 */
	void Scale(float s);

	/**
	 * Transform the matrix
	 * Multiplies 2 matrices together
	 * @return New transformed matrix
	 */
	Matrix2 Transform(const Matrix2 &m) const;

	/**
	 * Transform a vector with the matrix
	 * You can use this to transform a vector, rotating it, scaling it etc.
	 * @return New transformed vector
	 */
	Vector2 TransformVector(const Vector2 &v) const;

	/**
	 * Transform a point
	 * This will transform a point (x,y) and apply the new coordinates into tx and ty.
	 * @return New coordinates for the transformed point in tx and ty
	 */
	void TransformPoint(float x, float y, float &tx, float &ty);

protected:
};

} // namespace TLFX

#endif // _TLFX_MATRIX2_H
