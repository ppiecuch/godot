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
  int64_t _total_memory_Bytes = -1;
  int64_t _texture_memory_Bytes = -1;
  int64_t _available_memory_Bytes = -1;
  int64_t _frequency_MHz = 0;
  int _num_cores = 0;
  int _id = 0;

  GPU() = default;

 public:
  const std::string& vendor() const { return _vendor; }
  const std::string& name() const { return _name; }
  const std::string& driverVersion() const { return _driverVersion; }
  int64_t totalMemory_Bytes() const { return _total_memory_Bytes; }
  int64_t textureMemory_Bytes() const { return _texture_memory_Bytes; }
  int64_t availableMemory_Bytes() const { return _available_memory_Bytes; }
  int64_t frequency_MHz() const { return _frequency_MHz; }
  int num_cores() const { return _num_cores; }
  int id() const { return _id; }

  ~GPU() = default;
};

std::vector<GPU> getAllGPUs();

}  // namespace hwinfo
