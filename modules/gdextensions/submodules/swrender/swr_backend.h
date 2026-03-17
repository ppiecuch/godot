/**************************************************************************/
/*  swr_backend.h                                                         */
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

#ifndef SWR_BACKEND_H
#define SWR_BACKEND_H

#include "core/ustring.h"
#include <stdint.h>

class SWRBackend {
public:
	enum PrimitiveType {
		PRIM_POINTS = 0,
		PRIM_LINES,
		PRIM_LINE_STRIP,
		PRIM_LINE_LOOP,
		PRIM_TRIANGLES,
		PRIM_TRIANGLE_STRIP,
		PRIM_TRIANGLE_FAN,
		PRIM_QUADS,
	};

	enum DrawFlags {
		DRAW_HAS_COLORS = 1,
		DRAW_HAS_TEXCOORDS = 2,
		DRAW_HAS_NORMALS = 4,
	};

	virtual ~SWRBackend() {}

	virtual bool initialize(int width, int height) = 0;
	virtual void destroy() = 0;
	virtual String get_name() const = 0;
	virtual int get_width() const = 0;
	virtual int get_height() const = 0;

	// Framebuffer
	virtual void viewport(int x, int y, int w, int h) = 0;
	virtual void clear_color(float r, float g, float b, float a) = 0;
	virtual void clear(uint32_t mask) = 0;
	virtual void read_pixels(uint8_t *rgba_dest) = 0;

	// State
	virtual void set_depth_test(bool enabled) = 0;
	virtual void set_blend(bool enabled) = 0;
	virtual void set_cull_face(bool enabled) = 0;

	// Draw a batch of vertices with the given MVP matrix.
	// positions: 3 floats per vertex (x,y,z)
	// colors: 4 floats per vertex (r,g,b,a) or nullptr
	// texcoords: 2 floats per vertex (u,v) or nullptr
	// normals: 3 floats per vertex (nx,ny,nz) or nullptr
	// mvp: 16 floats, 4x4 column-major matrix
	// uniform_color: 4 floats (r,g,b,a), used when colors is nullptr
	virtual void draw(PrimitiveType type,
			const float *positions, int vertex_count,
			const float *colors,
			const float *texcoords,
			const float *normals,
			const float *mvp,
			const float *uniform_color) = 0;

	// Textures
	virtual uint32_t upload_texture(int w, int h, const uint8_t *rgba_data) = 0;
	virtual void bind_texture(uint32_t id) = 0;
	virtual void delete_texture(uint32_t id) = 0;

	static SWRBackend *create_portablegl();
	static SWRBackend *create_fusion2x();
};

#endif // SWR_BACKEND_H
