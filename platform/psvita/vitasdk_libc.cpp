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
#include <sys/socket.h>
#include <netdb.h>

#include <psp2/io/stat.h>
#include <psp2/net.h>

#define SCE_ERRNO_MASK 0xFF

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

extern "C" ssize_t readlink(const char *path, char *buf, size_t bufsize) {
	WARN_PRINT("readlink unsupported");
	return 0;
}

extern "C" int symlink(const char *path1, const char *path2) {
	WARN_PRINT("symlink unsupported");
	return R_ERR;
}

extern "C" struct hostent *gethostbyaddr(const void *__addr, socklen_t __len, int __type) {
	static struct hostent ent;
	char name[NI_MAXHOST];
	static char sname[NI_MAXHOST] = "";
	static char *addrlist[2] = { nullptr, nullptr };

	if (__type != AF_INET) {
		errno = SCE_NET_ERROR_RESOLVER_ENOSUPPORT;
		return nullptr;
	}

	int rid = sceNetResolverCreate("resolver", nullptr, 0);
	if (rid < 0) {
		errno = rid & SCE_ERRNO_MASK;
		return nullptr;
	}

	int err = sceNetResolverStartAton(rid, __addr, name, sizeof(name), 0, 0, 0);
	sceNetResolverDestroy(rid);
	if (err < 0) {
		errno = err & SCE_ERRNO_MASK;
		return nullptr;
	}

	strncpy(sname, name, NI_MAXHOST - 1);
	addrlist[0] = (char *) __addr;

	ent.h_name = sname;
	ent.h_aliases = 0;
	ent.h_addrtype = __type;
	ent.h_length = sizeof(struct SceNetInAddr);
	ent.h_addr_list = addrlist;
	ent.h_addr = addrlist[0];

	return &ent;
}
