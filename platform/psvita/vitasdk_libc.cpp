/*************************************************************************/
/*  vitasdk_libc.cpp                                                     */
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

extern "C" int scandir(const char *dir, struct dirent ***namelist_out,
		int (*filter)(const struct dirent *),
		int (*compar)(const struct dirent **, const struct dirent **)) {
	int ret = -1, dir_uid = -1, name_alloc = 4, name_count = 0;
	struct dirent **namelist = nullptr, *ent;
	SceIoDirent sce_ent;

	namelist = (dirent **)malloc(sizeof(*namelist) * name_alloc);
	if (namelist == nullptr) {
		LOG_PRINT("%s:%i: OOM\n", __FILE__, __LINE__);
		goto fail;
	}

	// try to read first..
	dir_uid = sceIoDopen(dir);
	if (dir_uid >= 0) {
		/* it is very important to clear SceIoDirent to be passed to sceIoDread(), */
		/* or else it may crash, probably misinterpreting something in it. */
		memset(&sce_ent, 0, sizeof(sce_ent));
		ret = sceIoDread(dir_uid, &sce_ent);
		if (ret < 0) {
			LOG_PRINT("sceIoDread(\"%s\") failed with %i\n", dir, ret);
			goto fail;
		}
	} else {
		LOG_PRINT("sceIoDopen(\"%s\") failed with %i\n", dir, dir_uid);
	}

	while (ret > 0) {
		ent = (dirent *)malloc(sizeof(*ent));
		if (ent == nullptr) {
			LOG_PRINT("%s:%i: OOM\n", __FILE__, __LINE__);
			goto fail;
		}
		ent->d_stat = sce_ent.d_stat;
		ent->d_stat.st_attr &= SCE_SO_IFMT; // serves as d_type
		strncpy(ent->d_name, sce_ent.d_name, sizeof(ent->d_name));
		ent->d_name[sizeof(ent->d_name) - 1] = 0;
		if (filter == nullptr || filter(ent))
			namelist[name_count++] = ent;
		else
			free(ent);

		if (name_count >= name_alloc) {
			dirent **tmp;
			name_alloc *= 2;
			tmp = (dirent **)realloc(namelist, sizeof(*namelist) * name_alloc);
			if (tmp == nullptr) {
				LOG_PRINT("%s:%i: OOM\n", __FILE__, __LINE__);
				goto fail;
			}
			namelist = tmp;
		}

		memset(&sce_ent, 0, sizeof(sce_ent));
		ret = sceIoDread(dir_uid, &sce_ent);
	}

	// sort
	if (compar != nullptr && name_count > 3) {
		qsort(&namelist[2], name_count - 2, sizeof(namelist[0]), (__compar_fn_t)compar);
	}

	// all done.
	ret = name_count;
	*namelist_out = namelist;
	goto end;

fail:
	if (namelist != nullptr) {
		while (name_count--) {
			free(namelist[name_count]);
		}
		free(namelist);
	}
end:
	if (dir_uid >= 0) {
		sceIoDclose(dir_uid);
	}
	return ret;
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
