/**************************************************************************/
/*  metal_compute_dispatch.mm                                             */
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

#include "metal_compute_dispatch.h"
#include "compute_shader_metal.h"

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#include "../shaders/blend_shape.metal.src.h"
#include "../shaders/particles.metal.src.h"

static ComputeShaderMetal *s_particles_compute = nullptr;
static ComputeShaderMetal *s_blend_shape_compute = nullptr;
static id<MTLDevice> s_device = nil;
static id<MTLCommandQueue> s_command_queue = nil;

// GL buffer name to Metal buffer mapping.
// In the current MGL architecture, GL buffers backed by Metal use
// MTLBuffer stored in the buffer's data.mtl_data field. Since the MGL
// context infrastructure isn't fully wired yet, we use glMapBufferRange
// to get the CPU pointer and create a shared Metal buffer from it.
#include "../mgl/mgl.h"

static id<MTLBuffer> _get_or_create_metal_buffer(uint32_t gl_buffer_name, size_t size) {
	if (!gl_buffer_name || !s_device) {
		return nil;
	}
	// Bind buffer and map its contents to get the CPU pointer
	glBindBuffer(GL_ARRAY_BUFFER, gl_buffer_name);
	void *ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, size,
			GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
	if (!ptr) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		return nil;
	}
	// Create a no-copy Metal buffer that shares the GL buffer's memory
	id<MTLBuffer> mtl_buf = [s_device newBufferWithBytesNoCopy:ptr
														length:size
													   options:MTLResourceStorageModeShared
												   deallocator:nil];
	if (!mtl_buf) {
		// Fallback: copy the data
		mtl_buf = [s_device newBufferWithBytes:ptr
										length:size
									   options:MTLResourceStorageModeShared];
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return mtl_buf;
}

bool metal_compute_init(void) {
	@autoreleasepool {
		s_device = MTLCreateSystemDefaultDevice();
		if (!s_device) {
			NSLog(@"[MetalCompute] Failed to create Metal device");
			return false;
		}

		s_command_queue = [s_device newCommandQueue];
		if (!s_command_queue) {
			NSLog(@"[MetalCompute] Failed to create command queue");
			return false;
		}

		s_particles_compute = new ComputeShaderMetal();
		s_particles_compute->set_device(
				(void *)CFBridgingRetain(s_device),
				(void *)CFBridgingRetain(s_command_queue));

		if (!s_particles_compute->compile(
					particles_metal_src,
					"particleKernel",
					nullptr)) {
			NSLog(@"[MetalCompute] Failed to compile particle compute shader");
			delete s_particles_compute;
			s_particles_compute = nullptr;
		}

		s_blend_shape_compute = new ComputeShaderMetal();
		s_blend_shape_compute->set_device(
				(void *)CFBridgingRetain(s_device),
				(void *)CFBridgingRetain(s_command_queue));

		if (!s_blend_shape_compute->compile(
					blend_shape_metal_src,
					"blendShapeKernel",
					nullptr)) {
			NSLog(@"[MetalCompute] Failed to compile blend shape compute shader");
			delete s_blend_shape_compute;
			s_blend_shape_compute = nullptr;
		}

		return true;
	}
}

void metal_compute_finish(void) {
	if (s_particles_compute) {
		delete s_particles_compute;
		s_particles_compute = nullptr;
	}
	if (s_blend_shape_compute) {
		delete s_blend_shape_compute;
		s_blend_shape_compute = nullptr;
	}
	s_command_queue = nil;
	s_device = nil;
}

