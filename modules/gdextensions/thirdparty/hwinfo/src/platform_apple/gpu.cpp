// kate: replace-tabs on; tab-indents on; tab-width 2; indent-width 2; indent-mode cstyle;

// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#include "hwinfo/hwinfo.h"

#ifdef HWINFO_APPLE

#include <sys/sysctl.h>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include <CoreGraphics/CoreGraphics.h>
#include <IOKit/IOKitLib.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLTypes.h>
#include <OpenGL/CGLRenderers.h>

#include "apple_utils.h"

#ifndef USE_OCL

static bool _get_renderer_property(CGLRendererInfoObj renderer_info, GLint renderer_index, CGLRendererProperty property, GLint *value) {
  CGLError err = CGLDescribeRenderer(renderer_info, renderer_index, property, value);
  if (err != kCGLNoError)
    hwinfo_error("CGLDescribeRenderer failed for property %d: %d %s\n", property, err, CGLErrorString(err));
  return (err == kCGLNoError);
}

#define Attr0 CGLPixelFormatAttribute(0)

static CGLPixelFormatObj _create_pixel_format_for_renderer(CGLRendererInfoObj renderer_info, GLint renderer, bool core) {
  GLint renderer_id;
  CGLPixelFormatAttribute attrs[] = {
    kCGLPFARendererID, Attr0,
    kCGLPFASingleRenderer,
    Attr0, Attr0, /* reserve spots for kCGLPFAOpenGLProfile, kCGLOGLPVersion_3_2_Core */
    Attr0
  };
  CGLPixelFormatObj pixel_format;
  GLint virtual_screens;
  if (core) {
  #if defined(MAC_OS_X_VERSION_10_7) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
    attrs[3] = kCGLPFAOpenGLProfile;
    attrs[4] = CGLPixelFormatAttribute(kCGLOGLPVersion_3_2_Core);
  #else
    return NULL;
  #endif
  }
  if (!_get_renderer_property(renderer_info, renderer, kCGLRPRendererID, &renderer_id)) {
      return NULL;
  }
  attrs[1] = CGLPixelFormatAttribute(renderer_id);
  CGLError err = CGLChoosePixelFormat(attrs, &pixel_format, &virtual_screens);
  if (err != kCGLNoError) {
      pixel_format = NULL;
  }
  return pixel_format;
}

struct mem_info_t {
  int32_t renderer_id;
  int32_t virtual_screen; // virtual screen number
  bool online, accelerated;
  int64_t video_mem;
  int64_t texture_mem;
};

static const char* _get_vendor_ident(GLint rend_id) {
  const GLint VENDOR_ID_SOFTWARE = 0x00020000;
  const GLint VENDOR_ID_AMD = 0x00021000;
  const GLint VENDOR_ID_NVIDIA = 0x00022000;
  const GLint VENDOR_ID_INTEL = 0x00024000;
  const GLint VENDOR_ID_APPLE = 0x00027000;
  const GLint vendor_id = rend_id & kCGLRendererIDMatchingMask & ~0xfff;
  if (vendor_id == VENDOR_ID_SOFTWARE) {
    return "SOFTWARE";
  } else if (vendor_id == VENDOR_ID_AMD) {
    return "AMD";
  } else if (vendor_id == VENDOR_ID_NVIDIA) {
    return "NVIDIA";
  } else if (vendor_id == VENDOR_ID_INTEL) {
    return "INTEL";
  } else if (vendor_id == VENDOR_ID_APPLE) {
    return "APPLE";
  }
  return NULL;
}

