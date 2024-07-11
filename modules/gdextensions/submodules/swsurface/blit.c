/**************************************************************************/
/*  blit.c                                                                */
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

/*
 * Simple DirectMedia Layer
 * Copyright (C) 1997-2022 Sam Lantinga <slouken@libsdl.org>
 */

/// The general purpose software blit routine

#include "_internal.h"

#include "RLEaccel_c.h"
#include "blit.h"
#include "blit_auto.h"
#include "blit_copy.h"
#include "blit_slow.h"
#include "pixels_c.h"

static int SDLCALL SDL_SoftBlit(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
	// Set up source and destination buffer pointers, and BLIT!
	if (!SDL_RectEmpty(srcrect)) {
		SDL_BlitFunc RunBlit;
		SDL_BlitInfo *info = &src->map->info;

		// Set up the blit information
		info->src = (Uint8 *)src->pixels + (Uint16)srcrect->y * src->pitch + (Uint16)srcrect->x * info->src_fmt->BytesPerPixel;
		info->src_w = srcrect->w;
		info->src_h = srcrect->h;
		info->src_pitch = src->pitch;
		info->src_skip = info->src_pitch - info->src_w * info->src_fmt->BytesPerPixel;
		info->dst = (Uint8 *)dst->pixels + (Uint16)dstrect->y * dst->pitch + (Uint16)dstrect->x * info->dst_fmt->BytesPerPixel;
		info->dst_w = dstrect->w;
		info->dst_h = dstrect->h;
		info->dst_pitch = dst->pitch;
		info->dst_skip = info->dst_pitch - info->dst_w * info->dst_fmt->BytesPerPixel;
		RunBlit = (SDL_BlitFunc)src->map->data;

		// Run the actual software blit
		RunBlit(info);
	}

	// Blit is done!
	return (0);
}

#if SDL_HAVE_BLIT_AUTO

#ifdef __MACOSX__
#include <sys/sysctl.h>

static SDL_bool SDL_UseAltivecPrefetch() {
	const char key[] = "hw.l3cachesize";
	u_int64_t result = 0;
	size_t typeSize = sizeof(result);

	if (sysctlbyname(key, &result, &typeSize, NULL, 0) == 0 && result > 0) {
		return SDL_TRUE;
	} else {
		return SDL_FALSE;
	}
}

#else

static SDL_bool SDL_UseAltivecPrefetch() {
	return SDL_TRUE; // Just guess G4
}

#endif // __MACOSX__

static SDL_BlitFunc SDL_ChooseBlitFunc(Uint32 src_format, Uint32 dst_format, int flags, SDL_BlitFuncEntry *entries) {
	int flagcheck = (flags & (SDL_COPY_MODULATE_COLOR | SDL_COPY_MODULATE_ALPHA | SDL_COPY_BLEND | SDL_COPY_ADD | SDL_COPY_MOD | SDL_COPY_MUL | SDL_COPY_COLORKEY | SDL_COPY_NEAREST));
	static int features = 0x7fffffff;

	// Get the available CPU features
	if (features == 0x7fffffff) {
		const char *override = SDL_getenv("SDL_BLIT_CPU_FEATURES");

		features = SDL_CPU_ANY;

		// Allow an override for testing ..
		if (override) {
			SDL_sscanf(override, "%u", &features);
		} else {
			if (SDL_HasMMX()) {
				features |= SDL_CPU_MMX;
			}
			if (SDL_Has3DNow()) {
				features |= SDL_CPU_3DNOW;
			}
			if (SDL_HasSSE()) {
				features |= SDL_CPU_SSE;
			}
			if (SDL_HasSSE2()) {
				features |= SDL_CPU_SSE2;
			}
			if (SDL_HasAltiVec()) {
				if (SDL_UseAltivecPrefetch()) {
					features |= SDL_CPU_ALTIVEC_PREFETCH;
				} else {
					features |= SDL_CPU_ALTIVEC_NOPREFETCH;
				}
			}
		}
	}

	for (int i = 0; entries[i].func; ++i) {
		// Check for matching pixel formats
		if (src_format != entries[i].src_format) {
			continue;
		}
		if (dst_format != entries[i].dst_format) {
			continue;
		}

		// Check flags
		if ((flagcheck & entries[i].flags) != flagcheck) {
			continue;
		}

		// Check CPU features
		if ((entries[i].cpu & features) != entries[i].cpu) {
			continue;
		}

		// We found the best one!
		return entries[i].func;
	}
	return NULL;
}
#endif // SDL_HAVE_BLIT_AUTO

