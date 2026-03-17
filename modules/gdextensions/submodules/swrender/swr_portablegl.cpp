/**************************************************************************/
/*  swr_portablegl.cpp                                                    */
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

#include "swr_portablegl.h"

#ifdef SWRENDER_PORTABLEGL

#include "swr_builtin_shaders.h"

#include <string.h>

// Avoid type name conflicts with Godot (Color, vec2, mat4 etc.)
#define PGL_PREFIX_TYPES
#define PGL_PREFIX_GLSL
#define PGL_EXCLUDE_STUBS

// Suppress assert in release builds
#ifndef DEBUG_ENABLED
#define PGL_ASSERT(x) ((void)0)
#endif

// Rename all GL functions to avoid symbol conflicts with Fusion2X
// and system OpenGL. PortableGL is a single-header library, so
// macro renames apply consistently to both declarations and definitions.
#define glActiveTexture swrpgl_glActiveTexture
#define glAttachShader swrpgl_glAttachShader
#define glBindBuffer swrpgl_glBindBuffer
#define glBindFramebuffer swrpgl_glBindFramebuffer
#define glBindRenderbuffer swrpgl_glBindRenderbuffer
#define glBindTexture swrpgl_glBindTexture
#define glBindVertexArray swrpgl_glBindVertexArray
#define glBlendColor swrpgl_glBlendColor
#define glBlendEquation swrpgl_glBlendEquation
#define glBlendEquationSeparate swrpgl_glBlendEquationSeparate
#define glBlendFunc swrpgl_glBlendFunc
#define glBlendFuncSeparate swrpgl_glBlendFuncSeparate
#define glBlitFramebuffer swrpgl_glBlitFramebuffer
#define glBlitNamedFramebuffer swrpgl_glBlitNamedFramebuffer
#define glBufferData swrpgl_glBufferData
#define glBufferSubData swrpgl_glBufferSubData
#define glCheckFramebufferStatus swrpgl_glCheckFramebufferStatus
#define glClear swrpgl_glClear
#define glClearBufferfi swrpgl_glClearBufferfi
#define glClearBufferfv swrpgl_glClearBufferfv
#define glClearBufferiv swrpgl_glClearBufferiv
#define glClearBufferuiv swrpgl_glClearBufferuiv
#define glClearColor swrpgl_glClearColor
#define glClearDepth swrpgl_glClearDepth
#define glClearDepthf swrpgl_glClearDepthf
#define glClearNamedFramebufferfi swrpgl_glClearNamedFramebufferfi
#define glClearNamedFramebufferfv swrpgl_glClearNamedFramebufferfv
#define glClearNamedFramebufferiv swrpgl_glClearNamedFramebufferiv
#define glClearNamedFramebufferuiv swrpgl_glClearNamedFramebufferuiv
#define glClearStencil swrpgl_glClearStencil
#define glColorMask swrpgl_glColorMask
#define glColorMaski swrpgl_glColorMaski
#define glCompileShader swrpgl_glCompileShader
#define glCompressedTexImage1D swrpgl_glCompressedTexImage1D
#define glCompressedTexImage2D swrpgl_glCompressedTexImage2D
#define glCompressedTexImage3D swrpgl_glCompressedTexImage3D
#define glCreateProgram swrpgl_glCreateProgram
#define glCreateShader swrpgl_glCreateShader
#define glCreateTextures swrpgl_glCreateTextures
#define glCullFace swrpgl_glCullFace
#define glDebugMessageCallback swrpgl_glDebugMessageCallback
#define glDeleteBuffers swrpgl_glDeleteBuffers
#define glDeleteFramebuffers swrpgl_glDeleteFramebuffers
#define glDeleteProgram swrpgl_glDeleteProgram
#define glDeleteRenderbuffers swrpgl_glDeleteRenderbuffers
#define glDeleteShader swrpgl_glDeleteShader
#define glDeleteTextures swrpgl_glDeleteTextures
#define glDeleteVertexArrays swrpgl_glDeleteVertexArrays
#define glDepthFunc swrpgl_glDepthFunc
#define glDepthMask swrpgl_glDepthMask
#define glDepthRange swrpgl_glDepthRange
#define glDepthRangef swrpgl_glDepthRangef
#define glDetachShader swrpgl_glDetachShader
#define glDisable swrpgl_glDisable
#define glDisableVertexArrayAttrib swrpgl_glDisableVertexArrayAttrib
#define glDisableVertexAttribArray swrpgl_glDisableVertexAttribArray
#define glDrawArrays swrpgl_glDrawArrays
#define glDrawArraysInstanced swrpgl_glDrawArraysInstanced
#define glDrawArraysInstancedBaseInstance swrpgl_glDrawArraysInstancedBaseInstance
#define glDrawBuffers swrpgl_glDrawBuffers
#define glDrawElements swrpgl_glDrawElements
#define glDrawElementsInstanced swrpgl_glDrawElementsInstanced
#define glDrawElementsInstancedBaseInstance swrpgl_glDrawElementsInstancedBaseInstance
#define glEnable swrpgl_glEnable
#define glEnableVertexArrayAttrib swrpgl_glEnableVertexArrayAttrib
#define glEnableVertexAttribArray swrpgl_glEnableVertexAttribArray
#define glFramebufferRenderbuffer swrpgl_glFramebufferRenderbuffer
#define glFramebufferTexture swrpgl_glFramebufferTexture
#define glFramebufferTexture1D swrpgl_glFramebufferTexture1D
#define glFramebufferTexture2D swrpgl_glFramebufferTexture2D
#define glFramebufferTexture3D swrpgl_glFramebufferTexture3D
#define glFramebufferTextureLayer swrpgl_glFramebufferTextureLayer
#define glFrontFace swrpgl_glFrontFace
#define glGenBuffers swrpgl_glGenBuffers
#define glGenFramebuffers swrpgl_glGenFramebuffers
#define glGenRenderbuffers swrpgl_glGenRenderbuffers
#define glGenTextures swrpgl_glGenTextures
#define glGenVertexArrays swrpgl_glGenVertexArrays
#define glGenerateMipmap swrpgl_glGenerateMipmap
#define glGetAttribLocation swrpgl_glGetAttribLocation
#define glGetBooleanv swrpgl_glGetBooleanv
#define glGetDoublev swrpgl_glGetDoublev
#define glGetError swrpgl_glGetError
#define glGetFloatv swrpgl_glGetFloatv
#define glGetInteger64v swrpgl_glGetInteger64v
#define glGetIntegerv swrpgl_glGetIntegerv
#define glGetProgramInfoLog swrpgl_glGetProgramInfoLog
#define glGetProgramiv swrpgl_glGetProgramiv
#define glGetShaderInfoLog swrpgl_glGetShaderInfoLog
#define glGetShaderiv swrpgl_glGetShaderiv
#define glGetString swrpgl_glGetString
#define glGetStringi swrpgl_glGetStringi
#define glGetUniformLocation swrpgl_glGetUniformLocation
#define glIsEnabled swrpgl_glIsEnabled
#define glIsFramebuffer swrpgl_glIsFramebuffer
#define glIsProgram swrpgl_glIsProgram
#define glIsRenderbuffer swrpgl_glIsRenderbuffer
#define glLineWidth swrpgl_glLineWidth
#define glLinkProgram swrpgl_glLinkProgram
#define glLogicOp swrpgl_glLogicOp
#define glMultiDrawArrays swrpgl_glMultiDrawArrays
#define glMultiDrawElements swrpgl_glMultiDrawElements
#define glNamedBufferData swrpgl_glNamedBufferData
#define glNamedBufferSubData swrpgl_glNamedBufferSubData
#define glNamedFramebufferDrawBuffers swrpgl_glNamedFramebufferDrawBuffers
#define glNamedFramebufferReadBuffer swrpgl_glNamedFramebufferReadBuffer
#define glNamedFramebufferTextureLayer swrpgl_glNamedFramebufferTextureLayer
#define glNamedRenderbufferStorageMultisample swrpgl_glNamedRenderbufferStorageMultisample
#define glPixelStorei swrpgl_glPixelStorei
#define glPointParameteri swrpgl_glPointParameteri
#define glPointSize swrpgl_glPointSize
#define glPolygonMode swrpgl_glPolygonMode
#define glPolygonOffset swrpgl_glPolygonOffset
#define glProvokingVertex swrpgl_glProvokingVertex
#define glReadBuffer swrpgl_glReadBuffer
#define glRenderbufferStorage swrpgl_glRenderbufferStorage
#define glRenderbufferStorageMultisample swrpgl_glRenderbufferStorageMultisample
#define glScissor swrpgl_glScissor
#define glShaderSource swrpgl_glShaderSource
#define glStencilFunc swrpgl_glStencilFunc
#define glStencilFuncSeparate swrpgl_glStencilFuncSeparate
#define glStencilMask swrpgl_glStencilMask
#define glStencilMaskSeparate swrpgl_glStencilMaskSeparate
#define glStencilOp swrpgl_glStencilOp
#define glStencilOpSeparate swrpgl_glStencilOpSeparate
#define glTexImage1D swrpgl_glTexImage1D
#define glTexImage2D swrpgl_glTexImage2D
#define glTexImage3D swrpgl_glTexImage3D
#define glTexParameterf swrpgl_glTexParameterf
#define glTexParameterfv swrpgl_glTexParameterfv
#define glTexParameteri swrpgl_glTexParameteri
#define glTexParameteriv swrpgl_glTexParameteriv
#define glTexParameterliv swrpgl_glTexParameterliv
#define glTexParameterluiv swrpgl_glTexParameterluiv
#define glTexSubImage1D swrpgl_glTexSubImage1D
#define glTexSubImage2D swrpgl_glTexSubImage2D
#define glTexSubImage3D swrpgl_glTexSubImage3D
#define glTextureParameterf swrpgl_glTextureParameterf
#define glTextureParameterfv swrpgl_glTextureParameterfv
#define glTextureParameteri swrpgl_glTextureParameteri
#define glTextureParameteriv swrpgl_glTextureParameteriv
#define glTextureParameterliv swrpgl_glTextureParameterliv
#define glTextureParameterluiv swrpgl_glTextureParameterluiv
#define glUniform1f swrpgl_glUniform1f
#define glUniform1fv swrpgl_glUniform1fv
#define glUniform1i swrpgl_glUniform1i
#define glUniform1iv swrpgl_glUniform1iv
#define glUniform1ui swrpgl_glUniform1ui
#define glUniform1uiv swrpgl_glUniform1uiv
#define glUniform2f swrpgl_glUniform2f
#define glUniform2fv swrpgl_glUniform2fv
#define glUniform2i swrpgl_glUniform2i
#define glUniform2iv swrpgl_glUniform2iv
#define glUniform2ui swrpgl_glUniform2ui
#define glUniform2uiv swrpgl_glUniform2uiv
#define glUniform3f swrpgl_glUniform3f
#define glUniform3fv swrpgl_glUniform3fv
#define glUniform3i swrpgl_glUniform3i
#define glUniform3iv swrpgl_glUniform3iv
#define glUniform3ui swrpgl_glUniform3ui
#define glUniform3uiv swrpgl_glUniform3uiv
#define glUniform4f swrpgl_glUniform4f
#define glUniform4fv swrpgl_glUniform4fv
#define glUniform4i swrpgl_glUniform4i
#define glUniform4iv swrpgl_glUniform4iv
#define glUniform4ui swrpgl_glUniform4ui
#define glUniform4uiv swrpgl_glUniform4uiv
#define glUniformMatrix2fv swrpgl_glUniformMatrix2fv
#define glUniformMatrix2x3fv swrpgl_glUniformMatrix2x3fv
#define glUniformMatrix2x4fv swrpgl_glUniformMatrix2x4fv
#define glUniformMatrix3fv swrpgl_glUniformMatrix3fv
#define glUniformMatrix3x2fv swrpgl_glUniformMatrix3x2fv
#define glUniformMatrix3x4fv swrpgl_glUniformMatrix3x4fv
#define glUniformMatrix4fv swrpgl_glUniformMatrix4fv
#define glUniformMatrix4x2fv swrpgl_glUniformMatrix4x2fv
#define glUniformMatrix4x3fv swrpgl_glUniformMatrix4x3fv
#define glUnmapBuffer swrpgl_glUnmapBuffer
#define glUnmapNamedBuffer swrpgl_glUnmapNamedBuffer
#define glUseProgram swrpgl_glUseProgram
#define glVertexAttribDivisor swrpgl_glVertexAttribDivisor
#define glVertexAttribPointer swrpgl_glVertexAttribPointer
#define glViewport swrpgl_glViewport

