/*************************************************************************/
/*  _cpuinfo.h                                                           */
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

/// CPU feature detection for SDL.

#ifndef _cpuinfo_h_
#define _cpuinfo_h_

#include "_stdinc.h"

// Need to do this here because intrin.h has C++ code in it
// Visual Studio 2005 has a bug where intrin.h conflicts with winnt.h
#if defined(_MSC_VER) && (_MSC_VER >= 1500) && (defined(_M_IX86) || defined(_M_X64))
#ifdef __clang__
// As of Clang 11, '_m_prefetchw' is conflicting with the winnt.h's version,
// so we define the needed '_m_prefetch' here as a pseudo-header, until the issue is fixed.

#ifndef __PRFCHWINTRIN_H
#define __PRFCHWINTRIN_H

static __inline__ void __attribute__((__always_inline__, __nodebug__))
_m_prefetch(void *__P) {
	__builtin_prefetch(__P, 0, 3 /* _MM_HINT_T0 */);
}

#endif /* __PRFCHWINTRIN_H */
#endif /* __clang__ */
#include <intrin.h>
#ifndef _WIN64
#ifndef __MMX__
#define __MMX__
#endif
#ifndef __3dNOW__
#define __3dNOW__
#endif
#endif
#ifndef __SSE__
#define __SSE__
#endif
#ifndef __SSE2__
#define __SSE2__
#endif
#ifndef __SSE3__
#define __SSE3__
#endif
#elif defined(__MINGW64_VERSION_MAJOR)
#include <intrin.h>
#if !defined(SDL_DISABLE_ARM_NEON_H) && defined(__ARM_NEON)
#include <arm_neon.h>
#endif
#else
/* altivec.h redefining bool causes a number of problems, see bugs 3993 and 4392, so you need to explicitly define SDL_ENABLE_ALTIVEC_H to have it included. */
#if defined(HAVE_ALTIVEC_H) && defined(__ALTIVEC__) && !defined(__APPLE_ALTIVEC__) && defined(SDL_ENABLE_ALTIVEC_H)
#include <altivec.h>
#endif
#if !defined(SDL_DISABLE_ARM_NEON_H)
#if defined(__ARM_NEON)
#include <arm_neon.h>
#elif defined(__WINDOWS__) || defined(__WINRT__)
/* Visual Studio doesn't define __ARM_ARCH, but _M_ARM (if set, always 7), and _M_ARM64 (if set, always 1). */
#if defined(_M_ARM)
#include <arm_neon.h>
#include <armintr.h>
#define __ARM_NEON 1 /* Set __ARM_NEON so that it can be used elsewhere, at compile time */
#endif
#if defined(_M_ARM64)
#include <arm64_neon.h>
#include <arm64intr.h>
#define __ARM_NEON 1 /* Set __ARM_NEON so that it can be used elsewhere, at compile time */
#endif
#endif
#endif
#endif /* compiler version */

#if defined(__3dNOW__) && !defined(SDL_DISABLE_MM3DNOW_H)
#include <mm3dnow.h>
#endif
#if defined(HAVE_IMMINTRIN_H) && !defined(SDL_DISABLE_IMMINTRIN_H)
#include <immintrin.h>
#else
#if defined(__MMX__) && !defined(SDL_DISABLE_MMINTRIN_H)
#include <mmintrin.h>
#endif
#if defined(__SSE__) && !defined(SDL_DISABLE_XMMINTRIN_H)
#include <xmmintrin.h>
#endif
#if defined(__SSE2__) && !defined(SDL_DISABLE_EMMINTRIN_H)
#include <emmintrin.h>
#endif
#if defined(__SSE3__) && !defined(SDL_DISABLE_PMMINTRIN_H)
#include <pmmintrin.h>
#endif
#endif /* HAVE_IMMINTRIN_H */

#include "_begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

// This is a guess for the cacheline size used for padding.
// Most x86 processors have a 64 byte cache line.
// The 64-bit PowerPC processors have a 128 byte cache line.
// We'll use the larger value to be generally safe.
#define SDL_CACHELINE_SIZE 128

// Get the number of CPU cores available.
extern DECLSPEC int SDLCALL SDL_GetCPUCount(void);

// Determine the L1 cache line size of the CPU.
// This is useful for determining multi-threaded structure padding or SIMD
// prefetch sizes.
extern DECLSPEC int SDLCALL SDL_GetCPUCacheLineSize(void);

// Determine whether the CPU has the RDTSC instruction.
// This always returns false on CPUs that aren't using Intel instruction sets.
extern DECLSPEC SDL_bool SDLCALL SDL_HasRDTSC(void);

// Determine whether the CPU has AltiVec features.
// This always returns false on CPUs that aren't using PowerPC instruction sets.
extern DECLSPEC SDL_bool SDLCALL SDL_HasAltiVec(void);

// Determine whether the CPU has MMX features.
// This always returns false on CPUs that aren't using Intel instruction sets.
extern DECLSPEC SDL_bool SDLCALL SDL_HasMMX(void);

// Determine whether the CPU has 3DNow! features.
// This always returns false on CPUs that aren't using AMD instruction sets.
extern DECLSPEC SDL_bool SDLCALL SDL_Has3DNow(void);

