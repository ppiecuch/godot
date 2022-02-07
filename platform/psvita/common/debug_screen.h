/*************************************************************************/
/*  debug_screen.h                                                       */
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

#ifndef DEBUG_SCREEN_H
#define DEBUG_SCREEN_H

#include "debug_screen_custom.h"

typedef struct ColorState {
	int fgTrueColorFlag; // flag if truecolors or ANSI/VTERM/GREYSCALE colors are used
	int bgTrueColorFlag; // flag if truecolors or ANSI/VTERM/GREYSCALE colors are used
	// truecolors
	uint32_t fgTrueColor; // color in RGB (internal BGR)
	uint32_t bgTrueColor; // color in RGB (internal BGR)
	// ANSI/VTERM/GREYSCALE colors
	unsigned char fgIndex; // ANSI/VTERM/GREYSCALE color code (0-255)
	unsigned char fgIntensity; // 22=normal, 1=increased ("bright"), 2=decreased ("dark")
	unsigned char bgIndex; // ANSI/VTERM/GREYSCALE color code (0-255)
	unsigned char bgIntensity; // 22=normal, 1=increased ("bright")
	int inversion; // flag if bg/fg colors are inverted

	// default colors (ANSI/VTERM/GREYSCALE)
	unsigned char fgIndexDefault; // default ANSI/VTERM/GREYSCALE color code
	unsigned char fgIntensityDefault; // 22=normal, 1=increased, 2=decreased
	unsigned char bgIndexDefault; // default ANSI/VTERM/GREYSCALE color code
	unsigned char bgIntensityDefault; // 22=normal, 1=increased
	int inversionDefault; // flag if bg/fg colors are inverted

	// current colors (e.g. inverted)
	uint32_t color_fg; // color in RGB (internal BGR)
	uint32_t color_bg; // color in RGB (internal BGR)
} ColorState;

typedef struct PsvDebugScreenFont {
	unsigned char *glyphs, width, height, first, last, size_w, size_h; // only values 0-255
} PsvDebugScreenFont;

#define SCREEN_WIDTH (960) // screen resolution x
#define SCREEN_HEIGHT (544) // screen resolution y

#ifdef DEBUG_SCREEN_CODE_INCLUDE // not recommended for your own projects, but for sake of backward compatibility
#include "debugScreen.c"
#else
#ifdef __cplusplus
extern "C" {
#endif
int psvDebugScreenInit();
int psvDebugScreenSet();
int psvDebugScreenPuts(const char *_text);
int psvDebugScreenPrintf(const char *format, ...);
void psvDebugScreenGetColorStateCopy(ColorState *copy);
void psvDebugScreenGetCoordsXY(int *x, int *y);
void psvDebugScreenSetCoordsXY(int *x, int *y);
PsvDebugScreenFont *psvDebugScreenGetFont(void);
PsvDebugScreenFont *psvDebugScreenSetFont(PsvDebugScreenFont *font);
PsvDebugScreenFont *psvDebugScreenScaleFont2x(PsvDebugScreenFont *source_font);
#ifdef __cplusplus
}
#endif
#endif

#endif // DEBUG_SCREEN_H
