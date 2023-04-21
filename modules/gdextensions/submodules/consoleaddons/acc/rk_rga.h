#include <errno.h>
#include <fcntl.h>
#include <linux/stddef.h>
#include <math.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "RgaUtils.h"
#include "im2d.hpp"

// BEGIN dma_alloc

#define DMA_HEAP_UNCACHE_PATH "/dev/dma_heap/system-uncached"
#define DMA_HEAP_PATH "/dev/dma_heap/system"
#define DMA_HEAP_DMA32_UNCACHE_PATCH "/dev/dma_heap/system-uncached-dma32"
#define DMA_HEAP_DMA32_PATCH "/dev/dma_heap/system-dma32"
#define CMA_HEAP_UNCACHE_PATH "/dev/dma_heap/cma-uncached"
#define RV1106_CMA_HEAP_PATH "/dev/rk_dma_heap/rk-dma-heap-cma"

static int dma_sync_device_to_cpu(int fd);
static int dma_sync_cpu_to_device(int fd);

static int dma_buf_alloc(const char *path, size_t size, int *fd, void **va);
static void dma_buf_free(size_t size, int *fd, void *va);

typedef unsigned long long __u64;
typedef unsigned int __u32;

struct dma_heap_allocation_data {
	__u64 len;
	__u32 fd;
	__u32 fd_flags;
	__u64 heap_flags;
};

struct dma_heap_import_data {
	__u64 len;
	__u32 fd;
	__u32 fd_flags;
	__u64 heap_flags;
};

#define DMA_HEAP_IOC_MAGIC 'H'
#define DMA_HEAP_IOCTL_ALLOC _IOWR(DMA_HEAP_IOC_MAGIC, 0x0, \
		struct dma_heap_allocation_data)
#define DMA_HEAP_IOCTL_IMPORT _IOWR(DMA_HEAP_IOC_MAGIC, 0x1, \
		struct dma_heap_import_data)

#define DMA_BUF_SYNC_READ (1 << 0)
#define DMA_BUF_SYNC_WRITE (2 << 0)
#define DMA_BUF_SYNC_RW (DMA_BUF_SYNC_READ | DMA_BUF_SYNC_WRITE)
#define DMA_BUF_SYNC_START (0 << 2)
#define DMA_BUF_SYNC_END (1 << 2)

struct dma_buf_sync {
	__u64 flags;
};

#define DMA_BUF_BASE 'b'
#define DMA_BUF_IOCTL_SYNC _IOW(DMA_BUF_BASE, 0, struct dma_buf_sync)

#define CMA_HEAP_SIZE 1024 * 1024

int dma_sync_device_to_cpu(int fd) {
	struct dma_buf_sync sync = { 0 };

	sync.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW;
	return ioctl(fd, DMA_BUF_IOCTL_SYNC, &sync);
}

int dma_sync_cpu_to_device(int fd) {
	struct dma_buf_sync sync = { 0 };

	sync.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_RW;
	return ioctl(fd, DMA_BUF_IOCTL_SYNC, &sync);
}

int dma_buf_alloc(const char *path, size_t size, int *fd, void **va) {
	int ret;
	int prot;
	void *mmap_va;
	int dma_heap_fd = -1;
	struct dma_heap_allocation_data buf_data;

	/* open dma_heap fd */
	if (dma_heap_fd < 0) {
		dma_heap_fd = open(path, O_RDWR);
		if (dma_heap_fd < 0) {
			printf("open %s fail!\n", path);
			return dma_heap_fd;
		}
	}

	/* alloc buffer */
	memset(&buf_data, 0x0, sizeof(struct dma_heap_allocation_data));

	buf_data.len = size;
	buf_data.fd_flags = O_CLOEXEC | O_RDWR;
	ret = ioctl(dma_heap_fd, DMA_HEAP_IOCTL_ALLOC, &buf_data);
	if (ret < 0) {
		printf("RK_DMA_HEAP_ALLOC_BUFFER failed\n");
		return ret;
	}

	/* mmap va */
	if (fcntl(buf_data.fd, F_GETFL) & O_RDWR)
		prot = PROT_READ | PROT_WRITE;
	else
		prot = PROT_READ;

	/* mmap contiguors buffer to user */
	mmap_va = (void *)mmap(NULL, buf_data.len, prot, MAP_SHARED, buf_data.fd, 0);
	if (mmap_va == MAP_FAILED) {
		printf("mmap failed: %s\n", strerror(errno));
		return -errno;
	}

	*va = mmap_va;
	*fd = buf_data.fd;

	close(dma_heap_fd);

	return 0;
}

void dma_buf_free(size_t size, int *fd, void *va) {
	int len;

	len = size;
	munmap(va, len);

	close(*fd);
	*fd = -1;
}

// END dma_alloc

struct data_bufer_t {
	uint8_t *buf;
	int w, h;
	int format
};

int rga_transform_flip_demo(const data_bufer_t &src_buf, data_bufer_t &dst_buf) {
	int ret = 0;
	int src_width, src_height, src_format;
	int dst_width, dst_height, dst_format;
	char *src_buf, *dst_buf;
	int src_buf_size, dst_buf_size;

	rga_buffer_t src_img, dst_img;
	rga_buffer_handle_t src_handle, dst_handle;

	memset(&src_img, 0, sizeof(src_img));
	memset(&dst_img, 0, sizeof(dst_img));

	src_width = 1280;
	src_height = 720;
	src_format = RK_FORMAT_RGBA_8888;

	dst_width = 1280;
	dst_height = 720;
	dst_format = RK_FORMAT_RGBA_8888;

	src_buf_size = src_width * src_height * get_bpp_from_format(src_format);
	dst_buf_size = dst_width * dst_height * get_bpp_from_format(dst_format);

	src_handle = importbuffer_virtualaddr(src_buf, src_buf_size);
	dst_handle = importbuffer_virtualaddr(dst_buf, dst_buf_size);
	if (src_handle == 0 || dst_handle == 0) {
		printf("importbuffer failed!\n");
		goto release_buffer;
	}

	src_img = wrapbuffer_handle(src_handle, src_width, src_height, src_format);
	dst_img = wrapbuffer_handle(dst_handle, dst_width, dst_height, dst_format);

	// Mirror the src image horizontally.
	//      src_img          dst_img
	//   --------------    --------------
	//   |.           |    |           .|
	//   |     <      | => |     >      |
	//   |            |    |            |
	//   --------------    --------------

	ret = imcheck(src_img, dst_img, {}, {});
	if (IM_STATUS_NOERROR != ret) {
		printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
		return -1;
	}

	ret = imflip(src_img, dst_img, IM_HAL_TRANSFORM_FLIP_H);
	if (ret == IM_STATUS_SUCCESS) {
		printf("%s running success!\n", LOG_TAG);
	} else {
		printf("%s running failed, %s\n", LOG_TAG, imStrError((IM_STATUS)ret));
		goto release_buffer;
	}

	write_image_to_file(dst_buf, LOCAL_FILE_PATH, dst_width, dst_height, dst_format, 0);

release_buffer:
	if (src_handle)
		releasebuffer_handle(src_handle);
	if (dst_handle)
		releasebuffer_handle(dst_handle);

	if (src_buf)
		free(src_buf);
	if (dst_buf)
		free(dst_buf);

	return ret;
}

