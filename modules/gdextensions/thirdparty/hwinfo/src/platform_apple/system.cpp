// kate: replace-tabs on; tab-indents on; tab-width 2; indent-width 2; indent-mode cstyle;

// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include "hwinfo/hwinfo.h"

#include <sys/sysctl.h>
#include <sys/types.h>
#include <mach/mach.h>
#include <mach/processor_info.h>
#include <mach/mach_host.h>
#include <stdint.h>
#include <string>

#include "apple_utils.h"

#if __has_include(<IOKit/IOKitLib.h>)
# include <IOKit/IOKitLib.h>
#endif

namespace hwinfo {

enum {
  UuidStringLen = sizeof("00000000-0000-0000-0000-000000000000") - 1
};

static uint64_t TicksPerSecond;

// _____________________________________________________________________________________________________________________
static std::string _get_boot_unique_id() {
  // "kern.bootsessionuuid" is only available by name
  char uuid[UuidStringLen + 1];
  size_t uuidlen = sizeof(uuid);
  if (sysctlbyname("kern.bootsessionuuid", uuid, &uuidlen, nullptr, 0) == 0 && uuidlen == sizeof(uuid)) {
        return std::string(uuid, uuidlen - 1);
  }
  return std::string();
}

static std::string _get_machine_unique_id() {
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

static std::vector<uint64_t> _get_cpu_times () {
  mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
  host_cpu_load_info_data_t host_info;
  kern_return_t error = host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)&host_info, &count);
  if (error != KERN_SUCCESS) {
    hwinfo_error("Error trying to get CPU usage: %s\n", mach_error_string(error));
    return std::vector<uint64_t>();
  } else {
    uint64_t userticks = host_info.cpu_ticks[CPU_STATE_USER] + host_info.cpu_ticks[CPU_STATE_NICE];
    uint64_t systicks = host_info.cpu_ticks[CPU_STATE_SYSTEM];
    uint64_t idleticks = host_info.cpu_ticks[CPU_STATE_IDLE];

    return { (userticks * 1000) / TicksPerSecond, (systicks * 1000) / TicksPerSecond, (idleticks * 1000) / TicksPerSecond };
  }
}

static uint64_t _get_machine_uptime() {
  struct timeval secs;

  if (gettimeofday(&secs, nullptr) != 0) {
    return 0;
  }

  uint64_t uptime = secs.tv_sec;

  static int mib[2] = { CTL_KERN, KERN_BOOTTIME };
  size_t len = sizeof(secs);
  // fetch sysctl "kern.boottime"
  if (sysctl(mib, 2, &secs, &len, nullptr, 0) != 0) {
    return 0;
  }

  return (uptime -= secs.tv_sec);
}

// https://stackoverflow.com/questions/6785069/get-cpu-percent-usage-on-macos?lq=1

static std::vector<float> _get_cpu_usage() {
  static int mib[2] = { CTL_HW, HW_NCPU };
  unsigned cpu_count = 0;
  size_t cpu_count_sz = sizeof(cpu_count);
  if (sysctl(mib, 2, &cpu_count, &cpu_count_sz, nullptr, 0) != 0) {
    cpu_count = 1; // fallback
  }

  std::vector<float> usage;

  processor_info_array_t cpu_info = 0, prev_cpu_info = 0;
  mach_msg_type_number_t num_cpu_info = 0, num_prev_cpu_info = 0;
  natural_t cpu = 0;
  kern_return_t error = host_processor_info(mach_host_self(), PROCESSOR_CPU_LOAD_INFO, &cpu, &cpu_info, &num_cpu_info);
  if (error == KERN_SUCCESS) {
    for (unsigned i = 0; i < cpu_count; ++i) {
      float in_use, total;
      if (prev_cpu_info) {
        in_use =
          ((cpu_info[(CPU_STATE_MAX * i) + CPU_STATE_USER] - prev_cpu_info[(CPU_STATE_MAX * i) + CPU_STATE_USER]) +
          (cpu_info[(CPU_STATE_MAX * i) + CPU_STATE_SYSTEM] - prev_cpu_info[(CPU_STATE_MAX * i) + CPU_STATE_SYSTEM]) +
          (cpu_info[(CPU_STATE_MAX * i) + CPU_STATE_NICE] - prev_cpu_info[(CPU_STATE_MAX * i) + CPU_STATE_NICE]));
        total = in_use + (cpu_info[(CPU_STATE_MAX * i) + CPU_STATE_IDLE] - prev_cpu_info[(CPU_STATE_MAX * i) + CPU_STATE_IDLE]);
      } else {
        in_use = cpu_info[(CPU_STATE_MAX * i) + CPU_STATE_USER] + cpu_info[(CPU_STATE_MAX * i) + CPU_STATE_SYSTEM] + cpu_info[(CPU_STATE_MAX * i) + CPU_STATE_NICE];
        total = in_use + cpu_info[(CPU_STATE_MAX * i) + CPU_STATE_IDLE];
      }

      usage.push_back(in_use / total); // usge of core i
    }

    if(prev_cpu_info) {
      size_t prev_cpu_info_sz = sizeof(integer_t) * num_prev_cpu_info;
      vm_deallocate(mach_task_self(), (vm_address_t)prev_cpu_info, prev_cpu_info_sz);
    }

    prev_cpu_info = cpu_info, num_prev_cpu_info = num_cpu_info;
    cpu_info = nullptr, num_cpu_info = 0;
  } else {
    hwinfo_error("Error trying to get CPU usage: %s\n", mach_error_string(error));
  }
  return usage;
}

size_t _get_num_processes() {
  static int mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL };
  size_t length = 0;
  if (sysctl(mib, 3, nullptr, &length, nullptr, 0) < 0) {
    return 0;
  }
  return (length / sizeof(struct kinfo_proc));
}

// _____________________________________________________________________________________________________________________
System::System() {
  TicksPerSecond = sysconf(_SC_CLK_TCK);

  register_property(SYS_PROCESS_REPORT, [](PropertyArg arg, int64_t *i, float *f, std::string *s){
    *s = cf_conv_string(get_processes_report(nullptr));
  });

  _machineUniqueId = _get_machine_unique_id();
  _bootUniqueId = _get_boot_unique_id();
  _cpuUsagePercent = _get_cpu_usage();
  _uptimeSeconds = _get_machine_uptime();
  _cpuStatsTime = _get_cpu_times();
  _numProcesses = _get_num_processes();
}

}  // namespace hwinfo
