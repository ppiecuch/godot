#!/bin/bash

# build_frt.sh - Cross-compile FRT (Godot 3.x) templates for aarch64-linux on macOS
#
# Uses our macOS-native cross-compiler (GCC 15.x, glibc 2.28) with the
# Godot buildroot SDK sysroot and SDL2 cross-compiled from source.
#
# This script builds TEMPLATES ONLY (tools=no) — never the editor.
#
# Usage:
#   ./misc/build/build_frt.sh [release|release_debug|all]   (default: all)
#
# Run from the Godot repo root.

set -e

# ---------------------------------------------------------------------------
# Logging
# ---------------------------------------------------------------------------

if [ -n "$BUILD_LOG" ]; then
	exec > >(tee -a "$BUILD_LOG") 2>&1
fi

_color_supported() {
	[[ -z "$BUILD_LOG" ]] && [[ -t 1 ]] && [[ "$TERM" =~ (xterm|screen|tmux|alacritty|kitty) ]]
}

log_step() {
	if _color_supported; then
		printf "\e[1;34m▸\e[0m \e[1m%s\e[0m\n" "$1"
	else
		printf "==> %s\n" "$1"
	fi
}

log_info() {
	if _color_supported; then
		printf "  \e[90m%s\e[0m\n" "$1"
	else
		printf "    %s\n" "$1"
	fi
}

log_success() {
	if _color_supported; then
		printf "\e[1;32m✓\e[0m \e[1m%s\e[0m\n" "$1"
	else
		printf "[OK] %s\n" "$1"
	fi
}

log_error() {
	if _color_supported; then
		printf "\e[1;31m✗\e[0m \e[1m%s\e[0m\n" "$1" >&2
	else
		printf "[ERROR] %s\n" "$1" >&2
	fi
}

# ---------------------------------------------------------------------------
# Source shared functions
# ---------------------------------------------------------------------------

START_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GODOT_DIR="$PWD"

if [ -f "${START_DIR}/build_functions.sh" ]; then
	source "${START_DIR}/build_functions.sh"
	CPU=$(_get_cpu)
else
	CPU=2
	if command -v getconf &>/dev/null; then
		CPU=$(getconf _NPROCESSORS_ONLN)
	elif [[ "$OSTYPE" == "darwin"* ]]; then
		CPU=$(sysctl -n hw.physicalcpu)
	fi
fi

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

BUILD_TARGET="${1:-all}"

TOOLCHAINS_DIR="/Volumes/WORKSPACE/build-private/macos-cross-toolchains"
TOOLCHAIN_ARCHIVE="${TOOLCHAINS_DIR}/output/aarch64-unknown-linux-gnu-aarch64-darwin.tar.gz"

WORK_DIR="${GODOT_DIR}/build/frt-cross"
SOURCES_DIR="${GODOT_DIR}/build/src"
OUTPUT_DIR="${GODOT_DIR}/bin"

TRIPLE="aarch64-linux-gnu"
OUR_TRIPLE="aarch64-unknown-linux-gnu"

SDL2_VERSION="2.32.10"
SDK_URL="https://github.com/godotengine/buildroot/releases/download/godot-2023.08.x-4/aarch64-godot-linux-gnu_sdk-buildroot.tar.bz2"
SDL2_URL="https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/SDL2-${SDL2_VERSION}.tar.gz"

NJOBS="${CPU}"

# Ensure scons is on PATH (MacPorts, Homebrew, etc.)
if ! command -v scons &>/dev/null; then
	export PATH="${PATH}:/opt/local/bin:/opt/macports/bin:/opt/macports/Library/Frameworks/Python.framework/Versions/Current/bin"
fi
if ! command -v scons &>/dev/null; then
	log_error "scons not found. Install it first."
	exit 1
fi

# ---------------------------------------------------------------------------
# Preflight
# ---------------------------------------------------------------------------

if [ ! -f "${GODOT_DIR}/platform/frt/detect.py" ]; then
	log_error "platform/frt/detect.py not found. Run from the Godot repo root."
	exit 1
fi

