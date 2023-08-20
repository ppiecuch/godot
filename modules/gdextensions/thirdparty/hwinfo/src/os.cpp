// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include "hwinfo/os.h"

#include <stdint.h>
#include <string>

namespace hwinfo {

// _____________________________________________________________________________________________________________________
static bool _get_is_big_endian() {
  static const char16_t dummy = 0x0102;
  return ((char*)&dummy)[0] == 0x01;
}

// _____________________________________________________________________________________________________________________
static bool _get_is_little_endian() {
  static const char16_t dummy = 0x0102;
  return ((char*)&dummy)[0] == 0x02;
}

// _____________________________________________________________________________________________________________________
bool OS::_bigEndian = _get_is_big_endian();
bool OS::_littleEndian = _get_is_little_endian();

}  // namespace hwinfo
