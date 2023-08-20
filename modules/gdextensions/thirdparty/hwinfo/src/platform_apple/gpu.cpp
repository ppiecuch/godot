// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#include "hwinfo/platform.h"

#ifdef HWINFO_APPLE

#include <iostream>

#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLTypes.h>
#include <OpenGL/CGLRenderers.h>
#include <CoreGraphics/CoreGraphics.h>

struct mem_info_t {
  int64_t video_mem;
  int64_t texture_mem;
};

static mem_info_t get_dedicated_video_memory_bytes(bool *err = nullptr) {
  GLint gl_rend_id = 0;
  CGLGetParameter(CGLGetCurrentContext(), kCGLCPCurrentRendererID, &gl_rend_id);

  CGLRendererInfoObj rend_obj = nullptr;
  CGOpenGLDisplayMask disp_mask = CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay);
  GLint rend_nb = 0;
  CGLQueryRendererInfo(disp_mask, &rend_obj, &rend_nb);
  for (GLint r = 0; r < rend_nb; ++r) {
    GLint rend_id = 0;
    if (CGLDescribeRenderer(rend_obj, r, kCGLRPRendererID, &rend_id) != kCGLNoError || rend_id != gl_rend_id) {
      continue;
    }

    // kCGLRPVideoMemoryMegabytes   = 131;
    // kCGLRPTextureMemoryMegabytes = 132;

    GLint vmem, tmem = 0;
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1070
    if (CGLDescribeRenderer(rend_obj, r, kCGLRPVideoMemoryMegabytes, &vmem) != kCGLNoError) {
      std::cerr << "Failed to retrive video memory info for current render device." << std::endl;
    }
    if (CGLDescribeRenderer(rend_obj, r, kCGLRPTextureMemoryMegabytes, &tmem) != kCGLNoError) {
      std::cerr << "Failed to retrive texture memory info for current render device." << std::endl;
    }
#else
    if (CGLDescribeRenderer(rend_obj, r, kCGLRPVideoMemory, &vmem) == kCGLNoError) {
    } else {
      std::cerr << "Failed to retrive video memory info for current render device." << std::endl;
    }
    if (CGLDescribeRenderer(rend_obj, r, kCGLRPTextureMemory, &tmem) == kCGLNoError) {
    } else {
      std::cerr << "Failed to retrive texture memory info for current render device." << std::endl;
    }
#endif
    if (err) {
      *err = false;
    }
    return { vmem, tmem };
  }

  std::cerr << "Current render device not found." << std::endl;

  if (err) {
    *err = true;
  }
  return { 0, 0 };
}

#ifndef USE_OCL

#include <regex>
#include <string>
#include <vector>

#include "hwinfo/gpu.h"

namespace hwinfo {

  std::vector<GPU> getAllGPUs() {
    std::vector<GPU> gpu;
    GPU g;
    const mem_info_t &info = get_dedicated_video_memory_bytes();
    g._total_memory_Bytes = info.video_mem;
    g._texture_memory_Bytes = info.texture_mem;
    gpu.push_back(g);
    return gpu;
  }

}  // namespace hwinfo

#endif  // USE_OCL
#endif  // HWINFO_APPLE
