/**************************************************************************/
/*  _render.c                                                             */
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

#include "_render.h"
#include "_private.h"
#include "_surface.h"
#include "blit.h"

static int Render_Copy(SDL_Surface *surface, SDL_Surface *texture,
		const SDL_Rect *srcrect, SDL_Rect *final_rect,
		const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip, float scale_x, float scale_y);

static int Blit_to_Screen(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *surface, SDL_Rect *dstrect, float scale_x, float scale_y, SDL_ScaleMode scaleMode);

int SDL_RenderCopy(SDL_Surface *surface, SDL_Surface *texture, const SDL_Rect *srcrect, SDL_Rect *dstrect) {
	SDL_Rect real_srcrect;
	SDL_Rect real_dstrect;
	int retval = 0;

#if SW_DONT_DRAW_WHILE_HIDDEN
	/* Don't draw while we're hidden */
	if (renderer->hidden) {
		return 0;
	}
#endif

	real_srcrect.x = 0;
	real_srcrect.y = 0;
	real_srcrect.w = texture->w;
	real_srcrect.h = texture->h;
	if (srcrect) {
		if (!SDL_IntersectRect(srcrect, &real_srcrect, &real_srcrect)) {
			return 0;
		}
	}

	real_dstrect.x = 0;
	real_dstrect.y = 0;
	real_dstrect.w = surface->w;
	real_dstrect.h = surface->h;
	if (dstrect) {
		if (!SDL_HasIntersection(dstrect, &real_dstrect)) {
			return 0;
		}
		real_dstrect = *dstrect;
	}

	if (srcrect->w == dstrect->w && srcrect->h == dstrect->h) {
		SDL_BlitSurface(texture, srcrect, surface, dstrect);
	} else {
		/* If scaling is ever done, permanently disable RLE (which doesn't support scaling)
		 * to avoid potentially frequent RLE encoding/decoding. */
		SDL_SetSurfaceRLE(surface, 0);
		SDL_ScaleMode scaleMode = (texture->map->info.flags & SDL_COPY_NEAREST) ? SDL_ScaleModeNearest : SDL_ScaleModeLinear;
		/* Prevent to do scaling + clipping on viewport boundaries as it may lose proportion */
		if (dstrect->x < 0 || dstrect->y < 0 || dstrect->x + dstrect->w > surface->w || dstrect->y + dstrect->h > surface->h) {
			SDL_Surface *tmp = SDL_CreateRGBSurfaceWithFormat(dstrect->w, dstrect->h, 0, texture->format->format);
			/* Scale to an intermediate surface, then blit */
			if (tmp) {
				SDL_Rect r;
				SDL_BlendMode blendmode;
				Uint8 alphaMod, rMod, gMod, bMod;

				SDL_GetSurfaceBlendMode(texture, &blendmode);
				SDL_GetSurfaceAlphaMod(texture, &alphaMod);
				SDL_GetSurfaceColorMod(texture, &rMod, &gMod, &bMod);

				r.x = 0;
				r.y = 0;
				r.w = dstrect->w;
				r.h = dstrect->h;

				SDL_SetSurfaceBlendMode(texture, SDL_BLENDMODE_NONE);
				SDL_SetSurfaceColorMod(texture, 255, 255, 255);
				SDL_SetSurfaceAlphaMod(texture, 255);

				SDL_PrivateUpperBlitScaled(texture, srcrect, tmp, &r, scaleMode);

				SDL_SetSurfaceColorMod(tmp, rMod, gMod, bMod);
				SDL_SetSurfaceAlphaMod(tmp, alphaMod);
				SDL_SetSurfaceBlendMode(tmp, blendmode);

				SDL_BlitSurface(tmp, NULL, surface, dstrect);
				SDL_FreeSurface(tmp);
			}
		} else {
			retval |= SDL_PrivateUpperBlitScaled(texture, srcrect, surface, dstrect, scaleMode);
		}
	}
	return retval;
}

