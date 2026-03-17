/**************************************************************************/
/*  swr_builtin_shaders.h                                                 */
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

#ifndef SWR_BUILTIN_SHADERS_H
#define SWR_BUILTIN_SHADERS_H

// Built-in shader programs for the PortableGL backend.
// PortableGL uses C function pointers instead of GLSL.
// These are auto-selected based on which vertex attributes
// are used between begin_mesh() and end_mesh().

enum SWRShaderType {
	SWR_SHADER_FLAT_COLOR = 0, // uniform color only
	SWR_SHADER_VERTEX_COLOR, // per-vertex color interpolation
	SWR_SHADER_TEXTURED, // texture sampling with uniform color
	SWR_SHADER_TEXTURED_VERTEX_COLOR, // texture + per-vertex color
	SWR_SHADER_MAX,
};

// Uniform block passed to all built-in shaders.
struct SWRUniforms {
	float mvp[16]; // 4x4 column-major MVP matrix
	float color[4]; // uniform color (r,g,b,a)
	int tex_bound; // whether a texture is bound
};

#endif // SWR_BUILTIN_SHADERS_H
