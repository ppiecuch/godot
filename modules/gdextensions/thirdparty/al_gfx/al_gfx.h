#ifndef AL_GFX_H
#define AL_GFX_H

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Config
 */
#define ALLEGRO_HAVE_STDINT_H 1

/* which color depths to include? */
#define ALLEGRO_COLOR8
#define ALLEGRO_COLOR16
#define ALLEGRO_COLOR24
#define ALLEGRO_COLOR32

#if UINTPTR_MAX == 0xffffffff
#define ALLEGRO_PTR32 1
#elif UINTPTR_MAX == 0xffffffffffffffffu
#define ALLEGRO_PTR64 1
#else
#if _M_X64 || _M_AMD64
#define ALLEGRO_PTR64 1
#elif _WIN32 || _WIN64
#if _WIN64
#define ALLEGRO_PTR64 1
#else
#define ALLEGRO_PTR32 1
#endif
#elif __GNUC__
#if __x86_64__ || __ppc64__ || __LP64__ || _LP64 || __amd64__ || __amd64 || __x86_64__ || __x86_64 || __aarch64__
#define ALLEGRO_PTR64 1
#else
#define ALLEGRO_PTR32 1
#endif
#endif
#endif

#if defined ALLEGRO_HAVE_INTTYPES_H
#include <inttypes.h>
#elif defined ALLEGRO_HAVE_STDINT_H
#include <stdint.h>
#else
#define int8_t signed char
#define uint8_t unsigned char
#define int16_t signed short
#define uint16_t unsigned short
#define int32_t signed int
#define uint32_t unsigned int
#if ALLEGRO_PTR32
#define intptr_t int32_t
#define uintptr_t uint32_t
#elif ALLEGRO_PTR64
#define intptr_t int64_t
#define uintptr_t uint64_t
#else
#error Failed to detect pointer size.
#endif
#endif

/* special definitions for the GCC compiler */
#ifdef __GNUC__
#define ALLEGRO_GCC

#ifndef AL_INLINE
#ifdef __cplusplus
#define AL_INLINE(type, name, args, code) \
	static inline type name args;         \
	static inline type name args code
/* Needed if this header is included by C99 user code, as
 * "extern __inline__" is defined differently in C99 (it exports
 * a new global function symbol).
 */
#elif __GNUC_STDC_INLINE__
#define AL_INLINE(type, name, args, code)                             \
	extern __inline__ __attribute__((__gnu_inline__)) type name args; \
	extern __inline__ __attribute__((__gnu_inline__)) type name args code
#else
#define AL_INLINE(type, name, args, code) \
	extern __inline__ type name args;     \
	extern __inline__ type name args code
#endif
#endif

#define AL_PRINTFUNC(type, name, args, a, b) AL_FUNC(type, name, args) __attribute__((format(printf, a, b)))

#ifndef INLINE
#define INLINE __inline__
#endif

#if __GNUC__ >= 3
/* SET: According to gcc volatile is ignored for a return type.
 * I think the code should just ensure that inportb is declared as an
 * __asm__ __volatile__ macro. If that's the case the extra volatile
 * doesn't have any sense.
 */
#define RET_VOLATILE
#else
#define RET_VOLATILE volatile
#endif

#ifndef ZERO_SIZE_ARRAY
#if __GNUC__ < 3
#define ZERO_SIZE_ARRAY(type, name) __extension__ type name[0]
#else
#define ZERO_SIZE_ARRAY(type, name) type name[] /* ISO C99 flexible array members */
#endif
#endif

#ifndef LONG_LONG
#define LONG_LONG long long
#ifdef ALLEGRO_GUESS_INTTYPES_OK
#define int64_t signed long long
#define uint64_t unsigned long long
#endif
#endif

#ifdef __i386__
#define ALLEGRO_I386
#ifndef ALLEGRO_NO_ASM
#define _AL_SINCOS(x, s, c) __asm__("fsincos" : "=t"(c), "=u"(s) : "0"(x))
#endif
#endif

#ifdef __amd64__
#define ALLEGRO_AMD64
#ifndef ALLEGRO_NO_ASM
#define _AL_SINCOS(x, s, c) __asm__("fsincos" : "=t"(c), "=u"(s) : "0"(x))
#endif
#endif

#ifdef __arm__
#define ALLEGRO_ARM
#endif

#ifndef AL_CONST
#define AL_CONST const
#endif

#ifndef AL_FUNC_DEPRECATED
#if (__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1))
#define AL_FUNC_DEPRECATED(type, name, args) AL_FUNC(__attribute__((deprecated)) type, name, args)
#define AL_PRINTFUNC_DEPRECATED(type, name, args, a, b) AL_PRINTFUNC(__attribute__((deprecated)) type, name, args, a, b)
#define AL_INLINE_DEPRECATED(type, name, args, code) AL_INLINE(__attribute__((deprecated)) type, name, args, code)
#endif
#endif

#ifndef AL_ALIAS
#define AL_ALIAS(DECL, CALL)                         \
	static __attribute__((unused)) __inline__ DECL { \
		return CALL;                                 \
	}
#endif

#ifndef AL_ALIAS_VOID_RET
#define AL_ALIAS_VOID_RET(DECL, CALL)                     \
	static __attribute__((unused)) __inline__ void DECL { \
		CALL;                                             \
	}
#endif
#endif

/* use constructor functions, if supported */
#ifdef ALLEGRO_USE_CONSTRUCTOR
#define CONSTRUCTOR_FUNCTION(func) func __attribute__((constructor))
#define DESTRUCTOR_FUNCTION(func) func __attribute__((destructor))
#endif

/* the rest of this file fills in some default definitions of language
 * features and helper functions, which are conditionalised so they will
 * only be included if none of the above headers defined custom versions.
 */

#ifndef _AL_SINCOS
#define _AL_SINCOS(x, s, c) \
	do {                    \
		(c) = cos(x);       \
		(s) = sin(x);       \
	} while (0)
#endif

#ifndef INLINE
#define INLINE
#endif

#ifndef RET_VOLATILE
#define RET_VOLATILE volatile
#endif

#ifndef ZERO_SIZE_ARRAY
#define ZERO_SIZE_ARRAY(type, name) type name[]
#endif

#ifndef AL_CONST
#define AL_CONST
#endif

#ifndef AL_VAR
#define AL_VAR(type, name) extern type name
#endif

#ifndef AL_ARRAY
#define AL_ARRAY(type, name) extern type name[]
#endif

#ifndef AL_FUNC
#define AL_FUNC(type, name, args) type name args
#endif

#ifndef AL_PRINTFUNC
#define AL_PRINTFUNC(type, name, args, a, b) AL_FUNC(type, name, args)
#endif

#ifndef AL_METHOD
#define AL_METHOD(type, name, args) type(*name) args
#endif

#ifndef AL_FUNCPTR
#define AL_FUNCPTR(type, name, args) extern type(*name) args
#endif

#ifndef AL_FUNCPTRARRAY
#define AL_FUNCPTRARRAY(type, name, args) extern type(*name[]) args
#endif

#ifndef AL_INLINE
#define AL_INLINE(type, name, args, code) type name args;
#endif

#ifndef AL_FUNC_DEPRECATED
#define AL_FUNC_DEPRECATED(type, name, args) AL_FUNC(type, name, args)
#define AL_PRINTFUNC_DEPRECATED(type, name, args, a, b) AL_PRINTFUNC(type, name, args, a, b)
#define AL_INLINE_DEPRECATED(type, name, args, code) AL_INLINE(type, name, args, code)
#endif

#ifndef AL_ALIAS
#define AL_ALIAS(DECL, CALL) \
	static INLINE DECL {     \
		return CALL;         \
	}
#endif

#ifndef AL_ALIAS_VOID_RET
#define AL_ALIAS_VOID_RET(DECL, CALL) \
	static INLINE void DECL {         \
		CALL;                         \
	}
#endif

#ifndef END_OF_MAIN
#define END_OF_MAIN()
#endif

/* fill in default memory locking macros */
#ifndef END_OF_FUNCTION
#define END_OF_FUNCTION(x)
#define END_OF_STATIC_FUNCTION(x)
#define LOCK_DATA(d, s)
#define LOCK_CODE(c, s)
#define UNLOCK_DATA(d, s)
#define LOCK_VARIABLE(x)
#define LOCK_FUNCTION(x)
#endif

/* fill in default filename behaviour */
#ifndef ALLEGRO_LFN
#define ALLEGRO_LFN 1
#endif

#if (defined ALLEGRO_DOS) || (defined ALLEGRO_WINDOWS)
#define OTHER_PATH_SEPARATOR '\\'
#define DEVICE_SEPARATOR ':'
#else
#define OTHER_PATH_SEPARATOR '/'
#define DEVICE_SEPARATOR '\0'
#endif

/* emulate the FA_* flags for platforms that don't already have them */
#ifndef FA_RDONLY
#define FA_RDONLY 1
#define FA_HIDDEN 2
#define FA_SYSTEM 4
#define FA_LABEL 8
#define FA_DIREC 16
#define FA_ARCH 32
#endif
#define FA_NONE 0
#define FA_ALL (~FA_NONE)

/* emulate missing library functions */
#ifdef ALLEGRO_NO_STRICMP
AL_FUNC(int, _alemu_stricmp, (AL_CONST char *s1, AL_CONST char *s2));
#define stricmp _alemu_stricmp
#endif

#ifdef ALLEGRO_NO_STRLWR
AL_FUNC(char *, _alemu_strlwr, (char *string));
#define strlwr _alemu_strlwr
#endif

#ifdef ALLEGRO_NO_STRUPR
AL_FUNC(char *, _alemu_strupr, (char *string));
#define strupr _alemu_strupr
#endif

#ifdef ALLEGRO_NO_MEMCMP
AL_FUNC(int, _alemu_memcmp, (AL_CONST void *s1, AL_CONST void *s2, size_t num));
#define memcmp _alemu_memcmp
#endif

/* if nobody put them elsewhere, video bitmaps go in regular memory */
#ifndef _video_ds
#define _video_ds() _default_ds()
#endif

/* detect endiannes */
/* This catches all modern GCCs (>= 4.6) and Clang (>=3.2) */
#if (defined __BYTE_ORDER__) && (defined __ORDER_LITTLE_ENDIAN__)
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define ALLEGRO_LITTLE_ENDIAN 1
#define ALLEGRO_BIG_ENDIAN 0
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define ALLEGRO_LITTLE_ENDIAN 0
#define ALLEGRO_BIG_ENDIAN 1
#endif
/* Try to derive from arch/compiler-specific macros */
#elif defined(_X86_) || defined(__x86_64__) || defined(__i386__) ||    \
		defined(__i486__) || defined(__i586__) || defined(__i686__) || \
		defined(__MIPSEL) || defined(_MIPSEL) || defined(MIPSEL) ||    \
		defined(__ARMEL__) ||                                          \
		defined(__MSP430__) ||                                         \
		(defined(__LITTLE_ENDIAN__) && __LITTLE_ENDIAN__ == 1) ||      \
		(defined(_LITTLE_ENDIAN) && _LITTLE_ENDIAN == 1) ||            \
		defined(_M_ARM) || defined(_M_ARM64) ||                        \
		defined(_M_IX86) || defined(_M_AMD64) /* MSVC */
#define ALLEGRO_LITTLE_ENDIAN 1
#define ALLEGRO_BIG_ENDIAN 0
#elif defined(__MIPSEB) || defined(_MIPSEB) || defined(MIPSEB) || \
		defined(__MICROBLAZEEB__) || defined(__ARMEB__) ||        \
		(defined(__BIG_ENDIAN__) && __BIG_ENDIAN__ == 1) ||       \
		(defined(_BIG_ENDIAN) && _BIG_ENDIAN == 1)
#define ALLEGRO_LITTLE_ENDIAN 0
#define ALLEGRO_BIG_ENDIAN 1
#else /* Try to get it from a header */
#if defined(__linux) || defined(__HAIKU__)
#include <endian.h>
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
#include <sys/endian.h>
#elif defined(__APPLE__)
#include <machine/endian.h>
#endif
#endif

#ifndef ALLEGRO_LITTLE_ENDIAN
#undef ALLEGRO_BIG_ENDIAN
#if defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN)
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define ALLEGRO_LITTLE_ENDIAN 1
#define ALLEGRO_BIG_ENDIAN 0
#elif __BYTE_ORDER == __BIG_ENDIAN
#define ALLEGRO_LITTLE_ENDIAN 0
#define ALLEGRO_BIG_ENDIAN 1
#endif
#elif defined(BYTE_ORDER) && defined(LITTLE_ENDIAN)
#if BYTE_ORDER == LITTLE_ENDIAN
#define ALLEGRO_LITTLE_ENDIAN 1
#define ALLEGRO_BIG_ENDIAN 0
#elif BYTE_ORDER == BIG_ENDIAN
#define ALLEGRO_LITTLE_ENDIAN 0
#define ALLEGRO_BIG_ENDIAN 1
#endif
#endif
#endif

/* In case the user passed one of -DALLEGRO_LITTLE_ENDIAN or BIG_ENDIAN in CPPFLAS, set the second one too */
#if defined(ALLEGRO_LITTLE_ENDIAN) && !(defined(ALLEGRO_BIG_ENDIAN))
#if ALLEGRO_LITTLE_ENDIAN == 0
#define ALLEGRO_BIG_ENDIAN 1
#else
#define ALLEGRO_BIG_ENDIAN 0
#endif
#elif defined(ALLEGRO_BIG_ENDIAN) && !(defined(ALLEGRO_LITTLE_ENDIAN))
#if ALLEGRO_BIG_ENDIAN == 0
#define ALLEGRO_LITTLE_ENDIAN 1
#else
#define ALLEGRO_LITTLE_ENDIAN 0
#endif
#endif

/* endian-independent 3-byte accessor macros */
#ifdef ALLEGRO_LITTLE_ENDIAN

#define READ3BYTES(p) ((*(unsigned char *)(p)) | (*((unsigned char *)(p) + 1) << 8) | (*((unsigned char *)(p) + 2) << 16))
#define WRITE3BYTES(p, c) ((*(unsigned char *)(p) = (c)), (*((unsigned char *)(p) + 1) = (c) >> 8), (*((unsigned char *)(p) + 2) = (c) >> 16))

#elif defined ALLEGRO_BIG_ENDIAN

#define READ3BYTES(p) ((*(unsigned char *)(p) << 16) | (*((unsigned char *)(p) + 1) << 8) | (*((unsigned char *)(p) + 2)))
#define WRITE3BYTES(p, c) ((*(unsigned char *)(p) = (c) >> 16), (*((unsigned char *)(p) + 1) = (c) >> 8), (*((unsigned char *)(p) + 2) = (c)))

#elif defined SCAN_DEPEND

#define READ3BYTES(p)
#define WRITE3BYTES(p, c)

#else
#error al-gfx: endianess not defined
#endif

#ifndef bmp_write8
#define bmp_write8(addr, c) (*((uint8_t *)(addr)) = (c))
#define bmp_write15(addr, c) (*((uint16_t *)(addr)) = (c))
#define bmp_write16(addr, c) (*((uint16_t *)(addr)) = (c))
#define bmp_write32(addr, c) (*((uint32_t *)(addr)) = (c))

#define bmp_read8(addr) (*((uint8_t *)(addr)))
#define bmp_read15(addr) (*((uint16_t *)(addr)))
#define bmp_read16(addr) (*((uint16_t *)(addr)))
#define bmp_read32(addr) (*((uint32_t *)(addr)))

AL_INLINE(int, bmp_read24, (uintptr_t addr), {
	unsigned char *p = (unsigned char *)addr;
	int c = READ3BYTES(p);
	return c;
})

AL_INLINE(void, bmp_write24, (uintptr_t addr, int c), {
	unsigned char *p = (unsigned char *)addr;
	WRITE3BYTES(p, c);
})

#endif

/* default random function definition */
#ifndef AL_RAND
#define AL_RAND() (rand())
#endif

/* parameters for the color conversion code */
#if (defined ALLEGRO_WINDOWS) || (defined ALLEGRO_QNX)
#define ALLEGRO_COLORCONV_ALIGNED_WIDTH
#define ALLEGRO_NO_COLORCOPY
#endif

/*
 *  Base
 */

#define ALLEGRO_VERSION 4
#define ALLEGRO_SUB_VERSION 4
#define ALLEGRO_WIP_VERSION 3
#define ALLEGRO_VERSION_STR "4.4.3"
#define ALLEGRO_DATE_STR "2019"
#define ALLEGRO_DATE 20190303 /* yyyymmdd */

/** Globals **/

#ifndef TRUE
#define TRUE -1
#define FALSE 0
#endif

#undef MIN
#undef MAX
#undef MID

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

/* Returns the median of x, y, z */
#define MID(x, y, z) ((x) > (y) ? ((y) > (z) ? (y) : ((x) > (z) ? (z) : (x))) : ((y) > (z) ? ((z) > (x) ? (z) : (x)) : (y)))

/* Optimized version of MID for when x <= z. */
#define CLAMP(x, y, z) MAX((x), MIN((y), (z)))

#undef ABS
#define ABS(x) (((x) >= 0) ? (x) : (-(x)))

#undef SGN
#define SGN(x) (((x) >= 0) ? 1 : -1)

#define AL_PI 3.14159265358979323846

#define AL_ID(a, b, c, d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

AL_VAR(int *, allegro_errno);

/*
 * File I/O.
 */

#define F_READ "r"
#define F_WRITE "w"
#define F_READ_PACKED "rp"
#define F_WRITE_PACKED "wp"
#define F_WRITE_NOPACK "w!"

struct RGB;
struct AL_FILE;

AL_FUNC(struct AL_FILE *, al_io_fopen, (AL_CONST char *filename, AL_CONST char *mode));
AL_FUNC(int, al_io_fclose, (struct AL_FILE * f));
AL_FUNC(int, al_io_fseek, (struct AL_FILE * f, int offset));
AL_FUNC(int, al_io_feof, (struct AL_FILE * f));
AL_FUNC(int, al_io_getc, (struct AL_FILE * f));
AL_FUNC(int, al_io_putc, (int c, struct AL_FILE *f));
AL_FUNC(int, al_io_igetw, (struct AL_FILE * f));
AL_FUNC(int, al_io_iputw, (int w, struct AL_FILE *f));
AL_FUNC(long, al_io_fread, (void *p, long n, struct AL_FILE *f));
AL_FUNC(long, al_io_fwrite, (AL_CONST void *p, long n, struct AL_FILE *f));

/*
 * Datafile access routines.
 */

AL_FUNC(struct BITMAP *, load_tga, (AL_CONST char *filename, struct RGB *pal));
AL_FUNC(struct BITMAP *, load_tga_pf, (struct AL_FILE * f, struct RGB *pal));
AL_FUNC(int, save_tga, (AL_CONST char *filename, struct BITMAP *bmp, AL_CONST struct RGB *pal));
AL_FUNC(int, save_tga_pf, (struct AL_FILE * f, struct BITMAP *bmp, AL_CONST struct RGB *pal));

/*
 * Fixed math
 */

typedef int32_t fixed;

AL_VAR(AL_CONST fixed, fixtorad_r);
AL_VAR(AL_CONST fixed, radtofix_r);

AL_FUNC(fixed, fixsqrt, (fixed x));
AL_FUNC(fixed, fixhypot, (fixed x, fixed y));
AL_FUNC(fixed, fixatan, (fixed x));
AL_FUNC(fixed, fixatan2, (fixed y, fixed x));

