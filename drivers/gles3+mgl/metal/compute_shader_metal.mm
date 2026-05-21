/**************************************************************************/
/*  compute_shader_metal.mm                                               */
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

#include "compute_shader_metal.h"

#include "core/print_string.h"

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

ComputeShaderMetal::ComputeShaderMetal() {
	mtl_device = nullptr;
	mtl_command_queue = nullptr;
	mtl_library = nullptr;
	mtl_compute_pipeline = nullptr;
	threadgroup_size = DEFAULT_THREADGROUP_SIZE;
}

ComputeShaderMetal::~ComputeShaderMetal() {
	finish();
}

void ComputeShaderMetal::set_device(void *p_device, void *p_command_queue) {
	mtl_device = p_device;
	mtl_command_queue = p_command_queue;
}

bool ComputeShaderMetal::compile(const char *p_source, const char *p_kernel_name, const char *p_defines) {
	ERR_FAIL_COND_V(!mtl_device, false);
	ERR_FAIL_COND_V(!p_source, false);
	ERR_FAIL_COND_V(!p_kernel_name, false);

	@autoreleasepool {
		id<MTLDevice> device = (__bridge id<MTLDevice>)mtl_device;
		NSError *error = nil;

		NSMutableString *source = [NSMutableString string];

		if (p_defines) {
			[source appendFormat:@"%s\n", p_defines];
		}
		[source appendFormat:@"%s", p_source];

		MTLCompileOptions *options = [[MTLCompileOptions alloc] init];
		options.fastMathEnabled = YES;

		id<MTLLibrary> library = [device newLibraryWithSource:source
													  options:options
														error:&error];
		if (!library) {
			if (error) {
				ERR_PRINT(String("ComputeShaderMetal: Failed to compile '") + p_kernel_name +
						"': " + String::utf8([[error localizedDescription] UTF8String]));
			}
			return false;
		}

		NSString *kernelName = [NSString stringWithUTF8String:p_kernel_name];
		id<MTLFunction> kernelFunction = [library newFunctionWithName:kernelName];
		if (!kernelFunction) {
			ERR_PRINT(String("ComputeShaderMetal: Kernel function '") + p_kernel_name + "' not found");
			return false;
		}

		id<MTLComputePipelineState> pipeline = [device newComputePipelineStateWithFunction:kernelFunction
																					 error:&error];
		if (!pipeline) {
			if (error) {
				ERR_PRINT(String("ComputeShaderMetal: Failed to create pipeline for '") + p_kernel_name +
						"': " + String::utf8([[error localizedDescription] UTF8String]));
			}
			return false;
		}

		// Release previous state if recompiling
		if (mtl_library) {
			CFRelease(mtl_library);
		}
		if (mtl_compute_pipeline) {
			CFRelease(mtl_compute_pipeline);
		}

		mtl_library = (void *)CFBridgingRetain(library);
		mtl_compute_pipeline = (void *)CFBridgingRetain(pipeline);

		threadgroup_size = MIN((uint32_t)[pipeline maxTotalThreadsPerThreadgroup], DEFAULT_THREADGROUP_SIZE);

		print_verbose(String("ComputeShaderMetal: Compiled '") + p_kernel_name +
				"' (threadgroup_size=" + itos(threadgroup_size) + ")");
	}

	return true;
}

void ComputeShaderMetal::dispatch(uint32_t p_thread_count,
		const BufferBinding *p_buffers, uint32_t p_buffer_count,
		bool p_wait_for_completion) {
	ERR_FAIL_COND(!mtl_compute_pipeline);
	ERR_FAIL_COND(!mtl_command_queue);

	@autoreleasepool {
		id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)mtl_command_queue;
		id<MTLCommandBuffer> commandBuffer = [queue commandBuffer];
		commandBuffer.label = @"ComputeShaderMetal::dispatch";

		dispatch_with_command_buffer((void *)CFBridgingRetain(commandBuffer),
				p_thread_count, p_buffers, p_buffer_count);

		[commandBuffer commit];

		if (p_wait_for_completion) {
			[commandBuffer waitUntilCompleted];
		}

		CFRelease((__bridge CFTypeRef)commandBuffer);
	}
}

void ComputeShaderMetal::dispatch_with_command_buffer(void *p_command_buffer,
		uint32_t p_thread_count,
		const BufferBinding *p_buffers, uint32_t p_buffer_count) {
	ERR_FAIL_COND(!mtl_compute_pipeline);
	ERR_FAIL_COND(!p_command_buffer);

	@autoreleasepool {
		id<MTLCommandBuffer> commandBuffer = (__bridge id<MTLCommandBuffer>)p_command_buffer;
		id<MTLComputePipelineState> pipeline = (__bridge id<MTLComputePipelineState>)mtl_compute_pipeline;

		id<MTLComputeCommandEncoder> encoder = [commandBuffer computeCommandEncoder];

		[encoder setComputePipelineState:pipeline];

		for (uint32_t i = 0; i < p_buffer_count; i++) {
			if (p_buffers[i].buffer) {
				id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)p_buffers[i].buffer;
				[encoder setBuffer:buffer offset:p_buffers[i].offset atIndex:p_buffers[i].index];
			}
		}

		MTLSize threadgroupSize = MTLSizeMake(threadgroup_size, 1, 1);
		uint32_t threadgroup_count = (p_thread_count + threadgroup_size - 1) / threadgroup_size;
		MTLSize threadgroupCount = MTLSizeMake(threadgroup_count, 1, 1);

		[encoder dispatchThreadgroups:threadgroupCount threadsPerThreadgroup:threadgroupSize];
		[encoder endEncoding];
	}
}

void ComputeShaderMetal::finish() {
	@autoreleasepool {
		if (mtl_compute_pipeline) {
			CFRelease(mtl_compute_pipeline);
			mtl_compute_pipeline = nullptr;
		}
		if (mtl_library) {
			CFRelease(mtl_library);
			mtl_library = nullptr;
		}
	}
}
