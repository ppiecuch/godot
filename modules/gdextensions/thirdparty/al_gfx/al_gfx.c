#include <errno.h>
#include <math.h>
#include <stdio.h>

#define AL_INLINE(type, name, args, code) type name args code

#define PREFIX_I "al-gfx INFO: "
#define PREFIX_W "al-gfx WARNING: "
#define PREFIX_E "al-gfx ERROR: "

#include "al_gfx.h"

/* get_replacement_mask_color:
 *  Helper function to get a replacement color for the bitmap's mask color.
 */
static int get_replacement_mask_color(BITMAP *bmp) {
	int depth = bitmap_color_depth(bmp);

	if (depth == 8) {
		if (rgb_map)
			return rgb_map->data[31][1][31];
		else
			return bestfit_color(_current_palette, 63, 1, 63);
	} else {
		int c, g = 0;
		do {
			c = makecol_depth(depth, 255, ++g, 255);
		} while (c == bitmap_mask_color(bmp));

		return c;
	}
}

/* blit_from_256:
 *  Expands 256 color images onto a truecolor destination.
 */
static void blit_from_256(BITMAP *src, BITMAP *dest, int s_x, int s_y, int d_x, int d_y, int w, int h) {
#ifdef ALLEGRO_COLOR8
	int *dest_palette_color;

	/* lookup table avoids repeated color format conversions */
	if (_color_conv & COLORCONV_KEEP_TRANS) {
		dest_palette_color = _AL_MALLOC_ATOMIC(256 * sizeof(int));
		memcpy(dest_palette_color, _palette_expansion_table(bitmap_color_depth(dest)), 256 * sizeof(int));

		int rc = get_replacement_mask_color(dest);

		dest_palette_color[MASK_COLOR_8] = bitmap_mask_color(dest);

		for (int c = 0; c < 256; c++) {
			if ((c != MASK_COLOR_8) && (dest_palette_color[c] == bitmap_mask_color(dest)))
				dest_palette_color[c] = rc;
		}
	} else
		dest_palette_color = _palette_expansion_table(bitmap_color_depth(dest));

/* worker macro */
#define EXPAND_BLIT(bits, dsize)                                       \
	{                                                                  \
		for (int y = 0; y < h; y++) {                                  \
			unsigned char *ss = src->line[s_y + y] + s_x;              \
			uintptr_t d = bmp_write_line(dest, d_y + y) + d_x * dsize; \
                                                                       \
			for (int x = 0; x < w; x++) {                              \
				bmp_write##bits(d, dest_palette_color[*ss]);           \
				ss++;                                                  \
				d += dsize;                                            \
			}                                                          \
		}                                                              \
	}

	/* expand the above macro for each possible output depth */
	switch (bitmap_color_depth(dest)) {
#ifdef ALLEGRO_COLOR16
		case 15:
		case 16:
			EXPAND_BLIT(16, sizeof(int16_t));
			break;
#endif

#ifdef ALLEGRO_COLOR24
		case 24:
			EXPAND_BLIT(24, 3);
			break;
#endif

#ifdef ALLEGRO_COLOR32
		case 32:
			EXPAND_BLIT(32, sizeof(int32_t));
			break;
#endif
	}

	if (_color_conv & COLORCONV_KEEP_TRANS)
		_AL_FREE(dest_palette_color);
#endif
}

/* worker macro for converting between two color formats, possibly with dithering */
#define CONVERT_BLIT_EX(sbits, ssize, dbits, dsize, MAKECOL)               \
	{                                                                      \
		if (_color_conv & COLORCONV_KEEP_TRANS) {                          \
			int rc = get_replacement_mask_color(dest);                     \
			int src_mask = bitmap_mask_color(src);                         \
			int dest_mask = bitmap_mask_color(dest);                       \
			for (int y = 0; y < h; y++) {                                  \
				uintptr_t s = bmp_read_line(src, s_y + y) + s_x * ssize;   \
				uintptr_t d = bmp_write_line(dest, d_y + y) + d_x * dsize; \
				for (int x = 0; x < w; x++) {                              \
					int c = bmp_read##sbits(s);                            \
					if (c == src_mask)                                     \
						c = dest_mask;                                     \
					else {                                                 \
						int r = getr##sbits(c);                            \
						int g = getg##sbits(c);                            \
						int b = getb##sbits(c);                            \
						int c = MAKECOL;                                   \
						if (c == dest_mask)                                \
							c = rc;                                        \
					}                                                      \
					bmp_write##dbits(d, c);                                \
					s += ssize;                                            \
					d += dsize;                                            \
				}                                                          \
			}                                                              \
		} else {                                                           \
			for (int y = 0; y < h; y++) {                                  \
				uintptr_t s = bmp_read_line(src, s_y + y) + s_x * ssize;   \
				uintptr_t d = bmp_write_line(dest, d_y + y) + d_x * dsize; \
				for (int x = 0; x < w; x++) {                              \
					int c = bmp_read##sbits(s);                            \
					int r = getr##sbits(c);                                \
					int g = getg##sbits(c);                                \
					int b = getb##sbits(c);                                \
					bmp_write##dbits(d, MAKECOL);                          \
					s += ssize;                                            \
					d += dsize;                                            \
				}                                                          \
			}                                                              \
		}                                                                  \
	}

#define CONVERT_BLIT(sbits, ssize, dbits, dsize) CONVERT_BLIT_EX(sbits, ssize, dbits, dsize, makecol##dbits(r, g, b))
#define CONVERT_DITHER_BLIT(sbits, ssize, dbits, dsize) CONVERT_BLIT_EX(sbits, ssize, dbits, dsize, makecol##dbits##_dither(r, g, b, x, y))

#ifdef ALLEGRO_COLOR8

/* dither_blit:
 *  Blits with Floyd-Steinberg error diffusion.
 */
static void dither_blit(BITMAP *src, BITMAP *dest, int s_x, int s_y, int d_x, int d_y, int w, int h) {
	int prev_drawmode = _drawing_mode;
	int *errline[3];
	int *errnextline[3];
	int errpixel[3];
	int v[3], e[3], n[3];

	/* allocate memory for the error buffers */
	for (int i = 0; i < 3; i++) {
		errline[i] = _AL_MALLOC_ATOMIC(sizeof(int) * w);
		errnextline[i] = _AL_MALLOC_ATOMIC(sizeof(int) * w);
	}

	/* free the buffers if there was an error allocating one */
	for (int i = 0; i < 3; i++) {
		if ((!errline[i]) || (!errnextline[i]))
			goto getout;
	}

	/* initialize the error buffers */
	for (int i = 0; i < 3; i++) {
		memset(errline[i], 0, sizeof(int) * w);
		memset(errnextline[i], 0, sizeof(int) * w);
		errpixel[i] = 0;
	}

	/* get the replacement color */
	int rc = get_replacement_mask_color(dest);

	_drawing_mode = DRAW_MODE_SOLID;

	/* dither!!! */
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			/* get the colour from the source bitmap */
			int c = getpixel(src, s_x + x, s_y + y);
			v[0] = getr_depth(bitmap_color_depth(src), c);
			v[1] = getg_depth(bitmap_color_depth(src), c);
			v[2] = getb_depth(bitmap_color_depth(src), c);

			/* add the error from previous pixels */
			for (int i = 0; i < 3; i++) {
				n[i] = v[i] + errline[i][x] + errpixel[i];

				if (n[i] > 255)
					n[i] = 255;

				if (n[i] < 0)
					n[i] = 0;
			}

			/* find the nearest matching colour */
			int nc = makecol8(n[0], n[1], n[2]);
			if (_color_conv & COLORCONV_KEEP_TRANS) {
				if (c == bitmap_mask_color(src))
					putpixel(dest, d_x + x, d_y + y, bitmap_mask_color(dest));
				else if (nc == bitmap_mask_color(dest))
					putpixel(dest, d_x + x, d_y + y, rc);
				else
					putpixel(dest, d_x + x, d_y + y, nc);
			} else {
				putpixel(dest, d_x + x, d_y + y, nc);
			}
			v[0] = getr8(nc);
			v[1] = getg8(nc);
			v[2] = getb8(nc);

			/* calculate the error and store it */
			for (int i = 0; i < 3; i++) {
				e[i] = n[i] - v[i];
				errpixel[i] = (int)((e[i] * 3) / 8);
				errnextline[i][x] += errpixel[i];

				if (x != w - 1)
					errnextline[i][x + 1] = (int)(e[i] / 4);
			}
		}

		/* update error buffers */
		for (int i = 0; i < 3; i++) {
			memcpy(errline[i], errnextline[i], sizeof(int) * w);
			memset(errnextline[i], 0, sizeof(int) * w);
		}
	}

	_drawing_mode = prev_drawmode;

getout:

	for (int i = 0; i < 3; i++) {
		if (errline[i])
			_AL_FREE(errline[i]);

		if (errnextline[i])
			_AL_FREE(errnextline[i]);
	}
}

#endif

/* blit_from_15:
 *  Converts 15 bpp images onto some other destination format.
 */
static void blit_from_15(BITMAP *src, BITMAP *dest, int s_x, int s_y, int d_x, int d_y, int w, int h) {
#ifdef ALLEGRO_COLOR16

	switch (bitmap_color_depth(dest)) {
#ifdef ALLEGRO_COLOR8
		case 8:
			if (_color_conv & COLORCONV_DITHER_PAL)
				dither_blit(src, dest, s_x, s_y, d_x, d_y, w, h);
			else
				CONVERT_BLIT(15, sizeof(int16_t), 8, 1)
			break;
#endif

		case 16:
			CONVERT_BLIT(15, sizeof(int16_t), 16, sizeof(int16_t))
			break;

#ifdef ALLEGRO_COLOR24
		case 24:
			CONVERT_BLIT(15, sizeof(int16_t), 24, 3)
			break;
#endif

#ifdef ALLEGRO_COLOR32
		case 32:
			CONVERT_BLIT(15, sizeof(int16_t), 32, sizeof(int32_t))
			break;
#endif
	}

#endif
}

/* blit_from_16:
 *  Converts 16 bpp images onto some other destination format.
 */
static void blit_from_16(BITMAP *src, BITMAP *dest, int s_x, int s_y, int d_x, int d_y, int w, int h) {
#ifdef ALLEGRO_COLOR16

	switch (bitmap_color_depth(dest)) {
#ifdef ALLEGRO_COLOR8
		case 8:
			if (_color_conv & COLORCONV_DITHER_PAL)
				dither_blit(src, dest, s_x, s_y, d_x, d_y, w, h);
			else
				CONVERT_BLIT(16, sizeof(int16_t), 8, 1)
			break;
#endif

		case 15:
			CONVERT_BLIT(16, sizeof(int16_t), 15, sizeof(int16_t))
			break;

#ifdef ALLEGRO_COLOR24
		case 24:
			CONVERT_BLIT(16, sizeof(int16_t), 24, 3)
			break;
#endif

#ifdef ALLEGRO_COLOR32
		case 32:
			CONVERT_BLIT(16, sizeof(int16_t), 32, sizeof(int32_t))
			break;
#endif
	}

#endif
}

/* blit_from_24:
 *  Converts 24 bpp images onto some other destination format.
 */
static void blit_from_24(BITMAP *src, BITMAP *dest, int s_x, int s_y, int d_x, int d_y, int w, int h) {
#ifdef ALLEGRO_COLOR24

	switch (bitmap_color_depth(dest)) {
#ifdef ALLEGRO_COLOR8
		case 8:
			if (_color_conv & COLORCONV_DITHER_PAL)
				dither_blit(src, dest, s_x, s_y, d_x, d_y, w, h);
			else
				CONVERT_BLIT(24, 3, 8, 1);
			break;
#endif

#ifdef ALLEGRO_COLOR16
		case 15:
			if (_color_conv & COLORCONV_DITHER_HI)
				CONVERT_DITHER_BLIT(24, 3, 15, sizeof(int16_t))
			else
				CONVERT_BLIT(24, 3, 15, sizeof(int16_t))
			break;

		case 16:
			if (_color_conv & COLORCONV_DITHER_HI)
				CONVERT_DITHER_BLIT(24, 3, 16, sizeof(int16_t))
			else
				CONVERT_BLIT(24, 3, 16, sizeof(int16_t))
			break;
#endif

#ifdef ALLEGRO_COLOR32
		case 32:
			CONVERT_BLIT(24, 3, 32, sizeof(int32_t))
			break;
#endif
	}

#endif
}

/* blit_from_32:
 *  Converts 32 bpp images onto some other destination format.
 */
static void blit_from_32(BITMAP *src, BITMAP *dest, int s_x, int s_y, int d_x, int d_y, int w, int h) {
#ifdef ALLEGRO_COLOR32

	switch (bitmap_color_depth(dest)) {
#ifdef ALLEGRO_COLOR8
		case 8:
			if (_color_conv & COLORCONV_DITHER_PAL)
				dither_blit(src, dest, s_x, s_y, d_x, d_y, w, h);
			else
				CONVERT_BLIT(32, sizeof(int32_t), 8, 1)
			break;
#endif

#ifdef ALLEGRO_COLOR16
		case 15:
			if (_color_conv & COLORCONV_DITHER_HI)
				CONVERT_DITHER_BLIT(32, sizeof(int32_t), 15, sizeof(int16_t))
			else
				CONVERT_BLIT(32, sizeof(int32_t), 15, sizeof(int16_t))
			break;

		case 16:
			if (_color_conv & COLORCONV_DITHER_HI)
				CONVERT_DITHER_BLIT(32, sizeof(int32_t), 16, sizeof(int16_t))
			else
				CONVERT_BLIT(32, sizeof(int32_t), 16, sizeof(int16_t))
			break;
#endif

#ifdef ALLEGRO_COLOR24
		case 24:
			CONVERT_BLIT(32, sizeof(int32_t), 24, 3)
			break;
#endif
	}

#endif
}

/* blit_between_formats:
 *  Blits an (already clipped) region between two bitmaps of different
 *  color depths, doing the appopriate format conversions.
 */
void _blit_between_formats(BITMAP *src, BITMAP *dest, int s_x, int s_y, int d_x, int d_y, int w, int h) {
	switch (bitmap_color_depth(src)) {
		case 8:
			blit_from_256(src, dest, s_x, s_y, d_x, d_y, w, h);
			break;

		case 15:
			blit_from_15(src, dest, s_x, s_y, d_x, d_y, w, h);
			break;

		case 16:
			blit_from_16(src, dest, s_x, s_y, d_x, d_y, w, h);
			break;

		case 24:
			blit_from_24(src, dest, s_x, s_y, d_x, d_y, w, h);
			break;

		case 32:
			blit_from_32(src, dest, s_x, s_y, d_x, d_y, w, h);
			break;
	}
}

/* blit_to_self:
 *  Blits an (already clipped) region between two areas of the same bitmap,
 *  checking which way around to do the blit.
 */
static void blit_to_self(BITMAP *src, BITMAP *dest, int s_x, int s_y, int d_x, int d_y, int w, int h) {
	unsigned long sx, sy, dx, dy;

	if (dest->id & BMP_ID_NOBLIT) {
		/* with single-banked cards we have to use a temporary bitmap */
		BITMAP *tmp = create_bitmap(w, h);
		if (tmp) {
			src->vtable->blit_to_memory(src, tmp, s_x, s_y, 0, 0, w, h);
			dest->vtable->blit_from_memory(tmp, dest, 0, 0, d_x, d_y, w, h);
			destroy_bitmap(tmp);
		}
	} else {
		/* check which way round to do the blit */
		sx = s_x + src->x_ofs;
		sy = s_y + src->y_ofs;

		dx = d_x + dest->x_ofs;
		dy = d_y + dest->y_ofs;

		if ((sx + w <= dx) || (dx + w <= sx) || (sy + h <= dy) || (dy + h <= sy))
			dest->vtable->blit_to_self(src, dest, s_x, s_y, d_x, d_y, w, h);
		else if ((sy > dy) || ((sy == dy) && (sx > dx)))
			dest->vtable->blit_to_self_forward(src, dest, s_x, s_y, d_x, d_y, w, h);
		else if ((sx != dx) || (sy != dy))
			dest->vtable->blit_to_self_backward(src, dest, s_x, s_y, d_x, d_y, w, h);
	}
}

/* helper for clipping a blit rectangle */
#define BLIT_CLIP()                                 \
	/* check for ridiculous cases */                \
	if ((s_x >= src->w) || (s_y >= src->h) ||       \
			(d_x >= dest->cr) || (d_y >= dest->cb)) \
		return;                                     \
                                                    \
	/* clip src left */                             \
	if (s_x < 0) {                                  \
		w += s_x;                                   \
		d_x -= s_x;                                 \
		s_x = 0;                                    \
	}                                               \
                                                    \
	/* clip src top */                              \
	if (s_y < 0) {                                  \
		h += s_y;                                   \
		d_y -= s_y;                                 \
		s_y = 0;                                    \
	}                                               \
                                                    \
	/* clip src right */                            \
	if (s_x + w > src->w)                           \
		w = src->w - s_x;                           \
                                                    \
	/* clip src bottom */                           \
	if (s_y + h > src->h)                           \
		h = src->h - s_y;                           \
                                                    \
	/* clip dest left */                            \
	if (d_x < dest->cl) {                           \
		d_x -= dest->cl;                            \
		w += d_x;                                   \
		s_x -= d_x;                                 \
		d_x = dest->cl;                             \
	}                                               \
                                                    \
	/* clip dest top */                             \
	if (d_y < dest->ct) {                           \
		d_y -= dest->ct;                            \
		h += d_y;                                   \
		s_y -= d_y;                                 \
		d_y = dest->ct;                             \
	}                                               \
                                                    \
	/* clip dest right */                           \
	if (d_x + w > dest->cr)                         \
		w = dest->cr - d_x;                         \
                                                    \
	/* clip dest bottom */                          \
	if (d_y + h > dest->cb)                         \
		h = dest->cb - d_y;                         \
                                                    \
	/* bottle out if zero size */                   \
	if ((w <= 0) || (h <= 0))                       \
		return;

/* blit:
 *  Copies an area of the source bitmap to the destination bitmap. s_x and
 *  s_y give the top left corner of the area of the source bitmap to copy,
 *  and d_x and d_y give the position in the destination bitmap. w and h
 *  give the size of the area to blit. This routine respects the clipping
 *  rectangle of the destination bitmap, and will work correctly even when
 *  the two memory areas overlap (ie. src and dest are the same).
 */
void blit(BITMAP *src, BITMAP *dest, int s_x, int s_y, int d_x, int d_y, int w, int h) {
	ASSERT(src);
	ASSERT(dest);
	BLIT_CLIP();

	if (src->vtable->color_depth != dest->vtable->color_depth) {
		/* need to do a color conversion */
		dest->vtable->blit_between_formats(src, dest, s_x, s_y, d_x, d_y, w, h);
	} else if (is_same_bitmap(src, dest)) {
		/* special handling for overlapping regions */
		blit_to_self(src, dest, s_x, s_y, d_x, d_y, w, h);
	} else {
		/* drawing onto memory bitmaps */
		dest->vtable->blit_to_self(src, dest, s_x, s_y, d_x, d_y, w, h);
	}
}

END_OF_FUNCTION(blit);

/* masked_blit:
 *  Version of blit() that skips zero pixels. The source must be a memory
 *  bitmap, and the source and dest regions must not overlap.
 */
void masked_blit(BITMAP *src, BITMAP *dest, int s_x, int s_y, int d_x, int d_y, int w, int h) {
	ASSERT(src);
	ASSERT(dest);
	ASSERT(src->vtable->color_depth == dest->vtable->color_depth);

	BLIT_CLIP();

	dest->vtable->masked_blit(src, dest, s_x, s_y, d_x, d_y, w, h);
}

/* drawing_mode:
 *  Sets the drawing mode. This only affects routines like putpixel,
 *  lines, rectangles, triangles, etc, not the blitting or sprite
 *  drawing functions.
 */
void drawing_mode(int mode, BITMAP *pattern, int x_anchor, int y_anchor) {
	_drawing_mode = mode;
	_drawing_pattern = pattern;
	_drawing_x_anchor = x_anchor;
	_drawing_y_anchor = y_anchor;

	if (pattern) {
		_drawing_x_mask = 1;
		while (_drawing_x_mask < (unsigned)pattern->w)
			_drawing_x_mask <<= 1; /* find power of two greater than w */

		if (_drawing_x_mask > (unsigned)pattern->w) {
			ASSERT(FALSE);
			_drawing_x_mask >>= 1; /* round down if required */
		}

		_drawing_x_mask--; /* convert to AND mask */

		_drawing_y_mask = 1;
		while (_drawing_y_mask < (unsigned)pattern->h)
			_drawing_y_mask <<= 1; /* find power of two greater than h */

		if (_drawing_y_mask > (unsigned)pattern->h) {
			ASSERT(FALSE);
			_drawing_y_mask >>= 1; /* round down if required */
		}

		_drawing_y_mask--; /* convert to AND mask */
	} else
		_drawing_x_mask = _drawing_y_mask = 0;
}

/* set_blender_mode:
 *  Specifies a custom set of blender functions for interpolating between
 *  truecolor pixels. The 24 bit blender is shared between the 24 and 32 bit
 *  modes. Pass a NULL table for unused color depths (you must not draw
 *  translucent graphics in modes without a handler, though!). Your blender
 *  will be passed two 32 bit colors in the appropriate format (5.5.5, 5.6.5,
 *  or 8.8.8), and an alpha value, should return the result of combining them.
 *  In translucent drawing modes, the two colors are taken from the source
 *  and destination images and the alpha is specified by this function. In
 *  lit modes, the alpha is specified when you call the drawing routine, and
 *  the interpolation is between the source color and the RGB values you pass
 *  to this function.
 */
void set_blender_mode(BLENDER_FUNC b15, BLENDER_FUNC b16, BLENDER_FUNC b24, int r, int g, int b, int a) {
	_blender_func15 = b15;
	_blender_func16 = b16;
	_blender_func24 = b24;
	_blender_func32 = b24;

	_blender_func15x = _blender_black;
	_blender_func16x = _blender_black;
	_blender_func24x = _blender_black;

	_blender_col_15 = makecol15(r, g, b);
	_blender_col_16 = makecol16(r, g, b);
	_blender_col_24 = makecol24(r, g, b);
	_blender_col_32 = makecol32(r, g, b);

	_blender_alpha = a;
}

/* set_blender_mode_ex
 *  Specifies a custom set of blender functions for interpolating between
 *  truecolor pixels, providing a more complete set of routines, which
 *  differentiate between 24 and 32 bit modes, and have special routines
 *  for blending 32 bit RGBA pixels onto a destination of any format.
 */
void set_blender_mode_ex(BLENDER_FUNC b15, BLENDER_FUNC b16, BLENDER_FUNC b24, BLENDER_FUNC b32, BLENDER_FUNC b15x, BLENDER_FUNC b16x, BLENDER_FUNC b24x, int r, int g, int b, int a) {
	_blender_func15 = b15;
	_blender_func16 = b16;
	_blender_func24 = b24;
	_blender_func32 = b32;

	_blender_func15x = b15x;
	_blender_func16x = b16x;
	_blender_func24x = b24x;

	_blender_col_15 = makecol15(r, g, b);
	_blender_col_16 = makecol16(r, g, b);
	_blender_col_24 = makecol24(r, g, b);
	_blender_col_32 = makecol32(r, g, b);

	_blender_alpha = a;
}

/* xor_mode:
 *  Shortcut function for toggling XOR mode on and off.
 */
void xor_mode(int on) { drawing_mode(on ? DRAW_MODE_XOR : DRAW_MODE_SOLID, NULL, 0, 0); }

/* solid_mode:
 *  Shortcut function for selecting solid drawing mode.
 */
void solid_mode(void) { drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0); }

/* clear_bitmap:
 *  Clears the bitmap to color 0.
 */
void clear_bitmap(BITMAP *bitmap) { clear_to_color(bitmap, 0); }

/* _bitmap_has_alpha:
 *  Checks whether this bitmap has an alpha channel.
 */
int _bitmap_has_alpha(BITMAP *bmp) {
	if (bitmap_color_depth(bmp) != 32)
		return FALSE;

	for (int y = 0; y < bmp->h; y++) {
		for (int x = 0; x < bmp->w; x++) {
			int c = getpixel(bmp, x, y);
			if (geta32(c))
				return TRUE;
		}
	}

	return FALSE;
}

/* set_color:
 *  Sets a single palette entry.
 */
void set_color(int index, AL_CONST RGB *p) {
	ASSERT(index >= 0 && index < PAL_SIZE);
	set_palette_range((struct RGB *)p - index, index, index, FALSE);
}

/* set_palette:
 *  Sets the entire color palette.
 */
void set_palette(AL_CONST PALETTE p) { set_palette_range(p, 0, PAL_SIZE - 1, TRUE); }

/* set_palette_range:
 *  Sets a part of the color palette.
 */
void set_palette_range(AL_CONST PALETTE p, int from, int to, int vsync) {
	ASSERT(from >= 0 && from < PAL_SIZE);
	ASSERT(to >= 0 && to < PAL_SIZE)

	for (int c = from; c <= to; c++) {
		_current_palette[c] = p[c];

		if (_color_depth != 8)
			palette_color[c] = makecol(_rgb_scale_6[p[c].r], _rgb_scale_6[p[c].g], _rgb_scale_6[p[c].b]);
	}

	_current_palette_changed = 0xFFFFFFFF & ~(1 << (_color_depth - 1));
}

/* previous palette, so the image loaders can restore it when they are done */
int _got_prev_current_palette = FALSE;
PALETTE _prev_current_palette;
static int prev_palette_color[PAL_SIZE];

/* select_palette:
 *  Sets the aspects of the palette tables that are used for converting
 *  between different image formats, without altering the display settings.
 *  The previous settings are copied onto a one-deep stack, from where they
 *  can be restored by calling unselect_palette().
 */
void select_palette(AL_CONST PALETTE p) {
	for (int c = 0; c < PAL_SIZE; c++) {
		_prev_current_palette[c] = _current_palette[c];
		_current_palette[c] = p[c];
	}

	if (_color_depth != 8) {
		for (int c = 0; c < PAL_SIZE; c++) {
			prev_palette_color[c] = palette_color[c];
			palette_color[c] = makecol(_rgb_scale_6[p[c].r], _rgb_scale_6[p[c].g], _rgb_scale_6[p[c].b]);
		}
	}

	_got_prev_current_palette = TRUE;

	_current_palette_changed = 0xFFFFFFFF & ~(1 << (_color_depth - 1));
}

/* unselect_palette:
 *  Restores palette settings from before the last call to select_palette().
 */
void unselect_palette(void) {
	for (int c = 0; c < PAL_SIZE; c++)
		_current_palette[c] = _prev_current_palette[c];

	if (_color_depth != 8) {
		for (int c = 0; c < PAL_SIZE; c++)
			palette_color[c] = prev_palette_color[c];
	}

	ASSERT(_got_prev_current_palette == TRUE);
	_got_prev_current_palette = FALSE;

	_current_palette_changed = 0xFFFFFFFF & ~(1 << (_color_depth - 1));
}

/* _palette_expansion_table:
 *  Creates a lookup table for expanding 256->truecolor.
 */
static int *palette_expansion_table(int bpp) {
	int *table;
	int c;

	switch (bpp) {
		case 15:
			table = _palette_color15;
			break;
		case 16:
			table = _palette_color16;
			break;
		case 24:
			table = _palette_color24;
			break;
		case 32:
			table = _palette_color32;
			break;
		default:
			ASSERT(FALSE);
			return NULL;
	}

	if (_current_palette_changed & (1 << (bpp - 1))) {
		for (c = 0; c < PAL_SIZE; c++) {
			table[c] = makecol_depth(bpp,
					_rgb_scale_6[_current_palette[c].r],
					_rgb_scale_6[_current_palette[c].g],
					_rgb_scale_6[_current_palette[c].b]);
		}

		_current_palette_changed &= ~(1 << (bpp - 1));
	}

	return table;
}

/* this has to be called through a function pointer, so MSVC asm can use it */
int *(*_palette_expansion_table)(int) = palette_expansion_table;

/* generate_332_palette:
 *  Used when loading a truecolor image into an 8 bit bitmap, to generate
 *  a 3.3.2 RGB palette.
 */
void generate_332_palette(PALETTE pal) {
	int c;

	for (c = 0; c < PAL_SIZE; c++) {
		pal[c].r = ((c >> 5) & 7) * 63 / 7;
		pal[c].g = ((c >> 2) & 7) * 63 / 7;
		pal[c].b = (c & 3) * 63 / 3;
	}

	pal[0].r = 63;
	pal[0].g = 0;
	pal[0].b = 63;

	pal[254].r = pal[254].g = pal[254].b = 0;
}

/* get_color:
 *  Retrieves a single color from the palette.
 */
void get_color(int index, RGB *p) {
	ASSERT(index >= 0 && index < PAL_SIZE);
	ASSERT(p);
	get_palette_range(p - index, index, index);
}

/* get_palette:
 *  Retrieves the entire color palette.
 */
void get_palette(PALETTE p) {
	get_palette_range(p, 0, PAL_SIZE - 1);
}

/* get_palette_range:
 *  Retrieves a part of the color palette.
 */
void get_palette_range(PALETTE p, int from, int to) {
	ASSERT(from >= 0 && from < PAL_SIZE);
	ASSERT(to >= 0 && to < PAL_SIZE);

	for (int c = from; c <= to; c++)
		p[c] = _current_palette[c];
}

/* fade_interpolate:
 *  Calculates a palette part way between source and dest, returning it
 *  in output. The pos indicates how far between the two extremes it should
 *  be: 0 = return source, 64 = return dest, 32 = return exactly half way.
 *  Only affects colors between from and to (inclusive).
 */
void fade_interpolate(AL_CONST PALETTE source, AL_CONST PALETTE dest, PALETTE output, int pos, int from, int to) {
	ASSERT(pos >= 0 && pos <= 64);
	ASSERT(from >= 0 && from < PAL_SIZE);
	ASSERT(to >= 0 && to < PAL_SIZE);

	for (int c = from; c <= to; c++) {
		output[c].r = ((int)source[c].r * (63 - pos) + (int)dest[c].r * pos) / 64;
		output[c].g = ((int)source[c].g * (63 - pos) + (int)dest[c].g * pos) / 64;
		output[c].b = ((int)source[c].b * (63 - pos) + (int)dest[c].b * pos) / 64;
	}
}

/* fade_from_range:
 *  Fades from source to dest, at the specified speed (1 is the slowest, 64
 *  is instantaneous). Only affects colors between from and to (inclusive,
 *  pass 0 and 255 to fade the entire palette).
 */
void fade_from_range(AL_CONST PALETTE source, AL_CONST PALETTE dest, int speed, int from, int to, PROGRESS_FUNC progress_source) {
	PALETTE temp;

	ASSERT(speed > 0 && speed <= 64);
	ASSERT(from >= 0 && from < PAL_SIZE);
	ASSERT(to >= 0 && to < PAL_SIZE);

	for (int c = 0; c < PAL_SIZE; c++)
		temp[c] = source[c];

	if (progress_source) {
		int start = progress_source();
		int c, last = -1;

		while ((c = (progress_source() - start) * speed / 2) < 64) {
			if (c != last) {
				fade_interpolate(source, dest, temp, c, from, to);
				set_palette_range(temp, from, to, TRUE);
				last = c;
			}
		}
	} else {
		for (int c = 0; c < 64; c += speed) {
			fade_interpolate(source, dest, temp, c, from, to);
			set_palette_range(temp, from, to, TRUE);
			set_palette_range(temp, from, to, TRUE);
		}
	}

	set_palette_range(dest, from, to, TRUE);
}

/* fade_in_range:
 *  Fades from a solid black palette to p, at the specified speed (1 is
 *  the slowest, 64 is instantaneous). Only affects colors between from and
 *  to (inclusive, pass 0 and 255 to fade the entire palette).
 */
void fade_in_range(AL_CONST PALETTE p, int speed, int from, int to, PROGRESS_FUNC progress_source) {
	ASSERT(speed > 0 && speed <= 64);
	ASSERT(from >= 0 && from < PAL_SIZE);
	ASSERT(to >= 0 && to < PAL_SIZE);
	fade_from_range(black_palette, p, speed, from, to, progress_source);
}

/* fade_out_range:
 *  Fades from the current palette to a solid black palette, at the
 *  specified speed (1 is the slowest, 64 is instantaneous). Only affects
 *  colors between from and to (inclusive, pass 0 and 255 to fade the
 *  entire palette).
 */
void fade_out_range(int speed, int from, int to, PROGRESS_FUNC progress_source) {
	PALETTE temp;
	ASSERT(speed > 0 && speed <= 64);
	ASSERT(from >= 0 && from < PAL_SIZE);
	ASSERT(to >= 0 && to < PAL_SIZE);

	get_palette(temp);
	fade_from_range(temp, black_palette, speed, from, to, progress_source);
}

/* fade_from:
 *  Fades from source to dest, at the specified speed (1 is the slowest, 64
 *  is instantaneous).
 */
void fade_from(AL_CONST PALETTE source, AL_CONST PALETTE dest, int speed, PROGRESS_FUNC progress_source) {
	ASSERT(speed > 0 && speed <= 64);
	fade_from_range(source, dest, speed, 0, PAL_SIZE - 1, progress_source);
}

/* fade_in:
 *  Fades from a solid black palette to p, at the specified speed (1 is
 *  the slowest, 64 is instantaneous).
 */
void fade_in(AL_CONST PALETTE p, int speed, PROGRESS_FUNC progress_source) {
	ASSERT(speed > 0 && speed <= 64);
	fade_in_range(p, speed, 0, PAL_SIZE - 1, progress_source);
}

/* fade_out:
 *  Fades from the current palette to a solid black palette, at the
 *  specified speed (1 is the slowest, 64 is instantaneous).
 */
void fade_out(int speed, PROGRESS_FUNC progress_source) {
	ASSERT(speed > 0 && speed <= 64);
	fade_out_range(speed, 0, PAL_SIZE - 1, progress_source);
}

/* rect:
 *  Draws an outline rectangle.
 */
void _soft_rect(BITMAP *bmp, int x1, int y1, int x2, int y2, int color) {
	int t;

	if (x2 < x1) {
		t = x1;
		x1 = x2;
		x2 = t;
	}

	if (y2 < y1) {
		t = y1;
		y1 = y2;
		y2 = t;
	}

	acquire_bitmap(bmp);

	hline(bmp, x1, y1, x2, color);

	if (y2 > y1)
		hline(bmp, x1, y2, x2, color);

	if (y2 - 1 >= y1 + 1) {
		vline(bmp, x1, y1 + 1, y2 - 1, color);

		if (x2 > x1)
			vline(bmp, x2, y1 + 1, y2 - 1, color);
	}

	release_bitmap(bmp);
}

/* _normal_rectfill:
 *  Draws a solid filled rectangle, using hfill() to do the work.
 */
void _normal_rectfill(BITMAP *bmp, int x1, int y1, int x2, int y2, int color) {
	int t;

	if (y1 > y2) {
		t = y1;
		y1 = y2;
		y2 = t;
	}

	if (bmp->clip) {
		if (x1 > x2) {
			t = x1;
			x1 = x2;
			x2 = t;
		}

		if (x1 < bmp->cl)
			x1 = bmp->cl;

		if (x2 >= bmp->cr)
			x2 = bmp->cr - 1;

		if (x2 < x1)
			return;

		if (y1 < bmp->ct)
			y1 = bmp->ct;

		if (y2 >= bmp->cb)
			y2 = bmp->cb - 1;

		if (y2 < y1)
			return;

		bmp->clip = FALSE;
		t = TRUE;
	} else
		t = FALSE;

	acquire_bitmap(bmp);

	while (y1 <= y2) {
		bmp->vtable->hfill(bmp, x1, y1, x2, color);
		y1++;
	};

	release_bitmap(bmp);

	bmp->clip = t;
}

/* do_line:
 *  Calculates all the points along a line between x1, y1 and x2, y2,
 *  calling the supplied function for each one. This will be passed a
 *  copy of the bmp parameter, the x and y position, and a copy of the
 *  d parameter (so do_line() can be used with putpixel()).
 */
void do_line(BITMAP *bmp, int x1, int y1, int x2, int y2, int d, void (*proc)(BITMAP *, int, int, int)) {
	int dx = x2 - x1;
	int dy = y2 - y1;
	int i1, i2;
	int x, y;
	int dd;

/* worker macro */
#define DO_LINE(pri_sign, pri_c, pri_cond, sec_sign, sec_c, sec_cond) \
	{                                                                 \
		if (d##pri_c == 0) {                                          \
			proc(bmp, x1, y1, d);                                     \
			return;                                                   \
		}                                                             \
                                                                      \
		i1 = 2 * d##sec_c;                                            \
		dd = i1 - (sec_sign(pri_sign d##pri_c));                      \
		i2 = dd - (sec_sign(pri_sign d##pri_c));                      \
                                                                      \
		x = x1;                                                       \
		y = y1;                                                       \
                                                                      \
		while (pri_c pri_cond pri_c##2) {                             \
			proc(bmp, x, y, d);                                       \
                                                                      \
			if (dd sec_cond 0) {                                      \
				sec_c = sec_c sec_sign 1;                             \
				dd += i2;                                             \
			} else                                                    \
				dd += i1;                                             \
                                                                      \
			pri_c = pri_c pri_sign 1;                                 \
		}                                                             \
	}

	if (dx >= 0) {
		if (dy >= 0) {
			if (dx >= dy) {
				/* (x1 <= x2) && (y1 <= y2) && (dx >= dy) */
				DO_LINE(+, x, <=, +, y, >=);
			} else {
				/* (x1 <= x2) && (y1 <= y2) && (dx < dy) */
				DO_LINE(+, y, <=, +, x, >=);
			}
		} else {
			if (dx >= -dy) {
				/* (x1 <= x2) && (y1 > y2) && (dx >= dy) */
				DO_LINE(+, x, <=, -, y, <=);
			} else {
				/* (x1 <= x2) && (y1 > y2) && (dx < dy) */
				DO_LINE(-, y, >=, +, x, >=);
			}
		}
	} else {
		if (dy >= 0) {
			if (-dx >= dy) {
				/* (x1 > x2) && (y1 <= y2) && (dx >= dy) */
				DO_LINE(-, x, >=, +, y, >=);
			} else {
				/* (x1 > x2) && (y1 <= y2) && (dx < dy) */
				DO_LINE(+, y, <=, -, x, <=);
			}
		} else {
			if (-dx >= -dy) {
				/* (x1 > x2) && (y1 > y2) && (dx >= dy) */
				DO_LINE(-, x, >=, -, y, <=);
			} else {
				/* (x1 > x2) && (y1 > y2) && (dx < dy) */
				DO_LINE(-, y, >=, -, x, <=);
			}
		}
	}

#undef DO_LINE
}

/* _normal_line:
 *  Draws a line from x1, y1 to x2, y2, using putpixel() to do the work.
 */
void _normal_line(BITMAP *bmp, int x1, int y1, int x2, int y2, int color) {
	int sx, sy, dx, dy, t;

	if (x1 == x2) {
		vline(bmp, x1, y1, y2, color);
		return;
	}

	if (y1 == y2) {
		hline(bmp, x1, y1, x2, color);
		return;
	}

	/* use a bounding box to check if the line needs clipping */
	if (bmp->clip) {
		sx = x1;
		sy = y1;
		dx = x2;
		dy = y2;

		if (sx > dx) {
			t = sx;
			sx = dx;
			dx = t;
		}

		if (sy > dy) {
			t = sy;
			sy = dy;
			dy = t;
		}

		if ((sx >= bmp->cr) || (sy >= bmp->cb) || (dx < bmp->cl) || (dy < bmp->ct))
			return;

		if ((sx >= bmp->cl) && (sy >= bmp->ct) && (dx < bmp->cr) && (dy < bmp->cb))
			bmp->clip = FALSE;

		t = TRUE;
	} else
		t = FALSE;

	acquire_bitmap(bmp);

	do_line(bmp, x1, y1, x2, y2, color, bmp->vtable->putpixel);

	release_bitmap(bmp);

	bmp->clip = t;
}

/* _fast_line:
 *  Draws a line from x1, y1 to x2, y2, using putpixel() to do the work.
 *  This is an implementation of the Cohen-Sutherland line clipping algorithm.
 *  Loops over the line until it can be either trivially rejected or trivially
 *  accepted. If it is neither rejected nor accepted, subdivide it into two
 *  segments, one of which can be rejected.
 */
void _fast_line(BITMAP *bmp, int x1, int y1, int x2, int y2, int color) {
	int code0, code1;
	int outcode;
	int x, y;
	int xmax, xmin, ymax, ymin;
	int done = 0, accept = 0;
	int clip_orig;
	ASSERT(bmp);

	if ((clip_orig = bmp->clip) != 0) { /* save clipping state */
#define TOP 0x8
#define BOTTOM 0x4
#define LEFT 0x2
#define RIGHT 0x1

#define COMPCLIP(code, x, y) \
	{                        \
		code = 0;            \
		if (y < ymin)        \
			code |= TOP;     \
		else if (y > ymax)   \
			code |= BOTTOM;  \
		if (x < xmin)        \
			code |= LEFT;    \
		else if (x > xmax)   \
			code |= RIGHT;   \
	}

		xmin = bmp->cl;
		xmax = bmp->cr - 1;
		ymin = bmp->ct;
		ymax = bmp->cb - 1;

		COMPCLIP(code0, x1, y1);
		COMPCLIP(code1, x2, y2);

		do {
			if (!(code0 | code1)) {
				/* Trivially accept. */
				accept = done = 1;
			} else if (code0 & code1) {
				/* Trivially reject. */
				done = 1;
			} else {
				/* Didn't reject or accept, so do some calculations. */
				outcode = code0 ? code0 : code1; /* pick one endpoint */

				if (outcode & TOP) {
					if (y2 == y1)
						x = x1;
					else
						x = x1 + (x2 - x1) * (ymin - y1) / (y2 - y1);
					y = ymin;
				} else if (outcode & BOTTOM) {
					if (y2 == y1)
						x = x1;
					else
						x = x1 + (x2 - x1) * (ymax - y1) / (y2 - y1);
					y = ymax;
				} else if (outcode & LEFT) {
					if (x2 == x1)
						y = y1;
					else
						y = y1 + (y2 - y1) * (xmin - x1) / (x2 - x1);
					x = xmin;
				} else { /* outcode & RIGHT */
					if (x2 == x1)
						y = y1;
					else
						y = y1 + (y2 - y1) * (xmax - x1) / (x2 - x1);
					x = xmax;
				}

				if (outcode == code0) {
					x1 = x;
					y1 = y;
					COMPCLIP(code0, x1, y1);
				} else {
					x2 = x;
					y2 = y;
					COMPCLIP(code1, x2, y2);
				}
			}
		} while (!done);

#undef COMPCLIP
#undef TOP
#undef BOTTOM
#undef LEFT
#undef RIGHT

		if (!accept)
			return;

		/* We have already done the clipping, no need to do it again. */
		bmp->clip = FALSE;
	}

	if (x1 == x2) {
		bmp->vtable->vline(bmp, x1, y1, y2, color);
	} else if (y1 == y2) {
		bmp->vtable->hline(bmp, x1, y1, x2, color);
	} else {
		acquire_bitmap(bmp);
		do_line(bmp, x1, y1, x2, y2, color, bmp->vtable->putpixel);
		release_bitmap(bmp);
	}

	/* Restore original clipping state. */
	bmp->clip = clip_orig;
}

/* do_circle:
 *  Helper function for the circle drawing routines. Calculates the points
 *  in a circle of radius r around point x, y, and calls the specified
 *  routine for each one. The output proc will be passed first a copy of
 *  the bmp parameter, then the x, y point, then a copy of the d parameter
 *  (so putpixel() can be used as the callback).
 */
void do_circle(BITMAP *bmp, int x, int y, int radius, int d, void (*proc)(BITMAP *, int, int, int)) {
	int cx = 0;
	int cy = radius;
	int df = 1 - radius;
	int d_e = 3;
	int d_se = -2 * radius + 5;

	do {
		proc(bmp, x + cx, y + cy, d);

		if (cx)
			proc(bmp, x - cx, y + cy, d);

		if (cy)
			proc(bmp, x + cx, y - cy, d);

		if ((cx) && (cy))
			proc(bmp, x - cx, y - cy, d);

		if (cx != cy) {
			proc(bmp, x + cy, y + cx, d);

			if (cx)
				proc(bmp, x + cy, y - cx, d);

			if (cy)
				proc(bmp, x - cy, y + cx, d);

			if (cx && cy)
				proc(bmp, x - cy, y - cx, d);
		}

		if (df < 0) {
			df += d_e;
			d_e += 2;
			d_se += 2;
		} else {
			df += d_se;
			d_e += 2;
			d_se += 4;
			cy--;
		}

		cx++;

	} while (cx <= cy);
}

/* circle:
 *  Draws a circle.
 */
void _soft_circle(BITMAP *bmp, int x, int y, int radius, int color) {
	int clip, sx, sy, dx, dy;
	ASSERT(bmp);

	if (bmp->clip) {
		sx = x - radius - 1;
		sy = y - radius - 1;
		dx = x + radius + 1;
		dy = y + radius + 1;

		if ((sx >= bmp->cr) || (sy >= bmp->cb) || (dx < bmp->cl) || (dy < bmp->ct))
			return;

		if ((sx >= bmp->cl) && (sy >= bmp->ct) && (dx < bmp->cr) && (dy < bmp->cb))
			bmp->clip = FALSE;

		clip = TRUE;
	} else
		clip = FALSE;

	acquire_bitmap(bmp);

	do_circle(bmp, x, y, radius, color, bmp->vtable->putpixel);

	release_bitmap(bmp);

	bmp->clip = clip;
}

/* circlefill:
 *  Draws a filled circle.
 */
void _soft_circlefill(BITMAP *bmp, int x, int y, int radius, int color) {
	int cx = 0;
	int cy = radius;
	int df = 1 - radius;
	int d_e = 3;
	int d_se = -2 * radius + 5;
	int clip, sx, sy, dx, dy;
	ASSERT(bmp);

	if (bmp->clip) {
		sx = x - radius - 1;
		sy = y - radius - 1;
		dx = x + radius + 1;
		dy = y + radius + 1;

		if ((sx >= bmp->cr) || (sy >= bmp->cb) || (dx < bmp->cl) || (dy < bmp->ct))
			return;

		if ((sx >= bmp->cl) && (sy >= bmp->ct) && (dx < bmp->cr) && (dy < bmp->cb))
			bmp->clip = FALSE;

		clip = TRUE;
	} else
		clip = FALSE;

	acquire_bitmap(bmp);

	do {
		bmp->vtable->hfill(bmp, x - cy, y - cx, x + cy, color);

		if (cx)
			bmp->vtable->hfill(bmp, x - cy, y + cx, x + cy, color);

		if (df < 0) {
			df += d_e;
			d_e += 2;
			d_se += 2;
		} else {
			if (cx != cy) {
				bmp->vtable->hfill(bmp, x - cx, y - cy, x + cx, color);

				if (cy)
					bmp->vtable->hfill(bmp, x - cx, y + cy, x + cx, color);
			}

			df += d_se;
			d_e += 2;
			d_se += 4;
			cy--;
		}

		cx++;

	} while (cx <= cy);

	release_bitmap(bmp);

	bmp->clip = clip;
}

/* do_ellipse:
 *  Helper function for the ellipse drawing routines. Calculates the points
 *  in an ellipse of radius rx and ry around point x, y, and calls the
 *  specified routine for each one. The output proc will be passed first a
 *  copy of the bmp parameter, then the x, y point, then a copy of the d
 *  parameter (so putpixel() can be used as the callback).
 */
void do_ellipse(BITMAP *bmp, int ix, int iy, int rx0, int ry0, int d,
		void (*proc)(BITMAP *, int, int, int)) {
	int rx, ry;
	int x, y;
	float x_change;
	float y_change;
	float ellipse_error;
	float two_a_sq;
	float two_b_sq;
	float stopping_x;
	float stopping_y;
	int midway_x = 0;

	rx = MAX(rx0, 0);
	ry = MAX(ry0, 0);

	two_a_sq = 2 * rx * rx;
	two_b_sq = 2 * ry * ry;

	x = rx;
	y = 0;

	x_change = ry * ry * (1 - 2 * rx);
	y_change = rx * rx;
	ellipse_error = 0.0;

	/* The following two variables decide when to stop.  It's easier than
	 * solving for this explicitly.
	 */
	stopping_x = two_b_sq * rx;
	stopping_y = 0.0;

	/* First set of points. */
	while (y <= ry) {
		proc(bmp, ix + x, iy + y, d);
		if (x != 0) {
			proc(bmp, ix - x, iy + y, d);
		}
		if (y != 0) {
			proc(bmp, ix + x, iy - y, d);
			if (x != 0) {
				proc(bmp, ix - x, iy - y, d);
			}
		}

		y++;
		stopping_y += two_a_sq;
		ellipse_error += y_change;
		y_change += two_a_sq;
		midway_x = x;

		if (stopping_x < stopping_y && x > 1) {
			break;
		}

		if ((2.0f * ellipse_error + x_change) > 0.0) {
			if (x) {
				x--;
				stopping_x -= two_b_sq;
				ellipse_error += x_change;
				x_change += two_b_sq;
			}
		}
	}

	/* To do the other half of the ellipse we reset to the top of it, and
	 * iterate in the opposite direction.
	 */
	x = 0;
	y = ry;

	x_change = ry * ry;
	y_change = rx * rx * (1 - 2 * ry);
	ellipse_error = 0.0;

	while (x < midway_x) {
		proc(bmp, ix + x, iy + y, d);
		if (x != 0) {
			proc(bmp, ix - x, iy + y, d);
		}
		if (y != 0) {
			proc(bmp, ix + x, iy - y, d);
			if (x != 0) {
				proc(bmp, ix - x, iy - y, d);
			}
		}

		x++;
		ellipse_error += x_change;
		x_change += two_b_sq;

		if ((2.0f * ellipse_error + y_change) > 0.0) {
			if (y) {
				y--;
				ellipse_error += y_change;
				y_change += two_a_sq;
			}
		}
	}
}

/* _soft_ellipse:
 *  Draws an ellipse.
 */
void _soft_ellipse(BITMAP *bmp, int x, int y, int rx, int ry, int color) {
	int clip, sx, sy, dx, dy;
	ASSERT(bmp);

	if (bmp->clip) {
		sx = x - rx - 1;
		sy = y - ry - 1;
		dx = x + rx + 1;
		dy = y + ry + 1;

		if ((sx >= bmp->cr) || (sy >= bmp->cb) || (dx < bmp->cl) || (dy < bmp->ct))
			return;

		if ((sx >= bmp->cl) && (sy >= bmp->ct) && (dx < bmp->cr) && (dy < bmp->cb))
			bmp->clip = FALSE;

		clip = TRUE;
	} else
		clip = FALSE;

	acquire_bitmap(bmp);

	do_ellipse(bmp, x, y, rx, ry, color, bmp->vtable->putpixel);

	release_bitmap(bmp);

	bmp->clip = clip;
}

/* _soft_ellipsefill:
 *  Draws a filled ellipse.
 */
void _soft_ellipsefill(BITMAP *bmp, int ix, int iy, int rx0, int ry0, int color) {
	int rx, ry;
	int x, y;
	float x_change;
	float y_change;
	float ellipse_error;
	float two_a_sq;
	float two_b_sq;
	float stopping_x;
	float stopping_y;
	int clip, sx, sy, dx, dy;
	int last_drawn_y;
	int old_y;
	int midway_x = 0;
	ASSERT(bmp);

	rx = MAX(rx0, 0);
	ry = MAX(ry0, 0);

	if (bmp->clip) {
		sx = ix - rx - 1;
		sy = iy - ry - 1;
		dx = ix + rx + 1;
		dy = iy + ry + 1;

		if ((sx >= bmp->cr) || (sy >= bmp->cb) || (dx < bmp->cl) || (dy < bmp->ct))
			return;

		if ((sx >= bmp->cl) && (sy >= bmp->ct) && (dx < bmp->cr) && (dy < bmp->cb))
			bmp->clip = FALSE;

		clip = TRUE;
	} else
		clip = FALSE;

	acquire_bitmap(bmp);

	two_a_sq = 2 * rx * rx;
	two_b_sq = 2 * ry * ry;

	x = rx;
	y = 0;

	x_change = ry * ry * (1 - 2 * rx);
	y_change = rx * rx;
	ellipse_error = 0.0;

	/* The following two variables decide when to stop.  It's easier than
	 * solving for this explicitly.
	 */
	stopping_x = two_b_sq * rx;
	stopping_y = 0.0;

	/* First set of points */
	while (y <= ry) {
		bmp->vtable->hfill(bmp, ix - x, iy + y, ix + x, color);
		if (y) {
			bmp->vtable->hfill(bmp, ix - x, iy - y, ix + x, color);
		}

		y++;
		stopping_y += two_a_sq;
		ellipse_error += y_change;
		y_change += two_a_sq;
		midway_x = x;

		if (stopping_x < stopping_y && x > 1) {
			break;
		}

		if ((2.0f * ellipse_error + x_change) > 0.0) {
			if (x) {
				x--;
				stopping_x -= two_b_sq;
				ellipse_error += x_change;
				x_change += two_b_sq;
			}
		}
	}

	last_drawn_y = y - 1;

	/* To do the other half of the ellipse we reset to the top of it, and
	 * iterate in the opposite direction until we reach the place we stopped at
	 * last time.
	 */
	x = 0;
	y = ry;

	x_change = ry * ry;
	y_change = rx * rx * (1 - 2 * ry);
	ellipse_error = 0.0;

	old_y = y;

	while (x < midway_x) {
		if (old_y != y) {
			bmp->vtable->hfill(bmp, ix - x + 1, iy + old_y, ix + x - 1, color);
			if (old_y) {
				bmp->vtable->hfill(bmp, ix - x + 1, iy - old_y, ix + x - 1, color);
			}
		}

		x++;
		ellipse_error += x_change;
		x_change += two_b_sq;
		old_y = y;

		if ((2.0f * ellipse_error + y_change) > 0.0) {
			if (y) {
				y--;
				ellipse_error += y_change;
				y_change += two_a_sq;
			}
		}
	}

	/* On occasion, a gap appears between the middle and upper halves.
	 * This 'afterthought' fills it in.
	 */
	if (old_y != last_drawn_y) {
		bmp->vtable->hfill(bmp, ix - x + 1, iy + old_y, ix + x - 1, color);
		if (old_y) {
			bmp->vtable->hfill(bmp, ix - x + 1, iy - old_y, ix + x - 1, color);
		}
	}

	release_bitmap(bmp);

	bmp->clip = clip;
}

/* get_point_on_arc:
 *  Helper function for the do_arc() function, converting from (radius, angle)
 *  to (x, y).
 */
static INLINE void get_point_on_arc(int r, fixed a, int *out_x, int *out_y, int *out_q) {
	double s, c;
	double double_a = (a & 0xffffff) * (AL_PI * 2 / (1 << 24));
	s = sin(double_a);
	c = cos(double_a);
	s = -s * r;
	c = c * r;
	*out_x = (int)((c < 0) ? (c - 0.5) : (c + 0.5));
	*out_y = (int)((s < 0) ? (s - 0.5) : (s + 0.5));

	if (c >= 0) {
		if (s <= 0)
			*out_q = 0; /* quadrant 0 */
		else
			*out_q = 3; /* quadrant 3 */
	} else {
		if (s <= 0)
			*out_q = 1; /* quadrant 1 */
		else
			*out_q = 2; /* quadrant 2 */
	}
}

/* do_arc:
 *  Helper function for the arc function. Calculates the points in an arc
 *  of radius r around point x, y, going anticlockwise from fixed point
 *  binary angle ang1 to ang2, and calls the specified routine for each one.
 *  The output proc will be passed first a copy of the bmp parameter, then
 *  the x, y point, then a copy of the d parameter (so putpixel() can be
 *  used as the callback).
 */
void do_arc(BITMAP *bmp, int x, int y, fixed ang1, fixed ang2, int r, int d, void (*proc)(BITMAP *, int, int, int)) {
	/* start position */
	int sx, sy;
	/* current position */
	int px, py;
	/* end position */
	int ex, ey;
	/* square of radius of circle */
	long rr;
	/* difference between main radius squared and radius squared of three
	   potential next points */
	long rr1, rr2, rr3;
	/* square of x and of y */
	unsigned long xx, yy, xx_new, yy_new;
	/* start quadrant, current quadrant and end quadrant */
	int sq, q, qe;
	/* direction of movement */
	int dx, dy;
	/* temporary variable for determining if we have reached end point */
	int det;

	/* Calculate the start point and the end point. */
	/* We have to flip y because bitmaps count y coordinates downwards. */
	get_point_on_arc(r, ang1, &sx, &sy, &q);
	px = sx;
	py = sy;
	get_point_on_arc(r, ang2, &ex, &ey, &qe);

	rr = r * r;
	xx = px * px;
	yy = py * py - rr;

	sq = q;

	if (q > qe) {
		/* qe must come after q. */
		qe += 4;
	} else if (q == qe) {
		/* If q==qe but the beginning comes after the end, make qe be
		 * strictly after q.
		 */
		if (((ang2 & 0xffffff) < (ang1 & 0xffffff)) ||
				(((ang1 & 0xffffff) < 0x400000) && ((ang2 & 0xffffff) >= 0xc00000)))
			qe += 4;
	}

	/* initial direction of movement */
	if (((q + 1) & 2) == 0)
		dy = -1;
	else
		dy = 1;
	if ((q & 2) == 0)
		dx = -1;
	else
		dx = 1;

	while (TRUE) {
		/* Change quadrant when needed.
		 * dx and dy determine the possible directions to go in this
		 * quadrant, so they must be updated when we change quadrant.
		 */
		if ((q & 1) == 0) {
			if (px == 0) {
				if (qe == q)
					break;
				q++;
				dy = -dy;
			}
		} else {
			if (py == 0) {
				if (qe == q)
					break;
				q++;
				dx = -dx;
			}
		}

		/* Are we in the final quadrant? */
		if (qe == q) {
			/* Have we reached (or passed) the end point both in x and y? */
			det = 0;

			if (dy > 0) {
				if (py >= ey)
					det++;
			} else {
				if (py <= ey)
					det++;
			}
			if (dx > 0) {
				if (px >= ex)
					det++;
			} else {
				if (px <= ex)
					det++;
			}

			if (det == 2)
				break;
		}

		proc(bmp, x + px, y + py, d);

		/* From here, we have only 3 possible directions of movement, eg.
		 * for the first quadrant:
		 *
		 *    .........
		 *    .........
		 *    ......21.
		 *    ......3*.
		 *
		 * These are reached by adding dx to px and/or adding dy to py.
		 * We need to find which of these points gives the best
		 * approximation of the (square of the) radius.
		 */

		xx_new = (px + dx) * (px + dx);
		yy_new = (py + dy) * (py + dy) - rr;
		rr1 = xx_new + yy;
		rr2 = xx_new + yy_new;
		rr3 = xx + yy_new;

		/* Set rr1, rr2, rr3 to be the difference from the main radius of the
		 * three points.
		 */
		if (rr1 < 0)
			rr1 = -rr1;
		if (rr2 < 0)
			rr2 = -rr2;
		if (rr3 < 0)
			rr3 = -rr3;

		if (rr3 >= MIN(rr1, rr2)) {
			px += dx;
			xx = xx_new;
		}
		if (rr1 > MIN(rr2, rr3)) {
			py += dy;
			yy = yy_new;
		}
	}
	/* Only draw last point if it doesn't overlap with first one. */
	if ((px != sx) || (py != sy) || (sq == qe))
		proc(bmp, x + px, y + py, d);
}

/* arc:
 *  Draws an arc.
 */
void _soft_arc(BITMAP *bmp, int x, int y, fixed ang1, fixed ang2, int r, int color) {
	ASSERT(bmp);
	acquire_bitmap(bmp);

	do_arc(bmp, x, y, ang1, ang2, r, color, bmp->vtable->putpixel);

	release_bitmap(bmp);
}

/* fill_edge_structure:
 *  Polygon helper function: initialises an edge structure for the 2d
 *  rasteriser.
 */
static void fill_edge_structure(POLYGON_EDGE *edge, AL_CONST int *i1, AL_CONST int *i2) {
	if (i2[1] < i1[1]) {
		AL_CONST int *it;

		it = i1;
		i1 = i2;
		i2 = it;
	}

	edge->top = i1[1];
	edge->bottom = i2[1];
	edge->x = (i1[0] << POLYGON_FIX_SHIFT) + (1 << (POLYGON_FIX_SHIFT - 1));
	if (i2[1] != i1[1]) {
		edge->dx = ((i2[0] - i1[0]) << POLYGON_FIX_SHIFT) / (i2[1] - i1[1]);
	} else {
		edge->dx = ((i2[0] - i1[0]) << POLYGON_FIX_SHIFT) << 1;
	}
	edge->w = MAX(ABS(edge->dx) - 1, 0);
	edge->prev = NULL;
	edge->next = NULL;
	if (edge->dx < 0)
		edge->x += edge->dx / 2;
}

/* _add_edge:
 *  Adds an edge structure to a linked list, returning the new head pointer.
 */
POLYGON_EDGE *_add_edge(POLYGON_EDGE *list, POLYGON_EDGE *edge, int sort_by_x) {
	POLYGON_EDGE *pos = list;
	POLYGON_EDGE *prev = NULL;

	if (sort_by_x) {
		while ((pos) && (pos->x < edge->x)) {
			prev = pos;
			pos = pos->next;
		}
	} else {
		while ((pos) && (pos->top < edge->top)) {
			prev = pos;
			pos = pos->next;
		}
	}

	edge->next = pos;
	edge->prev = prev;

	if (pos)
		pos->prev = edge;

	if (prev) {
		prev->next = edge;
		return list;
	} else
		return edge;
}

/* _remove_edge:
 *  Removes an edge structure from a list, returning the new head pointer.
 */
POLYGON_EDGE *_remove_edge(POLYGON_EDGE *list, POLYGON_EDGE *edge) {
	if (edge->next)
		edge->next->prev = edge->prev;

	if (edge->prev) {
		edge->prev->next = edge->next;
		return list;
	} else
		return edge->next;
}

/* polygon:
 *  Draws a filled polygon with an arbitrary number of corners. Pass the
 *  number of vertices, then an array containing a series of x, y points
 *  (a total of vertices*2 values).
 */
void _soft_polygon(BITMAP *bmp, int vertices, AL_CONST int *points, int color) {
	int top = INT_MAX;
	int bottom = INT_MIN;
	AL_CONST int *i1, *i2;
	POLYGON_EDGE *edge, *next_edge;
	POLYGON_EDGE *active_edges = NULL;
	POLYGON_EDGE *inactive_edges = NULL;
	ASSERT(bmp);

	/* allocate some space and fill the edge table */
	_grow_scratch_mem(sizeof(POLYGON_EDGE) * vertices);

	edge = (POLYGON_EDGE *)_scratch_mem;
	i1 = points;
	i2 = points + (vertices - 1) * 2;

	for (int c = 0; c < vertices; c++) {
		fill_edge_structure(edge, i1, i2);

		if (edge->bottom >= edge->top) {
			if (edge->top < top)
				top = edge->top;

			if (edge->bottom > bottom)
				bottom = edge->bottom;

			inactive_edges = _add_edge(inactive_edges, edge, FALSE);
			edge++;
		}

		i2 = i1;
		i1 += 2;
	}

	if (bottom >= bmp->cb)
		bottom = bmp->cb - 1;

	acquire_bitmap(bmp);

	/* for each scanline in the polygon... */
	for (int c = top; c <= bottom; c++) {
		int hid = 0;
		int b1 = 0;
		int e1 = 0;
		int up = 0;
		int draw = 0;
		int e;

		/* check for newly active edges */
		edge = inactive_edges;
		while ((edge) && (edge->top == c)) {
			next_edge = edge->next;
			inactive_edges = _remove_edge(inactive_edges, edge);
			active_edges = _add_edge(active_edges, edge, TRUE);
			edge = next_edge;
		}

		/* draw horizontal line segments */
		edge = active_edges;
		while (edge) {
			e = edge->w;
			if (edge->bottom != c) {
				up = 1 - up;
			} else {
				e = edge->w >> 1;
			}

			if (edge->top == c) {
				e = edge->w >> 1;
			}

			if ((draw < 1) && (up >= 1)) {
				b1 = (edge->x + e) >> POLYGON_FIX_SHIFT;
			} else if (draw >= 1) {
				/* filling the polygon */
				e1 = edge->x >> POLYGON_FIX_SHIFT;
				hid = MAX(hid, b1 + 1);

				if (hid <= e1 - 1) {
					bmp->vtable->hfill(bmp, hid, c, e1 - 1, color);
				}

				b1 = (edge->x + e) >> POLYGON_FIX_SHIFT;
			}

			/* drawing the edge */
			hid = MAX(hid, edge->x >> POLYGON_FIX_SHIFT);
			if (hid <= ((edge->x + e) >> POLYGON_FIX_SHIFT)) {
				bmp->vtable->hfill(bmp, hid, c, (edge->x + e) >> POLYGON_FIX_SHIFT, color);
				hid = 1 + ((edge->x + e) >> POLYGON_FIX_SHIFT);
			}

			edge = edge->next;
			draw = up;
		}

		/* update edges, sorting and removing dead ones */
		edge = active_edges;
		while (edge) {
			next_edge = edge->next;
			if (c >= edge->bottom) {
				active_edges = _remove_edge(active_edges, edge);
			} else {
				edge->x += edge->dx;
				if ((edge->top == c) && (edge->dx > 0)) {
					edge->x -= edge->dx / 2;
				}
				if ((edge->bottom == c + 1) && (edge->dx < 0)) {
					edge->x -= edge->dx / 2;
				}
				while ((edge->prev) && (edge->x < edge->prev->x)) {
					if (edge->next)
						edge->next->prev = edge->prev;
					edge->prev->next = edge->next;
					edge->next = edge->prev;
					edge->prev = edge->prev->prev;
					edge->next->prev = edge;
					if (edge->prev)
						edge->prev->next = edge;
					else
						active_edges = edge;
				}
			}
			edge = next_edge;
		}
	}

	release_bitmap(bmp);
}

/* triangle:
 *  Draws a filled triangle between the three points.
 */
void _soft_triangle(BITMAP *bmp, int x1, int y1, int x2, int y2, int x3, int y3, int color) {
	ASSERT(bmp);

#if (defined ALLEGRO_GCC) && (defined ALLEGRO_I386)
	/* note: this depends on a dodgy assumption about parameter passing
	 * conventions. I assume that the point coordinates are all on the
	 * stack in consecutive locations, so I can pass that block of stack
	 * memory as the array for polygon() without bothering to copy the
	 * data to a temporary location.
	 */
	polygon(bmp, 3, &x1, color);
#else
	{
		/* portable version for other platforms */
		int point[6];

		point[0] = x1;
		point[1] = y1;
		point[2] = x2;
		point[3] = y2;
		point[4] = x3;
		point[5] = y3;

		polygon(bmp, 3, point, color);
	}
#endif
}

void _poly_scanline_dummy(uintptr_t addr, int w, POLYGON_SEGMENT *info) {}

ZBUFFER *_zbuffer = NULL;

SCANLINE_FILLER _optim_alternative_drawer;

/* _fill_3d_edge_structure:
 *  Polygon helper function: initialises an edge structure for the 3d
 *  rasterising code, using fixed point vertex structures. Returns 1 on
 *  success, or 0 if the edge is horizontal or clipped out of existence.
 */
int _fill_3d_edge_structure(POLYGON_EDGE *edge, AL_CONST V3D *v1, AL_CONST V3D *v2, int flags, BITMAP *bmp) {
	int r1, r2, g1, g2, b1, b2;
	fixed h, step;

	/* swap vertices if they are the wrong way up */
	if (v2->y < v1->y) {
		AL_CONST V3D *vt;

		vt = v1;
		v1 = v2;
		v2 = vt;
	}

	/* set up screen rasterising parameters */
	edge->top = fixceil(v1->y);
	edge->bottom = fixceil(v2->y) - 1;

	if (edge->bottom < edge->top)
		return 0;

	h = v2->y - v1->y;
	step = (edge->top << 16) - v1->y;

	edge->dx = fixdiv(v2->x - v1->x, h);
	edge->x = v1->x + fixmul(step, edge->dx);

	edge->prev = NULL;
	edge->next = NULL;
	edge->w = 0;

	if (flags & INTERP_Z) {
		float h1 = 65536. / h;
		float step_f = fixtof(step);

		/* Z (depth) interpolation */
		float z1 = 65536. / v1->z;
		float z2 = 65536. / v2->z;

		edge->dat.dz = (z2 - z1) * h1;
		edge->dat.z = z1 + edge->dat.dz * step_f;

		if (flags & INTERP_FLOAT_UV) {
			/* floating point (perspective correct) texture interpolation */
			float fu1 = v1->u * z1;
			float fv1 = v1->v * z1;
			float fu2 = v2->u * z2;
			float fv2 = v2->v * z2;

			edge->dat.dfu = (fu2 - fu1) * h1;
			edge->dat.dfv = (fv2 - fv1) * h1;
			edge->dat.fu = fu1 + edge->dat.dfu * step_f;
			edge->dat.fv = fv1 + edge->dat.dfv * step_f;
		}
	}

	if (flags & INTERP_FLAT) {
		/* if clipping is enabled then clip edge */
		if (bmp->clip) {
			if (edge->top < bmp->ct) {
				edge->x += (bmp->ct - edge->top) * edge->dx;
				edge->top = bmp->ct;
			}

			if (edge->bottom >= bmp->cb)
				edge->bottom = bmp->cb - 1;
		}

		return (edge->bottom >= edge->top);
	}

	if (flags & INTERP_1COL) {
		/* single color shading interpolation */
		edge->dat.dc = fixdiv(itofix(v2->c - v1->c), h);
		edge->dat.c = itofix(v1->c) + fixmul(step, edge->dat.dc);
	}

	if (flags & INTERP_3COL) {
		/* RGB shading interpolation */
		if (flags & COLOR_TO_RGB) {
			AL_CONST int coldepth = bitmap_color_depth(bmp);
			r1 = getr_depth(coldepth, v1->c);
			r2 = getr_depth(coldepth, v2->c);
			g1 = getg_depth(coldepth, v1->c);
			g2 = getg_depth(coldepth, v2->c);
			b1 = getb_depth(coldepth, v1->c);
			b2 = getb_depth(coldepth, v2->c);
		} else {
			r1 = (v1->c >> 16) & 0xFF;
			r2 = (v2->c >> 16) & 0xFF;
			g1 = (v1->c >> 8) & 0xFF;
			g2 = (v2->c >> 8) & 0xFF;
			b1 = v1->c & 0xFF;
			b2 = v2->c & 0xFF;
		}

		edge->dat.dr = fixdiv(itofix(r2 - r1), h);
		edge->dat.dg = fixdiv(itofix(g2 - g1), h);
		edge->dat.db = fixdiv(itofix(b2 - b1), h);
		edge->dat.r = itofix(r1) + fixmul(step, edge->dat.dr);
		edge->dat.g = itofix(g1) + fixmul(step, edge->dat.dg);
		edge->dat.b = itofix(b1) + fixmul(step, edge->dat.db);
	}

	if (flags & INTERP_FIX_UV) {
		/* fixed point (affine) texture interpolation */
		edge->dat.du = fixdiv(v2->u - v1->u, h);
		edge->dat.dv = fixdiv(v2->v - v1->v, h);
		edge->dat.u = v1->u + fixmul(step, edge->dat.du);
		edge->dat.v = v1->v + fixmul(step, edge->dat.dv);
	}

	/* if clipping is enabled then clip edge */
	if (bmp->clip) {
		if (edge->top < bmp->ct) {
			int gap = bmp->ct - edge->top;
			edge->top = bmp->ct;
			edge->x += gap * edge->dx;
			_clip_polygon_segment(&(edge->dat), itofix(gap), flags);
		}

		if (edge->bottom >= bmp->cb)
			edge->bottom = bmp->cb - 1;
	}

	return (edge->bottom >= edge->top);
}

/* _fill_3d_edge_structure_f:
 *  Polygon helper function: initialises an edge structure for the 3d
 *  rasterising code, using floating point vertex structures. Returns 1 on
 *  success, or 0 if the edge is horizontal or clipped out of existence.
 */
int _fill_3d_edge_structure_f(POLYGON_EDGE *edge, AL_CONST V3D_f *v1, AL_CONST V3D_f *v2, int flags, BITMAP *bmp) {
	int r1, r2, g1, g2, b1, b2;
	fixed h, step;
	float h1;

	/* swap vertices if they are the wrong way up */
	if (v2->y < v1->y) {
		AL_CONST V3D_f *vt;

		vt = v1;
		v1 = v2;
		v2 = vt;
	}

	/* set up screen rasterising parameters */
	edge->top = fixceil(ftofix(v1->y));
	edge->bottom = fixceil(ftofix(v2->y)) - 1;

	if (edge->bottom < edge->top)
		return 0;

	h1 = 1.0 / (v2->y - v1->y);
	h = ftofix(v2->y - v1->y);
	step = (edge->top << 16) - ftofix(v1->y);

	edge->dx = ftofix((v2->x - v1->x) * h1);
	edge->x = ftofix(v1->x) + fixmul(step, edge->dx);

	edge->prev = NULL;
	edge->next = NULL;
	edge->w = 0;

	if (flags & INTERP_Z) {
		float step_f = fixtof(step);

		/* Z (depth) interpolation */
		float z1 = 1. / v1->z;
		float z2 = 1. / v2->z;

		edge->dat.dz = (z2 - z1) * h1;
		edge->dat.z = z1 + edge->dat.dz * step_f;

		if (flags & INTERP_FLOAT_UV) {
			/* floating point (perspective correct) texture interpolation */
			float fu1 = v1->u * z1 * 65536.;
			float fv1 = v1->v * z1 * 65536.;
			float fu2 = v2->u * z2 * 65536.;
			float fv2 = v2->v * z2 * 65536.;

			edge->dat.dfu = (fu2 - fu1) * h1;
			edge->dat.dfv = (fv2 - fv1) * h1;
			edge->dat.fu = fu1 + edge->dat.dfu * step_f;
			edge->dat.fv = fv1 + edge->dat.dfv * step_f;
		}
	}

	if (flags & INTERP_FLAT) {
		/* if clipping is enabled then clip edge */
		if (bmp->clip) {
			if (edge->top < bmp->ct) {
				edge->x += (bmp->ct - edge->top) * edge->dx;
				edge->top = bmp->ct;
			}

			if (edge->bottom >= bmp->cb)
				edge->bottom = bmp->cb - 1;
		}

		return (edge->bottom >= edge->top);
	}

	if (flags & INTERP_1COL) {
		/* single color shading interpolation */
		edge->dat.dc = fixdiv(itofix(v2->c - v1->c), h);
		edge->dat.c = itofix(v1->c) + fixmul(step, edge->dat.dc);
	}

	if (flags & INTERP_3COL) {
		/* RGB shading interpolation */
		if (flags & COLOR_TO_RGB) {
			AL_CONST int coldepth = bitmap_color_depth(bmp);
			r1 = getr_depth(coldepth, v1->c);
			r2 = getr_depth(coldepth, v2->c);
			g1 = getg_depth(coldepth, v1->c);
			g2 = getg_depth(coldepth, v2->c);
			b1 = getb_depth(coldepth, v1->c);
			b2 = getb_depth(coldepth, v2->c);
		} else {
			r1 = (v1->c >> 16) & 0xFF;
			r2 = (v2->c >> 16) & 0xFF;
			g1 = (v1->c >> 8) & 0xFF;
			g2 = (v2->c >> 8) & 0xFF;
			b1 = v1->c & 0xFF;
			b2 = v2->c & 0xFF;
		}

		edge->dat.dr = fixdiv(itofix(r2 - r1), h);
		edge->dat.dg = fixdiv(itofix(g2 - g1), h);
		edge->dat.db = fixdiv(itofix(b2 - b1), h);
		edge->dat.r = itofix(r1) + fixmul(step, edge->dat.dr);
		edge->dat.g = itofix(g1) + fixmul(step, edge->dat.dg);
		edge->dat.b = itofix(b1) + fixmul(step, edge->dat.db);
	}

	if (flags & INTERP_FIX_UV) {
		/* fixed point (affine) texture interpolation */
		edge->dat.du = ftofix((v2->u - v1->u) * h1);
		edge->dat.dv = ftofix((v2->v - v1->v) * h1);
		edge->dat.u = ftofix(v1->u) + fixmul(step, edge->dat.du);
		edge->dat.v = ftofix(v1->v) + fixmul(step, edge->dat.dv);
	}

	/* if clipping is enabled then clip edge */
	if (bmp->clip) {
		if (edge->top < bmp->ct) {
			int gap = bmp->ct - edge->top;
			edge->top = bmp->ct;
			edge->x += gap * edge->dx;
			_clip_polygon_segment_f(&(edge->dat), gap, flags);
		}

		if (edge->bottom >= bmp->cb)
			edge->bottom = bmp->cb - 1;
	}

	return (edge->bottom >= edge->top);
}

/* _get_scanline_filler:
 *  Helper function for deciding which rasterisation function and
 *  interpolation flags we should use for a specific polygon type.
 */
SCANLINE_FILLER _get_scanline_filler(int type, int *flags, POLYGON_SEGMENT *info, BITMAP *texture, BITMAP *bmp) {
	typedef struct POLYTYPE_INFO {
		SCANLINE_FILLER filler;
		SCANLINE_FILLER alternative;
	} POLYTYPE_INFO;

	static int polytype_interp_pal[] = {
		INTERP_FLAT,
		INTERP_1COL,
		INTERP_3COL,
		INTERP_FIX_UV,
		INTERP_Z | INTERP_FLOAT_UV | OPT_FLOAT_UV_TO_FIX,
		INTERP_FIX_UV,
		INTERP_Z | INTERP_FLOAT_UV | OPT_FLOAT_UV_TO_FIX,
		INTERP_FIX_UV | INTERP_1COL,
		INTERP_Z | INTERP_FLOAT_UV | INTERP_1COL | OPT_FLOAT_UV_TO_FIX,
		INTERP_FIX_UV | INTERP_1COL,
		INTERP_Z | INTERP_FLOAT_UV | INTERP_1COL | OPT_FLOAT_UV_TO_FIX,
		INTERP_FIX_UV,
		INTERP_Z | INTERP_FLOAT_UV | OPT_FLOAT_UV_TO_FIX,
		INTERP_FIX_UV,
		INTERP_Z | INTERP_FLOAT_UV | OPT_FLOAT_UV_TO_FIX
	};

	static int polytype_interp_tc[] = {
		INTERP_FLAT,
		INTERP_3COL | COLOR_TO_RGB,
		INTERP_3COL,
		INTERP_FIX_UV,
		INTERP_Z | INTERP_FLOAT_UV | OPT_FLOAT_UV_TO_FIX,
		INTERP_FIX_UV,
		INTERP_Z | INTERP_FLOAT_UV | OPT_FLOAT_UV_TO_FIX,
		INTERP_FIX_UV | INTERP_1COL,
		INTERP_Z | INTERP_FLOAT_UV | INTERP_1COL | OPT_FLOAT_UV_TO_FIX,
		INTERP_FIX_UV | INTERP_1COL,
		INTERP_Z | INTERP_FLOAT_UV | INTERP_1COL | OPT_FLOAT_UV_TO_FIX,
		INTERP_FIX_UV,
		INTERP_Z | INTERP_FLOAT_UV | OPT_FLOAT_UV_TO_FIX,
		INTERP_FIX_UV,
		INTERP_Z | INTERP_FLOAT_UV | OPT_FLOAT_UV_TO_FIX
	};

#ifdef ALLEGRO_COLOR8
	static POLYTYPE_INFO polytype_info8[] = {
		{ _poly_scanline_dummy, NULL },
		{ _poly_scanline_gcol8, NULL },
		{ _poly_scanline_grgb8, NULL },
		{ _poly_scanline_atex8, NULL },
		{ _poly_scanline_ptex8, _poly_scanline_atex8 },
		{ _poly_scanline_atex_mask8, NULL },
		{ _poly_scanline_ptex_mask8, _poly_scanline_atex_mask8 },
		{ _poly_scanline_atex_lit8, NULL },
		{ _poly_scanline_ptex_lit8, _poly_scanline_atex_lit8 },
		{ _poly_scanline_atex_mask_lit8, NULL },
		{ _poly_scanline_ptex_mask_lit8, _poly_scanline_atex_mask_lit8 },
		{ _poly_scanline_atex_trans8, NULL },
		{ _poly_scanline_ptex_trans8, _poly_scanline_atex_trans8 },
		{ _poly_scanline_atex_mask_trans8, NULL },
		{ _poly_scanline_ptex_mask_trans8, _poly_scanline_atex_mask_trans8 }
	};

#ifdef ALLEGRO_MMX
	static POLYTYPE_INFO polytype_info8x[] = {
		{ NULL, NULL },
		{ NULL, NULL },
		{ _poly_scanline_grgb8x, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL }
	};

	static POLYTYPE_INFO polytype_info8d[] = {
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL }
	};
#endif
#endif

#ifdef ALLEGRO_COLOR16
	static POLYTYPE_INFO polytype_info15[] = {
		{ _poly_scanline_dummy, NULL },
		{ _poly_scanline_grgb15, NULL },
		{ _poly_scanline_grgb15, NULL },
		{ _poly_scanline_atex16, NULL },
		{ _poly_scanline_ptex16, _poly_scanline_atex16 },
		{ _poly_scanline_atex_mask15, NULL },
		{ _poly_scanline_ptex_mask15, _poly_scanline_atex_mask15 },
		{ _poly_scanline_atex_lit15, NULL },
		{ _poly_scanline_ptex_lit15, _poly_scanline_atex_lit15 },
		{ _poly_scanline_atex_mask_lit15, NULL },
		{ _poly_scanline_ptex_mask_lit15, _poly_scanline_atex_mask_lit15 },
		{ _poly_scanline_atex_trans15, NULL },
		{ _poly_scanline_ptex_trans15, _poly_scanline_atex_trans15 },
		{ _poly_scanline_atex_mask_trans15, NULL },
		{ _poly_scanline_ptex_mask_trans15, _poly_scanline_atex_mask_trans15 }
	};

#ifdef ALLEGRO_MMX
	static POLYTYPE_INFO polytype_info15x[] = {
		{ NULL, NULL },
		{ _poly_scanline_grgb15x, NULL },
		{ _poly_scanline_grgb15x, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ _poly_scanline_atex_lit15x, NULL },
		{ _poly_scanline_ptex_lit15x, _poly_scanline_atex_lit15x },
		{ _poly_scanline_atex_mask_lit15x, NULL },
		{ _poly_scanline_ptex_mask_lit15x, _poly_scanline_atex_mask_lit15x },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL }
	};

	static POLYTYPE_INFO polytype_info15d[] = {
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ _poly_scanline_ptex_lit15d, _poly_scanline_atex_lit15x },
		{ NULL, NULL },
		{ _poly_scanline_ptex_mask_lit15d, _poly_scanline_atex_mask_lit15x },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL }
	};
#endif

	static POLYTYPE_INFO polytype_info16[] = {
		{ _poly_scanline_dummy, NULL },
		{ _poly_scanline_grgb16, NULL },
		{ _poly_scanline_grgb16, NULL },
		{ _poly_scanline_atex16, NULL },
		{ _poly_scanline_ptex16, _poly_scanline_atex16 },
		{ _poly_scanline_atex_mask16, NULL },
		{ _poly_scanline_ptex_mask16, _poly_scanline_atex_mask16 },
		{ _poly_scanline_atex_lit16, NULL },
		{ _poly_scanline_ptex_lit16, _poly_scanline_atex_lit16 },
		{ _poly_scanline_atex_mask_lit16, NULL },
		{ _poly_scanline_ptex_mask_lit16, _poly_scanline_atex_mask_lit16 },
		{ _poly_scanline_atex_trans16, NULL },
		{ _poly_scanline_ptex_trans16, _poly_scanline_atex_trans16 },
		{ _poly_scanline_atex_mask_trans16, NULL },
		{ _poly_scanline_ptex_mask_trans16, _poly_scanline_atex_mask_trans16 }
	};

#ifdef ALLEGRO_MMX
	static POLYTYPE_INFO polytype_info16x[] = {
		{ NULL, NULL },
		{ _poly_scanline_grgb16x, NULL },
		{ _poly_scanline_grgb16x, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ _poly_scanline_atex_lit16x, NULL },
		{ _poly_scanline_ptex_lit16x, _poly_scanline_atex_lit16x },
		{ _poly_scanline_atex_mask_lit16x, NULL },
		{ _poly_scanline_ptex_mask_lit16x, _poly_scanline_atex_mask_lit16x },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL }
	};

	static POLYTYPE_INFO polytype_info16d[] = {
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ _poly_scanline_ptex_lit16d, _poly_scanline_atex_lit16x },
		{ NULL, NULL },
		{ _poly_scanline_ptex_mask_lit16d, _poly_scanline_atex_mask_lit16x },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL }
	};
#endif
#endif

#ifdef ALLEGRO_COLOR24
	static POLYTYPE_INFO polytype_info24[] = {
		{ _poly_scanline_dummy, NULL },
		{ _poly_scanline_grgb24, NULL },
		{ _poly_scanline_grgb24, NULL },
		{ _poly_scanline_atex24, NULL },
		{ _poly_scanline_ptex24, _poly_scanline_atex24 },
		{ _poly_scanline_atex_mask24, NULL },
		{ _poly_scanline_ptex_mask24, _poly_scanline_atex_mask24 },
		{ _poly_scanline_atex_lit24, NULL },
		{ _poly_scanline_ptex_lit24, _poly_scanline_atex_lit24 },
		{ _poly_scanline_atex_mask_lit24, NULL },
		{ _poly_scanline_ptex_mask_lit24, _poly_scanline_atex_mask_lit24 },
		{ _poly_scanline_atex_trans24, NULL },
		{ _poly_scanline_ptex_trans24, _poly_scanline_atex_trans24 },
		{ _poly_scanline_atex_mask_trans24, NULL },
		{ _poly_scanline_ptex_mask_trans24, _poly_scanline_atex_mask_trans24 }
	};

#ifdef ALLEGRO_MMX
	static POLYTYPE_INFO polytype_info24x[] = {
		{ NULL, NULL },
		{ _poly_scanline_grgb24x, NULL },
		{ _poly_scanline_grgb24x, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ _poly_scanline_atex_lit24x, NULL },
		{ _poly_scanline_ptex_lit24x, _poly_scanline_atex_lit24x },
		{ _poly_scanline_atex_mask_lit24x, NULL },
		{ _poly_scanline_ptex_mask_lit24x, _poly_scanline_atex_mask_lit24x },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL }
	};

	static POLYTYPE_INFO polytype_info24d[] = {
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ _poly_scanline_ptex_lit24d, _poly_scanline_atex_lit24x },
		{ NULL, NULL },
		{ _poly_scanline_ptex_mask_lit24d, _poly_scanline_atex_mask_lit24x },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL }
	};
#endif
#endif

#ifdef ALLEGRO_COLOR32
	static POLYTYPE_INFO polytype_info32[] = {
		{ _poly_scanline_dummy, NULL },
		{ _poly_scanline_grgb32, NULL },
		{ _poly_scanline_grgb32, NULL },
		{ _poly_scanline_atex32, NULL },
		{ _poly_scanline_ptex32, _poly_scanline_atex32 },
		{ _poly_scanline_atex_mask32, NULL },
		{ _poly_scanline_ptex_mask32, _poly_scanline_atex_mask32 },
		{ _poly_scanline_atex_lit32, NULL },
		{ _poly_scanline_ptex_lit32, _poly_scanline_atex_lit32 },
		{ _poly_scanline_atex_mask_lit32, NULL },
		{ _poly_scanline_ptex_mask_lit32, _poly_scanline_atex_mask_lit32 },
		{ _poly_scanline_atex_trans32, NULL },
		{ _poly_scanline_ptex_trans32, _poly_scanline_atex_trans32 },
		{ _poly_scanline_atex_mask_trans32, NULL },
		{ _poly_scanline_ptex_mask_trans32, _poly_scanline_atex_mask_trans32 }
	};

#ifdef ALLEGRO_MMX
	static POLYTYPE_INFO polytype_info32x[] = {
		{ NULL, NULL },
		{ _poly_scanline_grgb32x, NULL },
		{ _poly_scanline_grgb32x, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ _poly_scanline_atex_lit32x, NULL },
		{ _poly_scanline_ptex_lit32x, _poly_scanline_atex_lit32x },
		{ _poly_scanline_atex_mask_lit32x, NULL },
		{ _poly_scanline_ptex_mask_lit32x, _poly_scanline_atex_mask_lit32x },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL }
	};

	static POLYTYPE_INFO polytype_info32d[] = {
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ _poly_scanline_ptex_lit32d, _poly_scanline_atex_lit32x },
		{ NULL, NULL },
		{ _poly_scanline_ptex_mask_lit32d, _poly_scanline_atex_mask_lit32x },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL },
		{ NULL, NULL }
	};
#endif
#endif

#ifdef ALLEGRO_COLOR8
	static POLYTYPE_INFO polytype_info8z[] = {
		{ _poly_zbuf_flat8, NULL },
		{ _poly_zbuf_gcol8, NULL },
		{ _poly_zbuf_grgb8, NULL },
		{ _poly_zbuf_atex8, NULL },
		{ _poly_zbuf_ptex8, _poly_zbuf_atex8 },
		{ _poly_zbuf_atex_mask8, NULL },
		{ _poly_zbuf_ptex_mask8, _poly_zbuf_atex_mask8 },
		{ _poly_zbuf_atex_lit8, NULL },
		{ _poly_zbuf_ptex_lit8, _poly_zbuf_atex_lit8 },
		{ _poly_zbuf_atex_mask_lit8, NULL },
		{ _poly_zbuf_ptex_mask_lit8, _poly_zbuf_atex_mask_lit8 },
		{ _poly_zbuf_atex_trans8, NULL },
		{ _poly_zbuf_ptex_trans8, _poly_zbuf_atex_trans8 },
		{ _poly_zbuf_atex_mask_trans8, NULL },
		{ _poly_zbuf_ptex_mask_trans8, _poly_zbuf_atex_mask_trans8 }
	};
#endif

#ifdef ALLEGRO_COLOR16
	static POLYTYPE_INFO polytype_info15z[] = {
		{ _poly_zbuf_flat16, NULL },
		{ _poly_zbuf_grgb15, NULL },
		{ _poly_zbuf_grgb15, NULL },
		{ _poly_zbuf_atex16, NULL },
		{ _poly_zbuf_ptex16, _poly_zbuf_atex16 },
		{ _poly_zbuf_atex_mask15, NULL },
		{ _poly_zbuf_ptex_mask15, _poly_zbuf_atex_mask15 },
		{ _poly_zbuf_atex_lit15, NULL },
		{ _poly_zbuf_ptex_lit15, _poly_zbuf_atex_lit15 },
		{ _poly_zbuf_atex_mask_lit15, NULL },
		{ _poly_zbuf_ptex_mask_lit15, _poly_zbuf_atex_mask_lit15 },
		{ _poly_zbuf_atex_trans15, NULL },
		{ _poly_zbuf_ptex_trans15, _poly_zbuf_atex_trans15 },
		{ _poly_zbuf_atex_mask_trans15, NULL },
		{ _poly_zbuf_ptex_mask_trans15, _poly_zbuf_atex_mask_trans15 }
	};

	static POLYTYPE_INFO polytype_info16z[] = {
		{ _poly_zbuf_flat16, NULL },
		{ _poly_zbuf_grgb16, NULL },
		{ _poly_zbuf_grgb16, NULL },
		{ _poly_zbuf_atex16, NULL },
		{ _poly_zbuf_ptex16, _poly_zbuf_atex16 },
		{ _poly_zbuf_atex_mask16, NULL },
		{ _poly_zbuf_ptex_mask16, _poly_zbuf_atex_mask16 },
		{ _poly_zbuf_atex_lit16, NULL },
		{ _poly_zbuf_ptex_lit16, _poly_zbuf_atex_lit16 },
		{ _poly_zbuf_atex_mask_lit16, NULL },
		{ _poly_zbuf_ptex_mask_lit16, _poly_zbuf_atex_mask_lit16 },
		{ _poly_zbuf_atex_trans16, NULL },
		{ _poly_zbuf_ptex_trans16, _poly_zbuf_atex_trans16 },
		{ _poly_zbuf_atex_mask_trans16, NULL },
		{ _poly_zbuf_ptex_mask_trans16, _poly_zbuf_atex_mask_trans16 }
	};
#endif

#ifdef ALLEGRO_COLOR24
	static POLYTYPE_INFO polytype_info24z[] = {
		{ _poly_zbuf_flat24, NULL },
		{ _poly_zbuf_grgb24, NULL },
		{ _poly_zbuf_grgb24, NULL },
		{ _poly_zbuf_atex24, NULL },
		{ _poly_zbuf_ptex24, _poly_zbuf_atex24 },
		{ _poly_zbuf_atex_mask24, NULL },
		{ _poly_zbuf_ptex_mask24, _poly_zbuf_atex_mask24 },
		{ _poly_zbuf_atex_lit24, NULL },
		{ _poly_zbuf_ptex_lit24, _poly_zbuf_atex_lit24 },
		{ _poly_zbuf_atex_mask_lit24, NULL },
		{ _poly_zbuf_ptex_mask_lit24, _poly_zbuf_atex_mask_lit24 },
		{ _poly_zbuf_atex_trans24, NULL },
		{ _poly_zbuf_ptex_trans24, _poly_zbuf_atex_trans24 },
		{ _poly_zbuf_atex_mask_trans24, NULL },
		{ _poly_zbuf_ptex_mask_trans24, _poly_zbuf_atex_mask_trans24 }
	};
#endif

#ifdef ALLEGRO_COLOR32
	static POLYTYPE_INFO polytype_info32z[] = {
		{ _poly_zbuf_flat32, NULL },
		{ _poly_zbuf_grgb32, NULL },
		{ _poly_zbuf_grgb32, NULL },
		{ _poly_zbuf_atex32, NULL },
		{ _poly_zbuf_ptex32, _poly_zbuf_atex32 },
		{ _poly_zbuf_atex_mask32, NULL },
		{ _poly_zbuf_ptex_mask32, _poly_zbuf_atex_mask32 },
		{ _poly_zbuf_atex_lit32, NULL },
		{ _poly_zbuf_ptex_lit32, _poly_zbuf_atex_lit32 },
		{ _poly_zbuf_atex_mask_lit32, NULL },
		{ _poly_zbuf_ptex_mask_lit32, _poly_zbuf_atex_mask_lit32 },
		{ _poly_zbuf_atex_trans32, NULL },
		{ _poly_zbuf_ptex_trans32, _poly_zbuf_atex_trans32 },
		{ _poly_zbuf_atex_mask_trans32, NULL },
		{ _poly_zbuf_ptex_mask_trans32, _poly_zbuf_atex_mask_trans32 }
	};
#endif

	int zbuf = type & POLYTYPE_ZBUF;

	int *interpinfo;
	POLYTYPE_INFO *typeinfo, *typeinfo_zbuf;

#ifdef ALLEGRO_MMX
	POLYTYPE_INFO *typeinfo_mmx, *typeinfo_3d;
#endif

	switch (bitmap_color_depth(bmp)) {
#ifdef ALLEGRO_COLOR8

		case 8:
			interpinfo = polytype_interp_pal;
			typeinfo = polytype_info8;
#ifdef ALLEGRO_MMX
			typeinfo_mmx = polytype_info8x;
			typeinfo_3d = polytype_info8d;
#endif
			typeinfo_zbuf = polytype_info8z;
			break;

#endif

#ifdef ALLEGRO_COLOR16

		case 15:
			interpinfo = polytype_interp_tc;
			typeinfo = polytype_info15;
#ifdef ALLEGRO_MMX
			typeinfo_mmx = polytype_info15x;
			typeinfo_3d = polytype_info15d;
#endif
			typeinfo_zbuf = polytype_info15z;
			break;

		case 16:
			interpinfo = polytype_interp_tc;
			typeinfo = polytype_info16;
#ifdef ALLEGRO_MMX
			typeinfo_mmx = polytype_info16x;
			typeinfo_3d = polytype_info16d;
#endif
			typeinfo_zbuf = polytype_info16z;
			break;

#endif

#ifdef ALLEGRO_COLOR24

		case 24:
			interpinfo = polytype_interp_tc;
			typeinfo = polytype_info24;
#ifdef ALLEGRO_MMX
			typeinfo_mmx = polytype_info24x;
			typeinfo_3d = polytype_info24d;
#endif
			typeinfo_zbuf = polytype_info24z;
			break;

#endif

#ifdef ALLEGRO_COLOR32

		case 32:
			interpinfo = polytype_interp_tc;
			typeinfo = polytype_info32;
#ifdef ALLEGRO_MMX
			typeinfo_mmx = polytype_info32x;
			typeinfo_3d = polytype_info32d;
#endif
			typeinfo_zbuf = polytype_info32z;
			break;

#endif

		default:
			return NULL;
	}

	type = CLAMP(0, type & ~POLYTYPE_ZBUF, POLYTYPE_MAX - 1);
	*flags = interpinfo[type];

	if (texture) {
		info->texture = texture->line[0];
		info->umask = texture->w - 1;
		info->vmask = texture->h - 1;
		info->vshift = 0;
		while ((1 << info->vshift) < texture->w)
			info->vshift++;
	} else {
		info->texture = NULL;
		info->umask = info->vmask = info->vshift = 0;
	}

	info->seg = bmp->seg;

	if (zbuf) {
		*flags |= INTERP_Z + INTERP_ZBUF;
		_optim_alternative_drawer = typeinfo_zbuf[type].alternative;
		return typeinfo_zbuf[type].filler;
	}

#ifdef ALLEGRO_MMX
	if ((cpu_capabilities & CPU_MMX) && (typeinfo_mmx[type].filler)) {
		if ((cpu_capabilities & CPU_3DNOW) && (typeinfo_3d[type].filler)) {
			_optim_alternative_drawer = typeinfo_3d[type].alternative;
			return typeinfo_3d[type].filler;
		}
		_optim_alternative_drawer = typeinfo_mmx[type].alternative;
		return typeinfo_mmx[type].filler;
	}
#endif

	_optim_alternative_drawer = typeinfo[type].alternative;

	return typeinfo[type].filler;
}

/* _clip_polygon_segment_f:
 *  Updates interpolation state values when skipping several places, eg.
 *  clipping the first part of a scanline.
 */
void _clip_polygon_segment_f(POLYGON_SEGMENT *info, int gap, int flags) {
	if (flags & INTERP_1COL)
		info->c += info->dc * gap;

	if (flags & INTERP_3COL) {
		info->r += info->dr * gap;
		info->g += info->dg * gap;
		info->b += info->db * gap;
	}

	if (flags & INTERP_FIX_UV) {
		info->u += info->du * gap;
		info->v += info->dv * gap;
	}

	if (flags & INTERP_Z) {
		info->z += info->dz * gap;

		if (flags & INTERP_FLOAT_UV) {
			info->fu += info->dfu * gap;
			info->fv += info->dfv * gap;
		}
	}
}

/* draw_polygon_segment:
 *  Polygon helper function to fill a scanline. Calculates deltas for
 *  whichever values need interpolating, clips the segment, and then calls
 *  the lowlevel scanline filler.
 */
static void draw_polygon_segment(BITMAP *bmp, int ytop, int ybottom, POLYGON_EDGE *e1, POLYGON_EDGE *e2, SCANLINE_FILLER drawer, int flags, int color, POLYGON_SEGMENT *info) {
	int x, y, w, gap;
	fixed step, width;
	POLYGON_SEGMENT *s1, *s2;
	AL_CONST SCANLINE_FILLER save_drawer = drawer;

	/* ensure that e1 is the left edge and e2 is the right edge */
	if ((e2->x < e1->x) || ((e1->x == e2->x) && (e2->dx < e1->dx))) {
		POLYGON_EDGE *et = e1;
		e1 = e2;
		e2 = et;
	}

	s1 = &(e1->dat);
	s2 = &(e2->dat);

	if (flags & INTERP_FLAT)
		info->c = color;

	/* for each scanline in the polygon... */
	for (y = ytop; y <= ybottom; y++) {
		x = fixceil(e1->x);
		w = fixceil(e2->x) - x;
		drawer = save_drawer;

		if (drawer == _poly_scanline_dummy) {
			if (w > 0)
				bmp->vtable->hfill(bmp, x, y, x + w - 1, color);
		} else {
			step = (x << 16) - e1->x;
			width = e2->x - e1->x;
			/*
			 *  Nasty trick :
			 *  In order to avoid divisions by zero, width is set to -1. This way s1 and s2
			 *  are still being updated but the scanline is not drawn since w == 0.
			 */
			if (width == 0)
				width = -(1L << 16);
			/*
			 *  End of nasty trick.
			 */
			if (flags & INTERP_1COL) {
				info->dc = fixdiv(s2->c - s1->c, width);
				info->c = s1->c + fixmul(step, info->dc);
				s1->c += s1->dc;
				s2->c += s2->dc;
			}

			if (flags & INTERP_3COL) {
				info->dr = fixdiv(s2->r - s1->r, width);
				info->dg = fixdiv(s2->g - s1->g, width);
				info->db = fixdiv(s2->b - s1->b, width);
				info->r = s1->r + fixmul(step, info->dr);
				info->g = s1->g + fixmul(step, info->dg);
				info->b = s1->b + fixmul(step, info->db);

				s1->r += s1->dr;
				s2->r += s2->dr;
				s1->g += s1->dg;
				s2->g += s2->dg;
				s1->b += s1->db;
				s2->b += s2->db;
			}

			if (flags & INTERP_FIX_UV) {
				info->du = fixdiv(s2->u - s1->u, width);
				info->dv = fixdiv(s2->v - s1->v, width);
				info->u = s1->u + fixmul(step, info->du);
				info->v = s1->v + fixmul(step, info->dv);

				s1->u += s1->du;
				s2->u += s2->du;
				s1->v += s1->dv;
				s2->v += s2->dv;
			}

			if (flags & INTERP_Z) {
				float step_f = fixtof(step);
				float w1 = 65536. / width;

				info->dz = (s2->z - s1->z) * w1;
				info->z = s1->z + info->dz * step_f;
				s1->z += s1->dz;
				s2->z += s2->dz;

				if (flags & INTERP_FLOAT_UV) {
					info->dfu = (s2->fu - s1->fu) * w1;
					info->dfv = (s2->fv - s1->fv) * w1;
					info->fu = s1->fu + info->dfu * step_f;
					info->fv = s1->fv + info->dfv * step_f;

					s1->fu += s1->dfu;
					s2->fu += s2->dfu;
					s1->fv += s1->dfv;
					s2->fv += s2->dfv;
				}
			}

			/* if clipping is enabled then clip the segment */
			if (bmp->clip) {
				if (x < bmp->cl) {
					gap = bmp->cl - x;
					x = bmp->cl;
					w -= gap;
					_clip_polygon_segment_f(info, gap, flags);
				}

				if (x + w > bmp->cr)
					w = bmp->cr - x;
			}

			if (w > 0) {
				int dx = x * BYTES_PER_PIXEL(bitmap_color_depth(bmp));

				if ((flags & OPT_FLOAT_UV_TO_FIX) && (info->dz == 0)) {
					float z1 = 1. / info->z;
					info->u = info->fu * z1;
					info->v = info->fv * z1;
					info->du = info->dfu * z1;
					info->dv = info->dfv * z1;
					drawer = _optim_alternative_drawer;
				}

				if (flags & INTERP_ZBUF)
					info->zbuf_addr = bmp_write_line(_zbuffer, y) + x * sizeof(float);

				info->read_addr = bmp_read_line(bmp, y) + dx;
				drawer(bmp_write_line(bmp, y) + dx, w, info);
			}
		}

		e1->x += e1->dx;
		e2->x += e2->dx;
	}
}

/* do_polygon3d:
 *  Helper function for rendering 3d polygon, used by both the fixed point
 *  and floating point drawing functions.
 */
static void do_polygon3d(BITMAP *bmp, int top, int bottom, POLYGON_EDGE *left_edge, SCANLINE_FILLER drawer, int flags, int color, POLYGON_SEGMENT *info) {
	int ytop, ybottom;
	POLYGON_EDGE *right_edge;
	ASSERT(bmp);

	acquire_bitmap(bmp);

	if ((left_edge->prev != left_edge->next) && (left_edge->prev->top == top))
		left_edge = left_edge->prev;

	right_edge = left_edge->next;

	ytop = top;
	for (;;) {
		if (right_edge->bottom <= left_edge->bottom)
			ybottom = right_edge->bottom;
		else
			ybottom = left_edge->bottom;

		/* fill the scanline */
		draw_polygon_segment(bmp, ytop, ybottom, left_edge, right_edge, drawer, flags, color, info);

		if (ybottom >= bottom)
			break;

		/* update edges */
		if (ybottom >= left_edge->bottom)
			left_edge = left_edge->prev;
		if (ybottom >= right_edge->bottom)
			right_edge = right_edge->next;

		ytop = ybottom + 1;
	}

	release_bitmap(bmp);
}

/* polygon3d:
 *  Draws a 3d polygon in the specified mode. The vertices parameter should
 *  be followed by that many pointers to V3D structures, which describe each
 *  vertex of the polygon.
 */
void _soft_polygon3d(BITMAP *bmp, int type, BITMAP *texture, int vc, V3D *vtx[]) {
	int c;
	int flags;
	int top = INT_MAX;
	int bottom = INT_MIN;
	V3D *v1, *v2;
	POLYGON_EDGE *edge, *edge0, *start_edge;
	POLYGON_EDGE *list_edges = NULL;
	POLYGON_SEGMENT info;
	SCANLINE_FILLER drawer;
	ASSERT(bmp);

	if (vc < 3)
		return;

	/* set up the drawing mode */
	drawer = _get_scanline_filler(type, &flags, &info, texture, bmp);
	if (!drawer)
		return;

	/* allocate some space for the active edge table */
	_grow_scratch_mem(sizeof(POLYGON_EDGE) * vc);
	start_edge = edge0 = edge = (POLYGON_EDGE *)_scratch_mem;

	/* fill the double-linked list of edges (order unimportant) */
	v2 = vtx[vc - 1];

	for (c = 0; c < vc; c++) {
		v1 = v2;
		v2 = vtx[c];

		if (_fill_3d_edge_structure(edge, v1, v2, flags, bmp)) {
			if (edge->top < top) {
				top = edge->top;
				start_edge = edge;
			}

			if (edge->bottom > bottom)
				bottom = edge->bottom;

			if (list_edges) {
				list_edges->next = edge;
				edge->prev = list_edges;
			}

			list_edges = edge;
			edge++;
		}
	}

	if (list_edges) {
		/* close the double-linked list */
		edge0->prev = --edge;
		edge->next = edge0;

		/* render the polygon */
		do_polygon3d(bmp, top, bottom, start_edge, drawer, flags, vtx[0]->c, &info);
	}
}

/* polygon3d_f:
 *  Floating point version of polygon3d().
 */
void _soft_polygon3d_f(BITMAP *bmp, int type, BITMAP *texture, int vc, V3D_f *vtx[]) {
	int c;
	int flags;
	int top = INT_MAX;
	int bottom = INT_MIN;
	V3D_f *v1, *v2;
	POLYGON_EDGE *edge, *edge0, *start_edge;
	POLYGON_EDGE *list_edges = NULL;
	POLYGON_SEGMENT info;
	SCANLINE_FILLER drawer;
	ASSERT(bmp);

	if (vc < 3)
		return;

	/* set up the drawing mode */
	drawer = _get_scanline_filler(type, &flags, &info, texture, bmp);
	if (!drawer)
		return;

	/* allocate some space for the active edge table */
	_grow_scratch_mem(sizeof(POLYGON_EDGE) * vc);
	start_edge = edge0 = edge = (POLYGON_EDGE *)_scratch_mem;

	/* fill the double-linked list of edges in clockwise order */
	v2 = vtx[vc - 1];

	for (c = 0; c < vc; c++) {
		v1 = v2;
		v2 = vtx[c];

		if (_fill_3d_edge_structure_f(edge, v1, v2, flags, bmp)) {
			if (edge->top < top) {
				top = edge->top;
				start_edge = edge;
			}

			if (edge->bottom > bottom)
				bottom = edge->bottom;

			if (list_edges) {
				list_edges->next = edge;
				edge->prev = list_edges;
			}

			list_edges = edge;
			edge++;
		}
	}

	if (list_edges) {
		/* close the double-linked list */
		edge0->prev = --edge;
		edge->next = edge0;

		/* render the polygon */
		do_polygon3d(bmp, top, bottom, start_edge, drawer, flags, vtx[0]->c, &info);
	}
}

/* draw_triangle_part:
 *  Triangle helper function to fill a triangle part. Computes interpolation,
 *  clips the segment, and then calls the lowlevel scanline filler.
 */
static void draw_triangle_part(BITMAP *bmp, int ytop, int ybottom, POLYGON_EDGE *left_edge, POLYGON_EDGE *right_edge, SCANLINE_FILLER drawer, int flags, int color, POLYGON_SEGMENT *info) {
	int x, y, w;
	int gap;
	AL_CONST int test_optim = (flags & OPT_FLOAT_UV_TO_FIX) && (info->dz == 0);
	fixed step;
	POLYGON_SEGMENT *s1;

	/* ensure that left_edge and right_edge are the right way round */
	if ((right_edge->x < left_edge->x) ||
			((left_edge->x == right_edge->x) && (right_edge->dx < left_edge->dx))) {
		POLYGON_EDGE *other_edge = left_edge;
		left_edge = right_edge;
		right_edge = other_edge;
	}

	s1 = &(left_edge->dat);

	if (flags & INTERP_FLAT)
		info->c = color;

	for (y = ytop; y <= ybottom; y++) {
		x = fixceil(left_edge->x);
		w = fixceil(right_edge->x) - x;
		step = (x << 16) - left_edge->x;

		if (drawer == _poly_scanline_dummy) {
			if (w > 0)
				bmp->vtable->hfill(bmp, x, y, x + w - 1, color);
		} else {
			if (flags & INTERP_1COL) {
				info->c = s1->c + fixmul(step, info->dc);
				s1->c += s1->dc;
			}

			if (flags & INTERP_3COL) {
				info->r = s1->r + fixmul(step, info->dr);
				info->g = s1->g + fixmul(step, info->dg);
				info->b = s1->b + fixmul(step, info->db);

				s1->r += s1->dr;
				s1->g += s1->dg;
				s1->b += s1->db;
			}

			if (flags & INTERP_FIX_UV) {
				info->u = s1->u + fixmul(step, info->du);
				info->v = s1->v + fixmul(step, info->dv);

				s1->u += s1->du;
				s1->v += s1->dv;
			}

			if (flags & INTERP_Z) {
				float step_f = fixtof(step);

				info->z = s1->z + info->dz * step_f;
				s1->z += s1->dz;

				if (flags & INTERP_FLOAT_UV) {
					info->fu = s1->fu + info->dfu * step_f;
					info->fv = s1->fv + info->dfv * step_f;

					s1->fu += s1->dfu;
					s1->fv += s1->dfv;
				}
			}

			/* if clipping is enabled then clip the segment */
			if (bmp->clip) {
				if (x < bmp->cl) {
					gap = bmp->cl - x;
					x = bmp->cl;
					w -= gap;
					_clip_polygon_segment_f(info, gap, flags);
				}

				if (x + w > bmp->cr)
					w = bmp->cr - x;
			}

			if (w > 0) {
				int dx = x * BYTES_PER_PIXEL(bitmap_color_depth(bmp));

				if (test_optim) {
					float z1 = 1. / info->z;
					info->u = info->fu * z1;
					info->v = info->fv * z1;
					info->du = info->dfu * z1;
					info->dv = info->dfv * z1;
					drawer = _optim_alternative_drawer;
				}

				if (flags & INTERP_ZBUF)
					info->zbuf_addr = bmp_write_line(_zbuffer, y) + x * sizeof(float);

				info->read_addr = bmp_read_line(bmp, y) + dx;
				drawer(bmp_write_line(bmp, y) + dx, w, info);
			}
		}

		left_edge->x += left_edge->dx;
		right_edge->x += right_edge->dx;
	}
}

/* _triangle_deltas:
 *  Triangle3d helper function to calculate the deltas. (For triangles,
 *  deltas are constant over the whole triangle).
 */
static void _triangle_deltas(BITMAP *bmp, fixed w, POLYGON_SEGMENT *s1, POLYGON_SEGMENT *info, AL_CONST V3D *v, int flags) {
	if (flags & INTERP_1COL)
		info->dc = fixdiv(s1->c - itofix(v->c), w);

	if (flags & INTERP_3COL) {
		int r, g, b;

		if (flags & COLOR_TO_RGB) {
			AL_CONST int coldepth = bitmap_color_depth(bmp);
			r = getr_depth(coldepth, v->c);
			g = getg_depth(coldepth, v->c);
			b = getb_depth(coldepth, v->c);
		} else {
			r = (v->c >> 16) & 0xFF;
			g = (v->c >> 8) & 0xFF;
			b = v->c & 0xFF;
		}

		info->dr = fixdiv(s1->r - itofix(r), w);
		info->dg = fixdiv(s1->g - itofix(g), w);
		info->db = fixdiv(s1->b - itofix(b), w);
	}

	if (flags & INTERP_FIX_UV) {
		info->du = fixdiv(s1->u - v->u, w);
		info->dv = fixdiv(s1->v - v->v, w);
	}

	if (flags & INTERP_Z) {
		float w1 = 65536. / w;

		/* Z (depth) interpolation */
		float z1 = 65536. / v->z;

		info->dz = (s1->z - z1) * w1;

		if (flags & INTERP_FLOAT_UV) {
			/* floating point (perspective correct) texture interpolation */
			float fu1 = v->u * z1;
			float fv1 = v->v * z1;

			info->dfu = (s1->fu - fu1) * w1;
			info->dfv = (s1->fv - fv1) * w1;
		}
	}
}

/* _triangle_deltas_f:
 *  Floating point version of _triangle_deltas().
 */
static void _triangle_deltas_f(BITMAP *bmp, fixed w, POLYGON_SEGMENT *s1, POLYGON_SEGMENT *info, AL_CONST V3D_f *v, int flags) {
	if (flags & INTERP_1COL)
		info->dc = fixdiv(s1->c - itofix(v->c), w);

	if (flags & INTERP_3COL) {
		int r, g, b;

		if (flags & COLOR_TO_RGB) {
			AL_CONST int coldepth = bitmap_color_depth(bmp);
			r = getr_depth(coldepth, v->c);
			g = getg_depth(coldepth, v->c);
			b = getb_depth(coldepth, v->c);
		} else {
			r = (v->c >> 16) & 0xFF;
			g = (v->c >> 8) & 0xFF;
			b = v->c & 0xFF;
		}

		info->dr = fixdiv(s1->r - itofix(r), w);
		info->dg = fixdiv(s1->g - itofix(g), w);
		info->db = fixdiv(s1->b - itofix(b), w);
	}

	if (flags & INTERP_FIX_UV) {
		info->du = fixdiv(s1->u - ftofix(v->u), w);
		info->dv = fixdiv(s1->v - ftofix(v->v), w);
	}

	if (flags & INTERP_Z) {
		float w1 = 65536. / w;

		/* Z (depth) interpolation */
		float z1 = 1. / v->z;

		info->dz = (s1->z - z1) * w1;

		if (flags & INTERP_FLOAT_UV) {
			/* floating point (perspective correct) texture interpolation */
			float fu1 = v->u * z1 * 65536.;
			float fv1 = v->v * z1 * 65536.;

			info->dfu = (s1->fu - fu1) * w1;
			info->dfv = (s1->fv - fv1) * w1;
		}
	}
}

/* _clip_polygon_segment:
 *  Fixed point version of _clip_polygon_segment_f().
 */
void _clip_polygon_segment(POLYGON_SEGMENT *info, fixed gap, int flags) {
	if (flags & INTERP_1COL)
		info->c += fixmul(info->dc, gap);

	if (flags & INTERP_3COL) {
		info->r += fixmul(info->dr, gap);
		info->g += fixmul(info->dg, gap);
		info->b += fixmul(info->db, gap);
	}

	if (flags & INTERP_FIX_UV) {
		info->u += fixmul(info->du, gap);
		info->v += fixmul(info->dv, gap);
	}

	if (flags & INTERP_Z) {
		float gap_f = fixtof(gap);

		info->z += info->dz * gap_f;

		if (flags & INTERP_FLOAT_UV) {
			info->fu += info->dfu * gap_f;
			info->fv += info->dfv * gap_f;
		}
	}
}

/* triangle3d:
 *  Draws a 3d triangle.
 */
void _soft_triangle3d(BITMAP *bmp, int type, BITMAP *texture, V3D *v1, V3D *v2, V3D *v3) {
	int flags;

	int color = v1->c;
	V3D *vt1, *vt2, *vt3;
	POLYGON_EDGE edge1, edge2;
	POLYGON_SEGMENT info;
	SCANLINE_FILLER drawer;
	ASSERT(bmp);

	/* set up the drawing mode */
	drawer = _get_scanline_filler(type, &flags, &info, texture, bmp);
	if (!drawer)
		return;

	/* sort the vertices so that vt1->y <= vt2->y <= vt3->y */
	if (v1->y > v2->y) {
		vt1 = v2;
		vt2 = v1;
	} else {
		vt1 = v1;
		vt2 = v2;
	}

	if (vt1->y > v3->y) {
		vt3 = vt1;
		vt1 = v3;
	} else
		vt3 = v3;

	if (vt2->y > vt3->y) {
		V3D *vtemp = vt2;
		vt2 = vt3;
		vt3 = vtemp;
	}

	/* do 3D triangle*/
	if (_fill_3d_edge_structure(&edge1, vt1, vt3, flags, bmp)) {
		acquire_bitmap(bmp);

		/* calculate deltas */
		if (drawer != _poly_scanline_dummy) {
			fixed w, h;
			POLYGON_SEGMENT s1 = edge1.dat;

			h = vt2->y - (edge1.top << 16);
			_clip_polygon_segment(&s1, h, flags);

			w = edge1.x + fixmul(h, edge1.dx) - vt2->x;
			if (w)
				_triangle_deltas(bmp, w, &s1, &info, vt2, flags);
		}

		/* draws part between y1 and y2 */
		if (_fill_3d_edge_structure(&edge2, vt1, vt2, flags, bmp))
			draw_triangle_part(bmp, edge2.top, edge2.bottom, &edge1, &edge2, drawer, flags, color, &info);

		/* draws part between y2 and y3 */
		if (_fill_3d_edge_structure(&edge2, vt2, vt3, flags, bmp))
			draw_triangle_part(bmp, edge2.top, edge2.bottom, &edge1, &edge2, drawer, flags, color, &info);

		release_bitmap(bmp);
	}
}

/* triangle3d_f:
 *  Draws a 3d triangle.
 */
void _soft_triangle3d_f(BITMAP *bmp, int type, BITMAP *texture, V3D_f *v1, V3D_f *v2, V3D_f *v3) {
	int flags;
	int color = v1->c;
	V3D_f *vt1, *vt2, *vt3;
	POLYGON_EDGE edge1, edge2;
	POLYGON_SEGMENT info;
	SCANLINE_FILLER drawer;
	ASSERT(bmp);

	/* set up the drawing mode */
	drawer = _get_scanline_filler(type, &flags, &info, texture, bmp);
	if (!drawer)
		return;

	/* sort the vertices so that vt1->y <= vt2->y <= vt3->y */
	if (v1->y > v2->y) {
		vt1 = v2;
		vt2 = v1;
	} else {
		vt1 = v1;
		vt2 = v2;
	}

	if (vt1->y > v3->y) {
		vt3 = vt1;
		vt1 = v3;
	} else
		vt3 = v3;

	if (vt2->y > vt3->y) {
		V3D_f *vtemp = vt2;
		vt2 = vt3;
		vt3 = vtemp;
	}

	/* do 3D triangle*/
	if (_fill_3d_edge_structure_f(&edge1, vt1, vt3, flags, bmp)) {
		acquire_bitmap(bmp);

		/* calculate deltas */
		if (drawer != _poly_scanline_dummy) {
			fixed w, h;
			POLYGON_SEGMENT s1 = edge1.dat;

			h = ftofix(vt2->y) - (edge1.top << 16);
			_clip_polygon_segment(&s1, h, flags);

			w = edge1.x + fixmul(h, edge1.dx) - ftofix(vt2->x);
			if (w)
				_triangle_deltas_f(bmp, w, &s1, &info, vt2, flags);
		}

		/* draws part between y1 and y2 */
		if (_fill_3d_edge_structure_f(&edge2, vt1, vt2, flags, bmp))
			draw_triangle_part(bmp, edge2.top, edge2.bottom, &edge1, &edge2, drawer, flags, color, &info);

		/* draws part between y2 and y3 */
		if (_fill_3d_edge_structure_f(&edge2, vt2, vt3, flags, bmp))
			draw_triangle_part(bmp, edge2.top, edge2.bottom, &edge1, &edge2, drawer, flags, color, &info);

		release_bitmap(bmp);
	}
}

/* quad3d:
 *  Draws a 3d quad.
 */
void _soft_quad3d(BITMAP *bmp, int type, BITMAP *texture, V3D *v1, V3D *v2, V3D *v3, V3D *v4) {
#if (defined ALLEGRO_GCC) && (defined ALLEGRO_I386)
	ASSERT(bmp);

	/* dodgy assumption alert! See comments for triangle() */
	polygon3d(bmp, type, texture, 4, &v1);

#else

	V3D *vertex[4];
	ASSERT(bmp);

	vertex[0] = v1;
	vertex[1] = v2;
	vertex[2] = v3;
	vertex[3] = v4;
	polygon3d(bmp, type, texture, 4, vertex);

#endif
}

/* quad3d_f:
 *  Draws a 3d quad.
 */
void _soft_quad3d_f(BITMAP *bmp, int type, BITMAP *texture, V3D_f *v1, V3D_f *v2, V3D_f *v3, V3D_f *v4) {
#if (defined ALLEGRO_GCC) && (defined ALLEGRO_I386)
	ASSERT(bmp);

	/* dodgy assumption alert! See comments for triangle() */
	polygon3d_f(bmp, type, texture, 4, &v1);

#else

	V3D_f *vertex[4];
	ASSERT(bmp);

	vertex[0] = v1;
	vertex[1] = v2;
	vertex[2] = v3;
	vertex[3] = v4;
	polygon3d_f(bmp, type, texture, 4, vertex);

#endif
}

/* create_zbuffer:
 *  Creates a new Z-buffer the size of the given bitmap.
 */
ZBUFFER *create_zbuffer(BITMAP *bmp) {
	ASSERT(bmp);
	return create_bitmap_ex(32, bmp->w, bmp->h);
}

/* clear_zbuffer:
 *  Clears the given z-buffer, z is the value written in the z-buffer
 *  - it is 1/(z coordinate), z=0 meaning far away.
 */
void clear_zbuffer(ZBUFFER *zbuf, float z) {
	union {
		float zf;
		long zi;
	} _zbuf_clip;
	ASSERT(zbuf);

	_zbuf_clip.zf = z;
	clear_to_color(zbuf, _zbuf_clip.zi);
}

/* destroy_zbuffer:
 *  Destroys the given z-buffer.
 */
void destroy_zbuffer(ZBUFFER *zbuf) {
	if (zbuf) {
		if (zbuf == _zbuffer)
			_zbuffer = NULL;
		destroy_bitmap(zbuf);
	}
}

/* set_zbuffer:
 *  Makes polygon drawing routines use the given BITMAP as z-buffer.
 */
void set_zbuffer(ZBUFFER *zbuf) {
	ASSERT(zbuf);
	_zbuffer = zbuf;
}

/* create_sub_zbuffer:
 *  Creates a new sub-z-buffer of the given z-buffer. A sub-z-buffer is
 *  exactly like a sub-bitmap, buf for z-buffers.
 */
ZBUFFER *create_sub_zbuffer(ZBUFFER *parent, int x, int y, int width, int height) {
	ASSERT(parent);
	/* For now, just use the code for BITMAPs. */
	return create_sub_bitmap(parent, x, y, width, height);
}

/*
 * Bezier spline plotter.
 * ======================
 */

/* calc_spline:
 *  Calculates a set of pixels for the bezier spline defined by the four
 *  points specified in the points array. The required resolution
 *  is specified by the npts parameter, which controls how many output
 *  pixels will be stored in the x and y arrays.
 */
void calc_spline(AL_CONST int points[8], int npts, int *out_x, int *out_y) {
	/* Derivatives of x(t) and y(t). */
	double x, dx, ddx, dddx;
	double y, dy, ddy, dddy;
	int i;

	/* Temp variables used in the setup. */
	double dt, dt2, dt3;
	double xdt2_term, xdt3_term;
	double ydt2_term, ydt3_term;

	dt = 1.0 / (npts - 1);
	dt2 = (dt * dt);
	dt3 = (dt2 * dt);

	/* x coordinates. */
	xdt2_term = 3 * (points[4] - 2 * points[2] + points[0]);
	xdt3_term = points[6] + 3 * (-points[4] + points[2]) - points[0];

	xdt2_term = dt2 * xdt2_term;
	xdt3_term = dt3 * xdt3_term;

	dddx = 6 * xdt3_term;
	ddx = -6 * xdt3_term + 2 * xdt2_term;
	dx = xdt3_term - xdt2_term + 3 * dt * (points[2] - points[0]);
	x = points[0];

	out_x[0] = points[0];

	x += .5;
	for (i = 1; i < npts; i++) {
		ddx += dddx;
		dx += ddx;
		x += dx;

		out_x[i] = (int)x;
	}

	/* y coordinates. */
	ydt2_term = 3 * (points[5] - 2 * points[3] + points[1]);
	ydt3_term = points[7] + 3 * (-points[5] + points[3]) - points[1];

	ydt2_term = dt2 * ydt2_term;
	ydt3_term = dt3 * ydt3_term;

	dddy = 6 * ydt3_term;
	ddy = -6 * ydt3_term + 2 * ydt2_term;
	dy = ydt3_term - ydt2_term + dt * 3 * (points[3] - points[1]);
	y = points[1];

	out_y[0] = points[1];

	y += .5;

	for (i = 1; i < npts; i++) {
		ddy += dddy;
		dy += ddy;
		y += dy;

		out_y[i] = (int)y;
	}
}

/* spline:
 *  Draws a bezier spline onto the specified bitmap in the specified color.
 */
void _soft_spline(BITMAP *bmp, AL_CONST int points[8], int color) {
#define MAX_POINTS 64

	int xpts[MAX_POINTS], ypts[MAX_POINTS];
	int i;
	int num_points;
	int c;
	int old_drawing_mode, old_drawing_x_anchor, old_drawing_y_anchor;
	BITMAP *old_drawing_pattern;
	ASSERT(bmp);

	/* Calculate the number of points to draw. We want to draw as few as
	   possible without loosing image quality. This algorithm is rather
	   random; I have no motivation for it at all except that it seems to work
	   quite well. The length of the spline is approximated by the sum of
	   distances from first to second to third to last point. The number of
	   points to draw is then the square root of this distance. I first tried
	   to make the number of points proportional to this distance without
	   taking the square root of it, but then short splines kind of had too
	   few points and long splines had too many. Since sqrt() increases more
	   for small input than for large, it seems in a way logical to use it,
	   but I don't precisely have any mathematical proof for it. So if someone
	   has a better idea of how this could be done, don't hesitate to let us
	   know...
	*/

#undef DIST
#define DIST(x, y) (sqrt((x) * (x) + (y) * (y)))
	num_points = (int)(sqrt(DIST(points[2] - points[0], points[3] - points[1]) +
							   DIST(points[4] - points[2], points[5] - points[3]) +
							   DIST(points[6] - points[4], points[7] - points[5])) *
			1.2);

	if (num_points > MAX_POINTS)
		num_points = MAX_POINTS;

	calc_spline(points, num_points, xpts, ypts);

	acquire_bitmap(bmp);

	if ((_drawing_mode == DRAW_MODE_XOR) ||
			(_drawing_mode == DRAW_MODE_TRANS)) {
		/* Must compensate for the end pixel being drawn twice,
	   hence the mess. */
		old_drawing_mode = _drawing_mode;
		old_drawing_pattern = _drawing_pattern;
		old_drawing_x_anchor = _drawing_x_anchor;
		old_drawing_y_anchor = _drawing_y_anchor;
		for (i = 1; i < num_points - 1; i++) {
			c = getpixel(bmp, xpts[i], ypts[i]);
			line(bmp, xpts[i - 1], ypts[i - 1], xpts[i], ypts[i], color);
			solid_mode();
			putpixel(bmp, xpts[i], ypts[i], c);
			drawing_mode(old_drawing_mode, old_drawing_pattern,
					old_drawing_x_anchor, old_drawing_y_anchor);
		}
		line(bmp, xpts[i - 1], ypts[i - 1], xpts[i], ypts[i], color);
	} else {
		for (i = 1; i < num_points; i++)
			line(bmp, xpts[i - 1], ypts[i - 1], xpts[i], ypts[i], color);
	}

	release_bitmap(bmp);
}

/*
 * The floodfill routine.
 * ======================
 */

typedef struct FLOODED_LINE /* store segments which have been flooded */
{
	short flags; /* status of the segment */
	short lpos, rpos; /* left and right ends of segment */
	short y; /* y coordinate of the segment */
	int next; /* linked list if several per line */
} FLOODED_LINE;

/* Note: a 'short' is not sufficient for 'next' above in some corner cases. */

static int flood_count; /* number of flooded segments */

#define FLOOD_IN_USE 1
#define FLOOD_TODO_ABOVE 2
#define FLOOD_TODO_BELOW 4

#define FLOOD_LINE(c) (((FLOODED_LINE *)_scratch_mem) + c)

/* flooder:
 *  Fills a horizontal line around the specified position, and adds it
 *  to the list of drawn segments. Returns the first x coordinate after
 *  the part of the line which it has dealt with.
 */
static int flooder(BITMAP *bmp, int x, int y, int src_color, int dest_color) {
	FLOODED_LINE *p;
	int left = 0, right = 0;
	unsigned long addr;
	int c;

/* helper for doing checks in each color depth */
#define FLOODER(bits, size)                                            \
	{                                                                  \
		/* check start pixel */                                        \
		if ((int)bmp_read##bits(addr + x * size) != src_color)         \
			return x + 1;                                              \
                                                                       \
		/* work left from starting point */                            \
		for (left = x - 1; left >= bmp->cl; left--) {                  \
			if ((int)bmp_read##bits(addr + left * size) != src_color)  \
				break;                                                 \
		}                                                              \
                                                                       \
		/* work right from starting point */                           \
		for (right = x + 1; right < bmp->cr; right++) {                \
			if ((int)bmp_read##bits(addr + right * size) != src_color) \
				break;                                                 \
		}                                                              \
	}

	ASSERT(bmp);

	addr = bmp_read_line(bmp, y);

	switch (bitmap_color_depth(bmp)) {
#ifdef ALLEGRO_COLOR8
		case 8:
			FLOODER(8, 1);
			break;
#endif

#ifdef ALLEGRO_COLOR16
		case 15:
		case 16:
			FLOODER(16, sizeof(short));
			break;
#endif

#ifdef ALLEGRO_COLOR24
		case 24:
			FLOODER(24, 3);
			break;
#endif

#ifdef ALLEGRO_COLOR32
		case 32:
			FLOODER(32, sizeof(int32_t));
			break;
#endif
	}

	left++;
	right--;

	/* draw the line */
	bmp->vtable->hfill(bmp, left, y, right, dest_color);

	/* store it in the list of flooded segments */
	c = y;
	p = FLOOD_LINE(c);

	if (p->flags) {
		while (p->next) {
			c = p->next;
			p = FLOOD_LINE(c);
		}

		p->next = c = flood_count++;
		_grow_scratch_mem(sizeof(FLOODED_LINE) * flood_count);
		p = FLOOD_LINE(c);
	}

	p->flags = FLOOD_IN_USE;
	p->lpos = left;
	p->rpos = right;
	p->y = y;
	p->next = 0;

	if (y > bmp->ct)
		p->flags |= FLOOD_TODO_ABOVE;

	if (y + 1 < bmp->cb)
		p->flags |= FLOOD_TODO_BELOW;

	return right + 2;
}

/* check_flood_line:
 *  Checks a line segment, using the scratch buffer is to store a list of
 *  segments which have already been drawn in order to minimise the required
 *  number of tests.
 */
static int check_flood_line(BITMAP *bmp, int y, int left, int right, int src_color, int dest_color) {
	int ret = FALSE;

	while (left <= right) {
		int c = y;

		for (;;) {
			FLOODED_LINE *p = FLOOD_LINE(c);

			if ((left >= p->lpos) && (left <= p->rpos)) {
				left = p->rpos + 2;
				break;
			}

			c = p->next;

			if (!c) {
				left = flooder(bmp, left, y, src_color, dest_color);
				ret = TRUE;
				break;
			}
		}
	}

	return ret;
}

/* floodfill:
 *  Fills an enclosed area (starting at point x, y) with the specified color.
 */
void _soft_floodfill(BITMAP *bmp, int x, int y, int color) {
	int src_color;
	int c, done;
	FLOODED_LINE *p;
	ASSERT(bmp);

	/* make sure we have a valid starting point */
	if ((x < bmp->cl) || (x >= bmp->cr) || (y < bmp->ct) || (y >= bmp->cb))
		return;

	acquire_bitmap(bmp);

	/* what color to replace? */
	src_color = getpixel(bmp, x, y);
	if (src_color == color) {
		release_bitmap(bmp);
		return;
	}

	/* set up the list of flooded segments */
	_grow_scratch_mem(sizeof(FLOODED_LINE) * bmp->cb);
	flood_count = bmp->cb;
	p = _scratch_mem;
	for (c = 0; c < flood_count; c++) {
		p[c].flags = 0;
		p[c].lpos = SHRT_MAX;
		p[c].rpos = SHRT_MIN;
		p[c].y = y;
		p[c].next = 0;
	}

	/* start up the flood algorithm */
	flooder(bmp, x, y, src_color, color);

	/* continue as long as there are some segments still to test */
	do {
		done = TRUE;

		/* for each line on the screen */
		for (c = 0; c < flood_count; c++) {
			p = FLOOD_LINE(c);

			/* check below the segment? */
			if (p->flags & FLOOD_TODO_BELOW) {
				p->flags &= ~FLOOD_TODO_BELOW;
				if (check_flood_line(bmp, p->y + 1, p->lpos, p->rpos, src_color, color)) {
					done = FALSE;
					p = FLOOD_LINE(c);
				}
			}

			/* check above the segment? */
			if (p->flags & FLOOD_TODO_ABOVE) {
				p->flags &= ~FLOOD_TODO_ABOVE;
				if (check_flood_line(bmp, p->y - 1, p->lpos, p->rpos, src_color, color)) {
					done = FALSE;
					/* special case shortcut for going backwards */
					if ((c < bmp->cb) && (c > 0))
						c -= 2;
				}
			}
		}

	} while (!done);

	release_bitmap(bmp);
}

/*
 * Gouraud shaded sprite renderer.
 * ===============================
 */

/* draw_gouraud_sprite:
 *  Draws a lit or tinted sprite, interpolating the four corner colors
 *  over the surface of the image.
 */
void _soft_draw_gouraud_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y, int c1, int c2, int c3, int c4) {
	int x1 = x;
	int y1 = y;
	int x2 = x + sprite->w;
	int y2 = y + sprite->h;
	int pixel;
	uintptr_t addr;

	ASSERT(bmp);
	ASSERT(sprite);
	ASSERT(bmp->vtable->color_depth == sprite->vtable->color_depth);

	/* set up vertical gradients for left and right sides */
	fixed mc1 = itofix(c4 - c1) / sprite->h;
	fixed mc2 = itofix(c3 - c2) / sprite->h;
	fixed lc = itofix(c1);
	fixed rc = itofix(c2);

	/* check clipping */
	if (bmp->clip) {
		if (y1 < bmp->ct) {
			lc += mc1 * (bmp->ct - y1);
			rc += mc2 * (bmp->ct - y1);
			y1 = bmp->ct;
		}
		y2 = MIN(y2, bmp->cb);
		x1 = MAX(x1, bmp->cl);
		x2 = MIN(x2, bmp->cr);
	}

	for (int j = y1; j < y2; j++) {
		/* set up horizontal gradient for line */
		fixed mh = (rc - lc) / sprite->w;
		fixed hc = lc;

		/* more clip checking */
		if ((bmp->clip) && (x < bmp->cl))
			hc += mh * (bmp->cl - x);

		/* draw routines for all linear modes */
		switch (bitmap_color_depth(bmp)) {
#ifdef ALLEGRO_COLOR8
			case 8:
				addr = bmp_write_line(bmp, j) + x1;
				for (int i = x1; i < x2; i++) {
					if (sprite->line[j - y][i - x]) {
						pixel = color_map->data[fixtoi(hc)][sprite->line[j - y][i - x]];
						bmp_write8(addr, pixel);
					}
					hc += mh;
					addr++;
				}
				break;

#endif

#ifdef ALLEGRO_COLOR16
			case 15:
			case 16:
				addr = bmp_write_line(bmp, j) + x1 * sizeof(short);
				for (int i = x1; i < x2; i++) {
					pixel = ((unsigned short *)sprite->line[j - y])[i - x];
					if (pixel != bmp->vtable->mask_color) {
						if (bitmap_color_depth(bmp) == 16)
							pixel = _blender_func16(pixel, _blender_col_16, fixtoi(hc));
						else
							pixel = _blender_func15(pixel, _blender_col_15, fixtoi(hc));
						bmp_write16(addr, pixel);
					}
					hc += mh;
					addr += sizeof(short);
				}
				break;

#endif

#ifdef ALLEGRO_COLOR24
			case 24:
				addr = bmp_write_line(bmp, j) + x1 * 3;
				for (int i = x1; i < x2; i++) {
					pixel = bmp_read24((unsigned long)(sprite->line[j - y] + (i - x) * 3));
					if (pixel != MASK_COLOR_24) {
						pixel = _blender_func24(pixel, _blender_col_24, fixtoi(hc));
						bmp_write24(addr, pixel);
					}
					hc += mh;
					addr += 3;
				}
				break;

#endif

#ifdef ALLEGRO_COLOR32
			case 32:
				addr = bmp_write_line(bmp, j) + x1 * sizeof(int32_t);
				for (int i = x1; i < x2; i++) {
					pixel = ((unsigned long *)sprite->line[j - y])[i - x];
					if (pixel != MASK_COLOR_32) {
						pixel = _blender_func32(pixel, _blender_col_32, fixtoi(hc));
						bmp_write32(addr, pixel);
					}
					hc += mh;
					addr += sizeof(int32_t);
				}
				break;

#endif
		}

		lc += mc1;
		rc += mc2;
	}
}

/* Fixed point math routines and lookup tables.
 * ============================================
 */

/* clang-format off */

/* precalculated fixed point (16.16) cosines for a full circle (0-255) */
fixed _cos_tbl[512] = {
	65536L,  65531L,  65516L,  65492L,  65457L,  65413L,  65358L,  65294L,
	65220L,  65137L,  65043L,  64940L,  64827L,  64704L,  64571L,  64429L,
	64277L,  64115L,  63944L,  63763L,  63572L,  63372L,  63162L,  62943L,
	62714L,  62476L,  62228L,  61971L,  61705L,  61429L,  61145L,  60851L,
	60547L,  60235L,  59914L,  59583L,  59244L,  58896L,  58538L,  58172L,
	57798L,  57414L,  57022L,  56621L,  56212L,  55794L,  55368L,  54934L,
	54491L,  54040L,  53581L,  53114L,  52639L,  52156L,  51665L,  51166L,
	50660L,  50146L,  49624L,  49095L,  48559L,  48015L,  47464L,  46906L,
	46341L,  45769L,  45190L,  44604L,  44011L,  43412L,  42806L,  42194L,
	41576L,  40951L,  40320L,  39683L,  39040L,  38391L,  37736L,  37076L,
	36410L,  35738L,  35062L,  34380L,  33692L,  33000L,  32303L,  31600L,
	30893L,  30182L,  29466L,  28745L,  28020L,  27291L,  26558L,  25821L,
	25080L,  24335L,  23586L,  22834L,  22078L,  21320L,  20557L,  19792L,
	19024L,  18253L,  17479L,  16703L,  15924L,  15143L,  14359L,  13573L,
	12785L,  11996L,  11204L,  10411L,  9616L,   8820L,   8022L,   7224L,
	6424L,   5623L,   4821L,   4019L,   3216L,   2412L,   1608L,   804L,
	0L,      -804L,   -1608L,  -2412L,  -3216L,  -4019L,  -4821L,  -5623L,
	-6424L,  -7224L,  -8022L,  -8820L,  -9616L,  -10411L, -11204L, -11996L,
	-12785L, -13573L, -14359L, -15143L, -15924L, -16703L, -17479L, -18253L,
	-19024L, -19792L, -20557L, -21320L, -22078L, -22834L, -23586L, -24335L,
	-25080L, -25821L, -26558L, -27291L, -28020L, -28745L, -29466L, -30182L,
	-30893L, -31600L, -32303L, -33000L, -33692L, -34380L, -35062L, -35738L,
	-36410L, -37076L, -37736L, -38391L, -39040L, -39683L, -40320L, -40951L,
	-41576L, -42194L, -42806L, -43412L, -44011L, -44604L, -45190L, -45769L,
	-46341L, -46906L, -47464L, -48015L, -48559L, -49095L, -49624L, -50146L,
	-50660L, -51166L, -51665L, -52156L, -52639L, -53114L, -53581L, -54040L,
	-54491L, -54934L, -55368L, -55794L, -56212L, -56621L, -57022L, -57414L,
	-57798L, -58172L, -58538L, -58896L, -59244L, -59583L, -59914L, -60235L,
	-60547L, -60851L, -61145L, -61429L, -61705L, -61971L, -62228L, -62476L,
	-62714L, -62943L, -63162L, -63372L, -63572L, -63763L, -63944L, -64115L,
	-64277L, -64429L, -64571L, -64704L, -64827L, -64940L, -65043L, -65137L,
	-65220L, -65294L, -65358L, -65413L, -65457L, -65492L, -65516L, -65531L,
	-65536L, -65531L, -65516L, -65492L, -65457L, -65413L, -65358L, -65294L,
	-65220L, -65137L, -65043L, -64940L, -64827L, -64704L, -64571L, -64429L,
	-64277L, -64115L, -63944L, -63763L, -63572L, -63372L, -63162L, -62943L,
	-62714L, -62476L, -62228L, -61971L, -61705L, -61429L, -61145L, -60851L,
	-60547L, -60235L, -59914L, -59583L, -59244L, -58896L, -58538L, -58172L,
	-57798L, -57414L, -57022L, -56621L, -56212L, -55794L, -55368L, -54934L,
	-54491L, -54040L, -53581L, -53114L, -52639L, -52156L, -51665L, -51166L,
	-50660L, -50146L, -49624L, -49095L, -48559L, -48015L, -47464L, -46906L,
	-46341L, -45769L, -45190L, -44604L, -44011L, -43412L, -42806L, -42194L,
	-41576L, -40951L, -40320L, -39683L, -39040L, -38391L, -37736L, -37076L,
	-36410L, -35738L, -35062L, -34380L, -33692L, -33000L, -32303L, -31600L,
	-30893L, -30182L, -29466L, -28745L, -28020L, -27291L, -26558L, -25821L,
	-25080L, -24335L, -23586L, -22834L, -22078L, -21320L, -20557L, -19792L,
	-19024L, -18253L, -17479L, -16703L, -15924L, -15143L, -14359L, -13573L,
	-12785L, -11996L, -11204L, -10411L, -9616L,  -8820L,  -8022L,  -7224L,
	-6424L,  -5623L,  -4821L,  -4019L,  -3216L,  -2412L,  -1608L,  -804L,
	0L,      804L,    1608L,   2412L,   3216L,   4019L,   4821L,   5623L,
	6424L,   7224L,   8022L,   8820L,   9616L,   10411L,  11204L,  11996L,
	12785L,  13573L,  14359L,  15143L,  15924L,  16703L,  17479L,  18253L,
	19024L,  19792L,  20557L,  21320L,  22078L,  22834L,  23586L,  24335L,
	25080L,  25821L,  26558L,  27291L,  28020L,  28745L,  29466L,  30182L,
	30893L,  31600L,  32303L,  33000L,  33692L,  34380L,  35062L,  35738L,
	36410L,  37076L,  37736L,  38391L,  39040L,  39683L,  40320L,  40951L,
	41576L,  42194L,  42806L,  43412L,  44011L,  44604L,  45190L,  45769L,
	46341L,  46906L,  47464L,  48015L,  48559L,  49095L,  49624L,  50146L,
	50660L,  51166L,  51665L,  52156L,  52639L,  53114L,  53581L,  54040L,
	54491L,  54934L,  55368L,  55794L,  56212L,  56621L,  57022L,  57414L,
	57798L,  58172L,  58538L,  58896L,  59244L,  59583L,  59914L,  60235L,
	60547L,  60851L,  61145L,  61429L,  61705L,  61971L,  62228L,  62476L,
	62714L,  62943L,  63162L,  63372L,  63572L,  63763L,  63944L,  64115L,
	64277L,  64429L,  64571L,  64704L,  64827L,  64940L,  65043L,  65137L,
	65220L,  65294L,  65358L,  65413L,  65457L,  65492L,  65516L,  65531L
};

/* precalculated fixed point (16.16) tangents for a half circle (0-127) */
fixed _tan_tbl[256] = {
	0L,      804L,    1609L,   2414L,   3220L,   4026L,   4834L,   5644L,
	6455L,   7268L,   8083L,   8901L,   9721L,   10545L,  11372L,  12202L,
	13036L,  13874L,  14717L,  15564L,  16416L,  17273L,  18136L,  19005L,
	19880L,  20762L,  21650L,  22546L,  23449L,  24360L,  25280L,  26208L,
	27146L,  28093L,  29050L,  30018L,  30996L,  31986L,  32988L,  34002L,
	35030L,  36071L,  37126L,  38196L,  39281L,  40382L,  41500L,  42636L,
	43790L,  44963L,  46156L,  47369L,  48605L,  49863L,  51145L,  52451L,
	53784L,  55144L,  56532L,  57950L,  59398L,  60880L,  62395L,  63947L,
	65536L,  67165L,  68835L,  70548L,  72308L,  74116L,  75974L,  77887L,
	79856L,  81885L,  83977L,  86135L,  88365L,  90670L,  93054L,  95523L,
	98082L,  100736L, 103493L, 106358L, 109340L, 112447L, 115687L, 119071L,
	122609L, 126314L, 130198L, 134276L, 138564L, 143081L, 147847L, 152884L,
	158218L, 163878L, 169896L, 176309L, 183161L, 190499L, 198380L, 206870L,
	216043L, 225990L, 236817L, 248648L, 261634L, 275959L, 291845L, 309568L,
	329472L, 351993L, 377693L, 407305L, 441808L, 482534L, 531352L, 590958L,
	665398L, 761030L, 888450L, 1066730L,1334016L,1779314L,2669641L,5340086L,
	-2147483647L,-5340086L,-2669641L,-1779314L,-1334016L,-1066730L,-888450L,-761030L,
	-665398L,-590958L,-531352L,-482534L,-441808L,-407305L,-377693L,-351993L,
	-329472L,-309568L,-291845L,-275959L,-261634L,-248648L,-236817L,-225990L,
	-216043L,-206870L,-198380L,-190499L,-183161L,-176309L,-169896L,-163878L,
	-158218L,-152884L,-147847L,-143081L,-138564L,-134276L,-130198L,-126314L,
	-122609L,-119071L,-115687L,-112447L,-109340L,-106358L,-103493L,-100736L,
	-98082L, -95523L, -93054L, -90670L, -88365L, -86135L, -83977L, -81885L,
	-79856L, -77887L, -75974L, -74116L, -72308L, -70548L, -68835L, -67165L,
	-65536L, -63947L, -62395L, -60880L, -59398L, -57950L, -56532L, -55144L,
	-53784L, -52451L, -51145L, -49863L, -48605L, -47369L, -46156L, -44963L,
	-43790L, -42636L, -41500L, -40382L, -39281L, -38196L, -37126L, -36071L,
	-35030L, -34002L, -32988L, -31986L, -30996L, -30018L, -29050L, -28093L,
	-27146L, -26208L, -25280L, -24360L, -23449L, -22546L, -21650L, -20762L,
	-19880L, -19005L, -18136L, -17273L, -16416L, -15564L, -14717L, -13874L,
	-13036L, -12202L, -11372L, -10545L, -9721L,  -8901L,  -8083L,  -7268L,
	-6455L,  -5644L,  -4834L,  -4026L,  -3220L,  -2414L,  -1609L,  -804L
};

/* precalculated fixed point (16.16) inverse cosines (-1 to 1) */
fixed _acos_tbl[513] = {
	0x800000L,  0x7C65C7L,  0x7AE75AL,  0x79C19EL,  0x78C9BEL,  0x77EF25L,  0x772953L,  0x76733AL,
	0x75C991L,  0x752A10L,  0x74930CL,  0x740345L,  0x7379C1L,  0x72F5BAL,  0x72768FL,  0x71FBBCL,
	0x7184D3L,  0x711174L,  0x70A152L,  0x703426L,  0x6FC9B5L,  0x6F61C9L,  0x6EFC36L,  0x6E98D1L,
	0x6E3777L,  0x6DD805L,  0x6D7A5EL,  0x6D1E68L,  0x6CC40BL,  0x6C6B2FL,  0x6C13C1L,  0x6BBDAFL,
	0x6B68E6L,  0x6B1558L,  0x6AC2F5L,  0x6A71B1L,  0x6A217EL,  0x69D251L,  0x698420L,  0x6936DFL,
	0x68EA85L,  0x689F0AL,  0x685465L,  0x680A8DL,  0x67C17DL,  0x67792CL,  0x673194L,  0x66EAAFL,
	0x66A476L,  0x665EE5L,  0x6619F5L,  0x65D5A2L,  0x6591E7L,  0x654EBFL,  0x650C26L,  0x64CA18L,
	0x648890L,  0x64478CL,  0x640706L,  0x63C6FCL,  0x63876BL,  0x63484FL,  0x6309A5L,  0x62CB6AL,
	0x628D9CL,  0x625037L,  0x621339L,  0x61D69FL,  0x619A68L,  0x615E90L,  0x612316L,  0x60E7F7L,
	0x60AD31L,  0x6072C3L,  0x6038A9L,  0x5FFEE3L,  0x5FC56EL,  0x5F8C49L,  0x5F5372L,  0x5F1AE7L,
	0x5EE2A7L,  0x5EAAB0L,  0x5E7301L,  0x5E3B98L,  0x5E0473L,  0x5DCD92L,  0x5D96F3L,  0x5D6095L,
	0x5D2A76L,  0x5CF496L,  0x5CBEF2L,  0x5C898BL,  0x5C545EL,  0x5C1F6BL,  0x5BEAB0L,  0x5BB62DL,
	0x5B81E1L,  0x5B4DCAL,  0x5B19E7L,  0x5AE638L,  0x5AB2BCL,  0x5A7F72L,  0x5A4C59L,  0x5A1970L,
	0x59E6B6L,  0x59B42AL,  0x5981CCL,  0x594F9BL,  0x591D96L,  0x58EBBDL,  0x58BA0EL,  0x588889L,
	0x58572DL,  0x5825FAL,  0x57F4EEL,  0x57C40AL,  0x57934DL,  0x5762B5L,  0x573243L,  0x5701F5L,
	0x56D1CCL,  0x56A1C6L,  0x5671E4L,  0x564224L,  0x561285L,  0x55E309L,  0x55B3ADL,  0x558471L,
	0x555555L,  0x552659L,  0x54F77BL,  0x54C8BCL,  0x549A1BL,  0x546B98L,  0x543D31L,  0x540EE7L,
	0x53E0B9L,  0x53B2A7L,  0x5384B0L,  0x5356D4L,  0x532912L,  0x52FB6BL,  0x52CDDDL,  0x52A068L,
	0x52730CL,  0x5245C9L,  0x52189EL,  0x51EB8BL,  0x51BE8FL,  0x5191AAL,  0x5164DCL,  0x513825L,
	0x510B83L,  0x50DEF7L,  0x50B280L,  0x50861FL,  0x5059D2L,  0x502D99L,  0x500175L,  0x4FD564L,
	0x4FA967L,  0x4F7D7DL,  0x4F51A6L,  0x4F25E2L,  0x4EFA30L,  0x4ECE90L,  0x4EA301L,  0x4E7784L,
	0x4E4C19L,  0x4E20BEL,  0x4DF574L,  0x4DCA3AL,  0x4D9F10L,  0x4D73F6L,  0x4D48ECL,  0x4D1DF1L,
	0x4CF305L,  0x4CC829L,  0x4C9D5AL,  0x4C729AL,  0x4C47E9L,  0x4C1D45L,  0x4BF2AEL,  0x4BC826L,
	0x4B9DAAL,  0x4B733BL,  0x4B48D9L,  0x4B1E84L,  0x4AF43BL,  0x4AC9FEL,  0x4A9FCDL,  0x4A75A7L,
	0x4A4B8DL,  0x4A217EL,  0x49F77AL,  0x49CD81L,  0x49A393L,  0x4979AFL,  0x494FD5L,  0x492605L,
	0x48FC3FL,  0x48D282L,  0x48A8CFL,  0x487F25L,  0x485584L,  0x482BECL,  0x48025DL,  0x47D8D6L,
	0x47AF57L,  0x4785E0L,  0x475C72L,  0x47330AL,  0x4709ABL,  0x46E052L,  0x46B701L,  0x468DB7L,
	0x466474L,  0x463B37L,  0x461201L,  0x45E8D0L,  0x45BFA6L,  0x459682L,  0x456D64L,  0x45444BL,
	0x451B37L,  0x44F229L,  0x44C920L,  0x44A01CL,  0x44771CL,  0x444E21L,  0x44252AL,  0x43FC38L,
	0x43D349L,  0x43AA5FL,  0x438178L,  0x435894L,  0x432FB4L,  0x4306D8L,  0x42DDFEL,  0x42B527L,
	0x428C53L,  0x426381L,  0x423AB2L,  0x4211E5L,  0x41E91AL,  0x41C051L,  0x41978AL,  0x416EC5L,
	0x414601L,  0x411D3EL,  0x40F47CL,  0x40CBBBL,  0x40A2FBL,  0x407A3CL,  0x40517DL,  0x4028BEL,
	0x400000L,  0x3FD742L,  0x3FAE83L,  0x3F85C4L,  0x3F5D05L,  0x3F3445L,  0x3F0B84L,  0x3EE2C2L,
	0x3EB9FFL,  0x3E913BL,  0x3E6876L,  0x3E3FAFL,  0x3E16E6L,  0x3DEE1BL,  0x3DC54EL,  0x3D9C7FL,
	0x3D73ADL,  0x3D4AD9L,  0x3D2202L,  0x3CF928L,  0x3CD04CL,  0x3CA76CL,  0x3C7E88L,  0x3C55A1L,
	0x3C2CB7L,  0x3C03C8L,  0x3BDAD6L,  0x3BB1DFL,  0x3B88E4L,  0x3B5FE4L,  0x3B36E0L,  0x3B0DD7L,
	0x3AE4C9L,  0x3ABBB5L,  0x3A929CL,  0x3A697EL,  0x3A405AL,  0x3A1730L,  0x39EDFFL,  0x39C4C9L,
	0x399B8CL,  0x397249L,  0x3948FFL,  0x391FAEL,  0x38F655L,  0x38CCF6L,  0x38A38EL,  0x387A20L,
	0x3850A9L,  0x38272AL,  0x37FDA3L,  0x37D414L,  0x37AA7CL,  0x3780DBL,  0x375731L,  0x372D7EL,
	0x3703C1L,  0x36D9FBL,  0x36B02BL,  0x368651L,  0x365C6DL,  0x36327FL,  0x360886L,  0x35DE82L,
	0x35B473L,  0x358A59L,  0x356033L,  0x353602L,  0x350BC5L,  0x34E17CL,  0x34B727L,  0x348CC5L,
	0x346256L,  0x3437DAL,  0x340D52L,  0x33E2BBL,  0x33B817L,  0x338D66L,  0x3362A6L,  0x3337D7L,
	0x330CFBL,  0x32E20FL,  0x32B714L,  0x328C0AL,  0x3260F0L,  0x3235C6L,  0x320A8CL,  0x31DF42L,
	0x31B3E7L,  0x31887CL,  0x315CFFL,  0x313170L,  0x3105D0L,  0x30DA1EL,  0x30AE5AL,  0x308283L,
	0x305699L,  0x302A9CL,  0x2FFE8BL,  0x2FD267L,  0x2FA62EL,  0x2F79E1L,  0x2F4D80L,  0x2F2109L,
	0x2EF47DL,  0x2EC7DBL,  0x2E9B24L,  0x2E6E56L,  0x2E4171L,  0x2E1475L,  0x2DE762L,  0x2DBA37L,
	0x2D8CF4L,  0x2D5F98L,  0x2D3223L,  0x2D0495L,  0x2CD6EEL,  0x2CA92CL,  0x2C7B50L,  0x2C4D59L,
	0x2C1F47L,  0x2BF119L,  0x2BC2CFL,  0x2B9468L,  0x2B65E5L,  0x2B3744L,  0x2B0885L,  0x2AD9A7L,
	0x2AAAABL,  0x2A7B8FL,  0x2A4C53L,  0x2A1CF7L,  0x29ED7BL,  0x29BDDCL,  0x298E1CL,  0x295E3AL,
	0x292E34L,  0x28FE0BL,  0x28CDBDL,  0x289D4BL,  0x286CB3L,  0x283BF6L,  0x280B12L,  0x27DA06L,
	0x27A8D3L,  0x277777L,  0x2745F2L,  0x271443L,  0x26E26AL,  0x26B065L,  0x267E34L,  0x264BD6L,
	0x26194AL,  0x25E690L,  0x25B3A7L,  0x25808EL,  0x254D44L,  0x2519C8L,  0x24E619L,  0x24B236L,
	0x247E1FL,  0x2449D3L,  0x241550L,  0x23E095L,  0x23ABA2L,  0x237675L,  0x23410EL,  0x230B6AL,
	0x22D58AL,  0x229F6BL,  0x22690DL,  0x22326EL,  0x21FB8DL,  0x21C468L,  0x218CFFL,  0x215550L,
	0x211D59L,  0x20E519L,  0x20AC8EL,  0x2073B7L,  0x203A92L,  0x20011DL,  0x1FC757L,  0x1F8D3DL,
	0x1F52CFL,  0x1F1809L,  0x1EDCEAL,  0x1EA170L,  0x1E6598L,  0x1E2961L,  0x1DECC7L,  0x1DAFC9L,
	0x1D7264L,  0x1D3496L,  0x1CF65BL,  0x1CB7B1L,  0x1C7895L,  0x1C3904L,  0x1BF8FAL,  0x1BB874L,
	0x1B7770L,  0x1B35E8L,  0x1AF3DAL,  0x1AB141L,  0x1A6E19L,  0x1A2A5EL,  0x19E60BL,  0x19A11BL,
	0x195B8AL,  0x191551L,  0x18CE6CL,  0x1886D4L,  0x183E83L,  0x17F573L,  0x17AB9BL,  0x1760F6L,
	0x17157BL,  0x16C921L,  0x167BE0L,  0x162DAFL,  0x15DE82L,  0x158E4FL,  0x153D0BL,  0x14EAA8L,
	0x14971AL,  0x144251L,  0x13EC3FL,  0x1394D1L,  0x133BF5L,  0x12E198L,  0x1285A2L,  0x1227FBL,
	0x11C889L,  0x11672FL,  0x1103CAL,  0x109E37L,  0x10364BL,  0xFCBDAL,   0xF5EAEL,   0xEEE8CL,
	0xE7B2DL,   0xE0444L,   0xD8971L,   0xD0A46L,   0xC863FL,   0xBFCBBL,   0xB6CF4L,   0xAD5F0L,
	0xA366FL,   0x98CC6L,   0x8D6ADL,   0x810DBL,   0x73642L,   0x63E62L,   0x518A6L,   0x39A39L,
	0x0L
};

/* clang-format on */

/* fixatan:
 *  Fixed point inverse tangent. Does a binary search on the tan table.
 */
fixed fixatan(fixed x) {
	int a, b, c; /* for binary search */
	fixed d; /* difference value for search */

	if (x >= 0) { /* search the first part of tan table */
		a = 0;
		b = 127;
	} else { /* search the second half instead */
		a = 128;
		b = 255;
	}

	do {
		c = (a + b) >> 1;
		d = x - _tan_tbl[c];

		if (d > 0)
			a = c + 1;
		else if (d < 0)
			b = c - 1;

	} while ((a <= b) && (d));

	if (x >= 0)
		return ((long)c) << 15;

	return (-0x00800000L + (((long)c) << 15));
}

/* fixatan2:
 *  Like the libc atan2, but for fixed point numbers.
 */
fixed fixatan2(fixed y, fixed x) {
	fixed r;

	if (x == 0) {
		if (y == 0) {
			*allegro_errno = EDOM;
			return 0L;
		} else
			return ((y < 0) ? -0x00400000L : 0x00400000L);
	}

	*allegro_errno = 0;
	r = fixdiv(y, x);

	if (*allegro_errno) {
		*allegro_errno = 0;
		return ((y < 0) ? -0x00400000L : 0x00400000L);
	}

	r = fixatan(r);

	if (x >= 0)
		return r;

	if (y >= 0)
		return 0x00800000L + r;

	return r - 0x00800000L;
}

/* fixtorad_r, radtofix_r:
 *  Ratios for converting between radians and fixed point angles.
 */
AL_CONST fixed fixtorad_r = (fixed)1608; /* 2pi/256 */
AL_CONST fixed radtofix_r = (fixed)2670177; /* 256/2pi */

/* fixsqrt:
 *  Fixed point square root routine for non-i386.
 */
fixed fixsqrt(fixed x) {
	if (x > 0)
		return ftofix(sqrt(fixtof(x)));

	if (x < 0)
		*allegro_errno = EDOM;

	return 0;
}

/* fixhypot:
 *  Fixed point sqrt (x*x+y*y) for non-i386.
 */
fixed fixhypot(fixed x, fixed y) {
	return ftofix(hypot(fixtof(x), fixtof(y)));
}

/*
 * Vector and matrix manipulation routines.
 * ========================================
 */

#define FLOATSINCOS(x, s, c) _AL_SINCOS((x) * AL_PI / 128.0, s, c)
#define floattan(x) tan((x) * AL_PI / 128.0)

MATRIX identity_matrix = {
	{
			/* 3x3 identity */
			{ 1 << 16, 0, 0 },
			{ 0, 1 << 16, 0 },
			{ 0, 0, 1 << 16 },
	},

	/* zero translation */
	{ 0, 0, 0 }
};

MATRIX_f identity_matrix_f = {
	{
			/* 3x3 identity */
			{ 1.0, 0.0, 0.0 },
			{ 0.0, 1.0, 0.0 },
			{ 0.0, 0.0, 1.0 },
	},

	/* zero translation */
	{ 0.0, 0.0, 0.0 }
};

/* get_translation_matrix:
 *  Constructs a 3d translation matrix. When applied to the vector
 *  (vx, vy, vx), this will produce (vx+x, vy+y, vz+z).
 */
void get_translation_matrix(MATRIX *m, fixed x, fixed y, fixed z) {
	ASSERT(m);
	*m = identity_matrix;

	m->t[0] = x;
	m->t[1] = y;
	m->t[2] = z;
}

/* get_translation_matrix_f:
 *  Floating point version of get_translation_matrix().
 */
void get_translation_matrix_f(MATRIX_f *m, float x, float y, float z) {
	ASSERT(m);
	*m = identity_matrix_f;

	m->t[0] = x;
	m->t[1] = y;
	m->t[2] = z;
}

/* get_scaling_matrix:
 *  Constructs a 3d scaling matrix. When applied to the vector
 *  (vx, vy, vx), this will produce (vx*x, vy*y, vz*z).
 */
void get_scaling_matrix(MATRIX *m, fixed x, fixed y, fixed z) {
	ASSERT(m);
	*m = identity_matrix;

	m->v[0][0] = x;
	m->v[1][1] = y;
	m->v[2][2] = z;
}

/* get_scaling_matrix_f:
 *  Floating point version of get_scaling_matrix().
 */
void get_scaling_matrix_f(MATRIX_f *m, float x, float y, float z) {
	ASSERT(m);
	*m = identity_matrix_f;

	m->v[0][0] = x;
	m->v[1][1] = y;
	m->v[2][2] = z;
}

/* get_x_rotate_matrix:
 *  Constructs a 3d transformation matrix, which will rotate points around
 *  the x axis by the specified amount (given in the Allegro fixed point,
 *  256 degrees to a circle format).
 */
void get_x_rotate_matrix(MATRIX *m, fixed r) {
	fixed c = fixcos(r);
	fixed s = fixsin(r);
	ASSERT(m);

	*m = identity_matrix;

	m->v[1][1] = c;
	m->v[1][2] = -s;

	m->v[2][1] = s;
	m->v[2][2] = c;
}

/* get_x_rotate_matrix_f:
 *  Floating point version of get_x_rotate_matrix().
 */
void get_x_rotate_matrix_f(MATRIX_f *m, float r) {
	float c, s;
	ASSERT(m);

	FLOATSINCOS(r, s, c);
	*m = identity_matrix_f;

	m->v[1][1] = c;
	m->v[1][2] = -s;

	m->v[2][1] = s;
	m->v[2][2] = c;
}

/* get_y_rotate_matrix:
 *  Constructs a 3d transformation matrix, which will rotate points around
 *  the y axis by the specified amount (given in the Allegro fixed point,
 *  256 degrees to a circle format).
 */
void get_y_rotate_matrix(MATRIX *m, fixed r) {
	fixed c = fixcos(r);
	fixed s = fixsin(r);
	ASSERT(m);

	*m = identity_matrix;

	m->v[0][0] = c;
	m->v[0][2] = s;

	m->v[2][0] = -s;
	m->v[2][2] = c;
}

/* get_y_rotate_matrix_f:
 *  Floating point version of get_y_rotate_matrix().
 */
void get_y_rotate_matrix_f(MATRIX_f *m, float r) {
	float c, s;
	ASSERT(m);

	FLOATSINCOS(r, s, c);
	*m = identity_matrix_f;

	m->v[0][0] = c;
	m->v[0][2] = s;

	m->v[2][0] = -s;
	m->v[2][2] = c;
}

/* get_z_rotate_matrix:
 *  Constructs a 3d transformation matrix, which will rotate points around
 *  the z axis by the specified amount (given in the Allegro fixed point,
 *  256 degrees to a circle format).
 */
void get_z_rotate_matrix(MATRIX *m, fixed r) {
	fixed c = fixcos(r);
	fixed s = fixsin(r);
	ASSERT(m);

	*m = identity_matrix;

	m->v[0][0] = c;
	m->v[0][1] = -s;

	m->v[1][0] = s;
	m->v[1][1] = c;
}

/* get_z_rotate_matrix_f:
 *  Floating point version of get_z_rotate_matrix().
 */
void get_z_rotate_matrix_f(MATRIX_f *m, float r) {
	float c, s;
	ASSERT(m);

	FLOATSINCOS(r, s, c);
	*m = identity_matrix_f;

	m->v[0][0] = c;
	m->v[0][1] = -s;

	m->v[1][0] = s;
	m->v[1][1] = c;
}

/* magical formulae for constructing rotation matrices */
#define MAKE_ROTATION(x, y, z)              \
	fixed sin_x = fixsin(x);                \
	fixed cos_x = fixcos(x);                \
                                            \
	fixed sin_y = fixsin(y);                \
	fixed cos_y = fixcos(y);                \
                                            \
	fixed sin_z = fixsin(z);                \
	fixed cos_z = fixcos(z);                \
                                            \
	fixed sinx_siny = fixmul(sin_x, sin_y); \
	fixed cosx_siny = fixmul(cos_x, sin_y);

#define MAKE_ROTATION_f(x, y, z)  \
	float sin_x, cos_x;           \
	float sin_y, cos_y;           \
	float sin_z, cos_z;           \
	float sinx_siny, cosx_siny;   \
                                  \
	FLOATSINCOS(x, sin_x, cos_x); \
	FLOATSINCOS(y, sin_y, cos_y); \
	FLOATSINCOS(z, sin_z, cos_z); \
                                  \
	sinx_siny = sin_x * sin_y;    \
	cosx_siny = cos_x * sin_y;

#define R00 (fixmul(cos_y, cos_z))
#define R10 (fixmul(sinx_siny, cos_z) - fixmul(cos_x, sin_z))
#define R20 (fixmul(cosx_siny, cos_z) + fixmul(sin_x, sin_z))

#define R01 (fixmul(cos_y, sin_z))
#define R11 (fixmul(sinx_siny, sin_z) + fixmul(cos_x, cos_z))
#define R21 (fixmul(cosx_siny, sin_z) - fixmul(sin_x, cos_z))

#define R02 (-sin_y)
#define R12 (fixmul(sin_x, cos_y))
#define R22 (fixmul(cos_x, cos_y))

#define R00_f (cos_y * cos_z)
#define R10_f ((sinx_siny * cos_z) - (cos_x * sin_z))
#define R20_f ((cosx_siny * cos_z) + (sin_x * sin_z))

#define R01_f (cos_y * sin_z)
#define R11_f ((sinx_siny * sin_z) + (cos_x * cos_z))
#define R21_f ((cosx_siny * sin_z) - (sin_x * cos_z))

#define R02_f (-sin_y)
#define R12_f (sin_x * cos_y)
#define R22_f (cos_x * cos_y)

/* get_rotation_matrix:
 *  Constructs a 3d transformation matrix, which will rotate points around
 *  all three axis by the specified amounts (given in the Allegro fixed
 *  point, 256 degrees to a circle format).
 */
void get_rotation_matrix(MATRIX *m, fixed x, fixed y, fixed z) {
	MAKE_ROTATION(x, y, z);
	ASSERT(m);

	m->v[0][0] = R00;
	m->v[0][1] = R01;
	m->v[0][2] = R02;

	m->v[1][0] = R10;
	m->v[1][1] = R11;
	m->v[1][2] = R12;

	m->v[2][0] = R20;
	m->v[2][1] = R21;
	m->v[2][2] = R22;

	m->t[0] = m->t[1] = m->t[2] = 0;
}

/* get_rotation_matrix_f:
 *  Floating point version of get_rotation_matrix().
 */
void get_rotation_matrix_f(MATRIX_f *m, float x, float y, float z) {
	MAKE_ROTATION_f(x, y, z);
	ASSERT(m);

	m->v[0][0] = R00_f;
	m->v[0][1] = R01_f;
	m->v[0][2] = R02_f;

	m->v[1][0] = R10_f;
	m->v[1][1] = R11_f;
	m->v[1][2] = R12_f;

	m->v[2][0] = R20_f;
	m->v[2][1] = R21_f;
	m->v[2][2] = R22_f;

	m->t[0] = m->t[1] = m->t[2] = 0;
}

/* get_align_matrix:
 *  Aligns a matrix along an arbitrary coordinate system.
 */
void get_align_matrix(MATRIX *m, fixed xfront, fixed yfront, fixed zfront, fixed xup, fixed yup, fixed zup) {
	fixed xright, yright, zright;
	ASSERT(m);

	xfront = -xfront;
	yfront = -yfront;
	zfront = -zfront;

	normalize_vector(&xfront, &yfront, &zfront);
	cross_product(xup, yup, zup, xfront, yfront, zfront, &xright, &yright, &zright);
	normalize_vector(&xright, &yright, &zright);
	cross_product(xfront, yfront, zfront, xright, yright, zright, &xup, &yup, &zup);
	/* No need to normalize up here, since right and front are perpendicular and normalized. */

	m->v[0][0] = xright;
	m->v[0][1] = xup;
	m->v[0][2] = xfront;

	m->v[1][0] = yright;
	m->v[1][1] = yup;
	m->v[1][2] = yfront;

	m->v[2][0] = zright;
	m->v[2][1] = zup;
	m->v[2][2] = zfront;

	m->t[0] = m->t[1] = m->t[2] = 0;
}

/* get_align_matrix_f:
 *  Floating point version of get_align_matrix().
 */
void get_align_matrix_f(MATRIX_f *m, float xfront, float yfront, float zfront, float xup, float yup, float zup) {
	float xright, yright, zright;
	ASSERT(m);

	xfront = -xfront;
	yfront = -yfront;
	zfront = -zfront;

	normalize_vector_f(&xfront, &yfront, &zfront);
	cross_product_f(xup, yup, zup, xfront, yfront, zfront, &xright, &yright, &zright);
	normalize_vector_f(&xright, &yright, &zright);
	cross_product_f(xfront, yfront, zfront, xright, yright, zright, &xup, &yup, &zup);
	/* No need to normalize up here, since right and front are perpendicular and normalized. */

	m->v[0][0] = xright;
	m->v[0][1] = xup;
	m->v[0][2] = xfront;

	m->v[1][0] = yright;
	m->v[1][1] = yup;
	m->v[1][2] = yfront;

	m->v[2][0] = zright;
	m->v[2][1] = zup;
	m->v[2][2] = zfront;

	m->t[0] = m->t[1] = m->t[2] = 0;
}

/* get_vector_rotation_matrix:
 *  Constructs a 3d transformation matrix, which will rotate points around
 *  the specified x,y,z vector by the specified angle (given in the Allegro
 *  fixed point, 256 degrees to a circle format), in a clockwise direction.
 */
void get_vector_rotation_matrix(MATRIX *m, fixed x, fixed y, fixed z, fixed a) {
	MATRIX_f rotation;
	int i, j;
	ASSERT(m);

	get_vector_rotation_matrix_f(&rotation, fixtof(x), fixtof(y), fixtof(z), fixtof(a));

	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			m->v[i][j] = ftofix(rotation.v[i][j]);

	m->t[0] = m->t[1] = m->t[2] = 0;
}

/* get_vector_rotation_matrix_f:
 *  Floating point version of get_vector_rotation_matrix().
 */
void get_vector_rotation_matrix_f(MATRIX_f *m, float x, float y, float z, float a) {
	float c, s, cc;
	ASSERT(m);

	FLOATSINCOS(a, s, c);
	cc = 1 - c;
	normalize_vector_f(&x, &y, &z);

	m->v[0][0] = (cc * x * x) + c;
	m->v[0][1] = (cc * x * y) + (z * s);
	m->v[0][2] = (cc * x * z) - (y * s);

	m->v[1][0] = (cc * x * y) - (z * s);
	m->v[1][1] = (cc * y * y) + c;
	m->v[1][2] = (cc * z * y) + (x * s);

	m->v[2][0] = (cc * x * z) + (y * s);
	m->v[2][1] = (cc * y * z) - (x * s);
	m->v[2][2] = (cc * z * z) + c;

	m->t[0] = m->t[1] = m->t[2] = 0;
}

/* get_transformation_matrix:
 *  Constructs a 3d transformation matrix, which will rotate points around
 *  all three axis by the specified amounts (given in the Allegro fixed
 *  point, 256 degrees to a circle format), scale the result by the
 *  specified amount (itofix(1) for no change of scale), and then translate
 *  to the requested x, y, z position.
 */
void get_transformation_matrix(MATRIX *m, fixed scale, fixed xrot, fixed yrot, fixed zrot, fixed x, fixed y, fixed z) {
	MAKE_ROTATION(xrot, yrot, zrot);
	ASSERT(m);

	m->v[0][0] = fixmul(R00, scale);
	m->v[0][1] = fixmul(R01, scale);
	m->v[0][2] = fixmul(R02, scale);

	m->v[1][0] = fixmul(R10, scale);
	m->v[1][1] = fixmul(R11, scale);
	m->v[1][2] = fixmul(R12, scale);

	m->v[2][0] = fixmul(R20, scale);
	m->v[2][1] = fixmul(R21, scale);
	m->v[2][2] = fixmul(R22, scale);

	m->t[0] = x;
	m->t[1] = y;
	m->t[2] = z;
}

/* get_transformation_matrix_f:
 *  Floating point version of get_transformation_matrix().
 */
void get_transformation_matrix_f(MATRIX_f *m, float scale, float xrot, float yrot, float zrot, float x, float y, float z) {
	MAKE_ROTATION_f(xrot, yrot, zrot);
	ASSERT(m);

	m->v[0][0] = R00_f * scale;
	m->v[0][1] = R01_f * scale;
	m->v[0][2] = R02_f * scale;

	m->v[1][0] = R10_f * scale;
	m->v[1][1] = R11_f * scale;
	m->v[1][2] = R12_f * scale;

	m->v[2][0] = R20_f * scale;
	m->v[2][1] = R21_f * scale;
	m->v[2][2] = R22_f * scale;

	m->t[0] = x;
	m->t[1] = y;
	m->t[2] = z;
}

/* get_camera_matrix:
 *  Constructs a camera matrix for translating world-space objects into
 *  a normalised view space, ready for the perspective projection. The
 *  x, y, and z parameters specify the camera position, xfront, yfront,
 *  and zfront is an 'in front' vector specifying which way the camera
 *  is facing (this can be any length: normalisation is not required),
 *  and xup, yup, and zup is the 'up' direction vector. Up is really only
 *  a 1.5d vector, since the front vector only leaves one degree of freedom
 *  for which way up to put the image, but it is simplest to specify it
 *  as a full 3d direction even though a lot of the information in it is
 *  discarded. The fov parameter specifies the field of view (ie. width
 *  of the camera focus) in fixed point, 256 degrees to the circle format.
 *  For typical projections, a field of view in the region 32-48 will work
 *  well. Finally, the aspect ratio is used to scale the Y dimensions of
 *  the image relative to the X axis, so you can use it to correct for
 *  the proportions of the output image (set it to 1 for no scaling).
 */
void get_camera_matrix(MATRIX *m, fixed x, fixed y, fixed z, fixed xfront, fixed yfront, fixed zfront, fixed xup, fixed yup, fixed zup, fixed fov, fixed aspect) {
	MATRIX_f camera;
	int i, j;
	ASSERT(m);

	get_camera_matrix_f(&camera,
			fixtof(x), fixtof(y), fixtof(z),
			fixtof(xfront), fixtof(yfront), fixtof(zfront),
			fixtof(xup), fixtof(yup), fixtof(zup),
			fixtof(fov), fixtof(aspect));

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++)
			m->v[i][j] = ftofix(camera.v[i][j]);

		m->t[i] = ftofix(camera.t[i]);
	}
}

/* get_camera_matrix_f:
 *  Floating point version of get_camera_matrix().
 */
void get_camera_matrix_f(MATRIX_f *m, float x, float y, float z, float xfront, float yfront, float zfront, float xup, float yup, float zup, float fov, float aspect) {
	MATRIX_f camera, scale;
	float xside, yside, zside, width, d;
	ASSERT(m);

	/* make 'in-front' into a unit vector, and negate it */
	normalize_vector_f(&xfront, &yfront, &zfront);
	xfront = -xfront;
	yfront = -yfront;
	zfront = -zfront;

	/* make sure 'up' is at right angles to 'in-front', and normalize */
	d = dot_product_f(xup, yup, zup, xfront, yfront, zfront);
	xup -= d * xfront;
	yup -= d * yfront;
	zup -= d * zfront;
	normalize_vector_f(&xup, &yup, &zup);

	/* calculate the 'sideways' vector */
	cross_product_f(xup, yup, zup, xfront, yfront, zfront, &xside, &yside, &zside);

	/* set matrix rotation parameters */
	camera.v[0][0] = xside;
	camera.v[0][1] = yside;
	camera.v[0][2] = zside;

	camera.v[1][0] = xup;
	camera.v[1][1] = yup;
	camera.v[1][2] = zup;

	camera.v[2][0] = xfront;
	camera.v[2][1] = yfront;
	camera.v[2][2] = zfront;

	/* set matrix translation parameters */
	camera.t[0] = -(x * xside + y * yside + z * zside);
	camera.t[1] = -(x * xup + y * yup + z * zup);
	camera.t[2] = -(x * xfront + y * yfront + z * zfront);

	/* construct a scaling matrix to deal with aspect ratio and FOV */
	width = floattan(64.0 - fov / 2);
	get_scaling_matrix_f(&scale, width, -aspect * width, -1.0);

	/* combine the camera and scaling matrices */
	matrix_mul_f(&camera, &scale, m);
}

/* qtranslate_matrix:
 *  Adds a position offset to an existing matrix.
 */
void qtranslate_matrix(MATRIX *m, fixed x, fixed y, fixed z) {
	ASSERT(m);
	m->t[0] += x;
	m->t[1] += y;
	m->t[2] += z;
}

/* qtranslate_matrix_f:
 *  Floating point version of qtranslate_matrix().
 */
void qtranslate_matrix_f(MATRIX_f *m, float x, float y, float z) {
	ASSERT(m);
	m->t[0] += x;
	m->t[1] += y;
	m->t[2] += z;
}

/* qscale_matrix:
 *  Adds a scaling factor to an existing matrix.
 */
void qscale_matrix(MATRIX *m, fixed scale) {
	int i, j;
	ASSERT(m);

	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			m->v[i][j] = fixmul(m->v[i][j], scale);
}

/* qscale_matrix_f:
 *  Floating point version of qscale_matrix().
 */
void qscale_matrix_f(MATRIX_f *m, float scale) {
	int i, j;
	ASSERT(m);

	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			m->v[i][j] *= scale;
}

/* matrix_mul:
 *  Multiplies two matrices, storing the result in out (this must be
 *  different from the two input matrices). The resulting matrix will
 *  have the same effect as the combination of m1 and m2, ie. when
 *  applied to a vector v, (v * out) = ((v * m1) * m2). Any number of
 *  transformations can be concatenated in this way.
 */
void matrix_mul(AL_CONST MATRIX *m1, AL_CONST MATRIX *m2, MATRIX *out) {
	MATRIX temp;
	int i, j;
	ASSERT(m1);
	ASSERT(m2);
	ASSERT(out);

	if (m1 == out) {
		temp = *m1;
		m1 = &temp;
	} else if (m2 == out) {
		temp = *m2;
		m2 = &temp;
	}

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			out->v[i][j] = fixmul(m1->v[0][j], m2->v[i][0]) +
					fixmul(m1->v[1][j], m2->v[i][1]) +
					fixmul(m1->v[2][j], m2->v[i][2]);
		}

		out->t[i] = fixmul(m1->t[0], m2->v[i][0]) +
				fixmul(m1->t[1], m2->v[i][1]) +
				fixmul(m1->t[2], m2->v[i][2]) +
				m2->t[i];
	}
}

/* matrix_mul_f:
 *  Floating point version of matrix_mul().
 */
void matrix_mul_f(AL_CONST MATRIX_f *m1, AL_CONST MATRIX_f *m2, MATRIX_f *out) {
	MATRIX_f temp;
	int i, j;
	ASSERT(m1);
	ASSERT(m2);
	ASSERT(out);

	if (m1 == out) {
		temp = *m1;
		m1 = &temp;
	} else if (m2 == out) {
		temp = *m2;
		m2 = &temp;
	}

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			out->v[i][j] = (m1->v[0][j] * m2->v[i][0]) +
					(m1->v[1][j] * m2->v[i][1]) +
					(m1->v[2][j] * m2->v[i][2]);
		}

		out->t[i] = (m1->t[0] * m2->v[i][0]) +
				(m1->t[1] * m2->v[i][1]) +
				(m1->t[2] * m2->v[i][2]) +
				m2->t[i];
	}
}

/* vector_length:
 *  Computes the length of a vector, using the son of the squaw...
 */
fixed vector_length(fixed x, fixed y, fixed z) {
	x >>= 8;
	y >>= 8;
	z >>= 8;

	return (fixsqrt(fixmul(x, x) + fixmul(y, y) + fixmul(z, z)) << 8);
}

/* vector_lengthf:
 *  Floating point version of vector_length().
 */
float vector_length_f(float x, float y, float z) {
	return sqrt(x * x + y * y + z * z);
}

/* normalize_vector:
 *  Converts the specified vector to a unit vector, which has the same
 *  orientation but a length of one.
 */
void normalize_vector(fixed *x, fixed *y, fixed *z) {
	fixed length = vector_length(*x, *y, *z);

	*x = fixdiv(*x, length);
	*y = fixdiv(*y, length);
	*z = fixdiv(*z, length);
}

/* normalize_vectorf:
 *  Floating point version of normalize_vector().
 */
void normalize_vector_f(float *x, float *y, float *z) {
	float length = 1.0 / vector_length_f(*x, *y, *z);

	*x *= length;
	*y *= length;
	*z *= length;
}

/* cross_product:
 *  Calculates the cross product of two vectors.
 */
void cross_product(fixed x1, fixed y1, fixed z1, fixed x2, fixed y2, fixed z2, fixed *xout, fixed *yout, fixed *zout) {
	ASSERT(xout);
	ASSERT(yout);
	ASSERT(zout);

	*xout = fixmul(y1, z2) - fixmul(z1, y2);
	*yout = fixmul(z1, x2) - fixmul(x1, z2);
	*zout = fixmul(x1, y2) - fixmul(y1, x2);
}

/* cross_productf:
 *  Floating point version of cross_product().
 */
void cross_product_f(float x1, float y1, float z1, float x2, float y2, float z2, float *xout, float *yout, float *zout) {
	ASSERT(xout);
	ASSERT(yout);
	ASSERT(zout);

	*xout = (y1 * z2) - (z1 * y2);
	*yout = (z1 * x2) - (x1 * z2);
	*zout = (x1 * y2) - (y1 * x2);
}

/* polygon_z_normal:
 *  Helper function for backface culling: returns the z component of the
 *  normal vector to the polygon formed from the three vertices.
 */
fixed polygon_z_normal(AL_CONST V3D *v1, AL_CONST V3D *v2, AL_CONST V3D *v3) {
	ASSERT(v1);
	ASSERT(v2);
	ASSERT(v3);
	return (fixmul(v2->x - v1->x, v3->y - v2->y) - fixmul(v3->x - v2->x, v2->y - v1->y));
}

/* polygon_z_normal_f:
 *  Floating point version of polygon_z_normal().
 */
float polygon_z_normal_f(AL_CONST V3D_f *v1, AL_CONST V3D_f *v2, AL_CONST V3D_f *v3) {
	ASSERT(v1);
	ASSERT(v2);
	ASSERT(v3);
	return ((v2->x - v1->x) * (v3->y - v2->y)) - ((v3->x - v2->x) * (v2->y - v1->y));
}

/* scaling factors for the perspective projection */
fixed _persp_xscale = 160 << 16;
fixed _persp_yscale = 100 << 16;
fixed _persp_xoffset = 160 << 16;
fixed _persp_yoffset = 100 << 16;

float _persp_xscale_f = 160.0;
float _persp_yscale_f = 100.0;
float _persp_xoffset_f = 160.0;
float _persp_yoffset_f = 100.0;

/* set_projection_viewport:
 *  Sets the viewport used to scale the output of the persp_project()
 *  function.
 */
void set_projection_viewport(int x, int y, int w, int h) {
	ASSERT(w > 0);
	ASSERT(h > 0);

	_persp_xscale = itofix(w / 2);
	_persp_yscale = itofix(h / 2);
	_persp_xoffset = itofix(x + w / 2);
	_persp_yoffset = itofix(y + h / 2);

	_persp_xscale_f = w / 2;
	_persp_yscale_f = h / 2;
	_persp_xoffset_f = x + w / 2;
	_persp_yoffset_f = y + h / 2;
}

/*
 * Color manipulation routines (blending, format conversion, lighting
 * table construction, etc).
 * ==================================================================
 */

/* makecol_depth:
 *  Converts R, G, and B values (ranging 0-255) to whatever pixel format
 *  is required by the specified color depth.
 */
int makecol_depth(int color_depth, int r, int g, int b) {
	switch (color_depth) {
		case 8:
			return makecol8(r, g, b);
		case 15:
			return makecol15(r, g, b);
		case 16:
			return makecol16(r, g, b);
		case 24:
			return makecol24(r, g, b);
		case 32:
			return makecol32(r, g, b);
	}
	return 0;
}

/* makeacol_depth:
 *  Converts R, G, B, and A values (ranging 0-255) to whatever pixel format
 *  is required by the specified color depth.
 */
int makeacol_depth(int color_depth, int r, int g, int b, int a) {
	switch (color_depth) {
		case 8:
			return makecol8(r, g, b);
		case 15:
			return makecol15(r, g, b);
		case 16:
			return makecol16(r, g, b);
		case 24:
			return makecol24(r, g, b);
		case 32:
			return makeacol32(r, g, b, a);
	}
	return 0;
}

/* makecol:
 *  Converts R, G, and B values (ranging 0-255) to whatever pixel format
 *  is required by the current video mode.
 */
int makecol(int r, int g, int b) { return makecol_depth(_color_depth, r, g, b); }

/* makeacol:
 *  Converts R, G, B, and A values (ranging 0-255) to whatever pixel format
 *  is required by the current video mode.
 */
int makeacol(int r, int g, int b, int a) { return makeacol_depth(_color_depth, r, g, b, a); }

/* getr_depth:
 *  Extracts the red component (ranging 0-255) from a pixel in the format
 *  being used by the specified color depth.
 */
int getr_depth(int color_depth, int c) {
	switch (color_depth) {
		case 8:
			return getr8(c);
		case 15:
			return getr15(c);
		case 16:
			return getr16(c);
		case 24:
			return getr24(c);
		case 32:
			return getr32(c);
	}
	return 0;
}

/* getg_depth:
 *  Extracts the green component (ranging 0-255) from a pixel in the format
 *  being used by the specified color depth.
 */
int getg_depth(int color_depth, int c) {
	switch (color_depth) {
		case 8:
			return getg8(c);
		case 15:
			return getg15(c);
		case 16:
			return getg16(c);
		case 24:
			return getg24(c);
		case 32:
			return getg32(c);
	}
	return 0;
}

/* getb_depth:
 *  Extracts the blue component (ranging 0-255) from a pixel in the format
 *  being used by the specified color depth.
 */
int getb_depth(int color_depth, int c) {
	switch (color_depth) {
		case 8:
			return getb8(c);
		case 15:
			return getb15(c);
		case 16:
			return getb16(c);
		case 24:
			return getb24(c);
		case 32:
			return getb32(c);
	}
	return 0;
}

/* geta_depth:
 *  Extracts the alpha component (ranging 0-255) from a pixel in the format
 *  being used by the specified color depth.
 */
int geta_depth(int color_depth, int c) {
	if (color_depth == 32)
		return geta32(c);
	return 0;
}

/* getr:
 *  Extracts the red component (ranging 0-255) from a pixel in the format
 *  being used by the current video mode.
 */
int getr(int c) { return getr_depth(_color_depth, c); }

/* getg:
 *  Extracts the green component (ranging 0-255) from a pixel in the format
 *  being used by the current video mode.
 */
int getg(int c) { return getg_depth(_color_depth, c); }

/* getb:
 *  Extracts the blue component (ranging 0-255) from a pixel in the format
 *  being used by the current video mode.
 */
int getb(int c) { return getb_depth(_color_depth, c); }

/* geta:
 *  Extracts the alpha component (ranging 0-255) from a pixel in the format
 *  being used by the current video mode.
 */
int geta(int c) { return geta_depth(_color_depth, c); }

/* 1.5k lookup table for color matching */
static unsigned int col_diff[3 * 128];

/* bestfit_init:
 *  Color matching is done with weighted squares, which are much faster
 *  if we pregenerate a little lookup table...
 */
static void bestfit_init(void) {
	for (int i = 1; i < 64; i++) {
		int k = i * i;
		col_diff[0 + i] = col_diff[0 + 128 - i] = k * (59 * 59);
		col_diff[128 + i] = col_diff[128 + 128 - i] = k * (30 * 30);
		col_diff[256 + i] = col_diff[256 + 128 - i] = k * (11 * 11);
	}
}

/* bestfit_color:
 *  Searches a palette for the color closest to the requested R, G, B value.
 */
int bestfit_color(AL_CONST PALETTE pal, int r, int g, int b) {
	int i, coldiff, lowest, bestfit;

	ASSERT(r >= 0 && r <= 63);
	ASSERT(g >= 0 && g <= 63);
	ASSERT(b >= 0 && b <= 63);

	if (col_diff[1] == 0)
		bestfit_init();

	bestfit = 0;
	lowest = INT_MAX;

	/* only the transparent (pink) color can be mapped to index 0 */
	if ((r == 63) && (g == 0) && (b == 63))
		i = 0;
	else
		i = 1;

	while (i < PAL_SIZE) {
		AL_CONST RGB *rgb = &pal[i];
		coldiff = (col_diff + 0)[(rgb->g - g) & 0x7F];
		if (coldiff < lowest) {
			coldiff += (col_diff + 128)[(rgb->r - r) & 0x7F];
			if (coldiff < lowest) {
				coldiff += (col_diff + 256)[(rgb->b - b) & 0x7F];
				if (coldiff < lowest) {
					bestfit = rgb - pal; /* faster than `bestfit = i;' */
					if (coldiff == 0)
						return bestfit;
					lowest = coldiff;
				}
			}
		}
		i++;
	}

	return bestfit;
}

/* makecol8:
 *  Converts R, G, and B values (ranging 0-255) to an 8 bit paletted color.
 *  If the global rgb_map table is initialised, it uses that, otherwise
 *  it searches through the current palette to find the best match.
 */
int makecol8(int r, int g, int b) {
	if (rgb_map)
		return rgb_map->data[r >> 3][g >> 3][b >> 3];
	else
		return bestfit_color(_current_palette, r >> 2, g >> 2, b >> 2);
}

/* hsv_to_rgb:
 *  Converts from HSV colorspace to RGB values.
 */
void hsv_to_rgb(float h, float s, float v, int *r, int *g, int *b) {
	float f, x, y, z;
	int i;

	ASSERT(s >= 0 && s <= 1);
	ASSERT(v >= 0 && v <= 1);

	v *= 255.0f;

	if (s == 0.0f) { /* ok since we don't divide by s, and faster */
		*r = *g = *b = v + 0.5f;
	} else {
		h = fmod(h, 360.0f) / 60.0f;
		if (h < 0.0f)
			h += 6.0f;

		i = (int)h;
		f = h - i;
		x = v * s;
		y = x * f;
		v += 0.5f; /* round to the nearest integer below */
		z = v - x;

		switch (i) {
			case 6:
			case 0:
				*r = v;
				*g = z + y;
				*b = z;
				break;

			case 1:
				*r = v - y;
				*g = v;
				*b = z;
				break;

			case 2:
				*r = z;
				*g = v;
				*b = z + y;
				break;

			case 3:
				*r = z;
				*g = v - y;
				*b = v;
				break;

			case 4:
				*r = z + y;
				*g = z;
				*b = v;
				break;

			case 5:
				*r = v;
				*g = z;
				*b = v - y;
				break;
		}
	}
}

/* rgb_to_hsv:
 *  Converts an RGB value into the HSV colorspace.
 */
void rgb_to_hsv(int r, int g, int b, float *h, float *s, float *v) {
	int delta;

	ASSERT(r >= 0 && r <= 255);
	ASSERT(g >= 0 && g <= 255);
	ASSERT(b >= 0 && b <= 255);

	if (r > g) {
		if (b > r) {
			/* b>r>g */
			delta = b - g;
			*h = 240.0f + ((r - g) * 60) / (float)delta;
			*s = (float)delta / (float)b;
			*v = (float)b * (1.0f / 255.0f);
		} else {
			/* r>g and r>b */
			delta = r - MIN(g, b);
			*h = ((g - b) * 60) / (float)delta;
			if (*h < 0.0f)
				*h += 360.0f;
			*s = (float)delta / (float)r;
			*v = (float)r * (1.0f / 255.0f);
		}
	} else {
		if (b > g) {
			/* b>g>=r */
			delta = b - r;
			*h = 240.0f + ((r - g) * 60) / (float)delta;
			*s = (float)delta / (float)b;
			*v = (float)b * (1.0f / 255.0f);
		} else {
			/* g>=b and g>=r */
			delta = g - MIN(r, b);
			if (delta == 0) {
				*h = 0.0f;
				if (g == 0)
					*s = *v = 0.0f;
				else {
					*s = (float)delta / (float)g;
					*v = (float)g * (1.0f / 255.0f);
				}
			} else {
				*h = 120.0f + ((b - r) * 60) / (float)delta;
				*s = (float)delta / (float)g;
				*v = (float)g * (1.0f / 255.0f);
			}
		}
	}
}

/* create_rgb_table:
 *  Fills an RGB_MAP lookup table with conversion data for the specified
 *  palette. This is the faster version by Jan Hubicka.
 *
 *  Uses alg. similar to floodfill - it adds one seed per every color in
 *  palette to its best position. Then areas around seed are filled by
 *  same color because it is best approximation for them, and then areas
 *  about them etc...
 *
 *  It does just about 80000 tests for distances and this is about 100
 *  times better than normal 256*32000 tests so the calculation time
 *  is now less than one second at all computers I tested.
 */
void create_rgb_table(RGB_MAP *table, AL_CONST PALETTE pal, void (*callback)(int pos)) {
#define UNUSED 65535
#define LAST 65532

/* macro add adds to single linked list */
#define add(i) (next[(i)] == UNUSED ? (next[(i)] = LAST,                                            \
											  (first != LAST ? (next[last] = (i)) : (first = (i))), \
											  (last = (i)))                                         \
									: 0)

/* same but w/o checking for first element */
#define add1(i) (next[(i)] == UNUSED ? (next[(i)] = LAST,        \
											   next[last] = (i), \
											   (last = (i)))     \
									 : 0)

/* calculates distance between two colors */
#define dist(a1, a2, a3, b1, b2, b3)                 \
	(col_diff[((a2) - (b2)) & 0x7F] +                \
			(col_diff + 128)[((a1) - (b1)) & 0x7F] + \
			(col_diff + 256)[((a3) - (b3)) & 0x7F])

/* converts r,g,b to position in array and back */
#define pos(r, g, b) \
	(((r) / 2) * 32 * 32 + ((g) / 2) * 32 + ((b) / 2))

#define depos(pal, r, g, b)                \
	((b) = ((pal) & 31) * 2,               \
			(g) = (((pal) >> 5) & 31) * 2, \
			(r) = (((pal) >> 10) & 31) * 2)

/* is current color better than pal1? */
#define better(r1, g1, b1, pal1)  \
	(((int)dist((r1), (g1), (b1), \
			 (pal1).r, (pal1).g, (pal1).b)) > (int)dist2)

/* checking of position */
#define dopos(rp, gp, bp, ts)                                                        \
	if ((rp > -1 || r > 0) && (rp < 1 || r < 61) &&                                  \
			(gp > -1 || g > 0) && (gp < 1 || g < 61) &&                              \
			(bp > -1 || b > 0) && (bp < 1 || b < 61)) {                              \
		i = first + rp * 32 * 32 + gp * 32 + bp;                                     \
		if (!data[i]) {                                                              \
			data[i] = val;                                                           \
			add1(i);                                                                 \
		} else if ((ts) && (data[i] != val)) {                                       \
			dist2 = (rp ? (col_diff + 128)[(r + 2 * rp - pal[val].r) & 0x7F] : r2) + \
					(gp ? (col_diff)[(g + 2 * gp - pal[val].g) & 0x7F] : g2) +       \
					(bp ? (col_diff + 256)[(b + 2 * bp - pal[val].b) & 0x7F] : b2);  \
			if (better((r + 2 * rp), (g + 2 * gp), (b + 2 * bp), pal[data[i]])) {    \
				data[i] = val;                                                       \
				add1(i);                                                             \
			}                                                                        \
		}                                                                            \
	}

	int i, curr, r, g, b, val, dist2;
	unsigned int r2, g2, b2;
	unsigned short next[32 * 32 * 32];
	unsigned char *data;
	int first = LAST;
	int last = LAST;
	int count = 0;
	int cbcount = 0;

#define AVERAGE_COUNT 18000

	if (col_diff[1] == 0)
		bestfit_init();

	memset(next, 255, sizeof(next));
	memset(table->data, 0, sizeof(char) * 32 * 32 * 32);

	data = (unsigned char *)table->data;

	/* add starting seeds for floodfill */
	for (i = 1; i < PAL_SIZE; i++) {
		curr = pos(pal[i].r, pal[i].g, pal[i].b);
		if (next[curr] == UNUSED) {
			data[curr] = i;
			add(curr);
		}
	}

	/* main floodfill: two versions of loop for faster growing in blue axis */
	while (first != LAST) {
		depos(first, r, g, b);

		/* calculate distance of current color */
		val = data[first];
		r2 = (col_diff + 128)[((pal[val].r) - (r)) & 0x7F];
		g2 = (col_diff)[((pal[val].g) - (g)) & 0x7F];
		b2 = (col_diff + 256)[((pal[val].b) - (b)) & 0x7F];

		/* try to grow to all directions */
		dopos(0, 0, 1, 1);
		dopos(0, 0, -1, 1);
		dopos(1, 0, 0, 1);
		dopos(-1, 0, 0, 1);
		dopos(0, 1, 0, 1);
		dopos(0, -1, 0, 1);

		/* faster growing of blue direction */
		if ((b > 0) && (data[first - 1] == val)) {
			b -= 2;
			first--;
			b2 = (col_diff + 256)[((pal[val].b) - (b)) & 0x7F];

			dopos(-1, 0, 0, 0);
			dopos(1, 0, 0, 0);
			dopos(0, -1, 0, 0);
			dopos(0, 1, 0, 0);

			first++;
		}

		/* get next from list */
		i = first;
		first = next[first];
		next[i] = UNUSED;

		/* second version of loop */
		if (first != LAST) {
			depos(first, r, g, b);

			val = data[first];
			r2 = (col_diff + 128)[((pal[val].r) - (r)) & 0x7F];
			g2 = (col_diff)[((pal[val].g) - (g)) & 0x7F];
			b2 = (col_diff + 256)[((pal[val].b) - (b)) & 0x7F];

			dopos(0, 0, 1, 1);
			dopos(0, 0, -1, 1);
			dopos(1, 0, 0, 1);
			dopos(-1, 0, 0, 1);
			dopos(0, 1, 0, 1);
			dopos(0, -1, 0, 1);

			if ((b < 61) && (data[first + 1] == val)) {
				b += 2;
				first++;
				b2 = (col_diff + 256)[((pal[val].b) - (b)) & 0x7f];

				dopos(-1, 0, 0, 0);
				dopos(1, 0, 0, 0);
				dopos(0, -1, 0, 0);
				dopos(0, 1, 0, 0);

				first--;
			}

			i = first;
			first = next[first];
			next[i] = UNUSED;
		}

		count++;
		if (count == (cbcount + 1) * AVERAGE_COUNT / 256) {
			if (cbcount < 256) {
				if (callback)
					callback(cbcount);
				cbcount++;
			}
		}
	}

	/* only the transparent (pink) color can be mapped to index 0 */
	if ((pal[0].r == 63) && (pal[0].g == 0) && (pal[0].b == 63))
		table->data[31][0][31] = 0;

	if (callback)
		while (cbcount < 256)
			callback(cbcount++);
}

/* create_light_table:
 *  Constructs a lighting color table for the specified palette. At light
 *  intensity 255 the table will produce the palette colors directly, and
 *  at level 0 it will produce the specified R, G, B value for all colors
 *  (this is specified in 0-63 VGA format). If the callback function is
 *  not NULL, it will be called 256 times during the calculation, allowing
 *  you to display a progress indicator.
 */
void create_light_table(COLOR_MAP *table, AL_CONST PALETTE pal, int r, int g, int b, void (*callback)(int pos)) {
	int r1, g1, b1, r2, g2, b2, x, y;
	unsigned int t1, t2;

	ASSERT(table);
	ASSERT(r >= 0 && r <= 63);
	ASSERT(g >= 0 && g <= 63);
	ASSERT(b >= 0 && b <= 63);

	if (rgb_map) {
		for (x = 0; x < PAL_SIZE - 1; x++) {
			t1 = x * 0x010101;
			t2 = 0xFFFFFF - t1;

			r1 = (1 << 24) + r * t2;
			g1 = (1 << 24) + g * t2;
			b1 = (1 << 24) + b * t2;

			for (y = 0; y < PAL_SIZE; y++) {
				r2 = (r1 + pal[y].r * t1) >> 25;
				g2 = (g1 + pal[y].g * t1) >> 25;
				b2 = (b1 + pal[y].b * t1) >> 25;

				table->data[x][y] = rgb_map->data[r2][g2][b2];
			}
		}
		if (callback)
			(*callback)(x);
	} else {
		for (x = 0; x < PAL_SIZE - 1; x++) {
			t1 = x * 0x010101;
			t2 = 0xFFFFFF - t1;

			r1 = (1 << 23) + r * t2;
			g1 = (1 << 23) + g * t2;
			b1 = (1 << 23) + b * t2;

			for (y = 0; y < PAL_SIZE; y++) {
				r2 = (r1 + pal[y].r * t1) >> 24;
				g2 = (g1 + pal[y].g * t1) >> 24;
				b2 = (b1 + pal[y].b * t1) >> 24;

				table->data[x][y] = bestfit_color(pal, r2, g2, b2);
			}
		}

		if (callback)
			(*callback)(x);
	}

	for (y = 0; y < PAL_SIZE; y++)
		table->data[255][y] = y;
}

/* create_trans_table:
 *  Constructs a translucency color table for the specified palette. The
 *  r, g, and b parameters specifiy the solidity of each color component,
 *  ranging from 0 (totally transparent) to 255 (totally solid). Source
 *  color #0 is a special case, and is set to leave the destination
 *  unchanged, so that masked sprites will draw correctly. If the callback
 *  function is not NULL, it will be called 256 times during the calculation,
 *  allowing you to display a progress indicator.
 */
void create_trans_table(COLOR_MAP *table, AL_CONST PALETTE pal, int r, int g, int b, void (*callback)(int pos)) {
	int tmp[768], *q;
	int x, y, i, j, k;
	unsigned char *p;
	int tr, tg, tb;
	int add;

	ASSERT(table);
	ASSERT(r >= 0 && r <= 255);
	ASSERT(g >= 0 && g <= 255);
	ASSERT(b >= 0 && b <= 255);

	/* This is a bit ugly, but accounts for the solidity parameters
	   being in the range 0-255 rather than 0-256. Given that the
	   precision of r,g,b components is only 6 bits it shouldn't do any
	   harm. */
	if (r > 128)
		r++;
	if (g > 128)
		g++;
	if (b > 128)
		b++;

	if (rgb_map)
		add = 255;
	else
		add = 127;

	for (x = 0; x < 256; x++) {
		tmp[x * 3] = pal[x].r * (256 - r) + add;
		tmp[x * 3 + 1] = pal[x].g * (256 - g) + add;
		tmp[x * 3 + 2] = pal[x].b * (256 - b) + add;
	}

	for (x = 1; x < PAL_SIZE; x++) {
		i = pal[x].r * r;
		j = pal[x].g * g;
		k = pal[x].b * b;

		p = table->data[x];
		q = tmp;

		if (rgb_map) {
			for (y = 0; y < PAL_SIZE; y++) {
				tr = (i + *(q++)) >> 9;
				tg = (j + *(q++)) >> 9;
				tb = (k + *(q++)) >> 9;
				p[y] = rgb_map->data[tr][tg][tb];
			}
		} else {
			for (y = 0; y < PAL_SIZE; y++) {
				tr = (i + *(q++)) >> 8;
				tg = (j + *(q++)) >> 8;
				tb = (k + *(q++)) >> 8;
				p[y] = bestfit_color(pal, tr, tg, tb);
			}
		}

		if (callback)
			(*callback)(x - 1);
	}

	for (y = 0; y < PAL_SIZE; y++) {
		table->data[0][y] = y;
		table->data[y][y] = y;
	}

	if (callback)
		(*callback)(255);
}

/* create_color_table:
 *  Creates a color mapping table, using a user-supplied callback to blend
 *  each pair of colors. Your blend routine will be passed a pointer to the
 *  palette and the two colors to be blended (x is the source color, y is
 *  the destination), and should return the desired output RGB for this
 *  combination. If the callback function is not NULL, it will be called
 *  256 times during the calculation, allowing you to display a progress
 *  indicator.
 */
void create_color_table(COLOR_MAP *table, AL_CONST PALETTE pal, void (*blend)(AL_CONST PALETTE pal, int x, int y, RGB *rgb), void (*callback)(int pos)) {
	int x, y;
	RGB c;

	for (x = 0; x < PAL_SIZE; x++) {
		for (y = 0; y < PAL_SIZE; y++) {
			blend(pal, x, y, &c);

			if (rgb_map)
				table->data[x][y] = rgb_map->data[c.r >> 1][c.g >> 1][c.b >> 1];
			else
				table->data[x][y] = bestfit_color(pal, c.r, c.g, c.b);
		}

		if (callback)
			(*callback)(x);
	}
}

/* create_blender_table:
 *  Fills the specified color mapping table with lookup data for doing a
 *  paletted equivalent of whatever truecolor blender mode is currently
 *  selected.
 */
void create_blender_table(COLOR_MAP *table, AL_CONST PALETTE pal, void (*callback)(int pos)) {
	int x, y, c;
	int r, g, b;
	int r1, g1, b1;
	int r2, g2, b2;

	ASSERT(_blender_func24);

	for (x = 0; x < PAL_SIZE; x++) {
		for (y = 0; y < PAL_SIZE; y++) {
			r1 = (pal[x].r << 2) | ((pal[x].r & 0x30) >> 4);
			g1 = (pal[x].g << 2) | ((pal[x].g & 0x30) >> 4);
			b1 = (pal[x].b << 2) | ((pal[x].b & 0x30) >> 4);

			r2 = (pal[y].r << 2) | ((pal[y].r & 0x30) >> 4);
			g2 = (pal[y].g << 2) | ((pal[y].g & 0x30) >> 4);
			b2 = (pal[y].b << 2) | ((pal[y].b & 0x30) >> 4);

			c = _blender_func24(makecol24(r1, g1, b1), makecol24(r2, g2, b2), _blender_alpha);

			r = getr24(c);
			g = getg24(c);
			b = getb24(c);

			if (rgb_map)
				table->data[x][y] = rgb_map->data[r >> 3][g >> 3][b >> 3];
			else
				table->data[x][y] = bestfit_color(pal, r >> 2, g >> 2, b >> 2);
		}

		if (callback)
			(*callback)(x);
	}
}

/*
 * Optimised palette generation routines.
 * ======================================
 */

#define DEFAULT_PREC 4
#define DEFAULT_FRACTION 5
#define DEFAULT_MAXSWAPS 16
#define DEFAULT_MINDIFF 9

/* the number of lists, prime number */
#define HASHTABLESIZE 1031

typedef struct NODE {
	struct NODE *next;
	int color, count;
} NODE;

typedef struct {
	int color, key;
} ITEM;

static int distinct;
static NODE *hash_table;

static void delete_list(NODE *list) {
	for (NODE *node = list, *next; node != NULL; node = next) {
		next = node->next;
		_AL_FREE(node);
	}
}

static void insert_node(int color) {
	NODE *p = &hash_table[color % HASHTABLESIZE];

	for (;;) {
		if (p->color == color) {
			/* this node (e.g. the color was already filled) */
			p->count++;
			return;
		}
		if (p->next)
			p = p->next;
		else
			break;
	}

	/* new color */
	if (p->count) {
		p->next = _AL_MALLOC(sizeof(NODE));
		p = p->next;
	}
	if (p != NULL) {
		p->color = color;
		p->count = 1;
		p->next = NULL;
		distinct++;
	}
}

/* helper function for builtin qsort function */
static int qsort_helper_ITEM(AL_CONST void *e1, AL_CONST void *e2) { return ((ITEM *)e2)->key - ((ITEM *)e1)->key; }

/* compare two color values */
static INLINE int compare_cols(int col1, int col2) {
	int b = ((col1 >> 16) & 0xFF) - ((col2 >> 16) & 0xFF);
	int g = ((col1 >> 8) & 0xFF) - ((col2 >> 8) & 0xFF);
	int r = ((col1 & 0xFF) - (col2 & 0xFF));
	return (((r < 0) ? -r : r) + ((g < 0) ? -g : g) + ((b < 0) ? -b : b));
}

/* Searches the array from 'item'th field comparing any pair of items.
 * Fills 'key' field of all items >= 'item'th with the difference
 * value (the smallest difference between the checked color and all
 * already used). Than only the last added item has to be compared with
 * all other not yet added colors, what is performed afterwards.
 */
static void optimize_colors(ITEM *array, int item, int palsize, int length, int mindiff) {
	int curbest = 0;

	/* iterate through the array comparing any item behind 'item' */
	for (int i = item; i < length; i++) {
		/* with all item in front of 'item' */
		for (int j = 0, curbest = 1000; j < item; j++) {
			int t = compare_cols(array[i].color, array[j].color);

			/* finding minimum difference (maximal to all used colors) */
			if (t < curbest) {
				curbest = t;
				if (t < mindiff)
					break;
			}
		}

		/* filling that minimum to 'key' field */
		array[i].key = curbest;
	}

	/* sort the array begind 'item' according to 'key' field */
	qsort(array + item, length - item, sizeof(ITEM), qsort_helper_ITEM);

	/* find the start of small values ('key') in array and safely reducing
	 * the number of items we'll work with
	 */
	for (int i = item; i < length; i++) {
		if (array[i].key < mindiff) {
			length = i;
			break;
		}
	}

	/* the most different color (from colors in [0,item)) */
	int bestpos = item;
	int best = array[item].key;

	/* swapping loop (the length goes from the size of palette) */
	for (int i = item; i < palsize; i++) {
		/* the 'i'th best is already known */
		if (best < mindiff) {
			return;
		} else {
			/* swap the focused color and the one with 'bestpos' (the most
			 * different) index
			 */
			int tmp = array[bestpos].color;
			array[bestpos] = array[i];
			array[i].color = tmp;

			/* fix the keys (can be only diminished with the last added color) */
			for (int j = i + 1, best = -1; j < length; j++) {
				int t = compare_cols(array[i].color, array[j].color);
				if (t < array[j].key) {
					array[j].key = t;
				}
				/* find the maximum for swapping */
				if (array[j].key > best) {
					best = array[j].key;
					bestpos = j;
				}
			}
		}
	}
}

/* searches the array of length for the color */
static INLINE int no_such_color(ITEM *array, int length, int color, int mask) {
	for (int i = 0; i < length; i++)
		if ((array[i].color & mask) == color)
			return 0;

	return 1;
}

/* copy color to palette */
static INLINE void copy_color(RGB *rgb, int color) {
	rgb->r = (color & 0xFF);
	rgb->g = (color >> 8) & 0xFF;
	rgb->b = (color >> 16) & 0xFF;
}

/* generate_optimized_palette_ex:
 *  Calculates a suitable palette for color reducing the specified truecolor
 *  image. If the rsvdcols parameter is not NULL, it contains an array of
 *  256 flags. If rsvdcols[n] > 0 the palette entry is assumed to be already
 *  set so I count with it. If rsvdcols[n] < 0 I mustn't assume anything about
 *  the entry. If rsvdcols[n] == 0 the entry is free for me to change.
 *
 *  Variable fraction controls, how big part of the palette should be
 *  filled with 'different colors', maxswaps gives upper boundary for
 *  number of swaps and mindiff chooses when to stop replacing values
 */
static int generate_optimized_palette_ex(BITMAP *image, PALETTE pal, AL_CONST signed char *rsvdcols, int bitsperrgb, int fraction, int maxswaps, int mindiff) {
	int x, y, imgdepth, numcols, palsize, rsvdcnt = 0, rsvduse = 0;
	unsigned int prec_mask, prec_mask2, bitmask15, bitmask16, bitmask24;
	signed char tmprsvd[256];
	int rshift, gshift, bshift;
	ITEM *colors;

	switch (bitsperrgb) {
		case 4:
			prec_mask = 0x3C3C3C;
			prec_mask2 = 0;
			bitmask15 = 0x7BDE; /* 0111 1011 1101 1110 */
			bitmask16 = 0xF79E; /* 1111 0111 1001 1110 */
			bitmask24 = 0xF0F0F0;
			break;

		case 5:
			prec_mask = 0x3E3E3E;
			prec_mask2 = 0x3C3C3C;
			bitmask15 = 0x7FFF; /* 0111 1111 1111 1111 */
			bitmask16 = 0xFFDF; /* 1111 1111 1101 1111 */
			bitmask24 = 0xF8F8F8;
			break;

		default:
			return -1;
	}

	distinct = 0;

	imgdepth = bitmap_color_depth(image);
	if (imgdepth == 8)
		return 0;

	hash_table = _AL_MALLOC(HASHTABLESIZE * sizeof(NODE));
	if (hash_table == NULL)
		return 0;

	for (int i = 0; i < HASHTABLESIZE; i++) {
		hash_table[i].next = NULL;
		hash_table[i].color = -1;
		hash_table[i].count = 0;
	}

	/* count the number of colors we shouldn't modify */
	if (rsvdcols) {
		for (int i = 0; i < 256; i++) {
			if (rsvdcols[i]) {
				rsvdcnt++;
				if (rsvdcols[i] > 0)
					rsvduse++;
			}
		}
	} else {
		pal[0].r = 63;
		pal[0].g = 0;
		pal[0].b = 63;

		tmprsvd[0] = 1;
		rsvdcnt++;
		rsvduse++;

		for (int i = 1; i < 256; i++)
			tmprsvd[i] = 0;

		rsvdcols = tmprsvd;
	}

	/* fix palette */
	for (int i = 0; i < 256; i++) {
		pal[i].r &= 0x3F;
		pal[i].g &= 0x3F;
		pal[i].b &= 0x3F;
	}

	/* fill the 'hash_table' with 4bit per RGB color values */

	switch (imgdepth) {
		case 32:
			for (y = 0; y < image->h; y++)
				for (x = 0; x < image->w; x++)
					insert_node(bmp_read32((uintptr_t)image->line[y] + x * sizeof(int32_t)) & bitmask24);
			break;

		case 24:
			for (y = 0; y < image->h; y++)
				for (x = 0; x < image->w; x++)
					insert_node(bmp_read24((uintptr_t)image->line[y] + x * 3) & bitmask24);
			break;

		case 16:
			for (y = 0; y < image->h; y++)
				for (x = 0; x < image->w; x++)
					insert_node(bmp_read16((uintptr_t)image->line[y] + x * sizeof(short)) & bitmask16);
			break;

		case 15:
			for (y = 0; y < image->h; y++)
				for (x = 0; x < image->w; x++)
					insert_node(bmp_read15((uintptr_t)image->line[y] + x * sizeof(short)) & bitmask15);
			break;

		default:
			return -1;
	}

	/* convert the 'hash_table' to array 'colors' */
	colors = _AL_MALLOC((rsvduse + distinct) * sizeof(ITEM));
	if (colors == NULL) {
		_AL_FREE(hash_table);
		return 0;
	}

	for (int i = 0, j = rsvduse; i < HASHTABLESIZE; i++) {
		if (hash_table[i].count) {
			NODE *node = &hash_table[i];

			do {
				colors[j].color = node->color;
				colors[j++].key = node->count;
				node = node->next;
			} while (node != NULL);

			if (hash_table[i].next)
				delete_list(hash_table[i].next);
		}
	}

	_AL_FREE(hash_table);

	/* sort the list with biggest count first */
	qsort(colors + rsvduse, distinct, sizeof(ITEM), qsort_helper_ITEM);

	/* we don't want to deal anymore with colors that are seldomly(?) used */
	numcols = rsvduse + distinct;
	palsize = 256 - rsvdcnt + rsvduse;

	/* change the format of the color information to some faster one
	 * (in fact to the 00BBBB?0 00GGGG?0 00RRRR?0).
	 */

	switch (imgdepth) {
		case 32:
			rshift = _rgb_r_shift_32 + 3;
			gshift = _rgb_g_shift_32 + 3;
			bshift = _rgb_b_shift_32 + 3;
			break;

		case 24:
			rshift = _rgb_r_shift_24 + 3;
			gshift = _rgb_g_shift_24 + 3;
			bshift = _rgb_b_shift_24 + 3;
			break;

		case 16:
			rshift = _rgb_r_shift_16;
			gshift = _rgb_g_shift_16 + 1;
			bshift = _rgb_b_shift_16;
			break;

		case 15:
			rshift = _rgb_r_shift_15;
			gshift = _rgb_g_shift_15;
			bshift = _rgb_b_shift_15;
			break;

		default:
			return -1;
	}

	for (int i = rsvduse; i < numcols; i++) {
		int r = (colors[i].color >> rshift) & 0x1F;
		int g = (colors[i].color >> gshift) & 0x1F;
		int b = (colors[i].color >> bshift) & 0x1F;
		colors[i].color = ((r << 1) | (g << 9) | (b << 17));
	}

	do {
		int j = 0, k = 0;

		/* there may be only small number of numcols colors, so we don't need
		 * any optimization
		 */
		if (numcols <= palsize)
			break;

		if (rsvduse > 0) {
			/* copy 'rsvd' to the 'colors' */
			for (int i = 0, j = 0; i < rsvduse; j++)
				if (rsvdcols[j] > 0)
					colors[i++].color = (pal[j].r | (pal[j].g << 8) | (pal[j].b << 16));

			/* reduce 'colors' skipping colors contained in 'rsvd' palette */
			for (int i = rsvduse, j = i; i < numcols; i++)
				if (no_such_color(colors, rsvduse, colors[i].color, prec_mask))
					colors[j++].color = colors[i].color;

			/* now there are j colors in 'common'  */
			numcols = j;

			/* now there might be enough free cells in palette */
			if (numcols <= palsize)
				break;
		}

		/* from 'start' will start swapping colors */
		int start = palsize - palsize / fraction;

		/* it may be slow, so don't let replace too many colors */
		if (start < (palsize - maxswaps))
			start = palsize - maxswaps;

		/* swap not less than 10 colors */
		if (start > (palsize - 10))
			start = rsvduse;

		/* don't swap reserved colors */
		if (start < rsvduse)
			start = rsvduse;

		if (bitsperrgb == 5) {
			/* do second pass on the colors we'll possibly use to replace (lower
			   bits per pixel to 4) - this would effectively lower the maximum
			   number of different colors to some 4000 (from 32000) */
			for (int i = start, k = i; i < numcols; i++) {
				for (j = 0; j < k; j++) {
					if ((colors[j].color & prec_mask2) == (colors[i].color & prec_mask2)) {
						j = -1;
						break;
					}
				}
				/* add this color if there is not similar one */
				if (j != -1)
					colors[k++].color = colors[i].color;
			}

			/* now there are k colors in 'common' */
			numcols = k;

			/* now there might be enough free cells in palette */
			if (numcols <= palsize)
				break;
		}

		/* start finding the most different colors */
		optimize_colors(colors, start, palsize, numcols, mindiff);

		numcols = palsize;
	} while (0);

	/* copy used colors to 'pal', skipping 'rsvd' */
	for (int i = rsvduse, j = 0; i < numcols; j++)
		if (!rsvdcols[j])
			copy_color(&pal[j], colors[i++].color);

	_AL_FREE(colors);

	return distinct;
}

int generate_optimized_palette(BITMAP *image, PALETTE pal, AL_CONST signed char rsvdcols[PAL_SIZE]) {
	ASSERT(image);
	return generate_optimized_palette_ex(image, pal, rsvdcols, DEFAULT_PREC, DEFAULT_FRACTION, DEFAULT_MAXSWAPS, DEFAULT_MINDIFF);
}

/*
 * Hicolor dithering routines.
 * ===========================
 */

static unsigned char dither_table[8] = { 0, 16, 68, 146, 170, 109, 187, 239 };
static unsigned char dither_ytable[8] = { 1, 5, 2, 7, 4, 0, 6, 3 };

/* makecol15_dither:
 * Calculates a dithered 15 bit pixel value.
 */
int makecol15_dither(int r, int g, int b, int x, int y) {
	int returned_r, returned_g, returned_b;
	int bpos;

	returned_r = r >> 3;
	returned_g = g >> 3;
	returned_b = b >> 3;

	y = dither_ytable[y & 7];

	bpos = (x + y) & 7;
	returned_r += (dither_table[r & 7] >> bpos) & 1;

	bpos = (bpos + 3) & 7;
	returned_b += (dither_table[b & 7] >> bpos) & 1;

	bpos = (bpos + 7) & 7;
	returned_g += (dither_table[g & 7] >> bpos) & 1;

	returned_r -= returned_r >> 5;
	returned_g -= returned_g >> 5;
	returned_b -= returned_b >> 5;

	return (returned_r << _rgb_r_shift_15) | (returned_g << _rgb_g_shift_15) | (returned_b << _rgb_b_shift_15);
}

/* makecol16_dither:
 *  Calculates a dithered 16 bit pixel value.
 */
int makecol16_dither(int r, int g, int b, int x, int y) {
	int returned_r, returned_g, returned_b;
	int bpos;

	returned_r = r >> 3;
	returned_g = g >> 2;
	returned_b = b >> 3;

	y = dither_ytable[y & 7];

	bpos = (x + y) & 7;
	returned_r += (dither_table[r & 7] >> bpos) & 1;

	bpos = (bpos + 3) & 7;
	returned_b += (dither_table[b & 7] >> bpos) & 1;

	bpos = (bpos + 7) & 7;
	returned_g += (dither_table[(g & 3) * 2] >> bpos) & 1;

	returned_r -= returned_r >> 5;
	returned_g -= returned_g >> 6;
	returned_b -= returned_b >> 5;

	return (returned_r << _rgb_r_shift_16) | (returned_g << _rgb_g_shift_16) | (returned_b << _rgb_b_shift_16);
}

/*
 * Graphics mode set and bitmap creation routines.
 * ===============================================
 */

extern void blit_end(void); /* for LOCK_FUNCTION; defined in blit.c */

int _sub_bitmap_id_count = 1; /* hash value for sub-bitmaps */
int _screen_split_position = 0; /* has the screen been split? */
int _safe_gfx_mode_change = 0; /* are we getting through GFX_SAFE? */
RGB_MAP *rgb_map = NULL; /* RGB -> palette entry conversion */
COLOR_MAP *color_map = NULL; /* translucency/lighting table */
#ifdef ALLEGRO_COLOR24 /* how many bits per pixel? */
int _color_depth = 24;
#else
int _color_depth = 8;
#endif
int _color_conv = COLORCONV_TOTAL; /* which formats to auto convert? */
static int color_conv_set = FALSE; /* has the user set conversion mode? */

int _palette_color8[256]; /* palette -> pixel mapping */
int _palette_color15[256];
int _palette_color16[256];
int _palette_color24[256];
int _palette_color32[256];

int *palette_color = _palette_color8;

BLENDER_FUNC _blender_func15 = NULL; /* truecolor pixel blender routines */
BLENDER_FUNC _blender_func16 = NULL;
BLENDER_FUNC _blender_func24 = NULL;
BLENDER_FUNC _blender_func32 = NULL;

BLENDER_FUNC _blender_func15x = NULL;
BLENDER_FUNC _blender_func16x = NULL;
BLENDER_FUNC _blender_func24x = NULL;

int _blender_col_15 = 0; /* for truecolor lit sprites */
int _blender_col_16 = 0;
int _blender_col_24 = 0;
int _blender_col_32 = 0;

int _blender_alpha = 0; /* for truecolor translucent drawing */

int _rgb_r_shift_15 = DEFAULT_RGB_R_SHIFT_15; /* truecolor pixel format */
int _rgb_g_shift_15 = DEFAULT_RGB_G_SHIFT_15;
int _rgb_b_shift_15 = DEFAULT_RGB_B_SHIFT_15;
int _rgb_r_shift_16 = DEFAULT_RGB_R_SHIFT_16;
int _rgb_g_shift_16 = DEFAULT_RGB_G_SHIFT_16;
int _rgb_b_shift_16 = DEFAULT_RGB_B_SHIFT_16;
int _rgb_r_shift_24 = DEFAULT_RGB_R_SHIFT_24;
int _rgb_g_shift_24 = DEFAULT_RGB_G_SHIFT_24;
int _rgb_b_shift_24 = DEFAULT_RGB_B_SHIFT_24;
int _rgb_r_shift_32 = DEFAULT_RGB_R_SHIFT_32;
int _rgb_g_shift_32 = DEFAULT_RGB_G_SHIFT_32;
int _rgb_b_shift_32 = DEFAULT_RGB_B_SHIFT_32;
int _rgb_a_shift_32 = DEFAULT_RGB_A_SHIFT_32;

/* lookup table for scaling 5 bit colors up to 8 bits */
int _rgb_scale_5[32] = {
	0, 8, 16, 24, 33, 41, 49, 57,
	66, 74, 82, 90, 99, 107, 115, 123,
	132, 140, 148, 156, 165, 173, 181, 189,
	198, 206, 214, 222, 231, 239, 247, 255
};

/* lookup table for scaling 6 bit colors up to 8 bits */
int _rgb_scale_6[64] = {
	0, 4, 8, 12, 16, 20, 24, 28,
	32, 36, 40, 44, 48, 52, 56, 60,
	65, 69, 73, 77, 81, 85, 89, 93,
	97, 101, 105, 109, 113, 117, 121, 125,
	130, 134, 138, 142, 146, 150, 154, 158,
	162, 166, 170, 174, 178, 182, 186, 190,
	195, 199, 203, 207, 211, 215, 219, 223,
	227, 231, 235, 239, 243, 247, 251, 255
};

#define BMP_MAX_SIZE 46340 /* sqrt(INT_MAX) */

/* the product of these must fit in an int */
static int failed_bitmap_w = BMP_MAX_SIZE;
static int failed_bitmap_h = BMP_MAX_SIZE;

/* lock_bitmap:
 *  Locks all the memory used by a bitmap structure.
 */
void lock_bitmap(BITMAP *bmp) {
	LOCK_DATA(bmp, sizeof(BITMAP) + sizeof(char *) * bmp->h);

	if (bmp->dat) {
		LOCK_DATA(bmp->dat, bmp->w * bmp->h * BYTES_PER_PIXEL(bitmap_color_depth(bmp)));
	}
}

/* set_color_depth:
 *  Sets the pixel size (in bits) which will be used by subsequent calls to
 *  set_gfx_mode() and create_bitmap(). Valid depths are 8, 15, 16, 24 and 32.
 */
void set_color_depth(int depth) {
	_color_depth = depth;

	switch (depth) {
		case 8:
			palette_color = _palette_color8;
			break;
		case 15:
			palette_color = _palette_color15;
			break;
		case 16:
			palette_color = _palette_color16;
			break;
		case 24:
			palette_color = _palette_color24;
			break;
		case 32:
			palette_color = _palette_color32;
			break;
		default:
			ASSERT(FALSE);
	}
}

/* get_color_depth:
 *  Returns the current color depth.
 */
int get_color_depth(void) { return _color_depth; }

/* set_color_conversion:
 *  Sets a bit mask specifying which types of color format conversions are
 *  valid when loading data from disk.
 */
void set_color_conversion(int mode) {
	_color_conv = mode;
	color_conv_set = TRUE;
}

/* get_color_conversion:
 *  Returns the bitmask specifying which types of color format
 *  conversion are valid when loading data from disk.
 */
int get_color_conversion(void) { return _color_conv; }

/* _color_load_depth:
 *  Works out which color depth an image should be loaded as, given the
 *  current conversion mode.
 */
int _color_load_depth(int depth, int hasalpha) {
	typedef struct CONVERSION_FLAGS {
		int flag;
		int in_depth;
		int out_depth;
		int hasalpha;
	} CONVERSION_FLAGS;

	static CONVERSION_FLAGS conversion_flags[] = {
		{ COLORCONV_8_TO_15, 8, 15, FALSE },
		{ COLORCONV_8_TO_16, 8, 16, FALSE },
		{ COLORCONV_8_TO_24, 8, 24, FALSE },
		{ COLORCONV_8_TO_32, 8, 32, FALSE },
		{ COLORCONV_15_TO_8, 15, 8, FALSE },
		{ COLORCONV_15_TO_16, 15, 16, FALSE },
		{ COLORCONV_15_TO_24, 15, 24, FALSE },
		{ COLORCONV_15_TO_32, 15, 32, FALSE },
		{ COLORCONV_16_TO_8, 16, 8, FALSE },
		{ COLORCONV_16_TO_15, 16, 15, FALSE },
		{ COLORCONV_16_TO_24, 16, 24, FALSE },
		{ COLORCONV_16_TO_32, 16, 32, FALSE },
		{ COLORCONV_24_TO_8, 24, 8, FALSE },
		{ COLORCONV_24_TO_15, 24, 15, FALSE },
		{ COLORCONV_24_TO_16, 24, 16, FALSE },
		{ COLORCONV_24_TO_32, 24, 32, FALSE },
		{ COLORCONV_32_TO_8, 32, 8, FALSE },
		{ COLORCONV_32_TO_15, 32, 15, FALSE },
		{ COLORCONV_32_TO_16, 32, 16, FALSE },
		{ COLORCONV_32_TO_24, 32, 24, FALSE },
		{ COLORCONV_32A_TO_8, 32, 8, TRUE },
		{ COLORCONV_32A_TO_15, 32, 15, TRUE },
		{ COLORCONV_32A_TO_16, 32, 16, TRUE },
		{ COLORCONV_32A_TO_24, 32, 24, TRUE }
	};

	ASSERT(color_conv_set);

	if (depth == _color_depth)
		return depth;

	for (int i = 0; i < (int)(sizeof(conversion_flags) / sizeof(CONVERSION_FLAGS)); i++) {
		if ((conversion_flags[i].in_depth == depth) &&
				(conversion_flags[i].out_depth == _color_depth) &&
				((conversion_flags[i].hasalpha != 0) == (hasalpha != 0))) {
			if (_color_conv & conversion_flags[i].flag)
				return _color_depth;
			else
				return depth;
		}
	}

	ASSERT(FALSE);
	return 0;
}

/* _get_vtable:
 *  Returns a pointer to the linear vtable for the specified color depth.
 */
GFX_VTABLE *_get_vtable(int color_depth) {
	for (int i = 0; _vtable_list[i].vtable; i++) {
		if (_vtable_list[i].color_depth == color_depth) {
			LOCK_DATA(_vtable_list[i].vtable, sizeof(GFX_VTABLE));
			LOCK_CODE(_vtable_list[i].vtable->draw_sprite, (long)_vtable_list[i].vtable->draw_sprite_end - (long)_vtable_list[i].vtable->draw_sprite);
			LOCK_CODE(_vtable_list[i].vtable->blit_from_memory, (long)_vtable_list[i].vtable->blit_end - (long)_vtable_list[i].vtable->blit_from_memory);
			return _vtable_list[i].vtable;
		}
	}

	return NULL;
}

/* create_bitmap_ex
 *  Creates a new memory bitmap in the specified color_depth
 */
BITMAP *create_bitmap_ex(int color_depth, int width, int height) {
	GFX_VTABLE *vtable;
	BITMAP *bitmap;
	int nr_pointers;
	int padding;

	ASSERT(width >= 0);
	ASSERT(height > 0);
	ASSERT(system_driver);

	vtable = _get_vtable(color_depth);
	if (!vtable)
		return NULL;

	/* We need at least two pointers when drawing, otherwise we get crashes with
	 * Electric Fence.  We think some of the assembly code assumes a second line
	 * pointer is always available.
	 */
	nr_pointers = MAX(2, height);
	bitmap = _AL_MALLOC(sizeof(BITMAP) + (sizeof(char *) * nr_pointers));
	if (!bitmap)
		return NULL;

	/* This avoids a crash for assembler code accessing the last pixel, as it
	 * read 4 bytes instead of 3.
	 */
	padding = (color_depth == 24) ? 1 : 0;

	bitmap->dat = _AL_MALLOC_ATOMIC(width * height * BYTES_PER_PIXEL(color_depth) + padding);
	if (!bitmap->dat) {
		_AL_FREE(bitmap);
		return NULL;
	}

	bitmap->w = bitmap->cr = width;
	bitmap->h = bitmap->cb = height;
	bitmap->clip = TRUE;
	bitmap->cl = bitmap->ct = 0;
	bitmap->vtable = vtable;
	bitmap->id = 0;
	bitmap->extra = NULL;
	bitmap->x_ofs = 0;
	bitmap->y_ofs = 0;
	bitmap->seg = _default_ds();

	if (height > 0) {
		bitmap->line[0] = bitmap->dat;
		for (int i = 1; i < height; i++)
			bitmap->line[i] = bitmap->line[i - 1] + width * BYTES_PER_PIXEL(color_depth);
	}

	return bitmap;
}

/* create_bitmap:
 *  Creates a new memory bitmap.
 */
BITMAP *create_bitmap(int width, int height) {
	ASSERT(width >= 0);
	ASSERT(height > 0);
	return create_bitmap_ex(_color_depth, width, height);
}

/* create_sub_bitmap:
 *  Creates a sub bitmap, ie. a bitmap sharing drawing memory with a
 *  pre-existing bitmap, but possibly with different clipping settings.
 *  Usually will be smaller, and positioned at some arbitrary point.
 *
 *  Mark Wodrich is the owner of the brain responsible this hugely useful
 *  and beautiful function.
 */
BITMAP *create_sub_bitmap(BITMAP *parent, int x, int y, int width, int height) {
	BITMAP *bitmap;
	int nr_pointers;

	ASSERT(parent);
	ASSERT((x >= 0) && (y >= 0) && (x < parent->w) && (y < parent->h));
	ASSERT((width > 0) && (height > 0));
	ASSERT(system_driver);

	if (x + width > parent->w)
		width = parent->w - x;

	if (y + height > parent->h)
		height = parent->h - y;

	if (parent->vtable->create_sub_bitmap)
		return parent->vtable->create_sub_bitmap(parent, x, y, width, height);

	/* get memory for structure and line pointers */
	/* (see create_bitmap for the reason we need at least two) */
	nr_pointers = MAX(2, height);
	bitmap = _AL_MALLOC(sizeof(BITMAP) + (sizeof(char *) * nr_pointers));
	if (!bitmap)
		return NULL;

	acquire_bitmap(parent);

	bitmap->w = bitmap->cr = width;
	bitmap->h = bitmap->cb = height;
	bitmap->clip = TRUE;
	bitmap->cl = bitmap->ct = 0;
	bitmap->vtable = parent->vtable;
	bitmap->dat = NULL;
	bitmap->extra = NULL;
	bitmap->x_ofs = x + parent->x_ofs;
	bitmap->y_ofs = y + parent->y_ofs;
	bitmap->seg = parent->seg;

	/* All bitmaps are created with zero ID's. When a sub-bitmap is created,
	 * a unique ID is needed to identify the relationship when blitting from
	 * one to the other. This is obtained from the global variable
	 * _sub_bitmap_id_count, which provides a sequence of integers (yes I
	 * know it will wrap eventually, but not for a long time :-) If the
	 * parent already has an ID the sub-bitmap adopts it, otherwise a new
	 * ID is given to both the parent and the child.
	 */
	if (!(parent->id & BMP_ID_MASK)) {
		parent->id |= _sub_bitmap_id_count;
		_sub_bitmap_id_count = (_sub_bitmap_id_count + 1) & BMP_ID_MASK;
	}

	bitmap->id = parent->id | BMP_ID_SUB;
	bitmap->id &= ~BMP_ID_LOCKED;

	x *= BYTES_PER_PIXEL(bitmap_color_depth(bitmap));

	/* setup line pointers: each line points to a line in the parent bitmap */
	for (int i = 0; i < height; i++)
		bitmap->line[i] = parent->line[y + i] + x;

	if (bitmap->vtable->set_clip)
		bitmap->vtable->set_clip(bitmap);

	if (parent->vtable->created_sub_bitmap)
		parent->vtable->created_sub_bitmap(bitmap, parent);

	release_bitmap(parent);

	return bitmap;
}

/* destroy_bitmap:
 *  Destroys a memory bitmap.
 */
void destroy_bitmap(BITMAP *bitmap) {
	if (bitmap) {
		/* normal memory or sub-bitmap destruction */
		if (bitmap->dat)
			_AL_FREE(bitmap->dat);

		_AL_FREE(bitmap);
	}
}

/* set_clip_rect:
 *  Sets the two opposite corners of the clipping rectangle to be used when
 *  drawing to the bitmap. Nothing will be drawn to positions outside of this
 *  rectangle. When a new bitmap is created the clipping rectangle will be
 *  set to the full area of the bitmap.
 */
void set_clip_rect(BITMAP *bitmap, int x1, int y1, int x2, int y2) {
	ASSERT(bitmap);

	/* internal clipping is inclusive-exclusive */
	x2++;
	y2++;

	bitmap->cl = CLAMP(0, x1, bitmap->w - 1);
	bitmap->ct = CLAMP(0, y1, bitmap->h - 1);
	bitmap->cr = CLAMP(0, x2, bitmap->w);
	bitmap->cb = CLAMP(0, y2, bitmap->h);

	if (bitmap->vtable->set_clip)
		bitmap->vtable->set_clip(bitmap);
}

/* add_clip_rect:
 *  Makes the new clipping rectangle the intersection between the given
 *  rectangle and the current one.
 */
void add_clip_rect(BITMAP *bitmap, int x1, int y1, int x2, int y2) {
	int cx1, cy1, cx2, cy2;

	ASSERT(bitmap);

	get_clip_rect(bitmap, &cx1, &cy1, &cx2, &cy2);

	x1 = MAX(x1, cx1);
	y1 = MAX(y1, cy1);
	x2 = MIN(x2, cx2);
	y2 = MIN(y2, cy2);

	set_clip_rect(bitmap, x1, y1, x2, y2);
}

/* set_clip:
 *  Sets the two opposite corners of the clipping rectangle to be used when
 *  drawing to the bitmap. Nothing will be drawn to positions outside of this
 *  rectangle. When a new bitmap is created the clipping rectangle will be
 *  set to the full area of the bitmap. If x1, y1, x2 and y2 are all zero
 *  clipping will be turned off, which will slightly speed up drawing
 *  operations but will allow memory to be corrupted if you attempt to draw
 *  off the edge of the bitmap.
 */
void set_clip(BITMAP *bitmap, int x1, int y1, int x2, int y2) {
	int t;

	ASSERT(bitmap);

	if ((!x1) && (!y1) && (!x2) && (!y2)) {
		set_clip_rect(bitmap, 0, 0, bitmap->w - 1, bitmap->h - 1);
		set_clip_state(bitmap, FALSE);
		return;
	}

	if (x2 < x1) {
		t = x1;
		x1 = x2;
		x2 = t;
	}

	if (y2 < y1) {
		t = y1;
		y1 = y2;
		y2 = t;
	}

	set_clip_rect(bitmap, x1, y1, x2, y2);
	set_clip_state(bitmap, TRUE);
}

/*
 * Sprite rotation routines.
 * =========================
 */

/* Scanline drawers. */

#define SCANLINE_DRAWER_GENERIC(name, INIT, PUTPIXEL)            \
	static void draw_scanline_##name(BITMAP *bmp, BITMAP *spr,   \
			fixed l_bmp_x, int bmp_y_i,                          \
			fixed r_bmp_x,                                       \
			fixed l_spr_x, fixed l_spr_y,                        \
			fixed spr_dx, fixed spr_dy) {                        \
		int mask_color;                                          \
                                                                 \
		INIT;                                                    \
		mask_color = bmp->vtable->mask_color;                    \
		r_bmp_x >>= 16;                                          \
		l_bmp_x >>= 16;                                          \
		for (; l_bmp_x <= r_bmp_x; l_bmp_x++) {                  \
			int c = getpixel(spr, l_spr_x >> 16, l_spr_y >> 16); \
			if (c != mask_color)                                 \
				PUTPIXEL;                                        \
			l_spr_x += spr_dx;                                   \
			l_spr_y += spr_dy;                                   \
		}                                                        \
	}

SCANLINE_DRAWER_GENERIC(generic_convert,
						int bmp_depth;
						int spr_depth;
						bmp_depth = bitmap_color_depth(bmp);
						spr_depth = bitmap_color_depth(spr);
						,
						putpixel(bmp, l_bmp_x, bmp_y_i,
								makecol_depth(bmp_depth,
										getr_depth(spr_depth, c),
										getg_depth(spr_depth, c),
										getb_depth(spr_depth, c))))

SCANLINE_DRAWER_GENERIC(generic,
						; /* nop */
						,
						putpixel(bmp, l_bmp_x, bmp_y_i, c))

#define SCANLINE_DRAWER(bits_pp, GETPIXEL)                        \
	static void draw_scanline_##bits_pp(BITMAP *bmp, BITMAP *spr, \
			fixed l_bmp_x, int bmp_y_i,                           \
			fixed r_bmp_x,                                        \
			fixed l_spr_x, fixed l_spr_y,                         \
			fixed spr_dx, fixed spr_dy) {                         \
		int c;                                                    \
		uintptr_t addr, end_addr;                                 \
		unsigned char **spr_line = spr->line;                     \
                                                                  \
		r_bmp_x >>= 16;                                           \
		l_bmp_x >>= 16;                                           \
		addr = bmp_write_line(bmp, bmp_y_i);                      \
		end_addr = addr + r_bmp_x * ((bits_pp + 7) / 8);          \
		addr += l_bmp_x * ((bits_pp + 7) / 8);                    \
		for (; addr <= end_addr; addr += ((bits_pp + 7) / 8)) {   \
			GETPIXEL;                                             \
			if (c != MASK_COLOR_##bits_pp)                        \
				bmp_write##bits_pp(addr, c);                      \
			l_spr_x += spr_dx;                                    \
			l_spr_y += spr_dy;                                    \
		}                                                         \
	}

#ifdef ALLEGRO_COLOR8
SCANLINE_DRAWER(8, c = spr_line[l_spr_y >> 16][l_spr_x >> 16]);
#endif

#ifdef ALLEGRO_COLOR16
SCANLINE_DRAWER(15, c = ((unsigned short *)spr_line[l_spr_y >> 16])[l_spr_x >> 16])
SCANLINE_DRAWER(16, c = ((unsigned short *)spr_line[l_spr_y >> 16])[l_spr_x >> 16])
#endif

#ifdef ALLEGRO_COLOR24
#ifdef ALLEGRO_LITTLE_ENDIAN
SCANLINE_DRAWER(24,
		{
			unsigned char *p = spr_line[l_spr_y >> 16] + (l_spr_x >> 16) * 3;
			c = p[0];
			c |= (int)p[1] << 8;
			c |= (int)p[2] << 16;
		})
#else
SCANLINE_DRAWER(24,
		{
			unsigned char *p = spr_line[l_spr_y >> 16] + (l_spr_x >> 16) * 3;
			c = (int)p[0] << 16;
			c |= (int)p[1] << 8;
			c |= p[2];
		})
#endif
#endif

#ifdef ALLEGRO_COLOR32
SCANLINE_DRAWER(32, c = ((uint32_t *)spr_line[l_spr_y >> 16])[l_spr_x >> 16])
#endif

/* _parallelogram_map:
 *  Worker routine for drawing rotated and/or scaled and/or flipped sprites:
 *  It actually maps the sprite to any parallelogram-shaped area of the
 *  bitmap. The top left corner is mapped to (xs[0], ys[0]), the top right to
 *  (xs[1], ys[1]), the bottom right to x (xs[2], ys[2]), and the bottom left
 *  to (xs[3], ys[3]). The corners are assumed to form a perfect
 *  parallelogram, i.e. xs[0]+xs[2] = xs[1]+xs[3]. The corners are given in
 *  fixed point format, so xs[] and ys[] are coordinates of the outer corners
 *  of corner pixels in clockwise order beginning with top left.
 *  All coordinates begin with 0 in top left corner of pixel (0, 0). So a
 *  rotation by 0 degrees of a sprite to the top left of a bitmap can be
 *  specified with coordinates (0, 0) for the top left pixel in source
 *  bitmap. With the default scanline drawer, a pixel in the destination
 *  bitmap is drawn if and only if its center is covered by any pixel in the
 *  sprite. The color of this covering sprite pixel is used to draw.
 *  If sub_pixel_accuracy=FALSE, then the scanline drawer will be called with
 *  *_bmp_x being a fixed point representation of the integers representing
 *  the x coordinate of the first and last point in bmp whose centre is
 *  covered by the sprite. If sub_pixel_accuracy=TRUE, then the scanline
 *  drawer will be called with the exact fixed point position of the first
 *  and last point in which the horizontal line passing through the centre is
 *  at least partly covered by the sprite. This is useful for doing
 *  anti-aliased blending.
 */
void _parallelogram_map(BITMAP *bmp, BITMAP *spr, fixed xs[4], fixed ys[4],
		void (*draw_scanline)(BITMAP *bmp, BITMAP *spr,
				fixed l_bmp_x, int bmp_y,
				fixed r_bmp_x,
				fixed l_spr_x, fixed l_spr_y,
				fixed spr_dx, fixed spr_dy),
		int sub_pixel_accuracy) {
	int top_index; /* Index in xs[] and ys[] to topmost point. */
	int right_index; /* Rightmost point has index (top_index+right_index) int xs[] and ys[]. */
	int index, i; /* Loop variables. */
	fixed corner_bmp_x[4], corner_bmp_y[4]; /* Coordinates in bmp ordered as top-right-bottom-left. */
	fixed corner_spr_x[4], corner_spr_y[4]; /* Coordinates in spr ordered as top-right-bottom-left. */
	int clip_bottom_i, l_bmp_y_bottom_i, r_bmp_y_bottom_i; /* y coordinate of bottom point, left point and right point. */
	fixed clip_left, clip_right; /* Left and right clipping. */
	fixed extra_scanline_fraction; /* Temporary variable. */

	/*
	 * Variables used in the loop
	 */
	fixed l_spr_x, l_spr_y, l_bmp_x, l_bmp_dx; /* Coordinates of sprite and bmp points in beginning of scanline. */
	fixed l_spr_dx, l_spr_dy; /* Increment of left sprite point as we move a scanline down. */
	fixed r_bmp_x, r_bmp_dx; /* Coordinates of sprite and bmp points in end of scanline. */
#ifdef KEEP_TRACK_OF_RIGHT_SPRITE_SCANLINE
	fixed r_spr_x, r_spr_y;
	fixed r_spr_dx, r_spr_dy; /* Increment of right sprite point as we move a scanline down. */
#endif
	fixed spr_dx, spr_dy; /* Increment of sprite point as we move right inside a scanline. */
	fixed l_spr_x_rounded, l_spr_y_rounded, l_bmp_x_rounded; /* Positions of beginning of scanline after rounding to integer coordinate in bmp. */
	fixed r_bmp_x_rounded;
	int bmp_y_i; /* Current scanline. */
	int right_edge_test; /* Right edge of scanline. */

	top_index = 0; /* Get index of topmost point. */
	if (ys[1] < ys[0])
		top_index = 1;
	if (ys[2] < ys[top_index])
		top_index = 2;
	if (ys[3] < ys[top_index])
		top_index = 3;

	/* Get direction of points: clockwise or anti-clockwise. */
	right_index = (double)(xs[(top_index + 1) & 3] - xs[top_index]) * (double)(ys[(top_index - 1) & 3] - ys[top_index]) >
					(double)(xs[(top_index - 1) & 3] - xs[top_index]) * (double)(ys[(top_index + 1) & 3] - ys[top_index])
			? 1
			: -1;

	//FIXME: why does fixmul overflow below?

	/* if (fixmul(xs[(top_index+1) & 3] - xs[top_index], ys[(top_index-1) & 3] - ys[top_index]) > fixmul(xs[(top_index-1) & 3] - xs[top_index], ys[(top_index+1) & 3] - ys[top_index]))
	   right_index = 1;
	else
	   right_index = -1; */

	/*
	 * Get coordinates of the corners.
	 */

	/* corner_*[0] is top, [1] is right, [2] is bottom, [3] is left. */
	index = top_index;
	for (i = 0; i < 4; i++) {
		corner_bmp_x[i] = xs[index];
		corner_bmp_y[i] = ys[index];
		if (index < 2)
			corner_spr_y[i] = 0;
		else
			/* Need `- 1' since otherwise it would be outside sprite. */
			corner_spr_y[i] = (spr->h << 16) - 1;
		if ((index == 0) || (index == 3))
			corner_spr_x[i] = 0;
		else
			corner_spr_x[i] = (spr->w << 16) - 1;
		index = (index + right_index) & 3;
	}

/*
 * Get scanline starts, ends and deltas, and clipping coordinates.
 */
#define top_bmp_y corner_bmp_y[0]
#define right_bmp_y corner_bmp_y[1]
#define bottom_bmp_y corner_bmp_y[2]
#define left_bmp_y corner_bmp_y[3]
#define top_bmp_x corner_bmp_x[0]
#define right_bmp_x corner_bmp_x[1]
#define bottom_bmp_x corner_bmp_x[2]
#define left_bmp_x corner_bmp_x[3]
#define top_spr_y corner_spr_y[0]
#define right_spr_y corner_spr_y[1]
#define bottom_spr_y corner_spr_y[2]
#define left_spr_y corner_spr_y[3]
#define top_spr_x corner_spr_x[0]
#define right_spr_x corner_spr_x[1]
#define bottom_spr_x corner_spr_x[2]
#define left_spr_x corner_spr_x[3]

	/* Calculate left and right clipping. */
	if (bmp->clip) {
		clip_left = bmp->cl << 16;
		clip_right = (bmp->cr << 16) - 1;
	} else {
		ASSERT(left_bmp_x >= 0 && top_bmp_x >= 0 && bottom_bmp_x >= 0 && right_bmp_x < (bmp->w << 16) && top_bmp_x < (bmp->w << 16) && bottom_bmp_x < (bmp->w << 16));
		clip_left = 0;
		clip_right = (bmp->w << 16) - 1;
	}

	/* Quit if we're totally outside. */
	if ((left_bmp_x > clip_right) &&
			(top_bmp_x > clip_right) &&
			(bottom_bmp_x > clip_right))
		return;
	if ((right_bmp_x < clip_left) &&
			(top_bmp_x < clip_left) &&
			(bottom_bmp_x < clip_left))
		return;

	/* Bottom clipping. */
	if (sub_pixel_accuracy)
		clip_bottom_i = (bottom_bmp_y + 0xffff) >> 16;
	else
		clip_bottom_i = (bottom_bmp_y + 0x8000) >> 16;
	if (bmp->clip) {
		if (clip_bottom_i > bmp->cb)
			clip_bottom_i = bmp->cb;
	} else {
		ASSERT(clip_bottom_i <= bmp->h);
	}

	/* Calculate y coordinate of first scanline. */
	if (sub_pixel_accuracy)
		bmp_y_i = top_bmp_y >> 16;
	else
		bmp_y_i = (top_bmp_y + 0x8000) >> 16;
	if (bmp->clip) {
		if (bmp_y_i < bmp->ct)
			bmp_y_i = bmp->ct;
	} else {
		ASSERT(bmp_y_i >= 0);
	}

	/* Sprite is above or below bottom clipping area. */
	if (bmp_y_i >= clip_bottom_i)
		return;

	extra_scanline_fraction = (bmp_y_i << 16) + 0x8000 - top_bmp_y; /* Vertical gap between top corner and centre of topmost scanline. */
	l_bmp_dx = fixdiv(left_bmp_x - top_bmp_x, left_bmp_y - top_bmp_y); /* Calculate x coordinate of beginning of scanline in bmp. */
	l_bmp_x = top_bmp_x + fixmul(extra_scanline_fraction, l_bmp_dx); /* Calculate x coordinate of beginning of scanline in spr. */
	/* note: all these are rounded down which is probably a Good Thing (tm) */
	l_spr_dx = fixdiv(left_spr_x - top_spr_x, left_bmp_y - top_bmp_y);
	l_spr_x = top_spr_x + fixmul(extra_scanline_fraction, l_spr_dx);
	l_spr_dy = fixdiv(left_spr_y - top_spr_y, left_bmp_y - top_bmp_y); /* Calculate y coordinate of beginning of scanline in spr. */
	l_spr_y = top_spr_y + fixmul(extra_scanline_fraction, l_spr_dy);

	l_bmp_y_bottom_i = (left_bmp_y + 0x8000) >> 16; /* Calculate left loop bound. */
	if (l_bmp_y_bottom_i > clip_bottom_i)
		l_bmp_y_bottom_i = clip_bottom_i;

	r_bmp_dx = fixdiv(right_bmp_x - top_bmp_x, right_bmp_y - top_bmp_y); /* Calculate x coordinate of end of scanline in bmp. */
	r_bmp_x = top_bmp_x + fixmul(extra_scanline_fraction, r_bmp_dx);
#ifdef KEEP_TRACK_OF_RIGHT_SPRITE_SCANLINE
	r_spr_dx = fixdiv(right_spr_x - top_spr_x, right_bmp_y - top_bmp_y); /* Calculate x coordinate of end of scanline in spr. */
	r_spr_x = top_spr_x + fixmul(extra_scanline_fraction, r_spr_dx);
	r_spr_dy = fixdiv(right_spr_y - top_spr_y, right_bmp_y - top_bmp_y); /* Calculate y coordinate of end of scanline in spr. */
	r_spr_y = top_spr_y + fixmul(extra_scanline_fraction, r_spr_dy);
#endif

	r_bmp_y_bottom_i = (right_bmp_y + 0x8000) >> 16; /* Calculate right loop bound. */

	/* Get dx and dy, the offsets to add to the source coordinates as we move
	   one pixel rightwards along a scanline. This formula can be derived by
	   considering the 2x2 matrix that transforms the sprite to the
	   parallelogram.
	   We'd better use double to get this as exact as possible, since any
	   errors will be accumulated along the scanline.
	*/
	spr_dx = (fixed)((ys[3] - ys[0]) * 65536.0 * (65536.0 * spr->w) / ((xs[1] - xs[0]) * (double)(ys[3] - ys[0]) - (xs[3] - xs[0]) * (double)(ys[1] - ys[0])));
	spr_dy = (fixed)((ys[1] - ys[0]) * 65536.0 * (65536.0 * spr->h) / ((xs[3] - xs[0]) * (double)(ys[1] - ys[0]) - (xs[1] - xs[0]) * (double)(ys[3] - ys[0])));

	/*
	 * Loop through scanlines.
	 */

	while (1) {
		/* Has beginning of scanline passed a corner? */
		if (bmp_y_i >= l_bmp_y_bottom_i) {
			/* Are we done? */
			if (bmp_y_i >= clip_bottom_i)
				break;

			/* Vertical gap between left corner and centre of scanline. */
			extra_scanline_fraction = (bmp_y_i << 16) + 0x8000 - left_bmp_y;
			/* Update x coordinate of beginning of scanline in bmp. */
			l_bmp_dx = fixdiv(bottom_bmp_x - left_bmp_x,
					bottom_bmp_y - left_bmp_y);
			l_bmp_x = left_bmp_x + fixmul(extra_scanline_fraction, l_bmp_dx);
			/* Update x coordinate of beginning of scanline in spr. */
			l_spr_dx = fixdiv(bottom_spr_x - left_spr_x,
					bottom_bmp_y - left_bmp_y);
			l_spr_x = left_spr_x + fixmul(extra_scanline_fraction, l_spr_dx);
			/* Update y coordinate of beginning of scanline in spr. */
			l_spr_dy = fixdiv(bottom_spr_y - left_spr_y,
					bottom_bmp_y - left_bmp_y);
			l_spr_y = left_spr_y + fixmul(extra_scanline_fraction, l_spr_dy);

			/* Update loop bound. */
			if (sub_pixel_accuracy)
				l_bmp_y_bottom_i = (bottom_bmp_y + 0xffff) >> 16;
			else
				l_bmp_y_bottom_i = (bottom_bmp_y + 0x8000) >> 16;
			if (l_bmp_y_bottom_i > clip_bottom_i)
				l_bmp_y_bottom_i = clip_bottom_i;
		}

		/* Has end of scanline passed a corner? */
		if (bmp_y_i >= r_bmp_y_bottom_i) {
			/* Vertical gap between right corner and centre of scanline. */
			extra_scanline_fraction = (bmp_y_i << 16) + 0x8000 - right_bmp_y;
			/* Update x coordinate of end of scanline in bmp. */
			r_bmp_dx = fixdiv(bottom_bmp_x - right_bmp_x,
					bottom_bmp_y - right_bmp_y);
			r_bmp_x = right_bmp_x + fixmul(extra_scanline_fraction, r_bmp_dx);
#ifdef KEEP_TRACK_OF_RIGHT_SPRITE_SCANLINE
			/* Update x coordinate of beginning of scanline in spr. */
			r_spr_dx = fixdiv(bottom_spr_x - right_spr_x,
					bottom_bmp_y - right_bmp_y);
			r_spr_x = right_spr_x + fixmul(extra_scanline_fraction, r_spr_dx);
			/* Update y coordinate of beginning of scanline in spr. */
			r_spr_dy = fixdiv(bottom_spr_y - right_spr_y,
					bottom_bmp_y - right_bmp_y);
			r_spr_y = right_spr_y + fixmul(extra_scanline_fraction, r_spr_dy);
#endif

			/* Update loop bound: We aren't supposed to use this any more, so just set it to some big enough value. */
			r_bmp_y_bottom_i = clip_bottom_i;
		}

		/* Make left bmp coordinate be an integer and clip it. */
		if (sub_pixel_accuracy)
			l_bmp_x_rounded = l_bmp_x;
		else
			l_bmp_x_rounded = (l_bmp_x + 0x8000) & ~0xffff;
		if (l_bmp_x_rounded < clip_left)
			l_bmp_x_rounded = clip_left;

		/* ... and move starting point in sprite accordingly. */
		if (sub_pixel_accuracy) {
			l_spr_x_rounded = l_spr_x + fixmul((l_bmp_x_rounded - l_bmp_x), spr_dx);
			l_spr_y_rounded = l_spr_y + fixmul((l_bmp_x_rounded - l_bmp_x), spr_dy);
		} else {
			l_spr_x_rounded = l_spr_x + fixmul(l_bmp_x_rounded + 0x7fff - l_bmp_x, spr_dx);
			l_spr_y_rounded = l_spr_y + fixmul(l_bmp_x_rounded + 0x7fff - l_bmp_x, spr_dy);
		}

		/* Make right bmp coordinate be an integer and clip it. */
		if (sub_pixel_accuracy)
			r_bmp_x_rounded = r_bmp_x;
		else
			r_bmp_x_rounded = (r_bmp_x - 0x8000) & ~0xffff;
		if (r_bmp_x_rounded > clip_right)
			r_bmp_x_rounded = clip_right;

		/* Draw! */
		if (l_bmp_x_rounded <= r_bmp_x_rounded) {
			if (!sub_pixel_accuracy) {
				/* The bodies of these ifs are only reached extremely seldom,
				   it's an ugly hack to avoid reading outside the sprite when
				   the rounding errors are accumulated the wrong way. It would
				   be nicer if we could ensure that this never happens by making
				   all multiplications and divisions be rounded up or down at
				   the correct places.
				   I did try another approach: recalculate the edges of the
				   scanline from scratch each scanline rather than incrementally.
				   Drawing a sprite with that routine took about 25% longer time
				   though.
				*/
				if ((unsigned)(l_spr_x_rounded >> 16) >= (unsigned)spr->w) {
					if (((l_spr_x_rounded < 0) && (spr_dx <= 0)) || ((l_spr_x_rounded > 0) && (spr_dx >= 0))) {
						goto skip_draw; /* This can happen. */
					} else {
						/* I don't think this can happen, but I can't prove it. */
						do {
							l_spr_x_rounded += spr_dx;
							l_bmp_x_rounded += 65536;
							if (l_bmp_x_rounded > r_bmp_x_rounded)
								goto skip_draw;
						} while ((unsigned)(l_spr_x_rounded >> 16) >= (unsigned)spr->w);
					}
				}
				right_edge_test = l_spr_x_rounded + ((r_bmp_x_rounded - l_bmp_x_rounded) >> 16) * spr_dx;
				if ((unsigned)(right_edge_test >> 16) >= (unsigned)spr->w) {
					if (((right_edge_test < 0) && (spr_dx <= 0)) ||
							((right_edge_test > 0) && (spr_dx >= 0))) {
						/* This can happen. */
						do {
							r_bmp_x_rounded -= 65536;
							right_edge_test -= spr_dx;
							if (l_bmp_x_rounded > r_bmp_x_rounded)
								goto skip_draw;
						} while ((unsigned)(right_edge_test >> 16) >= (unsigned)spr->w);
					} else {
						/* I don't think this can happen, but I can't prove it. */
						goto skip_draw;
					}
				}
				if ((unsigned)(l_spr_y_rounded >> 16) >= (unsigned)spr->h) {
					if (((l_spr_y_rounded < 0) && (spr_dy <= 0)) ||
							((l_spr_y_rounded > 0) && (spr_dy >= 0))) {
						/* This can happen. */
						goto skip_draw;
					} else {
						/* I don't think this can happen, but I can't prove it. */
						do {
							l_spr_y_rounded += spr_dy;
							l_bmp_x_rounded += 65536;
							if (l_bmp_x_rounded > r_bmp_x_rounded)
								goto skip_draw;
						} while (((unsigned)l_spr_y_rounded >> 16) >= (unsigned)spr->h);
					}
				}
				right_edge_test = l_spr_y_rounded +
						((r_bmp_x_rounded - l_bmp_x_rounded) >> 16) * spr_dy;
				if ((unsigned)(right_edge_test >> 16) >= (unsigned)spr->h) {
					if (((right_edge_test < 0) && (spr_dy <= 0)) ||
							((right_edge_test > 0) && (spr_dy >= 0))) {
						/* This can happen. */
						do {
							r_bmp_x_rounded -= 65536;
							right_edge_test -= spr_dy;
							if (l_bmp_x_rounded > r_bmp_x_rounded)
								goto skip_draw;
						} while ((unsigned)(right_edge_test >> 16) >= (unsigned)spr->h);
					} else {
						/* I don't think this can happen, but I can't prove it. */
						goto skip_draw;
					}
				}
			}
			draw_scanline(bmp, spr,
					l_bmp_x_rounded, bmp_y_i, r_bmp_x_rounded,
					l_spr_x_rounded, l_spr_y_rounded,
					spr_dx, spr_dy);
		}
	/* I'm not going to apoligize for this label and its gotos: to get
	   rid of it would just make the code look worse. */
	skip_draw:

		bmp_y_i++; /* Jump to next scanline. */
		l_bmp_x += l_bmp_dx; /* Update beginning of scanline. */
		l_spr_x += l_spr_dx;
		l_spr_y += l_spr_dy;
		r_bmp_x += r_bmp_dx; /* Update end of scanline. */
#ifdef KEEP_TRACK_OF_RIGHT_SPRITE_SCANLINE
		r_spr_x += r_spr_dx;
		r_spr_y += r_spr_dy;
#endif
	}
}

/* _parallelogram_map_standard:
 *  Helper function for calling _parallelogram_map() with the appropriate
 *  scanline drawer. I didn't want to include this in the
 *  _parallelogram_map() function since then you can bypass it and define
 *  your own scanline drawer, eg. for anti-aliased rotations.
 */
void _parallelogram_map_standard(BITMAP *bmp, BITMAP *sprite, fixed xs[4], fixed ys[4]) {
	int old_drawing_mode;
	if (bitmap_color_depth(bmp) != bitmap_color_depth(sprite)) { /* These scanline drawers use putpixel() so we must set solid mode. */
		old_drawing_mode = _drawing_mode;
		drawing_mode(DRAW_MODE_SOLID, _drawing_pattern, _drawing_x_anchor, _drawing_y_anchor);
		_parallelogram_map(bmp, sprite, xs, ys, draw_scanline_generic_convert, FALSE);
		drawing_mode(old_drawing_mode, _drawing_pattern, _drawing_x_anchor, _drawing_y_anchor);
	} else {
		switch (bitmap_color_depth(bmp)) {
#ifdef ALLEGRO_COLOR8
			case 8:
				_parallelogram_map(bmp, sprite, xs, ys, draw_scanline_8, FALSE);
				break;
#endif

#ifdef ALLEGRO_COLOR16
			case 15:
				_parallelogram_map(bmp, sprite, xs, ys, draw_scanline_15, FALSE);
				break;

			case 16:
				_parallelogram_map(bmp, sprite, xs, ys, draw_scanline_16, FALSE);
				break;
#endif

#ifdef ALLEGRO_COLOR24
			case 24:
				_parallelogram_map(bmp, sprite, xs, ys, draw_scanline_24, FALSE);
				break;
#endif

#ifdef ALLEGRO_COLOR32
			case 32:
				_parallelogram_map(bmp, sprite, xs, ys, draw_scanline_32, FALSE);
				break;
#endif

			default:
				ASSERT(0); /* NOTREACHED */
		}
	}
}

/* _rotate_scale_flip_coordinates:
 *  Calculates the coordinates for the rotated, scaled and flipped sprite,
 *  and passes them on to the given function.
 */
void _rotate_scale_flip_coordinates(fixed w, fixed h,
		fixed x, fixed y, fixed cx, fixed cy,
		fixed angle,
		fixed scale_x, fixed scale_y,
		int h_flip, int v_flip,
		fixed xs[4], fixed ys[4]) {
	fixed fix_cos, fix_sin;
	int tl = 0, tr = 1, bl = 3, br = 2;
	int tmp;
	double cos_angle, sin_angle;
	fixed xofs, yofs;

	/* Setting angle to the range -180...180 degrees makes sin & cos
	   more numerically stable. (Yes, this does have an effect for big
	   angles!) Note that using "real" sin() and cos() gives much better
	   precision than fixsin() and fixcos(). */
	angle = angle & 0xffffff;
	if (angle >= 0x800000)
		angle -= 0x1000000;

	_AL_SINCOS(angle * (AL_PI / (double)0x800000), sin_angle, cos_angle);

	if (cos_angle >= 0)
		fix_cos = (int)(cos_angle * 0x10000 + 0.5);
	else
		fix_cos = (int)(cos_angle * 0x10000 - 0.5);
	if (sin_angle >= 0)
		fix_sin = (int)(sin_angle * 0x10000 + 0.5);
	else
		fix_sin = (int)(sin_angle * 0x10000 - 0.5);

	/* Decide what order to take corners in. */
	if (v_flip) {
		tl = 3;
		tr = 2;
		bl = 0;
		br = 1;
	} else {
		tl = 0;
		tr = 1;
		bl = 3;
		br = 2;
	}
	if (h_flip) {
		tmp = tl;
		tl = tr;
		tr = tmp;
		tmp = bl;
		bl = br;
		br = tmp;
	}

	/* Calculate new coordinates of all corners. */
	w = fixmul(w, scale_x);
	h = fixmul(h, scale_y);
	cx = fixmul(cx, scale_x);
	cy = fixmul(cy, scale_y);

	xofs = x - fixmul(cx, fix_cos) + fixmul(cy, fix_sin);

	yofs = y - fixmul(cx, fix_sin) - fixmul(cy, fix_cos);

	xs[tl] = xofs;
	ys[tl] = yofs;
	xs[tr] = xofs + fixmul(w, fix_cos);
	ys[tr] = yofs + fixmul(w, fix_sin);
	xs[bl] = xofs - fixmul(h, fix_sin);
	ys[bl] = yofs + fixmul(h, fix_cos);

	xs[br] = xs[tr] + xs[bl] - xs[tl];
	ys[br] = ys[tr] + ys[bl] - ys[tl];
}

/* _pivot_scaled_sprite_flip:
 *  The most generic routine to which you specify the position with angles,
 *  scales, etc.
 */
void _pivot_scaled_sprite_flip(BITMAP *bmp, BITMAP *sprite,
		fixed x, fixed y, fixed cx, fixed cy,
		fixed angle, fixed scale, int v_flip) {
	fixed xs[4], ys[4];

	_rotate_scale_flip_coordinates(sprite->w << 16, sprite->h << 16,
			x, y, cx, cy, angle, scale, scale,
			FALSE, v_flip, xs, ys);

	_parallelogram_map_standard(bmp, sprite, xs, ys);
}

/*
 * Monochrome character drawing routines.
 * ======================================
 */

/* helper macro for drawing glyphs in each color depth */
#define DRAW_GLYPH(bits, size)                          \
	{                                                   \
		AL_CONST unsigned char *data = glyph->dat;      \
		unsigned long addr;                             \
		int w = glyph->w;                               \
		int h = glyph->h;                               \
		int stride = (w + 7) / 8;                       \
		int lgap = 0;                                   \
		int d, i, j;                                    \
                                                        \
		if (bmp->clip) {                                \
			/* clip the top */                          \
			if (y < bmp->ct) {                          \
				d = bmp->ct - y;                        \
                                                        \
				h -= d;                                 \
				if (h <= 0)                             \
					return;                             \
                                                        \
				data += d * stride;                     \
				y = bmp->ct;                            \
			}                                           \
                                                        \
			/* clip the bottom */                       \
			if (y + h >= bmp->cb) {                     \
				h = bmp->cb - y;                        \
				if (h <= 0)                             \
					return;                             \
			}                                           \
                                                        \
			/* clip the left */                         \
			if (x < bmp->cl) {                          \
				d = bmp->cl - x;                        \
                                                        \
				w -= d;                                 \
				if (w <= 0)                             \
					return;                             \
                                                        \
				data += d / 8;                          \
				lgap = d & 7;                           \
				x = bmp->cl;                            \
			}                                           \
                                                        \
			/* clip the right */                        \
			if (x + w >= bmp->cr) {                     \
				w = bmp->cr - x;                        \
				if (w <= 0)                             \
					return;                             \
			}                                           \
		}                                               \
                                                        \
		stride -= (lgap + w + 7) / 8;                   \
                                                        \
		/* draw it */                                   \
                                                        \
		while (h--) {                                   \
			addr = bmp_write_line(bmp, y++) + x * size; \
                                                        \
			j = 0;                                      \
			i = 0x80 >> lgap;                           \
			d = *(data++);                              \
                                                        \
			if (bg >= 0) {                              \
				/* opaque mode drawing loop */          \
				for (;;) {                              \
					if (d & i)                          \
						bmp_write##bits(addr, color);   \
					else                                \
						bmp_write##bits(addr, bg);      \
                                                        \
					j++;                                \
					if (j == w)                         \
						break;                          \
                                                        \
					i >>= 1;                            \
					if (!i) {                           \
						i = 0x80;                       \
						d = *(data++);                  \
					}                                   \
                                                        \
					addr += size;                       \
				}                                       \
			} else {                                    \
				/* masked mode drawing loop */          \
				for (;;) {                              \
					if (d & i)                          \
						bmp_write##bits(addr, color);   \
                                                        \
					j++;                                \
					if (j == w)                         \
						break;                          \
                                                        \
					i >>= 1;                            \
					if (!i) {                           \
						i = 0x80;                       \
						d = *(data++);                  \
					}                                   \
                                                        \
					addr += size;                       \
				}                                       \
			}                                           \
			data += stride;                             \
		}                                               \
	}

#ifdef ALLEGRO_COLOR8
/* _linear_draw_glyph8:
 *  Draws a glyph onto an 8 bit bitmap.
 */
void _linear_draw_glyph8(BITMAP *bmp, AL_CONST FONT_GLYPH *glyph, int x, int y, int color, int bg) { DRAW_GLYPH(8, 1); }
#endif

#ifdef ALLEGRO_COLOR16
/* _linear_draw_glyph16:
 *  Draws a glyph onto a 16 bit bitmap.
 */
void _linear_draw_glyph16(BITMAP *bmp, AL_CONST FONT_GLYPH *glyph, int x, int y, int color, int bg) {
	DRAW_GLYPH(16, sizeof(int16_t));
}
#endif

#ifdef ALLEGRO_COLOR24
/* _linear_draw_glyph24:
 *  Draws a glyph onto a 24 bit bitmap.
 */
void _linear_draw_glyph24(BITMAP *bmp, AL_CONST FONT_GLYPH *glyph, int x, int y, int color, int bg) { DRAW_GLYPH(24, 3); }
#endif

#ifdef ALLEGRO_COLOR32
/* _linear_draw_glyph32:
 *  Draws a glyph onto a 32 bit bitmap.
 */
void _linear_draw_glyph32(BITMAP *bmp, AL_CONST FONT_GLYPH *glyph, int x, int y, int color, int bg) { DRAW_GLYPH(32, sizeof(int32_t)); }
#endif

/*
 * Text drawing routines.
 * ======================
 */

/* textout_ex:
 *  Writes the null terminated string str onto bmp at position x, y, using
 *  the specified font, foreground color and background color (-1 is trans).
 *  If color is -1 and a proportional font is in use, it will be drawn
 *  using the colors from the original font bitmap (the one imported into
 *  the grabber program), which allows multicolored text output.
 */
void textout_ex(BITMAP *bmp, AL_CONST FONT *f, AL_CONST char *str, int x, int y, int color, int bg) {
	ASSERT(bmp);
	ASSERT(f);
	ASSERT(str);
	f->vtable->render(f, str, color, bg, bmp, x, y);
}

/* textout_centre_ex:
 *  Like textout_ex(), but uses the x coordinate as the centre rather than
 *  the left of the string.
 */
void textout_centre_ex(BITMAP *bmp, AL_CONST FONT *f, AL_CONST char *str, int x, int y, int color, int bg) {
	ASSERT(bmp);
	ASSERT(f);
	ASSERT(str);

	int len = text_length(f, str);
	f->vtable->render(f, str, color, bg, bmp, x - len / 2, y);
}

/* textout_right_ex:
 *  Like textout_ex(), but uses the x coordinate as the right rather than
 *  the left of the string.
 */
void textout_right_ex(BITMAP *bmp, AL_CONST FONT *f, AL_CONST char *str, int x, int y, int color, int bg) {
	ASSERT(bmp);
	ASSERT(f);
	ASSERT(str);

	int len = text_length(f, str);
	f->vtable->render(f, str, color, bg, bmp, x - len, y);
}

/* textout_justify_ex:
 *  Like textout_ex(), but justifies the string to the specified area.
 */
#define MAX_TOKEN 128

void textout_justify_ex(BITMAP *bmp, AL_CONST FONT *f, AL_CONST char *str, int x1, int x2, int y, int diff, int color, int bg) {
	char toks[32];
	char *tok[MAX_TOKEN];
	char *strbuf, *strlast;
	int i, minlen, last, space;
	float fleft, finc;
	ASSERT(bmp);
	ASSERT(f);
	ASSERT(str);

	i = usetc(toks, ' ');
	i += usetc(toks + i, '\t');
	i += usetc(toks + i, '\n');
	i += usetc(toks + i, '\r');
	usetc(toks + i, 0);

	/* count words and measure min length (without spaces) */
	strbuf = _al_ustrdup(str);
	if (!strbuf) {
		/* Can't justify ! */
		f->vtable->render(f, str, color, bg, bmp, x1, y);
		return;
	}

	minlen = 0;
	last = 0;
	tok[last] = ustrtok_r(strbuf, toks, &strlast);

	while (tok[last]) {
		minlen += text_length(f, tok[last]);
		if (++last == MAX_TOKEN)
			break;
		tok[last] = ustrtok_r(NULL, toks, &strlast);
	}

	/* amount of room for space between words */
	space = x2 - x1 - minlen;

	if ((space <= 0) || (space > diff) || (last < 2)) {
		/* can't justify */
		_AL_FREE(strbuf);
		f->vtable->render(f, str, color, bg, bmp, x1, y);
		return;
	}

	/* distribute space left evenly between words */
	fleft = (float)x1;
	finc = (float)space / (float)(last - 1);
	for (i = 0; i < last; i++) {
		f->vtable->render(f, tok[i], color, bg, bmp, (int)fleft, y);
		fleft += (float)text_length(f, tok[i]) + finc;
	}

	_AL_FREE(strbuf);
}

/* textprintf_ex:
 *  Formatted text output, using a printf() style format string.
 */
void textprintf_ex(BITMAP *bmp, AL_CONST FONT *f, int x, int y, int color, int bg, AL_CONST char *format, ...) {
	char buf[512];
	va_list ap;
	ASSERT(bmp);
	ASSERT(f);
	ASSERT(format);

	va_start(ap, format);
	uvszprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	textout_ex(bmp, f, buf, x, y, color, bg);
}

/* textprintf_centre_ex:
 *  Like textprintf_ex(), but uses the x coordinate as the centre rather than
 *  the left of the string.
 */
void textprintf_centre_ex(BITMAP *bmp, AL_CONST FONT *f, int x, int y, int color, int bg, AL_CONST char *format, ...) {
	char buf[512];
	va_list ap;
	ASSERT(bmp);
	ASSERT(f);
	ASSERT(format);

	va_start(ap, format);
	uvszprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	textout_centre_ex(bmp, f, buf, x, y, color, bg);
}

/* textprintf_right_ex:
 *  Like textprintf_ex(), but uses the x coordinate as the right rather than
 *  the left of the string.
 */
void textprintf_right_ex(BITMAP *bmp, AL_CONST FONT *f, int x, int y, int color, int bg, AL_CONST char *format, ...) {
	char buf[512];
	va_list ap;
	ASSERT(bmp);
	ASSERT(f);
	ASSERT(format);

	va_start(ap, format);
	uvszprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	textout_right_ex(bmp, f, buf, x, y, color, bg);
}

/* textprintf_justify_ex:
 *  Like textprintf_ex(), but right justifies the string to the specified area.
 */
void textprintf_justify_ex(BITMAP *bmp, AL_CONST FONT *f, int x1, int x2, int y, int diff, int color, int bg, AL_CONST char *format, ...) {
	char buf[512];
	va_list ap;
	ASSERT(bmp);
	ASSERT(f);
	ASSERT(format);

	va_start(ap, format);
	uvszprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	textout_justify_ex(bmp, f, buf, x1, x2, y, diff, color, bg);
}

/* text_length:
 *  Calculates the length of a string in a particular font.
 */
int text_length(AL_CONST FONT *f, AL_CONST char *str) {
	ASSERT(f);
	ASSERT(str);
	return f->vtable->text_length(f, str);
}

/* text_height:
 *  Returns the height of a character in the specified font.
 */
int text_height(AL_CONST FONT *f) {
	ASSERT(f);
	return f->vtable->font_height(f);
}

/* destroy_font:
 *  Frees the memory being used by a font structure.
 *  This is now wholly handled in the vtable.
 */
void destroy_font(FONT *f) {
	ASSERT(f);
	f->vtable->destroy(f);
}

/* The following code has been deprecated, as it relies on a global
 * variable. This is no good in multithreaded code, and using a bg
 * parameter also simplifies other code (for instance, the GUI).
 */

int _textmode = 0;

/* text_mode:
 *  Sets the mode in which text will be drawn. If mode is positive, text
 *  output will be opaque and the background will be set to mode. If mode
 *  is negative, text will be drawn transparently (ie. the background will
 *  not be altered). The default is a mode of zero.
 *  Returns previous mode.
 */
int text_mode(int mode) {
	int old_mode = _textmode;

	if (mode < 0)
		_textmode = -1;
	else
		_textmode = mode;

	return old_mode;
}

/* textout: (inlined)
 *  Writes the null terminated string str onto bmp at position x, y, using
 *  the current text mode and the specified font and foreground color.
 *  If color is -1 and a proportional font is in use, it will be drawn
 *  using the colors from the original font bitmap (the one imported into
 *  the grabber program), which allows multicolored text output.
 */

/* textout_centre: (inlined)
 *  Like textout(), but uses the x coordinate as the centre rather than
 *  the left of the string.
 */

/* textout_right: (inlined)
 *  Like textout(), but uses the x coordinate as the right rather than
 *  the left of the string.
 */

/* textout_justify: (inlined)
 *  Like textout(), but justifies the string to the specified area.
 */

/* textprintf:
 *  Formatted text output, using a printf() style format string.
 */
void textprintf(BITMAP *bmp, AL_CONST FONT *f, int x, int y, int color, AL_CONST char *format, ...) {
	char buf[512];
	va_list ap;
	ASSERT(bmp);
	ASSERT(f);
	ASSERT(format);

	va_start(ap, format);
	uvszprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	textout_ex(bmp, f, buf, x, y, color, _textmode);
}

/* textprintf_centre:
 *  Like textprintf(), but uses the x coordinate as the centre rather than
 *  the left of the string.
 */
void textprintf_centre(BITMAP *bmp, AL_CONST FONT *f, int x, int y, int color, AL_CONST char *format, ...) {
	char buf[512];
	va_list ap;
	ASSERT(bmp);
	ASSERT(f);
	ASSERT(format);

	va_start(ap, format);
	uvszprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	textout_centre_ex(bmp, f, buf, x, y, color, _textmode);
}

/* textprintf_right:
 *  Like textout(), but uses the x coordinate as the right rather than
 *  the left of the string.
 */
void textprintf_right(BITMAP *bmp, AL_CONST FONT *f, int x, int y, int color, AL_CONST char *format, ...) {
	char buf[512];
	va_list ap;
	ASSERT(bmp);
	ASSERT(f);
	ASSERT(format);

	va_start(ap, format);
	uvszprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	textout_right_ex(bmp, f, buf, x, y, color, _textmode);
}

/* textprintf_justify:
 *  Like textprintf(), but right justifies the string to the specified area.
 */
void textprintf_justify(BITMAP *bmp, AL_CONST FONT *f, int x1, int x2, int y, int diff, int color, AL_CONST char *format, ...) {
	char buf[512];
	va_list ap;
	ASSERT(bmp);
	ASSERT(f);
	ASSERT(format);

	va_start(ap, format);
	uvszprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	textout_justify_ex(bmp, f, buf, x1, x2, y, diff, color, _textmode);
}

/*
 * The default 8x8 font, and mono and color font vtables.
 *
 * Contains characters:
 *   ASCII          (0x0020 to 0x007F)
 *   Latin-1        (0x00A1 to 0x00FF)
 *   Extended-A     (0x0100 to 0x017F)
 *   Euro           (0x20AC)
 * ======================================================
 */

/* standard ASCII characters (0x20 to 0x7F) */
static FONT_GLYPH f_0x20 = { 8, 8, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
static FONT_GLYPH f_0x21 = { 8, 8, { 0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00 } };
static FONT_GLYPH f_0x22 = { 8, 8, { 0x6C, 0x6C, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00 } };
static FONT_GLYPH f_0x23 = { 8, 8, { 0x6C, 0x6C, 0xFE, 0x6C, 0xFE, 0x6C, 0x6C, 0x00 } };
static FONT_GLYPH f_0x24 = { 8, 8, { 0x18, 0x7E, 0xC0, 0x7C, 0x06, 0xFC, 0x18, 0x00 } };
static FONT_GLYPH f_0x25 = { 8, 8, { 0x00, 0xC6, 0xCC, 0x18, 0x30, 0x66, 0xC6, 0x00 } };
static FONT_GLYPH f_0x26 = { 8, 8, { 0x38, 0x6C, 0x38, 0x76, 0xDC, 0xCC, 0x76, 0x00 } };
static FONT_GLYPH f_0x27 = { 8, 8, { 0x30, 0x30, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00 } };
static FONT_GLYPH f_0x28 = { 8, 8, { 0x18, 0x30, 0x60, 0x60, 0x60, 0x30, 0x18, 0x00 } };
static FONT_GLYPH f_0x29 = { 8, 8, { 0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60, 0x00 } };
static FONT_GLYPH f_0x2A = { 8, 8, { 0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00 } };
static FONT_GLYPH f_0x2B = { 8, 8, { 0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x00 } };
static FONT_GLYPH f_0x2C = { 8, 8, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30 } };
static FONT_GLYPH f_0x2D = { 8, 8, { 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00 } };
static FONT_GLYPH f_0x2E = { 8, 8, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00 } };
static FONT_GLYPH f_0x2F = { 8, 8, { 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x00 } };
static FONT_GLYPH f_0x30 = { 8, 8, { 0x7C, 0xCE, 0xDE, 0xF6, 0xE6, 0xC6, 0x7C, 0x00 } };
static FONT_GLYPH f_0x31 = { 8, 8, { 0x30, 0x70, 0x30, 0x30, 0x30, 0x30, 0xFC, 0x00 } };
static FONT_GLYPH f_0x32 = { 8, 8, { 0x78, 0xCC, 0x0C, 0x38, 0x60, 0xCC, 0xFC, 0x00 } };
static FONT_GLYPH f_0x33 = { 8, 8, { 0x78, 0xCC, 0x0C, 0x38, 0x0C, 0xCC, 0x78, 0x00 } };
static FONT_GLYPH f_0x34 = { 8, 8, { 0x1C, 0x3C, 0x6C, 0xCC, 0xFE, 0x0C, 0x1E, 0x00 } };
static FONT_GLYPH f_0x35 = { 8, 8, { 0xFC, 0xC0, 0xF8, 0x0C, 0x0C, 0xCC, 0x78, 0x00 } };
static FONT_GLYPH f_0x36 = { 8, 8, { 0x38, 0x60, 0xC0, 0xF8, 0xCC, 0xCC, 0x78, 0x00 } };
static FONT_GLYPH f_0x37 = { 8, 8, { 0xFC, 0xCC, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00 } };
static FONT_GLYPH f_0x38 = { 8, 8, { 0x78, 0xCC, 0xCC, 0x78, 0xCC, 0xCC, 0x78, 0x00 } };
static FONT_GLYPH f_0x39 = { 8, 8, { 0x78, 0xCC, 0xCC, 0x7C, 0x0C, 0x18, 0x70, 0x00 } };
static FONT_GLYPH f_0x3A = { 8, 8, { 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00 } };
static FONT_GLYPH f_0x3B = { 8, 8, { 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x30 } };
static FONT_GLYPH f_0x3C = { 8, 8, { 0x18, 0x30, 0x60, 0xC0, 0x60, 0x30, 0x18, 0x00 } };
static FONT_GLYPH f_0x3D = { 8, 8, { 0x00, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00 } };
static FONT_GLYPH f_0x3E = { 8, 8, { 0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60, 0x00 } };
static FONT_GLYPH f_0x3F = { 8, 8, { 0x3C, 0x66, 0x0C, 0x18, 0x18, 0x00, 0x18, 0x00 } };
static FONT_GLYPH f_0x40 = { 8, 8, { 0x7C, 0xC6, 0xDE, 0xDE, 0xDC, 0xC0, 0x7C, 0x00 } };
static FONT_GLYPH f_0x41 = { 8, 8, { 0x30, 0x78, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0x00 } };
static FONT_GLYPH f_0x42 = { 8, 8, { 0xFC, 0x66, 0x66, 0x7C, 0x66, 0x66, 0xFC, 0x00 } };
static FONT_GLYPH f_0x43 = { 8, 8, { 0x3C, 0x66, 0xC0, 0xC0, 0xC0, 0x66, 0x3C, 0x00 } };
static FONT_GLYPH f_0x44 = { 8, 8, { 0xF8, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0xF8, 0x00 } };
static FONT_GLYPH f_0x45 = { 8, 8, { 0xFE, 0x62, 0x68, 0x78, 0x68, 0x62, 0xFE, 0x00 } };
static FONT_GLYPH f_0x46 = { 8, 8, { 0xFE, 0x62, 0x68, 0x78, 0x68, 0x60, 0xF0, 0x00 } };
static FONT_GLYPH f_0x47 = { 8, 8, { 0x3C, 0x66, 0xC0, 0xC0, 0xCE, 0x66, 0x3A, 0x00 } };
static FONT_GLYPH f_0x48 = { 8, 8, { 0xCC, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0xCC, 0x00 } };
static FONT_GLYPH f_0x49 = { 8, 8, { 0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00 } };
static FONT_GLYPH f_0x4A = { 8, 8, { 0x1E, 0x0C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78, 0x00 } };
static FONT_GLYPH f_0x4B = { 8, 8, { 0xE6, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0xE6, 0x00 } };
static FONT_GLYPH f_0x4C = { 8, 8, { 0xF0, 0x60, 0x60, 0x60, 0x62, 0x66, 0xFE, 0x00 } };
static FONT_GLYPH f_0x4D = { 8, 8, { 0xC6, 0xEE, 0xFE, 0xFE, 0xD6, 0xC6, 0xC6, 0x00 } };
static FONT_GLYPH f_0x4E = { 8, 8, { 0xC6, 0xE6, 0xF6, 0xDE, 0xCE, 0xC6, 0xC6, 0x00 } };
static FONT_GLYPH f_0x4F = { 8, 8, { 0x38, 0x6C, 0xC6, 0xC6, 0xC6, 0x6C, 0x38, 0x00 } };
static FONT_GLYPH f_0x50 = { 8, 8, { 0xFC, 0x66, 0x66, 0x7C, 0x60, 0x60, 0xF0, 0x00 } };
static FONT_GLYPH f_0x51 = { 8, 8, { 0x7C, 0xC6, 0xC6, 0xC6, 0xD6, 0x7C, 0x0E, 0x00 } };
static FONT_GLYPH f_0x52 = { 8, 8, { 0xFC, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0xE6, 0x00 } };
static FONT_GLYPH f_0x53 = { 8, 8, { 0x7C, 0xC6, 0xE0, 0x78, 0x0E, 0xC6, 0x7C, 0x00 } };
static FONT_GLYPH f_0x54 = { 8, 8, { 0xFC, 0xB4, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00 } };
static FONT_GLYPH f_0x55 = { 8, 8, { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xFC, 0x00 } };
static FONT_GLYPH f_0x56 = { 8, 8, { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x00 } };
static FONT_GLYPH f_0x57 = { 8, 8, { 0xC6, 0xC6, 0xC6, 0xC6, 0xD6, 0xFE, 0x6C, 0x00 } };
static FONT_GLYPH f_0x58 = { 8, 8, { 0xC6, 0xC6, 0x6C, 0x38, 0x6C, 0xC6, 0xC6, 0x00 } };
static FONT_GLYPH f_0x59 = { 8, 8, { 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x30, 0x78, 0x00 } };
static FONT_GLYPH f_0x5A = { 8, 8, { 0xFE, 0xC6, 0x8C, 0x18, 0x32, 0x66, 0xFE, 0x00 } };
static FONT_GLYPH f_0x5B = { 8, 8, { 0x78, 0x60, 0x60, 0x60, 0x60, 0x60, 0x78, 0x00 } };
static FONT_GLYPH f_0x5C = { 8, 8, { 0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x02, 0x00 } };
static FONT_GLYPH f_0x5D = { 8, 8, { 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78, 0x00 } };
static FONT_GLYPH f_0x5E = { 8, 8, { 0x10, 0x38, 0x6C, 0xC6, 0x00, 0x00, 0x00, 0x00 } };
static FONT_GLYPH f_0x5F = { 8, 8, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF } };
static FONT_GLYPH f_0x60 = { 8, 8, { 0x30, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00 } };
static FONT_GLYPH f_0x61 = { 8, 8, { 0x00, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x76, 0x00 } };
static FONT_GLYPH f_0x62 = { 8, 8, { 0xE0, 0x60, 0x60, 0x7C, 0x66, 0x66, 0xDC, 0x00 } };
static FONT_GLYPH f_0x63 = { 8, 8, { 0x00, 0x00, 0x78, 0xCC, 0xC0, 0xCC, 0x78, 0x00 } };
static FONT_GLYPH f_0x64 = { 8, 8, { 0x1C, 0x0C, 0x0C, 0x7C, 0xCC, 0xCC, 0x76, 0x00 } };
static FONT_GLYPH f_0x65 = { 8, 8, { 0x00, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00 } };
static FONT_GLYPH f_0x66 = { 8, 8, { 0x38, 0x6C, 0x64, 0xF0, 0x60, 0x60, 0xF0, 0x00 } };
static FONT_GLYPH f_0x67 = { 8, 8, { 0x00, 0x00, 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8 } };
static FONT_GLYPH f_0x68 = { 8, 8, { 0xE0, 0x60, 0x6C, 0x76, 0x66, 0x66, 0xE6, 0x00 } };
static FONT_GLYPH f_0x69 = { 8, 8, { 0x30, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00 } };
static FONT_GLYPH f_0x6A = { 8, 8, { 0x0C, 0x00, 0x1C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78 } };
static FONT_GLYPH f_0x6B = { 8, 8, { 0xE0, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0xE6, 0x00 } };
static FONT_GLYPH f_0x6C = { 8, 8, { 0x70, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00 } };
static FONT_GLYPH f_0x6D = { 8, 8, { 0x00, 0x00, 0xCC, 0xFE, 0xFE, 0xD6, 0xD6, 0x00 } };
static FONT_GLYPH f_0x6E = { 8, 8, { 0x00, 0x00, 0xB8, 0xCC, 0xCC, 0xCC, 0xCC, 0x00 } };
static FONT_GLYPH f_0x6F = { 8, 8, { 0x00, 0x00, 0x78, 0xCC, 0xCC, 0xCC, 0x78, 0x00 } };
static FONT_GLYPH f_0x70 = { 8, 8, { 0x00, 0x00, 0xDC, 0x66, 0x66, 0x7C, 0x60, 0xF0 } };
static FONT_GLYPH f_0x71 = { 8, 8, { 0x00, 0x00, 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0x1E } };
static FONT_GLYPH f_0x72 = { 8, 8, { 0x00, 0x00, 0xDC, 0x76, 0x62, 0x60, 0xF0, 0x00 } };
static FONT_GLYPH f_0x73 = { 8, 8, { 0x00, 0x00, 0x7C, 0xC0, 0x70, 0x1C, 0xF8, 0x00 } };
static FONT_GLYPH f_0x74 = { 8, 8, { 0x10, 0x30, 0xFC, 0x30, 0x30, 0x34, 0x18, 0x00 } };
static FONT_GLYPH f_0x75 = { 8, 8, { 0x00, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0x76, 0x00 } };
static FONT_GLYPH f_0x76 = { 8, 8, { 0x00, 0x00, 0xCC, 0xCC, 0xCC, 0x78, 0x30, 0x00 } };
static FONT_GLYPH f_0x77 = { 8, 8, { 0x00, 0x00, 0xC6, 0xC6, 0xD6, 0xFE, 0x6C, 0x00 } };
static FONT_GLYPH f_0x78 = { 8, 8, { 0x00, 0x00, 0xC6, 0x6C, 0x38, 0x6C, 0xC6, 0x00 } };
static FONT_GLYPH f_0x79 = { 8, 8, { 0x00, 0x00, 0xCC, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8 } };
static FONT_GLYPH f_0x7A = { 8, 8, { 0x00, 0x00, 0xFC, 0x98, 0x30, 0x64, 0xFC, 0x00 } };
static FONT_GLYPH f_0x7B = { 8, 8, { 0x1C, 0x30, 0x30, 0xE0, 0x30, 0x30, 0x1C, 0x00 } };
static FONT_GLYPH f_0x7C = { 8, 8, { 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00 } };
static FONT_GLYPH f_0x7D = { 8, 8, { 0xE0, 0x30, 0x30, 0x1C, 0x30, 0x30, 0xE0, 0x00 } };
static FONT_GLYPH f_0x7E = { 8, 8, { 0x76, 0xDC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
static FONT_GLYPH f_0x7F = { 8, 8, { 0x00, 0x10, 0x38, 0x6C, 0xC6, 0xC6, 0xFE, 0x00 } };

/* list of ASCII characters */
static FONT_GLYPH *ascii_data[] = {
	&f_0x20, &f_0x21, &f_0x22, &f_0x23, &f_0x24, &f_0x25, &f_0x26, &f_0x27,
	&f_0x28, &f_0x29, &f_0x2A, &f_0x2B, &f_0x2C, &f_0x2D, &f_0x2E, &f_0x2F,
	&f_0x30, &f_0x31, &f_0x32, &f_0x33, &f_0x34, &f_0x35, &f_0x36, &f_0x37,
	&f_0x38, &f_0x39, &f_0x3A, &f_0x3B, &f_0x3C, &f_0x3D, &f_0x3E, &f_0x3F,
	&f_0x40, &f_0x41, &f_0x42, &f_0x43, &f_0x44, &f_0x45, &f_0x46, &f_0x47,
	&f_0x48, &f_0x49, &f_0x4A, &f_0x4B, &f_0x4C, &f_0x4D, &f_0x4E, &f_0x4F,
	&f_0x50, &f_0x51, &f_0x52, &f_0x53, &f_0x54, &f_0x55, &f_0x56, &f_0x57,
	&f_0x58, &f_0x59, &f_0x5A, &f_0x5B, &f_0x5C, &f_0x5D, &f_0x5E, &f_0x5F,
	&f_0x60, &f_0x61, &f_0x62, &f_0x63, &f_0x64, &f_0x65, &f_0x66, &f_0x67,
	&f_0x68, &f_0x69, &f_0x6A, &f_0x6B, &f_0x6C, &f_0x6D, &f_0x6E, &f_0x6F,
	&f_0x70, &f_0x71, &f_0x72, &f_0x73, &f_0x74, &f_0x75, &f_0x76, &f_0x77,
	&f_0x78, &f_0x79, &f_0x7A, &f_0x7B, &f_0x7C, &f_0x7D, &f_0x7E, &f_0x7F
};

/* ANSI Latin-1 characters (0xA1 to 0xFF) */
static FONT_GLYPH f_0xA1 = { 8, 8, { 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x18, 0x00 } };
static FONT_GLYPH f_0xA2 = { 8, 8, { 0x18, 0x18, 0x7E, 0xC0, 0xC0, 0x7E, 0x18, 0x18 } };
static FONT_GLYPH f_0xA3 = { 8, 8, { 0x38, 0x6C, 0x64, 0xF0, 0x60, 0xE6, 0xFC, 0x00 } };
static FONT_GLYPH f_0xA4 = { 8, 8, { 0x00, 0xC6, 0x7C, 0xC6, 0xC6, 0x7C, 0xC6, 0x00 } };
static FONT_GLYPH f_0xA5 = { 8, 8, { 0xCC, 0xCC, 0x78, 0xFC, 0x30, 0xFC, 0x30, 0x30 } };
static FONT_GLYPH f_0xA6 = { 8, 8, { 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00 } };
static FONT_GLYPH f_0xA7 = { 8, 8, { 0x3E, 0x61, 0x3C, 0x66, 0x66, 0x3C, 0x86, 0x7C } };
static FONT_GLYPH f_0xA8 = { 8, 8, { 0x00, 0xC6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
static FONT_GLYPH f_0xA9 = { 8, 8, { 0x7E, 0x81, 0x9D, 0xA1, 0xA1, 0x9D, 0x81, 0x7E } };
static FONT_GLYPH f_0xAA = { 8, 8, { 0x3C, 0x6C, 0x6C, 0x3E, 0x00, 0x7E, 0x00, 0x00 } };
static FONT_GLYPH f_0xAB = { 8, 8, { 0x00, 0x33, 0x66, 0xCC, 0x66, 0x33, 0x00, 0x00 } };
static FONT_GLYPH f_0xAC = { 8, 8, { 0x00, 0x00, 0x00, 0xFC, 0x0C, 0x0C, 0x00, 0x00 } };
static FONT_GLYPH f_0xAD = { 8, 8, { 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00 } };
static FONT_GLYPH f_0xAE = { 8, 8, { 0x7E, 0x81, 0xB9, 0xA5, 0xB9, 0xA5, 0x81, 0x7E } };
static FONT_GLYPH f_0xAF = { 8, 8, { 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
static FONT_GLYPH f_0xB0 = { 8, 8, { 0x38, 0x6C, 0x6C, 0x38, 0x00, 0x00, 0x00, 0x00 } };
static FONT_GLYPH f_0xB1 = { 8, 8, { 0x30, 0x30, 0xFC, 0x30, 0x30, 0x00, 0xFC, 0x00 } };
static FONT_GLYPH f_0xB2 = { 8, 8, { 0x70, 0x18, 0x30, 0x60, 0x78, 0x00, 0x00, 0x00 } };
static FONT_GLYPH f_0xB3 = { 8, 8, { 0x78, 0x0C, 0x38, 0x0C, 0x78, 0x00, 0x00, 0x00 } };
static FONT_GLYPH f_0xB4 = { 8, 8, { 0x0C, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00 } };
static FONT_GLYPH f_0xB5 = { 8, 8, { 0x00, 0x00, 0x33, 0x33, 0x66, 0x7E, 0xC0, 0x80 } };
static FONT_GLYPH f_0xB6 = { 8, 8, { 0x7F, 0xDB, 0xDB, 0x7B, 0x1B, 0x1B, 0x1B, 0x00 } };
static FONT_GLYPH f_0xB7 = { 8, 8, { 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00 } };
static FONT_GLYPH f_0xB8 = { 8, 8, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x0C, 0x38 } };
static FONT_GLYPH f_0xB9 = { 8, 8, { 0x18, 0x38, 0x18, 0x18, 0x3C, 0x00, 0x00, 0x00 } };
static FONT_GLYPH f_0xBA = { 8, 8, { 0x38, 0x6C, 0x6C, 0x38, 0x00, 0x7C, 0x00, 0x00 } };
static FONT_GLYPH f_0xBB = { 8, 8, { 0x00, 0xCC, 0x66, 0x33, 0x66, 0xCC, 0x00, 0x00 } };
static FONT_GLYPH f_0xBC = { 8, 8, { 0xC3, 0xC6, 0xCC, 0xDB, 0x37, 0x6F, 0xCF, 0x03 } };
static FONT_GLYPH f_0xBD = { 8, 8, { 0xC3, 0xC6, 0xCC, 0xDE, 0x33, 0x66, 0xCC, 0x0F } };
static FONT_GLYPH f_0xBE = { 8, 8, { 0xE1, 0x32, 0xE4, 0x3A, 0xF6, 0x2A, 0x5F, 0x86 } };
static FONT_GLYPH f_0xBF = { 8, 8, { 0x30, 0x00, 0x30, 0x60, 0xC0, 0xCC, 0x78, 0x00 } };
static FONT_GLYPH f_0xC0 = { 8, 8, { 0x18, 0x0C, 0x38, 0x6C, 0xC6, 0xFE, 0xC6, 0x00 } };
static FONT_GLYPH f_0xC1 = { 8, 8, { 0x30, 0x60, 0x38, 0x6C, 0xC6, 0xFE, 0xC6, 0x00 } };
static FONT_GLYPH f_0xC2 = { 8, 8, { 0x7C, 0x82, 0x38, 0x6C, 0xC6, 0xFE, 0xC6, 0x00 } };
static FONT_GLYPH f_0xC3 = { 8, 8, { 0x76, 0xDC, 0x38, 0x6C, 0xC6, 0xFE, 0xC6, 0x00 } };
static FONT_GLYPH f_0xC4 = { 8, 8, { 0xC6, 0x00, 0x38, 0x6C, 0xC6, 0xFE, 0xC6, 0x00 } };
static FONT_GLYPH f_0xC5 = { 8, 8, { 0x10, 0x28, 0x38, 0x6C, 0xC6, 0xFE, 0xC6, 0x00 } };
static FONT_GLYPH f_0xC6 = { 8, 8, { 0x3E, 0x6C, 0xCC, 0xFE, 0xCC, 0xCC, 0xCE, 0x00 } };
static FONT_GLYPH f_0xC7 = { 8, 8, { 0x78, 0xCC, 0xC0, 0xCC, 0x78, 0x18, 0x0C, 0x78 } };
static FONT_GLYPH f_0xC8 = { 8, 8, { 0x30, 0x18, 0xFE, 0xC0, 0xFC, 0xC0, 0xFE, 0x00 } };
static FONT_GLYPH f_0xC9 = { 8, 8, { 0x0C, 0x18, 0xFE, 0xC0, 0xFC, 0xC0, 0xFE, 0x00 } };
static FONT_GLYPH f_0xCA = { 8, 8, { 0x7C, 0x82, 0xFE, 0xC0, 0xFC, 0xC0, 0xFE, 0x00 } };
static FONT_GLYPH f_0xCB = { 8, 8, { 0xC6, 0x00, 0xFE, 0xC0, 0xFC, 0xC0, 0xFE, 0x00 } };
static FONT_GLYPH f_0xCC = { 8, 8, { 0x30, 0x18, 0x3C, 0x18, 0x18, 0x18, 0x3C, 0x00 } };
static FONT_GLYPH f_0xCD = { 8, 8, { 0x0C, 0x18, 0x3C, 0x18, 0x18, 0x18, 0x3C, 0x00 } };
static FONT_GLYPH f_0xCE = { 8, 8, { 0x3C, 0x42, 0x3C, 0x18, 0x18, 0x18, 0x3C, 0x00 } };
static FONT_GLYPH f_0xCF = { 8, 8, { 0x66, 0x00, 0x3C, 0x18, 0x18, 0x18, 0x3C, 0x00 } };
static FONT_GLYPH f_0xD0 = { 8, 8, { 0xF8, 0x6C, 0x66, 0xF6, 0x66, 0x6C, 0xF8, 0x00 } };
static FONT_GLYPH f_0xD1 = { 8, 8, { 0xFC, 0x00, 0xCC, 0xEC, 0xFC, 0xDC, 0xCC, 0x00 } };
static FONT_GLYPH f_0xD2 = { 8, 8, { 0x30, 0x18, 0x7C, 0xC6, 0xC6, 0xC6, 0x7C, 0x00 } };
static FONT_GLYPH f_0xD3 = { 8, 8, { 0x18, 0x30, 0x7C, 0xC6, 0xC6, 0xC6, 0x7C, 0x00 } };
static FONT_GLYPH f_0xD4 = { 8, 8, { 0x7C, 0x82, 0x7C, 0xC6, 0xC6, 0xC6, 0x7C, 0x00 } };
static FONT_GLYPH f_0xD5 = { 8, 8, { 0x76, 0xDC, 0x7C, 0xC6, 0xC6, 0xC6, 0x7C, 0x00 } };
static FONT_GLYPH f_0xD6 = { 8, 8, { 0xC6, 0x00, 0x7C, 0xC6, 0xC6, 0xC6, 0x7C, 0x00 } };
static FONT_GLYPH f_0xD7 = { 8, 8, { 0x00, 0xC6, 0x6C, 0x38, 0x6C, 0xC6, 0x00, 0x00 } };
static FONT_GLYPH f_0xD8 = { 8, 8, { 0x3A, 0x6C, 0xCE, 0xD6, 0xE6, 0x6C, 0xB8, 0x00 } };
static FONT_GLYPH f_0xD9 = { 8, 8, { 0x60, 0x30, 0xC6, 0xC6, 0xC6, 0xC6, 0x7C, 0x00 } };
static FONT_GLYPH f_0xDA = { 8, 8, { 0x18, 0x30, 0xC6, 0xC6, 0xC6, 0xC6, 0x7C, 0x00 } };
static FONT_GLYPH f_0xDB = { 8, 8, { 0x7C, 0x82, 0x00, 0xC6, 0xC6, 0xC6, 0x7C, 0x00 } };
static FONT_GLYPH f_0xDC = { 8, 8, { 0xC6, 0x00, 0xC6, 0xC6, 0xC6, 0xC6, 0x7C, 0x00 } };
static FONT_GLYPH f_0xDD = { 8, 8, { 0x0C, 0x18, 0x66, 0x66, 0x3C, 0x18, 0x3C, 0x00 } };
static FONT_GLYPH f_0xDE = { 8, 8, { 0xE0, 0x60, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0xF0 } };
static FONT_GLYPH f_0xDF = { 8, 8, { 0x78, 0xCC, 0xCC, 0xD8, 0xCC, 0xC6, 0xCC, 0x00 } };
static FONT_GLYPH f_0xE0 = { 8, 8, { 0xE0, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x7E, 0x00 } };
static FONT_GLYPH f_0xE1 = { 8, 8, { 0x1C, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x7E, 0x00 } };
static FONT_GLYPH f_0xE2 = { 8, 8, { 0x7E, 0xC3, 0x3C, 0x06, 0x3E, 0x66, 0x3F, 0x00 } };
static FONT_GLYPH f_0xE3 = { 8, 8, { 0x76, 0xDC, 0x78, 0x0C, 0x7C, 0xCC, 0x7E, 0x00 } };
static FONT_GLYPH f_0xE4 = { 8, 8, { 0xCC, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x7E, 0x00 } };
static FONT_GLYPH f_0xE5 = { 8, 8, { 0x30, 0x30, 0x78, 0x0C, 0x7C, 0xCC, 0x7E, 0x00 } };
static FONT_GLYPH f_0xE6 = { 8, 8, { 0x00, 0x00, 0x7F, 0x0C, 0x7F, 0xCC, 0x7F, 0x00 } };
static FONT_GLYPH f_0xE7 = { 8, 8, { 0x00, 0x00, 0x78, 0xC0, 0xC0, 0x78, 0x0C, 0x38 } };
static FONT_GLYPH f_0xE8 = { 8, 8, { 0xE0, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00 } };
static FONT_GLYPH f_0xE9 = { 8, 8, { 0x1C, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00 } };
static FONT_GLYPH f_0xEA = { 8, 8, { 0x7E, 0xC3, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00 } };
static FONT_GLYPH f_0xEB = { 8, 8, { 0xCC, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00 } };
static FONT_GLYPH f_0xEC = { 8, 8, { 0xE0, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00 } };
static FONT_GLYPH f_0xED = { 8, 8, { 0x38, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00 } };
static FONT_GLYPH f_0xEE = { 8, 8, { 0x7C, 0xC6, 0x38, 0x18, 0x18, 0x18, 0x3C, 0x00 } };
static FONT_GLYPH f_0xEF = { 8, 8, { 0xCC, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00 } };
static FONT_GLYPH f_0xF0 = { 8, 8, { 0x08, 0x3C, 0x08, 0x7C, 0xCC, 0xCC, 0x78, 0x00 } };
static FONT_GLYPH f_0xF1 = { 8, 8, { 0x00, 0xF8, 0x00, 0xF8, 0xCC, 0xCC, 0xCC, 0x00 } };
static FONT_GLYPH f_0xF2 = { 8, 8, { 0x00, 0xE0, 0x00, 0x78, 0xCC, 0xCC, 0x78, 0x00 } };
static FONT_GLYPH f_0xF3 = { 8, 8, { 0x00, 0x1C, 0x00, 0x78, 0xCC, 0xCC, 0x78, 0x00 } };
static FONT_GLYPH f_0xF4 = { 8, 8, { 0x78, 0xCC, 0x00, 0x78, 0xCC, 0xCC, 0x78, 0x00 } };
static FONT_GLYPH f_0xF5 = { 8, 8, { 0x76, 0xDC, 0x00, 0x78, 0xCC, 0xCC, 0x78, 0x00 } };
static FONT_GLYPH f_0xF6 = { 8, 8, { 0x00, 0xCC, 0x00, 0x78, 0xCC, 0xCC, 0x78, 0x00 } };
static FONT_GLYPH f_0xF7 = { 8, 8, { 0x30, 0x30, 0x00, 0xFC, 0x00, 0x30, 0x30, 0x00 } };
static FONT_GLYPH f_0xF8 = { 8, 8, { 0x00, 0x02, 0x7C, 0xCE, 0xD6, 0xE6, 0x7C, 0x80 } };
static FONT_GLYPH f_0xF9 = { 8, 8, { 0x00, 0xE0, 0x00, 0xCC, 0xCC, 0xCC, 0x7E, 0x00 } };
static FONT_GLYPH f_0xFA = { 8, 8, { 0x00, 0x1C, 0x00, 0xCC, 0xCC, 0xCC, 0x7E, 0x00 } };
static FONT_GLYPH f_0xFB = { 8, 8, { 0x78, 0xCC, 0x00, 0xCC, 0xCC, 0xCC, 0x7E, 0x00 } };
static FONT_GLYPH f_0xFC = { 8, 8, { 0x00, 0xCC, 0x00, 0xCC, 0xCC, 0xCC, 0x7E, 0x00 } };
static FONT_GLYPH f_0xFD = { 8, 8, { 0x18, 0x30, 0xCC, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8 } };
static FONT_GLYPH f_0xFE = { 8, 8, { 0xF0, 0x60, 0x7C, 0x66, 0x7C, 0x60, 0xF0, 0x00 } };
static FONT_GLYPH f_0xFF = { 8, 8, { 0x00, 0xCC, 0x00, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8 } };

/* list of Latin-1 characters */
static FONT_GLYPH *latin1_data[] = {
	&f_0xA1, &f_0xA2, &f_0xA3, &f_0xA4, &f_0xA5, &f_0xA6, &f_0xA7,
	&f_0xA8, &f_0xA9, &f_0xAA, &f_0xAB, &f_0xAC, &f_0xAD, &f_0xAE, &f_0xAF,
	&f_0xB0, &f_0xB1, &f_0xB2, &f_0xB3, &f_0xB4, &f_0xB5, &f_0xB6, &f_0xB7,
	&f_0xB8, &f_0xB9, &f_0xBA, &f_0xBB, &f_0xBC, &f_0xBD, &f_0xBE, &f_0xBF,
	&f_0xC0, &f_0xC1, &f_0xC2, &f_0xC3, &f_0xC4, &f_0xC5, &f_0xC6, &f_0xC7,
	&f_0xC8, &f_0xC9, &f_0xCA, &f_0xCB, &f_0xCC, &f_0xCD, &f_0xCE, &f_0xCF,
	&f_0xD0, &f_0xD1, &f_0xD2, &f_0xD3, &f_0xD4, &f_0xD5, &f_0xD6, &f_0xD7,
	&f_0xD8, &f_0xD9, &f_0xDA, &f_0xDB, &f_0xDC, &f_0xDD, &f_0xDE, &f_0xDF,
	&f_0xE0, &f_0xE1, &f_0xE2, &f_0xE3, &f_0xE4, &f_0xE5, &f_0xE6, &f_0xE7,
	&f_0xE8, &f_0xE9, &f_0xEA, &f_0xEB, &f_0xEC, &f_0xED, &f_0xEE, &f_0xEF,
	&f_0xF0, &f_0xF1, &f_0xF2, &f_0xF3, &f_0xF4, &f_0xF5, &f_0xF6, &f_0xF7,
	&f_0xF8, &f_0xF9, &f_0xFA, &f_0xFB, &f_0xFC, &f_0xFD, &f_0xFE, &f_0xFF
};

/* Extended-A characters (0x100 to 0x17F) */
static FONT_GLYPH f_0x100 = { 8, 8, { 0xFE, 0x00, 0x38, 0x6C, 0xC6, 0xFE, 0xC6, 0x00 } };
static FONT_GLYPH f_0x101 = { 8, 8, { 0xFC, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x7E, 0x00 } };
static FONT_GLYPH f_0x102 = { 8, 8, { 0x82, 0x7C, 0x38, 0x6C, 0xC6, 0xFE, 0xC6, 0x00 } };
static FONT_GLYPH f_0x103 = { 8, 8, { 0xC3, 0x7E, 0x78, 0x0C, 0x7C, 0xCC, 0x7E, 0x00 } };
static FONT_GLYPH f_0x104 = { 8, 8, { 0x38, 0x6C, 0xC6, 0xFE, 0xC6, 0x1C, 0x30, 0x1E } };
static FONT_GLYPH f_0x105 = { 8, 8, { 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x7E, 0x30, 0x1C } };
static FONT_GLYPH f_0x106 = { 8, 8, { 0x0C, 0x18, 0x7C, 0xC6, 0xC0, 0xC6, 0x7C, 0x00 } };
static FONT_GLYPH f_0x107 = { 8, 8, { 0x1C, 0x00, 0x78, 0xCC, 0xC0, 0xCC, 0x78, 0x00 } };
static FONT_GLYPH f_0x108 = { 8, 8, { 0x7C, 0x82, 0x7C, 0xC6, 0xC0, 0xC6, 0x7C, 0x00 } };
static FONT_GLYPH f_0x109 = { 8, 8, { 0x7E, 0xC3, 0x78, 0xCC, 0xC0, 0xCC, 0x78, 0x00 } };
static FONT_GLYPH f_0x10A = { 8, 8, { 0x10, 0x00, 0x7C, 0xC6, 0xC0, 0xC6, 0x7C, 0x00 } };
static FONT_GLYPH f_0x10B = { 8, 8, { 0x10, 0x00, 0x78, 0xCC, 0xC0, 0xCC, 0x78, 0x00 } };
static FONT_GLYPH f_0x10C = { 8, 8, { 0x6C, 0x38, 0x7C, 0xC6, 0xC0, 0xC6, 0x7C, 0x00 } };
static FONT_GLYPH f_0x10D = { 8, 8, { 0x6C, 0x38, 0x78, 0xCC, 0xC0, 0xCC, 0x78, 0x00 } };
static FONT_GLYPH f_0x10E = { 8, 8, { 0x6C, 0x38, 0xF8, 0x66, 0x66, 0x66, 0xF8, 0x00 } };
static FONT_GLYPH f_0x10F = { 8, 8, { 0xBC, 0x4C, 0x0C, 0x7C, 0xCC, 0xCC, 0x76, 0x00 } };
static FONT_GLYPH f_0x110 = { 8, 8, { 0xF8, 0x6C, 0x66, 0xF6, 0x66, 0x6C, 0xF8, 0x00 } };
static FONT_GLYPH f_0x111 = { 8, 8, { 0x08, 0x3C, 0x08, 0x7C, 0xCC, 0xCC, 0x78, 0x00 } };
static FONT_GLYPH f_0x112 = { 8, 8, { 0xFE, 0x00, 0xFE, 0xC0, 0xFC, 0xC0, 0xFE, 0x00 } };
static FONT_GLYPH f_0x113 = { 8, 8, { 0xFC, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00 } };
static FONT_GLYPH f_0x114 = { 8, 8, { 0x6C, 0x38, 0xFE, 0xC0, 0xFC, 0xC0, 0xFE, 0x00 } };
static FONT_GLYPH f_0x115 = { 8, 8, { 0x6C, 0x38, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00 } };
static FONT_GLYPH f_0x116 = { 8, 8, { 0x10, 0x00, 0xFE, 0xC0, 0xFC, 0xC0, 0xFE, 0x00 } };
static FONT_GLYPH f_0x117 = { 8, 8, { 0x10, 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00 } };
static FONT_GLYPH f_0x118 = { 8, 8, { 0xFE, 0xC0, 0xFC, 0xC0, 0xFE, 0x18, 0x30, 0x1C } };
static FONT_GLYPH f_0x119 = { 8, 8, { 0x00, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x38, 0x0C } };
static FONT_GLYPH f_0x11A = { 8, 8, { 0x6C, 0x38, 0xFE, 0xC0, 0xFC, 0xC0, 0xFE, 0x00 } };
static FONT_GLYPH f_0x11B = { 8, 8, { 0x6C, 0x38, 0x78, 0xCC, 0xFC, 0xC0, 0x78, 0x00 } };
static FONT_GLYPH f_0x11C = { 8, 8, { 0x7C, 0x82, 0x7C, 0xC6, 0xC0, 0xCE, 0x7E, 0x00 } };
static FONT_GLYPH f_0x11D = { 8, 8, { 0x7E, 0xC3, 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8 } };
static FONT_GLYPH f_0x11E = { 8, 8, { 0x82, 0x7C, 0x7C, 0xC6, 0xC0, 0xCE, 0x7E, 0x00 } };
static FONT_GLYPH f_0x11F = { 8, 8, { 0xC3, 0x7E, 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8 } };
static FONT_GLYPH f_0x120 = { 8, 8, { 0x10, 0x00, 0x7C, 0xC6, 0xC0, 0xCE, 0x7E, 0x00 } };
static FONT_GLYPH f_0x121 = { 8, 8, { 0x10, 0x00, 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8 } };
static FONT_GLYPH f_0x122 = { 8, 8, { 0x7C, 0xC6, 0xC0, 0xCE, 0x7E, 0x18, 0x0C, 0x78 } };
static FONT_GLYPH f_0x123 = { 8, 8, { 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8, 0x0C, 0x38 } };
static FONT_GLYPH f_0x124 = { 8, 8, { 0x78, 0x84, 0xCC, 0xCC, 0xFC, 0xCC, 0xCC, 0x00 } };
static FONT_GLYPH f_0x125 = { 8, 8, { 0xEE, 0x7B, 0x6C, 0x76, 0x66, 0x66, 0xE6, 0x00 } };
static FONT_GLYPH f_0x126 = { 8, 8, { 0xCC, 0xFE, 0xCC, 0xFC, 0xCC, 0xCC, 0xCC, 0x00 } };
static FONT_GLYPH f_0x127 = { 8, 8, { 0xE0, 0xFE, 0x6C, 0x76, 0x66, 0x66, 0xE6, 0x00 } };
static FONT_GLYPH f_0x128 = { 8, 8, { 0x76, 0xDC, 0x78, 0x30, 0x30, 0x30, 0x78, 0x00 } };
static FONT_GLYPH f_0x129 = { 8, 8, { 0x76, 0xDC, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00 } };
static FONT_GLYPH f_0x12A = { 8, 8, { 0x78, 0x00, 0x78, 0x30, 0x30, 0x30, 0x78, 0x00 } };
static FONT_GLYPH f_0x12B = { 8, 8, { 0x78, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00 } };
static FONT_GLYPH f_0x12C = { 8, 8, { 0x84, 0x78, 0x78, 0x30, 0x30, 0x30, 0x78, 0x00 } };
static FONT_GLYPH f_0x12D = { 8, 8, { 0xC6, 0x7C, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00 } };
static FONT_GLYPH f_0x12E = { 8, 8, { 0x78, 0x30, 0x30, 0x30, 0x78, 0x18, 0x30, 0x1E } };
static FONT_GLYPH f_0x12F = { 8, 8, { 0x30, 0x00, 0x70, 0x30, 0x30, 0x78, 0x30, 0x1C } };
static FONT_GLYPH f_0x130 = { 8, 8, { 0x10, 0x00, 0x78, 0x30, 0x30, 0x30, 0x78, 0x00 } };
static FONT_GLYPH f_0x131 = { 8, 8, { 0x00, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00 } };
static FONT_GLYPH f_0x132 = { 8, 8, { 0xEE, 0x42, 0x42, 0x42, 0x52, 0x52, 0xEC, 0x00 } };
static FONT_GLYPH f_0x133 = { 8, 8, { 0x42, 0x00, 0xC6, 0x42, 0x42, 0x42, 0xE2, 0x0C } };
static FONT_GLYPH f_0x134 = { 8, 8, { 0x7C, 0x82, 0x0C, 0x0C, 0xCC, 0xCC, 0x78, 0x00 } };
static FONT_GLYPH f_0x135 = { 8, 8, { 0x7C, 0xC6, 0x1C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78 } };
static FONT_GLYPH f_0x136 = { 8, 8, { 0xE6, 0x6C, 0x78, 0x6C, 0xE6, 0x30, 0x18, 0xF0 } };
static FONT_GLYPH f_0x137 = { 8, 8, { 0xE0, 0x66, 0x6C, 0x78, 0x6C, 0xE6, 0x30, 0xE0 } };
static FONT_GLYPH f_0x138 = { 8, 8, { 0x00, 0x00, 0xE6, 0x6C, 0x78, 0x6C, 0xE6, 0x00 } };
static FONT_GLYPH f_0x139 = { 8, 8, { 0xF3, 0x66, 0x60, 0x60, 0x62, 0x66, 0xFE, 0x00 } };
static FONT_GLYPH f_0x13A = { 8, 8, { 0x73, 0x36, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00 } };
static FONT_GLYPH f_0x13B = { 8, 8, { 0xF0, 0x60, 0x62, 0x66, 0xFE, 0x18, 0x0C, 0x78 } };
static FONT_GLYPH f_0x13C = { 8, 8, { 0x70, 0x30, 0x30, 0x30, 0x30, 0x78, 0x0C, 0x38 } };
static FONT_GLYPH f_0x13D = { 8, 8, { 0xF5, 0x66, 0x60, 0x60, 0x62, 0x66, 0xFE, 0x00 } };
static FONT_GLYPH f_0x13E = { 8, 8, { 0x75, 0x36, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00 } };
static FONT_GLYPH f_0x13F = { 8, 8, { 0xF0, 0x60, 0x64, 0x60, 0x62, 0x66, 0xFE, 0x00 } };
static FONT_GLYPH f_0x140 = { 8, 8, { 0x70, 0x30, 0x30, 0x32, 0x30, 0x30, 0x78, 0x00 } };
static FONT_GLYPH f_0x141 = { 8, 8, { 0xF0, 0x60, 0x70, 0x60, 0xE2, 0x66, 0xFE, 0x00 } };
static FONT_GLYPH f_0x142 = { 8, 8, { 0x70, 0x30, 0x38, 0x30, 0x70, 0x30, 0x78, 0x00 } };
static FONT_GLYPH f_0x143 = { 8, 8, { 0x0C, 0x18, 0xCC, 0xEC, 0xFC, 0xDC, 0xCC, 0x00 } };
static FONT_GLYPH f_0x144 = { 8, 8, { 0x1C, 0x00, 0xB8, 0xCC, 0xCC, 0xCC, 0xCC, 0x00 } };
static FONT_GLYPH f_0x145 = { 8, 8, { 0xCC, 0xEC, 0xFC, 0xDC, 0xCC, 0x30, 0x18, 0xF0 } };
static FONT_GLYPH f_0x146 = { 8, 8, { 0x00, 0xB8, 0xCC, 0xCC, 0xCC, 0xCC, 0x30, 0xE0 } };
static FONT_GLYPH f_0x147 = { 8, 8, { 0x6C, 0x38, 0xCC, 0xEC, 0xFC, 0xDC, 0xCC, 0x00 } };
static FONT_GLYPH f_0x148 = { 8, 8, { 0x6C, 0x38, 0xB8, 0xCC, 0xCC, 0xCC, 0xCC, 0x00 } };
static FONT_GLYPH f_0x149 = { 8, 8, { 0xC0, 0x80, 0x5C, 0x66, 0x66, 0x66, 0x66, 0x00 } };
static FONT_GLYPH f_0x14A = { 8, 8, { 0x00, 0xCC, 0xEC, 0xFC, 0xDC, 0xCC, 0x0C, 0x38 } };
static FONT_GLYPH f_0x14B = { 8, 8, { 0x00, 0xB8, 0xCC, 0xCC, 0xCC, 0xCC, 0x0C, 0x38 } };
static FONT_GLYPH f_0x14C = { 8, 8, { 0xFE, 0x00, 0x7C, 0xC6, 0xC6, 0xC6, 0x7C, 0x00 } };
static FONT_GLYPH f_0x14D = { 8, 8, { 0x00, 0xFC, 0x00, 0x78, 0xCC, 0xCC, 0x78, 0x00 } };
static FONT_GLYPH f_0x14E = { 8, 8, { 0x6C, 0x38, 0x7C, 0xC6, 0xC6, 0xC6, 0x7C, 0x00 } };
static FONT_GLYPH f_0x14F = { 8, 8, { 0x6C, 0x38, 0x00, 0x78, 0xCC, 0xCC, 0x78, 0x00 } };
static FONT_GLYPH f_0x150 = { 8, 8, { 0x36, 0x6C, 0x7C, 0xC6, 0xC6, 0xC6, 0x7C, 0x00 } };
static FONT_GLYPH f_0x151 = { 8, 8, { 0x36, 0x6C, 0x00, 0x78, 0xCC, 0xCC, 0x78, 0x00 } };
static FONT_GLYPH f_0x152 = { 8, 8, { 0x7E, 0xDA, 0x88, 0x8C, 0x88, 0xDA, 0x7E, 0x00 } };
static FONT_GLYPH f_0x153 = { 8, 8, { 0x00, 0x00, 0x6C, 0x92, 0x9E, 0x90, 0x6C, 0x00 } };
static FONT_GLYPH f_0x154 = { 8, 8, { 0x0C, 0x18, 0xFC, 0x66, 0x7C, 0x6C, 0xE6, 0x00 } };
static FONT_GLYPH f_0x155 = { 8, 8, { 0x0C, 0x18, 0xDC, 0x76, 0x62, 0x60, 0xF0, 0x00 } };
static FONT_GLYPH f_0x156 = { 8, 8, { 0xFC, 0x66, 0x7C, 0x6C, 0xE6, 0x30, 0x18, 0xF0 } };
static FONT_GLYPH f_0x157 = { 8, 8, { 0x00, 0xDC, 0x76, 0x62, 0x60, 0xF0, 0x30, 0xE0 } };
static FONT_GLYPH f_0x158 = { 8, 8, { 0x6C, 0x38, 0xFC, 0x66, 0x7C, 0x6C, 0xE6, 0x00 } };
static FONT_GLYPH f_0x159 = { 8, 8, { 0x6C, 0x38, 0xDC, 0x76, 0x62, 0x60, 0xF0, 0x00 } };
static FONT_GLYPH f_0x15A = { 8, 8, { 0x0C, 0x18, 0x7C, 0xE0, 0x78, 0x0E, 0x7C, 0x00 } };
static FONT_GLYPH f_0x15B = { 8, 8, { 0x0C, 0x18, 0x7C, 0xC0, 0x70, 0x1C, 0xF8, 0x00 } };
static FONT_GLYPH f_0x15C = { 8, 8, { 0x7C, 0x82, 0x7C, 0xE0, 0x78, 0x0E, 0x7C, 0x00 } };
static FONT_GLYPH f_0x15D = { 8, 8, { 0x7C, 0xC6, 0x7C, 0xC0, 0x70, 0x1C, 0xF8, 0x00 } };
static FONT_GLYPH f_0x15E = { 8, 8, { 0x7C, 0xE0, 0x78, 0x0E, 0x7C, 0x18, 0x0C, 0x78 } };
static FONT_GLYPH f_0x15F = { 8, 8, { 0x00, 0x7C, 0xC0, 0x70, 0x1C, 0xF8, 0x0C, 0x38 } };
static FONT_GLYPH f_0x160 = { 8, 8, { 0x6C, 0x38, 0x7C, 0xE0, 0x78, 0x0E, 0x7C, 0x00 } };
static FONT_GLYPH f_0x161 = { 8, 8, { 0x6C, 0x38, 0x7C, 0xC0, 0x70, 0x1C, 0xF8, 0x00 } };
static FONT_GLYPH f_0x162 = { 8, 8, { 0xFC, 0x30, 0x30, 0x30, 0x78, 0x18, 0x0C, 0x38 } };
static FONT_GLYPH f_0x163 = { 8, 8, { 0x10, 0x30, 0xFC, 0x30, 0x34, 0x18, 0x0C, 0x38 } };
static FONT_GLYPH f_0x164 = { 8, 8, { 0x6C, 0x38, 0xFC, 0x30, 0x30, 0x30, 0x78, 0x00 } };
static FONT_GLYPH f_0x165 = { 8, 8, { 0x12, 0x3A, 0xFC, 0x30, 0x30, 0x34, 0x18, 0x00 } };
static FONT_GLYPH f_0x166 = { 8, 8, { 0xFC, 0xB4, 0x30, 0x30, 0xFC, 0x30, 0x78, 0x00 } };
static FONT_GLYPH f_0x167 = { 8, 8, { 0x10, 0x30, 0xFC, 0x30, 0xFC, 0x34, 0x18, 0x00 } };
static FONT_GLYPH f_0x168 = { 8, 8, { 0x76, 0xDC, 0xC6, 0xC6, 0xC6, 0xC6, 0x7C, 0x00 } };
static FONT_GLYPH f_0x169 = { 8, 8, { 0x76, 0xDC, 0x00, 0xCC, 0xCC, 0xCC, 0x7E, 0x00 } };
static FONT_GLYPH f_0x16A = { 8, 8, { 0xFE, 0x00, 0xC6, 0xC6, 0xC6, 0xC6, 0x7C, 0x00 } };
static FONT_GLYPH f_0x16B = { 8, 8, { 0x00, 0xFE, 0x00, 0xCC, 0xCC, 0xCC, 0x7E, 0x00 } };
static FONT_GLYPH f_0x16C = { 8, 8, { 0x6C, 0x38, 0xC6, 0xC6, 0xC6, 0xC6, 0x7C, 0x00 } };
static FONT_GLYPH f_0x16D = { 8, 8, { 0x6C, 0x38, 0x00, 0xCC, 0xCC, 0xCC, 0x7E, 0x00 } };
static FONT_GLYPH f_0x16E = { 8, 8, { 0x38, 0x6C, 0xFE, 0xD6, 0xC6, 0xC6, 0x7C, 0x00 } };
static FONT_GLYPH f_0x16F = { 8, 8, { 0x38, 0x6C, 0x38, 0xDC, 0xCC, 0xCC, 0x7E, 0x00 } };
static FONT_GLYPH f_0x170 = { 8, 8, { 0x36, 0x6C, 0xC6, 0xC6, 0xC6, 0xC6, 0x7C, 0x00 } };
static FONT_GLYPH f_0x171 = { 8, 8, { 0x36, 0x6C, 0x00, 0xCC, 0xCC, 0xCC, 0x7E, 0x00 } };
static FONT_GLYPH f_0x172 = { 8, 8, { 0xC6, 0xC6, 0xC6, 0xC6, 0x7C, 0x30, 0x60, 0x3C } };
static FONT_GLYPH f_0x173 = { 8, 8, { 0x00, 0x00, 0xCC, 0xCC, 0xCC, 0x7E, 0x18, 0x0E } };
static FONT_GLYPH f_0x174 = { 8, 8, { 0x7C, 0x82, 0xC6, 0xC6, 0xD6, 0xFE, 0x6C, 0x00 } };
static FONT_GLYPH f_0x175 = { 8, 8, { 0x7C, 0xC6, 0x00, 0xC6, 0xD6, 0xFE, 0x6C, 0x00 } };
static FONT_GLYPH f_0x176 = { 8, 8, { 0x7C, 0x82, 0xCC, 0xCC, 0x78, 0x30, 0x78, 0x00 } };
static FONT_GLYPH f_0x177 = { 8, 8, { 0x7C, 0xC6, 0xCC, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8 } };
static FONT_GLYPH f_0x178 = { 8, 8, { 0xCC, 0x00, 0xCC, 0xCC, 0x78, 0x30, 0x78, 0x00 } };
static FONT_GLYPH f_0x179 = { 8, 8, { 0x0C, 0x18, 0xFE, 0x8C, 0x18, 0x32, 0xFE, 0x00 } };
static FONT_GLYPH f_0x17A = { 8, 8, { 0x0C, 0x18, 0xFC, 0x98, 0x30, 0x64, 0xFC, 0x00 } };
static FONT_GLYPH f_0x17B = { 8, 8, { 0x10, 0x00, 0xFE, 0x8C, 0x18, 0x32, 0xFE, 0x00 } };
static FONT_GLYPH f_0x17C = { 8, 8, { 0x10, 0x00, 0xFC, 0x98, 0x30, 0x64, 0xFC, 0x00 } };
static FONT_GLYPH f_0x17D = { 8, 8, { 0x6C, 0x38, 0xFE, 0x8C, 0x18, 0x32, 0xFE, 0x00 } };
static FONT_GLYPH f_0x17E = { 8, 8, { 0x6C, 0x38, 0xFC, 0x98, 0x30, 0x64, 0xFC, 0x00 } };
static FONT_GLYPH f_0x17F = { 8, 8, { 0x38, 0x6C, 0x64, 0xE0, 0x60, 0x60, 0xE0, 0x00 } };

/* list of Extended-A characters */
static FONT_GLYPH *extended_a_data[] = {
	&f_0x100, &f_0x101, &f_0x102, &f_0x103, &f_0x104, &f_0x105, &f_0x106, &f_0x107,
	&f_0x108, &f_0x109, &f_0x10A, &f_0x10B, &f_0x10C, &f_0x10D, &f_0x10E, &f_0x10F,
	&f_0x110, &f_0x111, &f_0x112, &f_0x113, &f_0x114, &f_0x115, &f_0x116, &f_0x117,
	&f_0x118, &f_0x119, &f_0x11A, &f_0x11B, &f_0x11C, &f_0x11D, &f_0x11E, &f_0x11F,
	&f_0x120, &f_0x121, &f_0x122, &f_0x123, &f_0x124, &f_0x125, &f_0x126, &f_0x127,
	&f_0x128, &f_0x129, &f_0x12A, &f_0x12B, &f_0x12C, &f_0x12D, &f_0x12E, &f_0x12F,
	&f_0x130, &f_0x131, &f_0x132, &f_0x133, &f_0x134, &f_0x135, &f_0x136, &f_0x137,
	&f_0x138, &f_0x139, &f_0x13A, &f_0x13B, &f_0x13C, &f_0x13D, &f_0x13E, &f_0x13F,
	&f_0x140, &f_0x141, &f_0x142, &f_0x143, &f_0x144, &f_0x145, &f_0x146, &f_0x147,
	&f_0x148, &f_0x149, &f_0x14A, &f_0x14B, &f_0x14C, &f_0x14D, &f_0x14E, &f_0x14F,
	&f_0x150, &f_0x151, &f_0x152, &f_0x153, &f_0x154, &f_0x155, &f_0x156, &f_0x157,
	&f_0x158, &f_0x159, &f_0x15A, &f_0x15B, &f_0x15C, &f_0x15D, &f_0x15E, &f_0x15F,
	&f_0x160, &f_0x161, &f_0x162, &f_0x163, &f_0x164, &f_0x165, &f_0x166, &f_0x167,
	&f_0x168, &f_0x169, &f_0x16A, &f_0x16B, &f_0x16C, &f_0x16D, &f_0x16E, &f_0x16F,
	&f_0x170, &f_0x171, &f_0x172, &f_0x173, &f_0x174, &f_0x175, &f_0x176, &f_0x177,
	&f_0x178, &f_0x179, &f_0x17A, &f_0x17B, &f_0x17C, &f_0x17D, &f_0x17E, &f_0x17F
};

/* euro character (0x20AC) */
static FONT_GLYPH f_0x20AC = { 8, 8, { 0x3C, 0x62, 0xF8, 0x60, 0xF8, 0x62, 0x3C, 0x00 } };

/* euro character */
static FONT_GLYPH *euro_data[] = {
	&f_0x20AC
};

/* allegro_404_char:
 *  This is what we render missing glyphs as.
 */
int allegro_404_char = '^';

/* font_height:
 *  (mono and color vtable entry)
 *  Returns the height, in pixels of the font.
 */
static int font_height(AL_CONST FONT *f) {
	ASSERT(f);
	return f->height;
}

/* length:
 *  (mono and color vtable entry)
 *  Returns the length, in pixels, of a string as rendered in a font.
 */
static int length(AL_CONST FONT *f, AL_CONST char *text) {
	int ch = 0, w = 0;
	AL_CONST char *p = text;
	ASSERT(text);
	ASSERT(f);

	while ((ch = ugetxc(&p))) {
		w += f->vtable->char_length(f, ch);
	}

	return w;
}

/* _mono_find_glyph:
 *  Helper for mono vtable, below.
 */
FONT_GLYPH *_mono_find_glyph(AL_CONST FONT *f, int ch) {
	FONT_MONO_DATA *mf = (FONT_MONO_DATA *)(f->data);

	while (mf) {
		if (ch >= mf->begin && ch < mf->end)
			return mf->glyphs[ch - mf->begin];
		mf = mf->next;
	}

	/* if we don't find the character, then search for the missing
	   glyph, but don't get stuck in a loop. */
	if (ch != allegro_404_char)
		return _mono_find_glyph(f, allegro_404_char);
	return 0;
}

/* mono_char_length:
 *  (mono vtable entry)
 *  Returns the length, in pixels, of a character as rendered in a
 *  monochrome font.
 */
static int mono_char_length(AL_CONST FONT *f, int ch) {
	FONT_GLYPH *g = _mono_find_glyph(f, ch);
	return g ? g->w : 0;
}

/* mono_render_char:
 *  (mono vtable entry)
 *  Renders a character, in a monochrome font, onto a bitmap at a given
 *  location and in given colors. Returns the character width, in pixels.
 */
static int mono_render_char(AL_CONST FONT *f, int ch, int fg, int bg, BITMAP *bmp, int x, int y) {
	int w = 0;

	acquire_bitmap(bmp);

	FONT_GLYPH *g = _mono_find_glyph(f, ch);
	if (g) {
		bmp->vtable->draw_glyph(bmp, g, x, y + (f->height - g->h) / 2, fg, bg);
		w = g->w;
	}

	release_bitmap(bmp);

	return w;
}

/* mono_render:
 *  (mono vtable entry)
 *  Renders a string, in a monochrome font, onto a bitmap at a given
 *  location and in given colors.
 */
static void mono_render(AL_CONST FONT *f, AL_CONST char *text, int fg, int bg, BITMAP *bmp, int x, int y) {
	int ch = 0;
	AL_CONST char *p = text;

	acquire_bitmap(bmp);

	while ((ch = ugetxc(&p))) {
		x += f->vtable->render_char(f, ch, fg, bg, bmp, x, y);
	}

	release_bitmap(bmp);
}

/* mono_destroy:
 *  (mono vtable entry)
 *  Destroys a monochrome font.
 */
static void mono_destroy(FONT *f) {
	if (!f)
		return;

	FONT_MONO_DATA *mf = (FONT_MONO_DATA *)(f->data);
	while (mf) {
		FONT_MONO_DATA *next = mf->next;
		int i = 0;

		for (i = mf->begin; i < mf->end; i++)
			_AL_FREE(mf->glyphs[i - mf->begin]);

		_AL_FREE(mf->glyphs);
		_AL_FREE(mf);
		mf = next;
	}

	_AL_FREE(f);
}

/* mono_get_font_ranges:
 *  (mono vtable entry)
 *  Returns the number of character ranges in a font, or -1 if that information
 *   is not available.
 */
static int mono_get_font_ranges(FONT *f) {
	int ranges = 0;

	if (!f)
		return -1;

	FONT_MONO_DATA *mf = (FONT_MONO_DATA *)(f->data);

	while (mf) {
		FONT_MONO_DATA *next = mf->next;

		ranges++;
		if (!next)
			return ranges;
		mf = next;
	}

	return -1;
}

/* mono_get_font_range_begin:
 *  (mono vtable entry)
 *  Get first character for font range. Pass -1 to get the start of the font.
 */
static int mono_get_font_range_begin(FONT *f, int range) {
	if (!f || !f->data)
		return -1;

	if (range < 0)
		range = 0;
	int n = 0;

	FONT_MONO_DATA *mf = (FONT_MONO_DATA *)(f->data);
	while (mf && n <= range) {
		FONT_MONO_DATA *next = mf->next;

		if (!next || range == n)
			return mf->begin;
		mf = next;
		n++;
	}

	return -1;
}

/* mono_get_font_range_end:
 *  (mono vtable entry)
 *  Get last character for font range. Pass -1 to search the entire font.
 */
static int mono_get_font_range_end(FONT *f, int range) {
	if (!f)
		return -1;

	int n = 0;

	FONT_MONO_DATA *mf = (FONT_MONO_DATA *)(f->data);

	while (mf && (n <= range || range == -1)) {
		FONT_MONO_DATA *next = mf->next;
		if (!next || range == n)
			return mf->end - 1;
		mf = next;
		n++;
	}

	return -1;
}

/* mono_copy_glyph_range:
 *  Monochrome font helper function. Copies (part of) a glyph range
 */
static FONT_MONO_DATA *mono_copy_glyph_range(FONT_MONO_DATA *mf, int begin, int end) {
	if (begin < mf->begin || end > mf->end)
		return NULL;

	FONT_MONO_DATA *newmf = _AL_MALLOC(sizeof *newmf);

	if (!newmf)
		return NULL;

	newmf->begin = begin;
	newmf->end = end;
	newmf->next = NULL;
	int num = end - begin;

	FONT_GLYPH **gl = newmf->glyphs = _AL_MALLOC(num * sizeof *gl);
	for (int c = 0; c < num; c++) {
		FONT_GLYPH *g = mf->glyphs[begin - mf->begin + c];
		int sz = ((g->w + 7) / 8) * g->h;

		gl[c] = _AL_MALLOC(sz + sizeof *(gl[c]));
		gl[c]->w = g->w;
		gl[c]->h = g->h;
		memcpy(gl[c]->dat, g->dat, sz * sizeof *(g->dat));
	}

	return newmf;
}

/* mono_extract_font_range:
 *  (mono vtable entry)
 *  Extract a range of characters from a mono font
 */
static FONT *mono_extract_font_range(FONT *f, int begin, int end) {
	if (!f)
		return NULL;

	/* Special case: copy entire font */
	if (begin == -1 && end == -1) {
	}
	/* Copy from the beginning */
	else if (begin == -1 && end > mono_get_font_range_begin(f, -1)) {
	}
	/* Copy to the end */
	else if (end == -1 && begin <= mono_get_font_range_end(f, -1)) {
	}
	/* begin cannot be bigger than end */
	else if (begin <= end && begin != -1 && end != -1) {
	} else {
		return NULL;
	}

	/* Get output font */
	FONT *fontout = _AL_MALLOC(sizeof *fontout);

	fontout->height = f->height;
	fontout->vtable = f->vtable;
	fontout->data = NULL;

	/* Get real character ranges */
	int first = MAX(begin, mono_get_font_range_begin(f, -1));
	int last = (end > -1) ? MIN(end, mono_get_font_range_end(f, -1)) : mono_get_font_range_end(f, -1);
	last++;

	FONT_MONO_DATA *mf = NULL;
	FONT_MONO_DATA *mfin = f->data;
	while (mfin) {
		/* Find the range that is covered by the requested range. */
		/* Check if the requested and processed ranges at least overlap */
		if (((first >= mfin->begin && first < mfin->end) || (last <= mfin->end && last > mfin->begin))
				/* Check if the requested range wraps processed ranges */
				|| (first < mfin->begin && last > mfin->end)) {
			int local_begin, local_end;

			local_begin = MAX(mfin->begin, first);
			local_end = MIN(mfin->end, last);

			if (mf) {
				mf->next = mono_copy_glyph_range(mfin, local_begin, local_end);
				mf = mf->next;
			} else {
				mf = mono_copy_glyph_range(mfin, local_begin, local_end);
				fontout->data = mf;
			}
		}
		mfin = mfin->next;
	}

	return fontout;
}

/* mono_merge_fonts:
 *  (mono vtable entry)
 *  Merges font2 with font1 and returns a new font
 */
static FONT *mono_merge_fonts(FONT *font1, FONT *font2) {
	if (!font1 || !font2)
		return NULL;

	if (!is_mono_font(font1) || !is_mono_font(font2))
		return NULL;

	/* Get output font */
	FONT *fontout = _AL_MALLOC(sizeof *fontout);
	fontout->height = MAX(font1->height, font2->height);
	fontout->vtable = font1->vtable;
	FONT_MONO_DATA *mf = fontout->data = NULL;

	FONT_MONO_DATA *mf1 = font1->data;
	FONT_MONO_DATA *mf2 = font2->data;
	while (mf1 || mf2) {
		if (mf1 && (!mf2 || (mf1->begin < mf2->begin))) {
			if (mf) {
				mf->next = mono_copy_glyph_range(mf1, mf1->begin, mf1->end);
				mf = mf->next;
			} else {
				mf = mono_copy_glyph_range(mf1, mf1->begin, mf1->end);
				fontout->data = mf;
			}
			mf1 = mf1->next;
		} else {
			if (mf) {
				mf->next = mono_copy_glyph_range(mf2, mf2->begin, mf2->end);
				mf = mf->next;
			} else {
				mf = mono_copy_glyph_range(mf2, mf2->begin, mf2->end);
				fontout->data = mf;
			}
			mf2 = mf2->next;
		}
	}

	return fontout;
}

/* mono_transpose_font:
 *  (mono vtable entry)
 *  Transpose all glyphs in a font
 */
static int mono_transpose_font(FONT *f, int drange) {
	if (!f)
		return -1;

	FONT_MONO_DATA *mf = (FONT_MONO_DATA *)(f->data);

	while (mf) {
		FONT_MONO_DATA *next = mf->next;

		mf->begin += drange;
		mf->end += drange;
		mf = next;
	}

	return 0;
}

/* _color_find_glyph:
 *  Helper for color vtable entries, below.
 */
BITMAP *_color_find_glyph(AL_CONST FONT *f, int ch) {
	FONT_COLOR_DATA *cf = (FONT_COLOR_DATA *)(f->data);

	while (cf) {
		if (ch >= cf->begin && ch < cf->end)
			return cf->bitmaps[ch - cf->begin];
		cf = cf->next;
	}

	/* if we don't find the character, then search for the missing
	   glyph, but don't get stuck in a loop. */
	if (ch != allegro_404_char)
		return _color_find_glyph(f, allegro_404_char);
	return 0;
}

/* color_char_length:
 *  (color vtable entry)
 *  Returns the length of a character, in pixels, as it would be rendered
 *  in this font.
 */
static int color_char_length(AL_CONST FONT *f, int ch) {
	BITMAP *g = _color_find_glyph(f, ch);
	return g ? g->w : 0;
}

/* color_render_char:
 *  (color vtable entry)
 *  Renders a color character onto a bitmap, at the specified location, using
 *  the specified colors. If fg == -1, render as color, else render as
 *  mono; if bg == -1, render as transparent, else render as opaque.
 *  Returns the character width, in pixels.
 */
static int color_render_char(AL_CONST FONT *f, int ch, int fg, int bg, BITMAP *bmp, int x, int y) {
	int w = 0;
	int h = f->vtable->font_height(f);

	acquire_bitmap(bmp);

	if (fg < 0 && bg >= 0) {
		rectfill(bmp, x, y, x + f->vtable->char_length(f, ch) - 1, y + h - 1, bg);
	}

	BITMAP *g = _color_find_glyph(f, ch);
	if (g) {
		if (bitmap_color_depth(g) == 8) {
			if (fg < 0) {
				bmp->vtable->draw_256_sprite(bmp, g, x, y + (h - g->h) / 2);
			} else {
				bmp->vtable->draw_character(bmp, g, x, y + (h - g->h) / 2, fg, bg);
			}
		} else {
			if (bitmap_color_depth(g) == bitmap_color_depth(bmp)) {
				masked_blit(g, bmp, 0, 0, x, y + (h - g->h) / 2, g->w, g->h);
			} else {
				int color_conv_mode;
				BITMAP *tbmp;
				/* We need to do colour conversion - which is slow... */

				color_conv_mode = get_color_conversion();
				set_color_conversion(COLORCONV_MOST | COLORCONV_KEEP_TRANS);

				tbmp = create_bitmap_ex(bitmap_color_depth(bmp), g->w, g->h);
				blit(g, tbmp, 0, 0, 0, 0, g->w, g->h);

				set_color_conversion(color_conv_mode);

				masked_blit(tbmp, bmp, 0, 0, x, y + (h - g->h) / 2, g->w, g->h);

				destroy_bitmap(tbmp);
			}
		}

		w = g->w;
	}

	release_bitmap(bmp);

	return w;
}

/* trans_render_char:
 *  (trans vtable entry)
 *
 *  Renders a transparent character onto a bitmap, at the specified location,
 *  using the specified colors. fg is ignored. if bg == -1, render as
 *  transparent, else render as opaque. Returns the character width, in pixels.
 */
static int trans_render_char(AL_CONST FONT *f, int ch, int fg, int bg, BITMAP *bmp, int x, int y) {
	int w = 0;
	int h = f->vtable->font_height(f);

	acquire_bitmap(bmp);

	if (bg >= 0) {
		rectfill(bmp, x, y, x + f->vtable->char_length(f, ch) - 1, y + h - 1, bg);
	}

	BITMAP *g = _color_find_glyph(f, ch);
	if (g) {
		draw_trans_sprite(bmp, g, x, y + (h - g->h) / 2);
		w = g->w;
	}

	release_bitmap(bmp);

	return w;
}

/* color_render:
 *  (color vtable entry)
 *  Renders a color font onto a bitmap, at the specified location, using
 *  the specified colors. If fg == -1, render as color, else render as
 *  mono; if bg == -1, render as transparent, else render as opaque.
 */
static void color_render(AL_CONST FONT *f, AL_CONST char *text, int fg, int bg, BITMAP *bmp, int x, int y) {
	AL_CONST char *p = text;
	int ch = 0;

	acquire_bitmap(bmp);

	if (fg < 0 && bg >= 0) {
		rectfill(bmp, x, y, x + text_length(f, text) - 1, y + text_height(f) - 1, bg);
		bg = -1; /* to avoid filling rectangles for each character */
	}

	while ((ch = ugetxc(&p))) {
		x += f->vtable->render_char(f, ch, fg, bg, bmp, x, y);
	}

	release_bitmap(bmp);
}

/* color_destroy:
 *  (color vtable entry)
 *  Destroys a color font.
 */
static void color_destroy(FONT *f) {
	FONT_COLOR_DATA *cf;

	if (!f)
		return;

	cf = (FONT_COLOR_DATA *)(f->data);

	while (cf) {
		FONT_COLOR_DATA *next = cf->next;
		int i = 0;

		for (i = cf->begin; i < cf->end; i++)
			destroy_bitmap(cf->bitmaps[i - cf->begin]);

		_AL_FREE(cf->bitmaps);
		_AL_FREE(cf);

		cf = next;
	}

	_AL_FREE(f);
}

/* color_get_font_ranges:
 *  (color vtable entry)
 *  Returns the number of character ranges in a font, or -1 if that information
 *   is not available.
 */
static int color_get_font_ranges(FONT *f) {
	int ranges = 0;

	if (!f)
		return -1;

	FONT_COLOR_DATA *cf = (FONT_COLOR_DATA *)(f->data);

	while (cf) {
		FONT_COLOR_DATA *next = cf->next;

		ranges++;
		if (!next)
			return ranges;
		cf = next;
	}

	return -1;
}

/* color_get_font_range_begin:
 *  (color vtable entry)
 *  Get first character for font.
 */
static int color_get_font_range_begin(FONT *f, int range) {
	if (!f || !f->data)
		return -1;

	if (range < 0)
		range = 0;
	int n = 0;

	FONT_COLOR_DATA *cf = (FONT_COLOR_DATA *)(f->data);
	while (cf && n <= range) {
		FONT_COLOR_DATA *next = cf->next;

		if (!next || range == n)
			return cf->begin;
		cf = next;
		n++;
	}

	return -1;
}

/* color_get_font_range_end:
 *  (color vtable entry)
 *  Get last character for font range.
 */
static int color_get_font_range_end(FONT *f, int range) {
	if (!f)
		return -1;

	int n = 0;

	FONT_COLOR_DATA *cf = (FONT_COLOR_DATA *)(f->data);

	while (cf && (n <= range || range == -1)) {
		FONT_COLOR_DATA *next = cf->next;
		if (!next || range == n)
			return cf->end - 1;
		cf = next;
		n++;
	}

	return -1;
}

/* upgrade_to_color, upgrade_to_color_data:
 *  Helper functions. Upgrades a monochrome font to a color font.
 */
static FONT_COLOR_DATA *upgrade_to_color_data(FONT_MONO_DATA *mf) {
	FONT_COLOR_DATA *cf = _AL_MALLOC(sizeof *cf);
	BITMAP **bits = _AL_MALLOC((mf->end - mf->begin) * sizeof *bits);

	cf->begin = mf->begin;
	cf->end = mf->end;
	cf->bitmaps = bits;
	cf->next = 0;

	for (int i = mf->begin; i < mf->end; i++) {
		FONT_GLYPH *g = mf->glyphs[i - mf->begin];
		BITMAP *b = create_bitmap_ex(8, g->w, g->h);
		clear_to_color(b, 0);
		b->vtable->draw_glyph(b, g, 0, 0, 1, 0);

		bits[i - mf->begin] = b;
	}

	return cf;
}

static FONT *upgrade_to_color(FONT *f) {
	FONT_MONO_DATA *mf = f->data;
	FONT_COLOR_DATA *cf_write = 0;

	if (is_color_font(f))
		return NULL;
	FONT *outf = _AL_MALLOC(sizeof *outf);
	outf->vtable = font_vtable_color;
	outf->height = f->height;

	while (mf) {
		FONT_MONO_DATA *mf_next = mf->next;

		FONT_COLOR_DATA *cf = upgrade_to_color_data(mf);
		if (!cf_write)
			outf->data = cf;
		else
			cf_write->next = cf;

		cf_write = cf;
		mf = mf_next;
	}

	return outf;
}

/* color_copy_glyph_range:
 *  Colour font helper function. Copies (part of) a glyph range
 */
static FONT_COLOR_DATA *color_copy_glyph_range(FONT_COLOR_DATA *cf, int begin, int end) {
	if (begin < cf->begin || end > cf->end)
		return NULL;

	FONT_COLOR_DATA *newcf = _AL_MALLOC(sizeof *newcf);

	if (!newcf)
		return NULL;

	newcf->begin = begin;
	newcf->end = end;
	newcf->next = NULL;
	int num = end - begin;

	BITMAP **gl = newcf->bitmaps = _AL_MALLOC(num * sizeof *gl);
	for (int c = 0; c < num; c++) {
		BITMAP *g = cf->bitmaps[begin - cf->begin + c];
		gl[c] = create_bitmap_ex(bitmap_color_depth(g), g->w, g->h);
		blit(g, gl[c], 0, 0, 0, 0, g->w, g->h);
	}

	return newcf;
}

/* color_extract_font_range:
 *  (color vtable entry)
 *  Extract a range of characters from a color font
 */
static FONT *color_extract_font_range(FONT *f, int begin, int end) {
	if (!f)
		return NULL;

	/* Special case: copy entire font */
	if (begin == -1 && end == -1) {
	}
	/* Copy from the beginning */
	else if (begin == -1 && end > color_get_font_range_begin(f, -1)) {
	}
	/* Copy to the end */
	else if (end == -1 && begin <= color_get_font_range_end(f, -1)) {
	}
	/* begin cannot be bigger than end */
	else if (begin <= end && begin != -1 && end != -1) {
	} else {
		return NULL;
	}

	/* Get output font */
	FONT *fontout = _AL_MALLOC(sizeof *fontout);

	fontout->height = f->height;
	fontout->vtable = f->vtable;
	fontout->data = NULL;

	/* Get real character ranges */
	int first = MAX(begin, color_get_font_range_begin(f, -1));
	int last = (end > -1) ? MIN(end, color_get_font_range_end(f, -1)) : color_get_font_range_end(f, -1);
	last++;

	FONT_COLOR_DATA *cf = NULL;
	FONT_COLOR_DATA *cfin = f->data;
	while (cfin) {
		/* Find the range that is covered by the requested range. */
		/* Check if the requested and processed ranges at least overlap */
		if (((first >= cfin->begin && first < cfin->end) || (last <= cfin->end && last > cfin->begin))
				/* Check if the requested range wraps processed ranges */
				|| (first < cfin->begin && last > cfin->end)) {
			int local_begin, local_end;

			local_begin = MAX(cfin->begin, first);
			local_end = MIN(cfin->end, last);

			if (cf) {
				cf->next = color_copy_glyph_range(cfin, local_begin, local_end);
				cf = cf->next;
			} else {
				cf = color_copy_glyph_range(cfin, local_begin, local_end);
				fontout->data = cf;
			}
		}
		cfin = cfin->next;
	}

	return fontout;
}

/* color_merge_fonts:
 *  (color vtable entry)
 *  Merges font2 with font1 and returns a new font
 */
static FONT *color_merge_fonts(FONT *font1, FONT *font2) {
	FONT *font2_upgr = NULL;

	if (!font1 || !font2)
		return NULL;

	/* Promote font 2 to colour if it is a monochrome font */
	if (!is_color_font(font1))
		return NULL;

	if (is_mono_font(font2)) {
		font2_upgr = upgrade_to_color(font2);
		/* Couldn't update font */
		if (!font2_upgr)
			return NULL;
	} else
		font2_upgr = font2;

	if (!is_color_font(font2_upgr))
		return NULL;

	/* Get output font */
	FONT *fontout = _AL_MALLOC(sizeof *fontout);
	fontout->height = MAX(font1->height, font2->height);
	fontout->vtable = font1->vtable;
	FONT_COLOR_DATA *cf = fontout->data = NULL;

	FONT_COLOR_DATA *cf1 = font1->data;
	FONT_COLOR_DATA *cf2 = font2_upgr->data;
	while (cf1 || cf2) {
		if (cf1 && (!cf2 || (cf1->begin < cf2->begin))) {
			if (cf) {
				cf->next = color_copy_glyph_range(cf1, cf1->begin, cf1->end);
				cf = cf->next;
			} else {
				cf = color_copy_glyph_range(cf1, cf1->begin, cf1->end);
				fontout->data = cf;
			}
			cf1 = cf1->next;
		} else {
			if (cf) {
				cf->next = color_copy_glyph_range(cf2, cf2->begin, cf2->end);
				cf = cf->next;
			} else {
				cf = color_copy_glyph_range(cf2, cf2->begin, cf2->end);
				fontout->data = cf;
			}
			cf2 = cf2->next;
		}
	}

	if (font2_upgr != font2)
		destroy_font(font2_upgr);

	return fontout;
}

/* color_transpose_font:
 *  (color vtable entry)
 *  Transpose all glyphs in a font
 */
static int color_transpose_font(FONT *f, int drange) {
	if (!f)
		return -1;

	FONT_COLOR_DATA *cf = (FONT_COLOR_DATA *)(f->data);

	while (cf) {
		FONT_COLOR_DATA *next = cf->next;

		cf->begin += drange;
		cf->end += drange;
		cf = next;
	}

	return 0;
}

/********
 * vtable declarations
 ********/

FONT_VTABLE _font_vtable_mono = {
	font_height,
	mono_char_length,
	length,
	mono_render_char,
	mono_render,
	mono_destroy,

	mono_get_font_ranges,
	mono_get_font_range_begin,
	mono_get_font_range_end,
	mono_extract_font_range,
	mono_merge_fonts,
	mono_transpose_font
};

FONT_VTABLE *font_vtable_mono = &_font_vtable_mono;

FONT_VTABLE _font_vtable_color = {
	font_height,
	color_char_length,
	length,
	color_render_char,
	color_render,
	color_destroy,

	color_get_font_ranges,
	color_get_font_range_begin,
	color_get_font_range_end,
	color_extract_font_range,
	color_merge_fonts,
	color_transpose_font
};

FONT_VTABLE *font_vtable_color = &_font_vtable_color;

FONT_VTABLE _font_vtable_trans = {
	font_height,
	color_char_length,
	length,
	trans_render_char,
	color_render,
	color_destroy,

	color_get_font_ranges,
	color_get_font_range_begin,
	color_get_font_range_end,
	color_extract_font_range,
	color_merge_fonts,
	color_transpose_font
};

FONT_VTABLE *font_vtable_trans = &_font_vtable_trans;

/* font_has_alpha:
 *  Returns TRUE if a color font has an alpha channel.
 */
int font_has_alpha(FONT *fnt) {
	ASSERT(fnt);

	if (!is_color_font(fnt))
		return FALSE;

	FONT_COLOR_DATA *data = (FONT_COLOR_DATA *)(fnt->data);

	while (data) {
		for (int ch = data->begin; ch != data->end; ++ch)
			if (_bitmap_has_alpha(data->bitmaps[ch - data->begin]))
				return TRUE;
		data = data->next;
	}

	return FALSE;
}

/* make_trans_font:
 *  Modifes a font so glyphs are drawn with draw_trans_sprite.
 */
void make_trans_font(FONT *f) {
	ASSERT(f);
	ASSERT(f->vtable == font_vtable_color);

	f->vtable = font_vtable_trans;
}

/* is_trans_font:
 *  Returns non-zero if the font passed is a bitmapped colour font using
 *  draw_trans_sprite to render glyphs.
 */
int is_trans_font(FONT *f) {
	ASSERT(f);

	return (f->vtable == font_vtable_trans);
}

/* is_color_font:
 *  returns non-zero if the font passed is a bitmapped colour font
 */
int is_color_font(FONT *f) {
	ASSERT(f);

	return (f->vtable == font_vtable_color || f->vtable == font_vtable_trans);
}

/* is_mono_font:
 *  returns non-zero if the font passed is a monochrome font
 */
int is_mono_font(FONT *f) {
	ASSERT(f);

	return f->vtable == font_vtable_mono;
}

/* is_compatibe_font:
 *  returns non-zero if the two fonts are of similar type
 */
int is_compatible_font(FONT *f1, FONT *f2) {
	ASSERT(f1);
	ASSERT(f2);
	return f1->vtable == f2->vtable;
}

/* extract_font_range:
 *  Extracts a character range from a font f, and returns a new font containing
 *   only the extracted characters.
 * Returns NULL if the character range could not be extracted.
 */
FONT *extract_font_range(FONT *f, int begin, int end) {
	if (f->vtable->extract_font_range)
		return f->vtable->extract_font_range(f, begin, end);
	return NULL;
}

/* merge_fonts:
 *  Merges two fonts. May convert the two fonts to compatible types before
 *   merging, in which case converting the type of f2 to f1 is tried first.
 */
FONT *merge_fonts(FONT *f1, FONT *f2) {
	FONT *f = NULL;

	if (f1->vtable->merge_fonts)
		f = f1->vtable->merge_fonts(f1, f2);

	if (!f && f2->vtable->merge_fonts)
		f = f2->vtable->merge_fonts(f2, f1);

	return f;
}

/* get_font_ranges:
 *  Returns the number of character ranges in a font, or -1 if that information
 *   is not available.
 */
int get_font_ranges(FONT *f) {
	if (f->vtable->get_font_ranges)
		return f->vtable->get_font_ranges(f);

	return -1;
}

/* get_font_range_begin:
 *  Returns the starting character for the font in question, or -1 if that
 *   information is not available.
 */
int get_font_range_begin(FONT *f, int range) {
	if (f->vtable->get_font_range_begin)
		return f->vtable->get_font_range_begin(f, range);

	return -1;
}

/* get_font_range_end:
 *  Returns the last character for the font in question, or -1 if that
 *  information is not available.
 */
int get_font_range_end(FONT *f, int range) {
	if (f->vtable->get_font_range_end)
		return f->vtable->get_font_range_end(f, range);

	return -1;
}

/* transpose_font:
 *  Transposes all the glyphs in a font over a range drange. Returns 0 on
 *   success, or -1 on failure.
 */
int transpose_font(FONT *f, int drange) {
	if (f->vtable->transpose_font)
		return f->vtable->transpose_font(f, drange);

	return -1;
}

/********
 * Declaration of `_default_font' and `font'
 ********/

static FONT_MONO_DATA euro_monofont = {
	0x20AC, 0x20AD, /* begin, end characters */
	euro_data, /* the data set */
	0 /* next */
};

static FONT_MONO_DATA extended_a_monofont = {
	0x100, 0x180, /* begin, end characters */
	extended_a_data, /* the data set */
	&euro_monofont /* next */
};

static FONT_MONO_DATA latin1_monofont = {
	0x0A1, 0x100, /* begin, end characters */
	latin1_data, /* the data set */
	&extended_a_monofont /* next */
};

static FONT_MONO_DATA ascii_monofont = {
	0x20, 0x80, /* begin, end characters */
	ascii_data, /* the data set */
	&latin1_monofont /* next */
};

static FONT default_font = {
	&ascii_monofont, /* first lot of data */
	8, /* height */
	&_font_vtable_mono /* vtable */
};

FONT *font = &default_font;

/*
 * 256 color polygon scanline filler helpers (gouraud shading, tmapping, etc)
 * and sprite drawing functions.
 * ==========================================================================
 */

#ifdef ALLEGRO_COLOR8

#define PP_DEPTH 8

#define PIXEL_PTR unsigned char *
#define PTR_PER_PIXEL 1
#define OFFSET_PIXEL_PTR(p, x) ((PIXEL_PTR)(p) + (x))
#define INC_PIXEL_PTR(p) ((p)++)
#define INC_PIXEL_PTR_N(p, d) ((p) += d)
#define DEC_PIXEL_PTR(p) ((p)--)

#define PUT_PIXEL(p, c) bmp_write8((uintptr_t)(p), (c))
#define PUT_MEMORY_PIXEL(p, c) (*(p) = (c))
#define PUT_RGB(p, r, g, b) bmp_write8((uintptr_t)(p), makecol8((r), (g), (b)))
#define GET_PIXEL(p) bmp_read8((uintptr_t)(p))
#define GET_MEMORY_PIXEL(p) (*(p))

#define IS_MASK(c) ((c) == 0)
#define IS_SPRITE_MASK(b, c) ((c) == 0)

/* Blender for putpixel (DRAW_MODE_TRANS).  */
#define PP_BLENDER unsigned char *
#define MAKE_PP_BLENDER(c) (color_map->data[(c) & 0xFF])
#define PP_BLEND(b, o, n) ((b)[(o) & 0xFF])

/* Blender for draw_trans_*_sprite.  */
#define DTS_BLENDER COLOR_MAP *
#define MAKE_DTS_BLENDER() color_map
#define DTS_BLEND(b, o, n) ((b)->data[(n) & 0xFF][(o) & 0xFF])

/* Blender for draw_lit_*_sprite.  */
#define DLS_BLENDER unsigned char *
#define MAKE_DLS_BLENDER(a) (color_map->data[(a) & 0xFF])
#define DLS_BLEND(b, a, c) ((b)[(c) & 0xFF])
#define DLSX_BLEND(b, c) ((b)[(c) & 0xFF])

/* Blender for poly_scanline_*_lit.  */
#define PS_BLENDER COLOR_MAP *
#define MAKE_PS_BLENDER() color_map
#define PS_BLEND(b, o, c) ((b)->data[(o) & 0xFF][(c) & 0xFF])
#define PS_ALPHA_BLEND(b, o, c) ((b)->data[(o) & 0xFF][(c) & 0xFF])

#define PATTERN_LINE(y) _drawing_pattern->line[((y) - _drawing_y_anchor) & _drawing_y_mask]
#define GET_PATTERN_PIXEL(x, y) GET_MEMORY_PIXEL(OFFSET_PIXEL_PTR(PATTERN_LINE(y), ((x) - _drawing_x_anchor) & _drawing_x_mask))

#define RLE_PTR signed char *
#define RLE_IS_EOL(c) ((c) == 0)

#define FUNC_LINEAR_CLEAR_TO_COLOR _linear_clear_to_color8
#define FUNC_LINEAR_BLIT _linear_blit8
#define FUNC_LINEAR_BLIT_BACKWARD _linear_blit_backward8
#define FUNC_LINEAR_MASKED_BLIT _linear_masked_blit8

#define FUNC_LINEAR_PUTPIXEL _linear_putpixel8
#define FUNC_LINEAR_GETPIXEL _linear_getpixel8
#define FUNC_LINEAR_HLINE _linear_hline8
#define FUNC_LINEAR_VLINE _linear_vline8

#define FUNC_LINEAR_DRAW_SPRITE _linear_draw_sprite8
#define FUNC_LINEAR_DRAW_SPRITE_EX _linear_draw_sprite_ex8
#define FUNC_LINEAR_DRAW_256_SPRITE _linear_draw_256_sprite8
#define FUNC_LINEAR_DRAW_SPRITE_V_FLIP _linear_draw_sprite_v_flip8
#define FUNC_LINEAR_DRAW_SPRITE_H_FLIP _linear_draw_sprite_h_flip8
#define FUNC_LINEAR_DRAW_SPRITE_VH_FLIP _linear_draw_sprite_vh_flip8
#define FUNC_LINEAR_DRAW_TRANS_SPRITE _linear_draw_trans_sprite8
#define FUNC_LINEAR_DRAW_TRANS_RGBA_SPRITE _linear_draw_trans_rgba_sprite8
#define FUNC_LINEAR_DRAW_LIT_SPRITE _linear_draw_lit_sprite8
#define FUNC_LINEAR_DRAW_CHARACTER _linear_draw_character8
#define FUNC_LINEAR_DRAW_RLE_SPRITE _linear_draw_rle_sprite8
#define FUNC_LINEAR_DRAW_TRANS_RLE_SPRITE _linear_draw_trans_rle_sprite8
#define FUNC_LINEAR_DRAW_TRANS_RGBA_RLE_SPRITE _linear_draw_trans_rgba_rle_sprite8
#define FUNC_LINEAR_DRAW_LIT_RLE_SPRITE _linear_draw_lit_rle_sprite8

#define FUNC_LINEAR_DRAW_SPRITE_END _linear_draw_sprite8_end
#define FUNC_LINEAR_BLIT_END _linear_blit8_end

#define FUNC_POLY_SCANLINE_GCOL _poly_scanline_gcol8
#define FUNC_POLY_SCANLINE_GRGB _poly_scanline_grgb8
#define FUNC_POLY_SCANLINE_ATEX _poly_scanline_atex8
#define FUNC_POLY_SCANLINE_ATEX_MASK _poly_scanline_atex_mask8
#define FUNC_POLY_SCANLINE_ATEX_LIT _poly_scanline_atex_lit8
#define FUNC_POLY_SCANLINE_ATEX_MASK_LIT _poly_scanline_atex_mask_lit8
#define FUNC_POLY_SCANLINE_PTEX _poly_scanline_ptex8
#define FUNC_POLY_SCANLINE_PTEX_MASK _poly_scanline_ptex_mask8
#define FUNC_POLY_SCANLINE_PTEX_LIT _poly_scanline_ptex_lit8
#define FUNC_POLY_SCANLINE_PTEX_MASK_LIT _poly_scanline_ptex_mask_lit8
#define FUNC_POLY_SCANLINE_ATEX_TRANS _poly_scanline_atex_trans8
#define FUNC_POLY_SCANLINE_ATEX_MASK_TRANS _poly_scanline_atex_mask_trans8
#define FUNC_POLY_SCANLINE_PTEX_TRANS _poly_scanline_ptex_trans8
#define FUNC_POLY_SCANLINE_PTEX_MASK_TRANS _poly_scanline_ptex_mask_trans8

#define FUNC_POLY_ZBUF_FLAT _poly_zbuf_flat8
#define FUNC_POLY_ZBUF_GCOL _poly_zbuf_gcol8
#define FUNC_POLY_ZBUF_GRGB _poly_zbuf_grgb8
#define FUNC_POLY_ZBUF_ATEX _poly_zbuf_atex8
#define FUNC_POLY_ZBUF_ATEX_MASK _poly_zbuf_atex_mask8
#define FUNC_POLY_ZBUF_ATEX_LIT _poly_zbuf_atex_lit8
#define FUNC_POLY_ZBUF_ATEX_MASK_LIT _poly_zbuf_atex_mask_lit8
#define FUNC_POLY_ZBUF_PTEX _poly_zbuf_ptex8
#define FUNC_POLY_ZBUF_PTEX_MASK _poly_zbuf_ptex_mask8
#define FUNC_POLY_ZBUF_PTEX_LIT _poly_zbuf_ptex_lit8
#define FUNC_POLY_ZBUF_PTEX_MASK_LIT _poly_zbuf_ptex_mask_lit8
#define FUNC_POLY_ZBUF_ATEX_TRANS _poly_zbuf_atex_trans8
#define FUNC_POLY_ZBUF_ATEX_MASK_TRANS _poly_zbuf_atex_mask_trans8
#define FUNC_POLY_ZBUF_PTEX_TRANS _poly_zbuf_ptex_trans8
#define FUNC_POLY_ZBUF_PTEX_MASK_TRANS _poly_zbuf_ptex_mask_trans8

#define _bma_zbuf_gcol
#define _bma_scan_gcol

#include "al_gfx_inline.h"

#undef _bma_scan_gcol
#undef _bma_zbuf_gcol

#endif

/*
 * 15 bit color polygon scanline filler helpers (gouraud shading, tmapping, etc)
 * and sprite drawing functions.
 * =============================================================================
 */

#ifdef ALLEGRO_COLOR16

#define PP_DEPTH 15

#define PIXEL_PTR unsigned short *
#define PTR_PER_PIXEL 1
#define OFFSET_PIXEL_PTR(p, x) ((PIXEL_PTR)(p) + (x))
#define INC_PIXEL_PTR(p) ((p)++)
#define INC_PIXEL_PTR_N(p, d) ((p) += d)
#define DEC_PIXEL_PTR(p) ((p)--)

#define PUT_PIXEL(p, c) bmp_write15((uintptr_t)(p), (c))
#define PUT_MEMORY_PIXEL(p, c) (*(p) = (c))
#define PUT_RGB(p, r, g, b) bmp_write15((uintptr_t)(p), makecol15((r), (g), (b)))
#define GET_PIXEL(p) bmp_read15((uintptr_t)(p))
#define GET_MEMORY_PIXEL(p) (*(p))

#define IS_MASK(c) ((unsigned long)(c) == MASK_COLOR_15)
#define IS_SPRITE_MASK(b, c) ((unsigned long)(c) == MASK_COLOR_15)

/* Blender for putpixel (DRAW_MODE_TRANS).  */
#define PP_BLENDER BLENDER_FUNC
#define MAKE_PP_BLENDER(c) _blender_func15
#define PP_BLEND(b, o, n) ((*(b))((n), (o), _blender_alpha))

/* Blender for draw_trans_*_sprite.  */
#define DTS_BLENDER BLENDER_FUNC
#define MAKE_DTS_BLENDER() _blender_func15
#define DTS_BLEND(b, o, n) ((*(b))((n), (o), _blender_alpha))

/* Blender for draw_lit_*_sprite.  */
#define DLS_BLENDER BLENDER_FUNC
#define MAKE_DLS_BLENDER(a) _blender_func15
#define DLS_BLEND(b, a, n) ((*(b))(_blender_col_15, (n), (a)))
#define DLSX_BLEND(b, n) ((*(b))(_blender_col_15, (n), _blender_alpha))

/* Blender for RGBA sprites.  */
#define RGBA_BLENDER BLENDER_FUNC
#define MAKE_RGBA_BLENDER() _blender_func15x
#define RGBA_BLEND(b, o, n) ((*(b))((n), (o), _blender_alpha))

/* Blender for poly_scanline_*_lit.  */
#define PS_BLENDER BLENDER_FUNC
#define MAKE_PS_BLENDER() _blender_func15
#define PS_BLEND(b, o, c) ((*(b))((c), _blender_col_15, (o)))
#define PS_ALPHA_BLEND(b, o, c) ((*(b))((o), (c), _blender_alpha))

#define PATTERN_LINE(y) (PIXEL_PTR)(_drawing_pattern->line[((y) - _drawing_y_anchor) & _drawing_y_mask])
#define GET_PATTERN_PIXEL(x, y) GET_MEMORY_PIXEL(OFFSET_PIXEL_PTR(PATTERN_LINE(y), ((x) - _drawing_x_anchor) & _drawing_x_mask))

#define RLE_PTR signed short *
#define RLE_IS_EOL(c) ((unsigned short)(c) == MASK_COLOR_15)

#define FUNC_LINEAR_CLEAR_TO_COLOR _linear_clear_to_color15
#define FUNC_LINEAR_BLIT _linear_blit15
#define FUNC_LINEAR_BLIT_BACKWARD _linear_blit_backward15
#define FUNC_LINEAR_MASKED_BLIT _linear_masked_blit15

#define FUNC_LINEAR_PUTPIXEL _linear_putpixel15
#define FUNC_LINEAR_GETPIXEL _linear_getpixel15
#define FUNC_LINEAR_HLINE _linear_hline15
#define FUNC_LINEAR_VLINE _linear_vline15

#define FUNC_LINEAR_DRAW_SPRITE _linear_draw_sprite15
#define FUNC_LINEAR_DRAW_SPRITE_EX _linear_draw_sprite_ex15
#define FUNC_LINEAR_DRAW_256_SPRITE _linear_draw_256_sprite15
#define FUNC_LINEAR_DRAW_SPRITE_V_FLIP _linear_draw_sprite_v_flip15
#define FUNC_LINEAR_DRAW_SPRITE_H_FLIP _linear_draw_sprite_h_flip15
#define FUNC_LINEAR_DRAW_SPRITE_VH_FLIP _linear_draw_sprite_vh_flip15
#define FUNC_LINEAR_DRAW_TRANS_SPRITE _linear_draw_trans_sprite15
#define FUNC_LINEAR_DRAW_TRANS_RGBA_SPRITE _linear_draw_trans_rgba_sprite15
#define FUNC_LINEAR_DRAW_LIT_SPRITE _linear_draw_lit_sprite15
#define FUNC_LINEAR_DRAW_CHARACTER _linear_draw_character15
#define FUNC_LINEAR_DRAW_RLE_SPRITE _linear_draw_rle_sprite15
#define FUNC_LINEAR_DRAW_TRANS_RLE_SPRITE _linear_draw_trans_rle_sprite15
#define FUNC_LINEAR_DRAW_TRANS_RGBA_RLE_SPRITE _linear_draw_trans_rgba_rle_sprite15
#define FUNC_LINEAR_DRAW_LIT_RLE_SPRITE _linear_draw_lit_rle_sprite15

#define FUNC_LINEAR_DRAW_SPRITE_END _linear_draw_sprite15_end
#define FUNC_LINEAR_BLIT_END _linear_blit15_end

#define FUNC_POLY_SCANLINE_GRGB _poly_scanline_grgb15
#define FUNC_POLY_SCANLINE_ATEX _poly_scanline_atex15
#define FUNC_POLY_SCANLINE_ATEX_MASK _poly_scanline_atex_mask15
#define FUNC_POLY_SCANLINE_ATEX_LIT _poly_scanline_atex_lit15
#define FUNC_POLY_SCANLINE_ATEX_MASK_LIT _poly_scanline_atex_mask_lit15
#define FUNC_POLY_SCANLINE_PTEX _poly_scanline_ptex15
#define FUNC_POLY_SCANLINE_PTEX_MASK _poly_scanline_ptex_mask15
#define FUNC_POLY_SCANLINE_PTEX_LIT _poly_scanline_ptex_lit15
#define FUNC_POLY_SCANLINE_PTEX_MASK_LIT _poly_scanline_ptex_mask_lit15
#define FUNC_POLY_SCANLINE_ATEX_TRANS _poly_scanline_atex_trans15
#define FUNC_POLY_SCANLINE_ATEX_MASK_TRANS _poly_scanline_atex_mask_trans15
#define FUNC_POLY_SCANLINE_PTEX_TRANS _poly_scanline_ptex_trans15
#define FUNC_POLY_SCANLINE_PTEX_MASK_TRANS _poly_scanline_ptex_mask_trans15

#define FUNC_POLY_ZBUF_FLAT _poly_zbuf_flat15
#define FUNC_POLY_ZBUF_GRGB _poly_zbuf_grgb15
#define FUNC_POLY_ZBUF_ATEX _poly_zbuf_atex15
#define FUNC_POLY_ZBUF_ATEX_MASK _poly_zbuf_atex_mask15
#define FUNC_POLY_ZBUF_ATEX_LIT _poly_zbuf_atex_lit15
#define FUNC_POLY_ZBUF_ATEX_MASK_LIT _poly_zbuf_atex_mask_lit15
#define FUNC_POLY_ZBUF_PTEX _poly_zbuf_ptex15
#define FUNC_POLY_ZBUF_PTEX_MASK _poly_zbuf_ptex_mask15
#define FUNC_POLY_ZBUF_PTEX_LIT _poly_zbuf_ptex_lit15
#define FUNC_POLY_ZBUF_PTEX_MASK_LIT _poly_zbuf_ptex_mask_lit15
#define FUNC_POLY_ZBUF_ATEX_TRANS _poly_zbuf_atex_trans15
#define FUNC_POLY_ZBUF_ATEX_MASK_TRANS _poly_zbuf_atex_mask_trans15
#define FUNC_POLY_ZBUF_PTEX_TRANS _poly_zbuf_ptex_trans15
#define FUNC_POLY_ZBUF_PTEX_MASK_TRANS _poly_zbuf_ptex_mask_trans15

#undef _bma_scan_gcol
#undef _bma_zbuf_gcol

#include "al_gfx_inline.h"

#endif

/*
 * 16 bit color polygon scanline filler helpers (gouraud shading, tmapping, etc)
 * sprite drawing functions.
 * =============================================================================
 */

#ifdef ALLEGRO_COLOR16

#define PP_DEPTH 16

#define PIXEL_PTR unsigned short *
#define PTR_PER_PIXEL 1
#define OFFSET_PIXEL_PTR(p, x) ((PIXEL_PTR)(p) + (x))
#define INC_PIXEL_PTR(p) ((p)++)
#define INC_PIXEL_PTR_N(p, d) ((p) += d)
#define DEC_PIXEL_PTR(p) ((p)--)

#define PUT_PIXEL(p, c) bmp_write16((uintptr_t)(p), (c))
#define PUT_MEMORY_PIXEL(p, c) (*(p) = (c))
#define PUT_RGB(p, r, g, b) bmp_write16((uintptr_t)(p), makecol16((r), (g), (b)))
#define GET_PIXEL(p) bmp_read16((uintptr_t)(p))
#define GET_MEMORY_PIXEL(p) (*(p))

#define IS_MASK(c) ((unsigned long)(c) == MASK_COLOR_16)
#define IS_SPRITE_MASK(b, c) ((unsigned long)(c) == (unsigned long)(b)->vtable->mask_color)

/* Blender for putpixel (DRAW_MODE_TRANS).  */
#define PP_BLENDER BLENDER_FUNC
#define MAKE_PP_BLENDER(c) _blender_func16
#define PP_BLEND(b, o, n) ((*(b))((n), (o), _blender_alpha))

/* Blender for draw_trans_*_sprite.  */
#define DTS_BLENDER BLENDER_FUNC
#define MAKE_DTS_BLENDER() _blender_func16
#define DTS_BLEND(b, o, n) ((*(b))((n), (o), _blender_alpha))

/* Blender for draw_lit_*_sprite.  */
#define DLS_BLENDER BLENDER_FUNC
#define MAKE_DLS_BLENDER(a) _blender_func16
#define DLS_BLEND(b, a, n) ((*(b))(_blender_col_16, (n), (a)))
#define DLSX_BLEND(b, n) ((*(b))(_blender_col_16, (n), _blender_alpha))

/* Blender for RGBA sprites.  */
#define RGBA_BLENDER BLENDER_FUNC
#define MAKE_RGBA_BLENDER() _blender_func16x
#define RGBA_BLEND(b, o, n) ((*(b))((n), (o), _blender_alpha))

/* Blender for poly_scanline_*_lit.  */
#define PS_BLENDER BLENDER_FUNC
#define MAKE_PS_BLENDER() _blender_func16
#define PS_BLEND(b, o, c) ((*(b))((c), _blender_col_16, (o)))
#define PS_ALPHA_BLEND(b, o, c) ((*(b))((o), (c), _blender_alpha))

#define PATTERN_LINE(y) (PIXEL_PTR)(_drawing_pattern->line[((y) - _drawing_y_anchor) & _drawing_y_mask])
#define GET_PATTERN_PIXEL(x, y) GET_MEMORY_PIXEL(OFFSET_PIXEL_PTR(PATTERN_LINE(y), ((x) - _drawing_x_anchor) & _drawing_x_mask))

#define RLE_PTR signed short *
#define RLE_IS_EOL(c) ((unsigned short)(c) == MASK_COLOR_16)

#define FUNC_LINEAR_CLEAR_TO_COLOR _linear_clear_to_color16
#define FUNC_LINEAR_BLIT _linear_blit16
#define FUNC_LINEAR_BLIT_BACKWARD _linear_blit_backward16
#define FUNC_LINEAR_MASKED_BLIT _linear_masked_blit16

#define FUNC_LINEAR_PUTPIXEL _linear_putpixel16
#define FUNC_LINEAR_GETPIXEL _linear_getpixel16
#define FUNC_LINEAR_HLINE _linear_hline16
#define FUNC_LINEAR_VLINE _linear_vline16

#define FUNC_LINEAR_DRAW_SPRITE _linear_draw_sprite16
#define FUNC_LINEAR_DRAW_SPRITE_EX _linear_draw_sprite_ex16
#define FUNC_LINEAR_DRAW_256_SPRITE _linear_draw_256_sprite16
#define FUNC_LINEAR_DRAW_SPRITE_V_FLIP _linear_draw_sprite_v_flip16
#define FUNC_LINEAR_DRAW_SPRITE_H_FLIP _linear_draw_sprite_h_flip16
#define FUNC_LINEAR_DRAW_SPRITE_VH_FLIP _linear_draw_sprite_vh_flip16
#define FUNC_LINEAR_DRAW_TRANS_SPRITE _linear_draw_trans_sprite16
#define FUNC_LINEAR_DRAW_TRANS_RGBA_SPRITE _linear_draw_trans_rgba_sprite16
#define FUNC_LINEAR_DRAW_LIT_SPRITE _linear_draw_lit_sprite16
#define FUNC_LINEAR_DRAW_CHARACTER _linear_draw_character16
#define FUNC_LINEAR_DRAW_RLE_SPRITE _linear_draw_rle_sprite16
#define FUNC_LINEAR_DRAW_TRANS_RLE_SPRITE _linear_draw_trans_rle_sprite16
#define FUNC_LINEAR_DRAW_TRANS_RGBA_RLE_SPRITE _linear_draw_trans_rgba_rle_sprite16
#define FUNC_LINEAR_DRAW_LIT_RLE_SPRITE _linear_draw_lit_rle_sprite16

#define FUNC_LINEAR_DRAW_SPRITE_END _linear_draw_sprite16_end
#define FUNC_LINEAR_BLIT_END _linear_blit16_end

#define FUNC_POLY_SCANLINE_GRGB _poly_scanline_grgb16
#define FUNC_POLY_SCANLINE_ATEX _poly_scanline_atex16
#define FUNC_POLY_SCANLINE_ATEX_MASK _poly_scanline_atex_mask16
#define FUNC_POLY_SCANLINE_ATEX_LIT _poly_scanline_atex_lit16
#define FUNC_POLY_SCANLINE_ATEX_MASK_LIT _poly_scanline_atex_mask_lit16
#define FUNC_POLY_SCANLINE_PTEX _poly_scanline_ptex16
#define FUNC_POLY_SCANLINE_PTEX_MASK _poly_scanline_ptex_mask16
#define FUNC_POLY_SCANLINE_PTEX_LIT _poly_scanline_ptex_lit16
#define FUNC_POLY_SCANLINE_PTEX_MASK_LIT _poly_scanline_ptex_mask_lit16
#define FUNC_POLY_SCANLINE_ATEX_TRANS _poly_scanline_atex_trans16
#define FUNC_POLY_SCANLINE_ATEX_MASK_TRANS _poly_scanline_atex_mask_trans16
#define FUNC_POLY_SCANLINE_PTEX_TRANS _poly_scanline_ptex_trans16
#define FUNC_POLY_SCANLINE_PTEX_MASK_TRANS _poly_scanline_ptex_mask_trans16

#define FUNC_POLY_ZBUF_FLAT _poly_zbuf_flat16
#define FUNC_POLY_ZBUF_GRGB _poly_zbuf_grgb16
#define FUNC_POLY_ZBUF_ATEX _poly_zbuf_atex16
#define FUNC_POLY_ZBUF_ATEX_MASK _poly_zbuf_atex_mask16
#define FUNC_POLY_ZBUF_ATEX_LIT _poly_zbuf_atex_lit16
#define FUNC_POLY_ZBUF_ATEX_MASK_LIT _poly_zbuf_atex_mask_lit16
#define FUNC_POLY_ZBUF_PTEX _poly_zbuf_ptex16
#define FUNC_POLY_ZBUF_PTEX_MASK _poly_zbuf_ptex_mask16
#define FUNC_POLY_ZBUF_PTEX_LIT _poly_zbuf_ptex_lit16
#define FUNC_POLY_ZBUF_PTEX_MASK_LIT _poly_zbuf_ptex_mask_lit16
#define FUNC_POLY_ZBUF_ATEX_TRANS _poly_zbuf_atex_trans16
#define FUNC_POLY_ZBUF_ATEX_MASK_TRANS _poly_zbuf_atex_mask_trans16
#define FUNC_POLY_ZBUF_PTEX_TRANS _poly_zbuf_ptex_trans16
#define FUNC_POLY_ZBUF_PTEX_MASK_TRANS _poly_zbuf_ptex_mask_trans16

#undef _bma_scan_gcol
#undef _bma_zbuf_gcol

#include "al_gfx_inline.h"

#endif

/*
 * 24 bit color polygon scanline filler helpers (gouraud shading, tmapping, etc)
 * sprite drawing functions.
 * =============================================================================
 */

#ifdef ALLEGRO_COLOR24

#define PP_DEPTH 24

#define PIXEL_PTR unsigned char *
#define PTR_PER_PIXEL 3
#define OFFSET_PIXEL_PTR(p, x) ((PIXEL_PTR)(p) + 3 * (x))
#define INC_PIXEL_PTR(p) ((p) += 3)
#define INC_PIXEL_PTR_N(p, d) ((p) += 3 * d)
#define DEC_PIXEL_PTR(p) ((p) -= 3)

#define PUT_PIXEL(p, c) bmp_write24((uintptr_t)(p), (c))
#define PUT_MEMORY_PIXEL(p, c) WRITE3BYTES((p), (c))
#define PUT_RGB(p, r, g, b) bmp_write24((uintptr_t)(p), makecol24((r), (g), (b)))
#define GET_PIXEL(p) bmp_read24((uintptr_t)(p))
#define GET_MEMORY_PIXEL(p) READ3BYTES((p))
#define IS_MASK(c) ((unsigned long)(c) == MASK_COLOR_24)
#define IS_SPRITE_MASK(b, c) ((unsigned long)(c) == MASK_COLOR_24)

/* Blender for putpixel (DRAW_MODE_TRANS).  */
#define PP_BLENDER BLENDER_FUNC
#define MAKE_PP_BLENDER(c) _blender_func24
#define PP_BLEND(b, o, n) ((*(b))((n), (o), _blender_alpha))

/* Blender for draw_trans_*_sprite.  */
#define DTS_BLENDER BLENDER_FUNC
#define MAKE_DTS_BLENDER() _blender_func24
#define DTS_BLEND(b, o, n) ((*(b))((n), (o), _blender_alpha))

/* Blender for draw_lit_*_sprite.  */
#define DLS_BLENDER BLENDER_FUNC
#define MAKE_DLS_BLENDER(a) _blender_func24
#define DLS_BLEND(b, a, n) ((*(b))(_blender_col_24, (n), (a)))
#define DLSX_BLEND(b, n) ((*(b))(_blender_col_24, (n), _blender_alpha))

/* Blender for RGBA sprites.  */
#define RGBA_BLENDER BLENDER_FUNC
#define MAKE_RGBA_BLENDER() _blender_func24x
#define RGBA_BLEND(b, o, n) ((*(b))((n), (o), _blender_alpha))

/* Blender for poly_scanline_*_lit.  */
#define PS_BLENDER BLENDER_FUNC
#define MAKE_PS_BLENDER() _blender_func24
#define PS_BLEND(b, o, c) ((*(b))((c), _blender_col_24, (o)))
#define PS_ALPHA_BLEND(b, o, c) ((*(b))((o), (c), _blender_alpha))

#define PATTERN_LINE(y) (PIXEL_PTR)(_drawing_pattern->line[((y) - _drawing_y_anchor) & _drawing_y_mask])
#define GET_PATTERN_PIXEL(x, y) GET_MEMORY_PIXEL(OFFSET_PIXEL_PTR(PATTERN_LINE(y), ((x) - _drawing_x_anchor) & _drawing_x_mask))

#define RLE_PTR int32_t *
#define RLE_IS_EOL(c) ((unsigned long)(c) == MASK_COLOR_24)

#define FUNC_LINEAR_CLEAR_TO_COLOR _linear_clear_to_color24
#define FUNC_LINEAR_BLIT _linear_blit24
#define FUNC_LINEAR_BLIT_BACKWARD _linear_blit_backward24
#define FUNC_LINEAR_MASKED_BLIT _linear_masked_blit24

#define FUNC_LINEAR_PUTPIXEL _linear_putpixel24
#define FUNC_LINEAR_GETPIXEL _linear_getpixel24
#define FUNC_LINEAR_HLINE _linear_hline24
#define FUNC_LINEAR_VLINE _linear_vline24

#define FUNC_LINEAR_DRAW_SPRITE _linear_draw_sprite24
#define FUNC_LINEAR_DRAW_SPRITE_EX _linear_draw_sprite_ex24
#define FUNC_LINEAR_DRAW_256_SPRITE _linear_draw_256_sprite24
#define FUNC_LINEAR_DRAW_SPRITE_V_FLIP _linear_draw_sprite_v_flip24
#define FUNC_LINEAR_DRAW_SPRITE_H_FLIP _linear_draw_sprite_h_flip24
#define FUNC_LINEAR_DRAW_SPRITE_VH_FLIP _linear_draw_sprite_vh_flip24
#define FUNC_LINEAR_DRAW_TRANS_SPRITE _linear_draw_trans_sprite24
#define FUNC_LINEAR_DRAW_TRANS_RGBA_SPRITE _linear_draw_trans_rgba_sprite24
#define FUNC_LINEAR_DRAW_LIT_SPRITE _linear_draw_lit_sprite24
#define FUNC_LINEAR_DRAW_CHARACTER _linear_draw_character24
#define FUNC_LINEAR_DRAW_RLE_SPRITE _linear_draw_rle_sprite24
#define FUNC_LINEAR_DRAW_TRANS_RLE_SPRITE _linear_draw_trans_rle_sprite24
#define FUNC_LINEAR_DRAW_TRANS_RGBA_RLE_SPRITE _linear_draw_trans_rgba_rle_sprite24
#define FUNC_LINEAR_DRAW_LIT_RLE_SPRITE _linear_draw_lit_rle_sprite24

#define FUNC_LINEAR_DRAW_SPRITE_END _linear_draw_sprite24_end
#define FUNC_LINEAR_BLIT_END _linear_blit24_end

#define FUNC_POLY_SCANLINE_GRGB _poly_scanline_grgb24
#define FUNC_POLY_SCANLINE_ATEX _poly_scanline_atex24
#define FUNC_POLY_SCANLINE_ATEX_MASK _poly_scanline_atex_mask24
#define FUNC_POLY_SCANLINE_ATEX_LIT _poly_scanline_atex_lit24
#define FUNC_POLY_SCANLINE_ATEX_MASK_LIT _poly_scanline_atex_mask_lit24
#define FUNC_POLY_SCANLINE_PTEX _poly_scanline_ptex24
#define FUNC_POLY_SCANLINE_PTEX_MASK _poly_scanline_ptex_mask24
#define FUNC_POLY_SCANLINE_PTEX_LIT _poly_scanline_ptex_lit24
#define FUNC_POLY_SCANLINE_PTEX_MASK_LIT _poly_scanline_ptex_mask_lit24
#define FUNC_POLY_SCANLINE_ATEX_TRANS _poly_scanline_atex_trans24
#define FUNC_POLY_SCANLINE_ATEX_MASK_TRANS _poly_scanline_atex_mask_trans24
#define FUNC_POLY_SCANLINE_PTEX_TRANS _poly_scanline_ptex_trans24
#define FUNC_POLY_SCANLINE_PTEX_MASK_TRANS _poly_scanline_ptex_mask_trans24

#define FUNC_POLY_ZBUF_FLAT _poly_zbuf_flat24
#define FUNC_POLY_ZBUF_GRGB _poly_zbuf_grgb24
#define FUNC_POLY_ZBUF_ATEX _poly_zbuf_atex24
#define FUNC_POLY_ZBUF_ATEX_MASK _poly_zbuf_atex_mask24
#define FUNC_POLY_ZBUF_ATEX_LIT _poly_zbuf_atex_lit24
#define FUNC_POLY_ZBUF_ATEX_MASK_LIT _poly_zbuf_atex_mask_lit24
#define FUNC_POLY_ZBUF_PTEX _poly_zbuf_ptex24
#define FUNC_POLY_ZBUF_PTEX_MASK _poly_zbuf_ptex_mask24
#define FUNC_POLY_ZBUF_PTEX_LIT _poly_zbuf_ptex_lit24
#define FUNC_POLY_ZBUF_PTEX_MASK_LIT _poly_zbuf_ptex_mask_lit24
#define FUNC_POLY_ZBUF_ATEX_TRANS _poly_zbuf_atex_trans24
#define FUNC_POLY_ZBUF_ATEX_MASK_TRANS _poly_zbuf_atex_mask_trans24
#define FUNC_POLY_ZBUF_PTEX_TRANS _poly_zbuf_ptex_trans24
#define FUNC_POLY_ZBUF_PTEX_MASK_TRANS _poly_zbuf_ptex_mask_trans24

#undef _bma_scan_gcol
#undef _bma_zbuf_gcol

#include "al_gfx_inline.h"

#endif

/* 32 bit color polygon scanline filler helpers (gouraud shading, tmapping, etc)
 * sprite drawing functions.
 * =============================================================================
 */

#ifdef ALLEGRO_COLOR32

#define PP_DEPTH 32

#define PIXEL_PTR uint32_t *
#define PTR_PER_PIXEL 1
#define OFFSET_PIXEL_PTR(p, x) ((PIXEL_PTR)(p) + (x))
#define INC_PIXEL_PTR(p) ((p)++)
#define INC_PIXEL_PTR_N(p, d) ((p) += d)
#define DEC_PIXEL_PTR(p) ((p)--)

#define PUT_PIXEL(p, c) bmp_write32((uintptr_t)(p), (c))
#define PUT_MEMORY_PIXEL(p, c) (*(p) = (c))
#define PUT_RGB(p, r, g, b) bmp_write32((uintptr_t)(p), makecol32((r), (g), (b)))
#define GET_PIXEL(p) bmp_read32((uintptr_t)(p))
#define GET_MEMORY_PIXEL(p) (*(p))

#define IS_MASK(c) ((unsigned long)(c) == MASK_COLOR_32)
#define IS_SPRITE_MASK(b, c) ((unsigned long)(c) == MASK_COLOR_32)

/* Blender for putpixel (DRAW_MODE_TRANS).  */
#define PP_BLENDER BLENDER_FUNC
#define MAKE_PP_BLENDER(c) _blender_func32
#define PP_BLEND(b, o, n) ((*(b))((n), (o), _blender_alpha))

/* Blender for draw_trans_*_sprite.  */
#define DTS_BLENDER BLENDER_FUNC
#define MAKE_DTS_BLENDER() _blender_func32
#define DTS_BLEND(b, o, n) ((*(b))((n), (o), _blender_alpha))

/* Blender for draw_lit_*_sprite.  */
#define DLS_BLENDER BLENDER_FUNC
#define MAKE_DLS_BLENDER(a) _blender_func32
#define DLS_BLEND(b, a, n) ((*(b))(_blender_col_32, (n), (a)))
#define DLSX_BLEND(b, n) ((*(b))(_blender_col_32, (n), _blender_alpha))

/* Blender for poly_scanline_*_lit.  */
#define PS_BLENDER BLENDER_FUNC
#define MAKE_PS_BLENDER() _blender_func32
#define PS_BLEND(b, o, c) ((*(b))((c), _blender_col_32, (o)))
#define PS_ALPHA_BLEND(b, o, c) ((*(b))((o), (c), _blender_alpha))

#define PATTERN_LINE(y) (PIXEL_PTR)(_drawing_pattern->line[((y) - _drawing_y_anchor) & _drawing_y_mask])
#define GET_PATTERN_PIXEL(x, y) GET_MEMORY_PIXEL(OFFSET_PIXEL_PTR(PATTERN_LINE(y), ((x) - _drawing_x_anchor) & _drawing_x_mask))

#define RLE_PTR int32_t *
#define RLE_IS_EOL(c) ((unsigned long)(c) == MASK_COLOR_32)

#define FUNC_LINEAR_CLEAR_TO_COLOR _linear_clear_to_color32
#define FUNC_LINEAR_BLIT _linear_blit32
#define FUNC_LINEAR_BLIT_BACKWARD _linear_blit_backward32
#define FUNC_LINEAR_MASKED_BLIT _linear_masked_blit32

#define FUNC_LINEAR_PUTPIXEL _linear_putpixel32
#define FUNC_LINEAR_GETPIXEL _linear_getpixel32
#define FUNC_LINEAR_HLINE _linear_hline32
#define FUNC_LINEAR_VLINE _linear_vline32

#define FUNC_LINEAR_DRAW_SPRITE _linear_draw_sprite32
#define FUNC_LINEAR_DRAW_SPRITE_EX _linear_draw_sprite_ex32
#define FUNC_LINEAR_DRAW_256_SPRITE _linear_draw_256_sprite32
#define FUNC_LINEAR_DRAW_SPRITE_V_FLIP _linear_draw_sprite_v_flip32
#define FUNC_LINEAR_DRAW_SPRITE_H_FLIP _linear_draw_sprite_h_flip32
#define FUNC_LINEAR_DRAW_SPRITE_VH_FLIP _linear_draw_sprite_vh_flip32
#define FUNC_LINEAR_DRAW_TRANS_SPRITE _linear_draw_trans_sprite32
#define FUNC_LINEAR_DRAW_TRANS_RGBA_SPRITE _linear_draw_trans_rgba_sprite32
#define FUNC_LINEAR_DRAW_LIT_SPRITE _linear_draw_lit_sprite32
#define FUNC_LINEAR_DRAW_CHARACTER _linear_draw_character32
#define FUNC_LINEAR_DRAW_RLE_SPRITE _linear_draw_rle_sprite32
#define FUNC_LINEAR_DRAW_TRANS_RLE_SPRITE _linear_draw_trans_rle_sprite32
#define FUNC_LINEAR_DRAW_TRANS_RGBA_RLE_SPRITE _linear_draw_trans_rgba_rle_sprite32
#define FUNC_LINEAR_DRAW_LIT_RLE_SPRITE _linear_draw_lit_rle_sprite32

#define FUNC_LINEAR_DRAW_SPRITE_END _linear_draw_sprite32_end
#define FUNC_LINEAR_BLIT_END _linear_blit32_end

#define FUNC_POLY_SCANLINE_GRGB _poly_scanline_grgb32
#define FUNC_POLY_SCANLINE_ATEX _poly_scanline_atex32
#define FUNC_POLY_SCANLINE_ATEX_MASK _poly_scanline_atex_mask32
#define FUNC_POLY_SCANLINE_ATEX_LIT _poly_scanline_atex_lit32
#define FUNC_POLY_SCANLINE_ATEX_MASK_LIT _poly_scanline_atex_mask_lit32
#define FUNC_POLY_SCANLINE_PTEX _poly_scanline_ptex32
#define FUNC_POLY_SCANLINE_PTEX_MASK _poly_scanline_ptex_mask32
#define FUNC_POLY_SCANLINE_PTEX_LIT _poly_scanline_ptex_lit32
#define FUNC_POLY_SCANLINE_PTEX_MASK_LIT _poly_scanline_ptex_mask_lit32
#define FUNC_POLY_SCANLINE_ATEX_TRANS _poly_scanline_atex_trans32
#define FUNC_POLY_SCANLINE_ATEX_MASK_TRANS _poly_scanline_atex_mask_trans32
#define FUNC_POLY_SCANLINE_PTEX_TRANS _poly_scanline_ptex_trans32
#define FUNC_POLY_SCANLINE_PTEX_MASK_TRANS _poly_scanline_ptex_mask_trans32

#define FUNC_POLY_ZBUF_FLAT _poly_zbuf_flat32
#define FUNC_POLY_ZBUF_GRGB _poly_zbuf_grgb32
#define FUNC_POLY_ZBUF_ATEX _poly_zbuf_atex32
#define FUNC_POLY_ZBUF_ATEX_MASK _poly_zbuf_atex_mask32
#define FUNC_POLY_ZBUF_ATEX_LIT _poly_zbuf_atex_lit32
#define FUNC_POLY_ZBUF_ATEX_MASK_LIT _poly_zbuf_atex_mask_lit32
#define FUNC_POLY_ZBUF_PTEX _poly_zbuf_ptex32
#define FUNC_POLY_ZBUF_PTEX_MASK _poly_zbuf_ptex_mask32
#define FUNC_POLY_ZBUF_PTEX_LIT _poly_zbuf_ptex_lit32
#define FUNC_POLY_ZBUF_PTEX_MASK_LIT _poly_zbuf_ptex_mask_lit32
#define FUNC_POLY_ZBUF_ATEX_TRANS _poly_zbuf_atex_trans32
#define FUNC_POLY_ZBUF_ATEX_MASK_TRANS _poly_zbuf_atex_mask_trans32
#define FUNC_POLY_ZBUF_PTEX_TRANS _poly_zbuf_ptex_trans32
#define FUNC_POLY_ZBUF_PTEX_MASK_TRANS _poly_zbuf_ptex_mask_trans32

#undef _bma_zbuf_gcol
#undef _bma_zbuf_gcol

#include "al_gfx_inline.h"

#endif

/*
 * Interpolation routines for hicolor and truecolor pixels.
 * ========================================================
 */

#define BLEND(bpp, r, g, b) _blender_trans##bpp(makecol##bpp(r, g, b), y, n)
#define T(x, y, n) (((y) - (x)) * (n) / 255 + (x))

/* _blender_black:
 *  Fallback routine for when we don't have anything better to do.
 */
unsigned long _blender_black(unsigned long x, unsigned long y, unsigned long n) { return 0; }

/*
 * List of available bitmap vtables, kept in a seperate file so that
 * they can be overriden by user programs.
 * =================================================================
 */

#ifndef ALLEGRO_COLOR8
#undef COLOR_DEPTH_8
#define COLOR_DEPTH_8
#endif

#ifndef ALLEGRO_COLOR16
#undef COLOR_DEPTH_15
#undef COLOR_DEPTH_16
#define COLOR_DEPTH_15
#define COLOR_DEPTH_16
#endif

#ifndef ALLEGRO_COLOR24
#undef COLOR_DEPTH_24
#define COLOR_DEPTH_24
#endif

#ifndef ALLEGRO_COLOR32
#undef COLOR_DEPTH_32
#define COLOR_DEPTH_32
#endif

BEGIN_COLOR_DEPTH_LIST
COLOR_DEPTH_8
COLOR_DEPTH_15
COLOR_DEPTH_16
COLOR_DEPTH_24
COLOR_DEPTH_32
END_COLOR_DEPTH_LIST

/*
 * Table of functions for drawing onto 8 bit linear bitmaps.
 * =========================================================
 */

#ifdef ALLEGRO_COLOR8

void _linear_draw_sprite8_end(void);
void _linear_blit8_end(void);

GFX_VTABLE __linear_vtable8 = {
	8,
	MASK_COLOR_8,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	_linear_getpixel8,
	_linear_putpixel8,
	_linear_vline8,
	_linear_hline8,
	_linear_hline8,
	_normal_line,
	_fast_line,
	_normal_rectfill,
	_soft_triangle,
	_linear_draw_sprite8,
	_linear_draw_sprite8,
	_linear_draw_sprite_v_flip8,
	_linear_draw_sprite_h_flip8,
	_linear_draw_sprite_vh_flip8,
	_linear_draw_trans_sprite8,
	NULL,
	_linear_draw_lit_sprite8,
	_linear_draw_rle_sprite8,
	_linear_draw_trans_rle_sprite8,
	NULL,
	_linear_draw_lit_rle_sprite8,
	_linear_draw_character8,
	_linear_draw_glyph8,
	_linear_blit8,
	_linear_blit8,
	_linear_blit8,
	_linear_blit8,
	_linear_blit8,
	_linear_blit8,
	_linear_blit_backward8,
	_blit_between_formats,
	_linear_masked_blit8,
	_linear_clear_to_color8,
	_pivot_scaled_sprite_flip,
	NULL, // do_stretch_blit
	_soft_draw_gouraud_sprite,
	_linear_draw_sprite8_end,
	_linear_blit8_end,
	_soft_polygon,
	_soft_rect,
	_soft_circle,
	_soft_circlefill,
	_soft_ellipse,
	_soft_ellipsefill,
	_soft_arc,
	_soft_spline,
	_soft_floodfill,

	_soft_polygon3d,
	_soft_polygon3d_f,
	_soft_triangle3d,
	_soft_triangle3d_f,
	_soft_quad3d,
	_soft_quad3d_f,
	_linear_draw_sprite_ex8
};

#endif

/*
 * Table of functions for drawing onto 15 bit linear bitmaps.
 * ==========================================================
 */

#ifdef ALLEGRO_COLOR16

void _linear_draw_sprite16_end(void);
void _linear_blit16_end(void);

GFX_VTABLE __linear_vtable15 = {
	15,
	MASK_COLOR_15,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	_linear_getpixel16,
	_linear_putpixel15,
	_linear_vline15,
	_linear_hline15,
	_linear_hline15,
	_normal_line,
	_fast_line,
	_normal_rectfill,
	_soft_triangle,
	_linear_draw_sprite16,
	_linear_draw_256_sprite16,
	_linear_draw_sprite_v_flip16,
	_linear_draw_sprite_h_flip16,
	_linear_draw_sprite_vh_flip16,
	_linear_draw_trans_sprite15,
	_linear_draw_trans_rgba_sprite15,
	_linear_draw_lit_sprite15,
	_linear_draw_rle_sprite15,
	_linear_draw_trans_rle_sprite15,
	_linear_draw_trans_rgba_rle_sprite15,
	_linear_draw_lit_rle_sprite15,
	_linear_draw_character16,
	_linear_draw_glyph16,
	_linear_blit16,
	_linear_blit16,
	_linear_blit16,
	_linear_blit16,
	_linear_blit16,
	_linear_blit16,
	_linear_blit_backward16,
	_blit_between_formats,
	_linear_masked_blit16,
	_linear_clear_to_color16,
	_pivot_scaled_sprite_flip,
	NULL, // do_stretch_blit
	_soft_draw_gouraud_sprite,
	_linear_draw_sprite16_end,
	_linear_blit16_end,
	_soft_polygon,
	_soft_rect,
	_soft_circle,
	_soft_circlefill,
	_soft_ellipse,
	_soft_ellipsefill,
	_soft_arc,
	_soft_spline,
	_soft_floodfill,

	_soft_polygon3d,
	_soft_polygon3d_f,
	_soft_triangle3d,
	_soft_triangle3d_f,
	_soft_quad3d,
	_soft_quad3d_f,
	_linear_draw_sprite_ex16
};

#endif

/*
 * Table of functions for drawing onto 16 bit linear bitmaps.
 * ==========================================================
 */

#ifdef ALLEGRO_COLOR16

void _linear_draw_sprite16_end(void);
void _linear_blit16_end(void);

GFX_VTABLE __linear_vtable16 = {
	16,
	MASK_COLOR_16,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	_linear_getpixel16,
	_linear_putpixel16,
	_linear_vline16,
	_linear_hline16,
	_linear_hline16,
	_normal_line,
	_fast_line,
	_normal_rectfill,
	_soft_triangle,
	_linear_draw_sprite16,
	_linear_draw_256_sprite16,
	_linear_draw_sprite_v_flip16,
	_linear_draw_sprite_h_flip16,
	_linear_draw_sprite_vh_flip16,
	_linear_draw_trans_sprite16,
	_linear_draw_trans_rgba_sprite16,
	_linear_draw_lit_sprite16,
	_linear_draw_rle_sprite16,
	_linear_draw_trans_rle_sprite16,
	_linear_draw_trans_rgba_rle_sprite16,
	_linear_draw_lit_rle_sprite16,
	_linear_draw_character16,
	_linear_draw_glyph16,
	_linear_blit16,
	_linear_blit16,
	_linear_blit16,
	_linear_blit16,
	_linear_blit16,
	_linear_blit16,
	_linear_blit_backward16,
	_blit_between_formats,
	_linear_masked_blit16,
	_linear_clear_to_color16,
	_pivot_scaled_sprite_flip,
	NULL, // AL_METHOD(void, do_stretch_blit, (struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int source_width, int source_height, int dest_x, int dest_y, int dest_width, int dest_height, int masked))
	_soft_draw_gouraud_sprite,
	_linear_draw_sprite16_end,
	_linear_blit16_end,
	_soft_polygon,
	_soft_rect,
	_soft_circle,
	_soft_circlefill,
	_soft_ellipse,
	_soft_ellipsefill,
	_soft_arc,
	_soft_spline,
	_soft_floodfill,

	_soft_polygon3d,
	_soft_polygon3d_f,
	_soft_triangle3d,
	_soft_triangle3d_f,
	_soft_quad3d,
	_soft_quad3d_f,
	_linear_draw_sprite_ex16
};

#endif

/*
 * Table of functions for drawing onto 24 bit linear bitmaps.
 * ==========================================================
 */

#ifdef ALLEGRO_COLOR24

void _linear_draw_sprite24_end(void);
void _linear_blit24_end(void);

GFX_VTABLE __linear_vtable24 = {
	24,
	MASK_COLOR_24,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	_linear_getpixel24,
	_linear_putpixel24,
	_linear_vline24,
	_linear_hline24,
	_linear_hline24,
	_normal_line,
	_fast_line,
	_normal_rectfill,
	_soft_triangle,
	_linear_draw_sprite24,
	_linear_draw_256_sprite24,
	_linear_draw_sprite_v_flip24,
	_linear_draw_sprite_h_flip24,
	_linear_draw_sprite_vh_flip24,
	_linear_draw_trans_sprite24,
	_linear_draw_trans_rgba_sprite24,
	_linear_draw_lit_sprite24,
	_linear_draw_rle_sprite24,
	_linear_draw_trans_rle_sprite24,
	_linear_draw_trans_rgba_rle_sprite24,
	_linear_draw_lit_rle_sprite24,
	_linear_draw_character24,
	_linear_draw_glyph24,
	_linear_blit24,
	_linear_blit24,
	_linear_blit24,
	_linear_blit24,
	_linear_blit24,
	_linear_blit24,
	_linear_blit_backward24,
	_blit_between_formats,
	_linear_masked_blit24,
	_linear_clear_to_color24,
	_pivot_scaled_sprite_flip,
	NULL, // AL_METHOD(void, do_stretch_blit, (struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int source_width, int source_height, int dest_x, int dest_y, int dest_width, int dest_height, int masked))
	_soft_draw_gouraud_sprite,
	_linear_draw_sprite24_end,
	_linear_blit24_end,
	_soft_polygon,
	_soft_rect,
	_soft_circle,
	_soft_circlefill,
	_soft_ellipse,
	_soft_ellipsefill,
	_soft_arc,
	_soft_spline,
	_soft_floodfill,

	_soft_polygon3d,
	_soft_polygon3d_f,
	_soft_triangle3d,
	_soft_triangle3d_f,
	_soft_quad3d,
	_soft_quad3d_f,
	_linear_draw_sprite_ex24
};

#endif

/*
 * Table of functions for drawing onto 32 bit linear bitmaps.
 * ==========================================================
 */

#ifdef ALLEGRO_COLOR32

void _linear_draw_sprite32_end(void);
void _linear_blit32_end(void);

GFX_VTABLE __linear_vtable32 = {
	32,
	MASK_COLOR_32,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	_linear_getpixel32,
	_linear_putpixel32,
	_linear_vline32,
	_linear_hline32,
	_linear_hline32,
	_normal_line,
	_fast_line,
	_normal_rectfill,
	_soft_triangle,
	_linear_draw_sprite32,
	_linear_draw_256_sprite32,
	_linear_draw_sprite_v_flip32,
	_linear_draw_sprite_h_flip32,
	_linear_draw_sprite_vh_flip32,
	_linear_draw_trans_sprite32,
	_linear_draw_trans_sprite32,
	_linear_draw_lit_sprite32,
	_linear_draw_rle_sprite32,
	_linear_draw_trans_rle_sprite32,
	_linear_draw_trans_rle_sprite32,
	_linear_draw_lit_rle_sprite32,
	_linear_draw_character32,
	_linear_draw_glyph32,
	_linear_blit32,
	_linear_blit32,
	_linear_blit32,
	_linear_blit32,
	_linear_blit32,
	_linear_blit32,
	_linear_blit_backward32,
	_blit_between_formats,
	_linear_masked_blit32,
	_linear_clear_to_color32,
	_pivot_scaled_sprite_flip,
	NULL, // AL_METHOD(void, do_stretch_blit, (struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int source_width, int source_height, int dest_x, int dest_y, int dest_width, int dest_height, int masked))
	_soft_draw_gouraud_sprite,
	_linear_draw_sprite32_end,
	_linear_blit32_end,
	_soft_polygon,
	_soft_rect,
	_soft_circle,
	_soft_circlefill,
	_soft_ellipse,
	_soft_ellipsefill,
	_soft_arc,
	_soft_spline,
	_soft_floodfill,

	_soft_polygon3d,
	_soft_polygon3d_f,
	_soft_triangle3d,
	_soft_triangle3d_f,
	_soft_quad3d,
	_soft_quad3d_f,
	_linear_draw_sprite_ex32
};

#endif

/*
 * Top level bitmap reading routines.
 * TGA reader.
 * ==================================
 */

/* _fixup_loaded_bitmap:
 *  Helper function for adjusting the color depth of a loaded image.
 *  Converts the bitmap BMP to the color depth BPP. If BMP is a 8-bit
 *  bitmap, PAL must be the palette attached to the bitmap. If BPP is
 *  equal to 8, the conversion is performed either by building a palette
 *  optimized for the bitmap if PAL is not NULL (in which case PAL gets
 *  filled in with this palette) or by using the current palette if PAL
 *  is NULL. In any other cases, PAL is unused.
 */
BITMAP *_fixup_loaded_bitmap(BITMAP *bmp, PALETTE pal, int bpp) {
	ASSERT(bmp);

	BITMAP *b2 = create_bitmap_ex(bpp, bmp->w, bmp->h);
	if (!b2) {
		destroy_bitmap(bmp);
		return NULL;
	}

	if (bpp == 8) {
		RGB_MAP *old_map = rgb_map;

		if (pal)
			generate_optimized_palette(bmp, pal, NULL);
		else
			pal = _current_palette;

		rgb_map = _AL_MALLOC(sizeof(RGB_MAP));
		if (rgb_map != NULL)
			create_rgb_table(rgb_map, pal, NULL);

		blit(bmp, b2, 0, 0, 0, 0, bmp->w, bmp->h);

		if (rgb_map != NULL)
			_AL_FREE(rgb_map);
		rgb_map = old_map;
	} else if (bitmap_color_depth(bmp) == 8) {
		select_palette(pal);
		blit(bmp, b2, 0, 0, 0, 0, bmp->w, bmp->h);
		unselect_palette();
	} else {
		blit(bmp, b2, 0, 0, 0, 0, bmp->w, bmp->h);
	}

	destroy_bitmap(bmp);

	return b2;
}

/* raw_tga_read8:
 *  Helper for reading 256-color raw data from TGA files.
 */
static INLINE unsigned char *raw_tga_read8(unsigned char *b, int w, struct AL_FILE *f) { return b + al_io_fread(b, w, f); }

/* rle_tga_read8:
 *  Helper for reading 256-color RLE data from TGA files.
 */
static void rle_tga_read8(unsigned char *b, int w, struct AL_FILE *f) {
	int value, c = 0;

	do {
		int count = al_io_getc(f);
		if (count & 0x80) {
			/* run-length packet */
			count = (count & 0x7F) + 1;
			c += count;
			value = al_io_getc(f);
			while (count--)
				*b++ = value;
		} else {
			/* raw packet */
			count++;
			c += count;
			b = raw_tga_read8(b, count, f);
		}
	} while (c < w);
}

/* single_tga_read32:
 *  Helper for reading a single 32-bit data from TGA files.
 */
static INLINE int single_tga_read32(struct AL_FILE *f) {
	RGB value;
	int alpha;

	value.b = al_io_getc(f);
	value.g = al_io_getc(f);
	value.r = al_io_getc(f);
	alpha = al_io_getc(f);

	return makeacol32(value.r, value.g, value.b, alpha);
}

/* raw_tga_read32:
 *  Helper for reading 32-bit raw data from TGA files.
 */
static unsigned int *raw_tga_read32(unsigned int *b, int w, struct AL_FILE *f) {
	while (w--)
		*b++ = single_tga_read32(f);
	return b;
}

/* rle_tga_read32:
 *  Helper for reading 32-bit RLE data from TGA files.
 */
static void rle_tga_read32(unsigned int *b, int w, struct AL_FILE *f) {
	int color, c = 0;

	do {
		int count = al_io_getc(f);
		if (count & 0x80) {
			/* run-length packet */
			count = (count & 0x7F) + 1;
			c += count;
			color = single_tga_read32(f);
			while (count--)
				*b++ = color;
		} else {
			/* raw packet */
			count++;
			c += count;
			b = raw_tga_read32(b, count, f);
		}
	} while (c < w);
}

/* single_tga_read24:
 *  Helper for reading a single 24-bit data from TGA files.
 */
static INLINE int single_tga_read24(struct AL_FILE *f) {
	RGB value;

	value.b = al_io_getc(f);
	value.g = al_io_getc(f);
	value.r = al_io_getc(f);

	return makecol24(value.r, value.g, value.b);
}

/* raw_tga_read24:
 *  Helper for reading 24-bit raw data from TGA files.
 */
static unsigned char *raw_tga_read24(unsigned char *b, int w, struct AL_FILE *f) {
	while (w--) {
		int color = single_tga_read24(f);
		WRITE3BYTES(b, color);
		b += 3;
	}

	return b;
}

/* rle_tga_read24:
 *  Helper for reading 24-bit RLE data from TGA files.
 */
static void rle_tga_read24(unsigned char *b, int w, struct AL_FILE *f) {
	int color, c = 0;

	do {
		int count = al_io_getc(f);
		if (count & 0x80) {
			/* run-length packet */
			count = (count & 0x7F) + 1;
			c += count;
			color = single_tga_read24(f);
			while (count--) {
				WRITE3BYTES(b, color);
				b += 3;
			}
		} else {
			/* raw packet */
			count++;
			c += count;
			b = raw_tga_read24(b, count, f);
		}
	} while (c < w);
}

/* single_tga_read16:
 *  Helper for reading a single 16-bit data from TGA files.
 */
static INLINE int single_tga_read16(struct AL_FILE *f) {
	int value = al_io_igetw(f);
	return (((value >> 10) & 0x1F) << _rgb_r_shift_15) |
			(((value >> 5) & 0x1F) << _rgb_g_shift_15) |
			((value & 0x1F) << _rgb_b_shift_15);
}

/* raw_tga_read16:
 *  Helper for reading 16-bit raw data from TGA files.
 */
static unsigned short *raw_tga_read16(unsigned short *b, int w, struct AL_FILE *f) {
	while (w--)
		*b++ = single_tga_read16(f);

	return b;
}

/* rle_tga_read16:
 *  Helper for reading 16-bit RLE data from TGA files.
 */
static void rle_tga_read16(unsigned short *b, int w, struct AL_FILE *f) {
	int color, count, c = 0;

	do {
		count = al_io_getc(f);
		if (count & 0x80) {
			/* run-length packet */
			count = (count & 0x7F) + 1;
			c += count;
			color = single_tga_read16(f);
			while (count--)
				*b++ = color;
		} else {
			/* raw packet */
			count++;
			c += count;
			b = raw_tga_read16(b, count, f);
		}
	} while (c < w);
}

/* load_tga:
 *  Loads a TGA file, returning a bitmap structure and storing the
 *  palette data in the specified palette (this should be an array
 *  of at least 256 RGB structures).
 */
BITMAP *load_tga(AL_CONST char *filename, RGB *pal) {
	ASSERT(filename);

	struct AL_FILE *f = al_io_fopen(filename, F_READ);
	if (!f)
		return NULL;

	BITMAP *bmp = load_tga_pf(f, pal);

	al_io_fclose(f);

	return bmp;
}

/* load_tga_pf:
 *  Like load_tga, but starts loading from the current place in the PACKFILE
 *  specified. If successful the offset into the file will be left just after
 *  the image data. If unsuccessful the offset into the file is unspecified,
 *  i.e. you must either reset the offset to some known place or close the
 *  packfile. The packfile is not closed by this function.
 */
BITMAP *load_tga_pf(struct AL_FILE *f, RGB *pal) {
	unsigned char image_id[256], image_palette[256][3];
	unsigned char id_length, palette_type, image_type, palette_entry_size;
	unsigned char bpp, descriptor_bits;
	short unsigned int palette_colors;
	short unsigned int image_width, image_height;
	unsigned int c, i, y, yc;
	int dest_depth;
	int compressed;
	BITMAP *bmp;
	PALETTE tmppal;
	int want_palette = TRUE;
	ASSERT(f);

	/* we really need a palette */
	if (!pal) {
		want_palette = FALSE;
		pal = tmppal;
	}

	id_length = al_io_getc(f);
	palette_type = al_io_getc(f);
	image_type = al_io_getc(f);
	/* first_color */ al_io_igetw(f);
	palette_colors = al_io_igetw(f);
	palette_entry_size = al_io_getc(f);
	/* left */ al_io_igetw(f);
	/* top  */ al_io_igetw(f);
	image_width = al_io_igetw(f);
	image_height = al_io_igetw(f);
	bpp = al_io_getc(f);
	descriptor_bits = al_io_getc(f);

	al_io_fread(image_id, id_length, f);

	if (palette_type == 1) {
		for (i = 0; i < palette_colors; i++) {
			switch (palette_entry_size) {
				case 16:
					c = al_io_igetw(f);
					image_palette[i][0] = (c & 0x1F) << 3;
					image_palette[i][1] = ((c >> 5) & 0x1F) << 3;
					image_palette[i][2] = ((c >> 10) & 0x1F) << 3;
					break;

				case 24:
				case 32:
					image_palette[i][0] = al_io_getc(f);
					image_palette[i][1] = al_io_getc(f);
					image_palette[i][2] = al_io_getc(f);
					if (palette_entry_size == 32)
						al_io_getc(f);
					break;
			}
		}
	} else if (palette_type != 0) {
		return NULL;
	}

	/* Image type:
	 *    0 = no image data
	 *    1 = uncompressed color mapped
	 *    2 = uncompressed true color
	 *    3 = grayscale
	 *    9 = RLE color mapped
	 *   10 = RLE true color
	 *   11 = RLE grayscale
	 */
	compressed = (image_type & 8);
	image_type &= 7;

	if ((image_type < 1) || (image_type > 3)) {
		return NULL;
	}

	switch (image_type) {
		case 1:
			/* paletted image */
			if ((palette_type != 1) || (bpp != 8)) {
				return NULL;
			}

			for (i = 0; i < palette_colors; i++) {
				pal[i].r = image_palette[i][2] >> 2;
				pal[i].g = image_palette[i][1] >> 2;
				pal[i].b = image_palette[i][0] >> 2;
			}

			dest_depth = _color_load_depth(8, FALSE);
			break;

		case 2:
			/* truecolor image */
			if ((palette_type == 0) && ((bpp == 15) || (bpp == 16))) {
				bpp = 15;
				dest_depth = _color_load_depth(15, FALSE);
			} else if ((palette_type == 0) && ((bpp == 24) || (bpp == 32))) {
				dest_depth = _color_load_depth(bpp, (bpp == 32));
			} else {
				return NULL;
			}
			break;

		case 3:
			/* grayscale image */
			if ((palette_type != 0) || (bpp != 8)) {
				return NULL;
			}

			for (i = 0; i < 256; i++) {
				pal[i].r = i >> 2;
				pal[i].g = i >> 2;
				pal[i].b = i >> 2;
			}

			dest_depth = _color_load_depth(bpp, FALSE);
			break;

		default:
			return NULL;
	}

	bmp = create_bitmap_ex(bpp, image_width, image_height);
	if (!bmp) {
		return NULL;
	}

	*allegro_errno = 0;

	for (y = image_height; y; y--) {
		yc = (descriptor_bits & 0x20) ? image_height - y : y - 1;

		switch (image_type) {
			case 1:
			case 3:
				if (compressed)
					rle_tga_read8(bmp->line[yc], image_width, f);
				else
					raw_tga_read8(bmp->line[yc], image_width, f);
				break;

			case 2:
				if (bpp == 32) {
					if (compressed)
						rle_tga_read32((unsigned int *)bmp->line[yc], image_width, f);
					else
						raw_tga_read32((unsigned int *)bmp->line[yc], image_width, f);
				} else if (bpp == 24) {
					if (compressed)
						rle_tga_read24(bmp->line[yc], image_width, f);
					else
						raw_tga_read24(bmp->line[yc], image_width, f);
				} else {
					if (compressed)
						rle_tga_read16((unsigned short *)bmp->line[yc], image_width, f);
					else
						raw_tga_read16((unsigned short *)bmp->line[yc], image_width, f);
				}
				break;
		}
	}

	if (*allegro_errno) {
		destroy_bitmap(bmp);
		return NULL;
	}

	if (dest_depth != bpp) {
		/* restore original palette except if it comes from the bitmap */
		if ((bpp != 8) && (!want_palette))
			pal = NULL;

		bmp = _fixup_loaded_bitmap(bmp, pal, dest_depth);
	}

	/* construct a fake palette if 8-bit mode is not involved */
	if ((bpp != 8) && (dest_depth != 8) && want_palette)
		generate_332_palette(pal);

	return bmp;
}

/* save_tga:
 *  Writes a bitmap into a TGA file, using the specified palette (this
 *  should be an array of at least 256 RGB structures).
 */
int save_tga(AL_CONST char *filename, BITMAP *bmp, AL_CONST RGB *pal) {
	ASSERT(filename);

	struct AL_FILE *f = al_io_fopen(filename, F_WRITE);
	if (f == NULL)
		return -1;

	int ret = save_tga_pf(f, bmp, pal);

	al_io_fclose(f);

	return ret;
}

/* save_tga_pf:
 *  Like save_tga but writes into the PACKFILE given instead of a new file.
 *  The packfile is not closed after writing is completed. On success the
 *  offset into the file is left after the TGA file just written. On failure
 *  the offset is left at the end of whatever incomplete data was written.
 */
int save_tga_pf(struct AL_FILE *f, BITMAP *bmp, AL_CONST RGB *pal) {
	unsigned char image_palette[256][3];
	PALETTE tmppal;
	ASSERT(f);
	ASSERT(bmp);

	if (!pal) {
		get_palette(tmppal);
		pal = tmppal;
	}

	int depth = bitmap_color_depth(bmp);

	if (depth == 15)
		depth = 16;

	*allegro_errno = 0;

	al_io_putc(0, f); /* id length (no id saved) */
	al_io_putc((depth == 8) ? 1 : 0, f); /* palette type */
	al_io_putc((depth == 8) ? 1 : 2, f); /* image type */
	al_io_iputw(0, f); /* first colour */
	al_io_iputw((depth == 8) ? 256 : 0, f); /* number of colours */
	al_io_putc((depth == 8) ? 24 : 0, f); /* palette entry size */
	al_io_iputw(0, f); /* left */
	al_io_iputw(0, f); /* top */
	al_io_iputw(bmp->w, f); /* width */
	al_io_iputw(bmp->h, f); /* height */
	al_io_putc(depth, f); /* bits per pixel */
	al_io_putc(_bitmap_has_alpha(bmp) ? 8 : 0, f); /* descriptor (bottom to top, 8-bit alpha) */

	if (depth == 8) {
		for (int y = 0; y < 256; y++) {
			image_palette[y][2] = _rgb_scale_6[pal[y].r];
			image_palette[y][1] = _rgb_scale_6[pal[y].g];
			image_palette[y][0] = _rgb_scale_6[pal[y].b];
		}

		al_io_fwrite(image_palette, 768, f);
	}

	switch (bitmap_color_depth(bmp)) {
#ifdef ALLEGRO_COLOR8
		case 8:
			for (int y = bmp->h; y; y--)
				for (int x = 0; x < bmp->w; x++)
					al_io_putc(getpixel(bmp, x, y - 1), f);
			break;

#endif

#ifdef ALLEGRO_COLOR16
		case 15:
			for (int y = bmp->h; y; y--) {
				for (int x = 0; x < bmp->w; x++) {
					int c = getpixel(bmp, x, y - 1);
					int r = getr15(c);
					int g = getg15(c);
					int b = getb15(c);
					c = ((r << 7) & 0x7C00) | ((g << 2) & 0x3E0) | ((b >> 3) & 0x1F);
					al_io_iputw(c, f);
				}
			}
			break;
		case 16:
			for (int y = bmp->h; y; y--) {
				for (int x = 0; x < bmp->w; x++) {
					int c = getpixel(bmp, x, y - 1);
					int r = getr16(c);
					int g = getg16(c);
					int b = getb16(c);
					c = ((r << 7) & 0x7C00) | ((g << 2) & 0x3E0) | ((b >> 3) & 0x1F);
					al_io_iputw(c, f);
				}
			}
			break;

#endif

#ifdef ALLEGRO_COLOR24
		case 24:
			for (int y = bmp->h; y; y--) {
				for (int x = 0; x < bmp->w; x++) {
					int c = getpixel(bmp, x, y - 1);
					al_io_putc(getb24(c), f);
					al_io_putc(getg24(c), f);
					al_io_putc(getr24(c), f);
				}
			}
			break;

#endif

#ifdef ALLEGRO_COLOR32
		case 32:
			for (int y = bmp->h; y; y--) {
				for (int x = 0; x < bmp->w; x++) {
					int c = getpixel(bmp, x, y - 1);
					al_io_putc(getb32(c), f);
					al_io_putc(getg32(c), f);
					al_io_putc(getr32(c), f);
					al_io_putc(geta32(c), f);
				}
			}
			break;

#endif
	}

	if (*allegro_errno)
		return -1;
	else
		return 0;
}

/*
 * File I/O.
 * =========
 */

/* posix/stdio implementation */

typedef struct AL_FILE {
} AL_FILE;

inline static int stdio_fclose(void *userdata) {
	FILE *fp = userdata;
	return fclose(fp);
}

inline static int stdio_seek(void *userdata, int n) {
	FILE *fp = userdata;
	return fseek(fp, n, SEEK_CUR);
}

inline static int stdio_feof(void *userdata) {
	FILE *fp = userdata;
	return feof(fp);
}

inline static int stdio_getc(void *userdata) {
	FILE *fp = userdata;
	return fgetc(fp);
}

inline static int stdio_ungetc(int c, void *userdata) {
	FILE *fp = userdata;
	return ungetc(c, fp);
}

inline static long stdio_fread(void *p, long n, void *userdata) {
	FILE *fp = userdata;
	return fread(p, 1, n, fp);
}

inline static long stdio_fwrite(AL_CONST void *p, long n, void *userdata) {
	FILE *fp = userdata;
	return fwrite(p, 1, n, fp);
}

inline static int stdio_putc(int c, void *userdata) {
	FILE *fp = userdata;
	return fputc(c, fp);
}

#define _al_open(file, mode) ((AL_FILE *)fopen(file, mode))

AL_FILE *al_io_fopen(AL_CONST char *filename, AL_CONST char *mode) {
	ASSERT(filename);

	AL_FILE *fd = _al_open(filename, mode);

	if (fd == NULL) {
		*allegro_errno = errno;
		return NULL;
	}

	return fd;
}

int al_io_fclose(AL_FILE *f) { return stdio_fclose(f); }
int al_io_fseek(AL_FILE *f, int offset) { return stdio_seek(f, offset); }
int al_io_getc(struct AL_FILE *f) { return stdio_getc(f); }
int al_io_putc(int c, AL_FILE *f) { return stdio_putc(c, f); }
long al_io_fread(void *p, long n, struct AL_FILE *f) { return stdio_fread(p, n, f); }
long al_io_fwrite(AL_CONST void *p, long n, struct AL_FILE *f) { return stdio_fwrite(p, n, f); }

/* pack_igetw:
 *  Reads a 16 bit word from a file, using intel byte ordering.
 */
int al_io_igetw(AL_FILE *f) {
	int b1, b2;
	ASSERT(f);

	if ((b1 = al_io_getc(f)) != EOF)
		if ((b2 = al_io_getc(f)) != EOF)
			return ((b2 << 8) | b1);

	return EOF;
}

/* pack_iputw:
 *  Writes a 16 bit int to a file, using intel byte ordering.
 */
int al_io_iputw(int w, AL_FILE *f) {
	ASSERT(f);

	int b1 = (w & 0xFF00) >> 8;
	int b2 = w & 0x00FF;

	if (al_io_putc(b2, f) == b2)
		if (al_io_putc(b1, f) == b1)
			return w;

	return EOF;
}

/*
 * String handling functions (UTF-8, Unicode, ASCII, etc).
 * =======================================================
 */

/* ascii_getc:
 *  Reads a character from an ASCII string.
 */
static int ascii_getc(AL_CONST char *s) { return *((unsigned char *)s); }

/* ascii_getx:
 *  Reads a character from an ASCII string, advancing the pointer position.
 */
static int ascii_getx(char **s) { return *((unsigned char *)((*s)++)); }

/* ascii_setc:
 *  Sets a character in an ASCII string.
 */
static int ascii_setc(char *s, int c) {
	*s = c;
	return 1;
}

/* ascii_width:
 *  Returns the width of an ASCII character.
 */
static int ascii_width(AL_CONST char *s) { return 1; }

/* ascii_cwidth:
 *  Returns the width of an ASCII character.
 */
static int ascii_cwidth(int c) { return 1; }

/* ascii_isok:
 *  Checks whether this character can be encoded in 8-bit ASCII format.
 */
static int ascii_isok(int c) { return ((c >= 0) && (c <= 255)); }

/* lookup table for implementing 8-bit codepage modes */
static unsigned short _codepage_table[] = {
	0x00,
	0x01,
	0x02,
	0x03,
	0x04,
	0x05,
	0x06,
	0x07,
	0x08,
	0x09,
	0x0A,
	0x0B,
	0x0C,
	0x0D,
	0x0E,
	0x0F,
	0x10,
	0x11,
	0x12,
	0x13,
	0x14,
	0x15,
	0x16,
	0x17,
	0x18,
	0x19,
	0x1A,
	0x1B,
	0x1C,
	0x1D,
	0x1E,
	0x1F,
	0x20,
	0x21,
	0x22,
	0x23,
	0x24,
	0x25,
	0x26,
	0x27,
	0x28,
	0x29,
	0x2A,
	0x2B,
	0x2C,
	0x2D,
	0x2E,
	0x2F,
	0x30,
	0x31,
	0x32,
	0x33,
	0x34,
	0x35,
	0x36,
	0x37,
	0x38,
	0x39,
	0x3A,
	0x3B,
	0x3C,
	0x3D,
	0x3E,
	0x3F,
	0x40,
	0x41,
	0x42,
	0x43,
	0x44,
	0x45,
	0x46,
	0x47,
	0x48,
	0x49,
	0x4A,
	0x4B,
	0x4C,
	0x4D,
	0x4E,
	0x4F,
	0x50,
	0x51,
	0x52,
	0x53,
	0x54,
	0x55,
	0x56,
	0x57,
	0x58,
	0x59,
	0x5A,
	0x5B,
	0x5C,
	0x5D,
	0x5E,
	0x5F,
	0x60,
	0x61,
	0x62,
	0x63,
	0x64,
	0x65,
	0x66,
	0x67,
	0x68,
	0x69,
	0x6A,
	0x6B,
	0x6C,
	0x6D,
	0x6E,
	0x6F,
	0x70,
	0x71,
	0x72,
	0x73,
	0x74,
	0x75,
	0x76,
	0x77,
	0x78,
	0x79,
	0x7A,
	0x7B,
	0x7C,
	0x7D,
	0x7E,
	0x7F,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
	0x5E,
};

/* this default table reduces Latin-1 and Extended-A characters to 7 bits */
static unsigned short _codepage_extras[] = {
	0xA1, '!', 0xA2, 'c', 0xA3, '#', 0xB5, 'u', 0xBF, '?', 0xC0, 'A',
	0xC1, 'A', 0xC2, 'A', 0xC3, 'A', 0xC4, 'A', 0xC5, 'A', 0xC6, 'A',
	0xC7, 'C', 0xC8, 'E', 0xC9, 'E', 0xCA, 'E', 0xCB, 'E', 0xCC, 'I',
	0xCD, 'I', 0xCE, 'I', 0xCF, 'I', 0xD0, 'D', 0xD1, 'N', 0xD2, 'O',
	0xD3, 'O', 0xD4, 'O', 0xD5, 'O', 0xD6, 'O', 0xD7, 'X', 0xD8, '0',
	0xD9, 'U', 0xDA, 'U', 0xDB, 'U', 0xDC, 'U', 0xDD, 'Y', 0xDE, 'P',
	0xDF, 'S', 0xE0, 'a', 0xE1, 'a', 0xE2, 'a', 0xE3, 'a', 0xE4, 'a',
	0xE5, 'a', 0xE6, 'a', 0xE7, 'c', 0xE8, 'e', 0xE9, 'e', 0xEA, 'e',
	0xEB, 'e', 0xEC, 'i', 0xED, 'i', 0xEE, 'i', 0xEF, 'i', 0xF0, 'o',
	0xF1, 'n', 0xF2, 'o', 0xF3, 'o', 0xF4, 'o', 0xF5, 'o', 0xF6, 'o',
	0xF8, 'o', 0xF9, 'u', 0xFA, 'u', 0xFB, 'u', 0xFC, 'u', 0xFD, 'y',
	0xFE, 'p', 0xFF, 'y',

	0x100, 'A', 0x101, 'a', 0x102, 'A', 0x103, 'a', 0x104, 'A',
	0x105, 'a', 0x106, 'C', 0x107, 'c', 0x108, 'C', 0x109, 'c',
	0x10a, 'C', 0x10b, 'c', 0x10c, 'C', 0x10d, 'c', 0x10e, 'D',
	0x10f, 'd', 0x110, 'D', 0x111, 'd', 0x112, 'E', 0x113, 'e',
	0x114, 'E', 0x115, 'e', 0x116, 'E', 0x117, 'e', 0x118, 'E',
	0x119, 'e', 0x11a, 'E', 0x11b, 'e', 0x11c, 'G', 0x11d, 'g',
	0x11e, 'G', 0x11f, 'g', 0x120, 'G', 0x121, 'g', 0x122, 'G',
	0x123, 'g', 0x124, 'H', 0x125, 'h', 0x126, 'H', 0x127, 'h',
	0x128, 'I', 0x129, 'i', 0x12a, 'I', 0x12b, 'i', 0x12c, 'I',
	0x12d, 'i', 0x12e, 'I', 0x12f, 'i', 0x130, 'I', 0x131, 'i',
	0x134, 'J', 0x135, 'j', 0x136, 'K', 0x137, 'k', 0x138, 'K',
	0x139, 'L', 0x13a, 'l', 0x13b, 'L', 0x13c, 'l', 0x13d, 'L',
	0x13e, 'l', 0x13f, 'L', 0x140, 'l', 0x141, 'L', 0x142, 'l',
	0x143, 'N', 0x144, 'n', 0x145, 'N', 0x146, 'n', 0x147, 'N',
	0x148, 'n', 0x149, 'n', 0x14a, 'N', 0x14b, 'n', 0x14c, 'O',
	0x14d, 'o', 0x14e, 'O', 0x14f, 'o', 0x150, 'O', 0x151, 'o',
	0x154, 'R', 0x155, 'r', 0x156, 'R', 0x157, 'r', 0x158, 'R',
	0x159, 'r', 0x15a, 'S', 0x15b, 's', 0x15c, 'S', 0x15d, 's',
	0x15e, 'S', 0x15f, 's', 0x160, 'S', 0x161, 's', 0x162, 'T',
	0x163, 't', 0x164, 'T', 0x165, 't', 0x166, 'T', 0x167, 't',
	0x168, 'U', 0x169, 'u', 0x16a, 'U', 0x16b, 'u', 0x16c, 'U',
	0x16d, 'u', 0x16e, 'U', 0x16f, 'u', 0x170, 'U', 0x171, 'u',
	0x172, 'U', 0x173, 'u', 0x174, 'W', 0x175, 'w', 0x176, 'Y',
	0x177, 'y', 0x178, 'y', 0x179, 'Z', 0x17a, 'z', 0x17b, 'Z',
	0x17c, 'z', 0x17d, 'Z', 0x17e, 'z', 0
};

/* access via pointers so they can be changed by the user */
static AL_CONST unsigned short *codepage_table = _codepage_table;
static AL_CONST unsigned short *codepage_extras = _codepage_extras;

/* ascii_cp_getc:
 *  Reads a character from an ASCII codepage string.
 */
static int ascii_cp_getc(AL_CONST char *s) { return codepage_table[*((unsigned char *)s)]; }

/* ascii_cp_getx:
 *  Reads from an ASCII codepage string, advancing pointer position.
 */
static int ascii_cp_getx(char **s) { return codepage_table[*((unsigned char *)((*s)++))]; }

/* ascii_cp_setc:
 *  Sets a character in an ASCII codepage string.
 */
static int ascii_cp_setc(char *s, int c) {
	for (int i = 0; i < 256; i++) {
		if (codepage_table[i] == c) {
			*s = i;
			return 1;
		}
	}

	if (codepage_extras) {
		for (int i = 0; codepage_extras[i]; i += 2) {
			if (codepage_extras[i] == c) {
				*s = codepage_extras[i + 1];
				return 1;
			}
		}
	}

	*s = '^';
	return 1;
}

/* ascii_cp_isok:
 *  Checks whether this character can be encoded in ASCII codepage format.
 */
static int ascii_cp_isok(int c) {
	for (int i = 0; i < 256; i++) {
		if (codepage_table[i] == c)
			return TRUE;
	}

	if (codepage_extras) {
		for (int i = 0; codepage_extras[i]; i += 2) {
			if (codepage_extras[i] == c)
				return TRUE;
		}
	}

	return FALSE;
}

/* unicode_getc:
 *  Reads a character from a Unicode string.
 */
static int unicode_getc(AL_CONST char *s) { return *((unsigned short *)s); }

/* unicode_getx:
 *  Reads a character from a Unicode string, advancing the pointer position.
 */
static int unicode_getx(char **s) {
	int c = *((unsigned short *)(*s));
	(*s) += sizeof(unsigned short);
	return c;
}

/* unicode_setc:
 *  Sets a character in a Unicode string.
 */
static int unicode_setc(char *s, int c) {
	*((unsigned short *)s) = c;
	return sizeof(unsigned short);
}

/* unicode_width:
 *  Returns the width of a Unicode character.
 */
static int unicode_width(AL_CONST char *s) { return sizeof(unsigned short); }

/* unicode_cwidth:
 *  Returns the width of a Unicode character.
 */
static int unicode_cwidth(int c) { return sizeof(unsigned short); }

/* unicode_isok:
 *  Checks whether this character can be encoded in 16-bit Unicode format.
 */
static int unicode_isok(int c) { return ((c >= 0) && (c <= 65535)); }

/* utf8_getc:
 *  Reads a character from a UTF-8 string.
 */
static int utf8_getc(AL_CONST char *s) {
	int c = *((unsigned char *)(s++));
	int n, t;

	if (c & 0x80) {
		n = 1;
		while (c & (0x80 >> n))
			n++;

		c &= (1 << (8 - n)) - 1;

		while (--n > 0) {
			t = *((unsigned char *)(s++));

			if ((!(t & 0x80)) || (t & 0x40))
				return '^';

			c = (c << 6) | (t & 0x3F);
		}
	}
	return c;
}

/* utf8_getx:
 *  Reads a character from a UTF-8 string, advancing the pointer position.
 */
static int utf8_getx(char **s) {
	int c = *((unsigned char *)((*s)++));
	int n, t;

	if (c & 0x80) {
		n = 1;
		while (c & (0x80 >> n))
			n++;

		c &= (1 << (8 - n)) - 1;

		while (--n > 0) {
			t = *((unsigned char *)((*s)++));

			if ((!(t & 0x80)) || (t & 0x40)) {
				(*s)--;
				return '^';
			}

			c = (c << 6) | (t & 0x3F);
		}
	}

	return c;
}

/* utf8_width:
 *  Returns the width of a UTF-8 character.
 */
static int utf8_width(AL_CONST char *s) {
	int c = *((unsigned char *)s);
	int n = 1;

	if (c & 0x80) {
		while (c & (0x80 >> n))
			n++;
	}

	return n;
}

/* utf8_cwidth:
 *  Returns the width of a UTF-8 character.
 */
static int utf8_cwidth(int c) {
	if (c < 128)
		return 1;

	int bits = 7;
	while (c >= (1 << bits))
		bits++;

	int size = 2;
	int b = 11;

	while (b < bits) {
		size++;
		b += 5;
	}

	return size;
}

/* utf8_setc:
 *  Sets a character in a UTF-8 string.
 */
static int utf8_setc(char *s, int c) {
	if (c < 128) {
		*s = c;
		return 1;
	}

	int bits = 7;
	while (c >= (1 << bits))
		bits++;

	int size = 2;
	int b = 11;

	while (b < bits) {
		size++;
		b += 5;
	}

	b -= (7 - size);
	s[0] = c >> b;

	for (int i = 0; i < size; i++)
		s[0] |= (0x80 >> i);

	for (int i = 1; i < size; i++) {
		b -= 6;
		s[i] = 0x80 | ((c >> b) & 0x3F);
	}

	return size;
}

/* utf8_isok:
 *  Checks whether this character can be encoded in UTF-8 format.
 */
static int utf8_isok(int c) { return TRUE; }

/* string format table, to allow user expansion with other encodings */
static UTYPE_INFO utypes[] = {
	{ U_ASCII, ascii_getc, ascii_getx, ascii_setc, ascii_width, ascii_cwidth, ascii_isok, 1 },
	{ U_UTF8, utf8_getc, utf8_getx, utf8_setc, utf8_width, utf8_cwidth, utf8_isok, 4 },
	{ U_UNICODE, unicode_getc, unicode_getx, unicode_setc, unicode_width, unicode_cwidth, unicode_isok, 2 },
	{ U_ASCII_CP, ascii_cp_getc, ascii_cp_getx, ascii_cp_setc, ascii_width, ascii_cwidth, ascii_cp_isok, 1 },
	{ 0, NULL, NULL, NULL, NULL, NULL, NULL, 0 },
	{ 0, NULL, NULL, NULL, NULL, NULL, NULL, 0 },
	{ 0, NULL, NULL, NULL, NULL, NULL, NULL, 0 },
	{ 0, NULL, NULL, NULL, NULL, NULL, NULL, 0 }
};

static int utype = U_UTF8; /* current format information and worker routines */

int (*ugetc)(AL_CONST char *s) = utf8_getc; /* ugetc */
int (*ugetx)(char **s) = utf8_getx; /* ugetx: */
int (*ugetxc)(AL_CONST char **s) = (int (*)(AL_CONST char **))utf8_getx; /* ugetxc */
int (*usetc)(char *s, int c) = utf8_setc; /* usetc */
int (*uwidth)(AL_CONST char *s) = utf8_width; /* uwidth */
int (*ucwidth)(int c) = utf8_cwidth; /* ucwidth */

/* _find_utype:
 *  Helper for locating a string type description.
 */
UTYPE_INFO *_find_utype(int type) {
	if (type == U_CURRENT)
		type = utype;

	for (int i = 0; i < (int)(sizeof(utypes) / sizeof(UTYPE_INFO)); i++)
		if (utypes[i].id == type)
			return &utypes[i];

	return NULL;
}

/* ustrsizez:
 *  Returns the size of the specified string in bytes, including the
 *  trailing zero.
 */
static int ustrsizez(AL_CONST char *s) {
	AL_CONST char *orig = s;
	ASSERT(s);

	do {
	} while (ugetxc(&s) != 0);

	return (long)s - (long)orig;
}

/* ustrzcpy:
 *  Enhanced Unicode-aware version of the ANSI strcpy() function
 *  that can handle the size (in bytes) of the destination string.
 *  The raw Unicode-aware version of ANSI strcpy() is defined as:
 *   #define ustrcpy(dest, src) ustrzcpy(dest, INT_MAX, src)
 */
char *ustrzcpy(char *dest, int size, AL_CONST char *src) {
	int pos = 0;
	int c;
	ASSERT(dest);
	ASSERT(src);
	ASSERT(size > 0);

	size -= ucwidth(0);
	ASSERT(size >= 0);

	while ((c = ugetxc(&src)) != 0) {
		size -= ucwidth(c);
		if (size < 0)
			break;

		pos += usetc(dest + pos, c);
	}

	usetc(dest + pos, 0);

	return dest;
}

/* need_uconvert:
 *  Decides whether a conversion is required to make this string be in the
 *  new type. No conversion will be needed if both types are the same, or
 *  when going from ASCII <-> UTF8 where the data is 7-bit clean.
 */
int need_uconvert(AL_CONST char *s, int type, int newtype) {
	int c;
	ASSERT(s);

	if (type == U_CURRENT)
		type = utype;

	if (newtype == U_CURRENT)
		newtype = utype;

	if (type == newtype)
		return FALSE;

	if (((type == U_ASCII) || (type == U_UTF8)) && ((newtype == U_ASCII) || (newtype == U_UTF8))) {
		do {
			c = *((unsigned char *)(s++));
			if (!c)
				return FALSE;
		} while (c <= 127);
	}

	return TRUE;
}

/* do_uconvert:
 *  Converts a string from one format to another.
 */
void do_uconvert(AL_CONST char *s, int type, char *buf, int newtype, int size) {
	UTYPE_INFO *info, *outfo;
	int pos = 0;
	int c;
	ASSERT(s);
	ASSERT(buf);
	ASSERT(size > 0);

	info = _find_utype(type);
	if (!info)
		return;

	outfo = _find_utype(newtype);
	if (!outfo)
		return;

	size -= outfo->u_cwidth(0);
	ASSERT(size >= 0);

	while ((c = info->u_getx((char **)&s)) != 0) {
		if (!outfo->u_isok(c))
			c = '^';

		size -= outfo->u_cwidth(c);
		if (size < 0)
			break;

		pos += outfo->u_setc(buf + pos, c);
	}

	outfo->u_setc(buf + pos, 0);
}

/* uconvert:
 *  Higher level version of do_uconvert(). This routine is intelligent
 *  enough to just return a copy of the input string if no conversion
 *  is required, and to use an internal static buffer if no user buffer
 *  is provided.
 */
char *uconvert(AL_CONST char *s, int type, char *buf, int newtype, int size) {
	static char static_buf[1024];
	ASSERT(s);
	ASSERT(size >= 0);

	if (!need_uconvert(s, type, newtype))
		return (char *)s;

	if (!buf) {
		buf = static_buf;
		size = sizeof(static_buf);
	}

	do_uconvert(s, type, buf, newtype, size);
	return buf;
}

/* uwidth_max:
 *  Returns the largest possible size of a character in the specified
 *  encoding format, in bytes.
 */
int uwidth_max(int type) {
	UTYPE_INFO *info = _find_utype(type);
	if (!info)
		return 0;

	return info->u_width_max;
}

/* usetat:
 *  Modifies the character at the specified index within the string,
 *  handling adjustments for variable width data. Returns how far the
 *  rest of the string was moved.
 */
int usetat(char *s, int index, int c) {
	ASSERT(s);

	s += uoffset(s, index);

	int oldw = uwidth(s);
	int neww = ucwidth(c);

	if (oldw != neww)
		memmove(s + neww, s + oldw, ustrsizez(s + oldw));

	usetc(s, c);

	return neww - oldw;
}

/* ustrlen:
 *  Unicode-aware version of the ANSI strlen() function.
 */
int ustrlen(AL_CONST char *s) {
	int c = 0;
	ASSERT(s);

	while (ugetxc(&s))
		c++;

	return c;
}

/* uoffset:
 *  Returns the offset in bytes from the start of the string to the
 *  character at the specified index. If the index is negative, counts
 *  backward from the end of the string (-1 returns an offset to the
 *  last character).
 */
int uoffset(AL_CONST char *s, int index) {
	AL_CONST char *orig = s;
	AL_CONST char *last;
	ASSERT(s);

	if (index < 0)
		index += ustrlen(s);

	while (index-- > 0) {
		last = s;
		if (!ugetxc(&s)) {
			s = last;
			break;
		}
	}

	return (long)s - (long)orig;
}

/* ustrtok_r:
 *  Unicode-aware version of the strtok_r() function.
 */
char *ustrtok_r(char *s, AL_CONST char *set, char **last) {
	char *prev_str, *tok;
	AL_CONST char *setp;
	int c, sc;

	ASSERT(last);

	if (!s) {
		s = *last;

		if (!s)
			return NULL;
	}

skip_leading_delimiters:

	prev_str = s;
	c = ugetx(&s);

	setp = set;

	while ((sc = ugetxc(&setp)) != 0) {
		if (c == sc)
			goto skip_leading_delimiters;
	}

	if (!c) {
		*last = NULL;
		return NULL;
	}

	tok = prev_str;

	for (;;) {
		prev_str = s;
		c = ugetx(&s);

		setp = set;

		do {
			sc = ugetxc(&setp);
			if (sc == c) {
				if (!c) {
					*last = NULL;
					return tok;
				} else {
					s += usetat(prev_str, 0, 0);
					*last = s;
					return tok;
				}
			}
		} while (sc);
	}
}

/* information about the current format conversion mode */
typedef struct SPRINT_INFO {
	int flags;
	int field_width;
	int precision;
	int num_special;
} SPRINT_INFO;

#define SPRINT_FLAG_LEFT_JUSTIFY 1
#define SPRINT_FLAG_FORCE_PLUS_SIGN 2
#define SPRINT_FLAG_FORCE_SPACE 4
#define SPRINT_FLAG_ALTERNATE_CONVERSION 8
#define SPRINT_FLAG_PAD_ZERO 16
#define SPRINT_FLAG_SHORT_INT 32
#define SPRINT_FLAG_LONG_INT 64
#define SPRINT_FLAG_LONG_DOUBLE 128
#define SPRINT_FLAG_LONG_LONG 256

/* decoded string argument type */
typedef struct STRING_ARG {
	char *data;
	int size; /* in bytes without the terminating '\0' */
	struct STRING_ARG *next;
} STRING_ARG;

/* LONGEST:
 *  64-bit integers on platforms that support it, 32-bit otherwise.
 */
#ifdef LONG_LONG
#define LONGEST LONG_LONG
#else
#define LONGEST long
#endif

/* va_int:
 *  Helper for reading an integer from the varargs list.
 */
#ifdef LONG_LONG

#define va_int(args, flags)                                                                                                                            \
	(                                                                                                                                                  \
			((flags) & SPRINT_FLAG_SHORT_INT) ? va_arg(args, signed int)                                                                               \
											  : (((flags) & SPRINT_FLAG_LONG_LONG) ? va_arg(args, signed LONG_LONG)                                    \
																				   : (((flags) & SPRINT_FLAG_LONG_INT) ? va_arg(args, signed long int) \
																													   : va_arg(args, signed int))))

#else

#define va_int(args, flags)                                                                                       \
	(                                                                                                             \
			((flags) & SPRINT_FLAG_SHORT_INT) ? va_arg(args, signed int)                                          \
											  : (((flags) & SPRINT_FLAG_LONG_INT) ? va_arg(args, signed long int) \
																				  : va_arg(args, signed int)))

#endif

/* va_uint:
 *  Helper for reading an unsigned integer from the varargs list.
 */
#ifdef LONG_LONG

#define va_uint(args, flags)                                                                                                                             \
	(                                                                                                                                                    \
			((flags) & SPRINT_FLAG_SHORT_INT) ? va_arg(args, unsigned int)                                                                               \
											  : (((flags) & SPRINT_FLAG_LONG_LONG) ? va_arg(args, unsigned LONG_LONG)                                    \
																				   : (((flags) & SPRINT_FLAG_LONG_INT) ? va_arg(args, unsigned long int) \
																													   : va_arg(args, unsigned int))))

#else

#define va_uint(args, flags)                                                                                        \
	(                                                                                                               \
			((flags) & SPRINT_FLAG_SHORT_INT) ? va_arg(args, unsigned int)                                          \
											  : (((flags) & SPRINT_FLAG_LONG_INT) ? va_arg(args, unsigned long int) \
																				  : va_arg(args, unsigned int)))

#endif

/* sprint_plus_sign:
 *  Helper to add a plus sign or space in front of a number.
 */
#define sprint_plus_sign(len)                               \
	{                                                       \
		if (info->flags & SPRINT_FLAG_FORCE_PLUS_SIGN) {    \
			pos += usetc(string_arg->data + pos, '+');      \
			len++;                                          \
		} else if (info->flags & SPRINT_FLAG_FORCE_SPACE) { \
			pos += usetc(string_arg->data + pos, ' ');      \
			len++;                                          \
		}                                                   \
	}

/* sprint_i:
 *  Worker function for formatting integers.
 */
static int sprint_i(STRING_ARG *string_arg, unsigned LONGEST val, int precision) {
	char tmp[24]; /* for 64-bit integers */
	int i = 0, pos = string_arg->size;
	int len;

	do {
		tmp[i++] = val % 10;
		val /= 10;
	} while (val);

	for (len = i; len < precision; len++)
		pos += usetc(string_arg->data + pos, '0');

	while (i > 0)
		pos += usetc(string_arg->data + pos, tmp[--i] + '0');

	string_arg->size = pos;
	usetc(string_arg->data + pos, 0);

	return len;
}

/* sprint_int:
 *  Helper for formatting a signed integer.
 */
static int sprint_int(STRING_ARG *string_arg, SPRINT_INFO *info, LONGEST val) {
	int pos = 0, len = 0;

	/* 24 characters max for a 64-bit integer */
	string_arg->data = _AL_MALLOC((MAX(24, info->field_width) * uwidth_max(U_CURRENT) + ucwidth(0)) * sizeof(char));

	if (val < 0) {
		val = -val;
		pos += usetc(string_arg->data + pos, '-');
		len++;
	} else
		sprint_plus_sign(len);

	info->num_special = len;

	string_arg->size = pos;

	return sprint_i(string_arg, val, info->precision) + info->num_special;
}

/* sprint_unsigned:
 *  Helper for formatting an unsigned integer.
 */
static int sprint_unsigned(STRING_ARG *string_arg, SPRINT_INFO *info, unsigned LONGEST val) {
	int pos = 0;

	/* 24 characters max for a 64-bit integer */
	string_arg->data = _AL_MALLOC((MAX(24, info->field_width) * uwidth_max(U_CURRENT) + ucwidth(0)) * sizeof(char));

	sprint_plus_sign(info->num_special);

	string_arg->size = pos;

	return sprint_i(string_arg, val, info->precision) + info->num_special;
}

/* sprint_hex:
 *  Helper for formatting a hex integer.
 */
static int sprint_hex(STRING_ARG *string_arg, SPRINT_INFO *info, int caps, unsigned LONGEST val) {
	static char hex_digit_caps[] = "0123456789ABCDEF";
	static char hex_digit[] = "0123456789abcdef";

	char tmp[24]; /* for 64-bit integers */
	char *table;
	int pos = 0, i = 0;
	int len;

	/* 24 characters max for a 64-bit integer */
	string_arg->data = _AL_MALLOC((MAX(24, info->field_width) * uwidth_max(U_CURRENT) + ucwidth(0)) * sizeof(char));

	sprint_plus_sign(info->num_special);

	if (info->flags & SPRINT_FLAG_ALTERNATE_CONVERSION) {
		pos += usetc(string_arg->data + pos, '0');
		pos += usetc(string_arg->data + pos, 'x');
		info->num_special += 2;
	}

	do {
		tmp[i++] = val & 15;
		val >>= 4;
	} while (val);

	for (len = i; len < info->precision; len++)
		pos += usetc(string_arg->data + pos, '0');

	if (caps)
		table = hex_digit_caps;
	else
		table = hex_digit;

	while (i > 0)
		pos += usetc(string_arg->data + pos, table[(int)tmp[--i]]);

	string_arg->size = pos;
	usetc(string_arg->data + pos, 0);

	return len + info->num_special;
}

/* sprint_octal:
 *  Helper for formatting an octal integer.
 */
static int sprint_octal(STRING_ARG *string_arg, SPRINT_INFO *info, unsigned LONGEST val) {
	char tmp[24]; /* for 64-bit integers */
	int pos = 0, i = 0;
	int len;

	/* 24 characters max for a 64-bit integer */
	string_arg->data = _AL_MALLOC((MAX(24, info->field_width) * uwidth_max(U_CURRENT) + ucwidth(0)) * sizeof(char));

	sprint_plus_sign(info->num_special);

	if (info->flags & SPRINT_FLAG_ALTERNATE_CONVERSION) {
		pos += usetc(string_arg->data + pos, '0');
		info->num_special++;
	}

	do {
		tmp[i++] = val & 7;
		val >>= 3;
	} while (val);

	for (len = i; len < info->precision; len++)
		pos += usetc(string_arg->data + pos, '0');

	while (i > 0)
		pos += usetc(string_arg->data + pos, tmp[--i] + '0');

	string_arg->size = pos;
	usetc(string_arg->data + pos, 0);

	return len + info->num_special;
}

/* sprint_float:
 *  Helper for formatting a float (piggyback on the libc implementation).
 */
static int sprint_float(STRING_ARG *string_arg, SPRINT_INFO *info, double val, int conversion) {
	char format[256], tmp[256];
	int len = 0, size;

	format[len++] = '%';

	if (info->flags & SPRINT_FLAG_LEFT_JUSTIFY)
		format[len++] = '-';

	if (info->flags & SPRINT_FLAG_FORCE_PLUS_SIGN)
		format[len++] = '+';

	if (info->flags & SPRINT_FLAG_FORCE_SPACE)
		format[len++] = ' ';

	if (info->flags & SPRINT_FLAG_ALTERNATE_CONVERSION)
		format[len++] = '#';

	if (info->flags & SPRINT_FLAG_PAD_ZERO)
		format[len++] = '0';

	if (info->field_width > 0)
		len += sprintf(format + len, "%d", info->field_width);

	if (info->precision >= 0)
		len += sprintf(format + len, ".%d", info->precision);

	format[len++] = conversion;
	format[len] = 0;

	len = sprintf(tmp, format, val);
	size = len * uwidth_max(U_CURRENT) + ucwidth(0);

	string_arg->data = _AL_MALLOC(size * sizeof(char));

	do_uconvert(tmp, U_ASCII, string_arg->data, U_CURRENT, size);

	info->field_width = 0;

	string_arg->size = ustrsize(string_arg->data);

	return len;
}

/* sprint_char:
 *  Helper for formatting (!) a character.
 */
static int sprint_char(STRING_ARG *string_arg, SPRINT_INFO *info, long val) {
	int pos = 0;

	/* 1 character max for... a character */
	string_arg->data = _AL_MALLOC((MAX(1, info->field_width) * uwidth_max(U_CURRENT) + ucwidth(0)) * sizeof(char));

	pos += usetc(string_arg->data, val);

	string_arg->size = pos;
	usetc(string_arg->data + pos, 0);

	return 1;
}

/* sprint_string:
 *  Helper for formatting a string.
 */
static int sprint_string(STRING_ARG *string_arg, SPRINT_INFO *info, AL_CONST char *s) {
	int pos = 0, len = 0;
	int c;

	string_arg->data = _AL_MALLOC((MAX(ustrlen(s), info->field_width) * uwidth_max(U_CURRENT) + ucwidth(0)) * sizeof(char));

	while ((c = ugetxc(&s)) != 0) {
		if ((info->precision >= 0) && (len >= info->precision))
			break;

		pos += usetc(string_arg->data + pos, c);
		len++;
	}

	string_arg->size = pos;
	usetc(string_arg->data + pos, 0);

	return len;
}

/* decode_format_string:
 *  Worker function for decoding the format string (with those pretty '%' characters)
 */
static int decode_format_string(char *buf, STRING_ARG *string_arg, AL_CONST char *format, va_list args) {
	SPRINT_INFO info;
	int *pstr_pos;
	int done, slen, c, i, pos;
	int shift, shiftbytes, shiftfiller;
	int len = 0;

	while ((c = ugetxc(&format)) != 0) {
		if (c == '%') {
			if ((c = ugetc(format)) == '%') {
				/* percent sign escape */
				format += uwidth(format);
				buf += usetc(buf, '%');
				buf += usetc(buf, '%');
				len++;
			} else {
/* format specifier */
#define NEXT_C()                  \
	{                             \
		format += uwidth(format); \
		c = ugetc(format);        \
	}

				/* set default conversion flags */
				info.flags = 0;
				info.field_width = 0;
				info.precision = -1;
				info.num_special = 0;

				/* check for conversion flags */
				done = FALSE;

				do {
					switch (c) {
						case '-':
							info.flags |= SPRINT_FLAG_LEFT_JUSTIFY;
							NEXT_C();
							break;

						case '+':
							info.flags |= SPRINT_FLAG_FORCE_PLUS_SIGN;
							NEXT_C();
							break;

						case ' ':
							info.flags |= SPRINT_FLAG_FORCE_SPACE;
							NEXT_C();
							break;

						case '#':
							info.flags |= SPRINT_FLAG_ALTERNATE_CONVERSION;
							NEXT_C();
							break;

						case '0':
							info.flags |= SPRINT_FLAG_PAD_ZERO;
							NEXT_C();
							break;

						default:
							done = TRUE;
							break;
					}

				} while (!done);

				/* check for a field width specifier */
				if (c == '*') {
					NEXT_C();
					info.field_width = va_arg(args, int);
					if (info.field_width < 0) {
						info.flags |= SPRINT_FLAG_LEFT_JUSTIFY;
						info.field_width = -info.field_width;
					}
				} else if ((c >= '0') && (c <= '9')) {
					info.field_width = 0;
					do {
						info.field_width *= 10;
						info.field_width += c - '0';
						NEXT_C();
					} while ((c >= '0') && (c <= '9'));
				}

				/* check for a precision specifier */
				if (c == '.')
					NEXT_C();

				if (c == '*') {
					NEXT_C();
					info.precision = va_arg(args, int);
					if (info.precision < 0)
						info.precision = 0;
				} else if ((c >= '0') && (c <= '9')) {
					info.precision = 0;
					do {
						info.precision *= 10;
						info.precision += c - '0';
						NEXT_C();
					} while ((c >= '0') && (c <= '9'));
				}

				/* check for size qualifiers */
				done = FALSE;

				do {
					switch (c) {
						case 'h':
							info.flags |= SPRINT_FLAG_SHORT_INT;
							NEXT_C();
							break;

						case 'l':
							if (info.flags & SPRINT_FLAG_LONG_INT)
								info.flags |= SPRINT_FLAG_LONG_LONG;
							else
								info.flags |= SPRINT_FLAG_LONG_INT;
							NEXT_C();
							break;

						case 'L':
							info.flags |= (SPRINT_FLAG_LONG_DOUBLE | SPRINT_FLAG_LONG_LONG);
							NEXT_C();
							break;

						default:
							done = TRUE;
							break;
					}

				} while (!done);

				/* format the data */
				switch (c) {
					case 'c':
						/* character */
						slen = sprint_char(string_arg, &info, va_arg(args, int));
						NEXT_C();
						break;

					case 'd':
					case 'i':
						/* signed integer */
						slen = sprint_int(string_arg, &info, va_int(args, info.flags));
						NEXT_C();
						break;

					case 'D':
						/* signed long integer */
						slen = sprint_int(string_arg, &info, va_int(args, info.flags | SPRINT_FLAG_LONG_INT));
						NEXT_C();
						break;

					case 'e':
					case 'E':
					case 'f':
					case 'g':
					case 'G':
						/* double */
						if (info.flags & SPRINT_FLAG_LONG_DOUBLE)
							slen = sprint_float(string_arg, &info, va_arg(args, long double), c);
						else
							slen = sprint_float(string_arg, &info, va_arg(args, double), c);
						NEXT_C();
						break;

					case 'n':
						/* store current string position */
						pstr_pos = va_arg(args, int *);
						*pstr_pos = len;
						slen = -1;
						NEXT_C();
						break;

					case 'o':
						/* unsigned octal integer */
						slen = sprint_octal(string_arg, &info, va_uint(args, info.flags));
						NEXT_C();
						break;

					case 'p':
						/* pointer */
						slen = sprint_hex(string_arg, &info, FALSE, (unsigned long)(va_arg(args, void *)));
						NEXT_C();
						break;

					case 's':
						/* string */
						slen = sprint_string(string_arg, &info, va_arg(args, char *));
						NEXT_C();
						break;

					case 'u':
						/* unsigned integer */
						slen = sprint_unsigned(string_arg, &info, va_uint(args, info.flags));
						NEXT_C();
						break;

					case 'U':
						/* unsigned long integer */
						slen = sprint_unsigned(string_arg, &info, va_uint(args, info.flags | SPRINT_FLAG_LONG_INT));
						NEXT_C();
						break;

					case 'x':
					case 'X':
						/* unsigned hex integer */
						slen = sprint_hex(string_arg, &info, (c == 'X'), va_uint(args, info.flags));
						NEXT_C();
						break;

					default:
						/* weird shit... */
						slen = -1;
						break;
				}

				if (slen >= 0) {
					if (slen < info.field_width) {
						if (info.flags & SPRINT_FLAG_LEFT_JUSTIFY) {
							/* left align the result */
							pos = string_arg->size;
							while (slen < info.field_width) {
								pos += usetc(string_arg->data + pos, ' ');
								slen++;
							}

							string_arg->size = pos;
							usetc(string_arg->data + pos, 0);
						} else {
							/* right align the result */
							shift = info.field_width - slen;

							if (shift > 0) {
								pos = 0;

								if (info.flags & SPRINT_FLAG_PAD_ZERO) {
									shiftfiller = '0';

									for (i = 0; i < info.num_special; i++)
										pos += uwidth(string_arg->data + pos);
								} else
									shiftfiller = ' ';

								shiftbytes = shift * ucwidth(shiftfiller);
								memmove(string_arg->data + pos + shiftbytes, string_arg->data + pos, string_arg->size - pos + ucwidth(0));

								string_arg->size += shiftbytes;
								slen += shift;

								for (i = 0; i < shift; i++)
									pos += usetc(string_arg->data + pos, shiftfiller);
							}
						}
					}

					buf += usetc(buf, '%');
					buf += usetc(buf, 's');
					len += slen;

					/* allocate next item */
					string_arg->next = _AL_MALLOC(sizeof(STRING_ARG));
					string_arg = string_arg->next;
					string_arg->next = NULL;
				}
			}
		} else {
			/* normal character */
			buf += usetc(buf, c);
			len++;
		}
	}

	usetc(buf, 0);

	return len;
}

/* uvszprintf:
 *  Enhanced Unicode-aware version of the ANSI vsprintf() function
 *  than can handle the size (in bytes) of the destination buffer.
 *  The raw Unicode-aware version of ANSI vsprintf() is defined as:
 *   #define uvsprintf(buf, format, args) uvszprintf(buf, INT_MAX, format, args)
 */
int uvszprintf(char *buf, int size, AL_CONST char *format, va_list args) {
	char *decoded_format, *df;
	STRING_ARG *string_args, *iter_arg;
	int c, len;
	ASSERT(buf);
	ASSERT(size >= 0);
	ASSERT(format);

	/* decoding can only lower the length of the format string */
	df = decoded_format = _AL_MALLOC_ATOMIC(ustrsizez(format) * sizeof(char));

	/* allocate first item */
	string_args = _AL_MALLOC(sizeof(STRING_ARG));
	string_args->next = NULL;

	/* 1st pass: decode */
	len = decode_format_string(decoded_format, string_args, format, args);

	size -= ucwidth(0);
	iter_arg = string_args;

	/* 2nd pass: concatenate */
	while ((c = ugetx(&decoded_format)) != 0) {
		if (c == '%') {
			if ((c = ugetx(&decoded_format)) == '%') {
				/* percent sign escape */
				size -= ucwidth('%');
				if (size < 0)
					break;
				buf += usetc(buf, '%');
			} else if (c == 's') {
				/* string argument */
				ustrzcpy(buf, size + ucwidth(0), iter_arg->data);
				buf += iter_arg->size;
				size -= iter_arg->size;
				if (size < 0) {
					buf += size;
					break;
				}
				iter_arg = iter_arg->next;
			}
		} else {
			/* normal character */
			size -= ucwidth(c);
			if (size < 0)
				break;
			buf += usetc(buf, c);
		}
	}

	usetc(buf, 0);

	/* free allocated resources */
	while (string_args->next) {
		_AL_FREE(string_args->data);
		iter_arg = string_args;
		string_args = string_args->next;
		_AL_FREE(iter_arg);
	}
	_AL_FREE(string_args);
	_AL_FREE(df); /* alias for decoded_format */

	return len;
}

/* _al_ustrdup:
 *  Returns a newly allocated copy of the src string, which must later be
 *  freed by the caller using _AL_FREE().  This function is for internal use
 *  by Allegro clients only.  External code should use ustrdup().
 */
char *_al_ustrdup(AL_CONST char *src) {
	ASSERT(src);

	int size = ustrsizez(src);
	char *newstring = _AL_MALLOC(size);

	if (newstring)
		ustrzcpy(newstring, size, src);

	return newstring;
}

/* ustrsize:
 *  Returns the size of the specified string in bytes, not including the
 *  trailing zero.
 */
int ustrsize(AL_CONST char *s) {
	AL_CONST char *orig = s;
	AL_CONST char *last;
	ASSERT(s);

	do {
		last = s;
	} while (ugetxc(&s) != 0);

	return (long)last - (long)orig;
}

/*
 * Assorted globals and setup/cleanup routines.
 * ============================================
 */

#define LOGFILE "al-gfx.log"

int *allegro_errno = NULL; /* error value, which will work even with DLL linkage */
BITMAP *screen = NULL; /* a bitmap structure for accessing the physical screen */
int SCREEN_W = 0;
int SCREEN_H = 0;
int _drawing_mode = DRAW_MODE_SOLID; /* info about the current graphics drawing mode */

BITMAP *_drawing_pattern = NULL;

int _drawing_x_anchor = 0;
int _drawing_y_anchor = 0;

unsigned int _drawing_x_mask = 0;
unsigned int _drawing_y_mask = 0;

/* default palette structures */
PALETTE black_palette;
PALETTE _current_palette;

int _current_palette_changed = 0xFFFFFFFF;

PALETTE desktop_palette = {
	{ 63, 63, 63, 0 }, { 63, 0, 0, 0 }, { 0, 63, 0, 0 }, { 63, 63, 0, 0 },
	{ 0, 0, 63, 0 }, { 63, 0, 63, 0 }, { 0, 63, 63, 0 }, { 16, 16, 16, 0 },
	{ 31, 31, 31, 0 }, { 63, 31, 31, 0 }, { 31, 63, 31, 0 }, { 63, 63, 31, 0 },
	{ 31, 31, 63, 0 }, { 63, 31, 63, 0 }, { 31, 63, 63, 0 }, { 0, 0, 0, 0 }
};

PALETTE default_palette = {
	{ 0, 0, 0, 0 }, { 0, 0, 42, 0 }, { 0, 42, 0, 0 }, { 0, 42, 42, 0 },
	{ 42, 0, 0, 0 }, { 42, 0, 42, 0 }, { 42, 21, 0, 0 }, { 42, 42, 42, 0 },
	{ 21, 21, 21, 0 }, { 21, 21, 63, 0 }, { 21, 63, 21, 0 }, { 21, 63, 63, 0 },
	{ 63, 21, 21, 0 }, { 63, 21, 63, 0 }, { 63, 63, 21, 0 }, { 63, 63, 63, 0 },
	{ 0, 0, 0, 0 }, { 5, 5, 5, 0 }, { 8, 8, 8, 0 }, { 11, 11, 11, 0 },
	{ 14, 14, 14, 0 }, { 17, 17, 17, 0 }, { 20, 20, 20, 0 }, { 24, 24, 24, 0 },
	{ 28, 28, 28, 0 }, { 32, 32, 32, 0 }, { 36, 36, 36, 0 }, { 40, 40, 40, 0 },
	{ 45, 45, 45, 0 }, { 50, 50, 50, 0 }, { 56, 56, 56, 0 }, { 63, 63, 63, 0 },
	{ 0, 0, 63, 0 }, { 16, 0, 63, 0 }, { 31, 0, 63, 0 }, { 47, 0, 63, 0 },
	{ 63, 0, 63, 0 }, { 63, 0, 47, 0 }, { 63, 0, 31, 0 }, { 63, 0, 16, 0 },
	{ 63, 0, 0, 0 }, { 63, 16, 0, 0 }, { 63, 31, 0, 0 }, { 63, 47, 0, 0 },
	{ 63, 63, 0, 0 }, { 47, 63, 0, 0 }, { 31, 63, 0, 0 }, { 16, 63, 0, 0 },
	{ 0, 63, 0, 0 }, { 0, 63, 16, 0 }, { 0, 63, 31, 0 }, { 0, 63, 47, 0 },
	{ 0, 63, 63, 0 }, { 0, 47, 63, 0 }, { 0, 31, 63, 0 }, { 0, 16, 63, 0 },
	{ 31, 31, 63, 0 }, { 39, 31, 63, 0 }, { 47, 31, 63, 0 }, { 55, 31, 63, 0 },
	{ 63, 31, 63, 0 }, { 63, 31, 55, 0 }, { 63, 31, 47, 0 }, { 63, 31, 39, 0 },
	{ 63, 31, 31, 0 }, { 63, 39, 31, 0 }, { 63, 47, 31, 0 }, { 63, 55, 31, 0 },
	{ 63, 63, 31, 0 }, { 55, 63, 31, 0 }, { 47, 63, 31, 0 }, { 39, 63, 31, 0 },
	{ 31, 63, 31, 0 }, { 31, 63, 39, 0 }, { 31, 63, 47, 0 }, { 31, 63, 55, 0 },
	{ 31, 63, 63, 0 }, { 31, 55, 63, 0 }, { 31, 47, 63, 0 }, { 31, 39, 63, 0 },
	{ 45, 45, 63, 0 }, { 49, 45, 63, 0 }, { 54, 45, 63, 0 }, { 58, 45, 63, 0 },
	{ 63, 45, 63, 0 }, { 63, 45, 58, 0 }, { 63, 45, 54, 0 }, { 63, 45, 49, 0 },
	{ 63, 45, 45, 0 }, { 63, 49, 45, 0 }, { 63, 54, 45, 0 }, { 63, 58, 45, 0 },
	{ 63, 63, 45, 0 }, { 58, 63, 45, 0 }, { 54, 63, 45, 0 }, { 49, 63, 45, 0 },
	{ 45, 63, 45, 0 }, { 45, 63, 49, 0 }, { 45, 63, 54, 0 }, { 45, 63, 58, 0 },
	{ 45, 63, 63, 0 }, { 45, 58, 63, 0 }, { 45, 54, 63, 0 }, { 45, 49, 63, 0 },
	{ 0, 0, 28, 0 }, { 7, 0, 28, 0 }, { 14, 0, 28, 0 }, { 21, 0, 28, 0 },
	{ 28, 0, 28, 0 }, { 28, 0, 21, 0 }, { 28, 0, 14, 0 }, { 28, 0, 7, 0 },
	{ 28, 0, 0, 0 }, { 28, 7, 0, 0 }, { 28, 14, 0, 0 }, { 28, 21, 0, 0 },
	{ 28, 28, 0, 0 }, { 21, 28, 0, 0 }, { 14, 28, 0, 0 }, { 7, 28, 0, 0 },
	{ 0, 28, 0, 0 }, { 0, 28, 7, 0 }, { 0, 28, 14, 0 }, { 0, 28, 21, 0 },
	{ 0, 28, 28, 0 }, { 0, 21, 28, 0 }, { 0, 14, 28, 0 }, { 0, 7, 28, 0 },
	{ 14, 14, 28, 0 }, { 17, 14, 28, 0 }, { 21, 14, 28, 0 }, { 24, 14, 28, 0 },
	{ 28, 14, 28, 0 }, { 28, 14, 24, 0 }, { 28, 14, 21, 0 }, { 28, 14, 17, 0 },
	{ 28, 14, 14, 0 }, { 28, 17, 14, 0 }, { 28, 21, 14, 0 }, { 28, 24, 14, 0 },
	{ 28, 28, 14, 0 }, { 24, 28, 14, 0 }, { 21, 28, 14, 0 }, { 17, 28, 14, 0 },
	{ 14, 28, 14, 0 }, { 14, 28, 17, 0 }, { 14, 28, 21, 0 }, { 14, 28, 24, 0 },
	{ 14, 28, 28, 0 }, { 14, 24, 28, 0 }, { 14, 21, 28, 0 }, { 14, 17, 28, 0 },
	{ 20, 20, 28, 0 }, { 22, 20, 28, 0 }, { 24, 20, 28, 0 }, { 26, 20, 28, 0 },
	{ 28, 20, 28, 0 }, { 28, 20, 26, 0 }, { 28, 20, 24, 0 }, { 28, 20, 22, 0 },
	{ 28, 20, 20, 0 }, { 28, 22, 20, 0 }, { 28, 24, 20, 0 }, { 28, 26, 20, 0 },
	{ 28, 28, 20, 0 }, { 26, 28, 20, 0 }, { 24, 28, 20, 0 }, { 22, 28, 20, 0 },
	{ 20, 28, 20, 0 }, { 20, 28, 22, 0 }, { 20, 28, 24, 0 }, { 20, 28, 26, 0 },
	{ 20, 28, 28, 0 }, { 20, 26, 28, 0 }, { 20, 24, 28, 0 }, { 20, 22, 28, 0 },
	{ 0, 0, 16, 0 }, { 4, 0, 16, 0 }, { 8, 0, 16, 0 }, { 12, 0, 16, 0 },
	{ 16, 0, 16, 0 }, { 16, 0, 12, 0 }, { 16, 0, 8, 0 }, { 16, 0, 4, 0 },
	{ 16, 0, 0, 0 }, { 16, 4, 0, 0 }, { 16, 8, 0, 0 }, { 16, 12, 0, 0 },
	{ 16, 16, 0, 0 }, { 12, 16, 0, 0 }, { 8, 16, 0, 0 }, { 4, 16, 0, 0 },
	{ 0, 16, 0, 0 }, { 0, 16, 4, 0 }, { 0, 16, 8, 0 }, { 0, 16, 12, 0 },
	{ 0, 16, 16, 0 }, { 0, 12, 16, 0 }, { 0, 8, 16, 0 }, { 0, 4, 16, 0 },
	{ 8, 8, 16, 0 }, { 10, 8, 16, 0 }, { 12, 8, 16, 0 }, { 14, 8, 16, 0 },
	{ 16, 8, 16, 0 }, { 16, 8, 14, 0 }, { 16, 8, 12, 0 }, { 16, 8, 10, 0 },
	{ 16, 8, 8, 0 }, { 16, 10, 8, 0 }, { 16, 12, 8, 0 }, { 16, 14, 8, 0 },
	{ 16, 16, 8, 0 }, { 14, 16, 8, 0 }, { 12, 16, 8, 0 }, { 10, 16, 8, 0 },
	{ 8, 16, 8, 0 }, { 8, 16, 10, 0 }, { 8, 16, 12, 0 }, { 8, 16, 14, 0 },
	{ 8, 16, 16, 0 }, { 8, 14, 16, 0 }, { 8, 12, 16, 0 }, { 8, 10, 16, 0 },
	{ 11, 11, 16, 0 }, { 12, 11, 16, 0 }, { 13, 11, 16, 0 }, { 15, 11, 16, 0 },
	{ 16, 11, 16, 0 }, { 16, 11, 15, 0 }, { 16, 11, 13, 0 }, { 16, 11, 12, 0 },
	{ 16, 11, 11, 0 }, { 16, 12, 11, 0 }, { 16, 13, 11, 0 }, { 16, 15, 11, 0 },
	{ 16, 16, 11, 0 }, { 15, 16, 11, 0 }, { 13, 16, 11, 0 }, { 12, 16, 11, 0 },
	{ 11, 16, 11, 0 }, { 11, 16, 12, 0 }, { 11, 16, 13, 0 }, { 11, 16, 15, 0 },
	{ 11, 16, 16, 0 }, { 11, 15, 16, 0 }, { 11, 13, 16, 0 }, { 11, 12, 16, 0 },
	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
	{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 63, 63, 63, 0 }
};

/* colors for the standard GUI dialogs (alerts, file selector, etc) */
int gui_fg_color = 255;
int gui_mg_color = 8;
int gui_bg_color = 0;

/* a block of temporary working memory */
void *_scratch_mem = NULL;
int _scratch_mem_size = 0;

/*
 * _al_malloc:
 * _al_free:
 * _al_realloc:
 * Wrappers for when a program needs to manipulate memory that has been
 *  allocated by the library.
 */
void *_al_malloc(size_t size) { return malloc(size); }
void _al_free(void *mem) { free(mem); }
void *_al_realloc(void *mem, size_t size) { return realloc(mem, size); }

/*
 * install_error:
 * Install errno ppinter..
 */
int *install_error(int *errno_ptr) {
	if (errno_ptr)
		allegro_errno = errno_ptr;
	else
		allegro_errno = &errno;
	return allegro_errno;
}

/*
 * alloc_screen:
 * Allocate default bitmap for screen.
 */
BITMAP *alloc_screen(int width, int height) {
	if (screen != NULL) {
		release_screen();
		destroy_bitmap(screen);
		SCREEN_W = 0;
		SCREEN_H = 0;
	}
	screen = create_bitmap(width, height);
	if (screen != NULL) {
		SCREEN_W = width;
		SCREEN_H = height;
	}
	return screen;
}


/* ======================================================================== */
/* Color blending routines (from Allegro 4.4.3 colblend.c)                  */
/* Interpolation routines by Cloud Wu and Burton Radons.                     */
/* Alpha blending optimised by Peter Cech.                                   */
/* ======================================================================== */

#define BLEND(bpp, r, g, b)   _blender_trans##bpp(makecol##bpp(r, g, b), y, n)
#define T(x, y, n)            (((y) - (x)) * (n) / 255 + (x))


#if (defined ALLEGRO_COLOR24) || (defined ALLEGRO_COLOR32)


#if (defined ALLEGRO_NO_ASM) || (!defined ALLEGRO_I386)

/* _blender_trans24:
 *  24 bit trans blender function.
 */
unsigned long _blender_trans24(unsigned long x, unsigned long y, unsigned long n)
{
   unsigned long res, g;

   if (n)
      n++;

   res = ((x & 0xFF00FF) - (y & 0xFF00FF)) * n / 256 + y;
   y &= 0xFF00;
   x &= 0xFF00;
   g = (x - y) * n / 256 + y;

   res &= 0xFF00FF;
   g &= 0xFF00;

   return res | g;
}

#endif      /* C version */



/* _blender_alpha24:
 *  Combines a 32 bit RGBA sprite with a 24 bit RGB destination.
 */
unsigned long _blender_alpha24(unsigned long x, unsigned long y, unsigned long n)
{
   unsigned long xx = makecol24(getr32(x), getg32(x), getb32(x));
   unsigned long res, g;

   n = geta32(x);

   if (n)
      n++;

   res = ((xx & 0xFF00FF) - (y & 0xFF00FF)) * n / 256 + y;
   y &= 0xFF00;
   xx &= 0xFF00;
   g = (xx - y) * n / 256 + y;

   res &= 0xFF00FF;
   g &= 0xFF00;

   return res | g;
}



/* _blender_alpha32:
 *  Combines a 32 bit RGBA sprite with a 32 bit RGB destination.
 */
unsigned long _blender_alpha32(unsigned long x, unsigned long y, unsigned long n)
{
   unsigned long res, g;

   n = geta32(x);

   if (n)
      n++;

   res = ((x & 0xFF00FF) - (y & 0xFF00FF)) * n / 256 + y;
   y &= 0xFF00;
   x &= 0xFF00;
   g = (x - y) * n / 256 + y;

   res &= 0xFF00FF;
   g &= 0xFF00;

   return res | g;
}



/* _blender_alpha24_bgr:
 *  Combines a 32 bit RGBA sprite with a 24 bit RGB destination, optimised
 *  for when one is in a BGR format and the other is RGB.
 */
static unsigned long _blender_alpha24_bgr(unsigned long x, unsigned long y, unsigned long n)
{
   unsigned long res, g;

   n = x >> 24;

   if (n)
      n++;

   x = ((x>>16)&0xFF) | (x&0xFF00) | ((x<<16)&0xFF0000);

   res = ((x & 0xFF00FF) - (y & 0xFF00FF)) * n / 256 + y;
   y &= 0xFF00;
   x &= 0xFF00;
   g = (x - y) * n / 256 + y;

   res &= 0xFF00FF;
   g &= 0xFF00;

   return res | g;
}



/* _blender_add24:
 *  24 bit additive blender function.
 */
unsigned long _blender_add24(unsigned long x, unsigned long y, unsigned long n)
{
   int r = getr24(y) + getr24(x) * n / 256;
   int g = getg24(y) + getg24(x) * n / 256;
   int b = getb24(y) + getb24(x) * n / 256;

   r = MIN(r, 255);
   g = MIN(g, 255);
   b = MIN(b, 255);

   return makecol24(r, g, b);
}


#endif      /* end of 24/32 bit routines */


#if (defined ALLEGRO_COLOR15) || (defined ALLEGRO_COLOR16)


/* _blender_trans16:
 *  16 bit trans blender function.
 */
unsigned long _blender_trans16(unsigned long x, unsigned long y, unsigned long n)
{
   unsigned long result;

   if (n)
      n = (n + 1) / 8;

   x = ((x & 0xFFFF) | (x << 16)) & 0x7E0F81F;
   y = ((y & 0xFFFF) | (y << 16)) & 0x7E0F81F;

   result = ((x - y) * n / 32 + y) & 0x7E0F81F;

   return ((result & 0xFFFF) | (result >> 16));
}



/* _blender_alpha16:
 *  Combines a 32 bit RGBA sprite with a 16 bit RGB destination.
 */
unsigned long _blender_alpha16(unsigned long x, unsigned long y, unsigned long n)
{
   unsigned long result;

   n = geta32(x);

   if (n)
      n = (n + 1) / 8;

   x = makecol16(getr32(x), getg32(x), getb32(x));

   x = (x | (x << 16)) & 0x7E0F81F;
   y = ((y & 0xFFFF) | (y << 16)) & 0x7E0F81F;

   result = ((x - y) * n / 32 + y) & 0x7E0F81F;

   return ((result & 0xFFFF) | (result >> 16));
}



/* _blender_alpha16_rgb */
static unsigned long _blender_alpha16_rgb(unsigned long x, unsigned long y, unsigned long n)
{
   unsigned long result;

   n = x >> 24;

   if (n)
      n = (n + 1) / 8;

   x = ((x>>3)&0x001F) | ((x>>5)&0x07E0) | ((x>>8)&0xF800);

   x = (x | (x << 16)) & 0x7E0F81F;
   y = ((y & 0xFFFF) | (y << 16)) & 0x7E0F81F;

   result = ((x - y) * n / 32 + y) & 0x7E0F81F;

   return ((result & 0xFFFF) | (result >> 16));
}



/* _blender_alpha16_bgr */
static unsigned long _blender_alpha16_bgr(unsigned long x, unsigned long y, unsigned long n)
{
   unsigned long result;

   n = x >> 24;

   if (n)
      n = (n + 1) / 8;

   x = ((x>>19)&0x001F) | ((x>>5)&0x07E0) | ((x<<8)&0xF800);

   x = (x | (x << 16)) & 0x7E0F81F;
   y = ((y & 0xFFFF) | (y << 16)) & 0x7E0F81F;

   result = ((x - y) * n / 32 + y) & 0x7E0F81F;

   return ((result & 0xFFFF) | (result >> 16));
}



/* _blender_add16:
 *  16 bit additive blender function.
 */
unsigned long _blender_add16(unsigned long x, unsigned long y, unsigned long n)
{
   int r = getr16(y) + getr16(x) * n / 256;
   int g = getg16(y) + getg16(x) * n / 256;
   int b = getb16(y) + getb16(x) * n / 256;

   r = MIN(r, 255);
   g = MIN(g, 255);
   b = MIN(b, 255);

   return makecol16(r, g, b);
}



/* _blender_trans15:
 *  15 bit trans blender function.
 */
unsigned long _blender_trans15(unsigned long x, unsigned long y, unsigned long n)
{
   unsigned long result;

   if (n)
      n = (n + 1) / 8;

   x = ((x & 0xFFFF) | (x << 16)) & 0x3E07C1F;
   y = ((y & 0xFFFF) | (y << 16)) & 0x3E07C1F;

   result = ((x - y) * n / 32 + y) & 0x3E07C1F;

   return ((result & 0xFFFF) | (result >> 16));
}



/* _blender_alpha15:
 *  Combines a 32 bit RGBA sprite with a 15 bit RGB destination.
 */
unsigned long _blender_alpha15(unsigned long x, unsigned long y, unsigned long n)
{
   unsigned long result;

   n = geta32(x);

   if (n)
      n = (n + 1) / 8;

   x = makecol15(getr32(x), getg32(x), getb32(x));

   x = (x | (x << 16)) & 0x3E07C1F;
   y = ((y & 0xFFFF) | (y << 16)) & 0x3E07C1F;

   result = ((x - y) * n / 32 + y) & 0x3E07C1F;

   return ((result & 0xFFFF) | (result >> 16));
}



/* _blender_alpha15_rgb */
static unsigned long _blender_alpha15_rgb(unsigned long x, unsigned long y, unsigned long n)
{
   unsigned long result;

   n = x >> 24;

   if (n)
      n = (n + 1) / 8;

   x = ((x>>3)&0x001F) | ((x>>6)&0x03E0) | ((x>>9)&0xEC00);

   x = (x | (x << 16)) & 0x3E07C1F;
   y = ((y & 0xFFFF) | (y << 16)) & 0x3E07C1F;

   result = ((x - y) * n / 32 + y) & 0x3E07C1F;

   return ((result & 0xFFFF) | (result >> 16));
}



/* _blender_alpha15_bgr */
static unsigned long _blender_alpha15_bgr(unsigned long x, unsigned long y, unsigned long n)
{
   unsigned long result;

   n = x >> 24;

   if (n)
      n = (n + 1) / 8;

   x = ((x>>19)&0x001F) | ((x>>6)&0x03E0) | ((x<<7)&0xEC00);

   x = (x | (x << 16)) & 0x3E07C1F;
   y = ((y & 0xFFFF) | (y << 16)) & 0x3E07C1F;

   result = ((x - y) * n / 32 + y) & 0x3E07C1F;

   return ((result & 0xFFFF) | (result >> 16));
}



/* _blender_add15:
 *  15 bit additive blender function.
 */
unsigned long _blender_add15(unsigned long x, unsigned long y, unsigned long n)
{
   int r = getr15(y) + getr15(x) * n / 256;
   int g = getg15(y) + getg15(x) * n / 256;
   int b = getb15(y) + getb15(x) * n / 256;

   r = MIN(r, 255);
   g = MIN(g, 255);
   b = MIN(b, 255);

   return makecol15(r, g, b);
}


#endif      /* end of 15/16 bit routines */



#ifdef ALLEGRO_COLOR16
   #define BF16(name)   name
#else
   #define BF16(name)   _blender_black
#endif


#if (defined ALLEGRO_COLOR24) || (defined ALLEGRO_COLOR32)
   #define BF24(name)   name
#else
   #define BF24(name)   _blender_black
#endif



/* these functions are all the same, so we can generate them with a macro */
#define SET_BLENDER_FUNC(name)                                 \
   void set_##name##_blender(int r, int g, int b, int a)       \
   {                                                           \
      set_blender_mode(BF16(_blender_##name##15),              \
                       BF16(_blender_##name##16),              \
                       BF24(_blender_##name##24),              \
                       r, g, b, a);                            \
   }


SET_BLENDER_FUNC(trans);
SET_BLENDER_FUNC(add);



/* set_alpha_blender:
 *  Sets the special RGBA blending mode.
 */
void set_alpha_blender(void)
{
   BLENDER_FUNC f15, f16, f24, f32;
   int r, b;

   /* check which way around the 32 bit pixels are */
   if ((_rgb_g_shift_32 == 8) && (_rgb_a_shift_32 == 24)) {
      r = (_rgb_r_shift_32) ? 1 : 0;
      b = (_rgb_b_shift_32) ? 1 : 0;
   }
   else
      r = b = 0;

   #ifdef ALLEGRO_COLOR16

      /* decide which 15 bit blender to use */
      if ((_rgb_r_shift_15 == r*10) && (_rgb_g_shift_15 == 5) && (_rgb_b_shift_15 == b*10))
         f15 = _blender_alpha15_rgb;
      else if ((_rgb_r_shift_15 == b*10) && (_rgb_g_shift_15 == 5) && (_rgb_b_shift_15 == r*10))
         f15 = _blender_alpha15_bgr;
      else
         f15 = _blender_alpha15;

      /* decide which 16 bit blender to use */
      if ((_rgb_r_shift_16 == r*11) && (_rgb_g_shift_16 == 5) && (_rgb_b_shift_16 == b*11))
         f16 = _blender_alpha16_rgb;
      else if ((_rgb_r_shift_16 == b*11) && (_rgb_g_shift_16 == 5) && (_rgb_b_shift_16 == r*11))
         f16 = _blender_alpha16_bgr;
      else
         f16 = _blender_alpha16;

   #else

      f15 = _blender_black;
      f16 = _blender_black;

   #endif

   #ifdef ALLEGRO_COLOR24

      /* decide which 24 bit blender to use */
      if ((_rgb_r_shift_24 == r*16) && (_rgb_g_shift_24 == 8) && (_rgb_b_shift_24 == b*16))
         f24 = _blender_alpha32;
      else if ((_rgb_r_shift_24 == b*16) && (_rgb_g_shift_24 == 8) && (_rgb_b_shift_24 == r*16))
         f24 = _blender_alpha24_bgr;
      else
         f24 = _blender_alpha24;

   #else

      f24 = _blender_black;

   #endif

   #ifdef ALLEGRO_COLOR32
      f32 = _blender_alpha32;
   #else
      f32 = _blender_black;
   #endif

   set_blender_mode_ex(_blender_black, _blender_black, _blender_black,
                       f32, f15, f16, f24, 0, 0, 0, 0);
}



#ifdef ALLEGRO_COLOR32

/* _blender_write_alpha:
 *  Overlays an alpha channel onto an existing 32 bit RGBA bitmap.
 */
unsigned long _blender_write_alpha(unsigned long x, unsigned long y, unsigned long n)
{
   return (y & 0xFFFFFF) | (x << 24);
}

#endif



/* set_write_alpha_blender:
 *  Sets the special RGBA editing mode.
 */
void set_write_alpha_blender(void)
{
   BLENDER_FUNC f32;

   #ifdef ALLEGRO_COLOR32
      f32 = _blender_write_alpha;
   #else
      f32 = _blender_black;
   #endif

   set_blender_mode_ex(_blender_black, _blender_black, _blender_black,
                       f32,
                       _blender_black, _blender_black, _blender_black,
                       0, 0, 0, 0);
}


/* ======================================================================== */
/* RLE sprite creation/destruction — ported from Allegro 4 rle.c            */
/* ======================================================================== */

/* get_rle_sprite:
 *  Creates a run length encoded sprite based on the specified bitmap.
 *  The returned sprite is likely to be a lot smaller than the original
 *  bitmap, and can be drawn to the screen with draw_rle_sprite().
 *
 *  The compression is done individually for each line of the image.
 *  Format is a series of command bytes, 1-127 marks a run of that many
 *  solid pixels, negative numbers mark a gap of -n pixels, and 0 marks
 *  the end of a line (since zero can't occur anywhere else in the data,
 *  this can be used to find the start of a specified line when clipping).
 *  For truecolor RLE sprites, the data and command bytes are both in the
 *  same format (16 or 32 bits, 24 bpp data is padded to 32 bit alignment),
 *  and the mask color (bright pink) is used as the EOL marker.
 */
RLE_SPRITE *get_rle_sprite(BITMAP *bitmap)
{
   int depth;
   RLE_SPRITE *s;
   int x, y;
   int run;
   int pix;
   int c;
   ASSERT(bitmap);

   depth = bitmap_color_depth(bitmap);

   #define RLE_WRITE8(x) {                                                    \
      _grow_scratch_mem(c+1);                                                 \
      p = (signed char *)_scratch_mem;                                        \
      p[c] = x;                                                               \
      c++;                                                                    \
   }

   #define RLE_WRITE16(x) {                                                   \
      _grow_scratch_mem((c+1)*sizeof(int16_t));                               \
      p = (int16_t *)_scratch_mem;                                            \
      p[c] = x;                                                               \
      c++;                                                                    \
   }

   #define RLE_WRITE32(x) {                                                   \
      _grow_scratch_mem((c+1)*sizeof(int32_t));                               \
      p = (int32_t *)_scratch_mem;                                            \
      p[c] = x;                                                               \
      c++;                                                                    \
   }

   /* helper for building an RLE run */
   #define DO_RLE(bits)                                                       \
   {                                                                          \
      for (y=0; y<bitmap->h; y++) {                                           \
         run = -1;                                                            \
         for (x=0; x<bitmap->w; x++) {                                        \
            pix = getpixel(bitmap, x, y) & 0xFFFFFF;                          \
            if (pix != bitmap->vtable->mask_color) {                          \
               if ((run >= 0) && (p[run] > 0) && (p[run] < 127))             \
                  p[run]++;                                                   \
               else {                                                         \
                  run = c;                                                    \
                  RLE_WRITE##bits(1);                                         \
               }                                                              \
               RLE_WRITE##bits(getpixel(bitmap, x, y));                       \
            }                                                                 \
            else {                                                            \
               if ((run >= 0) && (p[run] < 0) && (p[run] > -128))            \
                  p[run]--;                                                   \
               else {                                                         \
                  run = c;                                                    \
                  RLE_WRITE##bits(-1);                                        \
               }                                                              \
            }                                                                 \
         }                                                                    \
         RLE_WRITE##bits(bitmap->vtable->mask_color);                         \
      }                                                                       \
   }

   c = 0;

   switch (depth) {

   #ifdef ALLEGRO_COLOR8
      case 8:
         {
            signed char *p = (signed char *)_scratch_mem;
            DO_RLE(8);
         }
         break;
   #endif

   #ifdef ALLEGRO_COLOR16
      case 15:
      case 16:
         {
            signed short *p = (signed short *)_scratch_mem;
            DO_RLE(16);
            c *= sizeof(short);
         }
         break;
   #endif

   #if (defined ALLEGRO_COLOR24) || (defined ALLEGRO_COLOR32)
      case 24:
      case 32:
         {
            int32_t *p = (int32_t *)_scratch_mem;
            DO_RLE(32);
            c *= sizeof(int32_t);
         }
         break;
   #endif
   }

   s = _AL_MALLOC(sizeof(RLE_SPRITE) + c);

   if (s) {
      s->w = bitmap->w;
      s->h = bitmap->h;
      s->color_depth = depth;
      s->size = c;
      memcpy(s->dat, _scratch_mem, c);
   }

   #undef RLE_WRITE8
   #undef RLE_WRITE16
   #undef RLE_WRITE32
   #undef DO_RLE

   return s;
}


/* destroy_rle_sprite:
 *  Destroys an RLE sprite structure returned by get_rle_sprite().
 */
void destroy_rle_sprite(RLE_SPRITE *sprite)
{
   if (sprite)
      _AL_FREE(sprite);
}