int rga_transform_rotate_demo() {
	int ret = 0;
	int src_width, src_height, src_format;
	int dst_width, dst_height, dst_format;
	char *src_buf, *dst_buf;
	int src_buf_size, dst_buf_size;

	rga_buffer_t src_img, dst_img;
	rga_buffer_handle_t src_handle, dst_handle;

	src_handle = importbuffer_virtualaddr(src_buf, src_buf_size);
	dst_handle = importbuffer_virtualaddr(dst_buf, dst_buf_size);
	if (src_handle == 0 || dst_handle == 0) {
		printf("importbuffer failed!\n");
		goto release_buffer;
	}

	src_img = wrapbuffer_handle(src_handle, src_width, src_height, src_format);
	dst_img = wrapbuffer_handle(dst_handle, dst_width, dst_height, dst_format);

	// Rotate the src image by 90°.
	//       src_img          dst_img
	//    --------------    -----------
	//    |.           |    |        .|
	//    |     <      | => |    ^    |
	//    |            |    |         |
	//    --------------    |         |
	//                      -----------

	ret = imcheck(src_img, dst_img, {}, {});
	if (IM_STATUS_NOERROR != ret) {
		printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
		return -1;
	}

	ret = imrotate(src_img, dst_img, IM_HAL_TRANSFORM_ROT_90);
	if (ret == IM_STATUS_SUCCESS) {
		printf("%s running success!\n", LOG_TAG);
	} else {
		printf("%s running failed, %s\n", LOG_TAG, imStrError((IM_STATUS)ret));
		goto release_buffer;
	}

	write_image_to_file(dst_buf, LOCAL_FILE_PATH, dst_width, dst_height, dst_format, 0);

release_buffer:
	if (src_handle)
		releasebuffer_handle(src_handle);
	if (dst_handle)
		releasebuffer_handle(dst_handle);

	if (src_buf)
		free(src_buf);
	if (dst_buf)
		free(dst_buf);

	return ret;
}

int rga_transform_rotate_flip_demo() {
	int ret = 0;
	int src_width, src_height, src_format;
	int dst_width, dst_height, dst_format;
	char *src_buf, *dst_buf;
	int src_buf_size, dst_buf_size;

	rga_buffer_t src_img, dst_img;
	rga_buffer_handle_t src_handle, dst_handle;

	src_handle = importbuffer_virtualaddr(src_buf, src_buf_size);
	dst_handle = importbuffer_virtualaddr(dst_buf, dst_buf_size);
	if (src_handle == 0 || dst_handle == 0) {
		printf("importbuffer failed!\n");
		goto release_buffer;
	}

	src_img = wrapbuffer_handle(src_handle, src_width, src_height, src_format);
	dst_img = wrapbuffer_handle(dst_handle, dst_width, dst_height, dst_format);

	// Rotate the src image by 270° and mirror it horizontally and vertically.
	//       src_img          dst_img
	//    --------------    -----------
	//    |.           |    |        .|
	//    |     <      | => |    ^    |
	//    |            |    |         |
	//    --------------    |         |
	//                      -----------

	ret = imcheck(src_img, dst_img, {}, {});
	if (IM_STATUS_NOERROR != ret) {
		printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
		return -1;
	}

	ret = improcess(src_img, dst_img, {}, {}, {}, {}, IM_HAL_TRANSFORM_ROT_270 | IM_HAL_TRANSFORM_FLIP_H_V);
	if (ret == IM_STATUS_SUCCESS) {
		printf("%s running success!\n", LOG_TAG);
	} else {
		printf("%s running failed, %s\n", LOG_TAG, imStrError((IM_STATUS)ret));
		goto release_buffer;
	}

	write_image_to_file(dst_buf, LOCAL_FILE_PATH, dst_width, dst_height, dst_format, 0);

release_buffer:
	if (src_handle)
		releasebuffer_handle(src_handle);
	if (dst_handle)
		releasebuffer_handle(dst_handle);

	if (src_buf)
		free(src_buf);
	if (dst_buf)
		free(dst_buf);

	return ret;
}

int rga_rop_demo() {
	int ret = 0;
	int src_width, src_height, src_format;
	int dst_width, dst_height, dst_format;
	char *src_buf, *dst_buf;
	int src_buf_size, dst_buf_size;

	rga_buffer_t src_img, dst_img;
	rga_buffer_handle_t src_handle, dst_handle;

	src_handle = importbuffer_virtualaddr(src_buf, src_buf_size);
	dst_handle = importbuffer_virtualaddr(dst_buf, dst_buf_size);
	if (src_handle == 0 || dst_handle == 0) {
		printf("importbuffer failed!\n");
		goto release_buffer;
	}

	src_img = wrapbuffer_handle(src_handle, src_width, src_height, src_format);
	dst_img = wrapbuffer_handle(dst_handle, dst_width, dst_height, dst_format);

	// src image and dst image for ROP calculation
	//       src_img +(ROP) dst_img => dst_img
	//    --------------     --------------    --------------
	//    |            |     |            |    |            |
	//    |  src_img   | ROP |  dst_img   | => |   dst_img  |
	//    |            |     |            |    |  (result)  |
	//    --------------     --------------    --------------

	ret = imcheck(src_img, dst_img, {}, {});
	if (IM_STATUS_NOERROR != ret) {
		printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
		return -1;
	}

	ret = imrop(src_img, dst_img, IM_ROP_AND);
	if (ret == IM_STATUS_SUCCESS) {
		printf("%s running success!\n", LOG_TAG);
	} else {
		printf("%s running failed, %s\n", LOG_TAG, imStrError((IM_STATUS)ret));
		goto release_buffer;
	}

	write_image_to_file(dst_buf, LOCAL_FILE_PATH, dst_width, dst_height, dst_format, 0);

release_buffer:
	if (src_handle)
		releasebuffer_handle(src_handle);
	if (dst_handle)
		releasebuffer_handle(dst_handle);

	if (src_buf)
		free(src_buf);
	if (dst_buf)
		free(dst_buf);

	return ret;
}

