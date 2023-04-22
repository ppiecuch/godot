/**************************************************************************/
/*  rh_drm_kms.h                                                          */
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

// Reference:
// ----------
// https://github.com/OtherCrashOverride/libgo2
// https://events.static.linuxfound.org/sites/events/files/lcjpcojp13_pinchart.pdf
// https://gitlab.freedesktop.org/daniels/kms-quads
// https://gist.github.com/Miouyouyou/2f227fd9d4116189625f501c0dcf0542

#include "core/error_macros.h"
#include "core/print_string.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "RgaApi.h"

typedef enum drmkms_rotation {
	ROTATION_DEGREES_0 = 0,
	ROTATION_DEGREES_90,
	ROTATION_DEGREES_180,
	ROTATION_DEGREES_270
} drmkms_rotation_t;

typedef struct drmkms_surface {
	drmkms_display_t *display;
	uint32_t gem_handle;
	uint64_t size;
	int width;
	int height;
	int stride;
	uint32_t format;
	int prime_fd;
	bool is_mapped;
	uint8_t *map;
} drmkms_surface_t;

int drmkms_drm_format_get_bpp(uint32_t drm_format) {
	int result;

	switch (drm_format) {
		case DRM_FORMAT_XRGB4444:
		case DRM_FORMAT_XBGR4444:
		case DRM_FORMAT_RGBX4444:
		case DRM_FORMAT_BGRX4444:

		case DRM_FORMAT_ARGB4444:
		case DRM_FORMAT_ABGR4444:
		case DRM_FORMAT_RGBA4444:
		case DRM_FORMAT_BGRA4444:

		case DRM_FORMAT_XRGB1555:
		case DRM_FORMAT_XBGR1555:
		case DRM_FORMAT_RGBX5551:
		case DRM_FORMAT_BGRX5551:

		case DRM_FORMAT_ARGB1555:
		case DRM_FORMAT_ABGR1555:
		case DRM_FORMAT_RGBA5551:
		case DRM_FORMAT_BGRA5551:

		case DRM_FORMAT_RGB565:
		case DRM_FORMAT_BGR565:
			result = 16;
			break;

		case DRM_FORMAT_RGB888:
		case DRM_FORMAT_BGR888:
			result = 24;
			break;

		case DRM_FORMAT_XRGB8888:
		case DRM_FORMAT_XBGR8888:
		case DRM_FORMAT_RGBX8888:
		case DRM_FORMAT_BGRX8888:

		case DRM_FORMAT_ARGB8888:
		case DRM_FORMAT_ABGR8888:
		case DRM_FORMAT_RGBA8888:
		case DRM_FORMAT_BGRA8888:

		case DRM_FORMAT_XRGB2101010:
		case DRM_FORMAT_XBGR2101010:
		case DRM_FORMAT_RGBX1010102:
		case DRM_FORMAT_BGRX1010102:

		case DRM_FORMAT_ARGB2101010:
		case DRM_FORMAT_ABGR2101010:
		case DRM_FORMAT_RGBA1010102:
		case DRM_FORMAT_BGRA1010102:
			result = 32;
			break;

		default:
			printf("unhandled DRM FORMAT.\n");
			result = 0;
	}

	return result;
}

drmkms_surface_t *drmkms_surface_create(drmkms_display_t *display, int width, int height, uint32_t format) {
	drmkms_surface_t *result = (drmkms_surface_t *)malloc(sizeof(*result));
	ERR_NULL_FAIL_V(result, nullptr);

	memset(result, 0, sizeof(*result));

	struct drm_mode_create_dumb args = { 0 };
	args.width = width;
	args.height = height;
	args.bpp = drmkms_drm_format_get_bpp(format);
	args.flags = 0;

	int io = drmIoctl(display->fd, DRM_IOCTL_MODE_CREATE_DUMB, &args);
	if (io < 0) {
		printf("DRM_IOCTL_MODE_CREATE_DUMB failed.\n");
		goto out;
	}

	result->display = display;
	result->gem_handle = args.handle;
	result->size = args.size;
	result->width = width;
	result->height = height;
	result->stride = args.pitch;
	result->format = format;

	return result;

out:
	free(result);
	return nullptr;
}

