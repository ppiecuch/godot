/**************************************************************************/
/*  _surface.cpp                                                          */
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

#include "_surface.h"
#include "RLEaccel_c.h"
#include "_cpuinfo.h"
#include "_pixels.h"
#include "_private.h"
#include "pixels_c.h"

#include <stdio.h>

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
		SDL_UnRLESurface(surface, 0);
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

int SDL_SetSurfaceRLE(SDL_Surface *surface, int flag) {
	int flags;

	if (!surface) {
		return SDL_InvalidParamError("surface");
	}

	flags = surface->map->info.flags;
	if (flag) {
		surface->map->info.flags |= SDL_COPY_RLE_DESIRED;
	} else {
		surface->map->info.flags &= ~SDL_COPY_RLE_DESIRED;
	}
	if (surface->map->info.flags != flags) {
		SDL_InvalidateMap(surface->map);
	}
	return 0;
}

SDL_bool SDL_HasSurfaceRLE(SDL_Surface *surface) {
	if (!surface) {
		return SDL_FALSE;
	}

	if (!(surface->map->info.flags & SDL_COPY_RLE_DESIRED)) {
		return SDL_FALSE;
	}

	return SDL_TRUE;
}

/// BEGIN Blitting

int SDL_LowerBlit(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
	/* Check to make sure the blit mapping is valid */
	if ((src->map->dst != dst) ||
			(dst->format->palette && src->map->dst_palette_version != dst->format->palette->version) ||
			(src->format->palette && src->map->src_palette_version != src->format->palette->version)) {
		if (SDL_MapSurface(src, dst) < 0) {
			return -1;
		}
		/* just here for debugging */
		/*         printf */
		/*             ("src = 0x%08X src->flags = %08X src->map->info.flags = %08x\ndst = 0x%08X dst->flags = %08X dst->map->info.flags = %08X\nsrc->map->blit = 0x%08x\n", */
		/*              src, dst->flags, src->map->info.flags, dst, dst->flags, */
		/*              dst->map->info.flags, src->map->blit); */
	}
	return src->map->blit(src, srcrect, dst, dstrect);
}

int SDL_UpperBlit(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
	SDL_Rect r_src, r_dst;

	/* Make sure the surfaces aren't locked */
	if (!src) {
		return SDL_InvalidParamError("src");
	} else if (!dst) {
		return SDL_InvalidParamError("dst");
	}

	/* Full src surface */
	r_src.x = 0;
	r_src.y = 0;
	r_src.w = src->w;
	r_src.h = src->h;

	if (dstrect) {
		r_dst.x = dstrect->x;
		r_dst.y = dstrect->y;
	} else {
		r_dst.x = 0;
		r_dst.y = 0;
	}

	/* clip the source rectangle to the source surface */
	if (srcrect) {
		SDL_Rect tmp;
		if (SDL_IntersectRect(srcrect, &r_src, &tmp) == SDL_FALSE) {
			goto end;
		}

		/* Shift dstrect, if srcrect origin has changed */
		r_dst.x += tmp.x - srcrect->x;
		r_dst.y += tmp.y - srcrect->y;

		/* Update srcrect */
		r_src = tmp;
	}

	/* There're no dstrect.w/h parameters. It's the same as srcrect */
	r_dst.w = r_src.w;
	r_dst.h = r_src.h;

	/* clip the destination rectangle against the clip rectangle */
	{
		SDL_Rect tmp;
		if (SDL_IntersectRect(&r_dst, &dst->clip_rect, &tmp) == SDL_FALSE) {
			goto end;
		}

		/* Shift srcrect, if dstrect has changed */
		r_src.x += tmp.x - r_dst.x;
		r_src.y += tmp.y - r_dst.y;
		r_src.w = tmp.w;
		r_src.h = tmp.h;

		/* Update dstrect */
		r_dst = tmp;
	}

	/* Switch back to a fast blit if we were previously stretching */
	if (src->map->info.flags & SDL_COPY_NEAREST) {
		src->map->info.flags &= ~SDL_COPY_NEAREST;
		SDL_InvalidateMap(src->map);
	}

	if (r_dst.w > 0 && r_dst.h > 0) {
		if (dstrect) { /* update output parameter */
			*dstrect = r_dst;
		}
		return SDL_LowerBlit(src, &r_src, dst, &r_dst);
	}

end:
	if (dstrect) { /* update output parameter */
		dstrect->w = dstrect->h = 0;
	}
	return 0;
}

/// END Blitting

/// BEGIN Blitting scaled

int SDL_UpperBlitScaled(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
	return SDL_PrivateUpperBlitScaled(src, srcrect, dst, dstrect, SDL_ScaleModeNearest);
}

