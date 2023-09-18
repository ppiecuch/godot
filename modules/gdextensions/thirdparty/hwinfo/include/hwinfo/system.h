// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <string>
#include <vector>

namespace hwinfo {

class System {
  std::string _machineUniqueId;
  std::string _bootUniqueId;
  uint64_t _uptimeSeconds;
  std::vector<float> _cpuUsagePercent;
  std::vector<uint64_t> _cpuStatsTime;
  size_t _numProcesses;

 public:
  const std::string& getMachineUniqueId() const { return _machineUniqueId; }
  const std::string& getBootUniqueId() const { return _bootUniqueId; }
  uint64_t getUptimeSeconds() const { return _uptimeSeconds; }
  std::vector<float> getCpuUsagePercent() const { return _cpuUsagePercent; }
  std::vector<uint64_t> getCpuStatsTime() const { return _cpuStatsTime; }
  size_t getNumProcesses() const { return _numProcesses; }

  System();
};

}  // namespace hwinfo
