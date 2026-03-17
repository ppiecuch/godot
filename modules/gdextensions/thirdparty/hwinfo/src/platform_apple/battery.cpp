// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#include "hwinfo/platform.h"

#ifdef HWINFO_APPLE

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <string>
#include <vector>

#include "hwinfo/battery.h"
#include "apple_utils.h"

namespace hwinfo {

// Helper: read a CFString property from an IOKit service
static std::string iokit_string(io_service_t service, const char *key) {
  CFStringRef cf_key = CFStringCreateWithCString(kCFAllocatorDefault, key, kCFStringEncodingUTF8);
  CFTypeRef ref = IORegistryEntryCreateCFProperty(service, cf_key, kCFAllocatorDefault, 0);
  CFRelease(cf_key);
  if (!ref) return "";
  if (CFGetTypeID(ref) == CFStringGetTypeID()) {
    std::string result = cf_conv_string((CFStringRef)ref);
    CFRelease(ref);
    return result;
  }
  CFRelease(ref);
  return "";
}

// Helper: read an integer property
static int64_t iokit_int(io_service_t service, const char *key, int64_t fallback = 0) {
  CFStringRef cf_key = CFStringCreateWithCString(kCFAllocatorDefault, key, kCFStringEncodingUTF8);
  CFTypeRef ref = IORegistryEntryCreateCFProperty(service, cf_key, kCFAllocatorDefault, 0);
  CFRelease(cf_key);
  if (!ref) return fallback;
  int64_t val = fallback;
  if (CFGetTypeID(ref) == CFNumberGetTypeID()) {
    CFNumberGetValue((CFNumberRef)ref, kCFNumberSInt64Type, &val);
  }
  CFRelease(ref);
  return val;
}

// Helper: read a boolean property
static bool iokit_bool(io_service_t service, const char *key, bool fallback = false) {
  CFStringRef cf_key = CFStringCreateWithCString(kCFAllocatorDefault, key, kCFStringEncodingUTF8);
  CFTypeRef ref = IORegistryEntryCreateCFProperty(service, cf_key, kCFAllocatorDefault, 0);
  CFRelease(cf_key);
  if (!ref) return fallback;
  bool val = fallback;
  if (CFGetTypeID(ref) == CFBooleanGetTypeID()) {
    val = CFBooleanGetValue((CFBooleanRef)ref);
  }
  CFRelease(ref);
  return val;
}

// Cache the battery service handle
static io_service_t get_battery_service() {
  return IOServiceGetMatchingService(kIOMasterPortDefault,
      IOServiceMatching("AppleSmartBattery"));
}

// _____________________________________________________________________________________________________________________
Battery::Battery(int8_t id) : _id(id) {
  io_service_t svc = get_battery_service();
  if (!svc) return;

  _vendor = iokit_string(svc, "Manufacturer");
  _model = iokit_string(svc, "DeviceName");
  _serialNumber = iokit_string(svc, "BatterySerialNumber");
  _technology = "Li-Ion";  // All Mac batteries are lithium-ion

  // MaxCapacity is in mAh; convert to approximate mWh using design voltage
  int64_t max_cap = iokit_int(svc, "MaxCapacity", 0);
  int64_t voltage = iokit_int(svc, "Voltage", 0);
  if (max_cap > 0 && voltage > 0) {
    _energyFull = (uint32_t)((max_cap * voltage) / 1000);  // mAh * mV / 1000 = mWh
  } else {
    _energyFull = (uint32_t)max_cap;
  }

  IOObjectRelease(svc);
}

// _____________________________________________________________________________________________________________________
std::string Battery::getVendor() const { return _vendor; }
std::string Battery::getModel() const { return _model; }
std::string Battery::getSerialNumber() const { return _serialNumber; }
std::string Battery::getTechnology() const { return _technology; }
uint32_t Battery::getEnergyFull() const { return _energyFull; }

// _____________________________________________________________________________________________________________________
std::string& Battery::vendor() { return _vendor; }
std::string& Battery::model() { return _model; }
std::string& Battery::serialNumber() { return _serialNumber; }
std::string& Battery::technology() { return _technology; }
uint32_t Battery::energyFull() { return _energyFull; }

// _____________________________________________________________________________________________________________________
uint32_t Battery::energyNow() const {
  io_service_t svc = get_battery_service();
  if (!svc) return 0;
  int64_t cur_cap = iokit_int(svc, "CurrentCapacity", 0);
  int64_t voltage = iokit_int(svc, "Voltage", 0);
  IOObjectRelease(svc);
  if (cur_cap > 0 && voltage > 0) {
    return (uint32_t)((cur_cap * voltage) / 1000);
  }
  return (uint32_t)cur_cap;
}

// _____________________________________________________________________________________________________________________
double Battery::capacity() {
  io_service_t svc = get_battery_service();
  if (!svc) return 0.0;
  int64_t cur = iokit_int(svc, "CurrentCapacity", 0);
  int64_t max = iokit_int(svc, "MaxCapacity", 1);
  IOObjectRelease(svc);
  if (max <= 0) return 0.0;
  return (double)cur / (double)max;
}

// _____________________________________________________________________________________________________________________
bool Battery::charging() const {
  io_service_t svc = get_battery_service();
  if (!svc) return false;
  bool val = iokit_bool(svc, "IsCharging", false);
  IOObjectRelease(svc);
  return val;
}

// _____________________________________________________________________________________________________________________
bool Battery::discharging() const {
  io_service_t svc = get_battery_service();
  if (!svc) return false;
  bool is_charging = iokit_bool(svc, "IsCharging", false);
  bool external = iokit_bool(svc, "ExternalConnected", false);
  IOObjectRelease(svc);
  return !is_charging && !external;
}

// =====================================================================================================================
// _____________________________________________________________________________________________________________________
std::vector<Battery> getAllBatteries() {
  std::vector<Battery> batteries;
  io_service_t svc = get_battery_service();
  if (svc) {
    bool installed = iokit_bool(svc, "BatteryInstalled", false);
    IOObjectRelease(svc);
    if (installed) {
      batteries.push_back(Battery(0));
    }
  }
  return batteries;
}

}  // namespace hwinfo

#endif  // HWINFO_APPLE
