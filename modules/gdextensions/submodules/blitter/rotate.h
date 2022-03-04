#ifndef _rotate_h_
#define _rotate_h_

#include "_surface.h"

#ifndef MIN
#define MIN(a,b)    (((a) < (b)) ? (a) : (b))
#endif

extern SDL_Surface *SDLgfx_rotateSurface(SDL_Surface *src, double angle, int smooth, int flipx, int flipy, const SDL_Rect *rect_dest, double cangle, double sangle, const SDL_FPoint *center);
extern void SDLgfx_rotozoomSurfaceSizeTrig(int width, int height, double angle, const SDL_FPoint *center, SDL_Rect *rect_dest, double *cangle, double *sangle);

#endif /* _rotate_h_ */
