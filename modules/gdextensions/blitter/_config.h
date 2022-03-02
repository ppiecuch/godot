/// This is a set of defines to configure the SDL features

#ifndef _config_h_
#define _config_h_

/* General platform specific identifiers */
#include "_platform.h"

/* Useful headers */
#define STDC_HEADERS 1
#define HAVE_ALLOCA_H 1
#define HAVE_CTYPE_H 1
#define HAVE_FLOAT_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_LIMITS_H 1
#define HAVE_MALLOC_H 1
#define HAVE_MATH_H 1
#define HAVE_STRINGS_H 1
#define HAVE_STRING_H 1
#define HAVE_SYS_TYPES_H 1

/* Enable assembly routines */
#define SDL_ASSEMBLY_ROUTINES 1
#ifdef CPU_POWERPC
# define SDL_ALTIVEC_BLITTERS 1
#else
# define SDL_ALTIVEC_BLITTERS 0
#endif
#ifdef CPU_ARM
# define SDL_ARM_SIMD_BLITTERS 1
# define SDL_ARM_NEON_BLITTERS 1
#else
# define SDL_ARM_SIMD_BLITTERS 0
# define SDL_ARM_NEON_BLITTERS 0
#endif

#endif /* _config_h_ */
