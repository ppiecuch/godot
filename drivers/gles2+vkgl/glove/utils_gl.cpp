/**************************************************************************/
/*  utils_gl.cpp                                                          */
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

/**
 * Copyright (C) 2015-2018 Think Silicon S.A. (https://think-silicon.com/)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public v3
 * License as published by the Free Software Foundation;
 */

/// A variety of OpengGL ES utility functions.

#include "utils_gl.h"
#include "glLogger.h"
#include "parser_helpers.h"

#define CASE_STR(c) \
	case GL_##c:    \
		return "GL_" STRINGIFY(c);

GLboolean
GlColorMaskHasBit(GLubyte colorMask, GLColorMaskBit bit) {
	FUN_ENTRY(GL_LOG_DEBUG);

	return ((colorMask >> bit) & 0x1) == 1 ? GL_TRUE : GL_FALSE;
}

GLubyte
GlColorMaskPack(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
	FUN_ENTRY(GL_LOG_DEBUG);

	return static_cast<GLubyte>((red << GLColorMaskBit::GLC_RED) |
			(green << GLColorMaskBit::GLC_GREEN) |
			(blue << GLColorMaskBit::GLC_BLUE) |
			(alpha << GLColorMaskBit::GLC_ALPHA));
}

const char *
GlAttribTypeToString(GLenum type) {
	FUN_ENTRY(GL_LOG_DEBUG);

	switch (type) {
		CASE_STR(BOOL);
		CASE_STR(BOOL_VEC2);
		CASE_STR(BOOL_VEC3);
		CASE_STR(BOOL_VEC4);

		CASE_STR(INT);
		CASE_STR(INT_VEC2);
		CASE_STR(INT_VEC3);
		CASE_STR(INT_VEC4);

		CASE_STR(FLOAT);
		CASE_STR(FLOAT_VEC2);
		CASE_STR(FLOAT_VEC3);
		CASE_STR(FLOAT_VEC4);

		CASE_STR(FLOAT_MAT2);
		CASE_STR(FLOAT_MAT3);
		CASE_STR(FLOAT_MAT4);
		default: {
			NOT_FOUND_ENUM(type);
			return STRINGIFY(GL_INVALID_ENUM);
		}
	}
}

GLenum
GlFormatToGlInternalFormat(GLenum format, GLenum type) {
	FUN_ENTRY(GL_LOG_DEBUG);

	switch (format) {
		case GL_DEPTH_COMPONENT16:
			return GL_DEPTH_COMPONENT16;
		case GL_DEPTH_COMPONENT24_OES:
			return GL_DEPTH_COMPONENT24_OES;
		case GL_DEPTH_COMPONENT32_OES:
			return GL_DEPTH_COMPONENT32_OES;
		case GL_STENCIL_INDEX8:
			return GL_STENCIL_INDEX8;
		case GL_STENCIL_INDEX4_OES:
			return GL_STENCIL_INDEX4_OES;
		case GL_BGRA_EXT:
		case GL_BGRA8_EXT:
			switch (type) {
				case GL_UNSIGNED_BYTE:
					return GL_BGRA8_EXT;
				default:
					NOT_FOUND_ENUM(type);
					return GL_INVALID_VALUE;
			}
		case GL_RGBA:
			switch (type) {
				case GL_UNSIGNED_SHORT_4_4_4_4:
					return GL_RGBA4;
				case GL_UNSIGNED_SHORT_5_5_5_1:
					return GL_RGB5_A1;
				case GL_UNSIGNED_BYTE:
					return GL_RGBA8_OES;
				default:
					NOT_FOUND_ENUM(type);
					return GL_INVALID_VALUE;
			}

		case GL_RGB:
			switch (type) {
				case GL_UNSIGNED_SHORT_5_6_5:
					return GL_RGB565;
				case GL_UNSIGNED_BYTE:
					return GL_RGB8_OES;
				default:
					NOT_FOUND_ENUM(type);
					return GL_INVALID_VALUE;
			}

		case GL_LUMINANCE_ALPHA:
			switch (type) {
				case GL_UNSIGNED_BYTE:
					return GL_LUMINANCE_ALPHA;
				default:
					NOT_FOUND_ENUM(type);
					return GL_INVALID_VALUE;
			}

		case GL_LUMINANCE:
			switch (type) {
				case GL_UNSIGNED_BYTE:
					return GL_LUMINANCE;
				default:
					NOT_FOUND_ENUM(type);
					return GL_INVALID_VALUE;
			}

		case GL_ALPHA:
			switch (type) {
				case GL_UNSIGNED_BYTE:
					return GL_ALPHA;
				default:
					NOT_FOUND_ENUM(type);
					return GL_INVALID_VALUE;
			}

		case GL_DEPTH24_STENCIL8_OES:
			switch (type) {
				case GL_UNSIGNED_INT_24_8_OES:
					return GL_DEPTH24_STENCIL8_OES;
				default:
					NOT_FOUND_ENUM(type);
					return GL_INVALID_VALUE;
			}

		default:
			NOT_FOUND_ENUM(format);
			return GL_INVALID_VALUE;
	}
}

