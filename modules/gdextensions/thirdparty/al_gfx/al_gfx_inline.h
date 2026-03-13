#ifndef PP_DEPTH
#error al-gfx:This file should not be included directly.
#endif

/* Define USE_MEMMOVE to use libc's memmove command for doing blits.
 * This helps some machines, while it doesn't seem to do much for others.
 * Left as a define so the older blit version can be easily reactivated for
 * testing.
 */
#undef USE_MEMMOVE
#define USE_MEMMOVE

/*
 * Linear graphics functions.
 * ==========================
 */

/* _linear_putpixel:
 *  Draws a pixel onto a linear bitmap.
 */
void FUNC_LINEAR_PUTPIXEL(BITMAP *dst, int dx, int dy, int color) {
	ASSERT(dst);

	if (dst->clip && ((dx < dst->cl) || (dx >= dst->cr) || (dy < dst->ct) || (dy >= dst->cb)))
		return;

	if (_drawing_mode == DRAW_MODE_SOLID) {
		PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, dy), dx);
		PUT_PIXEL(d, color);
	} else if (_drawing_mode == DRAW_MODE_XOR) {
		PIXEL_PTR s = OFFSET_PIXEL_PTR(bmp_read_line(dst, dy), dx);
		PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, dy), dx);
		unsigned long c = GET_PIXEL(s) ^ color;
		PUT_PIXEL(d, c);
	} else if (_drawing_mode == DRAW_MODE_TRANS) {
		PIXEL_PTR s = OFFSET_PIXEL_PTR(bmp_read_line(dst, dy), dx);
		PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, dy), dx);
		PP_BLENDER blender = MAKE_PP_BLENDER(color);
		unsigned long c = PP_BLEND(blender, GET_PIXEL(s), color);
		PUT_PIXEL(d, c);
	} else {
		unsigned long c = GET_PATTERN_PIXEL(dx, dy);
		PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, dy), dx);

		if (_drawing_mode == DRAW_MODE_COPY_PATTERN) {
			PUT_PIXEL(d, c);
		} else if (_drawing_mode == DRAW_MODE_SOLID_PATTERN) {
			if (!IS_MASK(c)) {
				PUT_PIXEL(d, color);
			} else {
				PUT_PIXEL(d, c);
			}
		} else if (_drawing_mode == DRAW_MODE_MASKED_PATTERN) {
			if (!IS_MASK(c)) {
				PUT_PIXEL(d, color);
			}
		}
	}
}

/* _linear_getpixel:
 *  Reads a pixel from a linear bitmap.
 */
int FUNC_LINEAR_GETPIXEL(BITMAP *src, int sx, int sy) {
	ASSERT(src);

	if ((sx < 0) || (sx >= src->w) || (sy < 0) || (sy >= src->h)) {
		return -1;
	} else {
		PIXEL_PTR s = OFFSET_PIXEL_PTR(bmp_read_line(src, sy), sx);
		unsigned long c = GET_PIXEL(s);
		return c;
	}
}

/* _linear_hline:
 *  Draws a horizontal line onto a linear bitmap.
 */
void FUNC_LINEAR_HLINE(BITMAP *dst, int dx1, int dy, int dx2, int color) {
	ASSERT(dst);

	if (dx1 > dx2) {
		int tmp = dx1;
		dx1 = dx2;
		dx2 = tmp;
	}
	if (dst->clip) {
		if (dx1 < dst->cl)
			dx1 = dst->cl;
		if (dx2 >= dst->cr)
			dx2 = dst->cr - 1;
		if ((dx1 > dx2) || (dy < dst->ct) || (dy >= dst->cb))
			return;
	}

	int w = dx2 - dx1;

	if (_drawing_mode == DRAW_MODE_SOLID) {
		PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, dy), dx1);
		do {
			PUT_PIXEL(d, color);
			INC_PIXEL_PTR(d);
		} while (--w >= 0);
	} else if (_drawing_mode == DRAW_MODE_XOR) {
		PIXEL_PTR s = OFFSET_PIXEL_PTR(bmp_read_line(dst, dy), dx1);
		PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, dy), dx1);
		do {
			unsigned long c = GET_PIXEL(s) ^ color;
			PUT_PIXEL(d, c);
			INC_PIXEL_PTR(s);
			INC_PIXEL_PTR(d);
		} while (--w >= 0);
	} else if (_drawing_mode == DRAW_MODE_TRANS) {
		PIXEL_PTR s = OFFSET_PIXEL_PTR(bmp_read_line(dst, dy), dx1);
		PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, dy), dx1);
		PP_BLENDER blender = MAKE_PP_BLENDER(color);
		do {
			unsigned long c = PP_BLEND(blender, GET_PIXEL(s), color);
			PUT_PIXEL(d, c);
			INC_PIXEL_PTR(s);
			INC_PIXEL_PTR(d);
		} while (--w >= 0);
	} else {
		PIXEL_PTR sline = PATTERN_LINE(dy);
		PIXEL_PTR s;
		PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, dy), dx1);

		int x = (dx1 - _drawing_x_anchor) & _drawing_x_mask;
		s = OFFSET_PIXEL_PTR(sline, x);
		w++;
		int curw = _drawing_x_mask + 1 - x;
		if (curw > w)
			curw = w;

		if (_drawing_mode == DRAW_MODE_COPY_PATTERN) {
			do {
				w -= curw;
				do {
					unsigned long c = GET_MEMORY_PIXEL(s);
					PUT_PIXEL(d, c);
					INC_PIXEL_PTR(s);
					INC_PIXEL_PTR(d);
				} while (--curw > 0);
				s = sline;
				curw = MIN(w, (int)_drawing_x_mask + 1);
			} while (curw > 0);
		} else if (_drawing_mode == DRAW_MODE_SOLID_PATTERN) {
			do {
				w -= curw;
				do {
					unsigned long c = GET_MEMORY_PIXEL(s);
					if (!IS_MASK(c)) {
						PUT_PIXEL(d, color);
					} else {
						PUT_PIXEL(d, c);
					}
					INC_PIXEL_PTR(s);
					INC_PIXEL_PTR(d);
				} while (--curw > 0);
				s = sline;
				curw = MIN(w, (int)_drawing_x_mask + 1);
			} while (curw > 0);
		} else if (_drawing_mode == DRAW_MODE_MASKED_PATTERN) {
			do {
				w -= curw;
				do {
					unsigned long c = GET_MEMORY_PIXEL(s);
					if (!IS_MASK(c)) {
						PUT_PIXEL(d, color);
					}
					INC_PIXEL_PTR(s);
					INC_PIXEL_PTR(d);
				} while (--curw > 0);
				s = sline;
				curw = MIN(w, (int)_drawing_x_mask + 1);
			} while (curw > 0);
		}
	}
}

/* _linear_vline:
 *  Draws a vertical line onto a linear bitmap.
 */
void FUNC_LINEAR_VLINE(BITMAP *dst, int dx, int dy1, int dy2, int color) {
	ASSERT(dst);

	if (dy1 > dy2) {
		int tmp = dy1;
		dy1 = dy2;
		dy2 = tmp;
	}
	if (dst->clip) {
		if (dy1 < dst->ct)
			dy1 = dst->ct;
		if (dy2 >= dst->cb)
			dy2 = dst->cb - 1;
		if ((dx < dst->cl) || (dx >= dst->cr) || (dy1 > dy2))
			return;
	}

	if (_drawing_mode == DRAW_MODE_SOLID) {
		for (int y = dy1; y <= dy2; y++) {
			PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, y), dx);
			PUT_PIXEL(d, color);
		}
	} else {
		int clip = dst->clip;

		dst->clip = 0;
		for (int y = dy1; y <= dy2; y++) {
			FUNC_LINEAR_PUTPIXEL(dst, dx, y, color);
		}
		dst->clip = clip;
	}
}

/*
 * Bitmap blitting functions.
 * ==========================
 */

#ifdef USE_MEMMOVE
#include <string.h>
#endif

/* _linear_clear_to_color:
 *   Fills a linear bitmp with the specified color.
 */
void FUNC_LINEAR_CLEAR_TO_COLOR(BITMAP *dst, int color) {
	ASSERT(dst);

	int w = dst->cr - dst->cl;

	for (int y = dst->ct; y < dst->cb; y++) {
		PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, y), dst->cl);

		for (int x = w - 1; x >= 0; INC_PIXEL_PTR(d), x--) {
			PUT_PIXEL(d, color);
		}
	}
}

/* _linear_blit:
 *  Normal forward blitting for linear bitmaps.
 */
void FUNC_LINEAR_BLIT(BITMAP *src, BITMAP *dst, int sx, int sy, int dx, int dy, int w, int h) {
	ASSERT(src);
	ASSERT(dst);

	for (int y = 0; y < h; y++) {
		PIXEL_PTR s = OFFSET_PIXEL_PTR(bmp_read_line(src, sy + y), sx);
		PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, dy + y), dx);

#ifndef USE_MEMMOVE
		for (int x = w - 1; x >= 0; INC_PIXEL_PTR(s), INC_PIXEL_PTR(d), x--) {
			unsigned long c = GET_PIXEL(s);
			PUT_PIXEL(d, c);
		}
#else
		memmove(d, s, w * sizeof(*s) * PTR_PER_PIXEL);
#endif
	}
}

/* _linear_blit_backward:
 *  Reverse blitting routine, for overlapping linear bitmaps.
 */
void FUNC_LINEAR_BLIT_BACKWARD(BITMAP *src, BITMAP *dst, int sx, int sy, int dx, int dy, int w, int h) {
	ASSERT(src);
	ASSERT(dst);

	for (int y = h - 1; y >= 0; y--) {
#ifndef USE_MEMMOVE
		PIXEL_PTR s = OFFSET_PIXEL_PTR(bmp_read_line(src, sy + y), sx + w - 1);
		PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, dy + y), dx + w - 1);

		for (int x = w - 1; x >= 0; DEC_PIXEL_PTR(s), DEC_PIXEL_PTR(d), x--) {
			unsigned long c = GET_PIXEL(s);
			PUT_PIXEL(d, c);
		}
#else
		PIXEL_PTR s = OFFSET_PIXEL_PTR(bmp_read_line(src, sy + y), sx);
		PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, dy + y), dx);

		memmove(d, s, w * sizeof(*s) * PTR_PER_PIXEL);
#endif
	}
}

void FUNC_LINEAR_BLIT_END(void) {}

/* _linear_masked_blit:
 *  Masked (skipping transparent pixels) blitting routine for linear bitmaps.
 */
void FUNC_LINEAR_MASKED_BLIT(BITMAP *src, BITMAP *dst, int sx, int sy, int dx, int dy, int w, int h) {
	ASSERT(src);
	ASSERT(dst);

	unsigned long mask_color = bitmap_mask_color(dst);

	for (int y = 0; y < h; y++) {
		PIXEL_PTR s = OFFSET_PIXEL_PTR(bmp_read_line(src, sy + y), sx);
		PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, dy + y), dx);

		for (int x = w - 1; x >= 0; INC_PIXEL_PTR(s), INC_PIXEL_PTR(d), x--) {
			unsigned long c = GET_PIXEL(s);
			if (c != mask_color) {
				PUT_PIXEL(d, c);
			}
		}
	}
}

/*
 * Polygon scanline filler helpers (gouraud shading, tmapping, etc).
 * =================================================================
 */

#ifdef _bma_scan_gcol

/* _poly_scanline_gcol:
 *  Fills a single-color gouraud shaded polygon scanline.
 */
void FUNC_POLY_SCANLINE_GCOL(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	ASSERT(addr);
	ASSERT(info);

	fixed c = info->c;
	fixed dc = info->dc;
	PIXEL_PTR d = (PIXEL_PTR)addr;

	for (int x = w - 1; x >= 0; INC_PIXEL_PTR(d), x--) {
		PUT_PIXEL(d, (c >> 16));
		c += dc;
	}
}

#endif /* _bma_scan_gcol */

/* _poly_scanline_grgb:
 *  Fills an gouraud shaded polygon scanline.
 */
void FUNC_POLY_SCANLINE_GRGB(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	ASSERT(addr);
	ASSERT(info);

	fixed r = info->r;
	fixed g = info->g;
	fixed b = info->b;
	fixed dr = info->dr;
	fixed dg = info->dg;
	fixed db = info->db;
	PIXEL_PTR d = (PIXEL_PTR)addr;

	for (int x = w - 1; x >= 0; INC_PIXEL_PTR(d), x--) {
		PUT_RGB(d, (r >> 16), (g >> 16), (b >> 16));
		r += dr;
		g += dg;
		b += db;
	}
}

