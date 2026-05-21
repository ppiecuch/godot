/**************************************************************************/
/*  metal_compute_dispatch.h                                              */
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

#ifndef METAL_COMPUTE_DISPATCH_H
#define METAL_COMPUTE_DISPATCH_H

#include <stdint.h>

// C interface for Metal compute dispatch, callable from rasterizer_storage_gles3.cpp
// (which is compiled as C++, not Obj-C++).

#ifdef __cplusplus
extern "C" {
#endif

// Particle compute dispatch.
// Reads from gl_buffer_in, writes to gl_buffer_out (GL buffer names).
// Returns true on success.
bool metal_compute_particles_dispatch(
		uint32_t gl_buffer_in,
		uint32_t gl_buffer_out,
		uint32_t particle_count,
		const void *uniforms_data,
		uint32_t uniforms_size,
		const void *attractors_data,
		uint32_t attractors_size);

// Blend shape compute dispatch.
// base_buffer, blend_buffer, output_buffer are GL buffer names.
// vertex_count is number of vertices to process.
// blend_amount is the weight for this blend shape.
// format_flags encode which attributes are present (ENABLE_NORMAL, etc).
// Returns true on success.
bool metal_compute_blend_shape_dispatch(
		uint32_t base_buffer,
		uint32_t blend_buffer,
		uint32_t output_buffer,
		uint32_t vertex_count,
		float blend_amount,
		uint32_t format_flags);

// Initialize Metal compute pipelines. Called once during storage init.
bool metal_compute_init(void);

// Cleanup Metal compute resources.
void metal_compute_finish(void);

#ifdef __cplusplus
}
#endif

#endif // METAL_COMPUTE_DISPATCH_H
