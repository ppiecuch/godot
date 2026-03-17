// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include "hwinfo/platform.h"

#ifdef HWINFO_APPLE

#include <mach/mach.h>
#include <mach/mach_time.h>
#include <math.h>
#include <pthread.h>
#include <sys/sysctl.h>

#include <algorithm>
#include <string>
#include <vector>

#include "hwinfo/cpu.h"
#include "hwinfo/cpuid.h"
#include "hwinfo/utils/stringutils.h"

namespace hwinfo {

// Helper: read a sysctl string
static std::string sysctl_string(const char *key) {
  char buf[512] = {};
  size_t len = sizeof(buf);
  if (sysctlbyname(key, buf, &len, nullptr, 0) == 0 && len > 0) {
    return std::string(buf, len - 1);  // trim null
  }
  return "";
}

// Helper: read a sysctl int64
static int64_t sysctl_i64(const char *key, int64_t fallback = -1) {
  int64_t val = 0;
  size_t len = sizeof(val);
  if (sysctlbyname(key, &val, &len, nullptr, 0) == 0) {
    return val;
  }
  // Try 32-bit
  int32_t val32 = 0;
  len = sizeof(val32);
  if (sysctlbyname(key, &val32, &len, nullptr, 0) == 0) {
    return val32;
  }
  return fallback;
}

// Helper: check if a sysctl hw.optional key is set
static bool sysctl_hw_optional(const char *feature) {
  std::string key = std::string("hw.optional.") + feature;
  int val = 0;
  size_t len = sizeof(val);
  if (sysctlbyname(key.c_str(), &val, &len, nullptr, 0) == 0) {
    return val != 0;
  }
  return false;
}

static std::string get_vendor() {
#if defined(HWINFO_X86)
  uint32_t regs[4]{0};
  cpuid::cpuid(0, 0, regs);
  std::string vendor;
  vendor += std::string((const char*)&regs[1], 4);
  vendor += std::string((const char*)&regs[3], 4);
  vendor += std::string((const char*)&regs[2], 4);
  return vendor;
#else
  // All ARM Macs are Apple Silicon
  return "Apple";
#endif
}

static std::string get_model_name() {
#ifdef HWINFO_X86
  std::string model;
  uint32_t regs[4]{};
  for (unsigned i = 0x80000002; i < 0x80000005; ++i) {
    cpuid::cpuid(i, 0, regs);
    for (int r = 0; r < 4; r++) {
      for (auto c : std::string((const char*)&regs[r], 4)) {
        if (std::isalnum(c) || c == '(' || c == ')' || c == '@' || c == ' ' || c == '-' || c == '.') {
          model += c;
        }
      }
    }
  }
  return model;
#else
  // ARM: use machdep.cpu.brand_string or hw.model
  size_t size = 256;
  std::string model;
  model.resize(size);
  if (sysctlbyname("machdep.cpu.brand_string", &model.front(), &size, nullptr, 0) == 0) {
    model.resize(size > 0 ? size - 1 : 0);
    return model;
  }
  // Fallback to hw.model (e.g., "MacBookPro18,1")
  model.resize(256);
  size = 256;
  if (sysctlbyname("hw.model", &model.front(), &size, nullptr, 0) == 0) {
    model.resize(size > 0 ? size - 1 : 0);
    return model;
  }
  return "<unknown>";
#endif // HWINFO_X86
}

static int get_num_logical_cores() {
  int logical = 0;
  size_t sz = sizeof(logical);
  if (sysctlbyname("hw.logicalcpu", &logical, &sz, nullptr, 0) == 0) {
    return logical;
  }
#if defined(HWINFO_X86)
  // x86 fallback via CPUID
  std::string vendor_id = get_vendor();
  std::for_each(vendor_id.begin(), vendor_id.end(), [](char& in) { in = ::toupper(in); });
  uint32_t regs[4]{};
  cpuid::cpuid(0, 0, regs);
  uint32_t HFS = regs[0];
  if (vendor_id.find("INTEL") != std::string::npos && HFS >= 0xb) {
    for (int lvl = 0; lvl < MAX_INTEL_TOP_LVL; ++lvl) {
      uint32_t regs_2[4]{};
      cpuid::cpuid(0x0b, lvl, regs_2);
      uint32_t currLevel = (LVL_TYPE & regs_2[2]) >> 8;
      if (currLevel == 0x02) {
        return static_cast<int>(LVL_CORES & regs_2[1]);
      }
    }
  } else if (vendor_id.find("AMD") != std::string::npos && HFS > 0) {
    cpuid::cpuid(1, 0, regs);
    return static_cast<int>(regs[1] >> 16) & 0xff;
  }
#endif
  return -1;
}

static int get_num_physical_cores() {
  int physical = 0;
  size_t sz = sizeof(physical);
  if (sysctlbyname("hw.physicalcpu", &physical, &sz, nullptr, 0) == 0) {
    return physical;
  }
  return -1;
}

static std::vector<std::string> get_cpu_flags() {
  std::vector<std::string> flags;
#if defined(HWINFO_X86)
  // x86: read machdep.cpu.features + machdep.cpu.leaf7_features
  std::string feat = sysctl_string("machdep.cpu.features");
  if (!feat.empty()) {
    // Split by space
    std::string token;
    for (char c : feat) {
      if (c == ' ') {
        if (!token.empty()) { flags.push_back(token); token.clear(); }
      } else {
        token += c;
      }
    }
    if (!token.empty()) flags.push_back(token);
  }
  std::string leaf7 = sysctl_string("machdep.cpu.leaf7_features");
  if (!leaf7.empty()) {
    std::string token;
    for (char c : leaf7) {
      if (c == ' ') {
        if (!token.empty()) { flags.push_back(token); token.clear(); }
      } else {
        token += c;
      }
    }
    if (!token.empty()) flags.push_back(token);
  }
#else
  // ARM: probe hw.optional.* features
  static const char *arm_features[] = {
    "neon", "neon_hpfp", "neon_fp16",
    "arm64", "armv8_crc32", "armv8_2_sha512", "armv8_2_sha3",
    "armv8_3_compnum",
    "AdvSIMD", "AdvSIMD_HPFPCvt",
    "ucnormal_mem", "arm_FEAT_FlagM", "arm_FEAT_FlagM2",
    "arm_FEAT_FHM", "arm_FEAT_DotProd", "arm_FEAT_SHA3",
    "arm_FEAT_RDM", "arm_FEAT_LSE", "arm_FEAT_SHA256",
    "arm_FEAT_SHA512", "arm_FEAT_SHA1",
    "arm_FEAT_AES", "arm_FEAT_PMULL",
    "arm_FEAT_SPECRES", "arm_FEAT_SB", "arm_FEAT_FRINTTS",
    "arm_FEAT_LRCPC", "arm_FEAT_LRCPC2",
    "arm_FEAT_FCMA", "arm_FEAT_JSCVT",
    "arm_FEAT_DIT", "arm_FEAT_DPB", "arm_FEAT_DPB2",
    "arm_FEAT_BF16", "arm_FEAT_I8MM",
    "arm_FEAT_BTI", "arm_FEAT_SSBS",
    "amx_version",
    nullptr
  };
  for (int i = 0; arm_features[i]; i++) {
    if (sysctl_hw_optional(arm_features[i])) {
      flags.push_back(arm_features[i]);
    }
  }
#endif
  return flags;
}

// _____________________________________________________________________________________________________________________
int64_t CPU::currentClockSpeed_MHz() const {
  // macOS doesn't reliably expose current frequency on Apple Silicon.
  // On x86, hw.cpufrequency gives current. On ARM, fall back to max.
  int64_t freq = sysctl_i64("hw.cpufrequency", -1);
  if (freq > 0) return freq / 1000000;
  return _maxClockSpeed_MHz > 0 ? _maxClockSpeed_MHz : _regularClockSpeed_MHz;
}

// =====================================================================================================================
// _____________________________________________________________________________________________________________________
std::vector<Socket> getAllSockets() {
  std::vector<Socket> sockets;
  CPU cpu;
  cpu._vendor = get_vendor();
  cpu._modelName = get_model_name();
  cpu._numPhysicalCores = get_num_physical_cores();
  cpu._numLogicalCores = get_num_logical_cores();

  // Frequencies (in Hz from sysctl)
  int64_t freq = sysctl_i64("hw.cpufrequency", -1);
  int64_t freq_max = sysctl_i64("hw.cpufrequency_max", freq);
  int64_t freq_min = sysctl_i64("hw.cpufrequency_min", freq);

  cpu._regularClockSpeed_MHz = freq > 0 ? freq / 1000000 : -1;
  cpu._maxClockSpeed_MHz = freq_max > 0 ? freq_max / 1000000 : cpu._regularClockSpeed_MHz;
  cpu._minClockSpeed_MHz = freq_min > 0 ? freq_min / 1000000 : cpu._regularClockSpeed_MHz;

  // Cache sizes — try L2 first (most commonly populated), then L3
  int64_t l2 = sysctl_i64("hw.l2cachesize", -1);
  int64_t l3 = sysctl_i64("hw.l3cachesize", -1);
  cpu._cacheSize_Bytes = l2 > 0 ? l2 : (l3 > 0 ? l3 : -1);

  // CPU feature flags
  cpu._flags = get_cpu_flags();

  sockets.push_back(Socket(cpu));
  return sockets;
}

}  // namespace hwinfo

#endif  // HWINFO_APPLE
