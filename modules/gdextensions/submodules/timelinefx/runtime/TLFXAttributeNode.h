/*************************************************************************/
/*  TLFXAttributeNode.h                                                  */
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

#ifndef _TLFX_ATTRIBUTENODE_H
#define _TLFX_ATTRIBUTENODE_H

namespace TLFX {

/**
 * Attribute Node type
 * <p>An Attribute Node is basically a type used to store effect and emitter attributes. They are in effect nodes on a line graph so that when an effect plays
 * out the attribute value is looked up on the line graph and interpolated if the current time is in between 2 attribute nodes. Think of the x-axis of the graph
 * as the time in milliseconds and the y-axis representing the value of the attribute. The line don't have to just be linear either, they can also be curves, see
 * #ToggleCurve and #SetCurvePoints. The Timeline Particle Editor that comes with this module uses graphs to design the particle effects.</p>
 */
struct AttributeNode {
	AttributeNode();

	float frame;
	float value;

	bool isCurve;

	float c0x, c0y, c1x, c1y;

	bool Compare(const AttributeNode &other) const;
	bool operator<(const AttributeNode &other) const;

	/**
	 * Set the curve points for the emitter change
	 * x0 and y0 are the coordinates of the point to the left of the attribute node, x1 and y1 are the coordinates to the right of the attribute node. Setting
	 * these will create a bezier curve. The bezier curves are restricted so that they cannot be drawn so that they loop over or behind the frame of the attribute nodes.
	 */
	void SetCurvePoints(float x0, float y0, float x1, float y1);

	/**
	 * Toggle whether this attribute node is curved or linear
	 */
	void ToggleCurve();
};

} // namespace TLFX

#endif // _TLFX_ATTRIBUTENODE_H
