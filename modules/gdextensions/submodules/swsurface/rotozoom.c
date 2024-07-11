/**************************************************************************/
/*  rotozoom.c                                                            */
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
 * rotozoom.c: rotozoomer, zoomer and shrinker for 32bit or 8bit surfaces
 *
 * Copyright (C) 2012-2014  Andreas Schiffler
 * Andreas Schiffler -- aschiffler at ferzkopp dot net
 */

#include <stdlib.h>
#include <string.h>

#include "rotozoom.h"

/* ---- Internally used structures */

// A 32 bit RGBA pixel.
typedef struct tColorRGBA {
	Uint8 r;
	Uint8 g;
	Uint8 b;
	Uint8 a;
} tColorRGBA;

// A 8bit Y/palette pixel.
typedef struct tColorY {
	Uint8 y;
} tColorY;

// Check for missing declarations

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

// Number of guard rows added to destination surfaces.
//
// This is a simple but effective workaround for observed issues.
// These rows allocate extra memory and are then hidden from the surface.
// Rows are added to the end of destination surfaces when they are allocated.
// This catches any potential overflows which seem to happen with
// just the right src image dimensions and scale/rotation and can lead
// to a situation where the program can segfault.
#define GUARD_ROWS (2)

// Lower limit of absolute zoom factor or rotation degrees.
#define VALUE_LIMIT 0.001

// Returns colorkey info for a surface
static SDL_INLINE Uint32 _colorkey(SDL_Surface *src) {
	Uint32 key = 0;
	SDL_GetColorKey(src, &key);
	return key;
}

// Internal 32 bit integer-factor averaging Shrinker.
//
// Shrinks 32 bit RGBA/ABGR 'src' surface to 'dst' surface.
// Averages color and alpha values values of src pixels to calculate dst pixels.
// Assumes src and dst surfaces are of 32 bit depth.
// Assumes dst surface was allocated with the correct dimensions.
static int _shrinkSurfaceRGBA(SDL_Surface *src, SDL_Surface *dst, int factorx, int factory) {
	int dgap, ra, ga, ba, aa;
	int n_average;
	tColorRGBA *sp, *osp, *oosp;
	tColorRGBA *dp;

	/*
	 * Averaging integer shrink
	 */

	/* Precalculate division factor */
	n_average = factorx * factory;

	/*
	 * Scan destination
	 */
	sp = (tColorRGBA *)src->pixels;

	dp = (tColorRGBA *)dst->pixels;
	dgap = dst->pitch - dst->w * 4;

	for (int y = 0; y < dst->h; y++) {
		osp = sp;
		for (int x = 0; x < dst->w; x++) {
			/* Trace out source box and accumulate */
			oosp = sp;
			ra = ga = ba = aa = 0;
			for (int dy = 0; dy < factory; dy++) {
				for (int dx = 0; dx < factorx; dx++) {
					ra += sp->r;
					ga += sp->g;
					ba += sp->b;
					aa += sp->a;

					sp++;
				}
				/* src dx loop */
				sp = (tColorRGBA *)((Uint8 *)sp + (src->pitch - 4 * factorx)); // next y
			}
			/* src dy loop */

			/* next box-x */
			sp = (tColorRGBA *)((Uint8 *)oosp + 4 * factorx);

			/* Store result in destination */
			dp->r = ra / n_average;
			dp->g = ga / n_average;
			dp->b = ba / n_average;
			dp->a = aa / n_average;

			/*
			 * Advance destination pointer
			 */
			dp++;
		}
		/* dst x loop */

		/* next box-y */
		sp = (tColorRGBA *)((Uint8 *)osp + src->pitch * factory);

		/*
		 * Advance destination pointers
		 */
		dp = (tColorRGBA *)((Uint8 *)dp + dgap);
	}
	/* dst y loop */

	return (0);
}

// Internal 8 bit integer-factor averaging shrinker.
//
// Shrinks 8bit Y 'src' surface to 'dst' surface.
// Averages color (brightness) values values of src pixels to calculate dst pixels.
// Assumes src and dst surfaces are of 8 bit depth.
// Assumes dst surface was allocated with the correct dimensions.
static int _shrinkSurfaceY(SDL_Surface *src, SDL_Surface *dst, int factorx, int factory) {
	int dx, dy, dgap, a;
	int n_average;
	Uint8 *sp, *osp, *oosp;
	Uint8 *dp;

	/*
	 * Averaging integer shrink
	 */

	/* Precalculate division factor */
	n_average = factorx * factory;

	/*
	 * Scan destination
	 */
	sp = (Uint8 *)src->pixels;

	dp = (Uint8 *)dst->pixels;
	dgap = dst->pitch - dst->w;

	for (int y = 0; y < dst->h; y++) {
		osp = sp;
		for (int x = 0; x < dst->w; x++) {
			/* Trace out source box and accumulate */
			oosp = sp;
			a = 0;
			for (dy = 0; dy < factory; dy++) {
				for (dx = 0; dx < factorx; dx++) {
					a += (*sp);
					/* next x */
					sp++;
				}
				/* end src dx loop */
				/* next y */
				sp = (Uint8 *)((Uint8 *)sp + (src->pitch - factorx));
			}
			/* end src dy loop */

			/* next box-x */
			sp = (Uint8 *)((Uint8 *)oosp + factorx);

			/* Store result in destination */
			*dp = a / n_average;

			/*
			 * Advance destination pointer
			 */
			dp++;
		}
		/* end dst x loop */

		/* next box-y */
		sp = (Uint8 *)((Uint8 *)osp + src->pitch * factory);

		/*
		 * Advance destination pointers
		 */
		dp = (Uint8 *)((Uint8 *)dp + dgap);
	}
	/* end dst y loop */

	return (0);
}

