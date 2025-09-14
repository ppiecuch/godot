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

union RID_Prop {
	int int_value;
	bool bool_value;
	real_t real_value;
	Vector2 vec2_value;
	Vector3 vec3_value;
	Color color_value;
	Transform transform_value;

	_FORCE_INLINE_ RID_Prop(int v) { int_value = v; }
	_FORCE_INLINE_ RID_Prop(bool v) { bool_value = v; }
	_FORCE_INLINE_ RID_Prop(float v) { real_value = v; }
	_FORCE_INLINE_ RID_Prop(double v) { real_value = v; }
	_FORCE_INLINE_ RID_Prop(const Vector2 &v) { vec2_value = v; }
	_FORCE_INLINE_ RID_Prop(const Vector3 &v) { vec3_value = v; }
	_FORCE_INLINE_ RID_Prop(const Color &v) { color_value = v; }
	_FORCE_INLINE_ RID_Prop(const Transform &v) { transform_value = v; }

	RID_Prop() {}
};

static constexpr size_t MAX_PROPS = 4;

class RID_PropsContainer {
private:
	struct PropsBlock {
		RID_Prop props[MAX_PROPS];
	};

	PropsBlock *_data;
	int32_t _kind;

public:
	// True zero-cost construction - just nullptr
	RID_PropsContainer() :
			_data(nullptr), _kind(0) {}

	// Destructor
	~RID_PropsContainer() {
		if (_data) {
			memdelete(_data);
		}
	}

	// Copy constructor
	RID_PropsContainer(const RID_PropsContainer &other) :
			_data(nullptr) {
		if (other._data) {
			_data = memnew(PropsBlock);
			*_data = *other._data;
		}
	}

	// Assignment operator
	RID_PropsContainer &operator=(const RID_PropsContainer &other) {
		if (this != &other) {
			if (_data) {
				memdelete(_data);
				_data = nullptr;
			}
			if (other._data) {
				_data = memnew(PropsBlock);
				*_data = *other._data;
			}
		}
		return *this;
	}

	// Check if empty (true zero overhead)
	bool empty() const {
		return _data == nullptr;
	}

	// Access operators
	RID_Prop &operator[](size_t idx) {
		CRASH_COND(!_data);
		CRASH_BAD_INDEX(idx, MAX_PROPS);
		return _data->props[idx];
	}

	const RID_Prop &operator[](size_t idx) const {
		CRASH_COND(!_data);
		CRASH_BAD_INDEX(idx, MAX_PROPS);
		return _data->props[idx];
	}

	// Get with optional default value
	RID_Prop get(size_t idx) const {
		if (!_data || idx >= MAX_PROPS) {
			return RID_Prop();
		}
		return _data->props[idx];
	}

	RID_Prop get(size_t idx, const RID_Prop &default_value) const {
		if (!_data || idx >= MAX_PROPS) {
			return default_value;
		}
		return _data->props[idx];
	}

	// Clear all properties
	void clear() {
		if (_data) {
			memdelete(_data);
			_data = nullptr;
		}
	}

	// Variadic template to set all properties at once
	template <typename... Args>
	void set_all(Args... args) {
		static_assert(sizeof...(args) <= MAX_PROPS, "Too many properties");

		if (sizeof...(args) == 0) {
			clear();
			return;
		}

		_ensure_allocated();
#ifndef DEBUG_ENABLED
		CRASH_COND(!_kind);
#endif
		size_t idx = 0;
		(((_data->props[idx++] = args)), ...);
	}

	// Assign values to specific indices
	void set(size_t idx, const RID_Prop &prop) {
		CRASH_BAD_INDEX(idx, MAX_PROPS);
		_ensure_allocated();
#ifndef DEBUG_ENABLED
		CRASH_COND(!_kind);
#endif
		_data->props[idx] = prop;
	}

	// Manage block's owner type
	int32_t get_kind() const { return _kind; }
	void set_kind(int32_t kind) {
#ifndef DEBUG_ENABLED
		CRASH_COND_MSG(_kind, "Block properties already assigned.");
#endif
		_ensure_allocated();
		_kind = kind;
	}

private:
	void _ensure_allocated() {
		if (!_data) {
			_data = memnew(PropsBlock);
		}
	}
};

#endif // RID_PROPS_H
