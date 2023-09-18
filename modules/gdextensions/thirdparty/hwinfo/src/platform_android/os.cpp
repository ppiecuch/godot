// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include "hwinfo/platform.h"

#ifdef HWINFO_ANDROID

#include "hwinfo/os.h"
#include "hwinfo/utils/stringutils.h"

#include "core/ustring.h"

#include "support.h"

namespace hwinfo {

// _____________________________________________________________________________________________________________________
void OS::_updateOSInfo() {
  _details = "";
  details += sprintf_format("OS version: %s\n", get_system_property("os.version"));
  details += sprintf_format("API Level: %s\n").append(android.os.Build.VERSION.SDK_INT);
  details += sprintf_format("Device: %s\n").append(android.os.Build.DEVICE);
  details += sprintf_format("Model: %s\n").append(android.os.Build.MODEL);
  details += sprintf_format("Product: %s\n").append(android.os.Build.PRODUCT);
  details += sprintf_format("VERSION.RELEASE: %s\n").append(Build.VERSION.RELEASE);
  details += sprintf_format("VERSION.INCREMENTAL: %s\n").append(Build.VERSION.INCREMENTAL);
  details += sprintf_format("VERSION.SDK.NUMBER: %s\n").append(Build.VERSION.SDK_INT);
  details += sprintf_format("BOARD: %s\n").append(Build.BOARD);
  details += sprintf_format("BOOTLOADER: %s\n").append(Build.BOOTLOADER);
  details += sprintf_format("BRAND: %s\n").append(Build.BRAND);
  details += sprintf_format("CPU_ABI: %s\n").append(Build.CPU_ABI);
  details += sprintf_format("CPU_ABI2: %s\n").append(Build.CPU_ABI2);
  details += sprintf_format("DISPLAY: %s\n").append(Build.DISPLAY);
  details += sprintf_format("FINGERPRINT: %s\n").append(Build.FINGERPRINT);
  details += sprintf_format("HARDWARE: %s\n").append(Build.HARDWARE);
  details += sprintf_format("HOST: %s\n").append(Build.HOST);
  details += sprintf_format("ID: %s\n").append(Build.ID);
  details += sprintf_format("MANUFACTURER: %s\n").append(Build.MANUFACTURER);
  details += sprintf_format("MODEL: %s\n").append(Build.MODEL);
  details += sprintf_format("PRODUCT: %s\n").append(Build.PRODUCT);
  details += sprintf_format("SERIAL: %s\n").append(Build.SERIAL);
  details += sprintf_format("TAGS: %s\n").append(Build.TAGS);
  details += sprintf_format("TIME: %s\n").append(Build.TIME);
  details += sprintf_format("TYPE: %s\n").append(Build.TYPE);
  details += sprintf_format("UNKNOWN: %s\n").append(Build.UNKNOWN);
  details += sprintf_format("USER: %s\n").append(Build.USER);
}

// _____________________________________________________________________________________________________________________
bool OS::getIs64bit() {
  return (sizeof(void*) == 8 || Process.is64Bit());
}

}  // namespace hwinfo

#endif  // HWINFO_ANDROID