// Internal 32 bit Zoomer with optional anti-aliasing by bilinear interpolation.
//
// Zooms 32 bit RGBA/ABGR 'src' surface to 'dst' surface.
// Assumes src and dst surfaces are of 32 bit depth.
// Assumes dst surface was allocated with the correct dimensions.
static int _zoomSurfaceRGBA(SDL_Surface *src, SDL_Surface *dst, int flipx, int flipy, int smooth) {
	int sx, sy, ssx, ssy, *sax, *say, *csax, *csay, *salast, csx, csy, ex, ey, cx, cy, sstep, sstepx, sstepy;
	tColorRGBA *c00, *c01, *c10, *c11;
	tColorRGBA *sp, *csp, *dp;
	int spixelgap, spixelw, spixelh, dgap, t1, t2;

	/*
	 * Allocate memory for row/column increments
	 */
	if ((sax = (int *)malloc((dst->w + 1) * sizeof(Uint32))) == NULL) {
		return (-1);
	}
	if ((say = (int *)malloc((dst->h + 1) * sizeof(Uint32))) == NULL) {
		free(sax);
		return (-1);
	}

	/*
	 * Precalculate row increments
	 */
	spixelw = (src->w - 1);
	spixelh = (src->h - 1);
	if (smooth) {
		sx = (int)(65536.0 * (float)spixelw / (float)(dst->w - 1));
		sy = (int)(65536.0 * (float)spixelh / (float)(dst->h - 1));
	} else {
		sx = (int)(65536.0 * (float)(src->w) / (float)(dst->w));
		sy = (int)(65536.0 * (float)(src->h) / (float)(dst->h));
	}

	/* Maximum scaled source size */
	ssx = (src->w << 16) - 1;
	ssy = (src->h << 16) - 1;

	/* Precalculate horizontal row increments */
	csx = 0;
	csax = sax;
	for (int x = 0; x <= dst->w; x++) {
		*csax = csx;
		csax++;
		csx += sx;

		/* Guard from overflows */
		if (csx > ssx) {
			csx = ssx;
		}
	}

	/* Precalculate vertical row increments */
	csy = 0;
	csay = say;
	for (int y = 0; y <= dst->h; y++) {
		*csay = csy;
		csay++;
		csy += sy;

		/* Guard from overflows */
		if (csy > ssy) {
			csy = ssy;
		}
	}

	sp = (tColorRGBA *)src->pixels;
	dp = (tColorRGBA *)dst->pixels;
	dgap = dst->pitch - dst->w * 4;
	spixelgap = src->pitch / 4;

	if (flipx)
		sp += spixelw;
	if (flipy)
		sp += (spixelgap * spixelh);

	/*
	 * Switch between interpolating and non-interpolating code
	 */
	if (smooth) {
		/*
		 * Interpolating Zoom
		 */
		csay = say;
		for (int y = 0; y < dst->h; y++) {
			csp = sp;
			csax = sax;
			for (int x = 0; x < dst->w; x++) {
				/*
				 * Setup color source pointers
				 */
				ex = (*csax & 0xffff);
				ey = (*csay & 0xffff);
				cx = (*csax >> 16);
				cy = (*csay >> 16);
				sstepx = cx < spixelw;
				sstepy = cy < spixelh;
				c00 = sp;
				c01 = sp;
				c10 = sp;
				if (sstepy) {
					if (flipy) {
						c10 -= spixelgap;
					} else {
						c10 += spixelgap;
					}
				}
				c11 = c10;
				if (sstepx) {
					if (flipx) {
						c01--;
						c11--;
					} else {
						c01++;
						c11++;
					}
				}

				/*
				 * Draw and interpolate colors
				 */
				t1 = ((((c01->r - c00->r) * ex) >> 16) + c00->r) & 0xff;
				t2 = ((((c11->r - c10->r) * ex) >> 16) + c10->r) & 0xff;
				dp->r = (((t2 - t1) * ey) >> 16) + t1;
				t1 = ((((c01->g - c00->g) * ex) >> 16) + c00->g) & 0xff;
				t2 = ((((c11->g - c10->g) * ex) >> 16) + c10->g) & 0xff;
				dp->g = (((t2 - t1) * ey) >> 16) + t1;
				t1 = ((((c01->b - c00->b) * ex) >> 16) + c00->b) & 0xff;
				t2 = ((((c11->b - c10->b) * ex) >> 16) + c10->b) & 0xff;
				dp->b = (((t2 - t1) * ey) >> 16) + t1;
				t1 = ((((c01->a - c00->a) * ex) >> 16) + c00->a) & 0xff;
				t2 = ((((c11->a - c10->a) * ex) >> 16) + c10->a) & 0xff;
				dp->a = (((t2 - t1) * ey) >> 16) + t1;
				/*
				 * Advance source pointer x
				 */
				salast = csax;
				csax++;
				sstep = (*csax >> 16) - (*salast >> 16);
				if (flipx) {
					sp -= sstep;
				} else {
					sp += sstep;
				}

				/*
				 * Advance destination pointer x
				 */
				dp++;
			}
			/*
			 * Advance source pointer y
			 */
			salast = csay;
			csay++;
			sstep = (*csay >> 16) - (*salast >> 16);
			sstep *= spixelgap;
			if (flipy) {
				sp = csp - sstep;
			} else {
				sp = csp + sstep;
			}

			/*
			 * Advance destination pointer y
			 */
			dp = (tColorRGBA *)((Uint8 *)dp + dgap);
		}
	} else {
		/*
		 * Non-Interpolating Zoom
		 */
		csay = say;
		for (int y = 0; y < dst->h; y++) {
			csp = sp;
			csax = sax;
			for (int x = 0; x < dst->w; x++) {
				/*
				 * Draw
				 */
				*dp = *sp;

				/*
				 * Advance source pointer x
				 */
				salast = csax;
				csax++;
				sstep = (*csax >> 16) - (*salast >> 16);
				if (flipx)
					sstep = -sstep;
				sp += sstep;

				/*
				 * Advance destination pointer x
				 */
				dp++;
			}
			/*
			 * Advance source pointer y
			 */
			salast = csay;
			csay++;
			sstep = (*csay >> 16) - (*salast >> 16);
			sstep *= spixelgap;
			if (flipy)
				sstep = -sstep;
			sp = csp + sstep;

			/*
			 * Advance destination pointer y
			 */
			dp = (tColorRGBA *)((Uint8 *)dp + dgap);
		}
	}

	/*
	 * Remove temp arrays
	 */
	free(sax);
	free(say);

	return (0);
}

// Internal 8 bit Zoomer without smoothing.
//
// Zooms 8bit palette/Y 'src' surface to 'dst' surface.
// Assumes src and dst surfaces are of 8 bit depth.
// Assumes dst surface was allocated with the correct dimensions.
static int _zoomSurfaceY(SDL_Surface *src, SDL_Surface *dst, int flipx, int flipy) {
	Uint32 *sax, *say, *csax, *csay;
	int csx, csy;
	Uint8 *sp, *dp, *csp;
	int dgap;

	/*
	 * Allocate memory for row increments
	 */
	if ((sax = (Uint32 *)malloc((dst->w + 1) * sizeof(Uint32))) == NULL) {
		return (-1);
	}
	if ((say = (Uint32 *)malloc((dst->h + 1) * sizeof(Uint32))) == NULL) {
		free(sax);
		return (-1);
	}

	/*
	 * Pointer setup
	 */
	sp = csp = (Uint8 *)src->pixels;
	dp = (Uint8 *)dst->pixels;
	dgap = dst->pitch - dst->w;

	if (flipx)
		csp += (src->w - 1);
	if (flipy)
		csp = ((Uint8 *)csp + src->pitch * (src->h - 1));

	/*
	 * Precalculate row increments
	 */
	csx = 0;
	csax = sax;
	for (int x = 0; x < dst->w; x++) {
		csx += src->w;
		*csax = 0;
		while (csx >= dst->w) {
			csx -= dst->w;
			(*csax)++;
		}
		(*csax) = (*csax) * (flipx ? -1 : 1);
		csax++;
	}
	csy = 0;
	csay = say;
	for (int y = 0; y < dst->h; y++) {
		csy += src->h;
		*csay = 0;
		while (csy >= dst->h) {
			csy -= dst->h;
			(*csay)++;
		}
		(*csay) = (*csay) * (flipy ? -1 : 1);
		csay++;
	}

	/*
	 * Draw
	 */
	csay = say;
	for (int y = 0; y < dst->h; y++) {
		csax = sax;
		sp = csp;
		for (int x = 0; x < dst->w; x++) {
			/*
			 * Draw
			 */
			*dp = *sp;
			/*
			 * Advance source pointers
			 */
			sp += (*csax);
			csax++;
			/*
			 * Advance destination pointer
			 */
			dp++;
		}
		/*
		 * Advance source pointer (for row)
		 */
		csp += ((*csay) * src->pitch);
		csay++;

		/*
		 * Advance destination pointers
		 */
		dp += dgap;
	}

	/*
	 * Remove temp arrays
	 */
	free(sax);
	free(say);

	return (0);
}

