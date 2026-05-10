/**************************************************************************/
/*  stack.h                                                               */
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

#pragma once

#include "core/local_vector.h"

// Currently just a simple implementation/wrapper over vector, for depth-first search.
template <class T, class U = uint32_t>
class Stack {
	LocalVector<T, U> container;
	U index = 0;

public:
	_FORCE_INLINE_ T &top() {
		return container[index - 1];
	}
	_FORCE_INLINE_ void push(const T &p_element) {
		if (index == container.size()) {
			container.push_back(p_element);
		} else {
			container[index] = p_element;
		}
		index++;
	}
	_FORCE_INLINE_ T &pop() {
		T &element = container[index - 1];
		index--;
		return element;
	}
	void reserve(U p_capacity) {
		container.reserve(p_capacity);
	}
	void clear() {
		container.clear();
		index = 0;
	}
	_FORCE_INLINE_ bool is_empty() const {
		return index == 0;
	}
	_FORCE_INLINE_ U size() const {
		return index;
	}
};
