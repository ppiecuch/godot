/**************************************************************************/
/*  hashfuncs.h                                                           */
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

#ifndef HASHFUNCS_H
#define HASHFUNCS_H

#include "core/math/math_defs.h"
#include "core/math/math_funcs.h"
#include "core/node_path.h"
#include "core/string_name.h"
#include "core/typedefs.h"
#include "core/ustring.h"

/**
 * Hashing functions
 */

/**
 * DJB2 Hash function
 * @param C String
 * @return 32-bits hashcode
 */
static inline uint32_t hash_djb2(const char *p_cstr) {
	const unsigned char *chr = (const unsigned char *)p_cstr;
	uint32_t hash = 5381;
	uint32_t c;

	while ((c = *chr++)) {
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}

	return hash;
}

static inline uint32_t hash_djb2_buffer(const uint8_t *p_buff, int p_len, uint32_t p_prev = 5381) {
	uint32_t hash = p_prev;

	for (int i = 0; i < p_len; i++) {
		hash = ((hash << 5) + hash) + p_buff[i]; /* hash * 33 + c */
	}

	return hash;
}

static inline uint32_t hash_djb2_one_32(uint32_t p_in, uint32_t p_prev = 5381) {
	return ((p_prev << 5) + p_prev) + p_in;
}

static inline uint32_t hash_one_uint64(uint64_t p_int) {
	uint64_t v = p_int;
	v = (~v) + (v << 18); // v = (v << 18) - v - 1;
	v = v ^ (v >> 31);
	v = v * 21; // v = (v + (v << 2)) + (v << 4);
	v = v ^ (v >> 11);
	v = v + (v << 6);
	v = v ^ (v >> 22);
	return (int)v;
}

static inline uint32_t hash_djb2_one_float(double p_in, uint32_t p_prev = 5381) {
	union {
		double d;
		uint64_t i;
	} u;

	// Normalize +/- 0.0 and NaN values so they hash the same.
	if (p_in == 0.0f) {
		u.d = 0.0;
	} else if (Math::is_nan(p_in)) {
		u.d = Math_NAN;
	} else {
		u.d = p_in;
	}

	return ((p_prev << 5) + p_prev) + hash_one_uint64(u.i);
}

template <class T>
static inline uint32_t make_uint32_t(T p_in) {
	union {
		T t;
		uint32_t _u32;
	} _u;
	_u._u32 = 0;
	_u.t = p_in;
	return _u._u32;
}

static inline uint64_t hash_djb2_one_64(uint64_t p_in, uint64_t p_prev = 5381) {
	return ((p_prev << 5) + p_prev) + p_in;
}

template <class T>
static inline uint64_t make_uint64_t(T p_in) {
	union {
		T t;
		uint64_t _u64;
	} _u;
	_u._u64 = 0; // in case p_in is smaller

	_u.t = p_in;
	return _u._u64;
}

struct HashMapHasherDefault {
	static _FORCE_INLINE_ uint32_t hash(const String &p_string) { return p_string.hash(); }
	static _FORCE_INLINE_ uint32_t hash(const char *p_cstr) { return hash_djb2(p_cstr); }
#ifdef NEED_LONG_INT
	static _FORCE_INLINE_ uint32_t hash(signed long p_int) { return (uint32_t)p_int; }
	static _FORCE_INLINE_ uint32_t hash(unsigned long p_int) { return (uint32_t)p_int; }
#endif
	static _FORCE_INLINE_ uint32_t hash(uint64_t p_int) { return hash_one_uint64(p_int); }
	static _FORCE_INLINE_ uint32_t hash(int64_t p_int) { return hash(uint64_t(p_int)); }
	static _FORCE_INLINE_ uint32_t hash(float p_float) { return hash_djb2_one_float(p_float); }
	static _FORCE_INLINE_ uint32_t hash(double p_double) { return hash_djb2_one_float(p_double); }
	static _FORCE_INLINE_ uint32_t hash(uint32_t p_int) { return p_int; }
	static _FORCE_INLINE_ uint32_t hash(int32_t p_int) { return (uint32_t)p_int; }
	static _FORCE_INLINE_ uint32_t hash(uint16_t p_int) { return p_int; }
	static _FORCE_INLINE_ uint32_t hash(int16_t p_int) { return (uint32_t)p_int; }
	static _FORCE_INLINE_ uint32_t hash(uint8_t p_int) { return p_int; }
	static _FORCE_INLINE_ uint32_t hash(int8_t p_int) { return (uint32_t)p_int; }
	static _FORCE_INLINE_ uint32_t hash(wchar_t p_wchar) { return (uint32_t)p_wchar; }

