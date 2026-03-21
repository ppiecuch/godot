/**************************************************************************/
/*  raw_packer.cpp                                                        */
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

#include "raw_packer.h"

static const CharType TerminatingZero('\0');

Error RawPacker::encode(const String &fmt, const Array &array, uint8_t *buf, int &len) {
	Variant::Type type;
	int j = 0;

	len = 0;

	for (int i = 0; i < fmt.length(); i++) {
		ERR_FAIL_COND_V(j >= array.size(), ERR_INVALID_DATA);

		type = array[j].get_type();

		switch (fmt[i]) {
			case 'c':
			case 'b':
			case 'B': {
				ERR_FAIL_COND_V(type != Variant::INT, ERR_INVALID_DATA);

				if (buf) {
					*buf = (uint8_t)array[j];
					buf += 1;
				}

				len += 1;

			} break;

			case '?': {
				ERR_FAIL_COND_V(type != Variant::BOOL, ERR_INVALID_DATA);

				if (buf) {
					*buf = (uint8_t)array[j];
					buf += 1;
				}

				len += 1;

			} break;

			case 'h':
			case 'H': {
				ERR_FAIL_COND_V(type != Variant::INT, ERR_INVALID_DATA);

				if (buf) {
					encode_uint16((uint16_t)array[j], buf);
					buf += 2;
				}

				len += 2;

			} break;

			case 'i':
			case 'I':

				//windows doesn't like longs
				/*case 'l':
			case 'L':*/
				{
					ERR_FAIL_COND_V(type != Variant::INT, ERR_INVALID_DATA);

					if (buf) {
						encode_uint32((uint32_t)array[j], buf);
						buf += 4;
					}

					len += 4;
				}
				break;

			case 'q':
			case 'Q': {
				ERR_FAIL_COND_V(type != Variant::INT, ERR_INVALID_DATA);

				if (buf) {
					encode_uint64((uint64_t)array[j], buf);
					buf += 8;
				}

				len += 8;

			} break;

			case 'f': {
				ERR_FAIL_COND_V(type != Variant::REAL, ERR_INVALID_DATA);

				if (buf) {
					encode_float((float)array[j], buf);
					buf += 4;
				}

				len += 4;

			} break;

			case 'd': {
				ERR_FAIL_COND_V(type != Variant::REAL, ERR_INVALID_DATA);

				if (buf) {
					encode_double((double)array[j], buf);
					buf += 8;
				}

				len += 8;

			} break;

			case 's': {
				ERR_FAIL_COND_V(type != Variant::STRING, ERR_INVALID_DATA);

				String size_str;

				for (int k = i + 1; k < fmt.length(); k++) {
					if (is_digit(fmt[k])) {
						size_str += fmt[k];
					} else {
						break;
					}
				}

				size_str += TerminatingZero;

				i += size_str.length();

				int str_size = size_str.to_int();

				if (str_size == 0) {
					if (buf) {
						*buf = '\0';
						buf += 1;
					}

					len += 1;
				} else {
					if (buf) {
						String str = (String)array[j];

						int k = 0;

						for (; k < str.size() && k < str_size; k++) {
							*buf = str[k];
							buf += 1;
						}

						for (; k < str_size; k++) {
							*buf = '\0';
							buf += 1;
						}
					}

					len += str_size;
				}

			} break;

			case 'v': {
				ERR_FAIL_COND_V(type != Variant::STRING, ERR_INVALID_DATA);

				String str = (String)array[j];

				if (buf) {
					for (int k = 0; k < str.size(); k++) {
						*buf = str[k];
						buf += 1;
					}
				}

				len += str.size();

			} break;

			default:
				ERR_FAIL_V(ERR_INVALID_PARAMETER);
		}

		j++;
	}

	return OK;
}

