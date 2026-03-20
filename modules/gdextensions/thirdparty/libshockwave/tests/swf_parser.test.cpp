// Standalone test runner for libshockwave
// Tests are embedded in the library source files (swfparser.cpp, swfvm.cpp)
// guarded by #ifdef DOCTEST. This file provides the test main and includes.

#ifndef LIBSHOCKWAVE_STANDALONE
#define LIBSHOCKWAVE_STANDALONE
#endif
#ifndef _FORCE_INLINE_
#define _FORCE_INLINE_ inline
#endif
#define DOCTEST
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

// Include the library source files with tests embedded
#include "../swfparser.cpp"
#include "../swfvm.cpp"