void drmkms_surface_destroy(drmkms_surface_t *surface) {
	struct drm_mode_destroy_dumb args = { 0 };
	args.handle = surface->gem_handle;

	int io = drmIoctl(surface->display->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &args);
	if (io < 0) {
		printf("DRM_IOCTL_MODE_DESTROY_DUMB failed.\n");
	}

	free(surface);
}

int drmkms_surface_prime_fd(drmkms_surface_t *surface) {
	if (surface->prime_fd <= 0) {
		int io = drmPrimeHandleToFD(surface->display->fd, surface->gem_handle, DRM_RDWR | DRM_CLOEXEC, &surface->prime_fd);
		if (io < 0) {
			printf("drmPrimeHandleToFD failed.\n");
			goto out;
		}
	}
	return surface->prime_fd;

out:
	surface->prime_fd = 0;
	return 0;
}

void *drmkms_surface_map(drmkms_surface_t *surface) {
	if (surface->is_mapped) {
		return surface->map;
	}
	int prime_fd = drmkms_surface_prime_fd(surface);
	surface->map = mmap(nullptr, surface->size, PROT_READ | PROT_WRITE, MAP_SHARED, prime_fd, 0);

	ERR_FAIL_COND_V(surface->map == MAP_FAILED, nullptr)

	surface->is_mapped = true;
	return surface->map;
}

void drmkms_surface_unmap(drmkms_surface_t *surface) {
	if (surface->is_mapped) {
		munmap(surface->map, surface->size);

		surface->is_mapped = false;
		surface->map = nullptr;
	}
}

static uint32_t drmkms_rkformat_get(uint32_t drm_fourcc) {
	switch (drm_fourcc) {
		case DRM_FORMAT_RGBA8888:
			return RK_FORMAT_RGBA_8888;

		case DRM_FORMAT_RGBX8888:
			return RK_FORMAT_RGBX_8888;

		case DRM_FORMAT_RGB888:
			return RK_FORMAT_RGB_888;

		case DRM_FORMAT_ARGB8888:
		case DRM_FORMAT_XRGB8888:
			return RK_FORMAT_BGRA_8888;

		case DRM_FORMAT_RGB565:
			return RK_FORMAT_RGB_565;

		case DRM_FORMAT_RGBA5551:
			return RK_FORMAT_RGBA_5551;

		case DRM_FORMAT_RGBA4444:
			return RK_FORMAT_RGBA_4444;

		case DRM_FORMAT_BGR888:
			return RK_FORMAT_BGR_888;

		default:
			printf("RKFORMAT not supported. ");
			printf("drm_fourcc=%c%c%c%c\n", drm_fourcc & 0xff, drm_fourcc >> 8 & 0xff, drm_fourcc >> 16 & 0xff, drm_fourcc >> 24);
			return 0;
	}
}

