/*************************************************************************/
/*  tags.h                                                               */
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

// Copyright (c) 2019 Windy Darian. MIT License.
// Custom script library for things.

// https://github.com/WindyDarian/godot_module_w/blob/master/w.lib.cpp

#pragma once

#include "core/math/quat.h"
#include "core/object.h"

class tags_impl_t;

class Tags : public Object {
	GDCLASS(Tags, Object);

	static Tags *singleton;

protected:
	static void _bind_methods();

public:
	/** Register a new tag. */
	void define_tag(const String &tag_name);
	/** Get tag as int from name. */
	int get_tag(const String &tag_name);
	/**
	** Test if tag a match tag b.
	** returns true if a is same or derived from b
	** a "foo.bar" matches b "foo"
	** a "foo" does NOT match b "foo.bar"
	*/
	bool match_tag(int a, int b);

	static Tags *get_singleton() { return singleton; };

	Tags();
	virtual ~Tags();

private:
	tags_impl_t *impl;
};
