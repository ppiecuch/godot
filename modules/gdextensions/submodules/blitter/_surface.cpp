/*************************************************************************/
/*  _surface.cpp                                                         */
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

#include "RLEaccel_c.h"
#include "pixels_c.h"

#include "_blendmode.h"
#include "_pixels.h"
#include "_surface.h"

// Calculate the pad-aligned scanline width of a surface
static Sint64 _CalculatePitch(Uint32 format, int width) {
	Sint64 pitch;

	if (SDL_ISPIXELFORMAT_FOURCC(format) || SDL_BITSPERPIXEL(format) >= 8) {
		pitch = ((Sint64)width * SDL_BYTESPERPIXEL(format));
	} else {
		pitch = (((Sint64)width * SDL_BITSPERPIXEL(format)) + 7) / 8;
	}
	pitch = (pitch + 3) & ~3; // 4-byte aligning for speed
	return pitch;
}

// Create an empty RGB surface of the appropriate depth using the given
// enum SDL_PIXELFORMAT_* format
SDL_Surface *SDL_CreateRGBSurfaceWithFormat(int width, int height, int depth, Uint32 format) {
	Sint64 pitch;
	SDL_Surface *surface;

	pitch = _CalculatePitch(format, width);
	if (pitch < 0 || pitch > SDL_MAX_SINT32) {
		// Overflow...
		SDL_OutOfMemory();
		return NULL;
	}

	// Allocate the surface
	surface = (SDL_Surface *)SDL_calloc(1, sizeof(*surface));
	if (surface == NULL) {
		SDL_OutOfMemory();
		return NULL;
	}

	surface->format = SDL_AllocFormat(format);
	if (!surface->format) {
		SDL_FreeSurface(surface);
		return NULL;
	}
	surface->w = width;
	surface->h = height;
	surface->pitch = (int)pitch;
	SDL_SetClipRect(surface, NULL);

	if (SDL_ISPIXELFORMAT_INDEXED(surface->format->format)) {
		SDL_Palette *palette =
				SDL_AllocPalette((1 << surface->format->BitsPerPixel));
		if (!palette) {
			SDL_FreeSurface(surface);
			return NULL;
		}
		if (palette->ncolors == 2) {
			/* Create a black and white bitmap palette */
			palette->colors[0].r = 0xFF;
			palette->colors[0].g = 0xFF;
			palette->colors[0].b = 0xFF;
			palette->colors[1].r = 0x00;
			palette->colors[1].g = 0x00;
			palette->colors[1].b = 0x00;
		}
		SDL_SetSurfacePalette(surface, palette);
		SDL_FreePalette(palette);
	}

	// Get the pixels
	if (surface->w && surface->h) {
		// Assumptions checked in surface_size_assumptions assert above
		Sint64 size = ((Sint64)surface->h * surface->pitch);
		if (size < 0 || size > SDL_MAX_SINT32) {
			// Overflow...
			SDL_FreeSurface(surface);
			SDL_OutOfMemory();
			return NULL;
		}

		surface->pixels = SDL_SIMDAlloc((size_t)size);
		if (!surface->pixels) {
			SDL_FreeSurface(surface);
			SDL_OutOfMemory();
			return NULL;
		}
		surface->flags |= SDL_SIMD_ALIGNED;
		// This is important for bitmaps
		SDL_memset(surface->pixels, 0, surface->h * surface->pitch);
	}

	// Allocate an empty mapping
	surface->map = SDL_AllocBlitMap();
	if (!surface->map) {
		SDL_FreeSurface(surface);
		return NULL;
	}

	// By default surface with an alpha mask are set up for blending
	if (surface->format->Amask) {
		SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
	}

	return surface;
}

// Create an RGB surface from an existing memory buffer
SDL_Surface *SDL_CreateRGBSurfaceFrom(void *pixels, int width, int height, int depth, int pitch, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask) {
	SDL_Surface *surface = SDL_CreateRGBSurface(0, 0, depth, Rmask, Gmask, Bmask, Amask);
	if (surface != NULL) {
		surface->flags |= SDL_PREALLOC;
		surface->pixels = pixels;
		surface->w = width;
		surface->h = height;
		surface->pitch = pitch;
		SDL_SetClipRect(surface, NULL);
	}
	return surface;
}

// Create an RGB surface with no buffer assigned
SDL_Surface *SDL_CreateRGBEmptySurface(int width, int height, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask) {
	// Get the pixel format
	Uint32 format = SDL_MasksToPixelFormatEnum(depth, Rmask, Gmask, Bmask, Amask);
	if (format == SDL_PIXELFORMAT_UNKNOWN) {
		SDL_SetError("Unknown pixel format");
		return NULL;
	}
	Sint64 pitch = _CalculatePitch(format, width);
	if (pitch < 0 || pitch > SDL_MAX_SINT32) {
		// Overflow...
		SDL_OutOfMemory();
		return NULL;
	}
	return SDL_CreateRGBSurfaceFrom(0, width, height, depth, pitch, Rmask, Gmask, Bmask, Amask);
}

