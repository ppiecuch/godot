/*************************************************************************/
/*  dblunt.c                                                             */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

// Dutch-Blunt low poly font system.
// (c)2015 Abraham Stolk

#include "dblunt.h" // our own interface.

#include <assert.h>
#include <string.h>

#include "vdata.h" // the font data.

#ifdef _MSC_VER
#define __restrict__
#endif

// Convert an ascii string to a stream of vertex data.
// Returns the number of triangles(x,y,x,y,x,y) written to the buffer.
int dblunt_string_to_vertices(
		const char *str, // characters to render.
		float *destbuf, // output buffer to hold vertices.
		int destbufsz, // size of output buffer in bytes.
		float posx, float posy, // target position for text render.
		float sclx, float scly, // scale of the text render.
		float *__restrict__ textw, // out: size of longest line.
		float *__restrict__ texth // out: height of text.
) {
	int trias_written = 0;
	float *buf_writer = destbuf;
	const int tri_capacity = destbufsz / (3 * 2 * sizeof(float));
	sclx *= 0.2f; // The 'M' glyph has size 4x5, so we compensate.
	scly *= 0.2f;
	const int l = (int)strlen(str);
	const float linepitch = 1.25f * 5.0f * scly;
	const float linespacing = 0.25f * 5.0f * scly;
	float x = posx;
	float y = posy - 5.0f * scly;
	float maxx = x;
	int numlines = 1;
	for (int charnr = 0; charnr < l; ++charnr) {
		int c = str[charnr];
		if (c == '\n') { // carriage return.
			y -= linepitch;
			x = posx;
			numlines += 1;
			continue;
		}
		if (c == ' ') {
			x += 3.0f * sclx;
			continue;
		}
		if (c > 127) {
			continue; // only render ascii 0 to 127
		}
		const float width = widths[c];
		const int sz = sizes[c];
		const int voffs = vdataoffsets[c];
		const int trias = sz / 3;
		if (x + width * sclx > maxx)
			maxx = x + width * sclx;
		if (trias_written + trias <= tri_capacity) {
			for (int vnr = 0; vnr < sz; ++vnr) {
				const float vx = vdata[voffs + vnr][0];
				const float vy = vdata[voffs + vnr][1];
				*buf_writer++ = x + vx * sclx;
				*buf_writer++ = y + vy * scly;
			}
			x += (width + 1.0f) * sclx; // advance to next pos
			trias_written += trias;
		}
	}
	*texth = numlines * linepitch - 1 * linespacing;
	*textw = maxx - posx;
	return trias_written;
}