// Internal 32 bit rotozoomer with optional anti-aliasing.
//
// Rotates and zooms 32 bit RGBA/ABGR 'src' surface to 'dst' surface based on the control
// parameters by scanning the destination surface and applying optionally anti-aliasing
// by bilinear interpolation.
// Assumes src and dst surfaces are of 32 bit depth.
// Assumes dst surface was allocated with the correct dimensions.
static void _transformSurfaceRGBA(SDL_Surface *src, SDL_Surface *dst, int cx, int cy, int isin, int icos, int flipx, int flipy, int smooth) {
	int t1, t2, dx, dy, xd, yd, sdx, sdy, ax, ay, ex, ey, sw, sh;
	tColorRGBA c00, c01, c10, c11, cswap;
	tColorRGBA *pc, *sp;
	int gap;

	/*
	 * Variable setup
	 */
	xd = ((src->w - dst->w) << 15);
	yd = ((src->h - dst->h) << 15);
	ax = (cx << 16) - (icos * cx);
	ay = (cy << 16) - (isin * cx);
	sw = src->w - 1;
	sh = src->h - 1;
	pc = (tColorRGBA *)dst->pixels;
	gap = dst->pitch - dst->w * 4;

	/*
	 * Switch between interpolating and non-interpolating code
	 */
	if (smooth) {
		for (int y = 0; y < dst->h; y++) {
			dy = cy - y;
			sdx = (ax + (isin * dy)) + xd;
			sdy = (ay - (icos * dy)) + yd;
			for (int x = 0; x < dst->w; x++) {
				dx = (sdx >> 16);
				dy = (sdy >> 16);
				if (flipx)
					dx = sw - dx;
				if (flipy)
					dy = sh - dy;
				if ((dx > -1) && (dy > -1) && (dx < (src->w - 1)) && (dy < (src->h - 1))) {
					sp = (tColorRGBA *)src->pixels;
					;
					sp += ((src->pitch / 4) * dy);
					sp += dx;
					c00 = *sp;
					sp += 1;
					c01 = *sp;
					sp += (src->pitch / 4);
					c11 = *sp;
					sp -= 1;
					c10 = *sp;
					if (flipx) {
						cswap = c00;
						c00 = c01;
						c01 = cswap;
						cswap = c10;
						c10 = c11;
						c11 = cswap;
					}
					if (flipy) {
						cswap = c00;
						c00 = c10;
						c10 = cswap;
						cswap = c01;
						c01 = c11;
						c11 = cswap;
					}
					/*
					 * Interpolate colors
					 */
					ex = (sdx & 0xffff);
					ey = (sdy & 0xffff);
					t1 = ((((c01.r - c00.r) * ex) >> 16) + c00.r) & 0xff;
					t2 = ((((c11.r - c10.r) * ex) >> 16) + c10.r) & 0xff;
					pc->r = (((t2 - t1) * ey) >> 16) + t1;
					t1 = ((((c01.g - c00.g) * ex) >> 16) + c00.g) & 0xff;
					t2 = ((((c11.g - c10.g) * ex) >> 16) + c10.g) & 0xff;
					pc->g = (((t2 - t1) * ey) >> 16) + t1;
					t1 = ((((c01.b - c00.b) * ex) >> 16) + c00.b) & 0xff;
					t2 = ((((c11.b - c10.b) * ex) >> 16) + c10.b) & 0xff;
					pc->b = (((t2 - t1) * ey) >> 16) + t1;
					t1 = ((((c01.a - c00.a) * ex) >> 16) + c00.a) & 0xff;
					t2 = ((((c11.a - c10.a) * ex) >> 16) + c10.a) & 0xff;
					pc->a = (((t2 - t1) * ey) >> 16) + t1;
				}
				sdx += icos;
				sdy += isin;
				pc++;
			}
			pc = (tColorRGBA *)((Uint8 *)pc + gap);
		}
	} else {
		for (int y = 0; y < dst->h; y++) {
			dy = cy - y;
			sdx = (ax + (isin * dy)) + xd;
			sdy = (ay - (icos * dy)) + yd;
			for (int x = 0; x < dst->w; x++) {
				dx = (short)(sdx >> 16);
				dy = (short)(sdy >> 16);
				if (flipx)
					dx = (src->w - 1) - dx;
				if (flipy)
					dy = (src->h - 1) - dy;
				if ((dx >= 0) && (dy >= 0) && (dx < src->w) && (dy < src->h)) {
					sp = (tColorRGBA *)((Uint8 *)src->pixels + src->pitch * dy);
					sp += dx;
					*pc = *sp;
				}
				sdx += icos;
				sdy += isin;
				pc++;
			}
			pc = (tColorRGBA *)((Uint8 *)pc + gap);
		}
	}
}

// Rotates and zooms 8 bit palette/Y 'src' surface to 'dst' surface without smoothing.
//
// Rotates and zooms 8 bit RGBA/ABGR 'src' surface to 'dst' surface based on the control
// parameters by scanning the destination surface.
// Assumes src and dst surfaces are of 8 bit depth.
// Assumes dst surface was allocated with the correct dimensions.
static void transformSurfaceY(SDL_Surface *src, SDL_Surface *dst, int cx, int cy, int isin, int icos, int flipx, int flipy) {
	int dx, dy, xd, yd, sdx, sdy, ax, ay;
	tColorY *pc, *sp;
	int gap;

	/*
	 * Variable setup
	 */
	xd = ((src->w - dst->w) << 15);
	yd = ((src->h - dst->h) << 15);
	ax = (cx << 16) - (icos * cx);
	ay = (cy << 16) - (isin * cx);
	pc = (tColorY *)dst->pixels;
	gap = dst->pitch - dst->w;
	/*
	 * Clear surface to colorkey
	 */
	memset(pc, (int)(_colorkey(src) & 0xff), dst->pitch * dst->h);
	/*
	 * Iterate through destination surface
	 */
	for (int y = 0; y < dst->h; y++) {
		dy = cy - y;
		sdx = (ax + (isin * dy)) + xd;
		sdy = (ay - (icos * dy)) + yd;
		for (int x = 0; x < dst->w; x++) {
			dx = (short)(sdx >> 16);
			dy = (short)(sdy >> 16);
			if (flipx)
				dx = (src->w - 1) - dx;
			if (flipy)
				dy = (src->h - 1) - dy;
			if ((dx >= 0) && (dy >= 0) && (dx < src->w) && (dy < src->h)) {
				sp = (tColorY *)(src->pixels);
				sp += (src->pitch * dy + dx);
				*pc = *sp;
			}
			sdx += icos;
			sdy += isin;
			pc++;
		}
		pc += gap;
	}
}

// Rotates a 8/16/24/32 bit surface in increments of 90 degrees.
//
// Specialized 90 degree rotator which rotates a 'src' surface in 90 degree
// increments clockwise returning a new surface. Faster than rotozoomer since
// no scanning or interpolation takes place. Input surface must be 8/16/24/32 bit.
// (code contributed by J. Schiller, improved by C. Allport and A. Schiffler)
SDL_Surface *rotateSurface90Degrees(SDL_Surface *src, int numClockwiseTurns) {
	int newWidth, newHeight;
	int bpp, bpr;
	SDL_Surface *dst;
	Uint8 *srcBuf;
	Uint8 *dstBuf;
	int normalizedClockwiseTurns;

	/* Has to be a valid surface pointer and be a Nbit surface where n is divisible by 8 */
	if (!src ||
			!src->format) {
		SDL_SetError("NULL source surface or source surface format");
		return NULL;
	}

	if ((src->format->BitsPerPixel % 8) != 0) {
		SDL_SetError("Invalid source surface bit depth");
		return NULL;
	}

	/* normalize numClockwiseTurns */
	normalizedClockwiseTurns = (numClockwiseTurns % 4);
	if (normalizedClockwiseTurns < 0) {
		normalizedClockwiseTurns += 4;
	}

	/* If turns are even, our new width/height will be the same as the source surface */
	if (normalizedClockwiseTurns % 2) {
		newWidth = src->h;
		newHeight = src->w;
	} else {
		newWidth = src->w;
		newHeight = src->h;
	}

	dst = SDL_CreateRGBSurface(newWidth, newHeight, src->format->BitsPerPixel,
			src->format->Rmask, src->format->Gmask, src->format->Bmask, src->format->Amask);
	if (!dst) {
		SDL_SetError("Could not create destination surface");
		return NULL;
	}

	/* Calculate byte-per-pixel */
	bpp = src->format->BitsPerPixel / 8;

	switch (normalizedClockwiseTurns) {
		case 0: /* Make a copy of the surface */
		{
			/* Unfortunately SDL_BlitSurface cannot be used to make a copy of the surface
			since it does not preserve alpha. */

			if (src->pitch == dst->pitch) {
				/* If the pitch is the same for both surfaces, the memory can be copied all at once. */
				memcpy(dst->pixels, src->pixels, (src->h * src->pitch));
			} else {
				/* If the pitch differs, copy each row separately */
				srcBuf = (Uint8 *)(src->pixels);
				dstBuf = (Uint8 *)(dst->pixels);
				bpr = src->w * bpp;
				for (int row = 0; row < src->h; row++) {
					memcpy(dstBuf, srcBuf, bpr);
					srcBuf += src->pitch;
					dstBuf += dst->pitch;
				}
			}
		} break;

		case 1: /* rotated 90 degrees clockwise */
		{
			for (int row = 0; row < src->h; ++row) {
				srcBuf = (Uint8 *)(src->pixels) + (row * src->pitch);
				dstBuf = (Uint8 *)(dst->pixels) + (dst->w - row - 1) * bpp;
				for (int col = 0; col < src->w; ++col) {
					memcpy(dstBuf, srcBuf, bpp);
					srcBuf += bpp;
					dstBuf += dst->pitch;
				}
			}
		} break;

		case 2: /* rotated 180 degrees clockwise */
		{
			for (int row = 0; row < src->h; ++row) {
				srcBuf = (Uint8 *)(src->pixels) + (row * src->pitch);
				dstBuf = (Uint8 *)(dst->pixels) + ((dst->h - row - 1) * dst->pitch) + (dst->w - 1) * bpp;
				for (int col = 0; col < src->w; ++col) {
					memcpy(dstBuf, srcBuf, bpp);
					srcBuf += bpp;
					dstBuf -= bpp;
				}
			}
		} break;

		case 3: /* rotated 270 degrees clockwise */
		{
			for (int row = 0; row < src->h; ++row) {
				srcBuf = (Uint8 *)(src->pixels) + (row * src->pitch);
				dstBuf = (Uint8 *)(dst->pixels) + (row * bpp) + ((dst->h - 1) * dst->pitch);
				for (int col = 0; col < src->w; ++col) {
					memcpy(dstBuf, srcBuf, bpp);
					srcBuf += bpp;
					dstBuf -= dst->pitch;
				}
			}
		} break;
	}
	/* end switch */
	return dst;
}