// Create an empty RGB surface of the appropriate depth
SDL_Surface *SDL_CreateRGBSurface(int width, int height, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask) {
	// Get the pixel format
	Uint32 format = SDL_MasksToPixelFormatEnum(depth, Rmask, Gmask, Bmask, Amask);
	if (format == SDL_PIXELFORMAT_UNKNOWN) {
		SDL_SetError("Unknown pixel format");
		return NULL;
	}

	return SDL_CreateRGBSurfaceWithFormat(width, height, depth, format);
}

// Free a surface created by the above function.
void SDL_FreeSurface(SDL_Surface *surface) {
	if (surface == NULL) {
		return;
	}
	if (surface->flags & SDL_DONTFREE) {
		return;
	}
	SDL_InvalidateMap(surface->map);

	SDL_InvalidateAllBlitMap(surface);

#if SDL_HAVE_RLE
	if (surface->flags & SDL_RLEACCEL) {
		UnRLESurface(surface, 0);
	}
#endif
	if (surface->format) {
		SDL_SetSurfacePalette(surface, NULL);
		SDL_FreeFormat(surface->format);
		surface->format = NULL;
	}
	if (surface->flags & SDL_PREALLOC) {
		// Don't free
	} else if (surface->flags & SDL_SIMD_ALIGNED) {
		// Free aligned
		SDL_SIMDFree(surface->pixels);
	} else {
		// Normal
		SDL_free(surface->pixels);
	}
	if (surface->map) {
		SDL_FreeBlitMap(surface->map);
	}
	SDL_free(surface);
}

int SDL_SetSurfaceAlphaMod(SDL_Surface *surface, Uint8 alpha) {
	int flags;

	if (!surface) {
		return -1;
	}

	surface->map->info.a = alpha;

	flags = surface->map->info.flags;
	if (alpha != 0xFF) {
		surface->map->info.flags |= SDL_COPY_MODULATE_ALPHA;
	} else {
		surface->map->info.flags &= ~SDL_COPY_MODULATE_ALPHA;
	}
	if (surface->map->info.flags != flags) {
		SDL_InvalidateMap(surface->map);
	}
	return 0;
}

int SDL_SetSurfaceBlendMode(SDL_Surface *surface, SDL_BlendMode blendMode) {
	int flags, status;

	if (!surface) {
		return -1;
	}

	status = 0;
	flags = surface->map->info.flags;
	surface->map->info.flags &=
			~(SDL_COPY_BLEND | SDL_COPY_ADD | SDL_COPY_MOD | SDL_COPY_MUL);
	switch (blendMode) {
		case SDL_BLENDMODE_NONE:
			break;
		case SDL_BLENDMODE_BLEND:
			surface->map->info.flags |= SDL_COPY_BLEND;
			break;
		case SDL_BLENDMODE_ADD:
			surface->map->info.flags |= SDL_COPY_ADD;
			break;
		case SDL_BLENDMODE_MOD:
			surface->map->info.flags |= SDL_COPY_MOD;
			break;
		case SDL_BLENDMODE_MUL:
			surface->map->info.flags |= SDL_COPY_MUL;
			break;
		default:
			status = SDL_Unsupported();
			break;
	}

	if (surface->map->info.flags != flags) {
		SDL_InvalidateMap(surface->map);
	}

	return status;
}

SDL_bool SDL_SetClipRect(SDL_Surface *surface, const SDL_Rect *rect) {
	SDL_Rect full_rect;

	// Don't do anything if there's no surface to act on
	if (!surface) {
		return SDL_FALSE;
	}

	// Set up the full surface rectangle
	full_rect.x = 0;
	full_rect.y = 0;
	full_rect.w = surface->w;
	full_rect.h = surface->h;

	// Set the clipping rectangle
	if (!rect) {
		surface->clip_rect = full_rect;
		return SDL_TRUE;
	}
	return SDL_IntersectRect(rect, &full_rect, &surface->clip_rect);
}

void SDL_GetClipRect(SDL_Surface *surface, SDL_Rect *rect) {
	if (surface && rect) {
		*rect = surface->clip_rect;
	}
}

int SDL_SetSurfacePalette(SDL_Surface *surface, SDL_Palette *palette) {
	if (!surface) {
		return SDL_InvalidParamError("SDL_SetSurfacePalette(): surface");
	}
	if (SDL_SetPixelFormatPalette(surface->format, palette) < 0) {
		return -1;
	}
	SDL_InvalidateMap(surface->map);

	return 0;
}
