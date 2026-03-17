// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include "hwinfo/platform.h"

#ifdef HWINFO_APPLE

#include <sys/sysctl.h>
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <string>

#include "hwinfo/mainboard.h"
#include "apple_utils.h"

namespace hwinfo {

// _____________________________________________________________________________________________________________________
MainBoard::MainBoard() {
  _vendor = "Apple Inc.";

  // hw.model gives the model identifier (e.g., "MacBookPro18,1", "Mac14,2")
  char model[128] = {};
  size_t model_len = sizeof(model);
  if (sysctlbyname("hw.model", model, &model_len, nullptr, 0) == 0) {
    _name = std::string(model);
  }

  // Board-id from IOKit (e.g., "Mac-937A206F2EE63C01")
  io_service_t service = IOServiceGetMatchingService(kIOMasterPortDefault,
      IOServiceMatching("IOPlatformExpertDevice"));
  if (service) {
    // board-id
    CFTypeRef board_ref = IORegistryEntryCreateCFProperty(service, CFSTR("board-id"), kCFAllocatorDefault, 0);
    if (board_ref) {
      if (CFGetTypeID(board_ref) == CFDataGetTypeID()) {
        CFDataRef data = (CFDataRef)board_ref;
        _version = std::string((const char *)CFDataGetBytePtr(data), CFDataGetLength(data));
        // Trim null terminator if present
        while (!_version.empty() && _version.back() == '\0') {
          _version.pop_back();
        }
      } else if (CFGetTypeID(board_ref) == CFStringGetTypeID()) {
        _version = cf_conv_string((CFStringRef)board_ref);
      }
      CFRelease(board_ref);
    }

    // Serial number
    CFStringRef serial_ref = (CFStringRef)IORegistryEntryCreateCFProperty(service,
        CFSTR(kIOPlatformSerialNumberKey), kCFAllocatorDefault, 0);
    if (serial_ref) {
      if (CFGetTypeID(serial_ref) == CFStringGetTypeID()) {
        _serialNumber = cf_conv_string(serial_ref);
      }
      CFRelease(serial_ref);
    }

    IOObjectRelease(service);
  }
}

// _____________________________________________________________________________________________________________________
const std::string& MainBoard::vendor() const { return _vendor; }
const std::string& MainBoard::name() const { return _name; }
const std::string& MainBoard::version() const { return _version; }
const std::string& MainBoard::serialNumber() const { return _serialNumber; }

}  // namespace hwinfo

#endif  // HWINFO_APPLE
