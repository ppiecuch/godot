/**************************************************************************/
/*  fb_font_symbol.cpp                                                    */
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

#include "fb_font_symbol.h"

#include "core/variant.h"

Vector<FBFontSymbolType> symbols_for_string(const String &str) {
	Vector<FBFontSymbolType> symbols;
	for (int i = 0; i < str.length(); i++) {
		int c = str.ord_at(i);
		if (c == '0') {
			symbols.push_back(FBFontSymbol0);
		} else if (c == '1') {
			symbols.push_back(FBFontSymbol1);
		} else if (c == '2') {
			symbols.push_back(FBFontSymbol2);
		} else if (c == '3') {
			symbols.push_back(FBFontSymbol3);
		} else if (c == '4') {
			symbols.push_back(FBFontSymbol4);
		} else if (c == '5') {
			symbols.push_back(FBFontSymbol5);
		} else if (c == '6') {
			symbols.push_back(FBFontSymbol6);
		} else if (c == '7') {
			symbols.push_back(FBFontSymbol7);
		} else if (c == '8') {
			symbols.push_back(FBFontSymbol8);
		} else if (c == '9') {
			symbols.push_back(FBFontSymbol9);
		} else if (c == 'A') {
			symbols.push_back(FBFontSymbolA);
		} else if (c == 'B') {
			symbols.push_back(FBFontSymbolB);
		} else if (c == 'C') {
			symbols.push_back(FBFontSymbolC);
		} else if (c == 'D') {
			symbols.push_back(FBFontSymbolD);
		} else if (c == 'E') {
			symbols.push_back(FBFontSymbolE);
		} else if (c == 'F') {
			symbols.push_back(FBFontSymbolF);
		} else if (c == 'G') {
			symbols.push_back(FBFontSymbolG);
		} else if (c == 'H') {
			symbols.push_back(FBFontSymbolH);
		} else if (c == 'I') {
			symbols.push_back(FBFontSymbolI);
		} else if (c == 'J') {
			symbols.push_back(FBFontSymbolJ);
		} else if (c == 'K') {
			symbols.push_back(FBFontSymbolK);
		} else if (c == 'L') {
			symbols.push_back(FBFontSymbolL);
		} else if (c == 'M') {
			symbols.push_back(FBFontSymbolM);
		} else if (c == 'N') {
			symbols.push_back(FBFontSymbolN);
		} else if (c == 'O') {
			symbols.push_back(FBFontSymbolO);
		} else if (c == 'P') {
			symbols.push_back(FBFontSymbolP);
		} else if (c == 'Q') {
			symbols.push_back(FBFontSymbolQ);
		} else if (c == 'R') {
			symbols.push_back(FBFontSymbolR);
		} else if (c == 'S') {
			symbols.push_back(FBFontSymbolS);
		} else if (c == 'T') {
			symbols.push_back(FBFontSymbolT);
		} else if (c == 'U') {
			symbols.push_back(FBFontSymbolU);
		} else if (c == 'V') {
			symbols.push_back(FBFontSymbolV);
		} else if (c == 'W') {
			symbols.push_back(FBFontSymbolW);
		} else if (c == 'X') {
			symbols.push_back(FBFontSymbolX);
		} else if (c == 'Y') {
			symbols.push_back(FBFontSymbolY);
		} else if (c == 'Z') {
			symbols.push_back(FBFontSymbolZ);
		} else if (c == ' ') {
			symbols.push_back(FBFontSymbolSpace);
		} else if (c == '!') {
			symbols.push_back(FBFontSymbolExclamationMark);
		} else if (c == ':') {
			symbols.push_back(FBFontSymbolColon);
		} else {
			symbols.push_back(FBFontSymbolSpace);
		}
	}
	return symbols;
}
