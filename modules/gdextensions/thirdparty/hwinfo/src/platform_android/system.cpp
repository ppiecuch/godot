// Copyright Leon Freist
// Android system implementation via /proc

#include "hwinfo/platform.h"

#ifdef HWINFO_ANDROID

#include <sys/sysinfo.h>
#include <dirent.h>
#include <fstream>
#include <string>
#include <vector>

#include "hwinfo/system.h"

namespace hwinfo {

static std::string _get_boot_unique_id() {
  std::ifstream f("/proc/sys/kernel/random/boot_id");
  if (f) {
    std::string uuid;
    std::getline(f, uuid);
    return uuid;
  }
  return "";
}

static std::string _get_machine_unique_id() {
  // Android: try /sys/class/android_usb/android0/iSerial or build fingerprint
  for (const char *path : {
    "/sys/class/android_usb/android0/iSerial",
    "/sys/class/net/wlan0/address"
  }) {
    std::ifstream f(path);
    if (f) {
      std::string id;
      std::getline(f, id);
      if (!id.empty()) return id;
    }
  }
  return "";
}

static std::vector<uint64_t> _get_cpu_times() {
  std::vector<uint64_t> times;
  std::ifstream f("/proc/stat");
  if (f) {
    std::string line;
    std::getline(f, line);
    if (line.substr(0, 3) == "cpu") {
      uint64_t user = 0, nice = 0, system = 0, idle = 0;
      sscanf(line.c_str(), "cpu %lu %lu %lu %lu", &user, &nice, &system, &idle);
      times.push_back(user + nice);
      times.push_back(system);
      times.push_back(idle);
    }
  }
  return times;
}

static std::vector<float> _get_cpu_usage() {
  std::vector<float> usage;
  std::ifstream f("/proc/stat");
  if (!f) return usage;
  std::string line;
  std::getline(f, line);  // skip aggregate
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

static uint64_t _get_machine_uptime() {
  struct sysinfo info;
  if (sysinfo(&info) == 0) {
    return (uint64_t)info.uptime;
  }
  return 0;
}

static size_t _get_num_processes() {
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

System::System() {
  _machineUniqueId = _get_machine_unique_id();
  _bootUniqueId = _get_boot_unique_id();
  _cpuUsagePercent = _get_cpu_usage();
  _uptimeSeconds = _get_machine_uptime();
  _cpuStatsTime = _get_cpu_times();
  _numProcesses = _get_num_processes();
}

}  // namespace hwinfo

#endif  // HWINFO_ANDROID
