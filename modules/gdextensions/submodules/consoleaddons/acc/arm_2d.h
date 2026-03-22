/**************************************************************************/
/*  arm_2d.h                                                              */
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

// Optimized 2D pixel-processing kernels inspired by ARM-software/Arm-2D.
// Provides SIMD-accelerated (NEON/Helium) paths for common image operations
// with portable C fallbacks. No Godot dependencies — operates on raw buffers.
//
// Reference: https://github.com/ARM-software/Arm-2D (Apache 2.0)

#ifndef ARM_2D_ACCEL_H
#define ARM_2D_ACCEL_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#define ARM2D_HAS_NEON 1
#else
#define ARM2D_HAS_NEON 0
#endif

#if defined(__ARM_FEATURE_MVE) && __ARM_FEATURE_MVE
#define ARM2D_HAS_HELIUM 1
#else
#define ARM2D_HAS_HELIUM 0
#endif

// ---------------------------------------------------------------------------
// Tile copy (blit) — row-based memcpy
// ---------------------------------------------------------------------------

// Copy a rectangular region row by row. Much faster than per-pixel copy when
// source and destination have different strides.
static inline void arm2d_blit_rect(
		const uint8_t *p_src, int p_src_stride,
		uint8_t *p_dst, int p_dst_stride,
		int p_width, int p_height, int p_pixel_size) {
	const int row_bytes = p_width * p_pixel_size;
	for (int y = 0; y < p_height; y++) {
		memcpy(p_dst, p_src, row_bytes);
		p_src += p_src_stride;
		p_dst += p_dst_stride;
	}
}

// ---------------------------------------------------------------------------
// Alpha blending — RGBA8 (4 bytes per pixel, A in byte 3)
// ---------------------------------------------------------------------------
// Blend formula (premultiplied-compatible):
//   dst.rgb = src.rgb * src.a / 255 + dst.rgb * (255 - src.a) / 255
//   dst.a   = src.a   + dst.a * (255 - src.a) / 255
//
// This matches Godot's Color::blend() for RGBA8 pixel layout.

#if ARM2D_HAS_NEON

// Process 4 RGBA8 pixels at once using NEON.
static inline void arm2d_blend_rgba8_neon(const uint8_t *p_src, uint8_t *p_dst, int p_count) {
	int i = 0;

	// Process 8 pixels at a time (deinterleave into R,G,B,A channels)
	for (; i + 7 < p_count; i += 8) {
		uint8x8x4_t src_px = vld4_u8(p_src + i * 4);
		uint8x8x4_t dst_px = vld4_u8(p_dst + i * 4);

		// src alpha and inverse alpha
		uint8x8_t sa = src_px.val[3];
		uint8x8_t inv_sa = vsub_u8(vdup_n_u8(255), sa);

		// Widen to 16-bit for multiply
		uint16x8_t sr = vmull_u8(src_px.val[0], sa);
		uint16x8_t sg = vmull_u8(src_px.val[1], sa);
		uint16x8_t sb = vmull_u8(src_px.val[2], sa);

		sr = vmlal_u8(sr, dst_px.val[0], inv_sa);
		sg = vmlal_u8(sg, dst_px.val[1], inv_sa);
		sb = vmlal_u8(sb, dst_px.val[2], inv_sa);

		// Alpha: sa + da * (255 - sa) / 255
		uint16x8_t s_alpha = vmull_u8(dst_px.val[3], inv_sa);

		// Divide by 255 using the (x + 128) >> 8 approximation
		// More accurate: (x + 1 + (x >> 8)) >> 8
		uint8x8_t out_r = vshrn_n_u16(vaddq_u16(sr, vdupq_n_u16(128)), 8);
		uint8x8_t out_g = vshrn_n_u16(vaddq_u16(sg, vdupq_n_u16(128)), 8);
		uint8x8_t out_b = vshrn_n_u16(vaddq_u16(sb, vdupq_n_u16(128)), 8);
		uint8x8_t out_a = vadd_u8(sa, vshrn_n_u16(vaddq_u16(s_alpha, vdupq_n_u16(128)), 8));

		uint8x8x4_t out_px = { { out_r, out_g, out_b, out_a } };
		vst4_u8(p_dst + i * 4, out_px);
	}

	// Scalar tail
	for (; i < p_count; i++) {
		const uint8_t *s = p_src + i * 4;
		uint8_t *d = p_dst + i * 4;
		uint32_t sa = s[3];
		if (sa == 0) {
			continue;
		}
		if (sa == 255) {
			memcpy(d, s, 4);
			continue;
		}
		uint32_t inv_sa = 255 - sa;
		d[0] = (uint8_t)((s[0] * sa + d[0] * inv_sa + 128) >> 8);
		d[1] = (uint8_t)((s[1] * sa + d[1] * inv_sa + 128) >> 8);
		d[2] = (uint8_t)((s[2] * sa + d[2] * inv_sa + 128) >> 8);
		d[3] = (uint8_t)(sa + ((d[3] * inv_sa + 128) >> 8));
	}
}