if [ ! -f "${TOOLCHAIN_ARCHIVE}" ]; then
	log_error "Toolchain archive not found: ${TOOLCHAIN_ARCHIVE}"
	exit 1
fi

if [ "$(uname)" != "Darwin" ]; then
	log_error "This script is designed for macOS cross-compilation."
	exit 1
fi

mkdir -p "${WORK_DIR}" "${SOURCES_DIR}"

# ---------------------------------------------------------------------------
# Step 1: Download Godot Buildroot SDK
# ---------------------------------------------------------------------------

SDK_ARCHIVE="${SOURCES_DIR}/godot-sdk-aarch64.tar.bz2"
SDK_DIR="${WORK_DIR}/godot-sdk"

if [ ! -d "${SDK_DIR}/aarch64-godot-linux-gnu_sdk-buildroot" ]; then
	log_step "Downloading Godot Buildroot SDK..."
	if [ ! -f "${SDK_ARCHIVE}" ]; then
		curl -fSL -o "${SDK_ARCHIVE}" "${SDK_URL}"
	else
		log_info "Using cached SDK archive"
	fi

	log_step "Extracting Godot SDK..."
	mkdir -p "${SDK_DIR}"
	tar xjf "${SDK_ARCHIVE}" -C "${SDK_DIR}"

	log_step "Relocating Godot SDK (macOS-compatible)..."
	cd "${SDK_DIR}/aarch64-godot-linux-gnu_sdk-buildroot"
	# The SDK's relocate-sdk.sh uses Linux sed -i syntax which fails on macOS.
	# Do the relocation manually with BSD-compatible sed.
	LOCFILE="share/buildroot/sdk-location"
	OLDPATH="$(cat "${LOCFILE}")"
	NEWPATH="${PWD}"
	if [ "${NEWPATH}" != "${OLDPATH}" ]; then
		export LC_ALL=C
		grep -lr "${OLDPATH}" . | while read -r FILE; do
			if file -b --mime-type "${FILE}" | grep -q '^text/' && [ "${FILE}" != "${LOCFILE}" ]; then
				sed -i '' "s|${OLDPATH}|${NEWPATH}|g" "${FILE}"
			fi
		done
		sed -i '' "s|${OLDPATH}|${NEWPATH}|g" "${LOCFILE}"
	fi
	cd "${GODOT_DIR}"
else
	log_info "Godot SDK already extracted"
fi

GODOT_SYSROOT="${SDK_DIR}/aarch64-godot-linux-gnu_sdk-buildroot/aarch64-godot-linux-gnu/sysroot"

# ---------------------------------------------------------------------------
# Step 2: Extract our toolchain
# ---------------------------------------------------------------------------

TOOLCHAIN_DIR="${WORK_DIR}/toolchain"

if [ ! -d "${TOOLCHAIN_DIR}/bin" ]; then
	log_step "Extracting cross-compilation toolchain..."
	mkdir -p "${TOOLCHAIN_DIR}"
	tar xzf "${TOOLCHAIN_ARCHIVE}" -C "${TOOLCHAIN_DIR}" --strip-components=1
else
	log_info "Toolchain already extracted"
fi

OUR_SYSROOT="${TOOLCHAIN_DIR}/${OUR_TRIPLE}/sysroot"

# ---------------------------------------------------------------------------
# Step 3: Merge Godot SDK sysroot into our toolchain
# ---------------------------------------------------------------------------

MERGE_STAMP="${WORK_DIR}/.sysroot-merged"

if [ ! -f "${MERGE_STAMP}" ]; then
	log_step "Merging Godot SDK sysroot into our toolchain..."
	rsync -a --ignore-existing "${GODOT_SYSROOT}/usr/include/" "${OUR_SYSROOT}/usr/include/"
	rsync -a --ignore-existing "${GODOT_SYSROOT}/usr/lib/" "${OUR_SYSROOT}/usr/lib/"
	# Also merge lib64 if present (some packages install there)
	if [ -d "${GODOT_SYSROOT}/usr/lib64" ]; then
		rsync -a --ignore-existing "${GODOT_SYSROOT}/usr/lib64/" "${OUR_SYSROOT}/usr/lib64/"
	fi
	touch "${MERGE_STAMP}"