#define PORTABLEGL_IMPLEMENTATION
#include "swr-portablegl/portablegl.h"

// ============================================================
// Built-in shader functions (C function pointers for PortableGL)
// ============================================================

// Flat color: MVP transform, uniform color
static void swr_vs_flat(float *vs_output, pgl_vec4 *vertex_attribs, Shader_Builtins *builtins, void *uniforms) {
	SWRUniforms *u = (SWRUniforms *)uniforms;
	pgl_mat4 mvp;
	memcpy(&mvp, u->mvp, sizeof(pgl_mat4));
	builtins->gl_Position = mult_mat4_vec4(mvp, vertex_attribs[0]);
	// No varying output needed — fragment shader uses uniform color
}

static void swr_fs_flat(float *fs_input, Shader_Builtins *builtins, void *uniforms) {
	SWRUniforms *u = (SWRUniforms *)uniforms;
	builtins->gl_FragColor.x = u->color[0];
	builtins->gl_FragColor.y = u->color[1];
	builtins->gl_FragColor.z = u->color[2];
	builtins->gl_FragColor.w = u->color[3];
}

// Vertex color: MVP transform, per-vertex color interpolation
static void swr_vs_vertcolor(float *vs_output, pgl_vec4 *vertex_attribs, Shader_Builtins *builtins, void *uniforms) {
	SWRUniforms *u = (SWRUniforms *)uniforms;
	pgl_mat4 mvp;
	memcpy(&mvp, u->mvp, sizeof(pgl_mat4));
	builtins->gl_Position = mult_mat4_vec4(mvp, vertex_attribs[0]);

	// Pass color as varying (4 floats)
	((pgl_vec4 *)vs_output)[0] = vertex_attribs[1];
}