#endif // ARM2D_HAS_NEON

// Scalar alpha blend for RGBA8 — portable C, still much faster than
// per-pixel get_pixel/Color::blend/set_pixel.
static inline void arm2d_blend_rgba8_scalar(const uint8_t *p_src, uint8_t *p_dst, int p_count) {
	for (int i = 0; i < p_count; i++) {
		const uint8_t *s = p_src + i * 4;
		uint8_t *d = p_dst + i * 4;
		uint32_t sa = s[3];
		if (sa == 0) {
			continue;
		}
		if (sa == 255) {
			memcpy(d, s, 4);
			continue;
		}
		uint32_t inv_sa = 255 - sa;
		d[0] = (uint8_t)((s[0] * sa + d[0] * inv_sa + 128) >> 8);
		d[1] = (uint8_t)((s[1] * sa + d[1] * inv_sa + 128) >> 8);
		d[2] = (uint8_t)((s[2] * sa + d[2] * inv_sa + 128) >> 8);
		d[3] = (uint8_t)(sa + ((d[3] * inv_sa + 128) >> 8));
	}
}

// Dispatch to best available implementation.
static inline void arm2d_blend_rgba8(const uint8_t *p_src, uint8_t *p_dst, int p_count) {
#if ARM2D_HAS_NEON
	arm2d_blend_rgba8_neon(p_src, p_dst, p_count);
#else
	arm2d_blend_rgba8_scalar(p_src, p_dst, p_count);
#endif
}

// Blend a rectangular region of RGBA8 pixels.
static inline void arm2d_blend_rect_rgba8(
		const uint8_t *p_src, int p_src_stride,
		uint8_t *p_dst, int p_dst_stride,
		int p_width, int p_height) {
	for (int y = 0; y < p_height; y++) {
		arm2d_blend_rgba8(p_src, p_dst, p_width);
		p_src += p_src_stride;
		p_dst += p_dst_stride;
	}
}

// ---------------------------------------------------------------------------
// Alpha blending — RGB8 with separate alpha mask (A8)
// ---------------------------------------------------------------------------

static inline void arm2d_blend_rgb8_with_mask(
		const uint8_t *p_src, const uint8_t *p_mask,
		uint8_t *p_dst, int p_count) {
	for (int i = 0; i < p_count; i++) {
		uint32_t sa = p_mask[i];
		if (sa == 0) {
			continue;
		}
		const uint8_t *s = p_src + i * 3;
		uint8_t *d = p_dst + i * 3;
		if (sa == 255) {
			d[0] = s[0];
			d[1] = s[1];
			d[2] = s[2];
			continue;
		}
		uint32_t inv_sa = 255 - sa;
		d[0] = (uint8_t)((s[0] * sa + d[0] * inv_sa + 128) >> 8);
		d[1] = (uint8_t)((s[1] * sa + d[1] * inv_sa + 128) >> 8);
		d[2] = (uint8_t)((s[2] * sa + d[2] * inv_sa + 128) >> 8);
	}
}

// ---------------------------------------------------------------------------
// Color fill — fill a rectangular region with a solid color
// ---------------------------------------------------------------------------

