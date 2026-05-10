/**************************************************************************/
/*  vector_graphics_linear_gradient.cpp                                   */
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

#include "vector_graphics_linear_gradient.h"
#include "core/core_string_names.h"

void VGLinearGradient::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_p1", "p1"), &VGLinearGradient::set_p1);
	ClassDB::bind_method(D_METHOD("get_p1"), &VGLinearGradient::get_p1);

	ClassDB::bind_method(D_METHOD("set_p2", "p2"), &VGLinearGradient::set_p2);
	ClassDB::bind_method(D_METHOD("get_p2"), &VGLinearGradient::get_p2);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "p1"), "set_p1", "get_p1");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "p2"), "set_p2", "get_p2");
}

VGLinearGradient::VGLinearGradient() {
	p1 = Vector2(0, 0);
	p2 = Vector2(100, 100);
}

void VGLinearGradient::set_p1(const Vector2 &p_p1) {
	p1 = p_p1;
	_change_notify("color");
}

Vector2 VGLinearGradient::get_p1() const {
	return p1;
}

void VGLinearGradient::set_p2(const Vector2 &p_p2) {
	p2 = p_p2;
	_change_notify("color");
}

Vector2 VGLinearGradient::get_p2() const {
	return p2;
}
