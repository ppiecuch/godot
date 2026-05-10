/**************************************************************************/
/*  gdnet_queue.h                                                         */
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

/* gdnet_rb.h */

#ifndef GDNET_QUEUE_H
#define GDNET_QUEUE_H

#include "core/os/memory.h"

template <class T, int SIZE = 1024>
class GDNetQueue {
	T *items[SIZE];

	int read_pos;
	int write_pos;

public:
	bool is_empty() {
		return (read_pos == write_pos);
	}

	bool is_full() {
		return ((write_pos + 1) % SIZE == read_pos);
	}

	int size() {
		if (write_pos > read_pos)
			return write_pos - read_pos;
		else if (write_pos < read_pos)
			return (SIZE - read_pos) + write_pos;
		else
			return 0;
	}

	void push(T *item) {
		ERR_FAIL_COND(is_full());

		items[write_pos] = item;
		write_pos = (write_pos + 1) % SIZE;
	}

	T *pop() {
		ERR_FAIL_COND_V(is_empty(), NULL);

		T *item = items[read_pos];
		read_pos = (read_pos + 1) % SIZE;
		return item;
	}

	void clear() {
		while (!is_empty()) {
			memdelete(pop());
		}
	}

	GDNetQueue() { read_pos = write_pos = 0; }
};

#endif
