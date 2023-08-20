/**************************************************************************/
/*  gd_core_defs.h                                                        */
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

#ifndef GD_CORE_DEFS_H
#define GD_CORE_DEFS_H

// Architecture
#define GD_ARCH_32BIT 0
#define GD_ARCH_64BIT 0

#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__) || defined(__64BIT__) || defined(__mips64) || defined(__powerpc64__) || defined(__ppc64__) || defined(__LP64__)
#undef GD_ARCH_64BIT
#define GD_ARCH_64BIT 64
#elif UINTPTR_MAX > UINT_MAX
#undef GD_ARCH_64BIT
#define GD_ARCH_64BIT 64
#else
#undef GD_ARCH_32BIT
#define GD_ARCH_32BIT 32
#endif //

// C++ variants
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
#define CPP17
#endif
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201402L) || __cplusplus >= 201402L)
#define CPP14
#endif
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201103L) || __cplusplus >= 201103L)
#define CPP11
#endif

// Exceptions availability
#ifndef _HAS_EXCEPTIONS
#if defined(__has_feature)
#if __has_feature(cxx_exceptions)
#define _HAS_EXCEPTIONS
#endif
#endif
#ifndef _HAS_EXCEPTIONS
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || (defined(_MSC_VER) && defined(_CPPUNWIND))
#define _HAS_EXCEPTIONS
#endif
#endif
#endif // _HAS_EXCEPTIONS

// Godot editor build
#if TOOLS_ENABLED
#define IN_EDITOR (Engine::get_singleton()->is_editor_hint() || OS::get_singleton()->is_no_window_mode_enabled())
#else
#define IN_EDITOR (false)
#endif

// Compiler
#ifndef _DEPRECATED
#if (__GNUC__ >= 4) /* technically, this arrived in gcc 3.1, but oh well. */
#define _DEPRECATED __attribute__((deprecated))
#else
#define _DEPRECATED
#endif
#endif

#ifndef _UNUSED
#ifdef __GNUC__
#define _UNUSED __attribute__((unused))
#else
#define _UNUSED
#endif
#endif

#endif // GD_CORE_DEFS_H
