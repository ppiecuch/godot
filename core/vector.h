/*************************************************************************/
/*  vector.h                                                             */
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

#ifndef VECTOR_H
#define VECTOR_H

/**
 * @class Vector
 * @author Juan Linietsky
 * Vector container. Regular Vector Container. Use with care and for smaller arrays when possible. Use PoolVector for large arrays.
 */

#include "core/cowdata.h"
#include "core/error_macros.h"
#include "core/os/memory.h"
#include "core/sort_array.h"

template <class T>
class VectorWriteProxy {
public:
	_FORCE_INLINE_ T &operator[](int p_index) {
		CRASH_BAD_INDEX(p_index, ((Vector<T> *)(this))->_cowdata.size());

		return ((Vector<T> *)(this))->_cowdata.ptrw()[p_index];
	}
};

template <class T>
class Vector {
	friend class VectorWriteProxy<T>;

public:
	typedef T value_type;

	VectorWriteProxy<T> write;

private:
	CowData<T> _cowdata;

public:
	// basic c++11 iterator
	struct iterator {
		Vector<T> *_array;
		int _index;
		iterator(Vector<T> *_array, int _index) :
				_array(_array), _index(_index) {}
		iterator(const Vector<T> *_array, int _index) :
				_array(const_cast<Vector<T> *>(_array)), _index(_index) {}
		_FORCE_INLINE_ bool operator!=(const iterator &other) const { return (_array != other._array) || (_index != other._index); }
		_FORCE_INLINE_ T &operator*() { return _array->write[_index]; }
		_FORCE_INLINE_ const T &operator*() const { return _array->get(_index); }
		iterator operator++() {
			_index++;
			return *this;
		}
	};
	typedef const iterator const_iterator;
	iterator begin() { return iterator(this, 0); }
	iterator end() { return iterator(this, size()); }
	const_iterator begin() const { return const_iterator(this, 0); }
	const_iterator end() const { return const_iterator(this, size()); }

	bool push_back(T p_elem);
	/// some convinient push functions (pair/triangle/quad)
	bool push_back(T p_elem1, T p_elem2);
	bool push_back(T p_elem1, T p_elem2, T p_elem3);
	bool push_back(T p_elem1, T p_elem2, T p_elem3, T p_elem4);
	bool push_multi(int p_num, T p_elem);

	void fill(T p_elem);
	void remove(int p_index) { _cowdata.remove(p_index); }
	void erase(const T &p_val) {
		int idx = find(p_val);
		if (idx >= 0) {
			remove(idx);
		}
	};
	void invert();

	_FORCE_INLINE_ T *ptrw() { return _cowdata.ptrw(); }
	_FORCE_INLINE_ const T *ptr() const { return _cowdata.ptr(); }
	_FORCE_INLINE_ void clear() { resize(0); }
	_FORCE_INLINE_ bool empty() const { return _cowdata.empty(); }

	_FORCE_INLINE_ T get(int p_index) { return _cowdata.get(p_index); }
	_FORCE_INLINE_ const T &get(int p_index) const { return _cowdata.get(p_index); }
	_FORCE_INLINE_ void set(int p_index, const T &p_elem) { _cowdata.set(p_index, p_elem); }
	_FORCE_INLINE_ int size() const { return _cowdata.size(); }
	_FORCE_INLINE_ const T &last(int p_index = 0) const { return _cowdata.get(_cowdata.size() - 1 - p_index); }
	Error resize(int p_size) { return _cowdata.resize(p_size); }
	_FORCE_INLINE_ const T &operator[](int p_index) const { return _cowdata.get(p_index); }
	Error insert(int p_pos, T p_val) { return _cowdata.insert(p_pos, p_val); }
	int find(const T &p_val, int p_from = 0) const { return _cowdata.find(p_val, p_from); }
	bool has(const T &p_val, int p_from = 0) const { return _cowdata.find(p_val, p_from) >= 0; }

	void append_array(Vector<T> p_other);
	void append_array(int p_num, const T p_other[]);

	template <class C>
	void sort_custom() {
		int len = _cowdata.size();
		if (len == 0) {
			return;
		}

		T *data = ptrw();
		SortArray<T, C> sorter;
		sorter.sort(data, len);
	}

	void sort() {
		sort_custom<_DefaultComparator<T>>();
	}

	void ordered_insert(const T &p_val) {
		int i;
		for (i = 0; i < _cowdata.size(); i++) {
			if (p_val < operator[](i)) {
				break;
			};
		};
		insert(i, p_val);
	}

	_FORCE_INLINE_ Vector() {}
	_FORCE_INLINE_ Vector(const Vector &p_from) { _cowdata._ref(p_from._cowdata); }
	inline Vector &operator=(const Vector &p_from) {
		_cowdata._ref(p_from._cowdata);
		return *this;
	}

	Vector<uint8_t> to_byte_array() const {
		Vector<uint8_t> ret;
		ret.resize(size() * sizeof(T));
		memcpy(ret.ptrw(), ptr(), sizeof(T) * size());
		return ret;
	}