// Figure out which of many blit routines to set up on a surface
int SDL_CalculateBlit(SDL_Surface *surface) {
	SDL_BlitFunc blit = NULL;
	SDL_BlitMap *map = surface->map;
	SDL_Surface *dst = map->dst;

	// We don't currently support blitting to < 8 bpp surfaces
	if (dst->format->BitsPerPixel < 8) {
		SDL_InvalidateMap(map);
		return SDL_SetError("Blit combination not supported");
	}

#if SDL_HAVE_RLE
	// Clean everything out to start
	if ((surface->flags & SDL_RLEACCEL) == SDL_RLEACCEL) {
		SDL_UnRLESurface(surface, 1);
	}
#endif

	map->blit = SDL_SoftBlit;
	map->info.src_fmt = surface->format;
	map->info.src_pitch = surface->pitch;
	map->info.dst_fmt = dst->format;
	map->info.dst_pitch = dst->pitch;

#if SDL_HAVE_RLE
	// See if we can do RLE acceleration
	if (map->info.flags & SDL_COPY_RLE_DESIRED) {
		if (SDL_RLESurface(surface) == 0) {
			return 0;
		}
	}
#endif

	// Choose a standard blit function
	if (map->identity && !(map->info.flags & ~SDL_COPY_RLE_DESIRED)) {
		blit = SDL_BlitCopy;
	} else if (surface->format->Rloss > 8 || dst->format->Rloss > 8) {
		// Greater than 8 bits per channel not supported yet
		SDL_InvalidateMap(map);
		return SDL_SetError("Blit combination not supported");
	}
#if SDL_HAVE_BLIT_0
	else if (surface->format->BitsPerPixel < 8 && SDL_ISPIXELFORMAT_INDEXED(surface->format->format)) {
		blit = SDL_CalculateBlit0(surface);
	}
#endif
#if SDL_HAVE_BLIT_1
	else if (surface->format->BytesPerPixel == 1 && SDL_ISPIXELFORMAT_INDEXED(surface->format->format)) {
		blit = SDL_CalculateBlit1(surface);
	}
#endif
#if SDL_HAVE_BLIT_A
	else if (map->info.flags & SDL_COPY_BLEND) {
		blit = SDL_CalculateBlitA(surface);
	}
#endif
#if SDL_HAVE_BLIT_N
	else {
		blit = SDL_CalculateBlitN(surface);
	}
#endif
#if SDL_HAVE_BLIT_AUTO
	if (blit == NULL) {
		Uint32 src_format = surface->format->format;
		Uint32 dst_format = dst->format->format;

		blit = SDL_ChooseBlitFunc(src_format, dst_format, map->info.flags, SDL_GeneratedBlitFuncTable);
	}
#endif

#ifndef TEST_SLOW_BLIT
	if (blit == NULL)
#endif
	{
		Uint32 src_format = surface->format->format;
		Uint32 dst_format = dst->format->format;

		if (!SDL_ISPIXELFORMAT_INDEXED(src_format) &&
				!SDL_ISPIXELFORMAT_FOURCC(src_format) &&
				!SDL_ISPIXELFORMAT_INDEXED(dst_format) &&
				!SDL_ISPIXELFORMAT_FOURCC(dst_format)) {
			blit = SDL_Blit_Slow;
		}
	}
	map->data = blit;

	// Make sure we have a blit function
	if (blit == NULL) {
		SDL_InvalidateMap(map);
		return SDL_SetError("Blit combination not supported");
	}

	return 0;
}

/// Custom blitters

// The structure passed to the low level blit functions.
typedef struct {
	Uint8 *s_pixels;
	int s_width;
	int s_height;
	int s_skip;
	Uint8 *d_pixels;
	int d_width;
	int d_height;
	int d_skip;
	void *aux_data;
	SDL_PixelFormat *src;
	Uint8 *table;
	SDL_PixelFormat *dst;
} SDL_gfxBlitInfo;