/* _poly_scanline_atex:
 *  Fills an affine texture mapped polygon scanline.
 */
void FUNC_POLY_SCANLINE_ATEX(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	ASSERT(addr);
	ASSERT(info);

	int vmask = info->vmask << info->vshift;
	int vshift = 16 - info->vshift;
	int umask = info->umask;
	fixed u = info->u;
	fixed v = info->v;
	fixed du = info->du;
	fixed dv = info->dv;
	PIXEL_PTR texture = (PIXEL_PTR)(info->texture);
	PIXEL_PTR d = (PIXEL_PTR)addr;

	for (int x = w - 1; x >= 0; INC_PIXEL_PTR(d), x--) {
		PIXEL_PTR s = OFFSET_PIXEL_PTR(texture, ((v >> vshift) & vmask) + ((u >> 16) & umask));
		unsigned long color = GET_MEMORY_PIXEL(s);

		PUT_PIXEL(d, color);
		u += du;
		v += dv;
	}
}

/* _poly_scanline_atex_mask:
 *  Fills a masked affine texture mapped polygon scanline.
 */
void FUNC_POLY_SCANLINE_ATEX_MASK(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	ASSERT(addr);
	ASSERT(info);

	int vmask = info->vmask << info->vshift;
	int vshift = 16 - info->vshift;
	int umask = info->umask;
	fixed u = info->u;
	fixed v = info->v;
	fixed du = info->du;
	fixed dv = info->dv;
	PIXEL_PTR texture = (PIXEL_PTR)(info->texture);
	PIXEL_PTR d = (PIXEL_PTR)addr;

	for (int x = w - 1; x >= 0; INC_PIXEL_PTR(d), x--) {
		PIXEL_PTR s = OFFSET_PIXEL_PTR(texture, ((v >> vshift) & vmask) + ((u >> 16) & umask));
		unsigned long color = GET_MEMORY_PIXEL(s);

		if (!IS_MASK(color)) {
			PUT_PIXEL(d, color);
		}
		u += du;
		v += dv;
	}
}

/* _poly_scanline_atex_lit:
 *  Fills a lit affine texture mapped polygon scanline.
 */
void FUNC_POLY_SCANLINE_ATEX_LIT(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	ASSERT(addr);
	ASSERT(info);

	int vmask = info->vmask << info->vshift;
	int vshift = 16 - info->vshift;
	int umask = info->umask;
	fixed u = info->u;
	fixed v = info->v;
	fixed c = info->c;
	fixed du = info->du;
	fixed dv = info->dv;
	fixed dc = info->dc;
	PS_BLENDER blender = MAKE_PS_BLENDER();
	PIXEL_PTR texture = (PIXEL_PTR)(info->texture);
	PIXEL_PTR d = (PIXEL_PTR)addr;

	for (int x = w - 1; x >= 0; INC_PIXEL_PTR(d), x--) {
		PIXEL_PTR s = OFFSET_PIXEL_PTR(texture, ((v >> vshift) & vmask) + ((u >> 16) & umask));
		unsigned long color = GET_MEMORY_PIXEL(s);
		color = PS_BLEND(blender, (c >> 16), color);

		PUT_PIXEL(d, color);
		u += du;
		v += dv;
		c += dc;
	}
}

/* _poly_scanline_atex_mask_lit:
 *  Fills a masked lit affine texture mapped polygon scanline.
 */
void FUNC_POLY_SCANLINE_ATEX_MASK_LIT(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	int vmask, vshift, umask;
	fixed u, v, c, du, dv, dc;
	PS_BLENDER blender;
	PIXEL_PTR texture;
	PIXEL_PTR d;

	ASSERT(addr);
	ASSERT(info);

	vmask = info->vmask << info->vshift;
	vshift = 16 - info->vshift;
	umask = info->umask;
	u = info->u;
	v = info->v;
	c = info->c;
	du = info->du;
	dv = info->dv;
	dc = info->dc;
	blender = MAKE_PS_BLENDER();
	texture = (PIXEL_PTR)(info->texture);
	d = (PIXEL_PTR)addr;

	for (int x = w - 1; x >= 0; INC_PIXEL_PTR(d), x--) {
		PIXEL_PTR s = OFFSET_PIXEL_PTR(texture, ((v >> vshift) & vmask) + ((u >> 16) & umask));
		unsigned long color = GET_MEMORY_PIXEL(s);

		if (!IS_MASK(color)) {
			color = PS_BLEND(blender, (c >> 16), color);
			PUT_PIXEL(d, color);
		}
		u += du;
		v += dv;
		c += dc;
	}
}

/* _poly_scanline_ptex:
 *  Fills a perspective correct texture mapped polygon scanline.
 */
void FUNC_POLY_SCANLINE_PTEX(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	int x, i, imax = 3;
	int vmask, vshift, umask;
	double fu, fv, fz, dfu, dfv, dfz, z1;
	PIXEL_PTR texture;
	PIXEL_PTR d;
	int64_t u, v;

	ASSERT(addr);
	ASSERT(info);

	vmask = info->vmask << info->vshift;
	vshift = 16 - info->vshift;
	umask = info->umask;
	fu = info->fu;
	fv = info->fv;
	fz = info->z;
	dfu = info->dfu * 4;
	dfv = info->dfv * 4;
	dfz = info->dz * 4;
	z1 = 1. / fz;
	texture = (PIXEL_PTR)(info->texture);
	d = (PIXEL_PTR)addr;
	u = fu * z1;
	v = fv * z1;

	/* update depth */
	fz += dfz;
	z1 = 1. / fz;

	for (x = w - 1; x >= 0; x -= 4) {
		int64_t nextu, nextv, du, dv;
		PIXEL_PTR s;
		unsigned long color;

		fu += dfu;
		fv += dfv;
		fz += dfz;
		nextu = fu * z1;
		nextv = fv * z1;
		z1 = 1. / fz;
		du = (nextu - u) >> 2;
		dv = (nextv - v) >> 2;

		/* scanline subdivision */
		if (x < 3)
			imax = x;
		for (i = imax; i >= 0; i--, INC_PIXEL_PTR(d)) {
			s = OFFSET_PIXEL_PTR(texture, ((v >> vshift) & vmask) + ((u >> 16) & umask));
			color = GET_MEMORY_PIXEL(s);

			PUT_PIXEL(d, color);
			u += du;
			v += dv;
		}
	}
}

/* _poly_scanline_ptex_mask:
 *  Fills a masked perspective correct texture mapped polygon scanline.
 */
void FUNC_POLY_SCANLINE_PTEX_MASK(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	int imax = 3;
	int vmask, vshift, umask;
	double fu, fv, fz, dfu, dfv, dfz, z1;
	PIXEL_PTR texture;
	PIXEL_PTR d;
	int64_t u, v;

	ASSERT(addr);
	ASSERT(info);

	vmask = info->vmask << info->vshift;
	vshift = 16 - info->vshift;
	umask = info->umask;
	fu = info->fu;
	fv = info->fv;
	fz = info->z;
	dfu = info->dfu * 4;
	dfv = info->dfv * 4;
	dfz = info->dz * 4;
	z1 = 1. / fz;
	texture = (PIXEL_PTR)(info->texture);
	d = (PIXEL_PTR)addr;
	u = fu * z1;
	v = fv * z1;

	/* update depth */
	fz += dfz;
	z1 = 1. / fz;

	for (int x = w - 1; x >= 0; x -= 4) {
		int64_t nextu, nextv, du, dv;
		PIXEL_PTR s;
		unsigned long color;

		fu += dfu;
		fv += dfv;
		fz += dfz;
		nextu = fu * z1;
		nextv = fv * z1;
		z1 = 1. / fz;
		du = (nextu - u) >> 2;
		dv = (nextv - v) >> 2;

		/* scanline subdivision */
		if (x < 3)
			imax = x;
		for (int i = imax; i >= 0; i--, INC_PIXEL_PTR(d)) {
			s = OFFSET_PIXEL_PTR(texture, ((v >> vshift) & vmask) + ((u >> 16) & umask));
			color = GET_MEMORY_PIXEL(s);

			if (!IS_MASK(color)) {
				PUT_PIXEL(d, color);
			}
			u += du;
			v += dv;
		}
	}
}

/* _poly_scanline_ptex_lit:
 *  Fills a lit perspective correct texture mapped polygon scanline.
 */
void FUNC_POLY_SCANLINE_PTEX_LIT(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	int imax = 3;
	int vmask, vshift, umask;
	fixed c, dc;
	double fu, fv, fz, dfu, dfv, dfz, z1;
	PS_BLENDER blender;
	PIXEL_PTR texture;
	PIXEL_PTR d;
	int64_t u, v;

	ASSERT(addr);
	ASSERT(info);

	vmask = info->vmask << info->vshift;
	vshift = 16 - info->vshift;
	umask = info->umask;
	c = info->c;
	dc = info->dc;
	fu = info->fu;
	fv = info->fv;
	fz = info->z;
	dfu = info->dfu * 4;
	dfv = info->dfv * 4;
	dfz = info->dz * 4;
	z1 = 1. / fz;
	blender = MAKE_PS_BLENDER();
	texture = (PIXEL_PTR)(info->texture);
	d = (PIXEL_PTR)addr;
	u = fu * z1;
	v = fv * z1;

	/* update depth */
	fz += dfz;
	z1 = 1. / fz;

	for (int x = w - 1; x >= 0; x -= 4) {
		int64_t nextu, nextv, du, dv;

		fu += dfu;
		fv += dfv;
		fz += dfz;
		nextu = fu * z1;
		nextv = fv * z1;
		z1 = 1. / fz;
		du = (nextu - u) >> 2;
		dv = (nextv - v) >> 2;

		/* scanline subdivision */
		if (x < 3)
			imax = x;
		for (int i = imax; i >= 0; i--, INC_PIXEL_PTR(d)) {
			PIXEL_PTR s = OFFSET_PIXEL_PTR(texture, ((v >> vshift) & vmask) + ((u >> 16) & umask));
			unsigned long color = GET_MEMORY_PIXEL(s);
			color = PS_BLEND(blender, (c >> 16), color);

			PUT_PIXEL(d, color);
			u += du;
			v += dv;
			c += dc;
		}
	}
}

/* _poly_scanline_ptex_mask_lit:
 *  Fills a masked lit perspective correct texture mapped polygon scanline.
 */
void FUNC_POLY_SCANLINE_PTEX_MASK_LIT(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	int imax = 3;
	int vmask, vshift, umask;
	fixed c, dc;
	double fu, fv, fz, dfu, dfv, dfz, z1;
	PS_BLENDER blender;
	PIXEL_PTR texture;
	PIXEL_PTR d;
	int64_t u, v;

	ASSERT(addr);
	ASSERT(info);

	vmask = info->vmask << info->vshift;
	vshift = 16 - info->vshift;
	umask = info->umask;
	c = info->c;
	dc = info->dc;
	fu = info->fu;
	fv = info->fv;
	fz = info->z;
	dfu = info->dfu * 4;
	dfv = info->dfv * 4;
	dfz = info->dz * 4;
	z1 = 1. / fz;
	blender = MAKE_PS_BLENDER();
	texture = (PIXEL_PTR)(info->texture);
	d = (PIXEL_PTR)addr;
	u = fu * z1;
	v = fv * z1;

	/* update depth */
	fz += dfz;
	z1 = 1. / fz;

	for (int x = w - 1; x >= 0; x -= 4) {
		int64_t nextu, nextv, du, dv;

		fu += dfu;
		fv += dfv;
		fz += dfz;
		nextu = fu * z1;
		nextv = fv * z1;
		z1 = 1. / fz;
		du = (nextu - u) >> 2;
		dv = (nextv - v) >> 2;

		/* scanline subdivision */
		if (x < 3)
			imax = x;
		for (int i = imax; i >= 0; i--, INC_PIXEL_PTR(d)) {
			PIXEL_PTR s = OFFSET_PIXEL_PTR(texture, ((v >> vshift) & vmask) + ((u >> 16) & umask));
			unsigned long color = GET_MEMORY_PIXEL(s);

			if (!IS_MASK(color)) {
				color = PS_BLEND(blender, (c >> 16), color);
				PUT_PIXEL(d, color);
			}
			u += du;
			v += dv;
			c += dc;
		}
	}
}

/* _poly_scanline_atex_trans:
 *  Fills a trans affine texture mapped polygon scanline.
 */
