/*************************************************************************/
/*  hershey_render.h                                                     */
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

// --------------------------------------------------------------------
//
//             DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
//                     Version 2, December 2004
//
//  Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>
//
//  Everyone is permitted to copy and distribute verbatim or modified
//  copies of this license document, and changing it is allowed as long
//  as the name is changed.
//
//             DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
//    TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
//
//   0. You just DO WHAT THE FUCK YOU WANT TO.
//
// --------------------------------------------------------------------

#ifndef HERSHEY_RENDER_H
#define HERSHEY_RENDER_H

#include <vector>

#include "core/variant.h"
#include "core/error_macros.h"
#include "core/math/math_funcs.h"
#include "core/math/vector2.h"

#define as_real_t(v) static_cast<real_t>(v)

// Structure use in letters animation.
struct hershey_string_t {
	real_t dx, dy; // segment unit extending
	Vector2 begin, end; // animated line end coords
	hershey_string_t(int16_t x1, int16_t y1, int16_t x2, int16_t y2) :
			begin({ as_real_t(x1), as_real_t(y1) }), end({ as_real_t(x2), as_real_t(y2) }) {
		const real_t angle = Math::atan2(real_t(y2 - y1), real_t(x2 - x1));
		dx = Math::cos(angle);
		dy = Math::sin(angle);
	}
};

inline static int hershey_string_width(const char *s, const char *font_data[], const int font_data_size[], const char font_width[]) {
	ERR_FAIL_NULL_V(s, 0);

	int16_t w = 0;
	while (*s) {
		w += font_width[*s++ - ' '];
	}
	return w;
}

/// Making geometry
/// ---------------

// make vertices array and returns the number of vertices

inline static int hershey_make(real_t x, real_t y, const char *s, const char *font_data[], const int font_data_size[], const char font_width[], PoolVector2Array &out_data, int *out_width = nullptr) {
	int w = 0;
	while (*s) {
		int index = *s++ - ' ', size = font_data_size[index];
		DEV_ASSERT(size%2 == 0);
		for (int vert = 0; vert < size; vert += 2) {
			out_data.push_back({x + font_data[index][vert] + w, y - font_data[index][vert+1]});
		}
		w += font_width[index];
	}
	if (out_width) {
		*out_width = w;
	}
	return out_data.size();
}

inline static int hershey_make(real_t x, real_t y, real_t scale, const char *s, const char *font_data[], const int font_data_size[], const char font_width[], PoolVector2Array &out_data, int *out_width = nullptr) {
	int w = 0;
	while (*s) {
		int index = *s++ - ' ', size = font_data_size[index];
		DEV_ASSERT(size%2 == 0);
		for (int vert = 0; vert < size; vert += 2) {
			out_data.push_back({x + font_data[index][vert] + w, y - font_data[index][vert+1]});
		}
		w += font_width[index];
	}
	if (out_width) {
		*out_width = w;
	}
	return out_data.size();
}

inline static int hershey_make(const char *s, const char *font_data[], const int font_data_size[], const char font_width[], PoolVector2Array &out_data, int *out_width = nullptr) {
	int w = 0;
	while (*s) {
		int index = *s++ - ' ', size = font_data_size[index];
		DEV_ASSERT(size%2 == 0);
		for (int vert = 0; vert < size; vert += 2) {
			out_data.push_back({as_real_t(font_data[index][vert]), as_real_t(font_data[index][vert+1])});
		}
		w += font_width[index];
	}
	if (out_width) {
		*out_width = w;
	}
	return out_data.size();
}

// make segments array and returns the number of segments (two vertices for segment)

inline static int hershey_make(const char *s, const char *font_data[], const int font_data_size[], const char font_width[], std::vector<hershey_string_t> &out_data, int *out_width = nullptr) {
	out_data.reserve(256);
	int w = 0;
	while (*s) {
		int index = *s++ - ' ', size = font_data_size[index];
		while ((size -= 4) >= 0)
			out_data.push_back(hershey_string_t(
					font_data[index][size] + w, // offset x coord
					-font_data[index][size + 1],
					font_data[index][size + 2] + w, // offset x coord
					-font_data[index][size + 3]));
		w += font_width[index];
	}
	if (out_width) {
		*out_width = w;
	}
	return out_data.size();
}

#endif /* HERSHEY_RENDER_H */