// Alpha adjustment table for custom blitter.
//
// The table provides values for a modified, non-linear
// transfer function which maintain brightness.
const unsigned int ALPHA_ADJUST_ARRAY[256] = {
	0, /* 0 */
	15, /* 1 */
	22, /* 2 */
	27, /* 3 */
	31, /* 4 */
	35, /* 5 */
	39, /* 6 */
	42, /* 7 */
	45, /* 8 */
	47, /* 9 */
	50, /* 10 */
	52, /* 11 */
	55, /* 12 */
	57, /* 13 */
	59, /* 14 */
	61, /* 15 */
	63, /* 16 */
	65, /* 17 */
	67, /* 18 */
	69, /* 19 */
	71, /* 20 */
	73, /* 21 */
	74, /* 22 */
	76, /* 23 */
	78, /* 24 */
	79, /* 25 */
	81, /* 26 */
	82, /* 27 */
	84, /* 28 */
	85, /* 29 */
	87, /* 30 */
	88, /* 31 */
	90, /* 32 */
	91, /* 33 */
	93, /* 34 */
	94, /* 35 */
	95, /* 36 */
	97, /* 37 */
	98, /* 38 */
	99, /* 39 */
	100, /* 40 */
	102, /* 41 */
	103, /* 42 */
	104, /* 43 */
	105, /* 44 */
	107, /* 45 */
	108, /* 46 */
	109, /* 47 */
	110, /* 48 */
	111, /* 49 */
	112, /* 50 */
	114, /* 51 */
	115, /* 52 */
	116, /* 53 */
	117, /* 54 */
	118, /* 55 */
	119, /* 56 */
	120, /* 57 */
	121, /* 58 */
	122, /* 59 */
	123, /* 60 */
	124, /* 61 */
	125, /* 62 */
	126, /* 63 */
	127, /* 64 */
	128, /* 65 */
	129, /* 66 */
	130, /* 67 */
	131, /* 68 */
	132, /* 69 */
	133, /* 70 */
	134, /* 71 */
	135, /* 72 */
	136, /* 73 */
	137, /* 74 */
	138, /* 75 */
	139, /* 76 */
	140, /* 77 */
	141, /* 78 */
	141, /* 79 */
	142, /* 80 */
	143, /* 81 */
	144, /* 82 */
	145, /* 83 */
	146, /* 84 */
	147, /* 85 */
	148, /* 86 */
	148, /* 87 */
	149, /* 88 */
	150, /* 89 */
	151, /* 90 */
	152, /* 91 */
	153, /* 92 */
	153, /* 93 */
	154, /* 94 */
	155, /* 95 */
	156, /* 96 */
	157, /* 97 */
	158, /* 98 */
	158, /* 99 */
	159, /* 100 */
	160, /* 101 */
	161, /* 102 */
	162, /* 103 */
	162, /* 104 */
	163, /* 105 */
	164, /* 106 */
	165, /* 107 */
	165, /* 108 */
	166, /* 109 */
	167, /* 110 */
	168, /* 111 */
	168, /* 112 */
	169, /* 113 */
	170, /* 114 */
	171, /* 115 */
	171, /* 116 */
	172, /* 117 */
	173, /* 118 */
	174, /* 119 */
	174, /* 120 */
	175, /* 121 */
	176, /* 122 */
	177, /* 123 */
	177, /* 124 */
	178, /* 125 */
	179, /* 126 */
	179, /* 127 */
	180, /* 128 */
	181, /* 129 */
	182, /* 130 */
	182, /* 131 */
	183, /* 132 */
	184, /* 133 */
	184, /* 134 */
	185, /* 135 */
	186, /* 136 */
	186, /* 137 */
	187, /* 138 */
	188, /* 139 */
	188, /* 140 */
	189, /* 141 */
	190, /* 142 */
	190, /* 143 */
	191, /* 144 */
	192, /* 145 */
	192, /* 146 */
	193, /* 147 */
	194, /* 148 */
	194, /* 149 */
	195, /* 150 */
	196, /* 151 */
	196, /* 152 */
	197, /* 153 */
	198, /* 154 */
	198, /* 155 */
	199, /* 156 */
	200, /* 157 */
	200, /* 158 */
	201, /* 159 */
	201, /* 160 */
	202, /* 161 */
	203, /* 162 */
	203, /* 163 */
	204, /* 164 */
	205, /* 165 */
	205, /* 166 */
	206, /* 167 */
	206, /* 168 */
	207, /* 169 */
	208, /* 170 */
	208, /* 171 */
	209, /* 172 */
	210, /* 173 */
	210, /* 174 */
	211, /* 175 */
	211, /* 176 */
	212, /* 177 */
	213, /* 178 */
	213, /* 179 */
	214, /* 180 */
	214, /* 181 */
	215, /* 182 */
	216, /* 183 */
	216, /* 184 */
	217, /* 185 */
	217, /* 186 */
	218, /* 187 */
	218, /* 188 */
	219, /* 189 */
	220, /* 190 */
	220, /* 191 */
	221, /* 192 */
	221, /* 193 */
	222, /* 194 */
	222, /* 195 */
	223, /* 196 */
	224, /* 197 */
	224, /* 198 */
	225, /* 199 */
	225, /* 200 */
	226, /* 201 */
	226, /* 202 */
	227, /* 203 */
	228, /* 204 */
	228, /* 205 */
	229, /* 206 */
	229, /* 207 */
	230, /* 208 */
	230, /* 209 */
	231, /* 210 */
	231, /* 211 */
	232, /* 212 */
	233, /* 213 */
	233, /* 214 */
	234, /* 215 */
	234, /* 216 */
	235, /* 217 */
	235, /* 218 */
	236, /* 219 */
	236, /* 220 */
	237, /* 221 */
	237, /* 222 */
	238, /* 223 */
	238, /* 224 */
	239, /* 225 */
	240, /* 226 */
	240, /* 227 */
	241, /* 228 */
	241, /* 229 */
	242, /* 230 */
	242, /* 231 */
	243, /* 232 */
	243, /* 233 */
	244, /* 234 */
	244, /* 235 */
	245, /* 236 */
	245, /* 237 */
	246, /* 238 */
	246, /* 239 */
	247, /* 240 */
	247, /* 241 */
	248, /* 242 */
	248, /* 243 */
	249, /* 244 */
	249, /* 245 */
	250, /* 246 */
	250, /* 247 */
	251, /* 248 */
	251, /* 249 */
	252, /* 250 */
	252, /* 251 */
	253, /* 252 */
	253, /* 253 */
	254, /* 254 */
	255, /* 255 */
};

