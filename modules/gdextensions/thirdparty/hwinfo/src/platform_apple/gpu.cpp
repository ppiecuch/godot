// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#include "hwinfo/platform.h"

#ifdef HWINFO_APPLE
#ifndef USE_OCL

#include <regex>
#include <string>
#include <vector>

#include "hwinfo/gpu.h"

namespace hwinfo {

// _____________________________________________________________________________________________________________________
const std::string& GPU::vendor() const {
  return _vendor;
}

// _____________________________________________________________________________________________________________________
const std::string& GPU::name() const {
  return _name;
}

// _____________________________________________________________________________________________________________________
const std::string& GPU::driverVersion() const {
  return _driverVersion;
}

// _____________________________________________________________________________________________________________________
int64_t GPU::memory_Bytes() const {
  return -1;
}

}  // namespace hwinfo

#endif  // USE_OCL
#endif  // HWINFO_APPLE