int SDL_RenderCopyEx(SDL_Surface *surface, SDL_Surface *texture, const SDL_Rect *srcrect, SDL_Rect *dstrect, const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip) {
	SDL_Rect real_srcrect;
	SDL_Rect real_dstrect;
	SDL_FPoint real_center;
	int retval = 0;

	if (flip == SDL_FLIP_NONE && (int)(angle / 360) == angle / 360) { /* fast path when we don't need rotation or flipping */
		return SDL_RenderCopy(surface, texture, srcrect, dstrect);
	}

#if SW_DONT_DRAW_WHILE_HIDDEN
	/* Don't draw while we're hidden */
	if (renderer->hidden) {
		return 0;
	}
#endif

	real_srcrect.x = 0;
	real_srcrect.y = 0;
	real_srcrect.w = texture->w;
	real_srcrect.h = texture->h;
	if (srcrect) {
		if (!SDL_IntersectRect(srcrect, &real_srcrect, &real_srcrect)) {
			return 0;
		}
	}

	/* We don't intersect the dstrect with the viewport as RenderCopy does because of potential rotation clipping issues... TODO: should we? */
	if (dstrect) {
		real_dstrect = *dstrect;
	} else {
		real_dstrect.x = 0;
		real_dstrect.y = 0;
		real_dstrect.w = surface->w;
		real_dstrect.h = surface->h;
	}

	if (center) {
		real_center = *center;
	} else {
		real_center.x = real_dstrect.w / 2.0f;
		real_center.y = real_dstrect.h / 2.0f;
	}

	retval |= Render_Copy(surface, texture, &real_srcrect, &real_dstrect, angle, &real_center, flip, 1, 1);

	return retval;
}

// Internal functions

static int Blit_to_Screen(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *surface, SDL_Rect *dstrect, float scale_x, float scale_y, SDL_ScaleMode scaleMode) {
	int retval;
	/* Renderer scaling, if needed */
	if (scale_x != 1 || scale_y != 1) {
		SDL_Rect r;
		r.x = (int)((float)dstrect->x * scale_x);
		r.y = (int)((float)dstrect->y * scale_y);
		r.w = (int)((float)dstrect->w * scale_x);
		r.h = (int)((float)dstrect->h * scale_y);
		retval = SDL_PrivateUpperBlitScaled(src, srcrect, surface, &r, scaleMode);
	} else {
		retval = SDL_BlitSurface(src, srcrect, surface, dstrect);
	}
	return retval;
}