void FUNC_POLY_SCANLINE_ATEX_TRANS(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	int vmask, vshift, umask;
	fixed u, v, du, dv;
	PIXEL_PTR texture;
	PIXEL_PTR d;
	PIXEL_PTR r;
	PS_BLENDER blender;

	ASSERT(addr);
	ASSERT(info);

	vmask = info->vmask << info->vshift;
	vshift = 16 - info->vshift;
	umask = info->umask;
	u = info->u;
	v = info->v;
	du = info->du;
	dv = info->dv;
	blender = MAKE_PS_BLENDER();
	texture = (PIXEL_PTR)(info->texture);
	d = (PIXEL_PTR)addr;
	r = (PIXEL_PTR)info->read_addr;

	for (int x = w - 1; x >= 0; INC_PIXEL_PTR(d), INC_PIXEL_PTR(r), x--) {
		PIXEL_PTR s = OFFSET_PIXEL_PTR(texture, ((v >> vshift) & vmask) + ((u >> 16) & umask));
		unsigned long color = GET_MEMORY_PIXEL(s);
		color = PS_ALPHA_BLEND(blender, color, GET_PIXEL(r));

		PUT_PIXEL(d, color);
		u += du;
		v += dv;
	}
}

/* _poly_scanline_atex_mask_trans:
 *  Fills a trans masked affine texture mapped polygon scanline.
 */
void FUNC_POLY_SCANLINE_ATEX_MASK_TRANS(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	int vmask, vshift, umask;
	fixed u, v, du, dv;
	PIXEL_PTR texture;
	PIXEL_PTR d;
	PIXEL_PTR r;
	PS_BLENDER blender;

	ASSERT(addr);
	ASSERT(info);

	vmask = info->vmask << info->vshift;
	vshift = 16 - info->vshift;
	umask = info->umask;
	u = info->u;
	v = info->v;
	du = info->du;
	dv = info->dv;
	blender = MAKE_PS_BLENDER();
	texture = (PIXEL_PTR)(info->texture);
	d = (PIXEL_PTR)addr;
	r = (PIXEL_PTR)info->read_addr;

	for (int x = w - 1; x >= 0; INC_PIXEL_PTR(d), INC_PIXEL_PTR(r), x--) {
		PIXEL_PTR s = OFFSET_PIXEL_PTR(texture, ((v >> vshift) & vmask) + ((u >> 16) & umask));
		unsigned long color = GET_MEMORY_PIXEL(s);

		if (!IS_MASK(color)) {
			color = PS_ALPHA_BLEND(blender, color, GET_PIXEL(r));
			PUT_PIXEL(d, color);
		}
		u += du;
		v += dv;
	}
}

/* _poly_scanline_ptex_trans:
 *  Fills a trans perspective correct texture mapped polygon scanline.
 */
void FUNC_POLY_SCANLINE_PTEX_TRANS(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	int imax = 3;
	int vmask, vshift, umask;
	double fu, fv, fz, dfu, dfv, dfz, z1;
	PS_BLENDER blender;
	PIXEL_PTR texture;
	PIXEL_PTR d;
	PIXEL_PTR r;
	int64_t u, v;

	ASSERT(addr);
	ASSERT(info);

	vmask = info->vmask << info->vshift;
	vshift = 16 - info->vshift;
	umask = info->umask;
	fu = info->fu;
	fv = info->fv;
	fz = info->z;
	dfu = info->dfu * 4;
	dfv = info->dfv * 4;
	dfz = info->dz * 4;
	z1 = 1. / fz;
	blender = MAKE_PS_BLENDER();
	texture = (PIXEL_PTR)(info->texture);
	d = (PIXEL_PTR)addr;
	r = (PIXEL_PTR)info->read_addr;
	u = fu * z1;
	v = fv * z1;

	/* update depth */
	fz += dfz;
	z1 = 1. / fz;

	for (int x = w - 1; x >= 0; x -= 4) {
		int64_t nextu, nextv, du, dv;

		fu += dfu;
		fv += dfv;
		fz += dfz;
		nextu = fu * z1;
		nextv = fv * z1;
		z1 = 1. / fz;
		du = (nextu - u) >> 2;
		dv = (nextv - v) >> 2;

		/* scanline subdivision */
		if (x < 3)
			imax = x;
		for (int i = imax; i >= 0; i--, INC_PIXEL_PTR(d), INC_PIXEL_PTR(r)) {
			PIXEL_PTR s = OFFSET_PIXEL_PTR(texture, ((v >> vshift) & vmask) + ((u >> 16) & umask));
			unsigned long color = GET_MEMORY_PIXEL(s);

			color = PS_ALPHA_BLEND(blender, color, GET_PIXEL(r));
			PUT_PIXEL(d, color);
			u += du;
			v += dv;
		}
	}
}

/* _poly_scanline_ptex_mask_trans:
 *  Fills a trans masked perspective correct texture mapped polygon scanline.
 */
void FUNC_POLY_SCANLINE_PTEX_MASK_TRANS(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	int imax = 3;
	int vmask, vshift, umask;
	double fu, fv, fz, dfu, dfv, dfz, z1;
	PS_BLENDER blender;
	PIXEL_PTR texture;
	PIXEL_PTR d;
	PIXEL_PTR r;
	int64_t u, v;

	ASSERT(addr);
	ASSERT(info);

	vmask = info->vmask << info->vshift;
	vshift = 16 - info->vshift;
	umask = info->umask;
	fu = info->fu;
	fv = info->fv;
	fz = info->z;
	dfu = info->dfu * 4;
	dfv = info->dfv * 4;
	dfz = info->dz * 4;
	z1 = 1. / fz;
	blender = MAKE_PS_BLENDER();
	texture = (PIXEL_PTR)(info->texture);
	d = (PIXEL_PTR)addr;
	r = (PIXEL_PTR)info->read_addr;
	u = fu * z1;
	v = fv * z1;

	/* update depth */
	fz += dfz;
	z1 = 1. / fz;

	for (int x = w - 1; x >= 0; x -= 4) {
		int64_t nextu, nextv, du, dv;

		fu += dfu;
		fv += dfv;
		fz += dfz;
		nextu = fu * z1;
		nextv = fv * z1;
		z1 = 1. / fz;
		du = (nextu - u) >> 2;
		dv = (nextv - v) >> 2;

		/* scanline subdivision */
		if (x < 3)
			imax = x;
		for (int i = imax; i >= 0; i--, INC_PIXEL_PTR(d), INC_PIXEL_PTR(r)) {
			PIXEL_PTR s = OFFSET_PIXEL_PTR(texture, ((v >> vshift) & vmask) + ((u >> 16) & umask));
			unsigned long color = GET_MEMORY_PIXEL(s);

			if (!IS_MASK(color)) {
				color = PS_ALPHA_BLEND(blender, color, GET_PIXEL(r));
				PUT_PIXEL(d, color);
			}
			u += du;
			v += dv;
		}
	}
}

/*
 * Z-buffered polygon filler helpers (gouraud shading, tmapping, etc).
 * ===================================================================
 */

#undef ZBUF_PTR
#define ZBUF_PTR float *

/* _poly_zbuf_flat:
 *  Fills a single-color polygon scanline.
 */
void FUNC_POLY_ZBUF_FLAT(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	ASSERT(addr);
	ASSERT(info);

	float z = info->z;
	unsigned long c = info->c;
	PIXEL_PTR d = (PIXEL_PTR)addr;
	ZBUF_PTR zb = (ZBUF_PTR)info->zbuf_addr;

	for (int x = w - 1; x >= 0; INC_PIXEL_PTR(d), x--) {
		if (*zb < z) {
			PUT_PIXEL(d, c);
			*zb = z;
		}
		zb++;
		z += info->dz;
	}
}

#ifdef _bma_zbuf_gcol

/* _poly_zbuf_gcol:
 *  Fills a single-color gouraud shaded polygon scanline.
 */
void FUNC_POLY_ZBUF_GCOL(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	ASSERT(addr);
	ASSERT(info);

	float z = info->z;
	fixed c = info->c;
	fixed dc = info->dc;
	PIXEL_PTR d = (PIXEL_PTR)addr;
	ZBUF_PTR zb = (ZBUF_PTR)info->zbuf_addr;

	for (int x = w - 1; x >= 0; INC_PIXEL_PTR(d), x--) {
		if (*zb < z) {
			PUT_PIXEL(d, (c >> 16));
			*zb = z;
		}
		c += dc;
		zb++;
		z += info->dz;
	}
}

#endif /* _bma_zbuf_gcol */

/* _poly_zbuf_grgb:
 *  Fills an gouraud shaded polygon scanline.
 */
void FUNC_POLY_ZBUF_GRGB(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	fixed r, g, b, dr, dg, db;
	PIXEL_PTR d;
	float z;
	ZBUF_PTR zb;

	ASSERT(addr);
	ASSERT(info);

	r = info->r;
	g = info->g;
	b = info->b;
	dr = info->dr;
	dg = info->dg;
	db = info->db;
	d = (PIXEL_PTR)addr;
	z = info->z;
	zb = (ZBUF_PTR)info->zbuf_addr;

	for (int x = w - 1; x >= 0; INC_PIXEL_PTR(d), x--) {
		if (*zb < z) {
			PUT_RGB(d, (r >> 16), (g >> 16), (b >> 16));
			*zb = z;
		}
		r += dr;
		g += dg;
		b += db;
		zb++;
		z += info->dz;
	}
}

/* _poly_zbuf_atex:
 *  Fills an affine texture mapped polygon scanline.
 */
void FUNC_POLY_ZBUF_ATEX(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	int vmask, vshift, umask;
	fixed u, v, du, dv;
	PIXEL_PTR texture;
	PIXEL_PTR d;
	float z;
	ZBUF_PTR zb;

	ASSERT(addr);
	ASSERT(info);

	vmask = info->vmask << info->vshift;
	vshift = 16 - info->vshift;
	umask = info->umask;
	u = info->u;
	v = info->v;
	du = info->du;
	dv = info->dv;
	texture = (PIXEL_PTR)(info->texture);
	d = (PIXEL_PTR)addr;
	z = info->z;
	zb = (ZBUF_PTR)info->zbuf_addr;

	for (int x = w - 1; x >= 0; INC_PIXEL_PTR(d), x--) {
		if (*zb < z) {
			PIXEL_PTR s = OFFSET_PIXEL_PTR(texture, ((v >> vshift) & vmask) + ((u >> 16) & umask));
			unsigned long color = GET_MEMORY_PIXEL(s);

			PUT_PIXEL(d, color);
			*zb = z;
		}
		u += du;
		v += dv;
		zb++;
		z += info->dz;
	}
}

/* _poly_zbuf_atex_mask:
 *  Fills a masked affine texture mapped polygon scanline.
 */
void FUNC_POLY_ZBUF_ATEX_MASK(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	int vmask, vshift, umask;
	fixed u, v, du, dv;
	PIXEL_PTR texture;
	PIXEL_PTR d;
	float z;
	ZBUF_PTR zb;

	ASSERT(addr);
	ASSERT(info);

	vmask = info->vmask << info->vshift;
	vshift = 16 - info->vshift;
	umask = info->umask;
	u = info->u;
	v = info->v;
	du = info->du;
	dv = info->dv;
	texture = (PIXEL_PTR)(info->texture);
	d = (PIXEL_PTR)addr;
	z = info->z;
	zb = (ZBUF_PTR)info->zbuf_addr;

	for (int x = w - 1; x >= 0; INC_PIXEL_PTR(d), x--) {
		if (*zb < z) {
			PIXEL_PTR s = OFFSET_PIXEL_PTR(texture, ((v >> vshift) & vmask) + ((u >> 16) & umask));
			unsigned long color = GET_MEMORY_PIXEL(s);

			if (!IS_MASK(color)) {
				PUT_PIXEL(d, color);
				*zb = z;
			}
		}
		u += du;
		v += dv;
		zb++;
		z += info->dz;
	}
}

/* _poly_zbuf_atex_lit:
 *  Fills a lit affine texture mapped polygon scanline.
 */
