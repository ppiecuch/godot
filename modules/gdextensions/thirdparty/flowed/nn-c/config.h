#ifndef NN_C_CONFIG_H
#define NN_C_CONFIG_H

#if defined(_WIN32)
#define isnan _isnan
#define copysign _copysign
#define rint (int)
#define M_PI 3.14159265358979323846
#define TRILIBRARY
#define NO_TIMER
#endif

#define ANSI_DECLARATORS
#define TRILIBRARY

#endif /* NN_C_CONFIG_H */
