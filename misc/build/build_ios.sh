#!/bin/bash

set -e

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
		printf "*** $1\n"
	fi
}
echo_bold() {
	if [[ "$TERM" =~ "xterm" ]]; then
		printf "\e[1m$1\e[0m\n"
	else
		printf "** $1 **\n"
	fi
}

export -f echo_header
export -f echo_success

# Config
# ------

CPU=$(sysctl -n hw.physicalcpu)

export SCONS="scons -j$CPU verbose=yes warnings=no progress=no"
# release_debug variant gets debug symbols so Xcode can extract a .dSYM at archive time.
# release variant strips them for minimum binary size.
export OPTIONS_DEBUG="debug_symbols=yes"
export OPTIONS_RELEASE="debug_symbols=no"

export IOS_SDK="14.2"
export IOS_LIPO="xcrun lipo"

# Classical

echo_header "*** Starting classical build for iOS..."

$SCONS platform=iphone $OPTIONS_DEBUG   arch=arm64 tools=no target=release_debug
$SCONS platform=iphone $OPTIONS_RELEASE arch=arm64 tools=no target=release

$SCONS platform=iphone $OPTIONS_DEBUG   arch=x86_64 ios_simulator=yes tools=no target=release_debug
$SCONS platform=iphone $OPTIONS_RELEASE arch=x86_64 ios_simulator=yes tools=no target=release

mkdir -p bin/templates/ios

$IOS_LIPO -create bin/libgodot.iphone.opt.arm64.a bin/libgodot.iphone.opt.x86_64.simulator.a -output bin/templates/ios/libgodot.iphone.opt.a
$IOS_LIPO -create bin/libgodot.iphone.opt.debug.arm64.a bin/libgodot.iphone.opt.debug.x86_64.simulator.a -output bin/templates/ios/libgodot.iphone.opt.debug.a

rm -v \
	bin/libgodot.iphone.opt.arm64.a bin/libgodot.iphone.opt.x86_64.simulator.a \
	bin/libgodot.iphone.opt.debug.arm64.a bin/libgodot.iphone.opt.debug.x86_64.simulator.a

# dSYM note: static libraries (.a) do not produce .dSYM files directly.
# Debug symbols are embedded in libgodot.iphone.opt.debug.a (built with debug_symbols=yes).
# Xcode extracts them and generates a .dSYM bundle automatically when archiving the .app.
# To use: in Xcode export preset set DEBUG_INFORMATION_FORMAT = dwarf-with-dsym.

# Look for platform plugins:
if [ -d "platform_plugins/ios" ]; then
	echo_header "*** Building platform plugins"
	(pushd "platform_plugins/ios"
		for plugin in fcuuid impact apn arkit camera icloud gamecenter photo_picker; do
			scons target=release_debug arch=arm64 simulator=no plugin=$plugin version=3.x
		done
	popd)
fi

echo_header "*** iOS build successful"
