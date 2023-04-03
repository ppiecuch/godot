/**************************************************************************/
/*  blit_slow.c                                                           */
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

#include "_internal.h"

#include "blit.h"
#include "blit_slow.h"

// The ONE TRUE BLITTER
// This puppy has to handle all the unoptimized cases - yes, it's slow.
void SDL_Blit_Slow(SDL_BlitInfo *info) {
	const int flags = info->flags;
	const Uint32 modulateR = info->r;
	const Uint32 modulateG = info->g;
	const Uint32 modulateB = info->b;
	const Uint32 modulateA = info->a;
	Uint32 srcpixel;
	Uint32 srcR, srcG, srcB, srcA;
	Uint32 dstpixel;
	Uint32 dstR, dstG, dstB, dstA;
	int srcy, srcx;
	Uint32 posy, posx;
	int incy, incx;
	SDL_PixelFormat *src_fmt = info->src_fmt;
	SDL_PixelFormat *dst_fmt = info->dst_fmt;
	int srcbpp = src_fmt->BytesPerPixel;
	int dstbpp = dst_fmt->BytesPerPixel;
	Uint32 rgbmask = ~src_fmt->Amask;
	Uint32 ckey = info->colorkey & rgbmask;

	incy = (info->src_h << 16) / info->dst_h;
	incx = (info->src_w << 16) / info->dst_w;
	posy = incy / 2; /* start at the middle of pixel */

	while (info->dst_h--) {
		Uint8 *src = 0;
		Uint8 *dst = info->dst;
		int n = info->dst_w;
		posx = incx / 2; /* start at the middle of pixel */
		srcy = posy >> 16;
		while (n--) {
			srcx = posx >> 16;
			src = (info->src + (srcy * info->src_pitch) + (srcx * srcbpp));
			if (src_fmt->Amask) {
				DISEMBLE_RGBA(src, srcbpp, src_fmt, srcpixel, srcR, srcG,
						srcB, srcA);
			} else {
				DISEMBLE_RGB(src, srcbpp, src_fmt, srcpixel, srcR, srcG,
						srcB);
				srcA = 0xFF;
			}
			if (flags & SDL_COPY_COLORKEY) {
				/* srcpixel isn't set for 24 bpp */
				if (srcbpp == 3) {
					srcpixel = (srcR << src_fmt->Rshift) |
							(srcG << src_fmt->Gshift) | (srcB << src_fmt->Bshift);
				}
				if ((srcpixel & rgbmask) == ckey) {
					posx += incx;
					dst += dstbpp;
					continue;
				}
			}
			if (dst_fmt->Amask) {
				DISEMBLE_RGBA(dst, dstbpp, dst_fmt, dstpixel, dstR, dstG,
						dstB, dstA);
			} else {
				DISEMBLE_RGB(dst, dstbpp, dst_fmt, dstpixel, dstR, dstG,
						dstB);
				dstA = 0xFF;
			}

			if (flags & SDL_COPY_MODULATE_COLOR) {
				srcR = (srcR * modulateR) / 255;
				srcG = (srcG * modulateG) / 255;
				srcB = (srcB * modulateB) / 255;
			}
			if (flags & SDL_COPY_MODULATE_ALPHA) {
				srcA = (srcA * modulateA) / 255;
			}
			if (flags & (SDL_COPY_BLEND | SDL_COPY_ADD)) {
				/* This goes away if we ever use premultiplied alpha */
				if (srcA < 255) {
					srcR = (srcR * srcA) / 255;
					srcG = (srcG * srcA) / 255;
					srcB = (srcB * srcA) / 255;
				}
			}
			switch (flags & (SDL_COPY_BLEND | SDL_COPY_ADD | SDL_COPY_MOD | SDL_COPY_MUL)) {
				case 0:
					dstR = srcR;
					dstG = srcG;
					dstB = srcB;
					dstA = srcA;
					break;
				case SDL_COPY_BLEND:
					dstR = srcR + ((255 - srcA) * dstR) / 255;
					dstG = srcG + ((255 - srcA) * dstG) / 255;
					dstB = srcB + ((255 - srcA) * dstB) / 255;
					dstA = srcA + ((255 - srcA) * dstA) / 255;
					break;
				case SDL_COPY_ADD:
					dstR = srcR + dstR;
					if (dstR > 255)
						dstR = 255;
					dstG = srcG + dstG;
					if (dstG > 255)
						dstG = 255;
					dstB = srcB + dstB;
					if (dstB > 255)
						dstB = 255;
					break;
				case SDL_COPY_MOD:
					dstR = (srcR * dstR) / 255;
					dstG = (srcG * dstG) / 255;
					dstB = (srcB * dstB) / 255;
					break;
				case SDL_COPY_MUL:
					dstR = ((srcR * dstR) + (dstR * (255 - srcA))) / 255;
					if (dstR > 255)
						dstR = 255;
					dstG = ((srcG * dstG) + (dstG * (255 - srcA))) / 255;
					if (dstG > 255)
						dstG = 255;
					dstB = ((srcB * dstB) + (dstB * (255 - srcA))) / 255;
					if (dstB > 255)
						dstB = 255;
					dstA = ((srcA * dstA) + (dstA * (255 - srcA))) / 255;
					if (dstA > 255)
						dstA = 255;
					break;
			}
			if (dst_fmt->Amask) {
				ASSEMBLE_RGBA(dst, dstbpp, dst_fmt, dstR, dstG, dstB, dstA);
			} else {
				ASSEMBLE_RGB(dst, dstbpp, dst_fmt, dstR, dstG, dstB);
			}
			posx += incx;
			dst += dstbpp;
		}
		posy += incy;
		info->dst += info->dst_pitch;
	}
}

/* vi: set ts=4 sw=4 expandtab: */
