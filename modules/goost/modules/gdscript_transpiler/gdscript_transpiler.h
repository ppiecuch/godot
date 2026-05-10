/**************************************************************************/
/*  gdscript_transpiler.h                                                 */
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

#include "core/reference.h"

#include "modules/gdscript/gdscript.h"
#include "modules/gdscript/gdscript_parser.h"

#include "gdscript_transpiler_utils.h"

class GDScriptTranspilerLanguage;

class GDScriptTranspiler {
	static Vector<GDScriptTranspilerLanguage *> transpiler;

public:
	static Variant transpile(const Ref<GDScript> &p_script, const String &p_language = "C++");
	static void get_supported_languages(List<String> *p_languages);
	static GDScriptTranspilerLanguage *recognize(const String &p_language);

	static void add_transpiler(GDScriptTranspilerLanguage *p_transpiler);
	static void remove_transpiler(GDScriptTranspilerLanguage *p_transpiler);

	static const Vector<GDScriptTranspilerLanguage *> &get_transpilers() { return transpiler; }

	static void cleanup();
};

class GDScriptTranspilerLanguage {
protected:
	Map<String, GDScriptTranspilerUtils::CodeBuilder> code;
	String script_path;

public:
	virtual String get_name() const = 0;
	virtual bool handles(const String &p_language) const;
	virtual Variant transpile(const Ref<GDScript> &p_script) = 0;
};
