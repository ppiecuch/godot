/**************************************************************************/
/*  typedef.h                                                             */
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

#ifndef BEHAVIOR_TREE_TYPEDEF_H
#define BEHAVIOR_TREE_TYPEDEF_H

#if defined(BEHAVIOR_TREE_AS_GODOT_MODULE)

#include "core/vector.h"
#define BT_STATIC_ASSERT(x, y)
#define BT_ASSERT(x)

#else

#include <algorithm>
#include <cassert>
#include <vector>
#define BT_STATIC_ASSERT(x, y) static_assert(x, y)
#define BT_ASSERT(x) assert(x)

#endif

namespace BehaviorTree {
struct Node;
}

namespace BehaviorTree {

enum E_State { BH_ERROR = 0,
	BH_SUCCESS = 1,
	BH_FAILURE = 2,
	BH_RUNNING = 3 };

typedef unsigned short IndexType;
const IndexType INDEX_TYPE_MAX = 0xffff;

struct NodeData {
	union {
		IndexType begin;
		IndexType index;
	};
	IndexType end;
};

#if defined(BEHAVIOR_TREE_AS_GODOT_MODULE)

template <typename T>
class BTVector : public Vector<T> {
public:
	T &back() { return Vector<T>::write[Vector<T>::size() - 1]; }
	T const &back() const { return Vector<T>::last(); }

	void pop_back() { Vector<T>::resize(Vector<T>::size() - 1); }

	void swap(BTVector &other) {
		other = *this;
		Vector<T>::clear();
	}
};
#if (__GNUC__ <= 4) && !defined(__clang__) && !defined(_MSC_VER)
#pragma GCC diagnostic pop
#endif

template <typename COMPARATOR, typename T>
void sort(BTVector<T> &vector) {
	vector.template sort_custom<COMPARATOR>();
}

#else

template <typename T>
class BTVector : public std::vector<T> {};

template <typename COMPARATOR, typename T>
void sort(BTVector<T> &vector) {
	std::sort(vector.begin(), vector.end(), COMPARATOR());
}

#endif

typedef BTVector<NodeData> BTStructure;
typedef BTVector<Node *> NodeList;

} //namespace BehaviorTree

#endif
