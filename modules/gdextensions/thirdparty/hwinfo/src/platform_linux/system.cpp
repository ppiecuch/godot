// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include "hwinfo/platform.h"

#ifdef HWINFO_UNIX
#ifndef HWINFO_APPLE

#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <fstream>
#include <string>
#include <vector>

#include "hwinfo/system.h"

namespace hwinfo {

enum {
  UuidStringLen = sizeof("00000000-0000-0000-0000-000000000000") - 1
};

// _____________________________________________________________________________________________________________________
static std::string _get_boot_unique_id() {
  std::ifstream f("/proc/sys/kernel/random/boot_id");
  if (f) {
    std::string uuid;
    std::getline(f, uuid);
    return uuid;
  }
  return "";
}

// _____________________________________________________________________________________________________________________
static std::string _get_machine_unique_id() {
  // Try /etc/machine-id first (systemd), then /var/lib/dbus/machine-id
  for (const char *path : {"/etc/machine-id", "/var/lib/dbus/machine-id"}) {
    std::ifstream f(path);
    if (f) {
      std::string id;
      std::getline(f, id);
      if (!id.empty()) return id;
    }
  }
  // Fallback: DMI product_uuid (requires root)
  std::ifstream f("/sys/class/dmi/id/product_uuid");
  if (f) {
    std::string uuid;
    std::getline(f, uuid);
    if (!uuid.empty()) return uuid;
  }
  return "";
}

// _____________________________________________________________________________________________________________________
static std::vector<uint64_t> _get_cpu_times() {
  std::vector<uint64_t> times;
  std::ifstream f("/proc/stat");
  if (f) {
    std::string line;
    std::getline(f, line);
    // Format: cpu  user nice system idle iowait irq softirq steal guest guest_nice
    if (line.substr(0, 3) == "cpu") {
      uint64_t user = 0, nice = 0, system = 0, idle = 0;
      sscanf(line.c_str(), "cpu %lu %lu %lu %lu", &user, &nice, &system, &idle);
      times.push_back(user + nice);  // user time (ms)
      times.push_back(system);        // system time
      times.push_back(idle);           // idle time
    }
  }
  return times;
}

// _____________________________________________________________________________________________________________________
static std::vector<float> _get_cpu_usage() {
  // Read /proc/stat for per-CPU usage
  std::vector<float> usage;
  std::ifstream f("/proc/stat");
  if (!f) return usage;

  std::string line;
  std::getline(f, line);  // skip aggregate "cpu" line

  while (std::getline(f, line)) {
    if (line.substr(0, 3) != "cpu") break;
    uint64_t user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0;
    sscanf(line.c_str(), "cpu%*d %lu %lu %lu %lu %lu %lu %lu",
           &user, &nice, &system, &idle, &iowait, &irq, &softirq);
    uint64_t total = user + nice + system + idle + iowait + irq + softirq;
    uint64_t active = total - idle - iowait;
    usage.push_back(total > 0 ? (float)active / (float)total : 0.0f);
  }
  return usage;
}

// _____________________________________________________________________________________________________________________
static uint64_t _get_machine_uptime() {
  struct sysinfo info;
  if (sysinfo(&info) == 0) {
    return (uint64_t)info.uptime;
  }
  // Fallback: /proc/uptime
  std::ifstream f("/proc/uptime");
  if (f) {
    double uptime = 0;
    f >> uptime;
    return (uint64_t)uptime;
  }
  return 0;
}

// _____________________________________________________________________________________________________________________
size_t _get_num_processes() {
  // Count directories in /proc that are numeric (each is a PID)
  size_t count = 0;
  DIR *dir = opendir("/proc");
  if (dir) {
    struct dirent *entry;
    while ((entry = readdir(dir))) {
      if (entry->d_type == DT_DIR) {
        bool is_pid = true;
        for (const char *c = entry->d_name; *c; c++) {
          if (*c < '0' || *c > '9') { is_pid = false; break; }
        }
        if (is_pid) count++;
      }
    }
    closedir(dir);
  }
  return count;
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

#endif  // !HWINFO_APPLE
#endif  // HWINFO_UNIX
