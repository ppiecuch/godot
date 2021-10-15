#!/bin/bash

set -e

# Config
# ------

CPU=2

if [[ "$OSTYPE" == "darwin"* ]]; then
	CPU=$(sysctl -n hw.physicalcpu)
elif [[ "$OSTYPE" == "linux"* ]]; then
	CPU=$(nproc)
fi

export ANDROID_SDK_ROOT=$HOME/Library/Android/sdk
export ANDROID_NDK_ROOT=$HOME/Library/Android/ndk

export SCONS="scons -j$CPU verbose=yes warnings=no progress=no"
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

if [ "$1" == "full" ]; then
	$SCONS platform=android android_arch=x86 $OPTIONS tools=no target=release_debug
	$SCONS platform=android android_arch=x86 $OPTIONS tools=no target=release

	$SCONS platform=android android_arch=x86_64 $OPTIONS tools=no target=release_debug
	$SCONS platform=android android_arch=x86_64 $OPTIONS tools=no target=release
fi

pushd platform/android/java
./gradlew generateGodotTemplates
popd

mkdir -p bin/templates/android

mv -v \
	bin/android_debug.apk bin/android_release.apk bin/godot-lib.debug.aar bin/godot-lib.release.aar \
	bin/templates/android/

# Look for platform plugins:
if [ -d "platform_plugins/android" ]; then
echo_header "*** Building platform plugins"
godot_lib="$(pwd)/platform/android/java/app/libs/release/godot-lib.release.aar"
(pushd "platform_plugins/android"
	for plugin in godot-google-play-billing godot-google-bluetooth godot-google-device-info; do
	(pushd $plugin
		if [ -e gd_build_plugin.sh ]; then
			./gd_build_plugin.sh "$godot_lib"
		else
			echo_header "*** Cannot find a gd_build_plugin.sh script for the plugin: $plugin"
			exit 1
		fi
	popd)
	done
popd)
fi

echo_header "*** Android build successful"