static inline void arm2d_fill_rect(
		uint8_t *p_dst, int p_dst_stride,
		const uint8_t *p_color, int p_pixel_size,
		int p_width, int p_height) {
	// Fill first row pixel by pixel
	uint8_t *row = p_dst;
	for (int x = 0; x < p_width; x++) {
		memcpy(row + x * p_pixel_size, p_color, p_pixel_size);
	}
	// Copy first row to remaining rows
	for (int y = 1; y < p_height; y++) {
		memcpy(p_dst + y * p_dst_stride, p_dst, p_width * p_pixel_size);
	}
}

// Fill with opacity (RGBA8 destination, any color with alpha blending).
static inline void arm2d_fill_rect_rgba8_with_opacity(
		uint8_t *p_dst, int p_dst_stride,
		uint8_t p_r, uint8_t p_g, uint8_t p_b, uint8_t p_a,
		int p_width, int p_height) {
	if (p_a == 255) {
		uint8_t color[4] = { p_r, p_g, p_b, 255 };
		arm2d_fill_rect(p_dst, p_dst_stride, color, 4, p_width, p_height);
		return;
	}
	if (p_a == 0) {
		return;
	}
	uint32_t sa = p_a;
	uint32_t inv_sa = 255 - sa;
	uint32_t sr = p_r * sa;
	uint32_t sg = p_g * sa;
	uint32_t sb = p_b * sa;

	for (int y = 0; y < p_height; y++) {
		uint8_t *row = p_dst + y * p_dst_stride;
		for (int x = 0; x < p_width; x++) {
			uint8_t *d = row + x * 4;
			d[0] = (uint8_t)((sr + d[0] * inv_sa + 128) >> 8);
			d[1] = (uint8_t)((sg + d[1] * inv_sa + 128) >> 8);
			d[2] = (uint8_t)((sb + d[2] * inv_sa + 128) >> 8);
			d[3] = (uint8_t)(sa + ((d[3] * inv_sa + 128) >> 8));
		}
	}
}

// ---------------------------------------------------------------------------
// Row swap — for flip_y (swap two entire rows)
// ---------------------------------------------------------------------------

static inline void arm2d_swap_rows(uint8_t *p_row_a, uint8_t *p_row_b, int p_row_bytes) {
	// Use a stack buffer for small rows, heap for large
	uint8_t stack_buf[4096];
	uint8_t *tmp = (p_row_bytes <= (int)sizeof(stack_buf)) ? stack_buf : (uint8_t *)malloc(p_row_bytes);
	memcpy(tmp, p_row_a, p_row_bytes);
	memcpy(p_row_a, p_row_b, p_row_bytes);
	memcpy(p_row_b, tmp, p_row_bytes);
	if (tmp != stack_buf) {
		free(tmp);
	}
}

// ---------------------------------------------------------------------------
// Row reverse — for flip_x (reverse pixel order within a row)
// ---------------------------------------------------------------------------

static inline void arm2d_reverse_row(uint8_t *p_row, int p_width, int p_pixel_size) {
	uint8_t tmp[16]; // Max pixel size in Godot is 16 bytes (RGBAH = 8, RGBAF = 16)
	int left = 0;
	int right = p_width - 1;
	while (left < right) {
		uint8_t *l = p_row + left * p_pixel_size;
		uint8_t *r = p_row + right * p_pixel_size;
		memcpy(tmp, l, p_pixel_size);
		memcpy(l, r, p_pixel_size);
		memcpy(r, tmp, p_pixel_size);
		left++;
		right--;
	}
}

// Optimized 4-byte pixel reverse (RGBA8, RGBX, etc.)
static inline void arm2d_reverse_row_4bpp(uint8_t *p_row, int p_width) {
	uint32_t *pixels = (uint32_t *)p_row;
	int left = 0;
	int right = p_width - 1;
	while (left < right) {
		uint32_t tmp = pixels[left];
		pixels[left] = pixels[right];
		pixels[right] = tmp;
		left++;
		right--;
	}
}

// ---------------------------------------------------------------------------
// Color format conversion kernels
// ---------------------------------------------------------------------------

