/**************************************************************************/
/*  shader_metal.mm                                                       */
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

#include "shader_metal.h"

#include "core/os/os.h"
#include "core/print_string.h"

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

ShaderMetal *ShaderMetal::active = nullptr;

ShaderMetal::ShaderMetal() {
	uniform_count = 0;
	texunit_pair_count = 0;
	conditional_count = 0;
	ubo_count = 0;
	attribute_pair_count = 0;

	mtl_device = nullptr;
	mtl_library = nullptr;
	mtl_vertex_function = nullptr;
	mtl_fragment_function = nullptr;
	mtl_uniform_buffer = nullptr;
	uniform_buffer_data = nullptr;
	uniform_buffer_size = 0;
	uniform_buffer_dirty = false;

	current_version = nullptr;

	conditional_defines = nullptr;
	uniform_names = nullptr;
	attribute_pairs = nullptr;
	texunit_pairs = nullptr;
	ubo_pairs = nullptr;
	uniform_pairs = nullptr;
	shader_code = nullptr;
}

ShaderMetal::~ShaderMetal() {
	finish();
}

void ShaderMetal::set_metal_device(void *p_device) {
	mtl_device = p_device;
}

void ShaderMetal::setup(const char **p_conditional_defines,
		int p_conditional_count,
		const Enum *p_enums,
		int p_enum_count,
		const EnumValue *p_enum_values,
		int p_enum_value_count,
		const char *p_shader_code) {
	conditional_defines = p_conditional_defines;
	conditional_count = p_conditional_count;
	shader_code = p_shader_code;

	// Initialize version tracking
	conditional_version.version = 0;
	conditional_version.code_version = 0;
	new_conditional_version = conditional_version;
}

ShaderMetal::Version *ShaderMetal::get_current_version() {
	VersionKey key = new_conditional_version;

	Version *v = version_map.getptr(key);
	if (v) {
		return v;
	}

	// Create new version
	Version new_version;
	new_version.version_key = key.key;
	new_version.valid = false;
	new_version.pipeline_state = nullptr;

	version_map[key] = new_version;
	v = version_map.getptr(key);

	// Create pipeline state for this version
	_create_pipeline_state(v);

	return v;
}

void ShaderMetal::_create_pipeline_state(Version *p_version) {
	if (!mtl_device || !shader_code) {
		return;
	}

	@autoreleasepool {
		id<MTLDevice> device = (__bridge id<MTLDevice>)mtl_device;
		NSError *error = nil;

		// Build shader source with conditionals
		NSMutableString *source = [NSMutableString string];

		// Add conditional defines based on version bits
		if (conditional_defines) {
			for (int i = 0; i < conditional_count; i++) {
				if (p_version->version_key & (1ULL << i)) {
					[source appendFormat:@"#define %s\n", conditional_defines[i]];
				}
			}
		}

		// Add the shader code
		[source appendFormat:@"%s", shader_code];

		// Compile the library
		id<MTLLibrary> library = [device newLibraryWithSource:source
													  options:nil
														error:&error];
		if (!library) {
			if (error) {
				NSLog(@"[ShaderMetal] Failed to compile shader %s: %@",
						get_shader_name().utf8().get_data(),
						[error localizedDescription]);
			}
			return;
		}

		// Get vertex and fragment functions
		id<MTLFunction> vertexFunction = [library newFunctionWithName:@"vertexFunction"];
		id<MTLFunction> fragmentFunction = [library newFunctionWithName:@"fragmentFunction"];

		if (!vertexFunction) {
			NSLog(@"[ShaderMetal] Failed to find vertexFunction in shader %s",
					get_shader_name().utf8().get_data());
			return;
		}

		if (!fragmentFunction) {
			NSLog(@"[ShaderMetal] Failed to find fragmentFunction in shader %s",
					get_shader_name().utf8().get_data());
			return;
		}

		// Create render pipeline descriptor
		MTLRenderPipelineDescriptor *pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
		pipelineDescriptor.label = [NSString stringWithFormat:@"%s_v%llu",
											 get_shader_name().utf8().get_data(), p_version->version_key];
		pipelineDescriptor.vertexFunction = vertexFunction;
		pipelineDescriptor.fragmentFunction = fragmentFunction;

		// Default color attachment format (can be overridden)
		pipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

		// Enable blending by default
		pipelineDescriptor.colorAttachments[0].blendingEnabled = YES;
		pipelineDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
		pipelineDescriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
		pipelineDescriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
		pipelineDescriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;

		// Create pipeline state
		id<MTLRenderPipelineState> pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineDescriptor
																						  error:&error];
		if (!pipelineState) {
			if (error) {
				NSLog(@"[ShaderMetal] Failed to create pipeline state for %s: %@",
						get_shader_name().utf8().get_data(),
						[error localizedDescription]);
			}
			return;
		}

		// Store the pipeline state (retain it manually since ARC is disabled)
		p_version->pipeline_state = (void *)CFBridgingRetain(pipelineState);
		p_version->valid = true;

		// Store library and functions for this shader instance
		if (!mtl_library) {
			mtl_library = (void *)CFBridgingRetain(library);
			mtl_vertex_function = (void *)CFBridgingRetain(vertexFunction);
			mtl_fragment_function = (void *)CFBridgingRetain(fragmentFunction);
		}

		print_verbose(String("ShaderMetal: Created pipeline for ") + get_shader_name() +
				" version " + itos(p_version->version_key));
	}
}

