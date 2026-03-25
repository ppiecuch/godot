// Copyright 2017 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// AArch64 implementation for Apple platforms (macOS, iOS, tvOS, watchOS).
// Uses sysctlbyname() to query individual CPU features.
// Based on upstream google/cpu_features impl_aarch64_macos_or_iphone.c

#include "cpu_features_macros.h"

#if (defined(CPU_FEATURES_ARCH_AARCH64) && (defined(__APPLE__) || defined(__MACH__))) || defined(TARGET_IOS_SIMULATOR)

#include "cpuinfo_aarch64.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions for introspection.
////////////////////////////////////////////////////////////////////////////////
#define INTROSPECTION_TABLE                                                \
  LINE(AARCH64_FP, fp, "fp", AARCH64_HWCAP_FP, 0)                          \
  LINE(AARCH64_ASIMD, asimd, "asimd", AARCH64_HWCAP_ASIMD, 0)              \
  LINE(AARCH64_EVTSTRM, evtstrm, "evtstrm", AARCH64_HWCAP_EVTSTRM, 0)      \
  LINE(AARCH64_AES, aes, "aes", AARCH64_HWCAP_AES, 0)                      \
  LINE(AARCH64_PMULL, pmull, "pmull", AARCH64_HWCAP_PMULL, 0)              \
  LINE(AARCH64_SHA1, sha1, "sha1", AARCH64_HWCAP_SHA1, 0)                  \
  LINE(AARCH64_SHA2, sha2, "sha2", AARCH64_HWCAP_SHA2, 0)                  \
  LINE(AARCH64_CRC32, crc32, "crc32", AARCH64_HWCAP_CRC32, 0)              \
  LINE(AARCH64_ATOMICS, atomics, "atomics", AARCH64_HWCAP_ATOMICS, 0)      \
  LINE(AARCH64_FPHP, fphp, "fphp", AARCH64_HWCAP_FPHP, 0)                  \
  LINE(AARCH64_ASIMDHP, asimdhp, "asimdhp", AARCH64_HWCAP_ASIMDHP, 0)      \
  LINE(AARCH64_CPUID, cpuid, "cpuid", AARCH64_HWCAP_CPUID, 0)              \
  LINE(AARCH64_ASIMDRDM, asimdrdm, "asimdrdm", AARCH64_HWCAP_ASIMDRDM, 0)  \
  LINE(AARCH64_JSCVT, jscvt, "jscvt", AARCH64_HWCAP_JSCVT, 0)              \
  LINE(AARCH64_FCMA, fcma, "fcma", AARCH64_HWCAP_FCMA, 0)                  \
  LINE(AARCH64_LRCPC, lrcpc, "lrcpc", AARCH64_HWCAP_LRCPC, 0)              \
  LINE(AARCH64_DCPOP, dcpop, "dcpop", AARCH64_HWCAP_DCPOP, 0)              \
  LINE(AARCH64_SHA3, sha3, "sha3", AARCH64_HWCAP_SHA3, 0)                  \
  LINE(AARCH64_SM3, sm3, "sm3", AARCH64_HWCAP_SM3, 0)                      \
  LINE(AARCH64_SM4, sm4, "sm4", AARCH64_HWCAP_SM4, 0)                      \
  LINE(AARCH64_ASIMDDP, asimddp, "asimddp", AARCH64_HWCAP_ASIMDDP, 0)      \
  LINE(AARCH64_SHA512, sha512, "sha512", AARCH64_HWCAP_SHA512, 0)          \
  LINE(AARCH64_SVE, sve, "sve", AARCH64_HWCAP_SVE, 0)                      \
  LINE(AARCH64_ASIMDFHM, asimdfhm, "asimdfhm", AARCH64_HWCAP_ASIMDFHM, 0)  \
  LINE(AARCH64_DIT, dit, "dit", AARCH64_HWCAP_DIT, 0)                      \
  LINE(AARCH64_USCAT, uscat, "uscat", AARCH64_HWCAP_USCAT, 0)              \
  LINE(AARCH64_ILRCPC, ilrcpc, "ilrcpc", AARCH64_HWCAP_ILRCPC, 0)          \
  LINE(AARCH64_FLAGM, flagm, "flagm", AARCH64_HWCAP_FLAGM, 0)              \
  LINE(AARCH64_SSBS, ssbs, "ssbs", AARCH64_HWCAP_SSBS, 0)                  \
  LINE(AARCH64_SB, sb, "sb", AARCH64_HWCAP_SB, 0)                          \
  LINE(AARCH64_PACA, paca, "paca", AARCH64_HWCAP_PACA, 0)                  \
  LINE(AARCH64_PACG, pacg, "pacg", AARCH64_HWCAP_PACG, 0)                  \
  LINE(AARCH64_DCPODP, dcpodp, "dcpodp", 0, AARCH64_HWCAP2_DCPODP)         \
  LINE(AARCH64_SVE2, sve2, "sve2", 0, AARCH64_HWCAP2_SVE2)                 \
  LINE(AARCH64_SVEAES, sveaes, "sveaes", 0, AARCH64_HWCAP2_SVEAES)         \
  LINE(AARCH64_SVEPMULL, svepmull, "svepmull", 0, AARCH64_HWCAP2_SVEPMULL) \
  LINE(AARCH64_SVEBITPERM, svebitperm, "svebitperm", 0,                    \
       AARCH64_HWCAP2_SVEBITPERM)                                          \
  LINE(AARCH64_SVESHA3, svesha3, "svesha3", 0, AARCH64_HWCAP2_SVESHA3)     \
  LINE(AARCH64_SVESM4, svesm4, "svesm4", 0, AARCH64_HWCAP2_SVESM4)         \
  LINE(AARCH64_FLAGM2, flagm2, "flagm2", 0, AARCH64_HWCAP2_FLAGM2)         \
  LINE(AARCH64_FRINT, frint, "frint", 0, AARCH64_HWCAP2_FRINT)             \
  LINE(AARCH64_SVEI8MM, svei8mm, "svei8mm", 0, AARCH64_HWCAP2_SVEI8MM)     \
  LINE(AARCH64_SVEF32MM, svef32mm, "svef32mm", 0, AARCH64_HWCAP2_SVEF32MM) \
  LINE(AARCH64_SVEF64MM, svef64mm, "svef64mm", 0, AARCH64_HWCAP2_SVEF64MM) \
  LINE(AARCH64_SVEBF16, svebf16, "svebf16", 0, AARCH64_HWCAP2_SVEBF16)     \
  LINE(AARCH64_I8MM, i8mm, "i8mm", 0, AARCH64_HWCAP2_I8MM)                 \
  LINE(AARCH64_BF16, bf16, "bf16", 0, AARCH64_HWCAP2_BF16)                 \
  LINE(AARCH64_DGH, dgh, "dgh", 0, AARCH64_HWCAP2_DGH)                     \
  LINE(AARCH64_RNG, rng, "rng", 0, AARCH64_HWCAP2_RNG)                     \
  LINE(AARCH64_BTI, bti, "bti", 0, AARCH64_HWCAP2_BTI)                     \
  LINE(AARCH64_MTE, mte, "mte", 0, AARCH64_HWCAP2_MTE)
