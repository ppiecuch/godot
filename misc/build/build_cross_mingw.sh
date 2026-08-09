#!/bin/bash

# Cross-compile Godot for Windows using MinGW-w64 toolchain at /opt/mingw-w64/

set -e

# Logging
# -------

if [ -n "$BUILD_LOG" ]; then
	exec > >(tee -a "$BUILD_LOG") 2>&1
fi

# Utilities
# ---------

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

# Setup
# -----

MINGW_ROOT="/opt/mingw-w64"
MINGW_BIN="$MINGW_ROOT/bin"

if [ ! -d "$MINGW_BIN" ]; then
	log_error "MinGW-w64 toolchain not found at $MINGW_ROOT"
	exit 1
fi

export PATH="$MINGW_BIN:$PATH"

# Prefix for cross-compilation
export MINGW32_PREFIX="$MINGW_BIN/i686-w64-mingw32-"
export MINGW64_PREFIX="$MINGW_BIN/x86_64-w64-mingw32-"

CPU=$(nproc 2>/dev/null || sysctl -n hw.physicalcpu 2>/dev/null || echo 2)

GODOT_DIR="$PWD"

if [ -f build_info.config ]; then
	log_step "Loading build_info.config"
	. build_info.config
	sed -e 's/^/  /' build_info.config
fi

# Parse arguments
# ---------------

build_templates=""
if [ "$1" == "templates" ]; then
	build_templates="yes"
	shift
fi

target="${1:-release_debug}"
shift 2>/dev/null || true

bits="${MINGW_BITS:-64}"

export SCONS_FLAGS="$SCONS_FLAGS no_editor_splash=yes"

# Building
# --------

if [ "$build_templates" == "yes" ]; then
	log_step "Building Windows export templates (${bits}-bit, ${CPU} cores)"

	log_info "Building release_debug template..."
	scons -j$CPU platform=windows bits=$bits target=release_debug tools=no \
		mingw_prefix_32="$MINGW32_PREFIX" mingw_prefix_64="$MINGW64_PREFIX" \
		use_mingw=yes $SCONS_FLAGS "$@"

	log_info "Building release template..."
	scons -j$CPU platform=windows bits=$bits target=release tools=no \
		mingw_prefix_32="$MINGW32_PREFIX" mingw_prefix_64="$MINGW64_PREFIX" \
		use_mingw=yes $SCONS_FLAGS "$@"

	log_success "Export templates built"
else
	log_step "Building Windows editor (${bits}-bit, $target, ${CPU} cores)"

	scons -j$CPU platform=windows bits=$bits target=$target tools=yes \
		mingw_prefix_32="$MINGW32_PREFIX" mingw_prefix_64="$MINGW64_PREFIX" \
		use_mingw=yes $SCONS_FLAGS "$@"

	log_success "Editor built"
fi

log_info "Output in $GODOT_DIR/bin/"
ls -lh "$GODOT_DIR/bin/"godot.windows.* 2>/dev/null || true