static mem_info_t _get_this_renderer_info(const GLint *this_rend_id, const GLint *pos, bool *err = nullptr) {
  static const GLint AllDisplaysMask = 0xFFFFFFFF;
  static const CGOpenGLDisplayMask disp_mask = AllDisplaysMask;
  CGLRendererInfoObj rend = nullptr;
  GLint nrend = 0;
  CGLQueryRendererInfo(disp_mask, &rend, &nrend);
  for (GLint r = 0; r < nrend; ++r) {
    GLint rend_id = 0;
    if (this_rend_id) {
      if (CGLDescribeRenderer(rend, r, kCGLRPRendererID, &rend_id) != kCGLNoError || rend_id != *this_rend_id) {
        continue;
      }
    } else if (pos) {
      if (CGLDescribeRenderer(rend, r, kCGLRPRendererID, &rend_id) != kCGLNoError || r != *pos) {
        continue;
      }
    } else {
      continue;
    }
    GLint online = false, accelerated = false, vmem = 0, tmem = 0;
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_7
    if (CGLDescribeRenderer(rend, r, kCGLRPVideoMemoryMegabytes, &vmem) != kCGLNoError) {
      hwinfo_error("Failed to retrive video memory info for current render device.");
    }
    if (CGLDescribeRenderer(rend, r, kCGLRPTextureMemoryMegabytes, &tmem) != kCGLNoError) {
      hwinfo_error("Failed to retrive texture memory info for current render device.");
    }
#else
    if (CGLDescribeRenderer(rend, r, kCGLRPVideoMemory, &vmem) != kCGLNoError) {
      hwinfo_error("Failed to retrive video memory info for current render device.");
    } else {
      vmem /= 1024 * 1024; // MB
    }
    if (CGLDescribeRenderer(rend, r, kCGLRPTextureMemory, &tmem) != kCGLNoError) {
      hwinfo_error("Failed to retrive texture memory info for current render device.");
    } else {
      vmem /= 1024 * 1024; // MB
    }
#endif
    CGLDescribeRenderer(rend, r, kCGLRPOnline, &online);
    CGLDescribeRenderer(rend, r, kCGLRPAccelerated, &accelerated);

    if (err) {
      *err = false;
    }

    // get corresponding virtual screen
    GLint virtual_screen = 0;
    CGLContextObj context_object = CGLGetCurrentContext();
    if (context_object) {
      CGLPixelFormatObj pixel_format = CGLGetPixelFormat(context_object);
      GLint nscreens = 0; // number of virtual screens
      CGLDescribePixelFormat(pixel_format, 0, kCGLPFAVirtualScreenCount, &nscreens);
      for (GLint i = 0; i != nscreens; ++i) {
        CGLSetVirtualScreen(context_object, i);
        GLint r;
        CGLGetParameter(context_object, kCGLCPCurrentRendererID, &r);
        for (GLint j = 0; j < nrend; ++j) {
          if (rend_id == r) {
            virtual_screen = i;
            break;
          }
        }
      }
    }

    return { rend_id, virtual_screen, bool(online), bool(accelerated), vmem, tmem };
  }

  hwinfo_error("Current render device not found.");

  if (err) {
    *err = true;
  }
  return { -1, -1, false, false, 0, 0 };
}

static mem_info_t _get_current_renderer_info(bool *err = nullptr) {
  GLint curr_rend_id = 0;
  CGLGetParameter(CGLGetCurrentContext(), kCGLCPCurrentRendererID, &curr_rend_id);
  return _get_this_renderer_info(&curr_rend_id, nullptr, err);
}

#define MAKE_FOURCC(ch0, ch1, ch2, ch3) ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) | ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24))

namespace hwinfo {

  struct GPUDescriptor {
    std::string gpu_name;
    std::string gpu_metal_bundle;
    std::string gpu_opengl_bundle;
    std::string gpu_bundle_id;
    uint32_t gpu_vendor_id = 0;
    uint32_t gpu_device_id = 0;
    uint32_t gpu_memory_mb = 0;
    uint32_t gpu_index = 0;
    bool gpu_headless = false;
    uint32_t pci_device = 0;
    uint64_t registry_id;
  };