int SDL_PrivateUpperBlitScaled(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect, SDL_ScaleMode scaleMode) {
	double src_x0, src_y0, src_x1, src_y1;
	double dst_x0, dst_y0, dst_x1, dst_y1;
	SDL_Rect final_src, final_dst;
	double scaling_w, scaling_h;
	int src_w, src_h;
	int dst_w, dst_h;

	/* Make sure the surfaces aren't locked */
	if (!src || !dst) {
		return SDL_InvalidParamError("SDL_UpperBlitScaled(): src/dst");
	}

	if (!srcrect) {
		src_w = src->w;
		src_h = src->h;
	} else {
		src_w = srcrect->w;
		src_h = srcrect->h;
	}

	if (!dstrect) {
		dst_w = dst->w;
		dst_h = dst->h;
	} else {
		dst_w = dstrect->w;
		dst_h = dstrect->h;
	}

	if (dst_w == src_w && dst_h == src_h) {
		/* No scaling, defer to regular blit */
		return SDL_BlitSurface(src, srcrect, dst, dstrect);
	}

	scaling_w = (double)dst_w / src_w;
	scaling_h = (double)dst_h / src_h;

	if (!dstrect) {
		dst_x0 = 0;
		dst_y0 = 0;
		dst_x1 = dst_w;
		dst_y1 = dst_h;
	} else {
		dst_x0 = dstrect->x;
		dst_y0 = dstrect->y;
		dst_x1 = dst_x0 + dst_w;
		dst_y1 = dst_y0 + dst_h;
	}

	if (!srcrect) {
		src_x0 = 0;
		src_y0 = 0;
		src_x1 = src_w;
		src_y1 = src_h;
	} else {
		src_x0 = srcrect->x;
		src_y0 = srcrect->y;
		src_x1 = src_x0 + src_w;
		src_y1 = src_y0 + src_h;

		/* Clip source rectangle to the source surface */

		if (src_x0 < 0) {
			dst_x0 -= src_x0 * scaling_w;
			src_x0 = 0;
		}

		if (src_x1 > src->w) {
			dst_x1 -= (src_x1 - src->w) * scaling_w;
			src_x1 = src->w;
		}

		if (src_y0 < 0) {
			dst_y0 -= src_y0 * scaling_h;
			src_y0 = 0;
		}

		if (src_y1 > src->h) {
			dst_y1 -= (src_y1 - src->h) * scaling_h;
			src_y1 = src->h;
		}
	}

	/* Clip destination rectangle to the clip rectangle */

	/* Translate to clip space for easier calculations */
	dst_x0 -= dst->clip_rect.x;
	dst_x1 -= dst->clip_rect.x;
	dst_y0 -= dst->clip_rect.y;
	dst_y1 -= dst->clip_rect.y;

	if (dst_x0 < 0) {
		src_x0 -= dst_x0 / scaling_w;
		dst_x0 = 0;
	}

	if (dst_x1 > dst->clip_rect.w) {
		src_x1 -= (dst_x1 - dst->clip_rect.w) / scaling_w;
		dst_x1 = dst->clip_rect.w;
	}

	if (dst_y0 < 0) {
		src_y0 -= dst_y0 / scaling_h;
		dst_y0 = 0;
	}

	if (dst_y1 > dst->clip_rect.h) {
		src_y1 -= (dst_y1 - dst->clip_rect.h) / scaling_h;
		dst_y1 = dst->clip_rect.h;
	}

	/* Translate back to surface coordinates */
	dst_x0 += dst->clip_rect.x;
	dst_x1 += dst->clip_rect.x;
	dst_y0 += dst->clip_rect.y;
	dst_y1 += dst->clip_rect.y;

	final_src.x = (int)SDL_round(src_x0);
	final_src.y = (int)SDL_round(src_y0);
	final_src.w = (int)SDL_round(src_x1 - src_x0);
	final_src.h = (int)SDL_round(src_y1 - src_y0);

	final_dst.x = (int)SDL_round(dst_x0);
	final_dst.y = (int)SDL_round(dst_y0);
	final_dst.w = (int)SDL_round(dst_x1 - dst_x0);
	final_dst.h = (int)SDL_round(dst_y1 - dst_y0);

	/* Clip again */
	{
		SDL_Rect tmp;
		tmp.x = 0;
		tmp.y = 0;
		tmp.w = src->w;
		tmp.h = src->h;
		SDL_IntersectRect(&tmp, &final_src, &final_src);
	}

	/* Clip again */
	SDL_IntersectRect(&dst->clip_rect, &final_dst, &final_dst);

	if (dstrect) {
		*dstrect = final_dst;
	}

	if (final_dst.w == 0 || final_dst.h == 0 ||
			final_src.w <= 0 || final_src.h <= 0) {
		/* No-op. */
		return 0;
	}

	return SDL_PrivateLowerBlitScaled(src, &final_src, dst, &final_dst, scaleMode);
}

/**
 *  This is a semi-private blit function and it performs low-level surface
 *  scaled blitting only.
 */
int SDL_LowerBlitScaled(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
	return SDL_PrivateLowerBlitScaled(src, srcrect, dst, dstrect, SDL_ScaleModeNearest);
}

int SDL_PrivateLowerBlitScaled(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect, SDL_ScaleMode scaleMode) {
	static const Uint32 complex_copy_flags = (SDL_COPY_MODULATE_COLOR | SDL_COPY_MODULATE_ALPHA |
			SDL_COPY_BLEND | SDL_COPY_ADD | SDL_COPY_MOD | SDL_COPY_MUL |
			SDL_COPY_COLORKEY);

	if (srcrect->w > SDL_MAX_UINT16 || srcrect->h > SDL_MAX_UINT16 ||
			dstrect->w > SDL_MAX_UINT16 || dstrect->h > SDL_MAX_UINT16) {
		return SDL_SetError("Size too large for scaling");
	}

	if (!(src->map->info.flags & SDL_COPY_NEAREST)) {
		src->map->info.flags |= SDL_COPY_NEAREST;
		SDL_InvalidateMap(src->map);
	}

	if (scaleMode == SDL_ScaleModeNearest) {
		if (!(src->map->info.flags & complex_copy_flags) &&
				src->format->format == dst->format->format &&
				!SDL_ISPIXELFORMAT_INDEXED(src->format->format)) {
			return SDL_SoftStretch(src, srcrect, dst, dstrect);
		} else {
			return SDL_LowerBlit(src, srcrect, dst, dstrect);
		}
	} else {
		if (!(src->map->info.flags & complex_copy_flags) &&
				src->format->format == dst->format->format &&
				!SDL_ISPIXELFORMAT_INDEXED(src->format->format) &&
				src->format->BytesPerPixel == 4 &&
				src->format->format != SDL_PIXELFORMAT_ARGB2101010) {
			/* fast path */
			return SDL_SoftStretchLinear(src, srcrect, dst, dstrect);
		} else {
			/* Use intermediate surface(s) */
			SDL_Surface *tmp1 = NULL;
			int ret;
			SDL_Rect srcrect2;
			int is_complex_copy_flags = (src->map->info.flags & complex_copy_flags);

			Uint8 r, g, b;
			Uint8 alpha;
			SDL_BlendMode blendMode;

			/* Save source infos */
			SDL_GetSurfaceColorMod(src, &r, &g, &b);
			SDL_GetSurfaceAlphaMod(src, &alpha);
			SDL_GetSurfaceBlendMode(src, &blendMode);
			srcrect2.x = srcrect->x;
			srcrect2.y = srcrect->y;
			srcrect2.w = srcrect->w;
			srcrect2.h = srcrect->h;

			/* Change source format if not appropriate for scaling */
			if (src->format->BytesPerPixel != 4 || src->format->format == SDL_PIXELFORMAT_ARGB2101010) {
				SDL_Rect tmprect;
				int fmt;
				tmprect.x = 0;
				tmprect.y = 0;
				tmprect.w = src->w;
				tmprect.h = src->h;
				if (dst->format->BytesPerPixel == 4 && dst->format->format != SDL_PIXELFORMAT_ARGB2101010) {
					fmt = dst->format->format;
				} else {
					fmt = SDL_PIXELFORMAT_ARGB8888;
				}
				tmp1 = SDL_CreateRGBSurfaceWithFormat(src->w, src->h, 0, fmt);
				SDL_LowerBlit(src, srcrect, tmp1, &tmprect);

				srcrect2.x = 0;
				srcrect2.y = 0;
				SDL_SetSurfaceColorMod(tmp1, r, g, b);
				SDL_SetSurfaceAlphaMod(tmp1, alpha);
				SDL_SetSurfaceBlendMode(tmp1, blendMode);

				src = tmp1;
			}

			/* Intermediate scaling */
			if (is_complex_copy_flags || src->format->format != dst->format->format) {
				SDL_Rect tmprect;
				SDL_Surface *tmp2 = SDL_CreateRGBSurfaceWithFormat(dstrect->w, dstrect->h, 0, src->format->format);
				SDL_SoftStretchLinear(src, &srcrect2, tmp2, NULL);

				SDL_SetSurfaceColorMod(tmp2, r, g, b);
				SDL_SetSurfaceAlphaMod(tmp2, alpha);
				SDL_SetSurfaceBlendMode(tmp2, blendMode);

				tmprect.x = 0;
				tmprect.y = 0;
				tmprect.w = dstrect->w;
				tmprect.h = dstrect->h;
				ret = SDL_LowerBlit(tmp2, &tmprect, dst, dstrect);
				SDL_FreeSurface(tmp2);
			} else {
				ret = SDL_SoftStretchLinear(src, &srcrect2, dst, dstrect);
			}

			SDL_FreeSurface(tmp1);
			return ret;
		}
	}
}

