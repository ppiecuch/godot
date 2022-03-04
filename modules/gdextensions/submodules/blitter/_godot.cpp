/*************************************************************************/
/*  _godot.cpp                                                           */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "_error.h"

#include "core/error_macros.h"
#include "core/os/memory.h"
#include "core/os/os.h"

#include <string.h>

#define ERR_MAX_STRLEN 128

// This file defines a structure that carries language-independent error messages
typedef struct SDL_error {
	int error; // This is a numeric value corresponding to the current error
	char str[ERR_MAX_STRLEN];
} SDL_error;

// Routine to get the thread-specific error variable
static SDL_error *_GetErrBuf(void) {
	// Non-thread-safe global error variable
	static SDL_error SDL_global_error;
	return &SDL_global_error;
}

extern "C" int SDL_Error(SDL_errorcode code) {
	switch (code) {
		case SDL_ENOMEM:
			return SDL_SetError("Out of memory");
		case SDL_EFREAD:
			return SDL_SetError("Error reading from datastream");
		case SDL_EFWRITE:
			return SDL_SetError("Error writing to datastream");
		case SDL_EFSEEK:
			return SDL_SetError("Error seeking in datastream");
		case SDL_UNSUPPORTED:
			return SDL_SetError("That operation is not supported");
		default:
			return SDL_SetError("Unknown SDL error");
	}
}

extern "C" int SDL_SetError(SDL_PRINTF_FORMAT_STRING const char *fmt, ...) {
	// Ignore call if invalid format pointer was passed
	if (fmt != NULL) {
		va_list ap;
		SDL_error *error = _GetErrBuf();
		error->error = 1; // mark error as valid
		va_start(ap, fmt);
		SDL_vsnprintf(error->str, ERR_MAX_STRLEN, fmt, ap);
		va_end(ap);
		ERR_PRINT(error->str);
	}
	return -1;
}

extern "C" void SDL_assert(SDL_bool cond) {
	CRASH_COND(cond);
}

// Memory management functions for SDL

extern "C" void *SDL_malloc(size_t size) {
	return memalloc(size);
}

extern "C" void *SDL_calloc(size_t nmemb, size_t size) {
	return memalloc(nmemb * size);
}

extern "C" void *SDL_realloc(void *mem, size_t size) {
	return memrealloc(mem, size);
}

extern "C" void SDL_free(void *mem) {
	memfree(mem);
}

extern "C" void *SDL_memset(SDL_OUT_BYTECAP(len) void *dst, int c, size_t len) {
	return memset(dst, c, len);
}

extern "C" void *SDL_memcpy(SDL_OUT_BYTECAP(len) void *dst, SDL_IN_BYTECAP(len) const void *src, size_t len) {
	return memcpy(dst, src, len);
}

extern "C" void *SDL_memmove(SDL_OUT_BYTECAP(len) void *dst, SDL_IN_BYTECAP(len) const void *src, size_t len) {
	return memmove(dst, src, len);
}

extern "C" int SDL_memcmp(const void *s1, const void *s2, size_t len) {
	return memcmp(s1, s2, len);
}

// Retrieve a variable named "name" from the environment

extern "C" const char *SDL_getenv(const char *name) {
	return OS::get_singleton()->get_environment(name).ascii().c_str();
}

// Print/scan string functions

#if defined(MINGW_ENABLED) || defined(_MSC_VER) && _MSC_VER < 1900
#define gd_vsnprintf(m_buffer, m_count, m_format, m_args_copy) vsnprintf_s(m_buffer, m_count, _TRUNCATE, m_format, m_args_copy)
#define gd_vscprintf(m_format, m_args_copy) _vscprintf(m_format, m_args_copy)
#else
#define gd_vsnprintf(m_buffer, m_count, m_format, m_args_copy) vsnprintf(m_buffer, m_count, m_format, m_args_copy)
#define gd_vscprintf(m_format, m_args_copy) vsnprintf(NULL, 0, m_format, m_args_copy)
#endif

