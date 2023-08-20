// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <string>

namespace hwinfo {

class OS {
  std::string _fullName = "<unknown>";
  std::string _name = "<unknown>";
  std::string _version = "<unknown version>";
  std::string _kernel = "<unknown>";
  std::string _details;
  bool _32bit = false;
  bool _64bit = false;
  static bool _bigEndian;
  static bool _littleEndian;

public:
  std::string fullName() const { return _fullName; }
  std::string name() const { return _name; }
  std::string version() const { return _version; }
  std::string kernel() const { return _kernel; }
  std::string details() const { return _details; }
  bool is32bit() const { return _32bit; }
  bool is64bit() const { return _64bit; }
  bool isBigEndian() const { return _bigEndian; }
  bool isLittleEndian() const { return _littleEndian; }

  OS();
  ~OS() = default;
};

}  // namespace hwinfo