// Internal target surface sizing function for rotozooms with trig result return.
void _rotozoomSurfaceSizeTrig(int width, int height, double angle, double zoomx, double zoomy, int *dstwidth, int *dstheight, double *canglezoom, double *sanglezoom) {
	double x, y, cx, cy, sx, sy;
	double radangle;
	int dstwidthhalf, dstheighthalf;

	/*
	 * Determine destination width and height by rotating a centered source box
	 */
	radangle = angle * (M_PI / 180.0);
	*sanglezoom = SDL_sin(radangle);
	*canglezoom = SDL_cos(radangle);
	*sanglezoom *= zoomx;
	*canglezoom *= zoomy;
	x = (double)(width / 2);
	y = (double)(height / 2);
	cx = *canglezoom * x;
	cy = *canglezoom * y;
	sx = *sanglezoom * x;
	sy = *sanglezoom * y;

	dstwidthhalf = MAX((int)ceil(MAX(MAX(MAX(fabs(cx + sy), fabs(cx - sy)), fabs(-cx + sy)), fabs(-cx - sy))), 1);
	dstheighthalf = MAX((int)ceil(MAX(MAX(MAX(fabs(sx + cy), fabs(sx - cy)), fabs(-sx + cy)), fabs(-sx - cy))), 1);
	*dstwidth = 2 * dstwidthhalf;
	*dstheight = 2 * dstheighthalf;
}

// Returns the size of the resulting target surface for a rotozoomSurfaceXY() call.
void rotozoomSurfaceSizeXY(int width, int height, double angle, double zoomx, double zoomy, int *dstwidth, int *dstheight) {
	double dummy_sanglezoom, dummy_canglezoom;

	_rotozoomSurfaceSizeTrig(width, height, angle, zoomx, zoomy, dstwidth, dstheight, &dummy_sanglezoom, &dummy_canglezoom);
}

// Returns the size of the resulting target surface for a rotozoomSurface() call.
void rotozoomSurfaceSize(int width, int height, double angle, double zoom, int *dstwidth, int *dstheight) {
	double dummy_sanglezoom, dummy_canglezoom;

	_rotozoomSurfaceSizeTrig(width, height, angle, zoom, zoom, dstwidth, dstheight, &dummy_sanglezoom, &dummy_canglezoom);
}

// Rotates and zooms a surface and optional anti-aliasing.
//
// Rotates and zoomes a 32bit or 8bit 'src' surface to newly created 'dst' surface.
// 'angle' is the rotation in degrees and 'zoom' a scaling factor. If 'smooth' is set
// then the destination 32bit surface is anti-aliased. If the surface is not 8bit
// or 32bit RGBA/ABGR it will be converted into a 32bit RGBA format on the fly.
SDL_Surface *rotozoomSurface(SDL_Surface *src, double angle, double zoom, int smooth) {
	return rotozoomSurfaceXY(src, angle, zoom, zoom, smooth);
}

