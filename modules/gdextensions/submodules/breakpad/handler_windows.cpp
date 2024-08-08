/**************************************************************************/
/*  handler_windows.cpp                                                   */
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

#include <windows.h>

#include <iostream>
#include <map>
#include <memory>
#include <sstream>

#include "client/windows/handler/exception_handler.h"

static bool dump_callback(
		const wchar_t *dump_path,
		const wchar_t *minidump_id,
		void *context,
		EXCEPTION_POINTERS *exinfo,
		MDRawAssertionInfo *assertion,
		bool succeeded) {
	std::wcout << L"dump_path: " << dump_path << std::endl;
	std::wcout << L"minidump_id: " << minidump_id << std::endl;
	return succeeded;
}

static void quick_breakpad_plugin_init(QuickBreakpadPlugin *self) {
	static google_breakpad::ExceptionHandler handler(L".", nullptr, dumpCallback, nullptr, google_breakpad::ExceptionHandler::HANDLER_ALL);
}
