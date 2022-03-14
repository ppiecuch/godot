#ifndef _DEBUG_H
#define _DEBUG_H

/* conditional debugmsg() trace */
#ifdef _DEBUG
#include <stdio.h>
#define debugmsg printf
#else
static inline void debugmsg(const char* format, ...) {}
#endif

#endif
