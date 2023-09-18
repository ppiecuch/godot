// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include "hwinfo/platform.h"

#ifdef HWINFO_APPLE

#include <TargetConditionals.h>

#include <sys/sysctl.h>
#include <sstream>
#include <string>

#include "hwinfo/os.h"
#include "hwinfo/utils/stringutils.h"

#include "apple_utils.h"

namespace hwinfo {

// _____________________________________________________________________________________________________________________
unsigned _get_system_version_from_kern() {
  // add a define for the run time os x version
  std::string osrelease(16, 0);
  size_t size = osrelease.size() - 1;
  sysctlbyname("kern.osrelease", &osrelease[0], &size, nullptr, 0);
  osrelease.back() = 0;
  const size_t major_dot = osrelease.find(".");
  const size_t minor_dot = (major_dot != std::string::npos ? osrelease.find(".", major_dot + 1) : std::string::npos);
  if(major_dot != std::string::npos && minor_dot != std::string::npos) {
    const size_t major_version = utils::stosize(osrelease.substr(0, major_dot));
    size_t os_minor_version = utils::stosize(osrelease.substr(major_dot + 1, major_dot - minor_dot - 1));

#if TARGET_OS_OSX
    // osrelease = kernel version
    // * <= 10.15: not os x version -> substract 4 (10.11 = 15 - 4, 10.10 = 14 - 4, 10.9 = 13 - 4, ...)
    // * >= 11.0: 110000 + x * 10000
    const size_t os_major_version = (major_version >= 20 ? 110000 + 10000 * (major_version - 20) : major_version - 4);
#else
    // osrelease = kernel version, not ios version -> substract 7 (NOTE: this won't run on anything < iOS 7.0,
    // so any differentiation below doesn't matter (5.0: darwin 11, 6.0: darwin 13, 7.0: darwin 14)
    size_t os_major_version = major_version - 7;

    // iOS 7.x and 8.x are both based on darwin 14.x -> need to differentiate through xnu kernel version
    // this is 24xx on iOS 7.x and 27xx on iOS 8.x
    // NOTE: iOS 9.x is based on darwin 15, so add 1 in this case as well (and all future versions)
    std::string kern_version(256, 0);
    size_t kern_version_size = kern_version.size() - 1;
    sysctlbyname("kern.version", &kern_version[0], &kern_version_size, nullptr, 0);
    if (kern_version.find("xnu-24") != std::string::npos) {
      // this is iOS 7.x, do nothing, os_major_version is already correct
    } else {
      // must be iOS 8.x or higher -> add 1 to the version
      ++os_major_version;
    }
#endif // TARGET_OS_OSX

    // mimic the compiled version string:
    // OS X >= 10.10 and <= 10.15:  10xxyy, x = major, y = minor
    // OS X >= 11.00             :  1xxyyy, x = major, y = minor
    // iOS                       :  xxyy00, x = major, y = minor
    size_t condensed_version;
#if TARGET_OS_OSX
    if (major_version < 20) { // 101000+
      condensed_version = 100000;
      condensed_version += os_major_version * 100;
      condensed_version += os_minor_version;
    } else {
      condensed_version = os_major_version;
      condensed_version += os_minor_version * 100;
    }
#else
    condensed_version = os_major_version * 10000;
    condensed_version += os_minor_version * 100;
#endif // TARGET_OS_OSX
    return condensed_version;
  }

  return 0;
}

// _____________________________________________________________________________________________________________________
OS::OS() {
  _32bit = false;
  _64bit = true;

  _name = "macOS";
  _fullName = "macOS <unknown version>";

  if (const char *prop = _get_sysctl_prop("kern.osproductversion")) {
    _version = prop;
    if (char *dot = (char*)strchr(prop, '.')) {
      *dot = 0;
      if (const char *os = get_os_name(atoi(prop), atoi(dot + 1))) {
        _fullName = "macOS " + std::string(os);
      }
    }
  }

  if (const char *prop = _get_sysctl_prop("kern.osrelease")) {
    _kernel = prop;
  }

  if (const char *prop = _get_sysctl_prop("kern.ostype")) {
    _details += "OS type: " + std::string(prop) + "\n";
  }
  if (const char *prop = get_os_target()) {
    _details += "OS family: " + std::string(prop) + "\n";
  }
  if (const char *prop = _get_sysctl_prop("kern.osrevision")) {
    _details += "OS revision: " + std::string(prop) + "\n";
  }
  if (int version = get_min_required_os_version()) {
    _details += "OS min. required build version: " + utils::to_string(version / 10000) + "." + utils::to_string(version / 100 % 100) + "." + utils::to_string(version % 100) + "\n";
  }
}

}  // namespace hwinfo

#endif  // HWINFO_APPLE