Error RawPacker::decode(const String &fmt, Array &array, const uint8_t *buf, int size) {
	int len = 0;

	for (int i = 0; i < fmt.length(); i++) {
		switch (fmt[i]) {
			case 'c':
			case 'b': {
				ERR_FAIL_COND_V(size < len + 1, ERR_INVALID_DATA);

				array.push_back(Variant((char)*buf));
				buf += 1;
				len += 1;
			} break;

			case 'B': {
				ERR_FAIL_COND_V(size < len + 1, ERR_INVALID_DATA);

				array.push_back(Variant((unsigned char)*buf));
				buf += 1;
				len += 1;
			} break;

			case '?': {
				ERR_FAIL_COND_V(size < len + 1, ERR_INVALID_DATA);

				array.push_back(Variant((bool)*buf));
				buf += 1;
				len += 1;
			} break;

			case 'h': {
				ERR_FAIL_COND_V(size < len + 2, ERR_INVALID_DATA);

				array.push_back(Variant((short)decode_uint16(buf)));
				buf += 2;
				len += 2;
			} break;

			case 'H': {
				ERR_FAIL_COND_V(size < len + 2, ERR_INVALID_DATA);

				array.push_back(Variant((unsigned short)decode_uint16(buf)));
				buf += 2;
				len += 2;
			} break;

			case 'i': {
				ERR_FAIL_COND_V(size < len + 4, ERR_INVALID_DATA);

				array.push_back(Variant((int)decode_uint32(buf)));
				buf += 4;
				len += 4;
			} break;

			case 'I': {
				ERR_FAIL_COND_V(size < len + 4, ERR_INVALID_DATA);

				array.push_back(Variant((unsigned int)decode_uint32(buf)));
				buf += 4;
				len += 4;
			} break;

#if 0 // windows doesn't like longs
			case 'l': {

				ERR_FAIL_COND_V(size<len+4,ERR_INVALID_DATA);

				array.push_back(Variant((long)decode_uint32(buf)));
				buf+=4;
				len+=4;
			} break;

			case 'L': {

				ERR_FAIL_COND_V(size<len+4,ERR_INVALID_DATA);

				array.push_back(Variant((unsigned long)decode_uint32(buf)));
				buf+=4;
				len+=4;
			} break;
#endif
			case 'q': {
				ERR_FAIL_COND_V(size < len + 8, ERR_INVALID_DATA);

				array.push_back(Variant((int64_t)decode_uint64(buf)));
				buf += 8;
				len += 8;
			} break;

			case 'Q': {
				ERR_FAIL_COND_V(size < len + 8, ERR_INVALID_DATA);

				array.push_back(Variant((uint64_t)decode_uint64(buf)));
				buf += 8;
				len += 8;
			} break;

			case 'f': {
				ERR_FAIL_COND_V(size < len + 4, ERR_INVALID_DATA);

				array.push_back(Variant((float)decode_float(buf)));
				buf += 4;
				len += 4;
			} break;

			case 'd': {
				ERR_FAIL_COND_V(size < len + 8, ERR_INVALID_DATA);

				array.push_back(Variant((double)decode_double(buf)));
				buf += 8;
				len += 8;
			} break;

			case 's': {
				String size_str;

				for (int k = i + 1; k < fmt.length(); k++) {
					if (is_digit(fmt[k])) {
						size_str += fmt[k];
					} else {
						break;
					}
				}

				size_str += TerminatingZero;

				i += size_str.length();

				int str_size = size_str.to_int();

				String str;

				if (str_size == 0) {
					ERR_FAIL_COND_V(size < len + 1, ERR_INVALID_DATA);

					str += TerminatingZero;
					buf += 1;
					len += 1;
				} else {
					ERR_FAIL_COND_V(size < len + str_size, ERR_INVALID_DATA);

					int k = 0;

					while (*buf != '\0' && k < str_size - 1) {
						str += (char)*buf;
						buf += 1;
						k += 1;
					}

					str += TerminatingZero;
					buf += (str_size - k);
					len += str_size;
				}

				array.push_back(str);
			} break;

			case 'v': {
				ERR_FAIL_COND_V(size < len + 1, ERR_INVALID_DATA);

				String str;

				while (*buf != '\0') {
					str += (char)*buf;
					buf += 1;
					len += 1;

					ERR_FAIL_COND_V(size < len + 1, ERR_INVALID_DATA);
				}

				str += TerminatingZero;
				buf += 1;
				len += 1;
				array.push_back(str);
			} break;

			default:
				ERR_FAIL_V(ERR_INVALID_PARAMETER);
		}
	}

	return OK;
}

bool RawPacker::is_digit(char c) {
	return (c == '0' || c == '1' ||
			c == '2' || c == '3' ||
			c == '4' || c == '5' ||
			c == '6' || c == '7' ||
			c == '8' || c == '9');
}

PoolByteArray RawPacker::pack(const String &fmt, const Array &array) {
	int len = 0;
	Error err = encode(fmt, array, NULL, len);

	if (err != OK)
		return PoolByteArray();

	PoolByteArray res;
	res.resize(len);

	PoolByteArray::Write w = res.write();
	encode(fmt, array, w.ptr(), len);

	return res;
}

Array RawPacker::unpack(const String &fmt, const PoolByteArray &array) {
	Array res;

	PoolByteArray::Read r = array.read();

	Error err = decode(fmt, res, r.ptr(), array.size());

	if (err != OK)
		return Array();

	return res;
}

