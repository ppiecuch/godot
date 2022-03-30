/*************************************************************************/
/*  TLFXAttributeNode.cpp                                                */
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

#include "TLFXAttributeNode.h"

namespace TLFX {

AttributeNode::AttributeNode() :
		frame(0), value(0), isCurve(false), c0x(0), c0y(0), c1x(0), c1y(0) {
}

bool AttributeNode::Compare(const AttributeNode &other) const {
	return frame > other.frame; // @todo dan sgn(frame - other.frame)
}

void AttributeNode::SetCurvePoints(float x0, float y0, float x1, float y1) {
	c0x = x0;
	c0y = y0;
	c1x = x1;
	c1y = y1;
	isCurve = true;
}

void AttributeNode::ToggleCurve() {
	isCurve = !isCurve;
}

bool AttributeNode::operator<(const AttributeNode &other) const {
	return Compare(other);
}

} // namespace TLFX
