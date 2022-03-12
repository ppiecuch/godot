/*************************************************************************/
/*  polyfonts.cpp                                                        */
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

/*
 Polyfonts is a polygon font drawing library for use with SDL. Any
 TTF font can be converted for use with this library. Contact the
 author for details.

 Copyright (C) 2003 Bob Pendleton

 Bob Pendleton
 Bob@Pendleton.com
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "polyfonts.h"

#include "core/error_macros.h"
#include "core/math/math_funcs.h"
#include "core/print_string.h"
#include "core/variant.h"

#define PI (3.1415926535897932384626433)
#define RtoD (180.0 / PI)
#define DtoR (PI / 180.0)

//  The following code sets the default compiled in
//  font. You can change the default font by
//  including a different font and changing the
//  declaration of the two variables. Or, you can
//  set them both to nullptr if you don't want a
//  default font.

#include "fonts/pfPSans16.h"

const pffont *getDefaultFont() {
	return (&pfPSans16);
}

/*-----------------------------------------------*/
pfpoint weightOffset[] = {
	{ 0.0, 0.0 },
	{ 0.0, 1.0 },
	{ 1.0, 1.0 },
	{ 1.0, 0.0 },
	//
	{ 0.0, 2.0 },
	{ 1.0, 2.0 },
	{ 2.0, 2.0 },
	{ 1.0, 2.0 },
	{ 0.0, 2.0 },
};

typedef struct {
	const char *name;
	int value;
} nameValue;

static nameValue glPrims[] = {
	{ "POLY_POINTS", POLY_POINTS },
	{ "POLY_LINES", POLY_LINES },
	{ "POLY_LINE_LOOP", POLY_LINE_LOOP },
	{ "POLY_LINE_STRIP", POLY_LINE_STRIP },
	{ "POLY_TRIANGLES", POLY_TRIANGLES },
	{ "POLY_TRIANGLE_STRIP", POLY_TRIANGLE_STRIP },
	{ "POLY_TRIANGLE_FAN", POLY_TRIANGLE_FAN },
	{ "POLY_QUADS", POLY_QUADS },
	{ "POLY_QUAD_STRIP", POLY_QUAD_STRIP },
	{ "POLY_POLYGON", POLY_POLYGON },
};

#define numWeights (sizeof(weightOffset) / sizeof(pfpoint))
#define unfix(value) ((float)(value)) / ((float)pfFixScale)

int lookupGlOp(char *op) {
	for (int i = 0; i < (sizeof(glPrims) / sizeof(nameValue)); i++) {
		if (0 == strcmp(glPrims[i].name, op)) {
			return i;
		}
	}
	return -1;
}

PolyFont::PolyFont(const pffont *f) {
	pfScaleX = DefaultScale;
	pfScaleY = DefaultScale;

	pfTextX = 0.0;
	pfTextY = 0.0;

	pfTextSkew = 0.0;
	pfTextWeight = 1;

	pfTextSin = 0.0;
	pfTextCos = 1.0;

	pfCenter = false;

	currentFont = nullptr;

	if (f != nullptr) {
		setFont(f);
	}
}

PolyFont::~PolyFont() {
	unloadFont();
}

float PolyFont::skew(float x, float y) const {
	return x + (pfTextSkew * y);
}

int PolyFont::setFont(const pffont *f) {
	if (nullptr != f) {
		currentFont = (pffont *)f;

		return 0;
	}

	return -1;
}

void PolyFont::validate(const pffont *font) {
	pfglyph *glyph = nullptr;

	if (nullptr == font) {
		WARN_PRINT("font is nullptr");
	}

	if (nullptr == font->name) {
		WARN_PRINT("fontname is NULL");
	}

	print_verbose(vformat("fontinfo = %s min:%s, max:%s, %d %p",
			font->name,
			Size2(font->minx, font->miny),
			Size2(font->maxx, font->maxy),
			font->numglyphs,
			font->glyphs));

	glyph = font->glyphs;
	if (nullptr == glyph) {
		WARN_PRINT("glyph point is nullptr");
	}

	print_verbose(vformat("NumGlyphs = %d", font->numglyphs));

	for (int i = 0; i < font->numglyphs; i++) {
		if (nullptr == glyph[i].segments) {
			printf("glyph[%d].segments = nullptr\n", i);
		}

		print_verbose(vformat("glyph[" + String::num(i) + "] = min:%s, max:%s, adv:%f, %hu, %hu",
				Size2(glyph[i].minx, glyph[i].miny),
				Size2(glyph[i].maxx, glyph[i].maxy),
				glyph[i].advance,
				glyph[i].glyph,
				glyph[i].numsegments));
	}
}

