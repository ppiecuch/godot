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

#include "cpu_features_macros.h"

#ifdef CPU_FEATURES_ARCH_ARM
#if defined(CPU_FEATURES_OS_LINUX) || defined(ANDROID)

#include "cpuinfo_arm.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions for introspection.
////////////////////////////////////////////////////////////////////////////////
#define INTROSPECTION_TABLE                                        \
  LINE(ARM_SWP, swp, "swp", ARM_HWCAP_SWP, 0)                      \
  LINE(ARM_HALF, half, "half", ARM_HWCAP_HALF, 0)                  \
  LINE(ARM_THUMB, thumb, "thumb", ARM_HWCAP_THUMB, 0)              \
  LINE(ARM_26BIT, _26bit, "26bit", ARM_HWCAP_26BIT, 0)             \
  LINE(ARM_FASTMULT, fastmult, "fastmult", ARM_HWCAP_FAST_MULT, 0) \
  LINE(ARM_FPA, fpa, "fpa", ARM_HWCAP_FPA, 0)                      \
  LINE(ARM_VFP, vfp, "vfp", ARM_HWCAP_VFP, 0)                      \
  LINE(ARM_EDSP, edsp, "edsp", ARM_HWCAP_EDSP, 0)                  \
  LINE(ARM_JAVA, java, "java", ARM_HWCAP_JAVA, 0)                  \
  LINE(ARM_IWMMXT, iwmmxt, "iwmmxt", ARM_HWCAP_IWMMXT, 0)          \
  LINE(ARM_CRUNCH, crunch, "crunch", ARM_HWCAP_CRUNCH, 0)          \
  LINE(ARM_THUMBEE, thumbee, "thumbee", ARM_HWCAP_THUMBEE, 0)      \
  LINE(ARM_NEON, neon, "neon", ARM_HWCAP_NEON, 0)                  \
  LINE(ARM_VFPV3, vfpv3, "vfpv3", ARM_HWCAP_VFPV3, 0)              \
  LINE(ARM_VFPV3D16, vfpv3d16, "vfpv3d16", ARM_HWCAP_VFPV3D16, 0)  \
  LINE(ARM_TLS, tls, "tls", ARM_HWCAP_TLS, 0)                      \
  LINE(ARM_VFPV4, vfpv4, "vfpv4", ARM_HWCAP_VFPV4, 0)              \
  LINE(ARM_IDIVA, idiva, "idiva", ARM_HWCAP_IDIVA, 0)              \
  LINE(ARM_IDIVT, idivt, "idivt", ARM_HWCAP_IDIVT, 0)              \
  LINE(ARM_VFPD32, vfpd32, "vfpd32", ARM_HWCAP_VFPD32, 0)          \
  LINE(ARM_LPAE, lpae, "lpae", ARM_HWCAP_LPAE, 0)                  \
  LINE(ARM_EVTSTRM, evtstrm, "evtstrm", ARM_HWCAP_EVTSTRM, 0)      \
  LINE(ARM_AES, aes, "aes", 0, ARM_HWCAP2_AES)                     \
  LINE(ARM_PMULL, pmull, "pmull", 0, ARM_HWCAP2_PMULL)             \
  LINE(ARM_SHA1, sha1, "sha1", 0, ARM_HWCAP2_SHA1)                 \
  LINE(ARM_SHA2, sha2, "sha2", 0, ARM_HWCAP2_SHA2)                 \
  LINE(ARM_CRC32, crc32, "crc32", 0, ARM_HWCAP2_CRC32)
#define INTROSPECTION_PREFIX Arm
#define INTROSPECTION_ENUM_PREFIX ARM
#include "define_introspection_and_hwcaps.inl"

////////////////////////////////////////////////////////////////////////////////
// Implementation.
////////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>
#include <string.h>

#include "bit_utils.h"

uint32_t GetArmCpuId(const ArmInfo* const info) {
  return (ExtractBitRange(info->implementer, 7, 0) << 24) |
         (ExtractBitRange(info->variant, 3, 0) << 20) |
         (ExtractBitRange(info->part, 11, 0) << 4) |
         (ExtractBitRange(info->revision, 3, 0) << 0);
}

static void FixErrors(ArmInfo* const info) {
  // Propagate cpu features.
  if (info->features.vfpv4) info->features.vfpv3 = true;
  if (info->features.neon) info->features.vfpv3 = true;
  if (info->features.vfpv3) info->features.vfp = true;
}

static const ArmInfo kEmptyArmInfo;

ArmInfo GetArmInfo(void) {
  ArmInfo info = kEmptyArmInfo;

  const HardwareCapabilities hwcaps = CpuFeatures_GetHardwareCapabilities();
  // if both hwcaps are zeros then GetArmInfo failed
  for (size_t i = 0; i < ARM_LAST_; ++i) {
    if (CpuFeatures_IsHwCapsSet(kHardwareCapabilities[i], hwcaps)) {
      kSetters[i](&info.features, true);
    }
  }

  FixErrors(&info);

  return info;
}

#endif  // defined(CPU_FEATURES_OS_LINUX) || defined(ANDROID)
#endif  // CPU_FEATURES_ARCH_ARM