static void swr_fs_vertcolor(float *fs_input, Shader_Builtins *builtins, void *uniforms) {
	builtins->gl_FragColor = ((pgl_vec4 *)fs_input)[0];
}

// Textured: MVP transform, texture sampling with uniform color tint
static void swr_vs_textured(float *vs_output, pgl_vec4 *vertex_attribs, Shader_Builtins *builtins, void *uniforms) {
	SWRUniforms *u = (SWRUniforms *)uniforms;
	pgl_mat4 mvp;
	memcpy(&mvp, u->mvp, sizeof(pgl_mat4));
	builtins->gl_Position = mult_mat4_vec4(mvp, vertex_attribs[0]);

	// Pass texcoord as varying (2 floats, padded to vec4 slot)
	vs_output[0] = vertex_attribs[1].x;
	vs_output[1] = vertex_attribs[1].y;
}

static void swr_fs_textured(float *fs_input, Shader_Builtins *builtins, void *uniforms) {
	SWRUniforms *u = (SWRUniforms *)uniforms;
	float s = fs_input[0];
	float t = fs_input[1];
	// texture2D equivalent — builtins provides the bound texture
	pgl_vec4 texel = texture2D(0, s, t);
	builtins->gl_FragColor.x = texel.x * u->color[0];
	builtins->gl_FragColor.y = texel.y * u->color[1];
	builtins->gl_FragColor.z = texel.z * u->color[2];
	builtins->gl_FragColor.w = texel.w * u->color[3];
}

