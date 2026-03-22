// Standalone test runner for filamath (Google Filament math library).
// Provides doctest main() and includes the test source.

#define DOCTEST
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

// Include the test source with all TEST_CASE definitions.
#include "test_filamath.cpp"