/// END Blitting scaled

/// BEGIN Stretch

static int SDL_LowerSoftStretchNearest(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, const SDL_Rect *dstrect);
static int SDL_LowerSoftStretchLinear(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, const SDL_Rect *dstrect);
static int SDL_UpperSoftStretch(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, const SDL_Rect *dstrect, SDL_ScaleMode scaleMode);

int SDL_SoftStretch(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, const SDL_Rect *dstrect) {
	return SDL_UpperSoftStretch(src, srcrect, dst, dstrect, SDL_ScaleModeNearest);
}

int SDL_SoftStretchLinear(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, const SDL_Rect *dstrect) {
	return SDL_UpperSoftStretch(src, srcrect, dst, dstrect, SDL_ScaleModeLinear);
}

static int SDL_UpperSoftStretch(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, const SDL_Rect *dstrect, SDL_ScaleMode scaleMode) {
	int ret;
	SDL_Rect full_src;
	SDL_Rect full_dst;

	if (src->format->format != dst->format->format) {
		return SDL_SetError("Only works with same format surfaces");
	}

	if (scaleMode != SDL_ScaleModeNearest) {
		if (src->format->BytesPerPixel != 4 || src->format->format == SDL_PIXELFORMAT_ARGB2101010) {
			return SDL_SetError("Wrong format");
		}
	}

	/* Verify the blit rectangles */
	if (srcrect) {
		if ((srcrect->x < 0) || (srcrect->y < 0) ||
				((srcrect->x + srcrect->w) > src->w) ||
				((srcrect->y + srcrect->h) > src->h)) {
			return SDL_SetError("Invalid source blit rectangle");
		}
	} else {
		full_src.x = 0;
		full_src.y = 0;
		full_src.w = src->w;
		full_src.h = src->h;
		srcrect = &full_src;
	}
	if (dstrect) {
		if ((dstrect->x < 0) || (dstrect->y < 0) ||
				((dstrect->x + dstrect->w) > dst->w) ||
				((dstrect->y + dstrect->h) > dst->h)) {
			return SDL_SetError("Invalid destination blit rectangle");
		}
	} else {
		full_dst.x = 0;
		full_dst.y = 0;
		full_dst.w = dst->w;
		full_dst.h = dst->h;
		dstrect = &full_dst;
	}

	if (dstrect->w <= 0 || dstrect->h <= 0) {
		return 0;
	}

	if (srcrect->w > SDL_MAX_UINT16 || srcrect->h > SDL_MAX_UINT16 ||
			dstrect->w > SDL_MAX_UINT16 || dstrect->h > SDL_MAX_UINT16) {
		return SDL_SetError("Size too large for scaling");
	}

	/* Lock the destination if it's in hardware */
	if (scaleMode == SDL_ScaleModeNearest) {
		ret = SDL_LowerSoftStretchNearest(src, srcrect, dst, dstrect);
	} else {
		ret = SDL_LowerSoftStretchLinear(src, srcrect, dst, dstrect);
	}

	return ret;
}

/* bilinear interpolation precision must be < 8
   Because with SSE: add-multiply: _mm_madd_epi16 works with signed int
   so pixels 0xb1...... are negatives and false the result
   same in NEON probably */
#define PRECISION 7

#define FIXED_POINT(i) ((Uint32)(i) << 16)
#define SRC_INDEX(fp) ((Uint32)(fp) >> 16)
#define INTEGER(fp) ((Uint32)(fp) >> PRECISION)
#define FRAC(fp) ((Uint32)(fp >> (16 - PRECISION)) & ((1 << PRECISION) - 1))
#define FRAC_ZERO 0
#define FRAC_ONE (1 << PRECISION)
#define FP_ONE FIXED_POINT(1)

#define BILINEAR___START                                                              \
	Sint64 fp_sum_h;                                                                  \
	int fp_step_h, left_pad_h, right_pad_h;                                           \
	Sint64 fp_sum_w;                                                                  \
	int fp_step_w, left_pad_w, right_pad_w;                                           \
	Sint64 fp_sum_w_init;                                                             \
	int left_pad_w_init, right_pad_w_init, dst_gap, middle_init;                      \
	get_scaler_datas(src_h, dst_h, &fp_sum_h, &fp_step_h, &left_pad_h, &right_pad_h); \
	get_scaler_datas(src_w, dst_w, &fp_sum_w, &fp_step_w, &left_pad_w, &right_pad_w); \
	fp_sum_w_init = fp_sum_w + left_pad_w * fp_step_w;                                \
	left_pad_w_init = left_pad_w;                                                     \
	right_pad_w_init = right_pad_w;                                                   \
	dst_gap = dst_pitch - 4 * dst_w;                                                  \
	middle_init = dst_w - left_pad_w - right_pad_w;

#define BILINEAR___HEIGHT                                              \
	int index_h, frac_h0, frac_h1, middle;                             \
	const Uint32 *src_h0, *src_h1;                                     \
	int no_padding;                                                    \
	Uint64 incr_h0, incr_h1;                                           \
                                                                       \
	no_padding = !(i < left_pad_h || i > dst_h - 1 - right_pad_h);     \
	index_h = SRC_INDEX(fp_sum_h);                                     \
	frac_h0 = FRAC(fp_sum_h);                                          \
                                                                       \
	index_h = no_padding ? index_h : (i < left_pad_h ? 0 : src_h - 1); \
	frac_h0 = no_padding ? frac_h0 : 0;                                \
	incr_h1 = no_padding ? src_pitch : 0;                              \
	incr_h0 = (Uint64)index_h * src_pitch;                             \
                                                                       \
	src_h0 = (const Uint32 *)((const Uint8 *)src + incr_h0);           \
	src_h1 = (const Uint32 *)((const Uint8 *)src_h0 + incr_h1);        \
                                                                       \
	fp_sum_h += fp_step_h;                                             \
                                                                       \
	frac_h1 = FRAC_ONE - frac_h0;                                      \
	fp_sum_w = fp_sum_w_init;                                          \
	right_pad_w = right_pad_w_init;                                    \
	left_pad_w = left_pad_w_init;                                      \
	middle = middle_init;

