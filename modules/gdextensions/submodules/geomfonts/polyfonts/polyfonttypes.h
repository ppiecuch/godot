#ifndef __POLYFONTTYPES__
#define __POLYFONTTYPES__

/*
  Polyfonts is a polygon font drawing library for use with SDL. Any
  TTF font can be converted for use with this library. Contact the
  author for details.

  Copyright (C) 2003 Bob Pendleton

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  as published by the Free Software Foundation; either version 2.1
  of the License, or (at your option) any later version.
    
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
    
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
  02111-1307 USA

  If you do not wish to comply with the terms of the LGPL please
  contact the author as other terms are available for a fee.
    
  Bob Pendleton
  Bob@Pendleton.com
*/

typedef unsigned char pfuint8;
typedef signed char pfint8;

typedef unsigned short int pfuint16;
typedef signed short int pfint16;

#define	POLY_POINTS			0x0001
#define	POLY_LINES			0x0002
#define	POLY_LINE_LOOP		0x0004
#define	POLY_LINE_STRIP		0x0008
#define	POLY_TRIANGLES		0x0010
#define	POLY_TRIANGLE_STRIP	0x0020
#define	POLY_TRIANGLE_FAN	0x0040
#define	POLY_QUADS			0x0080
#define	POLY_QUAD_STRIP		0x0100
#define	POLY_POLYGON		0x0200

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
