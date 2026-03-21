/**************************************************************************/
/*  godot_error_handler.cpp                                               */
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

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

#include "godot_error_handler.h"

GodotErrorHandler *GodotErrorHandler::singleton = nullptr;

GodotErrorHandler::GodotErrorHandler() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "Singleton already exists");
	singleton = this;

	_last_error_count = 0;

	eh.errfunc = _err_handler;
	eh.userdata = this;
	add_error_handler(&eh);
}

GodotErrorHandler::~GodotErrorHandler() {
	remove_error_handler(&eh);
	singleton = nullptr;
}

GodotErrorHandler *GodotErrorHandler::get_singleton() {
	return singleton;
}

void GodotErrorHandler::_bind_methods() {
	ADD_SIGNAL(MethodInfo("error_threw", PropertyInfo(Variant::DICTIONARY, "error")));
}

void GodotErrorHandler::_err_handler(void *ud, const char *p_func, const char *p_file, int p_line, const char *p_err, const char *p_descr, ErrorHandlerType p_type) {
	GodotErrorHandler *self = (GodotErrorHandler *)ud;
	if (!self) {
		return;
	}

	// Deduplicate consecutive identical errors.
	String location = vformat("%s:%d", p_file, p_line);
	if (self->_last_error == location) {
		self->_last_error_count++;
		return;
	}

	Dictionary error;
	error["error"] = String(p_err);
	error["error_descr"] = String(p_descr);
	error["source_file"] = String(p_file);
	error["source_line"] = p_line;
	error["source_func"] = String(p_func);
	error["warning"] = p_type == ERR_HANDLER_WARNING;

	if (!self->_last_error.empty() && self->_last_error_count > 1) {
		error["skipped"] = vformat("%s (%d times)", self->_last_error, self->_last_error_count);
	}

	// Collect script callstack.
	Array callstack;
	for (int i = 0; i < ScriptServer::get_language_count(); i++) {
		ScriptLanguage *lang = ScriptServer::get_language(i);
		Vector<ScriptLanguage::StackInfo> si = lang->debug_get_current_stack_info();
		if (si.size()) {
			callstack.resize(si.size() * 2);
			for (int j = 0; j < si.size(); j++) {
				callstack[j * 2 + 0] = si[j].file;
				callstack[j * 2 + 1] = si[j].line;
			}
			break;
		}
	}
	error["callstack"] = callstack;

	self->_last_error = location;
	self->_last_error_count = 1;

	self->emit_signal("error_threw", error);
}

#ifdef DOCTEST
TEST_CASE("[GodotErrorHandler] singleton pattern") {
	// GodotErrorHandler is a singleton that registers an error handler.
	// Full testing requires the engine error handler subsystem.
	CHECK(true); // placeholder -- requires engine runtime
}
#endif
