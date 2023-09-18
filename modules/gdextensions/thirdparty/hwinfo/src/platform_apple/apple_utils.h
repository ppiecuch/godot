// kate: replace-tabs on; tab-indents on; tab-width 2; indent-width 2; indent-mode cstyle;

// Copyright Leon Freist
// Author Leon Freist <freist@informatik.uni-freiburg.de>

#pragma once

#include <CoreFoundation/CoreFoundation.h>
#include <MacTypes.h>


/// Access OSX/iOS system properties

#ifdef __cplusplus
extern "C" {
#endif

char *_get_sysctl_prop(const char *key); // NOTE: returns static buffer

unsigned get_min_required_os_version();
unsigned get_max_allowed_os_version();

const char *get_os_target();
const char* get_computer_name();
const char *get_os_name(int major, int minor);

#ifdef __OBJC__

NSArray* get_all_metal_info();
NSString* get_all_metal_report();

NSArray* get_processes_info(int *error);
NSString* get_processes_report(int *error);

#else

CFArrayRef get_all_metal_info();
CFStringRef get_all_metal_report();

CFArrayRef get_processes_info(int *error);
CFStringRef get_processes_report(int *error);

#endif // __OBJC__

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include <string>

inline std::string cf_conv_string(CFStringRef str) {
  CFIndex length = CFStringGetLength(str);
  std::string result;

  if (!length)
    return result;

  CFIndex max_utf8_length = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8);
  std::unique_ptr<UInt8[]> buffer(new UInt8[max_utf8_length + 1]);
  CFIndex actual_utf8_length;
  CFStringGetBytes(str, CFRangeMake(0, length), kCFStringEncodingUTF8, 0, false, buffer.get(), max_utf8_length, &actual_utf8_length);
  buffer[actual_utf8_length] = 0;
  result.assign((const char *)buffer.get());

  return result;
}

#endif // __cplusplus

inline int64_t cf_conv_i64(CFNumberRef num, int64_t *i) { CFNumberGetValue(num, kCFNumberSInt64Type, i); return *i; }
inline int32_t cf_conv_i32(CFNumberRef num, int32_t *i) { CFNumberGetValue(num, kCFNumberSInt32Type, i); return *i; }