void FUNC_POLY_ZBUF_ATEX_LIT(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	int vmask, vshift, umask;
	fixed u, v, c, du, dv, dc;
	PIXEL_PTR texture;
	PIXEL_PTR d;
	PS_BLENDER blender;
	float z;
	ZBUF_PTR zb;

	ASSERT(addr);
	ASSERT(info);

	vmask = info->vmask << info->vshift;
	vshift = 16 - info->vshift;
	umask = info->umask;
	u = info->u;
	v = info->v;
	c = info->c;
	du = info->du;
	dv = info->dv;
	dc = info->dc;
	blender = MAKE_PS_BLENDER();
	texture = (PIXEL_PTR)(info->texture);
	d = (PIXEL_PTR)addr;
	z = info->z;
	zb = (ZBUF_PTR)info->zbuf_addr;

	for (int x = w - 1; x >= 0; INC_PIXEL_PTR(d), x--) {
		if (*zb < z) {
			PIXEL_PTR s = OFFSET_PIXEL_PTR(texture, ((v >> vshift) & vmask) + ((u >> 16) & umask));
			unsigned long color = GET_MEMORY_PIXEL(s);
			color = PS_BLEND(blender, (c >> 16), color);

			PUT_PIXEL(d, color);
			*zb = z;
		}
		u += du;
		v += dv;
		c += dc;
		zb++;
		z += info->dz;
	}
}

/* _poly_zbuf_atex_mask_lit:
 *  Fills a masked lit affine texture mapped polygon scanline.
 */
void FUNC_POLY_ZBUF_ATEX_MASK_LIT(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	int vmask, vshift, umask;
	fixed u, v, c, du, dv, dc;
	PIXEL_PTR texture;
	PIXEL_PTR d;
	PS_BLENDER blender;
	float z;
	ZBUF_PTR zb;

	ASSERT(addr);
	ASSERT(info);

	vmask = info->vmask << info->vshift;
	vshift = 16 - info->vshift;
	umask = info->umask;
	u = info->u;
	v = info->v;
	c = info->c;
	du = info->du;
	dv = info->dv;
	dc = info->dc;
	blender = MAKE_PS_BLENDER();
	texture = (PIXEL_PTR)(info->texture);
	d = (PIXEL_PTR)addr;
	z = info->z;
	zb = (ZBUF_PTR)info->zbuf_addr;

	for (int x = w - 1; x >= 0; INC_PIXEL_PTR(d), x--) {
		if (*zb < z) {
			PIXEL_PTR s = OFFSET_PIXEL_PTR(texture, ((v >> vshift) & vmask) + ((u >> 16) & umask));
			unsigned long color = GET_MEMORY_PIXEL(s);

			if (!IS_MASK(color)) {
				color = PS_BLEND(blender, (c >> 16), color);
				PUT_PIXEL(d, color);
				*zb = z;
			}
		}
		u += du;
		v += dv;
		c += dc;
		zb++;
		z += info->dz;
	}
}

/* _poly_zbuf_ptex:
 *  Fills a perspective correct texture mapped polygon scanline.
 */
void FUNC_POLY_ZBUF_PTEX(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	int vmask, vshift, umask;
	double fu, fv, fz, dfu, dfv, dfz;
	PIXEL_PTR texture;
	PIXEL_PTR d;
	ZBUF_PTR zb;

	ASSERT(addr);
	ASSERT(info);

	vmask = info->vmask << info->vshift;
	vshift = 16 - info->vshift;
	umask = info->umask;
	fu = info->fu;
	fv = info->fv;
	fz = info->z;
	dfu = info->dfu;
	dfv = info->dfv;
	dfz = info->dz;
	texture = (PIXEL_PTR)(info->texture);
	d = (PIXEL_PTR)addr;
	zb = (ZBUF_PTR)info->zbuf_addr;

	for (int x = w - 1; x >= 0; INC_PIXEL_PTR(d), x--) {
		if (*zb < fz) {
			long u = fu / fz;
			long v = fv / fz;
			PIXEL_PTR s = OFFSET_PIXEL_PTR(texture, ((v >> vshift) & vmask) + ((u >> 16) & umask));
			unsigned long color = GET_MEMORY_PIXEL(s);

			PUT_PIXEL(d, color);
			*zb = (float)fz;
		}
		fu += dfu;
		fv += dfv;
		fz += dfz;
		zb++;
	}
}

/* _poly_zbuf_ptex_mask:
 *  Fills a masked perspective correct texture mapped polygon scanline.
 */
void FUNC_POLY_ZBUF_PTEX_MASK(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	int vmask, vshift, umask;
	double fu, fv, fz, dfu, dfv, dfz;
	PIXEL_PTR texture;
	PIXEL_PTR d;
	ZBUF_PTR zb;

	ASSERT(addr);
	ASSERT(info);

	vmask = info->vmask << info->vshift;
	vshift = 16 - info->vshift;
	umask = info->umask;
	fu = info->fu;
	fv = info->fv;
	fz = info->z;
	dfu = info->dfu;
	dfv = info->dfv;
	dfz = info->dz;
	texture = (PIXEL_PTR)(info->texture);
	d = (PIXEL_PTR)addr;
	zb = (ZBUF_PTR)info->zbuf_addr;

	for (int x = w - 1; x >= 0; INC_PIXEL_PTR(d), x--) {
		if (*zb < fz) {
			long u = fu / fz;
			long v = fv / fz;
			PIXEL_PTR s = OFFSET_PIXEL_PTR(texture, ((v >> vshift) & vmask) + ((u >> 16) & umask));
			unsigned long color = GET_MEMORY_PIXEL(s);

			if (!IS_MASK(color)) {
				PUT_PIXEL(d, color);
				*zb = (float)fz;
			}
		}
		fu += dfu;
		fv += dfv;
		fz += dfz;
		zb++;
	}
}

/* _poly_zbuf_ptex_lit:
 *  Fills a lit perspective correct texture mapped polygon scanline.
 */
void FUNC_POLY_ZBUF_PTEX_LIT(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	int vmask, vshift, umask;
	fixed c, dc;
	double fu, fv, fz, dfu, dfv, dfz;
	PS_BLENDER blender;
	PIXEL_PTR texture;
	PIXEL_PTR d;
	ZBUF_PTR zb;

	ASSERT(addr);
	ASSERT(info);

	vmask = info->vmask << info->vshift;
	vshift = 16 - info->vshift;
	umask = info->umask;
	c = info->c;
	dc = info->dc;
	fu = info->fu;
	fv = info->fv;
	fz = info->z;
	dfu = info->dfu;
	dfv = info->dfv;
	dfz = info->dz;
	blender = MAKE_PS_BLENDER();
	texture = (PIXEL_PTR)(info->texture);
	d = (PIXEL_PTR)addr;
	zb = (ZBUF_PTR)info->zbuf_addr;

	for (int x = w - 1; x >= 0; INC_PIXEL_PTR(d), x--) {
		if (*zb < fz) {
			long u = fu / fz;
			long v = fv / fz;
			PIXEL_PTR s = OFFSET_PIXEL_PTR(texture, ((v >> vshift) & vmask) + ((u >> 16) & umask));
			unsigned long color = GET_MEMORY_PIXEL(s);
			color = PS_BLEND(blender, (c >> 16), color);

			PUT_PIXEL(d, color);
			*zb = (float)fz;
		}
		fu += dfu;
		fv += dfv;
		fz += dfz;
		c += dc;
		zb++;
	}
}

/* _poly_zbuf_ptex_mask_lit:
 *  Fills a masked lit perspective correct texture mapped polygon scanline.
 */
void FUNC_POLY_ZBUF_PTEX_MASK_LIT(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	int vmask, vshift, umask;
	fixed c, dc;
	double fu, fv, fz, dfu, dfv, dfz;
	PS_BLENDER blender;
	PIXEL_PTR texture;
	PIXEL_PTR d;
	ZBUF_PTR zb;

	ASSERT(addr);
	ASSERT(info);

	vmask = info->vmask << info->vshift;
	vshift = 16 - info->vshift;
	umask = info->umask;
	c = info->c;
	dc = info->dc;
	fu = info->fu;
	fv = info->fv;
	fz = info->z;
	dfu = info->dfu;
	dfv = info->dfv;
	dfz = info->dz;
	blender = MAKE_PS_BLENDER();
	texture = (PIXEL_PTR)(info->texture);
	d = (PIXEL_PTR)addr;
	zb = (ZBUF_PTR)info->zbuf_addr;

	for (int x = w - 1; x >= 0; INC_PIXEL_PTR(d), x--) {
		if (*zb < fz) {
			long u = fu / fz;
			long v = fv / fz;
			PIXEL_PTR s = OFFSET_PIXEL_PTR(texture, ((v >> vshift) & vmask) + ((u >> 16) & umask));
			unsigned long color = GET_MEMORY_PIXEL(s);

			if (!IS_MASK(color)) {
				color = PS_BLEND(blender, (c >> 16), color);
				PUT_PIXEL(d, color);
				*zb = (float)fz;
			}
		}
		fu += dfu;
		fv += dfv;
		fz += dfz;
		c += dc;
		zb++;
	}
}

/* _poly_zbuf_atex_trans:
 *  Fills a trans affine texture mapped polygon scanline.
 */
void FUNC_POLY_ZBUF_ATEX_TRANS(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	int x;
	int vmask, vshift, umask;
	fixed u, v, du, dv;
	PS_BLENDER blender;
	PIXEL_PTR texture;
	PIXEL_PTR d;
	PIXEL_PTR r;
	float z;
	ZBUF_PTR zb;

	ASSERT(addr);
	ASSERT(info);

	vmask = info->vmask << info->vshift;
	vshift = 16 - info->vshift;
	umask = info->umask;
	u = info->u;
	v = info->v;
	du = info->du;
	dv = info->dv;
	blender = MAKE_PS_BLENDER();
	texture = (PIXEL_PTR)(info->texture);
	d = (PIXEL_PTR)addr;
	r = (PIXEL_PTR)info->read_addr;
	z = info->z;
	zb = (ZBUF_PTR)info->zbuf_addr;

	for (x = w - 1; x >= 0; INC_PIXEL_PTR(d), INC_PIXEL_PTR(r), x--) {
		if (*zb < z) {
			PIXEL_PTR s = OFFSET_PIXEL_PTR(texture, ((v >> vshift) & vmask) + ((u >> 16) & umask));
			unsigned long color = GET_MEMORY_PIXEL(s);
			color = PS_ALPHA_BLEND(blender, color, GET_PIXEL(r));

			PUT_PIXEL(d, color);
			*zb = z;
		}
		u += du;
		v += dv;
		zb++;
		z += info->dz;
	}
}

/* _poly_zbuf_atex_mask_trans:
 *  Fills a trans masked affine texture mapped polygon scanline.
 */
void FUNC_POLY_ZBUF_ATEX_MASK_TRANS(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	int vmask, vshift, umask;
	fixed u, v, du, dv;
	PS_BLENDER blender;
	PIXEL_PTR texture;
	PIXEL_PTR d;
	PIXEL_PTR r;
	float z;
	ZBUF_PTR zb;

	ASSERT(addr);
	ASSERT(info);

	vmask = info->vmask << info->vshift;
	vshift = 16 - info->vshift;
	umask = info->umask;
	u = info->u;
	v = info->v;
	du = info->du;
	dv = info->dv;
	blender = MAKE_PS_BLENDER();
	texture = (PIXEL_PTR)(info->texture);
	d = (PIXEL_PTR)addr;
	r = (PIXEL_PTR)info->read_addr;
	z = info->z;
	zb = (ZBUF_PTR)info->zbuf_addr;

	for (int x = w - 1; x >= 0; INC_PIXEL_PTR(d), INC_PIXEL_PTR(r), x--) {
		if (*zb < z) {
			PIXEL_PTR s = OFFSET_PIXEL_PTR(texture, ((v >> vshift) & vmask) + ((u >> 16) & umask));
			unsigned long color = GET_MEMORY_PIXEL(s);
			if (!IS_MASK(color)) {
				color = PS_ALPHA_BLEND(blender, color, GET_PIXEL(r));
				PUT_PIXEL(d, color);
				*zb = z;
			}
		}
		u += du;
		v += dv;
		zb++;
		z += info->dz;
	}
}

/* _poly_zbuf_ptex_trans:
 *  Fills a trans perspective correct texture mapped polygon scanline.
 */
