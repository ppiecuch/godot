/*************************************************************************/
/*  win32_libc.cpp                                                       */
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

#include <core/error_macros.h>

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>

#define R_OK 0
#define R_ERR -1

struct dirent {
	char *d_name;
};

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

extern "C" int ftruncate(int fd, off_t length) {
	/***
	Set size of file stream 'fd' to 'length'.
		File pointer isn't modified (undefined behavior on shrinked files beyond the current pointer?)
		Shrinked bytes are lost
		Extended bytes are set to 0
		If file is mapped and shrinked beyond whole mapped pages, the pages are discarded
		In our impementation we ignore all mapping related code as ftruncate is called before a file is mapped or after it's unmapped within leveldb.
		The WinAPI call used, SetEndOfFile, requires that the file is unmapped, however there won't be an issue on this front (at this point).

		No extended bytes will be set to 0, as there doesn't seem to be a need for that. In order to perform this task, find the end of the file and add the extra 0s from there on,
		as necessary.
	***/

	HANDLE hF = (HANDLE)_get_osfhandle(fd);

	if(hF) {
		unsigned cpos = _tell(fd); // save current file pointer pos for restoring later
		if(cpos != 0xFFFFFFFF) {
			if(SetFilePointer((HANDLE)hF, length, 0, FILE_BEGIN) != 0xFFFFFFFF) { // set file pointer to length
				if(SetEndOfFile((HANDLE)hF)) {
					SetFilePointer((HANDLE)hF, cpos, 0, FILE_BEGIN); //file size has been changed, set pointer to back to cpos
					return 0; //returns 0 on success
				}
			}
		}
	}
	return -1;
}

extern "C" int scandir(const char *dir, struct dirent ***namelist_out,
		int (*filter)(const struct dirent *),
		int (*compar)(const struct dirent **, const struct dirent **)) {
}
