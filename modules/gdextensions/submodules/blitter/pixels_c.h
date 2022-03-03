/*************************************************************************/
/*  pixels_c.h                                                           */
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

#ifndef _pixels_c_h_
#define _pixels_c_h_

#include "_internal.h"

/// Useful functions and variables from pixel.c

#include "blit.h"

// Pixel format functions
extern int SDL_InitFormat(SDL_PixelFormat *format, Uint32 pixel_format);

// Blit mapping functions
extern SDL_BlitMap *SDL_AllocBlitMap(void);
extern void SDL_InvalidateMap(SDL_BlitMap *map);
extern int SDL_MapSurface(SDL_Surface *src, SDL_Surface *dst);
extern void SDL_FreeBlitMap(SDL_BlitMap *map);

extern void SDL_InvalidateAllBlitMap(SDL_Surface *surface);

// Miscellaneous functions
extern void SDL_DitherColors(SDL_Color *colors, int bpp);
extern Uint8 SDL_FindColor(SDL_Palette *pal, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
extern void SDL_DetectPalette(SDL_Palette *pal, SDL_bool *is_opaque, SDL_bool *has_alpha_channel);

#endif /* _pixels_c_h_ */

/* vi: set ts=4 sw=4 expandtab: */
