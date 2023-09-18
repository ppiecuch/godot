// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <string>
#include <vector>

namespace hwinfo {

class RAM {
  std::string _vendor = "<unknown>";
  std::string _name = "<unknown>";
  std::string _model = "<unknown>";
  std::string _serialNumber = "<unknown>";
  int64_t _total_Bytes = -1;
  int64_t _free_Bytes = -1;
  int64_t _available_Bytes = -1;
  int32_t _frequency = -1;

 public:
  const std::string& vendor() const { return _vendor; }
  const std::string& name() const { return _name; }
  const std::string& model() const { return _model; }
  const std::string& serialNumber() const { return _serialNumber; }
  int64_t total_Bytes() const { return _total_Bytes; }
  int64_t free_Bytes() const { return _free_Bytes; }
  int64_t available_Bytes() const { return _available_Bytes; }
  int32_t frequency() const { return _frequency; }

  RAM();
  ~RAM() = default;
};

std::vector<RAM> getAllRamBars();

}  // namespace hwinfo
