/**************************************************************************/
/*  polygodot.h                                                           */
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

#ifndef POLYGODOT_H
#define POLYGODOT_H

#include "core/color.h"

class PolyFontGodot;

enum PolyFontFace {
	PFOSANS16,
	PFOSANS7,
	PFOSANS8,
	PFOSANSBOLD16,
	PFOSANSBOLD7,
	PFOSANSBOLD8,
	PFOSANSBOLDITALIC16,
	PFOSANSBOLDITALIC7,
	PFOSANSBOLDITALIC8,
	PFOSANSITALIC16,
	PFOSANSITALIC7,
	PFOSANSITALIC8,
	PFOSANSMONO16,
	PFOSANSMONO7,
	PFOSANSMONO8,
	PFOSANSMONOBOLD16,
	PFOSANSMONOBOLD7,
	PFOSANSMONOBOLD8,
	PFOSANSMONOBOLDITALIC16,
	PFOSANSMONOBOLDITALIC7,
	PFOSANSMONOBOLDITALIC8,
	PFOSANSMONOITALIC16,
	PFOSANSMONOITALIC7,
	PFOSANSMONOITALIC8,
	PFOSERIF16,
	PFOSERIF7,
	PFOSERIF8,
	PFOSERIFBOLD16,
	PFOSERIFBOLD7,
	PFOSERIFBOLD8,
	PFPSANS16,
	PFPSANS7,
	PFPSANS8,
	PFPSANSBOLD16,
	PFPSANSBOLD7,
	PFPSANSBOLD8,
	PFPSANSBOLDITALIC16,
	PFPSANSBOLDITALIC7,
	PFPSANSBOLDITALIC8,
	PFPSANSITALIC16,
	PFPSANSITALIC7,
	PFPSANSITALIC8,
	PFPSANSMONO16,
	PFPSANSMONO7,
	PFPSANSMONO8,
	PFPSANSMONOBOLD16,
	PFPSANSMONOBOLD7,
	PFPSANSMONOBOLD8,
	PFPSANSMONOBOLDITALIC16,
	PFPSANSMONOBOLDITALIC7,
	PFPSANSMONOBOLDITALIC8,
	PFPSANSMONOITALIC16,
	PFPSANSMONOITALIC7,
	PFPSANSMONOITALIC8,
	PFPSERIF16,
	PFPSERIF7,
	PFPSERIF8,
	PFPSERIFBOLD16,
	PFPSERIFBOLD7,
	PFPSERIFBOLD8,
};

class PolyGodot {
	float width;
	Color color;

	PolyFontGodot *poly_font;

public:
	void setFont(PolyFontFace p_font);

	PolyGodot();
	~PolyGodot();
};

#endif // POLYGODOT_H
