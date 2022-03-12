/*************************************************************************/
/*  polyfonts.h                                                          */
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

#ifndef __POLYFONTS__
#define __POLYFONTS__

/*
 Polyfonts is a polygon font drawing library for use with SDL. Any
 TTF font can be converted for use with this library. Contact the
 author for details.

 Copyright (C) 2003 Bob Pendleton

 Bob Pendleton
 Bob@Pendleton.com
 */

#include "polyfonttypes.h"
#include <vector>

typedef std::vector<pfpoint> coord_vector;

const float DefaultScale = 20.0;

class PolyFont {
	float pfScaleX;
	float pfScaleY;

	float pfTextX;
	float pfTextY;

	float pfTextSkew;
	int pfTextWeight;

	float pfTextSin;
	float pfTextCos;

	bool pfCenter;

	pffont *currentFont;

public:
	PolyFont(const pffont *f = nullptr);
	~PolyFont();

	int setFont(const pffont *f);
	pffont *loadFont(const char *fileName);
	void unloadFont();

	const pffont *getCurrentFont();

	char *getFontName();
	int getStringBox(const char *c, float *minx, float *miny, float *maxx, float *maxy);
	int getStringBoxW(const wchar_t *c, float *minx, float *miny, float *maxx, float *maxy);
	int getFontBBox(float *minx, float *miny, float *maxx, float *maxy);
	float getFontHeight();
	float getFontWidth();
	float getFontAscent();
	float getFontDescent();

	int getFontNumGlyphs();
	wchar_t getChar(int glyph);

	void getScale(float *s);
	void getScaleXY(float *sx, float *sy);
	int getScaleBox(char *c, float *w, float *h);
	int getScaleBoxW(wchar_t *c, float *w, float *h);

	pfpoint getPosition() const;

	void setScale(float s);
	void setScaleXY(float sx, float sy);
	void setPosition(float x, float y);
	void setPosition(float *x, float *y);
	void setSkew(float s);
	void setWeight(int w);
	void setAngleR(float a);
	void setAngleD(float a);
	void setCenter(bool onOff);

	int setScaleBox(char *c, float w, float h);
	int setScaleBoxW(wchar_t *c, float w, float h);

	int getCharBBox(wchar_t c, float *minx, float *miny, float *maxx, float *maxy);
	float getCharAdvance(wchar_t c);
	float getCharHeight(wchar_t c);
	float getCharWidth(wchar_t c);
	float getCharAscent(wchar_t c);
	float getCharDescent(wchar_t c);

	float getStringWidth(const char *c);
	float getStringWidth(const wchar_t *c);
	float getStringWidth(float s, const char *c);
	float getStringWidth(float s, const wchar_t *c);

	int drawWideChar(wchar_t c);
	int drawChar(char c);

	int drawString(const wchar_t *c);
	int drawString(const char *c);

	int drawString(float x, float y, const char *c);
	int drawString(float x, float y, float sx, float sy, const char *c);

	// print test screen:
	void test(int screenWidth = 320);

	// main drawing function and interface:
	virtual void beginStringDraw() const = 0;
	virtual void doneStringDraw() const = 0;
	virtual bool polyDrawElements(int mode, coord_vector &indices) const = 0;
	virtual void setWidth(float width) = 0;
	virtual void setColor(float R, float G, float B, float A) = 0;

	static void validate(const pffont *font);

private:
	pfglyph *findGlyph(pfglyph *glyphs, int numglyphs, pfglyph *find) const;
	pfglyph *getGlyph(wchar_t c) const;

	float skew(float x, float y) const;

	int _getCharBBox(wchar_t c, float *minx, float *miny, float *maxx, float *maxy);
	float _getCharAdvance(wchar_t c);
};

const pffont *getDefaultFont(); // get current font:

#endif // __POLYFONTS__