// Rotates and zooms a surface with different horizontal and vertival scaling factors and optional anti-aliasing.
//
// Rotates and zooms a 32bit or 8bit 'src' surface to newly created 'dst' surface.
// 'angle' is the rotation in degrees, 'zoomx and 'zoomy' scaling factors. If 'smooth' is set
// then the destination 32bit surface is anti-aliased. If the surface is not 8bit
// or 32bit RGBA/ABGR it will be converted into a 32bit RGBA format on the fly.
SDL_Surface *rotozoomSurfaceXY(SDL_Surface *src, double angle, double zoomx, double zoomy, int smooth) {
	SDL_Surface *rz_src;
	SDL_Surface *rz_dst;
	double zoominv;
	double sanglezoom, canglezoom, sanglezoominv, canglezoominv;
	int dstwidthhalf, dstwidth, dstheighthalf, dstheight;
	int is32bit;
	int src_converted;
	int flipx, flipy;

	/*
	 * Sanity check
	 */
	if (src == NULL) {
		return (NULL);
	}

	/*
	 * Determine if source surface is 32bit or 8bit
	 */
	is32bit = (src->format->BitsPerPixel == 32);
	if ((is32bit) || (src->format->BitsPerPixel == 8)) {
		/*
		 * Use source surface 'as is'
		 */
		rz_src = src;
		src_converted = 0;
	} else {
		/*
		 * New source surface is 32bit with a defined RGBA ordering
		 */
		rz_src =
				SDL_CreateRGBSurface(src->w, src->h, 32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
						0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
#else
						0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
#endif
				);

		SDL_BlitSurface(src, NULL, rz_src, NULL);

		src_converted = 1;
		is32bit = 1;
	}

	/*
	 * Sanity check zoom factor
	 */
	flipx = (zoomx < 0.0);
	if (flipx)
		zoomx = -zoomx;
	flipy = (zoomy < 0.0);
	if (flipy)
		zoomy = -zoomy;
	if (zoomx < VALUE_LIMIT)
		zoomx = VALUE_LIMIT;
	if (zoomy < VALUE_LIMIT)
		zoomy = VALUE_LIMIT;
	zoominv = 65536.0 / (zoomx * zoomx);

	/*
	 * Check if we have a rotozoom or just a zoom
	 */
	if (fabs(angle) > VALUE_LIMIT) {
		/*
		 * Angle!=0: full rotozoom
		 */
		/*
		 * -----------------------
		 */

		/* Determine target size */
		_rotozoomSurfaceSizeTrig(rz_src->w, rz_src->h, angle, zoomx, zoomy, &dstwidth, &dstheight, &canglezoom, &sanglezoom);

		/*
		 * Calculate target factors from sin/cos and zoom
		 */
		sanglezoominv = sanglezoom;
		canglezoominv = canglezoom;
		sanglezoominv *= zoominv;
		canglezoominv *= zoominv;

		/* Calculate half size */
		dstwidthhalf = dstwidth / 2;
		dstheighthalf = dstheight / 2;

		/*
		 * Alloc space to completely contain the rotated surface
		 */
		rz_dst = NULL;
		if (is32bit) {
			/*
			 * Target surface is 32bit with source RGBA/ABGR ordering
			 */
			rz_dst =
					SDL_CreateRGBSurface(dstwidth, dstheight + GUARD_ROWS, 32,
							rz_src->format->Rmask, rz_src->format->Gmask,
							rz_src->format->Bmask, rz_src->format->Amask);
		} else {
			/*
			 * Target surface is 8bit
			 */
			rz_dst = SDL_CreateRGBSurface(dstwidth, dstheight + GUARD_ROWS, 8, 0, 0, 0, 0);
		}

		/* Check target */
		if (rz_dst == NULL)
			return NULL;

		/* Adjust for guard rows */
		rz_dst->h = dstheight;

		/*
		 * Check which kind of surface we have
		 */
		if (is32bit) {
			/*
			 * Call the 32bit transformation routine to do the rotation (using alpha)
			 */
			_transformSurfaceRGBA(rz_src, rz_dst, dstwidthhalf, dstheighthalf,
					(int)(sanglezoominv), (int)(canglezoominv),
					flipx, flipy,
					smooth);
		} else {
			/*
			 * Copy palette and colorkey info
			 */
			for (int i = 0; i < rz_src->format->palette->ncolors; i++) {
				rz_dst->format->palette->colors[i] = rz_src->format->palette->colors[i];
			}
			rz_dst->format->palette->ncolors = rz_src->format->palette->ncolors;
			/*
			 * Call the 8bit transformation routine to do the rotation
			 */
			transformSurfaceY(rz_src, rz_dst, dstwidthhalf, dstheighthalf,
					(int)(sanglezoominv), (int)(canglezoominv),
					flipx, flipy);
		}
	} else {
		/*
		 * Angle=0: Just a zoom
		 */
		/*
		 * --------------------
		 */

		/*
		 * Calculate target size
		 */
		zoomSurfaceSize(rz_src->w, rz_src->h, zoomx, zoomy, &dstwidth, &dstheight);

		/*
		 * Alloc space to completely contain the zoomed surface
		 */
		rz_dst = NULL;
		if (is32bit) {
			/*
			 * Target surface is 32bit with source RGBA/ABGR ordering
			 */
			rz_dst = SDL_CreateRGBSurface(dstwidth, dstheight + GUARD_ROWS, 32,
					rz_src->format->Rmask, rz_src->format->Gmask,
					rz_src->format->Bmask, rz_src->format->Amask);
		} else {
			/*
			 * Target surface is 8bit
			 */
			rz_dst = SDL_CreateRGBSurface(dstwidth, dstheight + GUARD_ROWS, 8, 0, 0, 0, 0);
		}

		/* Check target */
		if (rz_dst == NULL)
			return NULL;

		/* Adjust for guard rows */
		rz_dst->h = dstheight;

		/*
		 * Check which kind of surface we have
		 */
		if (is32bit) {
			/*
			 * Call the 32bit transformation routine to do the zooming (using alpha)
			 */
			_zoomSurfaceRGBA(rz_src, rz_dst, flipx, flipy, smooth);

		} else {
			/*
			 * Copy palette and colorkey info
			 */
			for (int i = 0; i < rz_src->format->palette->ncolors; i++) {
				rz_dst->format->palette->colors[i] = rz_src->format->palette->colors[i];
			}
			rz_dst->format->palette->ncolors = rz_src->format->palette->ncolors;

			/*
			 * Call the 8bit transformation routine to do the zooming
			 */
			_zoomSurfaceY(rz_src, rz_dst, flipx, flipy);
		}
	}

	/*
	 * Cleanup temp surface
	 */
	if (src_converted) {
		SDL_FreeSurface(rz_src);
	}

	/*
	 * Return destination surface
	 */
	return (rz_dst);
}

// Calculates the size of the target surface for a zoomSurface() call.
//
// The minimum size of the target surface is 1. The input factors can be positive or negative.
void zoomSurfaceSize(int width, int height, double zoomx, double zoomy, int *dstwidth, int *dstheight) {
	/*
	 * Make zoom factors positive
	 */
	int flipx, flipy;
	flipx = (zoomx < 0.0);
	if (flipx)
		zoomx = -zoomx;
	flipy = (zoomy < 0.0);
	if (flipy)
		zoomy = -zoomy;

	/*
	 * Sanity check zoom factors
	 */
	if (zoomx < VALUE_LIMIT) {
		zoomx = VALUE_LIMIT;
	}
	if (zoomy < VALUE_LIMIT) {
		zoomy = VALUE_LIMIT;
	}

	/*
	 * Calculate target size
	 */
	*dstwidth = (int)floor(((double)width * zoomx) + 0.5);
	*dstheight = (int)floor(((double)height * zoomy) + 0.5);
	if (*dstwidth < 1) {
		*dstwidth = 1;
	}
	if (*dstheight < 1) {
		*dstheight = 1;
	}
}

// Zoom a surface by independent horizontal and vertical factors with optional smoothing.
//
// Zooms a 32bit or 8bit 'src' surface to newly created 'dst' surface.
// 'zoomx' and 'zoomy' are scaling factors for width and height. If 'smooth' is on
// then the destination 32bit surface is anti-aliased. If the surface is not 8bit
// or 32bit RGBA/ABGR it will be converted into a 32bit RGBA format on the fly.
// If zoom factors are negative, the image is flipped on the axes.
SDL_Surface *zoomSurface(SDL_Surface *src, double zoomx, double zoomy, int smooth) {
	SDL_Surface *rz_src;
	SDL_Surface *rz_dst;
	int dstwidth, dstheight;
	int is32bit;
	int src_converted;
	int flipx, flipy;

	/*
	 * Sanity check
	 */
	if (src == NULL)
		return (NULL);

	/*
	 * Determine if source surface is 32bit or 8bit
	 */
	is32bit = (src->format->BitsPerPixel == 32);
	if ((is32bit) || (src->format->BitsPerPixel == 8)) {
		/*
		 * Use source surface 'as is'
		 */
		rz_src = src;
		src_converted = 0;
	} else {
		/*
		 * New source surface is 32bit with a defined RGBA ordering
		 */
		rz_src = SDL_CreateRGBSurface(src->w, src->h, 32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
				0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
#else
				0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
#endif
		);
		if (rz_src == NULL) {
			return NULL;
		}
		SDL_BlitSurface(src, NULL, rz_src, NULL);
		src_converted = 1;
		is32bit = 1;
	}

	flipx = (zoomx < 0);
	if (flipx)
		zoomx = -zoomx;
	flipy = (zoomy < 0);
	if (flipy)
		zoomy = -zoomy;

	/* Get size if target */
	zoomSurfaceSize(rz_src->w, rz_src->h, zoomx, zoomy, &dstwidth, &dstheight);

	/*
	 * Alloc space to completely contain the zoomed surface
	 */
	rz_dst = NULL;
	if (is32bit) {
		/*
		 * Target surface is 32bit with source RGBA/ABGR ordering
		 */
		rz_dst =
				SDL_CreateRGBSurface(dstwidth, dstheight + GUARD_ROWS, 32,
						rz_src->format->Rmask, rz_src->format->Gmask,
						rz_src->format->Bmask, rz_src->format->Amask);
	} else {
		/*
		 * Target surface is 8bit
		 */
		rz_dst = SDL_CreateRGBSurface(dstwidth, dstheight + GUARD_ROWS, 8, 0, 0, 0, 0);
	}

	/* Check target */
	if (rz_dst == NULL) {
		/*
		 * Cleanup temp surface
		 */
		if (src_converted) {
			SDL_FreeSurface(rz_src);
		}
		return NULL;
	}

	/* Adjust for guard rows */
	rz_dst->h = dstheight;

	/*
	 * Check which kind of surface we have
	 */
	if (is32bit) {
		/*
		 * Call the 32bit transformation routine to do the zooming (using alpha)
		 */
		_zoomSurfaceRGBA(rz_src, rz_dst, flipx, flipy, smooth);
	} else {
		/*
		 * Copy palette and colorkey info
		 */
		for (int i = 0; i < rz_src->format->palette->ncolors; i++) {
			rz_dst->format->palette->colors[i] = rz_src->format->palette->colors[i];
		}
		rz_dst->format->palette->ncolors = rz_src->format->palette->ncolors;
		/*
		 * Call the 8bit transformation routine to do the zooming
		 */
		_zoomSurfaceY(rz_src, rz_dst, flipx, flipy);
	}

	/*
	 * Cleanup temp surface
	 */
	if (src_converted) {
		SDL_FreeSurface(rz_src);
	}

	/*
	 * Return destination surface
	 */
	return (rz_dst);
}