void PolyFont::unloadFont() {
	pfglyph *glyphs = nullptr;

	if (nullptr == currentFont) {
		return;
	}

	if (getDefaultFont() == currentFont) {
		return;
	}

	if (1 != currentFont->loaded) {
		return;
	}

	if (nullptr == currentFont->name) {
		return;
	}
	free(currentFont->name);

	if (nullptr == currentFont->glyphs) {
		return;
	}

	glyphs = currentFont->glyphs;
	for (int i = 0; i < currentFont->numglyphs; i++) {
		if (nullptr != glyphs[i].segments) {
			free(glyphs[i].segments);
		}
	}

	free(currentFont->glyphs);
	free(currentFont);
	currentFont = nullptr;
}

pffont *PolyFont::loadFont(const char *fileName) {
	char buf[1024];

	float version = 0;
	int glyphcount = 0;
	char *fontname = nullptr;
	pffont *fontinfo = nullptr;
	pfglyph *glyphs = nullptr;

	FILE *f = fopen(fileName, "r");
	if (nullptr == f) {
		return nullptr;
	}

	while (nullptr != fgets(buf, sizeof(buf), f)) {
		if (0 == strcmp("/*PolyFontVersion\n", buf)) {
			fscanf(f, "%f\n", &version);
		} else if (0 == strcmp("/*fontinfo\n", buf)) {
			fontinfo = (pffont *)calloc(1, sizeof(pffont));
			if (nullptr == fontinfo) {
				fclose(f);
				return nullptr;
			}
			fgets(buf, sizeof(buf), f); /* skip a line */
			fscanf(f,
					"%f, %f, %f, %f, %d\n",
					&fontinfo->minx, &fontinfo->miny,
					&fontinfo->maxx, &fontinfo->maxy,
					&fontinfo->numglyphs);
			fontinfo->name = fontname;
			fontinfo->glyphs = glyphs;
			fontinfo->loaded = 1;
		} else if (0 == strcmp("/*fontname\n", buf)) {
			if (nullptr != fgets(buf, sizeof(buf), f)) {
				int len = strlen(buf);

				if (len >= sizeof(buf)) {
					fclose(f);
					return nullptr;
				}

				buf[len - 1] = '\0';

				fontname = (char *)calloc(len, sizeof(char));
				if (nullptr == fontname) {
					fclose(f);
					return nullptr;
				}

				strncpy(fontname, buf, len);
			}
		} else if (0 == strcmp("/*glyphcount\n", buf)) {
			fscanf(f, "%d\n", &glyphcount);

			glyphs = (pfglyph *)calloc(glyphcount, sizeof(pfglyph));
			if (nullptr == glyphs) {
				fclose(f);
				return nullptr;
			}
		} else if (0 == strcmp("/*glyphinfo\n", buf)) {
			int n = 0;
			fscanf(f, "%d\n", &n); /* glyph index */

			fgets(buf, sizeof(buf), f); /* skip a line */
			fscanf(f,
					"%f, %f, %f, %f, %f, %hu, %hu\n",
					&glyphs[n].minx, &glyphs[n].miny,
					&glyphs[n].maxx, &glyphs[n].maxy,
					&glyphs[n].advance,
					&glyphs[n].glyph,
					&glyphs[n].numsegments);
		} else if (0 == strcmp("/*glyphdata\n", buf)) {
			int n;
			int size;
			int i, j;
			int segs;
			char op[1024];
			int points;
			pfint16 *data = nullptr;

			fscanf(f, "%d,%d\n", &n, &size);

			data = (pfint16 *)calloc(size, sizeof(pfuint16));
			if (nullptr == data) {
				fclose(f);
				return nullptr;
			}
			glyphs[n].segments = data;

			for (i = 0; i < size; /**/) {
				while ((nullptr != fgets(buf, sizeof(buf), f)) &&
						(0 != strcmp("/*segment\n", buf))) {
				}
				fscanf(f, "%d\n", &segs);

				fgets(buf, sizeof(buf), f); /* skip a line */
				fscanf(f, "%s\n", &op[0]);
				fgets(buf, sizeof(buf), f); /* skip a line */
				fscanf(f, "%d\n", &points);

				data[i] = lookupGlOp(op);
				i++;
				data[i] = points;
				i++;

				for (j = 0; j < points; j++) {
					fgets(buf, sizeof(buf), f); /* skip a line */
					fscanf(f, "%hd,%hd\n", &data[i], &data[i + 1]);

					i += 2;
				}
			}
		}
	}

	fclose(f);
	return fontinfo;
}