// Determine whether the CPU has SSE features.
// This always returns false on CPUs that aren't using Intel instruction sets.
extern DECLSPEC SDL_bool SDLCALL SDL_HasSSE(void);

// Determine whether the CPU has SSE2 features.
// This always returns false on CPUs that aren't using Intel instruction sets.
extern DECLSPEC SDL_bool SDLCALL SDL_HasSSE2(void);

// Determine whether the CPU has SSE3 features.
// This always returns false on CPUs that aren't using Intel instruction sets.
extern DECLSPEC SDL_bool SDLCALL SDL_HasSSE3(void);

// Determine whether the CPU has SSE4.1 features.
//This always returns false on CPUs that aren't using Intel instruction sets.
extern DECLSPEC SDL_bool SDLCALL SDL_HasSSE41(void);

// Determine whether the CPU has SSE4.2 features.
// This always returns false on CPUs that aren't using Intel instruction sets.
extern DECLSPEC SDL_bool SDLCALL SDL_HasSSE42(void);

// Determine whether the CPU has AVX features.
// This always returns false on CPUs that aren't using Intel instruction sets.
extern DECLSPEC SDL_bool SDLCALL SDL_HasAVX(void);

// Determine whether the CPU has AVX2 features.
//This always returns false on CPUs that aren't using Intel instruction sets.
extern DECLSPEC SDL_bool SDLCALL SDL_HasAVX2(void);

// Determine whether the CPU has AVX-512F (foundation) features.
// This always returns false on CPUs that aren't using Intel instruction sets.
extern DECLSPEC SDL_bool SDLCALL SDL_HasAVX512F(void);

// Determine whether the CPU has ARM SIMD (ARMv6) features.
// This is different from ARM NEON, which is a different instruction set.
// This always returns false on CPUs that aren't using ARM instruction sets.
extern DECLSPEC SDL_bool SDLCALL SDL_HasARMSIMD(void);

// Determine whether the CPU has NEON (ARM SIMD) features.
// This always returns false on CPUs that aren't using ARM instruction sets.
extern DECLSPEC SDL_bool SDLCALL SDL_HasNEON(void);

// Report the alignment this system needs for SIMD allocations.
// This will return the minimum number of bytes to which a pointer must be
// aligned to be compatible with SIMD instructions on the current machine. For
// example, if the machine supports SSE only, it will return 16, but if it
// supports AVX-512F, it'll return 64 (etc). This only reports values for
// instruction sets SDL knows about, so if your SDL build doesn't have
// SDL_HasAVX512F(), then it might return 16 for the SSE support it sees and
// not 64 for the AVX-512 instructions that exist but SDL doesn't know about.
// Plan accordingly.
extern DECLSPEC size_t SDLCALL SDL_SIMDGetAlignment(void);

// Allocate memory in a SIMD-friendly way.
// This will allocate a block of memory that is suitable for use with SIMD
// instructions. Specifically, it will be properly aligned and padded for the
// system's supported vector instructions.
// The memory returned will be padded such that it is safe to read or write an
// incomplete vector at the end of the memory block. This can be useful so you
// don't have to drop back to a scalar fallback at the end of your SIMD
// processing loop to deal with the final elements without overflowing the
// allocated buffer.
//
// You must free this memory with SDL_FreeSIMD(), not free() or SDL_free() or
// delete[], etc.
//
// Note that SDL will only deal with SIMD instruction sets it is aware of; for
// example, SDL 2.0.8 knows that SSE wants 16-byte vectors (SDL_HasSSE()), and
// AVX2 wants 32 bytes (SDL_HasAVX2()), but doesn't know that AVX-512 wants
// 64. To be clear: if you can't decide to use an instruction set with an
// SDL_Has*() function, don't use that instruction set with memory allocated
// through here.
// SDL_AllocSIMD(0) will return a non-NULL pointer, assuming the system isn't
// out of memory, but you are not allowed to dereference it (because you only
// own zero bytes of that buffer).
extern DECLSPEC void *SDLCALL SDL_SIMDAlloc(const size_t len);

// Reallocate memory obtained from SDL_SIMDAlloc
// It is not valid to use this function on a pointer from anything but
// SDL_SIMDAlloc(). It can't be used on pointers from malloc, realloc,
// SDL_malloc, memalign, new[], etc.
extern DECLSPEC void *SDLCALL SDL_SIMDRealloc(void *mem, const size_t len);

// Deallocate memory obtained from SDL_SIMDAlloc
// It is not valid to use this function on a pointer from anything but
// SDL_SIMDAlloc() or SDL_SIMDRealloc(). It can't be used on pointers from
// malloc, realloc, SDL_malloc, memalign, new[], etc.
// However, SDL_SIMDFree(NULL) is a legal no-op.
// The memory pointed to by `ptr` is no longer valid for access upon return,
// and may be returned to the system or reused by a future allocation. The
// pointer passed to this function is no longer safe to dereference once this
// function returns, and should be discarded.
extern DECLSPEC void SDLCALL SDL_SIMDFree(void *ptr);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "_close_code.h"

#endif /* _cpuinfo_h_ */

/* vi: set ts=4 sw=4 expandtab: */
