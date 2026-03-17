// Copyright Leon Freist
// Android battery implementation via /sys/class/power_supply/

#include "hwinfo/platform.h"

#ifdef HWINFO_ANDROID

#include <fstream>
#include <string>
#include <vector>

#include "hwinfo/battery.h"

namespace hwinfo {

static std::string read_sysfs(const std::string &path) {
  std::ifstream f(path);
  if (f) {
    std::string val;
    std::getline(f, val);
    return val;
  }
  return "";
}

static int read_sysfs_int(const std::string &path, int fallback = 0) {
  std::string s = read_sysfs(path);
  if (!s.empty()) {
    try { return std::stoi(s); } catch (...) {}
  }
  return fallback;
}

static const std::string BAT_PATH = "/sys/class/power_supply/battery/";

Battery::Battery(int8_t id) : _id(id) {
  _vendor = read_sysfs(BAT_PATH + "manufacturer");
  _model = read_sysfs(BAT_PATH + "model_name");
  _serialNumber = read_sysfs(BAT_PATH + "serial_number");
  _technology = read_sysfs(BAT_PATH + "technology");
  // charge_full_design is in µAh, convert to mWh using voltage_now (µV)
  int64_t charge_full = read_sysfs_int(BAT_PATH + "charge_full_design", 0);
  int64_t voltage = read_sysfs_int(BAT_PATH + "voltage_now", 0);
  if (charge_full > 0 && voltage > 0) {
    _energyFull = (uint32_t)((charge_full / 1000) * (voltage / 1000) / 1000); // mWh
  }
  if (_vendor.empty()) _vendor = "<unknown>";
  if (_model.empty()) _model = "<unknown>";
}

std::string Battery::getVendor() const { return _vendor; }
std::string Battery::getModel() const { return _model; }
std::string Battery::getSerialNumber() const { return _serialNumber; }
std::string Battery::getTechnology() const { return _technology; }
uint32_t Battery::getEnergyFull() const { return _energyFull; }

std::string& Battery::vendor() { return _vendor; }
std::string& Battery::model() { return _model; }
std::string& Battery::serialNumber() { return _serialNumber; }
std::string& Battery::technology() { return _technology; }
uint32_t Battery::energyFull() { return _energyFull; }

uint32_t Battery::energyNow() const {
  int64_t charge_now = read_sysfs_int(BAT_PATH + "charge_now", 0);
  int64_t voltage = read_sysfs_int(BAT_PATH + "voltage_now", 0);
  if (charge_now > 0 && voltage > 0) {
    return (uint32_t)((charge_now / 1000) * (voltage / 1000) / 1000);
  }
  return 0;
}

double Battery::capacity() {
  int cap = read_sysfs_int(BAT_PATH + "capacity", -1);
  if (cap >= 0) return (double)cap / 100.0;
  return 0.0;
}

bool Battery::charging() const {
  return read_sysfs(BAT_PATH + "status") == "Charging";
}

bool Battery::discharging() const {
  return read_sysfs(BAT_PATH + "status") == "Discharging";
}

std::vector<Battery> getAllBatteries() {
  std::vector<Battery> batteries;
  std::ifstream f(BAT_PATH + "present");
  if (f) {
    int present = 0;
    f >> present;
    if (present) {
      batteries.push_back(Battery(0));
    }
  }
  return batteries;
}

}  // namespace hwinfo

#endif  // HWINFO_ANDROID
