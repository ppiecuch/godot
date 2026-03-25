/**************************************************************************/
/*  gd_cpu_features.cpp                                                   */
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

#include "gd_cpu_features.h"

#include "cpu_features/cpu_features_macros.h"
#include "cpu_features/cpu_features_types.h"

// Architecture detection using standard compiler macros (the local
// cpu_features_macros.h strips out arch detection, so we use compiler builtins).
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#define GD_CPU_ARCH_X86
#include "cpu_features/cpuinfo_x86.h"
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
#define GD_CPU_ARCH_AARCH64
#include "cpu_features/cpuinfo_aarch64.h"
#endif

#define MAXCPUNAME 49

// ── Helpers ─────────────────────────────────────────────────────────────────

static String trim_cpu_name(const char *raw, int max_len) {
	const char *start = raw;
	const char *end = raw + max_len - 1;
	while (*start == ' ') {
		start++;
	}
	while (end > start && (*end == ' ' || *end == '\0')) {
		end--;
	}
	return String::utf8(start, end - start + 1).strip_edges();
}

// ── Detection ───────────────────────────────────────────────────────────────

void CpuFeatures::detect_x86() {
#ifdef GD_CPU_ARCH_X86
	X86Info info = {};

	bool ok = GetX86Info(&info);
	if (!ok) {
		return;
	}

#if defined(ORBIS)
	cpu_name = "Orbis";
#elif defined(PROSPERO)
	cpu_name = "Prospero";
#else
	char brand[MAXCPUNAME] = "";
	FillX86BrandString(brand);
	cpu_name = trim_cpu_name(brand, MAXCPUNAME);
#endif

	vendor = String::utf8(info.vendor);

	// Detect SIMD level (highest supported)
	if (info.features.avx2) {
		simd_level = SIMD_AVX2;
	} else if (info.features.avx) {
		simd_level = SIMD_AVX;
	} else if (info.features.sse4_2) {
		simd_level = SIMD_SSE4_2;
	} else if (info.features.sse4_1) {
		simd_level = SIMD_SSE4_1;
	} else if (info.features.sse3) {
		simd_level = SIMD_SSE3;
	}

	// Microarchitecture
	X86Microarchitecture uarch = GetX86Microarchitecture(&info);
	microarchitecture_name = String::utf8(GetX86MicroarchitectureName(uarch));

	// Architecture
#if defined(__x86_64__) || defined(_M_X64)
	architecture = ARCH_X86_64;
#else
	architecture = ARCH_X86;
#endif

	// Feature flags
	const X86Features &f = info.features;
	feature_flags["fpu"] = (bool)f.fpu;
	feature_flags["tsc"] = (bool)f.tsc;
	feature_flags["mmx"] = (bool)f.mmx;
	feature_flags["sse"] = (bool)f.sse;
	feature_flags["sse2"] = (bool)f.sse2;
	feature_flags["sse3"] = (bool)f.sse3;
	feature_flags["ssse3"] = (bool)f.ssse3;
	feature_flags["sse4_1"] = (bool)f.sse4_1;
	feature_flags["sse4_2"] = (bool)f.sse4_2;
	feature_flags["sse4a"] = (bool)f.sse4a;
	feature_flags["avx"] = (bool)f.avx;
	feature_flags["avx2"] = (bool)f.avx2;
	feature_flags["avx512f"] = (bool)f.avx512f;
	feature_flags["avx512bw"] = (bool)f.avx512bw;
	feature_flags["avx512dq"] = (bool)f.avx512dq;
	feature_flags["avx512vl"] = (bool)f.avx512vl;
	feature_flags["aes"] = (bool)f.aes;
	feature_flags["sha"] = (bool)f.sha;
	feature_flags["fma3"] = (bool)f.fma3;
	feature_flags["f16c"] = (bool)f.f16c;
	feature_flags["bmi1"] = (bool)f.bmi1;
	feature_flags["bmi2"] = (bool)f.bmi2;
	feature_flags["popcnt"] = (bool)f.popcnt;
	feature_flags["sgx"] = (bool)f.sgx;
	feature_flags["cx16"] = (bool)f.cx16;
	feature_flags["rdrnd"] = (bool)f.rdrnd;
	feature_flags["rdseed"] = (bool)f.rdseed;
	feature_flags["pclmulqdq"] = (bool)f.pclmulqdq;

	detected = true;
#endif
}