#if defined(__clang__)
// Remove inlining of this function
// Compiler crash with clang 9.0.8 / android-ndk-r21d
// Compiler crash with clang 11.0.3 / Xcode
// OK with clang 11.0.5 / android-ndk-22
// OK with clang 12.0.0 / Xcode
__attribute__((noinline))
#endif // __clang__
static void
get_scaler_datas(int src_nb, int dst_nb, Sint64 *fp_start, int *fp_step, int *left_pad, int *right_pad) {
	int step = FIXED_POINT(src_nb) / (dst_nb); /* source step in fixed point */
	int x0 = FP_ONE / 2; /* dst first pixel center at 0.5 in fixed point */
	Sint64 fp_sum;
#if 0 /* scale to source coordinates */
    x0 *= src_nb;
    x0 /= dst_nb; /* x0 == step / 2 */
#else /* Use this code for perfect match with pixman */
	Sint64 tmp[2];
	tmp[0] = (Sint64)step * (x0 >> 16);
	tmp[1] = (Sint64)step * (x0 & 0xFFFF);
	x0 = (int)(tmp[0] + ((tmp[1] + 0x8000) >> 16)); /*  x0 == (step + 1) / 2  */
#endif
	/* -= 0.5, get back the pixel origin, in source coordinates  */
	x0 -= FP_ONE / 2;

	*fp_start = x0;
	*fp_step = step;
	*left_pad = 0;
	*right_pad = 0;

	fp_sum = x0;
	for (int i = 0; i < dst_nb; i++) {
		if (fp_sum < 0) {
			*left_pad += 1;
		} else {
			int index = SRC_INDEX(fp_sum);
			if (index > src_nb - 2) {
				*right_pad += 1;
			}
		}
		fp_sum += step;
	}
	// SDL_Log("%d -> %d  x0=%d step=%d left_pad=%d right_pad=%d", src_nb, dst_nb, *fp_start, *fp_step, *left_pad, *right_pad);
}

typedef struct color_t {
	Uint8 a;
	Uint8 b;
	Uint8 c;
	Uint8 d;
} color_t;

#if 0
static SDL_INLINE void printf_64(const char *str, void *var) {
	uint8_t *val = (uint8_t *)var;
	printf(" *   %s: %02x %02x %02x %02x _ %02x %02x %02x %02x\n", str, val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7]);
}
#endif

//
// Interpolated == x0 + frac * (x1 - x0) == x0 * (1 - frac) + x1 * frac
//

static SDL_INLINE void INTERPOL(const Uint32 *src_x0, const Uint32 *src_x1, int frac0, int frac1, Uint32 *dst) {
	const color_t *c0 = (const color_t *)src_x0;
	const color_t *c1 = (const color_t *)src_x1;
	color_t *cx = (color_t *)dst;
#if 0
    cx->a = c0->a + INTEGER(frac0 * (c1->a - c0->a));
    cx->b = c0->b + INTEGER(frac0 * (c1->b - c0->b));
    cx->c = c0->c + INTEGER(frac0 * (c1->c - c0->c));
    cx->d = c0->d + INTEGER(frac0 * (c1->d - c0->d));
#else
	cx->a = INTEGER(frac1 * c0->a + frac0 * c1->a);
	cx->b = INTEGER(frac1 * c0->b + frac0 * c1->b);
	cx->c = INTEGER(frac1 * c0->c + frac0 * c1->c);
	cx->d = INTEGER(frac1 * c0->d + frac0 * c1->d);
#endif
}

static SDL_INLINE void INTERPOL_BILINEAR(const Uint32 *s0, const Uint32 *s1, int frac_w0, int frac_h0, int frac_h1, Uint32 *dst) {
	Uint32 tmp[2];
	unsigned int frac_w1 = FRAC_ONE - frac_w0;

	/* Vertical first, store to 'tmp' */
	INTERPOL(s0, s1, frac_h0, frac_h1, tmp);
	INTERPOL(s0 + 1, s1 + 1, frac_h0, frac_h1, tmp + 1);

	/* Horizontal, store to 'dst' */
	INTERPOL(tmp, tmp + 1, frac_w0, frac_w1, dst);
}

static int scale_mat(const Uint32 *src, int src_w, int src_h, int src_pitch, Uint32 *dst, int dst_w, int dst_h, int dst_pitch) {
	BILINEAR___START

	for (int i = 0; i < dst_h; i++) {
		BILINEAR___HEIGHT

		while (left_pad_w--) {
			INTERPOL_BILINEAR(src_h0, src_h1, FRAC_ZERO, frac_h0, frac_h1, dst);
			dst += 1;
		}

		while (middle--) {
			const Uint32 *s_00_01;
			const Uint32 *s_10_11;
			int index_w = 4 * SRC_INDEX(fp_sum_w);
			int frac_w = FRAC(fp_sum_w);
			fp_sum_w += fp_step_w;

			/*
						x00 ... x0_ ..... x01
						.       .         .
						.       x         .
						.       .         .
						.       .         .
						x10 ... x1_ ..... x11
			*/
			s_00_01 = (const Uint32 *)((const Uint8 *)src_h0 + index_w);
			s_10_11 = (const Uint32 *)((const Uint8 *)src_h1 + index_w);

			INTERPOL_BILINEAR(s_00_01, s_10_11, frac_w, frac_h0, frac_h1, dst);

			dst += 1;
		}

		while (right_pad_w--) {
			int index_w = 4 * (src_w - 2);
			const Uint32 *s_00_01 = (const Uint32 *)((const Uint8 *)src_h0 + index_w);
			const Uint32 *s_10_11 = (const Uint32 *)((const Uint8 *)src_h1 + index_w);
			INTERPOL_BILINEAR(s_00_01, s_10_11, FRAC_ONE, frac_h0, frac_h1, dst);
			dst += 1;
		}
		dst = (Uint32 *)((Uint8 *)dst + dst_gap);
	}
	return 0;
}

#if defined(__SSE2__)
#define HAVE_SSE2_INTRINSICS
#endif

#if defined(__ARM_NEON)
#define HAVE_NEON_INTRINSICS 1
#define CAST_uint8x8_t (uint8x8_t)
#define CAST_uint32x2_t (uint32x2_t)
#endif

#if defined(__WINRT__) || defined(_MSC_VER)
#if defined(HAVE_NEON_INTRINSICS)
#undef CAST_uint8x8_t
#undef CAST_uint32x2_t
#define CAST_uint8x8_t
#define CAST_uint32x2_t
#endif
#endif

#if defined(HAVE_SSE2_INTRINSICS)

#if 0
static SDL_INLINE void printf_128(const char *str, __m128i var) {
	uint16_t *val = (uint16_t *)&var;
	printf(" *   %s: %04x %04x %04x %04x _ %04x %04x %04x %04x\n", str, val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7]);
}
#endif

static SDL_INLINE int hasSSE2() {
	static int val = -1;
	if (val != -1) {
		return val;
	}
	val = SDL_HasSSE2();
	return val;
}