#define INTROSPECTION_PREFIX Aarch64
#define INTROSPECTION_ENUM_PREFIX AARCH64
#include "define_introspection_and_hwcaps.inl"

////////////////////////////////////////////////////////////////////////////////
// Implementation — sysctlbyname-based feature detection for Apple platforms.
////////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <sys/sysctl.h>

static int GetDarwinSysCtlByNameValue(const char *name) {
	int enabled;
	size_t enabled_len = sizeof(enabled);
	const int failure = sysctlbyname(name, &enabled, &enabled_len, NULL, 0);
	return failure ? 0 : enabled;
}

static bool GetDarwinSysCtlByName(const char *name) {
	return GetDarwinSysCtlByNameValue(name) != 0;
}

static void FillCpuName(char *name, size_t name_len) {
	// Try hw.model first (e.g. "MacBookPro18,3" or "iPhone14,5")
	size_t size = name_len;
	if (sysctlbyname("hw.model", name, &size, NULL, 0) == 0 && size > 0) {
		return;
	}
	// Fallback to machdep.cpu.brand_string (macOS only, usually Intel)
	size = name_len;
	if (sysctlbyname("machdep.cpu.brand_string", name, &size, NULL, 0) == 0 && size > 0) {
		return;
	}
	// Last resort
	snprintf(name, name_len, "Apple ARM64");
}