GLenum
GlInternalFormatToGlType(GLenum internalformat) {
	FUN_ENTRY(GL_LOG_DEBUG);

	switch (internalformat) {
		case GL_RGBA4:
			return GL_UNSIGNED_SHORT_4_4_4_4;
		case GL_RGB5_A1:
			return GL_UNSIGNED_SHORT_5_5_5_1;
		case GL_RGB565:
			return GL_UNSIGNED_SHORT_5_6_5;
		case GL_ALPHA:
		case GL_LUMINANCE:
		case GL_LUMINANCE_ALPHA:
		case GL_DEPTH_COMPONENT16:
		case GL_DEPTH_COMPONENT24_OES:
		case GL_DEPTH_COMPONENT32_OES:
		case GL_STENCIL_INDEX8:
		case GL_STENCIL_INDEX4_OES:
		case GL_RGB:
		case GL_RGBA:
		case GL_RGB8_OES:
		case GL_BGRA8_EXT:
		case GL_RGBA8_OES:
			return GL_UNSIGNED_BYTE;
		case GL_DEPTH24_STENCIL8_OES:
			return GL_UNSIGNED_INT_24_8_OES;
		default:
			NOT_FOUND_ENUM(internalformat);
			return GL_INVALID_VALUE;
	}
}

GLenum
GlInternalFormatToGlFormat(GLenum internalFormat) {
	FUN_ENTRY(GL_LOG_DEBUG);

	switch (internalFormat) {
		case GL_ALPHA:
			return GL_ALPHA;
		case GL_LUMINANCE:
			return GL_LUMINANCE;
		case GL_LUMINANCE_ALPHA:
			return GL_LUMINANCE_ALPHA;
		case GL_RGB:
		case GL_RGB565:
		case GL_RGB8_OES:
			return GL_RGB;
		case GL_BGRA8_EXT:
			return GL_BGRA8_EXT;
		case GL_RGBA:
		case GL_RGBA8_OES:
		case GL_RGBA4:
		case GL_RGB5_A1:
			return GL_RGBA;
		case GL_DEPTH_COMPONENT16:
			return GL_DEPTH_COMPONENT16;
		case GL_DEPTH_COMPONENT24_OES:
			return GL_DEPTH_COMPONENT24_OES;
		case GL_DEPTH_COMPONENT32_OES:
			return GL_DEPTH_COMPONENT32_OES;
		case GL_DEPTH24_STENCIL8_OES:
			return GL_DEPTH24_STENCIL8_OES;
		case GL_STENCIL_INDEX8:
			return GL_STENCIL_INDEX8;
		case GL_STENCIL_INDEX4_OES:
			return GL_STENCIL_INDEX4_OES;
		default:
			NOT_FOUND_ENUM(internalFormat);
			return GL_INVALID_VALUE;
	}
}

