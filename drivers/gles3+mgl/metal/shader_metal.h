/**************************************************************************/
/*  shader_metal.h                                                        */
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

#ifndef SHADER_METAL_H
#define SHADER_METAL_H

#include "core/hash_map.h"
#include "core/local_vector.h"
#include "core/map.h"
#include "core/math/camera_matrix.h"
#include "core/self_list.h"
#include "core/variant.h"

#include <stdio.h>

// Forward declarations for Metal types (actual types defined in .mm file)
#ifdef __OBJC__
@protocol MTLDevice;
@protocol MTLLibrary;
@protocol MTLFunction;
@protocol MTLRenderPipelineState;
@protocol MTLBuffer;
@protocol MTLTexture;
@protocol MTLSamplerState;
@protocol MTLRenderCommandEncoder;
#else
typedef void *id;
#endif

class ShaderMetal {
protected:
	struct Enum {
		uint64_t mask;
		uint64_t shift;
		const char *defines[16];
	};

	struct EnumValue {
		uint64_t set_mask;
		uint64_t clear_mask;
	};

	struct AttributePair {
		const char *name;
		int index;
	};

	struct UniformPair {
		const char *name;
		Variant::Type type_hint;
		size_t offset; // Offset in uniform buffer
		size_t size; // Size of the uniform
	};

	struct TexUnitPair {
		const char *name;
		int index;
	};

	struct UBOPair {
		const char *name;
		int index;
	};

private:
	int uniform_count;
	int texunit_pair_count;
	int conditional_count;
	int ubo_count;
	int attribute_pair_count;

	// Metal-specific state
	void *mtl_device; // id<MTLDevice>
	void *mtl_library; // id<MTLLibrary>
	void *mtl_vertex_function; // id<MTLFunction>
	void *mtl_fragment_function; // id<MTLFunction>

	// Uniform buffer
	void *mtl_uniform_buffer; // id<MTLBuffer>
	uint8_t *uniform_buffer_data;
	size_t uniform_buffer_size;
	bool uniform_buffer_dirty;

	// Pipeline state cache (keyed by version)
	struct Version {
		uint64_t version_key;
		void *pipeline_state; // id<MTLRenderPipelineState>
		bool valid;

		Version() :
				version_key(0),
				pipeline_state(nullptr),
				valid(false) {}
	};

	union VersionKey {
		struct {
			uint32_t version;
			uint32_t code_version;
		};
		uint64_t key;
		bool operator==(const VersionKey &p_key) const { return key == p_key.key; }
		bool operator<(const VersionKey &p_key) const { return key < p_key.key; }
		VersionKey() :
				key(0) {}
		VersionKey(uint64_t p_key) :
				key(p_key) {}
	};

	struct VersionKeyHash {
		static _FORCE_INLINE_ uint32_t hash(const VersionKey &p_key) { return HashMapHasherDefault::hash(p_key.key); }
	};

	HashMap<VersionKey, Version, VersionKeyHash> version_map;
	Version *current_version;

	VersionKey conditional_version;
	VersionKey new_conditional_version;

	virtual String get_shader_name() const = 0;

	const char **conditional_defines;
	const char **uniform_names;
	const AttributePair *attribute_pairs;
	const TexUnitPair *texunit_pairs;
	const UBOPair *ubo_pairs;
	const UniformPair *uniform_pairs;
	const char *shader_code;

	static ShaderMetal *active;

	Version *get_current_version();
	void _create_pipeline_state(Version *p_version);
	void _update_uniform_buffer();

protected:
	_FORCE_INLINE_ int _get_uniform(int p_which) const;
	_FORCE_INLINE_ size_t _get_uniform_offset(int p_which) const;
	_FORCE_INLINE_ void _set_conditional(int p_which, bool p_value);

	void setup(const char **p_conditional_defines,
			int p_conditional_count,
			const Enum *p_enums,
			int p_enum_count,
			const EnumValue *p_enum_values,
			int p_enum_value_count,
			const char *p_shader_code);

	ShaderMetal();

public:
	enum {
		CUSTOM_SHADER_DISABLED = 0
	};

	static _FORCE_INLINE_ ShaderMetal *get_active() { return active; }

	// Core interface
	bool bind();
	void unbind();

	void clear_caches();

	uint32_t get_version() const { return new_conditional_version.version; }
	_FORCE_INLINE_ bool is_version_valid() const { return current_version && current_version->valid; }

	virtual void init() = 0;
	void finish();

	// Metal-specific methods
	void set_metal_device(void *p_device);
	void *get_pipeline_state() const;
	void *get_uniform_buffer() const;

	// Flush uniform buffer to GPU
	void flush_uniforms();

	// Set uniform by writing to buffer
	void set_uniform_data(int p_uniform, const void *p_data, size_t p_size);

	virtual ~ShaderMetal();
};

// Inline implementations

int ShaderMetal::_get_uniform(int p_which) const {
	ERR_FAIL_INDEX_V(p_which, uniform_count, -1);
	return p_which; // In Metal, we use the enum value directly as an index
}

size_t ShaderMetal::_get_uniform_offset(int p_which) const {
	ERR_FAIL_INDEX_V(p_which, uniform_count, 0);
	ERR_FAIL_COND_V(!uniform_pairs, 0);
	return uniform_pairs[p_which].offset;
}

void ShaderMetal::_set_conditional(int p_which, bool p_value) {
	ERR_FAIL_INDEX(p_which, conditional_count);
	if (p_value) {
		new_conditional_version.version |= (1 << p_which);
	} else {
		new_conditional_version.version &= ~(1 << p_which);
	}
}

#endif // SHADER_METAL_H
