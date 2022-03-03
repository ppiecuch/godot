/*************************************************************************/
/*  tags.cpp                                                             */
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

#include "tags.h"

using tag_t = int32_t;
const tag_t k_tag_null = 0;
const int k_tag_segment_bits = 8;
const int k_tag_max_segments = 4;

class tags_impl_t {
	/* Internal data of module. Currently mainly used for a tag system (inspired from Unreal's gameplay tag system). */
public:
	Dictionary tag_dictionary; // String to array [int tag_id, int next available segment sub id from 1]
	int next_root_tag_id = 1; // Initialize to 1 so 0 is a "null" option
	/*
	** Tag ID: a.b.c.d has ddccbbaa order! children always take most significant bits.
	*/

	void define_tag(const String &tag_name);
	tag_t get_tag(const String &tag_name);
	bool match_tag(tag_t a, tag_t b);
};

Tags *Tags::singleton = nullptr;

void tags_impl_t::define_tag(const String &tag_name) {
	int split_location = -1;

	Variant arr_parent = Variant();
	for (int segment = 0;; ++segment) {
		ERR_FAIL_COND_MSG(segment >= k_tag_max_segments, "Tag has too many segment (.) s!");

		split_location = tag_name.find_char('.', split_location + 1);
		String current_parent_tag_name = tag_name.substr(0, split_location);
		//Variant current_parent_tag_name_variant = {current_parent_tag_name};
		Variant *p_value = tag_dictionary.getptr(current_parent_tag_name);
		if (p_value) {
			if (split_location == -1) {
				ERR_PRINT((String("tag already exists!") + tag_name));
				return;
			}
			ERR_FAIL_COND_MSG(p_value->get_type() != Variant::Type::ARRAY, "Invalid type from tag dictionary. Expect array with [int tag_id, int sub_tag_count]");

			arr_parent = *p_value;
		} else {
			int next_segment_id;
			int parent_id;
			if (arr_parent) {
				Array arr_parent_arr = arr_parent.operator Array();
				parent_id = arr_parent_arr[0];
				next_segment_id = arr_parent_arr[1];
			} else {
				next_segment_id = next_root_tag_id;
				parent_id = 0;
			}
			ERR_FAIL_COND_MSG(next_segment_id >= (1 << k_tag_segment_bits), "Parent segment has too many sub tags!");

			Array new_entry;
			new_entry.resize(2);
			int new_entry_id = parent_id | (next_segment_id << (segment * k_tag_segment_bits));
			int inital_segment_id = 1;
			new_entry[0] = Variant(new_entry_id);
			new_entry[1] = Variant(inital_segment_id);

			tag_dictionary[current_parent_tag_name] = new_entry;

			if (arr_parent) {
				Array parent_array = arr_parent.operator Array();
				parent_array[1] = Variant(parent_array[1].operator signed int() + 1);
			} else {
				// No parent. We must be root.
				++next_root_tag_id;
			}
			arr_parent = new_entry;
		}

		if (split_location < 0) {
			break;
		}
	}
}
void Tags::define_tag(const String &tag_name) {
	impl->define_tag(tag_name);
}

tag_t tags_impl_t::get_tag(const String &tag_name) {
	Variant *p_value = tag_dictionary.getptr(tag_name);
	if (!p_value) {
		ERR_PRINT("Unregistered tag: " + tag_name);
		return k_tag_null;
	}
	return tag_t(p_value->operator Array()[0]);
}
int Tags::get_tag(const String &tag_name) {
	return impl->get_tag(tag_name);
}

bool tags_impl_t::match_tag(tag_t a, tag_t b) {
	// returns true if a is same or derived from b
	// a "foo.bar" matches b "foo"
	// a "foo" does NOT match b "foo.bar"

	ERR_FAIL_COND_V_MSG(a == k_tag_null, false, "Invalid tag a");
	ERR_FAIL_COND_V_MSG(b == k_tag_null, false, "Invalid tag b");

	uint32_t mask = (1 << k_tag_segment_bits) - 1;
	for (int i = 0; i < k_tag_max_segments; ++i) {
		uint32_t b_current_segment = (b >> i * k_tag_segment_bits) & mask;
		if (!b_current_segment) {
			break;
		}

		uint32_t a_current_segment = (a >> i * k_tag_segment_bits) & mask;
		if (a_current_segment == b_current_segment) {
			continue;
		} else {
			return false;
		}
	}
	return true;
}
bool Tags::match_tag(int a, int b) {
	return impl->match_tag(a, b);
}

void Tags::_bind_methods() {
	ClassDB::bind_method(D_METHOD("define_tag", "tag_name"), &Tags::define_tag);
	ClassDB::bind_method(D_METHOD("get_tag", "tag_name"), &Tags::get_tag);
	ClassDB::bind_method(D_METHOD("match_tag", "a", "b"), &Tags::match_tag);
}

Tags::Tags() {
	Tags::singleton = this;
	impl = memnew(tags_impl_t);
}

Tags::~Tags() {
	memdelete(impl);
	impl = nullptr;
}
