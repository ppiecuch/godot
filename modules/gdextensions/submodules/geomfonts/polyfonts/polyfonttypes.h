/**************************************************************************/
/*  polyfonttypes.h                                                       */
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

#ifndef __POLYFONTTYPES__
#define __POLYFONTTYPES__

/*
  Polyfonts is a polygon font drawing library for use with SDL. Any
  TTF font can be converted for use with this library. Contact the
  author for details.

  Copyright (C) 2003 Bob Pendleton

  Bob Pendleton
  Bob@Pendleton.com
*/

typedef unsigned char pfuint8;
typedef signed char pfint8;

typedef unsigned short int pfuint16;
typedef signed short int pfint16;

#define POLY_POINTS 0x0001
#define POLY_LINES 0x0002
#define POLY_LINE_LOOP 0x0004
#define POLY_LINE_STRIP 0x0008
#define POLY_TRIANGLES 0x0010
#define POLY_TRIANGLE_STRIP 0x0020
#define POLY_TRIANGLE_FAN 0x0040
#define POLY_QUADS 0x0080
#define POLY_QUAD_STRIP 0x0100
#define POLY_POLYGON 0x0200

#define pfFixScale (1 << 15)

typedef struct {
	float minx;
	float miny;
	float maxx;
	float maxy;
	float advance;
	pfuint16 glyph;
	pfuint16 numsegments;
	pfint16 *segments;
} pfglyph;

typedef struct {
	char *name;
	int loaded;
	float minx;
	float miny;
	float maxx;
	float maxy;
	int numglyphs;
	pfglyph *glyphs;
} pffont;

typedef struct {
	float x, y;
} pfpoint;

#endif // __POLYFONTTYPES__