	static _FORCE_INLINE_ uint32_t hash(const StringName &p_string_name) { return p_string_name.hash(); }
	static _FORCE_INLINE_ uint32_t hash(const NodePath &p_path) { return p_path.hash(); }

	//static _FORCE_INLINE_ uint32_t hash(const void* p_ptr)  { return uint32_t(uint64_t(p_ptr))*(0x9e3779b1L); }
};

template <typename T>
struct HashMapComparatorDefault {
	static bool compare(const T &p_lhs, const T &p_rhs) {
		return p_lhs == p_rhs;
	}

	bool compare(float p_lhs, float p_rhs) {
		return (p_lhs == p_rhs) || (Math::is_nan(p_lhs) && Math::is_nan(p_rhs));
	}

	bool compare(double p_lhs, double p_rhs) {
		return (p_lhs == p_rhs) || (Math::is_nan(p_lhs) && Math::is_nan(p_rhs));
	}
};

constexpr uint32_t HASH_TABLE_SIZE_MAX = 29;

const uint32_t hash_table_size_primes[HASH_TABLE_SIZE_MAX] = {
	5,
	13,
	23,
	47,
	97,
	193,
	389,
	769,
	1543,
	3079,
	6151,
	12289,
	24593,
	49157,
	98317,
	196613,
	393241,
	786433,
	1572869,
	3145739,
	6291469,
	12582917,
	25165843,
	50331653,
	100663319,
	201326611,
	402653189,
	805306457,
	1610612741,
};

// Computed with elem_i = UINT64_C (0 x FFFFFFFF FFFFFFFF ) / d_i + 1, where d_i is the i-th element of the above array.
const uint64_t hash_table_size_primes_inv[HASH_TABLE_SIZE_MAX] = {
	3689348814741910324,
	1418980313362273202,
	802032351030850071,
	392483916461905354,
	190172619316593316,
	95578984837873325,
	47420935922132524,
	23987963684927896,
	11955116055547344,
	5991147799191151,
	2998982941588287,
	1501077717772769,
	750081082979285,
	375261795343686,
	187625172388393,
	93822606204624,
	46909513691883,
	23456218233098,
	11728086747027,
	5864041509391,
	2932024948977,
	1466014921160,
	733007198436,
	366503839517,
	183251896093,
	91625960335,
	45812983922,
	22906489714,
	11453246088
};

// Fastmod computes ( n mod d ) given the precomputed c much faster than n % d.
// The implementation of fastmod is based on the following paper by Daniel Lemire et al.
// Faster Remainder by Direct Computation: Applications to Compilers and Software Libraries
// https://arxiv.org/abs/1902.01961

static _FORCE_INLINE_ uint32_t fastmod(const uint32_t n, const uint64_t c, const uint32_t d) {
#if defined(_MSC_VER)
	// Returns the upper 64 bits of the product of two 64-bit unsigned integers.
	// This intrinsic function is required since MSVC does not support unsigned 128-bit integers.
#if defined(_M_X64) || defined(_M_ARM64)
	return __umulh(c * n, d);
#else
	// Fallback to the slower method for 32-bit platforms.
	return n % d;
#endif // _M_X64 || _M_ARM64
#else
#ifdef __SIZEOF_INT128__
	// Prevent compiler warning, because we know what we are doing.
	uint64_t lowbits = c * n;
	__extension__ typedef unsigned __int128 uint128;
	return static_cast<uint64_t>(((uint128)lowbits * d) >> 64);
#else
	// Fallback to the slower method if no 128-bit unsigned integer type is available.
	return n % d;
#endif // __SIZEOF_INT128__
#endif // _MSC_VER
}

#endif // HASHFUNCS_H
