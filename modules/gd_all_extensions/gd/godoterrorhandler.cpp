/*************************************************************************/
/*  godoterrorhandler.cpp                                                */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "godoterrorhandler.h"

GodotErrorHandler *GodotErrorHandler::singleton = NULL;

GodotErrorHandler::GodotErrorHandler() {

	eh.errfunc = _err_handler;
	eh.userdata = this;
	add_error_handler(&eh);

	singleton = this;
}

GodotErrorHandler::~GodotErrorHandler() {

	remove_error_handler(&eh);
	singleton = NULL;
}

GodotErrorHandler *GodotErrorHandler::get_singleton() {
	return GodotErrorHandler::singleton;
}

void GodotErrorHandler::_err_handler(void *ud, const char *p_func, const char *p_file, int p_line, const char *p_err, const char *p_descr, ErrorHandlerType p_type) {

	GodotErrorHandler *gdh = (GodotErrorHandler *)ud;

	Dictionary errorObject;

	errorObject["error"] = p_err;
	errorObject["error_descr"] = p_descr;
	errorObject["source_file"] = p_file;
	errorObject["source_line"] = p_line;
	errorObject["source_func"] = p_func;
	errorObject["warning"] = p_type == ERR_HANDLER_WARNING;
	Array cstack;

	Vector<ScriptLanguage::StackInfo> si;
	ScriptLanguage *sl = 0;

	for (int i = 0; i < ScriptServer::get_language_count(); i++) {
		sl = ScriptServer::get_language(i);
		si = sl->debug_get_current_stack_info();
		if (si.size())
			break;
	}

	cstack.resize(si.size() * 2);
	for (int i = 0; i < si.size(); i++) {
		String path;
		int line = 0;
		if (sl->debug_get_stack_level_instance(i)->get_script().is_valid()) {
			path = si[i].file;
			line = si[i].line;
		}
		cstack[i * 2 + 0] = path;
		cstack[i * 2 + 1] = line;
	}

	errorObject["callstack"] = cstack;

	gdh->handle_error(errorObject);
}

void GodotErrorHandler::handle_error(Dictionary errorObject) {
	emit_signal("error_threw", errorObject);
}

void GodotErrorHandler::_bind_methods() {
	ADD_SIGNAL(MethodInfo("error_threw", PropertyInfo(Variant::DICTIONARY, "errorObject")));
}
