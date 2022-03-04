/*************************************************************************/
/*  _stdinc.h                                                            */
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

/// This is a general header that includes C language support.

#ifndef _stdinc_h_
#define _stdinc_h_

#include "_config.h"

#ifdef __APPLE__
#ifndef _DARWIN_C_SOURCE
#define _DARWIN_C_SOURCE 1 // for memset_pattern4()
#endif
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#if defined(STDC_HEADERS)
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#else
#if defined(HAVE_STDLIB_H)
#include <stdlib.h>
#elif defined(HAVE_MALLOC_H)
#include <malloc.h>
#endif
#if defined(HAVE_STDDEF_H)
#include <stddef.h>
#endif
#if defined(HAVE_STDARG_H)
#include <stdarg.h>
#endif
#endif
#ifdef HAVE_STRING_H
#if !defined(STDC_HEADERS) && defined(HAVE_MEMORY_H)
#include <memory.h>
#endif
#include <string.h>
#endif
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif
#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif
#ifdef HAVE_MATH_H
#if defined(__WINRT__)
// Defining _USE_MATH_DEFINES is required to get M_PI to be defined on
// WinRT.  See http://msdn.microsoft.com/en-us/library/4hwaceh6.aspx
// for more information.
#define _USE_MATH_DEFINES
#endif
#include <math.h>
#endif
#ifdef HAVE_FLOAT_H
#include <float.h>
#endif
#if defined(HAVE_ALLOCA) && !defined(alloca)
#if defined(HAVE_ALLOCA_H)
#include <alloca.h>
#elif defined(__GNUC__)
#define alloca __builtin_alloca
#elif defined(_MSC_VER)
#include <malloc.h>
#define alloca _alloca
#elif defined(__WATCOMC__)
#include <malloc.h>
#elif defined(__BORLANDC__)
#include <malloc.h>
#elif defined(__DMC__)
#include <stdlib.h>
#elif defined(__AIX__)
#pragma alloca
#elif defined(__MRC__)
void *alloca(unsigned);
#else
char *alloca();
#endif
#endif

// Check if the compiler supports a given builtin.
// Supported by virtually all clang versions and recent gcc. Use this
// instead of checking the clang version if possible.
#ifdef __has_builtin
#define _SDL_HAS_BUILTIN(x) __has_builtin(x)
#else
#define _SDL_HAS_BUILTIN(x) 0
#endif

// The number of elements in an array.
#define SDL_arraysize(array) (sizeof(array) / sizeof(array[0]))
#define SDL_TABLESIZE(table) SDL_arraysize(table)

/**
 *  Macro useful for building other macros with strings in them
 *
 *  e.g. #define LOG_ERROR(X) OutputDebugString(SDL_STRINGIFY_ARG(__FUNCTION__) ": " X "\n")
 */
#define SDL_STRINGIFY_ARG(arg) #arg

// Use proper C++ casts when compiled as C++ to be compatible with the option
// -Wold-style-cast of GCC (and -Werror=old-style-cast in GCC 4.2 and above).
#ifdef __cplusplus
#define SDL_reinterpret_cast(type, expression) reinterpret_cast<type>(expression)
#define SDL_static_cast(type, expression) static_cast<type>(expression)
#define SDL_const_cast(type, expression) const_cast<type>(expression)
#else
#define SDL_reinterpret_cast(type, expression) ((type)(expression))
#define SDL_static_cast(type, expression) ((type)(expression))
#define SDL_const_cast(type, expression) ((type)(expression))
#endif

// Define a four character code as a Uint32
#define SDL_FOURCC(A, B, C, D)                                             \
	((SDL_static_cast(Uint32, SDL_static_cast(Uint8, (A))) << 0) |         \
			(SDL_static_cast(Uint32, SDL_static_cast(Uint8, (B))) << 8) |  \
			(SDL_static_cast(Uint32, SDL_static_cast(Uint8, (C))) << 16) | \
			(SDL_static_cast(Uint32, SDL_static_cast(Uint8, (D))) << 24))

