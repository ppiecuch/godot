/**************************************************************************/
/*  console_logger.h                                                      */
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

#ifndef CONSOLE_LOGGER_H
#define CONSOLE_LOGGER_H

#include "core/io/logger.h"
#include "core/ustring.h"

#include "debug_console.h"

/* Feeds everything the engine logs -- print_line(), WARN_PRINT, ERR_PRINT, script and
 * shader errors -- into the LOG page of the in-app console, coloured by severity.
 *
 * Registered with OS::add_logger(), which hands ownership to the CompositeLogger, so the
 * instance lives as long as the OS singleton and must stay usable after the console node
 * is gone: every push goes through ConsoleInstance::log_from_thread(), which drops the
 * line when there is no console.
 *
 * logv() is called from arbitrary threads and must never log anything itself.
 */
class ConsoleLogger : public Logger {
	static void _push(const String &p_text, TextConsole::ColorIndex p_color);
	static void _pushv(const char *p_format, va_list p_list, TextConsole::ColorIndex p_color);

public:
	virtual void logv(const char *p_format, va_list p_list, bool p_err) _PRINTF_FORMAT_ATTRIBUTE_2_0;
	virtual void log_error(const char *p_function, const char *p_file, int p_line, const char *p_code, const char *p_rationale, ErrorType p_type = ERR_ERROR);

	virtual ~ConsoleLogger() {}
};

#endif // CONSOLE_LOGGER_H