static SDL_INLINE void INTERPOL_BILINEAR_SSE(const Uint32 *s0, const Uint32 *s1, int frac_w, __m128i v_frac_h0, __m128i v_frac_h1, Uint32 *dst, __m128i zero) {
	__m128i x_00_01, x_10_11; /* Pixels in 4*uint8 in row */
	__m128i v_frac_w0, k0, l0, d0, e0;

	int f, f2;
	f = frac_w;
	f2 = FRAC_ONE - frac_w;
	v_frac_w0 = _mm_set_epi16(f, f2, f, f2, f, f2, f, f2);

	x_00_01 = _mm_loadl_epi64((const __m128i *)s0); /* Load x00 and x01 */
	x_10_11 = _mm_loadl_epi64((const __m128i *)s1);

	/* Interpolated == x0 + frac * (x1 - x0) == x0 * (1 - frac) + x1 * frac */

	/* Interpolation vertical */
	k0 = _mm_mullo_epi16(_mm_unpacklo_epi8(x_00_01, zero), v_frac_h1);
	l0 = _mm_mullo_epi16(_mm_unpacklo_epi8(x_10_11, zero), v_frac_h0);
	k0 = _mm_add_epi16(k0, l0);

	/* For perfect match, clear the factionnal part eventually. */
	/*
	k0 = _mm_srli_epi16(k0, PRECISION);
	k0 = _mm_slli_epi16(k0, PRECISION);
	*/

	/* Interpolation horizontal */
	l0 = _mm_unpacklo_epi64(/* unused */ l0, k0);
	k0 = _mm_madd_epi16(_mm_unpackhi_epi16(l0, k0), v_frac_w0);

	/* Store 1 pixel */
	d0 = _mm_srli_epi32(k0, PRECISION * 2);
	e0 = _mm_packs_epi32(d0, d0);
	e0 = _mm_packus_epi16(e0, e0);
	*dst = _mm_cvtsi128_si32(e0);
}

static int scale_mat_SSE(const Uint32 *src, int src_w, int src_h, int src_pitch, Uint32 *dst, int dst_w, int dst_h, int dst_pitch) {
	BILINEAR___START

	for (i = 0; i < dst_h; i++) {
		int nb_block2;
		__m128i v_frac_h0;
		__m128i v_frac_h1;
		__m128i zero;

		BILINEAR___HEIGHT

		nb_block2 = middle / 2;

		v_frac_h0 = _mm_set_epi16(frac_h0, frac_h0, frac_h0, frac_h0, frac_h0, frac_h0, frac_h0, frac_h0);
		v_frac_h1 = _mm_set_epi16(frac_h1, frac_h1, frac_h1, frac_h1, frac_h1, frac_h1, frac_h1, frac_h1);
		zero = _mm_setzero_si128();

		while (left_pad_w--) {
			INTERPOL_BILINEAR_SSE(src_h0, src_h1, FRAC_ZERO, v_frac_h0, v_frac_h1, dst, zero);
			dst += 1;
		}

		while (nb_block2--) {
			int index_w_0, frac_w_0;
			int index_w_1, frac_w_1;

			const Uint32 *s_00_01, *s_02_03, *s_10_11, *s_12_13;

			__m128i x_00_01, x_10_11, x_02_03, x_12_13; /* Pixels in 4*uint8 in row */
			__m128i v_frac_w0, k0, l0, d0, e0;
			__m128i v_frac_w1, k1, l1, d1, e1;

			int f, f2;
			index_w_0 = 4 * SRC_INDEX(fp_sum_w);
			frac_w_0 = FRAC(fp_sum_w);
			fp_sum_w += fp_step_w;
			index_w_1 = 4 * SRC_INDEX(fp_sum_w);
			frac_w_1 = FRAC(fp_sum_w);
			fp_sum_w += fp_step_w;
			/*
						x00............ x01   x02...........x03
						.      .         .     .       .     .
						j0     f0        j1    j2      f1    j3
						.      .         .     .       .     .
						.      .         .     .       .     .
						.      .         .     .       .     .
						x10............ x11   x12...........x13
			 */
			s_00_01 = (const Uint32 *)((const Uint8 *)src_h0 + index_w_0);
			s_02_03 = (const Uint32 *)((const Uint8 *)src_h0 + index_w_1);
			s_10_11 = (const Uint32 *)((const Uint8 *)src_h1 + index_w_0);
			s_12_13 = (const Uint32 *)((const Uint8 *)src_h1 + index_w_1);

			f = frac_w_0;
			f2 = FRAC_ONE - frac_w_0;
			v_frac_w0 = _mm_set_epi16(f, f2, f, f2, f, f2, f, f2);

			f = frac_w_1;
			f2 = FRAC_ONE - frac_w_1;
			v_frac_w1 = _mm_set_epi16(f, f2, f, f2, f, f2, f, f2);

			x_00_01 = _mm_loadl_epi64((const __m128i *)s_00_01); /* Load x00 and x01 */
			x_02_03 = _mm_loadl_epi64((const __m128i *)s_02_03);
			x_10_11 = _mm_loadl_epi64((const __m128i *)s_10_11);
			x_12_13 = _mm_loadl_epi64((const __m128i *)s_12_13);

			/* Interpolation vertical */
			k0 = _mm_mullo_epi16(_mm_unpacklo_epi8(x_00_01, zero), v_frac_h1);
			l0 = _mm_mullo_epi16(_mm_unpacklo_epi8(x_10_11, zero), v_frac_h0);
			k0 = _mm_add_epi16(k0, l0);
			k1 = _mm_mullo_epi16(_mm_unpacklo_epi8(x_02_03, zero), v_frac_h1);
			l1 = _mm_mullo_epi16(_mm_unpacklo_epi8(x_12_13, zero), v_frac_h0);
			k1 = _mm_add_epi16(k1, l1);

			/* Interpolation horizontal */
			l0 = _mm_unpacklo_epi64(/* unused */ l0, k0);
			k0 = _mm_madd_epi16(_mm_unpackhi_epi16(l0, k0), v_frac_w0);
			l1 = _mm_unpacklo_epi64(/* unused */ l1, k1);
			k1 = _mm_madd_epi16(_mm_unpackhi_epi16(l1, k1), v_frac_w1);

			/* Store 1 pixel */
			d0 = _mm_srli_epi32(k0, PRECISION * 2);
			e0 = _mm_packs_epi32(d0, d0);
			e0 = _mm_packus_epi16(e0, e0);
			*dst++ = _mm_cvtsi128_si32(e0);

			/* Store 1 pixel */
			d1 = _mm_srli_epi32(k1, PRECISION * 2);
			e1 = _mm_packs_epi32(d1, d1);
			e1 = _mm_packus_epi16(e1, e1);
			*dst++ = _mm_cvtsi128_si32(e1);
		}

		/* Last point */
		if (middle & 0x1) {
			const Uint32 *s_00_01;
			const Uint32 *s_10_11;
			int index_w = 4 * SRC_INDEX(fp_sum_w);
			int frac_w = FRAC(fp_sum_w);
			fp_sum_w += fp_step_w;
			s_00_01 = (const Uint32 *)((const Uint8 *)src_h0 + index_w);
			s_10_11 = (const Uint32 *)((const Uint8 *)src_h1 + index_w);
			INTERPOL_BILINEAR_SSE(s_00_01, s_10_11, frac_w, v_frac_h0, v_frac_h1, dst, zero);
			dst += 1;
		}

		while (right_pad_w--) {
			int index_w = 4 * (src_w - 2);
			const Uint32 *s_00_01 = (const Uint32 *)((const Uint8 *)src_h0 + index_w);
			const Uint32 *s_10_11 = (const Uint32 *)((const Uint8 *)src_h1 + index_w);
			INTERPOL_BILINEAR_SSE(s_00_01, s_10_11, FRAC_ONE, v_frac_h0, v_frac_h1, dst, zero);
			dst += 1;
		}
		dst = (Uint32 *)((Uint8 *)dst + dst_gap);
	}
	return 0;
}
#endif // HAVE_SSE2_INTRINSICS

