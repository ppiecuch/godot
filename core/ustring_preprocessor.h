/**************************************************************************/
/*  ustring_preprocessor.h                                                */
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

// A general purpose preprocessor.
// -------------------------------
// https://github.com/codecat/ccpp/blob/master/ccpp.h
//
// Supported directives:
//   #define <word>
//   #undef <word>
//   #if <condition>
//   #else
//   #elif <condition>
//   #endif

#ifndef USTRING_PREPROCESSOR_H
#define USTRING_PREPROCESSOR_H

#include "core/local_vector.h"
#include "core/ustring.h"

class StringProcessor {
	typedef bool (*include_callback_t)(const String &path);
	typedef bool (*command_callback_t)(const String &command, const String &value);

	CharType *p;
	CharType *p_end;

	size_t line;
	size_t column;

	LocalVector<String> defines;
	LocalVector<uint32_t> stack;

	include_callback_t include_callback;
	command_callback_t command_callback;

	bool test_condition();

	void expect_eol();
	void consume_line();

	void overwrite(CharType *p, size_t len);

public:
	void add_define(const String &name);
	void remove_define(const String &name);

	bool has_define(const String &name);

	void set_include_callback(include_callback_t callback);
	void set_command_callback(command_callback_t callback);

	String process(String buffer);
	void process(CharType *buffer, size_t len);

	StringProcessor();
	StringProcessor(const String &multi_defines);
};

#endif // USTRING_PREPROCESSOR_H