AL_ARRAY(fixed, _cos_tbl);
AL_ARRAY(fixed, _tan_tbl);
AL_ARRAY(fixed, _acos_tbl);

/* ftofix and fixtof are used in generic C versions of fixmul and fixdiv */
AL_INLINE(fixed, ftofix, (double x), {
	if (x > 32767.0) {
		*allegro_errno = ERANGE;
		return 0x7FFFFFFF;
	}
	if (x < -32767.0) {
		*allegro_errno = ERANGE;
		return -0x7FFFFFFF;
	}
	return (fixed)(x * 65536.0 + (x < 0 ? -0.5 : 0.5));
})

AL_INLINE(double, fixtof, (fixed x), {
	return (double)x / 65536.0;
})

AL_INLINE(fixed, fixadd, (fixed x, fixed y), {
	fixed result = x + y;

	if (result >= 0) {
		if ((x < 0) && (y < 0)) {
			*allegro_errno = ERANGE;
			return -0x7FFFFFFF;
		} else
			return result;
	} else {
		if ((x > 0) && (y > 0)) {
			*allegro_errno = ERANGE;
			return 0x7FFFFFFF;
		} else
			return result;
	}
})

AL_INLINE(fixed, fixsub, (fixed x, fixed y), {
	fixed result = x - y;

	if (result >= 0) {
		if ((x < 0) && (y > 0)) {
			*allegro_errno = ERANGE;
			return -0x7FFFFFFF;
		} else
			return result;
	} else {
		if ((x > 0) && (y < 0)) {
			*allegro_errno = ERANGE;
			return 0x7FFFFFFF;
		} else
			return result;
	}
})

/* In benchmarks conducted circa May 2005 we found that, in the main:
 * - IA32 machines performed faster with one implementation;
 * - AMD64 and G4 machines performed faster with another implementation.
 *
 * Benchmarks were mainly done with differing versions of gcc.
 * Results varied with other compilers, optimisation levels, etc.
 * so this is not optimal, though a tenable compromise.
 *
 * Note that the following implementation are NOT what were benchmarked.
 * We had forgotten to put in overflow detection in those versions.
 * If you don't need overflow detection then previous versions in the
 * CVS tree might be worth looking at.
 */
#if (defined ALLEGRO_I386) || (!defined LONG_LONG)
AL_INLINE(fixed, fixmul, (fixed x, fixed y), {
	return ftofix(fixtof(x) * fixtof(y));
})
#else
AL_INLINE(fixed, fixmul, (fixed x, fixed y), {
	LONG_LONG lx = x;
	LONG_LONG ly = y;
	LONG_LONG lres = (lx * ly);

	if (lres > 0x7FFFFFFF0000LL) {
		*allegro_errno = ERANGE;
		return 0x7FFFFFFF;
	} else if (lres < -0x7FFFFFFF0000LL) {
		*allegro_errno = ERANGE;
		return 0x80000000;
	} else {
		int res = lres >> 16;
		return res;
	}
})
#endif /* fixmul() C implementations */

AL_INLINE(fixed, fixdiv, (fixed x, fixed y), {
	if (y == 0) {
		*allegro_errno = ERANGE;
		return (x < 0) ? -0x7FFFFFFF : 0x7FFFFFFF;
	} else
		return ftofix(fixtof(x) / fixtof(y));
})

AL_INLINE(int, fixfloor, (fixed x), {
	/* (x >> 16) is not portable */
	if (x >= 0)
		return (x >> 16);
	else
		return ~((~x) >> 16);
})

AL_INLINE(int, fixceil, (fixed x), {
	if (x > 0x7FFF0000) {
		*allegro_errno = ERANGE;
		return 0x7FFF;
	}

	return fixfloor(x + 0xFFFF);
})

AL_INLINE(fixed, itofix, (int x), {
	return x << 16;
})

AL_INLINE(int, fixtoi, (fixed x), {
	return fixfloor(x) + ((x & 0x8000) >> 15);
})

AL_INLINE(fixed, fixcos, (fixed x), {
	return _cos_tbl[((x + 0x4000) >> 15) & 0x1FF];
})

AL_INLINE(fixed, fixsin, (fixed x), {
	return _cos_tbl[((x - 0x400000 + 0x4000) >> 15) & 0x1FF];
})

AL_INLINE(fixed, fixtan, (fixed x), {
	return _tan_tbl[((x + 0x4000) >> 15) & 0xFF];
})

AL_INLINE(fixed, fixacos, (fixed x), {
	if ((x < -65536) || (x > 65536)) {
		*allegro_errno = EDOM;
		return 0;
	}

	return _acos_tbl[(x + 65536 + 127) >> 8];
})

AL_INLINE(fixed, fixasin, (fixed x), {
	if ((x < -65536) || (x > 65536)) {
		*allegro_errno = EDOM;
		return 0;
	}

	return 0x00400000 - _acos_tbl[(x + 65536 + 127) >> 8];
})

/**
 * Matrix
 */

typedef struct MATRIX /* transformation matrix (fixed point) */
{
	fixed v[3][3]; /* scaling and rotation */
	fixed t[3]; /* translation */
} MATRIX;

typedef struct MATRIX_f /* transformation matrix (floating point) */
{
	float v[3][3]; /* scaling and rotation */
	float t[3]; /* translation */
} MATRIX_f;

AL_VAR(MATRIX, identity_matrix);
AL_VAR(MATRIX_f, identity_matrix_f);

AL_FUNC(void, get_translation_matrix, (MATRIX * m, fixed x, fixed y, fixed z));
AL_FUNC(void, get_translation_matrix_f, (MATRIX_f * m, float x, float y, float z));

AL_FUNC(void, get_scaling_matrix, (MATRIX * m, fixed x, fixed y, fixed z));
AL_FUNC(void, get_scaling_matrix_f, (MATRIX_f * m, float x, float y, float z));

AL_FUNC(void, get_x_rotate_matrix, (MATRIX * m, fixed r));
AL_FUNC(void, get_x_rotate_matrix_f, (MATRIX_f * m, float r));

AL_FUNC(void, get_y_rotate_matrix, (MATRIX * m, fixed r));
AL_FUNC(void, get_y_rotate_matrix_f, (MATRIX_f * m, float r));

AL_FUNC(void, get_z_rotate_matrix, (MATRIX * m, fixed r));
AL_FUNC(void, get_z_rotate_matrix_f, (MATRIX_f * m, float r));

AL_FUNC(void, get_rotation_matrix, (MATRIX * m, fixed x, fixed y, fixed z));
AL_FUNC(void, get_rotation_matrix_f, (MATRIX_f * m, float x, float y, float z));

AL_FUNC(void, get_align_matrix, (MATRIX * m, fixed xfront, fixed yfront, fixed zfront, fixed xup, fixed yup, fixed zup));
AL_FUNC(void, get_align_matrix_f, (MATRIX_f * m, float xfront, float yfront, float zfront, float xup, float yup, float zup));

AL_FUNC(void, get_vector_rotation_matrix, (MATRIX * m, fixed x, fixed y, fixed z, fixed a));
AL_FUNC(void, get_vector_rotation_matrix_f, (MATRIX_f * m, float x, float y, float z, float a));

AL_FUNC(void, get_transformation_matrix, (MATRIX * m, fixed scale, fixed xrot, fixed yrot, fixed zrot, fixed x, fixed y, fixed z));
AL_FUNC(void, get_transformation_matrix_f, (MATRIX_f * m, float scale, float xrot, float yrot, float zrot, float x, float y, float z));

AL_FUNC(void, get_camera_matrix, (MATRIX * m, fixed x, fixed y, fixed z, fixed xfront, fixed yfront, fixed zfront, fixed xup, fixed yup, fixed zup, fixed fov, fixed aspect));
AL_FUNC(void, get_camera_matrix_f, (MATRIX_f * m, float x, float y, float z, float xfront, float yfront, float zfront, float xup, float yup, float zup, float fov, float aspect));

AL_FUNC(void, qtranslate_matrix, (MATRIX * m, fixed x, fixed y, fixed z));
AL_FUNC(void, qtranslate_matrix_f, (MATRIX_f * m, float x, float y, float z));

AL_FUNC(void, qscale_matrix, (MATRIX * m, fixed scale));
AL_FUNC(void, qscale_matrix_f, (MATRIX_f * m, float scale));

AL_FUNC(void, matrix_mul, (AL_CONST MATRIX * m1, AL_CONST MATRIX *m2, MATRIX *out));
AL_FUNC(void, matrix_mul_f, (AL_CONST MATRIX_f * m1, AL_CONST MATRIX_f *m2, MATRIX_f *out));

AL_FUNC(void, apply_matrix_f, (AL_CONST MATRIX_f * m, float x, float y, float z, float *xout, float *yout, float *zout));

#define CALC_ROW(n) (fixmul(x, m->v[n][0]) + \
		fixmul(y, m->v[n][1]) +              \
		fixmul(z, m->v[n][2]) +              \
		m->t[n])

AL_INLINE(void, apply_matrix, (MATRIX * m, fixed x, fixed y, fixed z, fixed *xout, fixed *yout, fixed *zout), {
	*xout = CALC_ROW(0);
	*yout = CALC_ROW(1);
	*zout = CALC_ROW(2);
})

#undef CALC_ROW

/**
 * 3dmath
 */

struct QUAT;
struct MATRIX_f;

AL_FUNC(fixed, vector_length, (fixed x, fixed y, fixed z));
AL_FUNC(float, vector_length_f, (float x, float y, float z));

AL_FUNC(void, normalize_vector, (fixed * x, fixed *y, fixed *z));
AL_FUNC(void, normalize_vector_f, (float *x, float *y, float *z));

AL_FUNC(void, cross_product, (fixed x1, fixed y_1, fixed z1, fixed x2, fixed y2, fixed z2, fixed *xout, fixed *yout, fixed *zout));
AL_FUNC(void, cross_product_f, (float x1, float y_1, float z1, float x2, float y2, float z2, float *xout, float *yout, float *zout));

AL_VAR(fixed, _persp_xscale);
AL_VAR(fixed, _persp_yscale);
AL_VAR(fixed, _persp_xoffset);
AL_VAR(fixed, _persp_yoffset);

AL_VAR(float, _persp_xscale_f);
AL_VAR(float, _persp_yscale_f);
AL_VAR(float, _persp_xoffset_f);
AL_VAR(float, _persp_yoffset_f);

AL_FUNC(void, set_projection_viewport, (int x, int y, int w, int h));

AL_FUNC(void, quat_to_matrix, (AL_CONST struct QUAT * q, struct MATRIX_f *m));
AL_FUNC(void, matrix_to_quat, (AL_CONST struct MATRIX_f * m, struct QUAT *q));

AL_INLINE(fixed, dot_product, (fixed x1, fixed y_1, fixed z1, fixed x2, fixed y2, fixed z2), {
	return fixmul(x1, x2) + fixmul(y_1, y2) + fixmul(z1, z2);
})

AL_INLINE(float, dot_product_f, (float x1, float y_1, float z1, float x2, float y2, float z2), {
	return (x1 * x2) + (y_1 * y2) + (z1 * z2);
})

AL_INLINE(void, persp_project, (fixed x, fixed y, fixed z, fixed *xout, fixed *yout), {
	*xout = fixmul(fixdiv(x, z), _persp_xscale) + _persp_xoffset;
	*yout = fixmul(fixdiv(y, z), _persp_yscale) + _persp_yoffset;
})

AL_INLINE(void, persp_project_f, (float x, float y, float z, float *xout, float *yout), {
	float z1 = 1.0f / z;
	*xout = ((x * z1) * _persp_xscale_f) + _persp_xoffset_f;
	*yout = ((y * z1) * _persp_yscale_f) + _persp_yoffset_f;
})

/**
 * Debug
 */

AL_FUNC(void, al_assert, (AL_CONST char *file, int linenr));
AL_PRINTFUNC(void, al_trace, (AL_CONST char *msg, ...), 1, 2);

AL_FUNC(void, register_assert_handler, (AL_METHOD(int, handler, (AL_CONST char *msg))));
AL_FUNC(void, register_trace_handler, (AL_METHOD(int, handler, (AL_CONST char *msg))));

#ifdef DEBUGMODE
#define ASSERT(condition)                  \
	{                                      \
		if (!(condition))                  \
			al_assert(__FILE__, __LINE__); \
	}
#define TRACE al_trace
#else
#define ASSERT(condition)
#define TRACE 1 ? (void)0 : al_trace
#endif

/*
 * 3d
 */

struct BITMAP;

typedef struct V3D /* a 3d point (fixed point version) */
{
	fixed x, y, z; /* position */
	fixed u, v; /* texture map coordinates */
	int c; /* color */
} V3D;

typedef struct V3D_f /* a 3d point (floating point version) */
{
	float x, y, z; /* position */
	float u, v; /* texture map coordinates */
	int c; /* color */
} V3D_f;

#define POLYTYPE_FLAT 0
#define POLYTYPE_GCOL 1
#define POLYTYPE_GRGB 2
#define POLYTYPE_ATEX 3
#define POLYTYPE_PTEX 4
#define POLYTYPE_ATEX_MASK 5
#define POLYTYPE_PTEX_MASK 6
#define POLYTYPE_ATEX_LIT 7
#define POLYTYPE_PTEX_LIT 8
#define POLYTYPE_ATEX_MASK_LIT 9
#define POLYTYPE_PTEX_MASK_LIT 10
#define POLYTYPE_ATEX_TRANS 11
#define POLYTYPE_PTEX_TRANS 12
#define POLYTYPE_ATEX_MASK_TRANS 13
#define POLYTYPE_PTEX_MASK_TRANS 14
#define POLYTYPE_MAX 15
#define POLYTYPE_ZBUF 16

AL_VAR(float, scene_gap);

AL_FUNC(void, _soft_polygon3d, (struct BITMAP * bmp, int type, struct BITMAP *texture, int vc, V3D *vtx[]));
AL_FUNC(void, _soft_polygon3d_f, (struct BITMAP * bmp, int type, struct BITMAP *texture, int vc, V3D_f *vtx[]));
AL_FUNC(void, _soft_triangle3d, (struct BITMAP * bmp, int type, struct BITMAP *texture, V3D *v1, V3D *v2, V3D *v3));
AL_FUNC(void, _soft_triangle3d_f, (struct BITMAP * bmp, int type, struct BITMAP *texture, V3D_f *v1, V3D_f *v2, V3D_f *v3));
AL_FUNC(void, _soft_quad3d, (struct BITMAP * bmp, int type, struct BITMAP *texture, V3D *v1, V3D *v2, V3D *v3, V3D *v4));
AL_FUNC(void, _soft_quad3d_f, (struct BITMAP * bmp, int type, struct BITMAP *texture, V3D_f *v1, V3D_f *v2, V3D_f *v3, V3D_f *v4));
AL_FUNC(int, clip3d, (int type, fixed min_z, fixed max_z, int vc, AL_CONST V3D *vtx[], V3D *vout[], V3D *vtmp[], int out[]));
AL_FUNC(int, clip3d_f, (int type, float min_z, float max_z, int vc, AL_CONST V3D_f *vtx[], V3D_f *vout[], V3D_f *vtmp[], int out[]));

AL_FUNC(fixed, polygon_z_normal, (AL_CONST V3D * v1, AL_CONST V3D *v2, AL_CONST V3D *v3));
AL_FUNC(float, polygon_z_normal_f, (AL_CONST V3D_f * v1, AL_CONST V3D_f *v2, AL_CONST V3D_f *v3));

/* Note: You are not supposed to mix ZBUFFER with BITMAP even though it is
 * currently possible. This is just the internal representation, and it may
 * change in the future.
 */
typedef struct BITMAP ZBUFFER;

AL_FUNC(ZBUFFER *, create_zbuffer, (struct BITMAP * bmp));
AL_FUNC(ZBUFFER *, create_sub_zbuffer, (ZBUFFER * parent, int x, int y, int width, int height));
AL_FUNC(void, set_zbuffer, (ZBUFFER * zbuf));
AL_FUNC(void, clear_zbuffer, (ZBUFFER * zbuf, float z));
AL_FUNC(void, destroy_zbuffer, (ZBUFFER * zbuf));

/*
 * Color and palette
 */

typedef struct RGB {
	unsigned char r, g, b;
	unsigned char filler;
} RGB;

#define PAL_SIZE 256

typedef RGB PALETTE[PAL_SIZE];

struct BITMAP;

AL_VAR(PALETTE, black_palette);
AL_VAR(PALETTE, desktop_palette);
AL_VAR(PALETTE, default_palette);

typedef struct {
	unsigned char data[32][32][32];
} RGB_MAP;

typedef struct {
	unsigned char data[PAL_SIZE][PAL_SIZE];
} COLOR_MAP;

AL_VAR(RGB_MAP *, rgb_map);
AL_VAR(COLOR_MAP *, color_map);

AL_VAR(PALETTE, _current_palette);

AL_VAR(int, _rgb_r_shift_15);
AL_VAR(int, _rgb_g_shift_15);
AL_VAR(int, _rgb_b_shift_15);
AL_VAR(int, _rgb_r_shift_16);
AL_VAR(int, _rgb_g_shift_16);
AL_VAR(int, _rgb_b_shift_16);
AL_VAR(int, _rgb_r_shift_24);
AL_VAR(int, _rgb_g_shift_24);
AL_VAR(int, _rgb_b_shift_24);
AL_VAR(int, _rgb_r_shift_32);
AL_VAR(int, _rgb_g_shift_32);
AL_VAR(int, _rgb_b_shift_32);
AL_VAR(int, _rgb_a_shift_32);

AL_ARRAY(int, _rgb_scale_5);
AL_ARRAY(int, _rgb_scale_6);

#define MASK_COLOR_8 0
#define MASK_COLOR_15 0x7C1F
#define MASK_COLOR_16 0xF81F
#define MASK_COLOR_24 0xFF00FF
#define MASK_COLOR_32 0xFF00FF

typedef AL_METHOD(unsigned long, PROGRESS_FUNC, ());

AL_VAR(int *, palette_color);

AL_FUNC(void, set_color, (int idx, AL_CONST RGB *p));
AL_FUNC(void, set_palette, (AL_CONST PALETTE p));
AL_FUNC(void, set_palette_range, (AL_CONST PALETTE p, int from, int to, int retracesync));

AL_FUNC(void, get_color, (int idx, RGB *p));
AL_FUNC(void, get_palette, (PALETTE p));
AL_FUNC(void, get_palette_range, (PALETTE p, int from, int to));

AL_FUNC(void, fade_interpolate, (AL_CONST PALETTE source, AL_CONST PALETTE dest, PALETTE output, int pos, int from, int to));
AL_FUNC(void, fade_from_range, (AL_CONST PALETTE source, AL_CONST PALETTE dest, int speed, int from, int to, PROGRESS_FUNC progress_source));
AL_FUNC(void, fade_in_range, (AL_CONST PALETTE p, int speed, int from, int to, PROGRESS_FUNC progress_source));
AL_FUNC(void, fade_out_range, (int speed, int from, int to, PROGRESS_FUNC progress_source));
AL_FUNC(void, fade_from, (AL_CONST PALETTE source, AL_CONST PALETTE dest, int speed, PROGRESS_FUNC progress_source));
AL_FUNC(void, fade_in, (AL_CONST PALETTE p, int speed, PROGRESS_FUNC progress_source));
AL_FUNC(void, fade_out, (int speed, PROGRESS_FUNC progress_source));

AL_FUNC(void, select_palette, (AL_CONST PALETTE p));
AL_FUNC(void, unselect_palette, (void));