  static void _initialize_descriptor_from_device_entry_m(GPUDescriptor& desc, io_registry_entry_t service_entry, CFMutableDictionaryRef service_info) {
    static CFStringRef IOMatchCategoryRef = CFSTR("io_match_category");
    static CFStringRef IOAcceleratorRef   = CFSTR("IOAccelerator");
    static CFStringRef CFBundleIdentifier = CFSTR("CFBundleIdentifier");
    static CFStringRef VendorIDRef        = CFSTR("vendor-id");
    static CFStringRef MetalPluginNameRef = CFSTR("MetalPluginName");
    static CFStringRef GLBundleNameRef    = CFSTR("IOGLBundleName");
    static CFStringRef ModelRef           = CFSTR("model");

    io_iterator_t child_iterator;
    if(IORegistryEntryGetChildIterator(service_entry, kIOServicePlane, &child_iterator) == kIOReturnSuccess) {
      io_registry_entry_t child_entry;
      while ((desc.registry_id == 0) && (child_entry = IOIteratorNext(child_iterator))) {
        CFStringRef io_match_category = (CFStringRef)IORegistryEntrySearchCFProperty(child_entry, kIOServicePlane, IOMatchCategoryRef, kCFAllocatorDefault, 0);
        if (io_match_category && (CFGetTypeID(io_match_category) == CFStringGetTypeID()) && (CFStringCompare(io_match_category, IOAcceleratorRef, 0) == kCFCompareEqualTo)) {
          CFMutableDictionaryRef properties = nullptr;
          if (kIOReturnSuccess == IORegistryEntryCreateCFProperties(child_entry, &properties, kCFAllocatorDefault, kIORegistryIterateRecursively)) {
            kern_return_t result = IORegistryEntryGetRegistryEntryID(child_entry, &desc.registry_id);
            if (result != kIOReturnSuccess) {
              hwinfo_error("Failed to access IORegistryEntryGetRegistryEntryID");
              continue;
            }
            CFStringRef bundle_id = (CFStringRef)CFDictionaryGetValue(properties, CFBundleIdentifier);
            if (bundle_id && (CFGetTypeID(bundle_id) == CFStringGetTypeID())) {
              desc.gpu_bundle_id = cf_conv_string(bundle_id);
            }
            {
              char buffer[0x40] = { 0 };
              size_t buffer_size = sizeof(buffer) / sizeof(buffer[0]);
              if (0 == sysctlbyname("hw.targettype", &buffer, &buffer_size, NULL, 0)) {
                uint32_t value = MAKE_FOURCC(buffer[0], buffer[1], buffer[2], buffer[3]);
                desc.gpu_device_id = value;
              }
            }

            CFDataRef vendor_id = (CFDataRef)CFDictionaryGetValue(properties, VendorIDRef);
            if (vendor_id && (CFGetTypeID(vendor_id) == CFDataGetTypeID())) {
              const uint32_t* value = reinterpret_cast<const uint32_t*>(CFDataGetBytePtr(vendor_id));
              desc.gpu_vendor_id = *value;
            }

            CFStringRef metal_plugin_name = (CFStringRef)CFDictionaryGetValue(properties, MetalPluginNameRef);
            if (metal_plugin_name && (CFGetTypeID(metal_plugin_name) == CFStringGetTypeID())) {
              desc.gpu_metal_bundle = cf_conv_string(metal_plugin_name);
            }

            CFStringRef gl_bundle_name = (CFStringRef)CFDictionaryGetValue(properties, GLBundleNameRef);
            if (gl_bundle_name && (CFGetTypeID(gl_bundle_name) == CFStringGetTypeID())) {
              desc.gpu_opengl_bundle = cf_conv_string(gl_bundle_name);
            }
            CFStringRef model_name = (CFStringRef)CFDictionaryGetValue(properties, ModelRef);
            if (model_name && (CFGetTypeID(model_name) == CFStringGetTypeID())) {
              desc.gpu_name = cf_conv_string(model_name);
            }
            {
              uint64_t value = 0;
              size_t value_size = sizeof(value);
              if (0 == sysctlbyname("hw.memsize", &value, &value_size, nullptr, 0)) {
                desc.gpu_memory_mb = uint64_t(float(value) * 0.75f) / 1024 / 1024;
              }
            }
          }
          if (properties) {
            CFRelease(properties);
          }
        }
        if (io_match_category) {
          CFRelease(io_match_category);
        }
        IOObjectRelease(child_entry);
      }
      IOObjectRelease(child_iterator);
    }
  }

#ifdef HWINFO_X86
  void initialize_descriptor_from_device_entry(GPUDescriptor& desc, io_registry_entry_t service_entry, CFMutableDictionaryRef service_info) {
    IOObjectRetain(service_entry);
    desc.pci_device = uint32_t(service_entry);

    static CFStringRef ModelRef = CFSTR("model");
    const CFDataRef model = (const CFDataRef)CFDictionaryGetValue(service_info, ModelRef);
    if (model) {
      if (CFGetTypeID(model) == CFDataGetTypeID()) {
        CFStringRef ModelName = CFStringCreateFromExternalRepresentation(kCFAllocatorDefault, model, kCFStringEncodingASCII);
        desc.gpu_name = cf_conv_string(ModelName);
      } else {
        CFRelease(model);
      }
    }

    static CFStringRef DeviceIDRef = CFSTR("device-id");
    const CFDataRef DeviceID = (const CFDataRef)CFDictionaryGetValue(service_info, DeviceIDRef);
    if (DeviceID && CFGetTypeID(DeviceID) == CFDataGetTypeID()) {
      const uint32_t* value = reinterpret_cast<const uint32_t*>(CFDataGetBytePtr(DeviceID));
      desc.gpu_device_id = *value;
    }

    static CFStringRef VendorIDRef = CFSTR("vendor-id");
    const CFDataRef VendorID = (const CFDataRef)CFDictionaryGetValue(service_info, VendorIDRef);
    if (DeviceID && CFGetTypeID(DeviceID) == CFDataGetTypeID())
    {
      const uint32_t* Value = reinterpret_cast<const uint32_t*>(CFDataGetBytePtr(VendorID));
      desc.gpu_vendor_id = *Value;
    }

    static CFStringRef HeadlessRef = CFSTR("headless");
    const CFBooleanRef headless = (const CFBooleanRef)CFDictionaryGetValue(service_info, HeadlessRef);
    if(headless && CFGetTypeID(headless) == CFBooleanGetTypeID()) {
      desc.gpu_headless = (bool)CFBooleanGetValue(headless);
    }

    static CFStringRef VRAMTotal = CFSTR("VRAM,totalMB");
    CFTypeRef vram = IORegistryEntrySearchCFProperty(service_entry, kIOServicePlane, VRAMTotal, kCFAllocatorDefault, kIORegistryIterateRecursively);
    if (vram) {
      if (CFGetTypeID(vram) == CFDataGetTypeID()) {
        const uint32_t* value = reinterpret_cast<const uint32_t*>(CFDataGetBytePtr((CFDataRef)vram));
        desc.gpu_memory_mb = *value;
      } else if (CFGetTypeID(vram) == CFNumberGetTypeID()) {
        CFNumberGetValue((CFNumberRef)vram, kCFNumberSInt32Type, &desc.gpu_memory_mb);
      }
      CFRelease(vram);
    }

    static CFStringRef MetalPluginName = CFSTR("MetalPluginName");
    const CFStringRef metal_lib_name = (const CFStringRef)IORegistryEntrySearchCFProperty(service_entry, kIOServicePlane, MetalPluginName, kCFAllocatorDefault, kIORegistryIterateRecursively);
    if (metal_lib_name) {
      if (CFGetTypeID(metal_lib_name) == CFStringGetTypeID()) {
        desc.gpu_metal_bundle = cf_conv_string(metal_lib_name);
      } else {
        CFRelease(metal_lib_name);
      }
    }

    CFStringRef bundle_id = nullptr;

    static CFStringRef CFBundleIdentifier = CFSTR("CFBundleIdentifier");

    io_iterator_t child_iterator;
    if (IORegistryEntryGetChildIterator(service_entry, kIOServicePlane, &child_iterator) == kIOReturnSuccess) {
      io_registry_entry_t child_entry;
      while ((bundle_id == nullptr) && (child_entry = IOIteratorNext(child_iterator))) {
        static CFStringRef IOMatchCategoryRef = CFSTR("io_match_category");
        CFStringRef io_match_category = (CFStringRef)IORegistryEntrySearchCFProperty(child_entry, kIOServicePlane, IOMatchCategoryRef, kCFAllocatorDefault, 0);
        static CFStringRef IOAcceleratorRef = CFSTR("IOAccelerator");
        if (io_match_category && CFGetTypeID(io_match_category) == CFStringGetTypeID() && CFStringCompare(io_match_category, IOAcceleratorRef, 0) == kCFCompareEqualTo) {
          bundle_id = (CFStringRef)IORegistryEntrySearchCFProperty(child_entry, kIOServicePlane, CFBundleIdentifier, kCFAllocatorDefault, 0);
          kern_return_t result = IORegistryEntryGetRegistryEntryID(child_entry, &desc.registry_id);
          if (result != kIOReturnSuccess) {
            hwinfo_error("Failed to access IORegistryEntryGetRegistryEntryID");
          }
        }
        if (io_match_category) {
          CFRelease(io_match_category);
        }
        IOObjectRelease(child_entry);
      }

      IOObjectRelease(child_iterator);
    }

    if (bundle_id == nullptr) {
      bundle_id = (CFStringRef)IORegistryEntrySearchCFProperty(service_entry, kIOServicePlane, CFBundleIdentifier, kCFAllocatorDefault, kIORegistryIterateRecursively);
    }

    if (bundle_id) {
      if(CFGetTypeID(bundle_id) == CFStringGetTypeID()) {
        desc.gpu_bundle_id = cf_conv_string(bundle_id);
      } else {
        CFRelease(bundle_id);
      }
    }
  }
#endif // HWINFO_X86

# define PLATFORM_IOSERVICE_MATCHING_NAME_ARM64  "AppleARMIODevice"
# define PLATFORM_IOSERVICE_MATCHING_NAME_X86    "IOPCIDevice"
# define PLATFORM_CLASS_CODE_NAME_ARM64          "device_type"
# define PLATFORM_CLASS_CODE_NAME_X86            "class-code"

#if defined(HWINFO_ARM)

