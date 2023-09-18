// kate: replace-tabs on; tab-indents on; tab-width 2; indent-width 2; indent-mode cstyle;

// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <hwinfo/battery.h>
#include <hwinfo/cpu.h>
#include <hwinfo/disk.h>
#include <hwinfo/gpu.h>
#include <hwinfo/mainboard.h>
#include <hwinfo/os.h>
#include <hwinfo/ram.h>
#include <hwinfo/system.h>

#include <cstdint>
#include <string>
#include <functional>

namespace hwinfo {

  enum CustomProperty {
    UNDEFIEND,
    GPU_NUM_DEVICES,
    GPU_TEXTURE_MEMORY_MB,
    GPU_AVAILABLE_MEMORY_MB,
    GPU_NUM_VIRTUAL_SCREENS,
    GPU_SUMMARY_REPORT,
    SYS_PROCESS_REPORT,
  };

  union PropertyArg {
    int64_t i;
    const char *s;
    float f;
    PropertyArg() {}
    PropertyArg(int32_t i) : i(i) {}
    PropertyArg(int64_t i) : i(i) {}
    PropertyArg(size_t i) : i(i) {}
    PropertyArg(const char *s) : s(s) {}
    PropertyArg(float f) : f(f) {}
    operator int32_t() const { return i; }
    operator int64_t() const { return i; }
    operator float() const { return f; }
    operator const char *() const { return s; }
  };

  int64_t get_int_property(CustomProperty prop, bool *missing = nullptr);
  float get_float_property(CustomProperty prop, bool *missing = nullptr);
  std::string get_string_property(CustomProperty prop, bool *missing = nullptr);

  int64_t get_int_property(CustomProperty prop, PropertyArg arg, bool *missing = nullptr);
  float get_float_property(CustomProperty prop, PropertyArg arg, bool *missing = nullptr);
  std::string get_string_property(CustomProperty prop, PropertyArg arg, bool *missing = nullptr);

  typedef std::function<void(PropertyArg, int64_t*, float*, std::string*)> PropertyHandler;

  void register_property(CustomProperty prop, PropertyHandler func);

} // namespace

#ifdef __cplusplus
extern "C" {
#endif

void hwinfo_error(const char *p_format, ...);
void hwinfo_info(const char *p_format, ...);

#ifdef __cplusplus
}
#endif