extern "C" int SDL_sscanf(const char *text, SDL_SCANF_FORMAT_STRING const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int rc = vsscanf(text, fmt, ap);
    va_end(ap);
    return rc;
}

extern "C" int SDL_vsnprintf(SDL_OUT_Z_CAP(maxlen) char *text, size_t maxlen, const char *fmt, va_list ap) {
	return gd_vsnprintf(text, maxlen, fmt, ap);
}

// CPU feature detection

#define CPU_HAS_RDTSC (1 << 0)
#define CPU_HAS_ALTIVEC (1 << 1)
#define CPU_HAS_MMX (1 << 2)
#define CPU_HAS_3DNOW (1 << 3)
#define CPU_HAS_SSE (1 << 4)
#define CPU_HAS_SSE2 (1 << 5)
#define CPU_HAS_SSE3 (1 << 6)
#define CPU_HAS_SSE41 (1 << 7)
#define CPU_HAS_SSE42 (1 << 8)
#define CPU_HAS_AVX (1 << 9)
#define CPU_HAS_AVX2 (1 << 10)
#define CPU_HAS_NEON (1 << 11)
#define CPU_HAS_AVX512F (1 << 12)
#define CPU_HAS_ARM_SIMD (1 << 13)

#if defined(__MACOSX__) && (defined(__ppc__) || defined(__ppc64__))
#include <sys/sysctl.h> /* For AltiVec check */
#elif defined(__OpenBSD__) && defined(__powerpc__)
#include <machine/cpu.h>
#include <sys/param.h>
#include <sys/sysctl.h> /* For AltiVec check */
#elif defined(__FreeBSD__) && defined(__powerpc__)
#include <machine/cpu.h>
#include <sys/auxv.h>
#endif

#if (defined(__LINUX__) || defined(__ANDROID__)) && defined(__arm__)
#include <elf.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#if defined(__ANDROID__) && defined(__arm__) && !defined(HAVE_GETAUXVAL)
#include <cpu-features.h>
#endif

#if defined(HAVE_GETAUXVAL)
#include <sys/auxv.h>
#endif

#ifndef AT_HWCAP
#define AT_HWCAP 16
#endif
#ifndef AT_PLATFORM
#define AT_PLATFORM 15
#endif
#ifndef HWCAP_NEON
#define HWCAP_NEON (1 << 12)
#endif
#endif