  inline bool is_running_on_apple_silicon() { return true; }

  static const char* get_io_service_matching_name() { return PLATFORM_IOSERVICE_MATCHING_NAME_ARM64; }
  static const char* get_class_code() { return PLATFORM_CLASS_CODE_NAME_ARM64; }

#elif defined(HWINFO_X86)

# define MAC_PROCESS_TYPE_NATIVE     0
# define MAC_PROCESS_TYPE_TRANSLATED 1
# define MAC_PROCESS_TYPE_UNKNOWN   -1

  static int get_process_translation_type() {
    static int mac_process_type = MAC_PROCESS_TYPE_UNKNOWN;
    if (mac_process_type == MAC_PROCESS_TYPE_UNKNOWN) {
      int v = 0;
      size_t size = sizeof(v);
      if (sysctlbyname("sysctl.proc_translated", &v, &size, NULL, 0) == -1) {
        if (errno == ENOENT) {
          v = MAC_PROCESS_TYPE_NATIVE;
        }
      }
      mac_process_type = v;
    }
    return mac_process_type;
  }

  inline bool is_running_on_apple_silicon() { return MAC_PROCESS_TYPE_TRANSLATED == get_process_translation_type(); }

  static const char* get_io_service_matching_name() { return is_running_on_apple_silicon() ? PLATFORM_IOSERVICE_MATCHING_NAME_ARM64 : PLATFORM_IOSERVICE_MATCHING_NAME_X86; }
  static const char* get_class_code() { return is_running_on_apple_silicon() ? PLATFORM_CLASS_CODE_NAME_ARM64 : PLATFORM_CLASS_CODE_NAME_X86; }
#endif // HWINFO_ARM

