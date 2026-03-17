// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#include "hwinfo/platform.h"

#ifdef HWINFO_APPLE

#include <sys/mount.h>
#include <sys/param.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/storage/IOBlockStorageDevice.h>
#include <IOKit/IOBSD.h>
#include <CoreFoundation/CoreFoundation.h>

#include <string>
#include <vector>
#include <set>

#include "hwinfo/disk.h"
#include "apple_utils.h"

namespace hwinfo {

// _____________________________________________________________________________________________________________________
const std::string& Disk::vendor() const { return _vendor; }
const std::string& Disk::model() const { return _model; }
const std::string& Disk::serialNumber() const { return _serialNumber; }
int64_t Disk::size_Bytes() const { return _size_Bytes; }

// Helper: walk IOKit registry to find the physical storage device for a BSD name
static bool get_disk_info_for_bsd(const char *bsd_name, std::string &vendor, std::string &model, std::string &serial) {
  // Strip partition suffix: "disk1s1" -> "disk1"
  std::string disk_name = bsd_name;
  // Remove leading /dev/ if present
  if (disk_name.find("/dev/") == 0) {
    disk_name = disk_name.substr(5);
  }
  // Strip 's' partition suffix: disk1s1 -> disk1
  auto spos = disk_name.find('s', 4);
  if (spos != std::string::npos) {
    disk_name = disk_name.substr(0, spos);
  }

  CFMutableDictionaryRef match = IOBSDNameMatching(kIOMasterPortDefault, 0, disk_name.c_str());
  if (!match) return false;

  io_service_t service = IOServiceGetMatchingService(kIOMasterPortDefault, match);
  if (!service) return false;

  // Walk up the registry tree to find IOBlockStorageDevice
  io_service_t parent = service;
  io_service_t current = service;
  IOObjectRetain(current);

  bool found = false;
  for (int depth = 0; depth < 10 && !found; depth++) {
    // Check if this node has "Device Characteristics"
    CFMutableDictionaryRef props = nullptr;
    if (IORegistryEntryCreateCFProperties(current, &props, kCFAllocatorDefault, 0) == kIOReturnSuccess && props) {
      CFDictionaryRef dev_chars = (CFDictionaryRef)CFDictionaryGetValue(props, CFSTR("Device Characteristics"));
      if (dev_chars && CFGetTypeID(dev_chars) == CFDictionaryGetTypeID()) {
        // Extract vendor
        CFStringRef cf_vendor = (CFStringRef)CFDictionaryGetValue(dev_chars, CFSTR("Vendor Name"));
        if (cf_vendor && CFGetTypeID(cf_vendor) == CFStringGetTypeID()) {
          vendor = cf_conv_string(cf_vendor);
        }
        // Extract model
        CFStringRef cf_model = (CFStringRef)CFDictionaryGetValue(dev_chars, CFSTR("Product Name"));
        if (cf_model && CFGetTypeID(cf_model) == CFStringGetTypeID()) {
          model = cf_conv_string(cf_model);
        }
        // Extract serial
        CFStringRef cf_serial = (CFStringRef)CFDictionaryGetValue(dev_chars, CFSTR("Serial Number"));
        if (cf_serial && CFGetTypeID(cf_serial) == CFStringGetTypeID()) {
          serial = cf_conv_string(cf_serial);
        }
        found = true;
      }
      CFRelease(props);
    }

    if (!found) {
      io_service_t next = 0;
      if (IORegistryEntryGetParentEntry(current, kIOServicePlane, &next) != kIOReturnSuccess) {
        break;
      }
      IOObjectRelease(current);
      current = next;
    }
  }

  IOObjectRelease(current);
  if (current != service) {
    IOObjectRelease(service);
  }
  return found;
}

// =====================================================================================================================
// _____________________________________________________________________________________________________________________
std::vector<Disk> getAllDisks() {
  std::vector<Disk> disks;
  std::set<std::string> seen_devices;

  struct statfs *mntbuf = nullptr;
  int count = getmntinfo(&mntbuf, MNT_NOWAIT);

  for (int i = 0; i < count; i++) {
    std::string dev = mntbuf[i].f_mntfromname;
    std::string fstype = mntbuf[i].f_fstypename;

    // Skip pseudo-filesystems
    if (fstype == "devfs" || fstype == "autofs" || fstype == "nullfs" || fstype == "vmhgfs") {
      continue;
    }
    // Skip non-device mounts
    if (dev.find("/dev/") != 0) {
      continue;
    }

    // Normalize to whole disk: /dev/disk1s1 -> disk1
    std::string disk_base = dev.substr(5);
    auto spos = disk_base.find('s', 4);
    if (spos != std::string::npos) {
      disk_base = disk_base.substr(0, spos);
    }

    // Skip duplicates (multiple partitions on same disk)
    if (seen_devices.count(disk_base)) {
      continue;
    }
    seen_devices.insert(disk_base);

    Disk d;

    // Size from statfs
    d._size_Bytes = (int64_t)mntbuf[i].f_blocks * (int64_t)mntbuf[i].f_bsize;

    // Get vendor/model/serial via IOKit
    std::string vendor, model_name, serial;
    if (get_disk_info_for_bsd(disk_base.c_str(), vendor, model_name, serial)) {
      d._vendor = vendor;
      d._model = model_name;
      d._serialNumber = serial;
    } else {
      // Fallback: use the BSD device name
      d._vendor = "Apple";
      d._model = disk_base;
    }

    disks.push_back(d);
  }

  return disks;
}

}  // namespace hwinfo

#endif  // HWINFO_APPLE
