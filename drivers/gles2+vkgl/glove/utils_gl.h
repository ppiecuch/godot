/**************************************************************************/
/*  utils_gl.h                                                            */
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

/// OpengGL ES Utility Functions

#ifndef UTILS_GL_H
#define UTILS_GL_H

#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"

#include <stdint.h>
#include <cstdio>

enum GLColorMaskBit {
	GLC_RED = 0,
	GLC_GREEN = 1,
	GLC_BLUE = 2,
	GLC_ALPHA = 3
};

GLboolean GlColorMaskHasBit(GLubyte colorMask, GLColorMaskBit bit);
GLubyte GlColorMaskPack(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
const char *GlAttribTypeToString(GLenum type);
GLenum GlFormatToGlInternalFormat(GLenum format, GLenum type);
GLenum GlInternalFormatToGlType(GLenum internalFormat);
GLenum GlInternalFormatToGlFormat(GLenum internalFormat);
int GlInternalFormatTypeToNumElements(GLenum format, GLenum type);
int32_t GlAttribTypeToElementSize(GLenum type);
int GlTypeToElementSize(GLenum type);
void GlFormatToStorageBits(GLenum format, GLint *r_, GLint *g_, GLint *b_, GLint *a_, GLint *d_, GLint *s_);
void GlFormatToStorageBits(GLenum format, GLfloat *r_, GLfloat *g_, GLfloat *b_, GLfloat *a_, GLfloat *d_, GLfloat *s_);
void GlFormatToStorageBits(GLenum format, GLboolean *r_, GLboolean *g_, GLboolean *b_, GLboolean *a_, GLboolean *d_, GLboolean *s_);
bool GlFormatIsDepthRenderable(GLenum format);
bool GlFormatIsStencilRenderable(GLenum format);
bool GlFormatIsColorRenderable(GLenum format);
uint32_t OccupiedLocationsPerGlType(GLenum type);
bool IsGlSampler(GLenum type);

#endif // UTILS_GL_H
