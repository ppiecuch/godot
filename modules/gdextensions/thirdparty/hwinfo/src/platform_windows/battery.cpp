// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#include "hwinfo/platform.h"

#ifdef HWINFO_WINDOWS

#include <windows.h>
#include <string>
#include <vector>

#include "hwinfo/WMIwrapper.h"
#include "hwinfo/battery.h"

namespace hwinfo {

// Helper: query a single WMI string field from Win32_Battery
static std::string wmi_battery_string(const char *field) {
  std::vector<const wchar_t *> res{};
  wmi::queryWMI("Win32_Battery", field, res);
  if (!res.empty() && res.front()) {
    std::wstring ws(res.front());
    return std::string(ws.begin(), ws.end());
  }
  return "<unknown>";
}

// =====================================================================================================================
Battery::Battery(int8_t id) : _id(id) {}

// _____________________________________________________________________________________________________________________
std::string Battery::getVendor() const {
  // WMI doesn't expose vendor directly; try Manufacturer from CIM_Battery
  return _vendor.empty() ? "<unknown>" : _vendor;
}

std::string Battery::getModel() const { return _model; }
std::string Battery::getSerialNumber() const { return _serialNumber; }
std::string Battery::getTechnology() const { return _technology; }
uint32_t Battery::getEnergyFull() const { return _energyFull; }

std::string& Battery::vendor() { return _vendor; }
std::string& Battery::model() { return _model; }
std::string& Battery::serialNumber() { return _serialNumber; }
std::string& Battery::technology() { return _technology; }
uint32_t Battery::energyFull() { return _energyFull; }

// _____________________________________________________________________________________________________________________
uint32_t Battery::energyNow() const {
  // Use GetSystemPowerStatus for current charge estimate
  SYSTEM_POWER_STATUS ps;
  if (GetSystemPowerStatus(&ps) && ps.BatteryLifePercent != 255) {
    if (_energyFull > 0) {
      return (uint32_t)((uint64_t)_energyFull * ps.BatteryLifePercent / 100);
    }
    return ps.BatteryLifePercent;
  }
  return 0;
}

// _____________________________________________________________________________________________________________________
double Battery::capacity() {
  SYSTEM_POWER_STATUS ps;
  if (GetSystemPowerStatus(&ps) && ps.BatteryLifePercent != 255) {
    return (double)ps.BatteryLifePercent / 100.0;
  }
  return 0.0;
}

// _____________________________________________________________________________________________________________________
bool Battery::charging() const {
  SYSTEM_POWER_STATUS ps;
  if (GetSystemPowerStatus(&ps)) {
    return (ps.BatteryFlag & 8) != 0;  // Bit 3 = charging
  }
  return false;
}

// _____________________________________________________________________________________________________________________
bool Battery::discharging() const {
  SYSTEM_POWER_STATUS ps;
  if (GetSystemPowerStatus(&ps)) {
    return ps.ACLineStatus == 0 && !(ps.BatteryFlag & 8);
  }
  return false;
}

// =====================================================================================================================
std::vector<Battery> getAllBatteries() {
  std::vector<Battery> batteries;

  // Check if system has a battery at all
  SYSTEM_POWER_STATUS ps;
  if (!GetSystemPowerStatus(&ps) || (ps.BatteryFlag & 128)) {
    // No battery present (bit 7 = no system battery)
    return batteries;
  }

  // Query WMI for battery details
  std::vector<const wchar_t *> names{};
  wmi::queryWMI("Win32_Battery", "Name", names);
  if (names.empty() || !names.front()) {
    // GetSystemPowerStatus says battery exists but WMI can't find it
    Battery bat(0);
    bat._model = "System Battery";
    batteries.push_back(bat);
    return batteries;
  }

  int8_t counter = 0;
  for (const auto &v : names) {
    Battery bat(counter++);
    std::wstring ws(v);
    bat._model = std::string(ws.begin(), ws.end());
    batteries.push_back(bat);
  }

  // Try to fill in more details from WMI
  if (!batteries.empty()) {
    // DesignCapacity (mWh)
    std::vector<const wchar_t *> cap{};
    wmi::queryWMI("Win32_Battery", "DesignCapacity", cap);
    if (!cap.empty() && cap.front()) {
      std::wstring ws(cap.front());
      try { batteries[0]._energyFull = std::stoul(std::string(ws.begin(), ws.end())); } catch (...) {}
    }
    // Chemistry → technology
    std::vector<const wchar_t *> chem{};
    wmi::queryWMI("Win32_Battery", "Chemistry", chem);
    if (!chem.empty() && chem.front()) {
      std::wstring ws(chem.front());
      int code = 0;
      try { code = std::stoi(std::string(ws.begin(), ws.end())); } catch (...) {}
      switch (code) {
        case 3: batteries[0]._technology = "Lead Acid"; break;
        case 4: batteries[0]._technology = "Nickel Cadmium"; break;
        case 5: batteries[0]._technology = "Nickel Metal Hydride"; break;
        case 6: batteries[0]._technology = "Li-Ion"; break;
        case 8: batteries[0]._technology = "Li-Polymer"; break;
        default: batteries[0]._technology = "Unknown"; break;
      }
    }
  }

  return batteries;
}

}  // namespace hwinfo

#endif  // HWINFO_WINDOWS
