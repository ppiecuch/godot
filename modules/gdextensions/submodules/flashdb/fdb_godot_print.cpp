// C++ bridge: routes FlashDB's printf-style logging through Godot's print system.
// Called from C code via the FDB_PRINT macro override in fdb_def.h.
// Respects _print_line_enabled so SUPPRESS_OUTPUT suppresses output in doctests.

#include "core/print_string.h"

#include <cstdarg>
#include <cstdio>

extern "C" void fdb_godot_print(const char *fmt, ...) {
	if (!_print_line_enabled) {
		return;
	}

	char buf[512];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	// Strip trailing newline — Godot's print_verbose adds its own
	size_t len = strlen(buf);
	while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
		buf[--len] = '\0';
	}

	if (len > 0) {
		print_verbose(String::utf8(buf));
	}
}
