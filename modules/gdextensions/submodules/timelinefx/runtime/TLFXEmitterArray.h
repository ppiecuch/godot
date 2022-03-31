/*************************************************************************/
/*  TLFXEmitterArray.h                                                   */
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

#ifndef _TLFX_EMITTERARRAY_H
#define _TLFX_EMITTERARRAY_H

#include "TLFXAttributeNode.h"

#include <list>
#include <vector>

namespace TLFX {

class EmitterArray {
public:
	EmitterArray(float min, float max);

	void Clear(unsigned int size = 0);
	AttributeNode *Add(float frame, float value);
	float Get(float frame, bool bezier = true) const;
	float operator()(float frame, bool bezier = true) const;
	float GetOT(float age, float lifetime, bool bezier = true) const;
	float operator()(float age, float lifetime, bool bezier = true) const;

	float Interpolate(float frame, bool bezier = true) const;
	float InterpolateOT(float age, float lifetime, bool bezier = true) const;

	void Sort();

	unsigned int GetAttributesCount() const;

	float GetMaxValue() const;

	// compiled
	void Compile();
	void CompileOT(float longestLife);
	void CompileOT();

	unsigned int GetLastFrame() const;
	float GetCompiled(unsigned int frame) const;
	void SetCompiled(unsigned int frame, float value);

	float &operator[](unsigned int frame);
	const float &operator[](unsigned int frame) const;

	int GetLife() const;
	void SetLife(int life);

protected:
	std::list<AttributeNode> _attributes;

	// compiled
	std::vector<float> _changes;
	int _life;
	bool _compiled;
	float _min, _max;

	static float GetBezierValue(const AttributeNode &lastec, const AttributeNode &a, float t, float yMin, float yMax);
	static void GetQuadBezier(float p0x, float p0y, float p1x, float p1y, float p2x, float p2y, float t, float yMin, float yMax, float &outX, float &outY, bool clamp = true);
	static void GetCubicBezier(float p0x, float p0y, float p1x, float p1y, float p2x, float p2y, float p3x, float p3y,
			float t, float yMin, float yMax, float &outX, float &outY, bool clamp = true);
};

} // namespace TLFX

#endif // _TLFX_EMITTERARRAY_H
