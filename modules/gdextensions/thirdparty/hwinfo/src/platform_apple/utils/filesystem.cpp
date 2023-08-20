#include "hwinfo/platform.h"

#ifdef HWINFO_APPLE

#include <string>
#include <vector>

#include "hwinfo/utils/filesystem.h"

bool hwinfo::filesystem::exists(const std::string& path) {
  return false; // TODO: implement if needed
}

std::vector<std::string> hwinfo::filesystem::getDirectoryEntries(const std::string& path) {
  return {}; // TODO: implement if needed
}

#endif  // HWINFO_APPLE
