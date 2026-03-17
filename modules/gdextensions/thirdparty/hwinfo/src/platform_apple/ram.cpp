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
static int64_t _get_total_bytes() {
  int64_t memsize = 0;
  size_t size = sizeof(memsize);
  if (sysctlbyname("hw.memsize", &memsize, &size, nullptr, 0) == 0) {
    return memsize;
  }
  return -1;
}

// _____________________________________________________________________________________________________________________
static int64_t _get_free_bytes() {
  mach_port_t host = mach_host_self();
  vm_size_t page_size;
  host_page_size(host, &page_size);

  vm_statistics64_data_t vm_stat;
  mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
  if (host_statistics64(host, HOST_VM_INFO64, (host_info64_t)&vm_stat, &count) == KERN_SUCCESS) {
    return (int64_t)vm_stat.free_count * page_size;
  }
  return -1;
}

// _____________________________________________________________________________________________________________________
static int64_t _get_available_bytes() {
  // Available = free + inactive (pages that can be reclaimed)
  mach_port_t host = mach_host_self();
  vm_size_t page_size;
  host_page_size(host, &page_size);

  vm_statistics64_data_t vm_stat;
  mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
  if (host_statistics64(host, HOST_VM_INFO64, (host_info64_t)&vm_stat, &count) == KERN_SUCCESS) {
    return (int64_t)(vm_stat.free_count + vm_stat.inactive_count) * page_size;
  }
  return -1;
}

// _____________________________________________________________________________________________________________________
RAM::RAM() {
  _total_Bytes = _get_total_bytes();
  _free_Bytes = _get_free_bytes();
  _available_Bytes = _get_available_bytes();
  _vendor = "Apple";
}

// _____________________________________________________________________________________________________________________
std::vector<RAM> getAllRamBars() {
  // macOS doesn't expose individual DIMM info via public APIs.
  // Return a single entry with the system-wide totals.
  std::vector<RAM> bars;
  bars.push_back(RAM());
  return bars;
}

}  // namespace hwinfo

#endif  // HWINFO_APPLE
