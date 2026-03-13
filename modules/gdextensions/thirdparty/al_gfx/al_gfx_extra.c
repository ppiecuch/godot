/*
 * al_gfx_extra.c — Stub implementations for al_gfx functions that are
 * declared in al_gfx.h but not yet ported from the full Allegro 4 source.
 */

#include "al_gfx.h"


/* ======================================================================== */
/* Stretch blit stubs (TODO: port from Allegro 4 cstretch.c)                */
/* ======================================================================== */

void stretch_blit(BITMAP *source, BITMAP *dest, int s_x, int s_y, int s_w, int s_h,
                  int d_x, int d_y, int d_w, int d_h)
{
   (void)source; (void)dest;
   (void)s_x; (void)s_y; (void)s_w; (void)s_h;
   (void)d_x; (void)d_y; (void)d_w; (void)d_h;
}

void masked_stretch_blit(BITMAP *source, BITMAP *dest, int s_x, int s_y, int s_w, int s_h,
                         int d_x, int d_y, int d_w, int d_h)
{
   (void)source; (void)dest;
   (void)s_x; (void)s_y; (void)s_w; (void)s_h;
   (void)d_x; (void)d_y; (void)d_w; (void)d_h;
}
