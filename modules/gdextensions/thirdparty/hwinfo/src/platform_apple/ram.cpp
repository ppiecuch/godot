// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include "hwinfo/platform.h"

#ifdef HWINFO_APPLE

#include <mach/mach.h>
#include <mach/vm_statistics.h>
#include <sys/sysctl.h>
#include <sys/types.h>

#include <string>
#include <vector>

#include "hwinfo/ram.h"

namespace hwinfo {

// _____________________________________________________________________________________________________________________
static int64_t get_total_bytes() {
  int64_t memsize = 0;
  size_t size = sizeof(memsize);
  if (sysctlbyname("hw.memsize", &memsize, &size, nullptr, 0) == 0) {
    return memsize;
  }
  return -1;
}

// _____________________________________________________________________________________________________________________
RAM::RAM() {
  _name = "<unknown>";
  _vendor = "<unknown>";
  _serialNumber = "<unknown>";
  _model = "<unknown>";
  _total_Bytes = get_total_bytes();
  _free_Bytes = -1;
  _available_Bytes = -1;
}

}  // namespace hwinfo

#endif  // HWINFO_APPLE
