/*************************************************************************/
/*  gfx_rotozoom.h                                                       */
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

#ifndef _gfx_rotozoom_h_
#define _gfx_rotozoom_h_

#include <math.h>

#include "_surface.h"

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

// Disable anti-aliasing (no smoothing).
#define SMOOTHING_OFF 0
// Enable anti-aliasing (smoothing).
#define SMOOTHING_ON 1

// Rotozoom functions
extern SDL_Surface *rotozoomSurface(SDL_Surface *src, double angle, double zoom, int smooth);
extern SDL_Surface *rotozoomSurfaceXY(SDL_Surface *src, double angle, double zoomx, double zoomy, int smooth);
extern void rotozoomSurfaceSize(int width, int height, double angle, double zoom, int *dstwidth, int *dstheight);
extern void rotozoomSurfaceSizeXY(int width, int height, double angle, double zoomx, double zoomy, int *dstwidth, int *dstheight);

// Zooming functions
extern SDL_Surface *zoomSurface(SDL_Surface *src, double zoomx, double zoomy, int smooth);
extern void zoomSurfaceSize(int width, int height, double zoomx, double zoomy, int *dstwidth, int *dstheight);

// Shrinking functions
extern SDL_Surface *shrinkSurface(SDL_Surface *src, int factorx, int factory);

// Specialized rotation functions
extern SDL_Surface *rotateSurface90Degrees(SDL_Surface *src, int numClockwiseTurns);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif /* _gfx_rotozoom_h_ */
