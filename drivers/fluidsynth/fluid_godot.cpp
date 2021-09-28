/*************************************************************************/
/*  fluid_godot.cpp                                                      */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "fluidconfig.h"

#include "core/os/dir_access.h"
#include "core/os/file_access.h"
#include "core/os/os.h"

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

extern "C" double
fluid_utime(void) {
	return OS::get_singleton()->get_ticks_usec();
}

enum {
	FILE_TEST_EXISTS,
	FILE_TEST_IS_REGULAR,
};
extern "C" bool
fluid_file_test(const char *path, int test_op) {
	if (test_op == FILE_TEST_EXISTS) {
		return FileAccess::exists(path) || DirAccess::exists(path);
	}
	if (test_op == FILE_TEST_IS_REGULAR) {
		return FileAccess::exists(path);
	}
	WARN_PRINT("Unknown file test operation");
	return false;
}

#ifdef HAVE_SYS_STAT_H
typedef struct stat fluid_stat_buf_t;
#else
typedef struct _fluid_stat_buf_t {
	int st_mtime;
} fluid_stat_buf_t;
#endif

static bool _is_link(const String &path) {
	DirAccessRef da(DirAccess::create_for_path(path));
	return da->is_link(path);
}

extern "C" bool fluid_stat(const char *path, fluid_stat_buf_t *statbuf) {
#ifdef HAVE_SYS_STAT_H
	if (DirAccess::exists(path)) {
		if (statbuf)
			statbuf->st_mode = S_IFDIR;
	} else if (FileAccess::exists(path)) {
		if (statbuf)
			statbuf->st_mode = S_IFREG;
	} else
		return false;
	if (_is_link(path)) {
#ifndef WINDOWS_ENABLED
		if (statbuf)
			statbuf->st_mode = S_IFLNK;
#endif
	}
	if (statbuf) {
		statbuf->st_ctime = statbuf->st_mtime = FileAccess::get_modified_time(path);
	}
#else
	if (!DirAccess::exists(path) && !FileAccess::exists(path))
		return false;
	if (statbuf) {
		statbuf->st_ctime = statbuf->st_mtime = FileAccess::get_modified_time(path);
	}
#endif
	return true;
}