void CpuFeatures::detect_aarch64() {
#if defined(GD_CPU_ARCH_AARCH64) || defined(TARGET_IOS_SIMULATOR)
	Aarch64Info info = {};

	simd_level = SIMD_NEON;
	architecture = ARCH_AARCH64;

#if defined(ANDROID) || defined(__LINUX__) || defined(TARGET_APPLE_ARM64) || defined(__APPLE__)
	bool ok = GetAarch64Info(&info);
	if (!ok) {
		cpu_name = "ARM64";
	} else {
		cpu_name = String::utf8(info.name).strip_edges();
	}
#else
	cpu_name = "ARM64";
#endif

	// Feature flags
	const Aarch64Features &f = info.features;
	feature_flags["fp"] = (bool)f.fp;
	feature_flags["asimd"] = (bool)f.asimd;
	feature_flags["aes"] = (bool)f.aes;
	feature_flags["pmull"] = (bool)f.pmull;
	feature_flags["sha1"] = (bool)f.sha1;
	feature_flags["sha2"] = (bool)f.sha2;
	feature_flags["crc32"] = (bool)f.crc32;
	feature_flags["atomics"] = (bool)f.atomics;
	feature_flags["fphp"] = (bool)f.fphp;
	feature_flags["asimdhp"] = (bool)f.asimdhp;
	feature_flags["asimdrdm"] = (bool)f.asimdrdm;
	feature_flags["jscvt"] = (bool)f.jscvt;
	feature_flags["fcma"] = (bool)f.fcma;
	feature_flags["lrcpc"] = (bool)f.lrcpc;
	feature_flags["dcpop"] = (bool)f.dcpop;
	feature_flags["sha3"] = (bool)f.sha3;
	feature_flags["sm3"] = (bool)f.sm3;
	feature_flags["sm4"] = (bool)f.sm4;
	feature_flags["asimddp"] = (bool)f.asimddp;
	feature_flags["sha512"] = (bool)f.sha512;
	feature_flags["sve"] = (bool)f.sve;
	feature_flags["asimdfhm"] = (bool)f.asimdfhm;
	feature_flags["neon"] = (bool)f.asimd;

	detected = true;
#endif
}

void CpuFeatures::detect() {
	if (detected) {
		return;
	}

#ifdef GD_CPU_ARCH_X86
	detect_x86();
#elif defined(CPU_FEATURES_ARCH_AARCH64) || defined(ARCH_ARM64) || defined(TARGET_IOS_SIMULATOR)
	detect_aarch64();
#endif
}

// ── Getters ─────────────────────────────────────────────────────────────────

String CpuFeatures::get_cpu_name() const {
	return cpu_name;
}

String CpuFeatures::get_vendor() const {
	return vendor;
}

int CpuFeatures::get_simd_level() const {
	return simd_level;
}

String CpuFeatures::get_simd_name() const {
	switch (simd_level) {
		case SIMD_SSE3:
			return "SSE3";
		case SIMD_SSE4_1:
			return "SSE4.1";
		case SIMD_SSE4_2:
			return "SSE4.2";
		case SIMD_AVX:
			return "AVX";
		case SIMD_AVX2:
			return "AVX2";
		case SIMD_NEON:
			return "NEON";
		default:
			return "None";
	}
}

int CpuFeatures::get_architecture() const {
	return architecture;
}

String CpuFeatures::get_architecture_name() const {
	switch (architecture) {
		case ARCH_X86:
			return "x86";
		case ARCH_X86_64:
			return "x86_64";
		case ARCH_ARM:
			return "ARM";
		case ARCH_AARCH64:
			return "AArch64";
		default:
			return "Unknown";
	}
}

String CpuFeatures::get_microarchitecture() const {
	return microarchitecture_name;
}

Dictionary CpuFeatures::get_features() const {
	return feature_flags;
}

bool CpuFeatures::has_feature(const String &feature) const {
	if (feature_flags.has(feature)) {
		return feature_flags[feature];
	}
	return false;
}

// ── Convenience ─────────────────────────────────────────────────────────────