int rga_resize_demo() {
	int ret = 0;
	int src_width, src_height, src_format;
	int dst_width, dst_height, dst_format;
	char *src_buf, *dst_buf;
	int src_buf_size, dst_buf_size;

	rga_buffer_t src_img, dst_img;
	rga_buffer_handle_t src_handle, dst_handle;

	src_handle = importbuffer_virtualaddr(src_buf, src_buf_size);
	dst_handle = importbuffer_virtualaddr(dst_buf, dst_buf_size);
	if (src_handle == 0 || dst_handle == 0) {
		printf("importbuffer failed!\n");
		goto release_buffer;
	}

	src_img = wrapbuffer_handle(src_handle, src_width, src_height, src_format);
	dst_img = wrapbuffer_handle(dst_handle, dst_width, dst_height, dst_format);

	// Scale up the src image to 1920*1080.
	//    --------------    ---------------------
	//    |            |    |                   |
	//    |  src_img   |    |     dst_img       |
	//    |            | => |                   |
	//    --------------    |                   |
	//                      |                   |
	//                      ---------------------

	ret = imcheck(src_img, dst_img, {}, {});
	if (IM_STATUS_NOERROR != ret) {
		printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
		return -1;
	}

	ret = imresize(src_img, dst_img);
	if (ret == IM_STATUS_SUCCESS) {
		printf("%s running success!\n", LOG_TAG);
	} else {
		printf("%s running failed, %s\n", LOG_TAG, imStrError((IM_STATUS)ret));
		goto release_buffer;
	}

	write_image_to_file(dst_buf, LOCAL_FILE_PATH, dst_width, dst_height, dst_format, 0);

release_buffer:
	if (src_handle)
		releasebuffer_handle(src_handle);
	if (dst_handle)
		releasebuffer_handle(dst_handle);

	if (src_buf)
		free(src_buf);
	if (dst_buf)
		free(dst_buf);

	return ret;
}

int rga_resize_rect_demo() {
	int ret = 0;
	int src_width, src_height, src_format;
	int dst_width, dst_height, dst_format;
	char *src_buf, *dst_buf;
	int src_buf_size, dst_buf_size;

	rga_buffer_t src_img, dst_img;
	im_rect dst_rect[4];
	rga_buffer_handle_t src_handle, dst_handle;

	src_handle = importbuffer_virtualaddr(src_buf, src_buf_size);
	dst_handle = importbuffer_virtualaddr(dst_buf, dst_buf_size);
	if (src_handle == 0 || dst_handle == 0) {
		printf("importbuffer failed!\n");
		goto release_buffer;
	}

	src_img = wrapbuffer_handle(src_handle, src_width, src_height, src_format);
	dst_img = wrapbuffer_handle(dst_handle, dst_width, dst_height, dst_format);

	// Scaling and splicing the src image into 4 grids
	//    --------------    ---------------------
	//    |            |    |         |         |
	//    |  src_img   |    |         |         |
	//    |            |    |         |         |
	//    -------------- => ---------------------
	//                      |         |         |
	//                      |         |         |
	//                      |         |         |
	//                      ---------------------

	dst_rect[0].x = 0;
	dst_rect[0].y = 0;
	dst_rect[0].width = dst_width / 2;
	dst_rect[0].height = dst_height / 2;

	dst_rect[1].x = dst_width / 2;
	dst_rect[1].y = 0;
	dst_rect[1].width = dst_width / 2;
	dst_rect[1].height = dst_height / 2;

	dst_rect[2].x = 0;
	dst_rect[2].y = dst_height / 2;
	dst_rect[2].width = dst_width / 2;
	dst_rect[2].height = dst_height / 2;

	dst_rect[3].x = dst_width / 2;
	dst_rect[3].y = dst_height / 2;
	dst_rect[3].width = dst_width / 2;
	dst_rect[3].height = dst_height / 2;

	for (int i = 0; i < 4; i++) {
		ret = imcheck(src_img, dst_img, {}, dst_rect[i]);
		if (IM_STATUS_NOERROR != ret) {
			printf("%d, [%d] check error! %s", __LINE__, i, imStrError((IM_STATUS)ret));
			return -1;
		}

		ret = improcess(src_img, dst_img, {}, {}, dst_rect[i], {}, IM_SYNC);
		if (ret == IM_STATUS_SUCCESS) {
			printf("%s [%d] running success!\n", LOG_TAG, i);
		} else {
			printf("%s [%d] running failed, %s\n", LOG_TAG, i, imStrError((IM_STATUS)ret));
			goto release_buffer;
		}
	}

	write_image_to_file(dst_buf, LOCAL_FILE_PATH, dst_width, dst_height, dst_format, 0);

release_buffer:
	if (src_handle)
		releasebuffer_handle(src_handle);
	if (dst_handle)
		releasebuffer_handle(dst_handle);

	if (src_buf)
		free(src_buf);
	if (dst_buf)
		free(dst_buf);

	return ret;
}

#define USE_DMA_HEAP 1

/* The current sample code is suitable for chips whose RGA hardware does not support average downsampling, and uses Y400 format for UV downsampling. */
static IM_STATUS local_downsampling_imcvtcolor(rga_buffer_t &orig_src, rga_buffer_t &orig_dst, int src_fmt, int dst_fmt) {
	int ret = 0;
	rga_buffer_t src_img, dst_img;
	im_rect src_rect, dst_rect;

	memset(&src_img, 0, sizeof(src_img));
	memset(&dst_img, 0, sizeof(dst_img));
	memset(&src_rect, 0, sizeof(src_rect));
	memset(&dst_rect, 0, sizeof(dst_rect));

	src_img = wrapbuffer_handle(orig_src.handle,
			orig_src.wstride, orig_src.hstride * get_bpp_from_format(orig_src.format),
			RK_FORMAT_YCbCr_400);
	dst_img = wrapbuffer_handle(orig_dst.handle,
			orig_dst.wstride, orig_dst.hstride * get_bpp_from_format(orig_dst.format),
			RK_FORMAT_YCbCr_400);

	if (src_img.handle != dst_img.handle) {
		src_rect.x = 0;
		src_rect.y = 0;
		src_rect.width = orig_src.wstride;
		src_rect.height = orig_src.hstride;

		dst_rect.x = 0;
		dst_rect.y = 0;
		dst_rect.width = orig_dst.wstride;
		dst_rect.height = orig_dst.hstride;

		ret = improcess(src_img, dst_img, {}, src_rect, dst_rect, {}, IM_SYNC);
		if (ret == IM_STATUS_SUCCESS) {
			printf("%s Y channel running success!\n", LOG_TAG);
		} else {
			printf("%s Y channel running failed!\n", LOG_TAG);
			return (IM_STATUS)ret;
		}
	}

	// Scale uv data.
	//    --------------    --------------
	//    |            |    |            |
	//    |   y_data   |    |   y_data   |
	//    |            |    |            |
	//    |            |    |            |
	//    --------------    --------------
	//    |            |    |            |
	//    | uv422_data | => | uv420_data |
	//    |            |    --------------
	//    |            |
	//    --------------

	src_rect.x = 0;
	src_rect.y = orig_src.hstride;
	src_rect.width = orig_src.wstride;
	src_rect.height = orig_src.hstride;

	dst_rect.x = 0;
	dst_rect.y = orig_dst.hstride;
	dst_rect.width = orig_dst.wstride;
	dst_rect.height = orig_dst.hstride / 2;

	ret = improcess(src_img, dst_img, {}, src_rect, dst_rect, {}, IM_SYNC);
	if (ret == IM_STATUS_SUCCESS) {
		printf("%s UV channel running success!\n", LOG_TAG);
	} else {
		printf("%s UV channel running failed!\n", LOG_TAG);
		return (IM_STATUS)ret;
	}

	return IM_STATUS_SUCCESS;
}