bool ShaderMetal::bind() {
	if (active == this && conditional_version.key == new_conditional_version.key) {
		// Already bound with same version
		return true;
	}

	// Get or create version for current conditionals
	Version *v = get_current_version();
	if (!v || !v->valid) {
		return false;
	}

	current_version = v;
	conditional_version = new_conditional_version;
	active = this;

	return true;
}

void ShaderMetal::unbind() {
	if (active == this) {
		active = nullptr;
	}
	current_version = nullptr;
}

void ShaderMetal::clear_caches() {
	@autoreleasepool {
		// Release all cached pipeline states using Godot's HashMap iteration pattern
		const VersionKey *key = nullptr;
		while ((key = version_map.next(key))) {
			Version *v = version_map.getptr(*key);
			if (v && v->pipeline_state) {
				CFRelease(v->pipeline_state);
				v->pipeline_state = nullptr;
			}
		}
		version_map.clear();
	}
}

void ShaderMetal::finish() {
	clear_caches();

	@autoreleasepool {
		// Release Metal objects
		if (mtl_uniform_buffer) {
			CFRelease(mtl_uniform_buffer);
			mtl_uniform_buffer = nullptr;
		}
		if (mtl_vertex_function) {
			CFRelease(mtl_vertex_function);
			mtl_vertex_function = nullptr;
		}
		if (mtl_fragment_function) {
			CFRelease(mtl_fragment_function);
			mtl_fragment_function = nullptr;
		}
		if (mtl_library) {
			CFRelease(mtl_library);
			mtl_library = nullptr;
		}
	}

	if (uniform_buffer_data) {
		memfree(uniform_buffer_data);
		uniform_buffer_data = nullptr;
	}

	uniform_buffer_size = 0;
	current_version = nullptr;

	if (active == this) {
		active = nullptr;
	}
}

void *ShaderMetal::get_pipeline_state() const {
	if (current_version && current_version->valid) {
		return current_version->pipeline_state;
	}
	return nullptr;
}

void *ShaderMetal::get_uniform_buffer() const {
	return mtl_uniform_buffer;
}

void ShaderMetal::set_uniform_data(int p_uniform, const void *p_data, size_t p_size) {
	ERR_FAIL_INDEX(p_uniform, uniform_count);
	ERR_FAIL_COND(!uniform_pairs);

	size_t offset = uniform_pairs[p_uniform].offset;
	size_t max_size = uniform_pairs[p_uniform].size;

	ERR_FAIL_COND(p_size > max_size);
	ERR_FAIL_COND(offset + p_size > uniform_buffer_size);

	if (!uniform_buffer_data) {
		// Allocate buffer on first use
		uniform_buffer_data = (uint8_t *)memalloc(uniform_buffer_size);
		memset(uniform_buffer_data, 0, uniform_buffer_size);
	}

	// Copy data to CPU buffer
	memcpy(uniform_buffer_data + offset, p_data, p_size);
	uniform_buffer_dirty = true;
}

void ShaderMetal::flush_uniforms() {
	if (!uniform_buffer_dirty || !uniform_buffer_data || !mtl_device) {
		return;
	}

	@autoreleasepool {
		id<MTLDevice> device = (__bridge id<MTLDevice>)mtl_device;

		if (!mtl_uniform_buffer) {
			// Create GPU buffer (retain manually since ARC is disabled)
			id<MTLBuffer> buffer = [device newBufferWithLength:uniform_buffer_size
													   options:MTLResourceStorageModeShared];
			mtl_uniform_buffer = (void *)CFBridgingRetain(buffer);
		}

		// Copy data to GPU buffer
		id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)mtl_uniform_buffer;
		memcpy([buffer contents], uniform_buffer_data, uniform_buffer_size);

		uniform_buffer_dirty = false;
	}
}

void ShaderMetal::_update_uniform_buffer() {
	flush_uniforms();
}