// Internal blitter using adjusted destination alpha during RGBA->RGBA blits.
//
// Performs the blit based on the 'info' structure and applies the transfer function
// to the destination 'a' values.
static void _SDL_gfxBlitBlitterRGBA(SDL_gfxBlitInfo *info) {
	int width = info->d_width;
	int height = info->d_height;
	Uint8 *src = info->s_pixels;
	int srcskip = info->s_skip;
	Uint8 *dst = info->d_pixels;
	int dstskip = info->d_skip;
	SDL_PixelFormat *srcfmt = info->src;
	SDL_PixelFormat *dstfmt = info->dst;
	Uint8 srcbpp = srcfmt->BytesPerPixel;
	Uint8 dstbpp = dstfmt->BytesPerPixel;

	while (height--) {
		DUFFS_LOOP4({
			Uint32 pixel;
			unsigned sR;
			unsigned sG;
			unsigned sB;
			unsigned sA;
			unsigned dR;
			unsigned dG;
			unsigned dB;
			unsigned dA;
			unsigned sAA;
			DISEMBLE_RGBA(src, srcbpp, srcfmt, pixel, sR, sG, sB, sA);
			DISEMBLE_RGBA(dst, dstbpp, dstfmt, pixel, dR, dG, dB, dA);
			sAA=ALPHA_ADJUST_ARRAY[sA & 255];
			ALPHA_BLEND_RGB(sR, sG, sB, sAA, dR, dG, dB);
			dA |= sAA;
			ASSEMBLE_RGBA(dst, dstbpp, dstfmt, dR, dG, dB, dA);
			src += srcbpp; dst += dstbpp; }, width);
		src += srcskip;
		dst += dstskip;
	}
}

// Internal blitter setup wrapper for RGBA->RGBA blits.
//
// Sets up the blitter info based on the 'src' and 'dst' surfaces and rectangles.
int _SDL_BlitRGBACall(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
	/*
	 * Set up source and destination buffer pointers, then blit
	 */
	if (srcrect->w && srcrect->h) {
		SDL_gfxBlitInfo info;

		/*
		 * Set up the blit information
		 */
		info.s_pixels = (Uint8 *)src->pixels + (Uint16)srcrect->y * src->pitch + (Uint16)srcrect->x * src->format->BytesPerPixel;
		info.s_width = srcrect->w;
		info.s_height = srcrect->h;
		info.s_skip = (int)(src->pitch - info.s_width * src->format->BytesPerPixel);
		info.d_pixels = (Uint8 *)dst->pixels + (Uint16)dstrect->y * dst->pitch + (Uint16)dstrect->x * dst->format->BytesPerPixel;
		info.d_width = dstrect->w;
		info.d_height = dstrect->h;
		info.d_skip = (int)(dst->pitch - info.d_width * dst->format->BytesPerPixel);
		info.aux_data = NULL;
		info.src = src->format;
		info.table = NULL;
		info.dst = dst->format;

		/*
		 * Run the actual software blitter
		 */
		_SDL_gfxBlitBlitterRGBA(&info);
		return 1;
	}

	return (0);
}