void RawPacker::_bind_methods() {
	ClassDB::bind_method(D_METHOD("pack", "format", "values"), &RawPacker::pack);
	ClassDB::bind_method(D_METHOD("unpack", "format", "data"), &RawPacker::unpack);
}

// -- Tests --

#ifdef DOCTEST

TEST_CASE("[RawPacker] pack and unpack integers") {
	RawPacker packer;

	SUBCASE("byte") {
		Array input;
		input.push_back(42);
		PoolByteArray packed = packer.pack("b", input);
		REQUIRE(packed.size() == 1);
		Array unpacked = packer.unpack("b", packed);
		REQUIRE(unpacked.size() == 1);
		CHECK(int(unpacked[0]) == 42);
	}

	SUBCASE("unsigned byte") {
		Array input;
		input.push_back(200);
		PoolByteArray packed = packer.pack("B", input);
		REQUIRE(packed.size() == 1);
		Array unpacked = packer.unpack("B", packed);
		REQUIRE(unpacked.size() == 1);
		CHECK(int(unpacked[0]) == 200);
	}

	SUBCASE("short") {
		Array input;
		input.push_back(0x1234);
		PoolByteArray packed = packer.pack("h", input);
		REQUIRE(packed.size() == 2);
		Array unpacked = packer.unpack("h", packed);
		REQUIRE(unpacked.size() == 1);
		CHECK(int(unpacked[0]) == 0x1234);
	}

	SUBCASE("int32") {
		Array input;
		input.push_back(0x12345678);
		PoolByteArray packed = packer.pack("i", input);
		REQUIRE(packed.size() == 4);
		Array unpacked = packer.unpack("i", packed);
		REQUIRE(unpacked.size() == 1);
		CHECK(int(unpacked[0]) == 0x12345678);
	}

	SUBCASE("int64") {
		Array input;
		input.push_back((int64_t)0x123456789ABCLL);
		PoolByteArray packed = packer.pack("q", input);
		REQUIRE(packed.size() == 8);
		Array unpacked = packer.unpack("q", packed);
		REQUIRE(unpacked.size() == 1);
		CHECK(int64_t(unpacked[0]) == (int64_t)0x123456789ABCLL);
	}
}

TEST_CASE("[RawPacker] pack and unpack float/double") {
	RawPacker packer;

	SUBCASE("float") {
		Array input;
		input.push_back(3.14f);
		PoolByteArray packed = packer.pack("f", input);
		REQUIRE(packed.size() == 4);
		Array unpacked = packer.unpack("f", packed);
		REQUIRE(unpacked.size() == 1);
		CHECK(float(unpacked[0]) == doctest::Approx(3.14f).epsilon(0.001));
	}

	SUBCASE("double") {
		Array input;
		input.push_back(2.718281828);
		PoolByteArray packed = packer.pack("d", input);
		REQUIRE(packed.size() == 8);
		Array unpacked = packer.unpack("d", packed);
		REQUIRE(unpacked.size() == 1);
		CHECK(double(unpacked[0]) == doctest::Approx(2.718281828).epsilon(0.0001));
	}
}

TEST_CASE("[RawPacker] pack and unpack bool") {
	RawPacker packer;
	Array input;
	input.push_back(true);
	input.push_back(false);
	PoolByteArray packed = packer.pack("??", input);
	REQUIRE(packed.size() == 2);
	Array unpacked = packer.unpack("??", packed);
	REQUIRE(unpacked.size() == 2);
	CHECK(bool(unpacked[0]) == true);
	CHECK(bool(unpacked[1]) == false);
}

TEST_CASE("[RawPacker] pack multiple types") {
	RawPacker packer;
	Array input;
	input.push_back(1); // byte
	input.push_back(1000); // short
	input.push_back(3.14f); // float
	PoolByteArray packed = packer.pack("bhf", input);
	REQUIRE(packed.size() == 1 + 2 + 4);
	Array unpacked = packer.unpack("bhf", packed);
	REQUIRE(unpacked.size() == 3);
	CHECK(int(unpacked[0]) == 1);
	CHECK(int(unpacked[1]) == 1000);
	CHECK(float(unpacked[2]) == doctest::Approx(3.14f).epsilon(0.001));
}

TEST_CASE("[RawPacker] empty input") {
	RawPacker packer;
	Array input;
	PoolByteArray packed = packer.pack("", input);
	CHECK(packed.size() == 0);
	Array unpacked = packer.unpack("", packed);
	CHECK(unpacked.size() == 0);
}

#endif
