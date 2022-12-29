/*************************************************************************/
/*  gd_core.cpp                                                          */
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

#include "core/variant.h"

#include <stdio.h>
#include <stdlib.h>

// va_copy was defined in the C99, but not in C++ standards before C++11.
// When you compile C++ without --std=c++<XX> option, compilers still define
// va_copy, otherwise you have to use the internal version (__va_copy).
#if !defined(va_copy)
#if defined(__GNUC__)
#define va_copy(d, s) __va_copy((d), (s))
#else
#define va_copy(d, s) ((d) = (s))
#endif
#endif

#if defined(MINGW_ENABLED)
#define gd_vsnprintf(m_buffer, m_count, m_format, m_args_copy) vsnprintf_s(m_buffer, m_count, _TRUNCATE, m_format, m_args_copy)
#define gd_vscprintf(m_format, m_args_copy) _vscprintf(m_format, m_args_copy)
#else
#define gd_vsnprintf(m_buffer, m_count, m_format, m_args_copy) vsnprintf(m_buffer, m_count, m_format, m_args_copy)
#define gd_vscprintf(m_format, m_args_copy) vsnprintf(NULL, 0, p_format, m_args_copy)
#endif

static char *_str_format_new(const char *p_format, va_list p_list) {
	va_list list;

	va_copy(list, p_list);
	int len = gd_vscprintf(p_format, list);
	va_end(list);

	len += 1; // for the trailing '/0'

	char *buffer(memnew_arr(char, len));

	va_copy(list, p_list);
	gd_vsnprintf(buffer, len, p_format, list);
	va_end(list);

	return buffer;
}

static String _str_format(const char *p_format, va_list p_list) {
	char *buffer = _str_format_new(p_format, p_list);

	String res(buffer);
	memdelete_arr(buffer);

	return res;
}


String string_format(const char *p_format, ...) {
	va_list list;

	va_start(list, p_format);
	String res = _str_format(p_format, list);
	va_end(list);

	return res;
}

String string_format(const Array &p_args) {
	String str;
	for (int i = 0; i < p_args.size(); i++) {
		str += p_args[i].operator String();
	}
	return str;
}