// Textured + vertex color
static void swr_vs_textured_vertcolor(float *vs_output, pgl_vec4 *vertex_attribs, Shader_Builtins *builtins, void *uniforms) {
	SWRUniforms *u = (SWRUniforms *)uniforms;
	pgl_mat4 mvp;
	memcpy(&mvp, u->mvp, sizeof(pgl_mat4));
	builtins->gl_Position = mult_mat4_vec4(mvp, vertex_attribs[0]);

	// varying[0] = color (4 floats)
	((pgl_vec4 *)vs_output)[0] = vertex_attribs[1];
	// varying[1].xy = texcoord (2 floats)
	vs_output[4] = vertex_attribs[2].x;
	vs_output[5] = vertex_attribs[2].y;
}

static void swr_fs_textured_vertcolor(float *fs_input, Shader_Builtins *builtins, void *uniforms) {
	pgl_vec4 color = ((pgl_vec4 *)fs_input)[0];
	float s = fs_input[4];
	float t = fs_input[5];
	pgl_vec4 texel = texture2D(0, s, t);
	builtins->gl_FragColor.x = texel.x * color.x;
	builtins->gl_FragColor.y = texel.y * color.y;
	builtins->gl_FragColor.z = texel.z * color.z;
	builtins->gl_FragColor.w = texel.w * color.w;
}

