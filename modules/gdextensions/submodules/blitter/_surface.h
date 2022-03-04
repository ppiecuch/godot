/*************************************************************************/
/*  _surface.h                                                           */
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

#ifndef _surface_h_
#define _surface_h_

#include "_blendmode.h"
#include "_error.h"
#include "_pixels.h"
#include "_rect.h"

#include "_begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

#define SDL_PREALLOC 0x00000001 // Surface uses preallocated memory
#define SDL_RLEACCEL 0x00000002 // Surface is RLE encoded
#define SDL_DONTFREE 0x00000004 // Surface is referenced internally
#define SDL_SIMD_ALIGNED 0x00000008 // Surface uses aligned memory

// A collection of pixels used in software blitting.
//
// Note: This structure should be treated as read-only, except for 'pixels',
//       which, if not NULL, contains the raw pixel data for the surface.
typedef struct SDL_Surface {
	Uint32 flags;
	SDL_PixelFormat *format;
	int w, h;
	int pitch;
	void *pixels;

	void *list_blitmap; // list of BlitMap that hold a reference to this surface */

	SDL_Rect clip_rect; // clipping information
	struct SDL_BlitMap *map; // info for fast blit mapping to other surfaces
} SDL_Surface;

// The type of function used for surface blitting functions.
typedef int(SDLCALL *SDL_blit)(struct SDL_Surface *src, SDL_Rect *srcrect, struct SDL_Surface *dst, SDL_Rect *dstrect);

// Allocate a new RGB surface with a specific pixel format.
//
// This function operates mostly like SDL_CreateRGBSurface(), except instead
// of providing pixel color masks, you provide it with a predefined format
// from SDL_PixelFormatEnum.
extern DECLSPEC SDL_Surface *SDLCALL SDL_CreateRGBSurfaceWithFormat(int width, int height, int depth, Uint32 format);

// Allocate a new RGB surface with existing pixel data.
//
// This function operates mostly like SDL_CreateRGBSurface(), except it does
// not allocate memory for the pixel data, instead the caller provides an
// existing buffer of data for the surface to use.
//
// No copy is made of the pixel data. Pixel data is not managed automatically;
// you must free the surface before you free the pixel data.
extern DECLSPEC SDL_Surface *SDLCALL SDL_CreateRGBSurfaceFrom(void *pixels, int width, int height, int depth, int pitch, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);

// Allocate a new RGB surface with empty pixel data.
//
// This function operates mostly like SDL_CreateRGBSurface(), except it does
// not allocate memory for the pixel data, instead the caller provides an
// existing buffer of data for the surface to use.
//
// No copy is made of the pixel data. Pixel data is not managed automatically;
// you must free the surface before you free the pixel data.
extern DECLSPEC SDL_Surface *SDLCALL SDL_CreateRGBEmptySurface(int width, int height, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);

// Allocate a new RGB surface.
//
// If `depth` is 4 or 8 bits, an empty palette is allocated for the surface.
// If `depth` is greater than 8 bits, the pixel format is set using the
// [RGBA]mask parameters.
//
// The [RGBA]mask parameters are the bitmasks used to extract that color from
// a pixel. For instance, `Rmask` being 0xFF000000 means the red data is
// stored in the most significant byte. Using zeros for the RGB masks sets a
// default value, based on the depth. For example:
//
// ```c++
// SDL_CreateRGBSurface(0,w,h,32,0,0,0,0);
// ```
//
// However, using zero for the Amask results in an Amask of 0.
// By default surfaces with an alpha mask are set up for blending as with:
// ```c++
// SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND)
// ```
// You can change this by calling SDL_SetSurfaceBlendMode() and selecting a
// different `blendMode`.
extern DECLSPEC SDL_Surface *SDLCALL SDL_CreateRGBSurface(int width, int height, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask);

// Free an RGB surface.
// It is safe to pass NULL to this function.
extern DECLSPEC void SDLCALL SDL_FreeSurface(SDL_Surface *surface);

