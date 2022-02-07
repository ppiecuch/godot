/*************************************************************************/
/*  sr_graph.h                                                           */
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

/* sr_graph - v1.2 - public domain plot tracer; no warranties implied, use at your own risk.

 Usage:
	 All colors are defined by their red, green, blue components
	 taking values in [0.0,1.0].
	 Start by creating a graph using:

		 setup(min x, max x, min y, max y, ratio, margins, red, green, blue);

				+----+(1,1)
				|    |
				|    |
		 (-1,-1)+----+

	 You specify the bounds of the graph (minimal and maximal values on both
	 axes), the screen ratio, the margins for all sides (between 0 - no margin - and 1 - no
	 graph -; left+right and bottom+top margins need to be < 2), and the  background color.
	 This will return an integer ID that you will use as a reference to the graph for later calls.

	 The graph can be customized by adding axes with arrows (automatically
	 oriented  toward the increasing values on both axes). You can specify their
	 width (in [0,1]) and set their color. By default the axes will pass through
	 the (0,0) point or stick to the sides of the graph the closest to it. You
	 can also force them to stick on the side by setting 'side axis' to true.

		 add_axes(graphID, width, red, green, blue, side axis);

	 A regular grid can also be added. Specify the spacing between grid lines on
	 both axis, the width (in [0,1]) and color. The grid can either start from
	 the left and bottom side of the graph, or be centered on the (0,0) point by
	 setting 'zero align' to true.

		 add_grid(graphID, step x, step y, width, red, green, blue, zero align);

	 You can plot continuous curves by specifying lists of X and Y coordinates,
	 along with the width ([0,1]) and color of the lines. Points clouds can be
	 added in a similar manner, by specifying X and Y coordinates, a point size
	 and color. Finally you can also add histograms, by specifying a set of
	 values and a number of bins ; spacing between bars and their color should
	 also be specified.

		 add_curve(graphID, x values, y values, line width, red, green, blue);
		 add_points(graphID, x values, y values, point size, red, green, blue);
		 add_hist(graphID, number of bins, values, spacing, red, green, blue);

	 These three functions return integers IDs that can then be used to update a
	 given plot on a given graph at a later time by calling respectively:

		 update_points(graphID, points cloud id, new x values, new y values);
		 update_curve(graphID, curve id, new x values, new y values);
		 update_hist(graphID, histogram id, new values);

	 All settings (width, size, bins, colors) are preserved. The IDs are managed
	 separately for each graph and each type of plot.

	 Call:

		 draw(graphID, screen ratio);

	 to draw a graph in the current viewport. If a screen ratio is specified,
	 the graph will be rescaled to avoid any deformation ; else, nothing is done
	 to prevent deformations when resizing the window.

 Revision history
	 1.0 First public version! Support for curves, bar charts, points, axes with
		 arrows, grid, real-time updates.
	 1.1 Raylib conversion; c-compatible code
	 1.2 Godot conversion; switch to PoolArrays, mesh drawing

 License:
	 See end of file.

 Reference:
	 https://github.com/armadillu/ofxHistoryPlot/blob/master/src/ofxHistoryPlot.cpp
	 https://www.desultoryquest.com/blog/real-time-plotting-on-windows-using-opengl/

*/

#ifndef SR_GRAPH_H
#define SR_GRAPH_H

#include "core/math/math_defs.h"
#include "core/variant.h"

int rg_setup(real_t minx, real_t maxx, real_t miny, real_t maxy, real_t ratio, const real_t margins[4], real_t bg_r, real_t bg_g, real_t bg_b);
unsigned *rg_palette(const char *name, int num_colors);
void rg_add_axes(int graph_id, real_t width, real_t axis_r, real_t axis_g, real_t axis_b, bool axis_on_side);
void rg_add_grid(int graph_id, real_t stepx, real_t stepy, real_t width, real_t lines_r, real_t lines_g, real_t lines_b, bool fromZero);
int rg_add_curve(int graph_id, const PoolRealArray &xs, const PoolRealArray &ys, real_t width, real_t color_r, real_t color_g, real_t color_b);
void rg_update_curve(int graph_id, int curve_id, const PoolRealArray &xs, const PoolRealArray &ys);
int rg_add_hist(int graph_id, unsigned bins, const PoolRealArray &ys, real_t spacing, real_t color_r, real_t color_g, real_t color_b);
void rg_update_hist(int graph_id, int hist_id, const PoolRealArray &ys);
int rg_add_points(int graph_id, const PoolRealArray &xs, const PoolRealArray &ys, real_t size, real_t color_r, real_t color_g, real_t color_b);
void rg_update_points(int graph_id, int points_id, const PoolRealArray &xs, const PoolRealArray &ys);
int rg_add_stack(int graph_id, real_t weight, const PoolRealArray &vs, char *pal);
void rg_update_stack(int graph_id, int stack_id, const PoolRealArray &vs);
void rg_draw(int graph_id, real_t ratio);

#endif // SR_GRAPH_H

/*
 ------------------------------------------------------------------------------
 This software is available under 2 licenses -- choose whichever you prefer.
 ------------------------------------------------------------------------------
 ALTERNATIVE A - MIT License
 Copyright (c) 2017 Simon Rodriguez
 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 ------------------------------------------------------------------------------
 ALTERNATIVE B - Public Domain (www.unlicense.org)
 This is free and unencumbered software released into the public domain.
 Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
 software, either in source code form or as a compiled binary, for any purpose,
 commercial or non-commercial, and by any means.
 In jurisdictions that recognize copyright laws, the author or authors of this
 software dedicate any and all copyright interest in the software to the public
 domain. We make this dedication for the benefit of the public at large and to
 the detriment of our heirs and successors. We intend this dedication to be an
 overt act of relinquishment in perpetuity of all present and future rights to
 this software under copyright law.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ------------------------------------------------------------------------------
*/
