// Copyright (c) 2019 Windy Darian. MIT License.
// Custom script library for things.

// https://github.com/WindyDarian/godot_module_w/blob/master/w.lib.cpp

#pragma once

#include "core/object.h"
#include "core/math/quat.h"

class tags_impl_t;

class Tags : public Object {
	GDCLASS(Tags, Object);
	static Tags* singleton;

protected:
	static void _bind_methods();

public:
	/** Register a new tag. */
	void define_tag(const String& tag_name);
	/** Get tag as int from name. */
	int get_tag(const String& tag_name);
	/**
	** Test if tag a match tag b.
	** returns true if a is same or derived from b
	** a "foo.bar" matches b "foo"
	** a "foo" does NOT match b "foo.bar"
	*/
	bool match_tag(int a, int b);

	static Tags* get_singleton() {return singleton;};
	Tags();
	virtual ~Tags();

private:
	tags_impl_t* impl;
};