int rga_resize_uv_downsampling_demo() {
	int ret = 0;
	int src_width, src_height, src_format;
	int dst_width, dst_height, dst_format;
	int src_dma_fd, dst_dma_fd;
	char *src_buf, *dst_buf;
	int src_buf_size, dst_buf_size;

	rga_buffer_t src_img, dst_img;
	im_rect src_rect, dst_rect;
	rga_buffer_handle_t src_handle, dst_handle;

#if USE_DMA_HEAP
	ret = dma_buf_alloc(DMA_HEAP_DMA32_UNCACHE_PATCH, src_buf_size, &src_dma_fd, (void **)&src_buf);
	if (ret < 0) {
		printf("alloc src dma32_heap buffer failed!\n");
		return -1;
	}

	ret = dma_buf_alloc(DMA_HEAP_DMA32_UNCACHE_PATCH, dst_buf_size, &dst_dma_fd, (void **)&dst_buf);
	if (ret < 0) {
		printf("alloc dst dma32_heap buffer failed!\n");
		return -1;
	}
#else
	(void)(src_dma_fd);
	(void)(dst_dma_fd);
#endif

	src_handle = importbuffer_virtualaddr(src_buf, src_buf_size);
	dst_handle = importbuffer_virtualaddr(dst_buf, dst_buf_size);
	if (src_handle == 0 || dst_handle == 0) {
		printf("importbuffer failed!\n");
		goto release_buffer;
	}

	src_img = wrapbuffer_handle(src_handle, src_width, src_height, src_format);
	dst_img = wrapbuffer_handle(dst_handle, dst_width, dst_height, dst_format);

	ret = imcheck(src_img, dst_img, {}, {});
	if (IM_STATUS_NOERROR != ret) {
		printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
		return -1;
	}

	ret = imcvtcolor(src_img, dst_img, RK_FORMAT_YCbCr_422_SP, RK_FORMAT_YCbCr_420_SP);
	if (ret == IM_STATUS_SUCCESS) {
		printf("%s RGA cvtcolor running success!\n", LOG_TAG);
	} else {
		printf("%s RGA cvtcolor running failed, %s\n", LOG_TAG, imStrError((IM_STATUS)ret));
		goto release_buffer;
	}

	printf("RGA output [0x%x, 0x%x, 0x%x, 0x%x]\n", dst_buf[0], dst_buf[1], dst_buf[2], dst_buf[3]);
	write_image_to_file(dst_buf, LOCAL_FILE_PATH, dst_width, dst_height, dst_format, 0);

	ret = local_downsampling_imcvtcolor(src_img, dst_img, RK_FORMAT_YCbCr_422_SP, RK_FORMAT_YCbCr_420_SP);
	if (ret == IM_STATUS_SUCCESS) {
		printf("%s local downsampling cvtcolor running success!\n", LOG_TAG);
	} else {
		printf("%s local downsampling cvtcolor running failed, %s\n", LOG_TAG, imStrError((IM_STATUS)ret));
		goto release_buffer;
	}

	printf("local output [0x%x, 0x%x, 0x%x, 0x%x]\n", src_buf[0], src_buf[1], src_buf[2], src_buf[3]);
	write_image_to_file(dst_buf, LOCAL_FILE_PATH, dst_width, dst_height, dst_format, 1);

release_buffer:
	if (src_handle)
		releasebuffer_handle(src_handle);
	if (dst_handle)
		releasebuffer_handle(dst_handle);

#if !USE_DMA_HEAP
	if (src_buf)
		free(src_buf);
	if (dst_buf)
		free(dst_buf);
#endif

	return ret;
}

int rga_fill_demo(void) {
	int ret = 0;
	int64_t ts;
	int dst_width, dst_height, dst_format;
	int dst_buf_size;
	char *dst_buf;
	int dst_dma_fd;
	rga_buffer_t dst = {};
	im_rect dst_rect = {};
	rga_buffer_handle_t dst_handle;

	// Allocate dma_buf within 4G from dma32_heap,
	// return dma_fd and virtual address.
	// ColorFill can only be used on buffers within 4G.
	ret = dma_buf_alloc(DMA_HEAP_DMA32_UNCACHE_PATCH, dst_buf_size, &dst_dma_fd, (void **)&dst_buf);
	if (ret < 0) {
		printf("alloc dma32_heap buffer failed!\n");
		return -1;
	}

	memset(dst_buf, 0x33, dst_buf_size);

	// Import the allocated dma_fd into RGA by calling
	// importbuffer_fd, and use the returned buffer_handle
	// to call RGA to process the image.
	dst_handle = importbuffer_fd(dst_dma_fd, dst_buf_size);
	if (dst_handle == 0) {
		printf("import dma_fd error!\n");
		ret = -1;
		goto free_buf;
	}

	dst = wrapbuffer_handle(dst_handle, dst_width, dst_height, dst_format);

	// Fills a rectangular area on the dst image with the specified color.
	//       dst_image
	//    --------------
	//    | -------    |
	//    | |color|    |
	//    | -------    |
	//    --------------

	dst_rect.x = 0;
	dst_rect.y = 0;
	dst_rect.width = 300;
	dst_rect.height = 200;

	ret = imcheck({}, dst, {}, dst_rect, IM_COLOR_FILL);
	if (IM_STATUS_NOERROR != ret) {
		printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
		goto release_buffer;
	}

	ret = imfill(dst, dst_rect, 0xff00ff00);
	if (ret == IM_STATUS_SUCCESS) {
		printf("%s running success!\n", LOG_TAG);
	} else {
		printf("%s running failed, %s\n", LOG_TAG, imStrError((IM_STATUS)ret));
		goto release_buffer;
	}

	printf("output [0x%x, 0x%x, 0x%x, 0x%x]\n", dst_buf[0], dst_buf[1], dst_buf[2], dst_buf[3]);
	write_image_to_file(dst_buf, LOCAL_FILE_PATH, dst_width, dst_height, dst_format, 0);

release_buffer:
	if (dst_handle > 0)
		releasebuffer_handle(dst_handle);

free_buf:
	dma_buf_free(dst_buf_size, &dst_dma_fd, dst_buf);

	return 0;
}