AL_FUNC(void, generate_332_palette, (PALETTE pal));
AL_FUNC(int, generate_optimized_palette, (struct BITMAP * image, PALETTE pal, AL_CONST signed char rsvdcols[256]));

AL_FUNC(void, create_rgb_table, (RGB_MAP * table, AL_CONST PALETTE pal, AL_METHOD(void, callback, (int pos))));
AL_FUNC(void, create_light_table, (COLOR_MAP * table, AL_CONST PALETTE pal, int r, int g, int b, AL_METHOD(void, callback, (int pos))));
AL_FUNC(void, create_trans_table, (COLOR_MAP * table, AL_CONST PALETTE pal, int r, int g, int b, AL_METHOD(void, callback, (int pos))));
AL_FUNC(void, create_color_table, (COLOR_MAP * table, AL_CONST PALETTE pal, AL_METHOD(void, blend, (AL_CONST PALETTE pal, int x, int y, RGB *rgb)), AL_METHOD(void, callback, (int pos))));
AL_FUNC(void, create_blender_table, (COLOR_MAP * table, AL_CONST PALETTE pal, AL_METHOD(void, callback, (int pos))));

typedef AL_METHOD(unsigned long, BLENDER_FUNC, (unsigned long x, unsigned long y, unsigned long n));

AL_FUNC(void, set_blender_mode, (BLENDER_FUNC b15, BLENDER_FUNC b16, BLENDER_FUNC b24, int r, int g, int b, int a));
AL_FUNC(void, set_blender_mode_ex, (BLENDER_FUNC b15, BLENDER_FUNC b16, BLENDER_FUNC b24, BLENDER_FUNC b32, BLENDER_FUNC b15x, BLENDER_FUNC b16x, BLENDER_FUNC b24x, int r, int g, int b, int a));

AL_FUNC(void, set_alpha_blender, (void));
AL_FUNC(void, set_write_alpha_blender, (void));
AL_FUNC(void, set_trans_blender, (int r, int g, int b, int a));
AL_FUNC(void, set_add_blender, (int r, int g, int b, int a));
AL_FUNC(void, set_burn_blender, (int r, int g, int b, int a));
AL_FUNC(void, set_color_blender, (int r, int g, int b, int a));
AL_FUNC(void, set_difference_blender, (int r, int g, int b, int a));
AL_FUNC(void, set_dissolve_blender, (int r, int g, int b, int a));
AL_FUNC(void, set_dodge_blender, (int r, int g, int b, int a));
AL_FUNC(void, set_hue_blender, (int r, int g, int b, int a));
AL_FUNC(void, set_invert_blender, (int r, int g, int b, int a));
AL_FUNC(void, set_luminance_blender, (int r, int g, int b, int a));
AL_FUNC(void, set_multiply_blender, (int r, int g, int b, int a));
AL_FUNC(void, set_saturation_blender, (int r, int g, int b, int a));
AL_FUNC(void, set_screen_blender, (int r, int g, int b, int a));

AL_FUNC(void, hsv_to_rgb, (float h, float s, float v, int *r, int *g, int *b));
AL_FUNC(void, rgb_to_hsv, (int r, int g, int b, float *h, float *s, float *v));

AL_FUNC(int, bestfit_color, (AL_CONST PALETTE pal, int r, int g, int b));

AL_FUNC(int, makecol, (int r, int g, int b));
AL_FUNC(int, makecol8, (int r, int g, int b));
AL_FUNC(int, makecol_depth, (int color_depth, int r, int g, int b));

AL_FUNC(int, makeacol, (int r, int g, int b, int a));
AL_FUNC(int, makeacol_depth, (int color_depth, int r, int g, int b, int a));

AL_FUNC(int, makecol15_dither, (int r, int g, int b, int x, int y));
AL_FUNC(int, makecol16_dither, (int r, int g, int b, int x, int y));

AL_FUNC(int, getr, (int c));
AL_FUNC(int, getg, (int c));
AL_FUNC(int, getb, (int c));
AL_FUNC(int, geta, (int c));

AL_FUNC(int, getr_depth, (int color_depth, int c));
AL_FUNC(int, getg_depth, (int color_depth, int c));
AL_FUNC(int, getb_depth, (int color_depth, int c));
AL_FUNC(int, geta_depth, (int color_depth, int c));

AL_INLINE(int, makecol15, (int r, int g, int b), {
	return (((r >> 3) << _rgb_r_shift_15) |
			((g >> 3) << _rgb_g_shift_15) |
			((b >> 3) << _rgb_b_shift_15));
})

AL_INLINE(int, makecol16, (int r, int g, int b), {
	return (((r >> 3) << _rgb_r_shift_16) |
			((g >> 2) << _rgb_g_shift_16) |
			((b >> 3) << _rgb_b_shift_16));
})

AL_INLINE(int, makecol24, (int r, int g, int b), {
	return ((r << _rgb_r_shift_24) |
			(g << _rgb_g_shift_24) |
			(b << _rgb_b_shift_24));
})

AL_INLINE(int, makecol32, (int r, int g, int b), {
	return ((r << _rgb_r_shift_32) |
			(g << _rgb_g_shift_32) |
			(b << _rgb_b_shift_32));
})

AL_INLINE(int, makeacol32, (int r, int g, int b, int a), {
	return ((r << _rgb_r_shift_32) |
			(g << _rgb_g_shift_32) |
			(b << _rgb_b_shift_32) |
			(a << _rgb_a_shift_32));
})
AL_INLINE(int, getr8, (int c), { return _rgb_scale_6[(int)_current_palette[c].r]; })
AL_INLINE(int, getg8, (int c), { return _rgb_scale_6[(int)_current_palette[c].g]; })
AL_INLINE(int, getb8, (int c), { return _rgb_scale_6[(int)_current_palette[c].b]; })
AL_INLINE(int, getr15, (int c), { return _rgb_scale_5[(c >> _rgb_r_shift_15) & 0x1F]; })
AL_INLINE(int, getg15, (int c), { return _rgb_scale_5[(c >> _rgb_g_shift_15) & 0x1F]; })
AL_INLINE(int, getb15, (int c), { return _rgb_scale_5[(c >> _rgb_b_shift_15) & 0x1F]; })
AL_INLINE(int, getr16, (int c), { return _rgb_scale_5[(c >> _rgb_r_shift_16) & 0x1F]; })
AL_INLINE(int, getg16, (int c), { return _rgb_scale_6[(c >> _rgb_g_shift_16) & 0x3F]; })
AL_INLINE(int, getb16, (int c), { return _rgb_scale_5[(c >> _rgb_b_shift_16) & 0x1F]; })
AL_INLINE(int, getr24, (int c), { return ((c >> _rgb_r_shift_24) & 0xFF); })
AL_INLINE(int, getg24, (int c), { return ((c >> _rgb_g_shift_24) & 0xFF); })
AL_INLINE(int, getb24, (int c), { return ((c >> _rgb_b_shift_24) & 0xFF); })
AL_INLINE(int, getr32, (int c), { return ((c >> _rgb_r_shift_32) & 0xFF); })
AL_INLINE(int, getg32, (int c), { return ((c >> _rgb_g_shift_32) & 0xFF); })
AL_INLINE(int, getb32, (int c), { return ((c >> _rgb_b_shift_32) & 0xFF); })
AL_INLINE(int, geta32, (int c), { return ((c >> _rgb_a_shift_32) & 0xFF); })
AL_INLINE(void, _set_color, (int idx, AL_CONST RGB *p), { set_color(idx, p); })

/*
 * Gfx
 */

struct RLE_SPRITE;
struct FONT_GLYPH;
struct RGB;

#define GFX_TEXT -1
#define GFX_AUTODETECT 0
#define GFX_AUTODETECT_FULLSCREEN 1
#define GFX_AUTODETECT_WINDOWED 2
#define GFX_SAFE AL_ID('S', 'A', 'F', 'E')
#define GFX_NONE AL_ID('N', 'O', 'N', 'E')

/* drawing modes for draw_sprite_ex() */
#define DRAW_SPRITE_NORMAL 0
#define DRAW_SPRITE_LIT 1
#define DRAW_SPRITE_TRANS 2

/* flipping modes for draw_sprite_ex() */
#define DRAW_SPRITE_NO_FLIP 0x0
#define DRAW_SPRITE_H_FLIP 0x1
#define DRAW_SPRITE_V_FLIP 0x2
#define DRAW_SPRITE_VH_FLIP 0x3

/* Blender mode defines, for the gfx_driver->set_blender_mode() function */
#define blender_mode_none 0
#define blender_mode_trans 1
#define blender_mode_add 2
#define blender_mode_burn 3
#define blender_mode_color 4
#define blender_mode_difference 5
#define blender_mode_dissolve 6
#define blender_mode_dodge 7
#define blender_mode_hue 8
#define blender_mode_invert 9
#define blender_mode_luminance 10
#define blender_mode_multiply 11
#define blender_mode_saturation 12
#define blender_mode_screen 13
#define blender_mode_alpha 14

