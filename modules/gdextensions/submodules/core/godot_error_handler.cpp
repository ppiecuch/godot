/*************************************************************************/
/*  godot_error_handler.cpp                                              */
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

#include "godot_error_handler.h"

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
	static String _lastError = "";
	static int _lastErrorCnt = 1;
	String _error = vformat("%s:%d", p_file, p_line);

	if (_lastError == _error) {
		// Skip repeating messages
		_lastErrorCnt++;
		return;
	}

	Dictionary errorObject;

	errorObject["error"] = p_err;
	errorObject["error_descr"] = p_descr;
	errorObject["source_file"] = p_file;
	errorObject["source_line"] = p_line;
	errorObject["source_func"] = p_func;
	errorObject["warning"] = p_type == ERR_HANDLER_WARNING;
	if (!_lastError.empty())
		errorObject["skipped"] = vformat("%s (%d times)", _lastError, _lastErrorCnt);
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

	_lastError = _error;
	_lastErrorCnt = 1;

	// GodotErrorHandler *gdh = (GodotErrorHandler *)ud;
	// _print_format_error(errorObject);
}

#if 0

#Godot Error Handler(from https \
		: //github.com/binogure-studio/godot-error-handler)

Simple error handler module for Godot Engine

## Installation

Copy `godoterrorhandler` to the `godot/modules` folder. Then [compile Godot](http://docs.godotengine.org/en/2.1/development/compiling/introduction_to_the_buildsystem.html)

## Usage

### Engine.cfg

```
[autoload]

errorhandler="*res://scripts/errorhandler.gd"
```

### Example

```gdscript
extends node

func _ready():
  GodotErrorHandler.connect('error_threw', self, 'error_handler')

func error_handler(dump = null):
#Dump contains following informations
#dump["error"] name
#dump["error_descr"] description
#dump["source_file"] file
#dump["source_line"] line number
#dump["source_func"] function name
#dump["warning"] = true / false
#dump["callstack"] = callstack(only in debug)
  print(dump)
```

#License

[See LICENSE file](./LICENSE)

#endif
