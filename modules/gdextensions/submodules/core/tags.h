/**************************************************************************/
/*  tags.h                                                                */
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

// Copyright (c) 2019 Windy Darian. MIT License.
// Custom script library for things.

// https://github.com/WindyDarian/godot_module_w/blob/master/w.lib.cpp

#ifndef COMMON_TAGS_H
#define COMMON_TAGS_H

#include "core/math/quat.h"
#include "core/object.h"

class tags_impl_t;

class Tags : public Object {
	GDCLASS(Tags, Object);

	static Tags *singleton;
	tags_impl_t *impl;

protected:
	static void _bind_methods();

public:
	void define_tag(const String &tag_name); // Register a new tag.
	int get_tag(const String &tag_name); // Get tag as int from name.
	// Test if tag a match tag b.
	// returns true if a is same or derived from b
	// a "foo.bar" matches b "foo"
	// a "foo" does NOT match b "foo.bar"
	bool match_tag(int a, int b);

	static Tags *get_singleton() { return singleton; };

	Tags();
	virtual ~Tags();
};

#endif // COMMON_TAGS_H
