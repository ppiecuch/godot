#include <core/error_macros.h>

#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>

#include <psp2/io/stat.h>

#define R_OK   0
#define R_ERR -1

extern "C" char *getcwd(char *path, size_t path_size) {
	char *buf = path;
	if (buf == NULL) {
		buf = (char*)malloc(1);
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

extern "C" int mkdir(const char*path, mode_t mode) {
	if(!sceIoMkdir(path, mode)) {
		WARN_PRINT("sceIoMkdir failed");
		return R_ERR;
	}
	return R_OK;
}

extern "C" int rmdir(const char *path) {
	if(!sceIoRmdir(path)) {
		WARN_PRINT("sceIoRmdir failed");
		return R_ERR;
	}
	return R_OK;
}

extern "C" int chdir(const char *path) {
	WARN_PRINT("chdir unsupported");
	return R_ERR;
}