	Vector<T> slice(int p_begin, int p_end = INT32_MAX) const {
		Vector<T> result;

		const int s = size();

		int begin = CLAMP(p_begin, -s, s);
		if (begin < 0) {
			begin += s;
		}
		int end = CLAMP(p_end, -s, s);
		if (end < 0) {
			end += s;
		}

		ERR_FAIL_COND_V(begin > end, result);

		int result_size = end - begin;
		result.resize(result_size);

		const T *const r = ptr();
		T *const w = result.ptrw();
		for (int i = 0; i < result_size; ++i) {
			w[i] = r[begin + i];
		}

		return result;
	}

	_FORCE_INLINE_ ~Vector() {}
};

template <class T>
void Vector<T>::invert() {
	for (int i = 0; i < size() / 2; i++) {
		T *p = ptrw();
		SWAP(p[i], p[size() - i - 1]);
	}
}

template <class T>
void Vector<T>::append_array(Vector<T> p_other) {
	const int ds = p_other.size();
	if (ds == 0) {
		return;
	}
	const int bs = size();
	resize(bs + ds);
	for (int i = 0; i < ds; ++i) {
		ptrw()[bs + i] = p_other[i];
	}
}

template <class T>
void Vector<T>::append_array(int p_num, const T p_other[]) {
	const int bs = size();
	resize(bs + p_num);
	for (int i = 0; i < p_num; ++i) {
		ptrw()[bs + i] = p_other[i];
	}
}

template <class T>
bool Vector<T>::push_back(T p_elem) {
	Error err = resize(size() + 1);
	ERR_FAIL_COND_V(err, true);
	set(size() - 1, p_elem);

	return false;
}

template <class T>
bool Vector<T>::push_back(T p_elem1, T p_elem2) {
	Error err = resize(size() + 2);
	ERR_FAIL_COND_V(err, true);
	set(size() - 2, p_elem1);
	set(size() - 1, p_elem2);

	return false;
}

template <class T>
bool Vector<T>::push_back(T p_elem1, T p_elem2, T p_elem3) {
	Error err = resize(size() + 3);
	ERR_FAIL_COND_V(err, true);
	set(size() - 3, p_elem1);
	set(size() - 2, p_elem2);
	set(size() - 1, p_elem3);

	return false;
}

template <class T>
bool Vector<T>::push_back(T p_elem1, T p_elem2, T p_elem3, T p_elem4) {
	Error err = resize(size() + 4);
	ERR_FAIL_COND_V(err, true);
	set(size() - 4, p_elem1);
	set(size() - 3, p_elem2);
	set(size() - 2, p_elem3);
	set(size() - 1, p_elem4);

	return false;
}

template <class T>
bool Vector<T>::push_multi(int p_num, T p_elem) {
	const int bs = size();
	Error err = resize(bs + p_num);
	ERR_FAIL_COND_V(err, true);
	for (int i = 0; i < p_num; ++i) {
		ptrw()[bs + i] = p_elem;
	}
	return false;
}

template <class T>
void Vector<T>::fill(T p_elem) {
	T *p = ptrw();
	for (int i = 0; i < size(); i++) {
		p[i] = p_elem;
	}
}

namespace helper {
template <class T>
Vector<T> vector() {
	Vector<T>();
}
template <class T>
Vector<T> vector(const T &p_arg1) {
	Vector<T> p;
	p.push_back(p_arg1);
	return p;
}
template <class T>
Vector<T> vector(const T &p_arg1, const T &p_arg2) {
	Vector<T> p;
	p.push_back(p_arg1);
	p.push_back(p_arg2);
	return p;
}
template <class T>
Vector<T> vector(const T &p_arg1, const T &p_arg2, const T &p_arg3) {
	Vector<T> p;
	p.push_back(p_arg1);
	p.push_back(p_arg2);
	p.push_back(p_arg3);
	return p;
}
template <class T>
Vector<T> vector(const T &p_arg1, const T &p_arg2, const T &p_arg3, const T &p_arg4) {
	Vector<T> p;
	p.push_back(p_arg1);
	p.push_back(p_arg2);
	p.push_back(p_arg3);
	p.push_back(p_arg4);
	return p;
}
template <class T>
Vector<T> vector(const T &p_arg1, const T &p_arg2, const T &p_arg3, const T &p_arg4, const T &p_arg5) {
	Vector<T> p;
	p.push_back(p_arg1);
	p.push_back(p_arg2);
	p.push_back(p_arg3);
	p.push_back(p_arg4);
	p.push_back(p_arg5);
	return p;
}
template <class T>
Vector<T> vector(const T &p_arg1, const T &p_arg2, const T &p_arg3, const T &p_arg4, const T &p_arg5, const T &p_arg6) {
	Vector<T> p;
	p.push_back(p_arg1);
	p.push_back(p_arg2);
	p.push_back(p_arg3);
	p.push_back(p_arg4);
	p.push_back(p_arg5);
	p.push_back(p_arg6);
	return p;
}
} // namespace helper

#endif // VECTOR_H
