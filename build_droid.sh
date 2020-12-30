#!/bin/bash

set -e

# Config
# ------

export SCONS="scons -j${NUM_CORES} verbose=yes warnings=no progress=no"
export OPTIONS="debug_symbols=no debug_experimental=no"
export TERM=xterm

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

echo_header "*** Starting classical build for Android..."

$SCONS platform=android android_arch=armv7 $OPTIONS tools=no target=release_debug
$SCONS platform=android android_arch=armv7 $OPTIONS tools=no target=release

$SCONS platform=android android_arch=arm64v8 $OPTIONS tools=no target=release_debug
$SCONS platform=android android_arch=arm64v8 $OPTIONS tools=no target=release

$SCONS platform=android android_arch=x86 $OPTIONS tools=no target=release_debug
$SCONS platform=android android_arch=x86 $OPTIONS tools=no target=release

$SCONS platform=android android_arch=x86_64 $OPTIONS tools=no target=release_debug
$SCONS platform=android android_arch=x86_64 $OPTIONS tools=no target=release

pushd platform/android/java
./gradlew generateGodotTemplates
popd

echo_header "*** Android build successful"
