#ifndef __POLYFONTS__
#define __POLYFONTS__

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

	int     setFont(const pffont *f);
	pffont *loadFont(const char *fileName);
	void    unloadFont();

	const pffont *getCurrentFont();

	char   *getFontName();
	int     getStringBox(const char *c, float *minx, float *miny, float *maxx, float *maxy);
	int     getStringBoxW(const wchar_t *c, float *minx, float *miny, float *maxx, float *maxy);
	int     getFontBBox(float *minx, float *miny, float *maxx, float *maxy);
	float   getFontHeight();
	float   getFontWidth();
	float   getFontAscent();
	float   getFontDescent();

	int     getFontNumGlyphs();
	wchar_t getChar(int glyph);

	void    getScale(float *s);
	void    getScaleXY(float *sx, float *sy);
	int     getScaleBox(char *c, float *w, float *h);
	int     getScaleBoxW(wchar_t *c, float *w, float *h);

	pfpoint getPosition() const;

	void    setScale(float s);
	void    setScaleXY(float sx, float sy);
	void    setPosition(float x, float y);
	void    setPosition(float *x, float *y);
	void    setSkew(float s);
	void    setWeight(int w);
	void    setAngleR(float a);
	void    setAngleD(float a);
	void    setCenter(bool onOff);

	int     setScaleBox(char *c, float w, float h);
	int     setScaleBoxW(wchar_t *c, float w, float h);

	int     getCharBBox(wchar_t c, float *minx, float *miny, float *maxx, float *maxy);
	float   getCharAdvance(wchar_t c);
	float   getCharHeight(wchar_t c);
	float   getCharWidth(wchar_t c);
	float   getCharAscent(wchar_t c);
	float   getCharDescent(wchar_t c);

	float   getStringWidth(const char *c);
	float   getStringWidth(const wchar_t *c);
	float   getStringWidth(float s, const char *c);
	float   getStringWidth(float s, const wchar_t *c);

	int     drawWideChar(wchar_t c);
	int     drawChar(char c);

	int     drawString(const wchar_t *c);
	int     drawString(const char *c);

	int     drawString(float x, float y, const char *c);
	int     drawString(float x, float y, float sx, float sy, const char *c);

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

	float   skew(float x, float y) const;

	int     _getCharBBox(wchar_t c, float *minx, float *miny, float *maxx, float *maxy);
	float   _getCharAdvance(wchar_t c);
};

const pffont *getDefaultFont(); // get current font:

#endif // __POLYFONTS__
