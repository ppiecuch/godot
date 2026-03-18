/**************************************************************************/
/*  lowpoly_font.h                                                        */
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

// LowPolySymbols 3D font renderer.
// Converts strings to triangle mesh vertex data (3D).

#ifndef LOWPOLY_FONT_H
#define LOWPOLY_FONT_H

#include "core/math/vector2.h"
#include "scene/resources/mesh.h"
#include "servers/visual_server.h"

#include "lowpoly_vdata.h"

// Font metrics (approximate, based on the normalized glyph data).
#define LP_FONT_HEIGHT 70.0f
#define LP_FONT_SPACING 5.0f
#define LP_SPACE_WIDTH 20.0f

static void lowpoly_font_draw_string(Ref<ArrayMesh> &p_mesh, const char *p_str,
		const Point3 &p_pos = Point3(), real_t p_scale = 1.0f) {
	PoolVector3Array verts;
	PoolColorArray colors;

	real_t x = p_pos.x;
	const real_t y = p_pos.y;
	const real_t z_base = p_pos.z;

	while (*p_str) {
		int c = (unsigned char)*p_str++;

		if (c == ' ') {
			x += LP_SPACE_WIDTH * p_scale;
			continue;
		}
		if (c == '\n') {
			// Newline not supported in single-line mode
			continue;
		}
		if (c >= LP_NUMGLYPHS || lp_sizes[c] == 0) {
			continue;
		}

		const int sz = lp_sizes[c];
		const int offset = lp_vdataoffsets[c];
		const float width = lp_widths[c];

		for (int i = 0; i < sz; i++) {
			const float vx = lp_vdata[offset + i][0] * p_scale + x;
			const float vy = lp_vdata[offset + i][1] * p_scale + y;
			const float vz = lp_vdata[offset + i][2] * p_scale + z_base;
			verts.push_back(Vector3(vx, vz, vy)); // swap Y/Z for Godot convention
		}

		x += (width + LP_FONT_SPACING) * p_scale;
	}

	if (verts.size() == 0) {
		return;
	}

	Array mesh_array;
	mesh_array.resize(VS::ARRAY_MAX);
	mesh_array[VS::ARRAY_VERTEX] = verts;
	p_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, mesh_array);
}

static Size2 lowpoly_font_text_size(const char *p_str, real_t p_scale = 1.0f) {
	real_t x = 0;
	real_t max_x = 0;

	while (*p_str) {
		int c = (unsigned char)*p_str++;

		if (c == ' ') {
			x += LP_SPACE_WIDTH * p_scale;
			continue;
		}
		if (c >= LP_NUMGLYPHS || lp_sizes[c] == 0) {
			continue;
		}

		x += (lp_widths[c] + LP_FONT_SPACING) * p_scale;
		if (x > max_x) {
			max_x = x;
		}
	}

	return Size2(max_x, LP_FONT_HEIGHT * p_scale);
}

static real_t lowpoly_font_string_width(const char *p_str, real_t p_scale = 1.0f) {
	return lowpoly_font_text_size(p_str, p_scale).x;
}

#endif // LOWPOLY_FONT_H
