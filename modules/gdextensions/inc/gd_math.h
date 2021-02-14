#ifndef GD_MATH_H
#define GD_MATH_H

#include "core/math/math_defs.h"
#include "core/math/math_funcs.h"

constexpr real_t Math_PI_by_180 = Math_PI / 180;
constexpr real_t Math_180_by_PI = 180 / Math_PI;

#define Math_Half_PI 1.57079632679
#define Math_Two_PI  6.28318530717958647692
#define Math_PI_Deg  0.0174532925
#define Math_Deg_PI  57.2957795130

#define Degrees_To_Radians(a) ((a) * Math_PI_Deg)
#define Radians_To_Degree(a)  ((a) * Math_Deg_PI)

#if defined __STRICT_ANSI__
# define _hypot(x,y) Math::sqrt((x)*(x)+(y)*(y))
#else
#ifdef REAL_T_IS_DOUBLE
static inline double _hypot(double x, double y) { return hypot(x, y); }
#else
static inline float _hypot(float x, float y) { return hypotf(x, y); }
#endif
#endif

#endif // GD_MATH_H
