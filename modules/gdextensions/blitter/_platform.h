///  Try to get a standard set of platform defines.

#ifndef _platform_h_
#define _platform_h_

#if defined(_AIX)
#undef __AIX__
#define __AIX__     1
#endif
#if defined(__HAIKU__)
#undef __HAIKU__
#define __HAIKU__   1
#endif
#if defined(bsdi) || defined(__bsdi) || defined(__bsdi__)
#undef __BSDI__
#define __BSDI__    1
#endif
#if defined(_arch_dreamcast)
#undef __DREAMCAST__
#define __DREAMCAST__   1
#endif
#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__DragonFly__)
#undef __FREEBSD__
#define __FREEBSD__ 1
#endif
#if defined(hpux) || defined(__hpux) || defined(__hpux__)
#undef __HPUX__
#define __HPUX__    1
#endif
#if defined(sgi) || defined(__sgi) || defined(__sgi__) || defined(_SGI_SOURCE)
#undef __IRIX__
#define __IRIX__    1
#endif
#if (defined(linux) || defined(__linux) || defined(__linux__))
#undef __LINUX__
#define __LINUX__   1
#endif
#if defined(ANDROID) || defined(__ANDROID__)
#undef __ANDROID__
#undef __LINUX__ /* do we need to do this? */
#define __ANDROID__ 1
#endif

#if defined(__APPLE__)
/* lets us know what version of Mac OS X we're compiling on */
#include "AvailabilityMacros.h"
#include "TargetConditionals.h"

/* Fix building with older SDKs that don't define these
   See this for more information:
   https://stackoverflow.com/questions/12132933/preprocessor-macro-for-os-x-targets
*/
#ifndef TARGET_OS_MACCATALYST
#define TARGET_OS_MACCATALYST 0
#endif
#ifndef TARGET_OS_IOS
#define TARGET_OS_IOS 0
#endif
#ifndef TARGET_OS_IPHONE
#define TARGET_OS_IPHONE 0
#endif
#ifndef TARGET_OS_TV
#define TARGET_OS_TV 0
#endif
#ifndef TARGET_OS_SIMULATOR
#define TARGET_OS_SIMULATOR 0
#endif

#if TARGET_OS_TV
#undef __TVOS__
#define __TVOS__ 1
#endif
#if TARGET_OS_IPHONE
/* if compiling for iOS */
#undef __IPHONEOS__
#define __IPHONEOS__ 1
#undef __MACOSX__
#else
/* if not compiling for iOS */
#undef __MACOSX__
#define __MACOSX__  1
#if MAC_OS_X_VERSION_MIN_REQUIRED < 1060
# error SDL for Mac OS X only supports deploying on 10.6 and above.
#endif /* MAC_OS_X_VERSION_MIN_REQUIRED < 1060 */
#endif /* TARGET_OS_IPHONE */
#endif /* defined(__APPLE__) */

#if defined(__NetBSD__)
#undef __NETBSD__
#define __NETBSD__  1
#endif
#if defined(__OpenBSD__)
#undef __OPENBSD__
#define __OPENBSD__ 1
#endif
#if defined(__OS2__) || defined(__EMX__)
#undef __OS2__
#define __OS2__     1
#endif
#if defined(osf) || defined(__osf) || defined(__osf__) || defined(_OSF_SOURCE)
#undef __OSF__
#define __OSF__     1
#endif
#if defined(__QNXNTO__)
#undef __QNXNTO__
#define __QNXNTO__  1
#endif
#if defined(riscos) || defined(__riscos) || defined(__riscos__)
#undef __RISCOS__
#define __RISCOS__  1
#endif
#if defined(__sun) && defined(__SVR4)
#undef __SOLARIS__
#define __SOLARIS__ 1
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
/* Try to find out if we're compiling for WinRT or non-WinRT */
#if defined(_MSC_VER) && defined(__has_include)
#if __has_include(<winapifamily.h>)
#define HAVE_WINAPIFAMILY_H 1
#else
#define HAVE_WINAPIFAMILY_H 0
#endif