int rga_fill_rectangle_array_demo(void) {
	int ret = 0;
	int64_t ts;
	int dst_width, dst_height, dst_format;
	int dst_buf_size;
	char *dst_buf;
	int dst_dma_fd;
	rga_buffer_t dst = {};
	im_rect dst_rect[4] = {};
	rga_buffer_handle_t dst_handle;

	// Allocate dma_buf within 4G from dma32_heap,
	// return dma_fd and virtual address.
	// ColorFill can only be used on buffers within 4G.
	ret = dma_buf_alloc(DMA_HEAP_DMA32_UNCACHE_PATCH, dst_buf_size, &dst_dma_fd, (void **)&dst_buf);
	if (ret < 0) {
		printf("alloc dma32_heap buffer failed!\n");
		return -1;
	}

	memset(dst_buf, 0x33, dst_buf_size);

	// Import the allocated dma_fd into RGA by calling
	// importbuffer_fd, and use the returned buffer_handle
	// to call RGA to process the image.
	dst_handle = importbuffer_fd(dst_dma_fd, dst_buf_size);
	if (dst_handle == 0) {
		printf("import dma_fd error!\n");
		ret = -1;
		goto free_buf;
	}

	dst = wrapbuffer_handle(dst_handle, dst_width, dst_height, dst_format);

	// Fills an array of rectangular areas on the dst image with the specified color.
	//       dst_image
	//    --------------
	//    | -------    |
	//    | |   --|--  |
	//    | ----|-- |  |
	//    |     -----  |
	//    --------------

	dst_rect[0] = { 0, 0, 100, 100 };
	dst_rect[1] = { 50, 50, 150, 150 };
	dst_rect[2] = { 100, 100, 200, 200 };
	dst_rect[3] = { 150, 150, 250, 250 };

	for (int i = 0; i < 4; i++) {
		ret = imcheck({}, dst, {}, dst_rect[i], IM_COLOR_FILL);
		if (IM_STATUS_NOERROR != ret) {
			printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
			goto release_buffer;
		}
	}

	ret = imrectangleArray(dst, dst_rect, 4, 0xff00ff00, 2);
	if (ret == IM_STATUS_SUCCESS) {
		printf("%s running success!\n", LOG_TAG);
	} else {
		printf("%s running failed, %s\n", LOG_TAG, imStrError((IM_STATUS)ret));
		goto release_buffer;
	}

	printf("output [0x%x, 0x%x, 0x%x, 0x%x]\n", dst_buf[0], dst_buf[1], dst_buf[2], dst_buf[3]);
	write_image_to_file(dst_buf, LOCAL_FILE_PATH, dst_width, dst_height, dst_format, 0);

release_buffer:
	if (dst_handle > 0)
		releasebuffer_handle(dst_handle);

free_buf:
	dma_buf_free(dst_buf_size, &dst_dma_fd, dst_buf);

	return 0;
}

int rga_fill_rectangle_demo(void) {
	int ret = 0;
	int64_t ts;
	int dst_width, dst_height, dst_format;
	int dst_buf_size;
	char *dst_buf;
	int dst_dma_fd;
	rga_buffer_t dst = {};
	im_rect dst_rect = {};
	rga_buffer_handle_t dst_handle;

	// Allocate dma_buf within 4G from dma32_heap,
	// return dma_fd and virtual address.
	// ColorFill can only be used on buffers within 4G.
	ret = dma_buf_alloc(DMA_HEAP_DMA32_UNCACHE_PATCH, dst_buf_size, &dst_dma_fd, (void **)&dst_buf);
	if (ret < 0) {
		printf("alloc dma32_heap buffer failed!\n");
		return -1;
	}

	memset(dst_buf, 0x33, dst_buf_size);

	// Import the allocated dma_fd into RGA by calling
	// importbuffer_fd, and use the returned buffer_handle
	// to call RGA to process the image.

	dst_handle = importbuffer_fd(dst_dma_fd, dst_buf_size);
	if (dst_handle == 0) {
		printf("import dma_fd error!\n");
		ret = -1;
		goto free_buf;
	}

	dst = wrapbuffer_handle(dst_handle, dst_width, dst_height, dst_format);

	// Fills a rectangular area on the dst image with the specified color.
	//       dst_image
	//    --------------
	//    | -------    |
	//    | |rect |    |
	//    | -------    |
	//    --------------

	dst_rect.x = 0;
	dst_rect.y = 0;
	dst_rect.width = 300;
	dst_rect.height = 200;

	ret = imcheck({}, dst, {}, dst_rect, IM_COLOR_FILL);
	if (IM_STATUS_NOERROR != ret) {
		printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
		goto release_buffer;
	}

	ret = imrectangle(dst, dst_rect, 0xff00ff00, 2);
	if (ret == IM_STATUS_SUCCESS) {
		printf("%s running success!\n", LOG_TAG);
	} else {
		printf("%s running failed, %s\n", LOG_TAG, imStrError((IM_STATUS)ret));
		goto release_buffer;
	}

	printf("output [0x%x, 0x%x, 0x%x, 0x%x]\n", dst_buf[0], dst_buf[1], dst_buf[2], dst_buf[3]);
	write_image_to_file(dst_buf, LOCAL_FILE_PATH, dst_width, dst_height, dst_format, 0);

release_buffer:
	if (dst_handle > 0)
		releasebuffer_handle(dst_handle);

free_buf:
	dma_buf_free(dst_buf_size, &dst_dma_fd, dst_buf);

	return 0;
}

int rga_copy_demo() {
	int ret = 0;
	int src_width, src_height, src_format;
	int dst_width, dst_height, dst_format;
	char *src_buf, *dst_buf;
	int src_buf_size, dst_buf_size;

	rga_buffer_t src_img, dst_img;
	rga_buffer_handle_t src_handle, dst_handle;

	src_handle = importbuffer_virtualaddr(src_buf, src_buf_size);
	dst_handle = importbuffer_virtualaddr(dst_buf, dst_buf_size);
	if (src_handle == 0 || dst_handle == 0) {
		printf("importbuffer failed!\n");
		goto release_buffer;
	}

	src_img = wrapbuffer_handle(src_handle, src_width, src_height, src_format);
	dst_img = wrapbuffer_handle(dst_handle, dst_width, dst_height, dst_format);

	// Copy the src image to the dst buffer.
	//   --------------        --------------
	//   |            |        |            |
	//   |  src_image |   =>   |  dst_image |
	//   |            |        |            |
	//   --------------        --------------

	ret = imcheck(src_img, dst_img, {}, {});
	if (IM_STATUS_NOERROR != ret) {
		printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
		return -1;
	}

	ret = imcopy(src_img, dst_img);
	if (ret == IM_STATUS_SUCCESS) {
		printf("%s running success!\n", LOG_TAG);
	} else {
		printf("%s running failed, %s\n", LOG_TAG, imStrError((IM_STATUS)ret));
		goto release_buffer;
	}

	printf("output [0x%x, 0x%x, 0x%x, 0x%x]\n", dst_buf[0], dst_buf[1], dst_buf[2], dst_buf[3]);
	write_image_to_file(dst_buf, LOCAL_FILE_PATH, dst_width, dst_height, dst_format, 0);

release_buffer:
	if (src_handle)
		releasebuffer_handle(src_handle);
	if (dst_handle)
		releasebuffer_handle(dst_handle);

	if (src_buf)
		free(src_buf);
	if (dst_buf)
		free(dst_buf);

	return ret;
}

