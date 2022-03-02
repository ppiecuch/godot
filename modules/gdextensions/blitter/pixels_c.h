#ifndef _pixels_c_h_
#define _pixels_c_h_

#include "_internal.h"

/* Useful functions and variables from pixel.c */

#include "blit.h"

/* Pixel format functions */
extern int SDL_InitFormat(SDL_PixelFormat * format, Uint32 pixel_format);

/* Blit mapping functions */
extern SDL_BlitMap *SDL_AllocBlitMap(void);
extern void SDL_InvalidateMap(SDL_BlitMap * map);
extern int SDL_MapSurface(SDL_Surface * src, SDL_Surface * dst);
extern void SDL_FreeBlitMap(SDL_BlitMap * map);

extern void SDL_InvalidateAllBlitMap(SDL_Surface *surface);

/* Miscellaneous functions */
extern void SDL_DitherColors(SDL_Color * colors, int bpp);
extern Uint8 SDL_FindColor(SDL_Palette * pal, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
extern void SDL_DetectPalette(SDL_Palette *pal, SDL_bool *is_opaque, SDL_bool *has_alpha_channel);

#endif /* _pixels_c_h_ */

/* vi: set ts=4 sw=4 expandtab: */