int GlInternalFormatTypeToNumElements(GLenum internalFormat, GLenum type) {
	FUN_ENTRY(GL_LOG_DEBUG);

	switch (type) {
		case GL_UNSIGNED_BYTE:
			switch (internalFormat) {
				case GL_STENCIL_INDEX4_OES:
				case GL_STENCIL_INDEX8:
				case GL_ALPHA8_OES:
				case GL_ALPHA:
				case GL_LUMINANCE:
					return 1;
				case GL_LUMINANCE_ALPHA:
					return 2;
				case GL_RGB8_OES:
				case GL_RGB:
					return 3;
				case GL_RGBA8_OES:
				case GL_BGRA8_EXT:
				case GL_BGRA_EXT:
				case GL_RGBA:
					return 4;
				default: {
					NOT_FOUND_ENUM(internalFormat);
					return 1;
				}
			}
		case GL_UNSIGNED_SHORT_4_4_4_4:
			switch (internalFormat) {
				case GL_RGBA4:
				case GL_RGBA8_OES:
				case GL_BGRA8_EXT:
				case GL_BGRA_EXT:
				case GL_RGBA:
					return 1;
				default: {
					NOT_FOUND_ENUM(internalFormat);
					return 1;
				}
			}
		case GL_UNSIGNED_SHORT_5_5_5_1:
			switch (internalFormat) {
				case GL_RGB5_A1:
				case GL_RGBA8_OES:
				case GL_BGRA8_EXT:
				case GL_BGRA_EXT:
				case GL_RGBA:
					return 1;
				default: {
					NOT_FOUND_ENUM(internalFormat);
					return 1;
				}
			}
		case GL_UNSIGNED_SHORT_5_6_5:
			switch (internalFormat) {
				case GL_RGB565:
				case GL_RGB8_OES:
				case GL_RGB:
					return 1;
				default: {
					NOT_FOUND_ENUM(internalFormat);
					return 1;
				}
			}
		case GL_UNSIGNED_INT_24_8_OES:
			switch (internalFormat) {
				case GL_DEPTH24_STENCIL8_OES:
					return 1;
				default: {
					NOT_FOUND_ENUM(internalFormat);
					return 1;
				}
			}
		default: {
			NOT_FOUND_ENUM(type);
			return 1;
		}
	}
}

int32_t
GlAttribTypeToElementSize(GLenum type) {
	FUN_ENTRY(GL_LOG_DEBUG);

	switch (type) {
		case GL_BYTE:
			return sizeof(GLbyte);
		case GL_UNSIGNED_BYTE:
			return sizeof(GLubyte);
		case GL_SHORT:
			return sizeof(GLshort);
		case GL_UNSIGNED_SHORT:
			return sizeof(GLushort);
		case GL_FIXED:
			return sizeof(GLfixed);
		case GL_FLOAT:
			return sizeof(GLfloat);
		default: {
			NOT_FOUND_ENUM(type);
			return sizeof(GLubyte);
		}
	}
}

int GlTypeToElementSize(GLenum type) {
	FUN_ENTRY(GL_LOG_DEBUG);

	switch (type) {
		case GL_UNSIGNED_BYTE:
			return sizeof(GLubyte);
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_5_5_5_1:
		case GL_UNSIGNED_SHORT_5_6_5:
			return sizeof(GLushort);
		case GL_UNSIGNED_INT_24_8_OES:
			return sizeof(GLuint);
		default: {
			NOT_FOUND_ENUM(type);
			return sizeof(GLubyte);
		}
	}
}

void GlFormatToStorageBits(GLenum format, GLint *r_, GLint *g_, GLint *b_, GLint *a_, GLint *d_, GLint *s_) {
	FUN_ENTRY(GL_LOG_DEBUG);

	GLint r, g, b, a, d, s;

	switch (format) {
		case GL_RGB565:
			r = 5;
			g = 6;
			b = 5;
			a = 0;
			d = 0;
			s = 0;
			break;

		case GL_RGBA4:
			r = 4;
			g = 4;
			b = 4;
			a = 4;
			d = 0;
			s = 0;
			break;

		case GL_RGB5_A1:
			r = 5;
			g = 5;
			b = 5;
			a = 1;
			d = 0;
			s = 0;
			break;

		case GL_RGB:
		case GL_RGB8_OES:
			r = 8;
			g = 8;
			b = 8;
			a = 0;
			d = 0;
			s = 0;
			break;

		case GL_RGBA:
		case GL_BGRA8_EXT:
		case GL_RGBA8_OES:
		case GL_BGRA_EXT:
			r = 8;
			g = 8;
			b = 8;
			a = 8;
			d = 0;
			s = 0;
			break;

		case GL_DEPTH_COMPONENT16:
			r = 0;
			g = 0;
			b = 0;
			a = 0;
			d = 16;
			s = 0;
			break;

		case GL_DEPTH_COMPONENT24_OES:
			r = 0;
			g = 0;
			b = 0;
			a = 0;
			d = 24;
			s = 0;
			break;

		case GL_DEPTH24_STENCIL8_OES:
			r = 0;
			g = 0;
			b = 0;
			a = 0;
			d = 24;
			s = 8;
			break;

		case GL_DEPTH_COMPONENT32_OES:
			r = 0;
			g = 0;
			b = 0;
			a = 0;
			d = 32;
			s = 0;
			break;

		case GL_STENCIL_INDEX1_OES:
		case GL_STENCIL_INDEX4_OES:
		case GL_STENCIL_INDEX8:
			r = 0;
			g = 0;
			b = 0;
			a = 0;
			d = 0;
			s = 8;
			break;

		default:
			r = 0;
			g = 0;
			b = 0;
			a = 0;
			d = 0;
			s = 0;
			break;
	}

	if (r_)
		*r_ = r;
	if (g_)
		*g_ = g;
	if (b_)
		*b_ = b;
	if (a_)
		*a_ = a;
	if (d_)
		*d_ = d;
	if (s_)
		*s_ = s;
}