typedef struct GFX_VTABLE /* functions for drawing onto bitmaps */
{
	int color_depth;
	int mask_color;
	AL_METHOD(void, set_clip, (struct BITMAP * bmp));
	AL_METHOD(void, acquire, (struct BITMAP * bmp));
	AL_METHOD(void, release, (struct BITMAP * bmp));
	AL_METHOD(struct BITMAP *, create_sub_bitmap, (struct BITMAP * parent, int x, int y, int width, int height));
	AL_METHOD(void, created_sub_bitmap, (struct BITMAP * bmp, struct BITMAP *parent));
	AL_METHOD(int, getpixel, (struct BITMAP * bmp, int x, int y));
	AL_METHOD(void, putpixel, (struct BITMAP * bmp, int x, int y, int color));
	AL_METHOD(void, vline, (struct BITMAP * bmp, int x, int y_1, int y2, int color));
	AL_METHOD(void, hline, (struct BITMAP * bmp, int x1, int y, int x2, int color));
	AL_METHOD(void, hfill, (struct BITMAP * bmp, int x1, int y, int x2, int color));
	AL_METHOD(void, line, (struct BITMAP * bmp, int x1, int y_1, int x2, int y2, int color));
	AL_METHOD(void, fastline, (struct BITMAP * bmp, int x1, int y_1, int x2, int y2, int color));
	AL_METHOD(void, rectfill, (struct BITMAP * bmp, int x1, int y_1, int x2, int y2, int color));
	AL_METHOD(void, triangle, (struct BITMAP * bmp, int x1, int y_1, int x2, int y2, int x3, int y3, int color));
	AL_METHOD(void, draw_sprite, (struct BITMAP * bmp, struct BITMAP *sprite, int x, int y));
	AL_METHOD(void, draw_256_sprite, (struct BITMAP * bmp, struct BITMAP *sprite, int x, int y));
	AL_METHOD(void, draw_sprite_v_flip, (struct BITMAP * bmp, struct BITMAP *sprite, int x, int y));
	AL_METHOD(void, draw_sprite_h_flip, (struct BITMAP * bmp, struct BITMAP *sprite, int x, int y));
	AL_METHOD(void, draw_sprite_vh_flip, (struct BITMAP * bmp, struct BITMAP *sprite, int x, int y));
	AL_METHOD(void, draw_trans_sprite, (struct BITMAP * bmp, struct BITMAP *sprite, int x, int y));
	AL_METHOD(void, draw_trans_rgba_sprite, (struct BITMAP * bmp, struct BITMAP *sprite, int x, int y));
	AL_METHOD(void, draw_lit_sprite, (struct BITMAP * bmp, struct BITMAP *sprite, int x, int y, int color));
	AL_METHOD(void, draw_rle_sprite, (struct BITMAP * bmp, AL_CONST struct RLE_SPRITE *sprite, int x, int y));
	AL_METHOD(void, draw_trans_rle_sprite, (struct BITMAP * bmp, AL_CONST struct RLE_SPRITE *sprite, int x, int y));
	AL_METHOD(void, draw_trans_rgba_rle_sprite, (struct BITMAP * bmp, AL_CONST struct RLE_SPRITE *sprite, int x, int y));
	AL_METHOD(void, draw_lit_rle_sprite, (struct BITMAP * bmp, AL_CONST struct RLE_SPRITE *sprite, int x, int y, int color));
	AL_METHOD(void, draw_character, (struct BITMAP * bmp, struct BITMAP *sprite, int x, int y, int color, int bg));
	AL_METHOD(void, draw_glyph, (struct BITMAP * bmp, AL_CONST struct FONT_GLYPH *glyph, int x, int y, int color, int bg));
	AL_METHOD(void, blit_from_memory, (struct BITMAP * source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
	AL_METHOD(void, blit_to_memory, (struct BITMAP * source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
	AL_METHOD(void, blit_from_system, (struct BITMAP * source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
	AL_METHOD(void, blit_to_system, (struct BITMAP * source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
	AL_METHOD(void, blit_to_self, (struct BITMAP * source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
	AL_METHOD(void, blit_to_self_forward, (struct BITMAP * source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
	AL_METHOD(void, blit_to_self_backward, (struct BITMAP * source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
	AL_METHOD(void, blit_between_formats, (struct BITMAP * source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
	AL_METHOD(void, masked_blit, (struct BITMAP * source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
	AL_METHOD(void, clear_to_color, (struct BITMAP * bitmap, int color));
	AL_METHOD(void, pivot_scaled_sprite_flip, (struct BITMAP * bmp, struct BITMAP *sprite, fixed x, fixed y, fixed cx, fixed cy, fixed angle, fixed scale, int v_flip));
	AL_METHOD(void, do_stretch_blit, (struct BITMAP * source, struct BITMAP *dest, int source_x, int source_y, int source_width, int source_height, int dest_x, int dest_y, int dest_width, int dest_height, int masked));
	AL_METHOD(void, draw_gouraud_sprite, (struct BITMAP * bmp, struct BITMAP *sprite, int x, int y, int c1, int c2, int c3, int c4));
	AL_METHOD(void, draw_sprite_end, (void));
	AL_METHOD(void, blit_end, (void));
	AL_METHOD(void, polygon, (struct BITMAP * bmp, int vertices, AL_CONST int *points, int color));
	AL_METHOD(void, rect, (struct BITMAP * bmp, int x1, int y_1, int x2, int y2, int color));
	AL_METHOD(void, circle, (struct BITMAP * bmp, int x, int y, int radius, int color));
	AL_METHOD(void, circlefill, (struct BITMAP * bmp, int x, int y, int radius, int color));
	AL_METHOD(void, ellipse, (struct BITMAP * bmp, int x, int y, int rx, int ry, int color));
	AL_METHOD(void, ellipsefill, (struct BITMAP * bmp, int x, int y, int rx, int ry, int color));
	AL_METHOD(void, arc, (struct BITMAP * bmp, int x, int y, fixed ang1, fixed ang2, int r, int color));
	AL_METHOD(void, spline, (struct BITMAP * bmp, AL_CONST int points[8], int color));
	AL_METHOD(void, floodfill, (struct BITMAP * bmp, int x, int y, int color));
	AL_METHOD(void, polygon3d, (struct BITMAP * bmp, int type, struct BITMAP *texture, int vc, V3D *vtx[]));
	AL_METHOD(void, polygon3d_f, (struct BITMAP * bmp, int type, struct BITMAP *texture, int vc, V3D_f *vtx[]));
	AL_METHOD(void, triangle3d, (struct BITMAP * bmp, int type, struct BITMAP *texture, V3D *v1, V3D *v2, V3D *v3));
	AL_METHOD(void, triangle3d_f, (struct BITMAP * bmp, int type, struct BITMAP *texture, V3D_f *v1, V3D_f *v2, V3D_f *v3));
	AL_METHOD(void, quad3d, (struct BITMAP * bmp, int type, struct BITMAP *texture, V3D *v1, V3D *v2, V3D *v3, V3D *v4));
	AL_METHOD(void, quad3d_f, (struct BITMAP * bmp, int type, struct BITMAP *texture, V3D_f *v1, V3D_f *v2, V3D_f *v3, V3D_f *v4));

	AL_METHOD(void, draw_sprite_ex, (struct BITMAP * bmp, struct BITMAP *sprite, int x, int y, int mode, int flip));
} GFX_VTABLE;

AL_VAR(GFX_VTABLE, __linear_vtable8);
AL_VAR(GFX_VTABLE, __linear_vtable15);
AL_VAR(GFX_VTABLE, __linear_vtable16);
AL_VAR(GFX_VTABLE, __linear_vtable24);
AL_VAR(GFX_VTABLE, __linear_vtable32);

typedef struct _VTABLE_INFO {
	int color_depth;
	GFX_VTABLE *vtable;
} _VTABLE_INFO;

AL_ARRAY(_VTABLE_INFO, _vtable_list);

/* macros for constructing the vtable list */
#define BEGIN_COLOR_DEPTH_LIST \
	_VTABLE_INFO _vtable_list[] = {
#define END_COLOR_DEPTH_LIST \
	{ 0, NULL }              \
	}                        \
	;

#define COLOR_DEPTH_8 { 8, &__linear_vtable8 },
#define COLOR_DEPTH_15 { 15, &__linear_vtable15 },
#define COLOR_DEPTH_16 { 16, &__linear_vtable16 },
#define COLOR_DEPTH_24 { 24, &__linear_vtable24 },
#define COLOR_DEPTH_32 { 32, &__linear_vtable32 },

typedef struct BITMAP /* a bitmap structure */
{
	int w, h; /* width and height in pixels */
	int clip; /* flag if clipping is turned on */
	int cl, cr, ct, cb; /* clip left, right, top and bottom values */
	GFX_VTABLE *vtable; /* drawing functions */
	void *dat; /* the memory we allocated for the bitmap */
	unsigned long id; /* for identifying sub-bitmaps */
	void *extra; /* points to a structure with more info */
	int x_ofs; /* horizontal offset (for sub-bitmaps) */
	int y_ofs; /* vertical offset (for sub-bitmaps) */
	int seg; /* bitmap segment */
	ZERO_SIZE_ARRAY(unsigned char *, line);
} BITMAP;

#define BMP_ID_SUB 0x20000000
#define BMP_ID_NOBLIT 0x08000000
#define BMP_ID_LOCKED 0x04000000
#define BMP_ID_AUTOLOCK 0x02000000
#define BMP_ID_MASK 0x01FFFFFF

AL_VAR(BITMAP *, screen);

AL_VAR(int, SCREEN_W);
AL_VAR(int, SCREEN_H);

#define COLORCONV_NONE 0

#define COLORCONV_8_TO_15 1
#define COLORCONV_8_TO_16 2
#define COLORCONV_8_TO_24 4
#define COLORCONV_8_TO_32 8

#define COLORCONV_15_TO_8 0x10
#define COLORCONV_15_TO_16 0x20
#define COLORCONV_15_TO_24 0x40
#define COLORCONV_15_TO_32 0x80

#define COLORCONV_16_TO_8 0x100
#define COLORCONV_16_TO_15 0x200
#define COLORCONV_16_TO_24 0x400
#define COLORCONV_16_TO_32 0x800

#define COLORCONV_24_TO_8 0x1000
#define COLORCONV_24_TO_15 0x2000
#define COLORCONV_24_TO_16 0x4000
#define COLORCONV_24_TO_32 0x8000

#define COLORCONV_32_TO_8 0x10000
#define COLORCONV_32_TO_15 0x20000
#define COLORCONV_32_TO_16 0x40000
#define COLORCONV_32_TO_24 0x80000

#define COLORCONV_32A_TO_8 0x100000
#define COLORCONV_32A_TO_15 0x200000
#define COLORCONV_32A_TO_16 0x400000
#define COLORCONV_32A_TO_24 0x800000

#define COLORCONV_DITHER_PAL 0x1000000
#define COLORCONV_DITHER_HI 0x2000000
#define COLORCONV_KEEP_TRANS 0x4000000

#define COLORCONV_DITHER (COLORCONV_DITHER_PAL | \
		COLORCONV_DITHER_HI)

#define COLORCONV_EXPAND_256 (COLORCONV_8_TO_15 | \
		COLORCONV_8_TO_16 |                       \
		COLORCONV_8_TO_24 |                       \
		COLORCONV_8_TO_32)

#define COLORCONV_REDUCE_TO_256 (COLORCONV_15_TO_8 | \
		COLORCONV_16_TO_8 |                          \
		COLORCONV_24_TO_8 |                          \
		COLORCONV_32_TO_8 |                          \
		COLORCONV_32A_TO_8)

#define COLORCONV_EXPAND_15_TO_16 COLORCONV_15_TO_16

#define COLORCONV_REDUCE_16_TO_15 COLORCONV_16_TO_15

#define COLORCONV_EXPAND_HI_TO_TRUE (COLORCONV_15_TO_24 | \
		COLORCONV_15_TO_32 |                              \
		COLORCONV_16_TO_24 |                              \
		COLORCONV_16_TO_32)

#define COLORCONV_REDUCE_TRUE_TO_HI (COLORCONV_24_TO_15 | \
		COLORCONV_24_TO_16 |                              \
		COLORCONV_32_TO_15 |                              \
		COLORCONV_32_TO_16)

#define COLORCONV_24_EQUALS_32 (COLORCONV_24_TO_32 | \
		COLORCONV_32_TO_24)

#define COLORCONV_TOTAL (COLORCONV_EXPAND_256 | \
		COLORCONV_REDUCE_TO_256 |               \
		COLORCONV_EXPAND_15_TO_16 |             \
		COLORCONV_REDUCE_16_TO_15 |             \
		COLORCONV_EXPAND_HI_TO_TRUE |           \
		COLORCONV_REDUCE_TRUE_TO_HI |           \
		COLORCONV_24_EQUALS_32 |                \
		COLORCONV_32A_TO_15 |                   \
		COLORCONV_32A_TO_16 |                   \
		COLORCONV_32A_TO_24)

#define COLORCONV_PARTIAL (COLORCONV_EXPAND_15_TO_16 | \
		COLORCONV_REDUCE_16_TO_15 |                    \
		COLORCONV_24_EQUALS_32)

#define COLORCONV_MOST (COLORCONV_EXPAND_15_TO_16 | \
		COLORCONV_REDUCE_16_TO_15 |                 \
		COLORCONV_EXPAND_HI_TO_TRUE |               \
		COLORCONV_REDUCE_TRUE_TO_HI |               \
		COLORCONV_24_EQUALS_32)

#define COLORCONV_KEEP_ALPHA (COLORCONV_TOTAL & ~(COLORCONV_32A_TO_8 | COLORCONV_32A_TO_15 | COLORCONV_32A_TO_16 | COLORCONV_32A_TO_24))

AL_FUNC(int *, install_error, (int *errno_ptr));
AL_FUNC(BITMAP *, alloc_screen, (int width, int height));
AL_FUNC(void, set_color_depth, (int depth));
AL_FUNC(int, get_color_depth, (void));
AL_FUNC(void, set_color_conversion, (int mode));
AL_FUNC(int, get_color_conversion, (void));
AL_FUNC(BITMAP *, create_bitmap, (int width, int height));
AL_FUNC(BITMAP *, create_bitmap_ex, (int color_depth, int width, int height));
AL_FUNC(BITMAP *, create_sub_bitmap, (BITMAP * parent, int x, int y, int width, int height));
AL_FUNC(void, destroy_bitmap, (BITMAP * bitmap));
AL_FUNC(void, set_clip_rect, (BITMAP * bitmap, int x1, int y_1, int x2, int y2));
AL_FUNC(void, add_clip_rect, (BITMAP * bitmap, int x1, int y_1, int x2, int y2));
AL_FUNC(void, clear_bitmap, (BITMAP * bitmap));

/* Bitfield for relaying graphics driver type information */
#define GFX_TYPE_UNKNOWN 0
#define GFX_TYPE_WINDOWED 1
#define GFX_TYPE_FULLSCREEN 2
#define GFX_TYPE_DEFINITE 4
#define GFX_TYPE_MAGIC 8

AL_FUNC(int, get_gfx_mode_type, (int graphics_card));
AL_FUNC(int, get_gfx_mode, (void));

#define SWITCH_NONE 0
#define SWITCH_PAUSE 1
#define SWITCH_AMNESIA 2
#define SWITCH_BACKGROUND 3
#define SWITCH_BACKAMNESIA 4

#define SWITCH_IN 0
#define SWITCH_OUT 1

AL_FUNC(int, set_display_switch_mode, (int mode));
AL_FUNC(int, get_display_switch_mode, (void));
AL_FUNC(int, set_display_switch_callback, (int dir, AL_METHOD(void, cb, (void))));
AL_FUNC(void, remove_display_switch_callback, (AL_METHOD(void, cb, (void))));

AL_FUNC(void, lock_bitmap, (struct BITMAP * bmp));

/**
 * Gfx inline
 */

AL_INLINE(int, _default_ds, (void), { return 0; })

typedef AL_METHOD(uintptr_t, _BMP_BANK_SWITCHER, (BITMAP * bmp, int lyne));
typedef AL_METHOD(void, _BMP_UNBANK_SWITCHER, (BITMAP * bmp));

typedef AL_METHOD(uintptr_t, _BMP_BANK_SWITCHER, (BITMAP * bmp, int lyne));
typedef AL_METHOD(void, _BMP_UNBANK_SWITCHER, (BITMAP * bmp));

AL_INLINE(uintptr_t, bmp_write_line, (BITMAP * bmp, int lyne), {
	return (uintptr_t)bmp->line[lyne];
})

AL_INLINE(uintptr_t, bmp_read_line, (BITMAP * bmp, int lyne), {
	return (uintptr_t)bmp->line[lyne];
})

AL_INLINE(void, clear_to_color, (BITMAP * bitmap, int color), {
	ASSERT(bitmap);
	bitmap->vtable->clear_to_color(bitmap, color);
})

AL_INLINE(int, bitmap_color_depth, (BITMAP * bmp), {
	ASSERT(bmp);
	return bmp->vtable->color_depth;
})

AL_INLINE(int, bitmap_mask_color, (BITMAP * bmp), {
	ASSERT(bmp);
	return bmp->vtable->mask_color;
})

AL_INLINE(int, is_same_bitmap, (BITMAP * bmp1, BITMAP *bmp2), {
	unsigned long m1;
	unsigned long m2;

	if ((!bmp1) || (!bmp2))
		return FALSE;

	if (bmp1 == bmp2)
		return TRUE;

	m1 = bmp1->id & BMP_ID_MASK;
	m2 = bmp2->id & BMP_ID_MASK;

	return ((m1) && (m1 == m2));
})

AL_INLINE(int, is_screen_bitmap, (BITMAP * bmp), {
	ASSERT(bmp);
	return is_same_bitmap(bmp, screen);
})

AL_INLINE(int, is_sub_bitmap, (BITMAP * bmp), {
	ASSERT(bmp);
	return (bmp->id & BMP_ID_SUB) != 0;
})

AL_INLINE(void, acquire_bitmap, (BITMAP * bmp), {
	ASSERT(bmp);
	if (bmp->vtable->acquire)
		bmp->vtable->acquire(bmp);
})

AL_INLINE(void, release_bitmap, (BITMAP * bmp), {
	ASSERT(bmp);
	if (bmp->vtable->release)
		bmp->vtable->release(bmp);
})

AL_INLINE(void, acquire_screen, (void), { acquire_bitmap(screen); })
AL_INLINE(void, release_screen, (void), { release_bitmap(screen); })

AL_INLINE(int, is_inside_bitmap, (BITMAP * bmp, int x, int y, int clip), {
	ASSERT(bmp);
	if (clip) {
		if (bmp->clip) /* internal clipping is inclusive-exclusive */
			return (x >= bmp->cl) && (y >= bmp->ct) && (x < bmp->cr) && (y < bmp->cb);
		else
			return TRUE;
	} else
		/* bitmap dimensions are always non-negative */
		return (unsigned int)x < (unsigned int)bmp->w && (unsigned int)y < (unsigned int)bmp->h;
})

AL_INLINE(void, get_clip_rect, (BITMAP * bitmap, int *x1, int *y_1, int *x2, int *y2), {
	ASSERT(bitmap);
	/* internal clipping is inclusive-exclusive */
	*x1 = bitmap->cl;
	*y_1 = bitmap->ct;
	*x2 = bitmap->cr - 1;
	*y2 = bitmap->cb - 1;
})

AL_INLINE(void, set_clip_state, (BITMAP * bitmap, int state), {
	ASSERT(bitmap);
	bitmap->clip = state;
})

AL_INLINE(int, get_clip_state, (BITMAP * bitmap), {
	ASSERT(bitmap);
	return bitmap->clip;
})

/*
 * AL internal
 */

/* length in bytes of the cpu_vendor string */
#define _AL_CPU_VENDOR_SIZE 32

AL_FUNCPTR(int, _al_trace_handler, (AL_CONST char *msg));

/* malloc wrappers */
#define _AL_MALLOC(SIZE) (_al_malloc(SIZE))
#define _AL_MALLOC_ATOMIC(SIZE) (_al_malloc(SIZE))
#define _AL_FREE(PTR) (_al_free(PTR))
#define _AL_REALLOC(PTR, SIZE) (_al_realloc(PTR, SIZE))

AL_FUNC(void *, _al_malloc, (size_t size));
AL_FUNC(void, _al_free, (void *mem));
AL_FUNC(void *, _al_realloc, (void *mem, size_t size));
AL_FUNC(char *, _al_strdup, (AL_CONST char *string));
AL_FUNC(char *, _al_ustrdup, (AL_CONST char *string));

#define U_ASCII AL_ID('A', 'S', 'C', '8')
#define U_ASCII_CP AL_ID('A', 'S', 'C', 'P')
#define U_UNICODE AL_ID('U', 'N', 'I', 'C')
#define U_UTF8 AL_ID('U', 'T', 'F', '8')
#define U_CURRENT AL_ID('c', 'u', 'r', '.')

AL_FUNCPTR(int, ugetc, (AL_CONST char *s));
AL_FUNCPTR(int, ugetx, (char **s));
AL_FUNCPTR(int, ugetxc, (AL_CONST char **s));
AL_FUNCPTR(int, usetc, (char *s, int c));
AL_FUNCPTR(int, uwidth, (AL_CONST char *s));
AL_FUNCPTR(int, ucwidth, (int c));
AL_FUNC(int, ustrlen, (AL_CONST char *s));
AL_FUNC(int, uoffset, (AL_CONST char *s, int idx));
AL_FUNC(int, ustrsize, (AL_CONST char *s));
AL_FUNC(char *, ustrtok_r, (char *s, AL_CONST char *set, char **last));
AL_FUNC(int, uvszprintf, (char *buf, int size, AL_CONST char *format, va_list args));

AL_FUNC(int, need_uconvert, (AL_CONST char *s, int type, int newtype));
AL_FUNC(void, do_uconvert, (AL_CONST char *s, int type, char *buf, int newtype, int size));
AL_FUNC(char *, uconvert, (AL_CONST char *s, int type, char *buf, int newtype, int size));
AL_FUNC(int, uwidth_max, (int type));

/* some Allegro functions need a block of scratch memory */
AL_VAR(void *, _scratch_mem);
AL_VAR(int, _scratch_mem_size);

AL_INLINE(void, _grow_scratch_mem, (int size),
		{
			if (size > _scratch_mem_size) {
				size = (size + 1023) & 0xFFFFFC00;
				_scratch_mem = _AL_REALLOC(_scratch_mem, size);
				_scratch_mem_size = size;
			}
		})

/* helper structure for talking to Unicode strings */
typedef struct UTYPE_INFO {
	int id;
	AL_METHOD(int, u_getc, (AL_CONST char *s));
	AL_METHOD(int, u_getx, (char **s));
	AL_METHOD(int, u_setc, (char *s, int c));
	AL_METHOD(int, u_width, (AL_CONST char *s));
	AL_METHOD(int, u_cwidth, (int c));
	AL_METHOD(int, u_isok, (int c));
	int u_width_max;
} UTYPE_INFO;

AL_FUNC(UTYPE_INFO *, _find_utype, (int type));

/* config stuff */
void _reload_config(void);

/* text- and font-related stuff */
typedef struct FONT_GLYPH /* a single monochrome font character */
{
	short w, h;
	ZERO_SIZE_ARRAY(unsigned char, dat);
} FONT_GLYPH;

struct FONT_VTABLE;

typedef struct FONT {
	void *data;
	int height;
	struct FONT_VTABLE *vtable;
} FONT;

AL_FUNC(int, font_has_alpha, (FONT * f));
AL_FUNC(void, make_trans_font, (FONT * f));

AL_FUNC(int, is_trans_font, (FONT * f));
AL_FUNC(int, is_color_font, (FONT * f));
AL_FUNC(int, is_mono_font, (FONT * f));
AL_FUNC(int, is_compatible_font, (FONT * f1, FONT *f2));

typedef struct FONT_VTABLE {
	AL_METHOD(int, font_height, (AL_CONST FONT * f));
	AL_METHOD(int, char_length, (AL_CONST FONT * f, int ch));
	AL_METHOD(int, text_length, (AL_CONST FONT * f, AL_CONST char *text));
	AL_METHOD(int, render_char, (AL_CONST FONT * f, int ch, int fg, int bg, BITMAP *bmp, int x, int y));
	AL_METHOD(void, render, (AL_CONST FONT * f, AL_CONST char *text, int fg, int bg, BITMAP *bmp, int x, int y));
	AL_METHOD(void, destroy, (FONT * f));

	AL_METHOD(int, get_font_ranges, (FONT * f));
	AL_METHOD(int, get_font_range_begin, (FONT * f, int range));
	AL_METHOD(int, get_font_range_end, (FONT * f, int range));
	AL_METHOD(FONT *, extract_font_range, (FONT * f, int begin, int end));
	AL_METHOD(FONT *, merge_fonts, (FONT * f1, FONT *f2));
	AL_METHOD(int, transpose_font, (FONT * f, int drange));
} FONT_VTABLE;

AL_VAR(FONT_VTABLE, _font_vtable_mono);
AL_VAR(FONT_VTABLE *, font_vtable_mono);
AL_VAR(FONT_VTABLE, _font_vtable_color);
AL_VAR(FONT_VTABLE *, font_vtable_color);
AL_VAR(FONT_VTABLE, _font_vtable_trans);
AL_VAR(FONT_VTABLE *, font_vtable_trans);

AL_FUNC(FONT_GLYPH *, _mono_find_glyph, (AL_CONST FONT * f, int ch));
AL_FUNC(BITMAP *, _color_find_glyph, (AL_CONST FONT * f, int ch));

AL_FUNC(FONT *, load_font, (AL_CONST char *filename, RGB *pal, void *param));
AL_FUNC(FONT *, load_font, (AL_CONST char *filename, RGB *pal, void *param));

AL_FUNC(FONT *, load_dat_font, (AL_CONST char *filename, RGB *pal, void *param));
AL_FUNC(FONT *, load_bios_font, (AL_CONST char *filename, RGB *pal, void *param));
AL_FUNC(FONT *, load_grx_font, (AL_CONST char *filename, RGB *pal, void *param));
AL_FUNC(FONT *, load_grx_or_bios_font, (AL_CONST char *filename, RGB *pal, void *param));
AL_FUNC(FONT *, load_bitmap_font, (AL_CONST char *fname, RGB *pal, void *param));
AL_FUNC(FONT *, load_txt_font, (AL_CONST char *fname, RGB *pal, void *param));

AL_FUNC(FONT *, grab_font_from_bitmap, (BITMAP * bmp));

AL_FUNC(int, get_font_ranges, (FONT * f));
AL_FUNC(int, get_font_range_begin, (FONT * f, int range));
AL_FUNC(int, get_font_range_end, (FONT * f, int range));
AL_FUNC(FONT *, extract_font_range, (FONT * f, int begin, int end));
AL_FUNC(FONT *, merge_fonts, (FONT * f1, FONT *f2));
AL_FUNC(int, transpose_font, (FONT * f, int drange));

typedef struct FONT_MONO_DATA {
	int begin, end; /* first char and one-past-the-end char */
	FONT_GLYPH **glyphs; /* our glyphs */
	struct FONT_MONO_DATA *next; /* linked list structure */
} FONT_MONO_DATA;

typedef struct FONT_COLOR_DATA {
	int begin, end; /* first char and one-past-the-end char */
	BITMAP **bitmaps; /* our glyphs */
	struct FONT_COLOR_DATA *next; /* linked list structure */
} FONT_COLOR_DATA;

/* stuff for setting up bitmaps */
AL_FUNC(GFX_VTABLE *, _get_vtable, (int color_depth));
AL_VAR(GFX_VTABLE, _screen_vtable);
AL_VAR(int, _sub_bitmap_id_count);
AL_VAR(int, _screen_split_position);

#ifdef ALLEGRO_I386
#define BYTES_PER_PIXEL(bpp) (((int)(bpp) + 7) / 8)
#else
#define BYTES_PER_PIXEL(bpp) (((bpp) <= 8) ? 1 : (((bpp) <= 16) ? 2 : (((bpp) <= 24) ? 3 : 4)))
#endif

AL_VAR(int, _color_conv);
AL_FUNC(int, _color_load_depth, (int depth, int hasalpha));
AL_FUNC(BITMAP *, _fixup_loaded_bitmap, (BITMAP * bmp, PALETTE pal, int bpp));
AL_FUNC(int, _bitmap_has_alpha, (BITMAP * bmp));

/* default truecolor pixel format */
#define DEFAULT_RGB_R_SHIFT_15 0
#define DEFAULT_RGB_G_SHIFT_15 5
#define DEFAULT_RGB_B_SHIFT_15 10
#define DEFAULT_RGB_R_SHIFT_16 0
#define DEFAULT_RGB_G_SHIFT_16 5
#define DEFAULT_RGB_B_SHIFT_16 11
#define DEFAULT_RGB_R_SHIFT_24 0
#define DEFAULT_RGB_G_SHIFT_24 8
#define DEFAULT_RGB_B_SHIFT_24 16
#define DEFAULT_RGB_R_SHIFT_32 0
#define DEFAULT_RGB_G_SHIFT_32 8
#define DEFAULT_RGB_B_SHIFT_32 16
#define DEFAULT_RGB_A_SHIFT_32 24

/* current drawing mode */
AL_VAR(int, _drawing_mode);
AL_VAR(BITMAP *, _drawing_pattern);
AL_VAR(int, _drawing_x_anchor);
AL_VAR(int, _drawing_y_anchor);
AL_VAR(unsigned int, _drawing_x_mask);
AL_VAR(unsigned int, _drawing_y_mask);

AL_FUNCPTR(int *, _palette_expansion_table, (int bpp));

AL_VAR(int, _color_depth);

AL_VAR(int, _current_palette_changed);
AL_VAR(PALETTE, _prev_current_palette);
AL_VAR(int, _got_prev_current_palette);

AL_ARRAY(int, _palette_color8);
AL_ARRAY(int, _palette_color15);
AL_ARRAY(int, _palette_color16);
AL_ARRAY(int, _palette_color24);
AL_ARRAY(int, _palette_color32);

/* truecolor blending functions */
AL_VAR(BLENDER_FUNC, _blender_func15);
AL_VAR(BLENDER_FUNC, _blender_func16);
AL_VAR(BLENDER_FUNC, _blender_func24);
AL_VAR(BLENDER_FUNC, _blender_func32);

AL_VAR(BLENDER_FUNC, _blender_func15x);
AL_VAR(BLENDER_FUNC, _blender_func16x);
AL_VAR(BLENDER_FUNC, _blender_func24x);

AL_VAR(int, _blender_col_15);
AL_VAR(int, _blender_col_16);
AL_VAR(int, _blender_col_24);
AL_VAR(int, _blender_col_32);

AL_VAR(int, _blender_alpha);

AL_FUNC(unsigned long, _blender_black, (unsigned long x, unsigned long y, unsigned long n));

#ifdef ALLEGRO_COLOR16

AL_FUNC(unsigned long, _blender_trans15, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_add15, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_burn15, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_color15, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_difference15, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_dissolve15, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_dodge15, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_hue15, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_invert15, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_luminance15, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_multiply15, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_saturation15, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_screen15, (unsigned long x, unsigned long y, unsigned long n));

AL_FUNC(unsigned long, _blender_trans16, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_add16, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_burn16, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_color16, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_difference16, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_dissolve16, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_dodge16, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_hue16, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_invert16, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_luminance16, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_multiply16, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_saturation16, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_screen16, (unsigned long x, unsigned long y, unsigned long n));

#endif

#if (defined ALLEGRO_COLOR24) || (defined ALLEGRO_COLOR32)

AL_FUNC(unsigned long, _blender_trans24, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_add24, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_burn24, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_color24, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_difference24, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_dissolve24, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_dodge24, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_hue24, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_invert24, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_luminance24, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_multiply24, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_saturation24, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_screen24, (unsigned long x, unsigned long y, unsigned long n));

#endif

AL_FUNC(unsigned long, _blender_alpha15, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_alpha16, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_alpha24, (unsigned long x, unsigned long y, unsigned long n));
AL_FUNC(unsigned long, _blender_alpha32, (unsigned long x, unsigned long y, unsigned long n));

AL_FUNC(unsigned long, _blender_write_alpha, (unsigned long x, unsigned long y, unsigned long n));

/* graphics drawing routines */
AL_FUNC(void, _normal_line, (BITMAP * bmp, int x1, int y_1, int x2, int y2, int color));
AL_FUNC(void, _fast_line, (BITMAP * bmp, int x1, int y_1, int x2, int y2, int color));
AL_FUNC(void, _normal_rectfill, (BITMAP * bmp, int x1, int y_1, int x2, int y2, int color));

#ifdef ALLEGRO_COLOR8

AL_FUNC(int, _linear_getpixel8, (BITMAP * bmp, int x, int y));
AL_FUNC(void, _linear_putpixel8, (BITMAP * bmp, int x, int y, int color));
AL_FUNC(void, _linear_vline8, (BITMAP * bmp, int x, int y_1, int y2, int color));
AL_FUNC(void, _linear_hline8, (BITMAP * bmp, int x1, int y, int x2, int color));
AL_FUNC(void, _linear_draw_sprite8, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_sprite_ex8, (BITMAP * bmp, BITMAP *sprite, int x, int y, int mode, int flip));
AL_FUNC(void, _linear_draw_sprite_v_flip8, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_sprite_h_flip8, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_sprite_vh_flip8, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_trans_sprite8, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_lit_sprite8, (BITMAP * bmp, BITMAP *sprite, int x, int y, int color));
AL_FUNC(void, _linear_draw_rle_sprite8, (BITMAP * bmp, AL_CONST struct RLE_SPRITE *sprite, int x, int y));
AL_FUNC(void, _linear_draw_trans_rle_sprite8, (BITMAP * bmp, AL_CONST struct RLE_SPRITE *sprite, int x, int y));
AL_FUNC(void, _linear_draw_lit_rle_sprite8, (BITMAP * bmp, AL_CONST struct RLE_SPRITE *sprite, int x, int y, int color));
AL_FUNC(void, _linear_draw_character8, (BITMAP * bmp, BITMAP *sprite, int x, int y, int color, int bg));
AL_FUNC(void, _linear_draw_glyph8, (BITMAP * bmp, AL_CONST FONT_GLYPH *glyph, int x, int y, int color, int bg));
AL_FUNC(void, _linear_blit8, (BITMAP * source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
AL_FUNC(void, _linear_blit_backward8, (BITMAP * source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
AL_FUNC(void, _linear_masked_blit8, (BITMAP * source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
AL_FUNC(void, _linear_clear_to_color8, (BITMAP * bitmap, int color));

#endif

#ifdef ALLEGRO_COLOR16

AL_FUNC(void, _linear_putpixel15, (BITMAP * bmp, int x, int y, int color));
AL_FUNC(void, _linear_vline15, (BITMAP * bmp, int x, int y_1, int y2, int color));
AL_FUNC(void, _linear_hline15, (BITMAP * bmp, int x1, int y, int x2, int color));
AL_FUNC(void, _linear_draw_trans_sprite15, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_trans_rgba_sprite15, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_lit_sprite15, (BITMAP * bmp, BITMAP *sprite, int x, int y, int color));
AL_FUNC(void, _linear_draw_rle_sprite15, (BITMAP * bmp, AL_CONST struct RLE_SPRITE *sprite, int x, int y));
AL_FUNC(void, _linear_draw_trans_rle_sprite15, (BITMAP * bmp, AL_CONST struct RLE_SPRITE *sprite, int x, int y));
AL_FUNC(void, _linear_draw_trans_rgba_rle_sprite15, (BITMAP * bmp, AL_CONST struct RLE_SPRITE *sprite, int x, int y));
AL_FUNC(void, _linear_draw_lit_rle_sprite15, (BITMAP * bmp, AL_CONST struct RLE_SPRITE *sprite, int x, int y, int color));

AL_FUNC(int, _linear_getpixel16, (BITMAP * bmp, int x, int y));
AL_FUNC(void, _linear_putpixel16, (BITMAP * bmp, int x, int y, int color));
AL_FUNC(void, _linear_vline16, (BITMAP * bmp, int x, int y_1, int y2, int color));
AL_FUNC(void, _linear_hline16, (BITMAP * bmp, int x1, int y, int x2, int color));
AL_FUNC(void, _linear_draw_sprite16, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_sprite_ex16, (BITMAP * bmp, BITMAP *sprite, int x, int y, int mode, int flip));
AL_FUNC(void, _linear_draw_256_sprite16, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_sprite_v_flip16, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_sprite_h_flip16, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_sprite_vh_flip16, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_trans_sprite16, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_trans_rgba_sprite16, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_lit_sprite16, (BITMAP * bmp, BITMAP *sprite, int x, int y, int color));
AL_FUNC(void, _linear_draw_rle_sprite16, (BITMAP * bmp, AL_CONST struct RLE_SPRITE *sprite, int x, int y));
AL_FUNC(void, _linear_draw_trans_rle_sprite16, (BITMAP * bmp, AL_CONST struct RLE_SPRITE *sprite, int x, int y));
AL_FUNC(void, _linear_draw_trans_rgba_rle_sprite16, (BITMAP * bmp, AL_CONST struct RLE_SPRITE *sprite, int x, int y));
AL_FUNC(void, _linear_draw_lit_rle_sprite16, (BITMAP * bmp, AL_CONST struct RLE_SPRITE *sprite, int x, int y, int color));
AL_FUNC(void, _linear_draw_character16, (BITMAP * bmp, BITMAP *sprite, int x, int y, int color, int bg));
AL_FUNC(void, _linear_draw_glyph16, (BITMAP * bmp, AL_CONST FONT_GLYPH *glyph, int x, int y, int color, int bg));
AL_FUNC(void, _linear_blit16, (BITMAP * source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
AL_FUNC(void, _linear_blit_backward16, (BITMAP * source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
AL_FUNC(void, _linear_masked_blit16, (BITMAP * source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
AL_FUNC(void, _linear_clear_to_color16, (BITMAP * bitmap, int color));

#endif

#ifdef ALLEGRO_COLOR24

AL_FUNC(int, _linear_getpixel24, (BITMAP * bmp, int x, int y));
AL_FUNC(void, _linear_putpixel24, (BITMAP * bmp, int x, int y, int color));
AL_FUNC(void, _linear_vline24, (BITMAP * bmp, int x, int y_1, int y2, int color));
AL_FUNC(void, _linear_hline24, (BITMAP * bmp, int x1, int y, int x2, int color));
AL_FUNC(void, _linear_draw_sprite24, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_sprite_ex24, (BITMAP * bmp, BITMAP *sprite, int x, int y, int mode, int flip));
AL_FUNC(void, _linear_draw_256_sprite24, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_sprite_v_flip24, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_sprite_h_flip24, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_sprite_vh_flip24, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_trans_sprite24, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_trans_rgba_sprite24, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_lit_sprite24, (BITMAP * bmp, BITMAP *sprite, int x, int y, int color));
AL_FUNC(void, _linear_draw_rle_sprite24, (BITMAP * bmp, AL_CONST struct RLE_SPRITE *sprite, int x, int y));
AL_FUNC(void, _linear_draw_trans_rle_sprite24, (BITMAP * bmp, AL_CONST struct RLE_SPRITE *sprite, int x, int y));
AL_FUNC(void, _linear_draw_trans_rgba_rle_sprite24, (BITMAP * bmp, AL_CONST struct RLE_SPRITE *sprite, int x, int y));
AL_FUNC(void, _linear_draw_lit_rle_sprite24, (BITMAP * bmp, AL_CONST struct RLE_SPRITE *sprite, int x, int y, int color));
AL_FUNC(void, _linear_draw_character24, (BITMAP * bmp, BITMAP *sprite, int x, int y, int color, int bg));
AL_FUNC(void, _linear_draw_glyph24, (BITMAP * bmp, AL_CONST FONT_GLYPH *glyph, int x, int y, int color, int bg));
AL_FUNC(void, _linear_blit24, (BITMAP * source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
AL_FUNC(void, _linear_blit_backward24, (BITMAP * source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
AL_FUNC(void, _linear_masked_blit24, (BITMAP * source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
AL_FUNC(void, _linear_clear_to_color24, (BITMAP * bitmap, int color));

#endif

#ifdef ALLEGRO_COLOR32

AL_FUNC(int, _linear_getpixel32, (BITMAP * bmp, int x, int y));
AL_FUNC(void, _linear_putpixel32, (BITMAP * bmp, int x, int y, int color));
AL_FUNC(void, _linear_vline32, (BITMAP * bmp, int x, int y_1, int y2, int color));
AL_FUNC(void, _linear_hline32, (BITMAP * bmp, int x1, int y, int x2, int color));
AL_FUNC(void, _linear_draw_sprite32, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_sprite_ex32, (BITMAP * bmp, BITMAP *sprite, int x, int y, int mode, int flip));
AL_FUNC(void, _linear_draw_256_sprite32, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_sprite_v_flip32, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_sprite_h_flip32, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_sprite_vh_flip32, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_trans_sprite32, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _linear_draw_lit_sprite32, (BITMAP * bmp, BITMAP *sprite, int x, int y, int color));
AL_FUNC(void, _linear_draw_rle_sprite32, (BITMAP * bmp, AL_CONST struct RLE_SPRITE *sprite, int x, int y));
AL_FUNC(void, _linear_draw_trans_rle_sprite32, (BITMAP * bmp, AL_CONST struct RLE_SPRITE *sprite, int x, int y));
AL_FUNC(void, _linear_draw_lit_rle_sprite32, (BITMAP * bmp, AL_CONST struct RLE_SPRITE *sprite, int x, int y, int color));
AL_FUNC(void, _linear_draw_character32, (BITMAP * bmp, BITMAP *sprite, int x, int y, int color, int bg));
AL_FUNC(void, _linear_draw_glyph32, (BITMAP * bmp, AL_CONST FONT_GLYPH *glyph, int x, int y, int color, int bg));
AL_FUNC(void, _linear_blit32, (BITMAP * source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
AL_FUNC(void, _linear_blit_backward32, (BITMAP * source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
AL_FUNC(void, _linear_masked_blit32, (BITMAP * source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
AL_FUNC(void, _linear_clear_to_color32, (BITMAP * bitmap, int color));

#endif

#ifdef ALLEGRO_GFX_HAS_VGA

AL_FUNC(int, _x_getpixel, (BITMAP * bmp, int x, int y));
AL_FUNC(void, _x_putpixel, (BITMAP * bmp, int x, int y, int color));
AL_FUNC(void, _x_vline, (BITMAP * bmp, int x, int y_1, int y2, int color));
AL_FUNC(void, _x_hline, (BITMAP * bmp, int x1, int y, int x2, int color));
AL_FUNC(void, _x_draw_sprite, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _x_draw_sprite_v_flip, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _x_draw_sprite_h_flip, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _x_draw_sprite_vh_flip, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _x_draw_trans_sprite, (BITMAP * bmp, BITMAP *sprite, int x, int y));
AL_FUNC(void, _x_draw_lit_sprite, (BITMAP * bmp, BITMAP *sprite, int x, int y, int color));
AL_FUNC(void, _x_draw_rle_sprite, (BITMAP * bmp, AL_CONST struct RLE_SPRITE *sprite, int x, int y));
AL_FUNC(void, _x_draw_trans_rle_sprite, (BITMAP * bmp, AL_CONST struct RLE_SPRITE *sprite, int x, int y));
AL_FUNC(void, _x_draw_lit_rle_sprite, (BITMAP * bmp, AL_CONST struct RLE_SPRITE *sprite, int x, int y, int color));
AL_FUNC(void, _x_draw_character, (BITMAP * bmp, BITMAP *sprite, int x, int y, int color, int bg));
AL_FUNC(void, _x_draw_glyph, (BITMAP * bmp, AL_CONST FONT_GLYPH *glyph, int x, int y, int color, int bg));
AL_FUNC(void, _x_blit_from_memory, (BITMAP * source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
AL_FUNC(void, _x_blit_to_memory, (BITMAP * source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
AL_FUNC(void, _x_blit, (BITMAP * source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
AL_FUNC(void, _x_blit_forward, (BITMAP * source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
AL_FUNC(void, _x_blit_backward, (BITMAP * source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
AL_FUNC(void, _x_masked_blit, (BITMAP * source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
AL_FUNC(void, _x_clear_to_color, (BITMAP * bitmap, int color));

#endif

/* color conversion routines */
typedef struct GRAPHICS_RECT {
	int width;
	int height;
	int pitch;
	void *data;
} GRAPHICS_RECT;

typedef void(COLORCONV_BLITTER_FUNC)(GRAPHICS_RECT *src_rect, GRAPHICS_RECT *dest_rect);

AL_FUNC(COLORCONV_BLITTER_FUNC *, _get_colorconv_blitter, (int from_depth, int to_depth));
AL_FUNC(void, _release_colorconv_blitter, (COLORCONV_BLITTER_FUNC * blitter));
AL_FUNC(void, _set_colorconv_palette, (AL_CONST struct RGB * p, int from, int to));
AL_FUNC(unsigned char *, _get_colorconv_map, (void));

#ifdef ALLEGRO_COLOR8

AL_FUNC(void, _colorconv_blit_8_to_8, (GRAPHICS_RECT * src_rect, GRAPHICS_RECT *dest_rect));
AL_FUNC(void, _colorconv_blit_8_to_15, (GRAPHICS_RECT * src_rect, GRAPHICS_RECT *dest_rect));
AL_FUNC(void, _colorconv_blit_8_to_16, (GRAPHICS_RECT * src_rect, GRAPHICS_RECT *dest_rect));
AL_FUNC(void, _colorconv_blit_8_to_24, (GRAPHICS_RECT * src_rect, GRAPHICS_RECT *dest_rect));
AL_FUNC(void, _colorconv_blit_8_to_32, (GRAPHICS_RECT * src_rect, GRAPHICS_RECT *dest_rect));

#endif

#ifdef ALLEGRO_COLOR16

AL_FUNC(void, _colorconv_blit_15_to_8, (GRAPHICS_RECT * src_rect, GRAPHICS_RECT *dest_rect));
AL_FUNC(void, _colorconv_blit_15_to_16, (GRAPHICS_RECT * src_rect, GRAPHICS_RECT *dest_rect));
AL_FUNC(void, _colorconv_blit_15_to_24, (GRAPHICS_RECT * src_rect, GRAPHICS_RECT *dest_rect));
AL_FUNC(void, _colorconv_blit_15_to_32, (GRAPHICS_RECT * src_rect, GRAPHICS_RECT *dest_rect));

AL_FUNC(void, _colorconv_blit_16_to_8, (GRAPHICS_RECT * src_rect, GRAPHICS_RECT *dest_rect));
AL_FUNC(void, _colorconv_blit_16_to_15, (GRAPHICS_RECT * src_rect, GRAPHICS_RECT *dest_rect));
AL_FUNC(void, _colorconv_blit_16_to_24, (GRAPHICS_RECT * src_rect, GRAPHICS_RECT *dest_rect));
AL_FUNC(void, _colorconv_blit_16_to_32, (GRAPHICS_RECT * src_rect, GRAPHICS_RECT *dest_rect));

#endif

#ifdef ALLEGRO_COLOR24

AL_FUNC(void, _colorconv_blit_24_to_8, (GRAPHICS_RECT * src_rect, GRAPHICS_RECT *dest_rect));
AL_FUNC(void, _colorconv_blit_24_to_15, (GRAPHICS_RECT * src_rect, GRAPHICS_RECT *dest_rect));
AL_FUNC(void, _colorconv_blit_24_to_16, (GRAPHICS_RECT * src_rect, GRAPHICS_RECT *dest_rect));
AL_FUNC(void, _colorconv_blit_24_to_32, (GRAPHICS_RECT * src_rect, GRAPHICS_RECT *dest_rect));

#endif

#ifdef ALLEGRO_COLOR32

AL_FUNC(void, _colorconv_blit_32_to_8, (GRAPHICS_RECT * src_rect, GRAPHICS_RECT *dest_rect));
AL_FUNC(void, _colorconv_blit_32_to_15, (GRAPHICS_RECT * src_rect, GRAPHICS_RECT *dest_rect));
AL_FUNC(void, _colorconv_blit_32_to_16, (GRAPHICS_RECT * src_rect, GRAPHICS_RECT *dest_rect));
AL_FUNC(void, _colorconv_blit_32_to_24, (GRAPHICS_RECT * src_rect, GRAPHICS_RECT *dest_rect));

#endif

/* color copy routines */
#ifndef ALLEGRO_NO_COLORCOPY

#ifdef ALLEGRO_COLOR16
AL_FUNC(void, _colorcopy_blit_15_to_15, (GRAPHICS_RECT * src_rect, GRAPHICS_RECT *dest_rect));
AL_FUNC(void, _colorcopy_blit_16_to_16, (GRAPHICS_RECT * src_rect, GRAPHICS_RECT *dest_rect));
#endif

#ifdef ALLEGRO_COLOR24
AL_FUNC(void, _colorcopy_blit_24_to_24, (GRAPHICS_RECT * src_rect, GRAPHICS_RECT *dest_rect));
#endif

#ifdef ALLEGRO_COLOR32
AL_FUNC(void, _colorcopy_blit_32_to_32, (GRAPHICS_RECT * src_rect, GRAPHICS_RECT *dest_rect));
#endif

#endif

/* generic color conversion blitter */
AL_FUNC(void, _blit_between_formats, (BITMAP * src, BITMAP *dest, int s_x, int s_y, int d_x, int d_y, int w, int h));

/* asm helper for stretch_blit() */
#ifndef SCAN_EXPORT
AL_FUNC(void, _do_stretch, (BITMAP * source, BITMAP *dest, void *drawer, int sx, fixed sy, fixed syd, int dx, int dy, int dh, int color_depth));
#endif

/* lower level functions for rotation */
AL_FUNC(void, _parallelogram_map, (BITMAP * bmp, BITMAP *spr, fixed xs[4], fixed ys[4], void (*draw_scanline)(BITMAP *bmp, BITMAP *spr, fixed l_bmp_x, int bmp_y, fixed r_bmp_x, fixed l_spr_x, fixed l_spr_y, fixed spr_dx, fixed spr_dy), int sub_pixel_accuracy));
AL_FUNC(void, _parallelogram_map_standard, (BITMAP * bmp, BITMAP *sprite, fixed xs[4], fixed ys[4]));
AL_FUNC(void, _rotate_scale_flip_coordinates, (fixed w, fixed h, fixed x, fixed y, fixed cx, fixed cy, fixed angle, fixed scale_x, fixed scale_y, int h_flip, int v_flip, fixed xs[4], fixed ys[4]));
AL_FUNC(void, _pivot_scaled_sprite_flip, (struct BITMAP * bmp, struct BITMAP *sprite, fixed x, fixed y, fixed cx, fixed cy, fixed angle, fixed scale, int v_flip));

/* number of fractional bits used by the polygon rasteriser */
#define POLYGON_FIX_SHIFT 18

/* bitfield specifying which polygon attributes need interpolating */
#define INTERP_FLAT 1 /* no interpolation */
#define INTERP_1COL 2 /* gcol or alpha */
#define INTERP_3COL 4 /* grgb */
#define INTERP_FIX_UV 8 /* atex */
#define INTERP_Z 16 /* always in scene3d */
#define INTERP_FLOAT_UV 32 /* ptex */
#define OPT_FLOAT_UV_TO_FIX 64 /* translate ptex to atex */
#define COLOR_TO_RGB 128 /* grgb to gcol for truecolor */
#define INTERP_ZBUF 256 /* z-buffered */
#define INTERP_THRU 512 /* any kind of transparent */
#define INTERP_NOSOLID 1024 /* non-solid modes for 8-bit flat */
#define INTERP_BLEND 2048 /* lit for truecolor */
#define INTERP_TRANS 4096 /* trans for truecolor */

/* information for polygon scanline fillers */
typedef struct POLYGON_SEGMENT {
	fixed u, v, du, dv; /* fixed point u/v coordinates */
	fixed c, dc; /* single color gouraud shade values */
	fixed r, g, b, dr, dg, db; /* RGB gouraud shade values */
	float z, dz; /* polygon depth (1/z) */
	float fu, fv, dfu, dfv; /* floating point u/v coordinates */
	unsigned char *texture; /* the texture map */
	int umask, vmask, vshift; /* texture map size information */
	int seg; /* destination bitmap selector */
	uintptr_t zbuf_addr; /* Z-buffer address */
	uintptr_t read_addr; /* reading address for transparency modes */
} POLYGON_SEGMENT;

/* prototype for the scanline filler functions */
typedef AL_METHOD(void, SCANLINE_FILLER, (uintptr_t addr, int w, POLYGON_SEGMENT *info));

/* an active polygon edge */
typedef struct POLYGON_EDGE {
	int top; /* top y position */
	int bottom; /* bottom y position */
	fixed x, dx; /* fixed point x position and gradient */
	fixed w; /* width of line segment */
	POLYGON_SEGMENT dat; /* texture/gouraud information */
	struct POLYGON_EDGE *prev; /* doubly linked list */
	struct POLYGON_EDGE *next;
	struct POLYGON_INFO *poly; /* father polygon */
} POLYGON_EDGE;

typedef struct POLYGON_INFO /* a polygon waiting rendering */
{
	struct POLYGON_INFO *next, *prev; /* double linked list */
	int inside; /* flag for "scanlining" */
	int flags; /* INTERP_* flags */
	int color; /* vtx[0]->c */
	float a, b, c; /* plane's coefficients -a/d, -b/d, -c/d */
	int dmode; /* drawing mode */
	BITMAP *dpat; /* drawing pattern */
	int xanchor, yanchor; /* for dpat */
	int alpha; /* blender alpha */
	int b15, b16, b24, b32; /* blender colors */
	COLOR_MAP *cmap; /* trans color map */
	SCANLINE_FILLER drawer; /* scanline drawing functions */
	SCANLINE_FILLER alt_drawer;
	POLYGON_EDGE *left_edge; /* true edges used in interpolation */
	POLYGON_EDGE *right_edge;
	POLYGON_SEGMENT info; /* base information for scanline functions */
} POLYGON_INFO;

/* global variable for z-buffer */
AL_VAR(BITMAP *, _zbuffer);

/* polygon helper functions */
AL_VAR(SCANLINE_FILLER, _optim_alternative_drawer);
AL_FUNC(POLYGON_EDGE *, _add_edge, (POLYGON_EDGE * list, POLYGON_EDGE *edge, int sort_by_x));
AL_FUNC(POLYGON_EDGE *, _remove_edge, (POLYGON_EDGE * list, POLYGON_EDGE *edge));
AL_FUNC(int, _fill_3d_edge_structure, (POLYGON_EDGE * edge, AL_CONST V3D *v1, AL_CONST V3D *v2, int flags, BITMAP *bmp));
AL_FUNC(int, _fill_3d_edge_structure_f, (POLYGON_EDGE * edge, AL_CONST V3D_f *v1, AL_CONST V3D_f *v2, int flags, BITMAP *bmp));
AL_FUNC(SCANLINE_FILLER, _get_scanline_filler, (int type, int *flags, POLYGON_SEGMENT *info, BITMAP *texture, BITMAP *bmp));
AL_FUNC(void, _clip_polygon_segment, (POLYGON_SEGMENT * info, fixed gap, int flags));
AL_FUNC(void, _clip_polygon_segment_f, (POLYGON_SEGMENT * info, int gap, int flags));

/* polygon scanline filler functions */
AL_FUNC(void, _poly_scanline_dummy, (uintptr_t addr, int w, POLYGON_SEGMENT *info));

#ifdef ALLEGRO_COLOR8

AL_FUNC(void, _poly_scanline_gcol8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_grgb8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_mask8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_mask8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_lit8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_lit8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_mask_lit8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_mask_lit8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_trans8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_trans8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_mask_trans8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_mask_trans8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));

#ifndef SCAN_EXPORT
AL_FUNC(void, _poly_scanline_grgb8x, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
#endif

AL_FUNC(void, _poly_zbuf_flat8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_gcol8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_grgb8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex_mask8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex_mask8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex_lit8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex_lit8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex_mask_lit8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex_mask_lit8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex_trans8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex_trans8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex_mask_trans8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex_mask_trans8, (uintptr_t addr, int w, POLYGON_SEGMENT *info));

#endif

#ifdef ALLEGRO_COLOR16

AL_FUNC(void, _poly_scanline_grgb15, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_mask15, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_mask15, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_lit15, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_lit15, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_mask_lit15, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_mask_lit15, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_trans15, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_trans15, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_mask_trans15, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_mask_trans15, (uintptr_t addr, int w, POLYGON_SEGMENT *info));

#ifndef SCAN_EXPORT
AL_FUNC(void, _poly_scanline_grgb15x, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_lit15x, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_lit15x, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_mask_lit15x, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_mask_lit15x, (uintptr_t addr, int w, POLYGON_SEGMENT *info));

AL_FUNC(void, _poly_scanline_ptex_lit15d, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_mask_lit15d, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
#endif

AL_FUNC(void, _poly_zbuf_grgb15, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex_mask15, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex_mask15, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex_lit15, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex_lit15, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex_mask_lit15, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex_mask_lit15, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex_trans15, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex_trans15, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex_mask_trans15, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex_mask_trans15, (uintptr_t addr, int w, POLYGON_SEGMENT *info));

AL_FUNC(void, _poly_scanline_grgb16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_mask16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_mask16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_lit16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_lit16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_mask_lit16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_mask_lit16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_trans16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_trans16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_mask_trans16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_mask_trans16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));

#ifndef SCAN_EXPORT
AL_FUNC(void, _poly_scanline_grgb16x, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_lit16x, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_lit16x, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_mask_lit16x, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_mask_lit16x, (uintptr_t addr, int w, POLYGON_SEGMENT *info));

AL_FUNC(void, _poly_scanline_ptex_lit16d, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_mask_lit16d, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
#endif

AL_FUNC(void, _poly_zbuf_flat16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_grgb16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex_mask16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex_mask16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex_lit16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex_lit16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex_mask_lit16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex_mask_lit16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex_trans16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex_trans16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex_mask_trans16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex_mask_trans16, (uintptr_t addr, int w, POLYGON_SEGMENT *info));

#endif

#ifdef ALLEGRO_COLOR24

AL_FUNC(void, _poly_scanline_grgb24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_mask24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_mask24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_lit24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_lit24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_mask_lit24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_mask_lit24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_trans24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_trans24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_mask_trans24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_mask_trans24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));

#ifndef SCAN_EXPORT
AL_FUNC(void, _poly_scanline_grgb24x, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_lit24x, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_lit24x, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_mask_lit24x, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_mask_lit24x, (uintptr_t addr, int w, POLYGON_SEGMENT *info));

AL_FUNC(void, _poly_scanline_ptex_lit24d, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_mask_lit24d, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
#endif

AL_FUNC(void, _poly_zbuf_flat24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_grgb24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex_mask24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex_mask24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex_lit24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex_lit24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex_mask_lit24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex_mask_lit24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex_trans24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex_trans24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex_mask_trans24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex_mask_trans24, (uintptr_t addr, int w, POLYGON_SEGMENT *info));

#endif

#ifdef ALLEGRO_COLOR32

AL_FUNC(void, _poly_scanline_grgb32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_mask32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_mask32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_lit32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_lit32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_mask_lit32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_mask_lit32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_trans32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_trans32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_mask_trans32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_mask_trans32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));

#ifndef SCAN_EXPORT
AL_FUNC(void, _poly_scanline_grgb32x, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_lit32x, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_lit32x, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_atex_mask_lit32x, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_mask_lit32x, (uintptr_t addr, int w, POLYGON_SEGMENT *info));

AL_FUNC(void, _poly_scanline_ptex_lit32d, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_scanline_ptex_mask_lit32d, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
#endif

AL_FUNC(void, _poly_zbuf_flat32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_grgb32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex_mask32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex_mask32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex_lit32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex_lit32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex_mask_lit32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex_mask_lit32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex_trans32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex_trans32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_atex_mask_trans32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));
AL_FUNC(void, _poly_zbuf_ptex_mask_trans32, (uintptr_t addr, int w, POLYGON_SEGMENT *info));

#endif

#define OLD_FONT_SIZE 95
#define LESS_OLD_FONT_SIZE 224

/* various libc stuff */
AL_FUNC(void *, _al_sane_realloc, (void *ptr, size_t size));
AL_FUNC(char *, _al_sane_strncpy, (char *dest, const char *src, size_t n));

#define _AL_RAND_MAX 0xFFFF
AL_FUNC(void, _al_srand, (int seed));
AL_FUNC(int, _al_rand, (void));

/*
 * Drawing
 */

#define DRAW_MODE_SOLID 0 /* flags for drawing_mode() */
#define DRAW_MODE_XOR 1
#define DRAW_MODE_COPY_PATTERN 2
#define DRAW_MODE_SOLID_PATTERN 3
#define DRAW_MODE_MASKED_PATTERN 4
#define DRAW_MODE_TRANS 5

AL_FUNC(void, drawing_mode, (int mode, struct BITMAP *pattern, int x_anchor, int y_anchor));
AL_FUNC(void, xor_mode, (int on));
AL_FUNC(void, solid_mode, (void));
AL_FUNC(void, do_line, (struct BITMAP * bmp, int x1, int y_1, int x2, int y2, int d, AL_METHOD(void, proc, (struct BITMAP *, int, int, int))));
AL_FUNC(void, _soft_triangle, (struct BITMAP * bmp, int x1, int y_1, int x2, int y2, int x3, int y3, int color));
AL_FUNC(void, _soft_polygon, (struct BITMAP * bmp, int vertices, AL_CONST int *points, int color));
AL_FUNC(void, _soft_rect, (struct BITMAP * bmp, int x1, int y_1, int x2, int y2, int color));
AL_FUNC(void, do_circle, (struct BITMAP * bmp, int x, int y, int radius, int d, AL_METHOD(void, proc, (struct BITMAP *, int, int, int))));
AL_FUNC(void, _soft_circle, (struct BITMAP * bmp, int x, int y, int radius, int color));
AL_FUNC(void, _soft_circlefill, (struct BITMAP * bmp, int x, int y, int radius, int color));
AL_FUNC(void, do_ellipse, (struct BITMAP * bmp, int x, int y, int rx, int ry, int d, AL_METHOD(void, proc, (struct BITMAP *, int, int, int))));
AL_FUNC(void, _soft_ellipse, (struct BITMAP * bmp, int x, int y, int rx, int ry, int color));
AL_FUNC(void, _soft_ellipsefill, (struct BITMAP * bmp, int x, int y, int rx, int ry, int color));
AL_FUNC(void, do_arc, (struct BITMAP * bmp, int x, int y, fixed ang1, fixed ang2, int r, int d, AL_METHOD(void, proc, (struct BITMAP *, int, int, int))));
AL_FUNC(void, _soft_arc, (struct BITMAP * bmp, int x, int y, fixed ang1, fixed ang2, int r, int color));
AL_FUNC(void, calc_spline, (AL_CONST int points[8], int npts, int *x, int *y));
AL_FUNC(void, _soft_spline, (struct BITMAP * bmp, AL_CONST int points[8], int color));
AL_FUNC(void, _soft_floodfill, (struct BITMAP * bmp, int x, int y, int color));
AL_FUNC(void, blit, (struct BITMAP * source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
AL_FUNC(void, masked_blit, (struct BITMAP * source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height));
AL_FUNC(void, stretch_blit, (struct BITMAP * s, struct BITMAP *d, int s_x, int s_y, int s_w, int s_h, int d_x, int d_y, int d_w, int d_h));
AL_FUNC(void, masked_stretch_blit, (struct BITMAP * s, struct BITMAP *d, int s_x, int s_y, int s_w, int s_h, int d_x, int d_y, int d_w, int d_h));
AL_FUNC(void, stretch_sprite, (struct BITMAP * bmp, struct BITMAP *sprite, int x, int y, int w, int h));
AL_FUNC(void, _soft_draw_gouraud_sprite, (struct BITMAP * bmp, struct BITMAP *sprite, int x, int y, int c1, int c2, int c3, int c4));

/* rotate+trans */
AL_FUNC(void, rotate_sprite_trans, (BITMAP * bmp, BITMAP *sprite, int x, int y, fixed angle));
AL_FUNC(void, rotate_sprite_v_flip_trans, (BITMAP * bmp, BITMAP *sprite, int x, int y, fixed angle));
AL_FUNC(void, rotate_scaled_sprite_trans, (BITMAP * bmp, BITMAP *sprite, int x, int y, fixed angle, fixed scale));
AL_FUNC(void, rotate_scaled_sprite_v_flip_trans, (BITMAP * bmp, BITMAP *sprite, int x, int y, fixed angle, fixed scale));
AL_FUNC(void, pivot_sprite_trans, (BITMAP * bmp, BITMAP *sprite, int x, int y, int cx, int cy, fixed angle));
AL_FUNC(void, pivot_sprite_v_flip_trans, (BITMAP * bmp, BITMAP *sprite, int x, int y, int cx, int cy, fixed angle));
AL_FUNC(void, pivot_scaled_sprite_trans, (BITMAP * bmp, BITMAP *sprite, int x, int y, int cx, int cy, fixed angle, fixed scale));
AL_FUNC(void, pivot_scaled_sprite_v_flip_trans, (BITMAP * bmp, BITMAP *sprite, int x, int y, int cx, int cy, fixed angle, fixed scale));
/* rotate+lit */
AL_FUNC(void, rotate_sprite_lit, (BITMAP * bmp, BITMAP *sprite, int x, int y, fixed angle, int color));
AL_FUNC(void, rotate_sprite_v_flip_lit, (BITMAP * bmp, BITMAP *sprite, int x, int y, fixed angle, int color));
AL_FUNC(void, rotate_scaled_sprite_lit, (BITMAP * bmp, BITMAP *sprite, int x, int y, fixed angle, fixed scale, int color));
AL_FUNC(void, rotate_scaled_sprite_v_flip_lit, (BITMAP * bmp, BITMAP *sprite, int x, int y, fixed angle, fixed scale, int color));
AL_FUNC(void, pivot_sprite_lit, (BITMAP * bmp, BITMAP *sprite, int x, int y, int cx, int cy, fixed angle, int color));
AL_FUNC(void, pivot_sprite_v_flip_lit, (BITMAP * bmp, BITMAP *sprite, int x, int y, int cx, int cy, fixed angle, int color));
AL_FUNC(void, pivot_scaled_sprite_lit, (BITMAP * bmp, BITMAP *sprite, int x, int y, int cx, int cy, fixed angle, fixed scale, int color));
AL_FUNC(void, pivot_scaled_sprite_v_flip_lit, (BITMAP * bmp, BITMAP *sprite, int x, int y, int cx, int cy, fixed angle, fixed scale, int color));

AL_INLINE(int, getpixel, (BITMAP * bmp, int x, int y), {
	ASSERT(bmp);
	return bmp->vtable->getpixel(bmp, x, y);
})

AL_INLINE(void, putpixel, (BITMAP * bmp, int x, int y, int color), {
	ASSERT(bmp);
	bmp->vtable->putpixel(bmp, x, y, color);
})

AL_INLINE(void, _allegro_vline, (BITMAP * bmp, int x, int y_1, int y2, int color), {
	ASSERT(bmp);
	bmp->vtable->vline(bmp, x, y_1, y2, color);
})

AL_INLINE(void, _allegro_hline, (BITMAP * bmp, int x1, int y, int x2, int color), {
	ASSERT(bmp);
	bmp->vtable->hline(bmp, x1, y, x2, color);
})

/* The curses API also contains functions called vline and hline so we have
 * called our functions _allegro_vline and _allegro_hline.  User programs
 * should use the vline/hline aliases as they are the official names.
 */
#ifndef ALLEGRO_NO_VHLINE_ALIAS
AL_ALIAS_VOID_RET(vline(BITMAP *bmp, int x, int y_1, int y2, int color), _allegro_vline(bmp, x, y_1, y2, color))
AL_ALIAS_VOID_RET(hline(BITMAP *bmp, int x1, int y, int x2, int color), _allegro_hline(bmp, x1, y, x2, color))
#endif

AL_INLINE(void, line, (BITMAP * bmp, int x1, int y_1, int x2, int y2, int color), {
	ASSERT(bmp);
	bmp->vtable->line(bmp, x1, y_1, x2, y2, color);
})

AL_INLINE(void, fastline, (BITMAP * bmp, int x1, int y_1, int x2, int y2, int color), {
	ASSERT(bmp);
	bmp->vtable->fastline(bmp, x1, y_1, x2, y2, color);
})

AL_INLINE(void, rectfill, (BITMAP * bmp, int x1, int y_1, int x2, int y2, int color), {
	ASSERT(bmp);
	bmp->vtable->rectfill(bmp, x1, y_1, x2, y2, color);
})

AL_INLINE(void, triangle, (BITMAP * bmp, int x1, int y_1, int x2, int y2, int x3, int y3, int color), {
	ASSERT(bmp);
	bmp->vtable->triangle(bmp, x1, y_1, x2, y2, x3, y3, color);
})

AL_INLINE(void, polygon, (BITMAP * bmp, int vertices, AL_CONST int *points, int color), {
	ASSERT(bmp);
	bmp->vtable->polygon(bmp, vertices, points, color);
})

AL_INLINE(void, rect, (BITMAP * bmp, int x1, int y_1, int x2, int y2, int color), {
	ASSERT(bmp);
	bmp->vtable->rect(bmp, x1, y_1, x2, y2, color);
})

AL_INLINE(void, circle, (BITMAP * bmp, int x, int y, int radius, int color), {
	ASSERT(bmp);
	bmp->vtable->circle(bmp, x, y, radius, color);
})

AL_INLINE(void, circlefill, (BITMAP * bmp, int x, int y, int radius, int color), {
	ASSERT(bmp);
	bmp->vtable->circlefill(bmp, x, y, radius, color);
})

AL_INLINE(void, ellipse, (BITMAP * bmp, int x, int y, int rx, int ry, int color), {
	ASSERT(bmp);
	bmp->vtable->ellipse(bmp, x, y, rx, ry, color);
})

AL_INLINE(void, ellipsefill, (BITMAP * bmp, int x, int y, int rx, int ry, int color), {
	ASSERT(bmp);
	bmp->vtable->ellipsefill(bmp, x, y, rx, ry, color);
})

AL_INLINE(void, arc, (BITMAP * bmp, int x, int y, fixed ang1, fixed ang2, int r, int color), {
	ASSERT(bmp);
	bmp->vtable->arc(bmp, x, y, ang1, ang2, r, color);
})

AL_INLINE(void, spline, (BITMAP * bmp, AL_CONST int points[8], int color), {
	ASSERT(bmp);
	bmp->vtable->spline(bmp, points, color);
})

AL_INLINE(void, floodfill, (BITMAP * bmp, int x, int y, int color), {
	ASSERT(bmp);
	bmp->vtable->floodfill(bmp, x, y, color);
})

AL_INLINE(void, polygon3d, (BITMAP * bmp, int type, BITMAP *texture, int vc, V3D *vtx[]), {
	ASSERT(bmp);
	bmp->vtable->polygon3d(bmp, type, texture, vc, vtx);
})

AL_INLINE(void, polygon3d_f, (BITMAP * bmp, int type, BITMAP *texture, int vc, V3D_f *vtx[]), {
	ASSERT(bmp);
	bmp->vtable->polygon3d_f(bmp, type, texture, vc, vtx);
})

AL_INLINE(void, triangle3d, (BITMAP * bmp, int type, BITMAP *texture, V3D *v1, V3D *v2, V3D *v3), {
	ASSERT(bmp);
	bmp->vtable->triangle3d(bmp, type, texture, v1, v2, v3);
})

AL_INLINE(void, triangle3d_f, (BITMAP * bmp, int type, BITMAP *texture, V3D_f *v1, V3D_f *v2, V3D_f *v3), {
	ASSERT(bmp);
	bmp->vtable->triangle3d_f(bmp, type, texture, v1, v2, v3);
})

AL_INLINE(void, quad3d, (BITMAP * bmp, int type, BITMAP *texture, V3D *v1, V3D *v2, V3D *v3, V3D *v4), {
	ASSERT(bmp);
	bmp->vtable->quad3d(bmp, type, texture, v1, v2, v3, v4);
})

AL_INLINE(void, quad3d_f, (BITMAP * bmp, int type, BITMAP *texture, V3D_f *v1, V3D_f *v2, V3D_f *v3, V3D_f *v4), {
	ASSERT(bmp);
	bmp->vtable->quad3d_f(bmp, type, texture, v1, v2, v3, v4);
})

AL_INLINE(void, draw_sprite, (BITMAP * bmp, BITMAP *sprite, int x, int y), {
	ASSERT(bmp);
	ASSERT(sprite);
	if (sprite->vtable->color_depth == 8) {
		bmp->vtable->draw_256_sprite(bmp, sprite, x, y);
	} else {
		ASSERT(bmp->vtable->color_depth == sprite->vtable->color_depth);
		bmp->vtable->draw_sprite(bmp, sprite, x, y);
	}
})

AL_INLINE(void, draw_sprite_ex, (BITMAP * bmp, BITMAP *sprite, int x, int y, int mode, int flip), {
	ASSERT(bmp);
	ASSERT(sprite);
	if (mode == DRAW_SPRITE_TRANS) {
		ASSERT((bmp->vtable->color_depth == sprite->vtable->color_depth) ||
				(sprite->vtable->color_depth == 32) ||
				((sprite->vtable->color_depth == 8) &&
						(bmp->vtable->color_depth == 32)));
		bmp->vtable->draw_sprite_ex(bmp, sprite, x, y, mode, flip);
	} else {
		ASSERT(bmp->vtable->color_depth == sprite->vtable->color_depth);
		bmp->vtable->draw_sprite_ex(bmp, sprite, x, y, mode, flip);
	}
})

AL_INLINE(void, draw_sprite_v_flip, (BITMAP * bmp, BITMAP *sprite, int x, int y), {
	ASSERT(bmp);
	ASSERT(sprite);
	ASSERT(bmp->vtable->color_depth == sprite->vtable->color_depth);
	bmp->vtable->draw_sprite_v_flip(bmp, sprite, x, y);
})

AL_INLINE(void, draw_sprite_h_flip, (BITMAP * bmp, BITMAP *sprite, int x, int y), {
	ASSERT(bmp);
	ASSERT(sprite);
	ASSERT(bmp->vtable->color_depth == sprite->vtable->color_depth);
	bmp->vtable->draw_sprite_h_flip(bmp, sprite, x, y);
})

AL_INLINE(void, draw_sprite_vh_flip, (BITMAP * bmp, BITMAP *sprite, int x, int y), {
	ASSERT(bmp);
	ASSERT(sprite);
	ASSERT(bmp->vtable->color_depth == sprite->vtable->color_depth)
	bmp->vtable->draw_sprite_vh_flip(bmp, sprite, x, y);
})

AL_INLINE(void, draw_trans_sprite, (BITMAP * bmp, BITMAP *sprite, int x, int y), {
	ASSERT(bmp);
	ASSERT(sprite);
	if (sprite->vtable->color_depth == 32) {
		ASSERT(bmp->vtable->draw_trans_rgba_sprite);
		bmp->vtable->draw_trans_rgba_sprite(bmp, sprite, x, y);
	} else {
		ASSERT((bmp->vtable->color_depth == sprite->vtable->color_depth) ||
				((bmp->vtable->color_depth == 32) &&
						(sprite->vtable->color_depth == 8)));
		bmp->vtable->draw_trans_sprite(bmp, sprite, x, y);
	}
})

AL_INLINE(void, draw_lit_sprite, (BITMAP * bmp, BITMAP *sprite, int x, int y, int color), {
	ASSERT(bmp);
	ASSERT(sprite);
	ASSERT(bmp->vtable->color_depth == sprite->vtable->color_depth);
	bmp->vtable->draw_lit_sprite(bmp, sprite, x, y, color);
})

AL_INLINE(void, draw_gouraud_sprite, (BITMAP * bmp, BITMAP *sprite, int x, int y, int c1, int c2, int c3, int c4), {
	ASSERT(bmp);
	ASSERT(sprite);
	ASSERT(bmp->vtable->color_depth == sprite->vtable->color_depth);

	bmp->vtable->draw_gouraud_sprite(bmp, sprite, x, y, c1, c2, c3, c4);
})

AL_INLINE(void, draw_character_ex, (BITMAP * bmp, BITMAP *sprite, int x, int y, int color, int bg), {
	ASSERT(bmp);
	ASSERT(sprite);
	ASSERT(sprite->vtable->color_depth == 8);
	bmp->vtable->draw_character(bmp, sprite, x, y, color, bg);
})

AL_INLINE(void, rotate_sprite, (BITMAP * bmp, BITMAP *sprite, int x, int y, fixed angle), {
	ASSERT(bmp);
	ASSERT(sprite);
	bmp->vtable->pivot_scaled_sprite_flip(bmp, sprite, (x << 16) + (sprite->w * 0x10000) / 2,
			(y << 16) + (sprite->h * 0x10000) / 2,
			sprite->w << 15, sprite->h << 15,
			angle, 0x10000, FALSE);
})

AL_INLINE(void, rotate_sprite_v_flip, (BITMAP * bmp, BITMAP *sprite, int x, int y, fixed angle), {
	ASSERT(bmp);
	ASSERT(sprite);
	bmp->vtable->pivot_scaled_sprite_flip(bmp, sprite, (x << 16) + (sprite->w * 0x10000) / 2,
			(y << 16) + (sprite->h * 0x10000) / 2,
			sprite->w << 15, sprite->h << 15,
			angle, 0x10000, TRUE);
})

AL_INLINE(void, rotate_scaled_sprite, (BITMAP * bmp, BITMAP *sprite, int x, int y, fixed angle, fixed scale), {
	ASSERT(bmp);
	ASSERT(sprite);
	bmp->vtable->pivot_scaled_sprite_flip(bmp, sprite, (x << 16) + (sprite->w * scale) / 2,
			(y << 16) + (sprite->h * scale) / 2,
			sprite->w << 15, sprite->h << 15,
			angle, scale, FALSE);
})

AL_INLINE(void, rotate_scaled_sprite_v_flip, (BITMAP * bmp, BITMAP *sprite, int x, int y, fixed angle, fixed scale), {
	ASSERT(bmp);
	ASSERT(sprite);
	bmp->vtable->pivot_scaled_sprite_flip(bmp, sprite, (x << 16) + (sprite->w * scale) / 2,
			(y << 16) + (sprite->h * scale) / 2,
			sprite->w << 15, sprite->h << 15,
			angle, scale, TRUE);
})

AL_INLINE(void, pivot_sprite, (BITMAP * bmp, BITMAP *sprite, int x, int y, int cx, int cy, fixed angle), {
	ASSERT(bmp);
	ASSERT(sprite);
	bmp->vtable->pivot_scaled_sprite_flip(bmp, sprite, x << 16, y << 16, cx << 16, cy << 16, angle, 0x10000, FALSE);
})

AL_INLINE(void, pivot_sprite_v_flip, (BITMAP * bmp, BITMAP *sprite, int x, int y, int cx, int cy, fixed angle), {
	ASSERT(bmp);
	ASSERT(sprite);

	bmp->vtable->pivot_scaled_sprite_flip(bmp, sprite, x << 16, y << 16, cx << 16, cy << 16, angle, 0x10000, TRUE);
})

AL_INLINE(void, pivot_scaled_sprite, (BITMAP * bmp, BITMAP *sprite, int x, int y, int cx, int cy, fixed angle, fixed scale), {
	ASSERT(bmp);
	ASSERT(sprite);
	bmp->vtable->pivot_scaled_sprite_flip(bmp, sprite, x << 16, y << 16, cx << 16, cy << 16, angle, scale, FALSE);
})

AL_INLINE(void, pivot_scaled_sprite_v_flip, (BITMAP * bmp, BITMAP *sprite, int x, int y, int cx, int cy, fixed angle, fixed scale), {
	ASSERT(bmp);
	ASSERT(sprite);
	bmp->vtable->pivot_scaled_sprite_flip(bmp, sprite, x << 16, y << 16, cx << 16, cy << 16, angle, scale, TRUE);
})

AL_INLINE(void, _putpixel, (BITMAP * bmp, int x, int y, int color), {
	uintptr_t addr = bmp_write_line(bmp, y);
	bmp_write8(addr + x, color);
})

AL_INLINE(int, _getpixel, (BITMAP * bmp, int x, int y), {
	uintptr_t addr = bmp_read_line(bmp, y);
	int c = bmp_read8(addr + x);
	return c;
})

AL_INLINE(void, _putpixel15, (BITMAP * bmp, int x, int y, int color), {
	uintptr_t addr = bmp_write_line(bmp, y);
	bmp_write15(addr + x * sizeof(short), color);
})

AL_INLINE(int, _getpixel15, (BITMAP * bmp, int x, int y), {
	uintptr_t addr = bmp_read_line(bmp, y);
	int c = bmp_read15(addr + x * sizeof(short));
	return c;
})

AL_INLINE(void, _putpixel16, (BITMAP * bmp, int x, int y, int color), {
	uintptr_t addr = bmp_write_line(bmp, y);
	bmp_write16(addr + x * sizeof(short), color);
})

AL_INLINE(int, _getpixel16, (BITMAP * bmp, int x, int y), {
	uintptr_t addr = bmp_read_line(bmp, y);
	int c = bmp_read16(addr + x * sizeof(short));
	return c;
})

AL_INLINE(void, _putpixel24, (BITMAP * bmp, int x, int y, int color), {
	uintptr_t addr = bmp_write_line(bmp, y);
	bmp_write24(addr + x * 3, color);
})

AL_INLINE(int, _getpixel24, (BITMAP * bmp, int x, int y), {
	uintptr_t addr = bmp_read_line(bmp, y);
	int c = bmp_read24(addr + x * 3);
	return c;
})

AL_INLINE(void, _putpixel32, (BITMAP * bmp, int x, int y, int color), {
	uintptr_t addr = bmp_write_line(bmp, y);
	bmp_write32(addr + x * sizeof(int32_t), color);
})

AL_INLINE(int, _getpixel32, (BITMAP * bmp, int x, int y), {
	uintptr_t addr = bmp_read_line(bmp, y);
	int c = bmp_read32(addr + x * sizeof(int32_t));
	return c;
})

/*
 * RLE sprites.
 * ============
 */

typedef struct RLE_SPRITE /* a RLE compressed sprite */
{
	int w, h; /* width and height in pixels */
	int color_depth; /* color depth of the image */
	int size; /* size of sprite data in bytes */
	ZERO_SIZE_ARRAY(signed char, dat);
} RLE_SPRITE;

AL_FUNC(RLE_SPRITE *, get_rle_sprite, (struct BITMAP * bitmap));
AL_FUNC(void, destroy_rle_sprite, (RLE_SPRITE * sprite));

AL_INLINE(void, draw_rle_sprite, (BITMAP * bmp, AL_CONST RLE_SPRITE *sprite, int x, int y), {
	ASSERT(bmp);
	ASSERT(sprite);
	ASSERT(bmp->vtable->color_depth == sprite->color_depth);
	bmp->vtable->draw_rle_sprite(bmp, sprite, x, y);
})

/*
 * Text output routines.
 * =====================
 */

struct BITMAP;
struct FONT_VTABLE;
struct FONT;

AL_VAR(struct FONT *, font);
AL_VAR(int, allegro_404_char);
AL_FUNC(void, textout_ex, (struct BITMAP * bmp, AL_CONST struct FONT *f, AL_CONST char *str, int x, int y, int color, int bg));
AL_FUNC(void, textout_centre_ex, (struct BITMAP * bmp, AL_CONST struct FONT *f, AL_CONST char *str, int x, int y, int color, int bg));
AL_FUNC(void, textout_right_ex, (struct BITMAP * bmp, AL_CONST struct FONT *f, AL_CONST char *str, int x, int y, int color, int bg));
AL_FUNC(void, textout_justify_ex, (struct BITMAP * bmp, AL_CONST struct FONT *f, AL_CONST char *str, int x1, int x2, int y, int diff, int color, int bg));
AL_PRINTFUNC(void, textprintf_ex, (struct BITMAP * bmp, AL_CONST struct FONT *f, int x, int y, int color, int bg, AL_CONST char *format, ...), 7, 8);
AL_PRINTFUNC(void, textprintf_centre_ex, (struct BITMAP * bmp, AL_CONST struct FONT *f, int x, int y, int color, int bg, AL_CONST char *format, ...), 7, 8);
AL_PRINTFUNC(void, textprintf_right_ex, (struct BITMAP * bmp, AL_CONST struct FONT *f, int x, int y, int color, int bg, AL_CONST char *format, ...), 7, 8);
AL_PRINTFUNC(void, textprintf_justify_ex, (struct BITMAP * bmp, AL_CONST struct FONT *f, int x1, int x2, int y, int diff, int color, int bg, AL_CONST char *format, ...), 9, 10);
AL_FUNC(int, text_length, (AL_CONST struct FONT * f, AL_CONST char *str));
AL_FUNC(int, text_height, (AL_CONST struct FONT * f));
AL_FUNC(void, destroy_font, (struct FONT * f));

#ifdef __cplusplus
}
#endif

/*
 * Fix class C++
 * =============
 */

#ifdef __cplusplus

class fix /* C++ wrapper for the fixed point routines */
{
public:
	fixed v;

	fix() :
			v(0) {}
	fix(const fix &x) :
			v(x.v) {}
	explicit fix(const int x) :
			v(itofix(x)) {}
	explicit fix(const long x) :
			v(itofix(x)) {}
	explicit fix(const unsigned int x) :
			v(itofix(x)) {}
	explicit fix(const unsigned long x) :
			v(itofix(x)) {}
	explicit fix(const float x) :
			v(ftofix(x)) {}
	explicit fix(const double x) :
			v(ftofix(x)) {}

	operator int() const { return fixtoi(v); }
	operator long() const { return fixtoi(v); }
	operator unsigned int() const { return fixtoi(v); }
	operator unsigned long() const { return fixtoi(v); }
	operator float() const { return fixtof(v); }
	operator double() const { return fixtof(v); }

	fix &operator=(const fix &x) {
		v = x.v;
		return *this;
	}
	fix &operator=(const int x) {
		v = itofix(x);
		return *this;
	}
	fix &operator=(const long x) {
		v = itofix(x);
		return *this;
	}
	fix &operator=(const unsigned int x) {
		v = itofix(x);
		return *this;
	}
	fix &operator=(const unsigned long x) {
		v = itofix(x);
		return *this;
	}
	fix &operator=(const float x) {
		v = ftofix(x);
		return *this;
	}
	fix &operator=(const double x) {
		v = ftofix(x);
		return *this;
	}

	fix &operator+=(const fix x) {
		v += x.v;
		return *this;
	}
	fix &operator+=(const int x) {
		v += itofix(x);
		return *this;
	}
	fix &operator+=(const long x) {
		v += itofix(x);
		return *this;
	}
	fix &operator+=(const float x) {
		v += ftofix(x);
		return *this;
	}
	fix &operator+=(const double x) {
		v += ftofix(x);
		return *this;
	}

	fix &operator-=(const fix x) {
		v -= x.v;
		return *this;
	}
	fix &operator-=(const int x) {
		v -= itofix(x);
		return *this;
	}
	fix &operator-=(const long x) {
		v -= itofix(x);
		return *this;
	}
	fix &operator-=(const float x) {
		v -= ftofix(x);
		return *this;
	}
	fix &operator-=(const double x) {
		v -= ftofix(x);
		return *this;
	}

	fix &operator*=(const fix x) {
		v = fixmul(v, x.v);
		return *this;
	}
	fix &operator*=(const int x) {
		v *= x;
		return *this;
	}
	fix &operator*=(const long x) {
		v *= x;
		return *this;
	}
	fix &operator*=(const float x) {
		v = ftofix(fixtof(v) * x);
		return *this;
	}
	fix &operator*=(const double x) {
		v = ftofix(fixtof(v) * x);
		return *this;
	}

	fix &operator/=(const fix x) {
		v = fixdiv(v, x.v);
		return *this;
	}
	fix &operator/=(const int x) {
		v /= x;
		return *this;
	}
	fix &operator/=(const long x) {
		v /= x;
		return *this;
	}
	fix &operator/=(const float x) {
		v = ftofix(fixtof(v) / x);
		return *this;
	}
	fix &operator/=(const double x) {
		v = ftofix(fixtof(v) / x);
		return *this;
	}

	fix &operator<<=(const int x) {
		v <<= x;
		return *this;
	}
	fix &operator>>=(const int x) {
		v >>= x;
		return *this;
	}

	fix &operator++() {
		v += itofix(1);
		return *this;
	}
	fix &operator--() {
		v -= itofix(1);
		return *this;
	}

	fix operator++(int) {
		fix t;
		t.v = v;
		v += itofix(1);
		return t;
	}
	fix operator--(int) {
		fix t;
		t.v = v;
		v -= itofix(1);
		return t;
	}

	fix operator-() const {
		fix t;
		t.v = -v;
		return t;
	}

	inline friend fix operator+(const fix x, const fix y);
	inline friend fix operator+(const fix x, const int y);
	inline friend fix operator+(const int x, const fix y);
	inline friend fix operator+(const fix x, const long y);
	inline friend fix operator+(const long x, const fix y);
	inline friend fix operator+(const fix x, const float y);
	inline friend fix operator+(const float x, const fix y);
	inline friend fix operator+(const fix x, const double y);
	inline friend fix operator+(const double x, const fix y);

	inline friend fix operator-(const fix x, const fix y);
	inline friend fix operator-(const fix x, const int y);
	inline friend fix operator-(const int x, const fix y);
	inline friend fix operator-(const fix x, const long y);
	inline friend fix operator-(const long x, const fix y);
	inline friend fix operator-(const fix x, const float y);
	inline friend fix operator-(const float x, const fix y);
	inline friend fix operator-(const fix x, const double y);
	inline friend fix operator-(const double x, const fix y);

	inline friend fix operator*(const fix x, const fix y);
	inline friend fix operator*(const fix x, const int y);
	inline friend fix operator*(const int x, const fix y);
	inline friend fix operator*(const fix x, const long y);
	inline friend fix operator*(const long x, const fix y);
	inline friend fix operator*(const fix x, const float y);
	inline friend fix operator*(const float x, const fix y);
	inline friend fix operator*(const fix x, const double y);
	inline friend fix operator*(const double x, const fix y);

	inline friend fix operator/(const fix x, const fix y);
	inline friend fix operator/(const fix x, const int y);
	inline friend fix operator/(const int x, const fix y);
	inline friend fix operator/(const fix x, const long y);
	inline friend fix operator/(const long x, const fix y);
	inline friend fix operator/(const fix x, const float y);
	inline friend fix operator/(const float x, const fix y);
	inline friend fix operator/(const fix x, const double y);
	inline friend fix operator/(const double x, const fix y);

	inline friend fix operator<<(const fix x, const int y);
	inline friend fix operator>>(const fix x, const int y);

	inline friend int operator==(const fix x, const fix y);
	inline friend int operator==(const fix x, const int y);
	inline friend int operator==(const int x, const fix y);
	inline friend int operator==(const fix x, const long y);
	inline friend int operator==(const long x, const fix y);
	inline friend int operator==(const fix x, const float y);
	inline friend int operator==(const float x, const fix y);
	inline friend int operator==(const fix x, const double y);
	inline friend int operator==(const double x, const fix y);

	inline friend int operator!=(const fix x, const fix y);
	inline friend int operator!=(const fix x, const int y);
	inline friend int operator!=(const int x, const fix y);
	inline friend int operator!=(const fix x, const long y);
	inline friend int operator!=(const long x, const fix y);
	inline friend int operator!=(const fix x, const float y);
	inline friend int operator!=(const float x, const fix y);
	inline friend int operator!=(const fix x, const double y);
	inline friend int operator!=(const double x, const fix y);

	inline friend int operator<(const fix x, const fix y);
	inline friend int operator<(const fix x, const int y);
	inline friend int operator<(const int x, const fix y);
	inline friend int operator<(const fix x, const long y);
	inline friend int operator<(const long x, const fix y);
	inline friend int operator<(const fix x, const float y);
	inline friend int operator<(const float x, const fix y);
	inline friend int operator<(const fix x, const double y);
	inline friend int operator<(const double x, const fix y);

	inline friend int operator>(const fix x, const fix y);
	inline friend int operator>(const fix x, const int y);
	inline friend int operator>(const int x, const fix y);
	inline friend int operator>(const fix x, const long y);
	inline friend int operator>(const long x, const fix y);
	inline friend int operator>(const fix x, const float y);
	inline friend int operator>(const float x, const fix y);
	inline friend int operator>(const fix x, const double y);
	inline friend int operator>(const double x, const fix y);

	inline friend int operator<=(const fix x, const fix y);
	inline friend int operator<=(const fix x, const int y);
	inline friend int operator<=(const int x, const fix y);
	inline friend int operator<=(const fix x, const long y);
	inline friend int operator<=(const long x, const fix y);
	inline friend int operator<=(const fix x, const float y);
	inline friend int operator<=(const float x, const fix y);
	inline friend int operator<=(const fix x, const double y);
	inline friend int operator<=(const double x, const fix y);

	inline friend int operator>=(const fix x, const fix y);
	inline friend int operator>=(const fix x, const int y);
	inline friend int operator>=(const int x, const fix y);
	inline friend int operator>=(const fix x, const long y);
	inline friend int operator>=(const long x, const fix y);
	inline friend int operator>=(const fix x, const float y);
	inline friend int operator>=(const float x, const fix y);
	inline friend int operator>=(const fix x, const double y);
	inline friend int operator>=(const double x, const fix y);

	inline friend fix sqrt(fix x);
	inline friend fix cos(fix x);
	inline friend fix sin(fix x);
	inline friend fix tan(fix x);
	inline friend fix acos(fix x);
	inline friend fix asin(fix x);
	inline friend fix atan(fix x);
	inline friend fix atan2(fix x, fix y);
};

inline fix operator+(const fix x, const fix y) {
	fix t;
	t.v = x.v + y.v;
	return t;
}
inline fix operator+(const fix x, const int y) {
	fix t;
	t.v = x.v + itofix(y);
	return t;
}
inline fix operator+(const int x, const fix y) {
	fix t;
	t.v = itofix(x) + y.v;
	return t;
}
inline fix operator+(const fix x, const long y) {
	fix t;
	t.v = x.v + itofix(y);
	return t;
}
inline fix operator+(const long x, const fix y) {
	fix t;
	t.v = itofix(x) + y.v;
	return t;
}
inline fix operator+(const fix x, const float y) {
	fix t;
	t.v = x.v + ftofix(y);
	return t;
}
inline fix operator+(const float x, const fix y) {
	fix t;
	t.v = ftofix(x) + y.v;
	return t;
}
inline fix operator+(const fix x, const double y) {
	fix t;
	t.v = x.v + ftofix(y);
	return t;
}
inline fix operator+(const double x, const fix y) {
	fix t;
	t.v = ftofix(x) + y.v;
	return t;
}

inline fix operator-(const fix x, const fix y) {
	fix t;
	t.v = x.v - y.v;
	return t;
}
inline fix operator-(const fix x, const int y) {
	fix t;
	t.v = x.v - itofix(y);
	return t;
}
inline fix operator-(const int x, const fix y) {
	fix t;
	t.v = itofix(x) - y.v;
	return t;
}
inline fix operator-(const fix x, const long y) {
	fix t;
	t.v = x.v - itofix(y);
	return t;
}
inline fix operator-(const long x, const fix y) {
	fix t;
	t.v = itofix(x) - y.v;
	return t;
}
inline fix operator-(const fix x, const float y) {
	fix t;
	t.v = x.v - ftofix(y);
	return t;
}
inline fix operator-(const float x, const fix y) {
	fix t;
	t.v = ftofix(x) - y.v;
	return t;
}
inline fix operator-(const fix x, const double y) {
	fix t;
	t.v = x.v - ftofix(y);
	return t;
}
inline fix operator-(const double x, const fix y) {
	fix t;
	t.v = ftofix(x) - y.v;
	return t;
}

inline fix operator*(const fix x, const fix y) {
	fix t;
	t.v = fixmul(x.v, y.v);
	return t;
}
inline fix operator*(const fix x, const int y) {
	fix t;
	t.v = x.v * y;
	return t;
}
inline fix operator*(const int x, const fix y) {
	fix t;
	t.v = x * y.v;
	return t;
}
inline fix operator*(const fix x, const long y) {
	fix t;
	t.v = x.v * y;
	return t;
}
inline fix operator*(const long x, const fix y) {
	fix t;
	t.v = x * y.v;
	return t;
}
inline fix operator*(const fix x, const float y) {
	fix t;
	t.v = ftofix(fixtof(x.v) * y);
	return t;
}
inline fix operator*(const float x, const fix y) {
	fix t;
	t.v = ftofix(x * fixtof(y.v));
	return t;
}
inline fix operator*(const fix x, const double y) {
	fix t;
	t.v = ftofix(fixtof(x.v) * y);
	return t;
}
inline fix operator*(const double x, const fix y) {
	fix t;
	t.v = ftofix(x * fixtof(y.v));
	return t;
}

inline fix operator/(const fix x, const fix y) {
	fix t;
	t.v = fixdiv(x.v, y.v);
	return t;
}
inline fix operator/(const fix x, const int y) {
	fix t;
	t.v = x.v / y;
	return t;
}
inline fix operator/(const int x, const fix y) {
	fix t;
	t.v = fixdiv(itofix(x), y.v);
	return t;
}
inline fix operator/(const fix x, const long y) {
	fix t;
	t.v = x.v / y;
	return t;
}
inline fix operator/(const long x, const fix y) {
	fix t;
	t.v = fixdiv(itofix(x), y.v);
	return t;
}
inline fix operator/(const fix x, const float y) {
	fix t;
	t.v = ftofix(fixtof(x.v) / y);
	return t;
}
inline fix operator/(const float x, const fix y) {
	fix t;
	t.v = ftofix(x / fixtof(y.v));
	return t;
}
inline fix operator/(const fix x, const double y) {
	fix t;
	t.v = ftofix(fixtof(x.v) / y);
	return t;
}
inline fix operator/(const double x, const fix y) {
	fix t;
	t.v = ftofix(x / fixtof(y.v));
	return t;
}

inline fix operator<<(const fix x, const int y) {
	fix t;
	t.v = x.v << y;
	return t;
}
inline fix operator>>(const fix x, const int y) {
	fix t;
	t.v = x.v >> y;
	return t;
}

inline int operator==(const fix x, const fix y) { return (x.v == y.v); }
inline int operator==(const fix x, const int y) { return (x.v == itofix(y)); }
inline int operator==(const int x, const fix y) { return (itofix(x) == y.v); }
inline int operator==(const fix x, const long y) { return (x.v == itofix(y)); }
inline int operator==(const long x, const fix y) { return (itofix(x) == y.v); }
inline int operator==(const fix x, const float y) { return (x.v == ftofix(y)); }
inline int operator==(const float x, const fix y) { return (ftofix(x) == y.v); }
inline int operator==(const fix x, const double y) { return (x.v == ftofix(y)); }
inline int operator==(const double x, const fix y) { return (ftofix(x) == y.v); }

inline int operator!=(const fix x, const fix y) { return (x.v != y.v); }
inline int operator!=(const fix x, const int y) { return (x.v != itofix(y)); }
inline int operator!=(const int x, const fix y) { return (itofix(x) != y.v); }
inline int operator!=(const fix x, const long y) { return (x.v != itofix(y)); }
inline int operator!=(const long x, const fix y) { return (itofix(x) != y.v); }
inline int operator!=(const fix x, const float y) { return (x.v != ftofix(y)); }
inline int operator!=(const float x, const fix y) { return (ftofix(x) != y.v); }
inline int operator!=(const fix x, const double y) { return (x.v != ftofix(y)); }
inline int operator!=(const double x, const fix y) { return (ftofix(x) != y.v); }

inline int operator<(const fix x, const fix y) { return (x.v < y.v); }
inline int operator<(const fix x, const int y) { return (x.v < itofix(y)); }
inline int operator<(const int x, const fix y) { return (itofix(x) < y.v); }
inline int operator<(const fix x, const long y) { return (x.v < itofix(y)); }
inline int operator<(const long x, const fix y) { return (itofix(x) < y.v); }
inline int operator<(const fix x, const float y) { return (x.v < ftofix(y)); }
inline int operator<(const float x, const fix y) { return (ftofix(x) < y.v); }
inline int operator<(const fix x, const double y) { return (x.v < ftofix(y)); }
inline int operator<(const double x, const fix y) { return (ftofix(x) < y.v); }

inline int operator>(const fix x, const fix y) { return (x.v > y.v); }
inline int operator>(const fix x, const int y) { return (x.v > itofix(y)); }
inline int operator>(const int x, const fix y) { return (itofix(x) > y.v); }
inline int operator>(const fix x, const long y) { return (x.v > itofix(y)); }
inline int operator>(const long x, const fix y) { return (itofix(x) > y.v); }
inline int operator>(const fix x, const float y) { return (x.v > ftofix(y)); }
inline int operator>(const float x, const fix y) { return (ftofix(x) > y.v); }
inline int operator>(const fix x, const double y) { return (x.v > ftofix(y)); }
inline int operator>(const double x, const fix y) { return (ftofix(x) > y.v); }

inline int operator<=(const fix x, const fix y) { return (x.v <= y.v); }
inline int operator<=(const fix x, const int y) { return (x.v <= itofix(y)); }
inline int operator<=(const int x, const fix y) { return (itofix(x) <= y.v); }
inline int operator<=(const fix x, const long y) { return (x.v <= itofix(y)); }
inline int operator<=(const long x, const fix y) { return (itofix(x) <= y.v); }
inline int operator<=(const fix x, const float y) { return (x.v <= ftofix(y)); }
inline int operator<=(const float x, const fix y) { return (ftofix(x) <= y.v); }
inline int operator<=(const fix x, const double y) { return (x.v <= ftofix(y)); }
inline int operator<=(const double x, const fix y) { return (ftofix(x) <= y.v); }

inline int operator>=(const fix x, const fix y) { return (x.v >= y.v); }
inline int operator>=(const fix x, const int y) { return (x.v >= itofix(y)); }
inline int operator>=(const int x, const fix y) { return (itofix(x) >= y.v); }
inline int operator>=(const fix x, const long y) { return (x.v >= itofix(y)); }
inline int operator>=(const long x, const fix y) { return (itofix(x) >= y.v); }
inline int operator>=(const fix x, const float y) { return (x.v >= ftofix(y)); }
inline int operator>=(const float x, const fix y) { return (ftofix(x) >= y.v); }
inline int operator>=(const fix x, const double y) { return (x.v >= ftofix(y)); }
inline int operator>=(const double x, const fix y) { return (ftofix(x) >= y.v); }

inline fix sqrt(fix x) {
	fix t;
	t.v = fixsqrt(x.v);
	return t;
}
inline fix cos(fix x) {
	fix t;
	t.v = fixcos(x.v);
	return t;
}
inline fix sin(fix x) {
	fix t;
	t.v = fixsin(x.v);
	return t;
}
inline fix tan(fix x) {
	fix t;
	t.v = fixtan(x.v);
	return t;
}
inline fix acos(fix x) {
	fix t;
	t.v = fixacos(x.v);
	return t;
}
inline fix asin(fix x) {
	fix t;
	t.v = fixasin(x.v);
	return t;
}
inline fix atan(fix x) {
	fix t;
	t.v = fixatan(x.v);
	return t;
}
inline fix atan2(fix x, fix y) {
	fix t;
	t.v = fixatan2(x.v, y.v);
	return t;
}

inline void get_translation_matrix(MATRIX *m, fix x, fix y, fix z) { get_translation_matrix(m, x.v, y.v, z.v); }
inline void get_scaling_matrix(MATRIX *m, fix x, fix y, fix z) { get_scaling_matrix(m, x.v, y.v, z.v); }

inline void get_x_rotate_matrix(MATRIX *m, fix r) { get_x_rotate_matrix(m, r.v); }
inline void get_y_rotate_matrix(MATRIX *m, fix r) { get_y_rotate_matrix(m, r.v); }
inline void get_z_rotate_matrix(MATRIX *m, fix r) { get_z_rotate_matrix(m, r.v); }
inline void get_rotation_matrix(MATRIX *m, fix x, fix y, fix z) { get_rotation_matrix(m, x.v, y.v, z.v); }

inline void get_align_matrix(MATRIX *m, fix xfront, fix yfront, fix zfront, fix xup, fix yup, fix zup) { get_align_matrix(m, xfront.v, yfront.v, zfront.v, xup.v, yup.v, zup.v); }

inline void get_vector_rotation_matrix(MATRIX *m, fix x, fix y, fix z, fix a) { get_vector_rotation_matrix(m, x.v, y.v, z.v, a.v); }

inline void get_transformation_matrix(MATRIX *m, fix scale, fix xrot, fix yrot, fix zrot, fix x, fix y, fix z) { get_transformation_matrix(m, scale.v, xrot.v, yrot.v, zrot.v, x.v, y.v, z.v); }

inline void get_camera_matrix(MATRIX *m, fix x, fix y, fix z, fix xfront, fix yfront, fix zfront, fix xup, fix yup, fix zup, fix fov, fix aspect) {
	get_camera_matrix(m, x.v, y.v, z.v, xfront.v, yfront.v, zfront.v, xup.v, yup.v, zup.v, fov.v, aspect.v);
}

inline void qtranslate_matrix(MATRIX *m, fix x, fix y, fix z) { qtranslate_matrix(m, x.v, y.v, z.v); }
inline void qscale_matrix(MATRIX *m, fix scale) { qscale_matrix(m, scale.v); }

inline fix vector_length(fix x, fix y, fix z) {
	fix t;
	t.v = vector_length(x.v, y.v, z.v);
	return t;
}

inline void normalize_vector(fix *x, fix *y, fix *z) { normalize_vector(&x->v, &y->v, &z->v); }
inline void cross_product(fix x1, fix y_1, fix z1, fix x2, fix y2, fix z2, fix *xout, fix *yout, fix *zout) { cross_product(x1.v, y_1.v, z1.v, x2.v, y2.v, z2.v, &xout->v, &yout->v, &zout->v); }

inline fix dot_product(fix x1, fix y_1, fix z1, fix x2, fix y2, fix z2) {
	fix t;
	t.v = dot_product(x1.v, y_1.v, z1.v, x2.v, y2.v, z2.v);
	return t;
}

inline void apply_matrix(MATRIX *m, fix x, fix y, fix z, fix *xout, fix *yout, fix *zout) { apply_matrix(m, x.v, y.v, z.v, &xout->v, &yout->v, &zout->v); }
inline void persp_project(fix x, fix y, fix z, fix *xout, fix *yout) { persp_project(x.v, y.v, z.v, &xout->v, &yout->v); }

#endif /* __cplusplus */

#endif /* ifndef AL_GFX_H */