static int Render_Copy(SDL_Surface *surface, SDL_Surface *texture,
		const SDL_Rect *srcrect, SDL_Rect *final_rect,
		const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip, float scale_x, float scale_y) {
	SDL_Rect tmp_rect;
	SDL_Surface *src_clone, *src_rotated, *src_scaled;
	SDL_Surface *mask = NULL, *mask_rotated = NULL;
	int retval = 0;
	SDL_BlendMode blendmode;
	Uint8 alphaMod, rMod, gMod, bMod;
	int applyModulation = SDL_FALSE;
	int blitRequired = SDL_FALSE;
	int isOpaque = SDL_FALSE;

	if (!surface) {
		return -1;
	}

	tmp_rect.x = 0;
	tmp_rect.y = 0;
	tmp_rect.w = final_rect->w;
	tmp_rect.h = final_rect->h;

	/* Clone the source surface but use its pixel buffer directly.
	 * The original source surface must be treated as read-only.
	 */
	src_clone = SDL_CreateRGBSurfaceFrom(texture->pixels, texture->w, texture->h, texture->format->BitsPerPixel, texture->pitch,
			texture->format->Rmask, texture->format->Gmask,
			texture->format->Bmask, texture->format->Amask);
	if (!src_clone) {
		return -1;
	}

	SDL_GetSurfaceBlendMode(texture, &blendmode);
	SDL_GetSurfaceAlphaMod(texture, &alphaMod);
	SDL_GetSurfaceColorMod(texture, &rMod, &gMod, &bMod);

	/* SDLgfx_rotateSurface only accepts 32-bit surfaces with a 8888 layout. Everything else has to be converted. */
	if (texture->format->BitsPerPixel != 32 || SDL_PIXELLAYOUT(texture->format->format) != SDL_PACKEDLAYOUT_8888 || !texture->format->Amask) {
		blitRequired = SDL_TRUE;
	}

	/* If scaling and cropping is necessary, it has to be taken care of before the rotation. */
	if (!(srcrect->w == final_rect->w && srcrect->h == final_rect->h && srcrect->x == 0 && srcrect->y == 0)) {
		blitRequired = SDL_TRUE;
	}

	/* srcrect is not selecting the whole texture surface, so cropping is needed */
	if (!(srcrect->w == texture->w && srcrect->h == texture->h && srcrect->x == 0 && srcrect->y == 0)) {
		blitRequired = SDL_TRUE;
	}

	/* The color and alpha modulation has to be applied before the rotation when using the NONE, MOD or MUL blend modes. */
	if ((blendmode == SDL_BLENDMODE_NONE || blendmode == SDL_BLENDMODE_MOD || blendmode == SDL_BLENDMODE_MUL) && (alphaMod & rMod & gMod & bMod) != 255) {
		applyModulation = SDL_TRUE;
		SDL_SetSurfaceAlphaMod(src_clone, alphaMod);
		SDL_SetSurfaceColorMod(src_clone, rMod, gMod, bMod);
	}

	/* Opaque surfaces are much easier to handle with the NONE blend mode. */
	if (blendmode == SDL_BLENDMODE_NONE && !texture->format->Amask && alphaMod == 255) {
		isOpaque = SDL_TRUE;
	}

	/* The NONE blend mode requires a mask for non-opaque surfaces. This mask will be used
	 * to clear the pixels in the destination surface. The other steps are explained below.
	 */
	if (blendmode == SDL_BLENDMODE_NONE && !isOpaque) {
		mask = SDL_CreateRGBSurface(final_rect->w, final_rect->h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
		if (!mask) {
			retval = -1;
		} else {
			SDL_SetSurfaceBlendMode(mask, SDL_BLENDMODE_MOD);
		}
	}

	SDL_ScaleMode scaleMode = (texture->map->info.flags & SDL_COPY_NEAREST) ? SDL_ScaleModeNearest : SDL_ScaleModeLinear;

	/* Create a new surface should there be a format mismatch or if scaling, cropping,
	 * or modulation is required. It's possible to use the source surface directly otherwise.
	 */
	if (!retval && (blitRequired || applyModulation)) {
		SDL_Rect scale_rect = tmp_rect;
		src_scaled = SDL_CreateRGBSurface(final_rect->w, final_rect->h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
		if (!src_scaled) {
			retval = -1;
		} else {
			SDL_SetSurfaceBlendMode(src_clone, SDL_BLENDMODE_NONE);
			retval = SDL_PrivateUpperBlitScaled(src_clone, srcrect, src_scaled, &scale_rect, scaleMode);
			SDL_FreeSurface(src_clone);
			src_clone = src_scaled;
			src_scaled = NULL;
		}
	}

	/* SDLgfx_rotateSurface is going to make decisions depending on the blend mode. */
	SDL_SetSurfaceBlendMode(src_clone, blendmode);

	if (!retval) {
		SDL_Rect rect_dest;
		double cangle, sangle;

		SDLgfx_rotozoomSurfaceSizeTrig(tmp_rect.w, tmp_rect.h, angle, center, &rect_dest, &cangle, &sangle);
		src_rotated = SDLgfx_rotateSurface(src_clone, angle,
				(scaleMode == SDL_ScaleModeNearest) ? 0 : 1, flip & SDL_FLIP_HORIZONTAL, flip & SDL_FLIP_VERTICAL,
				&rect_dest, cangle, sangle, center);
		if (!src_rotated) {
			retval = -1;
		}
		if (!retval && mask) {
			/* The mask needed for the NONE blend mode gets rotated with the same parameters. */
			mask_rotated = SDLgfx_rotateSurface(mask, angle,
					SDL_FALSE, 0, 0,
					&rect_dest, cangle, sangle, center);
			if (!mask_rotated) {
				retval = -1;
			}
		}
		if (!retval) {
			tmp_rect.x = final_rect->x + rect_dest.x;
			tmp_rect.y = final_rect->y + rect_dest.y;
			tmp_rect.w = rect_dest.w;
			tmp_rect.h = rect_dest.h;

			/* The NONE blend mode needs some special care with non-opaque surfaces.
			 * Other blend modes or opaque surfaces can be blitted directly.
			 */
			if (blendmode != SDL_BLENDMODE_NONE || isOpaque) {
				if (applyModulation == SDL_FALSE) {
					/* If the modulation wasn't already applied, make it happen now. */
					SDL_SetSurfaceAlphaMod(src_rotated, alphaMod);
					SDL_SetSurfaceColorMod(src_rotated, rMod, gMod, bMod);
				}
				/* Renderer scaling, if needed */
				retval = Blit_to_Screen(src_rotated, NULL, surface, &tmp_rect, scale_x, scale_y, scaleMode);
			} else {
				/* The NONE blend mode requires three steps to get the pixels onto the destination surface.
				 * First, the area where the rotated pixels will be blitted to get set to zero.
				 * This is accomplished by simply blitting a mask with the NONE blend mode.
				 * The colorkey set by the rotate function will discard the correct pixels.
				 */
				SDL_Rect mask_rect = tmp_rect;
				SDL_SetSurfaceBlendMode(mask_rotated, SDL_BLENDMODE_NONE);
				/* Renderer scaling, if needed */
				retval = Blit_to_Screen(mask_rotated, NULL, surface, &mask_rect, scale_x, scale_y, scaleMode);
				if (!retval) {
					/* The next step copies the alpha value. This is done with the BLEND blend mode and
					 * by modulating the source colors with 0. Since the destination is all zeros, this
					 * will effectively set the destination alpha to the source alpha.
					 */
					SDL_SetSurfaceColorMod(src_rotated, 0, 0, 0);
					mask_rect = tmp_rect;
					/* Renderer scaling, if needed */
					retval = Blit_to_Screen(src_rotated, NULL, surface, &mask_rect, scale_x, scale_y, scaleMode);
					if (!retval) {
						/* The last step gets the color values in place. The ADD blend mode simply adds them to
						 * the destination (where the color values are all zero). However, because the ADD blend
						 * mode modulates the colors with the alpha channel, a surface without an alpha mask needs
						 * to be created. This makes all source pixels opaque and the colors get copied correctly.
						 */
						SDL_Surface *src_rotated_rgb;
						src_rotated_rgb = SDL_CreateRGBSurfaceFrom(src_rotated->pixels, src_rotated->w, src_rotated->h,
								src_rotated->format->BitsPerPixel, src_rotated->pitch,
								src_rotated->format->Rmask, src_rotated->format->Gmask,
								src_rotated->format->Bmask, 0);
						if (!src_rotated_rgb) {
							retval = -1;
						} else {
							SDL_SetSurfaceBlendMode(src_rotated_rgb, SDL_BLENDMODE_ADD);
							/* Renderer scaling, if needed */
							retval = Blit_to_Screen(src_rotated_rgb, NULL, surface, &tmp_rect, scale_x, scale_y, scaleMode);
							SDL_FreeSurface(src_rotated_rgb);
						}
					}
				}
				SDL_FreeSurface(mask_rotated);
			}
			if (src_rotated) {
				SDL_FreeSurface(src_rotated);
			}
		}
	}
	if (mask) {
		SDL_FreeSurface(mask);
	}
	if (src_clone) {
		SDL_FreeSurface(src_clone);
	}
	return retval;
}
