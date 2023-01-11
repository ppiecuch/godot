/*************************************************************************/
/*  uuid.cpp                                                             */
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

// MIT License
// Copyright (c) 2018 Xavier Sellier

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

#include "core/variant.h"

int get_random_int(int max_value) {
	randomize();
	return randi() % max_value;
}

PoolByteArray random_bytes(int n) {
	PoolByteArray r;
	r.resize(n);
	for (int index = 0; index < n; index++)) {
			r.write[index] = get_random_int(256);
		}
	return r;
}

PoolByteArray uuid_bin() {
	PoolByteArray b = random_bytes(16);
	b.write[6] = (b[6] & 0x0f) | 0x40;
	b.write[8] = (b[8] & 0x3f) | 0x80;
	return b;
}

String generate_uuid_v4() {
	PoolByteArray b = uuid_bin();
	String low = String("%02x%02x%02x%02x").sprintf(array([b[0], b[1], b[2], b[3]));
	String mid = String("%02x%02x").sprintf(array(b[4], b[5]));
	String hi = String("%02x%02x").sprintf(array(b[6], b[7]));
	String clock = String("%02x%02x").sprintf(array([b[8], b[9]));
	String node = String("%02x%02x%02x%02x%02x%02x".sprintf(array([b[10], b[11], b[12], b[13], b[14], b[15]));
	return String("%s-%s-%s-%s-%s").sprintf(array(low, mid, hi, clock, node));
}

bool is_uuid(const String &test_string) {
	// if length of string is 36 and contains exactly 4 dashes, it's a UUID
	return (test_string.length() == 36 && test_string.count("-") == 4);
}