static int CPU_haveCPUID(void) {
	int has_CPUID = 0;

#if (defined(__GNUC__) || defined(__llvm__)) && defined(__i386__)
	__asm__(
			"        pushfl                      # Get original EFLAGS             \n"
			"        popl    %%eax                                                 \n"
			"        movl    %%eax,%%ecx                                           \n"
			"        xorl    $0x200000,%%eax     # Flip ID bit in EFLAGS           \n"
			"        pushl   %%eax               # Save new EFLAGS value on stack  \n"
			"        popfl                       # Replace current EFLAGS value    \n"
			"        pushfl                      # Get new EFLAGS                  \n"
			"        popl    %%eax               # Store new EFLAGS in EAX         \n"
			"        xorl    %%ecx,%%eax         # Can not toggle ID bit,          \n"
			"        jz      1f                  # Processor=80486                 \n"
			"        movl    $1,%0               # We have CPUID support           \n"
			"1:                                                                    \n"
			: "=m"(has_CPUID)
			:
			: "%eax", "%ecx");
#elif (defined(__GNUC__) || defined(__llvm__)) && defined(__x86_64__)
	/* Technically, if this is being compiled under __x86_64__ then it has
	   CPUid by definition.  But it's nice to be able to prove it.  :)      */
	__asm__(
			"        pushfq                      # Get original EFLAGS             \n"
			"        popq    %%rax                                                 \n"
			"        movq    %%rax,%%rcx                                           \n"
			"        xorl    $0x200000,%%eax     # Flip ID bit in EFLAGS           \n"
			"        pushq   %%rax               # Save new EFLAGS value on stack  \n"
			"        popfq                       # Replace current EFLAGS value    \n"
			"        pushfq                      # Get new EFLAGS                  \n"
			"        popq    %%rax               # Store new EFLAGS in EAX         \n"
			"        xorl    %%ecx,%%eax         # Can not toggle ID bit,          \n"
			"        jz      1f                  # Processor=80486                 \n"
			"        movl    $1,%0               # We have CPUID support           \n"
			"1:                                                                    \n"
			: "=m"(has_CPUID)
			:
			: "%rax", "%rcx");
#elif (defined(_MSC_VER) && defined(_M_IX86)) || defined(__WATCOMC__)
	__asm {
        pushfd                      ; Get original EFLAGS
        pop     eax
        mov     ecx, eax
        xor     eax, 200000h        ; Flip ID bit in EFLAGS
        push    eax                 ; Save new EFLAGS value on stack
        popfd                       ; Replace current EFLAGS value
        pushfd                      ; Get new EFLAGS
        pop     eax                 ; Store new EFLAGS in EAX
        xor     eax, ecx            ; Can not toggle ID bit,
        jz      done                ; Processor=80486
        mov     has_CPUID,1         ; We have CPUID support
done:
	}
#elif defined(_MSC_VER) && defined(_M_X64)
	has_CPUID = 1;
#elif defined(__sun) && defined(__i386)
	__asm(
			"       pushfl                 \n"
			"       popl    %eax           \n"
			"       movl    %eax,%ecx      \n"
			"       xorl    $0x200000,%eax \n"
			"       pushl   %eax           \n"
			"       popfl                  \n"
			"       pushfl                 \n"
			"       popl    %eax           \n"
			"       xorl    %ecx,%eax      \n"
			"       jz      1f             \n"
			"       movl    $1,-8(%ebp)    \n"
			"1:                            \n");
#elif defined(__sun) && defined(__amd64)
	__asm(
			"       pushfq                 \n"
			"       popq    %rax           \n"
			"       movq    %rax,%rcx      \n"
			"       xorl    $0x200000,%eax \n"
			"       pushq   %rax           \n"
			"       popfq                  \n"
			"       pushfq                 \n"
			"       popq    %rax           \n"
			"       xorl    %ecx,%eax      \n"
			"       jz      1f             \n"
			"       movl    $1,-8(%rbp)    \n"
			"1:                            \n");
#endif
	return has_CPUID;
}

#if (defined(__GNUC__) || defined(__llvm__)) && defined(__i386__)
#define cpuid(func, a, b, c, d)                  \
	__asm__ __volatile__(                        \
			"        pushl %%ebx        \n"      \
			"        xorl %%ecx,%%ecx   \n"      \
			"        cpuid              \n"      \
			"        movl %%ebx, %%esi  \n"      \
			"        popl %%ebx         \n"      \
			: "=a"(a), "=S"(b), "=c"(c), "=d"(d) \
			: "a"(func))
#elif (defined(__GNUC__) || defined(__llvm__)) && defined(__x86_64__)
#define cpuid(func, a, b, c, d)                  \
	__asm__ __volatile__(                        \
			"        pushq %%rbx        \n"      \
			"        xorq %%rcx,%%rcx   \n"      \
			"        cpuid              \n"      \
			"        movq %%rbx, %%rsi  \n"      \
			"        popq %%rbx         \n"      \
			: "=a"(a), "=S"(b), "=c"(c), "=d"(d) \
			: "a"(func))
#elif (defined(_MSC_VER) && defined(_M_IX86)) || defined(__WATCOMC__)
#define cpuid(func, a, b, c, d) \
	__asm { \
        __asm mov eax, func \
        __asm xor ecx, ecx \
        __asm cpuid \
        __asm mov a, eax \
        __asm mov b, ebx \
        __asm mov c, ecx \
        __asm mov d, edx                   \
	}
