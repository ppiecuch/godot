#include <sys/utsname.h>

#include <cstring>
#include <iostream>

#include "client/linux/handler/exception_handler.h"

static bool dump_callback(const google_breakpad::MinidumpDescriptor& descriptor, void *context, bool succeeded) {
	std::cout << "Dump path: " << descriptor.path() << std::endl;
	return succeeded;
}

static void quick_breakpad_plugin_init(QuickBreakpadPlugin* self) {
	google_breakpad::MinidumpDescriptor descriptor("/tmp");
	static google_breakpad::ExceptionHandler handler(descriptor, nullptr, dump_callback, nullptr, true, -1);
}