// Basic data types

#ifdef __CC_ARM
// ARM's compiler throws warnings if we use an enum: like "SDL_bool x = a < b;"
#define SDL_FALSE 0
#define SDL_TRUE 1
typedef int SDL_bool;
#else
typedef enum {
	SDL_FALSE = 0,
	SDL_TRUE = 1
} SDL_bool;
#endif

// A signed 8-bit integer type.
#define SDL_MAX_SINT8 ((Sint8)0x7F) /* 127 */
#define SDL_MIN_SINT8 ((Sint8)(~0x7F)) /* -128 */
typedef int8_t Sint8;

// An unsigned 8-bit integer type.
#define SDL_MAX_UINT8 ((Uint8)0xFF) /* 255 */
#define SDL_MIN_UINT8 ((Uint8)0x00) /* 0 */
typedef uint8_t Uint8;
// A signed 16-bit integer type.
#define SDL_MAX_SINT16 ((Sint16)0x7FFF) /* 32767 */
#define SDL_MIN_SINT16 ((Sint16)(~0x7FFF)) /* -32768 */
typedef int16_t Sint16;
// An unsigned 16-bit integer type.
#define SDL_MAX_UINT16 ((Uint16)0xFFFF) /* 65535 */
#define SDL_MIN_UINT16 ((Uint16)0x0000) /* 0 */
typedef uint16_t Uint16;
// A signed 32-bit integer type.
#define SDL_MAX_SINT32 ((Sint32)0x7FFFFFFF) /* 2147483647 */
#define SDL_MIN_SINT32 ((Sint32)(~0x7FFFFFFF)) /* -2147483648 */
typedef int32_t Sint32;
// An unsigned 32-bit integer type.
#define SDL_MAX_UINT32 ((Uint32)0xFFFFFFFFu) /* 4294967295 */
#define SDL_MIN_UINT32 ((Uint32)0x00000000) /* 0 */
typedef uint32_t Uint32;

// A signed 64-bit integer type.
#define SDL_MAX_SINT64 ((Sint64)0x7FFFFFFFFFFFFFFFll) /* 9223372036854775807 */
#define SDL_MIN_SINT64 ((Sint64)(~0x7FFFFFFFFFFFFFFFll)) /* -9223372036854775808 */
typedef int64_t Sint64;
// An unsigned 64-bit integer type.
#define SDL_MAX_UINT64 ((Uint64)0xFFFFFFFFFFFFFFFFull) /* 18446744073709551615 */
#define SDL_MIN_UINT64 ((Uint64)(0x0000000000000000ull)) /* 0 */
typedef uint64_t Uint64;

// Make sure we have macros for printing width-based integers.
// <stdint.h> should define these but this is not true all platforms.
// (for example win32)
#ifndef SDL_PRIs64
#ifdef PRIs64
#define SDL_PRIs64 PRIs64
#elif defined(__WIN32__)
#define SDL_PRIs64 "I64d"
#elif defined(__LINUX__) && defined(__LP64__)
#define SDL_PRIs64 "ld"
#else
#define SDL_PRIs64 "lld"
#endif
#endif
#ifndef SDL_PRIu64
#ifdef PRIu64
#define SDL_PRIu64 PRIu64
#elif defined(__WIN32__)
#define SDL_PRIu64 "I64u"
#elif defined(__LINUX__) && defined(__LP64__)
#define SDL_PRIu64 "lu"
#else
#define SDL_PRIu64 "llu"
#endif
#endif
#ifndef SDL_PRIx64
#ifdef PRIx64
#define SDL_PRIx64 PRIx64
#elif defined(__WIN32__)
#define SDL_PRIx64 "I64x"
#elif defined(__LINUX__) && defined(__LP64__)
#define SDL_PRIx64 "lx"
#else
#define SDL_PRIx64 "llx"
#endif
#endif
#ifndef SDL_PRIX64
#ifdef PRIX64
#define SDL_PRIX64 PRIX64
#elif defined(__WIN32__)
#define SDL_PRIX64 "I64X"
#elif defined(__LINUX__) && defined(__LP64__)
#define SDL_PRIX64 "lX"
#else
#define SDL_PRIX64 "llX"
#endif
#endif
#ifndef SDL_PRIs32
#ifdef PRId32
#define SDL_PRIs32 PRId32
#else
#define SDL_PRIs32 "d"
#endif
#endif
#ifndef SDL_PRIu32
#ifdef PRIu32
#define SDL_PRIu32 PRIu32
#else
#define SDL_PRIu32 "u"
#endif
#endif
#ifndef SDL_PRIx32
#ifdef PRIx32
#define SDL_PRIx32 PRIx32
#else
#define SDL_PRIx32 "x"
#endif
#endif
#ifndef SDL_PRIX32
#ifdef PRIX32
#define SDL_PRIX32 PRIX32
#else
#define SDL_PRIX32 "X"
#endif
#endif