// ============================================================
// PortableGL Backend Implementation
// ============================================================

struct SWRPortableGL::Impl {
	glContext ctx;
	u32 *backbuf;
	int width;
	int height;
	bool initialized;
	GLuint shader_programs[SWR_SHADER_MAX];
	SWRUniforms uniforms;
	GLuint vao;
	GLuint vbo_pos;
	GLuint vbo_color;
	GLuint vbo_texcoord;
};

SWRPortableGL::SWRPortableGL() {
	impl = nullptr;
}

SWRPortableGL::~SWRPortableGL() {
	destroy();
}

bool SWRPortableGL::initialize(int width, int height) {
	destroy();

	impl = new Impl();
	memset(impl, 0, sizeof(Impl));
	impl->width = width;
	impl->height = height;
	impl->backbuf = nullptr;

	// RGBA8 format: R in low byte on little-endian
	if (!init_glContext(&impl->ctx, &impl->backbuf, width, height, 32,
				0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000)) {
		delete impl;
		impl = nullptr;
		return false;
	}

	set_glContext(&impl->ctx);

	// Create shader programs
	// Flat color: 0 varyings
	{
		GLenum interp[] = { PGL_SMOOTH }; // dummy, not used
		impl->shader_programs[SWR_SHADER_FLAT_COLOR] =
				pglCreateProgram(swr_vs_flat, swr_fs_flat, 0, interp, GL_FALSE);
	}
	// Vertex color: 4 varyings (color rgba)
	{
		GLenum interp[] = { PGL_SMOOTH, PGL_SMOOTH, PGL_SMOOTH, PGL_SMOOTH };
		impl->shader_programs[SWR_SHADER_VERTEX_COLOR] =
				pglCreateProgram(swr_vs_vertcolor, swr_fs_vertcolor, 4, interp, GL_FALSE);
	}
	// Textured: 2 varyings (texcoord st)
	{
		GLenum interp[] = { PGL_SMOOTH, PGL_SMOOTH };
		impl->shader_programs[SWR_SHADER_TEXTURED] =
				pglCreateProgram(swr_vs_textured, swr_fs_textured, 2, interp, GL_FALSE);
	}
	// Textured + vertex color: 6 varyings (4 color + 2 texcoord)
	{
		GLenum interp[] = { PGL_SMOOTH, PGL_SMOOTH, PGL_SMOOTH, PGL_SMOOTH, PGL_SMOOTH, PGL_SMOOTH };
		impl->shader_programs[SWR_SHADER_TEXTURED_VERTEX_COLOR] =
				pglCreateProgram(swr_vs_textured_vertcolor, swr_fs_textured_vertcolor, 6, interp, GL_FALSE);
	}

	// Create shared VAO and VBOs
	glGenVertexArrays(1, &impl->vao);
	glBindVertexArray(impl->vao);

	glGenBuffers(1, &impl->vbo_pos);
	glGenBuffers(1, &impl->vbo_color);
	glGenBuffers(1, &impl->vbo_texcoord);

	// Default state
	impl->uniforms.color[0] = 1.0f;
	impl->uniforms.color[1] = 1.0f;
	impl->uniforms.color[2] = 1.0f;
	impl->uniforms.color[3] = 1.0f;

	// Set identity MVP
	memset(impl->uniforms.mvp, 0, sizeof(impl->uniforms.mvp));
	impl->uniforms.mvp[0] = 1.0f;
	impl->uniforms.mvp[5] = 1.0f;
	impl->uniforms.mvp[10] = 1.0f;
	impl->uniforms.mvp[15] = 1.0f;

	glViewport(0, 0, width, height);

	impl->initialized = true;
	return true;
}

