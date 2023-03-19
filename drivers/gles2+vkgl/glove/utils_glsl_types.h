/**************************************************************************/
/*  utils_glsl_types.h                                                    */
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
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 */

/// Basic Types in GLSL (ESSL)

#ifndef UTILS_GLSL_TYPES_H
#define UTILS_GLSL_TYPES_H

#include <iostream>

typedef uint32_t glsl_bool_t;
typedef int32_t glsl_int_t;
typedef uint32_t glsl_uint_t;
typedef float glsl_float_t;

typedef struct {
	glsl_bool_t b[2];
} glsl_bvec2_t;
typedef struct {
	glsl_int_t i[2];
} glsl_ivec2_t;
typedef struct {
	glsl_uint_t u[2];
} glsl_uvec2_t;
typedef struct {
	glsl_float_t f[2];
} glsl_vec2_t;

typedef struct {
	glsl_bool_t b[3];
} glsl_bvec3_t;
typedef struct {
	glsl_int_t i[3];
} glsl_ivec3_t;
typedef struct {
	glsl_uint_t u[3];
} glsl_uvec3_t;
typedef struct {
	glsl_float_t f[3];
} glsl_vec3_t;

typedef struct {
	glsl_bool_t b[4];
} glsl_bvec4_t;
typedef struct {
	glsl_int_t i[4];
} glsl_ivec4_t;
typedef struct {
	glsl_uint_t u[4];
} glsl_uvec4_t;
typedef struct {
	glsl_float_t f[4];
} glsl_vec4_t;

typedef struct {
	glsl_vec4_t fm[2];
} glsl_mat2_t;
typedef struct {
	glsl_vec4_t fm[3];
} glsl_mat3_t;
typedef struct {
	glsl_vec4_t fm[4];
} glsl_mat4_t;

typedef uint32_t glsl_sampler_t;

typedef enum {
	SHADER_TYPE_INVALID = 0,
	SHADER_TYPE_VERTEX = 1 << 0,
	SHADER_TYPE_FRAGMENT = 1 << 1
} shader_type_t;

enum ESSL_VERSION {
	ESSL_VERSION_100 = 100,
	ESSL_VERSION_400 = 400,
	ESSL_VERSION_MAX = 0
};

inline void GlslPrintShaderSource(shader_type_t shaderType, ESSL_VERSION version, const std::string source) {
	FUN_ENTRY(GL_LOG_TRACE);
	std::cout << "\n\n-------- "
			  << ((shaderType == SHADER_TYPE_VERTEX) ? "VERTEX" : "FRAGMENT") << " SHADER v" << version
			  << " --------\n\n"
			  << source << "\n"
			  << "--------------------------------------\n\n";
}

inline size_t GlslTypeToAllignment(GLenum type) {
	FUN_ENTRY(GL_LOG_TRACE);

	switch (type) {
		case GL_BOOL:
		case GL_INT:
		case GL_FLOAT:
			return 16;

		case GL_BOOL_VEC2:
		case GL_INT_VEC2:
		case GL_FLOAT_VEC2:
			return 16;

		case GL_BOOL_VEC3:
		case GL_INT_VEC3:
		case GL_FLOAT_VEC3:
			return 16;

		case GL_BOOL_VEC4:
		case GL_INT_VEC4:
		case GL_FLOAT_VEC4:
			return 16;

		case GL_FLOAT_MAT2:
			return 32;
		case GL_FLOAT_MAT3:
			return 48;
		case GL_FLOAT_MAT4:
			return 64;

		case GL_SAMPLER_2D:
		case GL_SAMPLER_CUBE:
			return 16;
		default:
			return 0;
	}
}

inline size_t GlslTypeToSize(GLenum type) {
	FUN_ENTRY(GL_LOG_TRACE);

	switch (type) {
		case GL_BOOL:
			return sizeof(glsl_bool_t);
		case GL_INT:
			return sizeof(glsl_int_t);
		case GL_FLOAT:
			return sizeof(glsl_float_t);

		case GL_BOOL_VEC2:
			return sizeof(glsl_bvec2_t);
		case GL_INT_VEC2:
			return sizeof(glsl_ivec2_t);
		case GL_FLOAT_VEC2:
			return sizeof(glsl_vec2_t);

		case GL_BOOL_VEC3:
			return sizeof(glsl_bvec3_t);
		case GL_INT_VEC3:
			return sizeof(glsl_ivec3_t);
		case GL_FLOAT_VEC3:
			return sizeof(glsl_vec3_t);

		case GL_BOOL_VEC4:
			return sizeof(glsl_bvec4_t);
		case GL_INT_VEC4:
			return sizeof(glsl_ivec4_t);
		case GL_FLOAT_VEC4:
			return sizeof(glsl_vec4_t);

		case GL_FLOAT_MAT2:
			return sizeof(glsl_mat2_t);
		case GL_FLOAT_MAT3:
			return sizeof(glsl_mat3_t);
		case GL_FLOAT_MAT4:
			return sizeof(glsl_mat4_t);

		case GL_SAMPLER_2D:
		case GL_SAMPLER_CUBE:
			return sizeof(glsl_sampler_t);
		default:
			return 0;
	}
}

#endif // UTILS_GLSL_TYPES_H