  // Enumerate the GPUs via IOKit to avoid dragging in OpenGL
  std::vector<GPUDescriptor> _enumarate_gpu_hw() {
    std::vector<GPUDescriptor> current_gpus;
    CFStringRef class_code_ref = CFStringCreateWithCString(kCFAllocatorDefault, get_class_code(), kCFStringEncodingUTF8);
    io_iterator_t io_iter;
    CFMutableDictionaryRef match_dictionary = IOServiceMatching(get_io_service_matching_name());
    if(IOServiceGetMatchingServices(kIOMasterPortDefault, match_dictionary, &io_iter) == kIOReturnSuccess) {
      uint32_t index = 0;
      io_registry_entry_t service_entry;
      while((service_entry = IOIteratorNext(io_iter))) {
        CFMutableDictionaryRef service_info;
        if(IORegistryEntryCreateCFProperties(service_entry, &service_info, kCFAllocatorDefault, kNilOptions) == kIOReturnSuccess) {
          const CFDataRef class_code = (const CFDataRef)CFDictionaryGetValue(service_info, class_code_ref);
          if(class_code && CFGetTypeID(class_code) == CFDataGetTypeID()) {
            if (is_running_on_apple_silicon()) {
              const char* class_code_value = reinterpret_cast<const char*>(CFDataGetBytePtr(class_code));
              if (!strncasecmp(class_code_value, "sgx", 3)) {
                GPUDescriptor desc;
                _initialize_descriptor_from_device_entry_m(desc, service_entry, service_info);
                if (!desc.gpu_metal_bundle.empty()) {
                  desc.gpu_index = index++;
                  current_gpus.push_back(desc);
                }
              }
            }
#ifdef HWINFO_X86
            else {
              const uint32_t* class_code_value = reinterpret_cast<const uint32_t*>(CFDataGetBytePtr(class_code));
              // GPUs are class-code 0x30000 || 0x38000
              if (class_code_value && (*class_code_value == 0x30000 || *class_code_value == 0x38000)) {
                GPUDescriptor desc;
                initialize_descriptor_from_device_entry(desc, service_entry, service_info);
                if (!desc.gpu_metal_bundle.empty()) {
                  desc.gpu_index = index++;
                  current_gpus.push_back(desc);
                }
              }
            }
#endif // HWINFO_X86
          }
          CFRelease(service_info);
        }
        IOObjectRelease(service_entry);
      }
      IOObjectRelease(io_iter);
    }
    CFRelease(class_code_ref);
    return current_gpus;
  }

