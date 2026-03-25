/**************************************************************************/
/*  gd_cpu_features.h                                                     */
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

#ifndef GD_CPU_FEATURES_H
#define GD_CPU_FEATURES_H

#include "core/reference.h"
#include "core/variant.h"

class CpuFeatures : public Reference {
	GDCLASS(CpuFeatures, Reference);

public:
	enum SimdLevel {
		SIMD_NONE,
		SIMD_SSE3,
		SIMD_SSE4_1,
		SIMD_SSE4_2,
		SIMD_AVX,
		SIMD_AVX2,
		SIMD_NEON,
	};

	enum Architecture {
		ARCH_UNKNOWN,
		ARCH_X86,
		ARCH_X86_64,
		ARCH_ARM,
		ARCH_AARCH64,
	};

private:
	bool detected;
	String cpu_name;
	String vendor;
	SimdLevel simd_level;
	Architecture architecture;
	Dictionary feature_flags;
	String microarchitecture_name;

	void detect();
	void detect_x86();
	void detect_aarch64();

protected:
	static void _bind_methods();

public:
	// --- Getters ---
	String get_cpu_name() const;
	String get_vendor() const;
	int get_simd_level() const;
	String get_simd_name() const;
	int get_architecture() const;
	String get_architecture_name() const;
	String get_microarchitecture() const;
	Dictionary get_features() const;
	bool has_feature(const String &feature) const;

	// --- Convenience ---
	bool has_sse3() const;
	bool has_sse4_1() const;
	bool has_sse4_2() const;
	bool has_avx() const;
	bool has_avx2() const;
	bool has_neon() const;
	bool has_aes() const;
	bool has_sha() const;

	// --- Summary ---
	Dictionary get_info() const;
	String get_summary() const;

	CpuFeatures();
};

VARIANT_ENUM_CAST(CpuFeatures::SimdLevel);
VARIANT_ENUM_CAST(CpuFeatures::Architecture);

#endif // GD_CPU_FEATURES_H