void PolyFont::setScale(float s) {
	pfScaleX = pfScaleY = s;
}

void PolyFont::setScaleXY(float sx, float sy) {
	pfScaleX = sx;
	pfScaleY = sy;
}

void PolyFont::setPosition(float x, float y) {
	pfTextX = x;
	pfTextY = y;
}

pfpoint PolyFont::getPosition() const {
	return (pfpoint){ pfTextX, pfTextY };
}

void PolyFont::setSkew(float s) {
	pfTextSkew = MIN(1.0, MAX(-1.0, s));
}

void PolyFont::setWeight(int w) {
	pfTextWeight = MIN(numWeights, MAX(1, w));
}

void PolyFont::setAngleR(float a) {
	pfTextSin = Math::sin(a);
	pfTextCos = Math::cos(a);
}

void PolyFont::setAngleD(float a) {
	setAngleR(a * DtoR);
}

void PolyFont::setCenter(bool onOff) {
	pfCenter = onOff;
}

int PolyFont::_getCharBBox(wchar_t c, float *minx, float *miny, float *maxx, float *maxy) {
	if (nullptr != currentFont) {
		pfglyph *g = getGlyph(c);
		if (nullptr != g) {
			*minx = g->minx;
			*miny = g->miny;

			*maxx = g->maxx;
			*maxy = g->maxy;
		}
		return 0;
	}

	*minx = 0;
	*miny = 0;
	*maxx = 0;
	*maxy = 0;

	return -1;
}

int PolyFont::getStringBox(const char *c, float *minx, float *miny, float *maxx, float *maxy) {
	float x1, y1, x2, y2;

	if (nullptr == c) {
		return -1;
	}

	if (-1 == _getCharBBox(*c, &x1, &y1, &x2, &y2)) {
		return -1;
	}

	*minx = x1;
	*miny = y1;
	*maxx = _getCharAdvance(*c);
	*maxy = y2;

	c++;

	while (0 != *c) {
		if (-1 == _getCharBBox(*c, &x1, &y1, &x2, &y2)) {
			return -1;
		}

		*miny = MIN(*miny, y1);
		*maxx += _getCharAdvance(*c);
		*maxy = MAX(*maxy, y2);

		c++;
	}

	return 0;
}

int PolyFont::getStringBoxW(const wchar_t *c, float *minx, float *miny, float *maxx, float *maxy) {
	float x1, y1, x2, y2;

	if (nullptr == c) {
		return -1;
	}

	if (-1 == _getCharBBox(*c, &x1, &y1, &x2, &y2)) {
		return -1;
	}

	*minx = x1;
	*miny = y1;
	*maxx = _getCharAdvance(*c);
	*maxy = y2;

	c++;

	while (0 != *c) {
		if (-1 == _getCharBBox(*c, &x1, &y1, &x2, &y2)) {
			return -1;
		}

		*miny = MIN(*miny, y1);
		*maxx += _getCharAdvance(*c);
		*maxy = MAX(*maxy, y2);

		c++;
	}

	return 0;
}

int PolyFont::setScaleBox(char *c, float w, float h) {
	float x1, y1, x2, y2;

	if (nullptr == c) {
		return -1;
	}

	if (-1 == getStringBox(c, &x1, &y1, &x2, &y2)) {
		return -1;
	}

	setScaleXY((w / (x2 - x1)), (h / (y2 - y1)));
	return 0;
}

int PolyFont::setScaleBoxW(wchar_t *c, float w, float h) {
	float x1, y1, x2, y2;

	if (nullptr == c) {
		return -1;
	}

	if (-1 == getStringBoxW(c, &x1, &y1, &x2, &y2)) {
		return -1;
	}

	setScaleXY((w / (x2 - x1)), (h / (y2 - y1)));
	return 0;
}

char *PolyFont::getFontName() {
	char *name = nullptr;

	if (nullptr != currentFont) {
		name = currentFont->name;
	}

	return name;
}

const pffont *PolyFont::getCurrentFont() {
	return currentFont;
}