// Shrink a surface by an integer ratio using averaging.
//
// Shrinks a 32bit or 8bit 'src' surface to a newly created 'dst' surface.
// 'factorx' and 'factory' are the shrinking ratios (i.e. 2=1/2 the size,
// 3=1/3 the size, etc.) The destination surface is antialiased by averaging
// the source box RGBA or Y information. If the surface is not 8bit
// or 32bit RGBA/ABGR it will be converted into a 32bit RGBA format on the fly.
// The input surface is not modified. The output surface is newly allocated.
/*@null@*/
SDL_Surface *shrinkSurface(SDL_Surface *src, int factorx, int factory) {
	int result;
	SDL_Surface *rz_src;
	SDL_Surface *rz_dst = NULL;
	int dstwidth, dstheight;
	int is32bit;
	int i, src_converted;
	int haveError = 0;

	/*
	 * Sanity check
	 */
	if (src == NULL) {
		return (NULL);
	}

	/*
	 * Determine if source surface is 32bit or 8bit
	 */
	is32bit = (src->format->BitsPerPixel == 32);
	if ((is32bit) || (src->format->BitsPerPixel == 8)) {
		/*
		 * Use source surface 'as is'
		 */
		rz_src = src;
		src_converted = 0;
	} else {
		/*
		 * New source surface is 32bit with a defined RGBA ordering
		 */
		rz_src = SDL_CreateRGBSurface(src->w, src->h, 32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
				0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
#else
				0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
#endif
		);
		if (rz_src == NULL) {
			haveError = 1;
			goto exitShrinkSurface;
		}

		SDL_BlitSurface(src, NULL, rz_src, NULL);
		src_converted = 1;
		is32bit = 1;
	}

	/* Get size for target */
	dstwidth = rz_src->w / factorx;
	while (dstwidth * factorx > rz_src->w) {
		dstwidth--;
	}
	dstheight = rz_src->h / factory;
	while (dstheight * factory > rz_src->h) {
		dstheight--;
	}

	/*
	 * Alloc space to completely contain the shrunken surface
	 * (with added guard rows)
	 */
	if (is32bit == 1) {
		/*
		 * Target surface is 32bit with source RGBA/ABGR ordering
		 */
		rz_dst = SDL_CreateRGBSurface(dstwidth, dstheight + GUARD_ROWS, 32,
				rz_src->format->Rmask, rz_src->format->Gmask,
				rz_src->format->Bmask, rz_src->format->Amask);
	} else {
		/*
		 * Target surface is 8bit
		 */
		rz_dst = SDL_CreateRGBSurface(dstwidth, dstheight + GUARD_ROWS, 8, 0, 0, 0, 0);
	}

	/* Check target */
	if (rz_dst == NULL) {
		haveError = 1;
		goto exitShrinkSurface;
	}

	/* Adjust for guard rows */
	rz_dst->h = dstheight;

	/*
	 * Check which kind of surface we have
	 */
	if (is32bit == 1) {
		/*
		 * Call the 32bit transformation routine to do the shrinking (using alpha)
		 */
		result = _shrinkSurfaceRGBA(rz_src, rz_dst, factorx, factory);
		if ((result != 0) || (rz_dst == NULL)) {
			haveError = 1;
			goto exitShrinkSurface;
		}
	} else {
		/*
		 * Copy palette and colorkey info
		 */
		for (i = 0; i < rz_src->format->palette->ncolors; i++) {
			rz_dst->format->palette->colors[i] = rz_src->format->palette->colors[i];
		}
		rz_dst->format->palette->ncolors = rz_src->format->palette->ncolors;
		/*
		 * Call the 8bit transformation routine to do the shrinking
		 */
		result = _shrinkSurfaceY(rz_src, rz_dst, factorx, factory);
		if (result != 0) {
			haveError = 1;
			goto exitShrinkSurface;
		}
	}

exitShrinkSurface:
	if (rz_src != NULL) {
		/*
		 * Cleanup temp surface
		 */
		if (src_converted == 1) {
			SDL_FreeSurface(rz_src);
		}
	}

	/* Check error state; maybe need to cleanup destination */
	if (haveError == 1) {
		if (rz_dst != NULL) {
			SDL_FreeSurface(rz_dst);
		}
		rz_dst = NULL;
	}

	/*
	 * Return destination surface
	 */
	return (rz_dst);
}

/// Internal

// Returns colorkey info for a surface
static Uint32 get_colorkey(SDL_Surface *src) {
	Uint32 key = 0;
	if (SDL_HasColorKey(src)) {
		SDL_GetColorKey(src, &key);
	}
	return key;
}

// rotate (sx, sy) by (angle, center) into (dx, dy)
static void rotate(double sx, double sy, double sinangle, double cosangle, const SDL_FPoint *center, double *dx, double *dy) {
	sx -= center->x;
	sy -= center->y;

	*dx = cosangle * sx - sinangle * sy;
	*dy = sinangle * sx + cosangle * sy;

	*dx += center->x;
	*dy += center->y;
}

// Internal target surface sizing function for rotations with trig result return.
void SDLgfx_rotozoomSurfaceSizeTrig(int width, int height, double angle, const SDL_FPoint *center, SDL_Rect *rect_dest, double *cangle, double *sangle) {
	int minx, maxx, miny, maxy;
	double radangle;
	double x0, x1, x2, x3;
	double y0, y1, y2, y3;
	double sinangle;
	double cosangle;

	radangle = angle * (M_PI / 180.0);
	sinangle = SDL_sin(radangle);
	cosangle = SDL_cos(radangle);

	/*
	 * Determine destination width and height by rotating a source box, at pixel center
	 */
	rotate(0.5, 0.5, sinangle, cosangle, center, &x0, &y0);
	rotate(width - 0.5, 0.5, sinangle, cosangle, center, &x1, &y1);
	rotate(0.5, height - 0.5, sinangle, cosangle, center, &x2, &y2);
	rotate(width - 0.5, height - 0.5, sinangle, cosangle, center, &x3, &y3);

	minx = (int)SDL_floor(SDL_min(SDL_min(x0, x1), SDL_min(x2, x3)));
	maxx = (int)SDL_ceil(SDL_max(SDL_max(x0, x1), SDL_max(x2, x3)));

	miny = (int)SDL_floor(SDL_min(SDL_min(y0, y1), SDL_min(y2, y3)));
	maxy = (int)SDL_ceil(SDL_max(SDL_max(y0, y1), SDL_max(y2, y3)));

	rect_dest->w = maxx - minx;
	rect_dest->h = maxy - miny;
	rect_dest->x = minx;
	rect_dest->y = miny;

	/* reverse the angle because our rotations are clockwise */
	*sangle = -sinangle;
	*cangle = cosangle;

	{
		/* The trig code below gets the wrong size (due to FP inaccuracy?) when angle is a multiple of 90 degrees */
		int angle90 = (int)(angle / 90);
		if (angle90 == angle / 90) { /* if the angle is a multiple of 90 degrees */
			angle90 %= 4;
			if (angle90 < 0) {
				angle90 += 4; /* 0:0 deg, 1:90 deg, 2:180 deg, 3:270 deg */
			}

			if (angle90 & 1) {
				rect_dest->w = height;
				rect_dest->h = width;
				*cangle = 0;
				*sangle = angle90 == 1 ? -1 : 1; /* reversed because our rotations are clockwise */
			} else {
				rect_dest->w = width;
				rect_dest->h = height;
				*cangle = angle90 == 0 ? 1 : -1;
				*sangle = 0;
			}
		}
	}
}