// Blitter for RGBA->RGBA blits with alpha adjustment.
//
// Verifies the input 'src' and 'dst' surfaces and rectangles and performs blit.
// The destination clip rectangle is honored.
int SDL_gfxBlitRGBA(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
	SDL_Rect sr, dr;
	int srcx, srcy, w, h;

	/*
	 * Make sure the surfaces aren't locked
	 */
	if (!src || !dst) {
		SDL_SetError("SDL_UpperBlit: passed a NULL surface");
		return (-1);
	}

	/*
	 * If the destination rectangle is NULL, use the entire dest surface
	 */
	if (dstrect == NULL) {
		dr.x = dr.y = 0;
		dr.w = dst->w;
		dr.h = dst->h;
	} else {
		dr = *dstrect;
	}

	/*
	 * Clip the source rectangle to the source surface
	 */
	if (srcrect) {
		int maxw, maxh;

		srcx = srcrect->x;
		w = srcrect->w;
		if (srcx < 0) {
			w += srcx;
			dr.x -= srcx;
			srcx = 0;
		}
		maxw = src->w - srcx;
		if (maxw < w)
			w = maxw;

		srcy = srcrect->y;
		h = srcrect->h;
		if (srcy < 0) {
			h += srcy;
			dr.y -= srcy;
			srcy = 0;
		}
		maxh = src->h - srcy;
		if (maxh < h)
			h = maxh;

	} else {
		srcx = srcy = 0;
		w = src->w;
		h = src->h;
	}

	/*
	 * Clip the destination rectangle against the clip rectangle
	 */
	{
		SDL_Rect *clip = &dst->clip_rect;
		int dx, dy;

		dx = clip->x - dr.x;
		if (dx > 0) {
			w -= dx;
			dr.x += dx;
			srcx += dx;
		}
		dx = dr.x + w - clip->x - clip->w;
		if (dx > 0)
			w -= dx;

		dy = clip->y - dr.y;
		if (dy > 0) {
			h -= dy;
			dr.y += dy;
			srcy += dy;
		}
		dy = dr.y + h - clip->y - clip->h;
		if (dy > 0)
			h -= dy;
	}

	if (w > 0 && h > 0) {
		sr.x = srcx;
		sr.y = srcy;
		sr.w = dr.w = w;
		sr.h = dr.h = h;
		return (_SDL_BlitRGBACall(src, &sr, dst, &dr));
	}

	return 0;
}

// Sets the alpha channel in a 32 bit surface.
//
// Helper function that sets the alpha channel in a 32 bit surface
// to a constant value.
// Only 32 bit surfaces can be used with this function.
int SDL_SetAlpha(SDL_Surface *src, Uint8 a) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	const int alpha_offset = 0;
#else
	const int alpha_offset = 3;
#endif
	int row_skip;
	Uint8 *pixels;

	/* Check if we have a 32bit surface */
	if ((src == NULL) ||
			(src->format == NULL) ||
			(src->format->BytesPerPixel != 4)) {
		SDL_SetError("SDL_SetAlpha: Invalid input surface.");
		return -1;
	}

	/* Process */
	pixels = (Uint8 *)src->pixels;
	row_skip = (src->pitch - (4 * src->w));
	pixels += alpha_offset;
	for (int i = 0; i < src->h; i++) {
		for (int j = 0; j < src->w; j++) {
			*pixels = a;
			pixels += 4;
		}
		pixels += row_skip;
	}

	return 1;
}

// Multiply the alpha channel in a 32bit surface.
//
// Helper function that multiplies the alpha channel in a 32 bit surface
// with a constant value. The final alpha is always scaled to the range
// 0-255 (i.e. the factor is a/256).
//
// Only 32 bit surfaces can be used with this function.
int SDL_gfxMultiplyAlpha(SDL_Surface *src, Uint8 a) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	const int alpha_offset = 0;
#else
	const int alpha_offset = 3;
#endif
	int row_skip;
	Uint8 *pixels;

	/* Check if we have a 32bit surface */
	if ((src == NULL) ||
			(src->format == NULL) ||
			(src->format->BytesPerPixel != 4)) {
		SDL_SetError("SDL_gfxMultiplyAlpha: Invalid input surface.");
		return -1;
	}

	/* Check if multiplication is needed */
	if (a == 255) {
		return 0;
	}

	/* Process */
	pixels = (Uint8 *)src->pixels;
	row_skip = (src->pitch - (4 * src->w));
	pixels += alpha_offset;
	for (int i = 0; i < src->h; i++) {
		for (int j = 0; j < src->w; j++) {
			*pixels = (Uint8)(((int)(*pixels) * a) >> 8);
			pixels += 4;
		}
		pixels += row_skip;
	}

	return 1;
}