bool metal_compute_particles_dispatch(
		uint32_t gl_buffer_in,
		uint32_t gl_buffer_out,
		uint32_t particle_count,
		const void *uniforms_data,
		uint32_t uniforms_size,
		const void *attractors_data,
		uint32_t attractors_size) {
	if (!s_particles_compute || !s_particles_compute->is_valid()) {
		return false;
	}

	@autoreleasepool {
		size_t buffer_size = particle_count * 24 * sizeof(float);
		id<MTLBuffer> buffer_in = _get_or_create_metal_buffer(gl_buffer_in, buffer_size);
		id<MTLBuffer> buffer_out = _get_or_create_metal_buffer(gl_buffer_out, buffer_size);

		if (!buffer_in || !buffer_out) {
			return false;
		}

		// Create temporary buffers for uniforms and attractors
		id<MTLBuffer> uniforms_buffer = [s_device newBufferWithBytes:uniforms_data
															  length:uniforms_size
															 options:MTLResourceStorageModeShared];

		id<MTLBuffer> attractors_buffer = nil;
		if (attractors_data && attractors_size > 0) {
			attractors_buffer = [s_device newBufferWithBytes:attractors_data
													  length:attractors_size
													 options:MTLResourceStorageModeShared];
		} else {
			// Empty attractors buffer
			attractors_buffer = [s_device newBufferWithLength:16
													  options:MTLResourceStorageModeShared];
		}

		ComputeShaderMetal::BufferBinding bindings[] = {
			{ (void *)CFBridgingRetain(uniforms_buffer), 0, 0 },
			{ (void *)CFBridgingRetain(attractors_buffer), 1, 0 },
			{ (void *)CFBridgingRetain(buffer_in), 2, 0 },
			{ (void *)CFBridgingRetain(buffer_out), 3, 0 },
		};

		s_particles_compute->dispatch(particle_count, bindings, 4, true);

		// Release retained references
		for (int i = 0; i < 4; i++) {
			CFRelease(bindings[i].buffer);
		}

		return true;
	}
}

bool metal_compute_blend_shape_dispatch(
		uint32_t base_buffer,
		uint32_t blend_buffer,
		uint32_t output_buffer,
		uint32_t vertex_count,
		float blend_amount,
		uint32_t format_flags) {
	if (!s_blend_shape_compute || !s_blend_shape_compute->is_valid()) {
		return false;
	}

	@autoreleasepool {
		// Estimate buffer size from vertex count and format (conservative)
		size_t buffer_size = vertex_count * 128; // max stride per vertex
		id<MTLBuffer> mtl_base = _get_or_create_metal_buffer(base_buffer, buffer_size);
		id<MTLBuffer> mtl_blend = blend_buffer ? _get_or_create_metal_buffer(blend_buffer, buffer_size) : nil;
		id<MTLBuffer> mtl_output = _get_or_create_metal_buffer(output_buffer, buffer_size);

		if (!mtl_base || !mtl_output) {
			return false;
		}

		// Uniforms: blend_amount + vertex_count
		struct {
			float blend_amount;
			uint32_t vertex_count;
		} uniforms = { blend_amount, vertex_count };

		id<MTLBuffer> uniforms_buffer = [s_device newBufferWithBytes:&uniforms
															  length:sizeof(uniforms)
															 options:MTLResourceStorageModeShared];

		// Buffer layout follows blend_shape.metal kernel signature:
		// buffer(0) = uniforms
		// buffer(1) = vertex_in (base)
		// buffer(9) = vertex_blend (accumulated)
		// buffer(17) = vertex_out
		ComputeShaderMetal::BufferBinding bindings[4];
		uint32_t binding_count = 3;

		bindings[0] = { (void *)CFBridgingRetain(uniforms_buffer), 0, 0 };
		bindings[1] = { (void *)CFBridgingRetain(mtl_base), 1, 0 };
		bindings[2] = { (void *)CFBridgingRetain(mtl_output), 17, 0 };

		if (mtl_blend) {
			bindings[3] = { (void *)CFBridgingRetain(mtl_blend), 9, 0 };
			binding_count = 4;
		}

		s_blend_shape_compute->dispatch(vertex_count, bindings, binding_count, true);

		for (uint32_t i = 0; i < binding_count; i++) {
			CFRelease(bindings[i].buffer);
		}

		return true;
	}
}