else
	log_info "Sysroot already merged"
fi

# ---------------------------------------------------------------------------
# Step 3b: Install EGL stub headers and library
# ---------------------------------------------------------------------------
# Godot core (drivers/gles2/rasterizer_gles2.cpp) includes <EGL/egl.h> and
# calls eglGetProcAddress() in its CAN_DEBUG path. FRT dlopen's EGL at
# runtime, but the compiler and linker need these at build time.
# The Godot buildroot SDK ships GLES2/GLES3/KHR but not EGL headers.

EGL_STAMP="${WORK_DIR}/.egl-stub-installed"

if [ ! -f "${EGL_STAMP}" ]; then
	log_step "Installing EGL stub headers and library..."
	mkdir -p "${OUR_SYSROOT}/usr/include/EGL"

	cat > "${OUR_SYSROOT}/usr/include/EGL/egl.h" << 'EGLEOF'
/* Minimal EGL header for cross-compilation (FRT dlopen's EGL at runtime) */
#ifndef __egl_h_
#define __egl_h_

#include <EGL/eglplatform.h>
#include <KHR/khrplatform.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int EGLBoolean;
typedef unsigned int EGLenum;
typedef void *EGLConfig;
typedef void *EGLContext;
typedef void *EGLDisplay;
typedef void *EGLSurface;
typedef void *EGLClientBuffer;
typedef int32_t EGLint;

typedef void *EGLNativeDisplayType;
typedef void *EGLNativePixmapType;
typedef void *EGLNativeWindowType;

#define EGL_DEFAULT_DISPLAY ((EGLNativeDisplayType)0)
#define EGL_NO_CONTEXT ((EGLContext)0)
#define EGL_NO_DISPLAY ((EGLDisplay)0)
#define EGL_NO_SURFACE ((EGLSurface)0)

#define EGL_OPENGL_ES_API 0x30A0
#define EGL_OPENGL_ES2_BIT 0x0004

typedef void (*__eglMustCastToProperFunctionPointerType)(void);
__eglMustCastToProperFunctionPointerType eglGetProcAddress(const char *procname);

#ifdef __cplusplus
}
#endif

#endif /* __egl_h_ */
EGLEOF

	cat > "${OUR_SYSROOT}/usr/include/EGL/eglplatform.h" << 'PLATEOF'
#ifndef __eglplatform_h_
#define __eglplatform_h_
#include <KHR/khrplatform.h>
typedef khronos_int32_t EGLint;
#endif
PLATEOF

	cat > "${OUR_SYSROOT}/usr/include/EGL/eglext.h" << 'EXTEOF'
#ifndef __eglext_h_
#define __eglext_h_
#include <EGL/egl.h>
#ifdef __cplusplus
extern "C" {
#endif
#ifndef EGL_KHR_debug
#define EGL_KHR_debug 1
typedef void *EGLObjectKHR;
typedef void *EGLLabelKHR;
typedef void (KHRONOS_APIENTRY *EGLDEBUGPROCKHR)(EGLenum error, const char *command, EGLint messageType, EGLLabelKHR threadLabel, EGLLabelKHR objectLabel, const char *message);
#define EGL_OBJECT_THREAD_KHR 0x33B0
#define EGL_OBJECT_DISPLAY_KHR 0x33B1
#define EGL_DEBUG_MSG_CRITICAL_KHR 0x33B9
#define EGL_DEBUG_MSG_ERROR_KHR 0x33BA
#define EGL_DEBUG_MSG_WARN_KHR 0x33BB
#define EGL_DEBUG_MSG_INFO_KHR 0x33BC
typedef EGLint (* PFNEGLDEBUGMESSAGECONTROLKHRPROC)(EGLDEBUGPROCKHR callback, const EGLint *attrib_list);
#endif
#ifdef __cplusplus
}
#endif
#endif
EXTEOF

	# Create symlinks for the toolchain first
	SYMLINK_DIR="${WORK_DIR}/bin-symlinks"
	mkdir -p "${SYMLINK_DIR}"
	for tool in gcc g++ gcc-ar gcc-ranlib ld as strip objcopy objdump readelf; do
		if [ -f "${TOOLCHAIN_DIR}/bin/${OUR_TRIPLE}-${tool}" ]; then
			ln -sf "${TOOLCHAIN_DIR}/bin/${OUR_TRIPLE}-${tool}" "${SYMLINK_DIR}/${TRIPLE}-${tool}"
		fi
	done

	# Build stub libEGL.so — provides eglGetProcAddress symbol for linking
	cat > /tmp/egl_stub.c << 'STUBEOF'
