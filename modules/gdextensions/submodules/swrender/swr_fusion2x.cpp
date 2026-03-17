/**************************************************************************/
/*  swr_fusion2x.cpp                                                      */
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

#include "swr_fusion2x.h"

#ifdef SWRENDER_FUSION2X

#include "swr-fusion2x/include/fusion2x.h"
#include "swr-fusion2x/include/fusion2x/GLES/gl.h"

#include <string.h>

// Fusion2X uses 16-bit R5G5A1B5 framebuffer with fixed-point math.
// This backend converts between float API and Fusion2X's fixed-point/16-bit.

struct SWRFusion2X::Impl {
	F2X_Context *ctx;
	F2X_RenderSurface surface;
	uint16_t *framebuf16;
	int width;
	int height;
	bool initialized;
	float clear_r, clear_g, clear_b, clear_a;
};

SWRFusion2X::SWRFusion2X() {
	impl = nullptr;
}

SWRFusion2X::~SWRFusion2X() {
	destroy();
}

bool SWRFusion2X::initialize(int width, int height) {
	destroy();

	impl = new Impl();
	memset(impl, 0, sizeof(Impl));
	impl->width = width;
	impl->height = height;

	// Allocate 16-bit framebuffer
	impl->framebuf16 = new uint16_t[width * height];
	memset(impl->framebuf16, 0, width * height * sizeof(uint16_t));

	impl->surface.format = F2X_FORMAT_UINT16_R5_G5_A1_B5;
	impl->surface.width = width;
	impl->surface.height = height;
	impl->surface.pitch = width * 2;
	impl->surface.data = impl->framebuf16;

	F2X_ContextCreateParams params;
	params.size = sizeof(params);
	impl->ctx = F2X_CreateContext(&params);
	if (!impl->ctx) {
		delete[] impl->framebuf16;
		delete impl;
		impl = nullptr;
		return false;
	}

	F2X_MakeCurrent(impl->ctx);
	F2X_SetParam(impl->ctx, F2X_COLOR_BUFFER, &impl->surface);

	impl->initialized = true;
	return true;
}

void SWRFusion2X::destroy() {
	if (impl) {
		if (impl->ctx) {
			F2X_DeleteContext(impl->ctx);
		}
		delete[] impl->framebuf16;
		delete impl;
		impl = nullptr;
	}
}

String SWRFusion2X::get_name() const {
	return "Fusion2X";
}

int SWRFusion2X::get_width() const {
	return impl ? impl->width : 0;
}

int SWRFusion2X::get_height() const {
	return impl ? impl->height : 0;
}

void SWRFusion2X::viewport(int x, int y, int w, int h) {
	if (!impl || !impl->initialized)
		return;
	F2X_MakeCurrent(impl->ctx);
	glViewport(x, y, w, h);
}

void SWRFusion2X::clear_color(float r, float g, float b, float a) {
	if (!impl)
		return;
	impl->clear_r = r;
	impl->clear_g = g;
	impl->clear_b = b;
	impl->clear_a = a;
	if (impl->initialized) {
		F2X_MakeCurrent(impl->ctx);
		// Fusion2X uses fixed-point: float * 65536
		glClearColorx((GLclampx)(r * 65536), (GLclampx)(g * 65536),
				(GLclampx)(b * 65536), (GLclampx)(a * 65536));
	}
}

void SWRFusion2X::clear(uint32_t mask) {
	if (!impl || !impl->initialized)
		return;
	F2X_MakeCurrent(impl->ctx);
	GLbitfield gl_mask = 0;
	if (mask & 1)
		gl_mask |= GL_COLOR_BUFFER_BIT;
	if (mask & 2)
		gl_mask |= GL_DEPTH_BUFFER_BIT;
	glClear(gl_mask);
}

void SWRFusion2X::read_pixels(uint8_t *rgba_dest) {
	if (!impl || !impl->initialized || !impl->framebuf16)
		return;

	// Convert R5G5A1B5 (16-bit) to RGBA8 (32-bit)
	int total = impl->width * impl->height;
	for (int i = 0; i < total; i++) {
		uint16_t px = impl->framebuf16[i];
		// R5G5A1B5: RRRR RGGG GGAB BBBB
		uint8_t r = (uint8_t)(((px >> 11) & 0x1F) * 255 / 31);
		uint8_t g = (uint8_t)(((px >> 6) & 0x1F) * 255 / 31);
		uint8_t b = (uint8_t)((px & 0x1F) * 255 / 31);
		uint8_t a = (px & 0x0020) ? 255 : 0;
		rgba_dest[i * 4 + 0] = r;
		rgba_dest[i * 4 + 1] = g;
		rgba_dest[i * 4 + 2] = b;
		rgba_dest[i * 4 + 3] = a;
	}
}

void SWRFusion2X::set_depth_test(bool enabled) {
	if (!impl || !impl->initialized)
		return;
	F2X_MakeCurrent(impl->ctx);
	if (enabled) {
		glEnable(GL_DEPTH_TEST);
	} else {
		glDisable(GL_DEPTH_TEST);
	}
}

void SWRFusion2X::set_blend(bool enabled) {
	if (!impl || !impl->initialized)
		return;
	F2X_MakeCurrent(impl->ctx);
	if (enabled) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else {
		glDisable(GL_BLEND);
	}
}

void SWRFusion2X::set_cull_face(bool enabled) {
	if (!impl || !impl->initialized)
		return;
	F2X_MakeCurrent(impl->ctx);
	if (enabled) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
}