void FUNC_POLY_ZBUF_PTEX_TRANS(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	int vmask, vshift, umask;
	double fu, fv, fz, dfu, dfv, dfz;
	PS_BLENDER blender;
	PIXEL_PTR texture;
	PIXEL_PTR d;
	PIXEL_PTR r;
	ZBUF_PTR zb;

	ASSERT(addr);
	ASSERT(info);

	vmask = info->vmask << info->vshift;
	vshift = 16 - info->vshift;
	umask = info->umask;
	fu = info->fu;
	fv = info->fv;
	fz = info->z;
	dfu = info->dfu;
	dfv = info->dfv;
	dfz = info->dz;
	blender = MAKE_PS_BLENDER();
	texture = (PIXEL_PTR)(info->texture);
	d = (PIXEL_PTR)addr;
	r = (PIXEL_PTR)info->read_addr;
	zb = (ZBUF_PTR)info->zbuf_addr;

	for (int x = w - 1; x >= 0; INC_PIXEL_PTR(d), INC_PIXEL_PTR(r), x--) {
		if (*zb < fz) {
			long u = fu / fz;
			long v = fv / fz;
			PIXEL_PTR s = OFFSET_PIXEL_PTR(texture, ((v >> vshift) & vmask) + ((u >> 16) & umask));
			unsigned long color = GET_MEMORY_PIXEL(s);
			color = PS_ALPHA_BLEND(blender, color, GET_PIXEL(r));

			PUT_PIXEL(d, color);
			*zb = (float)fz;
		}
		fu += dfu;
		fv += dfv;
		fz += dfz;
		zb++;
	}
}

/* _poly_zbuf_ptex_mask_trans:
 *  Fills a trans masked perspective correct texture mapped polygon scanline.
 */
void FUNC_POLY_ZBUF_PTEX_MASK_TRANS(uintptr_t addr, int w, POLYGON_SEGMENT *info) {
	ASSERT(addr);
	ASSERT(info);

	int vmask = info->vmask << info->vshift;
	int vshift = 16 - info->vshift;
	int umask = info->umask;
	double fu = info->fu;
	double fv = info->fv;
	double fz = info->z;
	double dfu = info->dfu;
	double dfv = info->dfv;
	double dfz = info->dz;
	PS_BLENDER blender = MAKE_PS_BLENDER();
	PIXEL_PTR texture = (PIXEL_PTR)(info->texture);
	PIXEL_PTR d = (PIXEL_PTR)addr;
	PIXEL_PTR r = (PIXEL_PTR)info->read_addr;
	ZBUF_PTR zb = (ZBUF_PTR)info->zbuf_addr;

	for (int x = w - 1; x >= 0; INC_PIXEL_PTR(d), INC_PIXEL_PTR(r), x--) {
		if (*zb < fz) {
			long u = fu / fz;
			long v = fv / fz;
			PIXEL_PTR s = OFFSET_PIXEL_PTR(texture, ((v >> vshift) & vmask) + ((u >> 16) & umask));
			unsigned long color = GET_MEMORY_PIXEL(s);
			if (!IS_MASK(color)) {
				color = PS_ALPHA_BLEND(blender, color, GET_PIXEL(r));
				PUT_PIXEL(d, color);
				*zb = (float)fz;
			}
		}
		fu += dfu;
		fv += dfv;
		fz += dfz;
		zb++;
	}
}

/*
 * Sprite drawing functions.
 * =========================
 */

/* _linear_draw_sprite_ex:
 *  Draws a masked sprite onto a linear bitmap at the specified dx, dy position,
 *  using drawing mode specified by 'mode' and flipping mode specified by
 *  'flip'.
 */
void FUNC_LINEAR_DRAW_SPRITE_EX(BITMAP *dst, BITMAP *src, int dx, int dy, int mode, int flip) {
	int w, h;
	int x_dir = 1, y_dir = 1;
	int dxbeg, dybeg;
	int sxbeg, sybeg;

	ASSERT(dst);
	ASSERT(src);

	if (flip == DRAW_SPRITE_V_FLIP) {
		y_dir = -1;
	}
	if (flip == DRAW_SPRITE_H_FLIP) {
		x_dir = -1;
	}
	if (flip == DRAW_SPRITE_VH_FLIP) {
		y_dir = -1;
		x_dir = -1;
	}

	if (dst->clip) {
		int tmp;

		tmp = dst->cl - dx;
		sxbeg = MAX(0, tmp);
		dxbeg = sxbeg + dx;

		tmp = dst->cr - dx;
		w = MIN(src->w, tmp) - sxbeg;
		if (w <= 0)
			return;

		if (flip == DRAW_SPRITE_H_FLIP || flip == DRAW_SPRITE_VH_FLIP) {
			/* use backward drawing onto dst */
			sxbeg = src->w - (sxbeg + w);
			dxbeg += w - 1;
		}

		tmp = dst->ct - dy;
		sybeg = MAX(0, tmp);
		dybeg = sybeg + dy;

		tmp = dst->cb - dy;
		h = MIN(src->h, tmp) - sybeg;
		if (h <= 0)
			return;

		if (flip == DRAW_SPRITE_V_FLIP || flip == DRAW_SPRITE_VH_FLIP) {
			/* use backward drawing onto dst */
			sybeg = src->h - (sybeg + h);
			dybeg += h - 1;
		}
	} else {
		w = src->w;
		h = src->h;
		sxbeg = 0;
		sybeg = 0;
		dxbeg = dx;
		if (flip == DRAW_SPRITE_H_FLIP || flip == DRAW_SPRITE_VH_FLIP) {
			dxbeg = dx + w - 1;
		}
		dybeg = dy;
		if (flip == DRAW_SPRITE_V_FLIP || flip == DRAW_SPRITE_VH_FLIP) {
			dybeg = dy + h - 1;
		}
	}

	DLS_BLENDER lit_blender = MAKE_DLS_BLENDER(0);
	DTS_BLENDER trans_blender = MAKE_DTS_BLENDER();

	for (int y = 0; y < h; y++) {
		PIXEL_PTR s = OFFSET_PIXEL_PTR(src->line[sybeg + y], sxbeg);
		PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, dybeg + y * y_dir), dxbeg);

		for (int x = w - 1; x >= 0; INC_PIXEL_PTR(s), INC_PIXEL_PTR_N(d, x_dir), x--) {
			unsigned long c = GET_MEMORY_PIXEL(s);
			if (!IS_SPRITE_MASK(src, c)) {
				switch (mode) {
					case DRAW_SPRITE_NORMAL:
						break;

					case DRAW_SPRITE_LIT:
						c = DLSX_BLEND(lit_blender, c);
						break;

					case DRAW_SPRITE_TRANS:
						c = DTS_BLEND(trans_blender, GET_PIXEL(d), c);
						break;
				}
				PUT_MEMORY_PIXEL(d, c);
			}
		}
	}
}

/* _linear_draw_sprite:
 *  Draws a sprite onto a linear bitmap at the specified x, y position,
 *  using a masked drawing mode where zero pixels are not output.
 */
void FUNC_LINEAR_DRAW_SPRITE(BITMAP *dst, BITMAP *src, int dx, int dy) {
	int w, h;
	int dxbeg, dybeg;
	int sxbeg, sybeg;

	ASSERT(dst);
	ASSERT(src);

	if (dst->clip) {
		int tmp;

		tmp = dst->cl - dx;
		sxbeg = ((tmp < 0) ? 0 : tmp);
		dxbeg = sxbeg + dx;

		tmp = dst->cr - dx;
		w = ((tmp > src->w) ? src->w : tmp) - sxbeg;
		if (w <= 0)
			return;

		tmp = dst->ct - dy;
		sybeg = ((tmp < 0) ? 0 : tmp);
		dybeg = sybeg + dy;

		tmp = dst->cb - dy;
		h = ((tmp > src->h) ? src->h : tmp) - sybeg;
		if (h <= 0)
			return;
	} else {
		w = src->w;
		h = src->h;
		sxbeg = 0;
		sybeg = 0;
		dxbeg = dx;
		dybeg = dy;
	}

	for (int y = 0; y < h; y++) {
		PIXEL_PTR s = OFFSET_PIXEL_PTR(src->line[sybeg + y], sxbeg);
		PIXEL_PTR d = OFFSET_PIXEL_PTR(dst->line[dybeg + y], dxbeg);

		for (int x = w - 1; x >= 0; INC_PIXEL_PTR(s), INC_PIXEL_PTR(d), x--) {
			unsigned long c = GET_MEMORY_PIXEL(s);
			if (!IS_SPRITE_MASK(src, c)) {
				PUT_MEMORY_PIXEL(d, c);
			}
		}
	}
}

void FUNC_LINEAR_DRAW_SPRITE_END(void) {}

/* _linear_draw_256_sprite:
 *  Draws a 256 coor sprite onto a linear bitmap at the specified x, y
 *  position, using a masked drawing mode where zero pixels are not output.
 */
void FUNC_LINEAR_DRAW_256_SPRITE(BITMAP *dst, BITMAP *src, int dx, int dy) {
	int w, h;
	int dxbeg, dybeg;
	int sxbeg, sybeg;

	ASSERT(dst);
	ASSERT(src);

	if (dst->clip) {
		int tmp;

		tmp = dst->cl - dx;
		sxbeg = ((tmp < 0) ? 0 : tmp);
		dxbeg = sxbeg + dx;

		tmp = dst->cr - dx;
		w = ((tmp > src->w) ? src->w : tmp) - sxbeg;
		if (w <= 0)
			return;

		tmp = dst->ct - dy;
		sybeg = ((tmp < 0) ? 0 : tmp);
		dybeg = sybeg + dy;

		tmp = dst->cb - dy;
		h = ((tmp > src->h) ? src->h : tmp) - sybeg;
		if (h <= 0)
			return;
	} else {
		w = src->w;
		h = src->h;
		sxbeg = 0;
		sybeg = 0;
		dxbeg = dx;
		dybeg = dy;
	}

	int *table = _palette_expansion_table(bitmap_color_depth(dst));
	ASSERT(table);

	for (int y = 0; y < h; y++) {
		unsigned char *s = src->line[sybeg + y] + sxbeg;
		PIXEL_PTR d = OFFSET_PIXEL_PTR(dst->line[dybeg + y], dxbeg);

		for (int x = w - 1; x >= 0; s++, INC_PIXEL_PTR(d), x--) {
			unsigned long c = *s;
			if (c != 0) {
				c = table[c];
				PUT_MEMORY_PIXEL(d, c);
			}
		}
	}
}

/* _linear_draw_sprite_v_flip:
 *  Draws a sprite to a linear bitmap, flipping vertically.
 */
void FUNC_LINEAR_DRAW_SPRITE_V_FLIP(BITMAP *dst, BITMAP *src, int dx, int dy) {
	int w, h;
	int dxbeg, dybeg;
	int sxbeg, sybeg;

	ASSERT(dst);
	ASSERT(src);

	if (dst->clip) {
		int tmp;

		tmp = dst->cl - dx;
		sxbeg = ((tmp < 0) ? 0 : tmp);
		dxbeg = sxbeg + dx;

		tmp = dst->cr - dx;
		w = ((tmp > src->w) ? src->w : tmp) - sxbeg;
		if (w <= 0)
			return;

		tmp = dst->ct - dy;
		sybeg = ((tmp < 0) ? 0 : tmp);
		dybeg = sybeg + dy;

		tmp = dst->cb - dy;
		h = ((tmp > src->h) ? src->h : tmp) - sybeg;
		if (h <= 0)
			return;

		/* use backward drawing onto dst */
		sybeg = src->h - (sybeg + h);
		dybeg += h - 1;
	} else {
		w = src->w;
		h = src->h;
		sxbeg = 0;
		sybeg = 0;
		dxbeg = dx;
		dybeg = dy + h - 1;
	}

	for (int y = 0; y < h; y++) {
		PIXEL_PTR s = OFFSET_PIXEL_PTR(src->line[sybeg + y], sxbeg);
		PIXEL_PTR d = OFFSET_PIXEL_PTR(dst->line[dybeg - y], dxbeg);

		for (int x = w - 1; x >= 0; INC_PIXEL_PTR(s), INC_PIXEL_PTR(d), x--) {
			unsigned long c = GET_MEMORY_PIXEL(s);
			if (!IS_SPRITE_MASK(src, c)) {
				PUT_MEMORY_PIXEL(d, c);
			}
		}
	}
}

/* _linear_draw_sprite_h_flip:
 *  Draws a sprite to a linear bitmap, flipping horizontally.
 */
