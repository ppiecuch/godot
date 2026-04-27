#!/bin/bash
# _update_h264.sh — Download OpenH264 pre-built binaries from Cisco's GitHub releases
#
# Usage:
#   ./_update_h264.sh              # Download latest (default: v2.6.0)
#   ./_update_h264.sh v2.5.0       # Download specific version
#
# Downloads platform-specific shared libraries and places them in deps/openh264/
# Also updates the include/ headers from the release.
#
# Repository: https://github.com/cisco/openh264

set -euo pipefail

VERSION="${1:-v2.6.0}"
VERSION_NUM="${VERSION#v}"  # Strip leading 'v'

REPO="https://github.com/cisco/openh264"
RELEASE_URL="${REPO}/releases/download/${VERSION}"

DEST_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "=== OpenH264 Update Script ==="
echo "Version: ${VERSION} (${VERSION_NUM})"
echo "Destination: ${DEST_DIR}"
echo ""

# Platform library downloads
# Format: filename -> local name
declare -A LIBS=(
    ["libopenh264-${VERSION_NUM}-linux64.7.so"]="libopenh264-${VERSION_NUM}-linux64.7.so"
    ["libopenh264-${VERSION_NUM}-linux32.7.so"]="libopenh264-${VERSION_NUM}-linux32.7.so"
    ["libopenh264-${VERSION_NUM}-osx-arm64.7.dylib"]="libopenh264-${VERSION_NUM}-osx-arm64.7.dylib"
    ["libopenh264-${VERSION_NUM}-osx-x64.7.dylib"]="libopenh264-${VERSION_NUM}-osx-x64.7.dylib"
    ["openh264-${VERSION_NUM}-win64.dll"]="openh264-${VERSION_NUM}-win64.dll"
    ["openh264-${VERSION_NUM}-win32.dll"]="openh264-${VERSION_NUM}-win32.dll"
    ["libopenh264-${VERSION_NUM}-android-arm64.7.so"]="libopenh264-${VERSION_NUM}-android-arm64.7.so"
    ["libopenh264-${VERSION_NUM}-android-arm.7.so"]="libopenh264-${VERSION_NUM}-android-arm.7.so"
    ["libopenh264-${VERSION_NUM}-android-x64.7.so"]="libopenh264-${VERSION_NUM}-android-x64.7.so"
    ["libopenh264-${VERSION_NUM}-android-x86.7.so"]="libopenh264-${VERSION_NUM}-android-x86.7.so"
)

# Cisco distributes as .bz2 compressed archives
TMPDIR=$(mktemp -d)
trap "rm -rf ${TMPDIR}" EXIT

download_lib() {
    local filename="$1"
    local url="${RELEASE_URL}/${filename}.bz2"
    local tmp_path="${TMPDIR}/${filename}.bz2"
    local dest_path="${DEST_DIR}/${filename}"

    echo -n "  Downloading ${filename}... "
    if curl -fsSL -o "${tmp_path}" "${url}" 2>/dev/null; then
        bunzip2 -f "${tmp_path}"
        mv "${TMPDIR}/${filename}" "${dest_path}"
        chmod +x "${dest_path}"
        echo "OK ($(du -h "${dest_path}" | cut -f1))"
        return 0
    else
        echo "SKIP (not available)"
        return 1
    fi
}

# Remove old library files
echo "Cleaning old libraries..."
rm -f "${DEST_DIR}"/libopenh264-*.so "${DEST_DIR}"/libopenh264-*.dylib "${DEST_DIR}"/openh264-*.dll
echo ""

# Download libraries
echo "Downloading libraries from ${RELEASE_URL}..."
SUCCESS=0
FAILED=0
for lib in "${!LIBS[@]}"; do
    if download_lib "${lib}"; then
        ((SUCCESS++))
    else
        ((FAILED++))
    fi
done

echo ""
echo "Downloaded: ${SUCCESS} libraries (${FAILED} skipped)"

# Download headers from source
echo ""
echo "Downloading headers..."
HEADER_URL="https://raw.githubusercontent.com/cisco/openh264/${VERSION}/codec/api/wels"
INCLUDE_DIR="${DEST_DIR}/include"
mkdir -p "${INCLUDE_DIR}"

for header in codec_api.h codec_app_def.h codec_def.h codec_ver.h; do
    echo -n "  ${header}... "
    if curl -fsSL -o "${INCLUDE_DIR}/${header}" "${HEADER_URL}/${header}" 2>/dev/null; then
        echo "OK"
    else
        echo "FAILED"
    fi
done

# Update version record
echo "${VERSION}" > "${DEST_DIR}/VERSION"

echo ""
echo "=== Done ==="
echo "OpenH264 ${VERSION} libraries and headers are in: ${DEST_DIR}"
echo ""
echo "NOTE: After updating, check GRCodec.cpp _get_search_paths() for the"
echo "      correct library filenames (version number in filename may have changed)."
echo ""
echo "NOTE: The SONAME version (.6 vs .7) may change between releases."
echo "      Update the filenames in GRCodec.cpp accordingly."
