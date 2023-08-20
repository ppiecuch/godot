// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include "hwinfo/system.h"

#include <sys/sysctl.h>
#include <stdint.h>
#include <string>

#if __has_include(<IOKit/IOKitLib.h>)
# include <IOKit/IOKitLib.h>
#endif

namespace hwinfo {

enum {
  UuidStringLen = sizeof("00000000-0000-0000-0000-000000000000") - 1
};

// _____________________________________________________________________________________________________________________
std::string _get_boot_unique_id() {
  // "kern.bootsessionuuid" is only available by name
  char uuid[UuidStringLen + 1];
  size_t uuidlen = sizeof(uuid);
  if (sysctlbyname("kern.bootsessionuuid", uuid, &uuidlen, nullptr, 0) == 0 && uuidlen == sizeof(uuid)) {
        return std::string(uuid, uuidlen - 1);
  }
  return std::string();
}

std::string _get_machine_unique_id() {
#if __has_include(<IOKit/IOKitLib.h>)
  char uuid[UuidStringLen + 1];
  static const mach_port_t defaultPort = 0; // Effectively kIOMasterPortDefault/kIOMainPortDefault
  io_service_t service = IOServiceGetMatchingService(defaultPort, IOServiceMatching("IOPlatformExpertDevice"));
  CFStringRef string_ref = (CFStringRef)IORegistryEntryCreateCFProperty(service, CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, 0);
  CFStringGetCString(string_ref, uuid, sizeof(uuid), kCFStringEncodingMacRoman);
  return std::string(uuid);
#else
  return std::string();
#endif
}

// _____________________________________________________________________________________________________________________
System::System() {
  _machineUniqueId = _get_machine_unique_id();
  _bootUniqueId = _get_boot_unique_id();
}

}  // namespace hwinfo
