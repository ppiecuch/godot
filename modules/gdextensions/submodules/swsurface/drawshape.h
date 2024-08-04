/**************************************************************************/
/*  drawshape.h                                                           */
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