void SWRPortableGL::destroy() {
	if (impl) {
		if (impl->initialized) {
			set_glContext(&impl->ctx);
			glDeleteBuffers(1, &impl->vbo_pos);
			glDeleteBuffers(1, &impl->vbo_color);
			glDeleteBuffers(1, &impl->vbo_texcoord);
			glDeleteVertexArrays(1, &impl->vao);
			for (int i = 0; i < SWR_SHADER_MAX; i++) {
				if (impl->shader_programs[i]) {
					glDeleteProgram(impl->shader_programs[i]);
				}
			}
			free_glContext(&impl->ctx);
		}
		delete impl;
		impl = nullptr;
	}
}

String SWRPortableGL::get_name() const {
	return "PortableGL";
}

int SWRPortableGL::get_width() const {
	return impl ? impl->width : 0;
}

int SWRPortableGL::get_height() const {
	return impl ? impl->height : 0;
}

void SWRPortableGL::viewport(int x, int y, int w, int h) {
	if (!impl || !impl->initialized)
		return;
	set_glContext(&impl->ctx);
	glViewport(x, y, w, h);
}

void SWRPortableGL::clear_color(float r, float g, float b, float a) {
	if (!impl || !impl->initialized)
		return;
	set_glContext(&impl->ctx);
	glClearColor(r, g, b, a);
}

void SWRPortableGL::clear(uint32_t mask) {
	if (!impl || !impl->initialized)
		return;
	set_glContext(&impl->ctx);
	GLbitfield gl_mask = 0;
	if (mask & 1)
		gl_mask |= GL_COLOR_BUFFER_BIT;
	if (mask & 2)
		gl_mask |= GL_DEPTH_BUFFER_BIT;
	glClear(gl_mask);
}

void SWRPortableGL::read_pixels(uint8_t *rgba_dest) {
	if (!impl || !impl->initialized || !impl->backbuf)
		return;

	// PortableGL backbuffer is u32* in the mask order we specified.
	// We set masks for RGBA8 byte order, so we can memcpy directly.
	int total = impl->width * impl->height;
	memcpy(rgba_dest, impl->backbuf, total * 4);
}

void SWRPortableGL::set_depth_test(bool enabled) {
	if (!impl || !impl->initialized)
		return;
	set_glContext(&impl->ctx);
	if (enabled) {
		glEnable(GL_DEPTH_TEST);
	} else {
		glDisable(GL_DEPTH_TEST);
	}
}

void SWRPortableGL::set_blend(bool enabled) {
	if (!impl || !impl->initialized)
		return;
	set_glContext(&impl->ctx);
	if (enabled) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	} else {
		glDisable(GL_BLEND);
	}
}

void SWRPortableGL::set_cull_face(bool enabled) {
	if (!impl || !impl->initialized)
		return;
	set_glContext(&impl->ctx);
	if (enabled) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
}

static GLenum _prim_to_gl(SWRBackend::PrimitiveType type) {
	switch (type) {
		case SWRBackend::PRIM_POINTS:
			return GL_POINTS;
		case SWRBackend::PRIM_LINES:
			return GL_LINES;
		case SWRBackend::PRIM_LINE_STRIP:
			return GL_LINE_STRIP;
		case SWRBackend::PRIM_LINE_LOOP:
			return GL_LINE_LOOP;
		case SWRBackend::PRIM_TRIANGLES:
			return GL_TRIANGLES;
		case SWRBackend::PRIM_TRIANGLE_STRIP:
			return GL_TRIANGLE_STRIP;
		case SWRBackend::PRIM_TRIANGLE_FAN:
			return GL_TRIANGLE_FAN;
		case SWRBackend::PRIM_QUADS:
			return GL_TRIANGLE_FAN; // approximate
		default:
			return GL_TRIANGLES;
	}
}