// Annotations to help code analysis tools
#ifdef SDL_DISABLE_ANALYZE_MACROS
#define SDL_IN_BYTECAP(x)
#define SDL_INOUT_Z_CAP(x)
#define SDL_OUT_Z_CAP(x)
#define SDL_OUT_CAP(x)
#define SDL_OUT_BYTECAP(x)
#define SDL_OUT_Z_BYTECAP(x)
#define SDL_PRINTF_FORMAT_STRING
#define SDL_SCANF_FORMAT_STRING
#define SDL_PRINTF_VARARG_FUNC(fmtargnumber)
#define SDL_SCANF_VARARG_FUNC(fmtargnumber)
#else
#if defined(_MSC_VER) && (_MSC_VER >= 1600) /* VS 2010 and above */
#include <sal.h>

#define SDL_IN_BYTECAP(x) _In_bytecount_(x)
#define SDL_INOUT_Z_CAP(x) _Inout_z_cap_(x)
#define SDL_OUT_Z_CAP(x) _Out_z_cap_(x)
#define SDL_OUT_CAP(x) _Out_cap_(x)
#define SDL_OUT_BYTECAP(x) _Out_bytecap_(x)
#define SDL_OUT_Z_BYTECAP(x) _Out_z_bytecap_(x)

#define SDL_PRINTF_FORMAT_STRING _Printf_format_string_
#define SDL_SCANF_FORMAT_STRING _Scanf_format_string_impl_
#else
#define SDL_IN_BYTECAP(x)
#define SDL_INOUT_Z_CAP(x)
#define SDL_OUT_Z_CAP(x)
#define SDL_OUT_CAP(x)
#define SDL_OUT_BYTECAP(x)
#define SDL_OUT_Z_BYTECAP(x)
#define SDL_PRINTF_FORMAT_STRING
#define SDL_SCANF_FORMAT_STRING
#endif
#if defined(__GNUC__)
#define SDL_PRINTF_VARARG_FUNC(fmtargnumber) __attribute__((format(__printf__, fmtargnumber, fmtargnumber + 1)))
#define SDL_SCANF_VARARG_FUNC(fmtargnumber) __attribute__((format(__scanf__, fmtargnumber, fmtargnumber + 1)))
#else
#define SDL_PRINTF_VARARG_FUNC(fmtargnumber)
#define SDL_SCANF_VARARG_FUNC(fmtargnumber)
#endif
#endif // SDL_DISABLE_ANALYZE_MACROS

#define SDL_COMPILE_TIME_ASSERT(name, x) typedef int SDL_compile_time_assert_##name[(x)*2 - 1]

