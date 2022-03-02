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
extern SDL_Surface* rotateSurface90Degrees(SDL_Surface *src, int numClockwiseTurns);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif /* _gfx_rotozoom_h_ */
