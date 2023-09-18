// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include "hwinfo/cpu.h"

#include <string>
#include <vector>

#include "hwinfo/cpuid.h"
#include "hwinfo/platform.h"

namespace hwinfo {

// _____________________________________________________________________________________________________________________
Socket::Socket(class hwinfo::CPU cpu) : _cpu(std::move(cpu)) {}

}  // namespace hwinfo