int rga_copy_fbc_demo() {
	int ret = 0;
	int src_width, src_height, src_format;
	int dst_width, dst_height, dst_format;
	char *src_buf, *dst_buf, *fbc_buf;
	int src_buf_size, dst_buf_size, fbc_buf_size;

	rga_buffer_t src_img, dst_img, fbc_img;
	rga_buffer_handle_t src_handle, dst_handle, fbc_handle;

	src_handle = importbuffer_virtualaddr(src_buf, src_buf_size);
	dst_handle = importbuffer_virtualaddr(dst_buf, dst_buf_size);
	fbc_handle = importbuffer_virtualaddr(fbc_buf, fbc_buf_size);
	if (src_handle == 0 || dst_handle == 0) {
		printf("importbuffer failed!\n");
		goto release_buffer;
	}

	src_img = wrapbuffer_handle(src_handle, src_width, src_height, src_format);
	dst_img = wrapbuffer_handle(dst_handle, dst_width, dst_height, dst_format);
	fbc_img = wrapbuffer_handle(fbc_handle, dst_width, dst_height, dst_format);

	// Copy the src(RASTER) image to the fbc(FBC) image.
	//        --------------        --------------
	//        |            |        |            |
	//        |  src_image |   =>   |  fbc_image |
	//        |            |        |            |
	//        --------------        --------------

	src_img.rd_mode = IM_RASTER_MODE;
	fbc_img.rd_mode = IM_FBC_MODE;

	ret = imcopy(src_img, fbc_img);
	if (ret == IM_STATUS_SUCCESS) {
		printf("%s RASTER -> FBC running success!\n", LOG_TAG);
	} else {
		printf("%s RASTER -> FBC running failed, %s\n", LOG_TAG, imStrError((IM_STATUS)ret));
		goto release_buffer;
	}

	printf("FBC [0x%x, 0x%x, 0x%x, 0x%x]\n", fbc_buf[0], fbc_buf[1], fbc_buf[2], fbc_buf[3]);
	write_image_to_fbc_file(fbc_buf, LOCAL_FILE_PATH, dst_width, dst_height, dst_format, 0);

	// Copy the fbc(FBC) image to the dst(RASTER) image.
	//   --------------        --------------
	//   |            |        |            |
	//   |  fbc_image |   =>   |  dst_image |
	//   |            |        |            |
	//   --------------        --------------

	fbc_img.rd_mode = IM_FBC_MODE;
	dst_img.rd_mode = IM_RASTER_MODE;

	ret = imcopy(fbc_img, dst_img);
	if (ret == IM_STATUS_SUCCESS) {
		printf("%s FBC -> RASTER running success!\n", LOG_TAG);
	} else {
		printf("%s FBC -> RASTER running failed, %s\n", LOG_TAG, imStrError((IM_STATUS)ret));
		goto release_buffer;
	}

	printf("RASTER [0x%x, 0x%x, 0x%x, 0x%x]\n", dst_buf[0], dst_buf[1], dst_buf[2], dst_buf[3]);
	write_image_to_file(dst_buf, LOCAL_FILE_PATH, dst_width, dst_height, dst_format, 1);

release_buffer:
	if (src_handle)
		releasebuffer_handle(src_handle);
	if (dst_handle)
		releasebuffer_handle(dst_handle);
	if (fbc_handle)
		releasebuffer_handle(fbc_handle);

	if (src_buf)
		free(src_buf);
	if (dst_buf)
		free(dst_buf);
	if (fbc_buf)
		free(fbc_buf);

	return ret;
}

int rga_copy_splice_demo() {
	int ret = 0;
	int left_width, left_height, left_format;
	int right_width, right_height, right_format;
	int dst_width, dst_height, dst_format;
	char *left_buf, *right_buf, *dst_buf;
	int left_buf_size, right_buf_size, dst_buf_size;

	rga_buffer_t left_img, right_img, dst_img;
	im_rect left_rect, right_rect;
	rga_buffer_handle_t left_handle, right_handle, dst_handle;

	left_handle = importbuffer_virtualaddr(left_buf, left_buf_size);
	right_handle = importbuffer_virtualaddr(right_buf, left_buf_size);
	dst_handle = importbuffer_virtualaddr(dst_buf, dst_buf_size);
	if (left_handle == 0 || dst_handle == 0) {
		printf("importbuffer failed!\n");
		goto release_buffer;
	}

	left_img = wrapbuffer_handle(left_handle, left_width, left_height, left_format);
	right_img = wrapbuffer_handle(right_handle, right_width, right_height, right_format);
	dst_img = wrapbuffer_handle(dst_handle, dst_width, dst_height, dst_format);

	// Splicing the left and right images to output.
	//   ---------------------------
	//   |            |            |
	//   | left_rect  | right_rect |
	//   |            |            |
	//   ---------------------------

	// 1). copy left image.
	//                                dst_img
	//    --------------    ---------------------------
	//    |            |    |            |            |
	//    |  left_img  | => |  left_rect |            |
	//    |            |    |            |            |
	//    --------------    ---------------------------

	left_rect.x = 0;
	left_rect.y = 0;
	left_rect.width = left_width;
	left_rect.height = left_height;

	ret = imcheck(left_img, dst_img, {}, left_rect);
	if (IM_STATUS_NOERROR != ret) {
		printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
		goto release_buffer;
	}

	ret = improcess(left_img, dst_img, {}, {}, left_rect, {}, IM_SYNC);
	if (ret == IM_STATUS_SUCCESS) {
		printf("%s left running success!\n", LOG_TAG);
	} else {
		printf("%s left running failed, %s\n", LOG_TAG, imStrError((IM_STATUS)ret));
		goto release_buffer;
	}

	// 2). copy right image.
	//                                dst_img
	//    --------------    ---------------------------
	//    |            |    |            |            |
	//    | right_img  | => |            | right_rect |
	//    |            |    |            |            |
	//    --------------    ---------------------------

	right_rect.x = left_width;
	right_rect.y = 0;
	right_rect.width = right_width;
	right_rect.height = right_height;

	ret = imcheck(right_img, dst_img, {}, right_rect);
	if (IM_STATUS_NOERROR != ret) {
		printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
		goto release_buffer;
	}

	ret = improcess(right_img, dst_img, {}, {}, right_rect, {}, IM_SYNC);
	if (ret == IM_STATUS_SUCCESS) {
		printf("%s right running success!\n", LOG_TAG);
	} else {
		printf("%s right running failed, %s\n", LOG_TAG, imStrError((IM_STATUS)ret));
		goto release_buffer;
	}

	printf("output [0x%x, 0x%x, 0x%x, 0x%x]\n", dst_buf[0], dst_buf[1], dst_buf[2], dst_buf[3]);
	write_image_to_file(dst_buf, LOCAL_FILE_PATH, dst_width, dst_height, dst_format, 0);

release_buffer:
	if (left_handle)
		releasebuffer_handle(left_handle);
	if (dst_handle)
		releasebuffer_handle(dst_handle);

	if (left_buf)
		free(left_buf);
	if (dst_buf)
		free(dst_buf);

	return ret;
}

