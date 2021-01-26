/*************************************************************************/
/*  starfield_res.h                                                      */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef _starfield_res_defined_
#define _starfield_res_defined_

#if defined(__cplusplus)
extern "C" {
#endif

extern const unsigned char
		starfield_1[],
		starfield_2[],
		starfield_3[],
		starfield_4[],
		starfield_5[],
		starfield_6[],
		starfield_7[],
		starfield_8[],
		starfield_9[],
		starfield_10[];

typedef struct {
	const char *image;
	const unsigned char *pixels;
	int size, width, height, channels;
} EmbedImageItem;

const static EmbedImageItem embed_starfield_res[] = {
	{ "star01.png", starfield_1, 3072, 32, 32, 3 },
	{ "star02.png", starfield_2, 768, 16, 16, 3 },
	{ "star03.png", starfield_3, 192, 8, 8, 3 },
	{ "star04.png", starfield_4, 768, 16, 16, 3 },
	{ "star05.png", starfield_5, 192, 8, 8, 3 },
	{ "star06.png", starfield_6, 768, 16, 16, 3 },
	{ "star07.png", starfield_7, 768, 16, 16, 3 },
	{ "frame0.png", starfield_8, 12288, 64, 64, 3 },
	{ "frame1.png", starfield_9, 12288, 64, 64, 3 },
	{ "frame2.png", starfield_10, 12288, 64, 64, 3 },
	{ NULL, NULL, 0, 0, 0, 0 }
};
const static int embed_starfield_res_count = 10;

#if defined(__cplusplus)
}
#endif
#endif // of starfield_res