  std::vector<GPU> getAllGPUs() {

    register_property(GPU_TEXTURE_MEMORY_MB, [](PropertyArg arg, int64_t *i, float *f, std::string *s) {
      const int index = arg;
      *i = _get_this_renderer_info(nullptr, &index).texture_mem;
    });
    register_property(GPU_NUM_DEVICES, [](PropertyArg arg, int64_t *i, float *f, std::string *s) {
      static const GLint AllDisplaysMask = 0xFFFFFFFF;
      static const CGOpenGLDisplayMask disp_mask = AllDisplaysMask;
      CGLRendererInfoObj rend = nullptr;
      GLint nrend = 0;
      CGLQueryRendererInfo(disp_mask, &rend, &nrend);
      *i = nrend;
    });
    register_property(GPU_NUM_VIRTUAL_SCREENS, [](PropertyArg arg, int64_t *i, float *f, std::string *s) { // number of virtual screens
      CGLContextObj context_object = CGLGetCurrentContext();
      if (context_object) {
        CGLPixelFormatObj pixel_format = CGLGetPixelFormat(context_object);
        GLint nscreens = 0;
        CGLDescribePixelFormat(pixel_format, 0, kCGLPFAVirtualScreenCount, &nscreens);
        *i = nscreens;
      }
    });

    register_property(GPU_SUMMARY_REPORT, [](PropertyArg arg, int64_t *i, float *f, std::string *s) {
      *s = cf_conv_string(get_all_metal_report());
    });

    std::vector<GPU> gpu;
    std::vector<GPUDescriptor> all = _enumarate_gpu_hw();
    for (size_t i = 0; i < all.size(); i++) {
      const GPUDescriptor &dev = all[i];
      GPU g;
      g._id = dev.gpu_device_id;
      switch(dev.gpu_vendor_id) {
        case 0x1002: g._vendor = "AMD"; break;
        case 0x8086: g._vendor = "INTEL"; break;
        case 0x10DE: g._vendor = "NVIDIA"; break;
        case 0x106B: g._vendor = "APPLE"; break;
        default: {
          static const GLint AllDisplaysMask = 0xFFFFFFFF;
          static const CGOpenGLDisplayMask disp_mask = AllDisplaysMask;
          CGLRendererInfoObj rend = nullptr;
          GLint nrend = 0;
          CGLQueryRendererInfo(disp_mask, &rend, &nrend);
          if (i < nrend) {
            GLint rend_id = 0;
            if (CGLDescribeRenderer(rend, i, kCGLRPRendererID, &rend_id) != kCGLNoError) {
              continue;
            }
            if (rend_id) {
              g._vendor = _get_vendor_ident(rend_id);
            }
          }
        }
      }
      g._totalMemoryMBytes = dev.gpu_memory_mb;
      gpu.push_back(g);
    }
    return gpu;
  }

}  // namespace hwinfo

#endif  // USE_OCL
#endif  // HWINFO_APPLE
