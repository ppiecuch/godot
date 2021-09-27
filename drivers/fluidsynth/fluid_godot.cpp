/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */

#include "core/os/os.h"
#include "core/os/file_access.h"
#include "core/os/dir_access.h"

extern "C" double
fluid_utime(void)
{
	return OS::get_singleton()->get_ticks_usec();
}

enum {
	FILE_TEST_EXISTS,
	FILE_TEST_IS_REGULAR,
};
extern "C" bool
fluid_file_test(const char *path, int test_op)
{
	if (test_op == FILE_TEST_EXISTS)
	{
		return FileAccess::exists(path) || DirAccess::exists(path);
	}
	if (test_op == FILE_TEST_IS_REGULAR)
	{
		return FileAccess::exists(path);
	}
	WARN_PRINT("Unknown file test operation");
	return false;
}
