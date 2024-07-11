/**************************************************************************/
/*  rotozoom.h                                                            */
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

#ifndef _rotozoom_h_
#define _rotozoom_h_

#include "_surface.h"

//f Disable anti-aliasing (no smoothing).
#define SMOOTHING_OFF 0

// Enable anti-aliasing (smoothing).
#define SMOOTHING_ON 1

/* Rotozoom functions */
DECLSPEC SDL_Surface *rotozoomSurface(SDL_Surface *src, double angle, double zoom, int smooth);
DECLSPEC SDL_Surface *rotozoomSurfaceXY(SDL_Surface *src, double angle, double zoomx, double zoomy, int smooth);
DECLSPEC void rotozoomSurfaceSize(int width, int height, double angle, double zoom, int *dstwidth, int *dstheight);
DECLSPEC void rotozoomSurfaceSizeXY(int width, int height, double angle, double zoomx, double zoomy, int *dstwidth, int *dstheight);

/* Zooming functions */
DECLSPEC SDL_Surface *zoomSurface(SDL_Surface *src, double zoomx, double zoomy, int smooth);
DECLSPEC void zoomSurfaceSize(int width, int height, double zoomx, double zoomy, int *dstwidth, int *dstheight);

/* Shrinking functions */
DECLSPEC SDL_Surface *shrinkSurface(SDL_Surface *src, int factorx, int factory);

/* Specialized rotation functions */
DECLSPEC SDL_Surface *rotateSurface90Degrees(SDL_Surface *src, int numClockwiseTurns);

#endif /* _rotozoom_h_ */
