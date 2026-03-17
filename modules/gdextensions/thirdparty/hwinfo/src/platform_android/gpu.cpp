// Copyright Leon Freist
// Android GPU implementation — limited info available without OpenGL context

#include "hwinfo/platform.h"

#ifdef HWINFO_ANDROID

#include <fstream>
#include <string>
#include <vector>

#include "hwinfo/gpu.h"

namespace hwinfo {

std::vector<GPU> getAllGPUs() {
  std::vector<GPU> gpus;

  // Try to read GPU info from /sys/class/kgsl/ (Qualcomm Adreno)
  std::ifstream f("/sys/class/kgsl/kgsl-3d0/gpu_model");
  if (f) {
    GPU g;
    std::getline(f, g._name);
    g._vendor = "Qualcomm";
    f.close();

    // Max frequency
    std::ifstream freq_f("/sys/class/kgsl/kgsl-3d0/max_gpuclk");
    if (freq_f) {
      int64_t freq = 0;
      freq_f >> freq;
      g._frequencyMHz = freq / 1000000;
    }

    gpus.push_back(g);
    return gpus;
  }

  // Try Mali GPU (/sys/class/misc/mali0/ or /sys/devices/platform/*.gpu/)
  std::ifstream mali_f("/sys/class/misc/mali0/device/gpuinfo");
  if (mali_f) {
    GPU g;
    std::getline(mali_f, g._name);
    g._vendor = "ARM";
    gpus.push_back(g);
    return gpus;
  }

  // Fallback: try reading from /proc/gpu or just report unknown
  GPU g;
  g._vendor = "<unknown>";
  g._name = "<unknown>";
  gpus.push_back(g);
  return gpus;
}

}  // namespace hwinfo

#endif  // HWINFO_ANDROID