void GlFormatToStorageBits(GLenum format, GLfloat *r_, GLfloat *g_, GLfloat *b_, GLfloat *a_, GLfloat *d_, GLfloat *s_) {
	FUN_ENTRY(GL_LOG_DEBUG);

	GLfloat r, g, b, a, d, s;

	switch (format) {
		case GL_RGB565:
			r = 5.0f;
			g = 6.0f;
			b = 5.0f;
			a = 0.0f;
			d = 0.0f;
			s = 0.0f;
			break;

		case GL_RGBA4:
			r = 4.0f;
			g = 4.0f;
			b = 4.0f;
			a = 4.0f;
			d = 0.0f;
			s = 0.0f;
			break;

		case GL_RGB5_A1:
			r = 5.0f;
			g = 5.0f;
			b = 5.0f;
			a = 1.0f;
			d = 0.0f;
			s = 0.0f;
			break;

		case GL_RGB:
		case GL_RGB8_OES:
			r = 8.0f;
			g = 8.0f;
			b = 8.0f;
			a = 0.0f;
			d = 0.0f;
			s = 0.0f;
			break;

		case GL_RGBA:
		case GL_BGRA8_EXT:
		case GL_RGBA8_OES:
		case GL_BGRA_EXT:
			r = 8.0f;
			g = 8.0f;
			b = 8.0f;
			a = 8.0f;
			d = 0.0f;
			s = 0.0f;
			break;

		case GL_DEPTH_COMPONENT16:
			r = 0.0f;
			g = 0.0f;
			b = 0.0f;
			a = 0.0f;
			d = 16.0f;
			s = 0.0f;
			break;

		case GL_DEPTH_COMPONENT24_OES:
			r = 0.0f;
			g = 0.0f;
			b = 0.0f;
			a = 0.0f;
			d = 24.0f;
			s = 0.0f;
			break;

		case GL_DEPTH24_STENCIL8_OES:
			r = 0.0f;
			g = 0.0f;
			b = 0.0f;
			a = 0.0f;
			d = 24.0f;
			s = 8.0f;
			break;

		case GL_DEPTH_COMPONENT32_OES:
			r = 0.0f;
			g = 0.0f;
			b = 0.0f;
			a = 0.0f;
			d = 32.0f;
			s = 0.0f;
			break;

		case GL_STENCIL_INDEX1_OES:
		case GL_STENCIL_INDEX4_OES:
		case GL_STENCIL_INDEX8:
			r = 0.0f;
			g = 0.0f;
			b = 0.0f;
			a = 0.0f;
			d = 0.0f;
			s = 8.0f;
			break;

		default:
			r = 0.0f;
			g = 0.0f;
			b = 0.0f;
			a = 0.0f;
			d = 0.0f;
			s = 0.0f;
			break;
	}

	if (r_)
		*r_ = r;
	if (g_)
		*g_ = g;
	if (b_)
		*b_ = b;
	if (a_)
		*a_ = a;
	if (d_)
		*d_ = d;
	if (s_)
		*s_ = s;
}

