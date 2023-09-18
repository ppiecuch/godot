#pragma once

#if defined(unix) || defined(__unix) || defined(__unix__)
#define HWINFO_UNIX
#endif
#if defined(__APPLE__)
#define HWINFO_APPLE
#endif
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#define HWINFO_WINDOWS
#endif
#if defined(__ANDROID__) || defined(ANDROID)
#define HWINFO_ANDROID
#endif

#if defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(_M_X64)
#define HWINFO_X86_64
#elif defined(__i386__) || defined(_M_IX86)
#define HWINFO_X86_32
#endif
#if defined(HWINFO_X86_64) || defined(HWINFO_X86_32)
#define HWINFO_X86
#endif

#if defined(__arm__) || defined(_M_ARM)
#define HWINFO_ARM32
#elif defined(__aarch64__)
#define HWINFO_ARM64
#endif
#if defined(HWINFO_ARM32) || defined(HWINFO_ARM64)
#define HWINFO_ARM
#endif
