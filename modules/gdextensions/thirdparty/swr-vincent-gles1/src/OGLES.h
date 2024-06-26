#ifndef EGL_OGLES_H
#define EGL_OGLES_H 1

// ==========================================================================
//
// OGLES.h		Base include file for 3D Rendering Library
//
// --------------------------------------------------------------------------
//
// 10-02-2003	Hans-Martin Will	initial version
// 12-17-2003	Hans-Martin Will	added configuration information
//
// --------------------------------------------------------------------------
//
// Copyright (c) 2004, Hans-Martin Will. All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are 
// met:
// 
//	 *  Redistributions of source code must retain the above copyright
// 		notice, this list of conditions and the following disclaimer. 
//   *	Redistributions in binary form must reproduce the above copyright
// 		notice, this list of conditions and the following disclaimer in the 
// 		documentation and/or other materials provided with the distribution. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
// THE POSSIBILITY OF SUCH DAMAGE.
//
// ==========================================================================


#define OGLES_API
#define APIENTRY

#if (defined(ARM) || defined(_ARM_) || defined(__MARM__)) && !defined(EGL_NO_COMPILE)
# define EGL_USE_JIT 1
#else
# define EGL_USE_JIT 0
#endif


#ifndef EGL_RELEASE
# define EGL_RELEASE "x.yy"
#endif

#define EGL_CONFIG_VENDOR			"Hans-Martin Will"
#define EGL_CONFIG_VERSION			"OpenGL ES-CM 1.1"
#define EGL_CONFIG_EXTENSIONS		"GL_OES_fixed_point "\
									"GL_OES_single_precision "\
									"GL_OES_read_format "\
									"GL_OES_query_matrix "\
									"GL_OES_matrix_palette "\
									"GL_OES_point_size_array "\
									"GL_OES_point_sprite "\
									"GL_OES_compressed_paletted_texture"

#define EGL_CONFIG_RENDERER			"Software"

#if EGL_USE_JIT
# define	EGL_JIT_VERSION			" JIT"
#else
# define	EGL_JIT_VERSION			" No JIT"
#endif

#define EGL_VERSION_NUMBER			EGL_RELEASE " " EGL_CONFIG_RENDERER EGL_JIT_VERSION

#define EGL_NUM_TEXTURE_UNITS		2

#define EGL_MIPMAP_PER_TEXEL		1

// --------------------------------------------------------------------------
// type definitions
// --------------------------------------------------------------------------

typedef signed char 		I8;
typedef unsigned char 		U8;
typedef short 				I16;
typedef unsigned short 		U16;
typedef int 				I32;
typedef unsigned int 		U32;

typedef unsigned long long	U64;
typedef long long			I64;

#if UINTPTR_MAX == 0xffffffff
# define PTR32 1
#elif UINTPTR_MAX == 0xffffffffffffffffu
# define PTR64 1
#else
# if _M_X64 || _M_AMD64
#   define PTR64 1
# elif _WIN32 || _WIN64
#  if _WIN64
#   define PTR64 1
#  else
#   define PTR32 1
#  endif
# elif __GNUC__
#  if __x86_64__ || __ppc64__ || __LP64__ || _LP64 || __amd64__ || __amd64 || __x86_64__ || __x86_64 || __aarch64__
#   define PTR64 1
#  else
#   define PTR32 1
#  endif
# endif
#endif

#if PTR32
typedef unsigned int		UINTPTR;
#elif PTR64
typedef unsigned long long	UINTPTR;
#else
# error Failed to detect pointer size.
#endif

#define INT_MAX_LIMIT 0xffffffff

#ifndef INVALID_HANDLE_VALUE
# define INVALID_HANDLE_VALUE NULL
#endif

#ifdef EGL_XSCALE
# define EGL_USE_CLZ
#endif

// --------------------------------------------------------------------------
// Memory helper macros
// --------------------------------------------------------------------------

// number of elements of an array
#define elementsof(a) (sizeof(a) / sizeof(*(a)))

// offset in bytes of a structure member from the base address
#ifndef offsetof
# define offsetof(s,m)   (size_t)&(((s *)0)->m)
#endif


#endif // EGL_OGLES_H
