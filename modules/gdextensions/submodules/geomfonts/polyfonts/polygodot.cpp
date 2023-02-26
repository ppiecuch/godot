/**************************************************************************/
/*  polygodot.cpp                                                         */
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

#include "polygodot.h"
#include "polyfonts.h"

#include "core/error_macros.h"
#include "core/variant.h"

#include "polyfonts.cpp"
#include "polyfonts_all.h"

void PolyGodot::setWidth(float w) {
	width = w;
};
void PolyGodot::setColor(float R, float G, float B, float A) {
	color = Color(R, G, B, A);
};

void PolyGodot::beginStringDraw() const {}
void PolyGodot::doneStringDraw() const {}

bool PolyGodot::polyDrawElements(int mode, coord_vector &indices) const {
	switch (mode) {
		case POLY_POINTS:
			break;
		case POLY_LINES:
			break;
		case POLY_LINE_STRIP:
			break;
		case POLY_LINE_LOOP:
			break;
		case POLY_TRIANGLES:
			break;
		case POLY_TRIANGLE_STRIP:
			break;
		case POLY_TRIANGLE_FAN:
			break;
		default:
			ERR_PRINT(vformat("polyDrawElements error: INVALID_ENUM - mode %d", mode));
			return false;
	}
	return true;
}

PolyGodot::PolyGodot() :
		PolyFont(getDefaultFont()) {}

PolyGodot::PolyGodot(const pffont *f) :
		PolyFont(f) {}

PolyGodot::~PolyGodot() {}