#if defined(HAVE_NEON_INTRINSICS)

static SDL_INLINE int hasNEON(void) {
	static int val = -1;
	if (val != -1) {
		return val;
	}
	val = SDL_HasNEON();
	return val;
}

static SDL_INLINE void INTERPOL_BILINEAR_NEON(const Uint32 *s0, const Uint32 *s1, int frac_w, uint8x8_t v_frac_h0, uint8x8_t v_frac_h1, Uint32 *dst) {
	uint8x8_t x_00_01, x_10_11; /* Pixels in 4*uint8 in row */
	uint16x8_t k0;
	uint32x4_t l0;
	uint16x8_t d0;
	uint8x8_t e0;

	x_00_01 = CAST_uint8x8_t vld1_u32(s0); /* Load 2 pixels */
	x_10_11 = CAST_uint8x8_t vld1_u32(s1);

	/* Interpolated == x0 + frac * (x1 - x0) == x0 * (1 - frac) + x1 * frac */
	k0 = vmull_u8(x_00_01, v_frac_h1); /* k0 := x0 * (1 - frac)    */
	k0 = vmlal_u8(k0, x_10_11, v_frac_h0); /* k0 += x1 * frac          */

	/* k0 now contains 2 interpolated pixels { j0, j1 } */
	l0 = vshll_n_u16(vget_low_u16(k0), PRECISION);
	l0 = vmlsl_n_u16(l0, vget_low_u16(k0), frac_w);
	l0 = vmlal_n_u16(l0, vget_high_u16(k0), frac_w);

	/* Shift and narrow */
	d0 = vcombine_u16(
			/* uint16x4_t */ vshrn_n_u32(l0, 2 * PRECISION),
			/* uint16x4_t */ vshrn_n_u32(l0, 2 * PRECISION));

	/* Narrow again */
	e0 = vmovn_u16(d0);

	/* Store 1 pixel */
	*dst = vget_lane_u32(CAST_uint32x2_t e0, 0);
}