// RGBA8 → RGB8 (drop alpha)
static inline void arm2d_convert_rgba8_to_rgb8(const uint8_t *p_src, uint8_t *p_dst, int p_count) {
#if ARM2D_HAS_NEON
	int i = 0;
	for (; i + 7 < p_count; i += 8) {
		uint8x8x4_t src = vld4_u8(p_src + i * 4);
		uint8x8x3_t dst = { { src.val[0], src.val[1], src.val[2] } };
		vst3_u8(p_dst + i * 3, dst);
	}
	for (; i < p_count; i++) {
		p_dst[i * 3 + 0] = p_src[i * 4 + 0];
		p_dst[i * 3 + 1] = p_src[i * 4 + 1];
		p_dst[i * 3 + 2] = p_src[i * 4 + 2];
	}
#else
	for (int i = 0; i < p_count; i++) {
		p_dst[i * 3 + 0] = p_src[i * 4 + 0];
		p_dst[i * 3 + 1] = p_src[i * 4 + 1];
		p_dst[i * 3 + 2] = p_src[i * 4 + 2];
	}
#endif
}

// RGB8 → RGBA8 (add opaque alpha)
static inline void arm2d_convert_rgb8_to_rgba8(const uint8_t *p_src, uint8_t *p_dst, int p_count) {
#if ARM2D_HAS_NEON
	int i = 0;
	for (; i + 7 < p_count; i += 8) {
		uint8x8x3_t src = vld3_u8(p_src + i * 3);
		uint8x8x4_t dst = { { src.val[0], src.val[1], src.val[2], vdup_n_u8(255) } };
		vst4_u8(p_dst + i * 4, dst);
	}
	for (; i < p_count; i++) {
		p_dst[i * 4 + 0] = p_src[i * 3 + 0];
		p_dst[i * 4 + 1] = p_src[i * 3 + 1];
		p_dst[i * 4 + 2] = p_src[i * 3 + 2];
		p_dst[i * 4 + 3] = 255;
	}
#else
	for (int i = 0; i < p_count; i++) {
		p_dst[i * 4 + 0] = p_src[i * 3 + 0];
		p_dst[i * 4 + 1] = p_src[i * 3 + 1];
		p_dst[i * 4 + 2] = p_src[i * 3 + 2];
		p_dst[i * 4 + 3] = 255;
	}
#endif
}

// RGBA8 → L8 (grayscale, ITU-R BT.601 weights)
static inline void arm2d_convert_rgba8_to_gray8(const uint8_t *p_src, uint8_t *p_dst, int p_count) {
#if ARM2D_HAS_NEON
	int i = 0;
	// BT.601: Y = 0.299*R + 0.587*G + 0.114*B
	// Fixed-point: Y = (77*R + 150*G + 29*B + 128) >> 8
	uint8x8_t coeff_r = vdup_n_u8(77);
	uint8x8_t coeff_g = vdup_n_u8(150);
	uint8x8_t coeff_b = vdup_n_u8(29);
	for (; i + 7 < p_count; i += 8) {
		uint8x8x4_t src = vld4_u8(p_src + i * 4);
		uint16x8_t acc = vmull_u8(src.val[0], coeff_r);
		acc = vmlal_u8(acc, src.val[1], coeff_g);
		acc = vmlal_u8(acc, src.val[2], coeff_b);
		uint8x8_t gray = vshrn_n_u16(vaddq_u16(acc, vdupq_n_u16(128)), 8);
		vst1_u8(p_dst + i, gray);
	}
	for (; i < p_count; i++) {
		const uint8_t *s = p_src + i * 4;
		p_dst[i] = (uint8_t)((77 * s[0] + 150 * s[1] + 29 * s[2] + 128) >> 8);
	}
#else
	for (int i = 0; i < p_count; i++) {
		const uint8_t *s = p_src + i * 4;
		p_dst[i] = (uint8_t)((77 * s[0] + 150 * s[1] + 29 * s[2] + 128) >> 8);
	}
#endif
}