void SWRFusion2X::draw(PrimitiveType type,
		const float *positions, int vertex_count,
		const float *colors,
		const float *texcoords,
		const float *normals,
		const float *mvp,
		const float *uniform_color) {
	if (!impl || !impl->initialized || vertex_count <= 0)
		return;

	F2X_MakeCurrent(impl->ctx);

	// Fusion2X GLES 1.0 uses fixed-point vertex arrays.
	// Convert float positions to fixed-point and submit via glDrawArrays.
	// For simplicity, use glVertexPointer with GL_FLOAT if supported,
	// otherwise convert to fixed-point.

	// Set modelview matrix from MVP (Fusion2X has separate projection/modelview,
	// but for simplicity we load MVP into modelview and set projection to identity)
	if (mvp) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		// Convert column-major float[16] to fixed-point
		GLfixed mat_fixed[16];
		for (int i = 0; i < 16; i++) {
			mat_fixed[i] = (GLfixed)(mvp[i] * 65536.0f);
		}
		glLoadMatrixx(mat_fixed);
	}

	// Set up vertex pointer
	glEnableClientState(GL_VERTEX_ARRAY);
	// Fusion2X expects fixed-point, convert positions
	int pos_count = vertex_count * 3;
	GLfixed *fixed_pos = new GLfixed[pos_count];
	for (int i = 0; i < pos_count; i++) {
		fixed_pos[i] = (GLfixed)(positions[i] * 65536.0f);
	}
	glVertexPointer(3, GL_FIXED, 0, fixed_pos);

	// Colors
	GLfixed *fixed_colors = nullptr;
	if (colors) {
		glEnableClientState(GL_COLOR_ARRAY);
		int col_count = vertex_count * 4;
		fixed_colors = new GLfixed[col_count];
		for (int i = 0; i < col_count; i++) {
			fixed_colors[i] = (GLfixed)(colors[i] * 65536.0f);
		}
		glColorPointer(4, GL_FIXED, 0, fixed_colors);
	} else if (uniform_color) {
		glDisableClientState(GL_COLOR_ARRAY);
		glColor4x(
				(GLfixed)(uniform_color[0] * 65536.0f),
				(GLfixed)(uniform_color[1] * 65536.0f),
				(GLfixed)(uniform_color[2] * 65536.0f),
				(GLfixed)(uniform_color[3] * 65536.0f));
	}

	// Texcoords
	GLfixed *fixed_texcoords = nullptr;
	if (texcoords) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		int tc_count = vertex_count * 2;
		fixed_texcoords = new GLfixed[tc_count];
		for (int i = 0; i < tc_count; i++) {
			fixed_texcoords[i] = (GLfixed)(texcoords[i] * 65536.0f);
		}
		glTexCoordPointer(2, GL_FIXED, 0, fixed_texcoords);
	}

	// Draw
	GLenum gl_mode;
	switch (type) {
		case PRIM_POINTS:
			gl_mode = GL_POINTS;
			break;
		case PRIM_LINES:
			gl_mode = GL_LINES;
			break;
		case PRIM_LINE_STRIP:
			gl_mode = GL_LINE_STRIP;
			break;
		case PRIM_LINE_LOOP:
			gl_mode = GL_LINE_LOOP;
			break;
		case PRIM_TRIANGLES:
			gl_mode = GL_TRIANGLES;
			break;
		case PRIM_TRIANGLE_STRIP:
			gl_mode = GL_TRIANGLE_STRIP;
			break;
		case PRIM_TRIANGLE_FAN:
			gl_mode = GL_TRIANGLE_FAN;
			break;
		case PRIM_QUADS:
			// Draw as triangle fans, 4 vertices per quad
			for (int q = 0; q < vertex_count / 4; q++) {
				glDrawArrays(GL_TRIANGLE_FAN, q * 4, 4);
			}
			gl_mode = 0; // already drawn
			break;
		default:
			gl_mode = GL_TRIANGLES;
			break;
	}

	if (gl_mode != 0 && type != PRIM_QUADS) {
		glDrawArrays(gl_mode, 0, vertex_count);
	}

	// Cleanup
	glDisableClientState(GL_VERTEX_ARRAY);
	if (colors)
		glDisableClientState(GL_COLOR_ARRAY);
	if (texcoords)
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	delete[] fixed_pos;
	delete[] fixed_colors;
	delete[] fixed_texcoords;
}

uint32_t SWRFusion2X::upload_texture(int w, int h, const uint8_t *rgba_data) {
	if (!impl || !impl->initialized)
		return 0;
	F2X_MakeCurrent(impl->ctx);

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_data);
	return (uint32_t)tex;
}

void SWRFusion2X::bind_texture(uint32_t id) {
	if (!impl || !impl->initialized)
		return;
	F2X_MakeCurrent(impl->ctx);
	glBindTexture(GL_TEXTURE_2D, (GLuint)id);
}

void SWRFusion2X::delete_texture(uint32_t id) {
	if (!impl || !impl->initialized)
		return;
	F2X_MakeCurrent(impl->ctx);
	GLuint tex = (GLuint)id;
	glDeleteTextures(1, &tex);
}

SWRBackend *SWRBackend::create_fusion2x() {
	return new SWRFusion2X();
}

#else // !SWRENDER_FUSION2X

SWRBackend *SWRBackend::create_fusion2x() {
	return nullptr;
}

#endif // SWRENDER_FUSION2X