SDL_COMPILE_TIME_ASSERT(uint8, sizeof(Uint8) == 1);
SDL_COMPILE_TIME_ASSERT(sint8, sizeof(Sint8) == 1);
SDL_COMPILE_TIME_ASSERT(uint16, sizeof(Uint16) == 2);
SDL_COMPILE_TIME_ASSERT(sint16, sizeof(Sint16) == 2);
SDL_COMPILE_TIME_ASSERT(uint32, sizeof(Uint32) == 4);
SDL_COMPILE_TIME_ASSERT(sint32, sizeof(Sint32) == 4);
SDL_COMPILE_TIME_ASSERT(uint64, sizeof(Uint64) == 8);
SDL_COMPILE_TIME_ASSERT(sint64, sizeof(Sint64) == 8);

// Check to make sure enums are the size of ints, for structure packing.
// For both Watcom C/C++ and Borland C/C++ the compiler option that makes
// enums having the size of an int must be enabled.
// This is "-b" for Borland C/C++ and "-ei" for Watcom C/C++ (v11).

#if !defined(__ANDROID__) && !defined(__VITA__)
// TODO: include/_stdinc.h:174: error: size of array '_DUMMY_ENUM' is negative
typedef enum {
	DUMMY_ENUM_VALUE
} SDL_DUMMY_ENUM;

SDL_COMPILE_TIME_ASSERT(enum, sizeof(SDL_DUMMY_ENUM) == sizeof(int));
#endif

#include "_begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

extern DECLSPEC void *SDLCALL SDL_malloc(size_t size);
extern DECLSPEC void *SDLCALL SDL_calloc(size_t nmemb, size_t size);
extern DECLSPEC void *SDLCALL SDL_realloc(void *mem, size_t size);
extern DECLSPEC void SDLCALL SDL_free(void *mem);

extern DECLSPEC const char *SDLCALL SDL_getenv(const char *name);

extern DECLSPEC void SDLCALL SDL_qsort(void *base, size_t nmemb, size_t size, int (*compare)(const void *, const void *));

// NOTE: these double-evaluate their arguments, so you should never have side effects in the parameters
#define SDL_min(x, y) (((x) < (y)) ? (x) : (y))
#define SDL_max(x, y) (((x) > (y)) ? (x) : (y))
#define SDL_clamp(x, a, b) (((x) < (a)) ? (a) : (((x) > (b)) ? (b) : (x)))

extern DECLSPEC Uint32 SDLCALL SDL_crc32(Uint32 crc, const void *data, size_t len);

extern DECLSPEC void *SDLCALL SDL_memset(SDL_OUT_BYTECAP(len) void *dst, int c, size_t len);

#define SDL_zero(x) SDL_memset(&(x), 0, sizeof((x)))
#define SDL_zerop(x) SDL_memset((x), 0, sizeof(*(x)))
#define SDL_zeroa(x) SDL_memset((x), 0, sizeof((x)))

// NOTE: that memset() is a byte assignment and this is a 32-bit assignment, so they're not directly equivalent.
SDL_FORCE_INLINE void SDL_memset4(void *dst, Uint32 val, size_t dwords) {
#ifdef __APPLE__
	memset_pattern4(dst, &val, dwords * 4);
#elif defined(__GNUC__) && defined(__i386__)
	int u0, u1, u2;
	__asm__ __volatile__(
			"cld \n\t"
			"rep ; stosl \n\t"
			: "=&D"(u0), "=&a"(u1), "=&c"(u2)
			: "0"(dst), "1"(val), "2"(SDL_static_cast(Uint32, dwords))
			: "memory");
#else
	size_t _n = (dwords + 3) / 4;
	Uint32 *_p = SDL_static_cast(Uint32 *, dst);
	Uint32 _val = (val);
	if (dwords == 0) {
		return;
	}
	switch (dwords % 4) {
		case 0:
			do {
				*_p++ = _val;
				SDL_FALLTHROUGH;
				case 3:
					*_p++ = _val;
					SDL_FALLTHROUGH;
				case 2:
					*_p++ = _val;
					SDL_FALLTHROUGH;
				case 1:
					*_p++ = _val;
			} while (--_n);
	}
#endif
}

