/**************************************************************************/
/*  sproutline.h                                                          */
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

/* sproutline - v0.10 - public domain sprite outline detector - http://github.org/ands/sproutline
						no warranty implied; use at your own risk

   You can #define S2O_MALLOC to avoid using malloc


   QUICK NOTES:
	  Primarily of interest to game developers.
	  - Recommended to be used with stb_image.
	  - Detects outlines in sprite images with alpha channels.
	  - Extracts outlines as clockwise paths.
	  - Simplifies outlines based on a distance metric.

   Full documentation under "DOCUMENTATION" below.


   Revision 0.10 release notes:

	  - Initial release of sproutline.h.

	  - Added S2O_MALLOC macro for replacing the memory allocator.
		Unlike most STB libraries, this macro doesn't support a context parameter,
		so if you need to pass a context in to the allocator, you'll have to
		store it in a global or a thread-local variable.


   Revision history:
	  0.10  (2015-10-22) initial version

 ============================    Contributors    =========================

 Andreas Mantler (ands)

License:
   This software is in the public domain. Where that dedication is not
   recognized, you are granted a perpetual, irrevocable license to copy
   and modify this file however you want.

*/

#ifndef S2O_INCLUDE_SPROUTLINE_H
#define S2O_INCLUDE_SPROUTLINE_H

// DOCUMENTATION
//
// Limitations:
//    - currently only works with images that have alpha channels
//
// Basic usage (with stb_image):
//    int w, h, n, l;
//    unsigned char *rgba = stbi_load(filename, &w, &h, &n, 4);
//    unsigned char *alpha = s2o_rgba_to_alpha(rgba, w, h);
//    unsigned char *thresholded = s2o_alpha_to_thresholded(alpha, w, h, ALPHA_THRESHOLD);
//    unsigned char *outlined = s2o_thresholded_to_outlined(thresholded, w, h);
//    s2o_point *outline = s2o_extract_outline_path(outlined, w, h, &l, 0);
//    while(l)
//    {
//        s2o_distance_based_path_simplification(outline, &l, DISTANCE_THRESHOLD);
//        // ... process outline here ...
//        // ... l = number of points in outline
//        // ... ALPHA_THRESHOLD = 1..255 (the min value to be considered solid)
//        // ... DISTANCE_THRESHOLD = 0.0f..Inf (~0.5f is a suitable value)
//        // ... a greater value results in fewer points in the output
//
//        outline = s2o_extract_outline_path(outlined, w, h, &l, outline);
//    };
//    free(outline);
//    free(outlined);
//    free(thresholded);
//    free(alpha);
//    free(rgba);
//
// s2o_rgba_to_alpha:
//    Expects an 'unsigned char *' to memory of w * h 4-byte pixels in 'RGBA' order.
//    The return value is an 'unsigned char *' to memory of w * h 1-byte pixel alpha components.
//
// s2o_alpha_to_thresholded:
//    Expects an 'unsigned char *' to memory of w * h 1-byte pixel alpha components.
//    The return value is an 'unsigned char *' to memory of w * h 1-byte values
//    that are 255 if the corresponding input is >= the specified threshold, otherwise 0.
//
// s2o_thresholded_to_outlined:
//    Expects an 'unsigned char *' to memory of w * h 1-byte pixels indicating their solidity {0, nonzero}.
//    The return value is an 'unsigned char *' to memory of w * h 1-byte pixels that indicate if the
//    corresponding input value is part of an outline (= is solid and has a non-solid neighbour).
//
// s2o_extract_outline_path:
//    Expects an 'unsigned char *' to memory of w * h 1-byte pixels indicating their outline membership.
//    The return value is an 's2o_point *' to memory of l s2o_point values consisting of a short x and y value.
//    The procedure scans the input data from top to bottom and starts extracting the first outline it finds.
//    The pixels corresponding to the extracted outline are set to 0 in the input, so that a subsequent call to
//    s2o_extract_outline_path extracts a different outline.
//    The length is set to 0 if no outline was found.
//
// s2o_distance_based_path_simplification:
//    Expects an 's2o_point *' to memory of l outline points.
//    The procedure throws out points in place that lie on or close to linear sections of the outline.
//    The distanceThreshold parameter specifies the min distance value for points to remain in the outline.
//
// ===========================================================================
//
// Philosophy
//
// This library is designed with the stb philosophy in mind.
// stb libraries are designed with the following priorities:
//
//    1. easy to use
//    2. easy to maintain
//    3. good performance
//
// Some secondary priorities arise directly from the first two, some of which
// make more explicit reasons why performance can't be emphasized.
//
//    - Portable ("ease of use")
//    - Small footprint ("easy to maintain")
//    - No dependencies ("ease of use")
//

typedef unsigned char s2o_uc;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef S2O_STATIC
#define S2ODEF static
#else
#define S2ODEF extern
#endif

//////////////////////////////////////////////////////////////////////////////
//
// PRIMARY API
//

S2ODEF s2o_uc *s2o_rgba_to_alpha(const s2o_uc *data, int w, int h);
S2ODEF s2o_uc *s2o_alpha_to_thresholded(const s2o_uc *data, int w, int h, s2o_uc threshold);
S2ODEF s2o_uc *s2o_thresholded_to_outlined(const s2o_uc *data, int w, int h);

typedef struct {
	short x, y;
} s2o_point;
S2ODEF s2o_point *s2o_extract_outline_path(s2o_uc *data, int w, int h, int *point_count, s2o_point *reusable_outline);
S2ODEF void s2o_distance_based_path_simplification(s2o_point *outline, int *outline_length, float distance_threshold);

#ifdef __cplusplus
}
#endif

#endif // S2O_INCLUDE_SPROUTLINE_H
