#!/bin/bash
# Build and run filamath standalone tests
# Usage: ./build_tests.sh [doctest flags]
#
# Examples:
#   ./build_tests.sh                          # run all tests
#   ./build_tests.sh -tc="*vec3*"             # run vec3 tests only
#   ./build_tests.sh -ts="filamath::quat"     # run quat test suite
#   ./build_tests.sh -ltc                     # list all test cases

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
FILAMATH_DIR="$SCRIPT_DIR/.."
GODOT_ROOT="$SCRIPT_DIR/../../../../.."
DOCTEST_DIR="$GODOT_ROOT/thirdparty"
BUILD_DIR="$SCRIPT_DIR/_build"

set -e

# Create a temporary include mapping so that #include <math/vec3.h>
# resolves to filamath/vec3.h (matching upstream filament layout).
mkdir -p "$BUILD_DIR"
ln -sfn "$FILAMATH_DIR" "$BUILD_DIR/math"

cleanup() { rm -rf "$BUILD_DIR"; }
trap cleanup EXIT

echo "Building filamath tests..."
${CXX:-c++} -o "$SCRIPT_DIR/filamath_tests" \
    -std=c++17 \
    -O1 \
    -I"$BUILD_DIR" \
    -I"$DOCTEST_DIR" \
    "$SCRIPT_DIR/test_main.cpp"

echo "Build successful. Running tests..."
"$SCRIPT_DIR/filamath_tests" "$@"