int rga_copy_splice_task_demo() {
	int ret = 0;
	int left_width, left_height, left_format;
	int right_width, right_height, right_format;
	int dst_width, dst_height, dst_format;
	char *left_buf, *right_buf, *dst_buf;
	int left_buf_size, right_buf_size, dst_buf_size;

	rga_buffer_t left_img, right_img, dst_img;
	im_rect left_rect, right_rect;
	rga_buffer_handle_t left_handle, right_handle, dst_handle;
	im_job_handle_t job_handle;

	left_handle = importbuffer_virtualaddr(left_buf, left_buf_size);
	right_handle = importbuffer_virtualaddr(right_buf, left_buf_size);
	dst_handle = importbuffer_virtualaddr(dst_buf, dst_buf_size);
	if (left_handle == 0 || dst_handle == 0) {
		printf("importbuffer failed!\n");
		goto release_buffer;
	}

	left_img = wrapbuffer_handle(left_handle, left_width, left_height, left_format);
	right_img = wrapbuffer_handle(right_handle, right_width, right_height, right_format);
	dst_img = wrapbuffer_handle(dst_handle, dst_width, dst_height, dst_format);

	// Splicing the left and right images to output.
	//   ---------------------------
	//   |            |            |
	//   | left_rect  | right_rect |
	//   |            |            |
	//   ---------------------------

	/* 1). Create a job handle. */
	job_handle = imbeginJob();
	if (job_handle <= 0) {
		printf("job begin failed![%d], %s\n", job_handle, imStrError());
		goto release_buffer;
	}

	// 2). Add a task of copying the left image.
	//                               dst_img
	//   --------------    ---------------------------
	//   |            |    |            |            |
	//   |  left_img  | => |  left_rect |            |
	//   |            |    |            |            |
	//   --------------    ---------------------------

	left_rect.x = 0;
	left_rect.y = 0;
	left_rect.width = left_width;
	left_rect.height = left_height;

	ret = imcheck(left_img, dst_img, {}, left_rect);
	if (IM_STATUS_NOERROR != ret) {
		printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
		goto cancel_job;
	}

	ret = improcessTask(job_handle, left_img, dst_img, {}, {}, left_rect, {}, NULL, IM_SYNC);
	if (ret == IM_STATUS_SUCCESS) {
		printf("%s job[%d] add left task success!\n", LOG_TAG, job_handle);
	} else {
		printf("%s job[%d] add left task failed, %s\n", LOG_TAG, job_handle, imStrError((IM_STATUS)ret));
		goto cancel_job;
	}

	// 3). Add a task of copying the right image.
	//                               dst_img
	//   --------------    ---------------------------
	//   |            |    |            |            |
	//   | right_img  | => |            | right_rect |
	//   |            |    |            |            |
	//   --------------    ---------------------------

	right_rect.x = left_width;
	right_rect.y = 0;
	right_rect.width = right_width;
	right_rect.height = right_height;

	ret = imcheck(right_img, dst_img, {}, right_rect);
	if (IM_STATUS_NOERROR != ret) {
		printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
		goto cancel_job;
	}

	ret = improcessTask(job_handle, right_img, dst_img, {}, {}, right_rect, {}, NULL, IM_SYNC);
	if (ret == IM_STATUS_SUCCESS) {
		printf("%s job[%d] add right task success!\n", LOG_TAG, job_handle);
	} else {
		printf("%s job[%d] add right task failed, %s\n", LOG_TAG, job_handle, imStrError((IM_STATUS)ret));
		goto cancel_job;
	}

	/*
	 * 4). Submit and wait for the job to complete.
	 */
	ret = imendJob(job_handle);
	if (ret == IM_STATUS_SUCCESS) {
		printf("%s job[%d] running success!\n", LOG_TAG, job_handle);
	} else {
		printf("%s job[%d] running failed, %s\n", LOG_TAG, job_handle, imStrError((IM_STATUS)ret));
		goto release_buffer;
	}

	printf("output [0x%x, 0x%x, 0x%x, 0x%x]\n", dst_buf[0], dst_buf[1], dst_buf[2], dst_buf[3]);
	write_image_to_file(dst_buf, LOCAL_FILE_PATH, dst_width, dst_height, dst_format, 0);

cancel_job:
	imcancelJob(job_handle);

release_buffer:
	if (left_handle)
		releasebuffer_handle(left_handle);
	if (dst_handle)
		releasebuffer_handle(dst_handle);

	if (left_buf)
		free(left_buf);
	if (dst_buf)
		free(dst_buf);

	return ret;
}

int rga_copy_tile8x8_demo() {
	int ret = 0;
	int src_width, src_height, src_format;
	int dst_width, dst_height, dst_format;
	char *src_buf, *dst_buf, *tile8_buf;
	int src_buf_size, dst_buf_size;

	rga_buffer_t src_img, dst_img, tile8_img;
	rga_buffer_handle_t src_handle, dst_handle, tile8_handle;

	src_handle = importbuffer_virtualaddr(src_buf, src_buf_size);
	dst_handle = importbuffer_virtualaddr(dst_buf, dst_buf_size);
	tile8_handle = importbuffer_virtualaddr(tile8_buf, dst_buf_size);
	if (src_handle == 0 || dst_handle == 0) {
		printf("importbuffer failed!\n");
		goto release_buffer;
	}

	src_img = wrapbuffer_handle(src_handle, src_width, src_height, src_format);
	dst_img = wrapbuffer_handle(dst_handle, dst_width, dst_height, dst_format);
	tile8_img = wrapbuffer_handle(tile8_handle, dst_width, dst_height, dst_format);

	// Copy the src(raster) image to the tile8(tile8x8) image.
	//   --------------        --------------
	//   |            |        |            |
	//   |  src_image |   =>   | tile8_image|
	//   |            |        |            |
	//   --------------        --------------

	src_img.rd_mode = IM_RASTER_MODE;
	tile8_img.rd_mode = IM_TILE_MODE;

	ret = imcopy(src_img, tile8_img);
	if (ret == IM_STATUS_SUCCESS) {
		printf("%s raster -> tile8x8 running success!\n", LOG_TAG);
	} else {
		printf("%s raster -> tile8x8 running failed, %s\n", LOG_TAG, imStrError((IM_STATUS)ret));
		goto release_buffer;
	}

	printf("tile8x8 [0x%x, 0x%x, 0x%x, 0x%x]\n", tile8_buf[0], tile8_buf[1], tile8_buf[2], tile8_buf[3]);
	write_image_to_file(tile8_buf, LOCAL_FILE_PATH, dst_width, dst_height, dst_format, 0);

	// Copy the tile8(tile8x8) image to the dst(raster) image.
	//   --------------        --------------
	//   |            |        |            |
	//   | tile8_image|   =>   |  dst_image |
	//   |            |        |            |
	//   --------------        --------------

	tile8_img.rd_mode = IM_TILE_MODE;
	dst_img.rd_mode = IM_RASTER_MODE;

	ret = imcopy(tile8_img, dst_img);
	if (ret == IM_STATUS_SUCCESS) {
		printf("%s tile8x8 -> raster running success!\n", LOG_TAG);
	} else {
		printf("%s tile8x8 -> raster running failed, %s\n", LOG_TAG, imStrError((IM_STATUS)ret));
		goto release_buffer;
	}

	printf("raster [0x%x, 0x%x, 0x%x, 0x%x]\n", dst_buf[0], dst_buf[1], dst_buf[2], dst_buf[3]);
	write_image_to_file(dst_buf, LOCAL_FILE_PATH, dst_width, dst_height, dst_format, 1);

release_buffer:
	if (src_handle)
		releasebuffer_handle(src_handle);
	if (dst_handle)
		releasebuffer_handle(dst_handle);
	if (tile8_handle)
		releasebuffer_handle(tile8_handle);

	if (src_buf)
		free(src_buf);
	if (dst_buf)
		free(dst_buf);
	if (tile8_buf)
		free(tile8_buf);

	return ret;
}

