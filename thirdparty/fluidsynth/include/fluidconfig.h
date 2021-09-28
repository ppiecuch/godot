#ifndef CONFIG_H
#define CONFIG_H

/* Define to activate sound output to files */
#define AUFILE_SUPPORT 1

/* Define if building for Mac OS X Darwin */
#ifdef __APPLE__
#define DARWIN 1
#define __LINUX__ 1
#endif

/* Soundfont to load automatically in some use cases */
#define DEFAULT_SOUNDFONT "default.sf2"

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define to 1 if you have the <fcntl.h> header file. */
#ifdef __LINUX__
#define HAVE_FCNTL_H 1
#endif

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <io.h> header file. */
#ifdef _WIN32
#define HAVE_IO_H
#endif

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if you have the <math.h> header file. */
#define HAVE_MATH_H 1

/* Define if compiling the mixer with multi-thread support */
#define ENABLE_MIXER_THREADS 1

/* Define to 1 if you have the <pthread.h> header file. */
#ifdef __LINUX__
#define HAVE_PTHREAD_H 1
#endif

/* Define to 1 if you have the <signal.h> header file. */
#ifdef __LINUX__
#define HAVE_SIGNAL_H 1
#endif

/* Define to 1 if you have the <stdarg.h> header file. */
#define HAVE_STDARG_H 1

/* Define to 1 if you have the <stdbool.h> header file. */
#define HAVE_STDBOOL_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdio.h> header file. */
#define HAVE_STDIO_H1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/mman.h> header file. */
#ifdef __LINUX__
#define HAVE_SYS_MMAN_H 1
#endif

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#ifdef __LINUX__
#define HAVE_UNISTD_H 1
#endif

/* Define to 1 if you have the <windows.h> header file. */
#ifdef _WIN32
#define HAVE_WINDOWS_H 1
#endif

/* Define if using the MinGW32 environment */
#undef MINGW32

/* Define to 1 if you have the ANSI C header files. */
#undef STDC_HEADERS

/* Define to do all DSP in single floating point precision */
#define WITH_FLOAT 1

/* Define to profile the DSP code */
#undef WITH_PROFILING

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
#if defined(__BIG_ENDIAN__)
#define WORDS_BIGENDIAN 1
#elif defined(__linux__)
#include <bits/endian.h>
#if __BYTE_ORDER == __BIG_ENDIAN
#define WORDS_BIGENDIAN 1
#endif
#else
#undef WORDS_BIGENDIAN
#endif

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#define inline inline
#endif

/* Define to 1 if you have the sinf() function. */
#define HAVE_SINF 1

/* Define to 1 if you have the cosf() function. */
#define HAVE_COSF 1

/* Define to 1 if you have the fabsf() function. */
#define HAVE_FABSF 1

/* Define to 1 if you have the powf() function. */
#define HAVE_POWF 1

/* Define to 1 if you have the sqrtf() function. */
#define HAVE_SQRTF 1

/* Define to 1 if you have the logf() function. */
#define HAVE_LOGF 1

#endif /* CONFIG_H */