#elif defined(_MSC_VER) && defined(_M_X64)
#define cpuid(func, a, b, c, d) \
	{                           \
		int CPUInfo[4];         \
		__cpuid(CPUInfo, func); \
		a = CPUInfo[0];         \
		b = CPUInfo[1];         \
		c = CPUInfo[2];         \
		d = CPUInfo[3];         \
	}
#else
#define cpuid(func, a, b, c, d) \
	do {                        \
		a = b = c = d = 0;      \
		(void)a;                \
		(void)b;                \
		(void)c;                \
		(void)d;                \
	} while (0)
#endif

static int CPU_CPUIDFeatures[4];
static int CPU_CPUIDMaxFunction = 0;
static SDL_bool CPU_OSSavesYMM = SDL_FALSE;
static SDL_bool CPU_OSSavesZMM = SDL_FALSE;

static void CPU_calcCPUIDFeatures(void) {
	static SDL_bool checked = SDL_FALSE;
	if (!checked) {
		checked = SDL_TRUE;
		if (CPU_haveCPUID()) {
			int a, b, c, d;
			cpuid(0, a, b, c, d);
			CPU_CPUIDMaxFunction = a;
			if (CPU_CPUIDMaxFunction >= 1) {
				cpuid(1, a, b, c, d);
				CPU_CPUIDFeatures[0] = a;
				CPU_CPUIDFeatures[1] = b;
				CPU_CPUIDFeatures[2] = c;
				CPU_CPUIDFeatures[3] = d;

				// Check to make sure we can call xgetbv
				if (c & 0x08000000) {
					// Call xgetbv to see if YMM (etc) register state is saved
#if (defined(__GNUC__) || defined(__llvm__)) && (defined(__i386__) || defined(__x86_64__))
					__asm__(".byte 0x0f, 0x01, 0xd0"
							: "=a"(a)
							: "c"(0)
							: "%edx");
#elif defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64)) && (_MSC_FULL_VER >= 160040219) /* VS2010 SP1 */
					a = (int)_xgetbv(0);
#elif (defined(_MSC_VER) && defined(_M_IX86)) || defined(__WATCOMC__)
					__asm
					{
                        xor ecx, ecx
                        _asm _emit 0x0f _asm _emit 0x01 _asm _emit 0xd0
                        mov a, eax
					}
#endif
					CPU_OSSavesYMM = ((a & 6) == 6) ? SDL_TRUE : SDL_FALSE;
					CPU_OSSavesZMM = (CPU_OSSavesYMM && ((a & 0xe0) == 0xe0)) ? SDL_TRUE : SDL_FALSE;
				}
			}
		}
	}
}

static int CPU_haveAltiVec(void) {
	volatile int altivec = 0;
#if (defined(__MACOSX__) && (defined(__ppc__) || defined(__ppc64__))) || (defined(__OpenBSD__) && defined(__powerpc__))
#ifdef __OpenBSD__
	int selectors[2] = { CTL_MACHDEP, CPU_ALTIVEC };
#else
	int selectors[2] = { CTL_HW, HW_VECTORUNIT };
#endif
	int hasVectorUnit = 0;
	size_t length = sizeof(hasVectorUnit);
	int error = sysctl(selectors, 2, &hasVectorUnit, &length, NULL, 0);
	if (0 == error) {
		altivec = (hasVectorUnit != 0);
	}
#elif defined(__FreeBSD__) && defined(__powerpc__)
	unsigned long cpufeatures = 0;
	elf_aux_info(AT_HWCAP, &cpufeatures, sizeof(cpufeatures));
	altivec = cpufeatures & PPC_FEATURE_HAS_ALTIVEC;
	return altivec;
#endif
	return altivec;
}

#if (defined(__ARM_ARCH) && (__ARM_ARCH >= 6)) || defined(__aarch64__)
static int CPU_haveARMSIMD(void) {
	return 1;
}

#elif !defined(__arm__)
static int CPU_haveARMSIMD(void) {
	return 0;
}