void FUNC_LINEAR_DRAW_SPRITE_H_FLIP(BITMAP *dst, BITMAP *src, int dx, int dy) {
	int w, h;
	int dxbeg, dybeg;
	int sxbeg, sybeg;

	ASSERT(dst);
	ASSERT(src);

	if (dst->clip) {
		int tmp;

		tmp = dst->cl - dx;
		sxbeg = ((tmp < 0) ? 0 : tmp);
		dxbeg = sxbeg + dx;

		tmp = dst->cr - dx;
		w = ((tmp > src->w) ? src->w : tmp) - sxbeg;
		if (w <= 0)
			return;

		/* use backward drawing onto dst */
		sxbeg = src->w - (sxbeg + w);
		dxbeg += w - 1;

		tmp = dst->ct - dy;
		sybeg = ((tmp < 0) ? 0 : tmp);
		dybeg = sybeg + dy;

		tmp = dst->cb - dy;
		h = ((tmp > src->h) ? src->h : tmp) - sybeg;
		if (h <= 0)
			return;
	} else {
		w = src->w;
		h = src->h;
		sxbeg = 0;
		sybeg = 0;
		dxbeg = dx + w - 1;
		dybeg = dy;
	}

	for (int y = 0; y < h; y++) {
		PIXEL_PTR s = OFFSET_PIXEL_PTR(src->line[sybeg + y], sxbeg);
		PIXEL_PTR d = OFFSET_PIXEL_PTR(dst->line[dybeg + y], dxbeg);

		for (int x = w - 1; x >= 0; INC_PIXEL_PTR(s), DEC_PIXEL_PTR(d), x--) {
			unsigned long c = GET_MEMORY_PIXEL(s);
			if (!IS_SPRITE_MASK(src, c)) {
				PUT_MEMORY_PIXEL(d, c);
			}
		}
	}
}

/* _linear_draw_sprite_vh_flip:
 *  Draws a sprite to a linear bitmap, flipping both vertically and horizontally.
 */
void FUNC_LINEAR_DRAW_SPRITE_VH_FLIP(BITMAP *dst, BITMAP *src, int dx, int dy) {
	int w, h;
	int dxbeg, dybeg;
	int sxbeg, sybeg;

	ASSERT(dst);
	ASSERT(src);

	if (dst->clip) {
		int tmp;

		tmp = dst->cl - dx;
		sxbeg = ((tmp < 0) ? 0 : tmp);
		dxbeg = sxbeg + dx;

		tmp = dst->cr - dx;
		w = ((tmp > src->w) ? src->w : tmp) - sxbeg;
		if (w <= 0)
			return;

		/* use backward drawing onto dst */
		sxbeg = src->w - (sxbeg + w);
		dxbeg += w - 1;

		tmp = dst->ct - dy;
		sybeg = ((tmp < 0) ? 0 : tmp);
		dybeg = sybeg + dy;

		tmp = dst->cb - dy;
		h = ((tmp > src->h) ? src->h : tmp) - sybeg;
		if (h <= 0)
			return;

		/* use backward drawing onto dst */
		sybeg = src->h - (sybeg + h);
		dybeg += h - 1;
	} else {
		w = src->w;
		h = src->h;
		sxbeg = 0;
		sybeg = 0;
		dxbeg = dx + w - 1;
		dybeg = dy + h - 1;
	}

	for (int y = 0; y < h; y++) {
		PIXEL_PTR s = OFFSET_PIXEL_PTR(src->line[sybeg + y], sxbeg);
		PIXEL_PTR d = OFFSET_PIXEL_PTR(dst->line[dybeg - y], dxbeg);

		for (int x = w - 1; x >= 0; INC_PIXEL_PTR(s), DEC_PIXEL_PTR(d), x--) {
			unsigned long c = GET_MEMORY_PIXEL(s);
			if (!IS_SPRITE_MASK(src, c)) {
				PUT_MEMORY_PIXEL(d, c);
			}
		}
	}
}

/* _linear_draw_trans_sprite:
 *  Draws a translucent sprite onto a linear bitmap.
 */
void FUNC_LINEAR_DRAW_TRANS_SPRITE(BITMAP *dst, BITMAP *src, int dx, int dy) {
	int w, h;
	int dxbeg, dybeg;
	int sxbeg, sybeg;

	ASSERT(dst);
	ASSERT(src);

	if (dst->clip) {
		int tmp;

		tmp = dst->cl - dx;
		sxbeg = ((tmp < 0) ? 0 : tmp);
		dxbeg = sxbeg + dx;

		tmp = dst->cr - dx;
		w = ((tmp > src->w) ? src->w : tmp) - sxbeg;
		if (w <= 0)
			return;

		tmp = dst->ct - dy;
		sybeg = ((tmp < 0) ? 0 : tmp);
		dybeg = sybeg + dy;

		tmp = dst->cb - dy;
		h = ((tmp > src->h) ? src->h : tmp) - sybeg;
		if (h <= 0)
			return;
	} else {
		w = src->w;
		h = src->h;
		sxbeg = 0;
		sybeg = 0;
		dxbeg = dx;
		dybeg = dy;
	}

	DTS_BLENDER blender = MAKE_DTS_BLENDER();

	if ((src->vtable->color_depth == 8) && (dst->vtable->color_depth != 8)) {
		for (int y = 0; y < h; y++) {
			unsigned char *s = src->line[sybeg + y] + sxbeg;
			PIXEL_PTR ds = OFFSET_PIXEL_PTR(bmp_read_line(dst, dybeg + y), dxbeg);
			PIXEL_PTR dd = OFFSET_PIXEL_PTR(bmp_write_line(dst, dybeg + y), dxbeg);

			for (int x = w - 1; x >= 0; s++, INC_PIXEL_PTR(ds), INC_PIXEL_PTR(dd), x--) {
				unsigned long c = *s;
#if PP_DEPTH == 8
				c = DTS_BLEND(blender, GET_PIXEL(ds), c);
				PUT_PIXEL(dd, c);
#else
				if (!IS_SPRITE_MASK(src, c)) {
					c = DTS_BLEND(blender, GET_PIXEL(ds), c);
					PUT_PIXEL(dd, c);
				}
#endif
			}
		}
	} else {
		for (int y = 0; y < h; y++) {
			PIXEL_PTR s = OFFSET_PIXEL_PTR(src->line[sybeg + y], sxbeg);
			PIXEL_PTR d = OFFSET_PIXEL_PTR(dst->line[dybeg + y], dxbeg);

			for (int x = w - 1; x >= 0; INC_PIXEL_PTR(s), INC_PIXEL_PTR(d), x--) {
				unsigned long c = GET_MEMORY_PIXEL(s);
#if PP_DEPTH == 8
				c = DTS_BLEND(blender, GET_MEMORY_PIXEL(d), c);
				PUT_MEMORY_PIXEL(d, c);
#else
				if (!IS_SPRITE_MASK(src, c)) {
					c = DTS_BLEND(blender, GET_MEMORY_PIXEL(d), c);
					PUT_MEMORY_PIXEL(d, c);
				}
#endif
			}
		}
	}
}

#if (PP_DEPTH != 8) && (PP_DEPTH != 32)

/* _linear_draw_trans_rgba_sprite:
 *  Draws a translucent RGBA sprite onto a linear bitmap.
 */
void FUNC_LINEAR_DRAW_TRANS_RGBA_SPRITE(BITMAP *dst, BITMAP *src, int dx, int dy) {
	int w, h;
	int dxbeg, dybeg;
	int sxbeg, sybeg;

	ASSERT(dst);
	ASSERT(src);

	if (dst->clip) {
		int tmp;

		tmp = dst->cl - dx;
		sxbeg = ((tmp < 0) ? 0 : tmp);
		dxbeg = sxbeg + dx;

		tmp = dst->cr - dx;
		w = ((tmp > src->w) ? src->w : tmp) - sxbeg;
		if (w <= 0)
			return;

		tmp = dst->ct - dy;
		sybeg = ((tmp < 0) ? 0 : tmp);
		dybeg = sybeg + dy;

		tmp = dst->cb - dy;
		h = ((tmp > src->h) ? src->h : tmp) - sybeg;
		if (h <= 0)
			return;
	} else {
		w = src->w;
		h = src->h;
		sxbeg = 0;
		sybeg = 0;
		dxbeg = dx;
		dybeg = dy;
	}

	RGBA_BLENDER blender = MAKE_RGBA_BLENDER();

	for (int y = 0; y < h; y++) {
		uint32_t *s = (uint32_t *)src->line[sybeg + y] + sxbeg;
		PIXEL_PTR ds = OFFSET_PIXEL_PTR(bmp_read_line(dst, dybeg + y), dxbeg);
		PIXEL_PTR dd = OFFSET_PIXEL_PTR(bmp_write_line(dst, dybeg + y), dxbeg);

		for (int x = w - 1; x >= 0; s++, INC_PIXEL_PTR(ds), INC_PIXEL_PTR(dd), x--) {
			unsigned long c = *s;

			if (c != MASK_COLOR_32) {
				c = RGBA_BLEND(blender, GET_PIXEL(ds), c);
				PUT_PIXEL(dd, c);
			}
		}
	}
}

#endif

/* _linear_draw_lit_sprite:
 *  Draws a lit sprite onto a linear bitmap.
 */
void FUNC_LINEAR_DRAW_LIT_SPRITE(BITMAP *dst, BITMAP *src, int dx, int dy, int color) {
	int w, h;
	int dxbeg, dybeg;
	int sxbeg, sybeg;

	ASSERT(dst);
	ASSERT(src);

	if (dst->clip) {
		int tmp;

		tmp = dst->cl - dx;
		sxbeg = ((tmp < 0) ? 0 : tmp);
		dxbeg = sxbeg + dx;

		tmp = dst->cr - dx;
		w = ((tmp > src->w) ? src->w : tmp) - sxbeg;
		if (w <= 0)
			return;

		tmp = dst->ct - dy;
		sybeg = ((tmp < 0) ? 0 : tmp);
		dybeg = sybeg + dy;

		tmp = dst->cb - dy;
		h = ((tmp > src->h) ? src->h : tmp) - sybeg;
		if (h <= 0)
			return;
	} else {
		w = src->w;
		h = src->h;
		sxbeg = 0;
		sybeg = 0;
		dxbeg = dx;
		dybeg = dy;
	}

	DLS_BLENDER blender = MAKE_DLS_BLENDER(color);

	for (int y = 0; y < h; y++) {
		PIXEL_PTR s = OFFSET_PIXEL_PTR(src->line[sybeg + y], sxbeg);
		PIXEL_PTR d = OFFSET_PIXEL_PTR(dst->line[dybeg + y], dxbeg);

		for (int x = w - 1; x >= 0; INC_PIXEL_PTR(s), INC_PIXEL_PTR(d), x--) {
			unsigned long c = GET_MEMORY_PIXEL(s);

			if (!IS_MASK(c)) {
				c = DLS_BLEND(blender, color, c);
				PUT_MEMORY_PIXEL(d, c);
			}
		}
	}
}

/* _linear_draw_character:
 *  For proportional font output onto a linear bitmap: uses the sprite as
 *  a mask, replacing all set pixels with the specified color.
 */
void FUNC_LINEAR_DRAW_CHARACTER(BITMAP *dst, BITMAP *src, int dx, int dy, int color, int bg) {
	int w, h;
	int dxbeg, dybeg;
	int sxbeg, sybeg;

	ASSERT(dst);
	ASSERT(src);

	if (dst->clip) {
		int tmp;

		tmp = dst->cl - dx;
		sxbeg = ((tmp < 0) ? 0 : tmp);
		dxbeg = sxbeg + dx;

		tmp = dst->cr - dx;
		w = ((tmp > src->w) ? src->w : tmp) - sxbeg;
		if (w <= 0)
			return;

		tmp = dst->ct - dy;
		sybeg = ((tmp < 0) ? 0 : tmp);
		dybeg = sybeg + dy;

		tmp = dst->cb - dy;
		h = ((tmp > src->h) ? src->h : tmp) - sybeg;
		if (h <= 0)
			return;
	} else {
		w = src->w;
		h = src->h;
		sxbeg = 0;
		sybeg = 0;
		dxbeg = dx;
		dybeg = dy;
	}

	if (bg < 0) { /* Masked character. */
		for (int y = 0; y < h; y++) {
			unsigned char *s = src->line[sybeg + y] + sxbeg;
			PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, dybeg + y), dxbeg);

			for (int x = w - 1; x >= 0; s++, INC_PIXEL_PTR(d), x--) {
				unsigned long c = *s;

				if (c != 0) {
					PUT_PIXEL(d, color);
				}
			}
		}
	} else { /* Opaque character. */
		for (int y = 0; y < h; y++) {
			unsigned char *s = src->line[sybeg + y] + sxbeg;
			PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, dybeg + y), dxbeg);

			for (int x = w - 1; x >= 0; s++, INC_PIXEL_PTR(d), x--) {
				unsigned long c = *s;

				if (c != 0) {
					PUT_PIXEL(d, color);
				} else {
					PUT_PIXEL(d, bg);
				}
			}
		}
	}
}

