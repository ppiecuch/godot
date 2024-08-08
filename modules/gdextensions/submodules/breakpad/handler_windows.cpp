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
