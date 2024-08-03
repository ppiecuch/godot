#ifndef _drawshape_h_
#define _drawshape_h_

#include "_internal.h"
#include "_surface.h"

#include "_begin_code.h"

extern int SDL_DrawPoint(SDL_Surface *dst, int x, int y, Uint32 color);
extern int SDL_DrawPoints(SDL_Surface *dst, const SDL_Point *points, int count, Uint32 color);

extern int SDL_BlendPoint(SDL_Surface *dst, int x, int y, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
extern int SDL_BlendPoints(SDL_Surface *dst, const SDL_Point *points, int count, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

extern int SDL_DrawLine(SDL_Surface *dst, int x1, int y1, int x2, int y2, Uint32 color);
extern int SDL_DrawLines(SDL_Surface *dst, const SDL_Point *points, int count, Uint32 color);

extern int SDL_BlendLine(SDL_Surface *dst, int x1, int y1, int x2, int y2, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
extern int SDL_BlendLines(SDL_Surface *dst, const SDL_Point *points, int count, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

extern int SDL_BlendFillRect(SDL_Surface *dst, const SDL_Rect *rect, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
extern int SDL_BlendFillRects(SDL_Surface *dst, const SDL_Rect *rects, int count, SDL_BlendMode blendMode, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

#include "_close_code.h"

#endif /* _drawshape_h_ */