/* If _USING_V110_SDK71_ is defined it means we are using the Windows XP toolset. */
#elif defined(_MSC_VER) && (_MSC_VER >= 1700 && !_USING_V110_SDK71_)    /* _MSC_VER == 1700 for Visual Studio 2012 */
#define HAVE_WINAPIFAMILY_H 1
#else
#define HAVE_WINAPIFAMILY_H 0
#endif

#if HAVE_WINAPIFAMILY_H
#include <winapifamily.h>
#define WINAPI_FAMILY_WINRT (!WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP) && WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP))
#else
#define WINAPI_FAMILY_WINRT 0
#endif /* HAVE_WINAPIFAMILY_H */

#if WINAPI_FAMILY_WINRT
#undef __WINRT__
#define __WINRT__ 1
#else
#undef __WINDOWS__
#define __WINDOWS__ 1
#endif
#endif /* defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__) */

#if defined(__WINDOWS__)
#undef __WIN32__
#define __WIN32__ 1
#endif
#if defined(__PSP__)
#undef __PSP__
#define __PSP__ 1
#endif

/* The NACL compiler defines __native_client__ and __pnacl__
 * Ref: http://www.chromium.org/nativeclient/pnacl/stability-of-the-pnacl-bitcode-abi
 */
#if defined(__native_client__)
#undef __LINUX__
#undef __NACL__
#define __NACL__ 1
#endif
#if defined(__pnacl__)
#undef __LINUX__
#undef __PNACL__
#define __PNACL__ 1
/* PNACL with newlib supports static linking only */
#define __SDL_NOGETPROCADDR__
#endif

#if defined(__vita__)
#define __VITA__ 1
#endif

/* CPU architecture */
#if defined(__x86_64__) || defined(_M_X64)
# define CPU_X86_64
# define CPU_X86
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
# define CPU_X86_32
# define CPU_X86
#elif defined(__ARM_ARCH_2__)
# define CPU_ARM2
# define CPU_ARM
#elif defined(__ARM_ARCH_3__) || defined(__ARM_ARCH_3M__)
# define CPU_ARM3
# define CPU_ARM
#elif defined(__ARM_ARCH_4T__) || defined(__TARGET_ARM_4T)
# define CPU_ARM4
# define CPU_ARM
#elif defined(__ARM_ARCH_5_) || defined(__ARM_ARCH_5E_)
# define CPU_ARM5
# define CPU_ARM
#elif defined(__ARM_ARCH_6T2_) || defined(__ARM_ARCH_6T2_)
# define CPU_ARM6T2
# define CPU_ARM
#elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__)
# define CPU_ARM6
# define CPU_ARM
#elif defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
# define CPU_ARM7
# define CPU_ARM
#elif defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
# define CPU_ARM7A
# define CPU_ARM
#elif defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
# define CPU_ARM7R
# define CPU_ARM
#elif defined(__ARM_ARCH_7M__)
# define CPU_ARM7M
# define CPU_ARM
#elif defined(__ARM_ARCH_7S__)
# define CPU_ARM7S
# define CPU_ARM
#elif defined(__aarch64__) || defined(_M_ARM64)
# define CPU_ARM64
# define CPU_ARM
#elif defined(mips) || defined(__mips__) || defined(__mips)
# define CPU_MIPS
#elif defined(__sh__)
# define CPU_SUPERH
#elif defined(__powerpc) || defined(__powerpc__) || defined(__powerpc64__) || defined(__POWERPC__) || defined(__ppc__) || defined(__PPC__) || defined(_ARCH_PPC)
# define CPU_POWERPC
#elif defined(__PPC64__) || defined(__ppc64__) || defined(_ARCH_PPC64)
# define CPU_POWERPC64
# define CPU_POWERPC
#elif defined(__sparc__) || defined(__sparc)
# define CPU_SPARC
#elif defined(__m68k__)
# define CPU_M68K
#else
# define CPU_UNKNOWN
#endif

#include "_begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

// Get the name of the platform.
// Here are the names returned for some (but not all) supported platforms:
// - "Windows"
// - "Mac OS X"
// - "Linux"
// - "iOS"
// - "Android"
// - "Unknown"
extern DECLSPEC const char * SDLCALL SDL_GetPlatform (void);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "_close_code.h"

#endif /* _platform_h_ */

/* vi: set ts=4 sw=4 expandtab: */