#elif defined(__LINUX__)
static int CPU_haveARMSIMD(void) {
	int arm_simd = 0;
	int fd;

	fd = open("/proc/self/auxv", O_RDONLY | O_CLOEXEC);
	if (fd >= 0) {
		Elf32_auxv_t aux;
		while (read(fd, &aux, sizeof aux) == sizeof aux) {
			if (aux.a_type == AT_PLATFORM) {
				const char *plat = (const char *)aux.a_un.a_val;
				if (plat) {
					arm_simd = SDL_strncmp(plat, "v6l", 3) == 0 ||
							SDL_strncmp(plat, "v7l", 3) == 0;
				}
			}
		}
		close(fd);
	}
	return arm_simd;
}

#else
static int CPU_haveARMSIMD(void) {
#warning SDL_HasARMSIMD is not implemented for this ARM platform. Write me.
	return 0;
}
#endif

#if defined(__LINUX__) && defined(__arm__) && !defined(HAVE_GETAUXVAL)
static int readProcAuxvForNeon(void) {
	int neon = 0;
	int fd;

	fd = open("/proc/self/auxv", O_RDONLY | O_CLOEXEC);
	if (fd >= 0) {
		Elf32_auxv_t aux;
		while (read(fd, &aux, sizeof(aux)) == sizeof(aux)) {
			if (aux.a_type == AT_HWCAP) {
				neon = (aux.a_un.a_val & HWCAP_NEON) == HWCAP_NEON;
				break;
			}
		}
		close(fd);
	}
	return neon;
}
#endif

static int CPU_haveNEON(void) {
// The way you detect NEON is a privileged instruction on ARM, so you have
// query the OS kernel in a platform-specific way. :/
#if (defined(__WINDOWS__) || defined(__WINRT__)) && (defined(_M_ARM) || defined(_M_ARM64))
// Visual Studio, for ARM, doesn't define __ARM_ARCH. Handle this first.
// Seems to have been removed
#if !defined(PF_ARM_NEON_INSTRUCTIONS_AVAILABLE)
#define PF_ARM_NEON_INSTRUCTIONS_AVAILABLE 19
#endif
	// All WinRT ARM devices are required to support NEON, but just in case.
	return IsProcessorFeaturePresent(PF_ARM_NEON_INSTRUCTIONS_AVAILABLE) != 0;
#elif (defined(__ARM_ARCH) && (__ARM_ARCH >= 8)) || defined(__aarch64__)
	return 1; /* ARMv8 always has non-optional NEON support. */
#elif __VITA__
	return 1;
#elif defined(__APPLE__) && defined(__ARM_ARCH) && (__ARM_ARCH >= 7)
	// (note that sysctlbyname("hw.optional.neon") doesn't work!)
	return 1; // all Apple ARMv7 chips and later have NEON.
#elif defined(__APPLE__)
	return 0; // assume anything else from Apple doesn't have NEON.
#elif !defined(__arm__)
	return 0; // not an ARM CPU at all.
#elif defined(__OpenBSD__)
	return 1; // OpenBSD only supports ARMv7 CPUs that have NEON.
#elif defined(HAVE_ELF_AUX_INFO)
	unsigned long hasneon = 0;
	if (elf_aux_info(AT_HWCAP, (void *)&hasneon, (int)sizeof(hasneon)) != 0) {
		return 0;
	}
	return ((hasneon & HWCAP_NEON) == HWCAP_NEON);
#elif (defined(__LINUX__) || defined(__ANDROID__)) && defined(HAVE_GETAUXVAL)
	return ((getauxval(AT_HWCAP) & HWCAP_NEON) == HWCAP_NEON);
#elif defined(__LINUX__)
	return readProcAuxvForNeon();
#elif defined(__ANDROID__)
	// Use NDK cpufeatures to read either /proc/self/auxv or /proc/cpuinfo
	{
		AndroidCpuFamily cpu_family = android_getCpuFamily();
		if (cpu_family == ANDROID_CPU_FAMILY_ARM) {
			uint64_t cpu_features = android_getCpuFeatures();
			if ((cpu_features & ANDROID_CPU_ARM_FEATURE_NEON) != 0) {
				return 1;
			}
		}
		return 0;
	}
#else
#warning SDL_HasNEON is not implemented for this ARM platform. Write me.
	return 0;
#endif
}

