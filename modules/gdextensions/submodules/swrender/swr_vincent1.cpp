/**************************************************************************/
/*  swr_vincent1.cpp                                                      */
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

#include "swr_vincent1.h"

#ifdef SWRENDER_VINCENT1

// Use the Vincent C++ API directly (EGL::Context, EGL::Surface) to avoid
// gl* symbol conflicts with other backends. We do NOT compile gl.cpp.
#include "swr-vincent-gles1/src/Config.h"
#include "swr-vincent-gles1/src/Context.h"
#include "swr-vincent-gles1/src/OGLES.h"
#include "swr-vincent-gles1/src/egl/Surface.h"

#include <string.h>

// On non-JIT platforms, FunctionCache::GetFunction is still called by the
// Rasterizer but CodeGenerator isn't compiled. Provide a stub that returns
// nullptr so the software C++ rasterization path is used instead.
#if !EGL_USE_JIT
#include "swr-vincent-gles1/src/armjit/FunctionCache.h"
namespace EGL {
FunctionCache::FunctionCache(size_t totalSize, float percentageKeep) :
		m_Code(nullptr), m_Used(0), m_Total(0), m_Functions(nullptr), m_MostRecentlyUsed(nullptr), m_LeastRecentlyUsed(nullptr), m_UsedFunctions(0), m_MaxFunctions(0), m_PercentageKeep(0) {}
FunctionCache::~FunctionCache() {}
void *FunctionCache::GetFunction(FunctionType, const RasterizerState &) { return nullptr; }
void FunctionCache::CompactCode() {}
} // namespace EGL
#endif

// Provide the thread-local storage keys that are normally defined in egl.cpp
// (which we don't compile to avoid gl* symbol conflicts).
#include <pthread.h>
pthread_key_t s_TlsIndexContext;
pthread_key_t s_TlsIndexError;

using namespace EGL;

struct SWRVincent1::Impl {
	Config config;
	Surface *surface;
	Context *context;
	int width;
	int height;
	bool initialized;
};

SWRVincent1::SWRVincent1() {
	impl = nullptr;
}

SWRVincent1::~SWRVincent1() {
	destroy();
}

bool SWRVincent1::initialize(int width, int height) {
	destroy();

	impl = new Impl();
	memset(impl, 0, sizeof(Impl));
	impl->width = width;
	impl->height = height;

	// Configure for RGBA rendering with depth and stencil
	impl->config = Config(
			32, // bufferSize
			5, // redSize (RGB565)
			6, // greenSize
			5, // blueSize
			8, // alphaSize (separate alpha buffer)
			EGL_NONE, // configCaveat
			1, // configID
			16, // depthSize
			0, // level
			width, // maxPBufferWidth
			height, // maxPBufferHeight
			width * height, // maxPBufferPixels
			EGL_FALSE, // nativeRenderable
			0, // nativeVisualID
			0, // nativeVisualType
			0, // sampleBuffers
			0, // samples
			8, // stencilSize
			EGL_PBUFFER_BIT, // surfaceType
			EGL_NONE, // transparentType
			0, 0, 0, // transparent RGB
			width, // width
			height // height
	);

	impl->surface = new Surface(impl->config);
	impl->context = new Context(impl->config);

	// Make current
	Context::SetCurrentContext(impl->context);
	impl->context->SetDrawSurface(impl->surface);
	impl->context->SetReadSurface(impl->surface);

	// Set default viewport
	impl->context->Viewport(0, 0, width, height);

	impl->initialized = true;
	return true;
}

void SWRVincent1::destroy() {
	if (impl) {
		if (impl->initialized) {
			if (Context::GetCurrentContext() == impl->context) {
				Context::SetCurrentContext(nullptr);
			}
			if (impl->context) {
				impl->context->Dispose();
			}
			if (impl->surface) {
				impl->surface->Dispose();
			}
		}
		delete impl;
		impl = nullptr;
	}
}

String SWRVincent1::get_name() const {
	return "Vincent";
}

int SWRVincent1::get_width() const {
	return impl ? impl->width : 0;
}

int SWRVincent1::get_height() const {
	return impl ? impl->height : 0;
}

bool SWRVincent1::is_jit_enabled() {
	return EGL_USE_JIT != 0;
}

void SWRVincent1::viewport(int x, int y, int w, int h) {
	if (!impl || !impl->initialized) {
		return;
	}
	Context::SetCurrentContext(impl->context);
	impl->context->Viewport(x, y, w, h);
}

void SWRVincent1::clear_color(float r, float g, float b, float a) {
	if (!impl || !impl->initialized) {
		return;
	}
	Context::SetCurrentContext(impl->context);
	impl->context->ClearColor(r, g, b, a);
}

void SWRVincent1::clear(uint32_t mask) {
	if (!impl || !impl->initialized) {
		return;
	}
	Context::SetCurrentContext(impl->context);
	GLbitfield gl_mask = 0;
	if (mask & 1) {
		gl_mask |= GL_COLOR_BUFFER_BIT;
	}
	if (mask & 2) {
		gl_mask |= GL_DEPTH_BUFFER_BIT;
	}
	impl->context->Clear(gl_mask);
}

