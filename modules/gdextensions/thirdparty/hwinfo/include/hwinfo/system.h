// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <string>
#include <vector>

namespace hwinfo {

class System {
  std::string _machineUniqueId;
  std::string _bootUniqueId;

 public:
  const std::string& getMachineUniqueId() const { return _machineUniqueId; }
  const std::string& getBootUniqueId() const { return _bootUniqueId; }

  System();
};

}  // namespace hwinfo
