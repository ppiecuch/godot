#!/bin/bash

set -e

CPU=$(sysctl -n hw.physicalcpu)
if [ -z "$CPU" ]; then
	CPU=2
fi

# `START_DIR` contains the directory where the script is located
START_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GODOT_DIR=$PWD
TEMPLATES_DIR="$HOME/Library/ApplicationSupport/Godot/templates/"

if ! command -v scons &> /dev/null
then
	export PATH=$PATH:/opt/local/bin
fi

# Utilities
# ---------

echo_header() {
	if [[ "$TERM" =~ "xterm" ]]; then
		printf "\e[1;4m$1\e[0m\n"
	else
		printf "[$1]\n"
	fi
}
echo_success() {
	if [[ "$TERM" =~ "xterm" ]]; then
		printf "\e[1;4;32m$1\e[0m\n"
	else
		printf "[$1]\n"
	fi
}

export -f echo_header
export -f echo_success


# Building
# --------

export SCONS_FLAGS="$SCONS_FLAGS no_editor_splash=yes CCFLAGS=-D__MACPORTS__"

if [ -z "$1" ]; then
	target="release_debug"
else
	target=$1
fi

if [ -z "$target" ]; then
	echo_header "*** Error: missing 'taget' info."
fi

echo_header "*** Building $target editor for macOS ..."
scons -j$CPU platform=osx $SCONS_FLAGS

if [ -x "$(command -v gcp)" ]; then
	cp="gcp -u"
else
	cp="cp"
fi

echo_header "*** Packaging app ..."
$cp -rv "$GODOT_DIR/misc/dist/osx_tools.app" "$GODOT_DIR/bin/Godot-master.app"
mkdir -p "$GODOT_DIR/bin/Godot-master.app/Contents/MacOS"
$cp -v "$GODOT_DIR/bin/godot.osx.tools.64" "$GODOT_DIR/bin/Godot-master.app/Contents/MacOS/Godot"

echo_success "*** Finished building editor for macOS."

if [ "$1" == "templates" ]; then
	echo_header "*** Building 64-bit debug export template for macOS ..."
	scons -j$CPU platform=osx bits=64 tools=no target=release_debug use_lto=yes $SCONS_FLAGS
	echo_header "*** Building 64-bit release export template for macOS ..."
	scons -j$CPU platform=osx bits=64 tools=no target=release use_lto=yes $SCONS_FLAGS
	strip "$GODOT_DIR/bin/godot.osx.opt.debug.64" "$GODOT_DIR/bin/godot.osx.opt.64"
	mv "$GODOT_DIR/bin/godot.osx.opt.debug.64" "$TEMPLATES_DIR"
	mv "$GODOT_DIR/bin/godot.osx.opt.64" "$TEMPLATES_DIR"

	echo_success "*** Finished building export templates for macOS."
fi