bool CpuFeatures::has_sse3() const { return has_feature("sse3"); }
bool CpuFeatures::has_sse4_1() const { return has_feature("sse4_1"); }
bool CpuFeatures::has_sse4_2() const { return has_feature("sse4_2"); }
bool CpuFeatures::has_avx() const { return has_feature("avx"); }
bool CpuFeatures::has_avx2() const { return has_feature("avx2"); }
bool CpuFeatures::has_neon() const { return has_feature("neon") || has_feature("asimd"); }
bool CpuFeatures::has_aes() const { return has_feature("aes"); }
bool CpuFeatures::has_sha() const { return has_feature("sha") || has_feature("sha1") || has_feature("sha2"); }

// ── Summary ─────────────────────────────────────────────────────────────────

Dictionary CpuFeatures::get_info() const {
	Dictionary d;
	d["cpu_name"] = cpu_name;
	d["vendor"] = vendor;
	d["architecture"] = get_architecture_name();
	d["microarchitecture"] = microarchitecture_name;
	d["simd"] = get_simd_name();
	d["simd_level"] = simd_level;
	d["features"] = feature_flags;
	d["detected"] = detected;
	return d;
}

String CpuFeatures::get_summary() const {
	String s;
	s += "CPU: " + cpu_name + "\n";
	if (!vendor.empty()) {
		s += "Vendor: " + vendor + "\n";
	}
	s += "Architecture: " + get_architecture_name() + "\n";
	if (!microarchitecture_name.empty()) {
		s += "Microarchitecture: " + microarchitecture_name + "\n";
	}
	s += "SIMD: " + get_simd_name() + "\n";

	// List enabled features
	Array keys = feature_flags.keys();
	keys.sort();
	String enabled;
	for (int i = 0; i < keys.size(); i++) {
		if ((bool)feature_flags[keys[i]]) {
			if (!enabled.empty()) {
				enabled += ", ";
			}
			enabled += String(keys[i]);
		}
	}
	if (!enabled.empty()) {
		s += "Features: " + enabled + "\n";
	}
	return s;
}

// ── Bind ────────────────────────────────────────────────────────────────────

void CpuFeatures::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_cpu_name"), &CpuFeatures::get_cpu_name);
	ClassDB::bind_method(D_METHOD("get_vendor"), &CpuFeatures::get_vendor);
	ClassDB::bind_method(D_METHOD("get_simd_level"), &CpuFeatures::get_simd_level);
	ClassDB::bind_method(D_METHOD("get_simd_name"), &CpuFeatures::get_simd_name);
	ClassDB::bind_method(D_METHOD("get_architecture"), &CpuFeatures::get_architecture);
	ClassDB::bind_method(D_METHOD("get_architecture_name"), &CpuFeatures::get_architecture_name);
	ClassDB::bind_method(D_METHOD("get_microarchitecture"), &CpuFeatures::get_microarchitecture);
	ClassDB::bind_method(D_METHOD("get_features"), &CpuFeatures::get_features);
	ClassDB::bind_method(D_METHOD("has_feature", "feature"), &CpuFeatures::has_feature);
	ClassDB::bind_method(D_METHOD("has_sse3"), &CpuFeatures::has_sse3);
	ClassDB::bind_method(D_METHOD("has_sse4_1"), &CpuFeatures::has_sse4_1);
	ClassDB::bind_method(D_METHOD("has_sse4_2"), &CpuFeatures::has_sse4_2);
	ClassDB::bind_method(D_METHOD("has_avx"), &CpuFeatures::has_avx);
	ClassDB::bind_method(D_METHOD("has_avx2"), &CpuFeatures::has_avx2);
	ClassDB::bind_method(D_METHOD("has_neon"), &CpuFeatures::has_neon);
	ClassDB::bind_method(D_METHOD("has_aes"), &CpuFeatures::has_aes);
	ClassDB::bind_method(D_METHOD("has_sha"), &CpuFeatures::has_sha);
	ClassDB::bind_method(D_METHOD("get_info"), &CpuFeatures::get_info);
	ClassDB::bind_method(D_METHOD("get_summary"), &CpuFeatures::get_summary);

	BIND_ENUM_CONSTANT(SIMD_NONE);
	BIND_ENUM_CONSTANT(SIMD_SSE3);
	BIND_ENUM_CONSTANT(SIMD_SSE4_1);
	BIND_ENUM_CONSTANT(SIMD_SSE4_2);
	BIND_ENUM_CONSTANT(SIMD_AVX);
	BIND_ENUM_CONSTANT(SIMD_AVX2);
	BIND_ENUM_CONSTANT(SIMD_NEON);

	BIND_ENUM_CONSTANT(ARCH_UNKNOWN);
	BIND_ENUM_CONSTANT(ARCH_X86);
	BIND_ENUM_CONSTANT(ARCH_X86_64);
	BIND_ENUM_CONSTANT(ARCH_ARM);
	BIND_ENUM_CONSTANT(ARCH_AARCH64);
}

