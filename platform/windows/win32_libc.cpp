/*************************************************************************/
/*  win32_libc.cpp                                                       */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
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

#define _CRT_SECURE_NO_WARNINGS

#include <core/error_macros.h>

#include <ctype.h>
#include <errno.h>
#include <io.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>

#include <windows.h>

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

	if (hF) {
		unsigned cpos = _tell(fd); // save current file pointer pos for restoring later
		if (cpos != 0xFFFFFFFF) {
			if (SetFilePointer((HANDLE)hF, length, 0, FILE_BEGIN) != 0xFFFFFFFF) { // set file pointer to length
				if (SetEndOfFile((HANDLE)hF)) {
					SetFilePointer((HANDLE)hF, cpos, 0, FILE_BEGIN); //file size has been changed, set pointer to back to cpos
					return 0; //returns 0 on success
				}
			}
		}
	}
	return -1;
}

extern "C" int scandir(const char *dirname, struct dirent ***namelist,
		int (*select)(const struct dirent *),
		int (*compar)(const struct dirent **, const struct dirent **)) {
	int len;
	char *find_in, *d;
	WIN32_FIND_DATA find;
	HANDLE h;
	int nDir = 0, NDir = 0;
	struct dirent **dir = 0, *select_dir;
	unsigned long ret;

	len = strlen(dirname);
	find_in = (char *)malloc(len + 5);

	if (!find_in)
		return -1;

	strcpy(find_in, dirname);
	for (d = find_in; *d; d++) {
		if (*d == '/')
			*d = '\\';
	}
	if ((len == 0)) {
		strcpy(find_in, ".\\*");
	}
	if ((len == 1) && (d[-1] == '.')) {
		strcpy(find_in, ".\\*");
	}
	if ((len > 0) && (d[-1] == '\\')) {
		*d++ = '*';
		*d = 0;
	}
	if ((len > 1) && (d[-1] == '.') && (d[-2] == '\\')) {
		d[-1] = '*';
	}

	if ((h = FindFirstFile(find_in, &find)) == INVALID_HANDLE_VALUE) {
		free(find_in);
		ret = GetLastError();
		if (ret != ERROR_NO_MORE_FILES) {
			nDir = -1;
		}
		*namelist = dir;
		return nDir;
	}

	do {
		select_dir = (struct dirent *)malloc(sizeof(struct dirent) + strlen(find.cFileName) + 2);
		strcpy(select_dir->d_name, find.cFileName);
		if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			strcat(select_dir->d_name, "/"); // Append a trailing slash to directory names...
		}
		if (!select || (*select)(select_dir)) {
			if (nDir == NDir) {
				struct dirent **temp_dir = (dirent **)calloc(sizeof(struct dirent *), NDir + 33);
				if (NDir) {
					memcpy(temp_dir, dir, sizeof(struct dirent *) * NDir);
				}
				if (dir) {
					free(dir);
				}
				dir = temp_dir;
				NDir += 32;
			}
			dir[nDir] = select_dir;
			nDir++;
			dir[nDir] = 0;
		} else {
			free(select_dir);
		}
	} while (FindNextFile(h, &find));
	ret = GetLastError();
	if (ret != ERROR_NO_MORE_FILES) {
		// don't return an error code, because the dir list may still be valid up to this point
	}
	FindClose(h);

	free(find_in);

	if (compar) {
		qsort(dir, nDir, sizeof(*dir), (int (*)(const void *, const void *))compar);
	}

	*namelist = dir;
	return nDir;
}

static LARGE_INTEGER get_FILETIME_offset() {
	SYSTEMTIME s;
	FILETIME f;
	LARGE_INTEGER t;

	s.wYear = 1970;
	s.wMonth = 1;
	s.wDay = 1;
	s.wHour = 0;
	s.wMinute = 0;
	s.wSecond = 0;
	s.wMilliseconds = 0;
	SystemTimeToFileTime(&s, &f);
	t.QuadPart = f.dwHighDateTime;
	t.QuadPart <<= 32;
	t.QuadPart |= f.dwLowDateTime;
	return (t);
}

extern "C" int clock_gettime(int /* clock_id */, struct timeval *tv) {
	LARGE_INTEGER t;
	FILETIME f;
	double microseconds;
	static LARGE_INTEGER offset;
	static double frequency_to_microseconds;
	static int initialized = 0;
	static BOOL use_performance_counter = 0;

	if (!initialized) {
		LARGE_INTEGER performance_frequency;
		initialized = 1;
		use_performance_counter = QueryPerformanceFrequency(&performance_frequency);
		if (use_performance_counter) {
			QueryPerformanceCounter(&offset);
			frequency_to_microseconds = (double)performance_frequency.QuadPart / 1000000.;
		} else {
			offset = get_FILETIME_offset();
			frequency_to_microseconds = 10.;
		}
	}
	if (use_performance_counter) {
		QueryPerformanceCounter(&t);
	} else {
		GetSystemTimeAsFileTime(&f);
		t.QuadPart = f.dwHighDateTime;
		t.QuadPart <<= 32;
		t.QuadPart |= f.dwLowDateTime;
	}

	t.QuadPart -= offset.QuadPart;
	microseconds = (double)t.QuadPart / frequency_to_microseconds;
	t.QuadPart = microseconds;
	tv->tv_sec = t.QuadPart / 1000000;
	tv->tv_usec = t.QuadPart % 1000000;
	return (0);
}

extern "C" char *stristr(const char *str1, const char *str2) {
	const char *p1 = str1;
	const char *p2 = str2;
	const char *r = *p2 == 0 ? str1 : 0;

	while (*p1 != 0 && *p2 != 0) {
		if (tolower((unsigned char)*p1) == tolower((unsigned char)*p2)) {
			if (r == 0) {
				r = p1;
			}
			p2++;
		} else {
			p2 = str2;
			if (r != 0) {
				p1 = r + 1;
			}
			if (tolower((unsigned char)*p1) == tolower((unsigned char)*p2)) {
				r = p1;
				p2++;
			} else {
				r = 0;
			}
		}
		p1++;
	}

	return *p2 == 0 ? (char *)r : 0;
}
