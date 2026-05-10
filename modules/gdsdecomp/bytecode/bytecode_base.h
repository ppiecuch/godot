/**************************************************************************/
/*  bytecode_base.h                                                       */
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

#ifndef GDSCRIPT_DECOMP_H
#define GDSCRIPT_DECOMP_H

#include "core/map.h"
#include "core/object.h"

class GDScriptDecomp : public Object {
	GDCLASS(GDScriptDecomp, Object);

protected:
	static void _bind_methods();
	static void _ensure_space(String &p_code);

	String script_text;
	String error_message;

public:
	virtual Error decompile_buffer(Vector<uint8_t> p_buffer) = 0;
	Error decompile_byte_code_encrypted(const String &p_path, Vector<uint8_t> p_key);
	Error decompile_byte_code(const String &p_path);

	String get_script_text();
	String get_error_message();
	String get_constant_string(Vector<Variant> &constants, uint32_t constId);

	Error decode_variant_3(Variant &r_variant, const uint8_t *p_buffer, int p_len, int *r_len = nullptr, bool p_allow_objects = false);
	Error decode_variant_2(Variant &r_variant, const uint8_t *p_buffer, int p_len, int *r_len = nullptr, bool p_allow_objects = false);
};

#endif
