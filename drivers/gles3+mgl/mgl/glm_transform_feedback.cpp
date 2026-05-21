/**************************************************************************/
/*  glm_transform_feedback.cpp                                            */
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

// Stub implementations for GLES3 transform feedback functions.
// Metal does not support transform feedback natively; particle
// simulation and blend shapes use compute shaders instead.
// These stubs prevent crashes when GL code paths call XFB functions.

#include "glm_context.h"

#include <stdio.h>
#include <string.h>

static bool _xfb_warned = false;

static void _xfb_warn_once() {
	if (!_xfb_warned) {
		fprintf(stderr, "[MGL] Transform feedback is not supported on Metal. "
						"Use compute shaders for particle/blend shape processing.\n");
		_xfb_warned = true;
	}
}

static void _stub_begin_transform_feedback(GLMContext ctx, GLenum primitiveMode) {
	_xfb_warn_once();
}

static void _stub_end_transform_feedback(GLMContext ctx) {
}

static void _stub_bind_transform_feedback(GLMContext ctx, GLenum target, GLuint id) {
}

static void _stub_delete_transform_feedbacks(GLMContext ctx, GLsizei n, const GLuint *ids) {
}

static void _stub_gen_transform_feedbacks(GLMContext ctx, GLsizei n, GLuint *ids) {
	for (GLsizei i = 0; i < n; i++) {
		ids[i] = i + 1;
	}
}

static GLboolean _stub_is_transform_feedback(GLMContext ctx, GLuint id) {
	return GL_FALSE;
}

static void _stub_pause_transform_feedback(GLMContext ctx) {
}

static void _stub_resume_transform_feedback(GLMContext ctx) {
}

static void _stub_draw_transform_feedback(GLMContext ctx, GLenum mode, GLuint id) {
	_xfb_warn_once();
}

static void _stub_draw_transform_feedback_stream(GLMContext ctx, GLenum mode, GLuint id, GLuint stream) {
	_xfb_warn_once();
}

static void _stub_draw_transform_feedback_instanced(GLMContext ctx, GLenum mode, GLuint id, GLsizei instancecount) {
	_xfb_warn_once();
}

static void _stub_draw_transform_feedback_stream_instanced(GLMContext ctx, GLenum mode, GLuint id, GLuint stream, GLsizei instancecount) {
	_xfb_warn_once();
}

static void _stub_create_transform_feedbacks(GLMContext ctx, GLsizei n, GLuint *ids) {
	for (GLsizei i = 0; i < n; i++) {
		ids[i] = i + 1;
	}
}

static void _stub_transform_feedback_buffer_base(GLMContext ctx, GLuint xfb, GLuint index, GLuint buffer) {
}

static void _stub_transform_feedback_buffer_range(GLMContext ctx, GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) {
}

static void _stub_get_transform_feedbackiv(GLMContext ctx, GLuint xfb, GLenum pname, GLint *param) {
	if (param) {
		*param = 0;
	}
}

static void _stub_get_transform_feedbacki_v(GLMContext ctx, GLuint xfb, GLenum pname, GLuint index, GLint *param) {
	if (param) {
		*param = 0;
	}
}

static void _stub_get_transform_feedbacki64_v(GLMContext ctx, GLuint xfb, GLenum pname, GLuint index, GLint64 *param) {
	if (param) {
		*param = 0;
	}
}

static void _stub_transform_feedback_varyings(GLMContext ctx, GLuint program, GLsizei count, const GLchar *const *varyings, GLenum bufferMode) {
}

static void _stub_get_transform_feedback_varying(GLMContext ctx, GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name) {
	if (length) {
		*length = 0;
	}
	if (size) {
		*size = 0;
	}
	if (type) {
		*type = GL_NONE;
	}
	if (name && bufSize > 0) {
		name[0] = '\0';
	}
}

void init_dispatch_transform_feedback(GLMContext ctx) {
	ctx->dispatch.begin_transform_feedback = _stub_begin_transform_feedback;
	ctx->dispatch.end_transform_feedback = _stub_end_transform_feedback;
	ctx->dispatch.bind_transform_feedback = _stub_bind_transform_feedback;
	ctx->dispatch.delete_transform_feedbacks = _stub_delete_transform_feedbacks;
	ctx->dispatch.gen_transform_feedbacks = _stub_gen_transform_feedbacks;
	ctx->dispatch.is_transform_feedback = _stub_is_transform_feedback;
	ctx->dispatch.pause_transform_feedback = _stub_pause_transform_feedback;
	ctx->dispatch.resume_transform_feedback = _stub_resume_transform_feedback;
	ctx->dispatch.draw_transform_feedback = _stub_draw_transform_feedback;
	ctx->dispatch.draw_transform_feedback_stream = _stub_draw_transform_feedback_stream;
	ctx->dispatch.draw_transform_feedback_instanced = _stub_draw_transform_feedback_instanced;
	ctx->dispatch.draw_transform_feedback_stream_instanced = _stub_draw_transform_feedback_stream_instanced;
	ctx->dispatch.create_transform_feedbacks = _stub_create_transform_feedbacks;
	ctx->dispatch.transform_feedback_buffer_base = _stub_transform_feedback_buffer_base;
	ctx->dispatch.transform_feedback_buffer_range = _stub_transform_feedback_buffer_range;
	ctx->dispatch.get_transform_feedbackiv = _stub_get_transform_feedbackiv;
	ctx->dispatch.get_transform_feedbacki_v = _stub_get_transform_feedbacki_v;
	ctx->dispatch.get_transform_feedbacki64_v = _stub_get_transform_feedbacki64_v;
	ctx->dispatch.transform_feedback_varyings = _stub_transform_feedback_varyings;
	ctx->dispatch.get_transform_feedback_varying = _stub_get_transform_feedback_varying;
}