void SWRVincent1::read_pixels(uint8_t *rgba_dest) {
	if (!impl || !impl->initialized) {
		return;
	}

	// Vincent uses RGB565 color buffer + separate U8 alpha buffer.
	// Convert to RGBA8 for output.
	U16 *color_buf = impl->surface->GetColorBuffer();
	U8 *alpha_buf = impl->surface->GetAlphaBuffer();
	int total = impl->width * impl->height;

	for (int i = 0; i < total; i++) {
		U16 px = color_buf[i];
		// RGB565: RRRRR GGGGGG BBBBB
		uint8_t r = (uint8_t)(((px >> 11) & 0x1F) * 255 / 31);
		uint8_t g = (uint8_t)(((px >> 5) & 0x3F) * 255 / 63);
		uint8_t b = (uint8_t)((px & 0x1F) * 255 / 31);
		uint8_t a = alpha_buf ? alpha_buf[i] : 255;
		rgba_dest[i * 4 + 0] = r;
		rgba_dest[i * 4 + 1] = g;
		rgba_dest[i * 4 + 2] = b;
		rgba_dest[i * 4 + 3] = a;
	}
}

void SWRVincent1::set_depth_test(bool enabled) {
	if (!impl || !impl->initialized) {
		return;
	}
	Context::SetCurrentContext(impl->context);
	if (enabled) {
		impl->context->Enable(GL_DEPTH_TEST);
	} else {
		impl->context->Disable(GL_DEPTH_TEST);
	}
}

void SWRVincent1::set_blend(bool enabled) {
	if (!impl || !impl->initialized) {
		return;
	}
	Context::SetCurrentContext(impl->context);
	if (enabled) {
		impl->context->Enable(GL_BLEND);
		impl->context->BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else {
		impl->context->Disable(GL_BLEND);
	}
}

void SWRVincent1::set_cull_face(bool enabled) {
	if (!impl || !impl->initialized) {
		return;
	}
	Context::SetCurrentContext(impl->context);
	if (enabled) {
		impl->context->Enable(GL_CULL_FACE);
	} else {
		impl->context->Disable(GL_CULL_FACE);
	}
}

void SWRVincent1::draw(PrimitiveType type,
		const float *positions, int vertex_count,
		const float *colors,
		const float *texcoords,
		const float *normals,
		const float *mvp,
		const float *uniform_color) {
	if (!impl || !impl->initialized || vertex_count <= 0) {
		return;
	}

	Context::SetCurrentContext(impl->context);

	// Load MVP matrix (Vincent has separate projection/modelview,
	// so load MVP into modelview and set projection to identity)
	if (mvp) {
		impl->context->MatrixMode(GL_PROJECTION);
		impl->context->LoadIdentity();
		impl->context->MatrixMode(GL_MODELVIEW);
		impl->context->LoadMatrixf(mvp);
	}

	// Set up vertex pointer
	impl->context->EnableClientState(GL_VERTEX_ARRAY);
	impl->context->VertexPointer(3, GL_FLOAT, 0, positions);

	// Colors
	if (colors) {
		impl->context->EnableClientState(GL_COLOR_ARRAY);
		impl->context->ColorPointer(4, GL_FLOAT, 0, colors);
	} else {
		impl->context->DisableClientState(GL_COLOR_ARRAY);
		if (uniform_color) {
			impl->context->Color4f(
					uniform_color[0], uniform_color[1],
					uniform_color[2], uniform_color[3]);
		}
	}

	// Texcoords
	if (texcoords) {
		impl->context->EnableClientState(GL_TEXTURE_COORD_ARRAY);
		impl->context->TexCoordPointer(2, GL_FLOAT, 0, texcoords);
	} else {
		impl->context->DisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	// Normals
	if (normals) {
		impl->context->EnableClientState(GL_NORMAL_ARRAY);
		impl->context->NormalPointer(GL_FLOAT, 0, normals);
	} else {
		impl->context->DisableClientState(GL_NORMAL_ARRAY);
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
				impl->context->DrawArrays(GL_TRIANGLE_FAN, q * 4, 4);
			}
			goto cleanup;
		default:
			gl_mode = GL_TRIANGLES;
			break;
	}

	impl->context->DrawArrays(gl_mode, 0, vertex_count);

cleanup:
	impl->context->DisableClientState(GL_VERTEX_ARRAY);
	if (colors) {
		impl->context->DisableClientState(GL_COLOR_ARRAY);
	}
	if (texcoords) {
		impl->context->DisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	if (normals) {
		impl->context->DisableClientState(GL_NORMAL_ARRAY);
	}
}

uint32_t SWRVincent1::upload_texture(int w, int h, const uint8_t *rgba_data) {
	if (!impl || !impl->initialized) {
		return 0;
	}
	Context::SetCurrentContext(impl->context);

	GLuint tex;
	impl->context->GenTextures(1, &tex);
	impl->context->BindTexture(GL_TEXTURE_2D, tex);
	impl->context->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	impl->context->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	impl->context->TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, rgba_data);
	return (uint32_t)tex;
}

void SWRVincent1::bind_texture(uint32_t id) {
	if (!impl || !impl->initialized) {
		return;
	}
	Context::SetCurrentContext(impl->context);
	impl->context->BindTexture(GL_TEXTURE_2D, (GLuint)id);
}

void SWRVincent1::delete_texture(uint32_t id) {
	if (!impl || !impl->initialized) {
		return;
	}
	Context::SetCurrentContext(impl->context);
	GLuint tex = (GLuint)id;
	impl->context->DeleteTextures(1, &tex);
}

SWRBackend *SWRBackend::create_vincent1() {
	return new SWRVincent1();
}

#else // !SWRENDER_VINCENT1

SWRBackend *SWRBackend::create_vincent1() {
	return nullptr;
}

#endif // SWRENDER_VINCENT1
