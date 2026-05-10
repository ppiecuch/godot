/**************************************************************************/
/*  gdscript_transpiler_utils.cpp                                         */
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

#include "gdscript_transpiler_utils.h"
#include "core/ucaps.h"

// Implementation borrowed from:
// https://github.com/godotengine/godot/blob/27ddb27da8a78b21a83ee7ee049fe92d32c66727/modules/mono/editor/bindings_generator.cpp#L117-L183

String GDScriptTranspilerUtils::snake_to_pascal_case(const String &p_identifier, bool p_input_is_upper) {
	String ret;
	Vector<String> parts = p_identifier.split("_", true);

	for (int i = 0; i < parts.size(); i++) {
		String part = parts[i];

		if (part.length()) {
			part[0] = _find_upper(part[0]);
			if (p_input_is_upper) {
				for (int j = 1; j < part.length(); j++)
					part[j] = _find_lower(part[j]);
			}
			ret += part;
		} else {
			if (i == 0 || i == (parts.size() - 1)) {
				// Preserve underscores at the beginning and end
				ret += "_";
			} else {
				// Preserve contiguous underscores
				if (parts[i - 1].length()) {
					ret += "__";
				} else {
					ret += "_";
				}
			}
		}
	}

	return ret;
}

String GDScriptTranspilerUtils::snake_to_camel_case(const String &p_identifier, bool p_input_is_upper) {
	String ret;
	Vector<String> parts = p_identifier.split("_", true);

	for (int i = 0; i < parts.size(); i++) {
		String part = parts[i];

		if (part.length()) {
			if (i != 0) {
				part[0] = _find_upper(part[0]);
			}
			if (p_input_is_upper) {
				for (int j = i != 0 ? 1 : 0; j < part.length(); j++)
					part[j] = _find_lower(part[j]);
			}
			ret += part;
		} else {
			if (i == 0 || i == (parts.size() - 1)) {
				// Preserve underscores at the beginning and end
				ret += "_";
			} else {
				// Preserve contiguous underscores
				if (parts[i - 1].length()) {
					ret += "__";
				} else {
					ret += "_";
				}
			}
		}
	}

	return ret;
}

String GDScriptTranspilerUtils::filepath_to_pascal_case(const String &p_path, bool p_input_is_upper) {
	return snake_to_pascal_case(p_path.get_basename().get_file(), p_input_is_upper);
}

// Common code generator

void GDScriptTranspilerUtils::CodeBuilder::set_indent_sequence(const String &p_indent_sequence) {
	indent_sequence = p_indent_sequence;
}

String GDScriptTranspilerUtils::CodeBuilder::get_indent_string() const {
	return indent_sequence.repeat(indent_level);
}

void GDScriptTranspilerUtils::CodeBuilder::operator+=(const String &p_string) {
	if (p_string == "\n") {
		code += "\n";
		return;
	}
	String str;
	str += get_indent_string();
	str += p_string;
	if (newline) {
		str += "\n";
	}
	code += str;
}

String GDScriptTranspilerUtils::CodeBuilder::get_code() const {
	return code.as_string().format(map);
}

GDScriptTranspilerUtils::CodeBuilder::CodeBuilder() {
	indent_level = 0;
	indent_sequence = "\t";
	newline = true;
}
