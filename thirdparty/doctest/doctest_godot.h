#pragma once

#include "core/os/dir_access.h"
#include "core/os/os.h"
#include "core/print_string.h"

// Suppress expected error output in negative tests.
// Usage: EXPECT_ERROR( call_that_triggers_error() );
#define EXPECT_ERROR(stmt) \
	do {                   \
		_print_error_enabled = false; \
		stmt;              \
		_print_error_enabled = true;  \
	} while (0)

String static _doctest_get_folder() {
	return vformat("__doctest__/%04d_%02d_%02d_%02d/",
					OS::get_singleton()->get_date().year,
					OS::get_singleton()->get_date().month,
					OS::get_singleton()->get_date().day,
					OS::get_singleton()->get_time().hour);
}

void static _doctest_prepare_folder() {
	DirAccess::create(DirAccess::ACCESS_FILESYSTEM)->make_dir_recursive(_doctest_get_folder());
}