#if defined(__e2k__)
inline int CPU_have3DNow(void) {
#if defined(__3dNOW__)
	return 1;
#else
	return 0;
#endif
}
#else
static int CPU_have3DNow(void) {
	if (CPU_CPUIDMaxFunction > 0) { /* that is, do we have CPUID at all? */
		int a, b, c, d;
		cpuid(0x80000000, a, b, c, d);
		if (a >= 0x80000001) {
			cpuid(0x80000001, a, b, c, d);
			return (d & 0x80000000);
		}
	}
	return 0;
}
#endif

#if defined(__e2k__)
#if defined(__MMX__)
#define CPU_haveMMX() (1)
#else
#define CPU_haveMMX() (0)
#endif
#if defined(__SSE__)
#define CPU_haveSSE() (1)
#else
#define CPU_haveSSE() (0)
#endif
#if defined(__SSE2__)
#define CPU_haveSSE2() (1)
#else
#define CPU_haveSSE2() (0)
#endif
#if defined(__SSE3__)
#define CPU_haveSSE3() (1)
#else
#define CPU_haveSSE3() (0)
#endif
#if defined(__SSE4_1__)
#define CPU_haveSSE41() (1)
#else
#define CPU_haveSSE41() (0)
#endif
#if defined(__SSE4_2__)
#define CPU_haveSSE42() (1)
#else
#define CPU_haveSSE42() (0)
#endif
#if defined(__AVX__)
#define CPU_haveAVX() (1)
#else
#define CPU_haveAVX() (0)
#endif
#else
#define CPU_haveMMX() (CPU_CPUIDFeatures[3] & 0x00800000)
#define CPU_haveSSE() (CPU_CPUIDFeatures[3] & 0x02000000)
#define CPU_haveSSE2() (CPU_CPUIDFeatures[3] & 0x04000000)
#define CPU_haveSSE3() (CPU_CPUIDFeatures[2] & 0x00000001)
#define CPU_haveSSE41() (CPU_CPUIDFeatures[2] & 0x00080000)
#define CPU_haveSSE42() (CPU_CPUIDFeatures[2] & 0x00100000)
#define CPU_haveAVX() (CPU_OSSavesYMM && (CPU_CPUIDFeatures[2] & 0x10000000))
#endif

#if defined(__e2k__)
inline int CPU_haveAVX2(void) {
#if defined(__AVX2__)
	return 1;
#else
	return 0;
#endif
}
#else
static int CPU_haveAVX2(void) {
	if (CPU_OSSavesYMM && (CPU_CPUIDMaxFunction >= 7)) {
		int a, b, c, d;
		(void)a;
		(void)b;
		(void)c;
		(void)d; // compiler warnings...
		cpuid(7, a, b, c, d);
		return (b & 0x00000020);
	}
	return 0;
}
#endif

#if defined(__e2k__)
inline int CPU_haveAVX512F(void) {
	return 0;
}
#else
static int CPU_haveAVX512F(void) {
	if (CPU_OSSavesZMM && (CPU_CPUIDMaxFunction >= 7)) {
		int a, b, c, d;
		(void)a;
		(void)b;
		(void)c;
		(void)d; // compiler warnings...
		cpuid(7, a, b, c, d);
		return (b & 0x00010000);
	}
	return 0;
}
#endif

static Uint32 SDL_CPUFeatures = 0xFFFFFFFF;
static Uint32 SDL_SIMDAlignment = 0xFFFFFFFF;

