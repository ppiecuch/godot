#include "_draw.h"
#include "_internal.h"

#include "drawshape.h"

/// Points

int SDL_DrawPoint(SDL_Surface *dst, int x, int y, Uint32 color) {
	if (!dst) {
		return SDL_InvalidParamError("SDL_DrawPoint(): dst");
	}

	/* This function doesn't work on surfaces < 8 bpp */
	if (dst->format->BitsPerPixel < 8) {
		return SDL_SetError("SDL_DrawPoint(): Unsupported surface format");
	}

	/* Perform clipping */
	if (x < dst->clip_rect.x || y < dst->clip_rect.y ||
			x >= (dst->clip_rect.x + dst->clip_rect.w) ||
			y >= (dst->clip_rect.y + dst->clip_rect.h)) {
		return 0;
	}

	switch (dst->format->BytesPerPixel) {
		case 1:
			DRAW_FASTSETPIXELXY1(x, y);
			break;
		case 2:
			DRAW_FASTSETPIXELXY2(x, y);
			break;
		case 3:
			return SDL_Unsupported();
		case 4:
			DRAW_FASTSETPIXELXY4(x, y);
			break;
	}
	return 0;
}

int SDL_DrawPoints(SDL_Surface *dst, const SDL_Point *points, int count, Uint32 color) {
	int minx, miny;
	int maxx, maxy;

	if (!dst) {
		return SDL_InvalidParamError("SDL_DrawPoints(): dst");
	}

	/* This function doesn't work on surfaces < 8 bpp */
	if (dst->format->BitsPerPixel < 8) {
		return SDL_SetError("SDL_DrawPoints(): Unsupported surface format");
	}

	minx = dst->clip_rect.x;
	maxx = dst->clip_rect.x + dst->clip_rect.w - 1;
	miny = dst->clip_rect.y;
	maxy = dst->clip_rect.y + dst->clip_rect.h - 1;

	for (int i = 0; i < count; ++i) {
		int x = points[i].x;
		int y = points[i].y;

		if (x < minx || x > maxx || y < miny || y > maxy) {
			continue;
		}

		switch (dst->format->BytesPerPixel) {
			case 1:
				DRAW_FASTSETPIXELXY1(x, y);
				break;
			case 2:
				DRAW_FASTSETPIXELXY2(x, y);
				break;
			case 3:
				return SDL_Unsupported();
			case 4:
				DRAW_FASTSETPIXELXY4(x, y);
				break;
		}
	}
	return 0;
}

static int SDL_BlendPoint_RGB555(SDL_Surface *dst, int x, int y, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	unsigned inva = 0xff - a;

	switch (blendMode) {
		case SDL_BLENDMODE_BLEND:
			DRAW_SETPIXELXY_BLEND_RGB555(x, y);
			break;
		case SDL_BLENDMODE_ADD:
			DRAW_SETPIXELXY_ADD_RGB555(x, y);
			break;
		case SDL_BLENDMODE_MOD:
			DRAW_SETPIXELXY_MOD_RGB555(x, y);
			break;
		case SDL_BLENDMODE_MUL:
			DRAW_SETPIXELXY_MUL_RGB555(x, y);
			break;
		default:
			DRAW_SETPIXELXY_RGB555(x, y);
			break;
	}
	return 0;
}

static int SDL_BlendPoint_RGB565(SDL_Surface *dst, int x, int y, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	unsigned inva = 0xff - a;

	switch (blendMode) {
		case SDL_BLENDMODE_BLEND:
			DRAW_SETPIXELXY_BLEND_RGB565(x, y);
			break;
		case SDL_BLENDMODE_ADD:
			DRAW_SETPIXELXY_ADD_RGB565(x, y);
			break;
		case SDL_BLENDMODE_MOD:
			DRAW_SETPIXELXY_MOD_RGB565(x, y);
			break;
		case SDL_BLENDMODE_MUL:
			DRAW_SETPIXELXY_MUL_RGB565(x, y);
			break;
		default:
			DRAW_SETPIXELXY_RGB565(x, y);
			break;
	}
	return 0;
}

static int SDL_BlendPoint_RGB888(SDL_Surface *dst, int x, int y, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	unsigned inva = 0xff - a;

	switch (blendMode) {
		case SDL_BLENDMODE_BLEND:
			DRAW_SETPIXELXY_BLEND_RGB888(x, y);
			break;
		case SDL_BLENDMODE_ADD:
			DRAW_SETPIXELXY_ADD_RGB888(x, y);
			break;
		case SDL_BLENDMODE_MOD:
			DRAW_SETPIXELXY_MOD_RGB888(x, y);
			break;
		case SDL_BLENDMODE_MUL:
			DRAW_SETPIXELXY_MUL_RGB888(x, y);
			break;
		default:
			DRAW_SETPIXELXY_RGB888(x, y);
			break;
	}
	return 0;
}

static int SDL_BlendPoint_ARGB8888(SDL_Surface *dst, int x, int y, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	unsigned inva = 0xff - a;

	switch (blendMode) {
		case SDL_BLENDMODE_BLEND:
			DRAW_SETPIXELXY_BLEND_ARGB8888(x, y);
			break;
		case SDL_BLENDMODE_ADD:
			DRAW_SETPIXELXY_ADD_ARGB8888(x, y);
			break;
		case SDL_BLENDMODE_MOD:
			DRAW_SETPIXELXY_MOD_ARGB8888(x, y);
			break;
		case SDL_BLENDMODE_MUL:
			DRAW_SETPIXELXY_MUL_ARGB8888(x, y);
			break;
		default:
			DRAW_SETPIXELXY_ARGB8888(x, y);
			break;
	}
	return 0;
}

static int SDL_BlendPoint_RGB(SDL_Surface *dst, int x, int y, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	SDL_PixelFormat *fmt = dst->format;
	unsigned inva = 0xff - a;

	switch (fmt->BytesPerPixel) {
		case 2:
			switch (blendMode) {
				case SDL_BLENDMODE_BLEND:
					DRAW_SETPIXELXY2_BLEND_RGB(x, y);
					break;
				case SDL_BLENDMODE_ADD:
					DRAW_SETPIXELXY2_ADD_RGB(x, y);
					break;
				case SDL_BLENDMODE_MOD:
					DRAW_SETPIXELXY2_MOD_RGB(x, y);
					break;
				case SDL_BLENDMODE_MUL:
					DRAW_SETPIXELXY2_MUL_RGB(x, y);
					break;
				default:
					DRAW_SETPIXELXY2_RGB(x, y);
					break;
			}
			return 0;
		case 4:
			switch (blendMode) {
				case SDL_BLENDMODE_BLEND:
					DRAW_SETPIXELXY4_BLEND_RGB(x, y);
					break;
				case SDL_BLENDMODE_ADD:
					DRAW_SETPIXELXY4_ADD_RGB(x, y);
					break;
				case SDL_BLENDMODE_MOD:
					DRAW_SETPIXELXY4_MOD_RGB(x, y);
					break;
				case SDL_BLENDMODE_MUL:
					DRAW_SETPIXELXY4_MUL_RGB(x, y);
					break;
				default:
					DRAW_SETPIXELXY4_RGB(x, y);
					break;
			}
			return 0;
		default:
			return SDL_Unsupported();
	}
}

static int SDL_BlendPoint_RGBA(SDL_Surface *dst, int x, int y, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	SDL_PixelFormat *fmt = dst->format;
	unsigned inva = 0xff - a;

	switch (fmt->BytesPerPixel) {
		case 4:
			switch (blendMode) {
				case SDL_BLENDMODE_BLEND:
					DRAW_SETPIXELXY4_BLEND_RGBA(x, y);
					break;
				case SDL_BLENDMODE_ADD:
					DRAW_SETPIXELXY4_ADD_RGBA(x, y);
					break;
				case SDL_BLENDMODE_MOD:
					DRAW_SETPIXELXY4_MOD_RGBA(x, y);
					break;
				case SDL_BLENDMODE_MUL:
					DRAW_SETPIXELXY4_MUL_RGBA(x, y);
					break;
				default:
					DRAW_SETPIXELXY4_RGBA(x, y);
					break;
			}
			return 0;
		default:
			return SDL_Unsupported();
	}
}

