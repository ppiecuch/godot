/**************************************************************************/
/*  rid_props.h                                                           */
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

#ifndef RID_PROPS_H
#define RID_PROPS_H

#include "core/color.h"
#include "core/list.h"
#include "core/math/transform.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"
#include "core/os/memory.h"
#include "core/rid_handle.h"
#include "core/safe_refcount.h"
#include "core/set.h"
#include "core/typedefs.h"
#include "core/vector.h"

union RID_Prop {
	int int_value;
	bool bool_value;
	real_t real_value;
	Vector2 vec2_value;
	Vector3 vec3_value;
	Color color_value;
	Transform transform_value;

	RID_Prop() {}
	_FORCE_INLINE_ RID_Prop(int v) { int_value = v; }
	_FORCE_INLINE_ RID_Prop(bool v) { bool_value = v; }
	_FORCE_INLINE_ RID_Prop(real_t v) { real_value = v; }
	_FORCE_INLINE_ RID_Prop(const Vector2 &v) { vec2_value = v; }
	_FORCE_INLINE_ RID_Prop(const Vector3 &v) { vec3_value = v; }
	_FORCE_INLINE_ RID_Prop(const Color &v) { color_value = v; }
	_FORCE_INLINE_ RID_Prop(const Transform &v) { transform_value = v; }
};

typedef Vector<RID_Prop> RID_Props_Vec;

struct _expand_props {
	template <typename... T>
	_expand_props(T &&...) {}
};

template <typename... props_types>
RID_Props_Vec *_create_props(props_types... args) {
	RID_Props_Vec *_props = memnew(RID_Props_Vec);
	_expand_props{ 0, (_props->push_back(args), 0)... };
}

#endif // RID_PROPS_H
