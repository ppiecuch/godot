/**************************************************************************/
/*  gdscript_transpiler_utils.h                                           */
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

#pragma once

#include "core/string_builder.h"
#include "core/ustring.h"
#include "core/variant.h"

namespace GDScriptTranspilerUtils {

String snake_to_pascal_case(const String &p_identifier, bool p_input_is_upper = false);
String snake_to_camel_case(const String &p_identifier, bool p_input_is_upper = false);
String filepath_to_pascal_case(const String &p_identifier, bool p_input_is_upper = false);

class CodeBuilder {
	StringBuilder code;
	unsigned int indent_level;
	String indent_sequence;
	bool newline;

public:
	Dictionary map;

public:
	String get_indent_string() const;
	void set_indent_sequence(const String &p_indent_sequence);
	String get_indent_sequence() const { return indent_sequence; }
	void indent(unsigned int p_steps = 1) { indent_level += p_steps; }
	void dedent(unsigned int p_steps = 1) { indent_level -= p_steps; }

	void set_newline_enabled(bool p_enabled) { newline = p_enabled; }

	void set_substitute_map(const Dictionary &p_map) { map = p_map; }
	Dictionary get_substitute_map() { return map; }

	void operator+=(const String &p_string);
	String get_code() const;

	CodeBuilder();
};

} // namespace GDScriptTranspilerUtils
