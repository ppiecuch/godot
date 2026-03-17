/**************************************************************************/
/*  swr_portablegl.h                                                      */
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

#ifndef SWR_PORTABLEGL_H
#define SWR_PORTABLEGL_H

#include "swr_backend.h"

// Forward declaration only. The PortableGL header is included
// exclusively in swr_portablegl.cpp to contain symbol scope.

class SWRPortableGL : public SWRBackend {
	struct Impl;
	Impl *impl;

public:
	bool initialize(int width, int height) override;
	void destroy() override;
	String get_name() const override;
	int get_width() const override;
	int get_height() const override;

	void viewport(int x, int y, int w, int h) override;
	void clear_color(float r, float g, float b, float a) override;
	void clear(uint32_t mask) override;
	void read_pixels(uint8_t *rgba_dest) override;

	void set_depth_test(bool enabled) override;
	void set_blend(bool enabled) override;
	void set_cull_face(bool enabled) override;

	void draw(PrimitiveType type,
			const float *positions, int vertex_count,
			const float *colors,
			const float *texcoords,
			const float *normals,
			const float *mvp,
			const float *uniform_color) override;

	uint32_t upload_texture(int w, int h, const uint8_t *rgba_data) override;
	void bind_texture(uint32_t id) override;
	void delete_texture(uint32_t id) override;

	SWRPortableGL();
	~SWRPortableGL();
};

#endif // SWR_PORTABLEGL_H