void SWRPortableGL::draw(PrimitiveType type,
		const float *positions, int vertex_count,
		const float *colors,
		const float *texcoords,
		const float *normals,
		const float *mvp,
		const float *uniform_color) {
	if (!impl || !impl->initialized || vertex_count <= 0)
		return;

	set_glContext(&impl->ctx);
	glBindVertexArray(impl->vao);

	// Select shader based on attributes
	SWRShaderType shader_type;
	if (texcoords && colors) {
		shader_type = SWR_SHADER_TEXTURED_VERTEX_COLOR;
	} else if (texcoords) {
		shader_type = SWR_SHADER_TEXTURED;
	} else if (colors) {
		shader_type = SWR_SHADER_VERTEX_COLOR;
	} else {
		shader_type = SWR_SHADER_FLAT_COLOR;
	}

	glUseProgram(impl->shader_programs[shader_type]);

	// Set uniforms
	if (mvp) {
		memcpy(impl->uniforms.mvp, mvp, 16 * sizeof(float));
	}
	if (uniform_color) {
		memcpy(impl->uniforms.color, uniform_color, 4 * sizeof(float));
	}
	pglSetUniform(&impl->uniforms);

	// Upload position data (attribute 0)
	glBindBuffer(GL_ARRAY_BUFFER, impl->vbo_pos);
	glBufferData(GL_ARRAY_BUFFER, vertex_count * 3 * sizeof(float), positions, GL_STREAM_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	int next_attrib = 1;

	if (shader_type == SWR_SHADER_VERTEX_COLOR || shader_type == SWR_SHADER_TEXTURED_VERTEX_COLOR) {
		glBindBuffer(GL_ARRAY_BUFFER, impl->vbo_color);
		glBufferData(GL_ARRAY_BUFFER, vertex_count * 4 * sizeof(float), colors, GL_STREAM_DRAW);
		glEnableVertexAttribArray(next_attrib);
		glVertexAttribPointer(next_attrib, 4, GL_FLOAT, GL_FALSE, 0, 0);
		next_attrib++;
	}

	if (shader_type == SWR_SHADER_TEXTURED || shader_type == SWR_SHADER_TEXTURED_VERTEX_COLOR) {
		glBindBuffer(GL_ARRAY_BUFFER, impl->vbo_texcoord);
		glBufferData(GL_ARRAY_BUFFER, vertex_count * 2 * sizeof(float), texcoords, GL_STREAM_DRAW);
		glEnableVertexAttribArray(next_attrib);
		glVertexAttribPointer(next_attrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
		next_attrib++;
	}

	// Handle quads by drawing as two triangles per quad
	if (type == PRIM_QUADS) {
		int quad_count = vertex_count / 4;
		for (int q = 0; q < quad_count; q++) {
			glDrawArrays(GL_TRIANGLE_FAN, q * 4, 4);
		}
	} else {
		glDrawArrays(_prim_to_gl(type), 0, vertex_count);
	}

	// Cleanup attrib state
	for (int i = 0; i < next_attrib; i++) {
		glDisableVertexAttribArray(i);
	}
}

uint32_t SWRPortableGL::upload_texture(int w, int h, const uint8_t *rgba_data) {
	if (!impl || !impl->initialized)
		return 0;
	set_glContext(&impl->ctx);

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_data);
	return (uint32_t)tex;
}

void SWRPortableGL::bind_texture(uint32_t id) {
	if (!impl || !impl->initialized)
		return;
	set_glContext(&impl->ctx);
	glBindTexture(GL_TEXTURE_2D, (GLuint)id);
}

void SWRPortableGL::delete_texture(uint32_t id) {
	if (!impl || !impl->initialized)
		return;
	set_glContext(&impl->ctx);
	GLuint tex = (GLuint)id;
	glDeleteTextures(1, &tex);
}

SWRBackend *SWRBackend::create_portablegl() {
	return new SWRPortableGL();
}

#else // !SWRENDER_PORTABLEGL

SWRBackend *SWRBackend::create_portablegl() {
	return nullptr;
}

#endif // SWRENDER_PORTABLEGL
