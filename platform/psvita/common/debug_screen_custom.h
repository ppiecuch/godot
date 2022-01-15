/*************************************************************************/
/*  debug_screen_custom.h                                                */
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

#ifndef DEBUG_SCREEN_CUSTOM_H
#define DEBUG_SCREEN_CUSTOM_H

// backward compatibility for sources based on older Vita SDK versions
#define psvDebugScreenSetFgColor(rgb) psvDebugScreenPrintf("\e[38;2;%lu;%lu;%lum", ((uint32_t)(rgb) >> 16) & 0xFF, ((uint32_t)(rgb) >> 8) & 0xFF, (uint32_t)(rgb)&0xFF)
#define psvDebugScreenSetBgColor(rgb) psvDebugScreenPrintf("\e[48;2;%lu;%lu;%lum", ((uint32_t)(rgb) >> 16) & 0xFF, ((uint32_t)(rgb) >> 8) & 0xFF, (uint32_t)(rgb)&0xFF)
#define psvDebugScreenClear(rgb)   \
	psvDebugScreenSetBgColor(rgb); \
	psvDebugScreenPuts("\e[H\e[2J")

// custom changes for non-Vita builds
#ifndef __vita__
#define psvDebugScreenInitReplacement(...) setvbuf(stdout, NULL, _IONBF, 0)
#endif

#endif // DEBUG_SCREEN_CUSTOM_H