int PolyFont::getFontBBox(float *minx, float *miny, float *maxx, float *maxy) {
	if (nullptr != currentFont) {
		*minx = pfScaleX * currentFont->minx;
		*miny = pfScaleY * currentFont->miny;

		*maxx = pfScaleX * currentFont->maxx;
		*maxy = pfScaleY * currentFont->maxy;

		if (pfTextSkew > 0) {
			*minx = skew(*minx, *miny);
			*maxx = skew(*maxx, *maxy);
		} else {
			*minx = skew(*minx, *maxy);
			*maxx = skew(*maxx, *miny);
		}

		return 0;
	}

	*minx = 0;
	*miny = 0;
	*maxx = 0;
	*maxy = 0;

	return -1;
}

float PolyFont::getFontHeight() {
	float minx, miny, maxx, maxy;

	if (-1 != getFontBBox(&minx, &miny, &maxx, &maxy)) {
		return maxy - miny;
	}

	return 0.0;
}

float PolyFont::getFontWidth() {
	float minx, miny, maxx, maxy;

	if (-1 != getFontBBox(&minx, &miny, &maxx, &maxy)) {
		return maxx - minx;
	}

	return 0.0;
}

float PolyFont::getFontAscent() {
	float minx, miny, maxx, maxy;

	if (-1 != getFontBBox(&minx, &miny, &maxx, &maxy)) {
		return maxy;
	}

	return 0.0;
}

float PolyFont::getFontDescent() {
	float minx, miny, maxx, maxy;

	if (-1 != getFontBBox(&minx, &miny, &maxx, &maxy)) {
		return miny;
	}

	return 0.0;
}

int PolyFont::getFontNumGlyphs() {
	if (nullptr != currentFont) {
		return currentFont->numglyphs;
	}

	return 0;
}

wchar_t PolyFont::getChar(int g) {
	wchar_t c = 0;
	int ng = -1;

	if (nullptr != currentFont) {
		ng = currentFont->numglyphs;
		if ((g >= 0) && (g < ng)) {
			c = currentFont->glyphs[g].glyph;
		}
	}

	return c;
}

static int comp(const void *key, const void *target) {
	pfglyph *k = (pfglyph *)key;
	pfglyph *t = (pfglyph *)target;

	return (k->glyph) - (t->glyph);
}

pfglyph *PolyFont::findGlyph(pfglyph *glyphs, int numglyphs, pfglyph *find) const {
	return (pfglyph *)bsearch((void *)find, (void *)glyphs, numglyphs, sizeof(pfglyph), comp);
}

pfglyph *PolyFont::getGlyph(wchar_t c) const {
	pfglyph *g = nullptr;
	pfglyph key;

	if (nullptr == currentFont) {
		return nullptr;
	}

	key.glyph = c;
	g = findGlyph(currentFont->glyphs, currentFont->numglyphs, &key);

	return g;
}

float PolyFont::_getCharAdvance(wchar_t c) {
	pfglyph *g = getGlyph(c);

	if (nullptr == g) {
		return 0.0;
	}

	return g->advance;
}

float PolyFont::getCharAdvance(wchar_t c) {
	pfglyph *g = getGlyph(c);

	if (nullptr == g) {
		return 0.0;
	}

	return (g->advance * pfScaleX);
}

int PolyFont::getCharBBox(wchar_t c, float *minx, float *miny, float *maxx, float *maxy) {
	if (0 == _getCharBBox(c, minx, miny, maxx, maxy)) {
		*minx = pfScaleX * (*minx);
		*miny = pfScaleY * (*miny);

		*maxx = pfScaleX * (*maxx);
		*maxy = pfScaleY * (*maxy);

		if (pfTextSkew > 0) {
			*minx = skew(*minx, *miny);
			*maxx = skew(*maxx, *maxy);
		} else {
			*minx = skew(*minx, *maxy);
			*maxx = skew(*maxx, *miny);
		}

		return 0;
	}

	*minx = 0;
	*miny = 0;
	*maxx = 0;
	*maxy = 0;

	return -1;
}

float PolyFont::getCharHeight(wchar_t c) {
	float minx, miny, maxx, maxy;

	if (-1 != getCharBBox(c, &minx, &miny, &maxx, &maxy)) {
		return maxy - miny;
	}

	return 0.0;
}

