/**************************************************************************/
/*  console_logger.cpp                                                      */
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

#include "debug_console.h"

#include "console_logger.h"

#include "core/vector.h"

#include <stdio.h>

// Re-entrancy guard: pushing a line must never end up logging again (an ERR_PRINT inside
// the push path would recurse until the stack is gone).
static thread_local bool _in_console_logger = false;

void ConsoleLogger::_push(const String &p_text, TextConsole::ColorIndex p_color) {
	if (_in_console_logger) {
		return;
	}
	_in_console_logger = true;
	// A message may carry several lines; the ring buffer stores one entry per line.
	const Vector<String> lines = p_text.split("\n", true);
	for (int i = 0; i < lines.size(); ++i) {
		if (i + 1 == lines.size() && lines[i].empty()) {
			break; // trailing newline, not an empty line
		}
		ConsoleInstance::log_from_thread(lines[i].replace("\r", "").replace("\t", "    "), p_color);
	}
	_in_console_logger = false;
}

void ConsoleLogger::_pushv(const char *p_format, va_list p_list, TextConsole::ColorIndex p_color) {
	va_list list_copy;
	va_copy(list_copy, p_list);

	char stack_buffer[1024];
	int len = vsnprintf(stack_buffer, sizeof(stack_buffer), p_format, p_list);
	if (len < 0) {
		va_end(list_copy);
		return;
	}
	if (len < (int)sizeof(stack_buffer)) {
		_push(String::utf8(stack_buffer), p_color);
	} else {
		Vector<char> heap_buffer;
		heap_buffer.resize(len + 1);
		vsnprintf(heap_buffer.ptrw(), len + 1, p_format, list_copy);
		_push(String::utf8(heap_buffer.ptr()), p_color);
	}
	va_end(list_copy);
}

void ConsoleLogger::logv(const char *p_format, va_list p_list, bool p_err) {
	if (!should_log(p_err)) {
		return;
	}
	_pushv(p_format, p_list, p_err ? TextConsole::COLOR_LIGHTRED : TextConsole::COLOR_DEFAULT);
}

void ConsoleLogger::log_error(const char *p_function, const char *p_file, int p_line, const char *p_code, const char *p_rationale, ErrorType p_type) {
	if (!should_log(true)) {
		return;
	}

	const char *err_type = "ERROR";
	TextConsole::ColorIndex color = TextConsole::COLOR_LIGHTRED;
	switch (p_type) {
		case ERR_WARNING:
			err_type = "WARNING";
			color = TextConsole::COLOR_YELLOW;
			break;
		case ERR_SCRIPT:
			err_type = "SCRIPT ERROR";
			color = TextConsole::COLOR_LIGHTMAGENTA;
			break;
		case ERR_SHADER:
			err_type = "SHADER ERROR";
			color = TextConsole::COLOR_LIGHTCYAN;
			break;
		case ERR_ERROR:
		default:
			break;
	}

	const char *details = (p_rationale && *p_rationale) ? p_rationale : p_code;
	_push(String(err_type) + ": " + String::utf8(details ? details : ""), color);
	// The origin line is dimmed: on a 40 column panel it is the part worth skipping.
	_push(String("   at: ") + String::utf8(p_function ? p_function : "") + " (" +
					String::utf8(p_file ? p_file : "") + ":" + itos(p_line) + ")",
			TextConsole::COLOR_DARKGRAY);
}
