// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace hwinfo {

class Socket;

class CPU {
  friend std::vector<Socket> getAllSockets();
  friend class Socket;

  std::string _modelName;
  std::string _vendor;
  int _numPhysicalCores{-1};
  int _numLogicalCores{-1};
  int64_t _maxClockSpeed_MHz{-1};
  int64_t _regularClockSpeed_MHz{-1};
  int64_t _minClockSpeed_MHz{-1};
  int64_t _cacheSize_Bytes{-1};
  std::vector<std::string> _flags{};

  int _core_id{-1};

  CPU() = default;

 public:
  const std::string& modelName() const { return _modelName; }
  const std::string& vendor() const { return _vendor; }
  int64_t cacheSize_Bytes() const { return _cacheSize_Bytes; }
  int numPhysicalCores() const { return _numPhysicalCores; }
  int numLogicalCores() const { return _numLogicalCores; }
  int64_t maxClockSpeed_MHz() const { return _maxClockSpeed_MHz; }
  int64_t regularClockSpeed_MHz() const { return _regularClockSpeed_MHz; }
  int64_t minClockSpeed_MHz() const { return _minClockSpeed_MHz; }
  int64_t currentClockSpeed_MHz() const;
  const std::vector<std::string>& flags() const { return _flags; }
  int id() const { return _core_id; }

  ~CPU() = default;
};

class Socket {
  friend std::vector<Socket> getAllSockets();

  explicit Socket(class CPU cpu);
  int _id{-1};
  class CPU _cpu;

 public:
  const CPU& cpu() const { return _cpu; }
  int id() const { return _id; }

  ~Socket() = default;
};

std::vector<Socket> getAllSockets();

}  // namespace hwinfo
