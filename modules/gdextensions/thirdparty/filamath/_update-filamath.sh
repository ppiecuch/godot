#!/bin/bash
#
# Update script for Google Filament math library (filamath).
# Extracts libs/math and libs/mathio from https://github.com/google/filament.git
#
# Usage: ./_update-filamath.sh
#
# Directory layout:
#   filamath/
#     *.h           <- libs/math/include/math/*.h (all headers)
#     io/           <- libs/mathio (ostream support)
#       ostream.h   <- libs/mathio/include/mathio/ostream.h
#       ostream.cpp <- libs/mathio/src/ostream.cpp
#     tests/        <- doctest-based tests (not overwritten)
#

set -e

REPO_URL="https://github.com/google/filament.git"
TEMP_DIR="_filament_tmp"

trap "{ if [ -d '$TEMP_DIR' ]; then rm -rf $TEMP_DIR; fi; exit 255; }" SIGINT SIGTERM ERR EXIT

echo "==> Cleanup old temp."
rm -rf "$TEMP_DIR"

echo "==> Cloning filament (sparse, depth=1)."
git clone --depth=1 --filter=blob:none --sparse "$REPO_URL" "$TEMP_DIR"
pushd "$TEMP_DIR" > /dev/null
git sparse-checkout set libs/math libs/mathio
popd > /dev/null

echo "==> Copying math headers."
# Remove old headers (but preserve tests/ and io/ and this script).
find . -maxdepth 1 -name '*.h' -delete

cp "$TEMP_DIR"/libs/math/include/math/*.h .

echo "==> Copying mathio."
mkdir -p io
cp "$TEMP_DIR"/libs/mathio/include/mathio/ostream.h io/ostream.h
cp "$TEMP_DIR"/libs/mathio/src/ostream.cpp io/ostream.cpp

echo "==> Cleanup temp."
rm -rf "$TEMP_DIR"

echo "==> Verifying."
echo "Headers:"
ls -1 *.h | wc -l | xargs printf "  %s header files\n"
echo "IO files:"
ls -1 io/* | wc -l | xargs printf "  %s io files\n"

echo "==> Done. Filament math library updated."
