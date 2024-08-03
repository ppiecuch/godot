/*
 * gfx_primitives.h: graphics primitives for SDL
 *
 * Copyright (C) 2012-2014  Andreas Schiffler
 * Andreas Schiffler -- aschiffler at ferzkopp dot net
 */

#ifndef _gfx_primitives_h_
#define _gfx_primitives_h_

#include <math.h>
#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

#include "_surface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ----- Versioning */

#define GFX_PRIMITIVES_MAJOR 1
#define GFX_PRIMITIVES_MINOR 0
#define GFX_PRIMITIVES_MICRO 4

/* ---- Function Prototypes */

/* Note: all ___Color routines expect the color to be in format 0xRRGGBBAA */

/* Pixel */

DECLSPEC int pixelColor(SDL_Surface *surface, Sint16 x, Sint16 y, Uint32 color);
DECLSPEC int pixelRGBA(SDL_Surface *surface, Sint16 x, Sint16 y, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Horizontal line */

DECLSPEC int hlineColor(SDL_Surface *surface, Sint16 x1, Sint16 x2, Sint16 y, Uint32 color);
DECLSPEC int hlineRGBA(SDL_Surface *surface, Sint16 x1, Sint16 x2, Sint16 y, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Vertical line */

DECLSPEC int vlineColor(SDL_Surface *surface, Sint16 x, Sint16 y1, Sint16 y2, Uint32 color);
DECLSPEC int vlineRGBA(SDL_Surface *surface, Sint16 x, Sint16 y1, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Rectangle */

DECLSPEC int rectangleColor(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);
DECLSPEC int rectangleRGBA(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Rounded-Corner Rectangle */

DECLSPEC int roundedRectangleColor(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 rad, Uint32 color);
DECLSPEC int roundedRectangleRGBA(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Filled rectangle (Box) */

DECLSPEC int boxColor(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);
DECLSPEC int boxRGBA(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Rounded-Corner Filled rectangle (Box) */

DECLSPEC int roundedBoxColor(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 rad, Uint32 color);
DECLSPEC int roundedBoxRGBA(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Line */

DECLSPEC int lineColor(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);
DECLSPEC int lineRGBA(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* AA Line */

DECLSPEC int aalineColor(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color);
DECLSPEC int aalineRGBA(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Thick Line */
DECLSPEC int thickLineColor(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 width, Uint32 color);
DECLSPEC int thickLineRGBA(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint8 width, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Circle */

DECLSPEC int circleColor(SDL_Surface *surface, Sint16 x, Sint16 y, Sint16 rad, Uint32 color);
DECLSPEC int circleRGBA(SDL_Surface *surface, Sint16 x, Sint16 y, Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Arc */

DECLSPEC int arcColor(SDL_Surface *surface, Sint16 x, Sint16 y, Sint16 rad, Sint16 start, Sint16 end, Uint32 color);
DECLSPEC int arcRGBA(SDL_Surface *surface, Sint16 x, Sint16 y, Sint16 rad, Sint16 start, Sint16 end, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* AA Circle */

DECLSPEC int aacircleColor(SDL_Surface *surface, Sint16 x, Sint16 y, Sint16 rad, Uint32 color);
DECLSPEC int aacircleRGBA(SDL_Surface *surface, Sint16 x, Sint16 y, Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Filled Circle */

DECLSPEC int filledCircleColor(SDL_Surface *surface, Sint16 x, Sint16 y, Sint16 r, Uint32 color);
DECLSPEC int filledCircleRGBA(SDL_Surface *surface, Sint16 x, Sint16 y, Sint16 rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Ellipse */

DECLSPEC int ellipseColor(SDL_Surface *surface, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint32 color);
DECLSPEC int ellipseRGBA(SDL_Surface *surface, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* AA Ellipse */

DECLSPEC int aaellipseColor(SDL_Surface *surface, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint32 color);
DECLSPEC int aaellipseRGBA(SDL_Surface *surface, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Filled Ellipse */

DECLSPEC int filledEllipseColor(SDL_Surface *surface, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint32 color);
DECLSPEC int filledEllipseRGBA(SDL_Surface *surface, Sint16 x, Sint16 y, Sint16 rx, Sint16 ry, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Pie */

DECLSPEC int pieColor(SDL_Surface *surface, Sint16 x, Sint16 y, Sint16 rad, Sint16 start, Sint16 end, Uint32 color);
DECLSPEC int pieRGBA(SDL_Surface *surface, Sint16 x, Sint16 y, Sint16 rad, Sint16 start, Sint16 end, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Filled Pie */

DECLSPEC int filledPieColor(SDL_Surface *surface, Sint16 x, Sint16 y, Sint16 rad, Sint16 start, Sint16 end, Uint32 color);
DECLSPEC int filledPieRGBA(SDL_Surface *surface, Sint16 x, Sint16 y, Sint16 rad, Sint16 start, Sint16 end, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Trigon */

DECLSPEC int trigonColor(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint32 color);
DECLSPEC int trigonRGBA(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* AA-Trigon */

DECLSPEC int aatrigonColor(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint32 color);
DECLSPEC int aatrigonRGBA(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Filled Trigon */

DECLSPEC int filledTrigonColor(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint32 color);
DECLSPEC int filledTrigonRGBA(SDL_Surface *surface, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Sint16 x3, Sint16 y3, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Polygon */

DECLSPEC int polygonColor(SDL_Surface *surface, const Sint16 *vx, const Sint16 *vy, int n, Uint32 color);
DECLSPEC int polygonRGBA(SDL_Surface *surface, const Sint16 *vx, const Sint16 *vy, int n, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* AA-Polygon */

DECLSPEC int aapolygonColor(SDL_Surface *surface, const Sint16 *vx, const Sint16 *vy, int n, Uint32 color);
DECLSPEC int aapolygonRGBA(SDL_Surface *surface, const Sint16 *vx, const Sint16 *vy, int n, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Filled Polygon */

DECLSPEC int filledPolygonColor(SDL_Surface *surface, const Sint16 *vx, const Sint16 *vy, int n, Uint32 color);
DECLSPEC int filledPolygonRGBA(SDL_Surface *surface, const Sint16 *vx, const Sint16 *vy, int n, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Textured Polygon */

DECLSPEC int texturedPolygon(SDL_Surface *surface, const Sint16 *vx, const Sint16 *vy, int n, SDL_Surface *texture, int texture_dx, int texture_dy);

/* Bezier */

DECLSPEC int bezierColor(SDL_Surface *surface, const Sint16 *vx, const Sint16 *vy, int n, int s, Uint32 color);
DECLSPEC int bezierRGBA(SDL_Surface *surface, const Sint16 *vx, const Sint16 *vy, int n, int s, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* Characters/Strings */

DECLSPEC void gfxPrimitivesSetFont(const void *fontdata, Uint32 cw, Uint32 ch);
DECLSPEC void gfxPrimitivesSetFontRotation(Uint32 rotation);
DECLSPEC int characterColor(SDL_Surface *surface, Sint16 x, Sint16 y, char c, Uint32 color);
DECLSPEC int characterRGBA(SDL_Surface *surface, Sint16 x, Sint16 y, char c, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
DECLSPEC int stringColor(SDL_Surface *surface, Sint16 x, Sint16 y, const char *s, Uint32 color);
DECLSPEC int stringRGBA(SDL_Surface *surface, Sint16 x, Sint16 y, const char *s, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

#ifdef __cplusplus
}
#endif

#endif /* _gfx_primitives_h_ */
