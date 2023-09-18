/**************************************************************************/
/*  utils.h                                                               */
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

#ifndef UTILS_H
#define UTILS_H

#include "core/typedefs.h"

#include <utility>

#define decnz(x)   \
	if ((x) > 0) { \
		(x)--;     \
	}
#define inctop(x, y) \
	if ((x) < (y)) { \
		(x)++;       \
	}
#define iff(cond, a, b) ((cond) ? (a) : (b))
#define sqr(x) ((x) * (x))
#define cube(x) ((x) * (x) * (x))

_FORCE_INLINE_ void swapint(int &a, int &b) { std::swap(a, b); }
_FORCE_INLINE_ void swapuint(unsigned int &a, unsigned int &b) { std::swap(a, b); }
_FORCE_INLINE_ void swapshort(short &a, short &b) { std::swap(a, b); }
_FORCE_INLINE_ void swapushort(unsigned short &a, unsigned short &b) { std::swap(a, b); }
_FORCE_INLINE_ void swapbyte(char &a, char &b) { std::swap(a, b); }
_FORCE_INLINE_ void swapubyte(unsigned char &a, unsigned char &b) { std::swap(a, b); }
_FORCE_INLINE_ void swapmem(void *a, void *b, int c) {
	for (int t = 0; t < c; t++) {
		swapbyte(((char *)a)[t], ((char *)b)[t]);
	}
}

#endif // UTILS_H
