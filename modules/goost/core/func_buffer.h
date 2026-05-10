/**************************************************************************/
/*  func_buffer.h                                                         */
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

#include "core/object.h"

class FuncBuffer {
public:
	struct Call {
		ObjectID instance_id;
		StringName target;
		uint8_t argcount;
		Vector<Variant> args;
	};

private:
	Vector<Call *> buffer;

public:
	Error push_call(ObjectID p_id, const StringName &p_method, const Variant **p_args, int p_argcount);
	Error push_call(ObjectID p_id, const StringName &p_method, VARIANT_ARG_DECLARE);

	Error push_call_unique(ObjectID p_id, const StringName &p_method, const Variant **p_args, int p_argcount);
	Error push_call_unique(ObjectID p_id, const StringName &p_method, VARIANT_ARG_DECLARE);

	void flush();

	~FuncBuffer() {
		for (int i = 0; i < buffer.size(); ++i) {
			memdelete(buffer[i]);
		}
		buffer.clear();
	}
};