float PolyFont::getCharWidth(wchar_t c) {
	float minx, miny, maxx, maxy;

	if (-1 != getCharBBox(c, &minx, &miny, &maxx, &maxy)) {
		return maxx - minx;
	}

	return 0.0;
}

float PolyFont::getCharAscent(wchar_t c) {
	float minx, miny, maxx, maxy;

	if (-1 != getCharBBox(c, &minx, &miny, &maxx, &maxy)) {
		return maxy;
	}

	return 0.0;
}

float PolyFont::getCharDescent(wchar_t c) {
	float minx, miny, maxx, maxy;

	if (-1 != getCharBBox(c, &minx, &miny, &maxx, &maxy)) {
		return miny;
	}

	return 0.0;
}

float PolyFont::getStringWidth(const char *c) {
	if (nullptr == c) {
		return 0;
	}

	float x1, y1, x2, y2;

	if (-1 == getStringBox(c, &x1, &y1, &x2, &y2)) {
		return 0;
	}

	return (x2 - x1);
};

float PolyFont::getStringWidth(const wchar_t *c) {
	if (nullptr == c) {
		return 0;
	}

	float x1, y1, x2, y2;

	if (-1 == getStringBoxW(c, &x1, &y1, &x2, &y2)) {
		return -1;
	}

	return (x2 - x1);
}

float PolyFont::getStringWidth(float s, const char *c) {
	return s * getStringWidth(c);
}

float PolyFont::getStringWidth(float s, const wchar_t *c) {
	return s * getStringWidth(c);
}

int PolyFont::drawWideChar(wchar_t c) {
	int i;
	int j;
	int k;
	pfglyph *g = getGlyph(c);
	pfint16 *d = nullptr;
	int segs = 0;
	int prim = 0;
	int points = 0;
	float gx, gy;
	float ox, oy;
	float tmp = -100.0;

	if (nullptr == g) {
		WARN_PRINT(vformat("drawWideChar: glyph not found (%c).", c));
		return -1;
	}

	ox = 0.0;
	oy = 0.0;
	if (pfCenter) {
		oy = pfScaleY * ((g->maxy + g->miny) / 2.0);
		ox = pfScaleX * ((g->maxx + g->minx) / 2.0);
	}

	for (k = 0; k < pfTextWeight; k++) {
		segs = g->numsegments;
		d = g->segments;

		for (i = 0; i < segs; i++) {
			prim = *d++;
			points = *d++;

			coord_vector vertices;
			for (j = 0; j < points; j++) {
				gx = unfix(*d++);
				gy = unfix(*d++);

				gx = (gx * pfScaleX);
				gy = (gy * pfScaleY);

				gx += weightOffset[k].x;
				gy += weightOffset[k].y;

				gx = skew(gx, gy);

				tmp = gx;
				gx = (pfTextX - ox) + ((pfTextCos * tmp) - (pfTextSin * gy));
				gy = (pfTextY + oy) - ((pfTextSin * tmp) + (pfTextCos * gy));
				vertices.push_back((pfpoint){ gx, gy });
			}
			polyDrawElements(prim, vertices);
		}
	}

	tmp = (g->advance * pfScaleX);
	pfTextX += tmp * pfTextCos;
	pfTextY -= tmp * pfTextSin;

	return 0;
}

int PolyFont::drawChar(char c) {
	return drawWideChar(c);
}

int PolyFont::drawString(const wchar_t *c) {
	if (nullptr == c) {
		return -1;
	}
	beginStringDraw();
	while (0 != *c) {
		drawWideChar(*c);
		c++;
	}
	doneStringDraw();

	return 0;
}

int PolyFont::drawString(const char *c) {
	if (nullptr == c) {
		return -1;
	}
	beginStringDraw();
	while (0 != *c) {
		drawWideChar(*c);
		c++;
	}
	doneStringDraw();

	return 0;
}

int PolyFont::drawString(float x, float y, const char *c) {
	setPosition(x, y);
	return drawString(c);
}

int PolyFont::drawString(float x, float y, float sx, float sy, const char *c) {
	setPosition(x, y);
	setScaleXY(sx, sy);
	return drawString(c);
}

void PolyFont::test(int screenWidth) {
	for (int i = 0; i < getFontNumGlyphs(); i++) {
		wchar_t glyph = getChar(i);

		if ((pfTextX + getCharAdvance(glyph)) > screenWidth) {
			pfTextX = 0;
			pfTextY += getFontHeight();
		}

		drawChar(glyph);
	}
}