int SDL_BlendPoint(SDL_Surface *dst, int x, int y, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	if (!dst) {
		return SDL_InvalidParamError("SDL_BlendPoint(): dst");
	}

	/* This function doesn't work on surfaces < 8 bpp */
	if (dst->format->BitsPerPixel < 8) {
		return SDL_SetError("SDL_BlendPoint(): Unsupported surface format");
	}

	/* Perform clipping */
	if (x < dst->clip_rect.x || y < dst->clip_rect.y ||
			x >= (dst->clip_rect.x + dst->clip_rect.w) ||
			y >= (dst->clip_rect.y + dst->clip_rect.h)) {
		return 0;
	}

	if (blendMode == SDL_BLENDMODE_BLEND || blendMode == SDL_BLENDMODE_ADD) {
		r = DRAW_MUL(r, a);
		g = DRAW_MUL(g, a);
		b = DRAW_MUL(b, a);
	}

	switch (dst->format->BitsPerPixel) {
		case 15:
			switch (dst->format->Rmask) {
				case 0x7C00:
					return SDL_BlendPoint_RGB555(dst, x, y, blendMode, r, g, b, a);
			}
			break;
		case 16:
			switch (dst->format->Rmask) {
				case 0xF800:
					return SDL_BlendPoint_RGB565(dst, x, y, blendMode, r, g, b, a);
			}
			break;
		case 32:
			switch (dst->format->Rmask) {
				case 0x00FF0000:
					if (!dst->format->Amask) {
						return SDL_BlendPoint_RGB888(dst, x, y, blendMode, r, g, b, a);
					} else {
						return SDL_BlendPoint_ARGB8888(dst, x, y, blendMode, r, g, b, a);
					}
					/* break; -Wunreachable-code-break */
			}
			break;
		default:
			break;
	}

	if (!dst->format->Amask) {
		return SDL_BlendPoint_RGB(dst, x, y, blendMode, r, g, b, a);
	} else {
		return SDL_BlendPoint_RGBA(dst, x, y, blendMode, r, g, b, a);
	}
}

int SDL_BlendPoints(SDL_Surface *dst, const SDL_Point *points, int count, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	int minx, miny;
	int maxx, maxy;
	int (*func)(SDL_Surface * dst, int x, int y, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a) = NULL;
	int status = 0;

	if (!dst) {
		return SDL_InvalidParamError("SDL_BlendPoints(): dst");
	}

	/* This function doesn't work on surfaces < 8 bpp */
	if (dst->format->BitsPerPixel < 8) {
		return SDL_SetError("SDL_BlendPoints(): Unsupported surface format");
	}

	if (blendMode == SDL_BLENDMODE_BLEND || blendMode == SDL_BLENDMODE_ADD) {
		r = DRAW_MUL(r, a);
		g = DRAW_MUL(g, a);
		b = DRAW_MUL(b, a);
	}

	/* FIXME: Does this function pointer slow things down significantly? */
	switch (dst->format->BitsPerPixel) {
		case 15:
			switch (dst->format->Rmask) {
				case 0x7C00:
					func = SDL_BlendPoint_RGB555;
					break;
			}
			break;
		case 16:
			switch (dst->format->Rmask) {
				case 0xF800:
					func = SDL_BlendPoint_RGB565;
					break;
			}
			break;
		case 32:
			switch (dst->format->Rmask) {
				case 0x00FF0000:
					if (!dst->format->Amask) {
						func = SDL_BlendPoint_RGB888;
					} else {
						func = SDL_BlendPoint_ARGB8888;
					}
					break;
			}
			break;
		default:
			break;
	}

	if (!func) {
		if (!dst->format->Amask) {
			func = SDL_BlendPoint_RGB;
		} else {
			func = SDL_BlendPoint_RGBA;
		}
	}

	minx = dst->clip_rect.x;
	maxx = dst->clip_rect.x + dst->clip_rect.w - 1;
	miny = dst->clip_rect.y;
	maxy = dst->clip_rect.y + dst->clip_rect.h - 1;

	for (int i = 0; i < count; ++i) {
		int x = points[i].x;
		int y = points[i].y;

		if (x < minx || x > maxx || y < miny || y > maxy) {
			continue;
		}
		status = func(dst, x, y, blendMode, r, g, b, a);
	}
	return status;
}

/// Lines

static void SDL_DrawLine1(SDL_Surface *dst, int x1, int y1, int x2, int y2, Uint32 color, SDL_bool draw_end) {
	if (y1 == y2) {
		int length;
		int pitch = (dst->pitch / dst->format->BytesPerPixel);
		Uint8 *pixel;
		if (x1 <= x2) {
			pixel = (Uint8 *)dst->pixels + y1 * pitch + x1;
			length = draw_end ? (x2 - x1 + 1) : (x2 - x1);
		} else {
			pixel = (Uint8 *)dst->pixels + y1 * pitch + x2;
			if (!draw_end) {
				++pixel;
			}
			length = draw_end ? (x1 - x2 + 1) : (x1 - x2);
		}
		SDL_memset(pixel, color, length);
	} else if (x1 == x2) {
		VLINE(Uint8, DRAW_FASTSETPIXEL1, draw_end);
	} else if (ABS(x1 - x2) == ABS(y1 - y2)) {
		DLINE(Uint8, DRAW_FASTSETPIXEL1, draw_end);
	} else {
		BLINE(x1, y1, x2, y2, DRAW_FASTSETPIXELXY1, draw_end);
	}
}

static void SDL_DrawLine2(SDL_Surface *dst, int x1, int y1, int x2, int y2, Uint32 color, SDL_bool draw_end) {
	if (y1 == y2) {
		HLINE(Uint16, DRAW_FASTSETPIXEL2, draw_end);
	} else if (x1 == x2) {
		VLINE(Uint16, DRAW_FASTSETPIXEL2, draw_end);
	} else if (ABS(x1 - x2) == ABS(y1 - y2)) {
		DLINE(Uint16, DRAW_FASTSETPIXEL2, draw_end);
	} else {
		Uint8 _r, _g, _b, _a;
		const SDL_PixelFormat *fmt = dst->format;
		SDL_GetRGBA(color, fmt, &_r, &_g, &_b, &_a);
		if (fmt->Rmask == 0x7C00) {
			AALINE(x1, y1, x2, y2,
					DRAW_FASTSETPIXELXY2, DRAW_SETPIXELXY_BLEND_RGB555,
					draw_end);
		} else if (fmt->Rmask == 0xF800) {
			AALINE(x1, y1, x2, y2,
					DRAW_FASTSETPIXELXY2, DRAW_SETPIXELXY_BLEND_RGB565,
					draw_end);
		} else {
			AALINE(x1, y1, x2, y2,
					DRAW_FASTSETPIXELXY2, DRAW_SETPIXELXY2_BLEND_RGB,
					draw_end);
		}
	}
}