void *eglGetProcAddress(const char *procname) { return 0; }
STUBEOF
	PATH="${SYMLINK_DIR}:${TOOLCHAIN_DIR}/bin:${PATH}" \
	${TRIPLE}-gcc -shared -o "${OUR_SYSROOT}/usr/lib/libEGL.so" /tmp/egl_stub.c \
		--sysroot="${OUR_SYSROOT}"
	rm -f /tmp/egl_stub.c

	touch "${EGL_STAMP}"
else
	log_info "EGL stub already installed"
fi

# ---------------------------------------------------------------------------
# Step 4: Cross-compile SDL2 from source
# ---------------------------------------------------------------------------

SDL2_PREFIX="${WORK_DIR}/sdl2/arm64"
SDL2_ARCHIVE="${SOURCES_DIR}/SDL2-${SDL2_VERSION}.tar.gz"

if [ ! -f "${SDL2_PREFIX}/lib/libSDL2.so" ]; then
	log_step "Downloading SDL2 ${SDL2_VERSION}..."
	if [ ! -f "${SDL2_ARCHIVE}" ]; then
		curl -fSL -o "${SDL2_ARCHIVE}" "${SDL2_URL}"
	else
		log_info "Using cached SDL2 archive"
	fi

	log_step "Cross-compiling SDL2..."
	SDL2_SRC="${WORK_DIR}/SDL2-${SDL2_VERSION}"
	if [ ! -d "${SDL2_SRC}" ]; then
		tar xzf "${SDL2_ARCHIVE}" -C "${WORK_DIR}"
	fi

	mkdir -p "${WORK_DIR}/sdl2-obj"
	cd "${WORK_DIR}/sdl2-obj"

	# Use our toolchain to build SDL2
	# Our binaries are named aarch64-unknown-linux-gnu-*, but SDL2 configure
	# needs --host= and looks for <host>-gcc. Create symlinks with the
	# standard triple naming.
	SYMLINK_DIR="${WORK_DIR}/bin-symlinks"
	mkdir -p "${SYMLINK_DIR}"
	for tool in gcc g++ gcc-ar gcc-ranlib ld as strip objcopy objdump readelf; do
		if [ -f "${TOOLCHAIN_DIR}/bin/${OUR_TRIPLE}-${tool}" ]; then
			ln -sf "${TOOLCHAIN_DIR}/bin/${OUR_TRIPLE}-${tool}" "${SYMLINK_DIR}/${TRIPLE}-${tool}"
		fi
	done
	# Also link cc -> gcc
	ln -sf "${TOOLCHAIN_DIR}/bin/${OUR_TRIPLE}-gcc" "${SYMLINK_DIR}/${TRIPLE}-cc"

	PATH="${SYMLINK_DIR}:${TOOLCHAIN_DIR}/bin:${PATH}" \
	"${SDL2_SRC}/configure" \
		--prefix="${SDL2_PREFIX}" \
		--host="${TRIPLE}" \
		--disable-video-wayland \
		--disable-video-x11 \
		--disable-video-opengl \
		--enable-video-opengles2 \
		--enable-video-kmsdrm \
		--disable-pulseaudio \
		--disable-jack \
		--disable-pipewire \
		CFLAGS="--sysroot=${OUR_SYSROOT}" \
		LDFLAGS="--sysroot=${OUR_SYSROOT}"

	PATH="${SYMLINK_DIR}:${TOOLCHAIN_DIR}/bin:${PATH}" \
	make -j${NJOBS}

	PATH="${SYMLINK_DIR}:${TOOLCHAIN_DIR}/bin:${PATH}" \
	make install

	cd "${GODOT_DIR}"
	rm -rf "${WORK_DIR}/sdl2-obj"

	log_success "SDL2 ${SDL2_VERSION} built and installed to ${SDL2_PREFIX}"
