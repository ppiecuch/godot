/**************************************************************************/
/*  gdscript_transpiler_bind.cpp                                          */
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

#include "gdscript_transpiler_bind.h"
#include "gdscript_transpiler.h"

_GDScriptTranspiler *_GDScriptTranspiler::singleton = nullptr;

Variant _GDScriptTranspiler::transpile(const Ref<GDScript> &p_script, const String &p_language) {
	return GDScriptTranspiler::transpile(p_script, p_language);
}

Array _GDScriptTranspiler::get_supported_languages() const {
	List<String> languages;
	GDScriptTranspiler::get_supported_languages(&languages);

	Array ret;
	for (List<String>::Element *E = languages.front(); E; E = E->next()) {
		ret.push_back(E->get());
	}
	return ret;
}

void _GDScriptTranspiler::_bind_methods() {
	ClassDB::bind_method(D_METHOD("transpile", "gdscript", "to_language"), &_GDScriptTranspiler::transpile, DEFVAL("C++"));
	ClassDB::bind_method(D_METHOD("get_supported_languages"), &_GDScriptTranspiler::get_supported_languages);
}

_GDScriptTranspiler::_GDScriptTranspiler() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "Singleton already exists");
	singleton = this;
}
