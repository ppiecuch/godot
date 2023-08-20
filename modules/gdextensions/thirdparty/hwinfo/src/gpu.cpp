// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#include <hwinfo/gpu.h>

#include <string>

#ifdef USE_OCL
#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_TARGET_OPENCL_VERSION 200
#include <CL/opencl.hpp>
#endif

namespace hwinfo {

#ifdef USE_OCL

struct GPU_CL {
  int id;
  std::string vendor;
  std::string name;
  std::string driver_version;
  int64_t frequency_MHz;
  int num_cores;
  int64_t memory_Bytes;
};

static std::vector<GPU_CL> get_cpu_cl_data() {
  std::vector<GPU_CL> gpus;
  std::vector<cl::Platform> cl_platforms;
  auto res = cl::Platform::get(&cl_platforms);
  if (res != CL_SUCCESS) {
    return {};
  }
  int id = 0;
  for (auto& clp : cl_platforms) {
    std::vector<cl::Device> cl_devices;
    clp.getDevices(CL_DEVICE_TYPE_GPU, &cl_devices);
    for (auto& cld : cl_devices) {
      GPU_CL gpu;
      cl::Context cl_context(cld);
      gpu.id = id;
      gpu.vendor = cld.getInfo<CL_DEVICE_VENDOR>();
      gpu.name = cld.getInfo<CL_DEVICE_NAME>();
      gpu.driver_version = cld.getInfo<CL_DRIVER_VERSION>();
      gpu.memory_Bytes = cld.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>();
      gpu.frequency_MHz = cld.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>();
      gpu.num_cores = cld.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
      gpus.push_back(gpu);
    }
  }
  return gpus;
}

std::vector<GPU> getAllGPUs() {
  const std::vector<GPU_CL> &ocl = get_cpu_cl_data();
  std::vector<GPU> gpu;
  for(const auto &info : ocl) {
    GPU g;
    g.__available_memory_Bytes = info.memory_Bytes;
    gpu.push_back(g);
  }
  return gpu;
}

#endif

}  // namespace hwinfo
