// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <string>
#include <vector>

namespace hwinfo {

class GPU {
  friend std::vector<GPU> getAllGPUs();

  std::string _vendor = "<unknown>";
  std::string _name = "<unknown>";
  std::string _driverVersion  = "<unknown version>";
  std::string _platformDetails;
  int64_t _totalMemoryMBytes = -1;
  int64_t _frequencyMHz = 0;
  int _numCores = 0;
  int _id = 0;

  GPU() = default;

 public:
  const std::string& vendor() const { return _vendor; }
  const std::string& name() const { return _name; }
  const std::string& driverVersion() const { return _driverVersion; }
  const std::string& platformDetails() const { return _platformDetails; }
  int64_t totalMemoryMBytes() const { return _totalMemoryMBytes; }
  int64_t frequencyMHz() const { return _frequencyMHz; }
  int num_cores() const { return _numCores; }
  int id() const { return _id; }

  ~GPU() = default;
};

std::vector<GPU> getAllGPUs();

}  // namespace hwinfo
