/**************************************************************************/
/*  gdscript_transpiler.cpp                                               */
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

#include "gdscript_transpiler.h"
#include "core/string_builder.h"

Vector<GDScriptTranspilerLanguage *> GDScriptTranspiler::transpiler;

void GDScriptTranspiler::add_transpiler(GDScriptTranspilerLanguage *p_transpiler) {
	transpiler.push_back(p_transpiler);
}

void GDScriptTranspiler::remove_transpiler(GDScriptTranspilerLanguage *p_transpiler) {
	transpiler.erase(p_transpiler);
}

void GDScriptTranspiler::cleanup() {
	while (transpiler.size()) {
		remove_transpiler(transpiler[0]);
	}
}

Variant GDScriptTranspiler::transpile(const Ref<GDScript> &p_script, const String &p_language) {
	for (int i = 0; i < transpiler.size(); ++i) {
		if (transpiler[i]->handles(p_language)) {
			return transpiler[i]->transpile(p_script);
		}
	}
	ERR_FAIL_V_MSG(Variant(), "Unsupported language.");
}

void GDScriptTranspiler::get_supported_languages(List<String> *p_languages) {
	for (int i = 0; i < transpiler.size(); ++i) {
		p_languages->push_back(transpiler[i]->get_name());
	}
}

bool GDScriptTranspilerLanguage::handles(const String &p_language) const {
	return p_language.to_lower() == get_name().to_lower();
}

GDScriptTranspilerLanguage *GDScriptTranspiler::recognize(const String &p_language) {
	for (int i = 0; i < transpiler.size(); ++i) {
		if (transpiler[i]->handles(p_language)) {
			return transpiler[i];
		}
	}
	ERR_FAIL_V_MSG(nullptr, "Unrecognized language.");
}