/* Computes source pointer X/Y increments for a rotation that's a multiple of 90 degrees. */
static void computeSourceIncrements90(SDL_Surface *src, int bpp, int angle, int flipx, int flipy, int *sincx, int *sincy, int *signx, int *signy) {
	int pitch = flipy ? -src->pitch : src->pitch;
	if (flipx) {
		bpp = -bpp;
	}
	switch (angle) { /* 0:0 deg, 1:90 deg, 2:180 deg, 3:270 deg */
		case 0:
			*sincx = bpp;
			*sincy = pitch - src->w * *sincx;
			*signx = *signy = 1;
			break;
		case 1:
			*sincx = -pitch;
			*sincy = bpp - *sincx * src->h;
			*signx = 1;
			*signy = -1;
			break;
		case 2:
			*sincx = -bpp;
			*sincy = -src->w * *sincx - pitch;
			*signx = *signy = -1;
			break;
		case 3:
		default:
			*sincx = pitch;
			*sincy = -*sincx * src->h - bpp;
			*signx = -1;
			*signy = 1;
			break;
	}
	if (flipx) {
		*signx = -*signx;
	}
	if (flipy) {
		*signy = -*signy;
	}
}

/* Performs a relatively fast rotation/flip when the angle is a multiple of 90 degrees. */
#define TRANSFORM_SURFACE_90(pixelType)                                                                         \
	int dincy = dst->pitch - dst->w * sizeof(pixelType), sincx, sincy, signx, signy;                            \
	Uint8 *sp = (Uint8 *)src->pixels, *dp = (Uint8 *)dst->pixels;                                               \
                                                                                                                \
	computeSourceIncrements90(src, sizeof(pixelType), angle, flipx, flipy, &sincx, &sincy, &signx, &signy);     \
	if (signx < 0)                                                                                              \
		sp += (src->w - 1) * sizeof(pixelType);                                                                 \
	if (signy < 0)                                                                                              \
		sp += (src->h - 1) * src->pitch;                                                                        \
                                                                                                                \
	for (int dy = 0; dy < dst->h; sp += sincy, dp += dincy, dy++) {                                             \
		if (sincx == sizeof(pixelType)) { /* if advancing src and dest equally, use SDL_memcpy */               \
			SDL_memcpy(dp, sp, dst->w * sizeof(pixelType));                                                     \
			sp += dst->w * sizeof(pixelType);                                                                   \
			dp += dst->w * sizeof(pixelType);                                                                   \
		} else {                                                                                                \
			for (Uint8 *de = dp + dst->w * sizeof(pixelType); dp != de; sp += sincx, dp += sizeof(pixelType)) { \
				*(pixelType *)dp = *(pixelType *)sp;                                                            \
			}                                                                                                   \
		}                                                                                                       \
	}

static void transformSurfaceRGBA90(SDL_Surface *src, SDL_Surface *dst, int angle, int flipx, int flipy) { TRANSFORM_SURFACE_90(tColorRGBA); }
static void transformSurfaceY90(SDL_Surface *src, SDL_Surface *dst, int angle, int flipx, int flipy) { TRANSFORM_SURFACE_90(tColorY); }

#undef TRANSFORM_SURFACE_90

// Internal 32 bit rotozoomer with optional anti-aliasing.
//
// Rotates and zooms 32 bit RGBA/ABGR 'src' surface to 'dst' surface based on the control
// parameters by scanning the destination surface and applying optionally anti-aliasing
// by bilinear interpolation.
// Assumes src and dst surfaces are of 32 bit depth.
// Assumes dst surface was allocated with the correct dimensions.
static void transformSurfaceRGBA(SDL_Surface *src, SDL_Surface *dst, int isin, int icos, int flipx, int flipy, int smooth, const SDL_Rect *rect_dest, const SDL_FPoint *center) {
	int sw, sh;
	int cx, cy;
	tColorRGBA c00, c01, c10, c11, cswap;
	tColorRGBA *pc, *sp;
	int gap;
	const int fp_half = (1 << 15);

	/*
	 * Variable setup
	 */
	sw = src->w - 1;
	sh = src->h - 1;
	pc = (tColorRGBA *)dst->pixels;
	gap = dst->pitch - dst->w * 4;
	cx = (int)(center->x * 65536.0);
	cy = (int)(center->y * 65536.0);

	/*
	 * Switch between interpolating and non-interpolating code
	 */
	if (smooth) {
		for (int y = 0; y < dst->h; y++) {
			double src_x = (rect_dest->x + 0 + 0.5 - center->x);
			double src_y = (rect_dest->y + y + 0.5 - center->y);
			int sdx = (int)((icos * src_x - isin * src_y) + cx - fp_half);
			int sdy = (int)((isin * src_x + icos * src_y) + cy - fp_half);
			for (int x = 0; x < dst->w; x++) {
				int dx = (sdx >> 16);
				int dy = (sdy >> 16);
				if (flipx) {
					dx = sw - dx;
				}
				if (flipy) {
					dy = sh - dy;
				}
				if ((dx > -1) && (dy > -1) && (dx < (src->w - 1)) && (dy < (src->h - 1))) {
					int ex, ey;
					int t1, t2;
					sp = (tColorRGBA *)((Uint8 *)src->pixels + src->pitch * dy) + dx;
					c00 = *sp;
					sp += 1;
					c01 = *sp;
					sp += (src->pitch / 4);
					c11 = *sp;
					sp -= 1;
					c10 = *sp;
					if (flipx) {
						cswap = c00;
						c00 = c01;
						c01 = cswap;
						cswap = c10;
						c10 = c11;
						c11 = cswap;
					}
					if (flipy) {
						cswap = c00;
						c00 = c10;
						c10 = cswap;
						cswap = c01;
						c01 = c11;
						c11 = cswap;
					}
					/*
					 * Interpolate colors
					 */
					ex = (sdx & 0xffff);
					ey = (sdy & 0xffff);
					t1 = ((((c01.r - c00.r) * ex) >> 16) + c00.r) & 0xff;
					t2 = ((((c11.r - c10.r) * ex) >> 16) + c10.r) & 0xff;
					pc->r = (((t2 - t1) * ey) >> 16) + t1;
					t1 = ((((c01.g - c00.g) * ex) >> 16) + c00.g) & 0xff;
					t2 = ((((c11.g - c10.g) * ex) >> 16) + c10.g) & 0xff;
					pc->g = (((t2 - t1) * ey) >> 16) + t1;
					t1 = ((((c01.b - c00.b) * ex) >> 16) + c00.b) & 0xff;
					t2 = ((((c11.b - c10.b) * ex) >> 16) + c10.b) & 0xff;
					pc->b = (((t2 - t1) * ey) >> 16) + t1;
					t1 = ((((c01.a - c00.a) * ex) >> 16) + c00.a) & 0xff;
					t2 = ((((c11.a - c10.a) * ex) >> 16) + c10.a) & 0xff;
					pc->a = (((t2 - t1) * ey) >> 16) + t1;
				}
				sdx += icos;
				sdy += isin;
				pc++;
			}
			pc = (tColorRGBA *)((Uint8 *)pc + gap);
		}
	} else {
		for (int y = 0; y < dst->h; y++) {
			double src_x = (rect_dest->x + 0 + 0.5 - center->x);
			double src_y = (rect_dest->y + y + 0.5 - center->y);
			int sdx = (int)((icos * src_x - isin * src_y) + cx - fp_half);
			int sdy = (int)((isin * src_x + icos * src_y) + cy - fp_half);
			for (int x = 0; x < dst->w; x++) {
				int dx = (sdx >> 16);
				int dy = (sdy >> 16);
				if ((unsigned)dx < (unsigned)src->w && (unsigned)dy < (unsigned)src->h) {
					if (flipx) {
						dx = sw - dx;
					}
					if (flipy) {
						dy = sh - dy;
					}
					*pc = *((tColorRGBA *)((Uint8 *)src->pixels + src->pitch * dy) + dx);
				}
				sdx += icos;
				sdy += isin;
				pc++;
			}
			pc = (tColorRGBA *)((Uint8 *)pc + gap);
		}
	}
}

