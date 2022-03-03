/*************************************************************************/
/*  queue.hpp                                                            */
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

//
// Created by Gen on 16/1/6.
//

#ifndef GODOT_MAIN_QUEUE_HPP
#define GODOT_MAIN_QUEUE_HPP

#include "../../core/os/memory.h"

namespace GdExtends {

template <class T>
class Queue {
	int _limit;
	int _size;
	int _offset;
	T *mem;

public:
	void alloc(int p_size) {
		if (mem) {
			memdelete_arr(mem);
		}
		mem = p_size > 0 ? memnew_arr(T, p_size) : NULL;
		_size = 0;
		_offset = 0;
		_limit = p_size;
	}

	_FORCE_INLINE_ int limit() { return _limit; }

	const T &get(int index) const {
		return operator[](index);
	}

	T &get(int index) {
		return operator[](index);
	}

	void set(int index, T value) {
		if (index < _size) {
			operator[](index);
		}
	}

	_FORCE_INLINE_ T &operator[](int p_index) {
		CRASH_BAD_INDEX(p_index, _size);
		return mem[(_offset + p_index) % _limit];
	}

	int size() const {
		return _size;
	}

	T *push(const T &p_value) {
		int off = (_offset + _size) % _limit;
		mem[off] = p_value;
		_size++;
		if (_size > _limit) {
			int m = _size - _limit;
			_size = _limit;
			_offset = (_offset + m) % _limit;
		}
		return &mem[off];
	}

	T pop() {
		if (_size > 0) {
			T res = mem[_offset++];
			_size--;
			return res;
		} else {
			return T();
		}
	}

	void clear() {
		_size = 0;
	}

	Queue() {
		mem = NULL;
		_limit = 0;
		_size = 0;
		_offset = 0;
	}
	~Queue() {
		if (mem) {
			memdelete_arr(mem);
		}
	}
};
} // namespace GdExtends

#endif //GODOT_MAIN_QUEUE_HPP
