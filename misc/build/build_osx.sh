#!/bin/bash

set -e

CPU=$(sysctl -n hw.physicalcpu)

if [ -z "$CPU" ]; then
	CPU=2
fi

# `START_DIR` contains the directory where the script is located
START_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GODOT_DIR=$PWD
TEMPLATES_DIR="$HOME/Library/Application Support/Godot/templates/"

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

if [ "$1" == "templates" ]; then
	build_templates="yes"
	shift
fi

if [ -z "$1" ]; then
	target="release_debug"
else
	target=$1
	shift
fi

if [ -z "$target" ]; then
	echo_header "*** Error: missing 'taget' info."
fi

A=$(uname -m)

if [ "${A}" = "x86_64" ]; then
	if [ "$(sysctl -in sysctl.proc_translated)" = "1" ]; then
		echo "(Running on Rosetta translation - force building native arm64)"
		A="arm64"
	fi
fi

echo_header "*** Building $target editor for macOS for architecture $A ..."
scons -j$CPU platform=osx arch=$A target=$target $SCONS_FLAGS

if [ -x "$(command -v gcp)" ]; then
	cp="gcp -u"
else
	cp="cp"
fi

echo_header "*** Packaging app ..."
rm -rf "$GODOT_DIR/bin/Godot-master.app"
$cp -rv "$GODOT_DIR/misc/dist/osx_tools.app" "$GODOT_DIR/bin/Godot-master.app"
mkdir -p "$GODOT_DIR/bin/Godot-master.app/Contents/MacOS"
$cp -v "$GODOT_DIR/bin/godot.osx.opt.tools.$A" "$GODOT_DIR/bin/Godot-master.app/Contents/MacOS/Godot"

echo_header "*** Signing executable for debugger ..."
codesign --verbose --deep --sign - --timestamp --entitlements "$GODOT_DIR/misc/dist/osx/editor.entitlements" "$GODOT_DIR/bin/Godot-master.app"

echo_success "*** Finished building editor for macOS."

if [ "$1" == "templates" ] || [ ! -z "$build_templates" ]; then
	echo_header "*** Building 64-bit release export template for macOS ..."

	scons -j$CPU platform=osx arch=x86_64 tools=no target=release lto=full $SCONS_FLAGS
	scons -j$CPU platform=osx arch=arm64 tools=no target=release lto=full $SCONS_FLAGS
	strip "$GODOT_DIR/bin/godot.osx.opt.x86_64"
	strip "$GODOT_DIR/bin/godot.osx.opt.arm64"
	lipo -create bin/godot.osx.opt.x86_64 bin/godot.osx.opt.arm64 -output bin/godot.osx.opt.64
	mv "$GODOT_DIR/bin/godot.osx.opt.64" "$TEMPLATES_DIR"
	rm "$GODOT_DIR/bin/godot.osx.opt.x86_64" "$GODOT_DIR/bin/godot.osx.opt.arm64"

	echo_success "*** Finished building export templates for macOS."
fi