static void SDL_DrawLine4(SDL_Surface *dst, int x1, int y1, int x2, int y2, Uint32 color, SDL_bool draw_end) {
	if (y1 == y2) {
		HLINE(Uint32, DRAW_FASTSETPIXEL4, draw_end);
	} else if (x1 == x2) {
		VLINE(Uint32, DRAW_FASTSETPIXEL4, draw_end);
	} else if (ABS(x1 - x2) == ABS(y1 - y2)) {
		DLINE(Uint32, DRAW_FASTSETPIXEL4, draw_end);
	} else {
		Uint8 _r, _g, _b, _a;
		const SDL_PixelFormat *fmt = dst->format;
		SDL_GetRGBA(color, fmt, &_r, &_g, &_b, &_a);
		if (fmt->Rmask == 0x00FF0000) {
			if (!fmt->Amask) {
				AALINE(x1, y1, x2, y2,
						DRAW_FASTSETPIXELXY4, DRAW_SETPIXELXY_BLEND_RGB888,
						draw_end);
			} else {
				AALINE(x1, y1, x2, y2,
						DRAW_FASTSETPIXELXY4, DRAW_SETPIXELXY_BLEND_ARGB8888,
						draw_end);
			}
		} else {
			AALINE(x1, y1, x2, y2,
					DRAW_FASTSETPIXELXY4, DRAW_SETPIXELXY4_BLEND_RGB,
					draw_end);
		}
	}
}

typedef void (*DrawLineFunc)(SDL_Surface *dst,
		int x1, int y1, int x2, int y2,
		Uint32 color, SDL_bool draw_end);

static DrawLineFunc SDL_CalculateDrawLineFunc(const SDL_PixelFormat *fmt) {
	switch (fmt->BytesPerPixel) {
		case 1:
			if (fmt->BitsPerPixel < 8) {
				break;
			}
			return SDL_DrawLine1;
		case 2:
			return SDL_DrawLine2;
		case 4:
			return SDL_DrawLine4;
	}
	return NULL;
}

int SDL_DrawLine(SDL_Surface *dst, int x1, int y1, int x2, int y2, Uint32 color) {
	DrawLineFunc func;

	if (!dst) {
		return SDL_InvalidParamError("SDL_DrawLine(): dst");
	}

	func = SDL_CalculateDrawLineFunc(dst->format);
	if (!func) {
		return SDL_SetError("SDL_DrawLine(): Unsupported surface format");
	}

	/* Perform clipping */
	/* FIXME: We don't actually want to clip, as it may change line slope */
	if (!SDL_IntersectRectAndLine(&dst->clip_rect, &x1, &y1, &x2, &y2)) {
		return 0;
	}

	func(dst, x1, y1, x2, y2, color, SDL_TRUE);
	return 0;
}

int SDL_DrawLines(SDL_Surface *dst, const SDL_Point *points, int count, Uint32 color) {
	SDL_bool draw_end;
	DrawLineFunc func;

	if (!dst) {
		return SDL_InvalidParamError("SDL_DrawLines(): dst");
	}

	func = SDL_CalculateDrawLineFunc(dst->format);
	if (!func) {
		return SDL_SetError("SDL_DrawLines(): Unsupported surface format");
	}

	for (int i = 1; i < count; ++i) {
		int x1 = points[i - 1].x;
		int y1 = points[i - 1].y;
		int x2 = points[i].x;
		int y2 = points[i].y;

		/* Perform clipping */
		/* FIXME: We don't actually want to clip, as it may change line slope */
		if (!SDL_IntersectRectAndLine(&dst->clip_rect, &x1, &y1, &x2, &y2)) {
			continue;
		}

		/* Draw the end if the whole line is a single point or it was clipped */
		draw_end = ((x1 == x2) && (y1 == y2)) || (x2 != points[i].x || y2 != points[i].y);

		func(dst, x1, y1, x2, y2, color, draw_end);
	}
	if (points[0].x != points[count - 1].x || points[0].y != points[count - 1].y) {
		SDL_DrawPoint(dst, points[count - 1].x, points[count - 1].y, color);
	}
	return 0;
}