void drmkms_surface_blit(drmkms_surface_t *srcSurface, int srcX, int srcY, int srcWidth, int srcHeight,
		drmkms_surface_t *dstSurface, int dstX, int dstY, int dstWidth, int dstHeight,
		drmkms_rotation_t rotation) {
	rga_info_t dst = { 0 };
	dst.fd = drmkms_surface_prime_fd(dstSurface);
	dst.mmuFlag = 1;
	dst.rect.xoffset = dstX;
	dst.rect.yoffset = dstY;
	dst.rect.width = dstWidth;
	dst.rect.height = dstHeight;
	dst.rect.wstride = dstSurface->stride / (drmkms_drm_format_get_bpp(dstSurface->format) / 8);
	dst.rect.hstride = dstSurface->height;
	dst.rect.format = drmkms_rkformat_get(dstSurface->format);

	rga_info_t src = { 0 };
	src.fd = drmkms_surface_prime_fd(srcSurface);
	src.mmuFlag = 1;

	switch (rotation) {
		case GO2_ROTATION_DEGREES_0:
			src.rotation = 0;
			break;

		case GO2_ROTATION_DEGREES_90:
			src.rotation = HAL_TRANSFORM_ROT_90;
			break;

		case GO2_ROTATION_DEGREES_180:
			src.rotation = HAL_TRANSFORM_ROT_180;
			break;

		case GO2_ROTATION_DEGREES_270:
			src.rotation = HAL_TRANSFORM_ROT_270;
			break;

		default:
			printf("rotation not supported.\n");
			return;
	}

	src.rect.xoffset = srcX;
	src.rect.yoffset = srcY;
	src.rect.width = srcWidth;
	src.rect.height = srcHeight;
	src.rect.wstride = srcSurface->stride / (drmkms_drm_format_get_bpp(srcSurface->format) / 8);
	src.rect.hstride = srcSurface->height;
	src.rect.format = drmkms_rkformat_get(srcSurface->format);

#if 0
	/* bicubic coefficient */
	enum {
		CATROM    = 0x0,
		MITCHELL  = 0x1,
		HERMITE   = 0x2,
		B_SPLINE  = 0x3,
	};
#endif
	src.scale_mode = 2;

	int ret = c_RkRgaBlit(&src, &dst, nullptr);
	if (ret) {
		printf("c_RkRgaBlit failed.\n");
	}
}

int drmkms_surface_save_as_png(drmkms_surface_t *surface, const char *filename) {
	png_structp png_ptr = nullptr;
	png_infop info_ptr = nullptr;
	png_bytep *row_pointers = nullptr;

	png_byte color_type = 0;
	png_byte bit_depth = 0;
	switch (surface->format) {
		case DRM_FORMAT_RGBA8888:
			color_type = PNG_COLOR_TYPE_RGBA;
			bit_depth = 8;
			break;

		case DRM_FORMAT_RGB888:
			color_type = PNG_COLOR_TYPE_RGB;
			bit_depth = 8;
			break;

		case DRM_FORMAT_RGBX8888:
		case DRM_FORMAT_RGB565:
		case DRM_FORMAT_ARGB8888:
		case DRM_FORMAT_XRGB8888:
		case DRM_FORMAT_RGBA5551:
		case DRM_FORMAT_RGBA4444:
		case DRM_FORMAT_BGR888:

		default:
			printf("The image format is not supported.\n");
			return -2;
	}

	// based on http://zarb.org/~gc/html/libpng.html

	/* create file */
	FILE *fp = fopen(filename, "wb");
	if (!fp) {
		printf("fopen failed. filename='%s'\n", filename);
		return -1;
	}

	/* initialize stuff */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

	if (!png_ptr) {
		printf("png_create_write_struct failed.\n");
		fclose(fp);
		return -1;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		printf("png_create_info_struct failed.\n");
		fclose(fp);
		return -1;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		printf("init_io failed.\n");
		goto out;
	}

	png_init_io(png_ptr, fp);

	/* write header */
	if (setjmp(png_jmpbuf(png_ptr))) {
		printf("write header failed.\n");
		goto out;
	}

	png_set_IHDR(png_ptr, info_ptr, surface->width, surface->height, bit_depth, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);

	/* write bytes */
	png_bytep src = (png_bytep)drmkms_surface_map(surface);
	row_pointers = malloc(sizeof(png_bytep) * surface->height);
	for (int y = 0; y < surface->height; ++y) {
		row_pointers[y] = src + (surface->stride * y);
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		printf("writing bytes failed.\n");
		goto out;
	}

	png_write_image(png_ptr, row_pointers);

	/* end write */
	if (setjmp(png_jmpbuf(png_ptr))) {
		printf("end of write failed.\n");
		goto out;
	}

	png_write_end(png_ptr, nullptr);

	/* cleanup heap allocation */
	free(row_pointers);

	fclose(fp);
	return 0;

out:
	if (info_ptr)
		png_destroy_info_struct(png_ptr, &info_ptr);

	if (png_ptr)
		png_destroy_write_struct(&png_ptr, (png_infopp) nullptr);

	if (row_pointers)
		free(row_pointers);

	if (fp)
		fclose(fp);

	return -1;
}
