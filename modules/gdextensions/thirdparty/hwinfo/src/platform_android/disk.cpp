// Copyright Leon Freist
// Android disk implementation via /sys/block/ and statfs

#include "hwinfo/platform.h"

#ifdef HWINFO_ANDROID

#include <sys/statfs.h>
#include <fstream>
#include <string>
#include <vector>

#include "hwinfo/disk.h"

namespace hwinfo {

const std::string& Disk::vendor() const { return _vendor; }
const std::string& Disk::model() const { return _model; }
const std::string& Disk::serialNumber() const { return _serialNumber; }
int64_t Disk::size_Bytes() const { return _size_Bytes; }

std::vector<Disk> getAllDisks() {
  std::vector<Disk> disks;

  // Primary internal storage via statfs
  struct statfs buf;
  if (statfs("/data", &buf) == 0) {
    Disk d;
    d._vendor = "Internal";
    d._model = "Internal Storage";
    d._size_Bytes = (int64_t)buf.f_blocks * (int64_t)buf.f_bsize;
    disks.push_back(d);
  }

  // External/SD card storage
  if (statfs("/sdcard", &buf) == 0 || statfs("/storage/sdcard1", &buf) == 0) {
    Disk d;
    d._vendor = "External";
    d._model = "SD Card";
    d._size_Bytes = (int64_t)buf.f_blocks * (int64_t)buf.f_bsize;
    // Avoid duplicate if /sdcard points to same filesystem
    if (disks.empty() || d._size_Bytes != disks[0]._size_Bytes) {
      disks.push_back(d);
    }
  }

  return disks;
}

}  // namespace hwinfo

#endif  // HWINFO_ANDROID
