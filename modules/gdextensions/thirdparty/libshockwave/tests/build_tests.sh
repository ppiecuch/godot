#!/bin/bash
# Build and run libshockwave tests
# Usage: ./build_tests.sh [doctest flags]

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
LIB_DIR="$SCRIPT_DIR/.."
GODOT_ROOT="$SCRIPT_DIR/../../../../.."
DOCTEST_DIR="$GODOT_ROOT/thirdparty"

set -e

echo "Building libshockwave tests..."
g++ -o swf_tests \
    -std=c++11 \
    -Wno-deprecated \
    -D_7ZIP_ST \
    -DLIBSHOCKWAVE_STANDALONE \
    -DLIBSHOCKWAVE_DISABLE_ZLIB \
    -DLIBSHOCKWAVE_DISABLE_LZMA \
    -I"$LIB_DIR/.." \
    -I"$DOCTEST_DIR" \
    -I"$GODOT_ROOT" \
    swf_parser.test.cpp

echo "Build successful. Running tests..."
./swf_tests "$@"
