// kate: replace-tabs on; tab-indents on; tab-width 2; indent-width 2; indent-mode cstyle;

// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include "hwinfo/hwinfo.h"

#include <sys/types.h>
#include <fcntl.h>
#include <stdint.h>

#include <string>
#include <vector>

#include "linux_utils.h"

namespace hwinfo {

enum {
  UuidStringLen = sizeof("00000000-0000-0000-0000-000000000000") - 1
};

// _____________________________________________________________________________________________________________________
static std::string _get_boot_unique_id() { // use low-level API here for simplicity
  int fd = io_safe_open("/proc/sys/kernel/random/boot_id", O_RDONLY);
  if (fd != -1) {
    char uuid[UuidStringLen] = { 0 };
    int64_t len = io_safe_read(fd, uuid, sizeof(uuid));
    io_safe_close(fd);
    if (len == UuidStringLen) {
      return std::string(uuid, UuidStringLen);
    }
  }
  return "";
}

static std::string _get_machine_unique_id() {
  // The modern name on Linux is /etc/machine-id, but that path is unlikely to
  // exist on non-Linux (non-systemd) systems. The old path is more than enough.
  static const char fullfilename[] = "/usr/local/var/lib/dbus/machine-id";
  const char *firstfilename = fullfilename + sizeof("/usr/local") - 1;
  int fd = io_safe_open(firstfilename, O_RDONLY);
  if (fd == -1 && errno == ENOENT) {
    fd = io_safe_open(fullfilename, O_RDONLY);
  }
  if (fd != -1) {
    char buffer[32]; // 128 bits, hex-encoded
    int64_t len = io_safe_read(fd, buffer, sizeof(buffer));
    io_safe_close(fd);
    if (len != -1) {
      return std::string(buffer, len);
    }
  }
}

static std::vector<uint64_t> _get_cpu_times () {
}

static std::vector<float> _get_cpu_usage() {
}

static uint64_t _get_machine_uptime() {
}

size_t _get_num_processes() {
}

// _____________________________________________________________________________________________________________________
System::System() {
  _machineUniqueId = _get_machine_unique_id();
  _bootUniqueId = _get_boot_unique_id();
  _cpuUsagePercent = _get_cpu_usage();
  _uptimeSeconds = _get_machine_uptime();
  _cpuStatsTime = _get_cpu_times();
  _numProcesses = _get_num_processes();
}

}  // namespace hwinfo