// ── Constructor ─────────────────────────────────────────────────────────────

CpuFeatures::CpuFeatures() :
		detected(false),
		simd_level(SIMD_NONE),
		architecture(ARCH_UNKNOWN) {
	detect();
}

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_SUITE("[[cpufeatures]] CpuFeatures") {
	TEST_CASE("constructor detects CPU") {
		CpuFeatures cpu;
		Dictionary info = cpu.get_info();
		CHECK((bool)info["detected"] == true);
	}

	TEST_CASE("cpu_name is non-empty") {
		CpuFeatures cpu;
		CHECK(!cpu.get_cpu_name().empty());
	}

	TEST_CASE("architecture is valid") {
		CpuFeatures cpu;
		int arch = cpu.get_architecture();
		CHECK(arch != CpuFeatures::ARCH_UNKNOWN);
		String arch_name = cpu.get_architecture_name();
		CHECK(!arch_name.empty());
		CHECK(arch_name != "Unknown");
	}

	TEST_CASE("architecture matches platform") {
#if defined(__x86_64__) || defined(_M_X64)
		CpuFeatures cpu;
		CHECK(cpu.get_architecture() == CpuFeatures::ARCH_X86_64);
		CHECK(cpu.get_architecture_name() == "x86_64");
#elif defined(__i386__) || defined(_M_IX86)
		CpuFeatures cpu;
		CHECK(cpu.get_architecture() == CpuFeatures::ARCH_X86);
		CHECK(cpu.get_architecture_name() == "x86");
#elif defined(__aarch64__) || defined(_M_ARM64)
		CpuFeatures cpu;
		CHECK(cpu.get_architecture() == CpuFeatures::ARCH_AARCH64);
		CHECK(cpu.get_architecture_name() == "AArch64");
#endif
	}

	TEST_CASE("simd_level is valid") {
		CpuFeatures cpu;
		int level = cpu.get_simd_level();
		CHECK(level >= CpuFeatures::SIMD_NONE);
		CHECK(level <= CpuFeatures::SIMD_NEON);
	}

	TEST_CASE("simd_name matches level") {
		CpuFeatures cpu;
		String name = cpu.get_simd_name();
		CHECK(!name.empty());
		int level = cpu.get_simd_level();
		if (level == CpuFeatures::SIMD_AVX2) {
			CHECK(name == "AVX2");
		} else if (level == CpuFeatures::SIMD_NEON) {
			CHECK(name == "NEON");
		} else if (level == CpuFeatures::SIMD_NONE) {
			CHECK(name == "None");
		}
	}

	TEST_CASE("features dictionary is populated") {
		CpuFeatures cpu;
		Dictionary features = cpu.get_features();
		CHECK(features.size() > 0);
	}

	TEST_CASE("feature values are boolean") {
		CpuFeatures cpu;
		Dictionary features = cpu.get_features();
		Array keys = features.keys();
		for (int i = 0; i < keys.size(); i++) {
			Variant v = features[keys[i]];
			CHECK(v.get_type() == Variant::BOOL);
		}
	}

	TEST_CASE("has_feature returns false for nonexistent") {
		CpuFeatures cpu;
		CHECK(cpu.has_feature("nonexistent_feature_xyz") == false);
	}

	TEST_CASE("x86 feature detection") {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
		CpuFeatures cpu;
		// x86_64 always has SSE2
		Dictionary features = cpu.get_features();
		CHECK(features.has("sse"));
		CHECK(features.has("sse2"));
		CHECK(features.has("avx"));
		CHECK(features.has("avx2"));
		CHECK(features.has("aes"));
		CHECK(features.has("popcnt"));
#endif
	}

	TEST_CASE("x86 vendor is set") {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
		CpuFeatures cpu;
		String vendor = cpu.get_vendor();
		CHECK(!vendor.empty());
		// Should be one of the known vendors
		bool known = vendor == "GenuineIntel" || vendor == "AuthenticAMD" || vendor == "HygonGenuine";
		CHECK(known);
#endif
	}

	TEST_CASE("x86 microarchitecture is set") {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
		CpuFeatures cpu;
		String uarch = cpu.get_microarchitecture();
		CHECK(!uarch.empty());
#endif
	}

	TEST_CASE("x86 simd hierarchy is consistent") {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
		CpuFeatures cpu;
		// AVX2 implies AVX implies SSE4.2 etc.
		if (cpu.has_avx2()) {
			CHECK(cpu.has_avx());
		}
		if (cpu.has_avx()) {
			CHECK(cpu.has_sse4_2());
		}
		if (cpu.has_sse4_2()) {
			CHECK(cpu.has_sse4_1());
		}
		if (cpu.has_sse4_1()) {
			CHECK(cpu.has_sse3());
		}
#endif
	}

	TEST_CASE("aarch64 feature detection") {
#if defined(__aarch64__) || defined(_M_ARM64)
		CpuFeatures cpu;
		Dictionary features = cpu.get_features();
		CHECK(features.has("asimd"));
		CHECK(features.has("fp"));
		CHECK(features.has("aes"));
		CHECK(features.has("crc32"));
		CHECK(features.has("neon"));
		// NEON is always present on AArch64
		CHECK(cpu.has_neon());
#endif
	}

	TEST_CASE("aarch64 simd level is NEON") {
#if defined(__aarch64__) || defined(_M_ARM64)
		CpuFeatures cpu;
		CHECK(cpu.get_simd_level() == CpuFeatures::SIMD_NEON);
		CHECK(cpu.get_simd_name() == "NEON");
#endif
	}

	TEST_CASE("convenience has_* methods match features dict") {
		CpuFeatures cpu;
		Dictionary features = cpu.get_features();

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
		if (features.has("sse3")) {
			CHECK(cpu.has_sse3() == (bool)features["sse3"]);
		}
		if (features.has("sse4_1")) {
			CHECK(cpu.has_sse4_1() == (bool)features["sse4_1"]);
		}
		if (features.has("avx")) {
			CHECK(cpu.has_avx() == (bool)features["avx"]);
		}
		if (features.has("aes")) {
			CHECK(cpu.has_aes() == (bool)features["aes"]);
		}
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
		CHECK(cpu.has_neon() == true);
		if (features.has("aes")) {
			CHECK(cpu.has_aes() == (bool)features["aes"]);
		}
#endif
	}

	TEST_CASE("get_info returns complete dictionary") {
		CpuFeatures cpu;
		Dictionary info = cpu.get_info();
		CHECK(info.has("cpu_name"));
		CHECK(info.has("vendor"));
		CHECK(info.has("architecture"));
		CHECK(info.has("microarchitecture"));
		CHECK(info.has("simd"));
		CHECK(info.has("simd_level"));
		CHECK(info.has("features"));
		CHECK(info.has("detected"));
	}

	TEST_CASE("get_summary returns non-empty string") {
		CpuFeatures cpu;
		String summary = cpu.get_summary();
		CHECK(!summary.empty());
		CHECK(summary.find("CPU:") >= 0);
		CHECK(summary.find("Architecture:") >= 0);
		CHECK(summary.find("SIMD:") >= 0);
	}

	TEST_CASE("summary contains cpu name") {
		CpuFeatures cpu;
		String summary = cpu.get_summary();
		String name = cpu.get_cpu_name();
		if (!name.empty()) {
			CHECK(summary.find(name) >= 0);
		}
	}

	TEST_CASE("multiple instances return consistent results") {
		CpuFeatures a;
		CpuFeatures b;
		CHECK(a.get_cpu_name() == b.get_cpu_name());
		CHECK(a.get_simd_level() == b.get_simd_level());
		CHECK(a.get_architecture() == b.get_architecture());
		CHECK(a.get_vendor() == b.get_vendor());
	}

	TEST_CASE("feature keys are lowercase strings") {
		CpuFeatures cpu;
		Dictionary features = cpu.get_features();
		Array keys = features.keys();
		for (int i = 0; i < keys.size(); i++) {
			String key = keys[i];
			CHECK(key == key.to_lower());
			CHECK(!key.empty());
		}
	}
}

#endif // DOCTEST
