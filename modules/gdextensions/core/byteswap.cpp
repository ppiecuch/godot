/**************************************************************************/
/*  byteswap.cpp                                                          */
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

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

#include "byteswap.h"

int Byteswap::reverse_int(int p_value) {
	uint32_t v = (uint32_t)p_value;
	return (int)((v >> 24) | ((v >> 8) & 0xFF00) | ((v << 8) & 0xFF0000) | (v << 24));
}

float Byteswap::reverse_float(float p_value) {
	float ret;
	const char *src = (const char *)&p_value;
	char *dst = (char *)&ret;
	dst[0] = src[3];
	dst[1] = src[2];
	dst[2] = src[1];
	dst[3] = src[0];
	return ret;
}

int64_t Byteswap::reverse_int64(int64_t p_value) {
	uint64_t v = (uint64_t)p_value;
	return (int64_t)((v >> 56) |
			((v >> 40) & 0xFF00ULL) |
			((v >> 24) & 0xFF0000ULL) |
			((v >> 8) & 0xFF000000ULL) |
			((v << 8) & 0xFF00000000ULL) |
			((v << 24) & 0xFF0000000000ULL) |
			((v << 40) & 0xFF000000000000ULL) |
			(v << 56));
}

int Byteswap::reverse_short(int p_value) {
	uint16_t v = (uint16_t)(p_value & 0xFFFF);
	return (int)((v >> 8) | (v << 8));
}

PoolByteArray Byteswap::reverse_bytes(const PoolByteArray &p_data) {
	int len = p_data.size();
	PoolByteArray result;
	result.resize(len);
	PoolByteArray::Read r = p_data.read();
	PoolByteArray::Write w = result.write();
	for (int i = 0; i < len; i++) {
		w[i] = r[len - 1 - i];
	}
	return result;
}

void Byteswap::_bind_methods() {
	ClassDB::bind_method(D_METHOD("reverse_int", "value"), &Byteswap::reverse_int);
	ClassDB::bind_method(D_METHOD("reverse_float", "value"), &Byteswap::reverse_float);
	ClassDB::bind_method(D_METHOD("reverse_int64", "value"), &Byteswap::reverse_int64);
	ClassDB::bind_method(D_METHOD("reverse_short", "value"), &Byteswap::reverse_short);
	ClassDB::bind_method(D_METHOD("reverse_bytes", "data"), &Byteswap::reverse_bytes);
	// Legacy bindings
	ClassDB::bind_method("reverseFloat", &Byteswap::reverseFloat);
	ClassDB::bind_method("reverseInt", &Byteswap::reverseInt);
}

// -- Tests --

#ifdef DOCTEST

TEST_CASE("[Byteswap] reverse_int") {
	CHECK(Byteswap().reverse_int(0x01020304) == 0x04030201);
	CHECK(Byteswap().reverse_int(0) == 0);
	CHECK(Byteswap().reverse_int(0xFF000000) == 0x000000FF);
	CHECK(Byteswap().reverse_int(0x00FF0000) == 0x0000FF00);
	// Double reverse should return original
	CHECK(Byteswap().reverse_int(Byteswap().reverse_int(0xDEADBEEF)) == (int)0xDEADBEEF);
}

TEST_CASE("[Byteswap] reverse_short") {
	CHECK(Byteswap().reverse_short(0x0102) == 0x0201);
	CHECK(Byteswap().reverse_short(0) == 0);
	CHECK(Byteswap().reverse_short(0xFF00) == 0x00FF);
	CHECK(Byteswap().reverse_short(Byteswap().reverse_short(0xABCD)) == 0xABCD);
}

TEST_CASE("[Byteswap] reverse_int64") {
	CHECK(Byteswap().reverse_int64(0x0102030405060708LL) == 0x0807060504030201LL);
	CHECK(Byteswap().reverse_int64(0) == 0);
	CHECK(Byteswap().reverse_int64(Byteswap().reverse_int64(0xDEADBEEFCAFEBABELL)) == (int64_t)0xDEADBEEFCAFEBABELL);
}

TEST_CASE("[Byteswap] reverse_float") {
	// Double reverse should preserve value
	float original = 3.14159f;
	CHECK(Byteswap().reverse_float(Byteswap().reverse_float(original)) == original);

	// Zero should remain zero
	CHECK(Byteswap().reverse_float(0.0f) == 0.0f);
}

TEST_CASE("[Byteswap] reverse_bytes") {
	PoolByteArray data;
	data.push_back(1);
	data.push_back(2);
	data.push_back(3);
	data.push_back(4);

	PoolByteArray reversed = Byteswap().reverse_bytes(data);
	REQUIRE(reversed.size() == 4);
	PoolByteArray::Read r = reversed.read();
	CHECK(r[0] == 4);
	CHECK(r[1] == 3);
	CHECK(r[2] == 2);
	CHECK(r[3] == 1);

	SUBCASE("empty array") {
		PoolByteArray empty;
		PoolByteArray result = Byteswap().reverse_bytes(empty);
		CHECK(result.size() == 0);
	}

	SUBCASE("single byte") {
		PoolByteArray single;
		single.push_back(42);
		PoolByteArray result = Byteswap().reverse_bytes(single);
		REQUIRE(result.size() == 1);
		CHECK(result.read()[0] == 42);
	}

	SUBCASE("double reverse is identity") {
		PoolByteArray result = Byteswap().reverse_bytes(Byteswap().reverse_bytes(data));
		REQUIRE(result.size() == data.size());
		PoolByteArray::Read r1 = data.read();
		PoolByteArray::Read r2 = result.read();
		for (int i = 0; i < data.size(); i++) {
			CHECK(r1[i] == r2[i]);
		}
	}
}

#endif