/* _linear_draw_rle_sprite:
 *  Draws an RLE sprite onto a linear bitmap at the specified position.
 */
void FUNC_LINEAR_DRAW_RLE_SPRITE(BITMAP *dst, AL_CONST RLE_SPRITE *src, int dx, int dy) {
	int w, h;
	int dxbeg, dybeg;
	int sxbeg, sybeg;

	ASSERT(dst);
	ASSERT(src);

	if (dst->clip) {
		int tmp;

		tmp = dst->cl - dx;
		sxbeg = ((tmp < 0) ? 0 : tmp);
		dxbeg = sxbeg + dx;

		tmp = dst->cr - dx;
		w = ((tmp > src->w) ? src->w : tmp) - sxbeg;
		if (w <= 0)
			return;

		tmp = dst->ct - dy;
		sybeg = ((tmp < 0) ? 0 : tmp);
		dybeg = sybeg + dy;

		tmp = dst->cb - dy;
		h = ((tmp > src->h) ? src->h : tmp) - sybeg;
		if (h <= 0)
			return;
	} else {
		w = src->w;
		h = src->h;
		sxbeg = 0;
		sybeg = 0;
		dxbeg = dx;
		dybeg = dy;
	}

	RLE_PTR s = (RLE_PTR)(src->dat);

	/* Clip top.  */
	for (int y = sybeg - 1; y >= 0; y--) {
		long c = *s++;

		while (!RLE_IS_EOL(c)) {
			if (c > 0)
				s += c;
			c = *s++;
		}
	}

	/* Visible part.  */
	if (sxbeg || dx + src->w >= dst->cr) {
		for (int y = 0; y < h; y++) {
			PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, dybeg + y), dxbeg);
			long c = *s++;

			/* Clip left. */
			for (int x = sxbeg; x > 0;) {
				if (RLE_IS_EOL(c))
					goto next_line;
				else if (c > 0) { /* Run of solid pixels. */
					if ((x - c) >= 0) { /* Fully clipped. */
						x -= c;
						s += c;
					} else { /* Visible on the right. */
						c -= x;
						s += x;
						break;
					}
				} else { /* Run of transparent pixels. */
					if ((x + c) >= 0) { /* Fully clipped. */
						x += c;
					} else { /* Visible on the right. */
						c += x;
						break;
					}
				}

				c = *s++;
			}

			/* Visible part.  */
			for (int x = w; x > 0;) {
				if (RLE_IS_EOL(c)) {
					goto next_line;
				} else if (c > 0) {
					/* Run of solid pixels.  */
					if ((x - c) >= 0) { /* Fully visible. */
						x -= c;
						for (c--; c >= 0; s++, INC_PIXEL_PTR(d), c--) {
							unsigned long col = *s;
							PUT_PIXEL(d, col);
						}
					} else { /* Clipped on the right. */
						c -= x;
						for (x--; x >= 0; s++, INC_PIXEL_PTR(d), x--) {
							unsigned long col = *s;
							PUT_PIXEL(d, col);
						}
						break;
					}
				} else { /* Run of transparent pixels. */
					x += c;
					d = OFFSET_PIXEL_PTR(d, -c);
				}

				c = *s++;
			}

			/* Clip right.  */
			while (!RLE_IS_EOL(c)) {
				if (c > 0)
					s += c;
				c = *s++;
			}

		next_line:;
		}
	} else {
		for (int y = 0; y < h; y++) {
			PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, dybeg + y), dxbeg);
			long c = *s++;

			/* Visible part. */
			for (int x = w; x > 0;) {
				if (RLE_IS_EOL(c))
					goto next_line2;
				else if (c > 0) {
					/* Run of solid pixels.  */
					if ((x - c) >= 0) { /* Fully visible. */
						x -= c;
						for (c--; c >= 0; s++, INC_PIXEL_PTR(d), c--) {
							unsigned long col = *s;
							PUT_PIXEL(d, col);
						}
					} else { /* Clipped on the right. */
						c -= x;
						for (x--; x >= 0; s++, INC_PIXEL_PTR(d), x--) {
							unsigned long col = *s;
							PUT_PIXEL(d, col);
						}
						break;
					}
				} else { /* Run of transparent pixels. */
					x += c;
					d = OFFSET_PIXEL_PTR(d, -c);
				}

				c = *s++;
			}

		next_line2:;
		}
	}
}

/* _linear_draw_trans_rle_sprite:
 *  Draws a translucent RLE sprite onto a linear bitmap.
 */
void FUNC_LINEAR_DRAW_TRANS_RLE_SPRITE(BITMAP *dst, AL_CONST RLE_SPRITE *src, int dx, int dy) {
	int w, h;
	int dxbeg, dybeg;
	int sxbeg, sybeg;

	ASSERT(dst);
	ASSERT(src);

	if (dst->clip) {
		int tmp;

		tmp = dst->cl - dx;
		sxbeg = ((tmp < 0) ? 0 : tmp);
		dxbeg = sxbeg + dx;

		tmp = dst->cr - dx;
		w = ((tmp > src->w) ? src->w : tmp) - sxbeg;
		if (w <= 0)
			return;

		tmp = dst->ct - dy;
		sybeg = ((tmp < 0) ? 0 : tmp);
		dybeg = sybeg + dy;

		tmp = dst->cb - dy;
		h = ((tmp > src->h) ? src->h : tmp) - sybeg;
		if (h <= 0)
			return;
	} else {
		w = src->w;
		h = src->h;
		sxbeg = 0;
		sybeg = 0;
		dxbeg = dx;
		dybeg = dy;
	}

	DTS_BLENDER blender = MAKE_DTS_BLENDER();
	RLE_PTR s = (RLE_PTR)(src->dat);

	/* Clip top.  */
	for (int y = sybeg - 1; y >= 0; y--) {
		long c = *s++;

		while (!RLE_IS_EOL(c)) {
			if (c > 0)
				s += c;
			c = *s++;
		}
	}

	/* Visible part. */
	if (sxbeg || dx + src->w >= dst->cr) {
		for (int y = 0; y < h; y++) {
			PIXEL_PTR ds = OFFSET_PIXEL_PTR(bmp_read_line(dst, dybeg + y), dxbeg);
			PIXEL_PTR dd = OFFSET_PIXEL_PTR(bmp_write_line(dst, dybeg + y), dxbeg);
			long c = *s++;

			/* Clip left.  */
			for (int x = sxbeg; x > 0;) {
				if (RLE_IS_EOL(c)) {
					goto next_line;
				} else if (c > 0) {
					/* Run of solid pixels.  */
					if ((x - c) >= 0) { /* Fully clipped. */
						x -= c;
						s += c;
					} else { /* Visible on the right. */
						c -= x;
						s += x;
						break;
					}
				} else { /* Run of transparent pixels. */
					if ((x + c) >= 0) { /* Fully clipped. */
						x += c;
					} else { /* Visible on the right. */
						c += x;
						break;
					}
				}

				c = *s++;
			}

			/* Visible part.  */
			for (int x = w; x > 0;) {
				if (RLE_IS_EOL(c)) {
					goto next_line;
				} else if (c > 0) { /* Run of solid pixels. */
					if ((x - c) >= 0) { /* Fully visible. */
						x -= c;
						for (c--; c >= 0; s++, INC_PIXEL_PTR(ds), INC_PIXEL_PTR(dd), c--) {
							unsigned long col = DTS_BLEND(blender, GET_PIXEL(ds), *s);
							PUT_PIXEL(dd, col);
						}
					} else { /* Clipped on the right. */
						c -= x;
						for (x--; x >= 0; s++, INC_PIXEL_PTR(ds), INC_PIXEL_PTR(dd), x--) {
							unsigned long col = DTS_BLEND(blender, GET_PIXEL(ds), *s);
							PUT_PIXEL(dd, col);
						}
						break;
					}
				} else { /* Run of transparent pixels. */
					x += c;
					ds = OFFSET_PIXEL_PTR(ds, -c);
					dd = OFFSET_PIXEL_PTR(dd, -c);
				}

				c = *s++;
			}

			/* Clip right.  */
			while (!RLE_IS_EOL(c)) {
				if (c > 0)
					s += c;
				c = *s++;
			}

		next_line: {}
		}
	} else {
		for (int y = 0; y < h; y++) {
			PIXEL_PTR ds = OFFSET_PIXEL_PTR(bmp_read_line(dst, dybeg + y), dxbeg);
			PIXEL_PTR dd = OFFSET_PIXEL_PTR(bmp_write_line(dst, dybeg + y), dxbeg);
			long c = *s++;

			/* Visible part. */
			for (int x = w; x > 0;) {
				if (RLE_IS_EOL(c))
					goto next_line2;
				else if (c > 0) { /* Run of solid pixels. */
					if ((x - c) >= 0) { /* Fully visible. */
						x -= c;
						for (c--; c >= 0; s++, INC_PIXEL_PTR(ds), INC_PIXEL_PTR(dd), c--) {
							unsigned long col = DTS_BLEND(blender, GET_PIXEL(ds), *s);
							PUT_PIXEL(dd, col);
						}
					} else { /* Clipped on the right. */
						c -= x;
						for (x--; x >= 0; s++, INC_PIXEL_PTR(ds), INC_PIXEL_PTR(dd), x--) {
							unsigned long col = DTS_BLEND(blender, GET_PIXEL(ds), *s);
							PUT_PIXEL(dd, col);
						}
						break;
					}
				} else { /* Run of transparent pixels. */
					x += c;
					ds = OFFSET_PIXEL_PTR(ds, -c);
					dd = OFFSET_PIXEL_PTR(dd, -c);
				}

				c = *s++;
			}

			/* Clip right.  */
			while (!RLE_IS_EOL(c)) {
				if (c > 0)
					s += c;
				c = *s++;
			}

		next_line2: {}
		}
	}
}

#if (PP_DEPTH != 8) && (PP_DEPTH != 32)

/* _linear_draw_trans_rgba_rle_sprite:
 *  Draws a translucent RGBA RLE sprite onto a linear bitmap.
 */
