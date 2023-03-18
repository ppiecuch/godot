/**************************************************************************/
/*  vitasdk_libc.cpp                                                      */
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
#include <core/variant.h>

#include <dirent.h>
#include <errno.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <psp2/io/devctl.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/net/net.h>

#define SCE_ERRNO_MASK 0xFF

#define R_OK 0
#define R_ERR -1

#define LOG_PRINT(fmt, file, line) WARN_PRINT(vformat(fmt, file, line))

int _get_info(const char *path, uint32_t *size, uint32_t *folders, uint32_t *files) {
	SceUID dfd = sceIoDopen(path);
	if (dfd >= 0) {
		struct SceIoDirent dir;
		memset(&dir, 0, sizeof(SceIoDirent));

		while (sceIoDread(dfd, &dir) > 0) {
			if (dir.d_name[0] != '.') {
				char *new_path = (char *)malloc(strlen(path) + strlen(dir.d_name) + 2);
				sprintf(new_path, "%s/%s", path, dir.d_name);

				int res = _get_info(new_path, size, folders, files);

				free(new_path);

				if (res < 0)
					return res;
			}

			memset(&dir, 0, sizeof(SceIoDirent));
		}

		sceIoDclose(dfd);

		(*folders)++;
	} else {
		SceIoStat stat;
		memset(&stat, 0, sizeof(SceIoStat));

		int res = sceIoGetstat(path, &stat);
		if (res < 0)
			return res;

		(*size) += stat.st_size;
		(*files)++;
	}

	return 0;
}

int _remove_file_or_dir(const char *path, int *removed = nullptr) {
	if (removed)
		*removed = 0;

	SceUID dfd = sceIoDopen(path);
	if (dfd >= 0) {
		struct SceIoDirent dir;
		memset(&dir, 0, sizeof(SceIoDirent));

		while (sceIoDread(dfd, &dir) > 0) {
			if (dir.d_name[0] != '.') {
				char *new_path = (char *)malloc(strlen(path) + strlen(dir.d_name) + 2);
				sprintf(new_path, "%s/%s", path, dir.d_name);

				int res = _remove_file_or_dir(new_path, 0);

				free(new_path);

				if (res < 0)
					return res;
			}
			memset(&dir, 0, sizeof(SceIoDirent));
		}
		sceIoDclose(dfd);

		sceIoRmdir(path);
		if (removed) {
			(*removed)++;
		}
	} else {
		sceIoRemove(path);
		if (removed) {
			(*removed)++;
		}
	}

	return 0;
}

int _file_copy(const char *src, const char *dest, int *copied = nullptr) {
	if (copied)
		*copied = 0;

	SceUID dfd = sceIoDopen(src);
	if (dfd >= 0) {
		struct SceIoDirent dir;
		memset(&dir, 0, sizeof(SceIoDirent));

		while (sceIoDread(dfd, &dir) > 0) {
			if (dir.d_name[0] != '.') {
				char *src_path = (char *)malloc(strlen(src) + strlen(dir.d_name) + 2);
				sprintf(src_path, "%s/%s", src, dir.d_name);

				char *dest_path = (char *)malloc(strlen(dest) + strlen(dir.d_name) + 2);
				sprintf(dest_path, "%s/%s", dest, dir.d_name);

				int res = _file_copy(src_path, dest_path, 0);

				free(dest_path);
				free(src_path);

				if (res < 0)
					return res;
			}

			memset(&dir, 0, sizeof(SceIoDirent));
		}

		sceIoDclose(dfd);

		sceIoMkdir(dest, 0777);
	} else {
		SceUID fdsrc = sceIoOpen(src, SCE_O_RDONLY, 0);
		if (fdsrc < 0)
			return fdsrc;

		SceUID fddest = sceIoOpen(dest, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
		if (fddest < 0)
			return fddest;

		uint32_t size_buf = 0x10000;
		void *buf = malloc(size_buf);

		int read;
		while ((read = sceIoRead(fdsrc, buf, size_buf)) > 0) {
			sceIoWrite(fddest, buf, read);
			if (copied) {
				*copied += read;
			}
		}

		free(buf);

		sceIoClose(fddest);
		sceIoClose(fdsrc);
	}

	return 0;
}

typedef struct {
	uint32_t max_clusters;
	uint32_t free_clusters;
	uint32_t max_sectors;
	uint32_t sector_size;
	uint32_t sector_count;
} dev_info_t;

uint64_t _ms_free_space() {
	dev_info_t devinfo;
	dev_info_t *info = &devinfo;
	sceIoDevctl("ms0:", 0x02425818, &info, sizeof(info), nullptr, 0);

	return (uint64_t)devinfo.free_clusters * (uint64_t)devinfo.sector_count * (uint64_t)devinfo.sector_size;
}

extern "C" ssize_t readlink(const char *path, char *buf, size_t bufsize) {
	WARN_PRINT("readlink unsupported");
	return 0;
}

extern "C" int symlink(const char *path1, const char *path2) {
	WARN_PRINT("symlink unsupported");
	return R_ERR;
}

extern "C" mode_t umask(mode_t mask) {
	return 0666;
}

extern "C" int utime(const char *path, const struct utimbuf *times) {
	WARN_PRINT("utime unsupported");
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

	int err = sceNetResolverStartAton(rid, (SceNetInAddr *)__addr, name, sizeof(name), 0, 0, 0);
	sceNetResolverDestroy(rid);
	if (err < 0) {
		errno = err & SCE_ERRNO_MASK;
		return nullptr;
	}

	strncpy(sname, name, NI_MAXHOST);
	addrlist[0] = (char *)__addr;

	ent.h_name = sname;
	ent.h_aliases = 0;
	ent.h_addrtype = __type;
	ent.h_length = sizeof(struct SceNetInAddr);
	ent.h_addr_list = addrlist;
	ent.h_addr = addrlist[0];

	return &ent;
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
