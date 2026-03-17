/**************************************************************************/
/*  iso_assoc_list.h                                                      */
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

#ifndef ISO_ASSOC_LIST_H
#define ISO_ASSOC_LIST_H

#include "core/vector.h"

// Unique-element list with O(1) unordered removal.
// Uses linear scan for contains/find (sufficient for typical isometric object counts).

template <typename T>
class IsoAssocList {
	Vector<T> _items;

public:
	_FORCE_INLINE_ int count() const { return _items.size(); }

	_FORCE_INLINE_ bool contains(const T &p_item) const {
		return _items.find(p_item) != -1;
	}

	bool add(const T &p_item) {
		if (_items.find(p_item) != -1) {
			return false;
		}
		_items.push_back(p_item);
		return true;
	}

	bool remove(const T &p_item) {
		int idx = _items.find(p_item);
		if (idx == -1) {
			return false;
		}
		int last = _items.size() - 1;
		if (idx != last) {
			_items.write[idx] = _items[last];
		}
		_items.resize(last);
		return true;
	}

	_FORCE_INLINE_ const T &operator[](int p_index) const {
		return _items[p_index];
	}

	T pop() {
		ERR_FAIL_COND_V(_items.size() == 0, T());
		T item = _items[_items.size() - 1];
		_items.resize(_items.size() - 1);
		return item;
	}

	void clear() {
		_items.clear();
	}

	IsoAssocList() {}
};

#endif // ISO_ASSOC_LIST_H