static Uint32 _GetCPUFeatures(void) {
	if (SDL_CPUFeatures == 0xFFFFFFFF) {
		CPU_calcCPUIDFeatures();
		SDL_CPUFeatures = 0;
		SDL_SIMDAlignment = sizeof(void *); // a good safe base value
		if (CPU_haveAltiVec()) {
			SDL_CPUFeatures |= CPU_HAS_ALTIVEC;
			SDL_SIMDAlignment = SDL_max(SDL_SIMDAlignment, 16);
		}
		if (CPU_haveMMX()) {
			SDL_CPUFeatures |= CPU_HAS_MMX;
			SDL_SIMDAlignment = SDL_max(SDL_SIMDAlignment, 8);
		}
		if (CPU_have3DNow()) {
			SDL_CPUFeatures |= CPU_HAS_3DNOW;
			SDL_SIMDAlignment = SDL_max(SDL_SIMDAlignment, 8);
		}
		if (CPU_haveSSE()) {
			SDL_CPUFeatures |= CPU_HAS_SSE;
			SDL_SIMDAlignment = SDL_max(SDL_SIMDAlignment, 16);
		}
		if (CPU_haveSSE2()) {
			SDL_CPUFeatures |= CPU_HAS_SSE2;
			SDL_SIMDAlignment = SDL_max(SDL_SIMDAlignment, 16);
		}
		if (CPU_haveSSE3()) {
			SDL_CPUFeatures |= CPU_HAS_SSE3;
			SDL_SIMDAlignment = SDL_max(SDL_SIMDAlignment, 16);
		}
		if (CPU_haveSSE41()) {
			SDL_CPUFeatures |= CPU_HAS_SSE41;
			SDL_SIMDAlignment = SDL_max(SDL_SIMDAlignment, 16);
		}
		if (CPU_haveSSE42()) {
			SDL_CPUFeatures |= CPU_HAS_SSE42;
			SDL_SIMDAlignment = SDL_max(SDL_SIMDAlignment, 16);
		}
		if (CPU_haveAVX()) {
			SDL_CPUFeatures |= CPU_HAS_AVX;
			SDL_SIMDAlignment = SDL_max(SDL_SIMDAlignment, 32);
		}
		if (CPU_haveAVX2()) {
			SDL_CPUFeatures |= CPU_HAS_AVX2;
			SDL_SIMDAlignment = SDL_max(SDL_SIMDAlignment, 32);
		}
		if (CPU_haveAVX512F()) {
			SDL_CPUFeatures |= CPU_HAS_AVX512F;
			SDL_SIMDAlignment = SDL_max(SDL_SIMDAlignment, 64);
		}
		if (CPU_haveARMSIMD()) {
			SDL_CPUFeatures |= CPU_HAS_ARM_SIMD;
			SDL_SIMDAlignment = SDL_max(SDL_SIMDAlignment, 16);
		}
		if (CPU_haveNEON()) {
			SDL_CPUFeatures |= CPU_HAS_NEON;
			SDL_SIMDAlignment = SDL_max(SDL_SIMDAlignment, 16);
		}
	}
	return SDL_CPUFeatures;
}

#define CPU_FEATURE_AVAILABLE(f) ((_GetCPUFeatures() & f) ? SDL_TRUE : SDL_FALSE)