void GlFormatToStorageBits(GLenum format, GLboolean *r_, GLboolean *g_, GLboolean *b_, GLboolean *a_, GLboolean *d_, GLboolean *s_) {
	FUN_ENTRY(GL_LOG_DEBUG);

	GLboolean r, g, b, a, d, s;

	switch (format) {
		case GL_RGB565:
			r = GL_TRUE;
			g = GL_TRUE;
			b = GL_TRUE;
			a = GL_FALSE;
			d = GL_FALSE;
			s = GL_FALSE;
			break;

		case GL_RGBA4:
			r = GL_TRUE;
			g = GL_TRUE;
			b = GL_TRUE;
			a = GL_TRUE;
			d = GL_FALSE;
			s = GL_FALSE;
			break;

		case GL_RGB5_A1:
			r = GL_TRUE;
			g = GL_TRUE;
			b = GL_TRUE;
			a = GL_TRUE;
			d = GL_FALSE;
			s = GL_FALSE;
			break;

		case GL_RGB:
		case GL_RGB8_OES:
			r = GL_TRUE;
			g = GL_TRUE;
			b = GL_TRUE;
			a = GL_FALSE;
			d = GL_FALSE;
			s = GL_FALSE;
			break;

		case GL_RGBA:
		case GL_BGRA8_EXT:
		case GL_RGBA8_OES:
		case GL_BGRA_EXT:
			r = GL_TRUE;
			g = GL_TRUE;
			b = GL_TRUE;
			a = GL_TRUE;
			d = GL_FALSE;
			s = GL_FALSE;
			break;

		case GL_DEPTH_COMPONENT16:
			r = GL_FALSE;
			g = GL_FALSE;
			b = GL_FALSE;
			a = GL_FALSE;
			d = GL_TRUE;
			s = GL_FALSE;
			break;

		case GL_DEPTH_COMPONENT24_OES:
			r = GL_FALSE;
			g = GL_FALSE;
			b = GL_FALSE;
			a = GL_FALSE;
			d = GL_TRUE;
			s = GL_FALSE;
			break;

		case GL_DEPTH24_STENCIL8_OES:
			r = GL_FALSE;
			g = GL_FALSE;
			b = GL_FALSE;
			a = GL_FALSE;
			d = GL_TRUE;
			s = GL_TRUE;
			break;

		case GL_DEPTH_COMPONENT32_OES:
			r = GL_FALSE;
			g = GL_FALSE;
			b = GL_FALSE;
			a = GL_FALSE;
			d = GL_TRUE;
			s = GL_FALSE;
			break;

		case GL_STENCIL_INDEX1_OES:
		case GL_STENCIL_INDEX4_OES:
		case GL_STENCIL_INDEX8:
			r = GL_FALSE;
			g = GL_FALSE;
			b = GL_FALSE;
			a = GL_FALSE;
			d = GL_FALSE;
			s = GL_TRUE;
			break;

		default:
			r = GL_FALSE;
			g = GL_FALSE;
			b = GL_FALSE;
			a = GL_FALSE;
			d = GL_FALSE;
			s = GL_FALSE;
			break;
	}

	if (r_)
		*r_ = r;
	if (g_)
		*g_ = g;
	if (b_)
		*b_ = b;
	if (a_)
		*a_ = a;
	if (d_)
		*d_ = d;
	if (s_)
		*s_ = s;
}

bool GlFormatIsDepthRenderable(GLenum format) {
	FUN_ENTRY(GL_LOG_DEBUG);

	switch (format) {
		case GL_DEPTH_COMPONENT16:
		case GL_DEPTH_COMPONENT24_OES:
		case GL_DEPTH_COMPONENT32_OES:
			return true;
		default:
			return false;
	}
}

bool GlFormatIsStencilRenderable(GLenum format) {
	FUN_ENTRY(GL_LOG_DEBUG);

	switch (format) {
		case GL_STENCIL_INDEX4_OES:
		case GL_STENCIL_INDEX8:
			return true;
		default:
			return false;
	}
}

bool GlFormatIsColorRenderable(GLenum format) {
	FUN_ENTRY(GL_LOG_DEBUG);

	switch (format) {
		case GL_RGB8_OES:
		case GL_RGBA8_OES:
		case GL_RGB565:
		case GL_RGBA4:
		case GL_RGB5_A1:
			return true;
		default:
			return false;
	}
}

uint32_t
OccupiedLocationsPerGlType(GLenum type) {
	FUN_ENTRY(GL_LOG_DEBUG);

	switch (type) {
		case GL_BOOL:
		case GL_INT:
		case GL_FLOAT:
		case GL_BOOL_VEC2:
		case GL_INT_VEC2:
		case GL_FLOAT_VEC2:
		case GL_BOOL_VEC3:
		case GL_INT_VEC3:
		case GL_FLOAT_VEC3:
		case GL_BOOL_VEC4:
		case GL_INT_VEC4:
		case GL_FLOAT_VEC4:
			return 1;
		case GL_FLOAT_MAT2:
			return 2;
		case GL_FLOAT_MAT3:
			return 3;
		case GL_FLOAT_MAT4:
			return 4;
		default:
			NOT_REACHED();
			return 0;
	}
}

bool IsGlSampler(GLenum type) {
	return (type == GL_SAMPLER_2D) || (type == GL_SAMPLER_CUBE);
}