bool GetAarch64Info(Aarch64Info *outInfo) {
	memset(outInfo, 0, sizeof(*outInfo));

	// CPU identification
	outInfo->implementer = GetDarwinSysCtlByNameValue("hw.cputype");
	outInfo->variant = GetDarwinSysCtlByNameValue("hw.cpusubtype");
	outInfo->part = GetDarwinSysCtlByNameValue("hw.cpufamily");
	outInfo->revision = GetDarwinSysCtlByNameValue("hw.cpusubfamily");

	// CPU name
	FillCpuName(outInfo->name, sizeof(outInfo->name));

	// Feature detection via sysctlbyname (from upstream google/cpu_features)
	outInfo->features.fp = GetDarwinSysCtlByName("hw.optional.floatingpoint");
	outInfo->features.asimd = GetDarwinSysCtlByName("hw.optional.AdvSIMD") ||
	                          GetDarwinSysCtlByName("hw.optional.arm.AdvSIMD");
	outInfo->features.aes = GetDarwinSysCtlByName("hw.optional.arm.FEAT_AES");
	outInfo->features.pmull = GetDarwinSysCtlByName("hw.optional.arm.FEAT_PMULL");
	outInfo->features.sha1 = GetDarwinSysCtlByName("hw.optional.arm.FEAT_SHA1");
	outInfo->features.sha2 = GetDarwinSysCtlByName("hw.optional.arm.FEAT_SHA256");
	outInfo->features.crc32 = GetDarwinSysCtlByName("hw.optional.armv8_crc32");
	outInfo->features.atomics = GetDarwinSysCtlByName("hw.optional.arm.FEAT_LSE");
	outInfo->features.fphp = GetDarwinSysCtlByName("hw.optional.arm.FEAT_FP16");
	outInfo->features.asimdhp = GetDarwinSysCtlByName("hw.optional.arm.AdvSIMD_HPFPCvt");
	outInfo->features.asimdrdm = GetDarwinSysCtlByName("hw.optional.arm.FEAT_RDM");
	outInfo->features.jscvt = GetDarwinSysCtlByName("hw.optional.arm.FEAT_JSCVT");
	outInfo->features.fcma = GetDarwinSysCtlByName("hw.optional.arm.FEAT_FCMA");
	outInfo->features.lrcpc = GetDarwinSysCtlByName("hw.optional.arm.FEAT_LRCPC");
	outInfo->features.dcpop = GetDarwinSysCtlByName("hw.optional.arm.FEAT_DPB");
	outInfo->features.sha3 = GetDarwinSysCtlByName("hw.optional.arm.FEAT_SHA3");
	outInfo->features.asimddp = GetDarwinSysCtlByName("hw.optional.arm.FEAT_DotProd");
	outInfo->features.sha512 = GetDarwinSysCtlByName("hw.optional.arm.FEAT_SHA512");
	outInfo->features.asimdfhm = GetDarwinSysCtlByName("hw.optional.arm.FEAT_FHM");
	outInfo->features.dit = GetDarwinSysCtlByName("hw.optional.arm.FEAT_DIT");
	outInfo->features.uscat = GetDarwinSysCtlByName("hw.optional.arm.FEAT_LSE2");
	outInfo->features.flagm = GetDarwinSysCtlByName("hw.optional.arm.FEAT_FlagM");
	outInfo->features.ssbs = GetDarwinSysCtlByName("hw.optional.arm.FEAT_SSBS");
	outInfo->features.sb = GetDarwinSysCtlByName("hw.optional.arm.FEAT_SB");
	outInfo->features.flagm2 = GetDarwinSysCtlByName("hw.optional.arm.FEAT_FlagM2");
	outInfo->features.frint = GetDarwinSysCtlByName("hw.optional.arm.FEAT_FRINTTS");
	outInfo->features.i8mm = GetDarwinSysCtlByName("hw.optional.arm.FEAT_I8MM");
	outInfo->features.bf16 = GetDarwinSysCtlByName("hw.optional.arm.FEAT_BF16");
	outInfo->features.bti = GetDarwinSysCtlByName("hw.optional.arm.FEAT_BTI");

	return true;
}

#endif