static int scale_mat_NEON(const Uint32 *src, int src_w, int src_h, int src_pitch, Uint32 *dst, int dst_w, int dst_h, int dst_pitch) {
	BILINEAR___START

	for (int i = 0; i < dst_h; i++) {
		int nb_block4;
		uint8x8_t v_frac_h0, v_frac_h1;

		BILINEAR___HEIGHT

		nb_block4 = middle / 4;

		v_frac_h0 = vmov_n_u8(frac_h0);
		v_frac_h1 = vmov_n_u8(frac_h1);

		while (left_pad_w--) {
			INTERPOL_BILINEAR_NEON(src_h0, src_h1, FRAC_ZERO, v_frac_h0, v_frac_h1, dst);
			dst += 1;
		}

		while (nb_block4--) {
			int index_w_0, frac_w_0;
			int index_w_1, frac_w_1;
			int index_w_2, frac_w_2;
			int index_w_3, frac_w_3;

			const Uint32 *s_00_01, *s_02_03, *s_04_05, *s_06_07;
			const Uint32 *s_10_11, *s_12_13, *s_14_15, *s_16_17;

			uint8x8_t x_00_01, x_10_11, x_02_03, x_12_13; /* Pixels in 4*uint8 in row */
			uint8x8_t x_04_05, x_14_15, x_06_07, x_16_17;

			uint16x8_t k0, k1, k2, k3;
			uint32x4_t l0, l1, l2, l3;
			uint16x8_t d0, d1;
			uint8x8_t e0, e1;
			uint32x4_t f0;

			index_w_0 = 4 * SRC_INDEX(fp_sum_w);
			frac_w_0 = FRAC(fp_sum_w);
			fp_sum_w += fp_step_w;
			index_w_1 = 4 * SRC_INDEX(fp_sum_w);
			frac_w_1 = FRAC(fp_sum_w);
			fp_sum_w += fp_step_w;
			index_w_2 = 4 * SRC_INDEX(fp_sum_w);
			frac_w_2 = FRAC(fp_sum_w);
			fp_sum_w += fp_step_w;
			index_w_3 = 4 * SRC_INDEX(fp_sum_w);
			frac_w_3 = FRAC(fp_sum_w);
			fp_sum_w += fp_step_w;

			s_00_01 = (const Uint32 *)((const Uint8 *)src_h0 + index_w_0);
			s_02_03 = (const Uint32 *)((const Uint8 *)src_h0 + index_w_1);
			s_04_05 = (const Uint32 *)((const Uint8 *)src_h0 + index_w_2);
			s_06_07 = (const Uint32 *)((const Uint8 *)src_h0 + index_w_3);
			s_10_11 = (const Uint32 *)((const Uint8 *)src_h1 + index_w_0);
			s_12_13 = (const Uint32 *)((const Uint8 *)src_h1 + index_w_1);
			s_14_15 = (const Uint32 *)((const Uint8 *)src_h1 + index_w_2);
			s_16_17 = (const Uint32 *)((const Uint8 *)src_h1 + index_w_3);

			/* Interpolation vertical */
			x_00_01 = CAST_uint8x8_t vld1_u32(s_00_01); /* Load 2 pixels */
			x_02_03 = CAST_uint8x8_t vld1_u32(s_02_03);
			x_04_05 = CAST_uint8x8_t vld1_u32(s_04_05);
			x_06_07 = CAST_uint8x8_t vld1_u32(s_06_07);
			x_10_11 = CAST_uint8x8_t vld1_u32(s_10_11);
			x_12_13 = CAST_uint8x8_t vld1_u32(s_12_13);
			x_14_15 = CAST_uint8x8_t vld1_u32(s_14_15);
			x_16_17 = CAST_uint8x8_t vld1_u32(s_16_17);

			/* Interpolated == x0 + frac * (x1 - x0) == x0 * (1 - frac) + x1 * frac */
			k0 = vmull_u8(x_00_01, v_frac_h1); /* k0 := x0 * (1 - frac)    */
			k0 = vmlal_u8(k0, x_10_11, v_frac_h0); /* k0 += x1 * frac          */

			k1 = vmull_u8(x_02_03, v_frac_h1);
			k1 = vmlal_u8(k1, x_12_13, v_frac_h0);

			k2 = vmull_u8(x_04_05, v_frac_h1);
			k2 = vmlal_u8(k2, x_14_15, v_frac_h0);

			k3 = vmull_u8(x_06_07, v_frac_h1);
			k3 = vmlal_u8(k3, x_16_17, v_frac_h0);

			/* k0 now contains 2 interpolated pixels { j0, j1 } */
			/* k1 now contains 2 interpolated pixels { j2, j3 } */
			/* k2 now contains 2 interpolated pixels { j4, j5 } */
			/* k3 now contains 2 interpolated pixels { j6, j7 } */

			l0 = vshll_n_u16(vget_low_u16(k0), PRECISION);
			l0 = vmlsl_n_u16(l0, vget_low_u16(k0), frac_w_0);
			l0 = vmlal_n_u16(l0, vget_high_u16(k0), frac_w_0);

			l1 = vshll_n_u16(vget_low_u16(k1), PRECISION);
			l1 = vmlsl_n_u16(l1, vget_low_u16(k1), frac_w_1);
			l1 = vmlal_n_u16(l1, vget_high_u16(k1), frac_w_1);

			l2 = vshll_n_u16(vget_low_u16(k2), PRECISION);
			l2 = vmlsl_n_u16(l2, vget_low_u16(k2), frac_w_2);
			l2 = vmlal_n_u16(l2, vget_high_u16(k2), frac_w_2);

			l3 = vshll_n_u16(vget_low_u16(k3), PRECISION);
			l3 = vmlsl_n_u16(l3, vget_low_u16(k3), frac_w_3);
			l3 = vmlal_n_u16(l3, vget_high_u16(k3), frac_w_3);

			/* shift and narrow */
			d0 = vcombine_u16(
					/* uint16x4_t */ vshrn_n_u32(l0, 2 * PRECISION),
					/* uint16x4_t */ vshrn_n_u32(l1, 2 * PRECISION));
			/* narrow again */
			e0 = vmovn_u16(d0);

			/* Shift and narrow */
			d1 = vcombine_u16(
					/* uint16x4_t */ vshrn_n_u32(l2, 2 * PRECISION),
					/* uint16x4_t */ vshrn_n_u32(l3, 2 * PRECISION));
			/* Narrow again */
			e1 = vmovn_u16(d1);

			f0 = vcombine_u32(CAST_uint32x2_t e0, CAST_uint32x2_t e1);
			/* Store 4 pixels */
			vst1q_u32(dst, f0);

			dst += 4;
		}

		if (middle & 0x2) {
			int index_w_0, frac_w_0;
			int index_w_1, frac_w_1;
			const Uint32 *s_00_01, *s_02_03;
			const Uint32 *s_10_11, *s_12_13;
			uint8x8_t x_00_01, x_10_11, x_02_03, x_12_13; /* Pixels in 4*uint8 in row */
			uint16x8_t k0, k1;
			uint32x4_t l0, l1;
			uint16x8_t d0;
			uint8x8_t e0;

			index_w_0 = 4 * SRC_INDEX(fp_sum_w);
			frac_w_0 = FRAC(fp_sum_w);
			fp_sum_w += fp_step_w;
			index_w_1 = 4 * SRC_INDEX(fp_sum_w);
			frac_w_1 = FRAC(fp_sum_w);
			fp_sum_w += fp_step_w;
			// x00............ x01   x02...........x03
			// .      .         .     .       .     .
			// j0   dest0       j1    j2    dest1   j3
			// .      .         .     .       .     .
			// .      .         .     .       .     .
			// .      .         .     .       .     .
			// x10............ x11   x12...........x13
			s_00_01 = (const Uint32 *)((const Uint8 *)src_h0 + index_w_0);
			s_02_03 = (const Uint32 *)((const Uint8 *)src_h0 + index_w_1);
			s_10_11 = (const Uint32 *)((const Uint8 *)src_h1 + index_w_0);
			s_12_13 = (const Uint32 *)((const Uint8 *)src_h1 + index_w_1);

			/* Interpolation vertical */
			x_00_01 = CAST_uint8x8_t vld1_u32(s_00_01); /* Load 2 pixels */
			x_02_03 = CAST_uint8x8_t vld1_u32(s_02_03);
			x_10_11 = CAST_uint8x8_t vld1_u32(s_10_11);
			x_12_13 = CAST_uint8x8_t vld1_u32(s_12_13);

			/* Interpolated == x0 + frac * (x1 - x0) == x0 * (1 - frac) + x1 * frac */
			k0 = vmull_u8(x_00_01, v_frac_h1); /* k0 := x0 * (1 - frac)    */
			k0 = vmlal_u8(k0, x_10_11, v_frac_h0); /* k0 += x1 * frac          */

			k1 = vmull_u8(x_02_03, v_frac_h1);
			k1 = vmlal_u8(k1, x_12_13, v_frac_h0);

			/* k0 now contains 2 interpolated pixels { j0, j1 } */
			/* k1 now contains 2 interpolated pixels { j2, j3 } */

			l0 = vshll_n_u16(vget_low_u16(k0), PRECISION);
			l0 = vmlsl_n_u16(l0, vget_low_u16(k0), frac_w_0);
			l0 = vmlal_n_u16(l0, vget_high_u16(k0), frac_w_0);

			l1 = vshll_n_u16(vget_low_u16(k1), PRECISION);
			l1 = vmlsl_n_u16(l1, vget_low_u16(k1), frac_w_1);
			l1 = vmlal_n_u16(l1, vget_high_u16(k1), frac_w_1);

			/* Shift and narrow */

			d0 = vcombine_u16(
					/* uint16x4_t */ vshrn_n_u32(l0, 2 * PRECISION),
					/* uint16x4_t */ vshrn_n_u32(l1, 2 * PRECISION));

			/* Narrow again */
			e0 = vmovn_u16(d0);

			/* Store 2 pixels */
			vst1_u32(dst, CAST_uint32x2_t e0);
			dst += 2;
		}

		/* Last point */
		if (middle & 0x1) {
			int index_w = 4 * SRC_INDEX(fp_sum_w);
			int frac_w = FRAC(fp_sum_w);
			const Uint32 *s_00_01 = (const Uint32 *)((const Uint8 *)src_h0 + index_w);
			const Uint32 *s_10_11 = (const Uint32 *)((const Uint8 *)src_h1 + index_w);
			INTERPOL_BILINEAR_NEON(s_00_01, s_10_11, frac_w, v_frac_h0, v_frac_h1, dst);
			dst += 1;
		}

		while (right_pad_w--) {
			int index_w = 4 * (src_w - 2);
			const Uint32 *s_00_01 = (const Uint32 *)((const Uint8 *)src_h0 + index_w);
			const Uint32 *s_10_11 = (const Uint32 *)((const Uint8 *)src_h1 + index_w);
			INTERPOL_BILINEAR_NEON(s_00_01, s_10_11, FRAC_ONE, v_frac_h0, v_frac_h1, dst);
			dst += 1;
		}

		dst = (Uint32 *)((Uint8 *)dst + dst_gap);
	}
	return 0;
}
#endif // HAVE_NEON_INTRINSICS