extern "C" SDL_bool SDL_HasAltiVec(void) {
	return CPU_FEATURE_AVAILABLE(CPU_HAS_ALTIVEC);
}
extern "C" SDL_bool SDL_HasMMX(void) {
	return CPU_FEATURE_AVAILABLE(CPU_HAS_MMX);
}
extern "C" SDL_bool SDL_Has3DNow(void) {
	return CPU_FEATURE_AVAILABLE(CPU_HAS_3DNOW);
}
extern "C" SDL_bool SDL_HasSSE(void) {
	return CPU_FEATURE_AVAILABLE(CPU_HAS_SSE);
}
extern "C" SDL_bool SDL_HasSSE2(void) {
	return CPU_FEATURE_AVAILABLE(CPU_HAS_SSE2);
}
extern "C" SDL_bool SDL_HasSSE3(void) {
	return CPU_FEATURE_AVAILABLE(CPU_HAS_SSE3);
}
extern "C" SDL_bool SDL_HasSSE41(void) {
	return CPU_FEATURE_AVAILABLE(CPU_HAS_SSE41);
}
extern "C" SDL_bool SDL_HasSSE42(void) {
	return CPU_FEATURE_AVAILABLE(CPU_HAS_SSE42);
}
extern "C" SDL_bool SDL_HasAVX(void) {
	return CPU_FEATURE_AVAILABLE(CPU_HAS_AVX);
}
extern "C" SDL_bool SDL_HasAVX2(void) {
	return CPU_FEATURE_AVAILABLE(CPU_HAS_AVX2);
}
extern "C" SDL_bool SDL_HasAVX512F(void) {
	return CPU_FEATURE_AVAILABLE(CPU_HAS_AVX512F);
}
extern "C" SDL_bool SDL_HasARMSIMD(void) {
	return CPU_FEATURE_AVAILABLE(CPU_HAS_ARM_SIMD);
}
extern "C" SDL_bool SDL_HasNEON(void) {
	return CPU_FEATURE_AVAILABLE(CPU_HAS_NEON);
}

// Allocate memory in a SIMD-friendly way.

extern "C" size_t SDL_SIMDGetAlignment(void) {
	if (SDL_SIMDAlignment == 0xFFFFFFFF) {
		_GetCPUFeatures(); // make sure this has been calculated
	}
	DEV_ASSERT(SDL_SIMDAlignment != 0);
	return SDL_SIMDAlignment;
}

extern "C" void *SDL_SIMDAlloc(const size_t len) {
	const size_t alignment = SDL_SIMDGetAlignment();
	const size_t padding = alignment - (len % alignment);
	const size_t padded = (padding != alignment) ? (len + padding) : len;
	Uint8 *retval = NULL;
	Uint8 *ptr = (Uint8 *)SDL_malloc(padded + alignment + sizeof(void *));
	if (ptr) {
		// store the actual allocated pointer right before our aligned pointer.
		retval = ptr + sizeof(void *);
		retval += alignment - (((size_t)retval) % alignment);
		*(((void **)retval) - 1) = ptr;
	}
	return retval;
}

extern "C" void *SDL_SIMDRealloc(void *mem, const size_t len) {
	const size_t alignment = SDL_SIMDGetAlignment();
	const size_t padding = alignment - (len % alignment);
	const size_t padded = (padding != alignment) ? (len + padding) : len;
	Uint8 *retval = (Uint8 *)mem;
	void *oldmem = mem;
	size_t memdiff = 0, ptrdiff;
	Uint8 *ptr;

	if (mem) {
		void **realptr = (void **)mem;
		realptr--;
		mem = *(((void **)mem) - 1);
		// Check the delta between the real pointer and user pointer
		memdiff = ((size_t)oldmem) - ((size_t)mem);
	}

	ptr = (Uint8 *)SDL_realloc(mem, padded + alignment + sizeof(void *));

	if (ptr == NULL) {
		return NULL; // Out of memory, bail!
	}

	// Store the actual allocated pointer right before our aligned pointer.
	retval = ptr + sizeof(void *);
	retval += alignment - (((size_t)retval) % alignment);

	// Make sure the delta is the same!
	if (mem) {
		ptrdiff = ((size_t)retval) - ((size_t)ptr);
		if (memdiff != ptrdiff) { // Delta has changed, copy to new offset!
			oldmem = (void *)(((uintptr_t)ptr) + memdiff);
			// Even though the data past the old `len` is undefined, this is the
			// only length value we have, and it guarantees that we copy all the
			// previous memory anyhow.
			SDL_memmove(retval, oldmem, len);
		}
	}

	// Actually store the allocated pointer, finally.
	*(((void **)retval) - 1) = ptr;
	return retval;
}

extern "C" void SDL_SIMDFree(void *ptr) {
	if (ptr) {
		void **realptr = (void **)ptr;
		realptr--;
		SDL_free(*(((void **)ptr) - 1));
	}
}