static void SDL_BlendLine_RGB2(SDL_Surface *dst, int x1, int y1, int x2, int y2,
		SDL_BlendMode blendMode, Uint8 _r, Uint8 _g, Uint8 _b, Uint8 _a,
		SDL_bool draw_end) {
	const SDL_PixelFormat *fmt = dst->format;
	unsigned r, g, b, a, inva;

	if (blendMode == SDL_BLENDMODE_BLEND || blendMode == SDL_BLENDMODE_ADD) {
		r = DRAW_MUL(_r, _a);
		g = DRAW_MUL(_g, _a);
		b = DRAW_MUL(_b, _a);
		a = _a;
	} else {
		r = _r;
		g = _g;
		b = _b;
		a = _a;
	}
	inva = (a ^ 0xff);

	if (y1 == y2) {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				HLINE(Uint16, DRAW_SETPIXEL_BLEND_RGB, draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				HLINE(Uint16, DRAW_SETPIXEL_ADD_RGB, draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				HLINE(Uint16, DRAW_SETPIXEL_MOD_RGB, draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				HLINE(Uint16, DRAW_SETPIXEL_MUL_RGB, draw_end);
				break;
			default:
				HLINE(Uint16, DRAW_SETPIXEL_RGB, draw_end);
				break;
		}
	} else if (x1 == x2) {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				VLINE(Uint16, DRAW_SETPIXEL_BLEND_RGB, draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				VLINE(Uint16, DRAW_SETPIXEL_ADD_RGB, draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				VLINE(Uint16, DRAW_SETPIXEL_MOD_RGB, draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				VLINE(Uint16, DRAW_SETPIXEL_MUL_RGB, draw_end);
				break;
			default:
				VLINE(Uint16, DRAW_SETPIXEL_RGB, draw_end);
				break;
		}
	} else if (ABS(x1 - x2) == ABS(y1 - y2)) {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				DLINE(Uint16, DRAW_SETPIXEL_BLEND_RGB, draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				DLINE(Uint16, DRAW_SETPIXEL_ADD_RGB, draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				DLINE(Uint16, DRAW_SETPIXEL_MOD_RGB, draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				DLINE(Uint16, DRAW_SETPIXEL_MUL_RGB, draw_end);
				break;
			default:
				DLINE(Uint16, DRAW_SETPIXEL_RGB, draw_end);
				break;
		}
	} else {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY2_BLEND_RGB, DRAW_SETPIXELXY2_BLEND_RGB,
						draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY2_ADD_RGB, DRAW_SETPIXELXY2_ADD_RGB,
						draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY2_MOD_RGB, DRAW_SETPIXELXY2_MOD_RGB,
						draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY2_MUL_RGB, DRAW_SETPIXELXY2_MUL_RGB,
						draw_end);
				break;
			default:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY2_RGB, DRAW_SETPIXELXY2_BLEND_RGB,
						draw_end);
				break;
		}
	}
}

static void SDL_BlendLine_RGB555(SDL_Surface *dst, int x1, int y1, int x2, int y2,
		SDL_BlendMode blendMode, Uint8 _r, Uint8 _g, Uint8 _b, Uint8 _a,
		SDL_bool draw_end) {
	unsigned r, g, b, a, inva;

	if (blendMode == SDL_BLENDMODE_BLEND || blendMode == SDL_BLENDMODE_ADD) {
		r = DRAW_MUL(_r, _a);
		g = DRAW_MUL(_g, _a);
		b = DRAW_MUL(_b, _a);
		a = _a;
	} else {
		r = _r;
		g = _g;
		b = _b;
		a = _a;
	}
	inva = (a ^ 0xff);

	if (y1 == y2) {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				HLINE(Uint16, DRAW_SETPIXEL_BLEND_RGB555, draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				HLINE(Uint16, DRAW_SETPIXEL_ADD_RGB555, draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				HLINE(Uint16, DRAW_SETPIXEL_MOD_RGB555, draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				HLINE(Uint16, DRAW_SETPIXEL_MUL_RGB555, draw_end);
				break;
			default:
				HLINE(Uint16, DRAW_SETPIXEL_RGB555, draw_end);
				break;
		}
	} else if (x1 == x2) {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				VLINE(Uint16, DRAW_SETPIXEL_BLEND_RGB555, draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				VLINE(Uint16, DRAW_SETPIXEL_ADD_RGB555, draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				VLINE(Uint16, DRAW_SETPIXEL_MOD_RGB555, draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				VLINE(Uint16, DRAW_SETPIXEL_MUL_RGB555, draw_end);
				break;
			default:
				VLINE(Uint16, DRAW_SETPIXEL_RGB555, draw_end);
				break;
		}
	} else if (ABS(x1 - x2) == ABS(y1 - y2)) {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				DLINE(Uint16, DRAW_SETPIXEL_BLEND_RGB555, draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				DLINE(Uint16, DRAW_SETPIXEL_ADD_RGB555, draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				DLINE(Uint16, DRAW_SETPIXEL_MOD_RGB555, draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				DLINE(Uint16, DRAW_SETPIXEL_MUL_RGB555, draw_end);
				break;
			default:
				DLINE(Uint16, DRAW_SETPIXEL_RGB555, draw_end);
				break;
		}
	} else {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY_BLEND_RGB555, DRAW_SETPIXELXY_BLEND_RGB555,
						draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY_ADD_RGB555, DRAW_SETPIXELXY_ADD_RGB555,
						draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY_MOD_RGB555, DRAW_SETPIXELXY_MOD_RGB555,
						draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY_MUL_RGB555, DRAW_SETPIXELXY_MUL_RGB555,
						draw_end);
				break;
			default:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY_RGB555, DRAW_SETPIXELXY_BLEND_RGB555,
						draw_end);
				break;
		}
	}
}

static void SDL_BlendLine_RGB565(SDL_Surface *dst, int x1, int y1, int x2, int y2,
		SDL_BlendMode blendMode, Uint8 _r, Uint8 _g, Uint8 _b, Uint8 _a,
		SDL_bool draw_end) {
	unsigned r, g, b, a, inva;

	if (blendMode == SDL_BLENDMODE_BLEND || blendMode == SDL_BLENDMODE_ADD) {
		r = DRAW_MUL(_r, _a);
		g = DRAW_MUL(_g, _a);
		b = DRAW_MUL(_b, _a);
		a = _a;
	} else {
		r = _r;
		g = _g;
		b = _b;
		a = _a;
	}
	inva = (a ^ 0xff);

	if (y1 == y2) {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				HLINE(Uint16, DRAW_SETPIXEL_BLEND_RGB565, draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				HLINE(Uint16, DRAW_SETPIXEL_ADD_RGB565, draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				HLINE(Uint16, DRAW_SETPIXEL_MOD_RGB565, draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				HLINE(Uint16, DRAW_SETPIXEL_MUL_RGB565, draw_end);
				break;
			default:
				HLINE(Uint16, DRAW_SETPIXEL_RGB565, draw_end);
				break;
		}
	} else if (x1 == x2) {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				VLINE(Uint16, DRAW_SETPIXEL_BLEND_RGB565, draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				VLINE(Uint16, DRAW_SETPIXEL_ADD_RGB565, draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				VLINE(Uint16, DRAW_SETPIXEL_MOD_RGB565, draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				VLINE(Uint16, DRAW_SETPIXEL_MUL_RGB565, draw_end);
				break;
			default:
				VLINE(Uint16, DRAW_SETPIXEL_RGB565, draw_end);
				break;
		}
	} else if (ABS(x1 - x2) == ABS(y1 - y2)) {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				DLINE(Uint16, DRAW_SETPIXEL_BLEND_RGB565, draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				DLINE(Uint16, DRAW_SETPIXEL_ADD_RGB565, draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				DLINE(Uint16, DRAW_SETPIXEL_MOD_RGB565, draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				DLINE(Uint16, DRAW_SETPIXEL_MUL_RGB565, draw_end);
				break;
			default:
				DLINE(Uint16, DRAW_SETPIXEL_RGB565, draw_end);
				break;
		}
	} else {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY_BLEND_RGB565, DRAW_SETPIXELXY_BLEND_RGB565,
						draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY_ADD_RGB565, DRAW_SETPIXELXY_ADD_RGB565,
						draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY_MOD_RGB565, DRAW_SETPIXELXY_MOD_RGB565,
						draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY_MUL_RGB565, DRAW_SETPIXELXY_MUL_RGB565,
						draw_end);
				break;
			default:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY_RGB565, DRAW_SETPIXELXY_BLEND_RGB565,
						draw_end);
				break;
		}
	}
}

static void SDL_BlendLine_RGB4(SDL_Surface *dst, int x1, int y1, int x2, int y2,
		SDL_BlendMode blendMode, Uint8 _r, Uint8 _g, Uint8 _b, Uint8 _a,
		SDL_bool draw_end) {
	const SDL_PixelFormat *fmt = dst->format;
	unsigned r, g, b, a, inva;

	if (blendMode == SDL_BLENDMODE_BLEND || blendMode == SDL_BLENDMODE_ADD) {
		r = DRAW_MUL(_r, _a);
		g = DRAW_MUL(_g, _a);
		b = DRAW_MUL(_b, _a);
		a = _a;
	} else {
		r = _r;
		g = _g;
		b = _b;
		a = _a;
	}
	inva = (a ^ 0xff);

	if (y1 == y2) {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				HLINE(Uint32, DRAW_SETPIXEL_BLEND_RGB, draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				HLINE(Uint32, DRAW_SETPIXEL_ADD_RGB, draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				HLINE(Uint32, DRAW_SETPIXEL_MOD_RGB, draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				HLINE(Uint32, DRAW_SETPIXEL_MUL_RGB, draw_end);
				break;
			default:
				HLINE(Uint32, DRAW_SETPIXEL_RGB, draw_end);
				break;
		}
	} else if (x1 == x2) {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				VLINE(Uint32, DRAW_SETPIXEL_BLEND_RGB, draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				VLINE(Uint32, DRAW_SETPIXEL_ADD_RGB, draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				VLINE(Uint32, DRAW_SETPIXEL_MOD_RGB, draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				VLINE(Uint32, DRAW_SETPIXEL_MUL_RGB, draw_end);
				break;
			default:
				VLINE(Uint32, DRAW_SETPIXEL_RGB, draw_end);
				break;
		}
	} else if (ABS(x1 - x2) == ABS(y1 - y2)) {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				DLINE(Uint32, DRAW_SETPIXEL_BLEND_RGB, draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				DLINE(Uint32, DRAW_SETPIXEL_ADD_RGB, draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				DLINE(Uint32, DRAW_SETPIXEL_MOD_RGB, draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				DLINE(Uint32, DRAW_SETPIXEL_MUL_RGB, draw_end);
				break;
			default:
				DLINE(Uint32, DRAW_SETPIXEL_RGB, draw_end);
				break;
		}
	} else {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY4_BLEND_RGB, DRAW_SETPIXELXY4_BLEND_RGB,
						draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY4_ADD_RGB, DRAW_SETPIXELXY4_ADD_RGB,
						draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY4_MOD_RGB, DRAW_SETPIXELXY4_MOD_RGB,
						draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY4_MUL_RGB, DRAW_SETPIXELXY4_MUL_RGB,
						draw_end);
				break;
			default:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY4_RGB, DRAW_SETPIXELXY4_BLEND_RGB,
						draw_end);
				break;
		}
	}
}

static void SDL_BlendLine_RGBA4(SDL_Surface *dst, int x1, int y1, int x2, int y2,
		SDL_BlendMode blendMode, Uint8 _r, Uint8 _g, Uint8 _b, Uint8 _a,
		SDL_bool draw_end) {
	const SDL_PixelFormat *fmt = dst->format;
	unsigned r, g, b, a, inva;

	if (blendMode == SDL_BLENDMODE_BLEND || blendMode == SDL_BLENDMODE_ADD) {
		r = DRAW_MUL(_r, _a);
		g = DRAW_MUL(_g, _a);
		b = DRAW_MUL(_b, _a);
		a = _a;
	} else {
		r = _r;
		g = _g;
		b = _b;
		a = _a;
	}
	inva = (a ^ 0xff);

	if (y1 == y2) {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				HLINE(Uint32, DRAW_SETPIXEL_BLEND_RGBA, draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				HLINE(Uint32, DRAW_SETPIXEL_ADD_RGBA, draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				HLINE(Uint32, DRAW_SETPIXEL_MOD_RGBA, draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				HLINE(Uint32, DRAW_SETPIXEL_MUL_RGBA, draw_end);
				break;
			default:
				HLINE(Uint32, DRAW_SETPIXEL_RGBA, draw_end);
				break;
		}
	} else if (x1 == x2) {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				VLINE(Uint32, DRAW_SETPIXEL_BLEND_RGBA, draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				VLINE(Uint32, DRAW_SETPIXEL_ADD_RGBA, draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				VLINE(Uint32, DRAW_SETPIXEL_MOD_RGBA, draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				VLINE(Uint32, DRAW_SETPIXEL_MUL_RGBA, draw_end);
				break;
			default:
				VLINE(Uint32, DRAW_SETPIXEL_RGBA, draw_end);
				break;
		}
	} else if (ABS(x1 - x2) == ABS(y1 - y2)) {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				DLINE(Uint32, DRAW_SETPIXEL_BLEND_RGBA, draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				DLINE(Uint32, DRAW_SETPIXEL_ADD_RGBA, draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				DLINE(Uint32, DRAW_SETPIXEL_MOD_RGBA, draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				DLINE(Uint32, DRAW_SETPIXEL_MUL_RGBA, draw_end);
				break;
			default:
				DLINE(Uint32, DRAW_SETPIXEL_RGBA, draw_end);
				break;
		}
	} else {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY4_BLEND_RGBA, DRAW_SETPIXELXY4_BLEND_RGBA,
						draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY4_ADD_RGBA, DRAW_SETPIXELXY4_ADD_RGBA,
						draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY4_MOD_RGBA, DRAW_SETPIXELXY4_MOD_RGBA,
						draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY4_MUL_RGBA, DRAW_SETPIXELXY4_MUL_RGBA,
						draw_end);
				break;
			default:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY4_RGBA, DRAW_SETPIXELXY4_BLEND_RGBA,
						draw_end);
				break;
		}
	}
}

static void SDL_BlendLine_RGB888(SDL_Surface *dst, int x1, int y1, int x2, int y2,
		SDL_BlendMode blendMode, Uint8 _r, Uint8 _g, Uint8 _b, Uint8 _a,
		SDL_bool draw_end) {
	unsigned r, g, b, a, inva;

	if (blendMode == SDL_BLENDMODE_BLEND || blendMode == SDL_BLENDMODE_ADD) {
		r = DRAW_MUL(_r, _a);
		g = DRAW_MUL(_g, _a);
		b = DRAW_MUL(_b, _a);
		a = _a;
	} else {
		r = _r;
		g = _g;
		b = _b;
		a = _a;
	}
	inva = (a ^ 0xff);

	if (y1 == y2) {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				HLINE(Uint32, DRAW_SETPIXEL_BLEND_RGB888, draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				HLINE(Uint32, DRAW_SETPIXEL_ADD_RGB888, draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				HLINE(Uint32, DRAW_SETPIXEL_MOD_RGB888, draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				HLINE(Uint32, DRAW_SETPIXEL_MUL_RGB888, draw_end);
				break;
			default:
				HLINE(Uint32, DRAW_SETPIXEL_RGB888, draw_end);
				break;
		}
	} else if (x1 == x2) {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				VLINE(Uint32, DRAW_SETPIXEL_BLEND_RGB888, draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				VLINE(Uint32, DRAW_SETPIXEL_ADD_RGB888, draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				VLINE(Uint32, DRAW_SETPIXEL_MOD_RGB888, draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				VLINE(Uint32, DRAW_SETPIXEL_MUL_RGB888, draw_end);
				break;
			default:
				VLINE(Uint32, DRAW_SETPIXEL_RGB888, draw_end);
				break;
		}
	} else if (ABS(x1 - x2) == ABS(y1 - y2)) {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				DLINE(Uint32, DRAW_SETPIXEL_BLEND_RGB888, draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				DLINE(Uint32, DRAW_SETPIXEL_ADD_RGB888, draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				DLINE(Uint32, DRAW_SETPIXEL_MOD_RGB888, draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				DLINE(Uint32, DRAW_SETPIXEL_MUL_RGB888, draw_end);
				break;
			default:
				DLINE(Uint32, DRAW_SETPIXEL_RGB888, draw_end);
				break;
		}
	} else {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY_BLEND_RGB888, DRAW_SETPIXELXY_BLEND_RGB888,
						draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY_ADD_RGB888, DRAW_SETPIXELXY_ADD_RGB888,
						draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY_MOD_RGB888, DRAW_SETPIXELXY_MOD_RGB888,
						draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY_MUL_RGB888, DRAW_SETPIXELXY_MUL_RGB888,
						draw_end);
				break;
			default:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY_RGB888, DRAW_SETPIXELXY_BLEND_RGB888,
						draw_end);
				break;
		}
	}
}

static void SDL_BlendLine_ARGB8888(SDL_Surface *dst, int x1, int y1, int x2, int y2,
		SDL_BlendMode blendMode, Uint8 _r, Uint8 _g, Uint8 _b, Uint8 _a,
		SDL_bool draw_end) {
	unsigned r, g, b, a, inva;

	if (blendMode == SDL_BLENDMODE_BLEND || blendMode == SDL_BLENDMODE_ADD) {
		r = DRAW_MUL(_r, _a);
		g = DRAW_MUL(_g, _a);
		b = DRAW_MUL(_b, _a);
		a = _a;
	} else {
		r = _r;
		g = _g;
		b = _b;
		a = _a;
	}
	inva = (a ^ 0xff);

	if (y1 == y2) {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				HLINE(Uint32, DRAW_SETPIXEL_BLEND_ARGB8888, draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				HLINE(Uint32, DRAW_SETPIXEL_ADD_ARGB8888, draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				HLINE(Uint32, DRAW_SETPIXEL_MOD_ARGB8888, draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				HLINE(Uint32, DRAW_SETPIXEL_MUL_ARGB8888, draw_end);
				break;
			default:
				HLINE(Uint32, DRAW_SETPIXEL_ARGB8888, draw_end);
				break;
		}
	} else if (x1 == x2) {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				VLINE(Uint32, DRAW_SETPIXEL_BLEND_ARGB8888, draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				VLINE(Uint32, DRAW_SETPIXEL_ADD_ARGB8888, draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				VLINE(Uint32, DRAW_SETPIXEL_MOD_ARGB8888, draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				VLINE(Uint32, DRAW_SETPIXEL_MUL_ARGB8888, draw_end);
				break;
			default:
				VLINE(Uint32, DRAW_SETPIXEL_ARGB8888, draw_end);
				break;
		}
	} else if (ABS(x1 - x2) == ABS(y1 - y2)) {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				DLINE(Uint32, DRAW_SETPIXEL_BLEND_ARGB8888, draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				DLINE(Uint32, DRAW_SETPIXEL_ADD_ARGB8888, draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				DLINE(Uint32, DRAW_SETPIXEL_MOD_ARGB8888, draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				DLINE(Uint32, DRAW_SETPIXEL_MUL_ARGB8888, draw_end);
				break;
			default:
				DLINE(Uint32, DRAW_SETPIXEL_ARGB8888, draw_end);
				break;
		}
	} else {
		switch (blendMode) {
			case SDL_BLENDMODE_BLEND:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY_BLEND_ARGB8888, DRAW_SETPIXELXY_BLEND_ARGB8888,
						draw_end);
				break;
			case SDL_BLENDMODE_ADD:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY_ADD_ARGB8888, DRAW_SETPIXELXY_ADD_ARGB8888,
						draw_end);
				break;
			case SDL_BLENDMODE_MOD:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY_MOD_ARGB8888, DRAW_SETPIXELXY_MOD_ARGB8888,
						draw_end);
				break;
			case SDL_BLENDMODE_MUL:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY_MUL_ARGB8888, DRAW_SETPIXELXY_MUL_ARGB8888,
						draw_end);
				break;
			default:
				AALINE(x1, y1, x2, y2,
						DRAW_SETPIXELXY_ARGB8888, DRAW_SETPIXELXY_BLEND_ARGB8888,
						draw_end);
				break;
		}
	}
}

typedef void (*BlendLineFunc)(SDL_Surface *dst,
		int x1, int y1, int x2, int y2,
		SDL_BlendMode blendMode,
		Uint8 r, Uint8 g, Uint8 b, Uint8 a,
		SDL_bool draw_end);

static BlendLineFunc SDL_CalculateBlendLineFunc(const SDL_PixelFormat *fmt) {
	switch (fmt->BytesPerPixel) {
		case 2:
			if (fmt->Rmask == 0x7C00) {
				return SDL_BlendLine_RGB555;
			} else if (fmt->Rmask == 0xF800) {
				return SDL_BlendLine_RGB565;
			} else {
				return SDL_BlendLine_RGB2;
			}
			/* break; -Wunreachable-code-break */
		case 4:
			if (fmt->Rmask == 0x00FF0000) {
				if (fmt->Amask) {
					return SDL_BlendLine_ARGB8888;
				} else {
					return SDL_BlendLine_RGB888;
				}
			} else {
				if (fmt->Amask) {
					return SDL_BlendLine_RGBA4;
				} else {
					return SDL_BlendLine_RGB4;
				}
			}
	}
	return NULL;
}

int SDL_BlendLine(SDL_Surface *dst, int x1, int y1, int x2, int y2, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	BlendLineFunc func;

	if (!dst) {
		return SDL_InvalidParamError("SDL_BlendLine(): dst");
	}

	func = SDL_CalculateBlendLineFunc(dst->format);
	if (!func) {
		return SDL_SetError("SDL_BlendLine(): Unsupported surface format");
	}

	/* Perform clipping */
	/* FIXME: We don't actually want to clip, as it may change line slope */
	if (!SDL_IntersectRectAndLine(&dst->clip_rect, &x1, &y1, &x2, &y2)) {
		return 0;
	}

	func(dst, x1, y1, x2, y2, blendMode, r, g, b, a, SDL_TRUE);
	return 0;
}

int SDL_BlendLines(SDL_Surface *dst, const SDL_Point *points, int count, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	SDL_bool draw_end;
	BlendLineFunc func;

	if (!dst) {
		return SDL_SetError("SDL_BlendLines(): Passed NULL destination surface");
	}

	func = SDL_CalculateBlendLineFunc(dst->format);
	if (!func) {
		return SDL_SetError("SDL_BlendLines(): Unsupported surface format");
	}

	for (int i = 1; i < count; ++i) {
		int x1 = points[i - 1].x;
		int y1 = points[i - 1].y;
		int x2 = points[i].x;
		int y2 = points[i].y;

		/* Perform clipping */
		/* FIXME: We don't actually want to clip, as it may change line slope */
		if (!SDL_IntersectRectAndLine(&dst->clip_rect, &x1, &y1, &x2, &y2)) {
			continue;
		}

		/* Draw the end if it was clipped */
		draw_end = (x2 != points[i].x || y2 != points[i].y);
		func(dst, x1, y1, x2, y2, blendMode, r, g, b, a, draw_end);
	}
	if (points[0].x != points[count - 1].x || points[0].y != points[count - 1].y) {
		SDL_BlendPoint(dst, points[count - 1].x, points[count - 1].y,
				blendMode, r, g, b, a);
	}
	return 0;
}

/// Rect

#ifdef __SSE__

#if defined(_MSC_VER) && !defined(__clang__)
#define SSE_BEGIN             \
	__m128 c128;              \
	c128.m128_u32[0] = color; \
	c128.m128_u32[1] = color; \
	c128.m128_u32[2] = color; \
	c128.m128_u32[3] = color;
#else
#define SSE_BEGIN                         \
	__m128 c128;                          \
	DECLARE_ALIGNED(Uint32, cccc[4], 16); \
	cccc[0] = color;                      \
	cccc[1] = color;                      \
	cccc[2] = color;                      \
	cccc[3] = color;                      \
	c128 = *(__m128 *)cccc;
#endif

#define SSE_WORK                                \
	for (int i = n / 64; i--;) {                \
		_mm_stream_ps((float *)(p + 0), c128);  \
		_mm_stream_ps((float *)(p + 16), c128); \
		_mm_stream_ps((float *)(p + 32), c128); \
		_mm_stream_ps((float *)(p + 48), c128); \
		p += 64;                                \
	}

#define SSE_END

#define DEFINE_SSE_FILLRECT(bpp, type)                                                         \
	static void SDL_FillRect##bpp##SSE(Uint8 *pixels, int pitch, Uint32 color, int w, int h) { \
		SSE_BEGIN;                                                                             \
		while (h--) {                                                                          \
			int = w * bpp;                                                                     \
			Uint8 *p = pixels;                                                                 \
                                                                                               \
			if (n > 63) {                                                                      \
				int adjust = 16 - ((uintptr_t)p & 15);                                         \
				if (adjust < 16) {                                                             \
					n -= adjust;                                                               \
					adjust /= bpp;                                                             \
					while (adjust--) {                                                         \
						*((type *)p) = (type)color;                                            \
						p += bpp;                                                              \
					}                                                                          \
				}                                                                              \
				SSE_WORK;                                                                      \
			}                                                                                  \
			if (n & 63) {                                                                      \
				int remainder = (n & 63);                                                      \
				remainder /= bpp;                                                              \
				while (remainder--) {                                                          \
					*((type *)p) = (type)color;                                                \
					p += bpp;                                                                  \
				}                                                                              \
			}                                                                                  \
			pixels += pitch;                                                                   \
		}                                                                                      \
                                                                                               \
		SSE_END;                                                                               \
	}

static void SDL_FillRect1SSE(Uint8 *pixels, int pitch, Uint32 color, int w, int h) {
	SSE_BEGIN;
	while (h--) {
		Uint8 *p = pixels;
		int n = w;

		if (n > 63) {
			int adjust = 16 - ((uintptr_t)p & 15);
			if (adjust) {
				n -= adjust;
				SDL_memset(p, color, adjust);
				p += adjust;
			}
			SSE_WORK;
		}
		if (n & 63) {
			int remainder = (n & 63);
			SDL_memset(p, color, remainder);
		}
		pixels += pitch;
	}

	SSE_END;
}
// DEFINE_SSE_FILLRECT(1, Uint8)
DEFINE_SSE_FILLRECT(2, Uint16)
DEFINE_SSE_FILLRECT(4, Uint32)

#endif /* __SSE__ */

static void SDL_FillRect1(Uint8 *pixels, int pitch, Uint32 color, int w, int h) {
	while (h--) {
		int n = w;
		Uint8 *p = pixels;

		if (n > 3) {
			switch ((uintptr_t)p & 3) {
				case 1:
					*p++ = (Uint8)color;
					--n;
					SDL_FALLTHROUGH;
				case 2:
					*p++ = (Uint8)color;
					--n;
					SDL_FALLTHROUGH;
				case 3:
					*p++ = (Uint8)color;
					--n;
			}
			SDL_memset4(p, color, (n >> 2));
		}
		if (n & 3) {
			p += (n & ~3);
			switch (n & 3) {
				case 3:
					*p++ = (Uint8)color;
					SDL_FALLTHROUGH;
				case 2:
					*p++ = (Uint8)color;
					SDL_FALLTHROUGH;
				case 1:
					*p++ = (Uint8)color;
			}
		}
		pixels += pitch;
	}
}

static void SDL_FillRect2(Uint8 *pixels, int pitch, Uint32 color, int w, int h) {
	while (h--) {
		int n = w;
		Uint16 *p = (Uint16 *)pixels;

		if (n > 1) {
			if ((uintptr_t)p & 2) {
				*p++ = (Uint16)color;
				--n;
			}
			SDL_memset4(p, color, (n >> 1));
		}
		if (n & 1) {
			p[n - 1] = (Uint16)color;
		}
		pixels += pitch;
	}
}

static void SDL_FillRect3(Uint8 *pixels, int pitch, Uint32 color, int w, int h) {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	Uint8 b1 = (Uint8)(color & 0xFF);
	Uint8 b2 = (Uint8)((color >> 8) & 0xFF);
	Uint8 b3 = (Uint8)((color >> 16) & 0xFF);
#elif SDL_BYTEORDER == SDL_BIG_ENDIAN
	Uint8 b1 = (Uint8)((color >> 16) & 0xFF);
	Uint8 b2 = (Uint8)((color >> 8) & 0xFF);
	Uint8 b3 = (Uint8)(color & 0xFF);
#endif
	while (h--) {
		int n = w;
		Uint8 *p = pixels;

		while (n--) {
			*p++ = b1;
			*p++ = b2;
			*p++ = b3;
		}
		pixels += pitch;
	}
}

static void SDL_FillRect4(Uint8 *pixels, int pitch, Uint32 color, int w, int h) {
	while (h--) {
		SDL_memset4(pixels, color, w);
		pixels += pitch;
	}
}

// This function performs a fast fill of the given rectangle with 'color'
int SDL_FillRect(SDL_Surface *dst, const SDL_Rect *rect, Uint32 color) {
	if (!dst) {
		return SDL_InvalidParamError("SDL_FillRect(): dst");
	}

	/* If 'rect' == NULL, then fill the whole surface */
	if (!rect) {
		rect = &dst->clip_rect;
		/* Don't attempt to fill if the surface's clip_rect is empty */
		if (SDL_RectEmpty(rect)) {
			return 0;
		}
	}

	return SDL_FillRects(dst, rect, 1, color);
}

int SDL_FillRects(SDL_Surface *dst, const SDL_Rect *rects, int count, Uint32 color) {
	SDL_Rect clipped;
	Uint8 *pixels;
	const SDL_Rect *rect;
	void (*fill_function)(Uint8 * pixels, int pitch, Uint32 color, int w, int h) = NULL;

	if (!dst) {
		return SDL_InvalidParamError("SDL_FillRects(): dst");
	}

	// Nothing to do
	if (dst->w == 0 || dst->h == 0) {
		return 0;
	}

	// Perform software fill
	if (!dst->pixels) {
		return SDL_SetError("SDL_FillRects(): You must lock the surface");
	}

	if (!rects) {
		return SDL_InvalidParamError("SDL_FillRects(): rects");
	}

	// This function doesn't usually work on surfaces < 8 bpp
	// Except: support for 4bits, when filling full size.
	if (dst->format->BitsPerPixel < 8) {
		if (count == 1) {
			const SDL_Rect *r = &rects[0];
			if (r->x == 0 && r->y == 0 && r->w == dst->w && r->w == dst->h) {
				if (dst->format->BitsPerPixel == 4) {
					Uint8 b = (((Uint8)color << 4) | (Uint8)color);
					SDL_memset(dst->pixels, b, dst->h * dst->pitch);
					return 1;
				}
			}
		}
		return SDL_SetError("SDL_FillRects(): Unsupported surface format");
	}

	if (fill_function == NULL) {
		switch (dst->format->BytesPerPixel) {
			case 1: {
				color |= (color << 8);
				color |= (color << 16);
#ifdef __SSE__
				if (SDL_HasSSE()) {
					fill_function = SDL_FillRect1SSE;
					break;
				}
#endif
				fill_function = SDL_FillRect1;
				break;
			}

			case 2: {
				color |= (color << 16);
#ifdef __SSE__
				if (SDL_HasSSE()) {
					fill_function = SDL_FillRect2SSE;
					break;
				}
#endif
				fill_function = SDL_FillRect2;
				break;
			}

			case 3:
				// 24-bit RGB is a slow path, at least for now.
				{
					fill_function = SDL_FillRect3;
					break;
				}

			case 4: {
#ifdef __SSE__
				if (SDL_HasSSE()) {
					fill_function = SDL_FillRect4SSE;
					break;
				}
#endif
				fill_function = SDL_FillRect4;
				break;
			}

			default:
				return SDL_SetError("Unsupported pixel format");
		}
	}

	for (int i = 0; i < count; ++i) {
		rect = &rects[i];
		// Perform clipping
		if (!SDL_IntersectRect(rect, &dst->clip_rect, &clipped)) {
			continue;
		}
		rect = &clipped;

		pixels = (Uint8 *)dst->pixels + rect->y * dst->pitch + rect->x * dst->format->BytesPerPixel;
		fill_function(pixels, dst->pitch, color, rect->w, rect->h);
	}

	// We're done!
	return 0;
}

static int SDL_BlendFillRect_RGB555(SDL_Surface *dst, const SDL_Rect *rect, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	unsigned inva = 0xff - a;

	switch (blendMode) {
		case SDL_BLENDMODE_BLEND:
			FILLRECT(Uint16, DRAW_SETPIXEL_BLEND_RGB555);
			break;
		case SDL_BLENDMODE_ADD:
			FILLRECT(Uint16, DRAW_SETPIXEL_ADD_RGB555);
			break;
		case SDL_BLENDMODE_MOD:
			FILLRECT(Uint16, DRAW_SETPIXEL_MOD_RGB555);
			break;
		case SDL_BLENDMODE_MUL:
			FILLRECT(Uint16, DRAW_SETPIXEL_MUL_RGB555);
			break;
		default:
			FILLRECT(Uint16, DRAW_SETPIXEL_RGB555);
			break;
	}
	return 0;
}

static int SDL_BlendFillRect_RGB565(SDL_Surface *dst, const SDL_Rect *rect, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	unsigned inva = 0xff - a;

	switch (blendMode) {
		case SDL_BLENDMODE_BLEND:
			FILLRECT(Uint16, DRAW_SETPIXEL_BLEND_RGB565);
			break;
		case SDL_BLENDMODE_ADD:
			FILLRECT(Uint16, DRAW_SETPIXEL_ADD_RGB565);
			break;
		case SDL_BLENDMODE_MOD:
			FILLRECT(Uint16, DRAW_SETPIXEL_MOD_RGB565);
			break;
		case SDL_BLENDMODE_MUL:
			FILLRECT(Uint16, DRAW_SETPIXEL_MUL_RGB565);
			break;
		default:
			FILLRECT(Uint16, DRAW_SETPIXEL_RGB565);
			break;
	}
	return 0;
}

static int SDL_BlendFillRect_RGB888(SDL_Surface *dst, const SDL_Rect *rect, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	unsigned inva = 0xff - a;

	switch (blendMode) {
		case SDL_BLENDMODE_BLEND:
			FILLRECT(Uint32, DRAW_SETPIXEL_BLEND_RGB888);
			break;
		case SDL_BLENDMODE_ADD:
			FILLRECT(Uint32, DRAW_SETPIXEL_ADD_RGB888);
			break;
		case SDL_BLENDMODE_MOD:
			FILLRECT(Uint32, DRAW_SETPIXEL_MOD_RGB888);
			break;
		case SDL_BLENDMODE_MUL:
			FILLRECT(Uint32, DRAW_SETPIXEL_MUL_RGB888);
			break;
		default:
			FILLRECT(Uint32, DRAW_SETPIXEL_RGB888);
			break;
	}
	return 0;
}

static int SDL_BlendFillRect_ARGB8888(SDL_Surface *dst, const SDL_Rect *rect, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	unsigned inva = 0xff - a;

	switch (blendMode) {
		case SDL_BLENDMODE_BLEND:
			FILLRECT(Uint32, DRAW_SETPIXEL_BLEND_ARGB8888);
			break;
		case SDL_BLENDMODE_ADD:
			FILLRECT(Uint32, DRAW_SETPIXEL_ADD_ARGB8888);
			break;
		case SDL_BLENDMODE_MOD:
			FILLRECT(Uint32, DRAW_SETPIXEL_MOD_ARGB8888);
			break;
		case SDL_BLENDMODE_MUL:
			FILLRECT(Uint32, DRAW_SETPIXEL_MUL_ARGB8888);
			break;
		default:
			FILLRECT(Uint32, DRAW_SETPIXEL_ARGB8888);
			break;
	}
	return 0;
}

static int SDL_BlendFillRect_RGB(SDL_Surface *dst, const SDL_Rect *rect, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	SDL_PixelFormat *fmt = dst->format;
	unsigned inva = 0xff - a;

	switch (fmt->BytesPerPixel) {
		case 2:
			switch (blendMode) {
				case SDL_BLENDMODE_BLEND:
					FILLRECT(Uint16, DRAW_SETPIXEL_BLEND_RGB);
					break;
				case SDL_BLENDMODE_ADD:
					FILLRECT(Uint16, DRAW_SETPIXEL_ADD_RGB);
					break;
				case SDL_BLENDMODE_MOD:
					FILLRECT(Uint16, DRAW_SETPIXEL_MOD_RGB);
					break;
				case SDL_BLENDMODE_MUL:
					FILLRECT(Uint16, DRAW_SETPIXEL_MUL_RGB);
					break;
				default:
					FILLRECT(Uint16, DRAW_SETPIXEL_RGB);
					break;
			}
			return 0;
		case 4:
			switch (blendMode) {
				case SDL_BLENDMODE_BLEND:
					FILLRECT(Uint32, DRAW_SETPIXEL_BLEND_RGB);
					break;
				case SDL_BLENDMODE_ADD:
					FILLRECT(Uint32, DRAW_SETPIXEL_ADD_RGB);
					break;
				case SDL_BLENDMODE_MOD:
					FILLRECT(Uint32, DRAW_SETPIXEL_MOD_RGB);
					break;
				case SDL_BLENDMODE_MUL:
					FILLRECT(Uint32, DRAW_SETPIXEL_MUL_RGB);
					break;
				default:
					FILLRECT(Uint32, DRAW_SETPIXEL_RGB);
					break;
			}
			return 0;
		default:
			return SDL_Unsupported();
	}
}

static int SDL_BlendFillRect_RGBA(SDL_Surface *dst, const SDL_Rect *rect, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	SDL_PixelFormat *fmt = dst->format;
	unsigned inva = 0xff - a;

	switch (fmt->BytesPerPixel) {
		case 4:
			switch (blendMode) {
				case SDL_BLENDMODE_BLEND:
					FILLRECT(Uint32, DRAW_SETPIXEL_BLEND_RGBA);
					break;
				case SDL_BLENDMODE_ADD:
					FILLRECT(Uint32, DRAW_SETPIXEL_ADD_RGBA);
					break;
				case SDL_BLENDMODE_MOD:
					FILLRECT(Uint32, DRAW_SETPIXEL_MOD_RGBA);
					break;
				case SDL_BLENDMODE_MUL:
					FILLRECT(Uint32, DRAW_SETPIXEL_MUL_RGBA);
					break;
				default:
					FILLRECT(Uint32, DRAW_SETPIXEL_RGBA);
					break;
			}
			return 0;
		default:
			return SDL_Unsupported();
	}
}

int SDL_BlendFillRect(SDL_Surface *dst, const SDL_Rect *rect, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	SDL_Rect clipped;

	if (!dst) {
		return SDL_InvalidParamError("SDL_BlendFillRect(): dst");
	}

	/* This function doesn't work on surfaces < 8 bpp */
	if (dst->format->BitsPerPixel < 8) {
		return SDL_SetError("SDL_BlendFillRect(): Unsupported surface format");
	}

	/* If 'rect' == NULL, then fill the whole surface */
	if (rect) {
		/* Perform clipping */
		if (!SDL_IntersectRect(rect, &dst->clip_rect, &clipped)) {
			return 0;
		}
		rect = &clipped;
	} else {
		rect = &dst->clip_rect;
	}

	if (blendMode == SDL_BLENDMODE_BLEND || blendMode == SDL_BLENDMODE_ADD) {
		r = DRAW_MUL(r, a);
		g = DRAW_MUL(g, a);
		b = DRAW_MUL(b, a);
	}

	switch (dst->format->BitsPerPixel) {
		case 15:
			switch (dst->format->Rmask) {
				case 0x7C00:
					return SDL_BlendFillRect_RGB555(dst, rect, blendMode, r, g, b, a);
			}
			break;
		case 16:
			switch (dst->format->Rmask) {
				case 0xF800:
					return SDL_BlendFillRect_RGB565(dst, rect, blendMode, r, g, b, a);
			}
			break;
		case 32:
			switch (dst->format->Rmask) {
				case 0x00FF0000:
					if (!dst->format->Amask) {
						return SDL_BlendFillRect_RGB888(dst, rect, blendMode, r, g, b, a);
					} else {
						return SDL_BlendFillRect_ARGB8888(dst, rect, blendMode, r, g, b, a);
					}
					/* break; -Wunreachable-code-break */
			}
			break;
		default:
			break;
	}

	if (!dst->format->Amask) {
		return SDL_BlendFillRect_RGB(dst, rect, blendMode, r, g, b, a);
	} else {
		return SDL_BlendFillRect_RGBA(dst, rect, blendMode, r, g, b, a);
	}
}

int SDL_BlendFillRects(SDL_Surface *dst, const SDL_Rect *rects, int count, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	SDL_Rect rect;
	int i;
	int (*func)(SDL_Surface * dst, const SDL_Rect *rect, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a) = NULL;
	int status = 0;

	if (!dst) {
		return SDL_InvalidParamError("SDL_BlendFillRects(): dst");
	}

	/* This function doesn't work on surfaces < 8 bpp */
	if (dst->format->BitsPerPixel < 8) {
		return SDL_SetError("SDL_BlendFillRects(): Unsupported surface format");
	}

	if (blendMode == SDL_BLENDMODE_BLEND || blendMode == SDL_BLENDMODE_ADD) {
		r = DRAW_MUL(r, a);
		g = DRAW_MUL(g, a);
		b = DRAW_MUL(b, a);
	}

	/* FIXME: Does this function pointer slow things down significantly? */
	switch (dst->format->BitsPerPixel) {
		case 15:
			switch (dst->format->Rmask) {
				case 0x7C00:
					func = SDL_BlendFillRect_RGB555;
			}
			break;
		case 16:
			switch (dst->format->Rmask) {
				case 0xF800:
					func = SDL_BlendFillRect_RGB565;
			}
			break;
		case 32:
			switch (dst->format->Rmask) {
				case 0x00FF0000:
					if (!dst->format->Amask) {
						func = SDL_BlendFillRect_RGB888;
					} else {
						func = SDL_BlendFillRect_ARGB8888;
					}
					break;
			}
			break;
		default:
			break;
	}

	if (!func) {
		if (!dst->format->Amask) {
			func = SDL_BlendFillRect_RGB;
		} else {
			func = SDL_BlendFillRect_RGBA;
		}
	}

	for (i = 0; i < count; ++i) {
		/* Perform clipping */
		if (!SDL_IntersectRect(&rects[i], &dst->clip_rect, &rect)) {
			continue;
		}
		status = func(dst, &rect, blendMode, r, g, b, a);
	}
	return status;
}