int SDL_LowerSoftStretchLinear(SDL_Surface *s, const SDL_Rect *srcrect, SDL_Surface *d, const SDL_Rect *dstrect) {
	int ret = -1;
	int src_w = srcrect->w;
	int src_h = srcrect->h;
	int dst_w = dstrect->w;
	int dst_h = dstrect->h;
	int src_pitch = s->pitch;
	int dst_pitch = d->pitch;
	Uint32 *src = (Uint32 *)((Uint8 *)s->pixels + srcrect->x * 4 + srcrect->y * src_pitch);
	Uint32 *dst = (Uint32 *)((Uint8 *)d->pixels + dstrect->x * 4 + dstrect->y * dst_pitch);

#if defined(HAVE_NEON_INTRINSICS)
	if (ret == -1 && hasNEON()) {
		ret = scale_mat_NEON(src, src_w, src_h, src_pitch, dst, dst_w, dst_h, dst_pitch);
	}
#endif // HAVE_NEON_INTRINSICS

#if defined(HAVE_SSE2_INTRINSICS)
	if (ret == -1 && hasSSE2()) {
		ret = scale_mat_SSE(src, src_w, src_h, src_pitch, dst, dst_w, dst_h, dst_pitch);
	}
#endif // HAVE_SSE2_INTRINSICS

	if (ret == -1) {
		ret = scale_mat(src, src_w, src_h, src_pitch, dst, dst_w, dst_h, dst_pitch);
	}

	return ret;
}

#define SDL_SCALE_NEAREST__START          \
	Uint64 posy, incy;                    \
	Uint64 posx, incx;                    \
	Uint64 srcy, srcx;                    \
	int dst_gap, n;                       \
	const Uint32 *src_h0;                 \
	incy = ((Uint64)src_h << 16) / dst_h; \
	incx = ((Uint64)src_w << 16) / dst_w; \
	dst_gap = dst_pitch - bpp * dst_w;    \
	posy = incy / 2;

#define SDL_SCALE_NEAREST__HEIGHT                                         \
	srcy = (posy >> 16);                                                  \
	src_h0 = (const Uint32 *)((const Uint8 *)src_ptr + srcy * src_pitch); \
	posy += incy;                                                         \
	posx = incx / 2;                                                      \
	n = dst_w;

static int scale_mat_nearest_1(const Uint32 *src_ptr, int src_w, int src_h, int src_pitch,
		Uint32 *dst, int dst_w, int dst_h, int dst_pitch) {
	Uint32 bpp = 1;
	SDL_SCALE_NEAREST__START
	for (int i = 0; i < dst_h; i++) {
		SDL_SCALE_NEAREST__HEIGHT
		while (n--) {
			const Uint8 *src;
			srcx = bpp * (posx >> 16);
			posx += incx;
			src = (const Uint8 *)src_h0 + srcx;
			*(Uint8 *)dst = *src;
			dst = (Uint32 *)((Uint8 *)dst + bpp);
		}
		dst = (Uint32 *)((Uint8 *)dst + dst_gap);
	}
	return 0;
}

static int scale_mat_nearest_2(const Uint32 *src_ptr, int src_w, int src_h, int src_pitch, Uint32 *dst, int dst_w, int dst_h, int dst_pitch) {
	Uint32 bpp = 2;
	SDL_SCALE_NEAREST__START
	for (int i = 0; i < dst_h; i++) {
		SDL_SCALE_NEAREST__HEIGHT
		while (n--) {
			const Uint16 *src;
			srcx = bpp * (posx >> 16);
			posx += incx;
			src = (const Uint16 *)((const Uint8 *)src_h0 + srcx);
			*(Uint16 *)dst = *src;
			dst = (Uint32 *)((Uint8 *)dst + bpp);
		}
		dst = (Uint32 *)((Uint8 *)dst + dst_gap);
	}
	return 0;
}

static int scale_mat_nearest_3(const Uint32 *src_ptr, int src_w, int src_h, int src_pitch, Uint32 *dst, int dst_w, int dst_h, int dst_pitch) {
	Uint32 bpp = 3;
	SDL_SCALE_NEAREST__START
	for (int i = 0; i < dst_h; i++) {
		SDL_SCALE_NEAREST__HEIGHT
		while (n--) {
			const Uint8 *src;
			srcx = bpp * (posx >> 16);
			posx += incx;
			src = (const Uint8 *)src_h0 + srcx;
			((Uint8 *)dst)[0] = src[0];
			((Uint8 *)dst)[1] = src[1];
			((Uint8 *)dst)[2] = src[2];
			dst = (Uint32 *)((Uint8 *)dst + bpp);
		}
		dst = (Uint32 *)((Uint8 *)dst + dst_gap);
	}
	return 0;
}

static int scale_mat_nearest_4(const Uint32 *src_ptr, int src_w, int src_h, int src_pitch, Uint32 *dst, int dst_w, int dst_h, int dst_pitch) {
	Uint32 bpp = 4;
	SDL_SCALE_NEAREST__START
	for (int i = 0; i < dst_h; i++) {
		SDL_SCALE_NEAREST__HEIGHT
		while (n--) {
			const Uint32 *src;
			srcx = bpp * (posx >> 16);
			posx += incx;
			src = (const Uint32 *)((const Uint8 *)src_h0 + srcx);
			*dst = *src;
			dst = (Uint32 *)((Uint8 *)dst + bpp);
		}
		dst = (Uint32 *)((Uint8 *)dst + dst_gap);
	}
	return 0;
}

int SDL_LowerSoftStretchNearest(SDL_Surface *s, const SDL_Rect *srcrect, SDL_Surface *d, const SDL_Rect *dstrect) {
	int src_w = srcrect->w;
	int src_h = srcrect->h;
	int dst_w = dstrect->w;
	int dst_h = dstrect->h;
	int src_pitch = s->pitch;
	int dst_pitch = d->pitch;

	const int bpp = d->format->BytesPerPixel;

	Uint32 *src = (Uint32 *)((Uint8 *)s->pixels + srcrect->x * bpp + srcrect->y * src_pitch);
	Uint32 *dst = (Uint32 *)((Uint8 *)d->pixels + dstrect->x * bpp + dstrect->y * dst_pitch);

	if (bpp == 4) {
		return scale_mat_nearest_4(src, src_w, src_h, src_pitch, dst, dst_w, dst_h, dst_pitch);
	} else if (bpp == 3) {
		return scale_mat_nearest_3(src, src_w, src_h, src_pitch, dst, dst_w, dst_h, dst_pitch);
	} else if (bpp == 2) {
		return scale_mat_nearest_2(src, src_w, src_h, src_pitch, dst, dst_w, dst_h, dst_pitch);
	} else {
		return scale_mat_nearest_1(src, src_w, src_h, src_pitch, dst, dst_w, dst_h, dst_pitch);
	}
}

/// END Stretch