// Rotates and zooms 8 bit palette/Y 'src' surface to 'dst' surface without smoothing.
//
// Rotates and zooms 8 bit RGBA/ABGR 'src' surface to 'dst' surface based on the control
// parameters by scanning the destination surface.
// Assumes src and dst surfaces are of 8 bit depth.
// Assumes dst surface was allocated with the correct dimensions.
static void _gfxTransformSurfaceY(SDL_Surface *src, SDL_Surface *dst, int isin, int icos, int flipx, int flipy, const SDL_Rect *rect_dest, const SDL_FPoint *center) {
	int sw, sh;
	int cx, cy;
	tColorY *pc;
	int gap;
	const int fp_half = (1 << 15);

	/*
	 * Variable setup
	 */
	sw = src->w - 1;
	sh = src->h - 1;
	pc = (tColorY *)dst->pixels;
	gap = dst->pitch - dst->w;
	cx = (int)(center->x * 65536.0);
	cy = (int)(center->y * 65536.0);

	/*
	 * Clear surface to colorkey
	 */
	SDL_memset(pc, (int)(get_colorkey(src) & 0xff), (size_t)dst->pitch * dst->h);
	/*
	 * Iterate through destination surface
	 */
	for (int y = 0; y < dst->h; y++) {
		double src_x = (rect_dest->x + 0 + 0.5 - center->x);
		double src_y = (rect_dest->y + y + 0.5 - center->y);
		int sdx = (int)((icos * src_x - isin * src_y) + cx - fp_half);
		int sdy = (int)((isin * src_x + icos * src_y) + cy - fp_half);
		for (int x = 0; x < dst->w; x++) {
			int dx = (sdx >> 16);
			int dy = (sdy >> 16);
			if ((unsigned)dx < (unsigned)src->w && (unsigned)dy < (unsigned)src->h) {
				if (flipx) {
					dx = sw - dx;
				}
				if (flipy) {
					dy = sh - dy;
				}
				*pc = *((tColorY *)src->pixels + src->pitch * dy + dx);
			}
			sdx += icos;
			sdy += isin;
			pc++;
		}
		pc += gap;
	}
}

// Rotates and zooms a surface with different horizontal and vertival scaling factors and optional anti-aliasing.
//
// Rotates a 32-bit or 8-bit 'src' surface to newly created 'dst' surface.
// 'angle' is the rotation in degrees, 'center' the rotation center. If 'smooth' is set
// then the destination 32-bit surface is anti-aliased. 8-bit surfaces must have a colorkey. 32-bit
// surfaces must have a 8888 layout with red, green, blue and alpha masks (any ordering goes).
// The blend mode of the 'src' surface has some effects on generation of the 'dst' surface: The NONE
// mode will set the BLEND mode on the 'dst' surface. The MOD mode either generates a white 'dst'
// surface and sets the colorkey or fills the it with the colorkey before copying the pixels.
// When using the NONE and MOD modes, color and alpha modulation must be applied before using this function.
SDL_Surface *SDLgfx_rotateSurface(SDL_Surface *src, double angle, int smooth, int flipx, int flipy, SDL_Rect *rect_dest, double cangle, double sangle, const SDL_FPoint *center) {
	SDL_Surface *rz_dst;
	int is8bit, angle90;
	int i;
	SDL_BlendMode blendmode;
	Uint32 colorkey = 0;
	int colorKeyAvailable = SDL_FALSE;
	double sangleinv, cangleinv;

	/* Sanity check */
	if (!src) {
		return NULL;
	}

	if (SDL_HasColorKey(src)) {
		if (SDL_GetColorKey(src, &colorkey) == 0) {
			colorKeyAvailable = SDL_TRUE;
		}
	}
	/* This function requires a 32-bit surface or 8-bit surface with a colorkey */
	is8bit = src->format->BitsPerPixel == 8 && colorKeyAvailable;
	if (!(is8bit || (src->format->BitsPerPixel == 32 && src->format->Amask))) {
		return NULL;
	}

	/* Calculate target factors from sine/cosine and zoom */
	sangleinv = sangle * 65536.0;
	cangleinv = cangle * 65536.0;

	/* Alloc space to completely contain the rotated surface */
	rz_dst = NULL;
	if (is8bit) {
		/* Target surface is 8 bit */
		rz_dst = SDL_CreateRGBSurfaceWithFormat(rect_dest->w, rect_dest->h + GUARD_ROWS, 8, src->format->format);
		if (rz_dst) {
			if (src->format->palette) {
				for (i = 0; i < src->format->palette->ncolors; i++) {
					rz_dst->format->palette->colors[i] = src->format->palette->colors[i];
				}
				rz_dst->format->palette->ncolors = src->format->palette->ncolors;
			}
		}
	} else {
		/* Target surface is 32 bit with source RGBA ordering */
		rz_dst = SDL_CreateRGBSurface(rect_dest->w, rect_dest->h + GUARD_ROWS, 32,
				src->format->Rmask, src->format->Gmask,
				src->format->Bmask, src->format->Amask);
	}

	/* Check target */
	if (!rz_dst) {
		return NULL;
	}

	/* Adjust for guard rows */
	rz_dst->h = rect_dest->h;

	SDL_GetSurfaceBlendMode(src, &blendmode);

	if (colorKeyAvailable == SDL_TRUE) {
		/* If available, the colorkey will be used to discard the pixels that are outside of the rotated area. */
		SDL_SetColorKey(rz_dst, SDL_TRUE, colorkey);
		SDL_FillRect(rz_dst, NULL, colorkey);
	} else if (blendmode == SDL_BLENDMODE_NONE) {
		blendmode = SDL_BLENDMODE_BLEND;
	} else if (blendmode == SDL_BLENDMODE_MOD || blendmode == SDL_BLENDMODE_MUL) {
		/* Without a colorkey, the target texture has to be white for the MOD and MUL blend mode so
		 * that the pixels outside the rotated area don't affect the destination surface.
		 */
		colorkey = SDL_MapRGBA(rz_dst->format, 255, 255, 255, 0);
		SDL_FillRect(rz_dst, NULL, colorkey);
		/* Setting a white colorkey for the destination surface makes the final blit discard
		 * all pixels outside of the rotated area. This doesn't interfere with anything because
		 * white pixels are already a no-op and the MOD blend mode does not interact with alpha.
		 */
		SDL_SetColorKey(rz_dst, SDL_TRUE, colorkey);
	}

	SDL_SetSurfaceBlendMode(rz_dst, blendmode);

	/* check if the rotation is a multiple of 90 degrees so we can take a fast path and also somewhat reduce
	 * the off-by-one problem in transformSurfaceRGBA that expresses itself when the rotation is near
	 * multiples of 90 degrees.
	 */
	angle90 = (int)(angle / 90);
	if (angle90 == angle / 90) {
		angle90 %= 4;
		if (angle90 < 0) {
			angle90 += 4; /* 0:0 deg, 1:90 deg, 2:180 deg, 3:270 deg */
		}
	} else {
		angle90 = -1;
	}

	if (is8bit) {
		/* Call the 8-bit transformation routine to do the rotation */
		if (angle90 >= 0) {
			transformSurfaceY90(src, rz_dst, angle90, flipx, flipy);
		} else {
			_gfxTransformSurfaceY(src, rz_dst, (int)sangleinv, (int)cangleinv, flipx, flipy, rect_dest, center);
		}
	} else {
		/* Call the 32-bit transformation routine to do the rotation */
		if (angle90 >= 0) {
			transformSurfaceRGBA90(src, rz_dst, angle90, flipx, flipy);
		} else {
			transformSurfaceRGBA(src, rz_dst, (int)sangleinv, (int)cangleinv, flipx, flipy, smooth, rect_dest, center);
		}
	}

	/* Return rotated surface */
	return rz_dst;
}
