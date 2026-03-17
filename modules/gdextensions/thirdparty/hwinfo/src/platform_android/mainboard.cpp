// Copyright Leon Freist
// Android mainboard implementation via /system/build.prop

#include "hwinfo/platform.h"

#ifdef HWINFO_ANDROID

#include <fstream>
#include <string>

#include "hwinfo/mainboard.h"

namespace hwinfo {

static std::string get_prop(const std::string &key) {
  // Read from /system/build.prop
  std::ifstream f("/system/build.prop");
  if (!f) return "";
  std::string line;
  while (std::getline(f, line)) {
    if (line.find(key + "=") == 0) {
      return line.substr(key.size() + 1);
    }
  }
  return "";
}

MainBoard::MainBoard() {
  _vendor = get_prop("ro.product.manufacturer");
  _name = get_prop("ro.product.board");
  _version = get_prop("ro.build.display.id");
  _serialNumber = get_prop("ro.serialno");

  if (_vendor.empty()) _vendor = get_prop("ro.product.brand");
  if (_name.empty()) _name = get_prop("ro.product.device");
  if (_vendor.empty()) _vendor = "<unknown>";
  if (_name.empty()) _name = "<unknown>";
}

const std::string& MainBoard::vendor() const { return _vendor; }
const std::string& MainBoard::name() const { return _name; }
const std::string& MainBoard::version() const { return _version; }
const std::string& MainBoard::serialNumber() const { return _serialNumber; }

}  // namespace hwinfo

#endif  // HWINFO_ANDROID
