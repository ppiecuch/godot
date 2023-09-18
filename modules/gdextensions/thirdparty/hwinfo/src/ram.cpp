// Copyright (c) Leon Freist <freist@informatik.uni-freiburg.de>
// This software is part of HWBenchmark

#include <string>
#include <vector>

#if defined(unix) || defined(__unix) || defined(__unix__)
# include <unistd.h>
#elif defined(__APPLE__)
# include <sys/sysctl.h>
#elif defined(_WIN32) || defined(_WIN64)
# include <windows.h>
# include "hwinfo/WMIwrapper.h"
#endif

#include "hwinfo/ram.h"

namespace hwinfo {

}  // namespace hwinfo