void FUNC_LINEAR_DRAW_TRANS_RGBA_RLE_SPRITE(BITMAP *dst, AL_CONST RLE_SPRITE *src, int dx, int dy) {
	int w, h;
	int dxbeg, dybeg;
	int sxbeg, sybeg;

	ASSERT(dst);
	ASSERT(src);

	if (dst->clip) {
		int tmp;

		tmp = dst->cl - dx;
		sxbeg = ((tmp < 0) ? 0 : tmp);
		dxbeg = sxbeg + dx;

		tmp = dst->cr - dx;
		w = ((tmp > src->w) ? src->w : tmp) - sxbeg;
		if (w <= 0)
			return;

		tmp = dst->ct - dy;
		sybeg = ((tmp < 0) ? 0 : tmp);
		dybeg = sybeg + dy;

		tmp = dst->cb - dy;
		h = ((tmp > src->h) ? src->h : tmp) - sybeg;
		if (h <= 0)
			return;
	} else {
		w = src->w;
		h = src->h;
		sxbeg = 0;
		sybeg = 0;
		dxbeg = dx;
		dybeg = dy;
	}

	RGBA_BLENDER blender = MAKE_RGBA_BLENDER();
	uint32_t *s = (uint32_t *)(src->dat);

	/* Clip top.  */
	for (int y = sybeg - 1; y >= 0; y--) {
		long c = *s++;

		while (c != MASK_COLOR_32) {
			if (c > 0)
				s += c;
			c = *s++;
		}
	}

	/* Visible part.  */
	if (sxbeg || dx + src->w >= dst->cr) {
		for (int y = 0; y < h; y++) {
			PIXEL_PTR ds = OFFSET_PIXEL_PTR(bmp_read_line(dst, dybeg + y), dxbeg);
			PIXEL_PTR dd = OFFSET_PIXEL_PTR(bmp_write_line(dst, dybeg + y), dxbeg);
			long c = *s++;

			/* Clip left.  */
			for (int x = sxbeg; x > 0;) {
				if (c == MASK_COLOR_32)
					goto next_line;
				else if (c > 0) { /* Run of solid pixels. */
					if ((x - c) >= 0) { /* Fully clipped. */
						x -= c;
						s += c;
					} else { /* Visible on the right. */
						c -= x;
						s += x;
						break;
					}
				} else { /* Run of transparent pixels. */
					if ((x + c) >= 0) { /* Fully clipped. */
						x += c;
					} else { /* Visible on the right. */
						c += x;
						break;
					}
				}
				c = *s++;
			}

			/* Visible part.  */
			for (int x = w; x > 0;) {
				if (c == MASK_COLOR_32)
					goto next_line;
				else if (c > 0) { /* Run of solid pixels. */
					if ((x - c) >= 0) { /* Fully visible. */
						x -= c;
						for (c--; c >= 0; s++, INC_PIXEL_PTR(ds), INC_PIXEL_PTR(dd), c--) {
							unsigned long col = RGBA_BLEND(blender, GET_PIXEL(ds), *s);
							PUT_PIXEL(dd, col);
						}
					} else { /* Clipped on the right. */
						c -= x;
						for (x--; x >= 0; s++, INC_PIXEL_PTR(ds), INC_PIXEL_PTR(dd), x--) {
							unsigned long col = RGBA_BLEND(blender, GET_PIXEL(ds), *s);
							PUT_PIXEL(dd, col);
						}
						break;
					}
				} else { /* Run of transparent pixels. */
					x += c;
					ds = OFFSET_PIXEL_PTR(ds, -c);
					dd = OFFSET_PIXEL_PTR(dd, -c);
				}

				c = *s++;
			}

			/* Clip right.  */
			while (c != MASK_COLOR_32) {
				if (c > 0)
					s += c;
				c = *s++;
			}

		next_line: {}
		}
	} else {
		for (int y = 0; y < h; y++) {
			PIXEL_PTR ds = OFFSET_PIXEL_PTR(bmp_read_line(dst, dybeg + y), dxbeg);
			PIXEL_PTR dd = OFFSET_PIXEL_PTR(bmp_write_line(dst, dybeg + y), dxbeg);
			long c = *s++;

			/* Visible part.  */
			for (int x = w; x > 0;) {
				if (c == MASK_COLOR_32) {
					goto next_line2;
				} else if (c > 0) { /* Run of solid pixels. */
					if ((x - c) >= 0) { /* Fully visible. */
						x -= c;
						for (c--; c >= 0; s++, INC_PIXEL_PTR(ds), INC_PIXEL_PTR(dd), c--) {
							unsigned long col = RGBA_BLEND(blender, GET_PIXEL(ds), *s);
							PUT_PIXEL(dd, col);
						}
					} else { /* Clipped on the right. */
						c -= x;
						for (x--; x >= 0; s++, INC_PIXEL_PTR(ds), INC_PIXEL_PTR(dd), x--) {
							unsigned long col = RGBA_BLEND(blender, GET_PIXEL(ds), *s);
							PUT_PIXEL(dd, col);
						}
						break;
					}
				} else { /* Run of transparent pixels. */
					x += c;
					ds = OFFSET_PIXEL_PTR(ds, -c);
					dd = OFFSET_PIXEL_PTR(dd, -c);
				}

				c = *s++;
			}
		next_line2: {}
		}
	}
}

#endif

/* _linear_draw_lit_rle_sprite:
 *  Draws a tinted RLE sprite onto a linear bitmap.
 */
void FUNC_LINEAR_DRAW_LIT_RLE_SPRITE(BITMAP *dst, AL_CONST RLE_SPRITE *src, int dx, int dy, int color) {
	int w, h;
	int dxbeg, dybeg;
	int sxbeg, sybeg;

	ASSERT(dst);
	ASSERT(src);

	if (dst->clip) {
		int tmp = dst->cl - dx;
		sxbeg = ((tmp < 0) ? 0 : tmp);
		dxbeg = sxbeg + dx;

		tmp = dst->cr - dx;
		w = ((tmp > src->w) ? src->w : tmp) - sxbeg;
		if (w <= 0)
			return;

		tmp = dst->ct - dy;
		sybeg = ((tmp < 0) ? 0 : tmp);
		dybeg = sybeg + dy;

		tmp = dst->cb - dy;
		h = ((tmp > src->h) ? src->h : tmp) - sybeg;
		if (h <= 0)
			return;
	} else {
		w = src->w;
		h = src->h;
		sxbeg = 0;
		sybeg = 0;
		dxbeg = dx;
		dybeg = dy;
	}

	DLS_BLENDER blender = MAKE_DLS_BLENDER(color);
	RLE_PTR s = (RLE_PTR)(src->dat);

	/* Clip top.  */
	for (int y = sybeg - 1; y >= 0; y--) {
		long c = *s++;

		while (!RLE_IS_EOL(c)) {
			if (c > 0)
				s += c;
			c = *s++;
		}
	}

	/* Visible part.  */
	if (sxbeg || dx + src->w >= dst->cr) {
		for (int y = 0; y < h; y++) {
			PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, dybeg + y), dxbeg);
			long c = *s++;

			/* Clip left.  */
			for (int x = sxbeg; x > 0;) {
				if (RLE_IS_EOL(c)) {
					goto next_line;
				} else if (c > 0) { /* Run of solid pixels. */
					if ((x - c) >= 0) { /* Fully clipped. */
						x -= c;
						s += c;
					} else { /* Visible on the right. */
						c -= x;
						s += x;
						break;
					}
				} else { /* Run of transparent pixels. */
					if ((x + c) >= 0) { /* Fully clipped. */
						x += c;
					} else { /* Visible on the right. */
						c += x;
						break;
					}
				}

				c = *s++;
			}

			/* Visible part.  */
			for (int x = w; x > 0;) {
				if (RLE_IS_EOL(c)) {
					goto next_line;
				} else if (c > 0) { /* Run of solid pixels. */
					if ((x - c) >= 0) { /* Fully visible. */
						x -= c;
						for (c--; c >= 0; s++, INC_PIXEL_PTR(d), c--) {
							unsigned long col = DLS_BLEND(blender, color, *s);
							PUT_PIXEL(d, col);
						}
					} else { /* Clipped on the right. */
						c -= x;
						for (x--; x >= 0; s++, INC_PIXEL_PTR(d), x--) {
							unsigned long col = DLS_BLEND(blender, color, *s);
							PUT_PIXEL(d, col);
						}
						break;
					}
				} else { /* Run of transparent pixels. */
					x += c;
					d = OFFSET_PIXEL_PTR(d, -c);
				}

				c = *s++;
			}

			/* Clip right.  */
			while (!RLE_IS_EOL(c)) {
				if (c > 0)
					s += c;
				c = *s++;
			}

		next_line: {}
		}
	} else {
		for (int y = 0; y < h; y++) {
			PIXEL_PTR d = OFFSET_PIXEL_PTR(bmp_write_line(dst, dybeg + y), dxbeg);
			long c = *s++;

			/* Visible part.  */
			for (int x = w; x > 0;) {
				if (RLE_IS_EOL(c)) {
					goto next_line2;
				} else if (c > 0) { /* Run of solid pixels. */
					if ((x - c) >= 0) { /* Fully visible. */
						x -= c;
						for (c--; c >= 0; s++, INC_PIXEL_PTR(d), c--) {
							unsigned long col = DLS_BLEND(blender, color, *s);
							PUT_PIXEL(d, col);
						}
					} else { /* Clipped on the right. */
						c -= x;
						for (x--; x >= 0; s++, INC_PIXEL_PTR(d), x--) {
							unsigned long col = DLS_BLEND(blender, color, *s);
							PUT_PIXEL(d, col);
						}
						break;
					}
				} else { /* Run of transparent pixels. */
					x += c;
					d = OFFSET_PIXEL_PTR(d, -c);
				}

				c = *s++;
			}

		next_line2: {}
		}
	}
}

#undef PP_DEPTH

#undef PIXEL_PTR
#undef PTR_PER_PIXEL
#undef OFFSET_PIXEL_PTR
#undef INC_PIXEL_PTR
#undef INC_PIXEL_PTR_N
#undef DEC_PIXEL_PTR

#undef PUT_PIXEL
#undef PUT_MEMORY_PIXEL
#undef PUT_RGB
#undef GET_PIXEL
#undef GET_MEMORY_PIXEL

#undef IS_MASK
#undef IS_SPRITE_MASK

#undef PP_BLENDER
#undef MAKE_PP_BLENDER
#undef PP_BLEND

#undef DTS_BLENDER
#undef MAKE_DTS_BLENDER
#undef DTS_BLEND

#undef DLS_BLENDER
#undef MAKE_DLS_BLENDER
#undef DLS_BLEND
#undef DLSX_BLEND

#undef RGBA_BLENDER
#undef MAKE_RGBA_BLENDER
#undef RGBA_BLEND

#undef PS_BLENDER
#undef MAKE_PS_BLENDER
#undef PS_BLEND
#undef PS_ALPHA_BLEND

#undef PATTERN_LINE
#undef GET_PATTERN_PIXEL

#undef RLE_PTR
#undef RLE_IS_EOL

#undef FUNC_LINEAR_CLEAR_TO_COLOR
#undef FUNC_LINEAR_BLIT
#undef FUNC_LINEAR_BLIT_BACKWARD
#undef FUNC_LINEAR_MASKED_BLIT

#undef FUNC_LINEAR_PUTPIXEL
#undef FUNC_LINEAR_GETPIXEL
#undef FUNC_LINEAR_HLINE
#undef FUNC_LINEAR_VLINE

#undef FUNC_LINEAR_DRAW_SPRITE
#undef FUNC_LINEAR_DRAW_SPRITE_EX
#undef FUNC_LINEAR_DRAW_256_SPRITE
#undef FUNC_LINEAR_DRAW_SPRITE_V_FLIP
#undef FUNC_LINEAR_DRAW_SPRITE_H_FLIP
#undef FUNC_LINEAR_DRAW_SPRITE_VH_FLIP
#undef FUNC_LINEAR_DRAW_TRANS_SPRITE
#undef FUNC_LINEAR_DRAW_TRANS_RGBA_SPRITE
#undef FUNC_LINEAR_DRAW_LIT_SPRITE
#undef FUNC_LINEAR_DRAW_CHARACTER
#undef FUNC_LINEAR_DRAW_RLE_SPRITE
#undef FUNC_LINEAR_DRAW_TRANS_RLE_SPRITE
#undef FUNC_LINEAR_DRAW_TRANS_RGBA_RLE_SPRITE
#undef FUNC_LINEAR_DRAW_LIT_RLE_SPRITE

#undef FUNC_LINEAR_DRAW_SPRITE_END
#undef FUNC_LINEAR_BLIT_END

#undef FUNC_POLY_SCANLINE_GCOL
#undef FUNC_POLY_SCANLINE_GRGB
#undef FUNC_POLY_SCANLINE_ATEX
#undef FUNC_POLY_SCANLINE_ATEX_MASK
#undef FUNC_POLY_SCANLINE_ATEX_LIT
#undef FUNC_POLY_SCANLINE_ATEX_MASK_LIT
#undef FUNC_POLY_SCANLINE_PTEX
#undef FUNC_POLY_SCANLINE_PTEX_MASK
#undef FUNC_POLY_SCANLINE_PTEX_LIT
#undef FUNC_POLY_SCANLINE_PTEX_MASK_LIT
#undef FUNC_POLY_SCANLINE_ATEX_TRANS
#undef FUNC_POLY_SCANLINE_ATEX_MASK_TRANS
#undef FUNC_POLY_SCANLINE_PTEX_TRANS
#undef FUNC_POLY_SCANLINE_PTEX_MASK_TRANS

#undef FUNC_POLY_ZBUF_FLAT
#undef FUNC_POLY_ZBUF_GCOL
#undef FUNC_POLY_ZBUF_GRGB
#undef FUNC_POLY_ZBUF_ATEX
#undef FUNC_POLY_ZBUF_ATEX_MASK
#undef FUNC_POLY_ZBUF_ATEX_LIT
#undef FUNC_POLY_ZBUF_ATEX_MASK_LIT
#undef FUNC_POLY_ZBUF_PTEX
#undef FUNC_POLY_ZBUF_PTEX_MASK
#undef FUNC_POLY_ZBUF_PTEX_LIT
#undef FUNC_POLY_ZBUF_PTEX_MASK_LIT
#undef FUNC_POLY_ZBUF_ATEX_TRANS
#undef FUNC_POLY_ZBUF_ATEX_MASK_TRANS
#undef FUNC_POLY_ZBUF_PTEX_TRANS
#undef FUNC_POLY_ZBUF_PTEX_MASK_TRANS