int rga_crop_demo() {
	int ret = 0;
	int src_width, src_height, src_format;
	int dst_width, dst_height, dst_format;
	char *src_buf, *dst_buf;
	int src_buf_size, dst_buf_size;

	rga_buffer_t src_img, dst_img;
	im_rect rect;
	rga_buffer_handle_t src_handle, dst_handle;

	src_handle = importbuffer_virtualaddr(src_buf, src_buf_size);
	dst_handle = importbuffer_virtualaddr(dst_buf, dst_buf_size);
	if (src_handle == 0 || dst_handle == 0) {
		printf("importbuffer failed!\n");
		goto release_buffer;
	}

	src_img = wrapbuffer_handle(src_handle, src_width, src_height, src_format);
	dst_img = wrapbuffer_handle(dst_handle, dst_width, dst_height, dst_format);

	// Crop a rectangular area(200*200) of src.
	//   ---------------------------    --------------
	//   |        src_img          |    |            |
	//   |     --------------      |    |  dst_img   |
	//   |     |  src_rect  |      | => |            |
	//   |     |            |      |    --------------
	//   |     --------------      |
	//   ---------------------------

	rect.x = 100;
	rect.y = 100;
	rect.width = 200;
	rect.height = 200;

	ret = imcheck(src_img, dst_img, {}, {});
	if (IM_STATUS_NOERROR != ret) {
		printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
		return -1;
	}

	ret = imcrop(src_img, dst_img, rect);
	if (ret == IM_STATUS_SUCCESS) {
		printf("%s running success!\n", LOG_TAG);
	} else {
		printf("%s running failed, %s\n", LOG_TAG, imStrError((IM_STATUS)ret));
		goto release_buffer;
	}

	write_image_to_file(dst_buf, LOCAL_FILE_PATH, dst_width, dst_height, dst_format, 0);

release_buffer:
	if (src_handle)
		releasebuffer_handle(src_handle);
	if (dst_handle)
		releasebuffer_handle(dst_handle);

	if (src_buf)
		free(src_buf);
	if (dst_buf)
		free(dst_buf);

	return ret;
}

int rga_crop_rect_demo() {
	int ret = 0;
	int src_width, src_height, src_format;
	int dst_width, dst_height, dst_format;
	char *src_buf, *dst_buf;
	int src_buf_size, dst_buf_size;

	rga_buffer_t src_img, dst_img;
	im_rect src_rect, dst_rect;
	rga_buffer_handle_t src_handle, dst_handle;

	src_handle = importbuffer_virtualaddr(src_buf, src_buf_size);
	dst_handle = importbuffer_virtualaddr(dst_buf, dst_buf_size);
	if (src_handle == 0 || dst_handle == 0) {
		printf("importbuffer failed!\n");
		goto release_buffer;
	}

	src_img = wrapbuffer_handle(src_handle, src_width, src_height, src_format);
	dst_img = wrapbuffer_handle(dst_handle, dst_width, dst_height, dst_format);

	// Copy a rectangular area on the src image to a rectangular area in dst image.
	//   ---------------------------      ---------------------------
	//   |        src_img          |      |        dst_img          |
	//   |     --------------      |      |                         |
	//   |     |            |      |      |     --------------      |
	//   |     |  src_rect  |      |  =>  |     |            |      |
	//   |     |            |      |      |     |  dst_rect  |      |
	//   |     --------------      |      |     |            |      |
	//   |                         |      |     --------------      |
	//   |                         |      |                         |
	//   ---------------------------      ---------------------------

	src_rect.x = 100;
	src_rect.y = 100;
	src_rect.width = 200;
	src_rect.height = 200;

	dst_rect.x = 200;
	dst_rect.y = 200;
	dst_rect.width = 200;
	dst_rect.height = 200;

	ret = imcheck(src_img, dst_img, src_rect, dst_rect);
	if (IM_STATUS_NOERROR != ret) {
		printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
		return -1;
	}

	ret = improcess(src_img, dst_img, {}, src_rect, dst_rect, {}, IM_SYNC);
	if (ret == IM_STATUS_SUCCESS) {
		printf("%s running success!\n", LOG_TAG);
	} else {
		printf("%s running failed, %s\n", LOG_TAG, imStrError((IM_STATUS)ret));
		goto release_buffer;
	}

	write_image_to_file(dst_buf, LOCAL_FILE_PATH, dst_width, dst_height, dst_format, 0);

release_buffer:
	if (src_handle)
		releasebuffer_handle(src_handle);
	if (dst_handle)
		releasebuffer_handle(dst_handle);

	if (src_buf)
		free(src_buf);
	if (dst_buf)
		free(dst_buf);

	return ret;
}

int rga_alpha_demo() {
	int ret = 0;
	int fg_width, fg_height, fg_format;
	int bg_width, bg_height, bg_format;
	char *fg_buf, *bg_buf;
	int fg_buf_size, bg_buf_size;

	rga_buffer_t fg_img, bg_img;
	rga_buffer_handle_t fg_handle, bg_handle;

	fg_handle = importbuffer_virtualaddr(fg_buf, fg_buf_size);
	bg_handle = importbuffer_virtualaddr(bg_buf, bg_buf_size);
	if (fg_handle == 0 || bg_handle == 0) {
		printf("importbuffer failed!\n");
		goto release_buffer;
	}

	fg_img = wrapbuffer_handle(fg_handle, fg_width, fg_height, fg_format);
	bg_img = wrapbuffer_handle(bg_handle, bg_width, bg_height, bg_format);

	// Here are two RGBA8888 images of the same size for src_over overlay.
	//   --------------        --------------      --------------
	//   |            |        |            |      |            |
	//   |   fg_img   |    +   |   bg_img   |  =>  | fg over bg |
	//   |            |        |            |      |            |
	//   --------------        --------------      --------------

	ret = imcheck(fg_img, bg_img, {}, {});
	if (IM_STATUS_NOERROR != ret) {
		printf("%d, check error! %s", __LINE__, imStrError((IM_STATUS)ret));
		return -1;
	}

	ret = imblend(fg_img, bg_img, IM_ALPHA_BLEND_SRC_OVER | IM_ALPHA_BLEND_PRE_MUL);
	if (ret == IM_STATUS_SUCCESS) {
		printf("%s running success!\n", LOG_TAG);
	} else {
		printf("%s running failed, %s\n", LOG_TAG, imStrError((IM_STATUS)ret));
		goto release_buffer;
	}

	write_image_to_file(bg_buf, LOCAL_FILE_PATH, bg_width, bg_height, bg_format, 0);

release_buffer:
	if (fg_handle)
		releasebuffer_handle(fg_handle);
	if (bg_handle)
		releasebuffer_handle(bg_handle);

	if (fg_buf)
		free(fg_buf);
	if (bg_buf)
		free(bg_buf);

	return ret;
}
