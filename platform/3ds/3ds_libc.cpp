/**************************************************************************/
/*  3ds_libc.cpp                                                          */
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

#include <core/error_macros.h>

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>

#define R_OK 0
#define R_ERR -1

extern "C" ssize_t readlink(const char *path, char *buf, size_t bufsize) {
	WARN_PRINT("readlink unsupported");
	return 0;
}

extern "C" int symlink(const char *path1, const char *path2) {
	WARN_PRINT("symlink unsupported");
	return R_ERR;
}

extern "C" int asprintf(char **ret, const char *format, ...) {
	va_list ap;
	*ret = nullptr; /* Ensure value can be passed to free() */

	va_start(ap, format);
	int count = vsnprintf(nullptr, 0, format, ap);
	va_end(ap);

	if (count >= 0) {
		char *buffer = (char *)malloc(count + 1);
		if (buffer == nullptr) {
			return -1;
		}
		va_start(ap, format);
		count = vsnprintf(buffer, count + 1, format, ap);
		va_end(ap);

		if (count < 0) {
			free(buffer);
			return count;
		}
		*ret = buffer;
	}
	return count;
}