extern DECLSPEC void *SDLCALL SDL_memcpy(SDL_OUT_BYTECAP(len) void *dst, SDL_IN_BYTECAP(len) const void *src, size_t len);
extern DECLSPEC void *SDLCALL SDL_memmove(SDL_OUT_BYTECAP(len) void *dst, SDL_IN_BYTECAP(len) const void *src, size_t len);
extern DECLSPEC int SDLCALL SDL_memcmp(const void *s1, const void *s2, size_t len);

extern DECLSPEC int SDLCALL SDL_sscanf(const char *text, SDL_SCANF_FORMAT_STRING const char *fmt, ...) SDL_SCANF_VARARG_FUNC(2);
extern DECLSPEC int SDLCALL SDL_vsnprintf(SDL_OUT_Z_CAP(maxlen) char *text, size_t maxlen, const char *fmt, va_list ap);

#ifndef HAVE_M_PI
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif
#endif

// Use this function to compute arc cosine of `x`.
//
// The definition of `y = acos(x)` is `x = cos(y)`.
//
// Domain: `-1 <= x <= 1`
// Range: `0 <= y <= Pi`
extern DECLSPEC double SDLCALL SDL_acos(double x);
extern DECLSPEC float SDLCALL SDL_acosf(float x);
extern DECLSPEC double SDLCALL SDL_asin(double x);
extern DECLSPEC float SDLCALL SDL_asinf(float x);
extern DECLSPEC double SDLCALL SDL_atan(double x);
extern DECLSPEC float SDLCALL SDL_atanf(float x);
extern DECLSPEC double SDLCALL SDL_atan2(double y, double x);
extern DECLSPEC float SDLCALL SDL_atan2f(float y, float x);
extern DECLSPEC double SDLCALL SDL_ceil(double x);
extern DECLSPEC float SDLCALL SDL_ceilf(float x);
extern DECLSPEC double SDLCALL SDL_cos(double x);
extern DECLSPEC float SDLCALL SDL_cosf(float x);
extern DECLSPEC double SDLCALL SDL_exp(double x);
extern DECLSPEC float SDLCALL SDL_expf(float x);
extern DECLSPEC double SDLCALL SDL_fabs(double x);
extern DECLSPEC float SDLCALL SDL_fabsf(float x);
extern DECLSPEC int SDLCALL SDL_abs(int x);
extern DECLSPEC double SDLCALL SDL_floor(double x);
extern DECLSPEC float SDLCALL SDL_floorf(float x);
extern DECLSPEC double SDLCALL SDL_fmod(double x, double y);
extern DECLSPEC float SDLCALL SDL_fmodf(float x, float y);
extern DECLSPEC double SDLCALL SDL_log(double x);
extern DECLSPEC float SDLCALL SDL_logf(float x);
extern DECLSPEC double SDLCALL SDL_log10(double x);
extern DECLSPEC float SDLCALL SDL_log10f(float x);
extern DECLSPEC double SDLCALL SDL_pow(double x, double y);
extern DECLSPEC float SDLCALL SDL_powf(float x, float y);
extern DECLSPEC double SDLCALL SDL_round(double x);
extern DECLSPEC float SDLCALL SDL_roundf(float x);
extern DECLSPEC double SDLCALL SDL_sin(double x);
extern DECLSPEC float SDLCALL SDL_sinf(float x);
extern DECLSPEC double SDLCALL SDL_sqrt(double x);
extern DECLSPEC float SDLCALL SDL_sqrtf(float x);
extern DECLSPEC double SDLCALL SDL_tan(double x);
extern DECLSPEC float SDLCALL SDL_tanf(float x);

SDL_FORCE_INLINE void *SDL_memcpy4(SDL_OUT_BYTECAP(dwords * 4) void *dst, SDL_IN_BYTECAP(dwords * 4) const void *src, size_t dwords) {
	return SDL_memcpy(dst, src, dwords * 4);
}

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "_close_code.h"

#endif /* _stdinc_h_ */

/* vi: set ts=4 sw=4 expandtab: */