// L8 → RGBA8 (grayscale to RGBA with opaque alpha)
static inline void arm2d_convert_gray8_to_rgba8(const uint8_t *p_src, uint8_t *p_dst, int p_count) {
#if ARM2D_HAS_NEON
	int i = 0;
	for (; i + 7 < p_count; i += 8) {
		uint8x8_t gray = vld1_u8(p_src + i);
		uint8x8x4_t dst = { { gray, gray, gray, vdup_n_u8(255) } };
		vst4_u8(p_dst + i * 4, dst);
	}
	for (; i < p_count; i++) {
		uint8_t g = p_src[i];
		p_dst[i * 4 + 0] = g;
		p_dst[i * 4 + 1] = g;
		p_dst[i * 4 + 2] = g;
		p_dst[i * 4 + 3] = 255;
	}
#else
	for (int i = 0; i < p_count; i++) {
		uint8_t g = p_src[i];
		p_dst[i * 4 + 0] = g;
		p_dst[i * 4 + 1] = g;
		p_dst[i * 4 + 2] = g;
		p_dst[i * 4 + 3] = 255;
	}
#endif
}

// ---------------------------------------------------------------------------
// Color inversion (filter) — RGBA8
// ---------------------------------------------------------------------------

static inline void arm2d_invert_colors_rgba8(uint8_t *p_data, int p_count) {
#if ARM2D_HAS_NEON
	int i = 0;
	uint8x16_t ones = vdupq_n_u8(255);
	// Mask to preserve alpha: invert RGB, keep A
	uint8x16_t mask = { 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 255, 255, 0 };
	for (; i + 3 < p_count; i += 4) {
		uint8x16_t px = vld1q_u8(p_data + i * 4);
		uint8x16_t inv = veorq_u8(px, mask);
		vst1q_u8(p_data + i * 4, inv);
	}
	for (; i < p_count; i++) {
		uint8_t *d = p_data + i * 4;
		d[0] = 255 - d[0];
		d[1] = 255 - d[1];
		d[2] = 255 - d[2];
		// d[3] alpha unchanged
	}
#else
	for (int i = 0; i < p_count; i++) {
		uint8_t *d = p_data + i * 4;
		d[0] = 255 - d[0];
		d[1] = 255 - d[1];
		d[2] = 255 - d[2];
	}
#endif
}

// ---------------------------------------------------------------------------
// Premultiply alpha — RGBA8
// ---------------------------------------------------------------------------

static inline void arm2d_premultiply_alpha_rgba8(uint8_t *p_data, int p_count) {
#if ARM2D_HAS_NEON
	int i = 0;
	for (; i + 7 < p_count; i += 8) {
		uint8x8x4_t px = vld4_u8(p_data + i * 4);
		uint16x8_t r = vmull_u8(px.val[0], px.val[3]);
		uint16x8_t g = vmull_u8(px.val[1], px.val[3]);
		uint16x8_t b = vmull_u8(px.val[2], px.val[3]);
		px.val[0] = vshrn_n_u16(vaddq_u16(r, vdupq_n_u16(128)), 8);
		px.val[1] = vshrn_n_u16(vaddq_u16(g, vdupq_n_u16(128)), 8);
		px.val[2] = vshrn_n_u16(vaddq_u16(b, vdupq_n_u16(128)), 8);
		// alpha unchanged
		vst4_u8(p_data + i * 4, px);
	}
	for (; i < p_count; i++) {
		uint8_t *d = p_data + i * 4;
		uint32_t a = d[3];
		d[0] = (uint8_t)((d[0] * a + 128) >> 8);
		d[1] = (uint8_t)((d[1] * a + 128) >> 8);
		d[2] = (uint8_t)((d[2] * a + 128) >> 8);
	}
#else
	for (int i = 0; i < p_count; i++) {
		uint8_t *d = p_data + i * 4;
		uint32_t a = d[3];
		d[0] = (uint8_t)((d[0] * a + 128) >> 8);
		d[1] = (uint8_t)((d[1] * a + 128) >> 8);
		d[2] = (uint8_t)((d[2] * a + 128) >> 8);
	}
#endif
}

#endif // ARM_2D_ACCEL_H
