// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include "hwinfo/platform.h"

#ifdef HWINFO_APPLE

#include "TargetConditionals.h"

#include <sys/sysctl.h>

#include <sstream>
#include <string>

#include "hwinfo/os.h"
#include "hwinfo/utils/stringutils.h"

namespace hwinfo {

// _____________________________________________________________________________________________________________________
static int _get_os_build_version() {
#if defined(__WATCH_OS_VERSION_MIN_REQUIRED)
    return __WATCH_OS_VERSION_MIN_REQUIRED;
#elif defined(__TV_OS_VERSION_MIN_REQUIRED)
    return __TV_OS_VERSION_MIN_REQUIRED;
#elif defined(__IPHONE_OS_VERSION_MIN_REQUIRED)
    return __IPHONE_OS_VERSION_MIN_REQUIRED;
#elif defined(__MAC_OS_X_VERSION_MIN_REQUIRED)
    return __MAC_OS_X_VERSION_MIN_REQUIRED;
#endif
  return 0;
}

static const char *_get_os_target() {
#if TARGET_OS_WATCH
    return "watchOS";
#elif TARGET_OS_TV
    return "tvOS";
#elif TARGET_OS_IPHONE
    return "iOS";
#elif TARGET_IPHONE_SIMULATOR
    return "iOS (Simulator)";
#elif TARGET_OS_MAC || TARGET_OS_MACCATALYST || TARGET_OS_SIMULATOR
    return "macOS";
#endif
  return nullptr;
}

static const char *_get_os_name(int major, int minor) {
  if (major == 14)
    return "Sonoma";
  if (major == 13)
    return "Ventura";
  if (major == 12)
    return "Monterey";
  if (major == 11)
    return "Big Sur";
  if (major == 10) {
    switch (minor) {
      case 9:
        return "Mavericks";
      case 10:
        return "Yosemite";
      case 11:
        return "El Capitan";
      case 12:
        return "Sierra";
      case 13:
        return "High Sierra";
      case 14:
        return "Mojave";
      case 15:
        return "Catalina";
      case 16:
        return "Big Sur";
    }
  }
  return nullptr;
}

// _____________________________________________________________________________________________________________________
OS::OS() {
  _32bit = false;
  _64bit = true;

  _name = "macOS";
  _fullName = "macOS <unknown version>";

  auto _get_sys_prop = [](const char *key) -> char * {
    size_t size = 512;
    static char prop[512+1] = { 0 };
    if (sysctlbyname(key, prop, &size, nullptr, 0) == 0) {
      prop[std::min(size, size_t(512))] = 0;
      return prop;
    }
    return nullptr;
  };

  if (const char *prop = _get_sys_prop("kern.osproductversion")) {
    _version = prop;
    if (char *dot = (char*)strchr(prop, '.')) {
      *dot = 0;
      if (const char *os = _get_os_name(atoi(prop), atoi(dot + 1))) {
        _fullName = "macOS " + std::string(os);
      }
    }
  }

  if (const char *prop = _get_sys_prop("kern.osrelease")) {
    _kernel = prop;
  }

  if (const char *prop = _get_sys_prop("kern.ostype")) {
    _details += "OS type: " + std::string(prop) + "\n";
  }
  if (const char *prop = _get_os_target()) {
    _details += "OS family: " + std::string(prop) + "\n";
  }
  if (const char *prop = _get_sys_prop("kern.osrevision")) {
    _details += "OS revision: " + std::string(prop) + "\n";
  }
  if (int version = _get_os_build_version()) {
    _details += "OS build version: " + utils::to_string(version / 10000) + "." + utils::to_string(version / 100 % 100) + "." + utils::to_string(version % 100) + "\n";
  }
}

}  // namespace hwinfo

#endif  // HWINFO_APPLE
