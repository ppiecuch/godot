/*************************************************************************/
/*  vitasdk_libc.cpp                                                     */
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
#include <stdlib.h>
#include <sys/types.h>

#include <psp2/io/stat.h>

#define R_OK 0
#define R_ERR -1

extern "C" char *getcwd(char *path, size_t path_size) {
	char *buf = path;
	if (buf == NULL) {
		buf = (char *)malloc(1);
		path_size = 1;
	}
	if (buf == NULL)
		return NULL;
	if (path_size < 1)
		return NULL;
	*buf = 0;
	return buf;
}

extern "C" int chmod(const char *path, mode_t mode) {
	WARN_PRINT("chmod unsupported");
	return R_ERR;
}

extern "C" int fchmod(int fdes, mode_t mode) {
	return R_OK;
}

extern "C" int mkdir(const char *path, mode_t mode) {
	if (!sceIoMkdir(path, mode)) {
		WARN_PRINT("sceIoMkdir failed");
		return R_ERR;
	}
	return R_OK;
}

extern "C" int rmdir(const char *path) {
	if (!sceIoRmdir(path)) {
		WARN_PRINT("sceIoRmdir failed");
		return R_ERR;
	}
	return R_OK;
}

extern "C" int chdir(const char *path) {
	WARN_PRINT("chdir unsupported");
	return R_ERR;
}