// Perform a fast fill of a rectangle with a specific color.
//
// `color` should be a pixel of the format used by the surface, and can be
// generated by SDL_MapRGB() or SDL_MapRGBA(). If the color value contains an
// alpha component then the destination is simply filled with that alpha
// information, no blending takes place.
// If there is a clip rectangle set on the destination (set via
// SDL_SetClipRect()), then this function will fill based on the intersection
// of the clip rectangle and `rect`.
extern DECLSPEC int SDLCALL SDL_FillRect(SDL_Surface *dst, const SDL_Rect *rect, Uint32 color);

// Perform a fast fill of a set of rectangles with a specific color.
//
// `color` should be a pixel of the format used by the surface, and can be
// generated by SDL_MapRGB() or SDL_MapRGBA(). If the color value contains an
// alpha component then the destination is simply filled with that alpha
// information, no blending takes place.
// If there is a clip rectangle set on the destination (set via
// SDL_SetClipRect()), then this function will fill based on the intersection
// of the clip rectangle and `rect`.
extern DECLSPEC int SDLCALL SDL_FillRects(SDL_Surface *dst, const SDL_Rect *rects, int count, Uint32 color);

// Perform a fast blit from the source surface to the destination surface.
extern DECLSPEC int SDLCALL SDL_BlitSurface(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);

// Set the color key (transparent pixel) in a surface.
//
// The color key defines a pixel value that will be treated as transparent in
// a blit. For example, one can use this to specify that cyan pixels should be
// considered transparent, and therefore not rendered.
//
// It is a pixel of the format used by the surface, as generated by
// SDL_MapRGB().
//
// RLE acceleration can substantially speed up blitting of images with large
// horizontal runs of transparent pixels. See SDL_SetSurfaceRLE() for details.
extern DECLSPEC int SDLCALL SDL_SetColorKey(SDL_Surface *surface, int flag, Uint32 key);

// Get the color key (transparent pixel) for a surface.
//
// The color key is a pixel of the format used by the surface, as generated by
// SDL_MapRGB().
// If the surface doesn't have color key enabled this function returns -1.
extern DECLSPEC int SDLCALL SDL_GetColorKey(SDL_Surface *surface, Uint32 *key);

// Returns whether the surface has a color key
// It is safe to pass a NULL `surface` here; it will return SDL_FALSE.
extern DECLSPEC SDL_bool SDLCALL SDL_HasColorKey(SDL_Surface *surface);

// Set the clipping rectangle for a surface.
//
// When `surface` is the destination of a blit, only the area within the clip
// rectangle is drawn into.
// Note that blits are automatically clipped to the edges of the source and
// destination surfaces.
extern DECLSPEC SDL_bool SDLCALL SDL_SetClipRect(SDL_Surface *surface, const SDL_Rect *rect);

// Get the clipping rectangle for a surface.
//
// When `surface` is the destination of a blit, only the area within the clip
// rectangle is drawn into.
extern DECLSPEC void SDLCALL SDL_GetClipRect(SDL_Surface *surface, SDL_Rect *rect);

// Set an additional alpha value used in blit operations.
//
// When this surface is blitted, during the blit operation the source alpha
// value is modulated by this alpha value according to the following formula:
//
// `srcA = srcA * (alpha / 255)`
extern DECLSPEC int SDLCALL SDL_SetSurfaceAlphaMod(SDL_Surface *surface, Uint8 alpha);

// Set the palette used by a surface.
// A single palette can be shared with many surfaces.
extern DECLSPEC int SDLCALL SDL_SetSurfacePalette(SDL_Surface *surface, SDL_Palette *palette);

// Set the blend mode used for blit operations.
//
// To copy a surface to another surface (or texture) without blending with the
// existing data, the blendmode of the SOURCE surface should be set to
// `SDL_BLENDMODE_NONE`.
extern DECLSPEC int SDLCALL SDL_SetSurfaceBlendMode(SDL_Surface *surface, SDL_BlendMode blendMode);

// Get the blend mode used for blit operations.
extern DECLSPEC int SDLCALL SDL_GetSurfaceBlendMode(SDL_Surface *surface, SDL_BlendMode *blendMode);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "_close_code.h"

#endif /* _surface_h_ */
