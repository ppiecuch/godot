/**************************************************************************/
/*  particles_l.c                                                         */
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

#ifndef HAVE_PARTICLES_SIZE_L

const unsigned char *particles_size_l[] = { 0 };

#else

#undef INCBIN_PREFIX
#define INCBIN_PREFIX
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_SILENCE_BITCODE_WARNING

#include "misc/incbin.h"

INCBIN(w1_l, "submodules/environment/waterfall/particles/L/w1.png");
INCBIN(w2_l, "submodules/environment/waterfall/particles/L/w2.png");
INCBIN(w3_l, "submodules/environment/waterfall/particles/L/w3.png");
INCBIN(w4_l, "submodules/environment/waterfall/particles/L/w4.png");
INCBIN(c1_l, "submodules/environment/waterfall/particles/L/c1.png");
INCBIN(c2_l, "submodules/environment/waterfall/particles/L/c2.png");

const unsigned char *particles_size_l[] = {
	w1_l_data,
	w2_l_data,
	w3_l_data,
	w4_l_data,
	c1_l_data,
	c1_l_data,
};

#endif // HAVE_PARTICLES_SIZE_L
