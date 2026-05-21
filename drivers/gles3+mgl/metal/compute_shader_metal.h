/**************************************************************************/
/*  compute_shader_metal.h                                                */
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

#ifndef COMPUTE_SHADER_METAL_H
#define COMPUTE_SHADER_METAL_H

#include "core/ustring.h"

#include <stddef.h>
#include <stdint.h>

class ComputeShaderMetal {
public:
	static const uint32_t DEFAULT_THREADGROUP_SIZE = 256;

private:
	void *mtl_device; // id<MTLDevice>
	void *mtl_command_queue; // id<MTLCommandQueue>
	void *mtl_library; // id<MTLLibrary>
	void *mtl_compute_pipeline; // id<MTLComputePipelineState>
	uint32_t threadgroup_size;

public:
	ComputeShaderMetal();
	~ComputeShaderMetal();

	void set_device(void *p_device, void *p_command_queue);

	bool compile(const char *p_source, const char *p_kernel_name, const char *p_defines = nullptr);

	bool is_valid() const { return mtl_compute_pipeline != nullptr; }

	struct BufferBinding {
		void *buffer; // id<MTLBuffer>
		uint32_t index;
		uint32_t offset;
	};

	void dispatch(uint32_t p_thread_count,
			const BufferBinding *p_buffers, uint32_t p_buffer_count,
			bool p_wait_for_completion = false);

	void dispatch_with_command_buffer(void *p_command_buffer,
			uint32_t p_thread_count,
			const BufferBinding *p_buffers, uint32_t p_buffer_count);

	uint32_t get_threadgroup_size() const { return threadgroup_size; }
	void set_threadgroup_size(uint32_t p_size) { threadgroup_size = p_size; }

	void *get_device() const { return mtl_device; }
	void *get_command_queue() const { return mtl_command_queue; }

	void finish();
};

#endif // COMPUTE_SHADER_METAL_H