else
	log_info "SDL2 already built"
fi

# ---------------------------------------------------------------------------
# Step 5: Set up cross pkg-config and toolchain wrappers
# ---------------------------------------------------------------------------

BIN_DIR="${WORK_DIR}/bin"
mkdir -p "${BIN_DIR}"

# pkg-config wrapper for FRT's configure_cross()
cat > "${BIN_DIR}/${TRIPLE}-pkg-config" << PKGEOF
#!/bin/sh
export PKG_CONFIG_LIBDIR="${SDL2_PREFIX}/lib/pkgconfig:${OUR_SYSROOT}/usr/lib/pkgconfig"
export PKG_CONFIG_SYSROOT_DIR=""
export PKG_CONFIG_PATH=""
exec pkg-config "\$@"
PKGEOF
chmod +x "${BIN_DIR}/${TRIPLE}-pkg-config"

# Symlink toolchain binaries with the standard triple that FRT expects
# (configure_cross sets CC = aarch64-linux-gnu-gcc, etc.)
SYMLINK_DIR="${WORK_DIR}/bin-symlinks"
mkdir -p "${SYMLINK_DIR}"
for tool in gcc g++ gcc-ar gcc-ranlib ld as strip objcopy objdump readelf; do
	if [ -f "${TOOLCHAIN_DIR}/bin/${OUR_TRIPLE}-${tool}" ]; then
		ln -sf "${TOOLCHAIN_DIR}/bin/${OUR_TRIPLE}-${tool}" "${SYMLINK_DIR}/${TRIPLE}-${tool}"
	fi
done

# ---------------------------------------------------------------------------
# Step 6: Build FRT templates with SCons (templates only, never editor)
# ---------------------------------------------------------------------------

export PATH="${SDL2_PREFIX}/bin:${BIN_DIR}:${SYMLINK_DIR}:${TOOLCHAIN_DIR}/bin:${PATH}"
export BUILD_NAME=frt

FRT_OPTIONS="platform=frt tools=no frt_arch=arm64v8 frt_cross=auto use_static_cpp=yes module_gdextensions_enabled=no"
PRODUCTION_OPTIONS="verbose=yes warnings=no progress=no production=yes"
SCONS_ARGS="${FRT_OPTIONS} ${PRODUCTION_OPTIONS}"

cd "${GODOT_DIR}"

build_frt_target() {
	local target="$1"
	log_step "Building FRT ${target} template for aarch64 (using ${NJOBS} CPUs)..."
	scons ${SCONS_ARGS} target="${target}" -j${NJOBS}
}

case "${BUILD_TARGET}" in
	release)
		build_frt_target release
		;;
	release_debug)
		build_frt_target release_debug
		;;
	all)
		build_frt_target release
		build_frt_target release_debug
		;;
	*)
		log_error "Unknown target: ${BUILD_TARGET} (use release, release_debug, or all)"
		exit 1
		;;
esac

# ---------------------------------------------------------------------------
# Step 7: Strip and verify
# ---------------------------------------------------------------------------

log_step "Stripping binaries..."

for bin in "${OUTPUT_DIR}"/godot.frt.*.arm64v8; do
	if [ -f "$bin" ]; then
		"${SYMLINK_DIR}/${TRIPLE}-strip" "$bin"
		log_info "Stripped: $(basename "$bin")"
	fi
done

log_step "Verifying binaries..."

for bin in "${OUTPUT_DIR}"/godot.frt.*.arm64v8; do
	if [ -f "$bin" ]; then
		log_info "$(file "$bin")"
		log_info "$(${SYMLINK_DIR}/${TRIPLE}-readelf -p .comment "$bin" 2>/dev/null | grep GCC || true)"
	fi
done

log_success "FRT cross-compilation complete"
log_info "Binaries in: ${OUTPUT_DIR}/"
ls -lh "${OUTPUT_DIR}"/godot.frt.*.arm64v8 2>/dev/null || true
