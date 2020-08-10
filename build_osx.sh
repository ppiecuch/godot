#!/bin/bash

set -e

# `DIR` contains the directory where the script is located
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GODOT_DIR=$DIR
TEMPLATES_DIR="$HOME/Library/ApplicationSupport/Godot/templates/"

# Utilities
# ---------

echo_header() {
	printf "\e[1;4m$1\e[0m\n"
}
echo_success() {
	printf "\e[1;4;32m$1\e[0m\n"
}

export -f echo_header
export -f echo_success


# Building
# --------

export SCONS_FLAGS="$SCONS_FLAGS CCFLAGS=-D__MACPORTS__"

echo_header "*** Building debug editor for macOS ..."
scons -j4 platform=osx define=DEBUG_ENABLED $SCONS_FLAGS

echo_header "*** Packaging app ..."
cp -rv "$GODOT_DIR/misc/dist/osx_tools.app" "$GODOT_DIR/bin/Godot-master.app"
mkdir -p "$GODOT_DIR/bin/Godot-master.app/Contents/MacOS"
cp -v "$GODOT_DIR/bin/godot.osx.tools.64" "$GODOT_DIR/bin/Godot-master.app/Contents/MacOS/Godot"

echo_success "*** Finished building editor for macOS."

if [ "$1" == "templates" ]; then
	echo_header "*** Building 64-bit debug export template for macOS ..."
	scons platform=osx bits=64 tools=no target=release_debug use_lto=yes $SCONS_FLAGS
	echo_header "*** Building 64-bit release export template for macOS ..."
	scons platform=osx bits=64 tools=no target=release use_lto=yes $SCONS_FLAGS
	strip "$GODOT_DIR/bin/godot.osx.opt.debug.64" "$GODOT_DIR/bin/godot.osx.opt.64"
	mv "$GODOT_DIR/bin/godot.osx.opt.debug.64" "$TEMPLATES_DIR"
	mv "$GODOT_DIR/bin/godot.osx.opt.64" "$TEMPLATES_DIR"

	echo_success "*** Finished building export templates for macOS."
fi
